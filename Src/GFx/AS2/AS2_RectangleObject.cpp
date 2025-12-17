/**************************************************************************

Filename    :   AS2_RectangleObject.cpp
Content     :   flash.geom.Rectangle reference class for ActionScript 2.0
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
#include "GFx/AS2/AS2_RectangleObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_RECTANGLE disables Rectangle class
#ifdef GFX_AS2_ENABLE_RECTANGLE

bool IsRectValid(const ASRect& r) 
{
    return !(NumberUtil::IsNaN(r.x1) || NumberUtil::IsNaN(r.y1) || NumberUtil::IsNaN(r.x2) || NumberUtil::IsNaN(r.y2));
}

void ValidateRect(ASRect* r)
{
    if (NumberUtil::IsNaN(r->x1)) r->x1 = 0;
    if (NumberUtil::IsNaN(r->x2)) r->x2 = 0;
    if (NumberUtil::IsNaN(r->y1)) r->y1 = 0;
    if (NumberUtil::IsNaN(r->y2)) r->y2 = 0;
}

// default parameters
static const Value Rectangle_DefaultParams[Rectangle_NumProperties] =
{
    0,  // x:Number
    0,  // y:Number
    0,  // w:Number
    0   // h:Number
};

// NaN parameters
static const Value Rectangle_NaNParams[Rectangle_NumProperties] = 
{
    NumberUtil::NaN(),
    NumberUtil::NaN(),
    NumberUtil::NaN(),
    NumberUtil::NaN()
};

// ****************************************************************************
// Helper function to retrieve the point properties from an object
//
static void GFxObject_GetRectangleProperties(Environment *penv, Object *pobj, Value params[Rectangle_NumProperties])
{
    ASStringContext* psc = penv->GetSC();
    pobj->GetConstMemberRaw(psc, "x", &params[Rectangle_X]);
    pobj->GetConstMemberRaw(psc, "y", &params[Rectangle_Y]);
    pobj->GetConstMemberRaw(psc, "width", &params[Rectangle_Width]);
    pobj->GetConstMemberRaw(psc, "height", &params[Rectangle_Height]);
}

/*
static void GFxObject_GetRectangleProperties(Environment *penv, Object *pobj, ASRect &r)
{
    ASStringContext* psc = penv->GetSC();
    Value params[Rectangle_NumProperties];
    pobj->GetConstMemberRaw(psc, "x", &params[Rectangle_X]);
    pobj->GetConstMemberRaw(psc, "y", &params[Rectangle_Y]);
    pobj->GetConstMemberRaw(psc, "width", &params[Rectangle_Width]);
    pobj->GetConstMemberRaw(psc, "height", &params[Rectangle_Height]);
    r.SetRect(params[Rectangle_X].ToNumber(penv),
              params[Rectangle_Y].ToNumber(penv),
              GSize<Number>(params[Rectangle_Width].ToNumber(penv),
                               params[Rectangle_Height].ToNumber(penv)));
}
*/

// ****************************************************************************
// Helper function to compute the top left (PointObject)
//
static Value Rectangle_ComputeTopLeft(Environment *penv, ASRect r)
{
    Ptr<PointObject> gpt = *SF_HEAP_NEW(penv->GetHeap()) PointObject(penv);
    PointObject* pt = gpt.GetPtr();
    pt->SetProperties(penv, r.TopLeft());
    return pt;
}

// ****************************************************************************
// Helper function to compute the bottom right (PointObject)
//
static Value Rectangle_ComputeBottomRight(Environment *penv, ASRect r)
{
    Ptr<PointObject> gpt = *SF_HEAP_NEW(penv->GetHeap()) PointObject(penv);
    PointObject* pt = gpt.GetPtr();
    pt->SetProperties(penv, r.BottomRight());
    return pt;
}

// ****************************************************************************
// Helper function to compute the size (PointObject)
//
static Value Rectangle_ComputeSize(Environment *penv, ASRect r)
{
    Ptr<PointObject> gpt = *SF_HEAP_NEW(penv->GetHeap()) PointObject(penv);
    PointObject *pt = gpt.GetPtr();    
    pt->SetProperties(penv, GASPoint(r.Width(), r.Height()));
    return pt;
}

