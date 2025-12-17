/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_ActionTypes.h
Content     :   ActionScript implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_AS2_ACTIONTYPES_H
#define INC_SF_GFX_AS2_ACTIONTYPES_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_ASString.h"
#include "GFx/GFx_Event.h"
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_StringManager.h"
#include "GFx/GFx_StreamContext.h"
#include "GFx/GFx_Types.h"
#include "GFx/GFx_Input.h"
#include "GFx/GFx_ButtonDef.h"
#include "GFx/AS2/AS2_RefCountCollector.h"
#include "Render/Text/Text_Core.h"

#include <stdio.h>
#include <wchar.h>

namespace Scaleform { namespace GFx {

class EventId;
class ASString;
class Stream;
//class InteractiveObject;
class MovieDataDef;

namespace AS2 {

// ***** Declared Classes
struct NameFunction;
struct NameNumber;
class WithStackEntry;
class ActionBuffer;

// ***** External Classes
class Value;
class Object;
class FnCall;
class ActionBuffer;
class ObjectInterface;
class Environment;
class GlobalContext;
class ASRefCountCollector;

class ASStringContext;

class ExecutionContext;

typedef Array<Value> ValueArray;

// Floating point type used by ActionScript.
// Generally, this should be Double - a 64-bit floating point value; however,
// on systems that do not have Double (PS2) this can be another type, such as a float.
// A significant problem with float is that in AS it is used to substitute integers,
// but it does not have enough mantissa bits so do so correctly. To account for this,
// we could store both Int + float in GASFloat and do some conditional logic...

typedef Double      Number;
// This is a version of float that can be stored in a union; thus GASFloat 
// must be convertible to GASFloatValue, though perhaps with some extra calls.
typedef Number   GASNumberValue;

//#define   GFX_ASNUMBER_NAN
#define GFX_ASNUMBER_ZERO   0.0

// C-code function type used by values.
typedef void (*CFunctionPtr)(const FnCall& fn);

// Name-Function pair for statically defined operations 
// (like String::concat, etc)
struct NameFunction
{
    const char*     Name;
    CFunctionPtr Function;

    static void AddConstMembers(ObjectInterface* pobj, ASStringContext *psc,
                                const NameFunction* pfunctions,
                                UByte flags = 0);
};

// Name-Number pair for statically defined constants 
struct NameNumber
{
    const char* Name;
    int         Number;
};

#ifdef GFX_AS_ENABLE_GC
// Refcount collection version

typedef RefCountBaseGC<StatMV_ActionScript_Mem> ASRefCountBaseType;
class ASRefCountCollector : public RefCountCollector<StatMV_ActionScript_Mem>
{
    unsigned FrameCnt;
    unsigned MaxRootCount;
    unsigned PeakRootCount;
    unsigned LastRootCount;
    unsigned LastCollectedRoots;
    unsigned LastPeakRootCount;
    unsigned TotalFramesCount;
    unsigned LastCollectionFrameNum;

    unsigned PresetMaxRootCount;
    unsigned MaxFramesBetweenCollections;

    SF_INLINE void Collect(Stats* pstat = NULL)
    {
        RefCountCollector<StatMV_ActionScript_Mem>::Collect(pstat);
    }
public:
    ASRefCountCollector();

    void SetParams(unsigned frameBetweenCollections, unsigned maxRootCount);

    // This method should be called every frame (every full advance). 
    // It evaluates necessity of collection and performs it if necessary.
    void AdvanceFrame(unsigned* movieFrameCnt, unsigned* movieLastCollectFrame, AmpStats* ampStats);

    // Forces collection.
    void ForceCollect(AmpStats* ampStats);

