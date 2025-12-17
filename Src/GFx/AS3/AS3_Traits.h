/**************************************************************************

Filename    :   AS3_Traits.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Traits_H
#define INC_AS3_Traits_H

#include "AS3_Object.h"
#include "Kernel/SF_AutoPtr.h"

namespace Scaleform { namespace GFx { namespace AS3
{
    
///////////////////////////////////////////////////////////////////////////
class ASStringNodePtrHashFunc
{
public:
    // Hash code is stored right in the node
    UPInt operator()(const Ptr<ASStringNode>& data) const
    // We MAY NOT use GetHashCode() instead of raw HashFlags here.
    // This will break a lot of tests.
    { return (UPInt) data->HashFlags; }
};

// This implementation is based on "copy-on-write" semantic.
// Parent's records are immutable.
class Slots
{
public:
    typedef SlotInfo ValueType;
    typedef HashUncachedLH<Ptr<ASStringNode>, UInt32, ASStringNodePtrHashFunc, StatMV_VM_SlotInfoHash_Mem> SetType;

    class CIterator
    {
        friend class Slots;
        friend class Traits;

    public:
        CIterator(const CIterator& other) : Ind(other.Ind), Parent(other.Parent) {}

    private:
        CIterator(const Slots& p, AbsoluteIndex ind) : Ind(ind), Parent(&p) {}

    public:
        bool operator==(const CIterator& it) const { return Parent == it.Parent && Ind.Get() == it.Ind.Get(); }
        bool operator!=(const CIterator& it) const { return Parent != it.Parent || Ind.Get() != it.Ind.Get(); }

        CIterator& operator++()
        {
            if (Ind < Parent->GetSlotInfoNum())
                ++Ind;

            return *this;
        }
        CIterator operator++(int)
        {
            CIterator it(*this);
            operator++();
            return it;
        }
        CIterator& operator--()
        {
            if (Ind.IsValid())
                --Ind;

            return *this;
        }
        CIterator operator--(int)
        {
            CIterator it(*this);
            operator--();
            return it;
        }

        bool operator<(const CIterator& other)
        {
            return Parent == other.Parent && Ind < other.Ind;
        }
        bool operator>(const CIterator& other)
        {
            return Parent == other.Parent && Ind > other.Ind;
        }

        bool IsEnd() const
        {
            return Ind.Get() >= Parent->GetSlotInfoNum();
        }

        const SlotInfo& GetSlotInfo() const
        {
            return Parent->GetSlotInfo(Ind);
        }
        ASStringNode* GetSlotName() const
        {
            return Parent->GetSlotNameNode(Ind);
        }

    private:
        AbsoluteIndex   Ind;
        const Slots*    Parent;
    };

    //
    CIterator Begin() const
    {
        return CIterator(*this, AbsoluteIndex(0));
    }
    CIterator End() const
    {
        return CIterator(*this, AbsoluteIndex(GetSlotInfoNum()));
    }

    //
    CIterator RBegin() const
    {
        return CIterator(*this, AbsoluteIndex(GetSlotInfoNum() - 1));
    }
    CIterator REnd() const
    {
        return CIterator(*this, AbsoluteIndex(-1));
    }

    class SlotFunct
    {
    public:
        virtual ~SlotFunct() {}

    public:
        virtual void operator()(const SlotInfo& si) = 0;
    };

    void ForEachSlot(SlotFunct& f) const;

public:
    Slots()
        : FirstOwnSlotNum(0)
        , Parent(NULL)
    {
    }

public:
    void Inherit(const Slots& parent);

    UPInt GetSlotInfoNum() const
    {
        return FirstOwnSlotNum + VArray.GetSize();
    }
    const SlotInfo& GetSlotInfo(AbsoluteIndex ind) const
    {
        SF_ASSERT(ind.IsValid());

        if (ind < FirstOwnSlotNum)
        {
            SF_ASSERT(Parent);
            return Parent->GetSlotInfo(ind);
        }

        return VArray[ind.Get() - FirstOwnSlotNum].Value;
    }
    ASStringNode* GetSlotNameNode(AbsoluteIndex ind) const
    {
        SF_ASSERT(ind.IsValid());

        if (ind < FirstOwnSlotNum)
        {
            SF_ASSERT(Parent);
            return Parent->GetSlotNameNode(ind);
        }

        return VArray[ind.Get() - FirstOwnSlotNum].Key;
    }
    ASString GetSlotName(AbsoluteIndex ind) const
    {
        return ASString(GetSlotNameNode(ind));
    }

    AbsoluteIndex AddSlotInfo(const ASString& name, const ValueType& v)
    {
        return Add(name, v);
    }

public:
    // Obtains a slot by index name + ns, returning null if not found.
    const SlotInfo* FindSlotInfo(const ASString& name, const Instances::fl::Namespace& ns) const
    {
        AbsoluteIndex ind = FindSlotInfoIndex(name, ns);

        return (ind.IsValid() ? &GetSlotInfo(ind) : NULL);
    }
    // Skip namespace check.
    const SetType::ValueType* FindSlotValues(const ASString& name) const
    {
        return Set.Get(name.GetNode());
    }

    SPInt GetPrevSlotIndex(SPInt ind) const
    {
        SF_ASSERT(ind >= 0);

        if (static_cast<UPInt>(ind) < FirstOwnSlotNum)
        {
            SF_ASSERT(Parent);
            return Parent->GetPrevSlotIndex(ind);
        }

        return VArray[ind - FirstOwnSlotNum].Prev;
    }

public:
    void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, RefCountBaseGC<Mem_Stat>::GcOp op 
        SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
    {
        const UPInt size = VArray.GetSize();

        for(UPInt i = 0; i < size; ++i)
            VArray[i].Value.ForEachChild_GC(prcc, op SF_DEBUG_ARG(owner));
    }

protected:
    AbsoluteIndex FindSlotInfoIndex(const ASString& name, const Instances::fl::Namespace& ns) const;

    // "ind" is global (includes parent slots)
    SlotInfo& GetOwnSlotInfo(AbsoluteIndex ind)
    {
        SF_ASSERT(ind.IsValid());
        SF_ASSERT(ind.Get() >= FirstOwnSlotNum);

        return VArray[ind.Get() - FirstOwnSlotNum].Value;
    }
    void SetSlotInfo(AbsoluteIndex ind, const ASString& name, const SlotInfo& v);

    // FindAdd finds a name, also considering namespace of the slot.
    SlotInfo& FindAddOwnSlotInfo(const ASString& name, const SlotInfo& v, AbsoluteIndex& ind);

protected:
    void SetKey(AbsoluteIndex ind, const ASString& k);
    // Return index of a newly added value.
    AbsoluteIndex Add(const ASString& k, const ValueType& v);

private:
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
    typedef ArrayLH<Pair, StatMV_VM_SlotInfo_Mem> VArrayType;

    UPInt           FirstOwnSlotNum;
    const Slots*    Parent;
    VArrayType      VArray;
    SetType         Set;
};


/////////////////////////////////////////////////////////////////cls/////////
struct MemberInfo
{
    const char*             Name;
    const char*             NamespaceName;
    // const Abc::TraitInfo*   TI; // ????

#ifdef SF_CC_GNU
    // GNU seems to have a bug related to initialization of bit fields.
    unsigned short          Offset;
#else
    unsigned                Offset:16; // 64K
#endif

    Abc::NamespaceKind      NsKind:4;
    SlotInfo::BindingType   BT:5; // One bit can be saved.
    unsigned                Const:1;
};

/////////////////////////////////////////////////////////////////cls/////////
// Forward declaration

namespace Classes
{
	class ClassClass;
    class UserDefined;
    class UDBase;
}

namespace Instances
{
    namespace fl
    {
        class GlobalObjectScript;
        class GlobalObjectCPP;
    }
}

namespace InstanceTraits
{
    class Traits;

#if defined(SF_AS3_AOTC2)
    class AOTraits;
#endif
}

namespace ClassTraits
{
    class UserDefined;
    class ClassClass;
    class Function;
}

class GC;
class VM;

///////////////////////////////////////////////////////////////////////////
class Traits : public GASRefCountBase, public Slots
{
    friend class VM; // Because of StoreScopeStack().
    friend class Class; // Because of SetConstructor().
    friend class Classes::UserDefined; // Because of StoreScopeStack().
    friend class Classes::UDBase; // Because of ConstructTail(), e.t.c.
    friend class ClassTraits::ClassClass; // Because of SetParentTraits().
    friend class ClassTraits::UserDefined; // Because of StoreScopeStack().
    friend class ClassTraits::Traits; // Required for ClassTraits::ClassClass.
    friend class ClassTraits::Function; // Because of SetConstructor().
    friend class InstanceTraits::UserDefined; // Because of SetupSlotValues().
    friend class Instance; // Because of ConstructTail(), e.t.c.
    friend class Instances::fl::GlobalObject; // Because of Add2VT().
    friend class Instances::fl::GlobalObjectCPP; // Because of Add2VT().
    friend class Instances::fl::GlobalObjectScript; // Because of SetupSlotValues().
    
#ifdef GFX_AS3_VERBOSE
private:
    virtual const char* GetAS3ObjectType() const { return "Traits"; }
    virtual Scaleform::String GetAS3ObjectName() const { return Scaleform::String(GetQualifiedName().ToCStr()); }
#endif
public:
    typedef Traits Self;

public:
    // "pt" - traits of the base class.
    // VM is not *const* on purpose (because of ExecuteCode() ) ...
    Traits(VM& _vm, const Traits* pt, bool isDynamic, bool isFinal);
    ~Traits();

private:
    // Special case for the ClassTraits::ClassClass object ...
    // VM is not *const* on purpose (because of ExecuteCode() ) ...
    Traits(VM& _vm);
        
public:
    // Only instance traits can be global.
    virtual bool IsGlobal() const;

public:
    // Constructor may be set up on demand.
    bool HasConstructorSetup() const
    {
        return pConstructor.GetPtr() != NULL;
    }
    const Class& GetConstructor() const;
    Class& GetConstructor();

    //
    const Traits* GetParent() const
    {
        return pParent;
    }

    //
    bool IsParentOf(const Traits& other) const;

    //
    const Class& GetClass() const
    {
        return GetConstructor();
    }
    Class& GetClass()
    {
        return GetConstructor();
    }

    //
    BuiltinTraitsType GetTraitsType() const { return TraitsType; }

    //
    enum QNameFormat { qnfWithColons, qnfWithDot };
    virtual ASString GetName() const = 0;
    virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const = 0;

public:
    unsigned GetMemSize() const { return MemSize; }
    virtual unsigned GetFixedMemSize() const;

public:
    bool IsDynamic() const { return (Flags & ObjectTypeMask) != 0; }
    bool IsSealed() const { return (Flags & ObjectTypeMask) == 0; }
    // If true object behaves as an array.
    bool IsArrayLike() const { return (Flags & ArrayLikeMask) != 0; }
    bool IsClassTraits() const { return (Flags & IsClassMask) != 0; }
    bool IsInstanceTraits() const { return (Flags & IsClassMask) == 0; }
    // This class is interface.
    bool IsInterface() const { return (Flags & IsInterfaceMask) != 0; }
    // Instances of this Traits have tail.
    bool HasTail() const { return (Flags & HasTailMask) != 0; }
    bool IsAbcObject() const { return (Flags & IsAbcObjectMask) != 0; }
    bool IsUserDefined() const { return IsAbcObject(); }
    bool IsFinal() const { return (Flags & IsFinalMask) != 0; }

    // TmpBit can be used by various algorithms.
    bool IsTmpBit() const { return (Flags & TmpBitMask) != 0; }
    void SetTmpBit(bool flag = true) const
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~TmpBitMask) | (-static_cast<SPInt>(flag) & TmpBitMask);
    }

public:
    // Should never be NULL.
    Instances::fl::Namespace& GetPublicNamespace() const;

public:
    VM& GetVM() const
    {
        SF_ASSERT(pVM);
        return *pVM;
    }
    VTable& GetVT() const;
    virtual VMAbcFile* GetFilePtr() const;
    VMAppDomain& GetAppDomain() const;
    StringManager& GetStringManager() const;
    GC& GetGC() const;
    Traits& GetSelf()
    {
        return *this;
    }
    // Get number of fixed slots, which are stored in an array of Values.
    UPInt GetFixedValueSlotNumber() const
    {
        return FixedValueSlotNumber;
    }
    const ScopeStackType& GetStoredScopeStack() const
    {
        return InitScope;
    }

    ASString GetASString(const char *pstr) const;
    ASString GetASString(const char *pstr, UPInt length) const;
    ASString GetASString(const String& str) const;

public:
    ///
    // Can throw exceptions.
    // Return false in case of a problem or an exception.
    CheckResult GetSlotValueUnsafe(Value& value, const ASString& name, const Instances::fl::Namespace& ns, Object* const obj = NULL) const 
    {
        const SlotInfo* si = Slots::FindSlotInfo(name, ns);

        return si != NULL && si->GetSlotValueUnsafe(value, obj);
    }
    // Can throw exceptions.
    // Return false in case of a problem or an exception.
    CheckResult GetSlotValueUnsafe(Value& value, AbsoluteIndex ind, Object* const obj = NULL) const 
    {
        return GetSlotInfo(ind).GetSlotValueUnsafe(value, obj);
    }
    
    ///
    // Can throw exceptions.        
    CheckResult SetSlotValue(const ASString& name, const Instances::fl::Namespace& ns, const Value& v, Object* const obj = NULL)
    {
        const SlotInfo* si = Slots::FindSlotInfo(name, ns);
        
        return si != NULL && si->SetSlotValue(GetVM(), v, obj);
    }
    // Can throw exceptions.        
    CheckResult SetSlotValue(AbsoluteIndex ind, const Value& v, Object* const obj = NULL)
    {
        return GetSlotInfo(ind).SetSlotValue(GetVM(), v, obj);
    }
    
public:
    const SlotInfo* FindSlotInfo(const ASString& name, const Instances::fl::Namespace& ns) const
    {
        return Slots::FindSlotInfo(name, ns);
    }
    const SlotInfo* FindSlotInfo(const Multiname& mn) const 
    {
        return Slots::FindSlotInfo(mn.GetName(), mn.GetNamespace());
    }
    const SlotInfo* FindSlotInfo(VMAbcFile& file, const Abc::Multiname& mn) const;
    const SlotInfo& GetSlotInfo(AbsoluteIndex ind) const
    {
        return Slots::GetSlotInfo(ind);
    }

protected:
    // "ind" is global (includes parent slots)
    SlotInfo& GetOwnSlotInfo(AbsoluteIndex ind)
    {
        return Slots::GetOwnSlotInfo(ind);
    }

protected:
    void SetSlot(AbsoluteIndex index, const ASString& name, Pickable<const Instances::fl::Namespace> ns,
                      VMAbcFile& file, const Abc::TraitInfo& ti, bool _const = false)
    {
        SF_ASSERT(index.Get() < GetSlotInfoNum());

        SetSlotInfo(
            index,
            name,
            SlotInfo(
                ns,
                file,
                ti,
                (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum),
                name.GetNode()
                )
            );
    }

    void AddSlotCPP(const ASString& name, Pickable<const Instances::fl::Namespace> ns,
                 const ClassTraits::Traits* tr, SlotInfo::BindingType dt, 
                 AbsoluteIndex offset, bool _const);
    void AddSlot(const ASString& name, Pickable<const Instances::fl::Namespace> ns,
                 const ClassTraits::Traits* tr, SlotInfo::BindingType dt, 
                 AbsoluteIndex offset, bool _const);
    AbsoluteIndex AddSlot(const ASString& name, Pickable<const Instances::fl::Namespace> ns,
        VMAbcFile& file, const Abc::TraitInfo& ti, bool const_);

    void AddSlot(
        const ASString& name, Pickable<const Instances::fl::Namespace> ns,
        SlotInfo::BindingType dt, 
        AbsoluteIndex offset,
        bool const_ = false
        );

public:
    // This call will not bind data.
    SlotInfo& AddSetSlot(RelativeIndex ind, const ASString& name, Pickable<const Instances::fl::Namespace> ns,
        VMAbcFile& file, const Abc::TraitInfo& ti, bool const_ = false);

    // Add a bunch of traits.
    // Return false in case of error.
    CheckResult AddSlots(const Abc::HasTraits& traits, VMAbcFile& file, unsigned parent_size);

    void AddSlot(const MemberInfo& mi);

public:
#ifdef SF_AS3_CLASS_AS_SLOT
    void AddSlotValueCPP(
        const ASString& name, 
        Pickable<const Instances::fl::Namespace> ns,
        const ClassTraits::Traits* tr
        )
    {
        AddSlotCPP(name, ns, tr, SlotInfo::BT_ValueArray, AbsoluteIndex(FixedValueSlotNumber++), false);
    }
#endif
    
public:
    // Set up values of slots ...
    // This method is not *const* because we can change slot values for virtual methods (VTable) ...
    virtual CheckResult SetupSlotValues(Object& /*for_obj*/) const;
    virtual Instances::fl::GlobalObjectScript* GetGlobalObjectScript() const;

    // Allocate memory for an Instance of this type (or type t if provided).
    void* Alloc() const;

