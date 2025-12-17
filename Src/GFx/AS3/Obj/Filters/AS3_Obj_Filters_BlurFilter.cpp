//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BlurFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_BlurFilter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_blurXGet, Value::Number> TFunc_Instances_BlurFilter_blurXGet;
typedef ThunkFunc1<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_blurXSet, const Value, Value::Number> TFunc_Instances_BlurFilter_blurXSet;
typedef ThunkFunc0<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_blurYGet, Value::Number> TFunc_Instances_BlurFilter_blurYGet;
typedef ThunkFunc1<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_blurYSet, const Value, Value::Number> TFunc_Instances_BlurFilter_blurYSet;
typedef ThunkFunc0<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_qualityGet, SInt32> TFunc_Instances_BlurFilter_qualityGet;
typedef ThunkFunc1<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_qualitySet, const Value, SInt32> TFunc_Instances_BlurFilter_qualitySet;
typedef ThunkFunc0<Instances::fl_filters::BlurFilter, Instances::fl_filters::BlurFilter::mid_clone, SPtr<Instances::fl_filters::BitmapFilter> > TFunc_Instances_BlurFilter_clone;

template <> const TFunc_Instances_BlurFilter_blurXGet::TMethod TFunc_Instances_BlurFilter_blurXGet::Method = &Instances::fl_filters::BlurFilter::blurXGet;
template <> const TFunc_Instances_BlurFilter_blurXSet::TMethod TFunc_Instances_BlurFilter_blurXSet::Method = &Instances::fl_filters::BlurFilter::blurXSet;
template <> const TFunc_Instances_BlurFilter_blurYGet::TMethod TFunc_Instances_BlurFilter_blurYGet::Method = &Instances::fl_filters::BlurFilter::blurYGet;
template <> const TFunc_Instances_BlurFilter_blurYSet::TMethod TFunc_Instances_BlurFilter_blurYSet::Method = &Instances::fl_filters::BlurFilter::blurYSet;
template <> const TFunc_Instances_BlurFilter_qualityGet::TMethod TFunc_Instances_BlurFilter_qualityGet::Method = &Instances::fl_filters::BlurFilter::qualityGet;
template <> const TFunc_Instances_BlurFilter_qualitySet::TMethod TFunc_Instances_BlurFilter_qualitySet::Method = &Instances::fl_filters::BlurFilter::qualitySet;
template <> const TFunc_Instances_BlurFilter_clone::TMethod TFunc_Instances_BlurFilter_clone::Method = &Instances::fl_filters::BlurFilter::clone;

namespace Instances { namespace fl_filters
{
    BlurFilter::BlurFilter(InstanceTraits::Traits& t)
    : Instances::fl_filters::BitmapFilter(t)
//##protect##"instance::BlurFilter::BlurFilter()$data"
//##protect##"instance::BlurFilter::BlurFilter()$data"
    {
//##protect##"instance::BlurFilter::BlurFilter()$code"
        FilterData = *SF_NEW Render::BlurFilter();
//##protect##"instance::BlurFilter::BlurFilter()$code"
    }

