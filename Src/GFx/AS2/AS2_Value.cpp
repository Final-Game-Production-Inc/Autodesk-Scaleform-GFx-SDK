/**************************************************************************

Filename    :   AS2_Value.cpp
Content     :   Value implementation
Created     :   
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_InteractiveObject.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_FunctionRefImpl.h"
#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_StringObject.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_BooleanObject.h"
#include "GFx/GFx_Log.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

// Utility.  Try to convert str to a number.  If successful,
// put the result in *result, and return true.  If not
// successful, put 0 in *result, and return false.
// Also, "Infinity", "-Infinity", and "NaN"
// are recognized.
// !DP (SInt32)strtoul is used for oct and hex to match Flash behavior  
static bool StringToNumber(Number* result, const char* str)
{
    char* tail = 0;
    UPInt len = SFstrlen (str);

    if (*str == '0' && SFtolower (*(str+1)) == 'x')  // hexadecimal
    {
        // hexadecimal values can't be negative
        *result = (Number)(SInt32)SFstrtoul(str, &tail, 0);
    }
    else
    {
        // check, does the string represent floating point number
        if (strcspn (str, ".Ee") != len)
        {
            *result = (Number)SFstrtod(str, &tail);
        }
        else
        {
            UPInt plen = len;
            const char* positive = str;
            int sign = 1;
            
            if (*str == '-')
            {
                positive = str + 1;
                sign = -1;
                --plen;
            }
            else if (*str == '+')
            {
                positive = str + 1;
                --plen;
            }
            
            // octal numbers may be negative
            if (*positive == '0' && strspn (positive, "01234567") == plen)  // octal 
            {
                *result = (Number)(SInt32)(SFstrtoul(positive, &tail, 8) * sign);
            }
            else
            {
                *result = (Number)(SFstrtod (positive, &tail) * sign);
            }
        }  
    }
    if (tail == str || *tail != 0)
    {
        //!AB, string values "Infinity", "-Infinity" should be
        // converted to NaN.
        // Moock mentioned that these strings should be converted in appropriate
        // number values, but in practice it is not so: neither for Flash 5-6, nor
        // for Flash 7+
        /*
        if(SFstrcmp(str, "NaN") == 0)
        {
            *result = NumberUtil::NaN();
            return true;
        }
        if(SFstrcmp(str, "Infinity") == 0)
        {
            *result = NumberUtil::POSITIVE_INFINITY();
            return true;
        }
        if(SFstrcmp(str, "-Infinity") == 0)
        {
            *result = NumberUtil::NEGATIVE_INFINITY();
            return true;
        }*/
        // Failed conversion to Number.
        return false;
    }
    return true;
}

ValueProperty::ValueProperty
    (ASRefCountCollector* pCC, const FunctionRef& getterMethod, const FunctionRef& setterMethod) 
:   ASRefCountBase<ValueProperty>(pCC),
    GetterMethod(getterMethod), SetterMethod(setterMethod)
{
}

//
// Value -- ActionScript value type
//

Value::Value(const Value& v)
{
    T.Type = v.T.Type;
   
    switch(v.T.Type)
    {    
    case BOOLEAN:
        V.BooleanValue = v.V.BooleanValue;
        break;

    case FUNCTIONNAME:
    case STRING:        
        V.pStringNode = v.V.pStringNode;
        V.pStringNode->AddRef();
        break;

    case NUMBER:
        NV.NumberValue = v.NV.NumberValue;
        break;
        
    case INTEGER:
        NV.Int32Value = v.NV.Int32Value;
        break;

    case OBJECT:
        if (v.V.pObjectValue)
        {
            if (v.V.pObjectValue->IsFunction())
            {            
                T.Type = FUNCTION;
                V.FunctionValue.Init(v.V.pObjectValue->ToFunction());
            }
            else
            {
                V.pObjectValue = v.V.pObjectValue;
                V.pObjectValue->AddRef();
            }
        }
        else
        {
            V.pObjectValue = 0;
        }        
        break;

    case CHARACTER:
        V.pCharHandle = v.V.pCharHandle;
        if (V.pCharHandle)
            V.pCharHandle->AddRef();
        break;

    case RESOLVEHANDLER:
    case FUNCTION:
        V.FunctionValue.Init(v.V.FunctionValue);
        break;
    
    case PROPERTY:
        V.pProperty = v.V.pProperty;
        V.pProperty->AddRef();
        break;

    default:;
    }    
}


void    Value::operator = (const Value& v)
{
    // Cleanup previous value.
    if (T.Type >= STRING)
        DropRefs();

    // Perform assignment: same as copy constructor above.
    // We paste this logic because it is used very heavily
    // and should thus be optimized as much as possible.
    T.Type = v.T.Type;

    switch(v.T.Type)
    {    
    case BOOLEAN:
        V.BooleanValue = v.V.BooleanValue;
        break;

    case FUNCTIONNAME:
    case STRING:        
        V.pStringNode = v.V.pStringNode;
        V.pStringNode->AddRef();
        break;

    case NUMBER:
        NV.NumberValue = v.NV.NumberValue;
        break;

    case INTEGER:
        NV.Int32Value = v.NV.Int32Value;
        break;

    case OBJECT:
        if (v.V.pObjectValue)
        {
            if (v.V.pObjectValue->IsFunction())
            {            
                T.Type = FUNCTION;
                V.FunctionValue.Init(v.V.pObjectValue->ToFunction());
            }
            else
            {
                V.pObjectValue = v.V.pObjectValue;
                V.pObjectValue->AddRef();
            }
        }
        else
        {
            V.pObjectValue = 0;
        }        
        break;

    case CHARACTER:
        V.pCharHandle = v.V.pCharHandle;
        if (V.pCharHandle)
            V.pCharHandle->AddRef();
        break;

    case RESOLVEHANDLER:
    case FUNCTION:
        V.FunctionValue.Init(v.V.FunctionValue);
        break;

    case PROPERTY:    
        V.pProperty = v.V.pProperty;
        V.pProperty->AddRef();
        break;
    }
}