public:
    GlobalSlotIndex GetNextPropertyIndex(GlobalSlotIndex ind);
    
    AbsoluteIndex GetFirstOwnSlotIndex() const
    {
        return FirstOwnSlotInd;
    }

    CIterator BeginFirstOwnSlot() const
    {
        return CIterator(*this, GetFirstOwnSlotIndex());
    }

public:
    virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;
    void ForEachChild_GC(Collector* prcc, const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const;
    void ForEachChild_GC_Slot(Collector* prcc, const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const;
    
protected:
    void SetArrayLike(bool flag = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~ArrayLikeMask) | (-static_cast<SPInt>(flag) & ArrayLikeMask);
    }
    void SetIsInterface(bool flag = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~IsInterfaceMask) | (-static_cast<SPInt>(flag) & IsInterfaceMask);
    }
    void SetDynamicType(bool isDynamic)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~ObjectTypeMask) | (-static_cast<SPInt>(isDynamic) & ObjectTypeMask);
    }
    void SetFinal(bool isFinal)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~IsFinalMask) | (-static_cast<SPInt>(isFinal) & IsFinalMask);
    }
    void SetHasTail(bool flag = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~HasTailMask) | (-static_cast<SPInt>(flag) & HasTailMask);
    }
    void SetIsAbcObject(bool flag = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~IsAbcObjectMask) | (-static_cast<SPInt>(flag) & IsAbcObjectMask);
    }
    void SetIsClassTraits(bool flag = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~IsClassMask) | (-static_cast<SPInt>(flag) & IsClassMask);
    }

