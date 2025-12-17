/**************************************************************************

Filename    :   AS3_VTable.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_VM.h"
#include "AS3_VTable.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
VTable::VTable(Traits& tr)
: pTraits(&tr)
SF_DEBUG_ARG(Names(tr.GetVM().GetStringManager().CreateEmptyString()))
{
    /*
    // This will introduce memory leaks.
    else if (vm.IsInitialized() && tr.IsInstanceTraits())
        // Class traits are not valid at the time when we create VT.
        MakeScopeStackCPP(tr);
    */
}

VTable::VTable(Traits& tr, const VTable& parent)
: pTraits(&tr)
, VTMethods(parent.VTMethods) // Deep copy.
SF_DEBUG_ARG(Names(parent.Names)) // Deep copy.
{
    /*
    // This will introduce memory leaks.
    else if (vm.IsInitialized() && tr.IsInstanceTraits())
        // Class traits are not valid at the time when we create VT.
        MakeScopeStackCPP(tr);
    */
}

// void VTable::MakeScopeStackCPP(const Traits& tr)
// {
//     const Traits* ptr = tr.GetParent();
// 
//     if (ptr)
//         MakeScopeStackCPP(*ptr);
//     else
//         InitScope.PushBack(GetVM().GetGlobalObjectCPPValue());
// 
//     if (tr.IsClassTraits())
//     {
//         const ClassTraits::Traits& ctr = static_cast<const ClassTraits::Traits&>(tr);
// 
//         SF_ASSERT(ctr.IsValid());
//         Class& cl = ctr.GetInstanceTraits().GetClass();
//         InitScope.PushBack(Value(&cl));
//     }
//     else
//     {
//         const Class& cl = static_cast<const InstanceTraits::Traits&>(tr).GetClass();
//         InitScope.PushBack(Value(&const_cast<Class&>(cl)));
//     }
// }

void VTable::GetMethod(Value& value, AbsoluteIndex ind, Object& _this, bool super) const
{
    const Value& m = VTMethods[ind.Get()];

    switch(m.GetKind())
    {
    case Value::kThunk:
        // Make a ThunkClosure.
        value = Value(&_this, m.AsThunk());
        return;
    case Value::kMethodInd:
        // Make a kVTableIndClosure.
        value = Value(&_this, static_cast<SInt32>(ind.Get()), super);
        return;
    default:
        break;
    }
    
    value = m;
}

Value VTable::GetValue(AbsoluteIndex ind) const
{
    const Value& m = VTMethods[ind.Get()];

    if (m.GetKind() == Value::kMethodInd)
        return Value(static_cast<SInt32>(ind.Get()), GetTraits(),  Value::kVTableInd);

    return m;
}

#if defined(GFX_AS3_VERBOSE)
void VTable::SetMethodName(AbsoluteIndex ind, SlotInfo::BindingType dt, const ASString& name)
{
	if (ind.Get() >= Names.GetSize())
		Names.Resize(ind.Get() + 1);

	switch(dt)
	{
	case SlotInfo::BT_Code:
		Names[ind.Get()] = name;

		break;
	case SlotInfo::BT_Get:
		Names[ind.Get()] = "get " + name;

		break;
	case SlotInfo::BT_Set:
		if (ind.Get() + 1 >= Names.GetSize())
			Names.Resize(ind.Get() + 1 + 1);
		Names[ind.Get() + 1] = "set " + name;

		break;
	default:
		SF_ASSERT(false);
		break;
	}
}
#endif

void VTable::SetMethod(AbsoluteIndex ind, const Value& m, SlotInfo::BindingType dt SF_DEBUG_ARG(const ASString& name))
{
    Value* vt_value = NULL;
    
    switch(dt)
    {
    case SlotInfo::BT_Code:
    case SlotInfo::BT_Get:
        vt_value = &VTMethods[ind.Get()];
        break;
    case SlotInfo::BT_Set:
        vt_value = &VTMethods[ind.Get() + 1];
        break;
    default:
        SF_ASSERT(false);
        break;
    }

	SF_DEBUG_CODE(SetMethodName(ind, dt, name);)

    if (m.GetKind() == Value::kInt)
    {
        SF_DEBUG_CODE(SF_UNUSED1(name);)
        *vt_value = Value(m.AsInt(), GetTraits(), Value::kMethodInd);
    }
    else
        *vt_value = m;
}

AbsoluteIndex VTable::AddMethod(const Value& m, SlotInfo::BindingType dt SF_DEBUG_ARG(const ASString& name))
{
    UPInt ind;
    
    switch(dt)
    {
    case SlotInfo::BT_Code:
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        break;
    case SlotInfo::BT_Get:
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        // Place for a setter.
        VTMethods.PushDefault();
        break;
    case SlotInfo::BT_Set:
        // Place for a getter.
        VTMethods.PushDefault();
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        break;
    default:
        SF_ASSERT(false);
        ind = 0;
        break;
    }
    
	SF_DEBUG_CODE(SetMethodName(AbsoluteIndex(ind), dt, name);)

    if (m.GetKind() == Value::kInt)
    {
        SF_DEBUG_CODE(SF_UNUSED1(name);)
        VTMethods[ind] = Value(m.AsInt(), GetTraits(), Value::kMethodInd);
    }
    else
        VTMethods[ind] = m;
    
    return AbsoluteIndex(VTMethods.GetSize() - (dt == SlotInfo::BT_Code ? 1 : 2));
}

