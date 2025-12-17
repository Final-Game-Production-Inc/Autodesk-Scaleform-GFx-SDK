/**************************************************************************

PublicHeader:   None
Filename    :   SF_Threads.h
Content     :   Contains thread-related (safe) functionality
Created     :   May 5, 2003
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Threads_H
#define INC_SF_Kernel_Threads_H

#include "SF_Types.h"
#include "SF_Atomic.h"
#include "SF_RefCount.h"
#include "SF_Array.h"

#if defined(SF_OS_WINMETRO)
#include <ppl.h>
#if defined(SF_USE_STD11_THREADS)
#include <thread>
#endif
#endif

#if defined(SF_OS_WII) || defined(SF_OS_WIIU)
#include <setjmp.h>
#include "SF_List.h"

#elif defined(SF_OS_3DS)
#include <setjmp.h>
#include <nn/os/os_Thread.h>
#endif

// Defines the infinite wait delay timeout
#define SF_WAIT_INFINITE       0xFFFFFFFF


namespace Scaleform {

// ****** Declared classes

// To be defined in the project configuration options
#ifdef SF_ENABLE_THREADS

// Declared with thread support only:
class   Waitable;
class   AcquireInterface;
class   Mutex;
class   WaitCondition;
class   Event;
class   Semaphore;

#else
//
// declare stubs for non threaded build
//

typedef void* ThreadId;

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId();

// Mutex
class Mutex 
{
public:
   Mutex(bool recursive = 1, bool multiWait = 0) { SF_UNUSED2(recursive, multiWait); }
   void  DoLock()    { }
   bool  TryLock()   { return false; }
   void  Unlock()    { }
   bool  IsLockedByAnotherThread()       { return false; }
   bool    IsSignaled() const    { return false; }
   void* GetAcquireInterface()   { return NULL; }
   bool    CanAcquire()          { return false; }
   bool    TryAcquire()          { return false; }  
   bool    TryAcquireCommit()    { return false; }
   bool    TryAcquireCancel()    { return false; }

    class Locker
    {
    public:
        Mutex *pMutex;
        Locker(Mutex *pmutex) : pMutex(0) { SF_UNUSED(pmutex); }
    };
};

// WaitCondition
class WaitCondition 
{
public:
    bool    Wait(Mutex *pmutex, unsigned delay = SF_WAIT_INFINITE) { SF_UNUSED2(pmutex, delay); return false; }
    void    Notify()      { }
    void    NotifyAll()   { }
};

// Thread
class Thread : public RefCountBase<Thread, Stat_Default_Mem>
{
public:
    enum ThreadState
    {
        NotRunning  = 0,
        Running     = 1,
        Suspended   = 2
    };
    enum ThreadPriority
    {
        CriticalPriority,
        HighestPriority,
        AboveNormalPriority,
        NormalPriority,
        BelowNormalPriority,
        LowestPriority,
        IdlePriority,
    };
    typedef int (*ThreadFn)(Thread *pthread, void* h);
    struct CreateParams
    {
        CreateParams(ThreadFn func = 0, void* hand = 0, UPInt ssize = 128 * 1024, 
            int proc = -1, ThreadState state = NotRunning, ThreadPriority prior = NormalPriority) 
        { SF_UNUSED6(func, hand, ssize, proc, state, prior); }
    };

    Thread(UPInt stackSize = 128 * 1024, int processor = -1) { SF_UNUSED2(stackSize, processor); }
    Thread(ThreadFn threadFunction, void*  userHandle = 0, UPInt stackSize = 128 * 1024,
        int processor = -1, ThreadState initialState = NotRunning) { SF_UNUSED5(threadFunction, userHandle, stackSize, processor, initialState); }
    explicit SF_EXPORT Thread(const CreateParams& params) { SF_UNUSED(params); }
    static  void SF_CDECL FinishAllThreads() { }
    int   Run()   { return 0; }
    void  OnExit(){ }
    bool  Start(ThreadState initialState = Running) { SF_UNUSED(initialState); return true; }
    void  Exit(int exitCode=0)    { SF_UNUSED(exitCode); }
    bool  Suspend()   { return true; }
    bool  Resume()    { return true; }

    bool          GetExitFlag() const     { return true; }
    void          SetExitFlag(bool exitFlag) { SF_UNUSED(exitFlag); }
    bool          IsFinished() const      { return true; }
    bool          IsSuspended() const     { return false; }
    ThreadState   GetThreadState() const  { return NotRunning; }
    static int GetCPUCount()              { return 1; }
    SF_INLINE int           GetExitCode() const { return 0; }

    // Returns an OS handle 
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
    void*          GetOSHandle() const { return NULL; }
#elif defined(SF_OS_WII) || defined(SF_OS_WIIU)
    OSThread *     GetOSHandle() const { return NULL; }
#elif defined(SF_OS_PS3)
    sys_ppu_thread_t GetOSHandle() const { return 0; }
#elif defined(SF_OS_PSVITA)
    SceUID         GetOSHandle() const { return 0; }
#elif defined(SF_OS_3DS)
    nn::os::Thread* GetOSHandle() const { return NULL; }
#else
    pthread_t      GetOSHandle() const { return 0; }
#endif

    ThreadId       GetThreadId() const { return 0; }

    static int               GetOSPriority(ThreadPriority)  { return 0; }
    static bool    Sleep(unsigned secs)           { SF_UNUSED(secs); return false; }
    static bool    MSleep(unsigned msecs)         { SF_UNUSED(msecs); return false; }

    bool   IsSignaled() const {  return false; }
    void*  GetAcquireInterface() { return this; }
    bool    CanAcquire() { return IsSignaled(); }
    bool    TryAcquire() { return IsSignaled(); }
    void    SetThreadName( const char* name ) { SF_UNUSED(name); }
#if defined(SF_OS_WII) || defined(SF_OS_WIIU)
public: // for Wii compiler 4.3 145
    struct ThreadDataWii : public ListNode<ThreadDataWii>
    {
        OSThread        ThreadHandle;
        jmp_buf         ExitJmp;
        UByte           Stack[32] __attribute__((aligned(32)));
    };
#endif
};

// Event
class Event
{
public:
    Event(bool setInitially = 0, bool multiWait = 0) { SF_UNUSED2(setInitially, multiWait); }
    bool  Wait(unsigned delay = SF_WAIT_INFINITE) { SF_UNUSED(delay); return true; } 
    void  SetEvent()  { }
    void  ResetEvent(){ }
    void  PulseEvent(){ }
    bool  IsSignaled() const  { return false; }
    void*  GetAcquireInterface() { return NULL; }
    bool    CanAcquire()      { return false; }
    bool    TryAcquire()      { return false; }
    bool    TryAcquireCommit(){ return false; }
    bool    TryAcquireCancel(){ return false; }
};
#endif


// The rest of the classes are only defined when thread support is enabled.
#ifdef SF_ENABLE_THREADS


// ***** Waitable and Acquisition Interface

/*
    AcquireInterface provides a two-stage acquisition interface for a resource. This interface is used
    primarily to acquire multiple objects that can be shared among threads. For this reason,
    AcquireInterface is often used together with Waitable. Whenever a waitable object changes state
    indicating that a shared resource is potentially available, this abstract acquisition interface is used to 
    acquire the resource in a resource-independent manner.
              
    In the first stage of acquisition TryAcquire is called to see if the resource can be obtained. If it succeeds, 
    the resource is considered to be acquired, and TryAcquireCommit is called to confirm the ownership. If the
    resource is, however, not needed (because, for example, acquisition of another object in a multi-object wait
    has failed) TryAcquireCancel can be called. 
*/