    // Forces emergency collect. This method is called when memory heap cap is 
    // reached. It tries to free as much memory as possible.
    void ForceEmergencyCollect(AmpStats* ampStats);
};
template <class T> class ASRefCountBase : public ASRefCountBaseType 
{
public:
    ASRefCountBase(ASRefCountCollector* pcc) :
      ASRefCountBaseType(pcc) {}
private:
    ASRefCountBase(const ASRefCountBase&) : ASRefCountBaseType(0) {} // forbidden
};
#define GASWeakPtr Ptr

#else
// Regular refcounting
class ASRefCountCollector { char __dummy; };
template <class T> class ASRefCountBase : public RefCountBaseWeakSupport<T, StatMV_ActionScript_Mem> 
{
public:
    ASRefCountBase(ASRefCountCollector*) :
      RefCountBaseWeakSupport<T, StatMV_ActionScript_Mem>() {}
private:
    ASRefCountBase(const ASRefCountBase&) {} // forbidden
};
#define GASWeakPtr WeakPtr
#endif //SF_NO_GC



// ***** WithStackEntry

// The "with" stack is for Pascal-like 'with() { }' statement scoping. Passed
// as argument for ActionBuffer execution and used by variable lookup methods.

class WithStackEntry
{
protected:
    union
    {
        Object*      pObject;
        InteractiveObject* pCharacter;
    };
    enum
    {
        Mask_IsObject = 0x80000000
    };
    // combo of Mask_IsObject bit (that indicates to use pObject)
    // and the end PC of with-block.
    UInt32              BlockEndPc;

    bool                IsObject() const { return (BlockEndPc & Mask_IsObject) != 0; }
public: 
    
    WithStackEntry() : BlockEndPc(0) { pObject = NULL; }
    WithStackEntry(Object* pobj, int end);
    WithStackEntry(InteractiveObject* pcharacter, int end);
    ~WithStackEntry();

    // Conversions: non-inline because they need to know character types.
    Object*          GetObject() const;
    InteractiveObject*     GetCharacter() const;
    ObjectInterface* GetObjectInterface() const;
    int                 GetBlockEndPc() const { return (int)(BlockEndPc & (~Mask_IsObject)); }
};

typedef ArrayLH_POD<WithStackEntry, StatMV_ActionScript_Mem> WithStackArray;


// ***** ActionBuffer

// ActionScript buffer for action opcodes. The associated dictionary is stored in a
// separate ActionBuffer class, which needs to be local for GFxMovie instances.
// This class holds actions from DoActions and DoInitActions tags as well as ones 
// from "on..." event handlers. It owns the action buffer (means, action buffer is 
// allocated in Read method and freed in dtor). 
// Allocations occur from global heap, since actions might be shared between
// different SWF files and these actions should be accessible even then the owning
// SWF file is unloaded.
// ActionBufferData ref-counting must be ThreadSafe since AS is executed that AddRefs to us
// from different threads.
class ActionBufferData : public RefCountBase<ActionBufferData, StatMD_ASBinaryData_Mem>
{
protected:
    // Create using CreateNew static method
    ActionBufferData() 
        : pBuffer(0), BufferLen(0), SwdHandle(0), SWFFileOffset(0)
    {   
        //AB: technically, pdataDef should be not null always to avoid crash in
        // situation when memory heap with opcodes is freed before AS-function object
        // is freed. However, gfxexport may use this ctor with NULL for FSCommand list.
        //SF_ASSERT(pdataDef);
    }
public:
    ~ActionBufferData() { if (pBuffer) SF_FREE(pBuffer); }

    // Use this method to create an instance
    static ActionBufferData* CreateNew();

    bool                IsNull() const          { return BufferLen < 1 || pBuffer[0] == 0; }
    unsigned            GetLength() const       { return BufferLen; }
    const UByte*        GetBufferPtr() const    { return (IsNull()) ? NULL : pBuffer; }

    UInt32              GetSWFFileOffset() const            { return SWFFileOffset; }
    void                SetSWFFileOffset(UInt32 swfOffset)  { SWFFileOffset = swfOffset; }
    UInt32              GetSwdHandle() const                { return SwdHandle; }
    void                SetSwdHandle(UInt32 handle)         { SwdHandle = handle; }

