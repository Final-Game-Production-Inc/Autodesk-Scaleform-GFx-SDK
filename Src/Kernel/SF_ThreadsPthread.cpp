/**************************************************************************

Filename    :   SF_ThreadsPthread.cpp
Platform    :   Unix, Linux, MacOS
Content     :   pthreads support
Created     :   May 5, 2003
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"
#include "SF_Hash.h"

#ifdef SF_ENABLE_THREADS

#include "SF_Timer.h"
#include "SF_Debug.h"

#include <pthread.h>
#include <time.h>

#ifdef SF_OS_PS3
#include <sys/sys_time.h>
#include <sys/timer.h>
#include <sys/synchronization.h>
#define sleep(x) sys_timer_sleep(x)
#define usleep(x) sys_timer_usleep(x)
using std::timespec;
#else
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#endif

#ifdef SF_OS_ORBIS
#include <kernel.h>
#endif

#ifdef __APPLE__
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

namespace Scaleform {

// ***** Mutex implementation

// Interface used internally in a mutex
class Mutex_AreadyLockedAcquireInterface : public AcquireInterface
{
public:
    // Mutex we belong to
    Mutex *pMutex;

    Mutex_AreadyLockedAcquireInterface()
    {
        pMutex = 0;
    }

    // Acquire interface implementation
    virtual bool    CanAcquire();
    virtual bool    TryAcquire();
    virtual bool    TryAcquireCommit();
    virtual bool    TryAcquireCancel();

    // Interface - no implementation
    virtual void        AddRef()                            { }
    virtual void        Release(unsigned flags=0)               { }    
};


// Acquire interface implementation
bool    Mutex_AreadyLockedAcquireInterface::CanAcquire()       { return 1; }
bool    Mutex_AreadyLockedAcquireInterface::TryAcquire()       { return pMutex->TryAcquire(); }
bool    Mutex_AreadyLockedAcquireInterface::TryAcquireCommit() { return pMutex->TryAcquireCommit(); }
bool    Mutex_AreadyLockedAcquireInterface::TryAcquireCancel() { return pMutex->TryAcquireCancel(); }


// *** Internal Mutex implementation structure

class MutexImpl : public NewOverrideBase<Stat_Default_Mem>
{
    // System mutex or semaphore
    pthread_mutex_t   SMutex;
    bool          Recursive;
    unsigned      LockCount;
    pthread_t     LockedBy;

    Mutex_AreadyLockedAcquireInterface AreadyLockedAcquire;
    
    friend class WaitConditionImpl;

public:
    // Constructor/destructor
    MutexImpl(Mutex* pmutex, bool recursive = 1);
    ~MutexImpl();

    // Locking functions
    void                DoLock();
    bool                TryLock();
    void                Unlock(Mutex* pmutex);
    // Returns 1 if the mutes is currently locked
    bool                IsLockedByAnotherThread(Mutex* pmutex);        
    bool                IsSignaled() const;
    AcquireInterface*   GetAcquireInterface(Mutex* pmutex);
};

pthread_mutexattr_t Lock::RecursiveAttr;
bool Lock::RecursiveAttrInit = 0;

// *** Constructor/destructor
MutexImpl::MutexImpl(Mutex* pmutex, bool recursive)
{   
    AreadyLockedAcquire.pMutex  = pmutex;
    Recursive           = recursive;
    LockCount           = 0;

    if (Recursive)
    {
        if (!Lock::RecursiveAttrInit)
        {
            pthread_mutexattr_init(&Lock::RecursiveAttr);
            pthread_mutexattr_settype(&Lock::RecursiveAttr, PTHREAD_MUTEX_RECURSIVE);
            Lock::RecursiveAttrInit = 1;
        }

        pthread_mutex_init(&SMutex, &Lock::RecursiveAttr);
    }
    else
        pthread_mutex_init(&SMutex, 0);
}

MutexImpl::~MutexImpl()
{
    pthread_mutex_destroy(&SMutex);
}


// Lock and try lock
void MutexImpl::DoLock()
{
    while (pthread_mutex_lock(&SMutex));
    LockCount++;
    LockedBy = pthread_self();
}

bool MutexImpl::TryLock()
{
    if (!pthread_mutex_trylock(&SMutex))
    {
        LockCount++;
        LockedBy = pthread_self();
        return 1;
    }
    
    return 0;
}

void MutexImpl::Unlock(Mutex* pmutex)
{
    SF_ASSERT(pthread_self() == LockedBy && LockCount > 0);

    unsigned lockCount;
    LockCount--;
    lockCount = LockCount;

    // At this point handlers, if any, MUST already be created and
    // lazy initialization for pHandlers can not be used. To address this,
    // we allow an optional handler enable flag to be passed in constructor.
    // If we allowed lazy initialization, a call to AddHandlers in another
    // thread could access us after pHandlers read, causing the handler
    // to never be called (never informed about Unlock).
    Mutex::CallableHandlers handlers;
    pmutex->GetCallableHandlers(&handlers);

    // Release and Notify waitable objects
    pthread_mutex_unlock(&SMutex);

    // Call wait handlers indirectly here in case owner
    // destroys mutex after finishing wait on it.
    if (lockCount == 0)
        handlers.CallWaitHandlers();
}

bool    MutexImpl::IsLockedByAnotherThread(Mutex* pmutex)
{
    // There could be multiple interpretations of IsLocked with respect to current thread
    if (LockCount == 0)
        return 0;
    if (pthread_self() != LockedBy)
        return 1;
    return 0;
}

bool    MutexImpl::IsSignaled() const
{
    // An mutex is signaled if it is not locked ANYWHERE
    // Note that this is different from IsLockedByAnotherThread function,
    // that takes current thread into account
    return LockCount == 0;
}

// Obtain the acquisition interface
AcquireInterface*  MutexImpl::GetAcquireInterface(Mutex* pmutex)
{
    // If the mutex is already locked by us, return 'owned' acquire interface
    if (LockCount && !IsLockedByAnotherThread(pmutex))
        return &AreadyLockedAcquire;
    // Otherwise, just return pointer to us
    return pmutex;
}



// *** Actual Mutex class implementation

Mutex::Mutex(bool recursive, bool multiWait)
    : Waitable(multiWait)
{
    // NOTE: RefCount mode already thread-safe for all waitables.
    pImpl = new MutexImpl(this, recursive);
}

Mutex::~Mutex()
{
    delete pImpl;
}

// Lock and try lock
void Mutex::DoLock()
{
    pImpl->DoLock();
}
bool Mutex::TryLock()
{
    return pImpl->TryLock();
}
void Mutex::Unlock()
{
    pImpl->Unlock(this);
}
bool    Mutex::IsLockedByAnotherThread()
{
    return pImpl->IsLockedByAnotherThread(this);
}
bool    Mutex::IsSignaled() const
{
    return pImpl->IsSignaled();
}
// Obtain the acquisition interface
AcquireInterface*  Mutex::GetAcquireInterface()
{
    return pImpl->GetAcquireInterface(this);
}

// Acquire interface implementation
bool    Mutex::CanAcquire()
{
    return !IsLockedByAnotherThread();
}
bool    Mutex::TryAcquire()
{
    return TryLock();
}
bool    Mutex::TryAcquireCommit()
{
    // Nothing.
    return 1;
}
bool    Mutex::TryAcquireCancel()
{
    Unlock();
    return 1;
}


// ***** Wait Condition Implementation

// Internal implementation class
class WaitConditionImpl : public NewOverrideBase<Stat_Default_Mem>
{
    pthread_mutex_t     SMutex;
    pthread_cond_t      Condv;

public:

    // Constructor/destructor
    WaitConditionImpl();
    ~WaitConditionImpl();

    // Release mutex and wait for condition. The mutex is re-aqured after the wait.
    bool    Wait(Mutex *pmutex, unsigned delay = SF_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    void    Notify();
    // Notify a condition, releasing all objects waiting
    void    NotifyAll();
};


WaitConditionImpl::WaitConditionImpl()
{
    pthread_mutex_init(&SMutex, 0);
    pthread_cond_init(&Condv, 0);
}

WaitConditionImpl::~WaitConditionImpl()
{
    pthread_mutex_destroy(&SMutex);
    pthread_cond_destroy(&Condv);
}    

bool    WaitConditionImpl::Wait(Mutex *pmutex, unsigned delay)
{
    bool            result = 1;
    unsigned            lockCount = pmutex->pImpl->LockCount;

    // Mutex must have been locked
    if (lockCount == 0)
        return 0;

    pthread_mutex_lock(&SMutex);

    // Finally, release a mutex or semaphore
    if (pmutex->pImpl->Recursive)
    {
        // Release the recursive mutex N times
        pmutex->pImpl->LockCount = 0;
        for(unsigned i=0; i<lockCount; i++)
            pthread_mutex_unlock(&pmutex->pImpl->SMutex);
        // NOTE: Do not need to use CallableHanders here because mutex
        // can not be destroyed by user if we are to re-acquire it later.
        pmutex->CallWaitHandlers();
    }
    else
    {
        pmutex->pImpl->LockCount = 0;
        pthread_mutex_unlock(&pmutex->pImpl->SMutex);
        pmutex->CallWaitHandlers();
    }

    // Note that there is a gap here between mutex.Unlock() and Wait().
    // The other mutex protects this gap.

    if (delay == SF_WAIT_INFINITE)
        pthread_cond_wait(&Condv,&SMutex);
    else
    {
        timespec ts;
#ifdef SF_OS_PS3
        sys_time_sec_t s;
        sys_time_nsec_t ns;
        sys_time_get_current_time(&s, &ns);

        ts.tv_sec = s + (delay / 1000);
        ts.tv_nsec = ns + (delay % 1000) * 1000000;

#else
        struct timeval tv;
        gettimeofday(&tv, 0);

        ts.tv_sec = tv.tv_sec + (delay / 1000);
        ts.tv_nsec = (tv.tv_usec + (delay % 1000) * 1000) * 1000;
#endif
        if (ts.tv_nsec > 999999999)
        {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        int r = pthread_cond_timedwait(&Condv,&SMutex, &ts);
        SF_ASSERT(r == 0 || r == ETIMEDOUT);
        if (r)
            result = 0;
    }

    pthread_mutex_unlock(&SMutex);

    // Re-aquire the mutex
    for(unsigned i=0; i<lockCount; i++)
        pmutex->DoLock(); 

    // Return the result
    return result;
}

// Notify a condition, releasing the least object in a queue
void    WaitConditionImpl::Notify()
{
    pthread_mutex_lock(&SMutex);
    pthread_cond_signal(&Condv);
    pthread_mutex_unlock(&SMutex);
}

// Notify a condition, releasing all objects waiting
void    WaitConditionImpl::NotifyAll()
{
    pthread_mutex_lock(&SMutex);
    pthread_cond_broadcast(&Condv);
    pthread_mutex_unlock(&SMutex);
}



// *** Actual implementation of WaitCondition

WaitCondition::WaitCondition()
{
    pImpl = new WaitConditionImpl;
}
WaitCondition::~WaitCondition()
{
    delete pImpl;
}
    
bool    WaitCondition::Wait(Mutex *pmutex, unsigned delay)
{
    return pImpl->Wait(pmutex, delay);
}
// Notification
void    WaitCondition::Notify()
{
    pImpl->Notify();
}
void    WaitCondition::NotifyAll()
{
    pImpl->NotifyAll();
}


// ***** Current thread

// Per-thread variable
/*
static __thread Thread* pCurrentThread = 0;

// Static function to return a pointer to the current thread
void    Thread::InitCurrentThread(Thread *pthread)
{
    pCurrentThread = pthread;
}

// Static function to return a pointer to the current thread
Thread*    Thread::GetThread()
{
    return pCurrentThread;
}
*/