// ****************************************************************************
// Helper function to recalculate dependent properties of 'topLeft'
//
static void Rectangle_SetTopLeft(Environment *penv, const Value &val, ASRect &r) 
{
    Object* o = val.ToObject(penv);
    if (o)
    {
        GASPoint pt;
        GFxObject_GetPointProperties(penv, o, pt);
        r.SetTopLeft(pt);
    }
}

// ****************************************************************************
// Helper function to recalculate dependent properties of 'bottomRight'
//
static void Rectangle_SetBottomRight(Environment *penv, const Value &val, ASRect &r) 
{
    Object* o = val.ToObject(penv);
    if (o)
    {
        GASPoint pt;
        GFxObject_GetPointProperties(penv, o, pt);
        r.SetBottomRight(pt);
    }
}

// ****************************************************************************
// Helper function to recalculate dependent properties of 'size'
//
static void Rectangle_SetSize(Environment *penv, const Value &val, ASRect &r) 
{
    Object* o = val.ToObject(penv);
    if (o)
    {
        GASPoint pt;
        GFxObject_GetPointProperties(penv, o, pt);
        r.SetSize(pt.x, pt.y);
    }
}

// ****************************************************************************
// GASRectangle  constructor
//
RectangleObject::RectangleObject(Environment *penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Rectangle));
    SetProperties(penv->GetSC(), Rectangle_DefaultParams);
}

// ****************************************************************************
// Handle get method for properties computed on the fly
//
bool RectangleObject::GetMember(Environment *penv, const ASString &name, Value* val)
{
    // try to handle this property
    // NOTE: Flash 8 is always case sensitive (Rectangle was introduced in Flash 8)
    if (name == "left") 
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = r.x1;
        return true;
    }
    else if (name == "right") 
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = r.x2;
        return true;
    }
    else if (name == "top")
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = r.y1;
        return true;
    }
    else if (name == "bottom")
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = r.y2;
        return true;
    }
    else if (name == "topLeft")
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = Rectangle_ComputeTopLeft(penv, r);
        return true;
    }
    else if (name == "bottomRight")
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = Rectangle_ComputeBottomRight(penv, r);
        return true;
    }
    else if (name == "size")
    { 
        ASRect r;
        GetProperties(penv, r);
        *val = Rectangle_ComputeSize(penv, r);
        return true;
    }
    // if property wasn't gobbled up by the custom handler, pass it along to the base class
    return Object::GetMember(penv, name, val);
}

