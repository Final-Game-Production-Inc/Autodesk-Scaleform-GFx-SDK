/**************************************************************************

Filename    :   SF_ThreadsWinAPI.cpp
Platform    :   WinAPI, XBox360
Content     :   Windows specific thread-related (safe) functionality
Created     :   May 5, 2003
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"
#include "SF_Hash.h"
#include "SF_Debug.h"

#ifdef SF_ENABLE_THREADS

// For _beginthreadex / _endtheadex
#include <process.h>

namespace Scaleform {

#ifdef SF_OS_WINMETRO

#define CreateMutex(attr,locked,name) CreateMutexExW(attr,name,locked,STANDARD_RIGHTS_ALL)
#define CreateSemaphore(attr,initial,maxv,name) CreateSemaphoreExW(attr,initial,maxv,name,0,STANDARD_RIGHTS_ALL)
#define CreateEvent(attr,reset,flags,name) CreateEventExW(attr,name,flags,STANDARD_RIGHTS_ALL | EVENT_MODIFY_STATE)
#define WaitForSingleObject(h,time) WaitForSingleObjectEx(h,time,0)

#endif

// ***** Win32 Mutex implementation

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
    virtual void    AddRef()                            { }
    virtual void    Release(unsigned flags=0)               { SF_UNUSED(flags); }
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
    HANDLE            hMutexOrSemaphore;
    bool              Recursive;
    volatile unsigned LockCount;

    Mutex_AreadyLockedAcquireInterface AreadyLockedAcquire;
    
    friend class WaitConditionImpl;


public:
    // Constructor/destructor
    MutexImpl(Mutex* pmutex, bool recursive = 1);
    ~MutexImpl();
/*
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
    // Allocation - ensure we use our allocator
    void *  operator new(UPInt sz)     
        { return SF_STATICALLOC(sz); }
    void    operator delete(void* pmem) 
        { SF_STATICFREE(pmem); }
    void*   operator new(UPInt sz, int blocktype, const char* pfilename, int line)
        { return SF_STATICALLOC(sz); }
    void    operator delete(void *p, int blocktype, const char* pfilename, int line)
        { SF_STATICFREE(p); }
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif
*/
    // Locking functions
    void                DoLock();
    bool                TryLock();
    void                Unlock(Mutex* pmutex);
    // Returns 1 if the mutes is currently locked
    bool                IsLockedByAnotherThread(Mutex* pmutex);        
    bool                IsSignaled() const;
    AcquireInterface*   GetAcquireInterface(Mutex* pmutex);
};



// *** Constructor/destructor
MutexImpl::MutexImpl(Mutex* pmutex, bool recursive)
{   
    AreadyLockedAcquire.pMutex  = pmutex;
    Recursive                   = recursive;
    LockCount                   = 0;
    hMutexOrSemaphore           = Recursive ? CreateMutex(NULL, 0, NULL) : CreateSemaphore(NULL, 1, 1, NULL);
}
MutexImpl::~MutexImpl()
{
    CloseHandle(hMutexOrSemaphore);
}


// Lock and try lock
void MutexImpl::DoLock()
{
    if (::WaitForSingleObject(hMutexOrSemaphore, INFINITE) != WAIT_OBJECT_0)
        return;
    LockCount++;
}

bool MutexImpl::TryLock()
{
    DWORD ret;
    if ((ret=::WaitForSingleObject(hMutexOrSemaphore, 0)) != WAIT_OBJECT_0)
        return 0;
    LockCount++;
    return 1;
}

void MutexImpl::Unlock(Mutex* pmutex)
{
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
    if ((Recursive ? ReleaseMutex(hMutexOrSemaphore) :
                     ReleaseSemaphore(hMutexOrSemaphore, 1, NULL))  != 0)
    {
        // Call wait handlers indirectly here in case owner
        // destroys mutex after finishing wait on it.
        if (lockCount == 0)
            handlers.CallWaitHandlers();
    }
}

bool    MutexImpl::IsLockedByAnotherThread(Mutex* pmutex)
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




// ***** Win32 Wait Condition Implementation

// Internal implementation class
class WaitConditionImpl : public NewOverrideBase<Stat_Default_Mem>
{   
    // Event pool entries for extra events
    struct EventPoolEntry  : public NewOverrideBase<Stat_Default_Mem>
    {
        HANDLE          hEvent;
        EventPoolEntry  *pNext;
        EventPoolEntry  *pPrev;
    };
    
    Lock                WaitQueueLoc;
    // Stores free events that can be used later
    EventPoolEntry  *   pFreeEventList;
    
    // A queue of waiting objects to be signaled    
    EventPoolEntry*     pQueueHead;
    EventPoolEntry*     pQueueTail;

    // Allocation functions for free events
    EventPoolEntry*     GetNewEvent();
    void                ReleaseEvent(EventPoolEntry* pevent);