Value::Value(Object* pobj)
{
    if (pobj && pobj->IsFunction()) 
    {
        T.Type = FUNCTION;
        V.FunctionValue.Init(pobj->ToFunction());
    }
    else
    {
        T.Type = OBJECT;
        V.pObjectValue = pobj;
        if (V.pObjectValue)   
            V.pObjectValue->AddRef(); 
    }
}

Value::Value(InteractiveObject* pcharacter)    
{    
    T.Type = CHARACTER;

    if (!pcharacter)    
        V.pCharHandle = 0;
    else if ((V.pCharHandle = pcharacter->GetCharacterHandle())!=0)
        V.pCharHandle->AddRef();
}

Value::Value(ASStringContext* psc, CFunctionPtr func)
{
    T.Type = FUNCTION;
    V.FunctionValue.Init(*SF_HEAP_NEW(psc->GetHeap()) CFunctionObject (psc, func));
} 

Value::Value(const FunctionRef& func)    
{
    T.Type = FUNCTION;
    V.FunctionValue.Init(func);
}

Value::Value(const FunctionRef& getter, 
                   const FunctionRef& setter, 
                   MemoryHeap* pheap, 
                   ASRefCountCollector* pCC)    
{
    T.Type = PROPERTY;
    V.pProperty = SF_HEAP_NEW(pheap) ValueProperty(pCC, getter, setter);
}

Value::~Value()
{
    if (T.Type >= STRING)
        DropRefs();
}

#ifdef GFX_AS_ENABLE_GC
template <class Functor>
void Value::ForEachChild_GC(Collector* prcc) const
{
    if (IsFunctionObject())
        V.FunctionValue.template ForEachChild_GC<Functor>(prcc);
    else if (IsObject() && V.pObjectValue)
        Functor::Call(prcc, V.pObjectValue);
    else if (IsProperty())
        Functor::Call(prcc, V.pProperty);
}
SF_GC_PREGEN_FOR_EACH_CHILD(Value)

void Value::Finalize_GC()
{
    if (IsFunctionObject())
        V.FunctionValue.Finalize_GC();
    else if (T.Type >= STRING && !IsObject() && !IsProperty())
        DropRefs();
    // Finalize_GC for Objects and Properties will be called by GRefCountCollector
}
#endif // SF_NO_GC

// Conversion to string.
ASString   Value::ToStringImpl(Environment* penv, int precision, bool debug) const
{
    SF_ASSERT(penv);
    ASString stringVal(penv->GetBuiltin(ASBuiltin_empty_));

    switch(T.Type)
    {
    case STRING:
        {
            // Don't need to do anything.
            stringVal.AssignNode(V.pStringNode);
        }
        break;

    case NUMBER:
        {  
            char buf[NumberUtil::TOSTRING_BUF_SIZE];
            stringVal = 
                penv->CreateString(NumberUtil::ToString(NV.NumberValue, buf, sizeof(buf), (precision < 0) ? 10 : -precision));

        }
        break;
    case INTEGER:
        {  
            char buf[NumberUtil::TOSTRING_BUF_SIZE];
            stringVal = 
                penv->CreateString(NumberUtil::IntToString(NV.Int32Value, buf, sizeof(buf)));
        }
        break;
    case UNSET:
    case UNDEFINED:
        {
            // Behavior depends on file version.  In
            // version 7+, it's "undefined", in versions
            // 6-, it's "".
            //
            // We'll go with the v7 behavior by default,
            // and conditionalize via Versioned()
            // functions.

            // MA: Should we (it is easy to do now with penv);
            // however, aren't there cases when v6 also returns 'undefined' ?
            stringVal = penv->GetBuiltin(ASBuiltin_undefined);
        }
        break;
    case NULLTYPE:
        {
            stringVal = penv->GetBuiltin(ASBuiltin_null);
        }
        break;
    case BOOLEAN:
        {
            stringVal = penv->GetBuiltin(V.BooleanValue ? ASBuiltin_true : ASBuiltin_false);
        }
        break;
    case OBJECT:
    case CHARACTER:
        {
            // @@ Moock says: 
            //    - "the value that results from calling ToString() on the object".
            //    - The default ToString() returns "[object Object]" but may be customized.
            //    - A Movieclip returns the absolute path of the object.
            // 

            // Do we have a "toString" method?
            Value             toStringFunc;
            ObjectInterface * piobj = ToObjectInterface(penv);

            if (!debug && piobj &&
                piobj->GetMemberRaw(penv->GetSC(),
                penv->GetBuiltin(ASBuiltin_toString), &toStringFunc)) 
            {
                //!AB: recursion guard is necessary to prevent infinite recursion in the case
                // if "toString" returns "this" or another object that may finally returns "this".
                // This is especially dangerous because "toString" method might be invoked 
                // implicitly during the type casting.
                if (penv->RecursionGuardStart())
                {
                    Value result;
                    //GAS_Invoke(toStringFunc, &result, piobj, penv, 0, 0, NULL);
                    FunctionRef  func(toStringFunc.ToFunction(penv));
                    if (func != NULL)
                    {
                        FnCall fnCall(&result, piobj, penv, 0, 0); 
                        func.Function->Invoke(fnCall, func.pLocalFrame, NULL);
                    }
                    stringVal = result.ToString(penv);
                }
                else
                {
                    // [type Object]
                    stringVal = penv->GetBuiltin(ASBuiltin_typeObject_);
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    if (penv->IsVerboseActionErrors())
                        penv->LogScriptError("Stack overflow, max level of 255 nested calls of toString is reached.");
                    #endif
                }
                penv->RecursionGuardEnd();
            }
            else
            {   
                // Used within action log or if object is null.            
                const char* ptext;

                if ((T.Type == OBJECT) && V.pObjectValue && 
                    (ptext = V.pObjectValue->GetTextValue(penv))!=0)
                {
                    stringVal = penv->CreateString(ptext);
                }
                else if ((T.Type == CHARACTER) && V.pCharHandle)
                {
                    stringVal = GetCharacterNamePath(penv);
                }
                else
                {
                    // This is the default: [object Object]
                    stringVal = penv->GetBuiltin(ASBuiltin_objectObject_);
                }
            }
        }
        break;
    case FUNCTION:
    case FUNCTIONNAME:
        {
            // [type Function]
            stringVal =  penv->GetBuiltin(ASBuiltin_typeFunction_);
        }
        break;
    case PROPERTY:
        {
            // [property]
            stringVal =  penv->CreateConstString("[property]");
        }
        break;
    case RESOLVEHANDLER:
        {
            // [resolveHandler]
            stringVal =  penv->CreateConstString("[resolveHandler]");
        }
        break;
    default:
        {
            stringVal = penv->GetBuiltin(ASBuiltin_badtype_); // "<bad type>";
            SF_ASSERT(0);
        }
    }
    return stringVal;
}

