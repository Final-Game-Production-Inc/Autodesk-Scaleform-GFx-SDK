/**************************************************************************

Filename    :   AS2_StringObject.h
Content     :   Implementation of AS2 String class
Created     :   
Authors     :   Maxim Shemanarev, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_STRINGOBJECT_H
#define INC_SF_GFX_STRINGOBJECT_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class StringObject;
class StringProto;
class StringCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class StringObject : public Object
{
protected:
    ASString sValue;

    void commonInit (Environment* penv);
    
    StringObject(ASStringContext *psc, Object* pprototype) :
        Object(psc), sValue(psc->GetBuiltin(ASBuiltin_empty_))
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        sValue.~ASString();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    StringObject(Environment* penv);
    StringObject(Environment* penv, const ASString& val);

    bool                GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val);
    const char*         GetTextValue(Environment* =0) const;
    ObjectType          GetObjectType() const   { return Object_String; }

    const ASString&     GetString() const               { return sValue; }
    void                SetString(const ASString& val) { sValue = val; }

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value       GetValue() const;
};

class StringProto : public Prototype<StringObject>
{
public:
    StringProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void GlobalCtor(const FnCall& fn);

    static void StringCharAt(const FnCall& fn);
    static void StringCharCodeAt(const FnCall& fn);
    static void StringConcat(const FnCall& fn);
    static void StringIndexOf(const FnCall& fn);
    static void StringLastIndexOf(const FnCall& fn);
    static void StringLocaleCompare(const FnCall& fn);
    static void StringSlice(const FnCall& fn);
    static void StringSplit(const FnCall& fn);        
    static void StringSubstr(const FnCall& fn);
    static void StringSubstring(const FnCall& fn);
    static void StringToLowerCase(const FnCall& fn);
    static void StringToString(const FnCall& fn);
    static void StringToUpperCase(const FnCall& fn);
    static void StringValueOf(const FnCall& fn);

    // Helper methods.
    static ASString StringSubstring(const ASString& str, 
                                     int start, int length);
    static Ptr<ArrayObject> StringSplit(Environment* penv, const ASString& str,
                                            const char* delimiters, int limit = 0x3FFFFFFF);

    // Creates a ASString based on two char* pointers
    static ASString CreateStringFromCStr(ASStringContext* psc, const char* start, const char* end = 0);
};

class StringCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void StringFromCharCode(const FnCall& fn);
public:
    StringCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment *penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_STRINGOBJECT_H
