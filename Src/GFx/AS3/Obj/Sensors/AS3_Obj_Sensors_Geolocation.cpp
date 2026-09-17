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

#include "AS3_Obj_Sensors_Geolocation.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_sensors::Geolocation, Instances::fl_sensors::Geolocation::mid_mutedGet, bool> TFunc_Instances_Geolocation_mutedGet;
typedef ThunkFunc1<Instances::fl_sensors::Geolocation, Instances::fl_sensors::Geolocation::mid_setRequestedUpdateInterval, const Value, Value::Number> TFunc_Instances_Geolocation_setRequestedUpdateInterval;

template <> const TFunc_Instances_Geolocation_mutedGet::TMethod TFunc_Instances_Geolocation_mutedGet::Method = &Instances::fl_sensors::Geolocation::mutedGet;
template <> const TFunc_Instances_Geolocation_setRequestedUpdateInterval::TMethod TFunc_Instances_Geolocation_setRequestedUpdateInterval::Method = &Instances::fl_sensors::Geolocation::setRequestedUpdateInterval;

namespace Instances { namespace fl_sensors
{
    Geolocation::Geolocation(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::Geolocation::Geolocation()$data"
	, GeolocationId(GeolocationIdCount++)
//##protect##"instance::Geolocation::Geolocation()$data"
    {
//##protect##"instance::Geolocation::Geolocation()$code"
		ASVM& asvm = static_cast<ASVM&>(VMRef);
		asvm.GetMovieRoot()->AddToGeolocations(this);
		asvm.GetMovieImpl()->RegisterGeolocation(GeolocationId);
//##protect##"instance::Geolocation::Geolocation()$code"
    }

    void Geolocation::mutedGet(bool& result)
    {
//##protect##"instance::Geolocation::mutedGet()"
		ASVM& asvm = static_cast<ASVM&>(VMRef);
        result = asvm.GetMovieImpl()->IsGeolocationMuted();
//##protect##"instance::Geolocation::mutedGet()"
    }
    void Geolocation::setRequestedUpdateInterval(const Value& result, Value::Number interval)
    {
//##protect##"instance::Geolocation::setRequestedUpdateInterval()"
		SF_UNUSED1(result);
		SInt32 intervalInteger;
		if (Value(interval).Convert2Int32(intervalInteger))
		{
			ASVM& asvm = static_cast<ASVM&>(VMRef);
			asvm.GetMovieImpl()->SetGeolocationInterval(GeolocationId, intervalInteger);
		}
//##protect##"instance::Geolocation::setRequestedUpdateInterval()"
    }

//##protect##"instance$methods"
		
	/* Static count for geolocation ids. */
	int Geolocation::GeolocationIdCount = 0;

	Geolocation::~Geolocation()
	{
		if (!VMRef.IsInAS3VMDestructor())
        {
			ASVM& asvm = static_cast<ASVM&>(VMRef);
			
			MovieRoot* pMovieRoot = asvm.GetMovieRoot();
			if (pMovieRoot)
				pMovieRoot->RemoveFromGeolocations(this);

			MovieImpl* pMovieImpl = asvm.GetMovieImpl();
			if (pMovieImpl)
				pMovieImpl->UnregisterGeolocation(GeolocationId);
		}
	}

	void Geolocation::addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
    {
        Instances::fl_events::EventDispatcher::addEventListener(result, type, listener, useCapture, priority, useWeakReference);

		ASVM& asvm = static_cast<ASVM&>(VMRef);
		if (asvm.GetStringManager().GetBuiltin(AS3Builtin_update) == type)
		{
			asvm.GetMovieRoot()->AddToGeolocations(this);
		}
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_sensors
{
    // const UInt16 Geolocation::tito[Geolocation::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* Geolocation::tit[3] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::NumberTI, 
    };
    const ThunkInfo Geolocation::ti[Geolocation::ThunkInfoNum] = {
        {TFunc_Instances_Geolocation_mutedGet::Func, &Geolocation::tit[0], "muted", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Geolocation_setRequestedUpdateInterval::Func, &Geolocation::tit[1], "setRequestedUpdateInterval", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Geolocation::Geolocation(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::Geolocation::Geolocation()"
//##protect##"InstanceTraits::Geolocation::Geolocation()"

    }

    void Geolocation::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Geolocation&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_sensors
{
    Geolocation::Geolocation(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Geolocation::Geolocation()"
//##protect##"class_::Geolocation::Geolocation()"
    }
    void Geolocation::isSupportedGet(bool& result)
    {
//##protect##"class_::Geolocation::isSupportedGet()"
		ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = asvm.GetMovieImpl()->IsGeolocationSupported();
//##protect##"class_::Geolocation::isSupportedGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_sensors::Geolocation, Classes::fl_sensors::Geolocation::mid_isSupportedGet, bool> TFunc_Classes_Geolocation_isSupportedGet;

template <> const TFunc_Classes_Geolocation_isSupportedGet::TMethod TFunc_Classes_Geolocation_isSupportedGet::Method = &Classes::fl_sensors::Geolocation::isSupportedGet;

namespace ClassTraits { namespace fl_sensors
{
    // const UInt16 Geolocation::tito[Geolocation::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* Geolocation::tit[1] = {
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Geolocation::ti[Geolocation::ThunkInfoNum] = {
        {TFunc_Classes_Geolocation_isSupportedGet::Func, &Geolocation::tit[0], "isSupported", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Geolocation::Geolocation(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Geolocation::Geolocation()"
//##protect##"ClassTraits::Geolocation::Geolocation()"

    }

    Pickable<Traits> Geolocation::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Geolocation(vm, AS3::fl_sensors::GeolocationCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_sensors::GeolocationCI));
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
    const TypeInfo GeolocationTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_sensors::Geolocation::InstanceType),
        ClassTraits::fl_sensors::Geolocation::ThunkInfoNum,
        0,
        InstanceTraits::fl_sensors::Geolocation::ThunkInfoNum,
        0,
        "Geolocation", "flash.sensors", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo GeolocationCI = {
        &GeolocationTI,
        ClassTraits::fl_sensors::Geolocation::MakeClassTraits,
        ClassTraits::fl_sensors::Geolocation::ti,
        NULL,
        InstanceTraits::fl_sensors::Geolocation::ti,
        NULL,
    };
}; // namespace fl_sensors


}}} // namespace Scaleform { namespace GFx { namespace AS3