// ****************************************************************************
// Intercept the set command so that dependent properties can be recalculated
//
bool RectangleObject::SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags)
{
    if (flags.GetFlags() & PropFlags::PropFlag_ReadOnly)
    {
        return false;
    }

    // every property of Rectangle can be set and each has its own dependent properties
    // handle the cases individually
    if  (name == "left")        
    { 
        ASRect r;
        GetProperties(penv, r);
        r.x1 = val.ToNumber(penv);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "top")
    { 
        ASRect r;
        GetProperties(penv, r);
        r.y1 = val.ToNumber(penv);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "right")
    { 
        ASRect r;
        GetProperties(penv, r);
        r.x2 = val.ToNumber(penv);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "bottom")  
    { 
        ASRect r;
        GetProperties(penv, r);
        r.y2 = val.ToNumber(penv);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "topLeft")
    { 
        ASRect r;
        GetProperties(penv, r);
        Rectangle_SetTopLeft(penv, val, r);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "bottomRight")
    { 
        ASRect r;
        GetProperties(penv, r);
        Rectangle_SetBottomRight(penv, val, r);
        SetProperties(penv, r);
        return true;
    }
    else if (name == "size")
    { 
        ASRect r;
        GetProperties(penv, r);
        Rectangle_SetSize(penv, val, r);
        SetProperties(penv, r);
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

// ****************************************************************************
// Get the Rectangle properties
//
void RectangleObject::GetProperties(ASStringContext *psc, Value params[Rectangle_NumProperties])
{
    GetConstMemberRaw(psc, "x", &(params[Rectangle_X]));
    GetConstMemberRaw(psc, "y", &(params[Rectangle_Y]));
    GetConstMemberRaw(psc, "width", &(params[Rectangle_Width]));
    GetConstMemberRaw(psc, "height", &(params[Rectangle_Height]));
}

void RectangleObject::GetProperties(Environment *penv, ASRect &r)
{
    ASStringContext *psc = penv->GetSC();
    Value params[Rectangle_NumProperties];
    GetConstMemberRaw(psc, "x", &(params[Rectangle_X]));
    GetConstMemberRaw(psc, "y", &(params[Rectangle_Y]));
    GetConstMemberRaw(psc, "width", &(params[Rectangle_Width]));
    GetConstMemberRaw(psc, "height", &(params[Rectangle_Height]));
    r.SetRect(params[Rectangle_X].ToNumber(penv),
                params[Rectangle_Y].ToNumber(penv),
                Render::Size<Number>(params[Rectangle_Width].ToNumber(penv),
                                params[Rectangle_Height].ToNumber(penv)));
}

// ****************************************************************************
// Set the Rectangle properties
//
void RectangleObject::SetProperties(ASStringContext *psc, const Value params[Rectangle_NumProperties])
{
    SetConstMemberRaw(psc, "x", params[Rectangle_X]);
    SetConstMemberRaw(psc, "y", params[Rectangle_Y]);
    SetConstMemberRaw(psc, "width", params[Rectangle_Width]);
    SetConstMemberRaw(psc, "height", params[Rectangle_Height]);
}

void RectangleObject::SetProperties(Environment *penv, const ASRect &r)
{
    ASStringContext *psc = penv->GetSC();
    SetConstMemberRaw(psc, "x", r.x1);
    SetConstMemberRaw(psc, "y", r.y1);
    SetConstMemberRaw(psc, "width", r.Width());
    SetConstMemberRaw(psc, "height", r.Height());
}

// ****************************************************************************
// AS to GFx function mapping
//
const NameFunction RectangleProto::FunctionTable[] = 
{
    { "clone", &RectangleProto::Clone },
    { "contains", &RectangleProto::Contains },
    { "containsPoint", &RectangleProto::ContainsPoint },
    { "containsRectangle", &RectangleProto::ContainsRectangle },
    { "equals", &RectangleProto::Equals },
    { "inflate", &RectangleProto::Inflate },
    { "inflatePoint", &RectangleProto::InflatePoint },
    { "intersection", &RectangleProto::Intersection },
    { "intersects", &RectangleProto::Intersects },
    { "isEmpty", &RectangleProto::IsEmpty },
    { "offset", &RectangleProto::Offset },
    { "offsetPoint", &RectangleProto::OffsetPoint },
    { "setEmpty", &RectangleProto::SetEmpty },
    { "toString", &RectangleProto::ToString },
    { "union", &RectangleProto::Union },
    { 0, 0 }
};

// ****************************************************************************
// GASRectangle Prototype constructor
//
RectangleProto::RectangleProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) :
    Prototype<RectangleObject>(psc, pprototype, constructor)
{
    // we make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    // add the on-the-fly calculated properties
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("left"), 0, PropFlags::PropFlag_DontDelete);
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("top"), 0, PropFlags::PropFlag_DontDelete);
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("right"), 0, PropFlags::PropFlag_DontDelete);
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("bottom"), 0, PropFlags::PropFlag_DontDelete);  
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("topLeft"), 0, PropFlags::PropFlag_DontDelete);
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("bottomRight"), 0, PropFlags::PropFlag_DontDelete);
    RectangleObject::SetMemberRaw(psc, psc->CreateConstString("size"), 0, PropFlags::PropFlag_DontDelete);
    //InitFunctionMembers(psc, FunctionTable, pprototype);
}

// ****************************************************************************
// Rectangle.clone method
//
void RectangleProto::Clone(const FnCall &fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject* p_this = (RectangleObject*) fn.ThisPtr;
    SF_ASSERT(p_this);
    Ptr<RectangleObject> p_clone = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);
    Value params[Rectangle_NumProperties];
    p_this->GetProperties(fn.Env->GetSC(), params);
    // begin copy of properties
    p_clone.GetPtr()->SetProperties(fn.Env->GetSC(), params);
    // end copy of properties
    fn.Result->SetAsObject(p_clone.GetPtr());
}