    // Queue operations
    void                QueuePush(EventPoolEntry* pentry);
    EventPoolEntry*     QueuePop();
    void                QueueFindAndRemove(EventPoolEntry* pentry);


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
    pFreeEventList  = 0;
    pQueueHead      =
    pQueueTail      = 0;
}

WaitConditionImpl::~WaitConditionImpl()
{
    // Free all the resources
    EventPoolEntry* p       = pFreeEventList;
    EventPoolEntry* pentry;

    while(p)
    {
        // Move to next
        pentry = p;
        p = p->pNext;
        // Delete old
        ::CloseHandle(pentry->hEvent);
        delete pentry;  
    }   
    // Shouldn't we also consider the queue?

    // To be safe
    pFreeEventList  = 0;
    pQueueHead      =
    pQueueTail      = 0;
}


// Allocation functions for free events
WaitConditionImpl::EventPoolEntry* WaitConditionImpl::GetNewEvent()
{
    EventPoolEntry* pentry;

    // If there are any free nodes, use them
    if (pFreeEventList)
    {
        pentry          = pFreeEventList;
        pFreeEventList  = pFreeEventList->pNext;        
    }
    else
    {
        // Allocate a new node
        pentry          = new EventPoolEntry;
        pentry->pNext   = 0;
        pentry->pPrev   = 0;
        // Non-signaled manual event
        pentry->hEvent  = ::CreateEvent(NULL, TRUE, 0, NULL);
    }
    
    return pentry;
}

void        WaitConditionImpl::ReleaseEvent(EventPoolEntry* pevent)
{
    // Mark event as non-signaled
    ::ResetEvent(pevent->hEvent);
    // And add it to free pool
    pevent->pNext   = pFreeEventList;
    pevent->pPrev   = 0;
    pFreeEventList  = pevent;
}

// Queue operations
void        WaitConditionImpl::QueuePush(EventPoolEntry* pentry)
{
    // Items already exist? Just add to tail
    if (pQueueTail)
    {
        pentry->pPrev       = pQueueTail;
        pQueueTail->pNext   = pentry;
        pentry->pNext       = 0;        
        pQueueTail          = pentry;       
    }
    else
    {
        // No items in queue
        pentry->pNext   = 
        pentry->pPrev   = 0;
        pQueueHead      =
        pQueueTail      = pentry;
    }
}

WaitConditionImpl::EventPoolEntry* WaitConditionImpl::QueuePop()
{
    EventPoolEntry* pentry = pQueueHead;

    // No items, null pointer
    if (pentry)
    {
        // More items after this one? just grab the first item
        if (pQueueHead->pNext)
        {       
            pQueueHead  = pentry->pNext;
            pQueueHead->pPrev = 0;      
        }
        else
        {
            // Last item left
            pQueueTail =
            pQueueHead = 0;
        }
    }   
    return pentry;
}

void        WaitConditionImpl::QueueFindAndRemove(EventPoolEntry* pentry)
{
    // Do an exhaustive search looking for an entry
    EventPoolEntry* p = pQueueHead;

    while(p)
    {
        // Entry found? Remove.
        if (p == pentry)
        {
            
            // Remove the node form the list
            // Prev link
            if (pentry->pPrev)
                pentry->pPrev->pNext = pentry->pNext;
            else
                pQueueHead = pentry->pNext;
            // Next link
            if (pentry->pNext)
                pentry->pNext->pPrev = pentry->pPrev;
            else
                pQueueTail = pentry->pPrev;
            // Done
            return;
        }

        // Move to next item
        p = p->pNext;
    }
}
    

