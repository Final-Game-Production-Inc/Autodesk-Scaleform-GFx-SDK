/**************************************************************************

Filename    :   SF_Threads.cpp
Content     :   System-independent thread-related (safe) functionality
Created     :   May 5, 2003
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Threads.h"

#ifdef SF_ENABLE_THREADS

#define SF_THREADS_CPP

#include "SF_Memory.h"
#include "SF_Timer.h"
#include "SF_Debug.h"

namespace Scaleform {

// ***** Waitable interface implementation

Waitable::Waitable(bool enable)
 : RefCountBase<Waitable, Stat_Default_Mem>()
{
    pHandlers = enable ? new HandlerArray : 0;
//    if (pHandlers)
//        pHandlers->Handlers.SetSizePolicy(ArrayPolicy::Buffer_NoShrink); // Anachronism
}


// Override destructor to clean up handlers.
Waitable::~Waitable()
{
    if (pHandlers)
        pHandlers->Release();
}

bool    Waitable::AddWaitHandler(WaitHandler handler, void*  pdata)
{    
    // Ensure that handlers and lock are allocated.
    if (!pHandlers)
    {
        SF_DEBUG_ERROR(1, "Waitable::AddWaitHandler - wait handers not supported unless enabled");
        return 0;
    }

    // NOTE: In general, it is assumed that the thread that installs or
    // removes wait handlers owns a reference to Waitable or is at least
    // guaranteed that the later will not die. Thus, we do not need to 
    // enforce AddRef/Release semantics here; those are only 
    // relevant for CallHandlers.

    HandlerStruct   hs(handler, pdata);
    Lock::Locker    lock(&pHandlers->HandlersLock);
    pHandlers->Handlers.PushBack(hs); 
    return 1;
}

bool    Waitable::RemoveWaitHandler(WaitHandler handler, void*  pdata)
{
    if (!pHandlers)
        return 0;

    HandlerStruct   hs(handler, pdata);
    bool            result = 0;
    Lock::Locker    lock(&pHandlers->HandlersLock);   

    // Find and erase the element    
    for(unsigned i=0; i<pHandlers->Handlers.GetSize(); i++)
        if (pHandlers->Handlers[i] == hs)
        {
            pHandlers->Handlers.RemoveAt(i);
            result = 1;
            break;
        }   
    return result;
}


void    Waitable::HandlerArray::Release()
{
    if ((RefCount.ExchangeAdd_NoSync(-1) - 1) == 0)
        delete this;
}

void    Waitable::HandlerArray::CallWaitHandlers()
{
    Lock::Locker lock(&HandlersLock);
    UPInt size = Handlers.GetSize();

    // No items... nothing to call
    if (size == 0)
        return;    
    
    if (size == 1)
    {
        HandlerStruct hs(Handlers[0]);
        // Just one handler to call
        Handlers[0].Handler(Handlers[0].pUserData);
    }
    else
    {
        // Have to copy the handlers.
        HandlerArrayType handlersCopy(Handlers);
        // call each one of the handlers
        for(UPInt i=0; i<handlersCopy.GetSize(); i++)
            handlersCopy[i].Handler(handlersCopy[i].pUserData);
    }
}

void    Waitable::CallWaitHandlers()
{
    if (pHandlers)
        pHandlers->CallWaitHandlers();
}

// Acquires the current object based on associated acquisition interface
bool    Waitable::Acquire(unsigned delay)
{
    Waitable*  pw = this;
    return AcquireInterface::AcquireOneOfMultipleObjects(&pw, 1, delay) == 0;
}


// Handler used to wait for a single object to become signaled
struct Waitable_SingleWaitData
{
    Waitable* pWaitable;
    Event*    pEvent;

    Waitable_SingleWaitData(Waitable* pwaitable, Event* pevent)
        : pWaitable(pwaitable), pEvent(pevent)
    { }
};

void    Waitable_SingleWaitHandler(void* hdata)
{
    Waitable_SingleWaitData* pdata = (Waitable_SingleWaitData*)hdata;
    if (pdata->pWaitable->IsSignaled())
        pdata->pEvent->PulseEvent();
}


// Wait for the current object to be signaled
bool    Waitable::Wait(unsigned delay)
{
    if (IsSignaled())
        return 1;
    if (delay == 0)
        return 0;

    // Event that we will be waiting for
    Event                       event;
    Waitable_SingleWaitData     waitData(this, &event);
    // Install handler
    if (!AddWaitHandler(Waitable_SingleWaitHandler, &waitData))
        return 0;
    if (IsSignaled())
    {
        RemoveWaitHandler(Waitable_SingleWaitHandler, &waitData);
        return 1;
    }
    // What about atomic transition from check to wait ?? Is it necessary here?

    // Wait for the event
    // We need to time the delays in between the functions so the wait is correct
    bool        result = 0;
    UInt32      dt = 0;
    unsigned    adjustedDelay = delay;
    if (delay != SF_WAIT_INFINITE)
        dt = Timer::GetTicksMs();

    // Wait on the event
    while (event.Wait(adjustedDelay))
    {   
        // Acquire all, if possible
        if (IsSignaled())
        {
            result = 1;
            break;
        }

        // Failed to acquire; adjust the delay and keep waiting
        if (delay != SF_WAIT_INFINITE)
        {           
            unsigned delta = (unsigned)(Timer::GetTicksMs() - dt);
            // If time has passed, fail
            if (delta >= delay)
                break;
            adjustedDelay = delay - delta;
        }   
    }

    // Remove the handler
    RemoveWaitHandler(Waitable_SingleWaitHandler, &waitData);
    return result;
}

// Always signaled by default
bool    Waitable::IsSignaled() const
{
    return 1;
}

// Just return an acquire interface that is always true
AcquireInterface*  Waitable::GetAcquireInterface()
{
    return DefaultAcquireInterface::GetDefaultAcquireInterface();
}



// *** Acquire Interface implementation


DefaultAcquireInterface* DefaultAcquireInterface::GetDefaultAcquireInterface()
{
    static DefaultAcquireInterface di;
    return &di;
}


bool    AcquireInterface::CanAcquire()
{
    // By default, the resource can always be acquired
    return 1;
}
    
// Try to acquire the resource, return 1 if succeeded.
bool    AcquireInterface::TryAcquire()
{
    // Default acquire does nothing, so it always succeeds
    return 1;
}
// Complete resource acquisition. Should not fail in general.
bool    AcquireInterface::TryAcquireCommit()
{
    return 1;
}
// Cancel resource acquisition. Should not fail in general.
bool    AcquireInterface::TryAcquireCancel()
{
    return 1;
}



struct AcquireInterface_AcquireDesc
{
    // Objects waiting
    Waitable**              pWaitList;
    unsigned                WaitCount;
    // Event to signal
    Event*                  pEvent;
    // Acquire list, if any
    AcquireInterface**      pAcquireList;
    
    // Simple constructor
    AcquireInterface_AcquireDesc(Waitable** pwaitList, unsigned waitCount, Event *pevent, AcquireInterface** pacquireList = 0)
    {
        pWaitList   = pwaitList;
        WaitCount   = waitCount;
        pEvent      = pevent;       
        pAcquireList = pacquireList;
    }

    // Handler add/remove functions
    bool    AddHandlers(Waitable::WaitHandler handler);
    void    RemoveHandlers(Waitable::WaitHandler handler);
};

// Structure that incorporates a simple acquire list
struct AcquireInterface_AcquireList
{
    AcquireInterface** pList;
    AcquireInterface*  StaticList[32];

    AcquireInterface_AcquireList(Waitable** pwaitList, unsigned waitCount)
    {       
        // Allocate & fill in acquire list
        pList = (waitCount>32) ? (AcquireInterface**)
                SF_ALLOC(sizeof(AcquireInterface*) * waitCount, Stat_Default_Mem) : StaticList; 
        for(unsigned i=0; i<waitCount; i++)
            pList[i] = pwaitList[i]->GetAcquireInterface();
    }

    ~AcquireInterface_AcquireList()
    {
        if (pList != StaticList)
            SF_FREE(pList);
    }
};



bool    AcquireInterface_AcquireDesc::AddHandlers(Waitable::WaitHandler handler)
{
    // Install handlers on all waitable objects
    for(unsigned i=0; i<WaitCount; i++)
    {
        // Add a handler
        if (!pWaitList[i]->AddWaitHandler(handler, this))
        {
            while(i > 0)
                pWaitList[--i]->RemoveWaitHandler(handler, this);
            return 0;
        }
    }
    return 1;
}

void    AcquireInterface_AcquireDesc::RemoveHandlers(Waitable::WaitHandler handler)
{
    // Remove handlers from all waitable objects
    for(unsigned i=0; i<WaitCount; i++)
        pWaitList[i]->RemoveWaitHandler(handler, this);
}


void AcquireInterface_MultipleWaitHandler(void* pdata)
{    
    AcquireInterface_AcquireDesc* pdesc = (AcquireInterface_AcquireDesc*)pdata;
    
    // If at least a single object cannot be acquired, fail
    for(unsigned i=0; i<pdesc->WaitCount; i++)
        if (!pdesc->pAcquireList[i]->CanAcquire())
            return;
    // All resources can be acquired, signal the event
    pdesc->pEvent->PulseEvent();
}

bool AcquireInterface_TryAcquireAll(AcquireInterface** pacquireList, unsigned waitCount)
{
    unsigned i;
    // First try to acquire all objects, see if it succeeds
    for(i=0; i<waitCount; i++)
        if (!pacquireList[i]->TryAcquire())
            break;
    // If acquisition succeeded, commit and we are done
    if (i == waitCount)
    {
        for(i=0; i<waitCount; i++)
            pacquireList[i]->TryAcquireCommit();
        return 1;
    }
    else
    {
        // Otherwise cancel all acquisitions that worked
        while(i > 0)
            pacquireList[--i]->TryAcquireCancel();
    }
    return 0;
}

// Static function to acquire multiple objects simultaneously
bool    AcquireInterface::AcquireMultipleObjects(Waitable** pwaitList, unsigned waitCount, unsigned delay)
{   
    AcquireInterface_AcquireList   acquireList(pwaitList, waitCount);
    if (AcquireInterface_TryAcquireAll(acquireList.pList, waitCount))
        return 1;
    // If delay was 0, just fail
    if (delay == 0)
        return 0;

    // Create a event & acquire descriptor
    Event                           event;
    AcquireInterface_AcquireDesc    acquireDesc(pwaitList, waitCount, &event, acquireList.pList);     

    // Install handlers on all waitable objects
    if (!acquireDesc.AddHandlers(AcquireInterface_MultipleWaitHandler))
        return 0;

    // Must check state again, in case AddHandlers was too
    // late and state of the object already changed before its call.
    if (AcquireInterface_TryAcquireAll(acquireList.pList, waitCount))
    {
        acquireDesc.RemoveHandlers(AcquireInterface_MultipleWaitHandler);
        return 1;
    }
    
    // We need to time the delays in between the functions so the wait is correct
    bool        result = 0;
    UInt32      dt = 0;
    unsigned    adjustedDelay = delay;
    if (delay != SF_WAIT_INFINITE)
        dt = Timer::GetTicksMs();

    // Wait on the event
    while (event.Wait(adjustedDelay))
    {   
        // Acquire all, if possible
        if (AcquireInterface_TryAcquireAll(acquireList.pList, waitCount))
        {
            result = 1;
            break;
        }

        // Failed to acquire; adjust the delay and keep waiting
        if (delay != SF_WAIT_INFINITE)
        {           
            unsigned delta = (unsigned)(Timer::GetTicksMs() - dt);
            // If time has passed, fail
            if (delta >= delay)
                break;
            adjustedDelay = delay - delta;
        }   
    }

    // Remove the wait handlers, done
    acquireDesc.RemoveHandlers(AcquireInterface_MultipleWaitHandler);
    return result;
}


// Wait handler for only one of multiple objects to acquire
void AcquireInterface_OneOfMultipleWaitHandler(void* pdata)
{
    AcquireInterface_AcquireDesc* pdesc = (AcquireInterface_AcquireDesc*)pdata;

    // It would be nice to know which waitable caused this, to avoid doing a
    // loop in most cases; but that data can not be passed by caller for object
    // lifetime reasons (by the time handlers are called waitable could have
    // already been killed by another thread).

    // If at least a single object can be acquired, signal the event
    for(unsigned i=0; i<pdesc->WaitCount; i++)
        if (pdesc->pWaitList[i]->GetAcquireInterface()->CanAcquire())
        {       
            pdesc->pEvent->PulseEvent();
            break;
        }
}

// Function to try acquire one of the objects in the least
int AcquireInterface_TryAcquireOne(Waitable** pwaitList, unsigned waitCount)
{
    unsigned i;
    // First try to at least one object
    for(i=0; i<waitCount; i++)
    {
        AcquireInterface* pi = pwaitList[i]->GetAcquireInterface();
        if (pi->TryAcquire())
        {
            pi->TryAcquireCommit();
            // Return the index acquired
            return (int) i;
        }
    }   
    // Return -1 for fail
    return -1;
}


// Static function to acquire one object out of a list
int     AcquireInterface::AcquireOneOfMultipleObjects(Waitable** pwaitList, unsigned waitCount, unsigned delay)
{
    int result = AcquireInterface_TryAcquireOne(pwaitList, waitCount);
    
    // If acquired, we are done
    if (result !=-1)
        return result;
    // If delay was 0, just fail
    if (delay == 0)
        return -1;

    // Create a event & acquire descriptor
    Event                           event;
    AcquireInterface_AcquireDesc    acquireDesc(pwaitList, waitCount, &event);  
    // Install handlers on all waitable objects
    if (!acquireDesc.AddHandlers(AcquireInterface_OneOfMultipleWaitHandler))
        return 0;

    // Must check state again, in case AddHandlers was too
    // late and state of the object already changed before its call.
    result = AcquireInterface_TryAcquireOne(pwaitList, waitCount);
    if (result != -1)
    {
        acquireDesc.RemoveHandlers(AcquireInterface_OneOfMultipleWaitHandler);
        return result;
    }

    // We need to time the delays in between the functions so the wait is correct   
    UInt32      dt = 0;
    unsigned    adjustedDelay = delay;
    if (delay != SF_WAIT_INFINITE)
        dt = Timer::GetTicksMs();

    // Wait on the event
    if ((result = AcquireInterface_TryAcquireOne(pwaitList, waitCount)) == -1)
        while (event.Wait(adjustedDelay))
        {   
            // Acquire one, if possible
            if ((result = AcquireInterface_TryAcquireOne(pwaitList, waitCount)) != -1)
                break;

            // Failed to acquire; adjust the delay and keep waiting
            if (delay != SF_WAIT_INFINITE)
            {
                // Milliseconds that already passed
                unsigned delta = (unsigned)(Timer::GetTicksMs() - dt);
                // If time has passed, fail
                if (delta >= delay)
                    break;
                adjustedDelay = delay - delta;
            }   
        }

    // Remove the wait handlers
    acquireDesc.RemoveHandlers(AcquireInterface_OneOfMultipleWaitHandler);
    return result;
}




// ***** Event implementation

// Constructor/destructor
Event::Event(bool setInitially, bool multiWait)
    : Waitable(multiWait)
{
    // NOTE: RefCount mode already thread-safe for all waitables.
    State   = setInitially;
    Temporary   = 0;
}

Event::~Event()
{
}

// Wait on an event condition until it is set
bool    Event::Wait(unsigned delay)
{
    Mutex::Locker lock(&StateMutex);   

    // Do the correct amount of waiting
    switch(delay)
    {   
        case 0:
            break;

        case SF_WAIT_INFINITE:
            while(!State)
                StateWaitCondition.Wait(&StateMutex);
            break;      

        default:
            if (!State)
                StateWaitCondition.Wait(&StateMutex, delay);
            break;
    }
    
    bool state = State;
    // Take care of temporary 'pulsing' of a state
    if (Temporary)
    {
        Temporary   = 0;
        State       = 0;
    }
    return state;
}

    
// Set en event, releasing objects waiting on it
void    Event::SetEvent()
{
    StateMutex.DoLock();
    State       = 1;
    Temporary   = 0;
    StateWaitCondition.NotifyAll();

    // Must AddRef to handlers before unlock.
    CallableHandlers handlers;
    GetCallableHandlers(&handlers);
    StateMutex.Unlock();   
    handlers.CallWaitHandlers(); 
}

// Reset an event, un-signaling it
void    Event::ResetEvent()
{
    Mutex::Locker lock(&StateMutex);
    State       = 0;
    Temporary   = 0;    
}

// Set and then reset an event once a waiter is released, will release only one waiter
// If threads are already waiting, one will be notified and released
// If threads are not waiting, the event is set until the first thread comes in
void    Event::PulseEvent()
{
    StateMutex.DoLock();
    State       = 1;
    Temporary   = 1;
    StateWaitCondition.NotifyAll();

    CallableHandlers handlers;
    GetCallableHandlers(&handlers);
    StateMutex.Unlock();
    handlers.CallWaitHandlers(); 
}

// Signaled override
bool    Event::IsSignaled() const
{
    //Mutex::Locker lock(&StateMutex);
    return State;
}

AcquireInterface* Event::GetAcquireInterface()
{
    return this;
}

// Acquire interface implementation 
bool    Event::CanAcquire()
{
    return IsSignaled();
}
bool    Event::TryAcquire()
{
    return IsSignaled();
}
bool    Event::TryAcquireCommit()
{
    Mutex::Locker lock(&StateMutex);
    if (Temporary)
    {
        Temporary   = 0;
        State       = 0;
    }
    return 1;
}
bool    Event::TryAcquireCancel()
{
    return 1;
}



// ***** Semaphore implementation

Semaphore::Semaphore(int maxValue, bool multiWait)
    : Waitable(multiWait)
{
    // NOTE: RefCount mode already thread-safe for all waitables.
    MaxValue = maxValue;
    Value    = 0;
}

Semaphore::~Semaphore()
{
}
    
// Get current value and max
int         Semaphore::GetMaxValue() const
{
    return MaxValue;
}
int         Semaphore::GetValue() const
{   
    return Value;
}
int         Semaphore::GetAvailable() const
{   
    return MaxValue - Value;
}
    
// *** Actions

// Obtains multiple value of a semaphore
// Returns 0 if query failed (count > max value error or timeout)
bool        Semaphore::ObtainSemaphore(int count, unsigned delay)
{
    if (count > MaxValue)
        return 0;

    Mutex::Locker  lock(&ValueMutex);
    
    // If value in range, done
    if ((Value + count) <= MaxValue)
    {
        Value += count;
        return 1;
    }

    // If delay == 0, return immediately
    if (delay == 0)
        return 0;

    // If infinite wait, special loop
    if (delay == SF_WAIT_INFINITE)
    {
        while ((Value + count) > MaxValue)
            ValueWaitCondition.Wait(&ValueMutex);
        Value += count;
        return 1;
    }
    
    // More complicated version of wait needs to take time into account
    unsigned    adjustedDelay = delay;
    UInt32      dt = Timer::GetTicksMs();
    
    while(ValueWaitCondition.Wait(&ValueMutex, adjustedDelay))
    {
        // If value in range, done
        if ((Value + count) <= MaxValue)
        {
            Value += count;
            return 1;
        }

        unsigned delta = (unsigned)(Timer::GetTicksMs() - dt);
        // If time has passed, fail
        if (delta >= delay)
            break;
        adjustedDelay = delay - delta;      
    }
    
    // Time-out occurred
    return 0;
}


// Release semaphore values
// Returns success code
bool        Semaphore::ReleaseSemaphore(int count)
{
    if (count == 0)
        return 1;

    ValueMutex.DoLock();
    
    // Always assign final result
    // If available() is called, it does not maker whether we get the old or new
    // value, but we should never get an intermediate state (such as a negative value)
    if ((Value - count) >= 0)
        Value = Value - count;      
    else
    {
        // Value decremented too far!!
        Value = 0;
    }           

    if (count == 1)
        ValueWaitCondition.Notify();
    else
        ValueWaitCondition.NotifyAll();

    CallableHandlers handlers;
    GetCallableHandlers(&handlers);
    ValueMutex.Unlock();
    handlers.CallWaitHandlers();
    return 1;   
}


// *** Operators

// Postfix increment/decrement, return value before operation
int         Semaphore::operator ++ (int)
{
    Mutex::Locker  lock(&ValueMutex);  
    // Wait for the value available
    while (Value >= MaxValue)
        ValueWaitCondition.Wait(&ValueMutex);
    Value++;
    return Value;
}

int         Semaphore::operator -- (int)
{
    ValueMutex.DoLock();
    if (Value > 0)
        Value--;    
    ValueWaitCondition.Notify();    

    CallableHandlers handlers;
    GetCallableHandlers(&handlers);
    ValueMutex.Unlock();    
    handlers.CallWaitHandlers();
    return Value;
}

// Postfix increment/decrement, return value before operation
int         Semaphore::operator += (int count)
{
    Mutex::Locker  lock(&ValueMutex);  
    // Wait for the value available
    while ((Value + count) > MaxValue)
        ValueWaitCondition.Wait(&ValueMutex);
    Value += count;
    return Value;
}

int         Semaphore::operator -= (int count)
{
    ValueMutex.DoLock();

    if ((Value - count) >= 0)
        Value = Value - count;      
    else        
        Value = 0;
        
    ValueWaitCondition.NotifyAll();
    CallableHandlers handlers;
    GetCallableHandlers(&handlers);
    ValueMutex.Unlock();
    handlers.CallWaitHandlers();
    return Value;
}

// Acquire interface implementation
// Default
bool        Semaphore::CanAcquire()
    { return GetAvailable() > 0; }
bool        Semaphore::TryAcquire()
    { return ObtainSemaphore(1, 0); }
bool        Semaphore::TryAcquireCommit()
    { return 1; }
bool        Semaphore::TryAcquireCancel()
    { return ReleaseSemaphore(1); }


// Waitable implementation
bool            Semaphore::IsSignaled() const
    { return GetAvailable() > 0; }
AcquireInterface*  Semaphore::GetAcquireInterface()
    { return this; }



// Waitable increment semaphore acquire interface
// Can be used to acquire multiple values of a semaphore at the same time
class SemaphoreWaitableIncrement : public Waitable, public AcquireInterface
{
    Semaphore*  pSemaphore;
    int         Count;

public:
    // Constructor/Destructor
    SemaphoreWaitableIncrement() : Waitable(1)
        { pSemaphore = 0; Count = 0; }
    SemaphoreWaitableIncrement(Semaphore *psemaphore, int count);
    ~SemaphoreWaitableIncrement();


    // Acquire interface implementation
    // Default
    virtual bool                CanAcquire();
    virtual bool                TryAcquire();
    virtual bool                TryAcquireCommit();
    virtual bool                TryAcquireCancel();

    // Waitable implementation
    virtual bool                IsSignaled() const; 
    virtual AcquireInterface*   GetAcquireInterface();

    // Interface - no implementation
    virtual void        AddRef()                            { }
    virtual void        Release(unsigned flags=0)               { SF_UNUSED(flags); }    
};

// Wait handler install in the owned semaphore in order to notify us
void SemaphoreWaitableIncrement_SemaphoreWaitHandler(void* hdata)
{ 
    Waitable* pincrement = (Waitable*) hdata;
    pincrement->CallWaitHandlers();
}

SemaphoreWaitableIncrement::SemaphoreWaitableIncrement(Semaphore *psemaphore, int count)
 : Waitable(1)
{
    pSemaphore  = psemaphore;
    Count       = count;
    // Install a handler so that we get informed about semaphore state changes
    pSemaphore->AddWaitHandler(SemaphoreWaitableIncrement_SemaphoreWaitHandler, this);
}
SemaphoreWaitableIncrement::~SemaphoreWaitableIncrement()
{
    pSemaphore->RemoveWaitHandler(SemaphoreWaitableIncrement_SemaphoreWaitHandler, this);
}

// Acquisition interface
bool        SemaphoreWaitableIncrement::CanAcquire()
{
    if (!pSemaphore) return 0;
    return pSemaphore->GetAvailable() >= Count;
}
bool        SemaphoreWaitableIncrement::TryAcquire()
{
    if (!pSemaphore) return 0;
    return pSemaphore->ObtainSemaphore(Count, 0);
}
bool        SemaphoreWaitableIncrement::TryAcquireCommit()
{
    if (!pSemaphore) return 0;
    return 1;
}
bool        SemaphoreWaitableIncrement::TryAcquireCancel()
{
    if (!pSemaphore) return 0;
    return pSemaphore->ReleaseSemaphore(Count);
}

// Waitable implementation
bool            SemaphoreWaitableIncrement::IsSignaled() const
    { if (!pSemaphore) return 0;    
      return pSemaphore->GetAvailable() >= Count; }
AcquireInterface*  SemaphoreWaitableIncrement::GetAcquireInterface()
    { return this; }



// Create a semaphore acquisition object that would increment a semaphore by a user-defined count
// This object can be passed to AcquireMultipleObjects functions, and will acquire several values form a semaphore
// This object must be released before the semaphore
Waitable*      Semaphore::CreateWaitableIncrement(int count)
{
    return new SemaphoreWaitableIncrement(this, count);
}

} // Scaleform

#else

namespace Scaleform {
    ThreadId GetCurrentThreadId() { return 0; }
} // Scaleform

#endif  // SF_ENABLE_THREADS
