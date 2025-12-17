/**************************************************************************

Filename    :   AS2_TransformObject.h
Content     :   flash.geom.Transform reference class for ActionScript 2.0
Created     :   6/22/2006
Authors     :   Artyom Bolgar, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_TRANSFORM_H
#define INC_SF_GFX_TRANSFORM_H

#include "Kernel/SF_RefCount.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_MatrixObject.h"
#include "GFx/AS2/AS2_ColorTransform.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_TRANSFORM disables Transform class
#ifdef GFX_AS2_ENABLE_TRANSFORM

// ***** Declared Classes
class TransformObject;
class TransformProto;
class Environment;

// ****************************************************************************
// GAS Transform class
//
class TransformObject : public Object
{
    friend class TransformProto;
    friend class TransformCtorFunction;

    MovieImpl*                  pMovieRoot;
    Ptr<GFx::CharacterHandle>   TargetHandle;

#ifdef GFX_AS2_ENABLE_MATRIX
    Ptr<MatrixObject>           Matrix;
#endif

#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
    Ptr<ColorTransformObject>   pColorTransform;
#endif 

#ifdef GFX_AS2_ENABLE_RECTANGLE
    Ptr<RectangleObject>        PixelBounds;
#endif 

protected:
    TransformObject(ASStringContext *psc, Object* pprototype)
        : Object(psc), pMovieRoot(0)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor>
    void ForEachChild_GC(Collector* prcc) const;
    virtual void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;
    virtual void Finalize_GC();
#endif //SF_NO_GC
public:
    TransformObject(Environment* penv, InteractiveObject *pcharacter = NULL);

    virtual ObjectType GetObjectType() const   { return Object_Transform; }

    // getters and setters
    virtual bool GetMember(Environment *penv, const ASString &name, Value* val);
    virtual bool SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags = PropFlags());

    void SetTarget(InteractiveObject *pcharacter);
};

// ****************************************************************************
// GAS Transform prototype class
//
class TransformProto : public Prototype<TransformObject>
{
public:
    TransformProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    static void GlobalCtor(const FnCall& fn);

    static const NameFunction FunctionTable[];
};

class TransformCtorFunction : public CFunctionObject
{
public:
    TransformCtorFunction(ASStringContext *psc) 
        : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;    

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};


#endif // SF_NO_FXPLAYER_AS_TRANSFORM

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_TRANSFORM_H