bool    WaitConditionImpl::Wait(Mutex *pmutex, unsigned delay)
{
    bool            result = 0;
    unsigned        i;
    unsigned        lockCount = pmutex->pImpl->LockCount;
    EventPoolEntry* pentry;

    // Mutex must have been locked
    if (lockCount == 0)
        return 0;
    
    // Add an object to the wait queue
    WaitQueueLoc.DoLock();
    QueuePush(pentry = GetNewEvent());
    WaitQueueLoc.Unlock();

    // Finally, release a mutex or semaphore
    if (pmutex->pImpl->Recursive)
    {
        // Release the recursive mutex N times
        pmutex->pImpl->LockCount = 0;
        for(i=0; i<lockCount; i++)
            ::ReleaseMutex(pmutex->pImpl->hMutexOrSemaphore);
        // NOTE: Do not need to use CallableHanders here because mutex
        // can not be destroyed by user if we are to re-acquire it later.
        pmutex->CallWaitHandlers();
    }
    else
    {
        pmutex->pImpl->LockCount = 0;
        ::ReleaseSemaphore(pmutex->pImpl->hMutexOrSemaphore, 1, NULL);
        pmutex->CallWaitHandlers();
    }

    // Note that there is a gap here between mutex.Unlock() and Wait(). However,
    // if notify() comes in at this point in the other thread it will set our
    // corresponding event so wait will just fall through, as expected.

    // Block and wait on the event
    DWORD waitResult = ::WaitForSingleObject(pentry->hEvent,
                            (delay == SF_WAIT_INFINITE) ? INFINITE : delay);
    /*
repeat_wait:
    DWORD waitResult =

    ::MsgWaitForMultipleObjects(1, &pentry->hEvent, FALSE,
                                (delay == SF_WAIT_INFINITE) ? INFINITE : delay,
                                QS_ALLINPUT);
    */

    WaitQueueLoc.DoLock();
    switch(waitResult)
    {
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0: 
            result = 1;
            // Wait was successful, therefore the event entry should already be removed
            // So just add entry back to a free list
            ReleaseEvent(pentry);
            break;
            /*
        case WAIT_OBJECT_0 + 1:
            // Messages in WINDOWS queue
            {
                MSG msg;
                PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);             
                WaitQueueLoc.Unlock();
                goto repeat_wait;
            }
            break; */
        default:
            // Timeout, our entry should still be in a queue
            QueueFindAndRemove(pentry);
            ReleaseEvent(pentry);
    }
    WaitQueueLoc.Unlock();

    // Re-aquire the mutex
    for(i=0; i<lockCount; i++)
        pmutex->DoLock(); 

    // Return the result
    return result;
}

// Notify a condition, releasing the least object in a queue
void    WaitConditionImpl::Notify()
{
    Lock::Locker   lock(&WaitQueueLoc);
    
    // Pop last entry & signal it
    EventPoolEntry* pentry = QueuePop();    
    if (pentry)
        ::SetEvent(pentry->hEvent); 
}