    void BlurFilter::blurXGet(Value::Number& result)
    {
//##protect##"instance::BlurFilter::blurXGet()"
        result = TwipsToPixels(GetBlurFilterData()->GetParams().BlurX);
//##protect##"instance::BlurFilter::blurXGet()"
    }
    void BlurFilter::blurXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::BlurFilter::blurXSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetBlurFilterData()->GetParams().BlurX = PixelsToTwips((float)value);
//##protect##"instance::BlurFilter::blurXSet()"
    }
    void BlurFilter::blurYGet(Value::Number& result)
    {
//##protect##"instance::BlurFilter::blurYGet()"
        result = TwipsToPixels(GetBlurFilterData()->GetParams().BlurY);
//##protect##"instance::BlurFilter::blurYGet()"
    }
    void BlurFilter::blurYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::BlurFilter::blurYSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetBlurFilterData()->GetParams().BlurY = PixelsToTwips((float)value);
//##protect##"instance::BlurFilter::blurYSet()"
    }
    void BlurFilter::qualityGet(SInt32& result)
    {
//##protect##"instance::BlurFilter::qualityGet()"
        result = (GetBlurFilterData()->GetParams().Passes);
//##protect##"instance::BlurFilter::qualityGet()"
    }
    void BlurFilter::qualitySet(const Value& result, SInt32 value)
    {
//##protect##"instance::BlurFilter::qualitySet()"
        SF_UNUSED(result);
        GetBlurFilterData()->GetParams().Passes = Alg::Clamp<unsigned>(value, 0, 15);
//##protect##"instance::BlurFilter::qualitySet()"
    }
    void BlurFilter::clone(SPtr<Instances::fl_filters::BitmapFilter>& result)
    {
//##protect##"instance::BlurFilter::clone()"
        InstanceTraits::fl_filters::BlurFilter& itr = static_cast<InstanceTraits::fl_filters::BlurFilter&>(GetTraits());
        Pickable<BlurFilter> r = itr.MakeInstance(itr);

        Value::Number blurX, blurY;
        SInt32 qual;
        blurXGet(blurX);
        blurYGet(blurY);
        qualityGet(qual);

        Value tempResult;
        r->blurXSet(tempResult, blurX);
        r->blurYSet(tempResult, blurY);
        r->qualitySet(tempResult, qual);
        result = r;
//##protect##"instance::BlurFilter::clone()"
    }

    SPtr<Instances::fl_filters::BitmapFilter> BlurFilter::clone()
    {
        SPtr<Instances::fl_filters::BitmapFilter> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void BlurFilter::AS3Constructor(unsigned argc, const Value* argv)
    {
        Value result;
        Value::Number bx(4), by(4);
        SInt32 qual(1);

        if ( argc >= 4 )
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("flash.filters::BlurFilter()") SF_DEBUG_ARG(0) SF_DEBUG_ARG(3) SF_DEBUG_ARG(argc)));

        if ( argc >= 1 && !argv[0].Convert2Number(bx)) return;
        if ( argc >= 2 && !argv[1].Convert2Number(by)) return;
        if ( argc >= 3 && !argv[2].Convert2Int32(qual)) return;

        blurXSet(result, bx);
        blurYSet(result, by);
        qualitySet(result, (SInt16)qual);
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    // const UInt16 BlurFilter::tito[BlurFilter::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 
    // };
    const TypeInfo* BlurFilter::tit[10] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl_filters::BitmapFilterTI, 
    };
    const ThunkInfo BlurFilter::ti[BlurFilter::ThunkInfoNum] = {
        {TFunc_Instances_BlurFilter_blurXGet::Func, &BlurFilter::tit[0], "blurX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_blurXSet::Func, &BlurFilter::tit[1], "blurX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_blurYGet::Func, &BlurFilter::tit[3], "blurY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_blurYSet::Func, &BlurFilter::tit[4], "blurY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_qualityGet::Func, &BlurFilter::tit[6], "quality", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_qualitySet::Func, &BlurFilter::tit[7], "quality", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BlurFilter_clone::Func, &BlurFilter::tit[9], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    BlurFilter::BlurFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"InstanceTraits::BlurFilter::BlurFilter()"
//##protect##"InstanceTraits::BlurFilter::BlurFilter()"

    }

    void BlurFilter::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<BlurFilter&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filters
{

    BlurFilter::BlurFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"ClassTraits::BlurFilter::BlurFilter()"
//##protect##"ClassTraits::BlurFilter::BlurFilter()"

    }

    Pickable<Traits> BlurFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) BlurFilter(vm, AS3::fl_filters::BlurFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::BlurFilterCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_filters
{
    const TypeInfo BlurFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::BlurFilter::InstanceType),
        0,
        0,
        InstanceTraits::fl_filters::BlurFilter::ThunkInfoNum,
        0,
        "BlurFilter", "flash.filters", &fl_filters::BitmapFilterTI,
        TypeInfo::None
    };

    const ClassInfo BlurFilterCI = {
        &BlurFilterTI,
        ClassTraits::fl_filters::BlurFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filters::BlurFilter::ti,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