// *** Thread constructors.

Thread::Thread(UPInt stackSize, int processor) : Waitable(1)
{
    // NOTE: RefCount mode already thread-safe for all Waitable objects.
    CreateParams params;
    params.stackSize = stackSize;
    params.processor = processor;
    Init(params);
}

Thread::Thread(Thread::ThreadFn threadFunction, void*  userHandle, UPInt stackSize,
                 int processor, Thread::ThreadState initialState)
    : Waitable(1)
{
    CreateParams params(threadFunction, userHandle, stackSize, processor, initialState);
    Init(params);
}

Thread::Thread(const CreateParams& params) : Waitable(1)
{
    Init(params);
}

void Thread::Init(const CreateParams& params)
{
    // Clear the variables    
    ThreadFlags     = 0;
    ThreadHandle    = 0;
    ExitCode        = 0;
    SuspendCount    = 0;
    StackSize       = params.stackSize;
    Processor       = params.processor;
    Priority        = params.priority;

    // Clear Function pointers
    ThreadFunction  = params.threadFunction;
    UserHandle      = params.userHandle;
    if (params.initialState != NotRunning)
        Start(params.initialState);
}

Thread::~Thread()
{
    // Thread should not running while object is being destroyed,
    // this would indicate ref-counting issue.
    //SF_ASSERT(IsRunning() == 0);

    // Clean up thread.    
    ThreadHandle = 0;
}