class   AcquireInterface
{
public:
    SF_EXPORT virtual ~AcquireInterface() { }
    
    // Call this function to test if an object can be acquired
    // Like TryAcquire() and TryAcquireCancel() pair, except does not actually try to acquire the object
    // This function is called from a different thread then the acquisition waiting thread, it is used to check
    // for potential availability of resource during Wait/Acquire functions
    SF_EXPORT virtual bool    CanAcquire();

    // These functions implement a two-stage acquiring protocol used by the AcquireMultiple functions.
    // First, TryAcquire is called to capture resource. If it succeeded, 
    // TryAcquireCommit is called to permanently obtain the resource.
    // TryAcquireCancel can be called instead of TryAcquireCommit
    // in order to release the resource, and leave it untouched.
    // These functions are only called from the waiter thread by the AcquireMultiple functions.
    
    // Try to acquire the resource, return 1 if succeeded.
    SF_EXPORT virtual bool    TryAcquire();
    // Complete resource acquisition. Should not fail in general.
    SF_EXPORT virtual bool    TryAcquireCommit();
    // Cancel resource acquisition. Should not fail in general.
    SF_EXPORT virtual bool    TryAcquireCancel();

    // Static function to acquire multiple objects simultaneously
    SF_EXPORT static  bool    AcquireMultipleObjects(Waitable** waitList, unsigned waitCount, 
                                                     unsigned delay = SF_WAIT_INFINITE);
    // Static function to acquire one of multiple objects in a list
    // Returns the index of the object acquired, -1 for fail (wait expired)
    SF_EXPORT static  int     AcquireOneOfMultipleObjects(Waitable** waitList, unsigned waitCount, 
                                                          unsigned delay = SF_WAIT_INFINITE);
};



