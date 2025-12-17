//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NativeApplication.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_NativeApplication.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class NativeWindow;
    class InteractiveIcon;
    class NativeMenu;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 NativeApplication_tito[29] = {
    //    0, 1, 2, 3, 5, 6, 7, 9, 11, 12, 13, 14, 15, 16, 18, 19, 21, 27, 28, 29, 30, 32, 34, 36, 38, 39, 41, 45, 46, 
    // };
    const TypeInfo* NativeApplication_tit[48] = {
        &AS3::fl::XMLTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_desktop::InteractiveIconTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::NativeMenuTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::NativeWindowTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_events::EventTI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeApplication_ti[29] = {
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[0], "applicationDescriptor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[1], "applicationID", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[2], "autoExit", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[3], "autoExit", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[5], "icon", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[6], "idleThreshold", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[7], "idleThreshold", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[9], "menu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[11], "openedWindows", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[12], "publisherID", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[13], "runtimePatchLevel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[14], "runtimeVersion", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[15], "startAtLogin", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[16], "startAtLogin", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[18], "timeSinceLastUserInput", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[19], "activate", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[21], "addEventListener", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[27], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[28], "copy", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[29], "cut", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[30], "dispatchEvent", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[32], "exit", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[34], "getDefaultApplication", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[36], "isSetAsDefaultApplication", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[38], "paste", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[39], "removeAsDefaultApplication", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[41], "removeEventListener", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[45], "selectAll", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeApplication_tit[46], "setAsDefaultApplication", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    NativeApplication::NativeApplication(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::NativeApplication::NativeApplication()"
//##protect##"ClassTraits::NativeApplication::NativeApplication()"

    }

    Pickable<Traits> NativeApplication::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeApplication(vm, AS3::fl_desktop::NativeApplicationCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::NativeApplicationCI));
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
    const TypeInfo NativeApplicationTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::NativeApplication::InstanceType),
        0,
        0,
        29,
        0,
        "NativeApplication", "flash.desktop", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo NativeApplicationCI = {
        &NativeApplicationTI,
        ClassTraits::fl_desktop::NativeApplication::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::NativeApplication_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