ASString   Value::ToDebugString(Environment* penv) const
{
    ASString s = ToStringImpl(penv, -1, 1);
    return s;
}

// Conversion to const GASString.
ASString   Value::ToStringVersioned(Environment* penv, unsigned version) const
{
    if (IsUndefined())
    {
        // Version-dependent behavior.
        if (version > 0 && version <= 6)
        {
            return penv->GetBuiltin(ASBuiltin_empty_);
        }
        else
        {
            return penv->GetBuiltin(ASBuiltin_undefined);
        }
    }
    
    return ToStringImpl(penv, -1, 0);
}


// Conversion to double/float.
Number   Value::ToNumber(Environment* penv) const
{
    if (T.Type == NUMBER)
    {
        return NV.NumberValue;
    }
    else if (T.Type == INTEGER)
    {
        return (Number)NV.Int32Value;
    }
    else if (T.Type == STRING)
    {
        // @@ Moock says the rule here is: if the
        // string is a valid float literal, then it
        // gets converted; otherwise it is set to NaN.
        //
        Number num;
        if (!StringToNumber(&num, V.pStringNode->pData))
        {
            // Failed conversion to Number.
            num = NumberUtil::NaN(); //!AB
        }
        return num;
    }    
    else if (T.Type == NULLTYPE)
    {
        unsigned version = penv->GetVersion();
        if (version <= 6)
            return GFX_ASNUMBER_ZERO;
        else
            return NumberUtil::NaN();
    }
    else if (T.Type == BOOLEAN)
    {
        return V.BooleanValue ? (Number)1 : (Number)0;
    }
    else if (T.Type == CHARACTER)
    {
        // ToNumber for movieclip returns always NaN (according to Moock)
        return NumberUtil::NaN();
    }
    else if ( ((T.Type == OBJECT) && V.pObjectValue) ||
              (T.Type == FUNCTION))
    {
        // @@ Moock says the result here should be
        // "the return value of the object's ValueOf() method".     
        //
        // Arrays and Movieclips should return NaN.

        Value             toValueFunc;
        ObjectInterface * piobj = ToObjectInterface(penv);

        if (penv && piobj->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_valueOf), &toValueFunc)) 
        {
            //!AB: recursion guard is necessary to prevent infinite recursion in the case
            // if "valueOf" returns "this" or another object that may finally returns "this".
            // This is especially dangerous because "valueOf" method might be invoked 
            // implicitly during the type casting.
            Number retVal;
            if (penv->RecursionGuardStart())
            {
                Value result;
                //GAS_Invoke(toValueFunc, &result, piobj, penv, 0, 0, NULL);
                FunctionRef  func(toValueFunc.ToFunction(penv));
                if (func != NULL)
                {
                    FnCall fnCall(&result, piobj, penv, 0, 0); 
                    func.Function->Invoke(fnCall, func.pLocalFrame, NULL);
                }

                if (result.IsPrimitive())
                    retVal = result.ToNumber(penv);
                else
                    retVal = NumberUtil::NaN();
            }
            else
            {
                retVal = NumberUtil::NaN();
                #ifdef GFX_AS2_VERBOSE_ERRORS
                if (penv->IsVerboseActionErrors())
                    penv->LogScriptError("Stack overflow, max level of 255 nested calls of valueOf is reached.");
                #endif
            }
            penv->RecursionGuardEnd();
            return retVal;
        }
        else
        {
            // AB: valueOf should be implemented for each object!
            // leave this branch just for now.
            // Text characters with var names could get in here.
            if (T.Type == CHARACTER)
                return NumberUtil::NaN();
            
            const char* ptextval = piobj->GetTextValue(penv);
            if (ptextval)
            {
                return (Number)atof(ptextval);
            }
        }
        return GFX_ASNUMBER_ZERO;
    }
    else if (IsUndefined())
    {
        unsigned version = penv->GetVersion();

        //!AB: Flash 6 and below: 0
        //     Flash 7 and above: NaN
        if (version > 0 && version <= 6)
            return GFX_ASNUMBER_ZERO;
        else
            return NumberUtil::NaN();
    }
    else
    {
        return GFX_ASNUMBER_ZERO;
    }
}

