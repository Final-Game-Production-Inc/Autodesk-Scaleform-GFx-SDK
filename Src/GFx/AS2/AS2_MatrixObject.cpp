/**************************************************************************

Filename    :   AS2_MatrixObject.cpp
Content     :   Matrix class implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_MatrixObject.h"
#include "AS2_FunctionRef.h"
#include "AS2_Value.h"
#include "GFx/AS2/AS2_PointObject.h"

#include "GFx/AS2/AS2_Action.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_MATRIX disables Matrix support
#ifdef GFX_AS2_ENABLE_MATRIX


MatrixObject::MatrixObject(Environment* penv)
: Object(penv)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Matrix));
    SetMatrix(penv, Render::Matrix2F());
}

MatrixObject::GASMatrixArray* MatrixObject::GetMatrixAsValuesArray(ASStringContext *psc, MatrixObject::GASMatrixArray* marr)
{
    // a  = Sx         (_M[0][0])
    // b  = Sky        (_M[1][0])
    // c  = Skx        (_M[0][1])
    // d  = Sy         (_M[1][1])
    // tx = Tx         (_M[0][2])
    // ty = Ty         (_M[1][2])
    if (!GetConstMemberRaw(psc, "a",  &(*marr)[0])) (*marr)[0] = 1;
    if (!GetConstMemberRaw(psc, "b",  &(*marr)[1])) (*marr)[1] = 0;
    if (!GetConstMemberRaw(psc, "c",  &(*marr)[2])) (*marr)[2] = 0;
    if (!GetConstMemberRaw(psc, "d",  &(*marr)[3])) (*marr)[3] = 1;
    if (!GetConstMemberRaw(psc, "tx", &(*marr)[4])) (*marr)[4] = 0;
    if (!GetConstMemberRaw(psc, "ty", &(*marr)[5])) (*marr)[5] = 0;
    return marr;
}


Render::Matrix2F MatrixObject::GetMatrix(Environment *penv)
{
    // a  = Sx         (_M[0][0])
    // b  = Sky        (_M[1][0])
    // c  = Skx        (_M[0][1])
    // d  = Sy         (_M[1][1])
    // tx = Tx         (_M[0][2])
    // ty = Ty         (_M[1][2])
    Render::Matrix2F m;
    Value v;
    ASStringContext *psc = penv->GetSC();
    if (GetConstMemberRaw(psc, "a",  &v)) m.Sx() = (float)v.ToNumber(penv); else m.Sx() = 1;
    if (GetConstMemberRaw(psc, "b",  &v)) m.Shy() = (float)v.ToNumber(penv); else m.Shy() = 0;
    if (GetConstMemberRaw(psc, "c",  &v)) m.Shx() = (float)v.ToNumber(penv); else m.Shx() = 0;
    if (GetConstMemberRaw(psc, "d",  &v)) m.Sy() = (float)v.ToNumber(penv); else m.Sy() = 1;
    if (GetConstMemberRaw(psc, "tx", &v)) m.Tx() = (float)v.ToNumber(penv); else m.Tx() = 0;
    if (GetConstMemberRaw(psc, "ty", &v)) m.Ty() = (float)v.ToNumber(penv); else m.Ty() = 0;
    return m;
}

void MatrixObject::SetMatrix(Environment* penv, const Render::Matrix2F& m)
{
    // a  = Sx         (_M[0][0])
    // b  = Sky        (_M[1][0])
    // c  = Skx        (_M[0][1])
    // d  = Sy         (_M[1][1])
    // tx = Tx         (_M[0][2])
    // ty = Ty         (_M[1][2])
    ASStringContext *psc = penv->GetSC();
    SetConstMemberRaw(psc, "a",  Value(m.Sx()));
    SetConstMemberRaw(psc, "b",  Value(m.Shy()));
    SetConstMemberRaw(psc, "c",  Value(m.Shx()));
    SetConstMemberRaw(psc, "d",  Value(m.Sy()));
    SetConstMemberRaw(psc, "tx", Value(m.Tx()));
    SetConstMemberRaw(psc, "ty", Value(m.Ty()));
}

void MatrixObject::SetMatrixTwips(ASStringContext* psc, const Render::Matrix2F& m)
{
    // a  = Sx         (_M[0][0])
    // b  = Sky        (_M[1][0])
    // c  = Skx        (_M[0][1])
    // d  = Sy         (_M[1][1])
    // tx = Tx         (_M[0][2])
    // ty = Ty         (_M[1][2])
    SetConstMemberRaw(psc, "a",  Value(m.Sx()));
    SetConstMemberRaw(psc, "b",  Value(m.Shy()));
    SetConstMemberRaw(psc, "c",  Value(m.Shx()));
    SetConstMemberRaw(psc, "d",  Value(m.Sy()));
    SetConstMemberRaw(psc, "tx", Value(TwipsToPixels(Double(m.Tx()))));
    SetConstMemberRaw(psc, "ty", Value(TwipsToPixels(Double(m.Ty()))));
}

void MatrixProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;     
    SF_ASSERT(pthis);
    if (!pthis) return;

    GASMatrixArray matrix;
    pthis->GetMatrixAsValuesArray(fn.Env->GetSC(), &matrix);
    
    // AddRef strings since we can't call ToCStr() on a temp primitive.
    ASString mxs[6] =
    {
        matrix[0].ToPrimitive(fn.Env).ToString(fn.Env, 6),
        matrix[1].ToPrimitive(fn.Env).ToString(fn.Env, 6),
        matrix[2].ToPrimitive(fn.Env).ToString(fn.Env, 6),
        matrix[3].ToPrimitive(fn.Env).ToString(fn.Env, 6),
        matrix[4].ToPrimitive(fn.Env).ToString(fn.Env, 6),
        matrix[5].ToPrimitive(fn.Env).ToString(fn.Env, 6)
    };

    String str;
    str += "(a=";
    str += mxs[0].ToCStr();
    str += ", b=";
    str += mxs[1].ToCStr();
    str += ", c=";
    str += mxs[2].ToCStr();
    str += ", d=";
    str += mxs[3].ToCStr();
    str += ", tx=";
    str += mxs[4].ToCStr();
    str += ", ty=";
    str += mxs[5].ToCStr();
    str += ")";

    fn.Result->SetString(fn.Env->CreateString(str));
}

void MatrixProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Matrix);
    // clone() : Matrix
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) 
    {
        fn.Result->SetUndefined();
        return;
    }

    Ptr<MatrixObject> m = *SF_HEAP_NEW(fn.Env->GetHeap()) MatrixObject(fn.Env);

    m->SetMatrix(fn.Env, pthis->GetMatrix(fn.Env));
    fn.Result->SetAsObject(m.GetPtr());
}

void MatrixProto::Concat(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Matrix);
    // concat(m:Matrix) : Void
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 0)
    {
        Object* arg = fn.Arg(0).ToObject(fn.Env);
        if(arg->GetObjectType() == Object_Matrix)
        {
            Render::Matrix2F m = pthis->GetMatrix(fn.Env);
            m.Append(((MatrixObject*)arg)->GetMatrix(fn.Env));
            pthis->SetMatrix(fn.Env, m);
        }
    }
}

void MatrixProto::CreateBox(const FnCall& fn)
{
    // createBox(scaleX:Number, scaleY:Number, [rotation:Number], [tx:Number], [ty:Number]) : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 1)
    {
        Render::Matrix2F m;
        float sx  = (float)(fn.Arg(0).ToNumber(fn.Env));
        float sy  = (float)(fn.Arg(1).ToNumber(fn.Env));
        float r   = 0;
        float tx  = 0;
        float ty  = 0;
        if (fn.NArgs > 2)
        {
            r = (float)(fn.Arg(2).ToNumber(fn.Env));
            if (fn.NArgs > 3)
            {
                tx = (float)(fn.Arg(3).ToNumber(fn.Env));
                if (fn.NArgs > 4)
                {
                    ty = (float)(fn.Arg(4).ToNumber(fn.Env));
                }
            }
        }

        m.AppendRotation(r);
        m.AppendScaling(sx, sy);
        m.AppendTranslation(tx, ty);

        pthis->SetMatrix(fn.Env, m);
    }
}

void MatrixProto::CreateGradientBox(const FnCall& fn)
{
    // createGradientBox(width:Number, height:Number, [rotation:Number], [tx:Number], [ty:Number]) : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 1)
    {
        Render::Matrix2F m;
        float w  = (float)(fn.Arg(0).ToNumber(fn.Env));
        float h  = (float)(fn.Arg(1).ToNumber(fn.Env));
        float r  = 0;
        float tx = w / 2;
        float ty = h / 2;
        if (fn.NArgs > 2)
        {
            r = (float)(fn.Arg(2).ToNumber(fn.Env));
            if (fn.NArgs > 3)
            {
                tx += (float)(fn.Arg(3).ToNumber(fn.Env));
                if (fn.NArgs > 4)
                {
                    ty += (float)(fn.Arg(4).ToNumber(fn.Env));
                }
            }
        }

        w *= GASGradientBoxMagicNumber; 
        h *= GASGradientBoxMagicNumber;

        m.AppendRotation(r);
        m.AppendScaling(w, h);
        m.AppendTranslation(tx, ty);

        pthis->SetMatrix(fn.Env, m);
    }
}

void MatrixProto::Identity(const FnCall& fn)
{
    // identity() : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    pthis->SetMatrix(fn.Env, Render::Matrix2F());
}

void MatrixProto::Invert(const FnCall& fn)
{
    // invert() : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    pthis->SetMatrix(fn.Env, pthis->GetMatrix(fn.Env).Invert());
}

void MatrixProto::Rotate(const FnCall& fn)
{
    // rotate(angle:Number) : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 0)
    {
        float r = (float)(fn.Arg(0).ToNumber(fn.Env));

        Render::Matrix2F m(pthis->GetMatrix(fn.Env));
        m.AppendRotation(r);

        pthis->SetMatrix(fn.Env, m);
    }
}

void MatrixProto::Scale(const FnCall& fn)
{
    // scale(sx:Number, sy:Number) : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 1)
    {
        float sx = (float)(fn.Arg(0).ToNumber(fn.Env));
        float sy = (float)(fn.Arg(1).ToNumber(fn.Env));

        Render::Matrix2F m(pthis->GetMatrix(fn.Env));
        m.AppendScaling(sx, sy);

        pthis->SetMatrix(fn.Env, m);
    }

// Just some tests of Render::Matrix2F.
//Render::Matrix2F m1(0.1, 0.2, 0.3, 0.4, 0.5, 0.6);
//m1.Rotate(1);
//m1.Scale(1.3, 1.8);
//m1.Rotate(-1.2);
//
//Render::Matrix2F m2 = m1;
//m1.CustomTranslate(-3.3, 4.4);
//m2.PrependTranslation(-3.3, 4.4);
//
//char b1[1024];
//char b2[1024];
//sprintf(b1, "%f %f %f %f %f %f", m1.Sx(),m1.Shx(),m1.Tx(),m1.Shy(),m1.Sy(),m1.Ty());
//sprintf(b2, "%f %f %f %f %f %f", m2.Sx(),m2.Shx(),m2.Tx(),m2.Shy(),m2.Sy(),m2.Ty());
//Render::Matrix2F m1(0.1, 0.2, 0.3, 0.4, 0.5, 0.6);
//Render::Matrix2F m2;
//m2.Rotate(1);
//m1.Multiply(m2);
//
//Render::Matrix2F m1(0.1, 0.2, 0.3, 0.4, 0.5, 0.6);
//Render::Matrix2F m2;
//m2.Rotate(1);
//m2.Multiply(m1);
//
//m1.Sx() = 0;
}

void MatrixProto::Translate(const FnCall& fn)
{
    // translate(tx:Number, ty:Number) : Void
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs > 1)
    {
        float tx = (float)(fn.Arg(0).ToNumber(fn.Env));
        float ty = (float)(fn.Arg(1).ToNumber(fn.Env));

        Render::Matrix2F m(pthis->GetMatrix(fn.Env));
        m.AppendTranslation(tx, ty);

        pthis->SetMatrix(fn.Env, m);
    }
}

void MatrixProto::TransformPoint(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_POINT

    // transformPoint(pt:Point) : Point
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Object* arg = 0;
    if (fn.NArgs > 0 && (arg = fn.Arg(0).ToObject(fn.Env)) != NULL &&
       arg->GetObjectType() == ObjectInterface::Object_Point)
    {
        Render::Matrix2F m = pthis->GetMatrix(fn.Env);
        Value ptprop[Point_NumProperties];
        ((PointObject*)arg)->GetProperties(fn.Env->GetSC(), ptprop);

        Ptr<PointObject> ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
        Value nptprop[Point_NumProperties];
        // begin transform point logic
        Value x1 = Value(m.Sx());
        x1.Mul(fn.Env, ptprop[Point_X]);
        Value x2 = Value(m.Shx());
        x2.Mul(fn.Env, ptprop[Point_Y]);
        nptprop[Point_X] = Value(m.Tx());
        nptprop[Point_X].Add(fn.Env, x1);
        nptprop[Point_X].Add(fn.Env, x2);
        
        x1 = Value(m.Shy());
        x1.Mul(fn.Env, ptprop[Point_X]);
        x2 = Value(m.Sy());
        x2.Mul(fn.Env, ptprop[Point_Y]);
        nptprop[Point_Y] = Value(m.Ty());
        nptprop[Point_Y].Add(fn.Env, x1);
        nptprop[Point_Y].Add(fn.Env, x2);
        
        /** Maxim's code
        ppt->SetPoint(fn.Env, 
                      GPoint2D(m.Sx() * p.x + m.Shx() * p.y + m.Tx(),
                               m.Shy() * p.x + m.Sy() * p.y + m.Ty()));
        **/
        // end transform point logic
        ppt->SetProperties(fn.Env->GetSC(), nptprop);
        fn.Result->SetAsObject(ppt.GetPtr());
    }
