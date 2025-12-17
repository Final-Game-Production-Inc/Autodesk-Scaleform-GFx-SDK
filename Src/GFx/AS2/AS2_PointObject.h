/**************************************************************************

Filename    :   AS2_PointObject.h
Content     :   flash.geom.Point reference class for ActionScript 2.0
Created     :   3/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_POINT_H
#define INC_SF_GFX_POINT_H

// SF_NO_FXPLAYER_AS_POINT disables Point class
#ifdef GFX_AS2_ENABLE_POINT

#include "GFx/AS2/AS2_Object.h"

namespace Scaleform { namespace GFx { namespace AS2 {


// ***** Declared Classes
class PointObject;
class PointProto;
class PointCtorFunction;

// Point property indices
enum Point_Property 
{
    Point_X, 
    Point_Y, 
    Point_NumProperties
};

// internal object
typedef Point<Number> GASPoint;

// loader functions
void GFxObject_GetPointProperties(Environment *penv, Object *pobj, Value params[Point_NumProperties]);
void GFxObject_GetPointProperties(Environment *penv, Object *pobj, GASPoint &pt);

// ****************************************************************************
// GAS Point class
// 
class PointObject : public Object
{
    friend class PointProto;
protected:
    PointObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    PointObject(Environment* penv);
    PointObject(Environment* penv, PointObject* po);

    virtual ObjectType GetObjectType() const { return Object_Point; }

    // getters and setters
    void GetProperties(ASStringContext *psc, Value params[Point_NumProperties]);
    void SetProperties(ASStringContext *psc, const Value params[Point_NumProperties]); 
    void GetProperties(Environment *penv, GASPoint &pt);
    void SetProperties(Environment *penv, const GASPoint &pt);

    virtual bool GetMember(Environment* penv, const ASString& name, Value* val);
};

// ****************************************************************************
// GAS Point prototype class
//
class PointProto : public Prototype<PointObject>
{
public:
    PointProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    static void Add(const FnCall& fn);
    static void Clone(const FnCall& fn);
    static void Equals(const FnCall& fn);
    static void Normalize(const FnCall& fn);
    static void Offset(const FnCall& fn);
    static void Subtract(const FnCall& fn);
    static void ToString(const FnCall& fn);
};

// ****************************************************************************
// GAS Point Constructor function class
//
class PointCtorFunction : public CFunctionObject
{
protected:
    static const NameFunction StaticFunctionTable[];

    static void Distance(const FnCall& fn);
    static void Interpolate(const FnCall& fn);
    static void Polar(const FnCall& fn); 

public:
    PointCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);

    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

void GFxObject_GetPointProperties(Environment *penv, Object *pobj, Value params[Point_NumProperties]);

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_POINT
#endif  // INC_SF_GFX_POINT_H
