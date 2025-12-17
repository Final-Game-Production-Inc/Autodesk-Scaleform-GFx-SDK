//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Jan, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Sensors_Accelerometer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_sensors::Accelerometer, Instances::fl_sensors::Accelerometer::mid_mutedGet, bool> TFunc_Instances_Accelerometer_mutedGet;
typedef ThunkFunc1<Instances::fl_sensors::Accelerometer, Instances::fl_sensors::Accelerometer::mid_setRequestedUpdateInterval, const Value, Value::Number> TFunc_Instances_Accelerometer_setRequestedUpdateInterval;

template <> const TFunc_Instances_Accelerometer_mutedGet::TMethod TFunc_Instances_Accelerometer_mutedGet::Method = &Instances::fl_sensors::Accelerometer::mutedGet;
template <> const TFunc_Instances_Accelerometer_setRequestedUpdateInterval::TMethod TFunc_Instances_Accelerometer_setRequestedUpdateInterval::Method = &Instances::fl_sensors::Accelerometer::setRequestedUpdateInterval;

namespace Instances { namespace fl_sensors
{
    Accelerometer::Accelerometer(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::Accelerometer::Accelerometer()$data"
	, AccelerometerId(AccelerometerIdCount++)
//##protect##"instance::Accelerometer::Accelerometer()$data"
    {
//##protect##"instance::Accelerometer::Accelerometer()$code"
		ASVM& asvm = static_cast<ASVM&>(VMRef);
		asvm.GetMovieRoot()->AddToAccelerometers(this);
		asvm.GetMovieImpl()->RegisterAccelerometer(AccelerometerId);
//##protect##"instance::Accelerometer::Accelerometer()$code"
    }

    void Accelerometer::mutedGet(bool& result)
    {
//##protect##"instance::Accelerometer::mutedGet()"
		ASVM& asvm = static_cast<ASVM&>(VMRef);
        result = asvm.GetMovieImpl()->IsAccelerometerMuted();
//##protect##"instance::Accelerometer::mutedGet()"
    }
    void Accelerometer::setRequestedUpdateInterval(const Value& result, Value::Number interval)
    {
//##protect##"instance::Accelerometer::setRequestedUpdateInterval()"
		SF_UNUSED1(result);
		SInt32 intervalInteger;
		if (Value(interval).Convert2Int32(intervalInteger))
		{
			ASVM& asvm = static_cast<ASVM&>(VMRef);
			asvm.GetMovieImpl()->SetAccelerometerInterval(AccelerometerId, intervalInteger);
		}
//##protect##"instance::Accelerometer::setRequestedUpdateInterval()"
    }

//##protect##"instance$methods"

	int Accelerometer::AccelerometerIdCount = 0;

	Accelerometer::~Accelerometer()
	 {
		if (!VMRef.IsInAS3VMDestructor())
        {
			ASVM& asvm = static_cast<ASVM&>(VMRef);
			MovieRoot* pMovieRoot = asvm.GetMovieRoot();
			if (!pMovieRoot)
				pMovieRoot->RemoveFromAccelerometers(this);

			MovieImpl* pMovieImpl = asvm.GetMovieImpl();
			if (pMovieImpl)
				pMovieImpl->UnregisterAccelerometer(AccelerometerId);
		}
	 }

	void Accelerometer::addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
    {
        Instances::fl_events::EventDispatcher::addEventListener(result, type, listener, useCapture, priority, useWeakReference);

		ASVM& asvm = static_cast<ASVM&>(VMRef);
		if (asvm.GetStringManager().GetBuiltin(AS3Builtin_update) == type)
		{
			asvm.GetMovieRoot()->AddToAccelerometers(this);
		}
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_sensors
{
    // const UInt16 Accelerometer::tito[Accelerometer::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* Accelerometer::tit[3] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::NumberTI, 
    };
    const ThunkInfo Accelerometer::ti[Accelerometer::ThunkInfoNum] = {
        {TFunc_Instances_Accelerometer_mutedGet::Func, &Accelerometer::tit[0], "muted", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Accelerometer_setRequestedUpdateInterval::Func, &Accelerometer::tit[1], "setRequestedUpdateInterval", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Accelerometer::Accelerometer(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::Accelerometer::Accelerometer()"
//##protect##"InstanceTraits::Accelerometer::Accelerometer()"

    }

    void Accelerometer::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Accelerometer&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_sensors
{
    Accelerometer::Accelerometer(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Accelerometer::Accelerometer()"
//##protect##"class_::Accelerometer::Accelerometer()"
    }
    void Accelerometer::isSupportedGet(bool& result)
    {
//##protect##"class_::Accelerometer::isSupportedGet()"
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = asvm.GetMovieImpl()->IsAccelerometerSupported();
//##protect##"class_::Accelerometer::isSupportedGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_sensors::Accelerometer, Classes::fl_sensors::Accelerometer::mid_isSupportedGet, bool> TFunc_Classes_Accelerometer_isSupportedGet;

template <> const TFunc_Classes_Accelerometer_isSupportedGet::TMethod TFunc_Classes_Accelerometer_isSupportedGet::Method = &Classes::fl_sensors::Accelerometer::isSupportedGet;

namespace ClassTraits { namespace fl_sensors
{
    // const UInt16 Accelerometer::tito[Accelerometer::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* Accelerometer::tit[1] = {
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Accelerometer::ti[Accelerometer::ThunkInfoNum] = {
        {TFunc_Classes_Accelerometer_isSupportedGet::Func, &Accelerometer::tit[0], "isSupported", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Accelerometer::Accelerometer(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Accelerometer::Accelerometer()"
//##protect##"ClassTraits::Accelerometer::Accelerometer()"

    }

    Pickable<Traits> Accelerometer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Accelerometer(vm, AS3::fl_sensors::AccelerometerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_sensors::AccelerometerCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_sensors
{
    const TypeInfo AccelerometerTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_sensors::Accelerometer::InstanceType),
        ClassTraits::fl_sensors::Accelerometer::ThunkInfoNum,
        0,
        InstanceTraits::fl_sensors::Accelerometer::ThunkInfoNum,
        0,
        "Accelerometer", "flash.sensors", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo AccelerometerCI = {
        &AccelerometerTI,
        ClassTraits::fl_sensors::Accelerometer::MakeClassTraits,
        ClassTraits::fl_sensors::Accelerometer::ti,
        NULL,
        InstanceTraits::fl_sensors::Accelerometer::ti,
        NULL,
    };
}; // namespace fl_sensors


}}} // namespace Scaleform { namespace GFx { namespace AS3

