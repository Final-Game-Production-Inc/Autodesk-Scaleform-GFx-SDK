//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Dictionary.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_Dictionary.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

///////////////////////////////////////////////////////////////////////////////
void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, 
                     bool WeakKeys, 
                     ValueContainerType& v, 
                     RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    ValueContainerType::Iterator it;

#if 1 // DO NOT delete this block.
    // It looks like we are not allowed to alter RefCount (remove values from containers)
    // during GC process if RefCount <= 1. This can lead to crash.
    // 
    // According to AB:
    // "This is happening during the MarkInCycle phase that actually calls Decrement to 
    // refcnt in order to determine if there is a circular reference. If refcnt reached 0 
    // then the object is considered garbage (with some additional conditions). If Release 
    // is called this time (that also causes Decrement) then the refcnt goes into negative 
    // and this definitely may cause the assertion."

#if 0
    // Try to remove not valid weak references on first run.
    if (WeakKeys && op == RefCountBaseGC<Mem_Stat>::MarkInCycleCall)
    {
        bool needToRemove = true;
        while (needToRemove)
        {
            needToRemove = false;
            it = v.Begin();

            for (; !it.IsEnd(); ++it)
            {
                if (!it->First.IsValidWeakRef())
                {
                    if (it->Second.IsClosure())
                    {
                        Object* obj = it->Second.GetClosure();
                        if (obj && obj->GetRefCount() > 1)
                            goto remove;
                        else
                            continue;
                    }
                    else if (it->Second.IsObject())
                    {
                        Object* obj = it->Second.GetObject();
                        if (obj && obj->GetRefCount() > 1)
                            goto remove;
                        else
                            continue;
                    }

                    remove:
                    {
                        it.Remove();
                        needToRemove = true;
                        break;
                    }
                }
            }
        }
    }
#else
    // Try to remove not valid weak references on first run.
    if (WeakKeys && op == RefCountBaseGC<Mem_Stat>::MarkInCycleCall)
    {
        bool needToRemove = true;
        while (needToRemove)
        {
            needToRemove = false;
            it = v.Begin();

            for (; !it.IsEnd(); ++it)
            {
                if (!it->First.IsValidWeakRef())
                {
                    typedef PtrReleaseProxy<Mem_Stat> ReleaseProxy;

                    Object* obj = NULL;
                    if (it->Second.IsClosure())
                        obj = it->Second.GetClosure();
                    else if (it->Second.IsObject())
                        obj = it->Second.GetObject();

                    if (obj)
                    {
                        Ptr<ReleaseProxy> pReleaseProxy;
                        pReleaseProxy = *SF_HEAP_AUTO_NEW(&v) ReleaseProxy();
                        pReleaseProxy->Data2 = obj;
                        prcc->AddDelayedReleaseProxy(pReleaseProxy);
                    }

                    it.Remove();
                    needToRemove = true;
                    break;
                }
            }
        }
    }
#endif

#endif

    it = v.Begin();
    for (; !it.IsEnd(); ++it)
    {
        // Visit key.
        if (!WeakKeys || it->First.IsValidWeakRef())
            AS3::ForEachChild_GC(prcc, it->First, op SF_DEBUG_ARG(owner));

        // Visit value.
        AS3::ForEachChild_GC(prcc, it->Second, op SF_DEBUG_ARG(owner));
    }
}

//##protect##"methods"

