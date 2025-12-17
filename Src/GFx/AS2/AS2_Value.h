/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_Value.h
Content     :   ActionScript Value implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFx_AS2_Value_H
#define INC_SF_GFx_AS2_Value_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "GFx/AS2/AS2_ActionTypes.h"
#include "GFx/AS2/AS2_FunctionRef.h"
#include "GFx/GFx_ASUtils.h"
#ifdef GFX_AS_ENABLE_GC
#include "AS2_RefCountCollector.h"
#endif // SF_NO_GC

namespace Scaleform {

namespace GFx {
class CharacterHandle;
class InteractiveObject;

namespace AS2 {

// ***** Declared Classes
class Value;
class FnCall;
// ***** External Classes
//class InteractiveObject;
class Environment;

class AvmCharacter;

struct ValueProperty : public ASRefCountBase<ValueProperty>
{
    FunctionRef              GetterMethod;
    FunctionRef              SetterMethod;

    ValueProperty(ASRefCountCollector* pCC,
                     const FunctionRef& getterMethod, 
                     const FunctionRef& setterMethod);

#ifdef GFX_AS_ENABLE_GC
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor>
    void ForEachChild_GC(Collector* prcc) const
    {
        GetterMethod.template ForEachChild_GC<Functor>(prcc);
        SetterMethod.template ForEachChild_GC<Functor>(prcc);
    }
    virtual void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
    {
        ASRefCountBaseType::CallForEachChild<ValueProperty>(prcc, operation);
    }
    void Finalize_GC() 
    {
        GetterMethod.Finalize_GC();
        SetterMethod.Finalize_GC();
    }
#endif //SF_NO_GC
};

// ***** Value


#if defined(SF_CC_MSVC) && defined(SF_CPU_X86)
// On Win32 ActionScript performs better with smaller structures,
// even if those cause GASNumberValues to be *misalligned*.
#pragma pack(push, 4)
#endif

// This is the main ActionScript value type, used for execution stack
// entry and all computations. It combines numeric, string, object
// and other necessary types.
class Value
{
public:

    // Override 'operator new' to use our allocator.
    // We avoid using GNewOverrideBase because in some compilers
    // it causes overhead and small size is critical for Value.
    #if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
    # undef new
    #endif
        SF_MEMORY_REDEFINE_NEW(Value, StatMV_ActionScript_Mem)
    #if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
    # define new SF_DEFINE_NEW
    #endif


    enum type
    {
        UNDEFINED,
        NULLTYPE,
        BOOLEAN,        
        NUMBER,

        INTEGER,    // for internal use only
        // These objects require DropRefs:
        STRING,
        OBJECT,
        CHARACTER,  // 'movieclip' - like type
        FUNCTION,   // C or AS function
        PROPERTY,   // getter/setter property

        UNSET,      // special value indicating value is not set
        FUNCTIONNAME,// function name that should be resolved to get object or function
        RESOLVEHANDLER // a function, __resolve handler
    };


    // *** Value Packing/Alignment Notes
    //
    // The arrangement of structures below is necessary to ensure that two
    // alignment conditions ensured in a portable manner:
    //  1) GASNumberValue gets correctly aligned.
    //  2) Value does not get padded to 24 bytes in 32-bit machines,
    //     when it can be 16.    
    // The condition (2) means that we can not put NumberValue into
    // the same same union as other values, as that would force such a
    // union to become 16 bytes; forcing Value to be 12.


    // Type and property flags of value.
    struct TypeDesc
    {        
        UByte               Type;        
        // Property flags. In general, not initialized or copied; these 
        // are only accessible through GetPropFlags() and SetPropFlags().
        // Property flags are managed explicitly in GASMember.
        UByte               PropFlags;
    };

    struct ValueType : public TypeDesc
    {
        union
        {
            ASStringNode*          pStringNode;
            Object*              pObjectValue;
            CharacterHandle*        pCharHandle;
            ValueProperty*       pProperty;
            FunctionRefBase      FunctionValue;
            bool                    BooleanValue;
        };
    };

    struct NumericType : public TypeDesc
    {
#if defined(SF_CC_MSVC) && defined(SF_CPU_X86)
        // Avoid packing warning on Win32.
//        UInt32          Dummy;
#endif        
        union
        {
            SInt32  Int32Value;
            UInt32  UInt32Value;
            double  NumberValue;
        };
    };
    
    union
    {
        TypeDesc        T;
        ValueType       V;
        NumericType     NV;
    };



    // *** Constructors.

