/**************************************************************************

Filename    :   SF_ThreadsPS3.cpp
Platform    :   PS3
Content     :   PPU thread support
Created     :   May 2009
Authors     :   Vladislav Merker, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"
#include "SF_Hash.h"

#ifdef SF_ENABLE_THREADS

#include "SF_Timer.h"

#include <sys/ppu_thread.h>
#include <sys/synchronization.h>
//#include <sys/sys_time.h>
#include <sys/timer.h>

#define PPU_THREAD_STACK_SIZE 0x20000

namespace Scaleform {

// *** Mutex implementation

// Interface used internally in a mutex
class Mutex_AreadyLockedAcquireInterface: public AcquireInterface
{
public:
    // Mutex we belong to
    Mutex *pMutex;

    Mutex_AreadyLockedAcquireInterface() { pMutex = 0; }

    // Acquire interface implementation
    virtual bool CanAcquire();
    virtual bool TryAcquire();
    virtual bool TryAcquireCommit();
    virtual bool TryAcquireCancel();

    // Interface - no implementation
    virtual void AddRef()                            {}
    virtual void Release(UPInt flags = 0)             {}    
};


// Acquire interface implementation
bool Mutex_AreadyLockedAcquireInterface::CanAcquire()       { return true; }
bool Mutex_AreadyLockedAcquireInterface::TryAcquire()       { return pMutex->TryAcquire(); }
bool Mutex_AreadyLockedAcquireInterface::TryAcquireCommit() { return pMutex->TryAcquireCommit(); }
bool Mutex_AreadyLockedAcquireInterface::TryAcquireCancel() { return pMutex->TryAcquireCancel(); }


// *** Internal mutex implementation structure

class MutexImpl: public NewOverrideBase<Stat_Default_Mem>
{
    sys_mutex_t      MutexId;
	bool             Recursive;
    sys_ppu_thread_t LockedBy;
    UPInt            LockCount;

    Mutex_AreadyLockedAcquireInterface AreadyLockedAcquire;
    
    friend class WaitConditionImpl;

public:
    MutexImpl(Mutex* pmutex, bool recursive = true);
    ~MutexImpl();

    // Locking functions
    void DoLock();
    bool TryLock();
    void Unlock(Mutex* pmutex);

    bool IsLockedByAnotherThread() const;
    bool IsSignaled() const;

    AcquireInterface* GetAcquireInterface(Mutex* pmutex);
};


// Constructor and destructor
MutexImpl::MutexImpl(Mutex* pmutex, bool recursive)
{   
    AreadyLockedAcquire.pMutex = pmutex;
    Recursive = recursive;
    LockCount = 0;

    sys_mutex_attribute_t mutexAttr;
    sys_mutex_attribute_initialize(mutexAttr);
    if(Recursive)
        mutexAttr.attr_recursive = SYS_SYNC_RECURSIVE;

    int err = sys_mutex_create(&MutexId, &mutexAttr);
    SF_ASSERT(err == CELL_OK);
    SF_UNUSED(err);
}

MutexImpl::~MutexImpl()
{
    int err = sys_mutex_destroy(MutexId);
    SF_ASSERT(err == CELL_OK);
    SF_UNUSED(err);
}


// Lock and try lock
void MutexImpl::DoLock()
{
    while(sys_mutex_lock(MutexId, 0) != CELL_OK);
    LockCount++;
    sys_ppu_thread_get_id(&LockedBy);
}

bool MutexImpl::TryLock()
{
    if(sys_mutex_trylock(MutexId) == CELL_OK)
    {
        LockCount++;
        sys_ppu_thread_get_id(&LockedBy);
        return true;
    }    
    return false;
}

// Unlock
void MutexImpl::Unlock(Mutex* pmutex)
{
    sys_ppu_thread_t thrId;
    sys_ppu_thread_get_id(&thrId);
    SF_ASSERT(thrId == LockedBy && LockCount > 0);
    SF_UNUSED(thrId);

    UPInt lockCount;
    LockCount--;
    lockCount = LockCount;

    Mutex::CallableHandlers handlers;
    pmutex->GetCallableHandlers(&handlers);

    sys_mutex_unlock(MutexId);

    if(lockCount == 0)
        handlers.CallWaitHandlers();
}

bool MutexImpl::IsLockedByAnotherThread() const
{
    if(LockCount == 0)
        return false;

    sys_ppu_thread_t thrId;
    sys_ppu_thread_get_id(&thrId);
    if(thrId != LockedBy)
        return true;

    return false;
}

bool MutexImpl::IsSignaled() const
{
    return LockCount == 0;
}

// Obtain the acquisition interface
AcquireInterface* MutexImpl::GetAcquireInterface(Mutex* pmutex)
{
    if(LockCount && !IsLockedByAnotherThread())
        return &AreadyLockedAcquire;
    return pmutex;
}


// *** Actual implementation of Mutex

Mutex::Mutex(bool recursive, bool multiWait):
    Waitable(multiWait) { pImpl = new MutexImpl(this, recursive); }
Mutex::~Mutex()         { delete pImpl; }

// Lock, try lock and unlock
void Mutex::DoLock()  { pImpl->DoLock(); }
bool Mutex::TryLock() { return pImpl->TryLock(); }
void Mutex::Unlock()  { pImpl->Unlock(this); }

bool Mutex::IsLockedByAnotherThread() { return pImpl->IsLockedByAnotherThread(); }
bool Mutex::IsSignaled() const        { return pImpl->IsSignaled(); }

// Obtain the acquisition interface
AcquireInterface* Mutex::GetAcquireInterface() { return pImpl->GetAcquireInterface(this); }

// Acquire interface implementation
bool Mutex::CanAcquire()       { return !IsLockedByAnotherThread(); }
bool Mutex::TryAcquire()       { return TryLock(); }
bool Mutex::TryAcquireCommit() { return true; }
bool Mutex::TryAcquireCancel() { Unlock(); return true; }


// ***** Wait Condition implementation

// Internal implementation class
class WaitConditionImpl: public NewOverrideBase<Stat_Default_Mem>
{
    sys_mutex_t MutexId;
    sys_cond_t  CondvId;

public:
    WaitConditionImpl();
    ~WaitConditionImpl();

    // Release mutex and wait for condition. The mutex is re-aqured after the wait
    bool Wait(Mutex *pmutex, UPInt delay = SF_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    // and releasing all objects waiting
    void Notify();
    void NotifyAll();
};


// Constructor and destructor
WaitConditionImpl::WaitConditionImpl()
{
    sys_mutex_attribute_t mutexAttr;
    sys_mutex_attribute_initialize(mutexAttr);
    int err = sys_mutex_create(&MutexId, &mutexAttr);
    SF_ASSERT(err == CELL_OK);
    SF_UNUSED(err);

    sys_cond_attribute_t  condvAttr;
    sys_cond_attribute_initialize(condvAttr);
    err = sys_cond_create(&CondvId, MutexId, &condvAttr);
    SF_ASSERT(err == CELL_OK);
}

WaitConditionImpl::~WaitConditionImpl()
{
    int err = sys_cond_destroy(CondvId);
    SF_ASSERT(err == CELL_OK);
    SF_UNUSED(err);

    err = sys_mutex_destroy(MutexId);
    SF_ASSERT(err == CELL_OK);
}


// Release mutex and wait for condition. The mutex is re-aqured after the wait
bool WaitConditionImpl::Wait(Mutex *pmutex, UPInt delay)
{
    bool ret = true;
    UPInt lockCount = pmutex->pImpl->LockCount;

    // Mutex must have been locked
    if(lockCount == 0)
        return false;

    sys_mutex_lock(MutexId, 0);

    if(pmutex->pImpl->Recursive)
    {
        // Release the recursive mutex N times
        pmutex->pImpl->LockCount = 0;
        for(UPInt i = 0; i < lockCount; i++)
            sys_mutex_unlock(pmutex->pImpl->MutexId);
        pmutex->CallWaitHandlers();
    }
    else
    {
        pmutex->pImpl->LockCount = 0;
        sys_mutex_unlock(pmutex->pImpl->MutexId);
        pmutex->CallWaitHandlers();
    }

    if(delay == SF_WAIT_INFINITE)
        sys_cond_wait(CondvId, 0);
    else
    {   // Timeout period in microseconds
        usecond_t timeout = delay * 1000;

        int err = sys_cond_wait(CondvId, timeout);
        SF_ASSERT(err == CELL_OK || err == ETIMEDOUT);
        if(err)
            ret = false;
    }

    sys_mutex_unlock(MutexId);

    // Re-aquire the mutex
    for(UPInt i = 0; i < lockCount; i++)
        pmutex->DoLock(); 

    return ret;
}

// Notify a condition, releasing the least object in a queue
void WaitConditionImpl::Notify()
{
    sys_mutex_lock(MutexId, 0);
    sys_cond_signal(CondvId);
    sys_mutex_unlock(MutexId);
}

// Notify a condition, releasing all objects waiting
void WaitConditionImpl::NotifyAll()
{
    sys_mutex_lock(MutexId, 0);
    sys_cond_signal_all(CondvId);
    sys_mutex_unlock(MutexId);
}


// *** Actual implementation of WaitCondition

WaitCondition::WaitCondition()  { pImpl = new WaitConditionImpl; }
WaitCondition::~WaitCondition() { delete pImpl; }

// Wait for condition
bool WaitCondition::Wait(Mutex *pmutex, UPInt delay) { return pImpl->Wait(pmutex, delay); }
// Notification
void WaitCondition::Notify()    { pImpl->Notify(); }
void WaitCondition::NotifyAll() { pImpl->NotifyAll(); }


// *** Thread constructors and destructor

Thread::Thread(UPInt stackSize, int processor): Waitable(1)
{
    CreateParams params;
    params.stackSize = stackSize;
    params.processor = processor;
    Init(params);
}

Thread::Thread(Thread::ThreadFn threadFunction, void*  userHandle, UPInt stackSize,
               int processor, Thread::ThreadState initialState): Waitable(1)
{
    CreateParams params(threadFunction, userHandle, stackSize, processor, initialState);
    Init(params);
}

Thread::Thread(const CreateParams& params): Waitable(1)
{
    Init(params);
}

void Thread::Init(const CreateParams& params)
{
    ThreadFlags     = 0;
    ThreadHandle    = 0;
    ExitCode        = 0;
    SuspendCount    = 0;
    StackSize       = params.stackSize;
    Processor       = params.processor;
    Priority        = params.priority;

    ThreadFunction  = params.threadFunction;
    UserHandle      = params.userHandle;

    if(params.initialState != NotRunning)
        Start(params.initialState);
}

Thread::~Thread()
{
    ThreadHandle = 0;
}


// *** Overridable user functions

// Default Run implementation
int Thread::Run()
{
    return (ThreadFunction) ? ThreadFunction(this, UserHandle) : 0;
}
void Thread::OnExit()
{   
}


// Finishes the thread and releases internal reference to it
void  Thread::FinishAndRelease()
{
    CallableHandlers handlers;
    GetCallableHandlers(&handlers);

    ThreadFlags &= (UInt32)~(SF_THREAD_STARTED);
    ThreadFlags |= SF_THREAD_FINISHED;

    Release();
    handlers.CallWaitHandlers();
}


// *** ThreadList - used to tack all created threads

class ThreadList: public NewOverrideBase<Stat_Default_Mem>
{
    struct ThreadHashOp
    {
        size_t operator()(const Thread* ptr)
            { return (((size_t)ptr) >> 6) ^ (size_t)ptr; }
    };

    HashSet<Thread*, ThreadHashOp> ThreadSet;
    Mutex               ThreadMutex;
    WaitCondition       ThreadsEmpty;
    sys_ppu_thread_t    RootThreadId;

    static ThreadList* volatile pRunningThreads;

    void addThread(Thread *pthr)
    {
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Add(pthr);
    }

    void removeThread(Thread *pthr)
    {
        Mutex::Locker lock(&ThreadMutex);
        ThreadSet.Remove(pthr);
        if(ThreadSet.GetSize() == 0)
            ThreadsEmpty.Notify();
    }

    void finishAllThreads()
    {
        // Only original root thread can call this
        sys_ppu_thread_t thrId;
        sys_ppu_thread_get_id(&thrId);
        SF_ASSERT(thrId == RootThreadId);
        SF_UNUSED(thrId);

        Mutex::Locker lock(&ThreadMutex);
        while(ThreadSet.GetSize() != 0)
            ThreadsEmpty.Wait(&ThreadMutex);
    }
    
public:
    ThreadList()  { sys_ppu_thread_get_id(&RootThreadId); }
    ~ThreadList() {}

    static void AddRunningThread(Thread *pthr)
    {
        if(!pRunningThreads)
        {
            pRunningThreads = new ThreadList;
            SF_ASSERT(pRunningThreads);
        }
        pRunningThreads->addThread(pthr);
    }

    static void RemoveRunningThread(Thread *pthr)
    {
        SF_ASSERT(pRunningThreads);        
        pRunningThreads->removeThread(pthr);
    }

    static void FinishAllThreads()
    {
        if(pRunningThreads)
        {           
            pRunningThreads->finishAllThreads();
            delete pRunningThreads;
            pRunningThreads = 0;
        }        
    }
};

// By default, we have no thread list
ThreadList* volatile ThreadList::pRunningThreads = 0;

void Thread::FinishAllThreads()
{
    ThreadList::FinishAllThreads();
}

int Thread::PRun()
{
    if(ThreadFlags & SF_THREAD_START_SUSPENDED)
    {
        Suspend();
        ThreadFlags &= (UInt32)~SF_THREAD_START_SUSPENDED;
    }

    ExitCode = Run();    
    return ExitCode;
}

bool Thread::GetExitFlag() const
{
    return (ThreadFlags & SF_THREAD_EXIT) != 0;
}       

void Thread::SetExitFlag(bool exitFlag)
{
    if(exitFlag)
        ThreadFlags |= SF_THREAD_EXIT;
    else
        ThreadFlags &= (UInt32) ~SF_THREAD_EXIT;
}

// State functions
bool Thread::IsFinished()  const { return (ThreadFlags & SF_THREAD_FINISHED) != 0; }
bool Thread::IsSuspended() const { return SuspendCount > 0; }

Thread::ThreadState Thread::GetThreadState() const
{
    if(IsSuspended())                   return Suspended;
    if(ThreadFlags & SF_THREAD_STARTED) return Running;
    return NotRunning;
}


// ***** Thread management

// The actual first function called on thread start
void Thread_PpuThreadStartFn(uint64_t phandle)
{
    Thread* pthr = (Thread*)(uintptr_t)phandle;
    int ret = pthr->PRun();
    pthr->Exit(ret);
}

/* static */
int Thread::GetOSPriority(ThreadPriority p)
//static inline int MapToSystemPrority(Thread::ThreadPriority p)
{
    switch(p)
    {
        case Thread::CriticalPriority:    return 0;
        case Thread::HighestPriority:     return 300;
        case Thread::AboveNormalPriority: return 600;
        case Thread::NormalPriority:      return 1000;
        case Thread::BelowNormalPriority: return 1500;
        case Thread::LowestPriority:      return 2500;
        case Thread::IdlePriority:        return 3071;
    }
    return 1000;
}

