/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_Action.h
Content     :   ActionScript implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_ACTION_H
#define INC_SF_GFX_ACTION_H

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_ActionTypes.h"
#include "GFx/AS2/AS2_Value.h"
#include "GFx/AS2/AS2_Object.h"

#include <wchar.h>

namespace Scaleform {
namespace GFx {

// ***** External Classes
class Stream;
//class InteractiveObject;
class MovieImpl;
class DisplayObject;

namespace AS2 {

class MovieRoot;

// ***** Declared Classes
class LocalFrame;
class Environment;
class GlobalContext; 


#if defined(GFX_AS2_VERBOSE) || defined(GFX_VERBOSE_PARSE_ACTION)
// Disassemble one instruction to the log.
class Disasm
{
    Log *         pLog;
    LogMessageId  MsgId;
public:

    Disasm(Log *plog, LogMessageId msgType)
    { pLog = plog; MsgId = msgType; }

    // Formatted version used
    void    LogF(const char* pfmt, ...)
    {
        if (pLog)
        {
            va_list argList;
            va_start(argList, pfmt);
            pLog->LogMessageVarg(MsgId, pfmt, argList);
            va_end(argList);
        }
    }

    void LogDisasm(const unsigned char* InstructionData);
};
#endif

// Helper class used to log messages during action execution
class ActionLogger : public LogBase<ActionLogger>
{
    Log *       pLog;
    bool        VerboseAction;
    bool        VerboseActionErrors;
    bool        UseSuffix; //print LogSuffix after message 
    const char* LogSuffix; 
public:         
    ActionLogger(DisplayObject *ptarget, const char* suffixStr = NULL);
    // LogBase implementation
    Log*    GetLog() const          { return pLog;  }
    bool    IsVerboseAction() const { return VerboseAction; }
    bool    IsVerboseActionErrors() const   { return VerboseActionErrors; }
    void    LogScriptMessageVarg(LogMessageId messageType, const char* pfmt, va_list argList);
    void    LogScriptError(const char* pfmt,...);
    void    LogScriptWarning(const char* pfmt,...);
    void    LogScriptMessage(const char* pfmt,...);

    // Formatted version used
    SF_INLINE void    LogDisasm(const unsigned char* instructionData);
};

// ***** Environment
#ifdef SF_BUILD_DEBUG
#define GFX_DEF_STACK_PAGE_SIZE 8 // to debug page allocations
#else
#define GFX_DEF_STACK_PAGE_SIZE 32
#endif


// ***** PagedStack - Stack used for ActionScript values.

// Performance of access is incredibly important here, as most of the values 
// are passed through the stack. The class is particularly optimized to ensure
// that the following methods are inlined and as efficient as possible:
// 
//  - Top0() - accesses the element on top of stack.
//  - Top1() - accesses the element one slot below stack top.
//  - Push() - push value; parameterized to allow copy constructors.
//  - Pop1() - pop one value from stack.
//
// Additionally the stack includes the following interesting features:
//  - Address of values on stack doesn't change.
//  - A default-constructed element always lives at the bottom of the stack.
//  - Top0()/Top1() are always valid, even if nothing was pushed.
//  - Pop() is allowed even if nothing was pushed.


template <class T, int DefPageSize = GFX_DEF_STACK_PAGE_SIZE>
class PagedStack : public NewOverrideBase<StatMV_ActionScript_Mem>
{    
    // State stack value pointers for fast access.
    T*                  pCurrent;
    T*                  pPageStart;
    T*                  pPageEnd;
    T*                  pPrevPageTop;

    // Page structure. This structure is SF_ALLOC'ed and NOT new'd because 
    // we need to manage construction/cleanup of T values manually.    
    struct Page
    {        
        T       Values[DefPageSize];
        Page*   pNext;
    };

    // Page array.
    ArrayLH<Page*>         Pages;
    // Already allocated pages in a linked list.
    Page*                   pReserved;
    

    // *** Page allocator.

    Page*   NewPage()
    {   
        Page* p;
        if (pReserved) 
        {
            p = pReserved;
            pReserved = p->pNext;
        }
        else
        {
            p = (Page*) SF_HEAP_AUTO_ALLOC(this, sizeof(Page)); // Was: StatMV_ActionScript_Mem
        }
        return p;
    }
       
    void    ReleasePage(Page* ppage)
    {
        ppage->pNext = pReserved;
        pReserved = ppage;
    }

    // *** PushPage and PopPage implementations.