// Static acquire interface that doesn't do anything
// All acquire calls on this interface always succeed.

class   DefaultAcquireInterface : public AcquireInterface
{
public:
    SF_EXPORT virtual ~DefaultAcquireInterface() { }

    // Does nothing

    SF_EXPORT static DefaultAcquireInterface* GetDefaultAcquireInterface();
};



/*
    Waitable interface is used for thread-shared objects that need to be waited on. A waitable object
    has an associated 'Signalled' state such that an object is signaled when its associated resource is available,
    and non-signaled when it is not available. Some objects, such as a Semaphore, can have multiple internal signaled
    states; in this case the object is considered to be signaled if ANY amount of a resource is available, and
    non-signaled if NO resource is available.

    Waiting on a Waitable object is achieved with help of handlers installed by the waiter threads. Whenever the
    objects state changes from non-signaled to signaled, or from one internal gradation of signaled to another,
    wait handlers are called (handlers should not get called when the object changes its state from signaled to
    non-signaled). When called, wait handlers can be used to wake up a waiter thread, so that it can compete for the
    acquisition of the object's associated resource. The actual acquisition of the resource can happen either through
    object-specific methods for with help of an associated AcquireInterface.
*/

class   Waitable : public RefCountBase<Waitable, Stat_Default_Mem>
{
public:

    // Wait handler type
    typedef void (*WaitHandler)(void* pdata);    

protected:
    class   HandlerStruct 
    {
    public: 
        WaitHandler Handler;
        void*       pUserData;

        // Default constructor - only used in garray.
        HandlerStruct()
        {
            Handler   = 0;
            pUserData = 0;
        }

        HandlerStruct(WaitHandler h, void*  pd)
        {
            Handler     = h;
            pUserData   = pd;
        }
        HandlerStruct(const HandlerStruct &src)
        {
            Handler     = src.Handler;
            pUserData   = src.pUserData;
        }

        HandlerStruct & operator = (const HandlerStruct &src)
        {
            Handler     = src.Handler;
            pUserData   = src.pUserData;
            return *this;
        }

        bool operator == (const HandlerStruct &src)
        {
            return (Handler == src.Handler) && (pUserData==src.pUserData);
        }
    };

    // Handler array and lock are allocated in a separate ref-counted object
    // so that their lifetime can be extended past the waitable object in
    // case other threads are still executing wait handlers. HandlerArray is
    // only allocated if multiWait was passed in a contsructor; otherwise
    // it is null.
    struct HandlerArray : public NewOverrideBase<Stat_Default_Mem>
    {
        typedef ArrayConstPolicy<0, 16, true> SizePolicyType; // MinCapacity=0, Granularity=16, NeverShrink
        typedef Array<HandlerStruct, Stat_Default_Mem, SizePolicyType> HandlerArrayType;

        AtomicInt<SInt32>       RefCount;
        HandlerArrayType        Handlers;
        Lock                    HandlersLock;

        HandlerArray() { RefCount.Value = 1; }

        void    AddRef()    { RefCount++; }
        // Decrement ref count. This needs to be thread-safe, since
        // a different thread could have also decremented the ref count.
        SF_EXPORT void  Release();

        // Calls all of the installed handlers during a lock.
        SF_EXPORT void  CallWaitHandlers();        
    };

    // A pointer to handler array, only allocated if 'enabled' flag was passed
    // in a constructor. Such setup is done to save memory and improve performance
    // of non-multiwait Mutex. Unfortunately, lazy initialization can not
    // be used for pHandlers, since access of this variable must be done before
    // system mutex Unlock, and thus can occur before AddHandlers in multi-object wait. 
    HandlerArray*      pHandlers;

public:

