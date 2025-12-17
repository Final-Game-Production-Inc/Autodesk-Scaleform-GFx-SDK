/**************************************************************************

Filename    :   AS2_RectangleObject.h
Content     :   flash.geom.Rectangle reference class for ActionScript 2.0
Created     :   3/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_RECTANGLE_H
#define INC_SF_GFX_RECTANGLE_H

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_RECTANGLE disables Rectangle class
#ifdef GFX_AS2_ENABLE_RECTANGLE

// ***** Declared Classes
class RectangleObject;
class RectangleProto;

// ***** External Classes
class Value;

// Rectangle property indices
enum Rectangle_Property
{
    Rectangle_X,
    Rectangle_Y,
    Rectangle_Width,
    Rectangle_Height,
    Rectangle_NumProperties
};

// internal object
typedef Render::Rect<Number> ASRect;
extern bool IsRectValid(const ASRect& r);

// ****************************************************************************
// GAS Rectangle class
//
class RectangleObject : public Object
{
    friend class RectangleProto;
protected:
    RectangleObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    RectangleObject(Environment* penv);

    virtual ObjectType GetObjectType() const { return Object_Rectangle; }

    // getters and setters
    void GetProperties(ASStringContext *psc, Value params[Rectangle_NumProperties]);
    void SetProperties(ASStringContext* psc, const Value params[Rectangle_NumProperties]); 
    void GetProperties(Environment *penv, ASRect &r);
    void SetProperties(Environment *penv, const ASRect &r);

    virtual bool GetMember(Environment *penv, const ASString &name, Value* val);
    virtual bool SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags = PropFlags());
};

// ****************************************************************************
// GAS Rectangle prototype class
//
class RectangleProto : public Prototype<RectangleObject>
{
public:
    RectangleProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    static void Clone(const FnCall& fn);
    static void Contains(const FnCall& fn);
    static void ContainsPoint(const FnCall& fn);
    static void ContainsRectangle(const FnCall& fn);
    static void Equals(const FnCall& fn);
    static void Inflate(const FnCall& fn);
    static void InflatePoint(const FnCall& fn);
    static void Intersection(const FnCall& fn);
    static void Intersects(const FnCall& fn);
    static void IsEmpty(const FnCall& fn);
    static void Offset(const FnCall& fn);
    static void OffsetPoint(const FnCall& fn);
    static void SetEmpty(const FnCall& fn);
    static void ToString(const FnCall& fn);
    static void Union(const FnCall& fn);
};

class RectangleCtorFunction : public CFunctionObject
{
public:
    RectangleCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;    

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};


#endif  //  SF_NO_FXPLAYER_AS_RECTANGLE

}}} //SF::GFx::AS2
#endif  // INC_SF_GFX_RECTANGLE_H