#else
    SF_DEBUG_WARNING(1, "Point ActionScript class was not included in this GFx build.");
    fn.Result->SetUndefined();
#endif  // SF_NO_FXPLAYER_AS_POINT
}

void MatrixProto::DeltaTransformPoint(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_POINT

    // deltaTransformPoint(pt:Point) : Point
    CHECK_THIS_PTR(fn, Matrix);
    MatrixObject* pthis = (MatrixObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Object* arg = 0;
    if (fn.NArgs > 0 && (arg = fn.Arg(0).ToObject(fn.Env)) != NULL &&
       arg->GetObjectType() == ObjectInterface::Object_Point)
    {
        Render::Matrix2F m = pthis->GetMatrix(fn.Env);
        Value ptprop[Point_NumProperties];
        ((PointObject*)arg)->GetProperties(fn.Env->GetSC(), ptprop);

        Ptr<PointObject> ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
        Value nptprop[Point_NumProperties];
        // begin delta transform point logic
        Value x1 = Value(m.Sx());
        x1.Mul(fn.Env, ptprop[Point_X]);
        nptprop[Point_X] = Value(m.Shx());
        nptprop[Point_X].Mul(fn.Env, ptprop[Point_Y]);
        nptprop[Point_X].Add(fn.Env, x1);

        x1 = Value(m.Shy());
        x1.Mul(fn.Env, ptprop[Point_X]);
        nptprop[Point_Y] = Value(m.Sy());
        nptprop[Point_Y].Mul(fn.Env, ptprop[Point_Y]);
        nptprop[Point_Y].Add(fn.Env, x1);
        /** Maxim's code
        ppt->SetPoint(fn.Env, 
                      GPoint2D(m.Sx() * p.GetX() + m.Shx() * p.GetY(),
                               m.Shy() * p.GetX() + m.Sy() * p.GetY()));
        **/
        // end delta transform point logic
        ppt->SetProperties(fn.Env->GetSC(), nptprop);
        fn.Result->SetAsObject(ppt.GetPtr());
    }
#else
    SF_DEBUG_WARNING(1, "Point ActionScript class was not included in this GFx build.");
    fn.Result->SetUndefined();
#endif  // SF_NO_FXPLAYER_AS_POINT
}

static const NameFunction GAS_MatrixFunctionTable[] = 
{
    { "toString",            &MatrixProto::ToString   },
    { "valueOf",             &MatrixProto::ToString   },

    { "clone",               &MatrixProto::Clone },
    { "concat",              &MatrixProto::Concat },
    { "createBox",           &MatrixProto::CreateBox },
    { "createGradientBox",   &MatrixProto::CreateGradientBox },
    { "deltaTransformPoint", &MatrixProto::DeltaTransformPoint },
    { "identity",            &MatrixProto::Identity },
    { "invert",              &MatrixProto::Invert },
    { "rotate",              &MatrixProto::Rotate },
    { "scale",               &MatrixProto::Scale },
    { "transformPoint",      &MatrixProto::TransformPoint },
    { "translate",           &MatrixProto::Translate },

    { 0, 0 }
};

 



MatrixProto::MatrixProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<MatrixObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_MatrixFunctionTable);
}