    // Enable flag must be passed to enable WaitHandlers; if not specified the
    // wait handlers and multi-object wait functionality is not available.
    Waitable(bool enable);
    
    virtual ~Waitable();

    // Invoke the associated wait handlers; only safe if we know Waitable could not
    // have died in response to it being signaled (otherwise GetCallableHandlers should be used).
    SF_EXPORT void            CallWaitHandlers();

    // A handle used to access wait handlers after waitable has been signaled
    // and thus can not be accessed directly (other threads can kill the object).    
    class  CallableHandlers
    {
        friend class Waitable;
        Ptr<HandlerArray>  pArray;        
    public:
        CallableHandlers() { }                
        // Calls handlers in a handle.
        inline void         CallWaitHandlers() { if (pArray) pArray->CallWaitHandlers(); }
    };
    
    inline  void            GetCallableHandlers(CallableHandlers *ph) { if (pHandlers) ph->pArray = pHandlers; }
    

    // Register a handler to be notified when the wait is finished.
    // The wait may be called on a different thread and is 
    // usually responsible for waking the waiting thread.
    SF_EXPORT bool          AddWaitHandler(WaitHandler handler, void*  pdata);
    SF_EXPORT bool          RemoveWaitHandler(WaitHandler handler, void*  pdata);

    // Wait for this object to become signaled
    SF_EXPORT bool          Wait(unsigned delay = SF_WAIT_INFINITE);   
    // Acquires the current object based on associated acquisition interface
    SF_EXPORT bool          Acquire(unsigned delay = SF_WAIT_INFINITE);

    // Returns 1 if the object is currently signaled, i.e. the associated
    // data is available and no wait is required. Note that this state can change
    // asynchronously based on behavior of other threads.
    SF_EXPORT virtual bool  IsSignaled() const;

    // Obtain the acquisition interface
    SF_EXPORT virtual AcquireInterface* GetAcquireInterface();
};



// ***** Mutex Class

// Mutex class represents a Mutex??a synchronization object that provides access 
// serialization between different threads, allowing one thread mutually exclusive access 
// to a resource.

class MutexImpl;

class Mutex : public Waitable, public AcquireInterface
{
    friend class WaitConditionImpl;    
    friend class MutexImpl;

    // Internal mutex structures
    MutexImpl  *pImpl; 

public:
        // Constructor/destructor
    SF_EXPORT Mutex(bool recursive = 1, bool multiWait = 0);
    SF_EXPORT ~Mutex();

    // Locking functions
    SF_EXPORT void  DoLock();
    SF_EXPORT bool  TryLock();
    SF_EXPORT void  Unlock();

    // Returns 1 if the mutes is currently locked by another thread
    // Returns 0 if the mutex is not locked by another thread, and can therefore be acquired. 
    SF_EXPORT bool  IsLockedByAnotherThread();

    // **  Waitable overrides
    // A mutex is only signaled if it is not locked by ANYONE, so IsSignalled() will
    // return 0 even if it is locked by US. This means that we cannot call Wait() on
    // a mutex that is locked by us; it will never return. However, we can call Acquire() instead.
    SF_EXPORT virtual bool    IsSignaled() const;
    SF_EXPORT virtual AcquireInterface* GetAcquireInterface();

    // ** Acquire Interface implementation
    SF_EXPORT virtual bool    CanAcquire();
    SF_EXPORT virtual bool    TryAcquire();   
    SF_EXPORT virtual bool    TryAcquireCommit();
    SF_EXPORT virtual bool    TryAcquireCancel();

    // Locker class
    // Used for automatic locking of a mutex
    class Locker
    {
    public:
        Mutex *pMutex;
        Locker(Mutex *pmutex)
            { pMutex = pmutex; pMutex->DoLock(); }
        ~Locker()
            { pMutex->Unlock(); }
    };
};



// ***** WaitCondition

/*
    WaitCondition is a synchronization primitive that can be used to implement what is known as a monitor.
    Dependent threads wait on a wait condition by calling Wait(), and get woken up by other threads that
    call Notify() or NotifyAll().

    The unique feature of this class is that it provides an atomic way of first releasing a Mutex, and then 
    starting a wait on a wait condition. If both the mutex and the wait condition are associated with the same
    resource, this ensures that any condition checked for while the mutex was locked does not change before
    the wait on the condition is actually initiated.
*/

