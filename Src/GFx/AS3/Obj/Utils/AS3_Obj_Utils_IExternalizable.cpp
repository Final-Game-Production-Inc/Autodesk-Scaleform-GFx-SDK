//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_IExternalizable.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_IExternalizable.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Utils_IDataOutput.h"
#include "AS3_Obj_Utils_IDataInput.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_utils
{
    // const UInt16 IExternalizable_tito[2] = {
    //    0, 2, 
    // };
    const TypeInfo* IExternalizable_tit[4] = {
        NULL, &AS3::fl_utils::IDataInputTI, 
        NULL, &AS3::fl_utils::IDataOutputTI, 
    };
    const ThunkInfo IExternalizable_ti[2] = {
        {ThunkInfo::EmptyFunc, &IExternalizable_tit[0], "readExternal", "flash.utils:IExternalizable", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IExternalizable_tit[2], "writeExternal", "flash.utils:IExternalizable", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    IExternalizable::IExternalizable(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IExternalizable::IExternalizable()"
//##protect##"ClassTraits::IExternalizable::IExternalizable()"

    }

    Pickable<Traits> IExternalizable::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IExternalizable(vm, AS3::fl_utils::IExternalizableCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::IExternalizableCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_utils
{
    const TypeInfo IExternalizableTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_utils::IExternalizable::InstanceType),
        0,
        0,
        2,
        0,
        "IExternalizable", "flash.utils", NULL,
        TypeInfo::None
    };

    const ClassInfo IExternalizableCI = {
        &IExternalizableTI,
        ClassTraits::fl_utils::IExternalizable::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_utils::IExternalizable_ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

