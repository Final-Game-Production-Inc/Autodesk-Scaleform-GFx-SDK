//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowInitOptions.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeWindowInitOptions.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 NativeWindowInitOptions_tito[12] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 
    // };
    const TypeInfo* NativeWindowInitOptions_tit[18] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeWindowInitOptions_ti[12] = {
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[0], "maximizable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[1], "maximizable", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[3], "minimizable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[4], "minimizable", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[6], "resizable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[7], "resizable", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[9], "systemChrome", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[10], "systemChrome", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[12], "transparent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[13], "transparent", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[15], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowInitOptions_tit[16], "type", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    NativeWindowInitOptions::NativeWindowInitOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowInitOptions::NativeWindowInitOptions()"
//##protect##"ClassTraits::NativeWindowInitOptions::NativeWindowInitOptions()"

    }

    Pickable<Traits> NativeWindowInitOptions::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowInitOptions(vm, AS3::fl_display::NativeWindowInitOptionsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeWindowInitOptionsCI));
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
    const TypeInfo NativeWindowInitOptionsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_display::NativeWindowInitOptions::InstanceType),
        0,
        0,
        12,
        0,
        "NativeWindowInitOptions", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowInitOptionsCI = {
        &NativeWindowInitOptionsTI,
        ClassTraits::fl_display::NativeWindowInitOptions::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::NativeWindowInitOptions_ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

