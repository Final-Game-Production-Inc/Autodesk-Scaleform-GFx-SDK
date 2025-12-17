/**************************************************************************

Filename    :   SF_ThreadsStd11.cpp
Content     :   Standard C++11 thread library support
Created     :   Nov 2012
Authors     :   Vladislav Merker

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"
#include "SF_Hash.h"

#if defined(SF_ENABLE_THREADS) && defined(SF_USE_STD11_THREADS)

#include "SF_Timer.h"
#include "SF_HeapNew.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>


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
    virtual void AddRef()                   {}
    virtual void Release(UPInt flags = 0)   {}    
};


// Acquire interface implementation
bool Mutex_AreadyLockedAcquireInterface::CanAcquire()       { return true; }
bool Mutex_AreadyLockedAcquireInterface::TryAcquire()       { return pMutex->TryAcquire(); }
bool Mutex_AreadyLockedAcquireInterface::TryAcquireCommit() { return pMutex->TryAcquireCommit(); }
bool Mutex_AreadyLockedAcquireInterface::TryAcquireCancel() { return pMutex->TryAcquireCancel(); }


// *** Internal mutex implementation structure

class MutexImpl: public NewOverrideBase<Stat_Default_Mem>
{
    std::recursive_mutex    RecurMutex;
    std::thread::id         LockedBy;
    UPInt                   LockCount;

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
    // Mutex is always recursive 
    SF_UNUSED(recursive);

    AreadyLockedAcquire.pMutex = pmutex;
    LockCount = 0;
}

MutexImpl::~MutexImpl()
{
}


// Lock and try lock
void MutexImpl::DoLock()
{
    RecurMutex.lock();
    LockCount++;
    LockedBy = std::this_thread::get_id();
}

bool MutexImpl::TryLock()
{
    if(RecurMutex.try_lock())
    {
        LockCount++;
        LockedBy = std::this_thread::get_id();
        return true;
    }    
    return false;
}

// Unlock
void MutexImpl::Unlock(Mutex* pmutex)
{
    std::thread::id thrId;
    thrId = std::this_thread::get_id();
    SF_ASSERT(thrId == LockedBy && LockCount > 0);
    SF_UNUSED(thrId);

    UPInt lockCount;
    LockCount--;
    lockCount = LockCount;

    Mutex::CallableHandlers handlers;
    pmutex->GetCallableHandlers(&handlers);

    RecurMutex.unlock();

    if(lockCount == 0)
        handlers.CallWaitHandlers();
}

bool MutexImpl::IsLockedByAnotherThread() const
{
    if(LockCount == 0)
        return false;

    std::thread::id thrId;
    thrId = std::this_thread::get_id();
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
    std::mutex              CondMutex;
    std::condition_variable CondVar;

public:
    WaitConditionImpl()  {};
    ~WaitConditionImpl() {};

    // Release mutex and wait for condition. The mutex is re-aqured after the wait
    bool Wait(Mutex *pmutex, unsigned delay = SF_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    // and releasing all objects waiting
    void Notify();
    void NotifyAll();
};


// Release mutex and wait for condition. The mutex is re-aqured after the wait
bool WaitConditionImpl::Wait(Mutex *pmutex, unsigned delay)
{
    bool ret = true;
    UPInt lockCount = pmutex->pImpl->LockCount;

    // Mutex must have been locked
    if(lockCount == 0)
        return false;

    std::unique_lock<std::mutex> lockCond(CondMutex);

    // Release the recursive mutex
    pmutex->pImpl->LockCount = 0;
    for(UPInt i = 0; i < lockCount; i++)
        pmutex->pImpl->RecurMutex.unlock();
    pmutex->CallWaitHandlers();

    if(delay == SF_WAIT_INFINITE)
        CondVar.wait(lockCond);
    else {
        auto timeout = std::chrono::microseconds(delay);
        if(CondVar.wait_for(lockCond, timeout) == std::cv_status::timeout)
            ret = false;
    }

    lockCond.unlock();

    // Re-aquire the mutex
    for(UPInt i = 0; i < lockCount; i++)
        pmutex->DoLock(); 

    return ret;
}

// Notify a condition, releasing the least object in a queue
void WaitConditionImpl::Notify()
{
    std::unique_lock<std::mutex> lockCond(CondMutex);
    CondVar.notify_one();
}

// Notify a condition, releasing all objects waiting
void WaitConditionImpl::NotifyAll()
{
    std::unique_lock<std::mutex> lockCond(CondMutex);
    CondVar.notify_all();
}


// *** Actual implementation of WaitCondition

WaitCondition::WaitCondition()  { pImpl = new WaitConditionImpl; }
WaitCondition::~WaitCondition() { delete pImpl; }

// Wait for condition
bool WaitCondition::Wait(Mutex *pmutex, unsigned delay) { return pImpl->Wait(pmutex, delay); }
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
    if (ThreadHandle)
        delete ThreadHandle;
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
    std::thread::id     RootThreadId;

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
        std::thread::id thrId;
        thrId = std::this_thread::get_id();
        SF_ASSERT(thrId == RootThreadId);
        SF_UNUSED(thrId);

        Mutex::Locker lock(&ThreadMutex);
        while(ThreadSet.GetSize() != 0)
            ThreadsEmpty.Wait(&ThreadMutex);
    }
    
public:
    ThreadList()  { RootThreadId = std::this_thread::get_id(); }
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

int Thread::GetOSPriority(ThreadPriority p)
{
    switch(p)
    {
    case Thread::CriticalPriority:      return THREAD_PRIORITY_TIME_CRITICAL;
    case Thread::HighestPriority:       return THREAD_PRIORITY_HIGHEST;
    case Thread::AboveNormalPriority:   return THREAD_PRIORITY_ABOVE_NORMAL;
    case Thread::NormalPriority:        return THREAD_PRIORITY_NORMAL;
    case Thread::BelowNormalPriority:   return THREAD_PRIORITY_BELOW_NORMAL;
    case Thread::LowestPriority:        return THREAD_PRIORITY_LOWEST;
    case Thread::IdlePriority:          return THREAD_PRIORITY_IDLE;
    }
    return THREAD_PRIORITY_NORMAL;
}

// The actual first function called on thread start
void Thread_Std11StartFunc(void* phandle)
{
    Thread* pthr = (Thread*)phandle;
    int ret = pthr->PRun();
    pthr->Exit(ret);
}

bool Thread::Start(ThreadState initialState)
{
    if(initialState == NotRunning)
        return false;

    // If the thread is already running then wait until its
    // finished to begin running this thread
    if((GetThreadState() != NotRunning) && !Wait())
        return false;

    ExitCode     = 0;
    SuspendCount = 0;
    ThreadFlags  = (initialState == Running) ? SF_THREAD_STARTED : SF_THREAD_START_SUSPENDED;

    // AddRef to us until the thread is finished
    AddRef();
    ThreadList::AddRunningThread(this);

    // Create a thread
    try {
        ThreadHandle = new std::thread(Thread_Std11StartFunc, this);
        IdValue = ThreadHandle->get_id();
        ThreadHandle->detach();
    }
    catch(const std::system_error& e) 
    {
        ThreadFlags = 0;
        Release();
        ThreadList::RemoveRunningThread(this);

        SF_UNUSED(e);
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

    SF_UNUSED(exitCode);
}

// Returns the unique Id of a thread it is called on, intended for comparison purposes.
// Using a thread local variable to cache the thread id - this_thread::get_id() is slow
const ThreadId g_InvalidThreadID = 0xFFFFFF;
__declspec(thread) ThreadId g_ThreadID = g_InvalidThreadID;
ThreadId GetCurrentThreadId()
{
    if (g_ThreadID == g_InvalidThreadID)
        g_ThreadID = std::this_thread::get_id().hash();
    return g_ThreadID;
}
#if 0
ThreadId GetCurrentThreadId()
{
    std::thread::id thrId = std::this_thread::get_id();
    return thrId.hash();
}
#endif

// *** Sleep functions

bool Thread::Sleep(unsigned secs)
{
    std::this_thread::sleep_for(std::chrono::seconds(secs));
    return true;
}

bool Thread::MSleep(unsigned msecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds(msecs));
    return true;
}

int Thread::GetCPUCount()
{
    auto cnt = std::thread::hardware_concurrency();
    // If cnt value is not well defined or not computable, return 1
    return cnt > 0 ? cnt : 1;
}


} // Scaleform

#endif // SF_ENABLE_THREADS && SF_USE_STD11_THREADS