class WaitConditionImpl;

class WaitCondition
{
    friend class WaitConditionImpl;
    // Internal implementation structure
    WaitConditionImpl *pImpl;

public:
    // Constructor/destructor
    SF_EXPORT WaitCondition();
    SF_EXPORT ~WaitCondition();

    // Release mutex and wait for condition. The mutex is re-aquired after the wait.
    // Delay is specified in milliseconds (1/1000 of a second).
    SF_EXPORT bool    Wait(Mutex *pmutex, unsigned delay = SF_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    SF_EXPORT void    Notify();
    // Notify a condition, releasing all objects waiting
    SF_EXPORT void    NotifyAll();
};



// ***** Event

class Event : public Waitable, public AcquireInterface
{
    // Event state, its mutex and the wait condition
    volatile bool   State;
    volatile bool   Temporary;  
    mutable Mutex   StateMutex;
    WaitCondition   StateWaitCondition;

public:
    // Constructor/destructor
    SF_EXPORT Event(bool setInitially = 0, bool multiWait = 0);
    SF_EXPORT ~Event();

    // Wait on an event condition until it is set
    // Delay is specified in milliseconds (1/1000 of a second).
    SF_EXPORT bool  Wait(unsigned delay = SF_WAIT_INFINITE);   
    
    // Set an event, releasing objects waiting on it
    SF_EXPORT void  SetEvent();
    // Reset an event, un-signaling it
    SF_EXPORT void  ResetEvent();
    // Set and then reset an event once a waiter is released.
    // If threads are already waiting, they will be notified and released
    // If threads are not waiting, the event is set until the first thread comes in
    SF_EXPORT void  PulseEvent();
    
    // Signaled override, an event is signaled once it is set
    SF_EXPORT virtual bool  IsSignaled() const;

    SF_EXPORT virtual AcquireInterface*  GetAcquireInterface();

    // Acquire interface implementation 
    SF_EXPORT virtual bool    CanAcquire();
    SF_EXPORT virtual bool    TryAcquire();
    SF_EXPORT virtual bool    TryAcquireCommit();
    SF_EXPORT virtual bool    TryAcquireCancel();
};



// ***** Semaphore

// The Semaphore class represents a Semaphore??a synchronization object 
// that allows a limited number of threads in one or more processes to access
// a resource. A Semaphore object maintains a count of the number of 
// threads currently accessing a specified resource. The Semaphore class is 
// primarily used to serialize thread execution, similar to Mutex, 
// however, unlike a mutex, a semaphore can be accessed by more than one thread
// at a time.

class Semaphore : public Waitable, public AcquireInterface
{   
protected:

    // Event state, its mutex and the wait condition
    int             MaxValue;
    volatile int    Value;
    mutable Mutex   ValueMutex;
    WaitCondition   ValueWaitCondition;

public:
    // Constructor/destructor
    SF_EXPORT Semaphore(int maxValue = 1, bool multiWait = 0);
    SF_EXPORT ~Semaphore();

    
    // Get current value and max
    int             GetMaxValue() const;
    int             GetValue() const;
    int             GetAvailable() const;

    
    // *** Actions

    // Obtains multiple value of a semaphore.
    // Returns 0 if query failed (count > max value error or timeout)
    // Delay is specified in milliseconds (1/1000 of a second).
    SF_EXPORT bool  ObtainSemaphore(int count = 1, unsigned delay = SF_WAIT_INFINITE);
    // Release semaphore values
    // Returns success code
    SF_EXPORT bool  ReleaseSemaphore(int count = 1);


    // *** Operators

    // Postfix increment/decrement, return value before operation
    SF_EXPORT int           operator ++ (int);
    SF_EXPORT int           operator -- (int);
    // Postfix increment/decrement, return value before operation
    SF_EXPORT int           operator += (int count);
    SF_EXPORT int           operator -= (int count);


    // *** Waitable objects

    // Create a semaphore acquisition object that would increment a semaphore by a user-defined count
    // This object can be passed to AcquireMultipleObjects functions, 
    // and will acquire several values form a semaphore
    // This object must be released before the semaphore
    SF_EXPORT Waitable*     CreateWaitableIncrement(int count);

