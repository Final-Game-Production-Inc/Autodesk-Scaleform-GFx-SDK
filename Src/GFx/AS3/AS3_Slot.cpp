/**************************************************************************

Filename    :   AS3_Slot.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_VM.h"
#include "AS3_Object.h"
#include "AS3_VTable.h"
#include "AS3_Slot.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_Global.h"

// We need to retrieve Class tag in case of AOTC
// Unfortunately, this affects performance.
#define SF_AS3_RETRIEVE_CLASS_TAG 1

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
SlotInfo::SlotInfo()
: ReadOnly(false)
, DontEnum(false)
, DontDelete(false)
, Internal(false)
, CppBinding(false)
, BindType(BT_Unknown)
, ValueInd(-1)
#ifdef SF_64BIT_POINTERS
, Padding01(0)
#endif    
, CTraits(NULL)
, File(NULL)
, TI(NULL)
{
}

SlotInfo::SlotInfo(
    Pickable<const Instances::fl::Namespace> ns, 
    const ClassTraits::Traits* ctraits,
    int a,
    const Ptr<ASStringNode>& name
    )
: ReadOnly(false)
, DontEnum(false)
, DontDelete(false)
, Internal(false)
, CppBinding(false)
, BindType(BT_Unknown)
, ValueInd(-1)
#ifdef SF_64BIT_POINTERS
, Padding01(0)
#endif    
, pNs(ns)
, CTraits(ctraits)
, File(NULL)
, TI(NULL)
#ifdef SF_AS3_NAME_IN_SLOTINFO
, Name(name)
#endif
{
    SetFlagsFromInt(a);
}

SlotInfo::SlotInfo(
     Pickable<const Instances::fl::Namespace> ns,
     VMAbcFile& file,
     const Abc::TraitInfo& ti,
     int a,
     const Ptr<ASStringNode>& name
     )
: ReadOnly(false)
, DontEnum(false)
, DontDelete(false)
, Internal(false)
, CppBinding(false)
, BindType(BT_Unknown)
, ValueInd(-1)
#ifdef SF_64BIT_POINTERS
, Padding01(0)
#endif    
, pNs(ns)
, File(&file)
, TI(&ti)
#ifdef SF_AS3_NAME_IN_SLOTINFO
, Name(name)
#endif
{
    SetFlagsFromInt(a);
}

SlotInfo::SlotInfo(const SlotInfo& other)
: ReadOnly(other.ReadOnly)
, DontEnum(other.DontEnum)
, DontDelete(other.DontDelete)
, Internal(other.Internal)
, CppBinding(other.CppBinding)
, BindType(other.BindType)
, ValueInd(other.ValueInd)
#ifdef SF_64BIT_POINTERS
, Padding01(0)
#endif    
, pNs(other.pNs)
, CTraits(other.CTraits)
, File(other.File)
, TI(other.TI)
#ifdef SF_AS3_NAME_IN_SLOTINFO
, Name(other.Name)
#endif
{
}

SlotInfo::~SlotInfo()
{
}

void SlotInfo::SetFlagsFromInt(int a)
{
    ReadOnly = (a & aReadOnly) != 0;
    DontEnum = (a & aDontEnum) != 0;
    DontDelete = (a & aDontDelete) != 0;
    Internal = (a & aInternal) != 0;
    CppBinding = (a & aCppBinding) != 0;
}

SlotInfo& SlotInfo::operator =(const SlotInfo& other)
{
    if (this != &other)
    {
        ReadOnly = other.ReadOnly;
        DontEnum = other.DontEnum;
        DontDelete = other.DontDelete;
        Internal = other.Internal;
        CppBinding = other.CppBinding;
        BindType = other.BindType;
        pNs = other.pNs;
        CTraits = other.CTraits;
        File = other.File;
        TI = other.TI;
        ValueInd = other.ValueInd;
#ifdef SF_AS3_NAME_IN_SLOTINFO
        Name = other.Name;
#endif
    }
    
    return *this;
}

bool SlotInfo::IsConst() const
{
    using namespace Abc;

    return IsReadonly() || (TI && (TI->GetType() == TraitInfo::tConst));
}

bool SlotInfo::IsClass() const
{
    using namespace Abc;

    // We are using BT_ValueArray to store classes in Global Object.
    // This is a dirty trick. There should be a better way to figure out that
    // we are dealing with classes.
    return BindType == BT_ValueArray || (TI && (TI->GetType() == TraitInfo::tClass));
}

bool SlotInfo::IsClassOrConst() const
{
    using namespace Abc;

    bool result = IsReadonly();
    if (!result && TI)
    {
        const TraitInfo::Type type = TI->GetType();
        result = (type == TraitInfo::tClass || type == TraitInfo::tConst);
    }

    return result;
}

CheckResult SlotInfo::GetSlotValueUnsafe(VM& vm, Value& value, const Value& _this, const VTable* vt, ValTarget vtt) const
{
    AbsoluteIndex vind(ValueInd);
    SF_ASSERT(vind.IsValid());

    const BindingType bt = GetBindingType();
    switch(bt)
    {
#ifdef SF_AS3_CLASS_AS_SLOT
    case BT_ValueArray:
        {
            SF_ASSERT(&vm.GetGlobalObjectCPP() == _this.GetObject());
            Instances::fl::GlobalObjectCPP* go = static_cast<Instances::fl::GlobalObjectCPP*>(_this.GetObject());
            value.AssignUnsafe(go->GetValueArray(vind));
        }
        break;
#endif
    case BT_Value:
        value.AssignUnsafe(GetSlotValue(_this.GetObject(), vind.Get()));
        break;
    case BT_ObjectAS:
        GetSlotObjectAS(_this.GetObject(), vind.Get()).GetValueUnsafe(value);
        break;
    case BT_ObjectCpp:
#if SF_AS3_RETRIEVE_CLASS_TAG
        {

            const UInt32 offset = vind.Get();
            SPtr<Object>* obj_cpp = (SPtr<Object>*)((char*)_this.GetObject() + offset);

            if (obj_cpp)
            {
                if (obj_cpp->GetPtr() != NULL && (*obj_cpp)->GetTraits().IsClassTraits())
                    value.AssignUnsafe(static_cast<Class*>(obj_cpp->GetPtr()));
                else
                    value.AssignUnsafe(obj_cpp->GetPtr());
            }
        }
#else
        value.AssignUnsafe(GetSlotObjectCpp(_this.GetObject(), vind.Get()));
#endif
        break;
    case BT_Boolean:
        value.SetBoolUnsafe(GetSlotBoolean(_this.GetObject(), vind.Get()));
        break;
    case BT_Int:
        value.SetSInt32Unsafe(GetSlotSInt(_this.GetObject(), vind.Get()));
        break;
    case BT_UInt:
        value.SetUInt32Unsafe(GetSlotUInt(_this.GetObject(), vind.Get()));
        break;
    case BT_Number:
        value.SetNumberUnsafe(GetSlotNumber(_this.GetObject(), vind.Get()));
        break;
    case BT_String:
        {
            ASStringNode* node = GetSlotStringNode(_this.GetObject(), vind.Get());

            if (node)
                value.AssignUnsafe(node);
            else
                // It is important to return Object, not a string in case of null.
                value.SetNullUnsafe();
        }
        break;
    case BT_ConstChar:
        value.AssignUnsafe(
            vm.GetStringManager().CreateConstString(
                GetSlotConstChar(_this.GetObject(), vind.Get())
                )
            );
        break;
    case BT_Code:
        {
            bool super = false;

            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();
            else
                super = true;

            // Original version.
            if (_this.IsObject() && vtt == valGet)
                vt->GetMethod(value, vind, *_this.GetObject(), super);
            else
                value.AssignUnsafe(vt->GetValue(vind));
        }
        break;
    case BT_Get:
    case BT_GetSet:
        {
            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

            const Value getter = vt->GetValue(vind);

            if (getter.IsCallable())
                vm.ExecuteUnsafe(getter, _this, value, 0, NULL);
            else
            {
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                    SF_DEBUG_ARG(getter)
                    ));
                return false;
            }
        }
        break;
    case BT_Set:
        // Error.
        vm.ThrowReferenceError(VM::Error(VM::eWriteOnlyError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData)
            SF_DEBUG_ARG(_this)
            ));
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::GetSlotValueUnsafe(Value& value, Object* obj) const
{
    AbsoluteIndex vind(ValueInd);
    SF_ASSERT(vind.IsValid());
    SF_ASSERT(obj);

    const BindingType bt = GetBindingType();
    switch(bt)
    {
#ifdef SF_AS3_CLASS_AS_SLOT
    case BT_ValueArray:
        {
            SF_ASSERT(&obj->GetVM().GetGlobalObjectCPP() == obj);
            const Instances::fl::GlobalObjectCPP* go = static_cast<const Instances::fl::GlobalObjectCPP*>(obj);
            value.AssignUnsafe(go->GetValueArray(vind));
        }
        break;
#endif
    case BT_Value:
        value.AssignUnsafe(GetSlotValue(obj, vind.Get()));
        break;
    case BT_ObjectAS:
        GetSlotObjectAS(obj, vind.Get()).GetValueUnsafe(value);
        break;
    case BT_ObjectCpp:
#if SF_AS3_RETRIEVE_CLASS_TAG
        {

            const UInt32 offset = vind.Get();
            SPtr<Object>* obj_cpp = (SPtr<Object>*)((char*)obj + offset);

            if (obj_cpp)
            {
                if (obj_cpp->GetPtr() != NULL && (*obj_cpp)->GetTraits().IsClassTraits())
                    value.AssignUnsafe(static_cast<Class*>(obj_cpp->GetPtr()));
                else
                    value.AssignUnsafe(obj_cpp->GetPtr());
            }
        }
#else
        // Old code.
        value.AssignUnsafe(GetSlotObjectCpp(obj, vind.Get()));
#endif
        break;
    case BT_Boolean:
        value.SetBoolUnsafe(GetSlotBoolean(obj, vind.Get()));
        break;
    case BT_Int:
        value.SetSInt32Unsafe(GetSlotSInt(obj, vind.Get()));
        break;
    case BT_UInt:
        value.SetUInt32Unsafe(GetSlotUInt(obj, vind.Get()));
        break;
    case BT_Number:
        value.SetNumberUnsafe(GetSlotNumber(obj, vind.Get()));
        break;
    case BT_String:
        {
            ASStringNode* node = GetSlotStringNode(obj, vind.Get());
            if (node)
                value.AssignUnsafe(node);
            else
                // It is important to return Object, not a string in case of null.
                value.SetNullUnsafe();
        }
        break;
    case BT_ConstChar:
        value.AssignUnsafe(
            obj->GetVM().GetStringManager().CreateConstString(
                GetSlotConstChar(obj, ValueInd)
                )
            );
        break;
    case BT_Code:
        // Be careful in case of a closure. "obj" is used to create a methods closure.
        obj->GetVT().GetMethod(value, vind, *obj, false);
        break;
    case BT_Get:
    case BT_GetSet:
        {
            const Value getter = obj->GetVT().GetValue(vind);

            VM& vm = obj->GetVM();
            if (getter.IsCallable())
                vm.ExecuteUnsafe(getter, Value(obj), value, 0, NULL);
            else
            {
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                    SF_DEBUG_ARG(obj->GetTraits().GetName().ToCStr())
                    ));
                return false;
            }
        }
        break;
    case BT_Set:
        {
            // Error.
            VM& vm = obj->GetVM();
            vm.ThrowReferenceError(VM::Error(VM::eWriteOnlyError, vm
                SF_DEBUG_ARG(GetQualifiedNameNode()->pData)
                SF_DEBUG_ARG(obj->GetTraits().GetName().ToCStr())
                ));
        }
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::SetSlotValue(VM& vm, const Value& v, const Value& _this, const VTable* vt) const
{
    // A ReferenceError is thrown if the property is const
    if (IsReadonly() && !vm.IsInInitializer())
    {
        // const properties can be assigned in initializers.
        vm.ThrowReferenceError(VM::Error(VM::eConstWriteError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData) 
            SF_DEBUG_ARG(_this)
            ));
        return false;
    }

    // Coerce to a slot data type.
    Value r;
    if (
        v.IsClass() || 
        //v.IsFunction() || // GetDataType() for functions doesn't work.
        IsCode()
        )
    {
        // Do not coerce classes.
        // Do not coerce Functions for the time being.
        r = v;
    } 
    else
    {
        const ClassTraits::Traits* ctr = GetDataType(vm);

        if (ctr == NULL)
        {
            vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(GetDataTypeName(vm))
                ));
            return false;
        }

        if (!ctr->Coerce(v, r))
        {
            vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(v)
                SF_DEBUG_ARG(GetDataTypeName(vm))
                ));
            return false;
        }
    }

    SF_ASSERT(ValueInd >= 0);
    const UInt32 vind = ValueInd;
    const BindingType bt = GetBindingType();
    switch(bt)
    {
#ifdef SF_AS3_CLASS_AS_SLOT
    case BT_ValueArray:
        {
            SF_ASSERT(&vm.GetGlobalObjectCPP() == _this.GetObject());
            Instances::fl::GlobalObjectCPP* go = static_cast<Instances::fl::GlobalObjectCPP*>(_this.GetObject());
            go->SetValueArray(AbsoluteIndex(ValueInd), r);
        }
        break;
#endif
    case BT_Value:
        AS3::SetSlotValue(_this.GetObject(), vind, r);
        break;
    case BT_ObjectAS:
        SetSlotObjectAS(_this.GetObject(), vind, r);
        break;
    case BT_ObjectCpp:
        SetSlotObjectCpp(_this.GetObject(), vind, r);
        break;
    case BT_Boolean:
        SetSlotBoolean(_this.GetObject(), vind, r);
        break;
    case BT_Int:
        SetSlotSInt(_this.GetObject(), vind, r);
        break;
    case BT_UInt:
        SetSlotUInt(_this.GetObject(), vind, r);
        break;
    case BT_Number:
        SetSlotNumber(_this.GetObject(), vind, Value::Number(r));
        break;
    case BT_String:
        SetSlotStringNode(_this.GetObject(), vind, r.GetStringNode());
        break;
    case BT_ConstChar:
        // No break on purpose.
    case BT_Get:
        // Error.
        vm.ThrowReferenceError(VM::Error(VM::eConstWriteError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData) 
            SF_DEBUG_ARG(_this)
            ));
        return false;
    case BT_Set:
    case BT_GetSet:
        {
            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

            const Value setter = vt->GetValue(AbsoluteIndex(vind + 1));

            if (setter.IsCallable())
            {
                Value result = Value::GetUndefined();

                vm.ExecuteUnsafe(setter, _this, result, 1, &r);
                if (vm.IsException())
                    return false;
            } else
            {
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                    SF_DEBUG_ARG(setter)
                    ));
                return false;
            }
        }
        break;
    case BT_Code:
        //if (_this.IsPrimitive() || vm.GetValueTraits(_this).IsSealed())
        if (_this.IsPrimitive())
        {
            // It looks like Tamarin works this way.
            // Primitive value is a sealed object.
            vm.ThrowReferenceError(VM::Error(VM::eWriteSealedError, vm
                SF_DEBUG_ARG(GetQualifiedNameNode()->pData)
                SF_DEBUG_ARG(_this)
                ));
        }
        else
        {
            // Sealed objects, which are not primitives seem to throw eCannotAssignToMethodError.
            vm.ThrowReferenceError(VM::Error(VM::eCannotAssignToMethodError, vm
                SF_DEBUG_ARG(GetQualifiedNameNode()->pData)
                SF_DEBUG_ARG(_this)
                ));
        }
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::SetSlotValue(VM& vm, const Value& v, Object* const obj) const
{
    SF_ASSERT(obj);

    // A ReferenceError is thrown if the property is const
    if (IsReadonly() && !vm.IsInInitializer())
    {
        // const properties can be assigned in initializers.
        vm.ThrowReferenceError(VM::Error(VM::eConstWriteError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData) 
            SF_DEBUG_ARG(obj->GetTraits().GetName().ToCStr())
            ));
        return false;
    }

    // Coerce to a slot data type.
    Value r;
    if (
        v.IsClass() || 
        //v.IsFunction() || // GetDataType() for functions doesn't work.
        IsCode()
        )
    {
        // Do not coerce classes.
        // Do not coerce Functions for the time being.
        r = v;
    } 
    else
    {
        const ClassTraits::Traits* ctr = GetDataType(vm);

        if (ctr == NULL)
        {
            vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(GetDataTypeName(vm))
                ));
            return false;
        }

        if (!ctr->Coerce(v, r))
        {
            vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(v)
                SF_DEBUG_ARG(GetDataTypeName(vm))
                ));
            return false;
        }
    }
    
    SF_ASSERT(ValueInd >= 0);
    const UInt32 vind = ValueInd;
    const BindingType bt = GetBindingType();
    switch(bt)
    {
#ifdef SF_AS3_CLASS_AS_SLOT
    case BT_ValueArray:
        {
            SF_ASSERT(&vm.GetGlobalObjectCPP() == obj);
            Instances::fl::GlobalObjectCPP* go = static_cast<Instances::fl::GlobalObjectCPP*>(obj);
            go->SetValueArray(AbsoluteIndex(ValueInd), r);
        }
        break;
#endif
    case BT_Value:
        AS3::SetSlotValue(obj, vind, r);
        break;
    case BT_ObjectAS:
        SetSlotObjectAS(obj, vind, r);
        break;
    case BT_ObjectCpp:
        SetSlotObjectCpp(obj, vind, r);
        break;
    case BT_Boolean:
        SetSlotBoolean(obj, vind, r);
        break;
    case BT_Int:
        SetSlotSInt(obj, vind, r);
        break;
    case BT_UInt:
        SetSlotUInt(obj, vind, r);
        break;
    case BT_Number:
        SetSlotNumber(obj, vind, Value::Number(r));
        break;
    case BT_String:
        SetSlotStringNode(obj, vind, r.GetStringNode());
        break;
    case BT_ConstChar:
        // No break on purpose.
    case BT_Get:
        // Error.
        vm.ThrowReferenceError(VM::Error(VM::eConstWriteError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData) 
            SF_DEBUG_ARG(obj->GetTraits().GetName().ToCStr())
            ));
        break;
    case BT_Set:
    case BT_GetSet:
        {
            const Value setter = obj->GetVT().GetValue(AbsoluteIndex(vind + 1));

            if (setter.IsCallable())
            {
                Value result = Value::GetUndefined();
                
                vm.ExecuteUnsafe(setter, Value(obj), result, 1, &r);
                if (vm.IsException())
                    return false;
            } else
            {
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                    SF_DEBUG_ARG(setter)
                    ));
                return false;
            }
        }
        break;
    case BT_Code:
        vm.ThrowReferenceError(VM::Error(VM::eCannotAssignToMethodError, vm
            SF_DEBUG_ARG(GetQualifiedNameNode()->pData)
            SF_DEBUG_ARG(obj->GetTraits().GetName().ToCStr())
            ));
        return false;
    default:
        return false;
    }

    return true;
}

template <typename T>
inline
void Destruct(char* addr)
{
    T* value = (T*)addr;
    value->~T();
}

void SlotInfo::DestroyPrimitiveMember(Object& obj) const
{
    SF_ASSERT(!IsCppBinding());

    char* addr = (char*)&obj + ValueInd;

    switch (GetBindingType())
    {
    case BT_Value:
        AS3::Destruct<Value>(addr);
        break;
    case BT_ObjectAS:
        AS3::Destruct<STPtr>(addr);
        break;
    case BT_String:
//         AS3::Destruct<ASString>(addr);
        {
            ASStringNode** node = (ASStringNode**)addr;
            if (*node)
                (*node)->Release();
        }
        break;
    default:
        break;
    }
}

void SlotInfo::ConstructPrimitiveMember(Object& obj) const
{
    SF_ASSERT(!IsCppBinding());

    char* addr = (char*)&obj + ValueInd;

    switch (GetBindingType())
    {
    case BT_Value:
        // Placement new.
        new(addr) Value();
        break;
    case BT_ObjectAS:
        // Placement new.
        new(addr) STPtr();
        break;
    case BT_String:
//         // Placement new.
//         new(addr) ASString(obj.GetStringManager().GetEmptyStringNode());
        *(ASStringNode**)addr = NULL;
        break;
    default:
        // Leave the rest uninitialized.
        break;
    }
}

void SlotInfo::ForEachChild_GC(Collector* prcc, const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    const BindingType bt = GetBindingType();
    switch(bt)
    {
#ifdef SF_AS3_CLASS_AS_SLOT
    case BT_ValueArray:
        {
            const Instances::fl::GlobalObjectCPP& go = static_cast<const Instances::fl::GlobalObjectCPP&>(obj);
            AS3::ForEachChild_GC(prcc, go.GetValueArray(AbsoluteIndex(ValueInd)), op SF_DEBUG_ARG(obj));
        }
        break;
#endif
    case BT_Value:
        AS3::ForEachChild_GC(prcc, *(Value*)((char*)&obj + ValueInd), op SF_DEBUG_ARG(obj));
        break;
    case BT_ObjectAS:
        {
            const STPtr& ptr = *(const STPtr*)((char*)&obj + ValueInd);

            ptr.ForEachChild_GC<Mem_Stat>(prcc, op SF_DEBUG_ARG(obj));
        }
        break;
    case BT_ObjectCpp:
        {
            const SPtr<Object>& ptr = *(const SPtr<Object>*)((char*)&obj + ValueInd);

            AS3::ForEachChild_GC<Object, Mem_Stat>(prcc, ptr, op SF_DEBUG_ARG(obj));
        }
        break;
    case BT_Boolean:
    case BT_Int:
    case BT_UInt:
    case BT_Number:
    case BT_String:
    case BT_ConstChar:
    case BT_Get:
    case BT_Set:
    case BT_GetSet:
    case BT_Code:
        break;
    default:
        // It is OK to have undefined binding because this situation can happen,
        // for example, when we are adding a slot, which is not bound yet
        // (happens all the time), and this causes garbage collection.
        // SF_ASSERT(false);
        break;
    }
}

const ClassTraits::Traits* SlotInfo::GetDataType(VM& vm) const
{
    if (CTraits.GetPtr() == NULL)
    {
        if (File != NULL && TI != NULL)
        {
            // Delayed type resolution.
            if (TI->IsData())
            {
                const Abc::Multiname& mn = TI->GetTypeName(File->GetAbcFile());
                const Multiname as3_nm(*File, mn); // For debugging.

                CTraits.SetPtr(vm.Resolve2ClassTraits(*File, mn));
            }
            else
                // !!! We probably should get result type here.
                CTraits.SetPtr(&vm.GetClassTraitsFunction());
        }
        else
        {
            const BindingType bt = GetBindingType();
            switch (bt)
            {
            case BT_Unknown:
#ifdef SF_AS3_CLASS_AS_SLOT
            case BT_ValueArray:
#endif
            case BT_Value:
                CTraits.SetPtr(&vm.GetClassTraitsClassClass());
                break;
            case BT_ObjectAS:
            case BT_ObjectCpp:
                CTraits.SetPtr(&vm.GetClassTraitsObject());
                break;
            case BT_Boolean:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsBoolean());
                break;
            case BT_Int:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsSInt());
                break;
            case BT_UInt:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsUInt());
                break;
            case BT_Number:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsNumber());
                break;
            case BT_String:
            case BT_ConstChar:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsString());
                break;
            case BT_Code:
            case BT_Get:
            case BT_Set:
            case BT_GetSet:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsFunction());
                break;
            }
        }
    }

    //SF_ASSERT(CTraits.GetPtr());
    return CTraits;
}

bool SlotInfo::IsAnyType() const
{
    bool result = false;

    if (File != NULL && TI != NULL)
    {
        const Abc::Multiname& mn = TI->GetTypeName(File->GetAbcFile());

        if (mn.IsAnyType())
            result = true;
    }

    return result;
}

bool SlotInfo::IsClassType() const
{
    bool result = false;

    if (File != NULL && TI != NULL)
    {
        const Abc::Multiname& mn = TI->GetTypeName(File->GetAbcFile());
        const Abc::ConstPool& cp = File->GetConstPool();
        const Abc::NamespaceInfo& ns = mn.GetNamespace(cp);

        if (mn.GetName(cp) == "Class" && ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = true;
    }

    return result;
}

#ifdef GFX_AS3_VERBOSE
ASString SlotInfo::GetDataTypeName(VM& vm) const
{
    ASString result = vm.GetStringManager().CreateEmptyString();

    if (CTraits.GetPtr() == NULL)
    {
        if (File != NULL && TI != NULL)
        {
            // Delayed type resolution.
            if (TI->IsData())
            {
                const Abc::Multiname& mn = TI->GetTypeName(File->GetAbcFile());
                result = File->GetInternedString(mn.GetNameInd());
            }
            else
                // !!! We probably should get result type here.
                result = vm.GetClassTraitsFunction().GetName();
        }
        else
        {
            const ClassTraits::Traits* tr = NULL;
            const BindingType bt = GetBindingType();

            switch (bt)
            {
            case BT_Unknown:
#ifdef SF_AS3_CLASS_AS_SLOT
            case BT_ValueArray:
#endif
            case BT_Value:
                CTraits.SetPtr(&vm.GetClassTraitsClassClass());
                break;
            case BT_ObjectAS:
            case BT_ObjectCpp:
                tr = &vm.GetClassTraitsObject();
                break;
            case BT_Boolean:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsBoolean();
                break;
            case BT_Int:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsSInt();
                break;
            case BT_UInt:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsUInt();
                break;
            case BT_Number:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsNumber();
                break;
            case BT_String:
            case BT_ConstChar:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsString();
                break;
            case BT_Code:
            case BT_Get:
            case BT_Set:
            case BT_GetSet:
                tr = (const ClassTraits::Traits*)&vm.GetClassTraitsFunction();
                break;
            }

            SF_ASSERT(tr);
            result = tr->GetName();
        }
    }
    else
        result = CTraits->GetName();

    return result;
}
#endif

void SlotInfo::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op 
                               SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
{
    AS3::ForEachChild_GC<VMAbcFile, Mem_Stat>(prcc, File, op SF_DEBUG_ARG(owner));
    AS3::ForEachChild_GC_Const<const Instances::fl::Namespace, Mem_Stat>(prcc, pNs, op SF_DEBUG_ARG(owner));
	AS3::ForEachChild_GC_Const<const ClassTraits::Traits, Mem_Stat>(prcc, CTraits, op SF_DEBUG_ARG(owner));
}

#ifdef SF_AS3_NAME_IN_SLOTINFO
ASString SlotInfo::GetName() const
{
    ASStringNode* name_ptr = GetQualifiedNameNode();
    SF_ASSERT(name_ptr);
    StringDataPtr qname(name_ptr->pData, name_ptr->Size);
    SPInt         index = qname.FindLastChar('/');

    if (index < 0)
        index = qname.FindLastChar('.');

    const StringDataPtr name = qname.GetTrimLeft(index + 1);

    return name_ptr->GetManager()->CreateString(name.ToCStr(), name.GetSize());
}
#endif

void SlotInfo::setNamespace(const Instances::fl::Namespace& ns)
{
    pNs = &ns;
}

///////////////////////////////////////////////////////////////////////////////
SlotContainerType::SlotContainerType()
: Size(0)
{
}

void SlotContainerType::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                                        SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
{
    // We MAY NOT use member Size here because of the Garbage Collector.
    // VArray and Size can be easily changed not synchronously.
    const UPInt size = VArray.GetSize();

    for(UPInt i = 0; i < size; ++i)
        VArray[i].Value.ForEachChild_GC(prcc, op SF_DEBUG_ARG(owner));
}

void SlotContainerType::SetKey(AbsoluteIndex ind, const ASString& k)
{
    const UPInt index = ind.Get();

    if (VArray[index].Key == k.GetNode())
        // The same key.
        return;

#if 0
    // This is not necessary.
    // Remove old value index.
    {
        SetType::Iterator it = KVHashSet.Find(VArray[index].Key);
        SPInt cur_ind = it->Second;
        SPInt prev_ind = cur_ind;

        while (cur_ind >= 0)
        {
            if (static_cast<UPInt>(cur_ind) == index)
            {
                if (cur_ind == prev_ind)
                    // This is a very first record.
                    it->Second = VArray[cur_ind].Prev;
                else
                    VArray[prev_ind].Prev = VArray[cur_ind].Prev;

                break;
            }

            prev_ind = cur_ind;
            cur_ind = VArray[cur_ind].Prev;
        }
    }
#endif

    // Set new value.
    {
        ASStringNode* node = k.GetNode();
        SF_ASSERT(ASString(VArray[index].Key) == k.GetManager()->CreateEmptyString());
        VArray[index].Key = node;
        SetType::Iterator it = KVHashSet.Find(node);

        if (it.IsEnd())
        {
            KVHashSet.Add(node, index);
            VArray[index].Prev = -1;
        }
        else
        {
            const SPInt cur_ind = it->Second;
            VArray[index].Prev = cur_ind;
            it->Second = index;
        }
    }
}

Scaleform::GFx::AS3::AbsoluteIndex SlotContainerType::Add(const ASString& k, const ValueType& v)
{
    UPInt result;

    {
        // No free indices.
        // We need to add new record.

        result = VArray.GetSize();
        VArray.PushBack(VArrayType::ValueType(v, k.GetNode()));

        // Size MUST be changed after all PushBack() calls because PushBack()
        // may call Garbage Collector, which will visit all elements of arrays.

        ++Size;
    }

    ASStringNode* node = k.GetNode();

    const SPInt index = static_cast<SPInt>(result);
    SetType::Iterator it = KVHashSet.Find(node);
    if (it.IsEnd())
    {
        KVHashSet.Add(node, index);
        VArray[index].Prev = -1;
    }
    else
    {
        const SPInt cur_ind = it->Second;
        VArray[index].Prev = cur_ind;
        it->Second = index;
    }

    return AbsoluteIndex(result);
}

Scaleform::GFx::AS3::AbsoluteIndex SlotContainerType::FindAdd( const ASString& k, const ValueType& v )
{
    AbsoluteIndex result(-1);

    ASStringNode* node = k.GetNode();
    SetType::Iterator it = KVHashSet.Find(node);
    if (it.IsEnd())
        result = Add(k, v);
    else
        result = AbsoluteIndex(it->Second);

    return result;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