void VTable::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    const UPInt size = VTMethods.GetSize();
    for (UPInt i = 0; i < size; ++i)
        AS3::ForEachChild_GC(prcc, VTMethods[i], op SF_DEBUG_ARG(*pTraits));
}

namespace InstanceTraits
{

    void Traits::AddInterfaceSlots(VMAbcFile* file_ptr, Traits& itr)
    {
        SF_ASSERT(itr.IsInterface());

        VM& vm = GetVM();

        for (CIterator it = itr.Begin(); !it.IsEnd(); ++it)
        {
            ASString        interfaceSlotName(it.GetSlotName());
            const SlotInfo& interfaceSlot = it.GetSlotInfo();

            // For each interface slot, we see if has already been added. If it is, as is the
            // case for 'constructor' and perhaps multiple interface use, no change is necessary.
            // If not, we find the method declared in 'public' namespace of the class and
            // create an alias to it for interface namespace.

            const SlotInfo* thisSlot = FindSlotInfo(interfaceSlotName, interfaceSlot.GetNamespace());
            if (!thisSlot)
            {
                thisSlot = FindSlotInfo(interfaceSlotName, vm.GetPublicNamespace());
                if (thisSlot)
                {
                    // Interface slot was added/overridden in public namespace.
                    // Let us add an alias.
                    // What should we do about protected and private namespaces.

                    // Just thisSlot->setNamespace() below won't work.
                    //thisSlot->setNamespace(interfaceSlot.GetNamespace());

                    SlotInfo newSlot(*thisSlot);
                    newSlot.setNamespace(interfaceSlot.GetNamespace());
                    AddSlotInfo(interfaceSlotName, newSlot);
                }
                else
                {
                    if (itr.IsInterface())
                    {
                        // Add this slot/code.
                        SlotInfo::BindingType binding_type = interfaceSlot.GetBindingType();
                        AbsoluteIndex ind(-1);

                        // This slot is an interface method. Let's mark slot's
                        // namespace as an "IsInterface" namespace.
                        const_cast<Instances::fl::Namespace&>(interfaceSlot.GetNamespace()).SetIsInterface();

                        SlotInfo& si = FindAddOwnSlotInfo(
                            interfaceSlotName, 
                            interfaceSlot,
                            ind
                            );

                        // Initialize a virtual table ...
                        // Add2VT will also bind the slot.
                        // Add2VT(si, vt_value, binding_type);

                        // Code in this block is a part of the Add2VT() method.
                        {
                            switch (binding_type)
                            {
                            case SlotInfo::BT_Code:
                            case SlotInfo::BT_Get:
                                {
                                    const Value& vt_value = itr.GetVT().GetRaw(interfaceSlot.GetAValueInd());

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type SF_DEBUG_ARG(si.GetQualifiedName())));
                                }
                                break;
                            case SlotInfo::BT_Set:
                                {
                                    const Value& vt_value = itr.GetVT().GetRaw(interfaceSlot.GetAValueInd() + 1);

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type SF_DEBUG_ARG(si.GetQualifiedName())));
                                }
                                break;
                            case SlotInfo::BT_GetSet:
                                // Add BT_Get part.
                                {
                                    binding_type = SlotInfo::BT_Get;
                                    const Value& vt_value = itr.GetVT().GetRaw(interfaceSlot.GetAValueInd());

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type SF_DEBUG_ARG(si.GetQualifiedName())));
                                }
                                // Add BT_Set part.
                                {
                                    binding_type = SlotInfo::BT_Set;
                                    const Value& vt_value = itr.GetVT().GetRaw(interfaceSlot.GetAValueInd() + 1);

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type SF_DEBUG_ARG(si.GetQualifiedName())));
                                }
                                break;
                            default:
                                SF_ASSERT(false);
                            };
                        }
                    }
                    else
                    {
                        // Class instance doesn't implement all traits of the interface!
                        // TBD: message with class, interface, and method name.
                        return GetVM().ThrowVerifyError(VM::Error(VM::eIllegalInterfaceMethodBodyError, GetVM()
                            SF_DEBUG_ARG(interfaceSlotName.ToCStr())
                            ));
                    }
                }
            }
            /* For debugging.
            else
                SF_ASSERT(false);
            */
        }

        // Do the same thing recursively for parent interfaces, if we have any.
        // The loop is not enough to propagate all the traits through interface
        // chain, since it only aliases slots that are overridden in the derived interface.
        InstanceTraits::Traits* parent_itr = const_cast<InstanceTraits::Traits*>(itr.GetParent());
        while (parent_itr)
        {
            parent_itr->AddInterfaceSlots2This(file_ptr, *this);

            parent_itr = const_cast<InstanceTraits::Traits*>(parent_itr->GetParent());
        }
    }

}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