// Notify a condition, releasing all objects waiting
void    WaitConditionImpl::NotifyAll()
{
    Lock::Locker   lock(&WaitQueueLoc);

    // Pop and signal all events
    // NOTE : There is no need to release the events, it's the waiters job to do so 
    EventPoolEntry* pentry = QueuePop();
    while (pentry)
    {
        ::SetEvent(pentry->hEvent);
        pentry = QueuePop();
    }
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
    
// Wait without a mutex
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

//  Per-thread variable
//  MA: Don't use TLS for now - portability issues with DLLs on 360, etc.
/*
#if !defined(SF_CC_MSVC) || (SF_CC_MSVC < 1300)
__declspec(thread)  Thread*    pCurrentThread      = 0;
#else
#pragma data_seg(".tls$")
__declspec(thread)  Thread*    pCurrentThread      = 0;
#pragma data_seg(".rwdata")
#endif
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
    IdValue         = 0;
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
    CleanupSystemThread();
    ThreadHandle = 0;
}


// *** Overridable User functions.

// Default Run implementation
int     Thread::Run()
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
        UPInt operator()(const Thread* ptr)
        {
            return (((UPInt)ptr) >> 6) ^ (UPInt)ptr;
        }
    };

    HashSet<Thread*, ThreadHashOp>  ThreadSet;
    Mutex                           ThreadMutex;
    WaitCondition                   ThreadsEmpty;
    // Track the root thread that created us.
    ThreadId                        RootThreadId;

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
        Mutex::Locker lock(&ThreadMutex);
        while (ThreadSet.GetSize() != 0)
            ThreadsEmpty.Wait(&ThreadMutex);
    }

public:

    ThreadList()
    {
        RootThreadId = GetCurrentThreadId();
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

int     Thread::PRun()
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



/* MA: Don't use TLS for now.

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
/* static */
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
unsigned WINAPI Thread_Win32StartFn(void * phandle)
{
    Thread *   pthread = (Thread*)phandle;
    if (pthread->Processor != -1)
    {
#if defined(SF_OS_XBOX360)
        DWORD ret = XSetThreadProcessor(GetCurrentThread(), pthread->Processor);
        SF_DEBUG_ERROR(ret == (DWORD)-1, "Could not set hardware processor for the thread");
        SF_UNUSED(ret);
#elif defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
        DWORD_PTR ret = SetThreadAffinityMask(GetCurrentThread(), (DWORD)pthread->Processor);
        SF_DEBUG_ERROR(ret == 0, "Could not set hardware processor for the thread");
        SF_UNUSED(ret);
#endif
    }
#if !defined(SF_OS_WINMETRO)
    BOOL ret = ::SetThreadPriority(GetCurrentThread(), Thread::GetOSPriority(pthread->Priority));
    SF_DEBUG_ERROR(ret == 0, "Could not set thread priority");
    SF_UNUSED(ret);
#endif
    // Ensure that ThreadId is assigned once thread is running, in case
    // beginthread hasn't filled it in yet.
    pthread->IdValue = reinterpret_cast<Scaleform::ThreadId>(static_cast<uintptr_t>(::GetCurrentThreadId()));

    DWORD       result = pthread->PRun();
    // Signal the thread as done and release it atomically.
    pthread->FinishAndRelease();
    // At this point Thread object might be dead; however we can still pass
    // it to RemoveRunningThread since it is only used as a key there.    
    ThreadList::RemoveRunningThread(pthread);
    return (unsigned) result;
}

bool    Thread::Start(ThreadState initialState)
{
    if (initialState == NotRunning)
        return 0;

    // If the thread is already running then wait
    // until its finished to begin running this thread
    if ((GetThreadState() != NotRunning) && !Wait())
        return 0;

    // Free old thread handle before creating the new one
    CleanupSystemThread();

    // AddRef to us until the thread is finished.
    AddRef();
    ThreadList::AddRunningThread(this);
    
    ExitCode        = 0;
    SuspendCount    = 0;
    ThreadFlags     = (initialState == Running) ? SF_THREAD_STARTED : SF_THREAD_START_SUSPENDED;
    ThreadHandle = (HANDLE) _beginthreadex(0, (unsigned)StackSize,
                                           Thread_Win32StartFn, this, 0, (unsigned*)&IdValue);

    // Failed? Fail the function
    if (ThreadHandle == 0)
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
    // Can't suspend a thread that wasn't started
    if (!(ThreadFlags & SF_THREAD_STARTED))
        return 0;

#if !defined(SF_OS_WINMETRO)
    if (::SuspendThread(ThreadHandle) != 0xFFFFFFFF)
    {        
        SuspendCount++;        
        return 1;
    }
#endif
    return 0;
}

// Resumes currently suspended thread
bool    Thread::Resume()
{
    // Can't suspend a thread that wasn't started
    if (!(ThreadFlags & SF_THREAD_STARTED))
        return 0;

#if !defined(SF_OS_WINMETRO)
	// Decrement count, and resume thread if it is 0
    SInt32 oldCount = SuspendCount.ExchangeAdd_Acquire(-1);
    if (oldCount >= 1)
    {
        if (oldCount == 1)
        {
            if (::ResumeThread(ThreadHandle) != 0xFFFFFFFF)            
                return 1;            
        }
        else
        {
            return 1;
        }
    }   
#endif
    return 0;
}


// Quits with an exit code  
void    Thread::Exit(int exitCode)
{
    // Can only exist the current thread.
    // MA: Don't use TLS for now.
    //if (GetThread() != this)
    //    return;

    // Call the virtual OnExit function.
    OnExit();   

    // Signal this thread object as done and release it's references.
    FinishAndRelease();
    ThreadList::RemoveRunningThread(this);

    // Call the exit function.
#ifndef SF_OS_WINMETRO
    _endthreadex((unsigned)exitCode);
#endif

}


void    Thread::CleanupSystemThread()
{
    if (ThreadHandle != 0)
    {
        ::CloseHandle(ThreadHandle);
        ThreadHandle = 0;
    }
}

// *** Sleep functions
// static
bool    Thread::Sleep(unsigned secs)
{
	return MSleep(secs*1000);
}

// static
bool    Thread::MSleep(unsigned msecs)
{
#if defined(SF_OS_WINMETRO)
	WaitForMultipleObjectsEx(0, 0, TRUE, msecs, FALSE);
#else
    ::Sleep(msecs);
#endif
    return 1;
}

void    Thread::SetThreadName( const char* name )
{
    SF_UNUSED(name);
#if !defined(SF_BUILD_SHIPPING) || defined(SF_BUILD_PROFILING)
    // Looks ugly, but it is the recommended way to name a thread.
    typedef struct tagTHREADNAME_INFO {
        DWORD dwType;     // Must be 0x1000
        LPCSTR szName;    // Pointer to name (in user address space)
        DWORD dwThreadID; // Thread ID (-1 for caller thread)
        DWORD dwFlags;    // Reserved for future use; must be zero
    } THREADNAME_INFO;

    THREADNAME_INFO info;

    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = static_cast<DWORD>(reinterpret_cast<uintptr_t>(GetThreadId()));
    info.dwFlags = 0;

    __try
    {
#ifdef _WIN64
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (const ULONG_PTR *)&info );
#else
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
#endif
    }
    __except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
    {
    }
#else
    SF_UNUSED(name);
#endif // SF_BUILD_SHIPPING
}

/* static */
int     Thread::GetCPUCount()
{
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwNumberOfProcessors;
#elif defined(SF_OS_XBOX)
    return 1;
#elif defined(SF_OS_XBOX360)
    return 6;
#else
	return 1;
#endif
}

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId()
{
    return reinterpret_cast<Scaleform::ThreadId>(static_cast<uintptr_t>(::GetCurrentThreadId()));
}

} // Scaleform

#endif


