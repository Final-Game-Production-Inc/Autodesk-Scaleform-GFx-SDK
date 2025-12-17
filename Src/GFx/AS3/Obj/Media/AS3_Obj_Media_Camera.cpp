//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Camera.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_Camera.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_media
{
    // const UInt16 Camera_tito[19] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 23, 26, 
    // };
    const TypeInfo* Camera_tit[29] = {
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::NumberTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
    };
    const Abc::ConstValue Camera_dva[2] = {
        {Abc::CONSTANT_True, 0}, 
        {Abc::CONSTANT_Int, 3}, 
    };
    const ThunkInfo Camera_ti[19] = {
        {ThunkInfo::EmptyFunc, &Camera_tit[0], "activityLevel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[1], "bandwidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[2], "currentFPS", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[3], "fps", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[4], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[5], "index", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[6], "keyFrameInterval", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[7], "loopback", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[8], "motionLevel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[9], "motionTimeout", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[10], "muted", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[11], "name", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[12], "quality", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[13], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[14], "setKeyFrameInterval", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[16], "setLoopback", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Camera_tit[18], "setMode", NULL, Abc::NS_Public, CT_Method, 3, 4, 0, 1, &Camera_dva[0]},
        {ThunkInfo::EmptyFunc, &Camera_tit[23], "setMotionLevel", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Camera_dva[1]},
        {ThunkInfo::EmptyFunc, &Camera_tit[26], "setQuality", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_media
{
    Camera::Camera(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Camera::Camera()"
//##protect##"class_::Camera::Camera()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_media
{
    // const UInt16 Camera_tito[1] = {
    //    0, 
    // };
    const TypeInfo* Camera_tit[2] = {
        &AS3::fl_media::CameraTI, &AS3::fl::StringTI, 
    };
    const Abc::ConstValue Camera_dva[1] = {
        {Abc::CONSTANT_Null, 0}, 
    };
    const ThunkInfo Camera_ti[1] = {
        {ThunkInfo::EmptyFunc, &Camera_tit[0], "getCamera", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Camera_dva[0]},
    };

    Camera::Camera(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Camera::Camera()"
//##protect##"ClassTraits::Camera::Camera()"

    }

    Pickable<Traits> Camera::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Camera(vm, AS3::fl_media::CameraCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::CameraCI));
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
    const TypeInfo CameraTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_media::Camera::InstanceType),
        1,
        0,
        19,
        0,
        "Camera", "flash.media", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo CameraCI = {
        &CameraTI,
        ClassTraits::fl_media::Camera::MakeClassTraits,
        ClassTraits::fl_media::Camera_ti,
        NULL,
        InstanceTraits::fl_media::Camera_ti,
        NULL,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