SInt32  Value::ToInt32(Environment* penv) const
{
    if (T.Type == INTEGER)
    {
        return NV.Int32Value;
    }
    else
    {
        // follow to algorithm described in ECMA-262, clause 9.5
        static const Number UINT32MAX = Number(~0u);
        static const Number INT32MAX  = Number(double(1UL<<31))-1;
        static const Number INT32MIN  = -Number(double(1UL<<31));

        Number v = ToNumber(penv);
        if (NumberUtil::IsNaNOrInfinity(v) || v == 0)
            return 0;
        if (v >= INT32MIN && v <= INT32MAX)
            return (SInt32)v;

        Number anv = floor(Alg::Abs(v));
        UInt32 uv = (UInt32)fmod(anv, UINT32MAX + 1);

        SInt32 rv = -SInt32(~0u - uv + 1); // actually, this is an analog of (uv - 2^32)
        if (v < 0) rv = -rv;

        return rv;
    }
}

UInt32    Value::ToUInt32(Environment* penv) const
{
    if (T.Type == INTEGER)
    {
        return NV.UInt32Value;
    }
    else
    {
        // follow to algorithm described in ECMA-262, clause 9.6
        static const Number UINT32MAX = Number(~0u);

        Number v = ToNumber(penv);
        if (NumberUtil::IsNaNOrInfinity(v) || v == 0)
            return 0;
        if (v >= 0 && v <= UINT32MAX)
            return (UInt32)v;

        Number anv = floor(Alg::Abs(v));
        UInt32 uv = (UInt32)fmod(anv, UINT32MAX + 1);
        if (v < 0) uv = 0u-uv;
        return uv;
    }
}

// Conversion to boolean.
bool    Value::ToBool(const Environment* penv) const
{
    if (T.Type == STRING)
    {
        //!AB, "true" and "false" should never be converted
        // to appropriate boolean value.
        // For Flash 6, the only string containing non-zero numeric
        // value is converted to boolean as "true".
        // For Flash 7+, any non-empty string is converted to 
        // boolean as "true".
        /*
        if (StringValue == "false")
        {
            return false;
        }
        else if (StringValue == "true")
        {
            return true;
        }*/

        // Empty string --> false
        if (V.pStringNode->Size == 0)
            return false;

        if (penv->GetVersion() <= 6)
        {
            // @@ Moock: "true if the string can
            // be converted to a valid nonzero number".
            //
            Number num;
            if (StringToNumber(&num, V.pStringNode->pData))
            {
                if (NumberUtil::IsNaN(num)) return false;
                return (!!num);
            }
            return false;
        }
        return true;
    }
    else if (T.Type == NUMBER)
    {
        // @@ Moock says, NaN --> false
        if (NumberUtil::IsNaN(NV.NumberValue))
            return false;
        return NV.NumberValue != GFX_ASNUMBER_ZERO;
    }
    else if (T.Type == INTEGER)
    {
        return NV.Int32Value != 0;
    }
    else if (T.Type == BOOLEAN)
    {           
        return V.BooleanValue;
    }
    else if (T.Type == OBJECT)
    {
        return V.pObjectValue != NULL;
    }
    else if (T.Type == CHARACTER)
    {
        return ToCharacter(penv) != NULL;
    }
    /*else if (Type == C_FUNCTION)
    {
        return C_FunctionValue != NULL;
    }  */
    else if (T.Type == FUNCTION)
    {
        return !V.FunctionValue.IsNull ();
    }
    else if (T.Type == FUNCTIONNAME)
    {
        return true;
    }
    else
    {
        SF_ASSERT(IsUndefined() || T.Type == NULLTYPE);
        return false;
    }
}


// Return value as an object.
Object*  Value::ToObject(const Environment* penv) const
{
    switch(T.Type)
    {
    case OBJECT:
        return V.pObjectValue;

    case FUNCTION:
        if (!V.FunctionValue.IsNull ())
            return V.FunctionValue.GetObjectPtr();
        break;
    case FUNCTIONNAME:
        {
            FunctionRef funcRef = ResolveFunctionName(penv);
            if (!funcRef.IsNull())
            {
                return funcRef.GetObjectPtr();
            }
        }
        break;
    case CHARACTER:
        {
            // If this occurs, it means that code used ToObject() call instead of ToObjectInterface() in a
            // place where characters should be permitted.
            // In rare cases that this is necessary, an explicit != CHARACTER check should be done,
            // so that we can keep this useful warning.
            SF_DEBUG_WARNING(1, "Value::ToObject() invoked on a character. Need to use ToObjectInterface?");
        }
        break;
    case PROPERTY:
        if (IsProperty())
        {
            AvmCharacter* paschar = penv->GetAvmTarget();
            if (paschar)
            {
                Value val;
                //!AB: environment shouldn't be constant here, since it can be modified by the
                // GetPropertyValue call. TODO - remove all 'const' attributes from env parameters.
                if (GetPropertyValue(const_cast<Environment*>(penv), paschar, &val))
                    return val.ToObject(penv);
            }
        }
        break;
    default:;
    }
    return NULL;
}