Object* GASMatrixCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) MatrixObject(penv);
}


void GASMatrixCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<MatrixObject> pmatrix;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Matrix && 
        !fn.ThisPtr->IsBuiltinPrototype())
        pmatrix = static_cast<MatrixObject*>(fn.ThisPtr);
    else
        pmatrix = *SF_HEAP_NEW(fn.Env->GetHeap()) MatrixObject(fn.Env);
    fn.Result->SetAsObject(pmatrix.GetPtr());

    if (fn.NArgs > 0)
    {   
        ASStringContext* psc = fn.Env->GetSC();

        pmatrix->SetConstMemberRaw(psc, "a", fn.Arg(0));
        if (fn.NArgs > 1)
        {
            pmatrix->SetConstMemberRaw(psc, "b", fn.Arg(1));
            if (fn.NArgs > 2)
            {
                pmatrix->SetConstMemberRaw(psc, "c", fn.Arg(2));
                if (fn.NArgs > 3)
                {
                    pmatrix->SetConstMemberRaw(psc, "d", fn.Arg(3));
                    if (fn.NArgs > 4)
                    {
                        pmatrix->SetConstMemberRaw(psc, "tx", fn.Arg(4));
                        if (fn.NArgs > 5)
                        {
                            pmatrix->SetConstMemberRaw(psc, "ty", fn.Arg(5));
                        }
                    }
                }
            }
        }
    }
}

FunctionRef GASMatrixCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) GASMatrixCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) MatrixProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Matrix, proto);
    pgc->FlashGeomPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Matrix), Value(ctor));
    return ctor;
}

#else

void GASMatrix_DummyFunction() {}   // Exists to quelch compiler warning

#endif  // SF_NO_FXPLAYER_AS_MATRIX

}}} //SF::GFx::AS2