    const char*         GetFileName ()      
    { 
#ifdef SF_BUILD_DEBUG
        return FileName.ToCStr();
#else
        return NULL;
#endif
    }

    // Reads action instructions from the SWF file stream (DoActions/DoInitActions tags)
    void                Read(Stream* in, unsigned actionLength);
    // Reads actions from event handlers.
    void                Read(StreamContext* psc, unsigned eventLength);
protected:
    // ActionScript byte-code data.
    UByte*              pBuffer;
    unsigned            BufferLen;

    UInt32              SwdHandle;
    UInt32              SWFFileOffset;

#ifdef SF_BUILD_DEBUG
    StringLH           FileName;
#endif
};

// ActionBuffer - movie view instance specific ActionBuffer class. This class
// caches dictionary data that needs to be local to movie view instances.

class ActionBuffer : public RefCountBaseNTS<ActionBuffer, StatMV_ActionScript_Mem>
{   
    friend class ExecutionContext;

    // Pointer to opcode byte data.
    Ptr<ActionBufferData>   pBufferData;  
    // Cached dictionary.
    ArrayCC<ASString, StatMV_ActionScript_Mem>   Dictionary;    
    int                         DeclDictProcessedAt;        

public:
    ActionBuffer(ASStringContext *psc, ActionBufferData *pbufferData);
    ~ActionBuffer();
   
    // Type call for execution; a distinction is sometimes necessary 
    // to determine how certain ops, such as 'var' declarations are handled.
    enum ExecuteType
    {
        Exec_Unknown    = 0, // just regular action buffer (frame)
        Exec_Function   = 1, // regular function
        Exec_Function2  = 2, // function2
        Exec_Event      = 3, // clip event
        Exec_SpecialEvent = 4  // special clip event, such as init, construct, load, unload.
                               // These events are not interrupted if object is being unloaded.
        // Must fit in a byte.
    };

    void    Execute(Environment* env);
    void    Execute(Environment* env,
                    int startPc, int execBytes,
                    Value* retval,
                    const WithStackArray* pinitialWithStack,
                    ExecuteType execType);

    void    ProcessDeclDict(ASStringContext *psc, unsigned StartPc, unsigned StopPc, class ActionLogger &logger);
    
    bool         IsNull() const       { return pBufferData->IsNull(); }
    unsigned     GetLength() const    { return pBufferData->GetLength(); }
    const UByte* GetBufferPtr() const { return pBufferData->GetBufferPtr(); }
    const ActionBufferData* GetBufferData() const { return pBufferData.GetPtr(); }

protected:
    // Used in GotoFrame2 (0x9F) and WaitForFrame2 (0x8D)
    bool ResolveFrameNumber (Environment* env, const Value& frameValue, InteractiveObject** pptarget, unsigned* frameNumber);
};

//
// SwfEvent
//
// For embedding event handlers in PlaceObject2Tag

class AvmSwfEvent : public NewOverrideBase<StatMD_Other_Mem>
{
public:
    // NOTE: DO NOT USE THESE AS VALUE TYPES IN AN
    // Array<>!  They cannot be moved!  The private
    // operator=(const SwfEvent&) should help guard
    // against that.

    EventId                     Event;
    Ptr<ActionBufferData>       pActionOpData;    

    AvmSwfEvent()
    { }

    void    Read(StreamContext* psc, UInt32 flags);

    void    AttachTo(InteractiveObject* ch) const;

private:
    // DON'T USE THESE
    AvmSwfEvent(const AvmSwfEvent& ) { SF_ASSERT(0); }
    void    operator=(const AvmSwfEvent& ) { SF_ASSERT(0); }
};


// ***** ASStringContext

// String lookup context is passed to functions, such as Object::GetMemberRaw to that they
// can access builtin constants, create new strings, and check for case sensitivity correctly.
// In Flash, some files are case-insensitive while others are case-sensitive. Because of that,
// SWFVersion in string context allows us to pick correct behavior.

class ASStringContext
{
public:

