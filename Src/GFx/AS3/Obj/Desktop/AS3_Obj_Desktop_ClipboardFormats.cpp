//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_ClipboardFormats.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_ClipboardFormats.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_desktop
{
    ClipboardFormats::ClipboardFormats(ClassTraits::Traits& t)
    : Class(t)
    , BITMAP_FORMAT("air:bitmap")
    , FILE_LIST_FORMAT("air:file list")
    , FILE_PROMISE_LIST_FORMAT("air:file promise list")
    , HTML_FORMAT("air:html")
    , TEXT_FORMAT("air:text")
    , URL_FORMAT("air:url")
    {
//##protect##"class_::ClipboardFormats::ClipboardFormats()"
//##protect##"class_::ClipboardFormats::ClipboardFormats()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_desktop
{
    const MemberInfo ClipboardFormats::mi[ClipboardFormats::MemberInfoNum] = {
        {"BITMAP_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, BITMAP_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"FILE_LIST_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, FILE_LIST_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"FILE_PROMISE_LIST_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, FILE_PROMISE_LIST_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HTML_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, HTML_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TEXT_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, TEXT_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"URL_FORMAT", NULL, OFFSETOF(Classes::fl_desktop::ClipboardFormats, URL_FORMAT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    ClipboardFormats::ClipboardFormats(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ClipboardFormats::ClipboardFormats()"
//##protect##"ClassTraits::ClipboardFormats::ClipboardFormats()"

    }

    Pickable<Traits> ClipboardFormats::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ClipboardFormats(vm, AS3::fl_desktop::ClipboardFormatsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::ClipboardFormatsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_desktop
{
    const TypeInfo ClipboardFormatsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::ClipboardFormats::InstanceType),
        0,
        ClassTraits::fl_desktop::ClipboardFormats::MemberInfoNum,
        0,
        0,
        "ClipboardFormats", "flash.desktop", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ClipboardFormatsCI = {
        &ClipboardFormatsTI,
        ClassTraits::fl_desktop::ClipboardFormats::MakeClassTraits,
        NULL,
        ClassTraits::fl_desktop::ClipboardFormats::mi,
        NULL,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

