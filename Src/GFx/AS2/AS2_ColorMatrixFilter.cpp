/**********************************************************************

Filename    :   GFxASColorMatrixFilter.cpp
Content     :   ColorMatrixFilter reference class for ActionScript 2.0
Created     :   
Authors     :   
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_ColorMatrixFilter.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ColorMatrixFilterObject::ColorMatrixFilterObject()
// : BitmapFilterObject(GFxFilterDesc::Filter_ColorMatrix)
// {
// }

ColorMatrixFilterObject::ColorMatrixFilterObject(Environment* penv)
: BitmapFilterObject(penv, Render::Filter_ColorMatrix)
{
}

ColorMatrixFilterObject::~ColorMatrixFilterObject()
{
}

bool ColorMatrixFilterObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags /* = PropFlags */)
{
    if (name == "matrix")
    {
        Object* pobj = val.ToObject(penv);
        Render::ColorMatrixFilter* filter = static_cast<Render::ColorMatrixFilter*>(pFilter.GetPtr());
        if ( !filter || filter->GetFilterType() != Render::Filter_ColorMatrix )
            return false;
        if (pobj && pobj->InstanceOf(penv, penv->GetPrototype(ASBuiltin_Array)))
        {
            const UInt32 Index[] = {0,1,2,3,16, 4,5,6,7,17, 8,9,10,11,18, 12,13,14,15,19};
            ArrayObject* parrobj = static_cast<ArrayObject*>(pobj);
            for (int i=0; i < parrobj->GetSize(); ++i)
                (*filter)[Index[i]] = (float)parrobj->GetElementPtr(i)->ToNumber(penv);
        }
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

bool ColorMatrixFilterObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name == "matrix")
    {
        Render::ColorMatrixFilter* filter = static_cast<Render::ColorMatrixFilter*>(pFilter.GetPtr());
        if ( !filter || filter->GetFilterType() != Render::Filter_ColorMatrix )
            return false;
        const UInt32 Index[] = {0,1,2,3,16, 4,5,6,7,17, 8,9,10,11,18, 12,13,14,15,19};
        Ptr<ArrayObject> arrayObj = *SF_HEAP_NEW(penv->GetHeap()) ArrayObject(penv);
        arrayObj->Resize(20);
        for(int i=0;i<20;i++)
            arrayObj->SetElement(i, Value((*filter)[Index[i]]));

        val->SetAsObject(arrayObj);
        return true;
    }
    else if (name == "blurX")
    {
        val->SetNumber(GetBlurX());
        return true;
    }
    else if (name == "blurY")
    {
        val->SetNumber(GetBlurY());
        return true;
    }
    else if (name == "color")
    {
        val->SetUInt(GetColor());
        return true;
    }
    else if (name == "inner")
    {
        val->SetBool(IsInnerShadow());
        return true;
    }
    else if (name == "knockout")
    {
        val->SetBool(IsKnockOut());
        return true;
    }
    else if (name == "quality")
    {
        val->SetInt(GetPasses());
        return true;
    }
    else if (name == "strength")
    {
        val->SetNumber(GetStrength());
        return true;
    }
    return Object::GetMember(penv, name, val);
}


// --------------------------------------------------------------------

const NameFunction ColorMatrixFilterProto::FunctionTable[] = 
{
    { "clone",         &ColorMatrixFilterProto::Clone },
    { 0, 0 }
};

ColorMatrixFilterProto::ColorMatrixFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<ColorMatrixFilterObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, 0);
}

void ColorMatrixFilterProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, ColorMatrixFilter);
    ColorMatrixFilterObject* pthis = (ColorMatrixFilterObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<Object> pfilter = *fn.Env->OperatorNew(fn.Env->GetGC()->FlashFiltersPackage, 
        fn.Env->GetBuiltin(ASBuiltin_ColorMatrixFilter), 0);
    ColorMatrixFilterObject* pnew = static_cast<ColorMatrixFilterObject*>(pfilter.GetPtr());
    pnew->SetFilter(pthis->GetFilter());
    fn.Result->SetAsObject(pfilter);
}


// --------------------------------------------------------------------


ColorMatrixFilterCtorFunction::ColorMatrixFilterCtorFunction(ASStringContext* psc) :
CFunctionObject(psc, ColorMatrixFilterCtorFunction::GlobalCtor)
{
    SF_UNUSED(psc);
}

void ColorMatrixFilterCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<ColorMatrixFilterObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_ColorMatrixFilter)
        pnode = static_cast<ColorMatrixFilterObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) ColorMatrixFilterObject(fn.Env);
    fn.Result->SetAsObject(pnode.GetPtr());

    if (fn.NArgs > 0)
    {
        // process matrix
        Object* pobj = fn.Arg(0).ToObject(fn.Env);
        if (pobj && pobj->InstanceOf(fn.Env, fn.Env->GetPrototype(ASBuiltin_Array)))
        {
            const UInt32 Index[] = {0,1,2,3,16, 4,5,6,7,17, 8,9,10,11,18, 12,13,14,15,19};

            ArrayObject* parrobj = static_cast<ArrayObject*>(pobj);
            if ( pnode->GetFilter() && pnode->GetFilter()->GetFilterType() == Render::Filter_ColorMatrix )
            {
                Render::ColorMatrixFilter* filter = static_cast<Render::ColorMatrixFilter*>(pnode->GetFilter());
                for (int i=0; i < parrobj->GetSize(); ++i)
                    (*filter)[Index[i]] = (float)parrobj->GetElementPtr(i)->ToNumber(fn.Env);
            }
        }
    }

    ASStringContext* psc = fn.Env->GetSC();
    pnode->SetMemberRaw(psc, psc->CreateConstString("matrix"), Value::UNSET);
}

Object* ColorMatrixFilterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) ColorMatrixFilterObject(penv);
}

FunctionRef ColorMatrixFilterCtorFunction::Register(GlobalContext* pgc)
{
    // Register BitmapFilter here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_BitmapFilter)))
        BitmapFilterCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) ColorMatrixFilterCtorFunction(&sc));
    Ptr<ColorMatrixFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) ColorMatrixFilterProto(&sc, pgc->GetPrototype(ASBuiltin_BitmapFilter), ctor);
    pgc->SetPrototype(ASBuiltin_ColorMatrixFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_ColorMatrixFilter), Value(ctor));
    return ctor;
}

}}}; //SF::GFx::AS2

#endif  // GFX_AS2_ENABLE_FILTERS
