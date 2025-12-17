/**************************************************************************

Filename    :   Platform_RTCommandQueue.h
Content     :   Render Thread command queue reference header
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_RTCommandQueue_H
#define SF_RTCommandQueue_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Atomic.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform {

class RTCommandQueue;

//------------------------------------------------------------------------

enum RTCommandType 
{
    Command_None,
    Command_MFunc,    
    Command_Exit
};


// Notifier class used to notify producer threads about command
// completion and/or queue availability. Relies on proper OS waits.
class RTNotifier : public ListNode<RTNotifier>,
                   public NewOverrideBase<Stat_Default_Mem>
{
    Event Ev;
public:   
    RTNotifier() { }
    void Wait()     { Ev.Wait(); }
    void Notify()   { Ev.PulseEvent(); }
};


class RTCommand
{
public:
    enum { MaxCommandSize = 256 };

    enum WaitType {
        NoWait,
        Wait
    };


    UPInt   Size;
    UInt16  Type;
    bool    WaitFlag;
    RTNotifier* pNotifier;

    RTCommand(RTCommandType type, UPInt size, WaitType wtype = Wait)
        : Size(size), Type((UInt16)type), WaitFlag(wtype == Wait), pNotifier(0) { }
    virtual ~RTCommand() { }

    bool          NeedsWait() const { return WaitFlag; }
    UPInt         GetSize() const { return Size; }
    
    virtual void  Execute(RTCommandQueue& data) const = 0;
};


class RTCommandBuffer
{
    UPInt Size;
    union {
        UPInt Align;
        UByte Data[RTCommand::MaxCommandSize];
    };

    RTCommand* getCommand() const { return (RTCommand*)Data; }

public:
    RTCommandBuffer() : Size(0) { }
    ~RTCommandBuffer()
    {
        if (Size)
            Destruct<RTCommand>(getCommand());
    }
    
    bool    HasCommand() const { return Size != 0; }
    UPInt   GetSize() const    { return Size; }
    bool    NeedsWait() const  { return getCommand()->NeedsWait(); }
    RTNotifier* GetNotifier() { return getCommand()->pNotifier; }

    void    SetCommand(void* data)
    {
        if (Size)
            Destruct<RTCommand>((RTCommand*)Data);
        RTCommand* cmd = (RTCommand*)data;
        Size = cmd->Size;
        SF_ASSERT(Size <= RTCommand::MaxCommandSize);
        memcpy(Data, (void*)cmd, Size);
    }

    void    Execute(RTCommandQueue& data)
    {
        SF_ASSERT(HasCommand());
        getCommand()->Execute(data);
    }
};



//------------------------------------------------------------------------
// Template argument processing helpers

// RTClean strips const and & modifiers from type; for example,
// 'typename RTClean<const Class&>::Type' yields 'Class'.
template <typename T>
struct RTClean           { typedef T Type; };
template <typename T>
struct RTClean<const T>  { typedef T Type; };
template <typename T>
struct RTClean<T&>       { typedef T Type; };
template <typename T>
struct RTClean<const T&> { typedef T Type; };

// RTSelf returns the type itself, used to disambiguate argument
// passing in PushCall family of functions.
template <typename T>
struct RTSelf { typedef T Type; };



//------------------------------------------------------------------------
// RTCommand - Member Function Call Stubs

template<class C, class R = void>
class RTCommandMF0 : public RTCommand
{
    typedef R (C::*FuncPtrType)();
    C*          pClass;
    FuncPtrType pFunc;
public:
    RTCommandMF0(C* c, FuncPtrType fn, WaitType wt)
      : RTCommand(Command_MFunc, sizeof(RTCommandMF0), wt),
        pClass(c), pFunc(fn) { }

    virtual void  Execute(RTCommandQueue&) const
    { (pClass->*pFunc)(); }
};

template<class C, class A0, class R = void>
class RTCommandMF1 : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0);
    C*              pClass;
    typename RTClean<A0>::Type A0Val;
    FuncPtrType     pFunc;
public:
    RTCommandMF1(C* c, FuncPtrType fn, A0 a0, WaitType wt)
        : RTCommand(Command_MFunc, sizeof(RTCommandMF1), wt),
          pClass(c), A0Val(a0), pFunc(fn) { }

    virtual void  Execute(RTCommandQueue&) const
    { (pClass->*pFunc)(A0Val); }
};

template<class C, class A0, class A1, class R = void>
class RTCommandMF2 : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0, A1);
    C*              pClass;
    FuncPtrType     pFunc;
    typename RTClean<A0>::Type A0Val;
    typename RTClean<A1>::Type A1Val;

public:
    RTCommandMF2(C* c, FuncPtrType fn, A0 a0, A1 a1, WaitType wt)
        : RTCommand(Command_MFunc, sizeof(RTCommandMF2), wt),
          pClass(c), pFunc(fn), A0Val(a0), A1Val(a1) { }

    virtual void  Execute(RTCommandQueue&) const
    { (pClass->*pFunc)(A0Val, A1Val); }
};

template<class C, class A0, class A1, class A2, class A3, class A4, class R = void>
class RTCommandMF5 : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0, A1, A2, A3, A4);
    C*              pClass;
    FuncPtrType     pFunc;
    typename RTClean<A0>::Type A0Val;
    typename RTClean<A1>::Type A1Val;
    typename RTClean<A2>::Type A2Val;
    typename RTClean<A3>::Type A3Val;
    typename RTClean<A4>::Type A4Val;

public:
    RTCommandMF5(C* c, FuncPtrType fn, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, WaitType wt)
        : RTCommand(Command_MFunc, sizeof(RTCommandMF5), wt),
        pClass(c), pFunc(fn), A0Val(a0), A1Val(a1), A2Val(a2), A3Val(a3), A4Val(a4) { }

    virtual void  Execute(RTCommandQueue&) const
    { (pClass->*pFunc)(A0Val, A1Val, A2Val, A3Val, A4Val); }
};

//------------------------------------------------------------------------
// RTCommand - Member Function Call Stubs with Return value

template<class C, class R>
class RTCommandMF0R : public RTCommand
{
    typedef R (C::*FuncPtrType)();
    C*          pClass;
    FuncPtrType pFunc;
    R*          pRet;    
public:
    RTCommandMF0R(C* c, FuncPtrType fn, R* ret, WaitType wt)
      : RTCommand(Command_MFunc, sizeof(RTCommandMF0R), wt),
        pClass(c), pFunc(fn), pRet(ret) { }

    virtual void  Execute(RTCommandQueue&) const
    { *pRet = (pClass->*pFunc)(); }
};

template<class C, class R, class A0>
class RTCommandMF1R : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0);
    C*              pClass;
    FuncPtrType     pFunc;
    R*              pRet;
    typename RTClean<A0>::Type A0Val;
public:
    RTCommandMF1R(C* c, FuncPtrType fn, R* ret, A0 a0, WaitType wt)
      : RTCommand(Command_MFunc, sizeof(RTCommandMF1R), wt),
        pClass(c), pFunc(fn), pRet(ret), A0Val(a0) { }
  
    virtual void  Execute(RTCommandQueue&) const
    { *pRet = (pClass->*pFunc)(A0Val); }
};

template<class C, class R, class A0, class A1>
class RTCommandMF2R : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0, A1);
    C*              pClass;
    FuncPtrType     pFunc;
    R*              pRet;
    typename RTClean<A0>::Type A0Val;
    typename RTClean<A1>::Type A1Val;
public:
    RTCommandMF2R(C* c, FuncPtrType fn, R* ret, A0 a0, A1 a1, WaitType wt)
        : RTCommand(Command_MFunc, sizeof(RTCommandMF2R), wt),
        pClass(c), pFunc(fn), pRet(ret), A0Val(a0), A1Val(a1) { }

    virtual void  Execute(RTCommandQueue&) const
    { *pRet = (pClass->*pFunc)(A0Val, A1Val); }
};

template<class C, class R, class A0, class A1, class A2>
class RTCommandMF3R : public RTCommand
{
    typedef R (C::*FuncPtrType)(A0, A1, A2);
    C*              pClass;
    FuncPtrType     pFunc;
    R*              pRet;
    typename RTClean<A0>::Type A0Val;
    typename RTClean<A1>::Type A1Val;
    typename RTClean<A2>::Type A2Val;
public:
    RTCommandMF3R(C* c, FuncPtrType fn, R* ret, A0 a0, A1 a1, A2 a2, WaitType wt)
        : RTCommand(Command_MFunc, sizeof(RTCommandMF3R), wt),
        pClass(c), pFunc(fn), pRet(ret), A0Val(a0), A1Val(a1), A2Val(a2) { }

    virtual void  Execute(RTCommandQueue&) const
    { *pRet = (pClass->*pFunc)(A0Val, A1Val, A2Val); }
};



//------------------------------------------------------------------------
// ***** CircularDataQueue

class CircularDataQueue
{
public:
    enum {
        DataAlignSize = 16,
        DataAlignMask = DataAlignSize - 1
    };

    CircularDataQueue(UPInt size, MemoryHeap* heap = 0);
    ~CircularDataQueue();

    // Adds data block of size, returns pointer.
    //  - It is expected that command will be constructed there
    // Return 0 if no space available.
    UByte*   PushData(UPInt size);

    // Obtains command data
    UByte*   PopDataBegin()
    { return(Tail != Head) ? pData + Tail : 0; }
    // Finishes PopDataBegin; originally pushed size must be provided.
    void     PopDataEnd(UPInt size);

private:    
    inline UPInt roundUpSize(UPInt size)
    { return (size + DataAlignMask) & ~(UPInt)DataAlignMask; }

    UPInt   Tail;   // Pointer to next item to be popped.
    UPInt   Head;   // Pointer to where next push will take place.
    UPInt   End;    // Replaces Size when Head < Tail.
    UPInt   Size;
    UByte*  pData;
};


//------------------------------------------------------------------------
// ***** RTCommandQueue

// RTCommandQueue implements consumer serviceable queue of commands, submitted
// through PushCall / PushCallAndWait. Consumer (renderer thread) obtains commands
// through PopCommand and executes them in-order.
// The system allows multiple producer threads, but only one consumer thread.

class RTCommandQueue
{
public:

    enum ThreadingType {
        MultiThreaded,
        SingleThreaded,
        // AutoDetect means that causes constructor to use multi-threading
        // if Thread::GetCPUCount() > 1. This is resolved in constructor and
        // GetThreadingType() reported value is adjusted appropriately.
        AutoDetectThreading
    };


    RTCommandQueue(ThreadingType ttype = MultiThreaded);
    virtual ~RTCommandQueue();

    ThreadingType GetThreadingType() const { return TType; }

    void          SetRenderThreadId(ThreadId renderThreadId) { RenderThreadId = renderThreadId; }

    struct CommandConstructor
    {    
        UPInt Size;
        CommandConstructor(UPInt size) : Size(size) { }
        virtual ~CommandConstructor() {}
        virtual bool        NeedWait() const = 0;
        virtual RTCommand*  Construct(void* data) const = 0;
    };

    template<class C>
    struct CommandConstructor0 : CommandConstructor 
    {
        const C& CRef;
        CommandConstructor0(const C& c)
            : CommandConstructor(sizeof(C)), CRef(c) { }
        virtual bool       NeedWait() const { return CRef.WaitFlag; }
        virtual RTCommand* Construct(void* data) const
        { return Scaleform::Construct<C>(data, CRef); }       
        void operator = (const CommandConstructor0& ) { } // Quiet VC warning.
    };

 /*
    template<class C, class A0, class A1>
    struct CommandConstructor2 : CommandConstructor 
    {
        const A0& A0Ref;
        const A1& A1Ref;
        CommandConstructor2(const A0& a0, const A1& a1)
            : CommandConstructor(sizeof(C)), A0Ref(a0), A1Ref(a1) { }
        virtual RTCommand* Construct(void* data) const
        { return Scaleform::Construct<C>(data, A0Ref, A1Ref); }
    };
   */


    template<class C>
    bool    PushCommand(const C& c)
    {
        if (TType == MultiThreaded || Scaleform::GetCurrentThreadId() != RenderThreadId)
        {
            CommandConstructor0<C> cmd(c);
            return pushCommand(&cmd);
        }
        
        c.Execute(*this);
        return true;
    }

    bool    PopCommand(RTCommandBuffer* buffer, unsigned delay = SF_WAIT_INFINITE);


    template<class C>
    bool    PushCommandAndWait(const C& c)
    {
        if (TType == MultiThreaded || Scaleform::GetCurrentThreadId() != RenderThreadId)
        {
            RTNotifier* notifier = 0;
            CommandConstructor0<C> cmd(c);
            if (pushCommand(&cmd, &notifier))
            {
                notifier->Wait();
                freeNotifier(notifier);
                return true;
            }
            return false;
        }
        c.Execute(*this);
        return true;
    }


    // Call with no wait
    // Argument class
    template<class C>
    bool    PushCall(C* p, void (C::*func)())
    { return PushCommand(RTCommandMF0<C>(p, func, RTCommand::NoWait)); }
    template<class C, class A0>
    bool    PushCall(C* p, void (C::*func)(A0), typename RTSelf<A0>::Type a0)
    { return PushCommand(RTCommandMF1<C, A0>(p, func, a0, RTCommand::NoWait)); }
    template<class C, class A0, class A1>
    bool    PushCall(C* p, void (C::*func)(A0, A1), typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommand(RTCommandMF2<C, A0, A1>(p, func, a0, a1, RTCommand::NoWait)); }
    // This class
    template<class C, class R>
    bool    PushCall(R (C::*func)())
    { return PushCommand(RTCommandMF0<C, R>((C*)this, func, RTCommand::NoWait)); }
    template<class C, class A0, class R>
    bool    PushCall(R (C::*func)(A0), typename RTSelf<A0>::Type a0)
    { return PushCommand(RTCommandMF1<C, A0, R>((C*)this, func, a0, RTCommand::NoWait)); }
    template<class C, class A0, class A1, class R>
    bool    PushCall(R (C::*func)(A0, A1), typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommand(RTCommandMF2<C, A0, A1, R>((C*)this, func, a0, a1, RTCommand::NoWait)); }
    template<class C, class A0, class A1, class A2, class A3, class A4, class R>
    bool    PushCall(R (C::*func)(A0, A1, A2, A3, A4), typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1,
                     typename RTSelf<A2>::Type a2, typename RTSelf<A3>::Type a3, typename RTSelf<A4>::Type a4)
    { return PushCommand(RTCommandMF5<C, A0, A1, A2, A3, A4, R>((C*)this, func, a0, a1, a2, a3, a4, RTCommand::NoWait)); }

    // Calls with wait
    // Argument class
    template<class C>
    bool    PushCallAndWait(C* p, void (C::*func)())
    { return PushCommandAndWait(RTCommandMF0<C>(p, func, RTCommand::Wait)); }
    template<class C, class A0>
    bool    PushCallAndWait(C* p, void (C::*func)(A0), typename RTSelf<A0>::Type a0)
    { return PushCommandAndWait(RTCommandMF1<C, A0>(p, func, a0, RTCommand::Wait)); }
    template<class C, class A0, class A1>
    bool    PushCallAndWait(C* p, void (C::*func)(A0, A1), typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommandAndWait(RTCommandMF2<C, A0, A1>(p, func, a0, a1, RTCommand::Wait)); }
    // This class
    template<class C>
    bool    PushCallAndWait(void (C::*func)())
    { return PushCommandAndWait(RTCommandMF0<C>((C*)this, func, RTCommand::Wait)); }
    template<class C, class A0>
    bool    PushCallAndWait(void (C::*func)(A0), typename RTSelf<A0>::Type a0)
    { return PushCommandAndWait(RTCommandMF1<C, A0>((C*)this, func, a0, RTCommand::Wait)); }
    template<class C, class A0, class A1>
    bool    PushCallAndWait(void (C::*func)(A0, A1), typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommandAndWait(RTCommandMF2<C, A0, A1>((C*)this, func, a0, a1, RTCommand::Wait)); }

    // With Result
    // Argument class
    template<class C, class R>
    bool    PushCallAndWaitResult(C* p, R (C::*func)(), R* ret)
    { return PushCommandAndWait(RTCommandMF0R<C, R>(p, func, ret, RTCommand::Wait)); }
    template<class C, class R, class A0>
    bool    PushCallAndWaitResult(C* p, R (C::*func)(A0), R* ret, typename RTSelf<A0>::Type a0)
    { return PushCommandAndWait(RTCommandMF1R<C, R, A0>(p, func, ret, a0, RTCommand::Wait)); }
    template<class C, class R, class A0, class A1>
    bool    PushCallAndWaitResult(C* p, R (C::*func)(A0, A1), R* ret, typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommandAndWait(RTCommandMF2R<C, R, A0, A1>(p, func, ret, a0, a1, RTCommand::Wait)); }
    // This class
    template<class C, class R>
    bool    PushCallAndWaitResult(R (C::*func)(), R* ret)
    { return PushCommandAndWait(RTCommandMF0R<C, R>((C*)this, func, ret, RTCommand::Wait)); }    
    template<class C, class R, class A0>
    bool    PushCallAndWaitResult(R (C::*func)(A0), R* ret, typename RTSelf<A0>::Type a0)
    { return PushCommandAndWait(RTCommandMF1R<C, R, A0>((C*)this, func, ret, a0, RTCommand::Wait)); }    
    template<class C, class R, class A0, class A1>
    bool    PushCallAndWaitResult(R (C::*func)(A0, A1), R* ret, typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1)
    { return PushCommandAndWait(RTCommandMF2R<C, R, A0, A1>((C*)this, func, ret, a0, a1, RTCommand::Wait)); }
    template<class C, class R, class A0, class A1, class A2>
    bool    PushCallAndWaitResult(R (C::*func)(A0, A1, A2), R* ret,
                                  typename RTSelf<A0>::Type a0, typename RTSelf<A1>::Type a1, typename RTSelf<A2>::Type a2)
    { return PushCommandAndWait(RTCommandMF3R<C, R, A0, A1, A2>((C*)this, func, ret, a0, a1, a2, RTCommand::Wait)); }


    // StopQueueProcessing stops further PushCall requests; it can be used
    // as a thread exit flag.
    void    StopQueueProcessing() { ProcessingStopped = true; }
    bool    IsProcessingStopped() const { return ProcessingStopped; }

protected:

    // Notifier management; notifiers are allocated for each producer thread
    // and/or writable object on demand and pooled in free list for reuse.
    RTNotifier* allocNotifier_NTS();
    void        freeNotifier_NTS(RTNotifier* notifier) { FreeNotifiers.PushBack(notifier); }
    void        freeNotifier(RTNotifier* notifier)
    { Lock::Locker l(&QueueLock); freeNotifier_NTS(notifier); }    
    void        freeNotifiers();

    bool    pushCommand(CommandConstructor* cmd, RTNotifier** pnotifier = 0);

    ThreadingType     TType;
    ThreadId          RenderThreadId;
    Lock              QueueLock;
    CircularDataQueue Queue;
    // Once processing is stopped, no more push calls are allowed.
    volatile bool     ProcessingStopped;
    bool              ConsumerSleeping;
    Event             ConsumerEvent;

    List<RTNotifier>  FreeNotifiers;
    // List of producer notifiers that were blocked before submitting
    // work due to queue being full.
    List<RTNotifier>  BlockedProducers;
};


} // Scaleform

#endif // SF_RTCommandQueue_H

