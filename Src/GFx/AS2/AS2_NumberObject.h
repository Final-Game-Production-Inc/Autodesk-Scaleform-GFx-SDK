/**************************************************************************

Filename    :   AS2_NumberObject.h
Content     :   Number object functinality
Created     :   March 10, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_NUMBER_H
#define INC_SF_GFX_NUMBER_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class NumberObject;
class NumberProto;
class NumberCtorFunction;

class NumberObject : public Object
{
protected:
    Number              mValue;
    mutable StringLH    StringValue;

    void CommonInit(Environment* penv);

    NumberObject(ASStringContext *psc, Object* pprototype) 
        : Object(psc), mValue(0)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        StringValue.~StringLH();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    NumberObject(Environment* penv);
    NumberObject(Environment* penv, Number val);

    virtual const char* GetTextValue(Environment* penv = 0) const;
    ObjectType          GetObjectType() const   { return Object_Number; }

    const char* ToString (int radix) const;

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value       GetValue() const;

    //
    // This method is used to invoke some methods for primitive number,
    // like it is possible to invoke toString (radix) or valueOf even for non-object Number.
    //
    static bool InvokePrimitiveMethod (const FnCall& fnCall, const ASString& methodName);
};

class NumberProto : public Prototype<NumberObject>
{
public:
    NumberProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);
};

class NumberCtorFunction : public CFunctionObject
{
public:
    NumberCtorFunction (ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif

