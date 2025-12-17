//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundTransform.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_SoundTransform.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_leftToLeftGet, Value::Number> TFunc_Instances_SoundTransform_leftToLeftGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_leftToLeftSet, const Value, Value::Number> TFunc_Instances_SoundTransform_leftToLeftSet;
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_leftToRightGet, Value::Number> TFunc_Instances_SoundTransform_leftToRightGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_leftToRightSet, const Value, Value::Number> TFunc_Instances_SoundTransform_leftToRightSet;
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_panGet, Value::Number> TFunc_Instances_SoundTransform_panGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_panSet, const Value, Value::Number> TFunc_Instances_SoundTransform_panSet;
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_rightToLeftGet, Value::Number> TFunc_Instances_SoundTransform_rightToLeftGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_rightToLeftSet, const Value, Value::Number> TFunc_Instances_SoundTransform_rightToLeftSet;
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_rightToRightGet, Value::Number> TFunc_Instances_SoundTransform_rightToRightGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_rightToRightSet, const Value, Value::Number> TFunc_Instances_SoundTransform_rightToRightSet;
typedef ThunkFunc0<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_volumeGet, Value::Number> TFunc_Instances_SoundTransform_volumeGet;
typedef ThunkFunc1<Instances::fl_media::SoundTransform, Instances::fl_media::SoundTransform::mid_volumeSet, const Value, Value::Number> TFunc_Instances_SoundTransform_volumeSet;

template <> const TFunc_Instances_SoundTransform_leftToLeftGet::TMethod TFunc_Instances_SoundTransform_leftToLeftGet::Method = &Instances::fl_media::SoundTransform::leftToLeftGet;
template <> const TFunc_Instances_SoundTransform_leftToLeftSet::TMethod TFunc_Instances_SoundTransform_leftToLeftSet::Method = &Instances::fl_media::SoundTransform::leftToLeftSet;
template <> const TFunc_Instances_SoundTransform_leftToRightGet::TMethod TFunc_Instances_SoundTransform_leftToRightGet::Method = &Instances::fl_media::SoundTransform::leftToRightGet;
template <> const TFunc_Instances_SoundTransform_leftToRightSet::TMethod TFunc_Instances_SoundTransform_leftToRightSet::Method = &Instances::fl_media::SoundTransform::leftToRightSet;
template <> const TFunc_Instances_SoundTransform_panGet::TMethod TFunc_Instances_SoundTransform_panGet::Method = &Instances::fl_media::SoundTransform::panGet;
template <> const TFunc_Instances_SoundTransform_panSet::TMethod TFunc_Instances_SoundTransform_panSet::Method = &Instances::fl_media::SoundTransform::panSet;
template <> const TFunc_Instances_SoundTransform_rightToLeftGet::TMethod TFunc_Instances_SoundTransform_rightToLeftGet::Method = &Instances::fl_media::SoundTransform::rightToLeftGet;
template <> const TFunc_Instances_SoundTransform_rightToLeftSet::TMethod TFunc_Instances_SoundTransform_rightToLeftSet::Method = &Instances::fl_media::SoundTransform::rightToLeftSet;
template <> const TFunc_Instances_SoundTransform_rightToRightGet::TMethod TFunc_Instances_SoundTransform_rightToRightGet::Method = &Instances::fl_media::SoundTransform::rightToRightGet;
template <> const TFunc_Instances_SoundTransform_rightToRightSet::TMethod TFunc_Instances_SoundTransform_rightToRightSet::Method = &Instances::fl_media::SoundTransform::rightToRightSet;
template <> const TFunc_Instances_SoundTransform_volumeGet::TMethod TFunc_Instances_SoundTransform_volumeGet::Method = &Instances::fl_media::SoundTransform::volumeGet;
template <> const TFunc_Instances_SoundTransform_volumeSet::TMethod TFunc_Instances_SoundTransform_volumeSet::Method = &Instances::fl_media::SoundTransform::volumeSet;

namespace Instances { namespace fl_media
{
    SoundTransform::SoundTransform(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::SoundTransform::SoundTransform()$data"
//##protect##"instance::SoundTransform::SoundTransform()$data"
    {
//##protect##"instance::SoundTransform::SoundTransform()$code"
#ifdef GFX_ENABLE_SOUND
        Volume = 1;
        Pan = 0;
#endif
//##protect##"instance::SoundTransform::SoundTransform()$code"
    }

    void SoundTransform::leftToLeftGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::leftToLeftGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::leftToLeftGet()");
//##protect##"instance::SoundTransform::leftToLeftGet()"
    }
    void SoundTransform::leftToLeftSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::leftToLeftSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::leftToLeftSet()");
//##protect##"instance::SoundTransform::leftToLeftSet()"
    }
    void SoundTransform::leftToRightGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::leftToRightGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::leftToRightGet()");
//##protect##"instance::SoundTransform::leftToRightGet()"
    }
    void SoundTransform::leftToRightSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::leftToRightSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::leftToRightSet()");
