/**************************************************************************

Filename    :   AS3_UDBase.cpp
Content     :   
Created     :   Feb, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_UDBase.h"
#include "AS3_VM.h"
#include "Obj/AS3_Obj_Object.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

namespace Classes
{
    UDBase::UDBase(ClassTraits::Traits& t)
    : Class(t)
    {
        t.ConstructTail(*this);
    }

    UDBase::~UDBase()
    {
        GetTraitsSafe().DestructTail(*this);
    }

    CheckResult UDBase::PreInit(const Value& _this) const
    {
        // Set up slot values.
        return GetClassTraits().GetInstanceTraits().SetupSlotValues(*_this.GetObject());
    }

    void UDBase::InitPrototype(AS3::Object& obj) const
    {
        AddConstructor(obj);
    }

} // namespace Classes

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