// *** Overridable User functions.

// Default Run implementation
int    Thread::Run()
{
    // Call pointer to function, if available.    
    return (ThreadFunction) ? ThreadFunction(this, UserHandle) : 0;
}
void    Thread::OnExit()
{   
}


// Finishes the thread and releases internal reference to it.
void    Thread::FinishAndRelease()
{
    // Get callable handlers so that they can still be called
    // after Thread object is released.
    CallableHandlers handlers;
    GetCallableHandlers(&handlers);

    // Note: thread must be US.
    ThreadFlags &= (UInt32)~(SF_THREAD_STARTED);
    ThreadFlags |= SF_THREAD_FINISHED;

    // Release our reference; this is equivalent to 'delete this'
    // from the point of view of our thread.
    Release();

    // Call handlers, if any, signifying completion.
    handlers.CallWaitHandlers();
}



// *** ThreadList - used to tack all created threads

class ThreadList : public NewOverrideBase<Stat_Default_Mem>
{
    //------------------------------------------------------------------------
    struct ThreadHashOp
    {
        size_t operator()(const Thread* ptr)
        {
            return (((size_t)ptr) >> 6) ^ (size_t)ptr;
        }
    };

    HashSet<Thread*, ThreadHashOp>        ThreadSet;
    Mutex                                 ThreadMutex;
    WaitCondition                         ThreadsEmpty;
    // Track the root thread that created us.
    pthread_t                             RootThreadId;