bool Thread::Start(ThreadState initialState)
{
    if(initialState == NotRunning)
        return false;

    // If the thread is already running then wait until its
    // finished to begin running this thread
    if((GetThreadState() != NotRunning) && !Wait())
        return false;

    // Default values of stack size and thread priority
    size_t stackSize = PPU_THREAD_STACK_SIZE;
    int thrPrio = Thread::GetOSPriority(NormalPriority);

    ExitCode     = 0;
    SuspendCount = 0;
    ThreadFlags  = (initialState == Running) ? 0 : SF_THREAD_START_SUSPENDED;

    // AddRef to us until the thread is finished
    AddRef();
    ThreadList::AddRunningThread(this);

    if(StackSize != PPU_THREAD_STACK_SIZE || Priority != NormalPriority)
    {
        stackSize = StackSize;
        thrPrio = Thread::GetOSPriority(Priority);
    }
    int err = sys_ppu_thread_create(&ThreadHandle, Thread_PpuThreadStartFn, (uintptr_t)this,
                                    thrPrio, stackSize, 0, 0);
    SF_ASSERT(err == CELL_OK);
    if(err)
    {
        ThreadFlags = 0;
        Release();
        ThreadList::RemoveRunningThread(this);
        return false;
    }
    return true;
}

// Suspend doesn't supported
bool Thread::Suspend() { return false; }
// Resume doesn't supported
bool Thread::Resume()  { return false; }

// Quits with an exit code  
void Thread::Exit(int exitCode)
{
    OnExit();   

    FinishAndRelease();
    ThreadList::RemoveRunningThread(this);

    sys_ppu_thread_exit((uint64_t)exitCode);
}


ThreadId GetCurrentThreadId()
{
    ThreadId ret;
    sys_ppu_thread_get_id(&ret);
    return ret;
}

// *** Sleep functions

bool Thread::Sleep(UPInt secs)
{
    sys_timer_sleep(secs);
    return true;
}

bool Thread::MSleep(UPInt msecs)
{
    sys_timer_usleep(msecs * 1000);
    return true;
}

int Thread::GetCPUCount()
{
    return 2;
}


// *** Lock static member

sys_lwmutex_attribute_t Lock::LockAttr = { SYS_SYNC_PRIORITY, SYS_SYNC_RECURSIVE };

} // Scaleform

#endif  // SF_ENABLE_THREADS