protected:
    // Set up values of slots ...
    CheckResult SetupSlotValues(VMAbcFile& file, const Abc::HasTraits& t, Object& for_obj) const;
    SlotInfo::BindingType GetBindingType(VMAbcFile& file, const Abc::Multiname& mn) const;

protected:
    void SetMemSize(unsigned size)
    {
        MemSize = size;
    }
    void CalculateMemSize(unsigned parent_size);

protected:
    virtual void InitOnDemand() const;
    static const ClassTraits::Traits* RetrieveParentClassTraits(VM& vm, const ClassInfo& ci, VMAppDomain& appDomain);

protected:
    CheckResult RegisterWithVT(const ASString& mn_name, const SlotInfo& nsi, const Value& v, SlotInfo::BindingType bt);

    // This method will not necessarily add to VT. It can just update VT.
    // It is just a wrapper for RegisterWithVT, which exists for convenience purpose.
    // Can throw exceptions.
    void Add2VT(const ClassInfo& ci, const ThunkInfo& func);
    void SetTraitsType(BuiltinTraitsType tt)
    {
        TraitsType = tt;
    }

    void ClearInitScope()
    {
        InitScope.Clear();
    }

protected:
    void SetConstructor(Class& c)
    {
        SF_ASSERT(pConstructor == NULL);
        pConstructor = &c;
    }
    void SetConstructor(Pickable<Class> pc)
    {
        SF_ASSERT(pConstructor == NULL);
        pConstructor = pc;
    }