namespace Instances { namespace fl_utils
{
    Dictionary::Dictionary(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Dictionary::Dictionary()$data"
    , WeakKeys(false)
//##protect##"instance::Dictionary::Dictionary()$data"
    {
//##protect##"instance::Dictionary::Dictionary()$code"
//##protect##"instance::Dictionary::Dictionary()$code"
    }


//##protect##"instance$methods"
    Dictionary::Dictionary(InstanceTraits::Traits& t, bool weakKeys)
        : Instances::fl::Object(t)
        , WeakKeys(weakKeys)
    {
    }

    Dictionary::~Dictionary()
    {
    }

    ValueContainerType::Iterator Dictionary::FindKey(const Multiname& prop_name) const
    {
        const Value& key = prop_name.GetName();
        ValueContainerType::Iterator it;

        Value::Number num;
        switch (key.GetKind())
        {
        case Value::kInt:
        case Value::kUInt:
            if (key.Convert2Number(num))
                it = ValueH.Find(Value(num));
            break;
        case Value::kString:
            if (GetStrNumber(key.AsStringNode(), num))
            {
                it = ValueH.Find(Value(num));
                break;
            }
            // No break on purpose.
        default:
            it = ValueH.Find(key);
        }

        return it;
    }

    void Dictionary::AddDynamicSlotValuePair(const Value& prop_name, const Value& value, SlotInfo::Attribute a)
    {
        SF_UNUSED1(a);

#if 0
        const Value& strong_key = prop_name;
        ValueContainerType::Iterator it = ValueH.Find(strong_key);

        if (!it.IsEnd())
        {
            if (!WeakKeys || it->First.IsValidWeakRef())
            {
                it->Second = value;
                return;
            }
            else
            {
                // Theoretically, we shouldn't get here because prop_name
                // should always be a strong reference.
                it.Remove();
                // New key/value pair will be added below.
            }
        }

        // Add a value.
        if (WeakKeys)
        {
            Value weak_key = strong_key;

            // Make a weak key.
            weak_key.MakeWeakRef();
            ValueH.Add(weak_key, value);
        }
        else
            ValueH.Add(strong_key, value);
#else
        Value strong_key = prop_name;

        if (strong_key.IsString())
        {
            Value::Number num;
            if (GetStrNumber(strong_key.AsStringNode(), num))
                strong_key.SetNumber(num);
        } else if (strong_key.IsInt())
            // There will be no problems here.
            strong_key.ToNumberValue().DoNotCheck();

        ValueContainerType::Iterator it = ValueH.Find(strong_key);

        if (!it.IsEnd())
        {
            if (!WeakKeys || it->First.IsValidWeakRef())
            {
                it->Second = value;
                return;
            }
            else
            {
                // Theoretically, we shouldn't get here because prop_name
                // should always be a strong reference.
                it.Remove();
                // New key/value pair will be added below.
            }
        }

        // Add a value.
        if (WeakKeys)
        {
            // Make a weak key.
            strong_key.MakeWeakRef();
        }

        ValueH.Add(strong_key, value);
#endif
    }

    PropRef Dictionary::FindDynamicSlot(const Multiname& prop_name)
    {
        // Dynamic properties are public.
        if (prop_name.ContainsNamespace(GetVM().GetPublicNamespace()))
        {
            ValueContainerType::Iterator it = FindKey(prop_name);

            if (!it.IsEnd())
            {
                if (!WeakKeys || it->First.IsValidWeakRef())
                    return PropRef(this, &it->Second);
                else
                    it.Remove();
            }
        }

        return PropRef();
    }

    CheckResult Dictionary::SetProperty(const Multiname& prop_name, const Value& value)
    {
        AddDynamicSlotValuePair(prop_name.GetName(), value);
        return true;
    }

    CheckResult Dictionary::GetProperty(const Multiname& prop_name, Value& value)
    {
        bool result = false;

        if (prop_name.ContainsNamespace(GetVM().GetPublicNamespace()))
        {
            ValueContainerType::Iterator it = FindKey(prop_name);

            if (!it.IsEnd())
            {
                if (!WeakKeys || it->First.IsValidWeakRef())
                {
                    value.Assign(it->Second);
                    result = true;
                }
                else
                    it.Remove();
            }
        }

        return result;
    }

    void Dictionary::GetDynamicProperty(AbsoluteIndex ind, Value& value)
    {
        ValueContainerType::ConstIterator bit = ValueH.Begin();
        ValueContainerType::ConstIterator it(bit.GetContainer(), ind.Get());

        value.Assign(it->Second);
    }

    CheckResult Dictionary::DeleteProperty(const Multiname& prop_name)
    {
        bool result = false;

        if (prop_name.ContainsNamespace(GetVM().GetPublicNamespace()))
        {
            ValueContainerType::Iterator it = FindKey(prop_name);

            if (!it.IsEnd())
            {
                it.Remove();
                result = true;
            }
        }

        return result;
    }

    void Dictionary::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC(prcc, WeakKeys, ValueH, op SF_DEBUG_ARG(*this));
    }

    void Dictionary::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        if (!ind.IsValid())
            return;

        ValueContainerType::ConstIterator bit = ValueH.Begin();
        ValueContainerType::ConstIterator it(bit.GetContainer(), AbsoluteIndex(ind).Get());

        name = it->First;
        if (WeakKeys)
        {
            SF_ASSERT(name.IsValidWeakRef());
            name.MakeStrongRef();
        }
    }

    GlobalSlotIndex Dictionary::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
        ValueContainerType::ConstIterator bit = ValueH.Begin();
        ValueContainerType::ConstIterator it(bit.GetContainer(), (ind.IsValid() ? static_cast<SPInt>(AbsoluteIndex(ind).Get()) : -1));

        if (!it.IsEnd())
        {
            for (++it; !it.IsEnd(); ++it)
            {
                if (!WeakKeys || it->First.IsValidWeakRef())
                    return GlobalSlotIndex(AbsoluteIndex(it.GetIndex()));
            }
        }

        return GlobalSlotIndex(0);
    }

    void Dictionary::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 0 && argv[0].Convert2Boolean())
            WeakKeys = true;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{

    Dictionary::Dictionary(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Dictionary::Dictionary()"
        // Not exactly an Array, but will let us use virtual methods instead of
        // global AS3::FindProperty().
        SetArrayLike();
        SetTraitsType(Traits_Dictionary);
//##protect##"InstanceTraits::Dictionary::Dictionary()"

    }

    void Dictionary::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Dictionary&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    Dictionary::Dictionary(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Dictionary::Dictionary()"
        SetTraitsType(Traits_Dictionary);
//##protect##"ClassTraits::Dictionary::Dictionary()"

    }

    Pickable<Traits> Dictionary::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Dictionary(vm, AS3::fl_utils::DictionaryCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::DictionaryCI));
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
    const TypeInfo DictionaryTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl_utils::Dictionary::InstanceType),
        0,
        0,
        0,
        0,
        "Dictionary", "flash.utils", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DictionaryCI = {
        &DictionaryTI,
        ClassTraits::fl_utils::Dictionary::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

