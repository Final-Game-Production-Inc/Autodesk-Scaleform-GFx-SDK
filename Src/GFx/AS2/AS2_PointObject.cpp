/**************************************************************************

Filename    :   AS2_PointObject.cpp
Content     :   flash.geom.Point reference class for ActionScript 2.0
Created     :   3/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_PointObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {


// SF_NO_FXPLAYER_AS_POINT disables Point class
#ifdef GFX_AS2_ENABLE_POINT

// default parameters
static const Value Point_DefaultParams[Point_NumProperties] = 
{
    0,  // x:Number
    0   // y:Number
};

// NaN parameters
static const Value Point_NanParams[Point_NumProperties] =
{
    NumberUtil::NaN(),
    NumberUtil::NaN()
};

// ****************************************************************************
// Helper function to retrieve the point properties from an object
//
void GFxObject_GetPointProperties(Environment *penv, Object *pobj, Value params[Point_NumProperties])
{
    ASStringContext* psc = penv->GetSC();
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &params[Point_X]);
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &params[Point_Y]);
}

void GFxObject_GetPointProperties(Environment *penv, Object *pobj, GASPoint &pt)
{
    ASStringContext* psc = penv->GetSC();
    Value params[Point_NumProperties];
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &(params[Point_X]));
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &(params[Point_Y]));
    pt.SetPoint(params[Point_X].ToNumber(penv), params[Point_Y].ToNumber(penv));
}

// ****************************************************************************
// GASPoint constructors
//
PointObject::PointObject(Environment *penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Point));
    SetProperties(penv->GetSC(), Point_DefaultParams);
}

PointObject::PointObject(Environment* penv, PointObject *po)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Point));
    Value params[Point_NumProperties];
    po->GetProperties(penv->GetSC(), params);
    SetProperties(penv->GetSC(), params);
}

// ****************************************************************************
// Handle get method for properties computed on the fly
//
bool PointObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    bool ret = false;
    ASStringContext* psc = penv->GetSC();
    // try to handle this property
    // NOTE: Flash 8 is always case sensitive (Point was introduced in Flash 8)
    if (name == psc->GetBuiltin(ASBuiltin_length))
    {
        GASPoint pt;
        GetProperties(penv, pt);
        *val = pt.Distance();
        ret = true;
    }
    // if property wasn't gobbled up by the custom handler, pass it along to the base class
    else
    {
        ret = Object::GetMember(penv, name, val);
    }
    return ret;
}

// ****************************************************************************
// Get the Point
//
void PointObject::GetProperties(ASStringContext *psc, Value params[Point_NumProperties])
{
    GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &(params[Point_X]));
    GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &(params[Point_Y]));

}

void PointObject::GetProperties(Environment *penv, GASPoint &pt)
{
    ASStringContext *psc = penv->GetSC();
    Value params[Point_NumProperties];
    GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &(params[Point_X]));
    GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &(params[Point_Y]));
    pt.SetPoint(params[Point_X].ToNumber(penv), params[Point_Y].ToNumber(penv));
}

// ****************************************************************************
// Set the Point
//
void PointObject::SetProperties(ASStringContext *psc, const Value params[Point_NumProperties])
{
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), params[Point_X]);
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), params[Point_Y]);

}

void PointObject::SetProperties(Environment *penv, const GASPoint &pt)
{
    ASStringContext *psc = penv->GetSC();
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), pt.x);
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), pt.y);
}

// ****************************************************************************
// AS to GFx function mapping
//
const NameFunction PointProto::FunctionTable[] = 
{
    { "add", &PointProto::Add },
    { "clone", &PointProto::Clone },
    { "equals", &PointProto::Equals },
    { "normalize", &PointProto::Normalize },
    { "offset", &PointProto::Offset },
    { "subtract", &PointProto::Subtract },
    { "toString", &PointProto::ToString },
    { 0, 0 }
};

// ****************************************************************************
// GASPoint prototype constructor
//
PointProto::PointProto(ASStringContext *psc, Object *pprototype, const FunctionRef &constructor) : 
    Prototype<PointObject>(psc, pprototype, constructor)
{
    // we make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    // add the on-the-fly calculated properties
    PointObject::SetMemberRaw(psc, psc->CreateConstString("length"), 0, PropFlags::PropFlag_DontDelete);
    //InitFunctionMembers(psc, FunctionTable, pprototype);
}

// ****************************************************************************
// Point.add method
//
void PointProto::Add(const FnCall& fn)
{
    Ptr<PointObject> retpt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    bool set = false;
    if (fn.NArgs > 0) 
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if (NULL != p)
        {
            CHECK_THIS_PTR(fn, Point);
            PointObject* pthis = (PointObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            GASPoint pt1, pt2;
            // get properties for this
            pthis->GetProperties(fn.Env, pt1);
            // get properties for parameter
            GFxObject_GetPointProperties(fn.Env, p, pt2);
            // begin add logic
            pt1 += pt2;
            retpt->SetProperties(fn.Env, pt1);
            // end add logic
            set = true;
        }
    }
    if (!set)
    {
        retpt->SetProperties(fn.Env->GetSC(), Point_NanParams);
    }
    fn.Result->SetAsObject(retpt.GetPtr());
}

// ****************************************************************************
// Point.clone method
//
void PointProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Point);
    PointObject* pthis = (PointObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    Ptr<PointObject> ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);

    ASStringContext* psc = fn.Env->GetSC();
    Value params[Point_NumProperties];
    pthis->GetProperties(psc, params);
    // begin copy of properties
    ppt.GetPtr()->SetProperties(psc, params);
    // end copy of properties

    fn.Result->SetAsObject(ppt.GetPtr());
}

// ****************************************************************************
// Point.equals method
//
void PointProto::Equals(const FnCall& fn)
{
    bool ret = false;
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if ((NULL != p) && (p->GetObjectType() == Object_Point))
        {       
            CHECK_THIS_PTR(fn, Point);
            PointObject* pthis = (PointObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            GASPoint pt1, pt2;
            // get properties for this
            pthis->GetProperties(fn.Env, pt1);
            // get properties for parameter
            GFxObject_GetPointProperties(fn.Env, p, pt2);
            // begin equals logic
            ret = (pt1 == pt2);
            // end equals logic
        }
    }
    fn.Result->SetBool(ret);
}

// ****************************************************************************
// Point.normalize method
//
void PointProto::Normalize(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Point);
    PointObject* pthis = (PointObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (fn.NArgs > 0)
    {
        Value p = fn.Arg(0);
        GASPoint pt1;
        pthis->GetProperties(fn.Env, pt1);
        // begin normalize logic
        Number m = (p.ToNumber(fn.Env) / pt1.Distance());
        pthis->SetProperties(fn.Env, pt1.Mul(m));
        // end normalize logic
    }
    else
    {
        pthis->SetProperties(fn.Env->GetSC(), Point_NanParams);
    }
}

// ****************************************************************************
// Point.offset method
//
void PointProto::Offset(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Point);
    PointObject* pthis = (PointObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    Value dx, dy;
    if (fn.NArgs > 0)
    {
        dx = fn.Arg(0);
        if (fn.NArgs > 1)
        {
            dy = fn.Arg(1); 
        }
    }
    GASPoint pt1;
    pthis->GetProperties(fn.Env, pt1);
    // begin offset logic   
    pthis->SetProperties(fn.Env, pt1.Offset(dx.ToNumber(fn.Env), dy.ToNumber(fn.Env)));
    // end offset logic
}

// ****************************************************************************
// Point.subtract method
//
void PointProto::Subtract(const FnCall& fn)
{
    Ptr<PointObject> retpt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    bool set = false;
    if (fn.NArgs > 0) 
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if ( NULL != p)
        {
            CHECK_THIS_PTR(fn, Point);
            PointObject* pthis = (PointObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            GASPoint pt1, pt2;
            // get properties for this
            pthis->GetProperties(fn.Env, pt1);
            // get properties for parameter
            GFxObject_GetPointProperties(fn.Env, p, pt2);
            // begin add logic
            pt1 -= pt2;
            retpt->SetProperties(fn.Env, pt1);
            // end add logic
            set = true;
        }
    }
    if (!set)
    {
        retpt->SetProperties(fn.Env->GetSC(), Point_NanParams);
    }
    fn.Result->SetAsObject(retpt.GetPtr());
}

// ****************************************************************************
// Point.toString method
//
void PointProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Point);
    PointObject* pthis = (PointObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Value params[Point_NumProperties];
    pthis->GetProperties(fn.Env->GetSC(), params);

    ASString ps[2] = 
    {
        params[Point_X].ToString(fn.Env, 6),
        params[Point_Y].ToString(fn.Env, 6)
    };

    String str;
    str += "(x=";
    str += ps[Point_X].ToCStr();
    str += ", y=";
    str += ps[Point_Y].ToCStr();
    str += ")";

    fn.Result->SetString(fn.Env->CreateString(str));
}


// ****************************************************************************
// AS to GFx static function mapping
//
const NameFunction PointCtorFunction::StaticFunctionTable[] = 
{
    { "interpolate", &PointCtorFunction::Interpolate },
    { "distance", &PointCtorFunction::Distance },
    { "polar", &PointCtorFunction::Polar },
    { 0, 0 }
};

// ****************************************************************************
// GASPoint ctor function constructor
//
PointCtorFunction::PointCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

// ****************************************************************************
// (Static) Point.distance method
//
void PointCtorFunction::Distance(const FnCall& fn)
{
    // handle weird behavior in AS 2.0..
    // if {x:10,y:10} and {x:20,y:20} are passed in as params, then the return value is undefined
    // if Point(10,10) and {x:20,y:20} are passed in as params, then the return value IS defined
    fn.Result->SetNumber(NumberUtil::NaN());
    if (fn.NArgs > 1) 
    {
        Object* p1 = fn.Arg(0).ToObject(fn.Env);
        Object* p2 = fn.Arg(1).ToObject(fn.Env);
        if (( NULL != p1) && ( NULL != p2))
        {
            if ((p1->GetObjectType() != Object_Point) &&
               (p2->GetObjectType() != Object_Point))
            {
                fn.Result->SetUndefined();
            }
            else
            {
                Value o1[Point_NumProperties], o2[Point_NumProperties];
                // get the first param's properties
                GFxObject_GetPointProperties(fn.Env, p1, o1);
                // get the second param's properties
                GFxObject_GetPointProperties(fn.Env, p2, o2);
                // begin distance logic
                Value xd = o2[Point_X];
                xd.Sub(fn.Env, o1[Point_X]);
                xd.Mul(fn.Env, xd);
                Value yd = o2[Point_Y];
                yd.Sub(fn.Env, o1[Point_Y]);
                yd.Mul(fn.Env, yd);
                xd.Add(fn.Env, yd);
                Number d = Value(Number(sqrt(xd.ToNumber(fn.Env)))).ToNumber(fn.Env);
                fn.Result->SetNumber(d);
                // end distance logic
            }
        }
    }
}

// ****************************************************************************
// (Static) Point.interpolate method
//
void PointCtorFunction::Interpolate(const FnCall& fn)
{
    // handle weird behavior in AS 2.0..
    // both params can be of the form {x:x_val, y:y_val} and the return value is a valid point object
    Ptr<PointObject> ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    bool set = false;
    if (fn.NArgs > 2) 
    {
        Object* p1 = fn.Arg(0).ToObject(fn.Env);
        Object* p2 = fn.Arg(1).ToObject(fn.Env);
        Value f = fn.Arg(2);
        if (( NULL != p1) && ( NULL != p2))
        {
            GASPoint pt1, pt2;
            // get the first param's properties
            GFxObject_GetPointProperties(fn.Env, p1, pt1);
            // get the second param's properties
            GFxObject_GetPointProperties(fn.Env, p2, pt2);
            Number ptf = f.ToNumber(fn.Env);
            // begin interpolate logic
            // x = ((pt1.x - pt2.x) * ptf) + pt2.x;
            // y = ((pt1.y - pt2.y) * ptf) + pt2.y;
            GASPoint ip( ((pt1.x - pt2.x) * ptf) + pt2.x, ((pt1.y - pt2.y) * ptf) + pt2.y );
            ppt->SetProperties(fn.Env, ip);
            // end interpolate logic
            set = true;
        }       
    }   
    if (!set)
    {
        ppt->SetProperties(fn.Env->GetSC(), Point_NanParams);
    }
    fn.Result->SetAsObject(ppt.GetPtr());
}

// ****************************************************************************
// (Static) Point.polar method
//
void PointCtorFunction::Polar(const FnCall& fn)
{
    Ptr<PointObject> ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    bool set = false;
    if (fn.NArgs > 1) 
    {
        Value length = fn.Arg(0);
        Value angle = fn.Arg(1);
        // begin polar logic
        Number l = length.ToNumber(fn.Env);
        Number a = angle.ToNumber(fn.Env);
        Double sa = sin(a);
        Double ca = cos(a);
        ppt->SetProperties(fn.Env, GASPoint(l*ca, l*sa));
        // end polar logic
        set = true;
    }   
    if (!set)
    {
        ppt->SetProperties(fn.Env->GetSC(), Point_NanParams);
    }
    fn.Result->SetAsObject(ppt.GetPtr());
}

// ****************************************************************************
// Called when the constructor is invoked for the Point class (new Point())
//
void PointCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<PointObject> ppt;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Point && 
        !fn.ThisPtr->IsBuiltinPrototype())
        ppt = static_cast<PointObject*>(fn.ThisPtr);
    else
        ppt = *SF_HEAP_NEW(fn.Env->GetHeap()) PointObject(fn.Env);
    fn.Result->SetAsObject(ppt.GetPtr());

    // handle constructor parameters
    if (fn.NArgs > 0)
    {
        Value params[Point_NumProperties];
        params[Point_X] = fn.Arg(0);
        if (fn.NArgs > 1)
        {
            params[Point_Y] = fn.Arg(1);
        }
        // set the parameters
        ppt->SetProperties(fn.Env->GetSC(), params);
    }
}

Object* PointCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) PointObject(penv);
}

FunctionRef PointCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef pointCtor(*SF_HEAP_NEW(pgc->GetHeap()) PointCtorFunction(&sc));
    Ptr<Object> pointProto = 
        *SF_HEAP_NEW(pgc->GetHeap()) PointProto(&sc, pgc->GetPrototype(ASBuiltin_Object), pointCtor);
    pgc->SetPrototype(ASBuiltin_Point, pointProto);
    pgc->FlashGeomPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Point), Value(pointCtor));
    return pointCtor;
}

#else

void GASPoint_DummyFunction() {}   // Exists to quelch compiler warning

#endif  //  SF_NO_FXPLAYER_AS_POINT

}}} //SF::GFx::AS2