private:
    void Add2StoredScopeStack(const Value& o);
    void StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss);

    // Add Virtual Table value.
    void Add2VT(
        SlotInfo& si, 
        const Value& v, 
        SlotInfo::BindingType new_bt
        );
    void Add2VT(
        const ASString& mn_name,
        const Instances::fl::Namespace& mn_ns,
        const SlotInfo& nsi,
        const Value& v,
        SlotInfo::BindingType bt
        );
    // This method doesn't change SlotInfo.
    // It is supposed to be used with overridden methods.
    void UpdateVT(
        const SlotInfo& si, 
        const Value& v, 
        SlotInfo::BindingType new_bt
        );
    // Update VT for Interface Method.
    void UpdateVT4IM(
        const ASString& mn_name, 
        const Instances::fl::Namespace& mn_ns, 
        const Value& v, 
        SlotInfo::BindingType bt
        );

    static SlotInfo::BindingType GetNewBT(SlotInfo::BindingType bt, SlotInfo::BindingType new_bt);

    // This method should be used only in case of UserDefined traits.
    void SetParentTraits(const Traits* pt)
    {
        SF_ASSERT(pParent.GetPtr() == NULL);
        pParent.SetPtr(pt);
    }
    void DestructTail(Object& obj) const;
    void ConstructTail(Object& obj) const;

    CheckResult AddSlotsWithID(const Abc::HasTraits& traits, VMAbcFile& file);
    // members - true if members, false if code.
    CheckResult AddSlotsWithoutID(const Abc::HasTraits& traits, VMAbcFile& file, bool members);

