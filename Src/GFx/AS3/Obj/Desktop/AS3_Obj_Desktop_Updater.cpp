//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_Updater.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_Updater.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 Updater_tito[1] = {
    //    0, 
    // };
    const TypeInfo* Updater_tit[3] = {
        NULL, &AS3::fl_filesystem::FileTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo Updater_ti[1] = {
        {ThunkInfo::EmptyFunc, &Updater_tit[0], "update", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    Updater::Updater(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Updater::Updater()"
//##protect##"ClassTraits::Updater::Updater()"

    }

    Pickable<Traits> Updater::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Updater(vm, AS3::fl_desktop::UpdaterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::UpdaterCI));
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
    const TypeInfo UpdaterTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::Updater::InstanceType),
        0,
        0,
        1,
        0,
        "Updater", "flash.desktop", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo UpdaterCI = {
        &UpdaterTI,
        ClassTraits::fl_desktop::Updater::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::Updater_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

