/**************************************************************************

Filename    :   SF_ThreadsWii.cpp
Platform    :   Wii
Content     :   Wii threads support
Created     :   May 5, 2003
Authors     :   Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"

#ifdef SF_ENABLE_THREADS

#include "SF_Hash.h"
#include "SF_Debug.h"
#ifdef SF_OS_WII
#include <revolution/os.h>
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
    OSMutex       Mutex;
    bool          Recursive;
    unsigned      LockCount;

    Mutex_AreadyLockedAcquireInterface AreadyLockedAcquire;
    
    friend class WaitConditionImpl;

public:
    // Constructor/destructor
    MutexImpl(Scaleform::Mutex* pmutex, bool recursive = 1);
    ~MutexImpl();

    // Locking functions
    void                DoLock();
    bool                TryLock();
    void                Unlock(Scaleform::Mutex* pmutex);
    // Returns 1 if the mutes is currently locked
    bool                IsLockedByAnotherThread(Scaleform::Mutex* pmutex);        
    bool                IsSignaled() const;
    AcquireInterface*   GetAcquireInterface(Scaleform::Mutex* pmutex);
};

// *** Constructor/destructor
MutexImpl::MutexImpl(Scaleform::Mutex* pmutex, bool recursive)
{   
    AreadyLockedAcquire.pMutex  = pmutex;
    Recursive           = recursive;
    LockCount           = 0;

    OSInitMutex(&Mutex);
}

MutexImpl::~MutexImpl()
{
}


// Lock and try lock
void MutexImpl::DoLock()
{
    OSLockMutex(&Mutex);
    LockCount++;
}

bool MutexImpl::TryLock()
{
    if (OSTryLockMutex(&Mutex))
    {
        LockCount++;
        return 1;
    }
    
    return 0;
}

void MutexImpl::Unlock(Scaleform::Mutex* pmutex)
{
    SF_ASSERT(LockCount > 0);

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
    OSUnlockMutex(&Mutex);

    // Call wait handlers indirectly here in case owner
    // destroys mutex after finishing wait on it.
    if (lockCount == 0)
        handlers.CallWaitHandlers();
}

bool    MutexImpl::IsLockedByAnotherThread(Scaleform::Mutex* pmutex)
{
    // There could be multiple interpretations of IsLocked with respect to current thread
    if (LockCount == 0)
        return 0;
    if (!TryLock())
        return 1;
    Unlock(pmutex);
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
AcquireInterface*  MutexImpl::GetAcquireInterface(Scaleform::Mutex* pmutex)
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
    OSMutex            Mutex;
    OSCond             Condv;
    AtomicInt<UInt32>  Waiters;
    AtomicInt<UInt32>  NotifyVar;
    Array<OSAlarm*>    WaitAlarms;

public:

    // Constructor/destructor
    WaitConditionImpl();
    ~WaitConditionImpl();

    // Release mutex and wait for condition. The mutex is re-aquired after the wait.
    bool    Wait(Scaleform::Mutex *pmutex, unsigned delay = SF_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    void    Notify();
    // Notify a condition, releasing all objects waiting
    void    NotifyAll();

    static void AlarmHandler(OSAlarm* palarm, OSContext* pcontext);
};


WaitConditionImpl::WaitConditionImpl() : Waiters(0), NotifyVar(0)
{
    OSInitMutex(&Mutex);
    OSInitCond(&Condv);
}

WaitConditionImpl::~WaitConditionImpl()
{
    for (unsigned i = 0; i < WaitAlarms.GetSize(); i++)
        SF_FREE(WaitAlarms[i]);
}

void WaitConditionImpl::AlarmHandler(OSAlarm* palarm, OSContext* pcontext)
{
    WaitConditionImpl *pImpl = (WaitConditionImpl *) palarm->userData;

    { AtomicOpsRawBase::FullSync sync; }

    if (pImpl->Mutex.count == 0 && pImpl->NotifyVar.CompareAndSet_Sync(0, UInt32(palarm)))
        OSSignalCond(&pImpl->Condv);
}

bool    WaitConditionImpl::Wait(Scaleform::Mutex *pmutex, unsigned delay)
{
    bool            result = 1;
    unsigned            lockCount = pmutex->pImpl->LockCount;

    // Mutex must have been locked
    if (lockCount == 0)
        return 0;

    // Mutex protects against missing events when processing alarms or Handlers
    OSLockMutex(&Mutex);

    SF_DEBUG_WARNING2(NotifyVar, "WaitConditionImpl::Wait %x entered with unprocessed Notify %d", this, NotifyVar.Value);
    
    // Finally, release a mutex or semaphore
    if (pmutex->pImpl->Recursive)
    {
        // Release the recursive mutex N times
        pmutex->pImpl->LockCount = 0;
        for(unsigned i=0; i<lockCount; i++)
            OSUnlockMutex(&pmutex->pImpl->Mutex);
        // NOTE: Do not need to use CallableHanders here because mutex
        // can not be destroyed by user if we are to re-acquire it later.
        pmutex->CallWaitHandlers();
    }
    else
    {
        pmutex->pImpl->LockCount = 0;
        OSUnlockMutex(&pmutex->pImpl->Mutex);
        pmutex->CallWaitHandlers();
    }

    SF_DEBUG_WARNING2(NotifyVar, "WaitConditionImpl::Wait %x will wait with unprocessed Notify %d", this, NotifyVar.Value);

    // Note that there is a gap here between mutex.Unlock() and Wait().
    // The other mutex protects this gap.
    Waiters++;

        OSAlarm * pwa = 0;
        unsigned      wai = ~0u;
        int      a;

        if (delay != SF_WAIT_INFINITE)
        {
            pwa = (OSAlarm*)SF_ALLOC(sizeof(OSAlarm), Stat_Default_Mem);
            wai = WaitAlarms.GetSize();

            // a periodic alarm is used so it can try again if the alarm goes off while a Notify 
            // (or other alarm) is still being processed
            memset(pwa, 0, sizeof(OSAlarm));
            pwa->userData = this;
            OSSetPeriodicAlarm(pwa, OSGetTime() + OSMicrosecondsToTicks(delay*1000), OSMicrosecondsToTicks(20), &AlarmHandler);
            WaitAlarms.PushBack(pwa);
        }

        do {
            a = -1;
            OSWaitCond(&Condv,&Mutex);

            UInt32 nv = NotifyVar.Load_Acquire();

            if (nv > 2)
            {
                OSAlarm *palarm = (OSAlarm*) nv;
                for (unsigned i = 0; i < WaitAlarms.GetSize(); i++)
                    if (palarm == WaitAlarms[i])
                        a = i;
                SF_ASSERT(a >= 0);
            }
            else if (nv == 1) // notify
            {
                NotifyVar = 0;
                a = -2;
            }
            else if (nv == 2) // notify all
                a = -2;
        }
        while (a >= -1 && a != wai);

        result = (a == -2);

        if (pwa)
        {
            if (!result) NotifyVar = 0;
            OSCancelAlarm(pwa);
        }

    Waiters--;
    if (Waiters == 0)
    {
        for (unsigned i = 0; i < WaitAlarms.GetSize(); i++)
        {
            OSCancelAlarm(WaitAlarms[i]);
            SF_FREE(WaitAlarms[i]);
        }
        WaitAlarms.Clear();
        NotifyVar = 0;
    }
    OSUnlockMutex(&Mutex);

    // Re-aquire the mutex
    for(unsigned i=0; i<lockCount; i++)
        pmutex->DoLock(); 

    // Return the result
    return result;
}

// Notify a condition, releasing the least object in a queue
void    WaitConditionImpl::Notify()
{
    OSLockMutex(&Mutex);
    if (Waiters)
    {
        while (!NotifyVar.CompareAndSet_Sync(0, 1))
        {
            // The thread never processed a previous Notify because of scheduling.
            OSUnlockMutex(&Mutex);
            OSYieldThread();
            OSLockMutex(&Mutex);
            if (Waiters == 0)
                goto endnotify;
        }
        OSSignalCond(&Condv);
    }
endnotify:
    OSUnlockMutex(&Mutex);
}

// Notify a condition, releasing all objects waiting
void    WaitConditionImpl::NotifyAll()
{
    OSLockMutex(&Mutex);
    while (Waiters)
    {
        while (!NotifyVar.CompareAndSet_Sync(0, 1))
        {
            // The thread never processed a previous Notify because of scheduling.
            OSUnlockMutex(&Mutex);
            OSYieldThread();
            OSLockMutex(&Mutex);
            if (Waiters == 0)
                goto endnotify;
        }
        OSSignalCond(&Condv);
    }
endnotify:
    OSUnlockMutex(&Mutex);
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
    
bool    WaitCondition::Wait(Scaleform::Mutex *pmutex, unsigned delay)
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
public: // for Wii compiler 4.3 145
    //------------------------------------------------------------------------
    struct ThreadHashOp
    {
        size_t operator()(const Thread* ptr)
        {
            return (((size_t)ptr) >> 6) ^ (size_t)ptr;
        }
    };
private:
    HashSet<Thread*, ThreadHashOp>  ThreadSet;
    List<Thread::ThreadDataWii>     ExitingThreads;
    Mutex                           ThreadMutex;
    WaitCondition                   ThreadsEmpty;
    // Track the root thread that created us.
    OSThread*                       RootThreadId;

    static ThreadList* volatile pRunningThreads;

    void addThread(Thread *pthread)
    {
        checkThreads();
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Add(pthread);
    }

    void removeThread(Thread *pthread, Thread::ThreadDataWii* pthreaddata)
    {
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Remove(pthread);
        ExitingThreads.PushBack(pthreaddata);
        if (ThreadSet.GetSize() == 0)
            ThreadsEmpty.Notify();
    }

    void finishAllThreads()
    {
        // Only original root thread can call this.
        SF_ASSERT(OSGetCurrentThread() == RootThreadId);
        {
        Mutex::Locker lock(&ThreadMutex);
        while (ThreadSet.GetSize() != 0)
            ThreadsEmpty.Wait(&ThreadMutex);
        }
        checkThreads();
    }

    // Free stacks and OS data of finished threads
    void checkThreads()
    {
        ThreadMutex.DoLock();
        Thread::ThreadDataWii* pthread = ExitingThreads.GetFirst();
        while (!ExitingThreads.IsNull(pthread))
        {
            ThreadMutex.Unlock();
            OSJoinThread(&pthread->ThreadHandle, NULL);
            ThreadMutex.DoLock();
            Thread::ThreadDataWii* pnext = ExitingThreads.GetNext(pthread);
            ExitingThreads.Remove(pthread);
            SF_FREE(pthread);
            pthread = pnext;
        }
        ThreadMutex.Unlock();
    }

public:

    ThreadList()
    {
        RootThreadId = OSGetCurrentThread();
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
    static void RemoveRunningThread(Thread *pthread, Thread::ThreadDataWii* pthreaddata)
    {
        SF_ASSERT(pRunningThreads);        
        pRunningThreads->removeThread(pthread, pthreaddata);
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



// ***** Thread management

// The actual first function called on thread start
#ifdef SF_OS_WII
void* Thread_PthreadStartFn(void* phandle)
#else
int Thread_PthreadStartFn(int, void* phandle)
#endif
{
    Thread* pthread = (Thread*)phandle;
    int     result = pthread->PRun();
#ifdef SF_OS_WII
    if (setjmp(pthread->ThreadData->ExitJmp))
        result = pthread->ExitCode;
#endif

    Thread::ThreadDataWii* pthreaddata = pthread->ThreadData;
    // Signal the thread as done and release it atomically.
    pthread->FinishAndRelease();
    // At this point Thread object might be dead; however we can still pass
    // it to RemoveRunningThread since it is only used as a key there.   
    ThreadList::RemoveRunningThread(pthread, pthreaddata);

#ifdef SF_OS_WII
    return (void*) result;
#else
    return result;
#endif
}

/* static */
int Thread::GetOSPriority(ThreadPriority p)
//static inline OSPriority MapToSystemPrority(Thread::ThreadPriority p)
{
    switch(p)
    {
    case Thread::CriticalPriority:     return 8;
    case Thread::HighestPriority:      return 10;
    case Thread::AboveNormalPriority:  return 12;
    case Thread::NormalPriority:       return 16;
    case Thread::BelowNormalPriority:  return 20;
    case Thread::LowestPriority:       return 24;
    case Thread::IdlePriority:         return 25;
    }
    return 16;
}

