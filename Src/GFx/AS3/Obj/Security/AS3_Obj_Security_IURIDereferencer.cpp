//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_IURIDereferencer.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Security_IURIDereferencer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class IDataInput;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_security
{
    // const UInt16 IURIDereferencer_tito[1] = {
    //    0, 
    // };
    const TypeInfo* IURIDereferencer_tit[2] = {
        &AS3::fl_utils::IDataInputTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo IURIDereferencer_ti[1] = {
        {ThunkInfo::EmptyFunc, &IURIDereferencer_tit[0], "dereference", "flash.security:IURIDereferencer", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_security
{

    IURIDereferencer::IURIDereferencer(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IURIDereferencer::IURIDereferencer()"
//##protect##"ClassTraits::IURIDereferencer::IURIDereferencer()"

    }

    Pickable<Traits> IURIDereferencer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IURIDereferencer(vm, AS3::fl_security::IURIDereferencerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_security::IURIDereferencerCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_security
{
    const TypeInfo IURIDereferencerTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_security::IURIDereferencer::InstanceType),
        0,
        0,
        1,
        0,
        "IURIDereferencer", "flash.security", NULL,
        TypeInfo::None
    };

    const ClassInfo IURIDereferencerCI = {
        &IURIDereferencerTI,
        ClassTraits::fl_security::IURIDereferencer::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_security::IURIDereferencer_ti,
        NULL,
    };
}; // namespace fl_security


}}} // namespace Scaleform { namespace GFx { namespace AS3