    // Acquire interface implementation
    // Default
    SF_EXPORT virtual bool              CanAcquire();
    SF_EXPORT virtual bool              TryAcquire();
    SF_EXPORT virtual bool              TryAcquireCommit();
    SF_EXPORT virtual bool              TryAcquireCancel();

    // GWaitable implementation
    SF_EXPORT virtual bool              IsSignaled() const; 
    SF_EXPORT virtual AcquireInterface* GetAcquireInterface();


    // Locker class, used for automatic acquisition of a semaphore
    class Locker
    {
    public:     
        Semaphore*  pSemaphore;
        int         Count;
        Locker(Semaphore *psemaphore, int count = 1)
            { pSemaphore = psemaphore; Count = count; 
              pSemaphore->ObtainSemaphore(count); }
        ~Locker()
            { pSemaphore->ReleaseSemaphore(Count); }
    };
};


// ***** Thread class

// ThreadId uniquely identifies a thread; returned by GetCurrentThreadId() and
// Thread::GetThreadId.
#if defined(SF_OS_PS3)
typedef sys_ppu_thread_t ThreadId;
#elif defined(SF_OS_PSVITA)
typedef SceUID ThreadId;
#elif defined(SF_OS_WINMETRO)
#if   defined(SF_USE_STD11_THREADS)
typedef size_t ThreadId;
#else
typedef const concurrency::task_group* ThreadId;
#endif
#else
typedef void* ThreadId;
#endif


// *** Thread flags

// Indicates that the thread is has been started, i.e. Start method has been called, and threads
// OnExit() method has not yet been called/returned.
#define SF_THREAD_STARTED               0x01
// This flag is set once the thread has ran, and finished.
#define SF_THREAD_FINISHED              0x02
// This flag is set temporarily if this thread was started suspended. It is used internally.
#define SF_THREAD_START_SUSPENDED       0x08
// This flag is used to ask a thread to exit. Message driven threads will usually check this flag
// and finish once it is set.
#define SF_THREAD_EXIT                  0x10


class Thread : public Waitable, public AcquireInterface 
{ // NOTE: Waitable must be the first base since it implements RefCountImpl.    

public:

    // *** Callback functions, can be used instead of overriding Run

    // Run function prototypes.    
    // Thread function and user handle passed to it, executed by the default
    // Thread::Run implementation if not null.
    typedef int (*ThreadFn)(Thread *pthread, void* h);
    
    // Thread ThreadFunction1 is executed if not 0, otherwise ThreadFunction2 is tried
    ThreadFn    ThreadFunction;    
    // User handle passes to a thread
    void*       UserHandle;

    // Thread state to start a thread with
    enum ThreadState
    {
        NotRunning  = 0,
        Running     = 1,
        Suspended   = 2
    };

    // Thread priority
    enum ThreadPriority
    {
        CriticalPriority,
        HighestPriority,
        AboveNormalPriority,
        NormalPriority,
        BelowNormalPriority,
        LowestPriority,
        IdlePriority,
    };

    // Thread constructor parameters
    struct CreateParams
    {
        CreateParams(ThreadFn func = 0, void* hand = 0, UPInt ssize = 128 * 1024, 
                     int proc = -1, ThreadState state = NotRunning, ThreadPriority prior = NormalPriority)
                     : threadFunction(func), userHandle(hand), stackSize(ssize), 
                       processor(proc), initialState(state), priority(prior) {}
        ThreadFn       threadFunction;   // Thread function
        void*          userHandle;       // User handle passes to a thread
        UPInt          stackSize;        // Thread stack size
        int            processor;        // Thread hardware processor
        ThreadState    initialState;     // 
        ThreadPriority priority;         // Thread priority
    };

    // *** Constructors

    // A default constructor always creates a thread in NotRunning state, because
    // the derived class has not yet been initialized. The derived class can call Start explicitly.
    // "processor" parameter specifies which hardware processor this thread will be run on. 
    // -1 means OS decides this. Implemented only from Win32 and XBox360
    SF_EXPORT        Thread(UPInt stackSize = 128 * 1024, int processor = -1);
    // Constructors that initialize the thread with a pointer to function.
    // An option to start a thread is available, but it should not be used if classes are derived from Thread.
    // "processor" parameter specifies which hardware processor this thread will be run on. 
    // -1 means OS decides this. Implemented only from Win32 and XBox360
    SF_EXPORT        Thread(ThreadFn threadFunction, void*  userHandle = 0, UPInt stackSize = 128 * 1024,
                             int processor = -1, ThreadState initialState = NotRunning);
    // Constructors that initialize the thread with a create parameters structure.
    explicit SF_EXPORT Thread(const CreateParams& params);

