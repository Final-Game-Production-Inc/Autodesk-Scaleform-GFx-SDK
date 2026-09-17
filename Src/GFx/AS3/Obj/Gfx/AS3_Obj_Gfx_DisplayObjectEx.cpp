//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   May, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_DisplayObjectEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    DisplayObjectEx::DisplayObjectEx(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::DisplayObjectEx::DisplayObjectEx()"
//##protect##"class_::DisplayObjectEx::DisplayObjectEx()"
    }
    void DisplayObjectEx::disableBatching(const Value& result, Instances::fl_display::DisplayObject* o, bool b)
    {
//##protect##"class_::DisplayObjectEx::disableBatching()"
        SF_UNUSED3(result, o, b);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            o->pDispObj->DisableBatching(b);
        }
//##protect##"class_::DisplayObjectEx::disableBatching()"
    }
    void DisplayObjectEx::isBatchingDisabled(bool& result, Instances::fl_display::DisplayObject* o)
    {
//##protect##"class_::DisplayObjectEx::isBatchingDisabled()"
        SF_UNUSED2(result, o);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            result = o->pDispObj->IsBatchingDisabled();
        }
//##protect##"class_::DisplayObjectEx::isBatchingDisabled()"
    }
    void DisplayObjectEx::setRendererString(const Value& result, Instances::fl_display::DisplayObject* o, const ASString& s)
    {
//##protect##"class_::DisplayObjectEx::setRendererString()"
        SF_UNUSED3(result, o, s);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            o->pDispObj->SetRendererString(s.ToCStr());
        }
//##protect##"class_::DisplayObjectEx::setRendererString()"
    }
    void DisplayObjectEx::getRendererString(ASString& result, Instances::fl_display::DisplayObject* o)
    {
//##protect##"class_::DisplayObjectEx::getRendererString()"
        SF_UNUSED2(result, o);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            const char* str = o->pDispObj->GetRendererString();
            if (str)
                result = str;
            else
                result = ASString(GetVM().GetStringManager().CreateEmptyString());
        }
//##protect##"class_::DisplayObjectEx::getRendererString()"
    }
    void DisplayObjectEx::setRendererFloat(const Value& result, Instances::fl_display::DisplayObject* o, Value::Number f)
    {
//##protect##"class_::DisplayObjectEx::setRendererFloat()"
        SF_UNUSED3(result, o, f);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            o->pDispObj->SetRendererFloat(float(f));
        }
//##protect##"class_::DisplayObjectEx::setRendererFloat()"
    }
    void DisplayObjectEx::getRendererFloat(Value::Number& result, Instances::fl_display::DisplayObject* o)
    {
//##protect##"class_::DisplayObjectEx::getRendererFloat()"
        SF_UNUSED2(result, o);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            result = Value::Number(o->pDispObj->GetRendererFloat());
        }
//##protect##"class_::DisplayObjectEx::getRendererFloat()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"
    void DisplayObjectEx::setInvertedMask(const Value& result, Instances::fl_display::DisplayObject* o, bool b) {
        SF_UNUSED3(result, o, b);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            o->pDispObj->SetInvertedMask(b);
        }
    }
    void DisplayObjectEx::getInvertedMask(bool& result, Instances::fl_display::DisplayObject* o) {
        SF_UNUSED2(result, o);
        if (o)
        {
            SF_ASSERT(o->pDispObj);
            result = o->pDispObj->GetInvertedMask();
        }
    }
}} // namespace Classes

typedef ThunkFunc2<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_disableBatching, const Value, Instances::fl_display::DisplayObject*, bool> TFunc_Classes_DisplayObjectEx_disableBatching;
typedef ThunkFunc1<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_isBatchingDisabled, bool, Instances::fl_display::DisplayObject*> TFunc_Classes_DisplayObjectEx_isBatchingDisabled;
typedef ThunkFunc2<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_setRendererString, const Value, Instances::fl_display::DisplayObject*, const ASString&> TFunc_Classes_DisplayObjectEx_setRendererString;
typedef ThunkFunc1<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_getRendererString, ASString, Instances::fl_display::DisplayObject*> TFunc_Classes_DisplayObjectEx_getRendererString;
typedef ThunkFunc2<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_setRendererFloat, const Value, Instances::fl_display::DisplayObject*, Value::Number> TFunc_Classes_DisplayObjectEx_setRendererFloat;
typedef ThunkFunc1<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_getRendererFloat, Value::Number, Instances::fl_display::DisplayObject*> TFunc_Classes_DisplayObjectEx_getRendererFloat;

// 4.5.32 Update Of The AS3 API
typedef ThunkFunc2<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_setInvertedMask, const Value, Instances::fl_display::DisplayObject*, bool> TFunc_Classes_DisplayObjectEx_setInvertedMask;
typedef ThunkFunc1<Classes::fl_gfx::DisplayObjectEx, Classes::fl_gfx::DisplayObjectEx::mid_getInvertedMask, bool, Instances::fl_display::DisplayObject*> TFunc_Classes_DisplayObjectEx_getInvertedMask;

