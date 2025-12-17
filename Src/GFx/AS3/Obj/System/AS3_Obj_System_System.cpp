//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_System.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_System.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
#include "AS3_Obj_System_IME.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
    namespace Classes
    {
        class IME;
    }

    void escapeMultiByteInternal(VM& vm, ASString& result, const ASString& value)
    {
        bool useCodePage = false;

        const Multiname mn(vm, "flash.utils.System");
        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(mn, vm.GetCurrentAppDomain());
        if (ctr && ctr->IsValid())
        {
            const Class& cl = ctr->GetClass();
            const Classes::fl_system::System& clSystem = static_cast<const Classes::fl_system::System&>(cl);
            useCodePage = clSystem.useCodePage;
        }

        String escapedStr;
        if (useCodePage)
        {
            // If System.useCodePage is true, the escaped string is encoded in the system code page.
            ASUtils::Escape(value.ToCStr(), value.GetSize(), &escapedStr);
        }
        else
        {
            // If System.useCodePage is false, the escaped string is encoded in UTF-8
            StringBuffer b;
            ASUtils::AS3::Escape(value.ToCStr(), value.GetSize(), b);
            escapedStr = b;
        }
        result = vm.GetStringManager().CreateString(escapedStr.ToCStr(), escapedStr.GetSize());
    }
    void unescapeMultiByteInternal(VM& vm, ASString& result, const ASString& value)
    {
        bool useCodePage = false;

        const Multiname mn(vm, "flash.utils.System");
        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(mn, vm.GetCurrentAppDomain());
        if (ctr && ctr->IsValid())
        {
            const Class& cl = ctr->GetClass();
            const Classes::fl_system::System& clSystem = static_cast<const Classes::fl_system::System&>(cl);
            useCodePage = clSystem.useCodePage;
        }

        String unescapedStr;
        bool ok = true;
        if (useCodePage)
        {
            // If System.useCodePage is true, the escaped string is decoded from the system code page.
            ASUtils::Unescape(value.ToCStr(), value.GetSize(), &unescapedStr);
        }
        else
        {
            // If System.useCodePage is false, the escaped string is decoded from UTF-8.
            StringBuffer b;
            ok = ASUtils::AS3::Unescape(value.ToCStr(), value.GetSize(), b);
            unescapedStr = b;
        }

        if (ok)
            result = vm.GetStringManager().CreateString(unescapedStr.ToCStr(), unescapedStr.GetSize());
    }
//##protect##"methods"

