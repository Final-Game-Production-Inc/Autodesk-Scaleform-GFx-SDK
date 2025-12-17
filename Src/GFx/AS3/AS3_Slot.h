/**************************************************************************

Filename    :   AS3_Slot.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Slot_H
#define INC_AS3_Slot_H

#include "AS3_Index.h"
#include "AS3_GC.h"
#include "Abc/AS3_Abc.h"

// It is much more convenient to store slot's name in slot itself
// at this time.
#define SF_AS3_NAME_IN_SLOTINFO

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class VTable;

namespace Instances 
{
    class Dictionary;
}

namespace InstanceTraits
{
    class Traits;
}

namespace ClassTraits
{
    class Traits;
}

///////////////////////////////////////////////////////////////////////////
class SlotInfo
{
    friend class Traits;
    friend class InstanceTraits::Traits; // Because of setNamespace().
    
public:
    enum
    {
        aReadOnlyOffset     = 0,
        aDontEnumOffset     = 1,
        aDontDeleteOffset   = 2,
        aInternalOffset     = 3,
        aCppBindingOffset   = 4,
        aBindingTypeOffset  = 5,
        aValueIndOffset     = 9
    };
    enum
    {
        aBindingTypeMask    = 0xF,
        aValueIndMask       = 0xFFFF,
    };

    enum Attribute
    {
        aNone       = 0,
        aReadOnly   = 1 << aReadOnlyOffset, // Constant ???
        aDontEnum   = 1 << aDontEnumOffset,
        aDontDelete = 1 << aDontDeleteOffset, // Slot ???
        aInternal   = 1 << aInternalOffset,
        aCppBinding = 1 << aCppBindingOffset
    };

    enum ValTarget
    {
        valGet,
        valExecute,
    };
    
public:
    // 15 values.
    enum BindingType
    {
        // DO NOT reorder.
        BT_Unknown = 0,
#ifdef SF_AS3_CLASS_AS_SLOT
        BT_ValueArray, // It should be used only with GlobalObjectCPP to store classes.
#endif
        BT_Value,
        BT_ObjectAS,
        BT_ObjectCpp,
        BT_Boolean,
        BT_Int,
        BT_UInt,
        BT_Number,
        BT_String,
        BT_ConstChar,
        BT_Code,
        BT_Get,
        BT_Set,
        BT_GetSet
    };
    
public:
    // !!! We should get rid of this constructor ...
    SlotInfo();
    SlotInfo(
        Pickable<const Instances::fl::Namespace> ns,
        const ClassTraits::Traits* ctraits,
        int a = aDontEnum,
        const Ptr<ASStringNode>& name = NULL
        );
    SlotInfo(const SlotInfo& other);
    ~SlotInfo();

    SlotInfo& operator =(const SlotInfo& other);

private:
    // This constructor should be available only to Traits.
    SlotInfo(
        Pickable<const Instances::fl::Namespace> ns,
        VMAbcFile& file,
        const Abc::TraitInfo& ti,
        int a = aDontEnum,
        const Ptr<ASStringNode>& name = NULL
        );

public:
    //
    bool IsBound2Value() const
    {
        //return ValueInd.IsValid() && GetBindingType() != BT_Unknown;
        return ValueInd >= 0 && GetBindingType() != BT_Unknown;
    }
    //
    bool IsReadonly() const
    {
        //return GetAttrs() & aReadOnly;
        return ReadOnly != 0;
    }
    //
    bool IsDontEnum() const
    {
        return DontEnum != 0;
    }
    void SetDontEnum(bool flag = true)
    {
        DontEnum = flag;
    }
    //
    bool IsDontDelete() const
    {
        return DontDelete != 0;
    }
    //
    bool IsInternal() const
    {
        return Internal != 0;
    }
    //
    // If true this slot is bound to a class member.
    bool IsCppBinding() const
    {
        return CppBinding != 0;
    }
    //
    bool IsCode() const
    {
        return GetBindingType() > BT_ConstChar;
    }
    //
    bool IsMethod() const
    {
        return GetBindingType() == BT_Code;
    }
    //
    bool IsGetter() const
    {
        const BindingType bt = GetBindingType();
        return bt == BT_Get || bt == BT_GetSet;
    }
    //
    bool IsSetter() const
    {
        const BindingType bt = GetBindingType();
        // return bt == BT_Set || bt == BT_GetSet;
        return bt > BT_Get;
    }
    //
    bool IsReadWrite() const
    {
        const BindingType bt = GetBindingType();
        return (bt < BT_ConstChar || bt == BT_GetSet) && !IsConst();
    }

    //
    bool IsConst() const;
    bool IsClass() const;
    bool IsClassOrConst() const;

    static
    bool IsValidCodeBT(SlotInfo::BindingType bt)
    {
        return bt == BT_Unknown || bt >= BT_Code;
    }
    
public:
    // We need "data" for instances ...
    // Can throw exceptions.
    CheckResult GetSlotValueUnsafe(Value& value, Object* obj) const;
    // Can throw exceptions.
    CheckResult GetSlotValueUnsafe(VM& vm, Value& value, const Value& _this, const VTable* vt, ValTarget vtt = valGet) const;
    // Can throw exceptions.        
    CheckResult SetSlotValue(VM& vm, const Value& v, Object* const obj) const;
    // Can throw exceptions.        
    CheckResult SetSlotValue(VM& vm, const Value& v, const Value& _this, const VTable* vt = NULL) const;

    // NULL means an error. A VerifyError exception will be thrown in this case.
    const ClassTraits::Traits* GetDataType(VM& vm) const;

    // Two methods below work only with AS3 slots (when ABC is available).
    bool IsAnyType() const;
    bool IsClassType() const;

#ifdef GFX_AS3_VERBOSE
    ASString GetDataTypeName(VM& vm) const;
#endif

    const Abc::TraitInfo* GetTraitInfoPtr() const
    {
        return TI;
    }
    const VMAbcFile* GetFilePtr() const
    {
        return File;
    }

public:
    // GC related.
    typedef RefCountCollector<Mem_Stat> Collector;
    void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                         SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const;
    void ForEachChild_GC(Collector* prcc, const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const;
    
private:
    // Bind() is made separate from SlotInfo() constructor because of logic 
    // in Traits::Add2VT(), which can reset offset and/or BindingType.
    void Bind(BindingType dt, AbsoluteIndex offset)
    {
        SetBindingType(dt);
        SetValueInd(offset);
    }

public:
    const Instances::fl::Namespace& GetNamespace() const
    {
        SF_ASSERT(pNs);
        return *pNs;
    }
    BindingType GetBindingType() const
    {
        return static_cast<BindingType>(BindType);
    }
    SInt32 GetValueInd() const
    {
        return static_cast<SInt32>(ValueInd);
    }
    AbsoluteIndex GetAValueInd() const
    {
        return AbsoluteIndex(ValueInd);
    }
#ifdef SF_AS3_NAME_IN_SLOTINFO
    ASString GetQualifiedName() const { return ASString(Name.GetPtr()); }
    ASStringNode* GetQualifiedNameNode() const { return Name.GetPtr(); }
    ASString GetName() const;
#endif
    
private:
    //
    void setNamespace(const Instances::fl::Namespace& ns);

    //
    void DestroyPrimitiveMember(Object& obj) const;
    void ConstructPrimitiveMember(Object& obj) const;

    void SetBindingType(BindingType bt)
    {
        BindType = bt;
    }
    void SetValueInd(AbsoluteIndex offset)
    {
        ValueInd = offset.GetRaw();
    }
    void SetFlagsFromInt(int a);

private:
    // !!! There is a compiler problem with SPInt + bit-fields + WiiU 
    // compiler (04/09/2012) !!!
    // Total: 32 bits.
    SInt32  ReadOnly:2;
    SInt32  DontEnum:2;
    SInt32  DontDelete:2;
    SInt32  Internal:2;
    SInt32  CppBinding:2;
    SInt32  BindType:5; // One bit for sign.
    SInt32  ValueInd:17; // 16 bit for data (64K) plus one bit for sign.

#ifdef SF_64BIT_POINTERS
    SInt32  Padding01;
#endif    
    
    SPtr<const Instances::fl::Namespace>    pNs;
    
    mutable SPtr<const ClassTraits::Traits> CTraits;
    SPtr<VMAbcFile>                         File;
    const Abc::TraitInfo*                   TI;

#ifdef SF_AS3_NAME_IN_SLOTINFO
    Ptr<ASStringNode>                       Name;
#endif
};

///////////////////////////////////////////////////////////////////////////
class ASStringNodeHashFunc
{
public:
    // Hash code is stored right in the node
    UPInt operator()(const ASStringNode* data) const
    { return (UPInt) data->HashFlags; }
    UPInt operator()(const ASString& data) const
    { return (UPInt) data.GetHashFlags(); }
};

class SlotContainerType
{
public:
    typedef SlotContainerType SelfType;
    typedef SlotInfo ValueType;

    struct Pair
    {
        Pair() : Prev(-1) {}
        Pair(const ValueType& _1, const Ptr<ASStringNode>& _2) 
            : Key(_2)
            , Prev(-1)
            , Value(_1)
        {
        }

        Ptr<ASStringNode>   Key;
        SPInt               Prev;
        ValueType           Value;
    };

    typedef HashUncachedLH<ASStringNode*, SPInt, ASStringNodeHashFunc, StatMV_VM_SlotInfoHash_Mem> SetType;

public:
    SlotContainerType();

public:
    // Operators.
    ValueType& operator [](AbsoluteIndex ind) { return VArray[ind.Get()].Value; }
    const ValueType& operator [](AbsoluteIndex ind) const { return VArray[ind.Get()].Value; }

public:
    //
    UPInt GetSize() const { return Size; }

    //
    AbsoluteIndex FindFirst(const ASString& k) const
    {
        const SetType::ValueType* pArr = FindValues(k);
        return AbsoluteIndex((pArr == NULL) ? -1 : (*pArr < 0 ? -1 : *pArr));
    }

    SPInt GetPrevInd(SPInt ind) const
    {
        return VArray[ind].Prev;
    }

    //
    AbsoluteIndex Find(const ASString& k) const { return FindFirst(k); }
    const SetType::ValueType* FindValues(const ASString& k) const
    {
        return KVHashSet.Get(k.GetNode());
    }

    // Get first value.
    const ValueType& Get(const ASString& k) const
    {
        const AbsoluteIndex ind = Find(k);
        SF_ASSERT(ind.IsValid());

        return VArray[ind.Get()].Value;
    }

    //
    ASStringNode* GetKey(AbsoluteIndex ind) const
    {
        return VArray[ind.Get()].Key.GetPtr();
    }

    void SetKey(AbsoluteIndex ind, const ASString& k);

    // Return index of a newly added value.
    AbsoluteIndex Add(const ASString& k, const ValueType& v);

    //
    AbsoluteIndex FindAdd(const ASString& k, const ValueType& v);

    //
    bool IsEmpty() const { return GetSize() == 0; }

public:
    typedef RefCountCollector<Mem_Stat> Collector;
    void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op 
                         SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const;

private:
    typedef ArrayLH<Pair, StatMV_VM_SlotInfo_Mem> VArrayType;

    // Using separate value for size helps solve problems with Garbage Collector.
    UPInt       Size;
    VArrayType  VArray;
    SetType     KVHashSet;
};

///////////////////////////////////////////////////////////////////////////////
inline
void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, const SlotContainerType& v, RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    v.ForEachChild_GC(prcc, op SF_DEBUG_ARG(owner));
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Slot_H

