/**************************************************************************

Filename    :   AS3_Obj_ClassClass.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Obj_ClassClass.h"
#include "AS3_Obj_Object.h"
#include "../AS3_VM.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
    
namespace Classes
{
    ClassClass::ClassClass(ClassTraits::Traits& t)
    : Class(t)
    {
        // !!! Prototype shouldn't be set up in the ClassClass constructor.
    }
    
    ClassClass::~ClassClass()
    {
    }

    void ClassClass::SetupPrototype()
    {
        SetPrototype(GetVM().MakeObject());
        InitPrototype(GetPrototype());
    }
} // namespace Classes

namespace ClassTraits
{
    ClassClass::ClassClass(VM& vm)
        : Traits(vm) // These Traits were created by them self ...
    {
        MemoryHeap* mh = vm.GetMemoryHeap();

        Pickable<InstanceTraits::Traits> it(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraits::ClassClass(vm, AS3::fl::ClassCI));
        SetInstanceTraits(it);

        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) Classes::ClassClass(*this));
#ifndef SF_AS3_SETUP_INSTANCE_CONSTRUCTOR_IN_CLASS
        it->SetConstructor(cl);
#endif        
        
        // The line below is presented only in ClassClass.
        SetConstructor(*cl);
    }

    bool ClassClass::Coerce(const Value& value, Value& result) const
    {
        result = value;
        return true;
    }

    void ClassClass::MakeObject(Value& result, AS3::Traits* t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

} // namespace ClassTraits

namespace InstanceTraits
{
    void ClassClass::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        // ClassClass MAY NOT create objects ...
        SF_ASSERT(false);
    }

} // namespace InstanceTraits

namespace fl {
    const TypeInfo ClassTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        // ClassClass MAY NOT create objects ...
        0,
        0, 0, 0, 0,
        "Class", "", NULL,
        TypeInfo::None
    };

    const ClassInfo ClassCI = {
        &ClassTI,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
} // namespace fl

}}} // namespace Scaleform { namespace GFx { namespace AS3