    // This is called after pCurrent has been incremented to make space
    // available for new item, i.e. when pCurrent reached pPageEnd;
    // Adds a new page to stack and adjusts pointer values to refer to it.
    void    PushPage()
    {
        SF_ASSERT(pCurrent == pPageEnd);

        Page* pnewPage = NewPage();
        if (pnewPage)
        {
            Pages.PushBack(pnewPage);
            // Previous page top == last page.
            pPrevPageTop= pPageEnd - 1;
            // Init new page pointers.
            pPageStart  = pnewPage->Values;
            pPageEnd    = pPageStart + DefPageSize;
            pCurrent    = pPageStart;            
        }
        else
        {
            // If for some reason allocation failed, we will overwrite last value.
            // (i.e. PushPage) will adjust pCurrent down by 1.
            pCurrent--;
        }
    }

    // Called from Pop() when we need to adjust pointer to the last page.
    // At his point, pCurrent should have already been decremented.
    void    PopPage()
    {
        SF_ASSERT(pCurrent < pPageStart);

        if (Pages.GetSize() > 1)
        {            
            ReleasePage(Pages.Back());
            Pages.PopBack();

            Page* ppage = Pages.Back();

            pPageStart  = ppage->Values;
            pPageEnd    = pPageStart + DefPageSize;
            pCurrent    = pPageEnd - 1;

            if (Pages.GetSize() > 1)
            {
                pPrevPageTop = Pages[Pages.GetSize() - 2]->Values + DefPageSize - 1;
            }
            else
            {
                // If there are no more values, point pPrevPageTop to
                // the empty value in the bottom of the stack.
                pPrevPageTop = pPageStart;
            }            
        }
        else
        {
            // Nothing to pop. Adjust pCurrent and push default
            // value back on stack.
            pCurrent++;
            Construct<T>(pCurrent);
        }
    }

public:

    PagedStack()
        : Pages(), pReserved(0)
    {
        // Always allocate one page.       
        Page *ppage = NewPage();
        SF_ASSERT(ppage);
        
        // Initialize the first element.
        // There is always an empty 'undefined' value living at the bottom of the stack.
        Pages.Reserve(15);
        Pages.PushBack(ppage);
        pPageStart  = ppage->Values;        
        pPageEnd    = pPageStart + DefPageSize;
        pCurrent    = pPageStart;
        pPrevPageTop= pCurrent;
        Construct<T>(pCurrent);
    }

    ~PagedStack()        
    {
        // Call destructor for all but the last page item.
        // (Stack will refuse to destroy last bottom item).
        Pop(TopIndex());
        // Call last item destructor and release its page.
        pCurrent->~T();
        ReleasePage(Pages.Back());

        // And release all reserved pages.        
        while(pReserved)
        {
            Page* pnext = pReserved->pNext;
            SF_FREE(pReserved);
            pReserved = pnext;
        }
    }

    void Reset()
    {
        // Call destructor for all but the last page item.
        // (Stack will refuse to destroy last bottom item).
        Pop(TopIndex());
        pCurrent->~T();
        Page *ppage = Pages.Back();

        pPageStart  = ppage->Values;        
        pPageEnd    = pPageStart + DefPageSize;
        pCurrent    = pPageStart;
        pPrevPageTop= pCurrent;
        Construct<T>(pCurrent);
    }

    // ***** High-performance Stack Access

    // Top0() and Top1() are used most commonly to access the stack.
    // Inline and optimize them as much as possible.

    // There is *always* one valid value on stack.
    // For an empty stack this value is at index 0: (undefined).
    SF_INLINE T*   Top0() const
    {        
        return pCurrent;
    }

    SF_INLINE T*   Top1() const
    {     
        return (pCurrent > pPageStart) ? (pCurrent - 1) : pPrevPageTop;     
    }

    template<class V>
    SF_INLINE void Push(const V& val)
    {
        pCurrent++;
        if (pCurrent >= pPageEnd)
        {
            // Adds a new page to stack and adjusts pointer values to refer to it.
            // If for some reason allocation failed, we will overwrite last value.
            // (i.e. PushPage) will adjust pCurrent down by 1.
            PushPage();
        }
        // Use copy constructor to initialize stack top.
        ConstructAlt<T, V>(pCurrent, val);
    }

    // Pop one element from stack. 
    SF_INLINE void Pop1()
    {
        pCurrent->~T();
        pCurrent--;
        if (pCurrent < pPageStart)
        {
            // We can always pop an element from stack. If these isn't one left after 
            // pop, PopPage will create a default undefined object and make it available.
            PopPage();
        }
    }

    SF_INLINE void Pop2()
    {
        if ((pCurrent-2) < pPageStart)
            Pop(2);
        else
        {
            pCurrent->~T();
            pCurrent--;
            pCurrent->~T();
            pCurrent--;
        }
    }

    void    Pop3()
    {
        if ((pCurrent-3) < pPageStart)
            Pop(3);
        else
        {
            pCurrent->~T();
            pCurrent--;
            pCurrent->~T();
            pCurrent--;
            pCurrent->~T();
            pCurrent--;
        }
    }

