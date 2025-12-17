//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_SystemEx.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_SystemEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    SystemEx::SystemEx(ClassTraits::Traits& t)
    : Class(t)
    , REPORT_SHORT_FILENAMES(0x1)
    , REPORT_NO_CIRCULAR_REFERENCES(0x2)
    , REPORT_SUPPRESS_OVERALL_STATS(0x4)
    , REPORT_ONLY_ANON_OBJ_ADDRESSES(0x8)
    {
//##protect##"class_::SystemEx::SystemEx()"
//##protect##"class_::SystemEx::SystemEx()"
    }
    void SystemEx::actionVerboseSet(const Value& result, bool verbose)
    {
//##protect##"class_::SystemEx::actionVerboseSet()"

        SF_UNUSED1(result);
        static_cast<ASVM&>(GetVM()).GetMovieImpl()->SetVerboseAction(verbose);

        FlashUI& ui = GetVM().GetUI();
        ui.SetNeedToCheckOpCode(verbose);
//##protect##"class_::SystemEx::actionVerboseSet()"
    }
    void SystemEx::actionVerboseGet(bool& result)
    {
//##protect##"class_::SystemEx::actionVerboseGet()"

        result = static_cast<ASVM&>(GetVM()).GetMovieImpl()->IsVerboseAction();

//##protect##"class_::SystemEx::actionVerboseGet()"
    }
    void SystemEx::setBackgroundAlpha(const Value& result, Value::Number value)
    {
//##protect##"class_::SystemEx::setBackgroundAlpha()"

        SF_UNUSED1(result);
        static_cast<ASVM&>(GetVM()).GetMovieImpl()->SetBackgroundAlpha(static_cast<float>(value));

//##protect##"class_::SystemEx::setBackgroundAlpha()"
    }
    void SystemEx::getStackTrace(ASString& result)
    {
//##protect##"class_::SystemEx::getStackTrace()"

        GetVM().GetStackTraceASString(result);

//##protect##"class_::SystemEx::getStackTrace()"
    }
    void SystemEx::getCodeFileName(ASString& result)
    {
//##protect##"class_::SystemEx::getCodeFileName()"

        GetVM().GetCodeFileName(result);

//##protect##"class_::SystemEx::getCodeFileName()"
    }
    void SystemEx::getCodeFileNames(SPtr<Instances::fl::Array>& result)
    {
//##protect##"class_::SystemEx::getCodeFileNames()"
        SF_UNUSED1(result);

        Scaleform::Array<SPtr<VMAbcFile> > srcArr;
        UPInt n = GetVM().GetAllLoadedAbcFiles(&srcArr);

        SPtr<Instances::fl::Array> destArr = GetVM().MakeArray();

        for (UPInt i = 0; i < n; ++i)
        {
            const Scaleform::String& name = srcArr[i]->GetAbcFile().GetName();
            Value v(GetVM().GetStringManager().CreateString(name.ToCStr()));
            destArr->PushBack(v);
        }
        result = destArr;
        
//##protect##"class_::SystemEx::getCodeFileNames()"
    }
    void SystemEx::describeType(ASString& result, const Value& v)
    {
//##protect##"class_::SystemEx::describeType()"
        result = GetVM().describeTypeEx(v);
//##protect##"class_::SystemEx::describeType()"
    }
    void SystemEx::printObjectsReport(const Value& result, bool runGarbageCollector, UInt32 reportFlags, const ASString& swfFilter)
    {
//##protect##"class_::SystemEx::printObjectsReport()"
        SF_UNUSED1(result);

        MovieImpl* pmovieImpl = static_cast<ASVM&>(GetVM()).GetMovieImpl();
        if (runGarbageCollector)
            pmovieImpl->ForceCollectGarbage();
        pmovieImpl->PrintObjectsReport(reportFlags, NULL, swfFilter.ToCStr());

//##protect##"class_::SystemEx::printObjectsReport()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_actionVerboseSet, const Value, bool> TFunc_Classes_SystemEx_actionVerboseSet;
typedef ThunkFunc0<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_actionVerboseGet, bool> TFunc_Classes_SystemEx_actionVerboseGet;
typedef ThunkFunc1<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_setBackgroundAlpha, const Value, Value::Number> TFunc_Classes_SystemEx_setBackgroundAlpha;
typedef ThunkFunc0<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_getStackTrace, ASString> TFunc_Classes_SystemEx_getStackTrace;
typedef ThunkFunc0<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_getCodeFileName, ASString> TFunc_Classes_SystemEx_getCodeFileName;
typedef ThunkFunc0<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_getCodeFileNames, SPtr<Instances::fl::Array> > TFunc_Classes_SystemEx_getCodeFileNames;
typedef ThunkFunc1<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_describeType, ASString, const Value&> TFunc_Classes_SystemEx_describeType;
typedef ThunkFunc3<Classes::fl_gfx::SystemEx, Classes::fl_gfx::SystemEx::mid_printObjectsReport, const Value, bool, UInt32, const ASString&> TFunc_Classes_SystemEx_printObjectsReport;

template <> const TFunc_Classes_SystemEx_actionVerboseSet::TMethod TFunc_Classes_SystemEx_actionVerboseSet::Method = &Classes::fl_gfx::SystemEx::actionVerboseSet;
template <> const TFunc_Classes_SystemEx_actionVerboseGet::TMethod TFunc_Classes_SystemEx_actionVerboseGet::Method = &Classes::fl_gfx::SystemEx::actionVerboseGet;
template <> const TFunc_Classes_SystemEx_setBackgroundAlpha::TMethod TFunc_Classes_SystemEx_setBackgroundAlpha::Method = &Classes::fl_gfx::SystemEx::setBackgroundAlpha;
template <> const TFunc_Classes_SystemEx_getStackTrace::TMethod TFunc_Classes_SystemEx_getStackTrace::Method = &Classes::fl_gfx::SystemEx::getStackTrace;
template <> const TFunc_Classes_SystemEx_getCodeFileName::TMethod TFunc_Classes_SystemEx_getCodeFileName::Method = &Classes::fl_gfx::SystemEx::getCodeFileName;
template <> const TFunc_Classes_SystemEx_getCodeFileNames::TMethod TFunc_Classes_SystemEx_getCodeFileNames::Method = &Classes::fl_gfx::SystemEx::getCodeFileNames;
template <> const TFunc_Classes_SystemEx_describeType::TMethod TFunc_Classes_SystemEx_describeType::Method = &Classes::fl_gfx::SystemEx::describeType;
template <> const TFunc_Classes_SystemEx_printObjectsReport::TMethod TFunc_Classes_SystemEx_printObjectsReport::Method = &Classes::fl_gfx::SystemEx::printObjectsReport;

namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo SystemEx::mi[SystemEx::MemberInfoNum] = {
        {"REPORT_SHORT_FILENAMES", NULL, OFFSETOF(Classes::fl_gfx::SystemEx, REPORT_SHORT_FILENAMES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"REPORT_NO_CIRCULAR_REFERENCES", NULL, OFFSETOF(Classes::fl_gfx::SystemEx, REPORT_NO_CIRCULAR_REFERENCES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"REPORT_SUPPRESS_OVERALL_STATS", NULL, OFFSETOF(Classes::fl_gfx::SystemEx, REPORT_SUPPRESS_OVERALL_STATS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"REPORT_ONLY_ANON_OBJ_ADDRESSES", NULL, OFFSETOF(Classes::fl_gfx::SystemEx, REPORT_ONLY_ANON_OBJ_ADDRESSES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };

    // const UInt16 SystemEx::tito[SystemEx::ThunkInfoNum] = {
    //    0, 2, 3, 5, 6, 7, 8, 10, 
    // };
    const TypeInfo* SystemEx::tit[14] = {
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, NULL, 
        NULL, &AS3::fl::BooleanTI, &AS3::fl::uintTI, &AS3::fl::StringTI, 
    };
    const Abc::ConstValue SystemEx::dva[3] = {
        {Abc::CONSTANT_True, 0}, {Abc::CONSTANT_UInt, 7}, {Abc::CONSTANT_Null, 0}, 
    };
    const ThunkInfo SystemEx::ti[SystemEx::ThunkInfoNum] = {
        {TFunc_Classes_SystemEx_actionVerboseSet::Func, &SystemEx::tit[0], "actionVerbose", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_SystemEx_actionVerboseGet::Func, &SystemEx::tit[2], "actionVerbose", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_SystemEx_setBackgroundAlpha::Func, &SystemEx::tit[3], "setBackgroundAlpha", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_SystemEx_getStackTrace::Func, &SystemEx::tit[5], "getStackTrace", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_SystemEx_getCodeFileName::Func, &SystemEx::tit[6], "getCodeFileName", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_SystemEx_getCodeFileNames::Func, &SystemEx::tit[7], "getCodeFileNames", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_SystemEx_describeType::Func, &SystemEx::tit[8], "describeType", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_SystemEx_printObjectsReport::Func, &SystemEx::tit[10], "printObjectsReport", NULL, Abc::NS_Public, CT_Method, 0, 3, 0, 3, &SystemEx::dva[0]},
    };

    SystemEx::SystemEx(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SystemEx::SystemEx()"
//##protect##"ClassTraits::SystemEx::SystemEx()"

    }

    Pickable<Traits> SystemEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SystemEx(vm, AS3::fl_gfx::SystemExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::SystemExCI));
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
    const TypeInfo SystemExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::SystemEx::InstanceType),
        ClassTraits::fl_gfx::SystemEx::ThunkInfoNum,
        ClassTraits::fl_gfx::SystemEx::MemberInfoNum,
        0,
        0,
        "SystemEx", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SystemExCI = {
        &SystemExTI,
        ClassTraits::fl_gfx::SystemEx::MakeClassTraits,
        ClassTraits::fl_gfx::SystemEx::ti,
        ClassTraits::fl_gfx::SystemEx::mi,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