// ****************************************************************************
// Rectangle.clone method
//
void RectangleProto::Contains(const FnCall& fn)
{
    if (fn.NArgs > 1)
    {
        Value px = fn.Arg(0);
        Value py = fn.Arg(1);
        CHECK_THIS_PTR(fn, Rectangle);
        RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
        SF_ASSERT(pthis);
        ASRect r;
        // get properties for this
        pthis->GetProperties(fn.Env, r);
        // begin contains logic
        Number pxn = px.ToNumber(fn.Env);
        Number pyn = py.ToNumber(fn.Env);
        if (NumberUtil::IsNaN(pxn) || NumberUtil::IsNaN(pyn))
        {
            fn.Result->SetBool(false);
            return;
        }
        // AS 2.0 contains method is not closed on the right and bottom boundaries
        // handle that case
        if (pxn == r.x2)
        {
            pxn += 1.0;
        }
        if (pyn == r.y2)
        {
            pyn += 1.0;
        }
        fn.Result->SetBool(r.Contains(pxn, pyn));
        // end contains logic
    }
    else
    {
        fn.Result->SetBool(false);
    }
}

// ****************************************************************************
// Rectangle.containsPoint method
//
void RectangleProto::ContainsPoint(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if ( NULL != p )
        {
            CHECK_THIS_PTR(fn, Rectangle);
            RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            ASRect r;
            Value ptv[Point_NumProperties];
            // get properties for this
            pthis->GetProperties(fn.Env, r);
            // get properties for parameter
            GFxObject_GetPointProperties(fn.Env, p, ptv);
            // if param is not a point, then x and y both must be defined, else return undefined
            if ( (p->GetObjectType() != Object_Point) && (ptv[Point_X].IsUndefined() || ptv[Point_Y].IsUndefined()) )
            {
                return;
            }
            // begin contains logic
            GASPoint pt;
            ((PointObject*)p)->GetProperties(fn.Env, pt);
            if (NumberUtil::IsNaN(pt.x) || NumberUtil::IsNaN(pt.y))
            {
                fn.Result->SetBool(false);
                return;
            }
            // AS 2.0 contains method is not closed on the right and bottom boundaries
            // handle that case
            if (pt.x == r.x2)
            {
                pt.x += 1.0;
            }
            if (pt.y == r.y2)
            {
                pt.y += 1.0;
            }
            fn.Result->SetBool(r.Contains(pt));
            // end contains logic
        }
    }
}

// ****************************************************************************
// Rectangle.containsRectangle method
//
void RectangleProto::ContainsRectangle(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if ( NULL != p )
        {
            CHECK_THIS_PTR(fn, Rectangle);
            RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            ASRect r1;
            Value r2v[Rectangle_NumProperties];
            // get properties for this
            pthis->GetProperties(fn.Env, r1);
            // get properties for parameter
            GFxObject_GetRectangleProperties(fn.Env, p, r2v);
            // x, y, w, h must be defined, else return undefined
            if ( r2v[Rectangle_X].IsUndefined() || r2v[Rectangle_Y].IsUndefined() ||
                 r2v[Rectangle_Width].IsUndefined() || r2v[Rectangle_Height].IsUndefined() )
            {
                return;
            }
            // begin contains logic
            ASRect r2(r2v[Rectangle_X].ToNumber(fn.Env),
                       r2v[Rectangle_Y].ToNumber(fn.Env),
                       Render::Size<Number>(r2v[Rectangle_Width].ToNumber(fn.Env),
                                        r2v[Rectangle_Height].ToNumber(fn.Env)));           
            fn.Result->SetBool(r1.Contains(r2));
            // end contains logic
        }
    }
}

// ****************************************************************************
// Rectangle.equals method
//
void RectangleProto::Equals(const FnCall& fn)
{
    bool ret = false;
    if ( fn.NArgs > 0 )
    {
        Object *p = fn.Arg(0).ToObject(fn.Env);
        if (( NULL != p) && (p->GetObjectType() == Object_Rectangle))
        {
            CHECK_THIS_PTR(fn, Rectangle);
            RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            Ptr<RectangleObject> parr = (RectangleObject*)p;
            ASRect r1, r2;
            // get properties for this
            pthis->GetProperties(fn.Env, r1);
            // get properties for parameter
            parr->GetProperties(fn.Env, r2);
            if (IsRectValid(r1) && IsRectValid(r2))
                ret = (r1 == r2);
        }
    }
    fn.Result->SetBool(ret);
}