    // General Pop(number) implementation.
    void    Pop(unsigned n)
    {
        while(n > 0)
        {
            Pop1();
            n--;
        }
    }

    // Returns absolute index of the item on top (*pCurrent).
    SF_INLINE unsigned TopIndex () const
    {        
        return (((unsigned)Pages.GetSize() - 1) * DefPageSize) + (unsigned)(pCurrent - pPageStart);
    }

    
    // *** General Top(offset) / Bottom(offset) implementation.

    // These are used less frequently to can be less efficient then above.

    T*  Top(unsigned offset) const
    {
        // Compute where in stack we are...
        T*       p        = 0; 
        unsigned topIndex = TopIndex();

        if (offset <= topIndex)
        {
            const unsigned absidx  = topIndex - offset;
            const unsigned pageidx = absidx / DefPageSize;
            const unsigned idx     = absidx % DefPageSize;
            p = &Pages[pageidx]->Values[idx];
        }
        return p;
    }

    T*      Bottom(unsigned offset) const
    {
        T*   p = 0;

        if (offset <= TopIndex())
        {        
            const unsigned pageidx = offset / DefPageSize;
            const unsigned idx     = offset % DefPageSize;
            p = &Pages[pageidx]->Values[idx];
        }
        return p;
    }
};



// ActionScript "environment", essentially VM state?
class Environment : public LogBase<Environment>, public NewOverrideBase<StatMV_ActionScript_Mem>
{
public:
    enum
    {
        MAX_RECURSION_LEVEL = 255
    };
    struct TryDescr
    {
        const UByte*    pTryBlock;
        unsigned        TryBeginPC;
        unsigned        TopStackIndex;

        TryDescr() { TopStackIndex = 0; }

        unsigned GetTrySize() const     { return pTryBlock[1] | (((UInt16)pTryBlock[2]) << 8); }
        unsigned GetCatchSize() const   { return pTryBlock[3] | (((UInt16)pTryBlock[4]) << 8); }
        unsigned GetFinallySize() const { return pTryBlock[5] | (((UInt16)pTryBlock[6]) << 8); }
        bool IsCatchInRegister() const  { return (pTryBlock[0] & 0x4) != 0; }
        bool IsFinallyBlockFlag() const { return (pTryBlock[0] & 0x2) != 0; }
        bool IsCatchBlockFlag() const   { return (pTryBlock[0] & 0x1) != 0; }
        ASString GetCatchName(Environment* penv) const 
        { 
            ASString str = penv->GetBuiltin(ASBuiltin_empty_);
            if (IsCatchBlockFlag())
            {
                if (!IsCatchInRegister())
                    str = penv->CreateString((const char*) &pTryBlock[7]);
            }
            return str;
        }
        int GetCatchRegister() const 
        { 
            if (IsCatchBlockFlag())
            {
                if (IsCatchInRegister())
                    return (int)((UInt16)pTryBlock[7]);
            }
            return -1;
        }
    };

protected:
    PagedStack<Value>   Stack;
    Value               GlobalRegister[4];
    ArrayLH<Value>      LocalRegister;  // function2 uses this
    InteractiveObject*  Target;
    // Cached global contest and SWF version fast access.
    mutable ASStringContext StringContext;
    
    PagedStack<Ptr<FunctionObject> > CallStack; // stack of calls

    ActionLogger*        pASLogger; //used to print filename, when available

    // try/catch/finally/throw stuff
    ArrayLH<TryDescr>   TryBlocks;             // array of "try" blocks
    Value               ThrowingValue;         // contains thrown object/value
    int                 ExecutionNestingLevel; // used to detect unhandled exceptions
    UInt16              FuncCallNestingLevel;  // count of nested function calls; MAX_RECURSION_LEVEL are allowed.
    bool                Unrolling       :1;    // indicates that "finally" blocks are being exctd
    bool                IsInvalidTarget :1;    // set to true, if tellTarget can't find the target

public:

    // For local vars.  Use empty names to separate frames.
    ArrayLH<Ptr<LocalFrame> >  LocalFrames;
    
    //AB: do we need to store whole stack here?
    //const WithStackArray* pWithStack; 


    Environment()
        :
        Target(0),
        pASLogger(NULL),
        ThrowingValue(Value::UNSET),
        ExecutionNestingLevel(0),
        FuncCallNestingLevel(0),
        Unrolling(false),
        IsInvalidTarget(false)
    {
    }

    ~Environment() 
    { }

    void            SetTarget(InteractiveObject* ptarget);     
    void            SetInvalidTarget(InteractiveObject* ptarget);
    // Used to set target right after construction
    void            SetTargetOnConstruct(InteractiveObject* ptarget);
    SF_INLINE bool   IsTargetValid() const { return !IsInvalidTarget; }