template <> const TFunc_Classes_DisplayObjectEx_disableBatching::TMethod TFunc_Classes_DisplayObjectEx_disableBatching::Method = &Classes::fl_gfx::DisplayObjectEx::disableBatching;
template <> const TFunc_Classes_DisplayObjectEx_isBatchingDisabled::TMethod TFunc_Classes_DisplayObjectEx_isBatchingDisabled::Method = &Classes::fl_gfx::DisplayObjectEx::isBatchingDisabled;
template <> const TFunc_Classes_DisplayObjectEx_setRendererString::TMethod TFunc_Classes_DisplayObjectEx_setRendererString::Method = &Classes::fl_gfx::DisplayObjectEx::setRendererString;
template <> const TFunc_Classes_DisplayObjectEx_getRendererString::TMethod TFunc_Classes_DisplayObjectEx_getRendererString::Method = &Classes::fl_gfx::DisplayObjectEx::getRendererString;
template <> const TFunc_Classes_DisplayObjectEx_setRendererFloat::TMethod TFunc_Classes_DisplayObjectEx_setRendererFloat::Method = &Classes::fl_gfx::DisplayObjectEx::setRendererFloat;
template <> const TFunc_Classes_DisplayObjectEx_getRendererFloat::TMethod TFunc_Classes_DisplayObjectEx_getRendererFloat::Method = &Classes::fl_gfx::DisplayObjectEx::getRendererFloat;

// Scaleform 4.5.32 Update Of The AS3 API
template <> const TFunc_Classes_DisplayObjectEx_setInvertedMask::TMethod TFunc_Classes_DisplayObjectEx_setInvertedMask::Method = &Classes::fl_gfx::DisplayObjectEx::setInvertedMask;
template <> const TFunc_Classes_DisplayObjectEx_getInvertedMask::TMethod TFunc_Classes_DisplayObjectEx_getInvertedMask::Method = &Classes::fl_gfx::DisplayObjectEx::getInvertedMask;


namespace ClassTraits { namespace fl_gfx
{
    // const UInt16 DisplayObjectEx::tito[DisplayObjectEx::ThunkInfoNum] = {
    //    0, 3, 5, 8, 10, 13, 16, 18
    // };
    const TypeInfo* DisplayObjectEx::tit[20] = {
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, &AS3::fl_display::DisplayObjectTI, 

        // For Adapt Scaelform 4.5.32 API,
        // So, DisplayObjectEx::tit Should Be 20 Is Right.
        // OtherWise, There's Has A Bug To Fail To Load.
        NULL, &AS3::fl_display::DisplayObjectTI,& AS3::fl::BooleanTI,
        & AS3::fl::BooleanTI,& AS3::fl_display::DisplayObjectTI,
    };
    const ThunkInfo DisplayObjectEx::ti[DisplayObjectEx::ThunkInfoNum] = {
        {TFunc_Classes_DisplayObjectEx_disableBatching::Func, &DisplayObjectEx::tit[0], "disableBatching", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_isBatchingDisabled::Func, &DisplayObjectEx::tit[3], "isBatchingDisabled", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_setRendererString::Func, &DisplayObjectEx::tit[5], "setRendererString", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_getRendererString::Func, &DisplayObjectEx::tit[8], "getRendererString", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_setRendererFloat::Func, &DisplayObjectEx::tit[10], "setRendererFloat", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_getRendererFloat::Func, &DisplayObjectEx::tit[13], "getRendererFloat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},

        // 4.5.32 API Usued
        {TFunc_Classes_DisplayObjectEx_setInvertedMask::Func, &DisplayObjectEx::tit[16], "setInvertedMask", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_DisplayObjectEx_getInvertedMask::Func, &DisplayObjectEx::tit[18], "getInvertedMask", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL}
    };

    DisplayObjectEx::DisplayObjectEx(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::DisplayObjectEx::DisplayObjectEx()"
//##protect##"ClassTraits::DisplayObjectEx::DisplayObjectEx()"

    }

    Pickable<Traits> DisplayObjectEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DisplayObjectEx(vm, AS3::fl_gfx::DisplayObjectExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::DisplayObjectExCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo DisplayObjectExTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_gfx::DisplayObjectEx::InstanceType),
        ClassTraits::fl_gfx::DisplayObjectEx::ThunkInfoNum,
        0,
        0,
        0,
        "DisplayObjectEx", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DisplayObjectExCI = {
        &DisplayObjectExTI,
        ClassTraits::fl_gfx::DisplayObjectEx::MakeClassTraits,
        ClassTraits::fl_gfx::DisplayObjectEx::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