// ****************************************************************************
// Rectangle.inflate method
//
void RectangleProto::Inflate(const FnCall& fn)
{
    Value dw, dh;
    if (fn.NArgs > 0)
    {
        dw = fn.Arg(0);
        if (fn.NArgs > 1)
        {
            dh = fn.Arg(1);
        }
    }
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    ASRect o1;
    // get properties for this
    pthis->GetProperties(fn.Env, o1);
    // begin inflate logic
    o1.Expand(dw.ToNumber(fn.Env), dh.ToNumber(fn.Env));
    pthis->SetProperties(fn.Env, o1);
    // end inflate logic        
}

// ****************************************************************************
// Rectangle.inflatePoint method
//
void RectangleProto::InflatePoint(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        CHECK_THIS_PTR(fn, Rectangle);
        RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
        SF_ASSERT(pthis);
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if ( NULL == p )
        {
            pthis->SetProperties(fn.Env->GetSC(), Rectangle_NaNParams);
            return;
        }   
        ASRect o1;
        GASPoint o2;
        // get properties for this
        pthis->GetProperties(fn.Env, o1);
        // get properties for parameter
        GFxObject_GetPointProperties(fn.Env, p, o2);
        // begin contains logic
        o1.Expand(o2.x, o2.y);
        // end contains logic
        pthis->SetProperties(fn.Env, o1);
    }
}

// ****************************************************************************
// Rectangle.intersection method
//
void RectangleProto::Intersection(const FnCall& fn)
{
    ASRect ret(0,0,0,0);
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if (p != NULL)
        {
            ASRect o1;
            Value o2v[Rectangle_NumProperties];
            // get properties for this
            pthis->GetProperties(fn.Env, o1);
            // get properties for parameter
            GFxObject_GetRectangleProperties(fn.Env, p, o2v);
            ASRect o2(o2v[Rectangle_X].ToNumber(fn.Env),
                o2v[Rectangle_Y].ToNumber(fn.Env),
                Render::Size<Number>(o2v[Rectangle_Width].ToNumber(fn.Env),
                o2v[Rectangle_Height].ToNumber(fn.Env)));

            if (!IsRectValid(o2))
            {
                ret.SetRect(0, 0, 0, 0);
            }
            else
            {
                // begin intersection logic
                o1.IntersectRect(&ret, o2);
                // AS 2.0 returns an empty rect if width or height is 0, NaN or undefined
                // handle that case
                if (!IsRectValid(ret) || (ret.Width() == 0) || (ret.Height() == 0))
                {
                    ret.SetRect(0, 0, 0, 0);
                }
                // end intersection logic
            }
        }
    }
    Ptr<RectangleObject> obj = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);
    obj.GetPtr()->SetProperties(fn.Env, ret);
    fn.Result->SetAsObject(obj.GetPtr());
}

// ****************************************************************************
// Rectangle.intersects method
//
void RectangleProto::Intersects(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    bool ret = false;
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if (p != NULL)
        {
            ASRect o1;
            Value o2v[Rectangle_NumProperties];
            // get properties for this
            pthis->GetProperties(fn.Env, o1);
            // get properties for parameter
            GFxObject_GetRectangleProperties(fn.Env, p, o2v);
            ASRect o2(o2v[Rectangle_X].ToNumber(fn.Env),
                o2v[Rectangle_Y].ToNumber(fn.Env),
                Render::Size<Number>(o2v[Rectangle_Width].ToNumber(fn.Env),
                o2v[Rectangle_Height].ToNumber(fn.Env)));
            // begin intersection logic
            ASRect r(0,0,0,0);
            if (!IsRectValid(o1) || !IsRectValid(o2))
                ret = false;
            else
            {
                o1.IntersectRect(&r, o2);
                ret = true;
                // AS 2.0 returns an empty rect if width or height is 0, NaN or undefined
                // handle that case
                if (!IsRectValid(r) ||  (r.Width() == 0) || (r.Height() == 0))
                {
                    ret = false;
                }
                // end intersection logic
            }
        }
    }
    fn.Result->SetBool(ret);
}

