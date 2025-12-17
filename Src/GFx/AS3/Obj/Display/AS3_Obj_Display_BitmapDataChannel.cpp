//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_BitmapDataChannel.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_BitmapDataChannel.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_display
{
    BitmapDataChannel::BitmapDataChannel(ClassTraits::Traits& t)
    : Class(t)
    , ALPHA(8)
    , BLUE(4)
    , GREEN(2)
    , RED(1)
    {
//##protect##"class_::BitmapDataChannel::BitmapDataChannel()"
//##protect##"class_::BitmapDataChannel::BitmapDataChannel()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo BitmapDataChannel::mi[BitmapDataChannel::MemberInfoNum] = {
        {"ALPHA", NULL, OFFSETOF(Classes::fl_display::BitmapDataChannel, ALPHA), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"BLUE", NULL, OFFSETOF(Classes::fl_display::BitmapDataChannel, BLUE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"GREEN", NULL, OFFSETOF(Classes::fl_display::BitmapDataChannel, GREEN), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"RED", NULL, OFFSETOF(Classes::fl_display::BitmapDataChannel, RED), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    BitmapDataChannel::BitmapDataChannel(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::BitmapDataChannel::BitmapDataChannel()"
//##protect##"ClassTraits::BitmapDataChannel::BitmapDataChannel()"

    }

    Pickable<Traits> BitmapDataChannel::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) BitmapDataChannel(vm, AS3::fl_display::BitmapDataChannelCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::BitmapDataChannelCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo BitmapDataChannelTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::BitmapDataChannel::InstanceType),
        0,
        ClassTraits::fl_display::BitmapDataChannel::MemberInfoNum,
        0,
        0,
        "BitmapDataChannel", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo BitmapDataChannelCI = {
        &BitmapDataChannelTI,
        ClassTraits::fl_display::BitmapDataChannel::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::BitmapDataChannel::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