protected:
    // Index of the very first not inherited slot.
    // This member is a proxy between GetParent().GetSlotInfoNum() in case of InstanceTraits
    // and ClassTraits, which do not inherit parent's traits.
    AbsoluteIndex       FirstOwnSlotInd;
    // Number of fixed slots, which are stored in an array of Values.
    // This member is currently used only by GlobalObjectCPP.
    UPInt               FixedValueSlotNumber;
    
private:
    enum {
        ArrayLikeOffset = 0,
        ObjectTypeOffset = 1,
        IsInterfaceOffset = 2,
        HasTailOffset = 3,
        IsAbcObjectOffset = 4,
        IsClassOffset = 5,
        IsFinalOffset = 6,
        TmpBitOffset = 7,
    };
    enum {
        ArrayLikeMask = 1 << ArrayLikeOffset,
        ObjectTypeMask = 1 << ObjectTypeOffset, // Matches ObjectType in ClasInfo.
        IsInterfaceMask = 1 << IsInterfaceOffset,
        HasTailMask = 1 << HasTailOffset,
        IsAbcObjectMask = 1 << IsAbcObjectOffset,
        IsClassMask = 1 << IsClassOffset,
        IsFinalMask = 1 << IsFinalOffset,
        TmpBitMask = 1 << TmpBitOffset,
    };

    unsigned            MemSize; // MemSize = FixedMemSize + TailMemSize.
    mutable SPInt       Flags;

    // It is used to speed up type info retrieval.
    BuiltinTraitsType   TraitsType;

    VM*                 pVM;
    
    // A class, which created this object ...
    // constructor is *not const* on purpose ...
    SPtr<Class>         pConstructor;
    SPtr<const Traits>  pParent;
    mutable AutoPtr<VTable> pVTable;
    // We keep a copy of scope stack.
    ScopeStackType      InitScope;
}; // class Traits

