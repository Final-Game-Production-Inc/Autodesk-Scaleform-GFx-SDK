//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Catch.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Catch.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl
{
    Catch::Catch(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Catch::Catch()$data"
//##protect##"instance::Catch::Catch()$data"
    {
//##protect##"instance::Catch::Catch()$code"
//##protect##"instance::Catch::Catch()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{

    Catch::Catch(VM& vm, const ClassInfo& ci)
    : CTraits(vm, ci)
    {
//##protect##"InstanceTraits::Catch::Catch()"
        SetTraitsType(Traits_Catch);
//##protect##"InstanceTraits::Catch::Catch()"

    }

    void Catch::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Catch&>(t));
    }

//##protect##"instance_traits$methods"
    Catch::Catch(VMAbcFile& file, VM& vm, const Abc::MethodBodyInfo::ExceptionInfo& e)
    : CTraits(vm, AS3::fl::CatchCI)
    {
        SetTraitsType(Traits_Catch);

        // Adobe's documentation has wrong info about exc_type and var_name.
        // Different versions of documentation contain different explanations.
        // In reality (at least for now) they both refer to indices in a
        // multiname array.

        StringManager& sm = vm.GetStringManager();
        ASString name = sm.CreateEmptyString();
        const UInt32 name_ind = e.GetVariableNameInd();

        // name_ind == 0 means "any multiname".
        if (name_ind != 0)
        {
            const Abc::Multiname& mn = file.GetConstPool().GetMultiname(name_ind);
            const StringDataPtr str = mn.GetName(file.GetConstPool());

            name = sm.CreateString(str.ToCStr(), str.GetSize());
        }

        const SInt32 et_ind = e.GetExceptionTypeInd();
        SlotInfo::BindingType bt = SlotInfo::BT_Value;

        // et_ind == 0 means "any multiname".
        if (et_ind != 0)
        {
            const Abc::Multiname& mn = file.GetConstPool().GetMultiname(et_ind);

            bt = GetBindingType(file, mn);
        }

        // Theoretically, we should get namespace from a name's multiname.
        AddSlot(name, GetVM().MakePublicNamespace(), bt, AbsoluteIndex(0));
        CalculateMemSize(sizeof(Instances::fl::Catch));
    }
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    Catch::Catch(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Catch::Catch()"
//##protect##"class_::Catch::Catch()"
    }
//##protect##"class_$methods"
    SPtr<InstanceTraits::fl::Catch> Catch::MakeInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e)
    {
        SPtr<InstanceTraits::fl::Catch> tr = MakePickable(SF_HEAP_AUTO_NEW(this) InstanceTraits::fl::Catch(file, GetVM(), e));
        return tr;
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{

    Catch::Catch(VM& vm, const ClassInfo& ci)
    : Traits(vm, ci)
    {
//##protect##"ClassTraits::Catch::Catch()"
        SetTraitsType(Traits_Catch);
//##protect##"ClassTraits::Catch::Catch()"

    }

    Pickable<Traits> Catch::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Catch(vm, AS3::fl::CatchCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::CatchCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo CatchTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::Catch::InstanceType),
        0,
        0,
        0,
        0,
        "Catch", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo CatchCI = {
        &CatchTI,
        ClassTraits::fl::Catch::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