// ****************************************************************************
// Rectangle.isEmpty method
///
void RectangleProto::IsEmpty(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    bool ret = false;
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    Value params[Rectangle_NumProperties];
    pthis->GetProperties(fn.Env->GetSC(), params);
    // begin isempty logic
    // AS 2.0 isempty method return true iff w==h==0 or (w||h)==NaN
    if ( NumberUtil::IsNaN(params[Rectangle_Width].ToNumber(fn.Env)) || 
        NumberUtil::IsNaN(params[Rectangle_Height].ToNumber(fn.Env)) )
    {
        ret = true;
    }
    else
    {
        ASRect o1 = (params[Rectangle_X].ToNumber(fn.Env),
            params[Rectangle_Y].ToNumber(fn.Env),
            Render::Size<Number>(params[Rectangle_Width].ToNumber(fn.Env),
            params[Rectangle_Height].ToNumber(fn.Env)));
        ret = o1.IsEmpty();
    }
    // end isempty logic
    fn.Result->SetBool(ret);
}

// ****************************************************************************
// Rectangle.offset method
//
void RectangleProto::Offset(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    Value dx, dy;
    if (fn.NArgs > 0)
    {
        dx = fn.Arg(0);
        if (fn.NArgs > 1)
        {
            dy = fn.Arg(1);
        }
    }
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    Value o1[Rectangle_NumProperties];
    // get properties for this
    pthis->GetProperties(fn.Env->GetSC(), o1);
    // begin offset logic
    // in AS 2.0, width and height do not change
    // handle that
    o1[Rectangle_X] = o1[Rectangle_X].ToNumber(fn.Env) + dx.ToNumber(fn.Env);
    o1[Rectangle_Y] = o1[Rectangle_Y].ToNumber(fn.Env) + dy.ToNumber(fn.Env);
    pthis->SetProperties(fn.Env->GetSC(), o1);
    // end offset logic     
}

// ****************************************************************************
// Rectangle.offsetPoint method
///
void RectangleProto::OffsetPoint(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        CHECK_THIS_PTR(fn, Rectangle);
        RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
        SF_ASSERT(pthis);
        Object* p = fn.Arg(0).ToObject(fn.Env);
        Value o1v[Rectangle_NumProperties];
        if ( NULL == p )
        {
            // get properties for this
            pthis->GetProperties(fn.Env->GetSC(), o1v);
            o1v[Rectangle_X] = NumberUtil::NaN();
            o1v[Rectangle_Y] = NumberUtil::NaN();
            pthis->SetProperties(fn.Env->GetSC(), o1v);
            return;
        }
        // get properties for parameter
        Value o2v[Point_NumProperties];
        GFxObject_GetPointProperties(fn.Env, p, o2v);
        // if param is not a point, then x and y both must be defined, else return undefined
        if ( (p->GetObjectType() != Object_Point) && (o2v[Point_X].IsUndefined() || o2v[Point_Y].IsUndefined()) )
        {
            // get properties for this
            pthis->GetProperties(fn.Env->GetSC(), o1v);
            o1v[Rectangle_X] = NumberUtil::NaN();
            o1v[Rectangle_Y] = NumberUtil::NaN();         
            pthis->SetProperties(fn.Env->GetSC(), o1v);
            return;
        }
        // get properties for this
        pthis->GetProperties(fn.Env->GetSC(), o1v);
        // begin contains logic
        o1v[Rectangle_X] = o1v[Rectangle_X].ToNumber(fn.Env) + o2v[Point_X].ToNumber(fn.Env);
        o1v[Rectangle_Y] = o1v[Rectangle_Y].ToNumber(fn.Env) + o2v[Point_Y].ToNumber(fn.Env);        
        // end contains logic
        pthis->SetProperties(fn.Env->GetSC(), o1v);
    }
}

// ****************************************************************************
// Rectangle.setEmpty method
//
void RectangleProto::SetEmpty(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    pthis->SetProperties(fn.Env, ASRect(0, 0, 0, 0));
}

