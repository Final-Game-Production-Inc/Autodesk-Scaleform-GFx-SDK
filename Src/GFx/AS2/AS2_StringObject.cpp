/**************************************************************************

Filename    :   AS2_StringObject.cpp
Content     :   Implementation of AS2 String class
Created     :   
Authors     :   Maxim Shemanarev, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_StringObject.h"
#include "Kernel/SF_UTF8Util.h"

namespace Scaleform { namespace GFx { namespace AS2 {

StringObject::StringObject(Environment* penv)
    : Object(penv), sValue(penv->GetBuiltin(ASBuiltin_empty_))
{
    commonInit(penv);
}

StringObject::StringObject(Environment* penv, const ASString& val)
    : Object(penv), sValue (val)
{
    commonInit(penv);
}

void StringObject::commonInit(Environment* penv)
{    
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_String));
}
    
const char* StringObject::GetTextValue(Environment*) const
{
    return sValue.ToCStr();
}

bool StringObject::GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
{
    // if (name == "length")        
    if (psc->GetBuiltin(ASBuiltin_length).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()))
    {
        if (!Object::GetMemberRaw(psc, name, val) || !val->IsSet())
            val->SetInt(sValue.GetLength());
        return true;
    }
    if (Object::GetMemberRaw(psc, name, val))
    {
        return true;
    }
    return false;
}

void        StringObject::SetValue(Environment* penv, const Value& v)
{
    ASString s(v.ToString(penv));
    sValue = s.ToCStr();
}

Value    StringObject::GetValue() const
{
    return Value(sValue);
}

//////////////////////////////////////////
//
void StringProto::StringCharAt(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    const ASString& str = pThis->GetString();

    ASString retVal(fn.Env->GetBuiltin(ASBuiltin_empty_));
    int index = (int) fn.Arg(0).ToNumber(fn.Env);
    if (index >= 0 && index < (int)str.GetLength())
    {
        retVal = retVal.AppendChar(str.GetCharAt(index));
    }
    fn.Result->SetString(retVal);
}

void StringProto::StringCharCodeAt(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    const ASString& str = pThis->GetString();

    if (fn.NArgs >= 1)
    {
        int index = (int) fn.Arg(0).ToNumber(fn.Env);
        if (index >= 0 && index < (int)str.GetLength())
        {
            fn.Result->SetNumber((Number)str.GetCharAt(index));
            return;
        }
    }
    fn.Result->SetNumber(NumberUtil::NaN());
}

void StringProto::StringConcat(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    // Use resizable String buffer (avoid hash re-calculation).
    StringBuffer retVal(pThis->GetString().ToCStr(), pThis->GetString().GetSize());    
    for (int i = 0; i < fn.NArgs; i++)
    {
        // Must create temporary to cold pointer.
        ASString s(fn.Arg(i).ToString(fn.Env));
        retVal += s.ToCStr();
    }

    fn.Result->SetString(fn.Env->CreateString(retVal.ToCStr(),retVal.GetSize()));
}

void StringProto::StringIndexOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
    {
        fn.Result->SetNumber(-1);
        return;
    }
    
    ASString   searchAddRef(fn.Arg(0).ToString(fn.Env));
    if (searchAddRef.GetLength() == 0)
    {
        // if an empty string is being searched - return zero.
        fn.Result->SetNumber(0);
        return;
    }
    const char* search = searchAddRef.ToCStr();
    // Ok since GetString() returns ref to living object
    const char* str = pThis->GetString().ToCStr();

    int start = 0;
    if (fn.NArgs > 1)
    {
        start = (int) fn.Arg(1).ToNumber(fn.Env);
    }

    UInt32 chr;
    UInt32 first = UTF8Util::DecodeNextChar(&search);

    for(int i = 0; 
        (chr = UTF8Util::DecodeNextChar(&str)) != 0;
        i++)
    {
        if(i >= start && chr == first)
        {
            const char* s1 = str;
            const char* s2 = search;
            UInt32 c1, c2=0;
            for(;;)
            {
                c1 = UTF8Util::DecodeNextChar(&s1);
                c2 = UTF8Util::DecodeNextChar(&s2);

                if(c1 == 0 || c2 == 0 || c1 != c2) 
                    break;
            }
            if(c2 == 0)
            {
                fn.Result->SetInt(i);
                return;
            }
            if(c1 == 0)
            {
                fn.Result->SetInt(-1);
                return;
            }
        }
    }
    fn.Result->SetNumber(-1);
    return;
}

void StringProto::StringLastIndexOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs < 1)
    {
        fn.Result->SetNumber(-1);
        return;
    }

    ASString asStr(pThis->GetString());
    ASString searchAddRef(fn.Arg(0).ToString(fn.Env));
    if (searchAddRef.GetLength() == 0)
    {
        // if an empty string is being searched - return the length of the
        // "this" string.
        fn.Result->SetNumber(Number(asStr.GetLength()));
        return;
    }
    const char* str = asStr.ToCStr();

    const char* search = searchAddRef.ToCStr();
    int start = 0x7FFFFFF;
    if (fn.NArgs > 1)
    {
        start = (int) fn.Arg(1).ToNumber(fn.Env);
    }

    UInt32 chr;
    UInt32 first = UTF8Util::DecodeNextChar(&search);
    int lastIndex = -1;

    for(int i = 0; 
        (chr = UTF8Util::DecodeNextChar(&str)) != 0;
        i++)
    {
        if(i <= start && chr == first)
        {
            const char* s1 = str;
            const char* s2 = search;
            UInt32 c1, c2=0;
            for(;;)
            {
                c1 = UTF8Util::DecodeNextChar(&s1);
                c2 = UTF8Util::DecodeNextChar(&s2);

                if(c1 == 0 || c2 == 0 || c1 != c2) 
                    break;
            }
            if(c2 == 0)
            {
                lastIndex = i;
            }
            if(c1 == 0)
            {
                break;
            }
        }
    }
    fn.Result->SetInt(lastIndex);
    return;
}

void StringProto::StringSlice(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    int start  = 0;
    int length = -1;
        
    if (fn.NArgs >= 1)
    {
        start = (int) fn.Arg(0).ToNumber(fn.Env);
        if (start < 0)
            start += pThis->GetString().GetLength();
    }
    if (fn.NArgs >= 2)
    {
        int end = (int) fn.Arg(1).ToNumber(fn.Env);
        if (end < 0)
            end += pThis->GetString().GetLength();
        if (end < start) 
        {   // empty string
            fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_empty_));
            return;
        }
        length = end - start;
    }    
    fn.Result->SetString(StringSubstring(pThis->GetString(), start, length));
}

void StringProto::StringSplit(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    ASString   search(fn.Env->GetBuiltin(ASBuiltin_empty_));    
    const char* psearch = 0;
    
    if (fn.NArgs >= 1)
    {        
        search = fn.Arg(0).ToString(fn.Env);
        psearch = search.ToCStr();
    }

    int limit = 0x3FFFFFFF;
    if (fn.NArgs >= 2)
    {
        limit = (int)fn.Arg(1).ToNumber(fn.Env);
        if(limit < 0) limit = 0;
    }

    Ptr<ArrayObject> retVal = StringSplit(fn.Env, pThis->GetString(), psearch, limit);
    fn.Result->SetAsObject(retVal.GetPtr());
}

Ptr<ArrayObject> StringProto::StringSplit(Environment* penv, const ASString& str, const char* delimiters, int limit)
{
    Ptr<ArrayObject> retVal = *static_cast<ArrayObject*>
                                  (penv->OperatorNew(penv->GetBuiltin(ASBuiltin_Array)));

    const char*         search = delimiters;
    ASStringContext*   psc = penv->GetSC();    
    const char*         pstr = str.ToCStr();
    
    if(limit < 0) limit = 0;

    UInt32 c1, c2;
    
    if (search == 0)
    {
        retVal->PushBack(Value(str));
    }
    else if (*search == 0)
    {
        String tmpStr;

        while((c1 = UTF8Util::DecodeNextChar(&pstr)) != 0)
        {
            tmpStr.Clear();
            tmpStr.AppendChar(c1);
            retVal->PushBack(Value(psc->CreateString(tmpStr)));
        }
    }
    else
    {
        int count = 0;
        const char* start = pstr;

        for(;;)
        {
            const char* s2   = search;
            const char* end  = pstr;
            const char* prev = pstr;
            const char* pnext = NULL;

            for(;;)
            {
                prev = pstr;
                c1 = UTF8Util::DecodeNextChar(&pstr);
                c2 = UTF8Util::DecodeNextChar(&s2);
                if (!pnext)
                    pnext = pstr;
                if (c1 == 0 || c2 == 0) 
                {
                    break;
                }
                if (c1 != c2)
                {
                    pstr = pnext;
                    break;
                }
            }
            if (c2 == 0)
            {
                if (count >= limit) break;
                retVal->PushBack(Value(CreateStringFromCStr(psc, start, end)));
                pstr = start = prev;
                ++count;
            }
            if (c1 == 0)
            {
                if (count < limit)
                {                
                    if (c2 == 0)
                        retVal->PushBack(Value(CreateStringFromCStr(psc, start, end)));
                    else
                        retVal->PushBack(Value(CreateStringFromCStr(psc, start)));
                }
                break;
            }
        }
    }
    return retVal;
}

// This function reproduces the behavior of 
// function "substring" (not method!) except
// for that it's zero-based instead of one-based
//
// The rules:
// start:  zero-based, 
//         if < 0 then start = 0
//         if >= strlen then return empty
//
// length: if = 0 then return empty
//         if < 0    then return tail
//         if > tail then return tail
ASString StringProto::StringSubstring(const ASString& self, int start, int length)
{
    if(length == 0)    
        return self.GetManager()->CreateEmptyString();

    if (start < 0)
        start = 0;
    
    int utf8Len = self.GetLength();
    if (start >= utf8Len)    
        return self.GetManager()->CreateEmptyString();

    if (length < 0 || start + length > utf8Len)
    {
        length = utf8Len - start;
    }

    return self.Substring(start, start + length);
}

ASString StringProto::CreateStringFromCStr(ASStringContext* psc, const char* start, const char* end)
{
    int len = (end == 0) ? (int)strlen(start) : int(end - start);    
    if (len < 0)
        len = 0;    
    if (len > 0)
        return psc->CreateString(start, len);
    return psc->GetBuiltin(ASBuiltin_empty_);
}


void StringProto::StringSubstr(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    int start  = 0;
    int length = -1;
    if (fn.NArgs >= 1)
    {
        start = (int) fn.Arg(0).ToNumber(fn.Env);
        if(start < 0) start += pThis->GetString().GetLength();
    }
    if (fn.NArgs >= 2)
    {
        length = (int) fn.Arg(1).ToNumber(fn.Env);
        if (length < 0) 
        {
            // The behavior of method "substr" differs
            // from function "substring". 
            // In "substring" if length is negative it 
            // returns the tail, while in method it returns 
            // an empty string.
            length = 0;
        }
    }    
    fn.Result->SetString(StringSubstring(pThis->GetString(), start, length));
}

void StringProto::StringSubstring(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    
    int start  = 0;
    int length = -1;

    if (fn.NArgs >= 1)
    {
        start = (int) fn.Arg(0).ToNumber(fn.Env);
    }
    if (fn.NArgs >= 2)
    {
        int end = (int) fn.Arg(1).ToNumber(fn.Env);
        if (end < start) 
        {
            // This code reproduces the behavior of the
            // original Flash player. The docs say:
            //-------
            // "If the value of start is greater than the value 
            //  of end, the parameters are automatically swapped 
            //  before the function executes"
            //-------
            // But it's more complex than that: 
            // If 'start' is out of range it always returns 
            // an empty string, no matter if the actual 
            // start-end range intersects with 0...length-1.
            // In this case it calls GetUTF8Length() twice, 
            // but it's OK because the end < start case is pretty
            // rear (almost never happens).
            int utf8Len = pThis->GetString().GetLength();
            if(start >= utf8Len)
            {
                fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_empty_));
                return;

            }
            Alg::Swap(start, end);
        }
        if (start < 0)
            start = 0;
        length = end - start;
    }

    fn.Result->SetString(StringSubstring(pThis->GetString(), start, length));
}

void StringProto::StringToLowerCase(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    fn.Result->SetString(pThis->GetString().ToLower());
}

void StringProto::StringToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    fn.Result->SetString(pThis->GetString());
}

void StringProto::StringToUpperCase(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);    
    fn.Result->SetString(pThis->GetString().ToUpper());    
}

void StringProto::StringValueOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    fn.Result->SetString(pThis->GetString());
}

// extension function. Compares strings according to locale.
// 1st parameter is the string to compare with.
// 2nd optional parameter is boolean value; set to 'true' if need
// case-insensitive comparison. By default, comparison is case-sensitive.
void StringProto::StringLocaleCompare(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (!fn.Env->CheckExtensions())
        return;

    CHECK_THIS_PTR(fn, String);
    StringObject* pThis = (StringObject*) fn.ThisPtr;
    SF_ASSERT(pThis);
    if (fn.NArgs < 1)
        return;

    ASString str = fn.Arg(0).ToString(fn.Env);
    bool caseSensitive = true;
    if (fn.NArgs >= 2)
        caseSensitive = !fn.Arg(1).ToBool(fn.Env);
    fn.Result->SetNumber(pThis->GetString().LocaleCompare_CaseCheck(str, caseSensitive));
}

static const NameFunction GAS_StringFunctionTable[] = 
{
    { "charAt",       &StringProto::StringCharAt       },
    { "charCodeAt",   &StringProto::StringCharCodeAt   },
    { "concat",       &StringProto::StringConcat       },
    { "indexOf",      &StringProto::StringIndexOf      },
    { "lastIndexOf",  &StringProto::StringLastIndexOf  },
    { "localeCompare",&StringProto::StringLocaleCompare}, //extension
    { "slice",        &StringProto::StringSlice        },
    { "split",        &StringProto::StringSplit        },
    { "substr",       &StringProto::StringSubstr       },
    { "substring",    &StringProto::StringSubstring    },
    { "toLowerCase",  &StringProto::StringToLowerCase  },
    { "toString",     &StringProto::StringToString     },
    { "toUpperCase",  &StringProto::StringToUpperCase  },
    { "valueOf",      &StringProto::StringValueOf      },
    { 0, 0 }
};

StringProto::StringProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<StringObject>(psc, pprototype, constructor)
{
    sValue = psc->GetBuiltin(ASBuiltin_typeObject_);
    InitFunctionMembers(psc, GAS_StringFunctionTable);
}

//////////////////
const NameFunction StringCtorFunction::StaticFunctionTable[] = 
{
    { "fromCharCode", &StringCtorFunction::StringFromCharCode },
    { 0, 0 }
};

StringCtorFunction::StringCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void StringCtorFunction::StringFromCharCode(const FnCall& fn)
{
    // Takes a variable number of args.  Each arg
    // is a numeric character code.  Construct The
    // string from the character codes.

    StringBuffer result;

    for (int i = 0; i < fn.NArgs; i++)
    {
        UInt32 c = (UInt32) fn.Arg(i).ToNumber(fn.Env);
        result.AppendChar(c);
    }
    fn.Result->SetString(fn.Env->CreateString(result.ToCStr(),result.GetSize()));
}

Object* StringCtorFunction::CreateNewObject(Environment *penv) const 
{ 
    Object* o = SF_HEAP_NEW(penv->GetHeap()) StringObject(penv); 
    o->SetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_length), Value(Value::UNSET));
    return o;
}

void StringCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_String &&
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        StringObject* nobj = static_cast<StringObject*>(fn.ThisPtr);
        Value retVal = (fn.NArgs > 0)? fn.Arg(0) : Value(fn.Env->GetBuiltin(ASBuiltin_empty_));
        nobj->SetValue(fn.Env, retVal);
        *fn.Result = retVal;
    }
    else
    {
        if (fn.NArgs == 0)
            fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_empty_));
        else
            fn.Result->SetString(fn.Arg(0).ToString(fn.Env));
    }
}

FunctionRef StringCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) StringCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) StringProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_String, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_String), Value(ctor));
    return ctor;
}
}}} // SF::GFx::AS2