    SF_INLINE InteractiveObject*  GetTarget() const        { SF_ASSERT(Target); return Target;  }
    AvmCharacter*   GetAvmTarget() const;
    bool            NeedTermination(ActionBuffer::ExecuteType execType) const;
    
    // LogBase implementation.
    Log*            GetLog() const;         
    bool            IsVerboseAction() const;
    bool            IsVerboseActionErrors() const;
    
    void            LogScriptError(const char* pfmt,...) const;
    void            LogScriptWarning(const char* pfmt,...) const;
    void            LogScriptMessage(const char* pfmt,...) const;
    void            SetASLogger(ActionLogger*   pasLogger) {pASLogger=pasLogger;}
    ActionLogger*   GetASLogger() {return pASLogger;}

    // Determines if gfxExtensions are enabled; returns 1 if enabled, 0 otherwise.
    bool            CheckExtensions() const;

    // Helper similar to DisplayObject.  
    //MovieImpl*   GetMovieImpl() const; //@TEMP!
    MovieImpl*      GetMovieImpl() const;
    MovieRoot*      GetAS2Root() const;
    SF_INLINE unsigned GetVersion() const                  { return StringContext.GetVersion(); }
    SF_INLINE bool     IsCaseSensitive() const      { return StringContext.IsCaseSensitive(); }    
    
    // These are used *A LOT* - short names for convenience.
    // GetGC: Always returns a global object, never null.
    SF_INLINE GlobalContext*   GetGC() const            { return StringContext.pContext; }
    // GetSC: Always returns string context, never null.
    SF_INLINE ASStringContext* GetSC() const            { return &StringContext; }
    // GetHeap: Always returns the current movie view heap.
    SF_INLINE MemoryHeap*      GetHeap() const;
    // Convenient prototype access (delegates to GC).
    SF_INLINE Object*          GetPrototype(ASBuiltinType type) const;
    SF_INLINE StringManager*   GetStringManager() const { return GetSC()->GetStringManager(); }
    
    // Inline to built-in access through the context
    SF_INLINE const ASString&  GetBuiltin(ASBuiltinType btype) const;
    SF_INLINE ASString  CreateConstString(const char *pstr) const;
    SF_INLINE ASString  CreateString(const char *pstr) const;
    SF_INLINE ASString  CreateString(const wchar_t *pwstr) const;
    SF_INLINE ASString  CreateString(const char *pstr, UPInt length) const;
    SF_INLINE ASString  CreateString(const String& str) const;

    // Stack access/manipulation
    // @@ TODO do more checking on these
    template<class V>
    SF_INLINE void      Push(const V& val)          { Stack.Push(val); }
    SF_INLINE void      Drop1()                     { Stack.Pop1(); }
    SF_INLINE void      Drop2()                     { Stack.Pop2(); }
    SF_INLINE void      Drop3()                     { Stack.Pop3(); }
    SF_INLINE void      Drop(unsigned count)        { Stack.Pop(count); }
    SF_INLINE Value&    Top()                       { return *Stack.Top0(); }   
    SF_INLINE Value&    Top1()                      { return *Stack.Top1(); }
    SF_INLINE Value&    Top(unsigned dist)          { return *Stack.Top(dist); }
    SF_INLINE Value&    Bottom(unsigned index)      { return *Stack.Bottom (index); }
    SF_INLINE int       GetTopIndex() const         { return int(Stack.TopIndex()); }

    enum ExcludeFlags 
    {
        IgnoreLocals        = 0x1, // ignore local vars (incl this)
        IgnoreContainers    = 0x2, // ignore _root,_levelN,_global,
        NoLogOutput         = 0x4  // do not output error msgs into the log
    };

    // Invokes GetMember for the "pthisObj" and resolves properties (invokes getters)
    // or invokes __resolve handler.
    bool    GetMember(ObjectInterface* pthisObj, 
        const ASString& memberName, 
        Value* pdestVal);

    // Resolves properties (invokes getter methods) and invokes __resolve handlers.
    bool    GetVariable(const ASString& varname, Value* presult, const WithStackArray* pwithStack = NULL, 
                        InteractiveObject **ppnewTarget = 0, Value* powner = 0, unsigned excludeFlags = 0);
    // Similar to GetVariable, determines if a var is available based on varName path.
    bool    IsAvailable(const ASString& varname, const WithStackArray* pwithStack = NULL) const;

    struct GetVarParams
    {
        const ASString&                        VarName;
        Value*                               pResult;
        const WithStackArray*                pWithStack;
        InteractiveObject **                    ppNewTarget;
        Value*                               pOwner;
        unsigned                                ExcludeFlags;

