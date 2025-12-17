//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_ActionScriptVersion.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_ActionScriptVersion.h"
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
    ActionScriptVersion::ActionScriptVersion(ClassTraits::Traits& t)
    : Class(t)
    , ACTIONSCRIPT2(2)
    , ACTIONSCRIPT3(3)
    {
//##protect##"class_::ActionScriptVersion::ActionScriptVersion()"
//##protect##"class_::ActionScriptVersion::ActionScriptVersion()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo ActionScriptVersion::mi[ActionScriptVersion::MemberInfoNum] = {
        {"ACTIONSCRIPT2", NULL, OFFSETOF(Classes::fl_display::ActionScriptVersion, ACTIONSCRIPT2), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"ACTIONSCRIPT3", NULL, OFFSETOF(Classes::fl_display::ActionScriptVersion, ACTIONSCRIPT3), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    ActionScriptVersion::ActionScriptVersion(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ActionScriptVersion::ActionScriptVersion()"
//##protect##"ClassTraits::ActionScriptVersion::ActionScriptVersion()"

    }

    Pickable<Traits> ActionScriptVersion::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ActionScriptVersion(vm, AS3::fl_display::ActionScriptVersionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::ActionScriptVersionCI));
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
    const TypeInfo ActionScriptVersionTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::ActionScriptVersion::InstanceType),
        0,
        ClassTraits::fl_display::ActionScriptVersion::MemberInfoNum,
        0,
        0,
        "ActionScriptVersion", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ActionScriptVersionCI = {
        &ActionScriptVersionTI,
        ClassTraits::fl_display::ActionScriptVersion::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::ActionScriptVersion::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