    // Destructor.
    SF_EXPORT virtual ~Thread();

    // Waits for all Threads to finish; should be called only from the root
    // application thread. Once this function returns, we know that all other
    // thread's references to Thread object have been released.
    SF_EXPORT static  void SF_CDECL FinishAllThreads();


    // *** Overridable Run function for thread processing

    // - returning from this method will end the execution of the thread
    // - return value is usually 0 for success 
    SF_EXPORT virtual int   Run();
    // Called after return/exit function
    SF_EXPORT virtual void  OnExit();


    // *** Thread management

    // Starts the thread if its not already running
    // - internally sets up the threading and calls Run()
    // - initial state can either be Running or Suspended, NotRunning will just fail and do nothing
    // - returns the exit code
    SF_EXPORT virtual bool  Start(ThreadState initialState = Running);

    // Quits with an exit code
    SF_EXPORT virtual void  Exit(int exitCode=0);

    // Suspend the thread until resumed
    // Returns 1 for success, 0 for failure.
    SF_EXPORT         bool  Suspend();
    // Resumes currently suspended thread
    // Returns 1 for success, 0 for failure.
    SF_EXPORT         bool  Resume();

    // Static function to return a pointer to the current thread
    //SF_EXPORT static Thread* GetThread();


    // *** Thread status query functions

    SF_EXPORT bool          GetExitFlag() const;
    SF_EXPORT void          SetExitFlag(bool exitFlag);

    // Determines whether the thread was running and is now finished
    SF_EXPORT bool          IsFinished() const;
    // Determines if the thread is currently suspended
    SF_EXPORT bool          IsSuspended() const;
    // Returns current thread state
    SF_EXPORT ThreadState   GetThreadState() const;

    // Returns the number of available CPUs on the system 
    SF_EXPORT static int    GetCPUCount();

    // Returns the thread exit code. Exit code is initialized to 0,
    // and set to the return value if Run function after the thread is finished.
    SF_INLINE int           GetExitCode() const { return ExitCode; }
    // Returns an OS handle 
#if (defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)) && !defined(SF_OS_WINMETRO)
    void*          GetOSHandle() const { return ThreadHandle; }
#elif defined(SF_OS_WINMETRO)
#if   defined(SF_USE_STD11_THREADS)
    std::thread*        GetOSHandle() const { return ThreadHandle; }
#else
    ThreadId	        GetOSHandle() const { return &ThreadHandle; }
#endif
#elif defined(SF_OS_WII) || defined(SF_OS_WIIU)
    OSThread *          GetOSHandle() const { return ThreadData ? const_cast<OSThread*>(&ThreadData->ThreadHandle) : 0; }
#elif defined(SF_OS_PS3)
    sys_ppu_thread_t    GetOSHandle() const { return ThreadHandle; }
#elif defined(SF_OS_PSVITA)
    SceUID              GetOSHandle() const { return ThreadHandle; }
#elif defined(SF_OS_3DS)
    nn::os::Thread*     GetOSHandle() const { return ThreadHandle; }
#else
    pthread_t           GetOSHandle() const { return ThreadHandle; }
#endif

#if (defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)) && !defined(SF_OS_WINMETRO)
    ThreadId       GetThreadId() const { return IdValue; }
#elif defined(SF_OS_WINMETRO)
#if   defined(SF_USE_STD11_THREADS)
    ThreadId       GetThreadId() const { return IdValue != std::thread::id() ? IdValue.hash() : 0; }
#else
    ThreadId       GetThreadId() const { return (ThreadId)GetOSHandle(); }
#endif
#else
    ThreadId       GetThreadId() const { return (ThreadId)GetOSHandle(); }
#endif

    static int               GetOSPriority(ThreadPriority);
    // *** Sleep

    // Sleep secs seconds
    SF_EXPORT static bool    Sleep(unsigned secs);
    // Sleep msecs milliseconds
    SF_EXPORT static bool    MSleep(unsigned msecs);