        GetVarParams(const ASString& varname, Value* presult) : 
            VarName(varname), pResult(presult), pWithStack(NULL), ppNewTarget(NULL),
            pOwner(NULL), ExcludeFlags(0) {}
        GetVarParams(const ASString& varname, Value* presult, const WithStackArray* pwithStack, 
                     InteractiveObject **ppnewTarget = 0, Value* powner = 0, unsigned excludeFlags = 0) : 
            VarName(varname), pResult(presult), pWithStack(pwithStack), ppNewTarget(ppnewTarget),
            pOwner(powner), ExcludeFlags(excludeFlags) {}
        GetVarParams& operator=(const GetVarParams&) { return *this; }
    };
    // parses path; no property or __resolve invokes
    bool    FindAndGetVariableRaw(const GetVarParams& params) const;
    // no path stuff, no property or __resolve invokes
    bool    GetVariableRaw(const GetVarParams& params) const;
    bool    GetVariableRaw(const ASString& varname,
                           Value* presult,
                           const WithStackArray* pwithStack,
                           Value* powner,
                           unsigned excludeFlags) const
    {
        return GetVariableRaw(GetVarParams(varname, presult, pwithStack, NULL, powner, excludeFlags));
    }
    Bool3W  CheckGlobalAndLevels(const Environment::GetVarParams& params) const;
    // find owner of varname. Return UNDEFINED for "this","_root","_levelN","_global".
    bool    FindOwnerOfMember (const ASString& varname, Value* presult,
                               const WithStackArray* pwithStack = NULL) const;

    // find variable by path. if onlyTarget == true then only characters can appear in the path
    bool    FindVariable(const GetVarParams& params, bool onlyTargets = false, ASString* varName = 0) const;

    bool    SetVariable(const ASString& path, const Value& val,
                        const WithStackArray* pwithStack = NULL, bool doDisplayErrors = true);
    // no path stuff:
    void    SetVariableRaw(const ASString& path, const Value& val,
                           const WithStackArray* pwithStack = NULL);

    void    SetLocal(const ASString& varname, const Value& val);
    void    AddLocal(const ASString& varname, const Value& val);    // when you know it doesn't exist.
    void    DeclareLocal(const ASString& varname); // Declare varname; undefined unless it already exists.

    // Parameter/local stack frame management.
    int     GetLocalFrameTop() const { return (unsigned)LocalFrames.GetSize(); }
    void    SetLocalFrameTop(unsigned t) { SF_ASSERT(t <= LocalFrames.GetSize()); LocalFrames.Resize(t); }
    void    AddFrameBarrier() { LocalFrames.PushBack(0); }
    
    LocalFrame* GetTopLocalFrame (int off = 0) const;
    LocalFrame* CreateNewLocalFrame ();

    // Local registers.
    void    AddLocalRegisters(unsigned RegisterCount)
    {
        LocalRegister.Resize(LocalRegister.GetSize() + RegisterCount);
    }
    void    DropLocalRegisters(unsigned RegisterCount)
    {
        LocalRegister.Resize(LocalRegister.GetSize() - RegisterCount);
    }
    Value*   LocalRegisterPtr(unsigned reg);

    // Internal.
    bool            FindLocal(const ASString& varname, Value* pvalue) const;
    Value*       FindLocal(const ASString& varname);
    const Value* FindLocal(const ASString& varname) const;  
    // excludeFlags - 0 or NoLogOutput
    InteractiveObject* FindTarget(const ASString& path, unsigned excludeFlags = 0) const;
    InteractiveObject* FindTargetByValue(const Value& val);

    Object*      OperatorNew(const FunctionRef& constructor, int nargs = 0, int argsTopOff = -1);
    Object*      OperatorNew(Object* ppackageObj, const ASString &className, int nargs = 0, int argsTopOff = -1);
    Object*      OperatorNew(const ASString &className, int nargs = 0, int argsTopOff = -1);

    FunctionRef  GetConstructor(ASBuiltinType className);

    static bool     ParsePath(ASStringContext* psc, const ASString& varPath, ASString* path, ASString* var);
    static bool     IsPath(const ASString& varPath);

    Value           PrimitiveToTempObject(int index);
    Value           PrimitiveToTempObject(const Value& v);

    SF_INLINE void     CallPush(const FunctionObject* pfuncObj)
    {
        CallStack.Push(Ptr<FunctionObject>(const_cast<FunctionObject*>(pfuncObj)));
    }
    SF_INLINE void     CallPop()
    {
        SF_ASSERT((int)CallStack.TopIndex() >= 0);
        CallStack.Pop1();
    }
    SF_INLINE Value CallTop(int off) const
    {
        if (CallStack.TopIndex() < (unsigned)off)
            return Value(Value::NULLTYPE);
        else
            return Value(CallStack.Top(off)->GetPtr());
    }
    SF_INLINE unsigned GetCallStackSize() const { return (unsigned)CallStack.TopIndex(); }