namespace ClassTraits
{
    // ClassTraits are different from InstanceTraits in a way that they do not
    // combine Slots from parent classes to speed up search.
    class Traits : public AS3::Traits
    {
        friend class AS3::VM; // Because of SetInstanceTraits().
        friend class ClassClass; // Because of Traits(VM& vm) ...

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits"; }
        virtual Scaleform::String GetAS3ObjectName() const { return Scaleform::String(GetQualifiedName().ToCStr()); }
#endif
    public:
        Traits(VM& vm, const ClassInfo& ci);
        // pt must always be NULL. We need it just to make a signature 
        // different from Traits(VM& vm), which is supposed to be used only
        // with ClassClass.
        Traits(VM& vm, const ClassTraits::Traits* pt);

    public:
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;
        void RegisterSlots();

        const Traits* GetParent() const
        {
            return static_cast<const Traits*>(AS3::Traits::GetParent());
        }
        InstanceTraits::Traits& GetInstanceTraits() const
        {
            SF_ASSERT(IsValid());
            return *ITraits;
        }
        bool IsValid() const
        {
            return ITraits.GetPtr() != NULL;
        }

    public:
        // From documentation to the *istype* opcode:
        // let valueType refer to *this* type. If valueType is the same as otherType, result
        // is true. If otherType is a base type of valueType, or an implemented interface of valueType,
        // then result is true. Otherwise result is set to false.
        bool IsParentTypeOf(const Traits& other) const;
        bool IsOfType(const Traits& other) const
        {
            return other.IsParentTypeOf(*this);
        }

    public:
        // Coerce *value* to this type (value of this class) ...
        virtual bool Coerce(const Value& value, Value& result) const;
        bool CoerceValue(const Value& value, Value& result) const;