    static ThreadList* volatile pRunningThreads;

    void addThread(Thread *pthread)
    {
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Add(pthread);
    }

    void removeThread(Thread *pthread)
    {
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Remove(pthread);
        if (ThreadSet.GetSize() == 0)
            ThreadsEmpty.Notify();
    }

    void finishAllThreads()
    {
        // Only original root thread can call this.
        SF_ASSERT(pthread_self() == RootThreadId);

        Mutex::Locker lock(&ThreadMutex);
        while (ThreadSet.GetSize() != 0)
            ThreadsEmpty.Wait(&ThreadMutex);
    }

public:

    ThreadList()
    {
        RootThreadId = pthread_self();
    }
    ~ThreadList() { }


    static void AddRunningThread(Thread *pthread)
    {
        // Non-atomic creation ok since only the root thread
        if (!pRunningThreads)
        {
            pRunningThreads = new ThreadList;
            SF_ASSERT(pRunningThreads);
        }
        pRunningThreads->addThread(pthread);
    }

    // NOTE: 'pthread' might be a dead pointer when this is
    // called so it should not be accessed; it is only used
    // for removal.
    static void RemoveRunningThread(Thread *pthread)
    {
        SF_ASSERT(pRunningThreads);        
        pRunningThreads->removeThread(pthread);
    }