    SF_INLINE Value& GetGlobalRegister(int r)
    {
        SF_ASSERT(r >= 0 && unsigned(r) < sizeof(GlobalRegister)/sizeof(GlobalRegister[0]));
        return GlobalRegister[r];
    }
    SF_INLINE const Value& GetGlobalRegister(int r) const
    {
        SF_ASSERT(r >= 0 && unsigned(r) < sizeof(GlobalRegister)/sizeof(GlobalRegister[0]));
        return GlobalRegister[r];
    }

    // recursion guard for toString, valueOf, etc methods.
    // Returns true, if max recursion number is not reached.
    SF_INLINE bool RecursionGuardStart() 
    { 
        return (FuncCallNestingLevel++ < MAX_RECURSION_LEVEL);
    }
    SF_INLINE void RecursionGuardEnd()
    {
        SF_ASSERT(FuncCallNestingLevel > 0);
        --FuncCallNestingLevel;
    }

    void Reset();

    // try/catch/finally/throw support
    int CheckExceptions(ActionBuffer* pactBuf, 
        int nextPc, 
        int* plocalTryBlockCount, 
        Value* retval,
        const WithStackArray* pwithStack,
        ActionBuffer::ExecuteType execType);

    void      CheckTryBlocks(int pc, int* plocalTryBlockCount);
    void      PushTryBlock(const TryDescr&);
    TryDescr  PopTryBlock();
    TryDescr& PeekTryBlock();
    void      Throw(const Value& val) { ThrowingValue = val; }
    void      ClearThrowing()            { ThrowingValue.SetUnset(); Unrolling = false; }
    bool      IsThrowing() const         { return ThrowingValue.IsSet(); }
    void      SetUnrolling()             { Unrolling = true; }
    void      ClearUnrolling()           { Unrolling = false; }
    bool      IsUnrolling() const        { return Unrolling; }
    const Value& GetThrowingValue() const { return ThrowingValue; }
    bool      IsInsideTryBlock(int pc);
    bool      IsInsideCatchBlock(int pc);
    bool      IsInsideFinallyBlock(int pc);

    void      EnteringExecution() { ++ExecutionNestingLevel; }
    void      LeavingExecution()  { --ExecutionNestingLevel; SF_ASSERT(ExecutionNestingLevel >= 0); }
    bool      IsExecutionNestingLevelZero() const { return ExecutionNestingLevel == 0; }

    void      InvalidateOptAdvanceList() const;

    // Returns garbage collector, if turned on. Returns NULL otherwise.
    ASRefCountCollector* GetCollector(); 
};

//
// Some handy helpers
//

// Global Context, created once for a root movie
class GlobalContext : public RefCountBaseNTS<GlobalContext, StatMV_ActionScript_Mem>
{
    friend class Environment;
public:
    typedef FunctionRef (*RegisterBuiltinClassFunc)(GlobalContext*);
    struct ClassRegEntry
    {
        RegisterBuiltinClassFunc    RegistrarFunc;
        Ptr<FunctionObject>     ResolvedFunc;

        ClassRegEntry(RegisterBuiltinClassFunc f):RegistrarFunc(f) {}

        bool IsResolved() const { return ResolvedFunc.GetPtr() != NULL; }
        FunctionObject* GetResolvedFunc() const { return ResolvedFunc; }
    };
private:
    HashUncachedLH<ASBuiltinType, Ptr<Object>, FixedSizeHash<ASBuiltinType> >   Prototypes;
    ASStringHash<FunctionRef>                           RegisteredClasses;  
    ASStringHash<ClassRegEntry>                         BuiltinClassesRegistry;
    MovieImpl*                                          pMovieRoot;
    MemoryHeap*                                         pHeap;
public:
    Object* FlashGeomPackage; // obj for "flash.geom" package. Is holding by pGlobal
    Object* SystemPackage; // obj for "system" package. Is holding by pGlobal
    Object* FlashExternalPackage; // flash.external package
    Object* FlashDisplayPackage;  // flash.display package
    Object* FlashFiltersPackage; // flash.filters package
public:
    
    // Constructor, initializes the context variables
    GlobalContext(MovieImpl*);
    //GlobalContext(MemoryHeap*);
    ~GlobalContext();

    // Publicly accessible global members
    // ActionScript global object, corresponding to '_global'
    Ptr<Object>             pGlobal;

    Bool3W                  GFxExtensions;
 
    // A map of standard member names for efficient access. This hash
    // table is used by GASCharacter implementation. The values in it
    // are of InteractiveObject::StandardMember type.
    ASStringHash<SByte>     StandardMemberMap;