bool    Thread::Start(ThreadState initialState)
{
    if (initialState == NotRunning)
        return 0;

    // If the thread is already running then wait
    // until its finished to begin running this thread
    if ((GetThreadState() != NotRunning) && !Wait())
        return 0;

    ExitCode        = 0;
    SuspendCount    = 0;
    ThreadFlags     = (initialState == Running) ? 0 : SF_THREAD_START_SUSPENDED;

    // AddRef to us until the thread is finished
    AddRef();
    ThreadList::AddRunningThread(this);

    ThreadData = (ThreadDataWii*) SF_MEMALIGN(sizeof(ThreadDataWii) + StackSize - 32, 32, Stat_Default_Mem);
    UByte* Stack = ThreadData->Stack;
#ifdef SF_OS_WII
    if (!OSCreateThread(&ThreadData->ThreadHandle, Thread_PthreadStartFn, this, ((UByte*)Stack)+StackSize, StackSize,
                        Thread::GetOSPriority(Priority), 0))
#else
    if (!OSCreateThread(&ThreadData->ThreadHandle, Thread_PthreadStartFn, Processor, this, ((UByte*)Stack)+StackSize, StackSize,
                        Thread::GetOSPriority(Priority), 0))
#endif
    {
        ThreadFlags = 0;
        SF_FREE(ThreadData);

        Thread::ThreadDataWii* pthreaddata = ThreadData;
        Release();
        ThreadList::RemoveRunningThread(this, pthreaddata);
        return 0;
    }

    OSResumeThread(&ThreadData->ThreadHandle);

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

    ExitCode = exitCode;
#ifdef SF_OS_WII
    longjmp(ThreadData->ExitJmp, 1);
#endif
}

ThreadId GetCurrentThreadId()
{
    return OSGetCurrentThread();
}

// *** Sleep functions

/* static */
bool    Thread::Sleep(unsigned secs)
{
    if (secs)
        OSSleepSeconds(secs);
    else
        OSYieldThread();
    return 1;
}

/* static */
bool    Thread::MSleep(unsigned msecs)
{
    if (msecs)
        OSSleepMilliseconds(msecs);
    else
        OSYieldThread();
    return 1;
}
/* static */
int     Thread::GetCPUCount()
{
#ifdef SF_OS_WII
    return 1;
#else
    return 3;
#endif
}

} // Scaleform

#endif  // SF_ENABLE_THREADS