    ASStringContext(GlobalContext* pcontext = 0, unsigned version = 0)
    {
        pContext = pcontext;
        SWFVersion = (UByte)version;
    }

    // Provided StringManager interface    
    GlobalContext*      pContext;
    UByte               SWFVersion;

    inline void         UpdateVersion(unsigned version)  { SWFVersion = (UByte) version; }
    inline unsigned     GetVersion() const           { return (unsigned) SWFVersion; }
    inline bool         IsCaseSensitive() const      { return SWFVersion > 6; }    

    inline MemoryHeap* GetHeap() const;

    // String access and use (implemented in GASAction.h).
    const ASString&    GetBuiltin(ASBuiltinType btype) const;
    ASString    CreateConstString(const char *pstr) const;
    ASString    CreateString(const char *pstr) const;    
    ASString    CreateString(const wchar_t *pwstr) const;
    ASString    CreateString(const char *pstr, UPInt length) const;    
    ASString    CreateString(const String& str) const;

    bool         CompareConstString_CaseCheck(const ASString& pstr1, const char* pstr2);
    bool         CompareConstString_CaseInsensitive(const ASString& pstr1, const char* pstr2);

    ASRefCountCollector* GetGC();
    class MovieImpl* GetMovieImpl();

    // helper methods
    void InvalidateOptAdvanceList();

    StringManager* GetStringManager() const;
};

// Button Action for AS2
class ButtonAction : public ButtonActionBase
{
public:
    enum ConditionType
    {
        IDLE_TO_OVER_UP = 1 << 0,
        OVER_UP_TO_IDLE = 1 << 1,
        OVER_UP_TO_OVER_DOWN = 1 << 2,
        OVER_DOWN_TO_OVER_UP = 1 << 3,
        OVER_DOWN_TO_OUT_DOWN = 1 << 4,
        OUT_DOWN_TO_OVER_DOWN = 1 << 5,
        OUT_DOWN_TO_IDLE = 1 << 6,
        IDLE_TO_OVER_DOWN = 1 << 7,
        OVER_DOWN_TO_IDLE = 1 << 8,
    };
    int                                                         Conditions;
    ArrayLH<Ptr<ActionBufferData>,StatMD_CharDefs_Mem>  Actions;

    ~ButtonAction();
    virtual void    Read(Stream* in, TagType tagType, unsigned actionLength);
};

// Memory context implementation specific to AS2
class MemoryContextImpl : public MemoryContext
{
public:
    MemoryHeap*                 Heap;
    Ptr<ASStringManager>        StringMgr; // should come before ASGC for
                                           // correct deinit order
#ifdef GFX_AS_ENABLE_GC 
    Ptr<ASRefCountCollector>    ASGC;
#endif
    Ptr<Render::Text::Allocator> TextAllocator;

    struct HeapLimit : MemoryHeap::LimitHandler
    {
        enum
        {
            INITIAL_DYNAMIC_LIMIT = (128*1024)
        };

        MemoryContextImpl*  MemContext;
        UPInt               UserLevelLimit;
        UPInt               LastCollectionFootprint;
        UPInt               CurrentLimit;
        float               HeapLimitMultiplier;

        HeapLimit() :  MemContext(NULL), UserLevelLimit(0), LastCollectionFootprint(0), 
            CurrentLimit(0), HeapLimitMultiplier(0.25) {}

        void Collect(MemoryHeap* heap);
        void Reset(MemoryHeap* heap);

        virtual bool OnExceedLimit(MemoryHeap* heap, UPInt overLimit);
        virtual void OnFreeSegment(MemoryHeap* heap, UPInt freeingSize);
    } LimHandler;

    MemoryContextImpl() : Heap(NULL)
    {
        LimHandler.MemContext = this;
    }
    ~MemoryContextImpl()
    {
        Heap->SetLimitHandler(NULL);
    }
};


} // AS2

class SwfEvent : public AS2::AvmSwfEvent
{
};

}} // SF::GFx

#endif //INC_SF_GFX_ACTIONTYPES_H