    // this method returns built-in prototype (and register a class if necc).
    Object* GetPrototype(ASBuiltinType type);
    void SetPrototype(ASBuiltinType type, Ptr<Object> objproto)
    {
        Prototypes.Add(type, objproto);
    }
    // the method gets currently set prototype retrieving it from the corresponding
    // constructor function. 
    Object* GetActualPrototype(Environment* penv, ASBuiltinType type);

    // registerClass related methods
    bool RegisterClass(ASStringContext* psc, const ASString& className, const FunctionRef& ctorFunction);
    bool UnregisterClass(ASStringContext* psc, const ASString& className);
    bool FindRegisteredClass(ASStringContext* psc, const ASString& className, FunctionRef* pctorFunction);
    void UnregisterAllClasses();

    static Object* AddPackage(ASStringContext *psc, Object* pparent, Object* objProto, const char* const packageName);
    
    ASString FindClassName(Environment *penv, ObjectInterface* iobj);

    SF_INLINE bool CheckExtensions() const { return GFxExtensions.IsTrue(); }

    // Returns the registration function. Note, it returns the function only once
    // (per class) to avoid multiple registrations.
    ClassRegEntry* GetBuiltinClassRegistrar(ASString className);

    template <ASBuiltinType type, class CtorFunc>
    void AddBuiltinClassRegistry(ASStringContext& sc, Object* pdest)
    {
        if (BuiltinClassesRegistry.Get(GetBuiltin(type)) == NULL)
        {
            BuiltinClassesRegistry.Add(GetBuiltin(type), CtorFunc::Register);
            pdest->SetMemberRaw(&sc, GetBuiltin(type), Value(Value::FUNCTIONNAME, GetBuiltin(type)));
        }
    }
    FunctionObject* ResolveFunctionName(const ASString& functionName);

    MemoryHeap*         GetHeap() const         { return pHeap; }

    MovieImpl*          GetMovieImpl()          { return pMovieRoot; }
    const MovieImpl*    GetMovieImpl() const    { return pMovieRoot; }
    MovieRoot*          GetAS2Root();
    const MovieRoot*    GetAS2Root() const;
    void                DetachMovieRoot()       { pMovieRoot = NULL; }

    ASRefCountCollector* GetGC();
    AS2::StringManager* GetStringManager() const;

    void    PreClean(bool preserveBuiltinProps = false);
    void    Init(MovieImpl* proot);