//##protect##"instance::SoundTransform::leftToRightSet()"
    }
    void SoundTransform::panGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::panGet()"
#ifdef GFX_ENABLE_SOUND
        result = Pan;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::panGet()");
#endif
//##protect##"instance::SoundTransform::panGet()"
    }
    void SoundTransform::panSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::panSet()"
#ifdef GFX_ENABLE_SOUND
        SF_UNUSED(result);
        Pan = value;
#else
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::panSet()");
#endif
//##protect##"instance::SoundTransform::panSet()"
    }
    void SoundTransform::rightToLeftGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::rightToLeftGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::rightToLeftGet()");
//##protect##"instance::SoundTransform::rightToLeftGet()"
    }
    void SoundTransform::rightToLeftSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::rightToLeftSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::rightToLeftSet()");
//##protect##"instance::SoundTransform::rightToLeftSet()"
    }
    void SoundTransform::rightToRightGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::rightToRightGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::rightToRightGet()");
//##protect##"instance::SoundTransform::rightToRightGet()"
    }
    void SoundTransform::rightToRightSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::rightToRightSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::rightToRightSet()");
//##protect##"instance::SoundTransform::rightToRightSet()"
    }
    void SoundTransform::volumeGet(Value::Number& result)
    {
//##protect##"instance::SoundTransform::volumeGet()"
#ifdef GFX_ENABLE_SOUND
        result = Volume;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SoundTransform::volumeGet()");
#endif
//##protect##"instance::SoundTransform::volumeGet()"
    }
    void SoundTransform::volumeSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SoundTransform::volumeSet()"
#ifdef GFX_ENABLE_SOUND
        SF_UNUSED(result);
        Volume = value;
#else
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SoundTransform::volumeSet()");
#endif
//##protect##"instance::SoundTransform::volumeSet()"
    }

//##protect##"instance$methods"
#ifdef GFX_ENABLE_SOUND
    void SoundTransform::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1)
        {
            argv[0].Convert2Number(Volume).DoNotCheck();
            if (argc >= 2)
                argv[1].Convert2Number(Pan).DoNotCheck();
        }
    }
#endif
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    // const UInt16 SoundTransform::tito[SoundTransform::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 
    // };
    const TypeInfo* SoundTransform::tit[18] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
    };
    const ThunkInfo SoundTransform::ti[SoundTransform::ThunkInfoNum] = {
        {TFunc_Instances_SoundTransform_leftToLeftGet::Func, &SoundTransform::tit[0], "leftToLeft", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_leftToLeftSet::Func, &SoundTransform::tit[1], "leftToLeft", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_leftToRightGet::Func, &SoundTransform::tit[3], "leftToRight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_leftToRightSet::Func, &SoundTransform::tit[4], "leftToRight", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_panGet::Func, &SoundTransform::tit[6], "pan", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_panSet::Func, &SoundTransform::tit[7], "pan", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_rightToLeftGet::Func, &SoundTransform::tit[9], "rightToLeft", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_rightToLeftSet::Func, &SoundTransform::tit[10], "rightToLeft", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_rightToRightGet::Func, &SoundTransform::tit[12], "rightToRight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_rightToRightSet::Func, &SoundTransform::tit[13], "rightToRight", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_volumeGet::Func, &SoundTransform::tit[15], "volume", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SoundTransform_volumeSet::Func, &SoundTransform::tit[16], "volume", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

    SoundTransform::SoundTransform(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::SoundTransform::SoundTransform()"
//##protect##"InstanceTraits::SoundTransform::SoundTransform()"

    }

    void SoundTransform::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<SoundTransform&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_media
{

    SoundTransform::SoundTransform(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SoundTransform::SoundTransform()"
//##protect##"ClassTraits::SoundTransform::SoundTransform()"

    }

    Pickable<Traits> SoundTransform::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SoundTransform(vm, AS3::fl_media::SoundTransformCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::SoundTransformCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_media
{
    const TypeInfo SoundTransformTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_media::SoundTransform::InstanceType),
        0,
        0,
        InstanceTraits::fl_media::SoundTransform::ThunkInfoNum,
        0,
        "SoundTransform", "flash.media", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SoundTransformCI = {
        &SoundTransformTI,
        ClassTraits::fl_media::SoundTransform::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_media::SoundTransform::ti,
        NULL,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