    Value()        
    {
        T.Type = UNDEFINED;
    }    
    Value(type typeVal)
    {
        T.Type = (UByte)typeVal;
    }
    Value(type typeVal, const ASString& str)
    {
        T.Type = (UByte)typeVal;
        V.pStringNode = str.GetNode();
        V.pStringNode->AddRef();
    }
    Value(const ASString& str)        
    {
        T.Type = STRING;
        V.pStringNode = str.GetNode();
        V.pStringNode->AddRef();
    }
    Value(bool val)
    { 
        T.Type = BOOLEAN;
        V.BooleanValue = val;
    }
    Value(int val)
    {
        T.Type = INTEGER;
        NV.Int32Value = SInt32(val);
        //@DBG
        //T.Type = NUMBER;
        //NV.NumberValue = val;
    }
    Value(long val)
    {
        T.Type = INTEGER;
        NV.Int32Value = SInt32(val); //!AB: should it be Int64? 
        //@DBG
        //T.Type = NUMBER;
        //NV.NumberValue = val;
    }   
    // Floating - point constructor.
    Value(Double val)
    {
        T.Type = NUMBER;
        NV.NumberValue = val;
    }
    Value(float val)
    {
        T.Type = NUMBER;
        NV.NumberValue = double(val);
    }
    
    Value(Object* obj);
    Value(InteractiveObject* pcharacter);
    Value(ASStringContext* psc, CFunctionPtr func);
    Value(const FunctionRef& func);
    Value(const FunctionRef& getter, 
             const FunctionRef& setter, 
             MemoryHeap* pheap, 
             ASRefCountCollector* pCC);

    // Copy constructor - optimized implementation in C++.
    // This is called *very* frequently, so we do *NOT* want
    // to make use of operator =.
    Value(const Value& v);

    // Destructor: cleanup only necessary for allocated types.
    ~Value();

#ifdef GFX_AS_ENABLE_GC
    typedef RefCountCollector<StatMV_ActionScript_Mem> Collector;
    template <class Functor>
    void ForEachChild_GC(Collector* prcc) const;
    void Finalize_GC();
#endif // SF_NO_GC

    // Cleanup: Useful when changing types/values.
    void    DropRefs();
    
    // Assignment.
    void    operator = (const Value& v);
    
    type GetType() const        { return (type)T.Type; }
    bool TypesMatch(const Value& val) const
    {
        if (GetType() != val.GetType())
        {
            // special case to compare numeric types. INTEGER and NUMBER
            // should be treated as matching types.
            return ((GetType()     == NUMBER || GetType()     == INTEGER) &&
                    (val.GetType() == NUMBER || val.GetType() == INTEGER));
        }
        return true;
    }

    // Property flag value access for GASMember.
    UByte GetPropFlags() const  { return T.PropFlags;  }
    void  SetPropFlags(UByte f) { T.PropFlags = f;  }

    bool IsObject() const       { return T.Type == OBJECT; }
    bool IsFunction() const     { return T.Type == FUNCTION || T.Type == FUNCTIONNAME; }
    bool IsFunctionObject() const { return T.Type == FUNCTION; }
    bool IsFunctionName() const { return T.Type == FUNCTIONNAME; }
    bool IsUndefined() const    { return T.Type == UNDEFINED || T.Type == UNSET; }
    bool IsCharacter() const    { return T.Type == CHARACTER; }
    bool IsString() const       { return T.Type == STRING; }
    bool IsNumber() const       { return T.Type == NUMBER || T.Type == INTEGER; }
    bool IsFloatingNumber() const { return T.Type == NUMBER; }
    bool IsIntegerNumber() const  { return T.Type == INTEGER; }
    bool IsBoolean() const      { return T.Type == BOOLEAN; }
    bool IsNull() const         { return T.Type == NULLTYPE; }
    bool IsSet() const          { return T.Type != UNSET; }
    bool IsProperty() const     { return T.Type == PROPERTY; }
    bool IsPrimitive() const    { return IsString() || IsBoolean() || IsNull() || IsNumber(); }
    bool IsResolveHandler() const { return T.Type == RESOLVEHANDLER; }
    
    // precision - >0 - specifies the precision for Number-to-String conversion
    //             <0 - default precision will be used (maximal)
    //             ignored, if conversion is not from Number.
    SF_INLINE ASString   ToString(Environment* penv, int precision = -1) const
    {
        return ToStringImpl(penv, precision, 0);
    }
    ASString           ToStringVersioned(Environment* penv, unsigned version) const;
    // Same as above, but will not invoke action script; useful for trace logs.
    ASString           ToDebugString(Environment* penv) const;
    // Used to implement above.
    ASString           ToStringImpl(Environment* penv, int precision, bool debug) const;


