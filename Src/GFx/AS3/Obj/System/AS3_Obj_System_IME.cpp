//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_IME.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_IME.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
#include "GFx/IME/GFx_IMEManager.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_system
{
    IME::IME(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::IME::IME()"
//##protect##"class_::IME::IME()"
    }
    void IME::conversionModeSet(const Value& result, const ASString& value)
    {
//##protect##"class_::IME::conversionModeSet()"
#ifndef SF_NO_IME_SUPPORT		
		SF_UNUSED(result);
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl(); 
		Ptr<IMEManagerBase> pimeManager = pmovieImpl->GetIMEManager();
		UInt32 fdwConversion = 0;

		if(pimeManager)
		{
			if (!SFstrcmp(value.ToCStr(), "ALPHANUMERIC_FULL"))
			{
				fdwConversion = 0;
			}
			if (!SFstrcmp(value.ToCStr(), "ALPHANUMERIC_HALF"))
			{
				fdwConversion = 0x01;
			}
			if (!SFstrcmp(value.ToCStr(), "JAPANESE_HIRAGANA"))
			{
				fdwConversion = 0x04;
			}
			if (!SFstrcmp(value.ToCStr(), "JAPANESE_KATAKANA_FULL"))
			{
				fdwConversion = 0x08;
			}
			if (!SFstrcmp(value.ToCStr(), "JAPANESE_KATAKANA_HALF"))
			{
				fdwConversion = 0x016;
			}
			pimeManager->SetConversionMode(fdwConversion);
		}
#else
		SF_UNUSED2(result, value);
#endif
//##protect##"class_::IME::conversionModeSet()"
    }
    void IME::conversionModeGet(ASString& result)
    {
//##protect##"class_::IME::conversionModeGet()"
#ifndef SF_NO_IME_SUPPORT		
		// Try to get IME Manager.
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl(); 
		Ptr<IMEManagerBase> pimeManager = pmovieImpl->GetIMEManager();
		result = "UNKNOWN";

		if(pimeManager)
		{
			result = pimeManager->GetConversionMode();
		}
#else
		SF_UNUSED(result);
#endif
//##protect##"class_::IME::conversionModeGet()"
    }
    void IME::enabledGet(bool& result)
    {
//##protect##"class_::IME::enabledGet()"
#ifndef SF_NO_IME_SUPPORT
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl(); 
		Ptr<IMEManagerBase> pimeManager = pmovieImpl->GetIMEManager();
		if(pimeManager)
		{
			result = pimeManager->GetEnabled();
		}
#else
		SF_UNUSED(result);
#endif
//##protect##"class_::IME::enabledGet()"
    }
    void IME::enabledSet(const Value& result, bool value)
    {
//##protect##"class_::IME::enabledSet()"
#ifndef SF_NO_IME_SUPPORT
		SF_UNUSED(result);
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl(); 
		Ptr<IMEManagerBase> pimeManager = pmovieImpl->GetIMEManager();
		if(pimeManager)
		{
			pimeManager->SetEnabled(value);
		}
#else
		SF_UNUSED2(result, value);
#endif
//##protect##"class_::IME::enabledSet()"
    }
    void IME::doConversion(const Value& result)
    {
//##protect##"class_::IME::doConversion()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::IME::doConversion()");
//##protect##"class_::IME::doConversion()"
    }
    void IME::setCompositionString(const Value& result, const ASString& composition)
    {
//##protect##"class_::IME::setCompositionString()"
        SF_UNUSED2(result, composition);
        WARN_NOT_IMPLEMENTED("class_::IME::setCompositionString()");
//##protect##"class_::IME::setCompositionString()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_system::IME, Classes::fl_system::IME::mid_conversionModeSet, const Value, const ASString&> TFunc_Classes_IME_conversionModeSet;
typedef ThunkFunc0<Classes::fl_system::IME, Classes::fl_system::IME::mid_conversionModeGet, ASString> TFunc_Classes_IME_conversionModeGet;
typedef ThunkFunc0<Classes::fl_system::IME, Classes::fl_system::IME::mid_enabledGet, bool> TFunc_Classes_IME_enabledGet;
typedef ThunkFunc1<Classes::fl_system::IME, Classes::fl_system::IME::mid_enabledSet, const Value, bool> TFunc_Classes_IME_enabledSet;
typedef ThunkFunc0<Classes::fl_system::IME, Classes::fl_system::IME::mid_doConversion, const Value> TFunc_Classes_IME_doConversion;
typedef ThunkFunc1<Classes::fl_system::IME, Classes::fl_system::IME::mid_setCompositionString, const Value, const ASString&> TFunc_Classes_IME_setCompositionString;

template <> const TFunc_Classes_IME_conversionModeSet::TMethod TFunc_Classes_IME_conversionModeSet::Method = &Classes::fl_system::IME::conversionModeSet;
template <> const TFunc_Classes_IME_conversionModeGet::TMethod TFunc_Classes_IME_conversionModeGet::Method = &Classes::fl_system::IME::conversionModeGet;
template <> const TFunc_Classes_IME_enabledGet::TMethod TFunc_Classes_IME_enabledGet::Method = &Classes::fl_system::IME::enabledGet;
template <> const TFunc_Classes_IME_enabledSet::TMethod TFunc_Classes_IME_enabledSet::Method = &Classes::fl_system::IME::enabledSet;
template <> const TFunc_Classes_IME_doConversion::TMethod TFunc_Classes_IME_doConversion::Method = &Classes::fl_system::IME::doConversion;
template <> const TFunc_Classes_IME_setCompositionString::TMethod TFunc_Classes_IME_setCompositionString::Method = &Classes::fl_system::IME::setCompositionString;

namespace ClassTraits { namespace fl_system
{
    // const UInt16 IME::tito[IME::ThunkInfoNum] = {
    //    0, 2, 3, 4, 6, 7, 
    // };
    const TypeInfo* IME::tit[9] = {
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo IME::ti[IME::ThunkInfoNum] = {
        {TFunc_Classes_IME_conversionModeSet::Func, &IME::tit[0], "conversionMode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_IME_conversionModeGet::Func, &IME::tit[2], "conversionMode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_IME_enabledGet::Func, &IME::tit[3], "enabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_IME_enabledSet::Func, &IME::tit[4], "enabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_IME_doConversion::Func, &IME::tit[6], "doConversion", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_IME_setCompositionString::Func, &IME::tit[7], "setCompositionString", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    IME::IME(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::IME::IME()"
//##protect##"ClassTraits::IME::IME()"

    }

    Pickable<Traits> IME::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IME(vm, AS3::fl_system::IMECI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::IMECI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_system
{
    const TypeInfo IMETI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_system::IME::InstanceType),
        ClassTraits::fl_system::IME::ThunkInfoNum,
        0,
        0,
        0,
        "IME", "flash.system", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo IMECI = {
        &IMETI,
        ClassTraits::fl_system::IME::MakeClassTraits,
        ClassTraits::fl_system::IME::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