AvmCharacter*     Value::ToAvmCharacter(const Environment* penv) const
{
    return GFx::AS2::ToAvmCharacter(ToCharacter(penv));
}

InteractiveObject*     Value::ToCharacter(const Environment* penv) const
{
    if (T.Type == CHARACTER && penv)
    {
        if (V.pCharHandle)
        {
            DisplayObject* dob = V.pCharHandle->ResolveCharacter(penv->GetMovieImpl());
            return (dob) ? dob->CharToInteractiveObject() : NULL;
        }
    }
    return 0;   
}

ObjectInterface* Value::ToObjectInterface(const Environment* penv) const
{
    if (T.Type == CHARACTER)
        return ToAvmCharacter(penv);
    return ToObject(penv);
}   



// Return value as a function.  Returns NULL if value is
// not a function.
FunctionRef  Value::ToFunction(const Environment* penv) const
{
    switch(T.Type)
    {
    case FUNCTIONNAME:
        return ResolveFunctionName(penv);
    case FUNCTION:
        return FunctionRef(V.FunctionValue);
    default:;
    }
    return FunctionRef();
}

FunctionRef  Value::ToResolveHandler() const
{
    if (T.Type == RESOLVEHANDLER)
        return V.FunctionValue;
    return FunctionRef();
}

// Force type to number.
void    Value::ConvertToNumber(Environment* pEnv)
{
    SetNumber(ToNumber(pEnv));
}

// Force type to string.
void    Value::ConvertToString(Environment* pEnv)
{
    ASString str = ToString(pEnv);    
    DropRefs();
    T.Type = STRING;  // force type.
    V.pStringNode = str.GetNode();
    V.pStringNode->AddRef();
}


// Force type to string.
void    Value::ConvertToStringVersioned(Environment* pEnv, unsigned version)
{
    ASString str = ToStringVersioned(pEnv, version);    
    DropRefs();
    T.Type = STRING;  // force type.
    V.pStringNode = str.GetNode();
    V.pStringNode->AddRef();
}


void    Value::SetAsObject(Object* obj)
{
    if (obj && obj->IsFunction())
    {
        SetAsFunction(obj->ToFunction());
    }
    else if (T.Type != OBJECT || V.pObjectValue != obj)
    {
        DropRefs();
        T.Type         = OBJECT;
        V.pObjectValue = obj;
        if (V.pObjectValue)
            V.pObjectValue->AddRef();
    }
}
    
void    Value::SetAsCharacterHandle(CharacterHandle* pchar)
{
    if (T.Type != CHARACTER || V.pCharHandle != pchar)
    {
        DropRefs();
        T.Type        = CHARACTER;
        V.pCharHandle = pchar;
        if (V.pCharHandle)
            V.pCharHandle->AddRef();
    }
}

void    Value::SetAsCharacter(InteractiveObject* pchar)
{
    SetAsCharacterHandle(pchar ? pchar->GetCharacterHandle() : 0);
}

// void    Value::SetAsCharacter(InteractiveObject* pchar) 
// { 
//     SetAsCharacter(::ToASCharacter(pchar)); 
// }

void    Value::SetAsObjectInterface(ObjectInterface* pobj)
{
    // More expensive then either SetAsObject or SetAsCharacter, should be called only when necessary.
    if (pobj->IsASCharacter())
        SetAsCharacter(pobj->ToCharacter());
    else
        SetAsObject(pobj->ToASObject());
}


void    Value::SetAsFunction(const FunctionRefBase& func)
{
    if (T.Type != FUNCTION || V.FunctionValue != func)
    {
        DropRefs();
        T.Type = FUNCTION;
        V.FunctionValue.Init(func);
    }
}

void    Value::SetAsResolveHandler(const FunctionRefBase& func)
{
    DropRefs();
    T.Type = RESOLVEHANDLER;
    V.FunctionValue.Init(func);
}


void    Value::Add(Environment* penv, const Value& v)  
{ 
    Value pv1, pv2;

    pv1 = ToPrimitive (penv);
    pv2 = v.ToPrimitive (penv);

    if(pv1.T.Type == STRING || pv2.T.Type == STRING) 
    {
        unsigned version = penv->GetVersion();

        pv1.ConvertToStringVersioned(penv, version);
        pv1.StringConcat(penv, pv2.ToStringVersioned(penv, version));
        SetString(pv1.ToString(penv));
    }
    else 
    {
        SetNumber(pv1.ToNumber(penv) + pv2.ToNumber(penv)); 
    }
}

void    Value::Add(Environment* penv, int v2)  
{ 
    Value pv1;

    pv1 = ToPrimitive (penv);

    if(pv1.T.Type == STRING) 
    {
        unsigned version = penv->GetVersion();

        pv1.ConvertToStringVersioned(penv, version);
        pv1.StringConcat(penv, Value(v2).ToStringVersioned(penv, version));
        SetString(pv1.ToString(penv));
    }
    else 
    {
        SetNumber(pv1.ToNumber(penv) + (Number)v2); 
    }
}

void    Value::Sub (Environment* penv, const Value& v)   
{ 
    Number v1 = ToNumber(penv);
    Number v2 = v.ToNumber(penv);
    SetNumber(v1 - v2); 
}

void    Value::Sub (Environment* penv, int v2)   
{ 
    Number v1 = ToNumber(penv);
    SetNumber(v1 - (Number)v2); 
}

