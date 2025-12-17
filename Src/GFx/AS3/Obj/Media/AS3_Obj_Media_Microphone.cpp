//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Microphone.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_Microphone.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Media_SoundTransform.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_media
{
    // const UInt16 Microphone_tito[16] = {
    //    0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 13, 15, 16, 18, 21, 
    // };
    const TypeInfo* Microphone_tit[23] = {
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl_media::SoundTransformTI, 
        NULL, &AS3::fl_media::SoundTransformTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue Microphone_dva[2] = {
        {Abc::CONSTANT_True, 0}, 
        {Abc::CONSTANT_Int, 4}, 
    };
    const ThunkInfo Microphone_ti[16] = {
        {ThunkInfo::EmptyFunc, &Microphone_tit[0], "activityLevel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[1], "gain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[2], "gain", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[4], "index", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[5], "muted", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[6], "name", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[7], "rate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[8], "rate", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[10], "silenceLevel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[11], "silenceTimeout", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[12], "soundTransform", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[13], "soundTransform", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[15], "useEchoSuppression", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Microphone_tit[16], "setLoopBack", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Microphone_dva[0]},
        {ThunkInfo::EmptyFunc, &Microphone_tit[18], "setSilenceLevel", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Microphone_dva[1]},
        {ThunkInfo::EmptyFunc, &Microphone_tit[21], "setUseEchoSuppression", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_media
{
    Microphone::Microphone(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Microphone::Microphone()"
//##protect##"class_::Microphone::Microphone()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_media
{
    // const UInt16 Microphone_tito[1] = {
    //    0, 
    // };
    const TypeInfo* Microphone_tit[2] = {
        &AS3::fl_media::MicrophoneTI, &AS3::fl::int_TI, 
    };
    const ThunkInfo Microphone_ti[1] = {
        {ThunkInfo::EmptyFunc, &Microphone_tit[0], "getMicrophone", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
    };

    Microphone::Microphone(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Microphone::Microphone()"
//##protect##"ClassTraits::Microphone::Microphone()"

    }

    Pickable<Traits> Microphone::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Microphone(vm, AS3::fl_media::MicrophoneCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::MicrophoneCI));
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
    const TypeInfo MicrophoneTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_media::Microphone::InstanceType),
        1,
        0,
        16,
        0,
        "Microphone", "flash.media", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo MicrophoneCI = {
        &MicrophoneTI,
        ClassTraits::fl_media::Microphone::MakeClassTraits,
        ClassTraits::fl_media::Microphone_ti,
        NULL,
        InstanceTraits::fl_media::Microphone_ti,
        NULL,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