        virtual ASString GetName() const;
        virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const;

        // temporarily public.
        void SetInstanceTraits(Pickable<InstanceTraits::Traits> itr);

    protected:
        // Can throw exceptions.
        virtual void InitOnDemand() const;

    private:
        // This constructor is called by ClassClass only ...
        explicit Traits(VM& vm);

    private:
        // Instance traits are presented in Class traits only.
        SPtr<InstanceTraits::Traits> ITraits;
    }; // class Traits

#if defined(SF_AS3_AOTC2)
    // Ahead Of Time Traits.
    class AOTraits : public Traits
    {
    public:
        AOTraits(VM& vm, const ClassInfo& ci) : Traits(vm, ci) {}

    public:
        InstanceTraits::AOTraits& GetInstanceTraits() const;
        Instances::fl::GlobalObject& GetGO() const;
    };
#endif

} // namespace ClassTraits

///////////////////////////////////////////////////////////////////////////
inline
const ClassTraits::Traits& Class::GetClassTraits() const
{
    return static_cast<const ClassTraits::Traits&>(GetTraits());
}

inline
ClassTraits::Traits& Class::GetClassTraits()
{
    return static_cast<ClassTraits::Traits&>(GetTraits());
}

inline
const InstanceTraits::Traits& Class::GetInstanceTraits() const
{
    return GetClassTraits().GetInstanceTraits();
}

inline
InstanceTraits::Traits& Class::GetInstanceTraits()
{
    return GetClassTraits().GetInstanceTraits();
}

///////////////////////////////////////////////////////////////////////////
inline
VTable& Object::GetVT()
{
    return GetTraits().GetVT();
}

inline
const VTable& Object::GetVT() const
{
    return GetTraits().GetVT();
}

inline
BuiltinTraitsType Object::GetTraitsType() const
{
    return GetTraits().GetTraitsType();
}

inline
bool Object::IsDynamic() const
{
    return GetTraits().IsDynamic();
}

inline
const Class& Object::GetClass() const
{
    return GetTraits().GetConstructor();
}

inline
Class& Object::GetClass()
{
    return GetTraits().GetConstructor();
}

inline 
VM& Object::GetVM() const
{
    return GetTraits().GetVM();
}

inline 
VM& Object::GetVMSafe() const
{
    return GetTraitsSafe().GetVM();
}

inline 
ASString Object::GetName() const
{
    return GetTraits().GetName();
}

inline 
CheckResult Object::GetSlotValueUnsafe(SlotIndex ind, Value& value)
{
    return GetTraits().GetSlotValueUnsafe(value, AbsoluteIndex(ind, GetTraits()), this);
}

inline 
CheckResult Object::GetSlotValueUnsafe(GlobalSlotIndex ind, Value& value)
{
    return GetTraits().GetSlotValueUnsafe(value, AbsoluteIndex(ind), this);
}

inline 
const ScopeStackType& Object::GetStoredScopeStack() const 
{
    return GetTraits().GetStoredScopeStack();
}

///////////////////////////////////////////////////////////////////////////
inline
AbsoluteIndex::AbsoluteIndex(RelativeIndex ind, const Traits& t)
: Index(static_cast<SPInt>(ind.Get()) + t.GetFirstOwnSlotIndex().Get())
{
}

inline
AbsoluteIndex::AbsoluteIndex(SlotIndex ind, const Traits& t)
: Index(static_cast<SPInt>(ind.Get()) - 1 + t.GetFirstOwnSlotIndex().Get())
{
}

inline
AbsoluteIndex::AbsoluteIndex(GlobalSlotIndex ind)
: Index(static_cast<SPInt>(ind.Get()) - 1)
{
}

///////////////////////////////////////////////////////////////////////////
inline
RelativeIndex::RelativeIndex(AbsoluteIndex ind, const Traits& t)
: Index(ind.Get() - t.GetFirstOwnSlotIndex().Get())
{
}

///////////////////////////////////////////////////////////////////////////
inline
SlotIndex::SlotIndex(GlobalSlotIndex ind, const Traits& t)
: Index(ind.Get() - t.GetFirstOwnSlotIndex().Get())
{
    SF_ASSERT(ind.Get() > t.GetFirstOwnSlotIndex().Get());
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Traits_H