void    Value::Mul (Environment* penv, const Value& v)   
{ 
    Number v1 = ToNumber(penv);
    Number v2 = v.ToNumber(penv);
    SetNumber(v1 * v2); 
}

// Return true if operands are equal.
bool    Value::IsEqual (Environment* penv, const Value& v) const
{
    bool    thisNullType = (IsUndefined()) || (T.Type == NULLTYPE);
    bool    vNullType    = (v.IsUndefined()) || (v.GetType() == NULLTYPE);

    if (thisNullType || vNullType)
    {
        return thisNullType == vNullType;
    }

    switch(T.Type)
    {
        case STRING:
            if (v.IsNumber())
                // ECMA-262, 11.9.3.17
                return Value(ToNumber(penv)).IsEqual(penv, v);
            else if (v.T.Type == BOOLEAN)
                // ECMA-262, 11.9.3.19
                return IsEqual(penv, Value(v.ToNumber(penv)));
            else if (v.IsFunction() || v.IsObject())
            {
                // ECMA-262, 11.9.3.20
                Value primVal = v.ToPrimitive(penv);
                if (primVal.IsPrimitive())
                    return IsEqual(penv, primVal);
                else
                    return false;
            }
            return ASString(V.pStringNode) == v.ToString(penv);

        case NUMBER:
        {
            Number pv;
            if (v.T.Type == BOOLEAN)
                // ECMA-262, 11.9.3.19
                pv = v.ToNumber(penv);
            else if (v.IsFunction() || v.IsObject())
            {
                // ECMA-262, 11.9.3.20
                Value primVal = v.ToPrimitive(penv);
                if (primVal.IsPrimitive())
                    return IsEqual(penv, primVal);
                else
                    return false;
            }
            else
                pv = v.ToNumber(penv);
            // ECMA-262, 11.9.3.5 - 7
            if (NumberUtil::IsNaN(NV.NumberValue) && NumberUtil::IsNaN(pv))
                return true;
            else if (NumberUtil::IsNaN(NV.NumberValue) || NumberUtil::IsNaN(pv))
                return false;
            return Double(NV.NumberValue) == pv;
        }
        case INTEGER:
            {
                if (v.T.Type == NUMBER)
                {
                    // can't compare integer with floating point directly;
                    // so, convert integer to a fp and compare.
                    Value dv;
                    dv.SetNumber((Number)NV.Int32Value);
                    return dv.IsEqual(penv, v);
                }
                SInt32 pv;
                if (v.T.Type == BOOLEAN)
                    // ECMA-262, 11.9.3.19
                    pv = v.ToInt32(penv);
                else if (v.IsFunction() || v.IsObject())
                {
                    // ECMA-262, 11.9.3.20
                    Value primVal = v.ToPrimitive(penv);
                    if (primVal.IsPrimitive())
                        return IsEqual(penv, primVal);
                    else
                        return false;
                }
                else
                    pv = v.ToInt32(penv);
                // ECMA-262, 11.9.3.5 - 7
                return NV.Int32Value == pv;
            }
        case BOOLEAN:
            // According to ECMA-262, clause 11.9.3, if type(x) or (y) is Boolean
            // return ToNumber(x) == y (or x == ToNumber(y))
            return Value(ToNumber(penv)).IsEqual(penv, v);

        // Objects and functions are compared by reference.
        case OBJECT:
        case FUNCTION:
            if (v.IsNumber() || v.T.Type == STRING || v.T.Type == BOOLEAN)
            {
                // ECMA-262, 11.9.3.19
                // ECMA-262, 11.9.3.21
                Value primVal = ToPrimitive(penv);
                if (primVal.IsPrimitive())
                    return primVal.IsEqual(penv, v);
                else
                    return false;
            }
            if (T.Type == OBJECT && v.T.Type == OBJECT)
                return V.pObjectValue == v.V.pObjectValue;
            else if (T.Type == FUNCTION && v.T.Type == FUNCTION)
                return V.FunctionValue == v.V.FunctionValue;
            return false;

        // Flash MovieClips are always compared by path only
        // (although this is technically not always correct).
        case CHARACTER:
            if (v.T.Type != CHARACTER)
                return false; // type mismatch, return false
            if (!V.pCharHandle || !v.V.pCharHandle) // should not happen.
                return V.pCharHandle == v.V.pCharHandle;    
            return GetCharacterNamePath(penv) == v.GetCharacterNamePath(penv);


        default:
        break;
    }

    // That's it. Anything else?    
    return T.Type == v.T.Type;
}

// action: 0 - equals, -1 - less than, 1 - greater than
// return value: either BOOLEAN (true\false) or undefined.
Value    Value::Compare (Environment* penv, const Value& v, int action) const
{
    if (action == 0)
        return Value(IsEqual(penv, v));

    // do comparison according to ECMA-262, 11.8.5
    Value pv1 = ToPrimitive(penv);
    Value pv2 = v.ToPrimitive(penv);
    bool result = false;
    if (pv1.IsString() && pv2.IsString())
    {
        // do strings comparison, see 11.8.5.16
        if (action < 0)
            return (pv1.ToString(penv) < pv2.ToString(penv));
        else
            return (pv1.ToString(penv) > pv2.ToString(penv));
    }
    if (penv->GetVersion() > 6 && (pv1.IsUndefined() || pv2.IsUndefined()))
        return Value();

    // do operator < comparison
    Number val1, val2;
    if (action < 0)
    {
        val1 = pv1.ToNumber(penv);
        val2 = pv2.ToNumber(penv);
    }
    else
    {
        val2 = pv1.ToNumber(penv);
        val1 = pv2.ToNumber(penv);
    }

    if (NumberUtil::IsNaN(val1) || NumberUtil::IsNaN(val2))
        return Value();
    if (val1 == val2)
        result = false;
    else if (NumberUtil::IsNEGATIVE_ZERO(val1) && NumberUtil::IsPOSITIVE_ZERO(val2))
        result = false;
    else if (NumberUtil::IsNEGATIVE_ZERO(val2) && NumberUtil::IsPOSITIVE_ZERO(val1))
        result = false;
    else if (NumberUtil::IsPOSITIVE_INFINITY(val1))
        result = false;
    else if (NumberUtil::IsPOSITIVE_INFINITY(val2))
        result = true;
    else if (NumberUtil::IsNEGATIVE_INFINITY(val2))
        result = false;
    else if (NumberUtil::IsNEGATIVE_INFINITY(val1))
        result = true;
    else
        result = (val1 < val2);
    return Value(result);
}


