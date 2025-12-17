//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundMixer.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_SoundMixer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#include "AS3_Obj_Media_SoundTransform.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_media
{
    // const UInt16 SoundMixer_tito[2] = {
    //    0, 2, 
    // };
    const TypeInfo* SoundMixer_tit[4] = {
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl_media::SoundTransformTI, 
    };
    const ThunkInfo SoundMixer_ti[2] = {
        {ThunkInfo::EmptyFunc, &SoundMixer_tit[0], "bufferTime", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SoundMixer_tit[2], "soundTransform", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_media
{
    SoundMixer::SoundMixer(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::SoundMixer::SoundMixer()"
//##protect##"class_::SoundMixer::SoundMixer()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_media
{
    // const UInt16 SoundMixer_tito[3] = {
    //    0, 1, 5, 
    // };
    const TypeInfo* SoundMixer_tit[6] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::BooleanTI, &AS3::fl::int_TI, 
        NULL, 
    };
    const ThunkInfo SoundMixer_ti[3] = {
        {ThunkInfo::EmptyFunc, &SoundMixer_tit[0], "areSoundsInaccessible", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SoundMixer_tit[1], "computeSpectrum", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SoundMixer_tit[5], "stopAll", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    SoundMixer::SoundMixer(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SoundMixer::SoundMixer()"
//##protect##"ClassTraits::SoundMixer::SoundMixer()"

    }

    Pickable<Traits> SoundMixer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SoundMixer(vm, AS3::fl_media::SoundMixerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::SoundMixerCI));
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
    const TypeInfo SoundMixerTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_media::SoundMixer::InstanceType),
        3,
        0,
        2,
        0,
        "SoundMixer", "flash.media", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SoundMixerCI = {
        &SoundMixerTI,
        ClassTraits::fl_media::SoundMixer::MakeClassTraits,
        ClassTraits::fl_media::SoundMixer_ti,
        NULL,
        InstanceTraits::fl_media::SoundMixer_ti,
        NULL,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

