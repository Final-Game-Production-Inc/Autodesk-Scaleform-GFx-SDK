/**************************************************************************

Filename    :   AS2_ColorTransform.h
Content     :   flash.geom.Rectangle reference class for ActionScript 2.0
Created     :   3/19/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_COLORTRANSFORM_H
#define INC_SF_GFX_COLORTRANSFORM_H

#include "GFx/GFx_Types.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {


// SF_NO_FXPLAYER_AS_COLORTRANSFORM disables ColorTransform support
#ifdef GFX_AS2_ENABLE_COLORTRANSFORM


// **** Declared Classes
class ColorTransformObject;
class ColorTransformProto;

// **** External Classes
class Value;

// ColorTransform property indices
enum GFxColorTransform_Property
{
    GFxColorTransform_RedMultiplier,
    GFxColorTransform_GreenMultiplier,
    GFxColorTransform_BlueMultiplier,
    GFxColorTransform_AlphaMultiplier,
    GFxColorTransform_RedOffset,
    GFxColorTransform_GreenOffset,
    GFxColorTransform_BlueOffset,
    GFxColorTransform_AlphaOffset,
    GFxColorTransform_NumProperties
};

// ****************************************************************************
// GAS ColorTransform class
//
class ColorTransformObject : public Object
{
    friend class ColorTransformProto;
    friend class ColorTransformCtorFunction;

    Cxform mColorTransform;
protected:
    ColorTransformObject(ASStringContext *psc, Object* pprototype) 
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    ColorTransformObject(Environment* penv);

    virtual ObjectType GetObjectType() const { return Object_ColorTransform; }

    const Cxform GetCxform();
    void         SetCxform(const Cxform &ct);

    // getters and setters
    void         GetProperties(ASStringContext *psc, Value params[GFxColorTransform_NumProperties]);
    void         SetProperties(ASStringContext *psc, const Value params[GFxColorTransform_NumProperties]);

    virtual bool GetMember(Environment *penv, const ASString &name, Value* val);
    virtual bool SetMember(Environment *penv, const ASString &name, const Value &val,
                           const PropFlags& flags = PropFlags());
};

// ****************************************************************************
// GAS ColorTransform prototype class
//
class ColorTransformProto : public Prototype<ColorTransformObject>
{
public:
    ColorTransformProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    static void Concat(const FnCall& fn);
    static void ToString(const FnCall& fn);

};

class ColorTransformCtorFunction : public CFunctionObject
{
public:
    ColorTransformCtorFunction(ASStringContext *psc) 
        : CFunctionObject(psc, GlobalCtor) {}

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

void GFxObject_GetColorTransformProperties(Environment *penv, Object *pobj,
                                           Value params[GFxColorTransform_NumProperties]);

#endif  //  SF_NO_FXPLAYER_AS_COLORTRANSFORM
}}} //SF::GFx::AS2
#endif  // INC_SF_GFX_COLORTRANSFORM_H