    static void FinishAllThreads()
    {
        // This is ok because only root thread can wait for other thread finish.
        if (pRunningThreads)
        {           
            pRunningThreads->finishAllThreads();
            delete pRunningThreads;
            pRunningThreads = 0;
        }        
    }
};

// By default, we have no thread list.
ThreadList* volatile ThreadList::pRunningThreads = 0;


// FinishAllThreads - exposed publicly in Thread.
void Thread::FinishAllThreads()
{
    ThreadList::FinishAllThreads();
}

// *** Run override

int    Thread::PRun()
{
    // Suspend us on start, if requested
    if (ThreadFlags & SF_THREAD_START_SUSPENDED)
    {
        Suspend();
        ThreadFlags &= (UInt32)~SF_THREAD_START_SUSPENDED;
    }

    // Call the virtual run function
    ExitCode = Run();    
    return ExitCode;
}




// *** User overridables

bool    Thread::GetExitFlag() const
{
    return (ThreadFlags & SF_THREAD_EXIT) != 0;
}       

void    Thread::SetExitFlag(bool exitFlag)
{
    // The below is atomic since ThreadFlags is AtomicInt.
    if (exitFlag)
        ThreadFlags |= SF_THREAD_EXIT;
    else
        ThreadFlags &= (UInt32) ~SF_THREAD_EXIT;
}


// Determines whether the thread was running and is now finished
bool    Thread::IsFinished() const
{
    return (ThreadFlags & SF_THREAD_FINISHED) != 0;
}
// Determines whether the thread is suspended
bool    Thread::IsSuspended() const
{   
    return SuspendCount > 0;
}
// Returns current thread state
Thread::ThreadState Thread::GetThreadState() const
{
    if (IsSuspended())
        return Suspended;    
    if (ThreadFlags & SF_THREAD_STARTED)
        return Running;    
    return NotRunning;
}
/*
static const char* mapsched_policy(int policy)
{
    switch(policy)
    {
    case SCHED_OTHER:
        return "SCHED_OTHER";
    case SCHED_RR:
        return "SCHED_RR";
    case SCHED_FIFO:
        return "SCHED_FIFO";

    }
    return "UNKNOWN";
}
    int policy;
    sched_param sparam;
    pthread_getschedparam(pthread_self(), &policy, &sparam);
    int max_prior = sched_get_priority_max(policy);
    int min_prior = sched_get_priority_min(policy);
    printf(" !!!! policy: %s, priority: %d, max priority: %d, min priority: %d\n", mapsched_policy(policy), sparam.sched_priority, max_prior, min_prior);
#include <stdio.h>
*/
// ***** Thread management

// The actual first function called on thread start
void* Thread_PthreadStartFn(void* phandle)
{
    Thread* pthread = (Thread*)phandle;
    int     result = pthread->PRun();
    // Signal the thread as done and release it atomically.
    pthread->FinishAndRelease();
    // At this point Thread object might be dead; however we can still pass
    // it to RemoveRunningThread since it is only used as a key there.   
    ThreadList::RemoveRunningThread(pthread);
    return reinterpret_cast<void*>(result);
}

int Thread::InitAttr = 0;
pthread_attr_t Thread::Attr; 

/* static */
int Thread::GetOSPriority(ThreadPriority p)
//static inline int MapToSystemPrority(Thread::ThreadPriority p)
{
#if defined(SF_OS_PS3)
    switch(p)
    {
    case Thread::CriticalPriority:     return 0;
    case Thread::HighestPriority:      return 300;
    case Thread::AboveNormalPriority:  return 600;
    case Thread::NormalPriority:       return 1000;
    case Thread::BelowNormalPriority:  return 1500;
    case Thread::LowestPriority:       return 2500;
    case Thread::IdlePriority:         return 3071;
    }                                  return 1000;
#elif defined(SF_OS_ORBIS)
    switch(p)
    {
    case Thread::CriticalPriority:     return SCE_KERNEL_PRIO_FIFO_HIGHEST;
    case Thread::HighestPriority:      return SCE_KERNEL_PRIO_FIFO_HIGHEST;
    case Thread::AboveNormalPriority:  return SCE_KERNEL_PRIO_FIFO_DEFAULT - 50;
    case Thread::NormalPriority:       return SCE_KERNEL_PRIO_FIFO_DEFAULT;
    case Thread::BelowNormalPriority:  return SCE_KERNEL_PRIO_FIFO_DEFAULT + 50;
    case Thread::LowestPriority:       return SCE_KERNEL_PRIO_FIFO_LOWEST;
    case Thread::IdlePriority:         return SCE_KERNEL_PRIO_FIFO_LOWEST;
    }                                  return SCE_KERNEL_PRIO_FIFO_DEFAULT;
#else
    SF_UNUSED(p);
    return -1;
#endif
}