// Sets *this to this string plus the given string.
void    Value::StringConcat(Environment* penv, const ASString& str)
{
    ASString tempstr(ToString(penv) + str);
    DropRefs();
    T.Type        = STRING;
    V.pStringNode = tempstr.GetNode();
    V.pStringNode->AddRef();
}

// This hack is used to prevent memory leak in situations like this:
//
//  var o = new Object;
//  o._listeners = new Array;
//  o._listeners[0] = o;
//
// This is used in Tween class, for example.
// After GC is implemented we will remove this hack. (!AB)

static void GAS_CheckForListenersMemLeak(Object* pobjectValue)
{
#ifndef GFX_AS_ENABLE_GC
    if (pobjectValue->GetRefCount() == 2 && pobjectValue->IsListenerSet)
    {
        Member val;
        // Hack-in-hack: use raw key because we don't have string context
        // here, while this is called from DropRefs and destructor.
        ASString::RawCompareKey rkey("_listeners", sizeof("_listeners")-1);

        if (pobjectValue->Members.GetAlt(rkey, &val))
        {
            Object* vobj = val.Value.ToObject(NULL);
            if (vobj && vobj->GetObjectType() == Object::Object_Array)
            {
                ArrayObject* arrayObj = static_cast<ArrayObject*>(vobj);
                for(int i = 0, n = arrayObj->GetSize(); i < n; ++i)
                {
                    Value* elem = arrayObj->GetElementPtr(i);
                    if (elem && elem->IsObject())
                    {
                        if (elem->ToObject(NULL) == pobjectValue)
                        {
                            elem->V.pObjectValue->Release();
                            elem->V.pObjectValue = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
#else
    SF_UNUSED(pobjectValue);
#endif
}


// Drop any ref counts we have; this happens prior to changing our value.
void    Value::DropRefs()
{
    switch(T.Type)
    {
    case FUNCTIONNAME:
    case STRING:
        // pStringNode can not be null (null strings are not supported)!
        V.pStringNode->Release();
        break;

    case FUNCTION:
    case RESOLVEHANDLER:
        if (V.FunctionValue != NULL)
        {
            V.FunctionValue.DropRefs();
        }
        break;

    case OBJECT:
        if (V.pObjectValue)
        {
            // Tween class '_listeners' leak detection hack! Comments above.
            GAS_CheckForListenersMemLeak(V.pObjectValue);
            V.pObjectValue->Release();
            V.pObjectValue = 0;
        }
        break;

    case CHARACTER:
        if (V.pCharHandle)
        {
            V.pCharHandle->Release();
            V.pCharHandle = 0;
        }
        break;

    case PROPERTY:
        if (V.pProperty)
        {
            V.pProperty->Release();
            V.pProperty = 0;
        }
        break;
    }
    
}

// TODO: use hint
Value Value::ToPrimitive(Environment* penv, Value::Hint /*hint*/) const
{
    if ((T.Type == OBJECT) || (T.Type == CHARACTER) || (T.Type == FUNCTION))
    {
        Value             toValueFunc;
        ObjectInterface * piobj = ToObjectInterface(penv);

        if (penv && piobj && piobj->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_valueOf), &toValueFunc)) 
        {
            Value result;
            if (penv->RecursionGuardStart())
            {
                //GAS_Invoke(toValueFunc, &result, piobj, penv, 0, 0, NULL);
                FunctionRef  func(toValueFunc.ToFunction(penv));
                if (func != NULL)
                {
                    FnCall fnCall(&result, piobj, penv, 0, 0); 
                    func.Function->Invoke(fnCall, func.pLocalFrame, NULL);
                }
            }
            else
            {
                #ifdef GFX_AS2_VERBOSE_ERRORS
                if (penv->IsVerboseActionErrors())
                    penv->LogScriptError("Stack overflow, max level of 255 nested calls of valueOf is reached.");
                #endif
            }
            penv->RecursionGuardEnd();

            return result;
        }
        else
        {
            Value    ret;
            const char* pstr;

            if ((T.Type == CHARACTER) && V.pCharHandle)
            {
                ret.SetString(GetCharacterNamePath(penv));
            }
            else if ((T.Type == OBJECT) && V.pObjectValue && 
                     (pstr = V.pObjectValue->GetTextValue(penv))!=0)
            {
                ret.SetString(penv->CreateString(pstr));
            }
            else
            {
                ret.SetString(ToString(penv));//??
                //ret.SetString ("[object Object]"); // default (??)
            }
            return ret;
            /* //AB, will think about this later
            else if (penv != 0) {
                Log* plog = penv->GetTarget()->GetLog();

                plog->LogScriptError ("Error: \"valueOf\" method is not found in ToPrimitive\n");
            }
            else 
                SF_ASSERT (penv != 0); //AB ??? not sure what to do*/
        }
    }
    else if (T.Type == FUNCTIONNAME)
    {
        FunctionRef funcRef = ResolveFunctionName(penv);
        if (!funcRef.IsNull())
        {
            return funcRef;
        }
    }

    // TODO: do we need to do anything with other types?
    return *this;
}

bool        Value::GetPropertyValue(Environment* penv, ObjectInterface* pthis, Value* value) const
{
    if (IsProperty() && penv)
    {
        if (!V.pProperty->GetterMethod.IsNull())
        {
            Value result;
            V.pProperty->GetterMethod.Invoke(FnCall(&result, pthis, penv, 0, 0));
            *value = result;
            return true;
        }
        else
        {
            if (penv->IsVerboseActionErrors())
                penv->LogScriptError("Getter method is null.");
        }
    }
    return false;
}

void        Value::SetPropertyValue(Environment* penv, ObjectInterface* pthis, const Value& val)
{
    if (IsProperty() && penv)
    {
        if (!V.pProperty->SetterMethod.IsNull())
        {
            Value result;
            penv->Push(val);
            V.pProperty->SetterMethod.Invoke(FnCall(&result, pthis, penv, 1, penv->GetTopIndex()));
            penv->Drop(1);
            //?? What to do if penv is NULL?
        }
        else
        {
            if (penv->IsVerboseActionErrors())
                penv->LogScriptError("Setter method is null.");
        }
    }
}

const ASString& Value::GetCharacterNamePath(Environment* penv) const
{
    // If character is resolvable - return GetNamePath,  else return empty string
    if(V.pCharHandle)
        if(V.pCharHandle->ResolveCharacter(penv->GetMovieImpl()))
            return V.pCharHandle->GetNamePath();
    return penv->GetBuiltin(ASBuiltin_empty_);
}

FunctionRef Value::ResolveFunctionName(const Environment* penv) const
{
    if (penv && T.Type == FUNCTIONNAME)
    {
        ASString functionName(penv->GetBuiltin(ASBuiltin_empty_));
        functionName.AssignNode(V.pStringNode);

        FunctionRef ctor(penv->GetGC()->ResolveFunctionName(functionName));
        return ctor;
    }
    return FunctionRef();
}

void    Value::Div (Environment* penv, const Value& v)   
{ 
    Number v1 = ToNumber(penv);
    Number v2 = v.ToNumber(penv);
    if (NumberUtil::IsNaN(v1) || NumberUtil::IsNaN(v2))
        SetNumber(NumberUtil::NaN());
    else
    {
        Number res;
        if (v2 == 0)
        {
            if (v1 == 0)
                res = NumberUtil::NaN();               // division 0 / 0
            else if (v1 >= 0)
                res = NumberUtil::POSITIVE_INFINITY(); // division v / 0
            else
                res = NumberUtil::NEGATIVE_INFINITY(); // division -v / 0
        }
        else
            res = v1 / v2; 
        SetNumber(res); 
    }
}  

//////////////////////////////////////////////////////////////////////////
ValueGuard::ValueGuard(const Environment* penv, const Value& val)
: pEnv(penv), mValue(val)
{
    SF_ASSERT(pEnv);
    if (val.IsCharacter())
    {
        pChar = val.ToCharacter(pEnv);
        if (pChar)
            pChar->AddRef();
    }
    else
        pChar = NULL;
}

ValueGuard::~ValueGuard()
{
    if (pChar)
        pChar->Release();
}

ValueGuard& ValueGuard::operator=(const Value& val)
{
    SF_ASSERT(pEnv);
    mValue = val;
    if (pChar)
        pChar->Release();
    if (val.IsCharacter())
    {
        pChar = val.ToCharacter(pEnv);
        pChar->AddRef();
    }
    else
        pChar = NULL;
    return *this;
}

/////// FnCall //////
// Access a particular argument.
Value& FnCall::Arg(int n) const
{
    SF_ASSERT(n < NArgs);
    return Env->Bottom(FirstArgBottomIndex - n);
}

// Logging script support
Log* FnCall::GetLog() const
{ 
    return Env->GetLog();   
}

bool    FnCall::IsVerboseAction() const
{ 
    return Env->IsVerboseAction(); 
}

bool    FnCall::IsVerboseActionErrors() const
{ 
    return Env->IsVerboseActionErrors(); 
}

bool    FnCall::CheckThisPtr(unsigned type) const
{
    if (!ThisPtr || ThisPtr->GetObjectType() != (ObjectInterface::ObjectType)type)
    {
        return false;
    }
    return true;
}

void FnCall::ThisPtrError(const char* className, const char* psrcfile, int line) const
{
#ifdef SF_BUILD_DEBUG
    const char* pf = strrchr(psrcfile, '\\');
    if (!pf)
        pf = strrchr(psrcfile, '/');
    if (pf)
        ++pf;
    else
        pf = psrcfile;
    Env->LogScriptError
        ("Error: Null or invalid 'this' is used for a method of %s class, file %s, line %d\n", 
        className, pf, line);
#else
    SF_UNUSED2(psrcfile, line);
    Env->LogScriptError
        ("Error: Null or invalid 'this' is used for a method of %s class.\n", 
        className);
#endif
}

}}} // SF::GFx::AS2
