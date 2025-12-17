//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_ColorMatrixFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_ColorMatrixFilter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_filters::ColorMatrixFilter, Instances::fl_filters::ColorMatrixFilter::mid_matrixGet, SPtr<Instances::fl::Array> > TFunc_Instances_ColorMatrixFilter_matrixGet;
typedef ThunkFunc1<Instances::fl_filters::ColorMatrixFilter, Instances::fl_filters::ColorMatrixFilter::mid_matrixSet, const Value, Instances::fl::Array*> TFunc_Instances_ColorMatrixFilter_matrixSet;
typedef ThunkFunc0<Instances::fl_filters::ColorMatrixFilter, Instances::fl_filters::ColorMatrixFilter::mid_clone, SPtr<Instances::fl_filters::BitmapFilter> > TFunc_Instances_ColorMatrixFilter_clone;

template <> const TFunc_Instances_ColorMatrixFilter_matrixGet::TMethod TFunc_Instances_ColorMatrixFilter_matrixGet::Method = &Instances::fl_filters::ColorMatrixFilter::matrixGet;
template <> const TFunc_Instances_ColorMatrixFilter_matrixSet::TMethod TFunc_Instances_ColorMatrixFilter_matrixSet::Method = &Instances::fl_filters::ColorMatrixFilter::matrixSet;
template <> const TFunc_Instances_ColorMatrixFilter_clone::TMethod TFunc_Instances_ColorMatrixFilter_clone::Method = &Instances::fl_filters::ColorMatrixFilter::clone;

namespace Instances { namespace fl_filters
{
    ColorMatrixFilter::ColorMatrixFilter(InstanceTraits::Traits& t)
    : Instances::fl_filters::BitmapFilter(t)
//##protect##"instance::ColorMatrixFilter::ColorMatrixFilter()$data"
//##protect##"instance::ColorMatrixFilter::ColorMatrixFilter()$data"
    {
//##protect##"instance::ColorMatrixFilter::ColorMatrixFilter()$code"
        FilterData = *SF_NEW Render::ColorMatrixFilter();
//##protect##"instance::ColorMatrixFilter::ColorMatrixFilter()$code"
    }

    void ColorMatrixFilter::matrixGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::ColorMatrixFilter::matrixGet()"
        SPtr<Instances::fl::Array> asArray = GetVM().MakeArray();
        asArray->Resize(20);
        const Render::ColorMatrixFilter* filterData = GetColorMatrixFilterData();
        for ( unsigned i = 0; i < 20; i++ )
        {
            Value v;

            // Require a conversion, because the render stores the matrix internally with
            // the multiply matrix first, and then the addition row.
            if ( i%5 == 0 && i != 0 )
                v.SetNumber((Value::Number)(*filterData)[16+i/5]);
            else
                v.SetNumber((Value::Number)(*filterData)[i/5*4+i%5]);
            asArray->Set(i, v);
        }
        result = asArray;
//##protect##"instance::ColorMatrixFilter::matrixGet()"
    }
    void ColorMatrixFilter::matrixSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::ColorMatrixFilter::matrixSet()"
        if ( value == 0 )
            return;
        SF_UNUSED(result);
        Render::ColorMatrixFilter* filterData = GetColorMatrixFilterData();
        for ( unsigned i = 0; i < value->GetSize(); ++i)
        {
            Value::Number n;
            if ( !value->At(i).Convert2Number(n) )
                return;

            if ( i >= 20 )
                continue;

            // Require a conversion, because the render stores the matrix internally with
            // the multiply matrix first, and then the addition row.
            if ( i%5 == 4 )
                (*filterData)[16+i/5] = (float)n / 255.0f;
            else
                (*filterData)[i/5*4+i%5] = (float)n;
        }
//##protect##"instance::ColorMatrixFilter::matrixSet()"
    }
    void ColorMatrixFilter::clone(SPtr<Instances::fl_filters::BitmapFilter>& result)
    {
//##protect##"instance::ColorMatrixFilter::clone()"

        InstanceTraits::fl_filters::ColorMatrixFilter & itr = static_cast<InstanceTraits::fl_filters::ColorMatrixFilter&>(GetTraits());
        Pickable<ColorMatrixFilter> r = itr.MakeInstance(itr);

        // Not performance efficient way of cloning a matrix.
        SPtr<Instances::fl::Array> matrix;
        Value tempResult;
        matrixGet(matrix);
        r->matrixSet(tempResult, static_cast<Instances::fl::Array*>(matrix.GetPtr()));
        
        result = r;
//##protect##"instance::ColorMatrixFilter::clone()"
    }

    SPtr<Instances::fl::Array> ColorMatrixFilter::matrixGet()
    {
        SPtr<Instances::fl::Array> result;
        matrixGet(result);
        return result;
    }
    SPtr<Instances::fl_filters::BitmapFilter> ColorMatrixFilter::clone()
    {
        SPtr<Instances::fl_filters::BitmapFilter> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void ColorMatrixFilter::AS3Constructor(unsigned argc, const Value* argv)
    {
        if ( argc >= 2 )
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("flash.filters::ColorMatrixFilter()") SF_DEBUG_ARG(0) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));

        if (argc > 0 && IsValidArrayObject(argv[0]))
        {
            Value result;
            matrixSet(result, static_cast<Instances::fl::Array*>(argv[0].GetObject()));
        }
    }

    bool ColorMatrixFilter::IsValidArrayObject(const Value& v) const
    {
        bool result = false;

        if (v.IsObject() && v.GetObject())
        {
            Traits& tr = v.GetObject()->GetTraits();

            if (tr.GetTraitsType() == Traits_Array && tr.IsInstanceTraits())
                result = true;
        }

        return result;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    // const UInt16 ColorMatrixFilter::tito[ColorMatrixFilter::ThunkInfoNum] = {
    //    0, 1, 3, 
    // };
    const TypeInfo* ColorMatrixFilter::tit[4] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl_filters::BitmapFilterTI, 
    };
    const ThunkInfo ColorMatrixFilter::ti[ColorMatrixFilter::ThunkInfoNum] = {
        {TFunc_Instances_ColorMatrixFilter_matrixGet::Func, &ColorMatrixFilter::tit[0], "matrix", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ColorMatrixFilter_matrixSet::Func, &ColorMatrixFilter::tit[1], "matrix", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ColorMatrixFilter_clone::Func, &ColorMatrixFilter::tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    ColorMatrixFilter::ColorMatrixFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"InstanceTraits::ColorMatrixFilter::ColorMatrixFilter()"
//##protect##"InstanceTraits::ColorMatrixFilter::ColorMatrixFilter()"

    }

    void ColorMatrixFilter::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ColorMatrixFilter&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filters
{

    ColorMatrixFilter::ColorMatrixFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"ClassTraits::ColorMatrixFilter::ColorMatrixFilter()"
//##protect##"ClassTraits::ColorMatrixFilter::ColorMatrixFilter()"

    }

    Pickable<Traits> ColorMatrixFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ColorMatrixFilter(vm, AS3::fl_filters::ColorMatrixFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::ColorMatrixFilterCI));
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
    const TypeInfo ColorMatrixFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::ColorMatrixFilter::InstanceType),
        0,
        0,
        InstanceTraits::fl_filters::ColorMatrixFilter::ThunkInfoNum,
        0,
        "ColorMatrixFilter", "flash.filters", &fl_filters::BitmapFilterTI,
        TypeInfo::None
    };

    const ClassInfo ColorMatrixFilterCI = {
        &ColorMatrixFilterTI,
        ClassTraits::fl_filters::ColorMatrixFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filters::ColorMatrixFilter::ti,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

