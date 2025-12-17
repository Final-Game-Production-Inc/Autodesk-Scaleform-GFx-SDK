/**************************************************************************

Filename    :   AS2_BooleanObject.h
Content     :   Boolean object functinality
Created     :   April 10, 2006
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_BOOLEAN_H
#define INC_SF_GFX_BOOLEAN_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class BooleanObject;
class BooleanProto;
class BooleanCtorFunction;


class BooleanObject : public Object
{
protected:
    bool bValue;
    
    void CommonInit (Environment* penv);

    BooleanObject(ASStringContext* psc, Object* pprototype) : 
        Object(psc), bValue (0) 
    { 
        Set__proto__(psc, pprototype);
    }
public:
    BooleanObject(Environment* penv);
    BooleanObject(Environment* penv, bool val);
    virtual const char* GetTextValue(Environment* penv = 0) const;

    ObjectType      GetObjectType() const   { return Object_Boolean; }

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value    GetValue() const;

    //
    // This method is used to invoke some methods for primitive number,
    // like it is possible to invoke toString (radix) or valueOf even for non-object Boolean.
    //
    static bool InvokePrimitiveMethod(const FnCall& fnCall, const ASString& methodName);
};

class BooleanProto : public Prototype<BooleanObject>
{
public:
    BooleanProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);
};

class BooleanCtorFunction : public CFunctionObject
{
public:
    BooleanCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;        

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2
#endif //BOOLEAN