    Number           ToNumber(Environment* penv) const;
    SInt32              ToInt32(Environment* penv) const;
    UInt32              ToUInt32(Environment* penv) const;

    bool                ToBool(const Environment* penv) const;
    // converts value to an object. Parameter "penv" is required only
    // if function name resolving may occur; in other cases NULL may be used.
    Object*          ToObject(const Environment* penv) const;
    InteractiveObject*  ToCharacter(const Environment* penv) const;
    AvmCharacter*       ToAvmCharacter(const Environment* penv) const;
    ObjectInterface* ToObjectInterface(const Environment* penv) const;
    // converts value to a function reference object. Parameter "penv" is required only
    // if function name resolving may occur; in other cases NULL may be used.
    FunctionRef      ToFunction(const Environment* penv) const;
    FunctionRef      ToResolveHandler() const;

    enum Hint {
        NoHint, HintToNumber, HintToString
    };
    Value            ToPrimitive(Environment* penv, Hint hint = NoHint) const;

    void    ConvertToNumber(Environment* penv);
    void    ConvertToString(Environment* penv);
    void    ConvertToStringVersioned(Environment* penv, unsigned version);

    // These Set*()'s are more type-safe; should be used
    // in preference to generic overloaded Set().  You are
    // more likely to get a warning/error if misused.
    void    SetUndefined()                      { DropRefs(); T.Type = UNDEFINED; }
    void    SetNull()                           { DropRefs(); T.Type = NULLTYPE; }
    void    SetString(const ASString& str)      { if (T.Type >= STRING) DropRefs(); T.Type = STRING; V.pStringNode = str.GetNode(); V.pStringNode->AddRef(); }    
    void    SetNumber(double val)               { if (T.Type >= STRING) DropRefs(); T.Type = NUMBER; NV.NumberValue = val; }
    void    SetBool(bool val)                   { DropRefs(); T.Type = BOOLEAN; V.BooleanValue = val; }
    void    SetInt(int val)                     { if (T.Type >= STRING) DropRefs(); T.Type = INTEGER; NV.Int32Value = val; }
    void    SetUInt(unsigned val)               { if (T.Type >= STRING) DropRefs(); T.Type = INTEGER; NV.UInt32Value = val; }
    void    SetAsObject(Object* pobj);   
    void    SetAsCharacterHandle(CharacterHandle* pchar);
    //void    SetAsCharacter(InteractiveObject* pchar);
    void    SetAsCharacter(InteractiveObject* pchar);
    void    SetAsObjectInterface(ObjectInterface* pobj);
    void    SetAsFunction(const FunctionRefBase& func);
    void    SetAsResolveHandler(const FunctionRefBase& func);
    void    SetUnset()                          { DropRefs(); T.Type = UNSET; }  

    // return true, if this equals to v
    bool    IsEqual (Environment* penv, const Value& v) const;
    // compares this and v. 
    // action: 0 - equal, <0 - less, >0 - greater
    // return Boolean true or false, if comparison satisfies\not satisfies to the action. Otherwise, undefined.
    Value  Compare (Environment* penv, const Value& v, int action) const;
    
    // special versions of Add/Sub used for increment/decrement
    void    Add (Environment* penv, int v2);
    void    Sub (Environment* penv, int v2);

    void    Add (Environment* penv, const Value& v);
    void    Sub (Environment* penv, const Value& v);
    void    Mul (Environment* penv, const Value& v);
    void    Div (Environment* penv, const Value& v);   
    void    And (Environment* penv, const Value& v)   { SetInt(ToInt32(penv) & v.ToInt32(penv)); }
    void    Or  (Environment* penv, const Value& v)   { SetInt(ToInt32(penv) | v.ToInt32(penv)); }
    void    Xor (Environment* penv, const Value& v)   { SetInt(ToInt32(penv) ^ v.ToInt32(penv)); }
    void    Shl (Environment* penv, const Value& v)   { SetInt(ToInt32(penv) << v.ToInt32(penv)); }
    void    Asr (Environment* penv, const Value& v)   { SetInt(ToInt32(penv) >> v.ToInt32(penv)); }
    void    Lsr (Environment* penv, const Value& v)   { SetInt((ToUInt32(penv) >> v.ToInt32(penv))); }

    void    StringConcat(Environment* penv, const ASString& str);

    bool    GetPropertyValue(Environment* penv, ObjectInterface* pthis, Value* value) const;
    void    SetPropertyValue(Environment* penv, ObjectInterface* pthis, const Value& val); 

    const ASString& GetCharacterNamePath(Environment* penv) const; 
protected:
    FunctionRef ResolveFunctionName(const Environment* penv) const;
};

// This class might be used to preserve Value members from deletion.
// This is mostly actual for preserving characters from deletion, since the
// Value stores only GFxCharHandle and the character itself might be freed.
// This class stores addrefed InteractiveObject* if the character is stored in Value.
class ValueGuard
{
    const Environment*  pEnv;
    Value               mValue;
    InteractiveObject*  pChar;
public:
    explicit ValueGuard(const Environment* penv) : pEnv(penv), pChar(NULL) { SF_ASSERT(pEnv); }
    ValueGuard(const Environment* penv, const Value& val);
    ~ValueGuard();

    ValueGuard& operator=(const Value& val);
};

#if defined(SF_CC_MSVC) && defined(SF_CPU_X86)
#pragma pack(pop)
#endif


// Parameters/environment for C functions callable from ActionScript.
class FnCall : public LogBase<FnCall>
{
public:
    Value*              Result;
    ObjectInterface*    ThisPtr;
    FunctionRef         ThisFunctionRef; // if this is represented as function
    Environment*        Env;
    int                 NArgs;
    int                 FirstArgBottomIndex;

    FnCall(Value* ResIn, ObjectInterface* ThisIn, Environment* EnvIn, int NargsIn, int FirstIn)
        :
        Result(ResIn),
        ThisPtr(ThisIn),
        Env(EnvIn),
        NArgs(NargsIn),
        FirstArgBottomIndex(FirstIn)
    {
    }

    FnCall(Value* ResIn, const Value& ThisIn, Environment* EnvIn, int NargsIn, int FirstIn)
        :
        Result(ResIn),
        ThisPtr(ThisIn.ToObjectInterface(EnvIn)),
        Env(EnvIn),
        NArgs(NargsIn),
        FirstArgBottomIndex(FirstIn)
    {
        SF_ASSERT(!ThisIn.IsPrimitive());
        if (ThisIn.IsFunction())
            ThisFunctionRef = ThisIn.ToFunction(EnvIn);
    }

    virtual ~FnCall () { }

    // Access a particular argument.
    Value& Arg(int n) const;

    // Logging script support
    Log*         GetLog() const;
    bool            IsVerboseAction() const;
    bool            IsVerboseActionErrors() const;

    bool            CheckThisPtr
        (unsigned type) const;
    void            ThisPtrError
        (const char* className, const char* psrcfile, int line) const;
};

#ifdef SF_BUILD_DEBUG
#define CHECK_THIS_PTR(fn,classname)\
    do {\
    if (!fn.CheckThisPtr(ObjectInterface::Object_##classname)) \
        { fn.ThisPtrError(#classname, __FILE__, __LINE__); return; } \
    } while(0)
#define CHECK_THIS_PTR2(fn,classname1,classname2)\
    do {\
        bool ctptr1 = fn.CheckThisPtr(ObjectInterface::Object_##classname1); \
        bool ctptr2 = fn.CheckThisPtr(ObjectInterface::Object_##classname2); \
        if (!ctptr1 && !ctptr2) { \
            if      (!ctptr1) { fn.ThisPtrError(#classname1, __FILE__, __LINE__); return; } \
            else if (!ctptr2) { fn.ThisPtrError(#classname2, __FILE__, __LINE__); return; } \
        } \
    } while(0)
#else
#define CHECK_THIS_PTR(fn,classname)\
    do {\
    if (!fn.CheckThisPtr(ObjectInterface::Object_##classname)) \
        { fn.ThisPtrError(#classname, NULL, 0); return; } \
    } while(0)
#define CHECK_THIS_PTR2(fn,classname1,classname2)\
    do {\
        bool ctptr1 = fn.CheckThisPtr(ObjectInterface::Object_##classname1); \
        bool ctptr2 = fn.CheckThisPtr(ObjectInterface::Object_##classname2); \
        if (!ctptr1 && !ctptr2) { \
            if      (!ctptr1) { fn.ThisPtrError(#classname1, NULL, 0); return; } \
            else if (!ctptr2) { fn.ThisPtrError(#classname2, NULL, 0); return; } \
        } \
    } while(0)
#endif

} // SF
} // AS2
} // GFx

#endif //INC_SF_GFx_AS2_Value_H