    // *** Waitable interface implementation

    // A thread is signaled once it has finished. Waitable interface
    // can be used to wait for a thread to finish.
    virtual bool   IsSignaled() const {  return (ThreadFlags & SF_THREAD_FINISHED) != 0; }

    // A thread can be acquired if it has finished.
    // However, the acquisition of a thread does not require any release logic.
    virtual AcquireInterface*  GetAcquireInterface() { return this; }

    // *** Acquire interface implementation    
    virtual bool    CanAcquire() { return IsSignaled(); }
    virtual bool    TryAcquire() { return IsSignaled(); }
    // Default implementation (that just succeeds) is fine here
    //SF_EXPORT   virtual bool    TryAcquireCommit();
    //SF_EXPORT   virtual bool    TryAcquireCancel();

    // *** Debugging functionality
#if ((defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)) && !defined(SF_OS_WINMETRO)) || defined(SF_OS_ORBIS)
    virtual void    SetThreadName( const char* name );
#else
    virtual void    SetThreadName( const char* name ) { SF_UNUSED(name); }
#endif

private:
#if (defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)) && ! defined(SF_OS_WINMETRO)
    friend unsigned WINAPI Thread_Win32StartFn(void *pthread);

#elif defined(SF_OS_WINMETRO)
#if   defined(SF_USE_STD11_THREADS)
    friend void Thread_Std11StartFunc(void *phandle);
#else
    friend void WINAPIV Thread_WinRTStartFn(void *pthread);
#endif
#elif defined(SF_OS_WII)
    friend void *Thread_PthreadStartFn(void * phandle);
    friend class ThreadList;

#elif defined(SF_OS_WIIU)
    friend int Thread_PthreadStartFn(int, void * phandle);
    friend class ThreadList;

#elif defined(SF_OS_3DS)
    friend void Thread_PthreadStartFn(void * phandle);

#elif defined(SF_OS_PS3)
    friend void Thread_PpuThreadStartFn(uint64_t phandle);

#elif defined(SF_OS_PSVITA)
    friend int Thread_Psp2ThreadStartFn(SceSize size, void* phandle);

#else
    friend void *Thread_PthreadStartFn(void * phandle);

    static int            InitAttr;
    static pthread_attr_t Attr;
#endif

protected:    
    // Thread state flags
    AtomicInt<UInt32>   ThreadFlags;
    AtomicInt<SInt32>   SuspendCount;
    UPInt               StackSize;

    // Hardware processor which this thread is running on.
    int                 Processor;
    ThreadPriority      Priority;

#if (defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)) && !defined(SF_OS_WINMETRO)
    void*               ThreadHandle;
    volatile ThreadId   IdValue;

    // System-specific cleanup function called from destructor
    void                CleanupSystemThread();

#elif defined(SF_OS_WINMETRO)
#if   defined(SF_USE_STD11_THREADS)
    std::thread*        ThreadHandle;
    std::thread::id     IdValue;
#else
    concurrency::task_group ThreadHandle;
#endif

#elif defined(SF_OS_WII) || defined(SF_OS_WIIU)
public: // for Wii compiler 4.3 145
    struct ThreadDataWii : public ListNode<ThreadDataWii>
    {
        OSThread        ThreadHandle;
        jmp_buf         ExitJmp;
        UByte           Stack[32] __attribute__((aligned(32)));
    };
protected:
    ThreadDataWii*      ThreadData;
#elif defined(SF_OS_3DS)
    jmp_buf             ExitJmp;
    nn::os::Thread*     ThreadHandle;
#elif defined(SF_OS_PS3)
    sys_ppu_thread_t    ThreadHandle;
#elif defined(SF_OS_PSVITA)
    SceUID              ThreadHandle;
#else
    pthread_t           ThreadHandle;
#endif

    // Exit code of the thread, as returned by Run.
    int                 ExitCode;

    // Internal run function.
    int                 PRun();    
    // Finishes the thread and releases internal reference to it.
    void                FinishAndRelease();

    void                Init(const CreateParams& params);

    // Protected copy constructor
    Thread(const Thread &source) : Waitable(1) { SF_UNUSED(source); }
};

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId();


#endif // SF_ENABLE_THREADS

} // Scaleform

#endif // INC_SF_Kernel_Threads_H