// ****************************************************************************
// Rectangle.toString method
//
void RectangleProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject* pthis = (RectangleObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Value params[Rectangle_NumProperties];
    pthis->GetProperties(fn.Env->GetSC(), params);

    ASString ps[4] = 
    {
        params[Rectangle_X].ToString(fn.Env, 6),
        params[Rectangle_Y].ToString(fn.Env, 6),
        params[Rectangle_Width].ToString(fn.Env, 6),
        params[Rectangle_Height].ToString(fn.Env, 6)
    };

    String str;
    str += "(x=";
    str += ps[0].ToCStr();
    str += ", y=";
    str += ps[1].ToCStr();
    str += ", width=";
    str += ps[2].ToCStr();
    str += ", height=";
    str += ps[3].ToCStr();
    str += ")";

    fn.Result->SetString(fn.Env->CreateString(str));
}

// ****************************************************************************
// Rectangle.union method
//
void RectangleProto::Union(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Rectangle);
    RectangleObject *pthis = (RectangleObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    Ptr<RectangleObject> obj = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);
    fn.Result->SetAsObject(obj.GetPtr());
    if (fn.NArgs > 0)
    {           
        ASRect ret(NumberUtil::NaN(), NumberUtil::NaN(), NumberUtil::NaN(), NumberUtil::NaN());
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if (p != NULL)
        {
            ASRect o1;
            Value o2v[Rectangle_NumProperties];
            // get properties for this
            pthis->GetProperties(fn.Env, o1);
            // get properties for parameter
            GFxObject_GetRectangleProperties(fn.Env, p, o2v);
            ASRect o2(o2v[Rectangle_X].ToNumber(fn.Env),
                o2v[Rectangle_Y].ToNumber(fn.Env),
                Render::Size<Number>(o2v[Rectangle_Width].ToNumber(fn.Env),
                o2v[Rectangle_Height].ToNumber(fn.Env)));
            // begin union logic
            ValidateRect(&o1); //?
            ValidateRect(&o2); //?
            o1.UnionRect(&ret, o2);
            if (NumberUtil::IsNaN(o2v[Rectangle_X].ToNumber(fn.Env)))
            {
                ret.x1 = NumberUtil::NaN();
            }
            if (NumberUtil::IsNaN(o2v[Rectangle_Y].ToNumber(fn.Env)))
            {
                ret.y1 = NumberUtil::NaN();
            }
            // end union logic
        }
        obj.GetPtr()->SetProperties(fn.Env, ret);
    }
    else
    {
        obj.GetPtr()->SetProperties(fn.Env->GetSC(), Rectangle_NaNParams);
    }   
}

// ****************************************************************************
// Called when the constructor is invoked for the Rectangle class
//
void RectangleCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<RectangleObject> prect;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Rectangle && 
        !fn.ThisPtr->IsBuiltinPrototype())
        prect = static_cast<RectangleObject*>(fn.ThisPtr);
    else
        prect = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);
    fn.Result->SetAsObject(prect.GetPtr());

    // handle the constructor parameters
    if (fn.NArgs > 0)
    {
        Value params[Rectangle_NumProperties];
        params[Rectangle_X] = fn.Arg(0);
        if (fn.NArgs > 1)
        {
            params[Rectangle_Y] = fn.Arg(1);
            if (fn.NArgs > 2)
            {
                params[Rectangle_Width] = fn.Arg(2);
                if (fn.NArgs > 3)
                {
                    params[Rectangle_Height] = fn.Arg(3);
                }
            }
        }

        // set the parameters
        prect->SetProperties(fn.Env->GetSC(), params);
    }
}

Object* RectangleCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) RectangleObject(penv);
}

FunctionRef RectangleCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  rectCtor(*SF_HEAP_NEW(pgc->GetHeap()) RectangleCtorFunction(&sc));
    Ptr<Object> rectProto = *SF_HEAP_NEW(pgc->GetHeap()) RectangleProto(&sc, pgc->GetPrototype(ASBuiltin_Object), rectCtor);
    pgc->SetPrototype(ASBuiltin_Rectangle, rectProto);
    pgc->FlashGeomPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Rectangle), Value(rectCtor));
    return rectCtor;
}

#else

void GASRectangle_DummyFunction() {}   // Exists to quelch compiler warning

#endif //   SF_NO_FXPLAYER_AS_RECTANGLE
}}} //SF::GFx::AS2