    // convenience method for access to string manager
    SF_INLINE const ASString&    GetBuiltin(ASBuiltinType btype) const
    {
        return GetStringManager()->GetBuiltin(btype);
    }
    SF_INLINE ASString    CreateConstString(const char *pstr) const
    {
        return GetStringManager()->CreateConstString(pstr);
    }
    SF_INLINE ASString    CreateString(const char *pstr) const
    {
        return GetStringManager()->CreateString(pstr);
    }
    SF_INLINE ASString    CreateString(const wchar_t *pwstr) const
    {
        return GetStringManager()->CreateString(pwstr);
    }
    SF_INLINE ASString    CreateString(const char *pstr, UPInt length) const
    {
        return GetStringManager()->CreateString(pstr, length);
    }
    SF_INLINE ASString    CreateString(const String& str) const
    {
        return GetStringManager()->CreateString(str);
    }
//     bool         CompareConstString_CaseCheck(const GASString& pstr1, const char* pstr2)
//     {
//         return GetStringManager()->CompareConstString_CaseCheck(pstr1, pstr2);
//     }
//     bool         CompareConstString_CaseInsensitive(const GASString& pstr1, const char* pstr2)
//     {
//         return GetStringManager()->CompareConstString_CaseInsensitive(pstr1, pstr2);
//     }

protected:   
    void    InitStandardMembers();
};

// Inline in environment for built-in access.
SF_INLINE const ASString&     Environment::GetBuiltin(ASBuiltinType btype) const
{    
    return (const ASString&)GetGC()->GetStringManager()->GetBuiltinNodeHolder(btype);
}
SF_INLINE ASString  Environment::CreateString(const char *pstr) const
{
    return GetGC()->GetStringManager()->CreateString(pstr);
}
SF_INLINE ASString  Environment::CreateString(const wchar_t *pwstr) const
{
    return GetGC()->GetStringManager()->CreateString(pwstr);
}
SF_INLINE ASString  Environment::CreateConstString(const char *pstr) const
{
    return GetGC()->GetStringManager()->CreateConstString(pstr);
}
SF_INLINE ASString  Environment::CreateString(const char *pstr, UPInt length) const
{
    return GetGC()->GetStringManager()->CreateString(pstr, length);
}
SF_INLINE ASString  Environment::CreateString(const String& str) const
{
    return GetGC()->GetStringManager()->CreateString(str);
}
SF_INLINE Object* Environment::GetPrototype(ASBuiltinType type) const
{
    return GetGC()->GetPrototype(type);
}
SF_INLINE bool Environment::CheckExtensions() const
{
    return GetGC()->CheckExtensions();
}

SF_INLINE Object* Environment::OperatorNew(const ASString &className, int nargs, int argsTopOff)
{
    return OperatorNew(GetGC()->pGlobal, className, nargs, argsTopOff);
}

SF_INLINE Value* Environment::FindLocal(const ASString& varname)
{
    return const_cast<Value*>(const_cast<const Environment*>(this)->FindLocal(varname));
}

SF_INLINE bool Environment::FindLocal(const ASString& varname, Value* pvalue) const
{
    const Value* pval = FindLocal(varname);
    if (pval && pvalue)
    {
        *pvalue = *pval;
        return true;
    }
    return (pval != NULL);
}
SF_INLINE MemoryHeap* Environment::GetHeap() const
{
    return GetGC()->GetHeap();
}


// Context-dependent inlines in ASStringContext.
SF_INLINE MemoryHeap* ASStringContext::GetHeap() const
{
    return pContext->GetHeap();
}
SF_INLINE const ASString& ASStringContext::GetBuiltin(ASBuiltinType btype) const
{
    return (const ASString&)pContext->GetStringManager()->GetBuiltinNodeHolder(btype);
}
SF_INLINE ASString    ASStringContext::CreateConstString(const char *pstr) const
{
    return pContext->GetStringManager()->CreateConstString(pstr);
}
SF_INLINE ASString    ASStringContext::CreateString(const char *pstr) const
{
    return pContext->GetStringManager()->CreateString(pstr);
}
SF_INLINE ASString    ASStringContext::CreateString(const wchar_t *pwstr) const
{
    return pContext->GetStringManager()->CreateString(pwstr);
}
SF_INLINE ASString    ASStringContext::CreateString(const char *pstr, UPInt length) const
{
    return pContext->GetStringManager()->CreateString(pstr, length);
}
SF_INLINE ASString    ASStringContext::CreateString(const String& str) const
{
    return pContext->GetStringManager()->CreateString(str);
}
SF_INLINE bool         ASStringContext::CompareConstString_CaseCheck(const ASString& pstr1, const char* pstr2)
{
    return CreateConstString(pstr2).Compare_CaseCheck(pstr1, IsCaseSensitive());
}
SF_INLINE bool         ASStringContext::CompareConstString_CaseInsensitive(const ASString& pstr1, const char* pstr2)
{
    return CreateConstString(pstr2).Compare_CaseCheck(pstr1, false);
}

SF_INLINE ASRefCountCollector* ASStringContext::GetGC() 
{ 
    return (pContext) ? pContext->GetGC() : NULL; 
}

SF_INLINE class MovieImpl* ASStringContext::GetMovieImpl() 
{ 
    return (pContext) ? pContext->GetMovieImpl() : NULL; 
}

SF_INLINE Value& LocalFrame::Arg(int n) const
{
    SF_ASSERT(n < NArgs);
    return Env->Bottom(FirstArgBottomIndex - n);
}

// Dispatching methods from C++.
bool        GAS_InvokeParsed(
                const char* pmethodName,
                Value* presult,
                ObjectInterface* pthisPtr,
                Environment* penv,
                const char* pmethodArgFmt, 
                va_list args);

bool        GAS_InvokeParsed(const Value& method,
                             Value* presult,
                             ObjectInterface* pthisPtr,
                             Environment* penv,
                             const char* pmethodArgFmt, 
                             va_list args,
                             const char* pmethodName);

bool        GAS_Invoke(
                const Value& method,
                Value* presult,
                ObjectInterface* pthisPtr,
                Environment* penv, 
                int nargs, int firstArgBottomIndex,
                const char* pmethodName);

bool        GAS_Invoke(
                const Value& method,
                Value* presult,
                const Value& thisPtr,
                Environment* penv, 
                int nargs, int firstArgBottomIndex,
                const char* pmethodName);

bool        GAS_Invoke(
                const char* pmethodName,
                Value* presult,
                ObjectInterface* pthis,
                Environment* penv,
                int numArgs, int firstArgBottomIndex);

SF_INLINE bool GAS_Invoke0(const Value& method,
                        Value* presult,
                        ObjectInterface* pthis,
                        Environment* penv)
{
    return GAS_Invoke(method, presult, pthis, penv, 0, penv->GetTopIndex() + 1, NULL);
}

#ifdef SF_OS_WIN32
#define snprintf _snprintf
#endif // SF_OS_WIN32

#ifdef SF_OS_SYMBIAN
#include <stdarg.h>

static SF_INLINE int snprintf(char *out, int n, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = G_vsprintf(out,fmt,args);
    va_end(args);
    return ret;
}
#endif

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_ACTION_H