bool    Thread::Start(ThreadState initialState)
{
    if (initialState == NotRunning)
        return 0;

    if (!InitAttr)
    {
        pthread_attr_init(&Attr);
        pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setstacksize(&Attr, 128 * 1024);
        sched_param sparam;
        sparam.sched_priority = Thread::GetOSPriority(NormalPriority);
        pthread_attr_setschedparam(&Attr, &sparam);
        InitAttr = 1;
    }

    // If the thread is already running then wait
    // until its finished to begin running this thread
    if ((GetThreadState() != NotRunning) && !Wait())
        return 0;

    ExitCode        = 0;
    SuspendCount    = 0;
    ThreadFlags     = (initialState == Running) ? SF_THREAD_STARTED : SF_THREAD_START_SUSPENDED;

    // AddRef to us until the thread is finished
    AddRef();
    ThreadList::AddRunningThread(this);

    int result;
    if (StackSize != 128 * 1024 || Priority != NormalPriority)
    {
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setstacksize(&attr, StackSize);
        sched_param sparam;
        sparam.sched_priority = Thread::GetOSPriority(Priority);
        pthread_attr_setschedparam(&attr, &sparam);
        result = pthread_create(&ThreadHandle, &attr, Thread_PthreadStartFn, this);
        pthread_attr_destroy(&attr);
    }
    else
        result = pthread_create(&ThreadHandle, &Attr, Thread_PthreadStartFn, this);

    if (result)
    {
        ThreadFlags = 0;
        Release();
        ThreadList::RemoveRunningThread(this);
        return 0;
    }
    return 1;
}


// Suspend the thread until resumed
bool    Thread::Suspend()
{
    SF_DEBUG_WARNING(1, "cannot suspend threads on this system");
    return 0;
}

// Resumes currently suspended thread
bool    Thread::Resume()
{
    return 0;
}


// Quits with an exit code  
void    Thread::Exit(int exitCode)
{
    // Can only exist the current thread
   // if (GetThread() != this)
   //     return;

    // Call the virtual OnExit function
    OnExit();   

    // Signal this thread object as done and release it's references.
    FinishAndRelease();
    ThreadList::RemoveRunningThread(this);

    pthread_exit(reinterpret_cast<void *>(exitCode));
}

ThreadId GetCurrentThreadId()
{
    return (void*)pthread_self();
}

// *** Sleep functions

/* static */
bool    Thread::Sleep(unsigned secs)
{
    sleep(secs);
    return 1;
}
/* static */
bool    Thread::MSleep(unsigned msecs)
{
    usleep(msecs*1000);
    return 1;
}

#ifdef SF_OS_ORBIS
void Thread::SetThreadName( const char* name )
{
    scePthreadRename(ThreadHandle, name);
}
#endif

/* static */
int     Thread::GetCPUCount()
{
#ifdef __APPLE__
    int numCPU;
    int mib[4];
    size_t len = sizeof(numCPU);     
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    sysctl(mib, 2, &numCPU, &len, 0, 0);
    return Alg::Max(numCPU, 1);
#elif defined(SF_OS_ORBIS)
    return 8;
#else
    return 1;
#endif
}

#ifdef SF_OS_PS3

sys_lwmutex_attribute_t Lock::LockAttr = { SYS_SYNC_PRIORITY, SYS_SYNC_RECURSIVE };

#endif

} // Scaleform

#endif  // SF_ENABLE_THREADS
