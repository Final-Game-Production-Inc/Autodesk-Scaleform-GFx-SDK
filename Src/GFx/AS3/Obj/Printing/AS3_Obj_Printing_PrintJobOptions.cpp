//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Printing_PrintJobOptions.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Printing_PrintJobOptions.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_printing
{
    PrintJobOptions::PrintJobOptions(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , printAsBitmap(false)
//##protect##"instance::PrintJobOptions::PrintJobOptions()$data"
//##protect##"instance::PrintJobOptions::PrintJobOptions()$data"
    {
//##protect##"instance::PrintJobOptions::PrintJobOptions()$code"
//##protect##"instance::PrintJobOptions::PrintJobOptions()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_printing
{
    const MemberInfo PrintJobOptions::mi[PrintJobOptions::MemberInfoNum] = {
        {"printAsBitmap", NULL, OFFSETOF(Instances::fl_printing::PrintJobOptions, printAsBitmap), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    PrintJobOptions::PrintJobOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::PrintJobOptions::PrintJobOptions()"
//##protect##"InstanceTraits::PrintJobOptions::PrintJobOptions()"

    }

    void PrintJobOptions::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<PrintJobOptions&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_printing
{

    PrintJobOptions::PrintJobOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::PrintJobOptions::PrintJobOptions()"
//##protect##"ClassTraits::PrintJobOptions::PrintJobOptions()"

    }

    Pickable<Traits> PrintJobOptions::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) PrintJobOptions(vm, AS3::fl_printing::PrintJobOptionsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_printing::PrintJobOptionsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_printing
{
    const TypeInfo PrintJobOptionsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_printing::PrintJobOptions::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_printing::PrintJobOptions::MemberInfoNum,
        "PrintJobOptions", "flash.printing", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo PrintJobOptionsCI = {
        &PrintJobOptionsTI,
        ClassTraits::fl_printing::PrintJobOptions::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_printing::PrintJobOptions::mi,
    };
}; // namespace fl_printing


}}} // namespace Scaleform { namespace GFx { namespace AS3

