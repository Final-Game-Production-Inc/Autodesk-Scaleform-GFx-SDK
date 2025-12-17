//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Printing_PrintJob.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Printing_PrintJob.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
#include "../Display/AS3_Obj_Display_Sprite.h"
#include "AS3_Obj_Printing_PrintJobOptions.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_printing
{
    // const UInt16 PrintJob_tito[8] = {
    //    0, 1, 2, 3, 4, 5, 10, 11, 
    // };
    const TypeInfo* PrintJob_tit[12] = {
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::SpriteTI, &AS3::fl_geom::RectangleTI, &AS3::fl_printing::PrintJobOptionsTI, &AS3::fl::int_TI, 
        NULL, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo PrintJob_ti[8] = {
        {ThunkInfo::EmptyFunc, &PrintJob_tit[0], "orientation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[1], "pageHeight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[2], "pageWidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[3], "paperHeight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[4], "paperWidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[5], "addPage", NULL, Abc::NS_Public, CT_Method, 1, 4, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[10], "send", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &PrintJob_tit[11], "start", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_printing
{

    PrintJob::PrintJob(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::PrintJob::PrintJob()"
//##protect##"ClassTraits::PrintJob::PrintJob()"

    }

    Pickable<Traits> PrintJob::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) PrintJob(vm, AS3::fl_printing::PrintJobCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_printing::PrintJobCI));
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
    const TypeInfo PrintJobTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_printing::PrintJob::InstanceType),
        0,
        0,
        8,
        0,
        "PrintJob", "flash.printing", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo PrintJobCI = {
        &PrintJobTI,
        ClassTraits::fl_printing::PrintJob::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_printing::PrintJob_ti,
        NULL,
    };
}; // namespace fl_printing


}}} // namespace Scaleform { namespace GFx { namespace AS3