namespace Classes { namespace fl_system
{
    System::System(ClassTraits::Traits& t)
    : Class(t)
    , useCodePage()
    {
//##protect##"class_::System::System()"
//##protect##"class_::System::System()"
    }
    void System::imeGet(Value& result)
    {
//##protect##"class_::System::imeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::System::imeGet()");
//##protect##"class_::System::imeGet()"
    }
    void System::totalMemoryGet(UInt32& result)
    {
//##protect##"class_::System::totalMemoryGet()"
        result = static_cast<UInt32>(Memory::pGlobalHeap->GetTotalFootprint());
//##protect##"class_::System::totalMemoryGet()"
    }
    void System::exit(const Value& result, UInt32 code)
    {
//##protect##"class_::System::exit()"
        SF_UNUSED2(result, code);
        static_cast<ASVM&>(GetVM()).GetMovieImpl()->RequestExit();
//##protect##"class_::System::exit()"
    }
    void System::gc(const Value& result)
    {
//##protect##"class_::System::gc()"
        SF_UNUSED1(result);
        GetVM().GetGC().ScheduleCollect(ASRefCountCollector::Collect_Full);
//##protect##"class_::System::gc()"
    }
    void System::pause(const Value& result)
    {
//##protect##"class_::System::pause()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::System::pause()");
//##protect##"class_::System::pause()"
    }
    void System::resume(const Value& result)
    {
//##protect##"class_::System::resume()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::System::resume()");
//##protect##"class_::System::resume()"
    }
    void System::setClipboard(const Value& result, const ASString& string)
    {
//##protect##"class_::System::setClipboard()"
        SF_UNUSED2(result, string);
        WARN_NOT_IMPLEMENTED("class_::System::setClipboard()");
//##protect##"class_::System::setClipboard()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_system::System, Classes::fl_system::System::mid_imeGet, Value> TFunc_Classes_System_imeGet;
typedef ThunkFunc0<Classes::fl_system::System, Classes::fl_system::System::mid_totalMemoryGet, UInt32> TFunc_Classes_System_totalMemoryGet;
typedef ThunkFunc1<Classes::fl_system::System, Classes::fl_system::System::mid_exit, const Value, UInt32> TFunc_Classes_System_exit;
typedef ThunkFunc0<Classes::fl_system::System, Classes::fl_system::System::mid_gc, const Value> TFunc_Classes_System_gc;
typedef ThunkFunc0<Classes::fl_system::System, Classes::fl_system::System::mid_pause, const Value> TFunc_Classes_System_pause;
typedef ThunkFunc0<Classes::fl_system::System, Classes::fl_system::System::mid_resume, const Value> TFunc_Classes_System_resume;
typedef ThunkFunc1<Classes::fl_system::System, Classes::fl_system::System::mid_setClipboard, const Value, const ASString&> TFunc_Classes_System_setClipboard;

template <> const TFunc_Classes_System_imeGet::TMethod TFunc_Classes_System_imeGet::Method = &Classes::fl_system::System::imeGet;
template <> const TFunc_Classes_System_totalMemoryGet::TMethod TFunc_Classes_System_totalMemoryGet::Method = &Classes::fl_system::System::totalMemoryGet;
template <> const TFunc_Classes_System_exit::TMethod TFunc_Classes_System_exit::Method = &Classes::fl_system::System::exit;
template <> const TFunc_Classes_System_gc::TMethod TFunc_Classes_System_gc::Method = &Classes::fl_system::System::gc;
template <> const TFunc_Classes_System_pause::TMethod TFunc_Classes_System_pause::Method = &Classes::fl_system::System::pause;
template <> const TFunc_Classes_System_resume::TMethod TFunc_Classes_System_resume::Method = &Classes::fl_system::System::resume;
template <> const TFunc_Classes_System_setClipboard::TMethod TFunc_Classes_System_setClipboard::Method = &Classes::fl_system::System::setClipboard;

namespace ClassTraits { namespace fl_system
{
    const MemberInfo System::mi[System::MemberInfoNum] = {
        {"useCodePage", NULL, OFFSETOF(Classes::fl_system::System, useCodePage), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };

    // const UInt16 System::tito[System::ThunkInfoNum] = {
    //    0, 1, 2, 4, 5, 6, 7, 
    // };
    const TypeInfo* System::tit[9] = {
        NULL, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, 
        NULL, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo System::ti[System::ThunkInfoNum] = {
        {TFunc_Classes_System_imeGet::Func, &System::tit[0], "ime", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_System_totalMemoryGet::Func, &System::tit[1], "totalMemory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_System_exit::Func, &System::tit[2], "exit", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_System_gc::Func, &System::tit[4], "gc", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_System_pause::Func, &System::tit[5], "pause", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_System_resume::Func, &System::tit[6], "resume", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_System_setClipboard::Func, &System::tit[7], "setClipboard", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    System::System(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::System::System()"
//##protect##"ClassTraits::System::System()"

    }

    Pickable<Traits> System::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) System(vm, AS3::fl_system::SystemCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::SystemCI));
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
    const TypeInfo SystemTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_system::System::InstanceType),
        ClassTraits::fl_system::System::ThunkInfoNum,
        ClassTraits::fl_system::System::MemberInfoNum,
        0,
        0,
        "System", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SystemCI = {
        &SystemTI,
        ClassTraits::fl_system::System::MakeClassTraits,
        ClassTraits::fl_system::System::ti,
        ClassTraits::fl_system::System::mi,
        NULL,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

