/**************************************************************************

Filename    :   AS2_TransformObject.cpp
Content     :   flash.geom.Transform reference class for ActionScript 2.0
Created     :   6/22/2006
Authors     :   Artyom Bolgar, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_TransformObject.h"
#include "AS2_FunctionRef.h"
#include "GFx/AS2/AS2_MatrixObject.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "GFx/AS2/AS2_ColorTransform.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_TRANSFORM disables Transform class
#ifdef GFX_AS2_ENABLE_TRANSFORM

// ****************************************************************************
// GASTransform  constructor
//
// Why are we storing the movie root and target handle? Flash resolves 
// characters using it's handle (name). The behavior for the following code in
// ActionScript..
// Frame 1 => (Create an instance mc1)
//            var t1:Transform = new Transform(mc1);
//            var m1:Matrix = new Matrix();
//            m2 = new Matrix(0.5, 0, 0, 1.4, 0, 0);
//            m1.concat(m2);
//            t1.matrix = m1;
// Frame 2 => (Delete mc1)
// Frame 3 => (Create another instance mc1)
//            m1 = t1.matrix;
//            m2 = new Matrix(0.5, 0, 0, 1.4, 0, 0);
//            m1.concat(m2);
//            t1.matrix = m1;
//
// ..is that the second mc1 instance will have the transform t1 applied to it.
//
TransformObject::TransformObject(Environment* penv, InteractiveObject *pcharacter)
: Object(penv), pMovieRoot(0)
{
    SetTarget(pcharacter);
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Transform));

#ifdef GFX_AS2_ENABLE_MATRIX
    Matrix = *static_cast<MatrixObject*>
        (penv->OperatorNew(penv->GetGC()->FlashGeomPackage, penv->GetBuiltin(ASBuiltin_Matrix)));
#endif

#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
    pColorTransform = *static_cast<ColorTransformObject*>
        (penv->OperatorNew(penv->GetGC()->FlashGeomPackage, penv->GetBuiltin(ASBuiltin_ColorTransform)));
#endif

#ifdef GFX_AS2_ENABLE_RECTANGLE
    PixelBounds = *static_cast<RectangleObject*>
        (penv->OperatorNew(penv->GetGC()->FlashGeomPackage, penv->GetBuiltin(ASBuiltin_Rectangle)));
#endif
}

#ifdef GFX_AS_ENABLE_GC
template <class Functor>
void TransformObject::ForEachChild_GC(Collector* prcc) const
{
    Object::template ForEachChild_GC<Functor>(prcc);
#ifdef GFX_AS2_ENABLE_MATRIX
    if (Matrix)
        Functor::Call(prcc, Matrix);
#endif
#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
    if (pColorTransform)
        Functor::Call(prcc, pColorTransform);
#endif
#ifdef GFX_AS2_ENABLE_RECTANGLE
    if (PixelBounds)
        Functor::Call(prcc, PixelBounds);
#endif
}
SF_GC_PREGEN_FOR_EACH_CHILD(TransformObject)

void TransformObject::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<TransformObject>(prcc, operation);
}

void TransformObject::Finalize_GC()
{
    pMovieRoot = 0;
    TargetHandle.~Ptr<CharacterHandle>();
    Object::Finalize_GC();
}
#endif //SF_NO_GC

void TransformObject::SetTarget(InteractiveObject *pcharacter)
{
    if (pcharacter)
    {
        TargetHandle    = pcharacter->GetCharacterHandle();
        pMovieRoot      = pcharacter->GetMovieImpl();
    }
    else
    {
        pMovieRoot      = NULL;
        TargetHandle    = NULL;
    }
}

// ****************************************************************************
// Intercept the get command so that dependent properties can be recalculated
//
bool TransformObject::GetMember(Environment *penv, const ASString& name, Value* val)
{
    if (name == "pixelBounds")
    {

#ifdef GFX_AS2_ENABLE_RECTANGLE
        bool rv = true;        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                RectF bounds = ch->GetBounds(ch->GetMatrix());
                // Flash has 0.05 granularity and will cause the values below to
                // be off.
                ASRect r((Number)Alg::IRound(TwipsToPixels(bounds.x1)), (Number)Alg::IRound(TwipsToPixels(bounds.y1)), 
                          Size<Number>((Number)Alg::IRound(TwipsToPixels(bounds.Width())), 
                                           (Number)Alg::IRound(TwipsToPixels(bounds.Height()))));
                PixelBounds->SetProperties(penv, r);
                val->SetAsObject(PixelBounds);
            }
            else 
                rv = false;
        }
        else 
            rv = false;
        if (!rv) val->SetUndefined();
        return rv;
#else
        SF_DEBUG_WARNING(1, "Rectangle ActionScript class was not included in this GFx build."); 
        val->SetUndefined();
        return false;
#endif
    }
    else if (name == "colorTransform")
    {

#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
        bool rv = true;        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                Cxform ct = ch->GetCxform();
                pColorTransform->SetCxform(ct);
                val->SetAsObject(pColorTransform);
            }
            else 
                rv = false;
        }
        else 
            rv = false;
        if (!rv) val->SetUndefined();
        return rv;
#else
        SF_DEBUG_WARNING(1, "ColorTransform ActionScript class was not included in this GFx build."); 
        val->SetUndefined();
        return false;
#endif
    }
    else if (name == "matrix")
    {
#ifdef GFX_AS2_ENABLE_MATRIX
        bool rv = true;        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                Render::Matrix2F m = ch->GetMatrix();
                Matrix->SetMatrixTwips(penv->GetSC(), m);
                val->SetAsObject(Matrix);
            }
            else 
                rv = false;
        }
        else 
            rv = false;
        if (!rv) val->SetUndefined();
        return rv;
#else
        SF_DEBUG_WARNING(1, "Matrix ActionScript class was not included in this GFx build."); 
        val->SetUndefined();
        return false;
#endif
    }
    else if (name == "concatenatedColorTransform")
    {

#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
        // concatenatedColorTransform should be computed on the fly:
        // "A ColorTransform object representing the combined color transformations 
        // applied to this object and all of its parent objects, back to the root 
        // level. If different color transformations have been applied at different 
        // levels, each of those transformations will be concatenated into one ColorTransform 
        // object for this property."        
        Cxform ct;
        // get the ASCharacter for this transform        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                // run through the scene hierarchy and cat the color transforms
                DisplayObject* curr = ch.GetPtr();
                while (curr != NULL)
                {
                    ct.Prepend(curr->GetCxform());
                    curr = curr->GetParent();
                }
            }
        }
        // return a Object
        Ptr<ColorTransformObject> pct = *SF_HEAP_NEW(penv->GetHeap()) ColorTransformObject(penv);
        pct->SetCxform(ct);
        *val = Value(pct.GetPtr());
        return true;
#else
        SF_DEBUG_WARNING(1, "ColorTransform ActionScript class was not included in this GFx build."); 
        val->SetUndefined();
        return false;    
#endif
    }
    else if (name == "concatenatedMatrix")
    {

#ifdef GFX_AS2_ENABLE_MATRIX
        // concatenatedMatrix should be computed on the fly:
        // "A Matrix object representing the combined transformation matrices of this 
        // object and all of its parent objects, back to the root level. If different 
        // transformation matrices have been applied at different levels, each of those 
        // matrices will be concatenated into one matrix for this property."
        Render::Matrix2F mt;
        // get the ASCharacter for this transform        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                // run through the scene hierarchy and cat the color transforms
                DisplayObject* curr = ch.GetPtr();
                while (curr != NULL)
                {
                    mt.Prepend(curr->GetMatrix());  // or Append?
                    curr = curr->GetParent();
                }
            }
        }
        // return a Object
        Ptr<MatrixObject> pmt = *SF_HEAP_NEW(penv->GetHeap()) MatrixObject(penv);
        pmt->SetMatrixTwips(penv->GetSC(), mt);
        *val = Value(pmt.GetPtr());
        return true;
#else
        SF_DEBUG_WARNING(1, "Matrix ActionScript class was not included in this GFx build."); 
        val->SetUndefined();
        return false;  
#endif
    }
    return Object::GetMember(penv, name, val);
}

// ****************************************************************************
// Intercept the set command so that dependent properties can be recalculated
//
bool    TransformObject::SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    if (name == "pixelBounds")
    {
        // Does nothing
        return true;
    }
    else if (name == "colorTransform")
    {

#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                // get ColorTransform from val
                Ptr<Object> obj = val.ToObject(penv);
                if (obj && obj->GetObjectType() == Object::Object_ColorTransform)
                {
                    ColorTransformObject* ctobj = (ColorTransformObject*)obj.GetPtr();
                    if (ctobj)
                    {
                        Cxform ct = ctobj->GetCxform();
                        ch->SetCxform(ct);
                        ch->SetAcceptAnimMoves(false);
                    }
                }
            }
        }
        return true;
#else
        SF_DEBUG_WARNING(1, "ColorTransform ActionScript class was not included in this GFx build."); 
        return false;
#endif
    }
    else if (name == "matrix")
    {

#ifdef GFX_AS2_ENABLE_MATRIX        
        if (pMovieRoot)
        {
            Ptr<DisplayObject> ch = TargetHandle->ResolveCharacter(pMovieRoot);
            if (ch)
            {
                // get Render::Matrix2F from val
                Ptr<Object> obj = val.ToObject(penv);
                if (obj && obj->GetObjectType() == Object::Object_Matrix)
                {
                    MatrixObject* mobj = (MatrixObject*)obj.GetPtr();
                    if (mobj)
                    {
                        Render::Matrix2F m = mobj->GetMatrix(penv);
                        m.Tx() = PixelsToTwips(m.Tx());
                        m.Ty() = PixelsToTwips(m.Ty());
                        ch->SetMatrix(m);

                        DisplayObject::GeomDataType geomData;
                        ch->GetGeomData(geomData);
                        geomData.X          = (int)m.Tx();
                        geomData.Y          = (int)m.Ty();
                        geomData.Rotation   = m.GetRotationDouble()*180/SF_MATH_PI;
                        geomData.XScale     = m.GetXScaleDouble()*100;
                        geomData.YScale     = m.GetYScaleDouble()*100;
                        ch->SetGeomData(geomData);
                    }
                }
            }
        }
        return true;
#else
        SF_DEBUG_WARNING(1, "Matrix ActionScript class was not included in this GFx build."); 
        return false;
#endif
    }
    return Object::SetMember(penv, name, val, flags);
}

// ****************************************************************************
// AS to GFx function mapping
//
const NameFunction TransformProto::FunctionTable[] = 
{
    // no functions
    { 0, 0 }
};

// ****************************************************************************
// GASTransform Prototype constructor
//
TransformProto::TransformProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<TransformObject>(psc, pprototype, constructor)
{
    // we make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    // add the on-the-fly calculated properties
    Value undef;
    TransformObject::SetMemberRaw(psc, psc->CreateConstString("matrix"), undef, PropFlags::PropFlag_DontDelete);
    TransformObject::SetMemberRaw(psc, psc->CreateConstString("concatenatedMatrix"), undef, PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
    TransformObject::SetMemberRaw(psc, psc->CreateConstString("colorTransform"), undef, PropFlags::PropFlag_DontDelete);
    TransformObject::SetMemberRaw(psc, psc->CreateConstString("concatenatedColorTransform"), undef, PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
    TransformObject::SetMemberRaw(psc, psc->CreateConstString("pixelBounds"), undef, PropFlags::PropFlag_DontDelete);
    //InitFunctionMembers(psc, FunctionTable, pprototype);
}

// ****************************************************************************
// Called when the constructor is invoked for the Transform class
//
void TransformCtorFunction::GlobalCtor(const FnCall& fn)
{
    InteractiveObject* ptarget = NULL;
    if (fn.NArgs >= 1)
    {
        ptarget = fn.Env->FindTargetByValue(fn.Arg(0));
        if (NULL == ptarget)
        {
            fn.Result->SetUndefined();
        }
        else
        {
            Ptr<TransformObject> ptransform;
            if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Transform && 
                !fn.ThisPtr->IsBuiltinPrototype())
                ptransform = static_cast<TransformObject*>(fn.ThisPtr);
            else
                ptransform = *SF_HEAP_NEW(fn.Env->GetHeap()) TransformObject(fn.Env);
            ptransform->SetTarget(ptarget);
            fn.Result->SetAsObject(ptransform.GetPtr());
        }
    }
}

Object* TransformCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) TransformObject(penv);
}

FunctionRef TransformCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) TransformCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) TransformProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Transform, proto);
    pgc->FlashGeomPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Transform), Value(ctor));
    return ctor;
}

#else

void GASTransform_DummyFunction() {}   // Exists to quelch compiler warning

#endif  //  SF_NO_FXPLAYER_AS_TRANSFORM

}}} //SF::GFx::AS2
