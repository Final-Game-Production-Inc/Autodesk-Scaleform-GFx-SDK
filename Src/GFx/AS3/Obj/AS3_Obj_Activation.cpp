/**************************************************************************

Filename    :   AS3_Obj_Activation.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Obj_Activation.h"
#include "../AS3_VM.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

namespace InstanceTraits
{
    Activation::Activation( VMAbcFile& file, VM& vm, const Abc::MethodBodyInfo& mbi SF_DEBUG_ARG(const ASString& name)) 
    : RTraits(
        vm, 
        //vm.GetStringManager().CreateConstString("activation@") + mbi.GetMethodInfo(file.GetMethods()).GetName(), 
        vm.GetStringManager().CreateConstString("activation@") SF_DEBUG_CODE(+ name),
        vm.MakePublicNamespace(),
        NULL, // vm.GetClassClass(), 
        true, // DynamicObject
        true  // Final
        )
    {
        SetTraitsType(Traits_Activation);

        // Activation doesn't belong to any VMAbcFile.
        // Instance size will be set by AddSlots().

        // Seems to be not up-to-date any more.
        //SF_ASSERT(GetSlots().GetCount() == const_cast<consts Class&>(c).GetTraits().GetSlots().GetCount());

        const Abc::HasTraits& code_traits = static_cast<const Abc::HasTraits&>(mbi);
        // Theoretically, we shouldn't have any problems here.
        AddSlots(code_traits, file, GetFixedMemSize()).DoNotCheck();
    }

    Activation::Activation(VM& vm) 
    : RTraits(
        vm, 
        vm.GetStringManager().CreateEmptyString(), 
        vm.MakePublicNamespace(),
        NULL, // vm.GetClassClass(), 
        true, // DynamicObject
        true  // Final
        )
    {
        SetTraitsType(Traits_Activation);

        SetMemSize(sizeof(Instance));
    }

    void Activation::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    unsigned Activation::GetFixedMemSize() const
    {
        return sizeof(Instance);
    }

}

}}} // namespace Scaleform { namespace GFx { namespace AS3

