/**************************************************************************

Filename    :   AS3_Abc.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Abc_H
#define INC_AS3_Abc_H

#include "AS3_Abc_ConstPool.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_AmpInterface.h"

///////////////////////////////////////////////////////////////////////////////
// Byte code won't work for the time being.
#define SF_AS3_USE_WORDCODE

// #define SF_AS3_NO_PARAMNAMES

// Not finished yet.
// #define ENABLE_STRICT_SETSLOT

///////////////////////////////////////////////////////////////////////////////
// Forward declaration.
namespace Scaleform { namespace GFx { namespace AS3 
{ 
    class VMAbcFile;
    class Tracer;
    class CallFrame;
}}}

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
// TOpCode can be either byte or word code.
#ifdef SF_AS3_USE_WORDCODE
    typedef ArrayLH_POD<UPInt, StatMV_VM_AbcFile_Mem>  TOpCode;
#else
    typedef ArrayLH_POD<UInt8, StatMV_VM_AbcFile_Mem>  TOpCode;
#endif

typedef const TOpCode::ValueType*   TOpCodeVTP;

///////////////////////////////////////////////////////////////////////////
///
template <typename T>
inline
int Read8(const T* data, TCodeOffset& cp)
{
    return static_cast<int>(data[cp++]);
}
template <typename T>
inline
int Read16(const T* data, TCodeOffset& cp)
{
    return static_cast<int>(data[cp++]);
}
template <typename T>
inline
int ReadS24(const T* data, TCodeOffset& cp)
{
    return static_cast<int>(data[cp++]);
}
template <typename T>
inline
int ReadU30(const T* data, TCodeOffset& cp)
{
    return static_cast<int>(data[cp++]);
}
template <typename T>
inline
UInt32 ReadU32(const T* data, TCodeOffset& cp)
{
    return ReadU30(data, cp);
}
template <typename T>
inline
SInt32 ReadS32(const T* data, TCodeOffset& cp)
{
    return static_cast<SInt32>(ReadU30(data, cp));
}
template <typename T>
inline
UPInt ReadUPInt(const T* data, TCodeOffset& cp)
{
    return static_cast<UPInt>(data[cp++]);
}
template <typename T>
inline
Double ReadDouble(const T* data, TCodeOffset& cp)
{
    SF_ASSERT(false);
    return 0;
}
template <typename T>
inline
String ReadString(const T* data, TCodeOffset& cp, UPInt size)
{
    SF_ASSERT(false);
    return String();
}
template <typename T>
inline
StringDataPtr ReadStringPtr(const T* data, TCodeOffset& cp, UPInt size)
{
    SF_ASSERT(false);
    return StringDataPtr();
}

///////////////////////////////////////////////////////////////////////////
///
template <typename T>
inline
int Read8(const T*& cp)
{
    return static_cast<int>(*cp++);
}
template <typename T>
inline
int Read16(const T*& cp)
{
    return static_cast<int>(*cp++);
}
template <typename T>
inline
int ReadS24(const T*& cp)
{
    return static_cast<int>(*cp++);
}
template <typename T>
inline
int ReadU30(const T*& cp)
{
    return static_cast<int>(*cp++);
}
template <typename T>
inline
UInt32 ReadU32(const T*& cp)
{
    return ReadU30(cp);
}
template <typename T>
inline
SInt32 ReadS32(const T*& cp)
{
    return static_cast<SInt32>(ReadU30(cp));
}
template <typename T>
inline
UPInt ReadUPInt(const T*& cp)
{
    return *cp++;
}
template <typename T>
inline
Double ReadDouble(const T*& /*cp*/)
{
    SF_ASSERT(false);
    return 0;
}
template <typename T>
inline
String ReadString(const T*& /*cp*/, UPInt /*size*/)
{
    SF_ASSERT(false);
    return String();
}
template <typename T>
inline
StringDataPtr ReadStringPtr(const T*& /*cp*/, UPInt /*size*/)
{
    SF_ASSERT(false);
    return StringDataPtr();
}

///
template <typename T>
inline
int JumpOffsetCorrection()
{
    return 1;
}

///
template <>
inline
int Read8(const UInt8* data, TCodeOffset& cp)
{
    return data[cp++];
}
template <>
int Read16(const UInt8* data, TCodeOffset& cp);
template <>
int ReadS24(const UInt8* data, TCodeOffset& cp);
template <>
int ReadU30(const UInt8* data, TCodeOffset& cp);
template <>
Double ReadDouble(const UInt8* data, TCodeOffset& cp);
template <>
String ReadString(const UInt8* data, TCodeOffset& cp, UPInt size);
template <>
StringDataPtr ReadStringPtr(const UInt8* data, TCodeOffset& cp, UPInt size);

///
template <>
inline
int Read8(const UInt8*& cp)
{
    return *cp++;
}
template <>
int Read16(const UInt8*& cp);
template <>
int ReadS24(const UInt8*& cp);
template <>
int ReadU30(const UInt8*& cp);
template <>
inline
UPInt ReadUPInt(const UInt8*& /*cp*/)
{
    SF_ASSERT(false);
    return 0;
}
template <>
Double ReadDouble(const UInt8*& cp);
template <>
String ReadString(const UInt8*& cp, UPInt size);
template <>
StringDataPtr ReadStringPtr(const UInt8*& cp, UPInt size);

template <>
inline
int JumpOffsetCorrection<UInt8>()
{
    return 3;
}

///////////////////////////////////////////////////////////////////////////
// Taken from AVM2 overview and reordered. CONSTANT_TypeName is not documented.
enum ValueKind
{
    CONSTANT_Undefined = AbcKind_unused, // -
    CONSTANT_Utf8 = AbcKind_Utf8, // string
    // Gap ...
    CONSTANT_Int = AbcKind_Int, // integer
    CONSTANT_UInt = AbcKind_UInt, // uinteger
    CONSTANT_Double = AbcKind_Double, // double
    CONSTANT_Namespace = AbcKind_Namespace, // namespace
    CONSTANT_True = AbcKind_True, // -
    CONSTANT_False = AbcKind_False, // -
    CONSTANT_Null = AbcKind_Null, // -
    CONSTANT_PackageNamespace = AbcKind_PackageNamespace, // namespace
    CONSTANT_PackageInternalNs = AbcKind_PackageInternalNs, // Namespace
    CONSTANT_ProtectedNamespace = AbcKind_ProtectedNamespace, // Namespace
    CONSTANT_ExplicitNamespace = AbcKind_ExplicitNamespace, // Namespace
    CONSTANT_StaticProtectedNs = AbcKind_StaticProtectedNs, // Namespace
    CONSTANT_PrivateNs = AbcKind_PrivateNs // namespace
};

// Used for debugging.
bool IsValidValueKind(UInt8 vk);

///////////////////////////////////////////////////////////////////////////
//
// ValueDetail is used to represent default values for MethodTable optional
// parameters and TraitTable slots. It combines a Kind identifying the
// constant pool table to use (CONSTANT_Int, Double, etc) with an
// index within that constant pool.
// Matches AVM2 4.5.1 "option_detail".

class ValueDetail
{
    friend bool Read(const UInt8*& ptr, ValueDetail& obj);

public:
    ValueDetail()
        : Kind(CONSTANT_Undefined)
        , ValueIndex(-1)
    {
    }
    ValueDetail(SInd ind, ValueKind kind)
        : Kind(static_cast<UInt8>(kind))
        , ValueIndex(ind)
    {
    }
    
public:
    // Return the index within a constant pool.
    SInd GetIndex() const
    {
        return ValueIndex;
    }
    // Return ValueKind identifying the type of constant pool to index.
    ValueKind GetKind() const
    {
        return static_cast<ValueKind>(Kind);
    }
    
private:
    UInt8   Kind;
    SInd    ValueIndex;
};

// ConstValue is similar to the class ValueDetail, except it has a different layout.
struct ConstValue
{
    // Return the index within a constant pool.
    UInt16 GetIndex() const
    {
        return ValueIndex;
    }
    // Return ValueKind identifying the type of constant pool to index.
    ValueKind GetKind() const
    {
        return static_cast<ValueKind>(Kind);
    }

    // Kind uses only five bits.
    UInt16  Kind;
    UInt16  ValueIndex;
};

///////////////////////////////////////////////////////////////////////////

// MethodInfo describes a single Methods signature including its name,
// parameter types, signatures and default values. Note that Methods
// body bytecode data is stored separately.
// Matches AVM2 4.5 "method_info".

class MethodBodyInfo;
class MethodBodyTable;

class MethodInfo : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    // Required for Array<MethodInfo>
    MethodInfo()
    : RetTypeInd(-1)
    , MethodBodyInfoInd(-1)
    {
    }

public:
    enum Flag
    {
        fNEED_ARGUMENTS = 0x01,
        fNEED_ACTIVATION = 0x02,
        fNEED_REST = 0x04,
        fHAS_OPTIONAL = 0x08,
        fSET_DXNS = 0x40,
        fHAS_PARAM_NAMES = 0x80
    };

    //
    static bool NeedArguments(UInt8 flags)
    {
        return flags & fNEED_ARGUMENTS;
    }
    //
    static bool NeedActivation(UInt8 flags)
    {
        return (flags & fNEED_ACTIVATION) != 0;
    }
    //
    static bool NeedRest(UInt8 flags)
    {
        return (flags & fNEED_REST) != 0;
    }
    //
    static bool HasOptionalParams(UInt8 flags)
    {
        return (flags & fHAS_OPTIONAL) != 0;
    }
    //
    static bool NeedToSetDXNS(UInt8 flags)
    {
        return (flags & fSET_DXNS) != 0;
    }
    //
    static bool HasParamNames(UInt8 flags)
    {
        return (flags & fHAS_PARAM_NAMES) != 0;
    }

public:
    const Multiname& GetReturnType(const ConstPool& cp) const
    {
        // (AVM2 4.5)
        return cp.GetMultiname(RetTypeInd);
    }

    //
    bool NeedArguments() const
    {
        return NeedArguments(Flags);
    }
    bool NeedActivation() const
    {
        return NeedActivation(Flags);
    }
    bool NeedRest() const
    {
        return NeedRest(Flags);
    }
    bool HasOptionalParams() const
    {
        return HasOptionalParams(Flags);
    }
    bool NeedToSetDXNS() const
    {
        return NeedToSetDXNS(Flags);
    }
    bool HasParamNames() const
    {
        return HasParamNames(Flags);
    }

#ifndef SF_AS3_NO_PARAMNAMES
    StringDataPtr GetParamName(const ConstPool& cp, UPInt i) const
    {
        return cp.GetStrings()[ParamNames[i]].ToStringDataPtr();
    }
#endif

    UPInt GetOptionalParamCount() const
    {
        return OptionalParams.GetSize();
    }
    const ValueDetail& GetOptionalParam(UPInt ind) const
    {
        SF_ASSERT(ind < OptionalParams.GetSize());

        return OptionalParams[ind];
    }

    const Multiname& GetParamType(const ConstPool& cp, UPInt ind) const
    {
        SF_ASSERT(ind < ParamTypes.GetSize());
        SF_ASSERT(ParamTypes[ind] < cp.GetMultinameCount());

        return cp.GetMultiname(ParamTypes[ind]);
    }
    UPInt GetParamCount() const
    {
        return ParamTypes.GetSize();
    }
    MbiInd GetMethodBodyInfoInd() const
    {
        return MbiInd(MethodBodyInfoInd);
    }
    const MethodBodyInfo& GetMethodBodyInfo(const MethodBodyTable& mbt) const;

private:
    void SetMethodBodyInfoInd(MbiInd ind)
    {
        SF_ASSERT(MethodBodyInfoInd == -1);
        MethodBodyInfoInd = ind.Get();
    }

// private:
public:
    UInt8                   Flags;
    SInd                    RetTypeInd;
    //     StringDataPtr           Name; // Not used.
    SInd                    MethodBodyInfoInd;
    ArrayLH_POD<unsigned, StatMV_VM_AbcFile_Mem>   ParamTypes;
    ArrayLH<ValueDetail, StatMV_VM_AbcFile_Mem>    OptionalParams;
#ifndef SF_AS3_NO_PARAMNAMES
    ArrayLH_POD<SInd, StatMV_VM_AbcFile_Mem>       ParamNames;
#endif
};

///////////////////////////////////////////////////////////////////////////

// MethodTable is an array of indexable MethodInfos stored in an
// ABC file Methods table (File::Methods).

class MethodTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~MethodTable()
    {
        for (UPInt i = 0; i < Info.GetSize(); ++i)
            delete Info[i];
    }

public:
    UPInt GetSize() const
    {
        return Info.GetSize();
    }
    const MethodInfo& Get(Abc::MiInd ind) const
    {
        SF_ASSERT(ind.IsValid());
        SF_ASSERT(static_cast<UPInt>(ind.Get()) < Info.GetSize());
        SF_ASSERT(Info[ind.Get()]);
        return *Info[ind.Get()];
    }

public:
    void Clear()
    {
        Info.Clear();
    }

private:
    ArrayLH_POD<MethodInfo*, StatMV_VM_AbcFile_Mem> Info;
};

///////////////////////////////////////////////////////////////////////////

// MetadataTable allows embedding arbitrary key/value pairs into the ABC file.
// MetadataTable tables are indexed from TraitTable.
// Matches AVM2 4.6 "metadata_info".

class MetadataInfo : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    // Item is interpreted as key/value pair of indices into the
    // String table of the constant pool..
    // Item is POD.    
    class Item
    {
        friend class Reader;

    public:
        Item()
        : KeyInd(0)
        , ValueInd(0)
        {
        }

    public:
        SInd GetKeyInd() const { return KeyInd; }
        SInd GetValueInd() const { return ValueInd; }

    private:
        SInd    KeyInd;
        SInd    ValueInd;
    };

public:
    typedef ArrayLH<Item, StatMV_VM_AbcFile_Mem> ItemsT;

    StringDataPtr GetName() const
    {
        return Name;
    }

    const ItemsT& GetItems() const
    {
        return Items;
    }

private:
    StringDataPtr   Name;
    ItemsT          Items;
};

///////////////////////////////////////////////////////////////////////////

// MetadataTable is an array of indexable MetadataInfo entries
// representing all Metadata in a file (File::Metadata).

class MetadataTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~MetadataTable()
    {
        for (UPInt i = 0; i < Info.GetSize(); ++i)
            delete Info[i];
    }

public:
    UPInt GetSize() const
    {
        return Info.GetSize();
    }
    const MetadataInfo& Get(UPInt num) const
    {
        SF_ASSERT(num < Info.GetSize());
        SF_ASSERT(Info[num]);
        return *Info[num];
    }

public:
    void Clear()
    {
        Info.Clear();
    }

private:
    ArrayLH_POD<MetadataInfo*, StatMV_VM_AbcFile_Mem> Info;
};


///////////////////////////////////////////////////////////////////////////
class File;

///////////////////////////////////////////////////////////////////////////

// TraitInfo describes a single property of object or class, such as a
// data Slot, Method, Getter/Setter, etc. A Traits includes a name, type
// and associated data. TraitTable are aggregated in the TraitsTable of the
// constant pool and referenced by Traits index-arrays within MethodBodyTable,
// Instance, ClassTable and ScriptTable.
// Matches AVM2 4.8 "traits_info".

class TraitInfo : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    // Required for Array<TraitInfo>
    TraitInfo()
    : name_ind(-1)
    , SlotId(-1)
    , Ind(-1)
    {
    }

public:
    enum Type
    {
        tSlot = 0,
        tMethod = 1,
        tGetter = 2,
        tSetter = 3,
        tClass = 4,
        tFunction = 5,
        tConst = 6
    };
    enum Kind
    {
        kFinal = 0x1,
        kOverride = 0x2,
        kMetadata = 0x4
    };

    class MetaInfo : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
    {
        friend class Reader;

    public:
        MetaInfo()
        {
        }

    public:
        UPInt GetSize() const
        {
            return info.GetSize();
        }
        inline const MetadataInfo& Get(const MetadataTable& md, AbsoluteIndex ind) const;

    private:
        ArrayLH_POD<SInd, StatMV_VM_AbcFile_Mem> info;
    };

public:
    Type GetType() const
    {
        return static_cast<Type>(kind & 0xF);
    }
    bool IsData() const
    {
        Type type = GetType();
        
        return (type == tSlot || 
                type == tConst ||
                type == tClass ||
                type == tFunction
                );
    }
    bool IsCode() const
    {
        return !IsData();
    }
    bool IsSlotOrConst() const
    {
        Type type = GetType();

        return (type == tSlot || type == tConst);
    }
    bool HasMethodInd() const
    {
        Type type = GetType();
        
        return (type == tMethod ||
                type == tGetter ||
                type == tSetter
                );
    }
    bool HasSlotID() const
    {
        return IsData();
    }

    SInt32 GetSlotID() const
    {
        SF_ASSERT(HasSlotID());
        return SlotId;
    }

    MiInd GetMethodInd() const
    {
        SF_ASSERT(HasMethodInd());
        return MiInd(Ind);
    }

    const Multiname& GetTypeName(const File& f) const;
    
    // DefaultValue is specific to Slot and Const.
    bool HasDefaultValue() const
    {
        SF_ASSERT(IsSlotOrConst());
        return default_value.GetIndex() > 0;
    }
    const ValueDetail& GetDefaultValue() const
    {
        SF_ASSERT(IsSlotOrConst());
        return default_value;
    }

    //
    bool HasMetaData() const
    {
        return HasMetaData(GetKind());
    }
    const MetaInfo& GetMetaInfo() const
    {
        SF_ASSERT(HasMetaData());
        return meta_info;
    }

public:
    UInt8 GetKind() const
    {
        return kind >> 4;
    }

    const Multiname& GetName(const ConstPool& cp) const;
    
private:
    static bool IsFinal(UInt8 kind)
    {
        return kind & kFinal;
    }
    static bool IsOverride(UInt8 kind)
    {
        return (kind & kOverride) != 0;
    }
    static bool HasMetaData(UInt8 kind)
    {
        return (kind & kMetadata) != 0;
    }

private:
    UInt8       kind;
    SInd        name_ind;
    SInd        SlotId;
    SInd        Ind;
    ValueDetail default_value; // For a Slot.
    MetaInfo    meta_info;
};

///////////////////////////////////////////////////////////////////////////

// TraitTable is an array of indexable TraitInfo entries representing
// aggregated traits of all objects in the ABC File (File::Metadata).

class TraitTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~TraitTable()
    {
        for (UPInt i = 0; i < TraitInfos.GetSize(); ++i)
            delete TraitInfos[i];
    }

public:
    UPInt GetSize() const
    {
        return TraitInfos.GetSize();
    }
    const TraitInfo& Get(UPInt num) const
    {
        SF_ASSERT(num < TraitInfos.GetSize());
        SF_ASSERT(TraitInfos[num]);
        return *TraitInfos[num];
    }

public:
    void Clear()
    {
        TraitInfos.Clear();
    }

private:
    ArrayLH_POD<TraitInfo*, StatMV_VM_AbcFile_Mem> TraitInfos;
};

///////////////////////////////////////////////////////////////////////////

// HasTraits base class injects a set of traits into a derived class that needs them.
// Traits are represented as indices into a shared File::Traits table.

class HasTraits : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    typedef ArrayLH_POD<SInd, StatMV_VM_AbcFile_Mem> ObjectTraits;
    
public:
    // Required for Array<HasTraits>
    HasTraits()
    {
    }
    HasTraits(const HasTraits& other)
    : obj_traits(other.obj_traits)
    {
    }

public:
    UPInt GetTraitsCount() const
    {
        return obj_traits.GetSize();
    }
    const TraitInfo& GetTraitInfo(const TraitTable& t, AbsoluteIndex ind) const
    {
        return t.Get(obj_traits[ind.Get()]);
    }
    int GetObjectTraitsInd(AbsoluteIndex ind) const
    {
        return obj_traits[ind.Get()];
    }

private:
    // We use this one only with the Stream operator ...
    ObjectTraits& GetObjectTraits()
    {
        return obj_traits;
    }

private:
    ObjectTraits obj_traits;
};

///////////////////////////////////////////////////////////////////////////

// HasCode is a mix-in base class that combines a Method index with
// the ability to carry Traits. The associated Methods is typically
// used as intitializer (object instance constructor, static class initializer,
// Scripts initializer).

class HasCode : public HasTraits
{
    friend class Reader;

public:
    HasCode()
    : method_info_ind(-1)
    {
    }
    
public:
    MiInd GetMethodInfoInd() const
    {
        return MiInd(method_info_ind);
    }
    const MethodInfo& GetMethodInfo(const MethodTable& m) const
    {
        return m.Get(MiInd(method_info_ind));
    }

private:
    SInd    method_info_ind;
};

///////////////////////////////////////////////////////////////////////////

// The Instance class defines the characteristics of a run-time object
// (a class instance) within the AVM2. It includes object traits, but not
// the static traits of the class, which are stored separately in ClassTable.
// The corresponding "class_info" entry is  used in order to fully define an
// ActionScript 3.0 ClassTable.
// Matches AVM2 4.7 "instance_info".

// In Instance, Method defined in HasCode is used as an 
// instance initializer/constructor.

class Instance : public HasCode
{
    friend class Reader;

public:
    Instance()
    : name_ind(-1)
    , super_name_ind(-1)
    , protected_namespace_ind(-1)
    {
    }

public:
    enum Flag
    {
        fSealed = 0x01,
        fFinal = 0x02,
        fInterface = 0x04,
        fProtectedNs = 0x08
    };

    // the referenced names specify the interfaces implemented by this class.
    class Interfaces : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
    {
        friend class Reader;

    public:
        UPInt GetSize() const
        {
            return info.GetSize();
        }
        const Multiname& GetName(const ConstPool& cp, AbsoluteIndex ind) const;

    private:
        // indices into the multiname array of the constant pool ...
        // None of the indices may be zero.
        ArrayLH_POD<int, StatMV_VM_AbcFile_Mem> info;
    };

public:
    bool HasSuperClass() const
    {
        return super_name_ind != 0;
    }

    SInd GetNameInd() const
    {
        return name_ind;
    }
    const Multiname& GetName(const ConstPool& cp) const;
    const Multiname& GetSuperClassName(const ConstPool& cp) const;

    const NamespaceInfo& GetProtectedNamespace(const ConstPool& cp) const;
    const Interfaces& GetInterfaces() const
    {
        return implemented_interfaces;
    }

public:
    bool IsSealed() const
    {
        return flags & fSealed;
    }
    bool IsFinal() const
    {
        return (flags & fFinal) != 0;
    }
    bool IsInterface() const
    {
        return (flags & fInterface) != 0;
    }
    bool HasProtectedNamespace() const
    {
        return (flags & fProtectedNs) != 0;
    }
    
private:
    UInt8       flags;
    SInd        name_ind;
    SInd        super_name_ind;
    SInd        protected_namespace_ind;
    Interfaces  implemented_interfaces;
};

///////////////////////////////////////////////////////////////////////////

// StaticInfo defines static characteristics of an ActionScript 3.0 class.
// Matches AVM2 4.9 "class_info".

class StaticInfo : public HasCode
{
    friend class Reader;
};

///////////////////////////////////////////////////////////////////////////

// ClassInfo aggregates Instance and static Class data associated with
// an ActionScript class into one object.

class ClassInfo : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;
    
public:
    bool HasSuperClass() const
    {
        return GetInstanceInfo().HasSuperClass();
    }
    SInd GetNameInd() const
    {
        return GetInstanceInfo().GetNameInd();
    }
    const Multiname& GetName(const ConstPool& cp) const
    {
        return GetInstanceInfo().GetName(cp);
    }
    const Multiname& GetSuperClassName(const ConstPool& cp) const
    {
        return GetInstanceInfo().GetSuperClassName(cp);
    }
    
public:
    const Instance& GetInstanceInfo() const
    {
        return inst_info;
    }
    const StaticInfo& GetStaticInfo() const
    {
        return stat_info;
    }

private:
    Instance& GetInstanceInfo()
    {
        return inst_info;
    }
    StaticInfo& GetStaticInfo()
    {
        return stat_info;
    }

private:
    Instance    inst_info;
    StaticInfo  stat_info;
};

///////////////////////////////////////////////////////////////////////////

// ClassTable is an array of ClassInfo objects describing all classes
// defined in an ABC file.

class ClassTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~ClassTable()
    {
        for (UPInt i = 0; i < Info.GetSize(); ++i)
            delete Info[i];
    }

public:
    UPInt GetSize() const
    {
        return Info.GetSize();
    }
    const ClassInfo& Get(UPInt num) const
    {
        SF_ASSERT(num < Info.GetSize());
        SF_ASSERT(Info[num]);
        return *Info[num];
    }

public:
    void Clear()
    {
        Info.Clear();
    }

private:
    ArrayLH_POD<ClassInfo*, StatMV_VM_AbcFile_Mem> Info;
};

///////////////////////////////////////////////////////////////////////////
// ??? Similar to StaticInfo ???

// ScriptInfo is used to define characteristics of an ActionScript 3.0 Scripts.
// Includes traits and an initializer method.
// Matches AVM 4.10 "script_info".

class ScriptInfo : public HasCode
{
    friend class Reader;
};

///////////////////////////////////////////////////////////////////////////
class ScriptTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~ScriptTable()
    {
        for (UPInt i = 0; i < Info.GetSize(); ++i)
            delete Info[i];
    }

public:
    UPInt GetSize() const
    {
        return Info.GetSize();
    }
    const ScriptInfo& Get(UPInt num) const
    {
        SF_ASSERT(num < Info.GetSize());
        SF_ASSERT(Info[num]);
        return *Info[num];
    }

public:
    void Clear()
    {
        Info.Clear();
    }

private:
    ArrayLH_POD<ScriptInfo*, StatMV_VM_AbcFile_Mem> Info;
};

///////////////////////////////////////////////////////////////////////////

// Code is an array of executable opcodes,
// typically associated with a MethodBody.

class Code : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    Code()
    {
    }
    virtual ~Code()
    {
    }

public:
    enum OpCode
    {
        op_nop = 0x02,
        op_throw = 0x03,
        op_getsuper = 0x04,
        op_setsuper = 0x05,
        op_dxns = 0x06,
        op_dxnslate = 0x07,
        op_kill = 0x08,
        op_label = 0x09,
        op_inclocal_ti = 0x0A, // My own ...
        op_declocal_ti = 0x0B, // My own ...
        op_ifnlt = 0x0C,
        op_ifnle = 0x0D,
        op_ifngt = 0x0E,
        op_ifnge = 0x0F,
        op_jump = 0x10,
        op_iftrue = 0x11,
        op_iffalse = 0x12,
        op_ifeq = 0x13,
        op_ifne = 0x14,
        op_iflt = 0x15,
        op_ifle = 0x16,
        op_ifgt = 0x17,
        op_ifge = 0x18,
        op_ifstricteq = 0x19,
        op_ifstrictne = 0x1A,
        op_lookupswitch = 0x1B,
        op_pushwith = 0x1C,
        op_popscope = 0x1D,
        op_nextname = 0x1E,
        op_hasnext = 0x1F,
        op_pushnull = 0x20,
        op_pushundefined = 0x21,
        op_not_tb = 0x22, // My own ...
        op_nextvalue = 0x23,
        op_pushbyte = 0x24,
        op_pushshort = 0x25,
        op_pushtrue = 0x26,
        op_pushfalse = 0x27,
        op_pushnan = 0x28,
        op_pop = 0x29,
        op_dup = 0x2A,
        op_swap = 0x2B,
        op_pushstring = 0x2C,
        op_pushint = 0x2D,
        op_pushuint = 0x2E,
        op_pushdouble = 0x2F,
        op_pushscope = 0x30,
        op_pushnamespace = 0x31,
        op_hasnext2 = 0x32,
        op_iftrue_tb = 0x33, // My own ...
        op_iffalse_tb = 0x34, // My own ...
        op_increment_tu = 0x35, // My own ... // op_li8 = 0x35,
        op_decrement_tu = 0x36, // My own ... // op_li16 = 0x36,
        op_inclocal_tu = 0x37, // My own ... // op_li32 = 0x37,
        op_declocal_tu = 0x38, // My own ... // op_lf32 = 0x38,
        op_lf64 = 0x39,
        op_si8 = 0x3A,
        op_si16 = 0x3B,
        op_si32 = 0x3C,
        op_sf32 = 0x3D,
        op_sf64 = 0x3E,
        op_negate_ti = 0x3F, // My own ...
        op_newfunction = 0x40,
        op_call = 0x41,
        op_construct = 0x42,
        op_callmethod = 0x43,
        op_callstatic = 0x44,
        op_callsuper = 0x45,
        op_callproperty = 0x46,
        op_returnvoid = 0x47,
        op_returnvalue = 0x48,
        op_constructsuper = 0x49,
        op_constructprop = 0x4A,
        //op_callsuperid = 0x4B,
        op_callproplex = 0x4C,
        //op_callinterface = 0x4D,
        op_callsupervoid = 0x4E,
        op_callpropvoid = 0x4F,
        op_sxi1 = 0x50,
        op_sxi8 = 0x51,
        op_sxi16 = 0x52,
        op_applytype = 0x53,
        op_negate_td = 0x54, // My own ...
        op_newobject = 0x55,
        op_newarray = 0x56,
        op_newactivation = 0x57,
        op_newclass = 0x58,
        op_getdescendants = 0x59,
        op_newcatch = 0x5A,
        //op_findpropglobalstrict = 0x5B, // NEW internal only
        //op_findpropglobal = 0x5C, // NEW internal only
        op_findpropstrict = 0x5D,
        op_findproperty = 0x5E,
        //op_finddef = 0x5F,
        op_getlex = 0x60,
        op_setproperty = 0x61,
        op_getlocal = 0x62,
        op_setlocal = 0x63,
        op_getglobalscope = 0x64,
        op_getscopeobject = 0x65,
        op_getproperty = 0x66,
        op_getouterscope = 0x67, // My own
        op_initproperty = 0x68,
        op_dup_nrc = 0x69, // My own ... (Not ref-counted version of dup)
        op_deleteproperty = 0x6A,
        op_pop_nrc = 0x6B, // My own ... (Not ref-counted version of pop)
        op_getslot = 0x6C,
        op_setslot = 0x6D,
        op_getglobalslot = 0x6E,
        op_setglobalslot = 0x6F,
        op_convert_s = 0x70,
        op_esc_xelem = 0x71,
        op_esc_xattr = 0x72,
        op_convert_i = 0x73,
        op_convert_u = 0x74,
        op_convert_d = 0x75,
        op_convert_b = 0x76,
        op_convert_o = 0x77,
        op_checkfilter = 0x78,

        op_add_ti = 0x79, // My own ...
        op_subtract_ti = 0x7A, // My own ...
        op_multiply_ti = 0x7B, // My own ...
        op_add_td = 0x7C, // My own ...
        op_subtract_td = 0x7D, // My own ...
        op_multiply_td = 0x7E, // My own ...
        op_divide_td = 0x7F, // My own ...

        op_coerce = 0x80,
        //op_coerce_b = 0x81,
        op_coerce_a = 0x82,
        //op_coerce_i = 0x83,
        //op_coerce_d = 0x84,
        op_coerce_s = 0x85,
        op_astype = 0x86,
        op_astypelate = 0x87,
        //op_coerce_u = 0x88,
        //op_coerce_o = 0x89,
        
        op_ifnlt_ti = 0x8A, // My own ...
        op_ifnle_ti = 0x8B, // My own ...
        op_ifngt_ti = 0x8C, // My own ...
        op_ifnge_ti = 0x8D, // My own ...
        op_ifeq_ti = 0x8E, // My own ...
        op_ifge_ti = 0x8F, // My own ...
        
        op_negate = 0x90,
        op_increment = 0x91,
        op_inclocal = 0x92,
        op_decrement = 0x93,
        op_declocal = 0x94,
        op_typeof = 0x95,
        op_not = 0x96,
        op_bitnot = 0x97,
        op_increment_ti = 0x98, // My own ...
        op_decrement_ti = 0x99, // My own ...
        //op_concat = 0x9A,
        op_add_d = 0x9B, // Not in the manual.
        
        op_ifgt_ti = 0x9C, // My own ...
        op_ifle_ti = 0x9D, // My own ...
        op_iflt_ti = 0x9E, // My own ...
        op_ifne_ti = 0x9F, // My own ...
        
        op_add = 0xA0,
        op_subtract = 0xA1,
        op_multiply = 0xA2,
        op_divide = 0xA3,
        op_modulo = 0xA4,
        op_lshift = 0xA5,
        op_rshift = 0xA6,
        op_urshift = 0xA7,
        op_bitand = 0xA8,
        op_bitor = 0xA9,
        op_bitxor = 0xAA,
        op_equals = 0xAB,
        op_strictequals = 0xAC,
        op_lessthan = 0xAD,
        op_lessequals = 0xAE,
        op_greaterthan = 0xAF,
        op_greaterequals = 0xB0,
        op_instanceof = 0xB1,
        op_istype = 0xB2,
        op_istypelate = 0xB3,
        op_in = 0xB4,

        op_getabsobject = 0xB5, // My own ...
        op_getabsslot = 0xB6, // My own ...
        op_setabsslot = 0xB7, // My own ...
        op_initabsslot = 0xB8, // My own ...
        op_callsupermethod = 0xB9, // My own ...
        op_callgetter = 0xBA, // My own ...
        op_callsupergetter = 0xBB, // My own ...
        op_ifgt_td = 0xBC, // My own ...
        op_ifle_td = 0xBD, // My own ...
        op_iflt_td = 0xBE, // My own ...
        op_ifne_td = 0xBF, // My own ...

        op_increment_i = 0xC0,
        op_decrement_i = 0xC1,
        op_inclocal_i = 0xC2,
        op_declocal_i = 0xC3,
        op_negate_i = 0xC4,
        op_add_i = 0xC5,
        op_subtract_i = 0xC6,
        op_multiply_i = 0xC7,

        op_ifnlt_td = 0xC8, // My own ...
        op_ifnle_td = 0xC9, // My own ...
        op_ifngt_td = 0xCA, // My own ...
        op_ifnge_td = 0xCB, // My own ...
        op_ifeq_td = 0xCC, // My own ...
        op_ifge_td = 0xCD, // My own ...
        op_callobject = 0xCE, // My own ...

        op_getlocal0 = 0xD0,
        op_getlocal1 = 0xD1,
        op_getlocal2 = 0xD2,
        op_getlocal3 = 0xD3,
        op_setlocal0 = 0xD4,
        op_setlocal1 = 0xD5,
        op_setlocal2 = 0xD6,
        op_setlocal3 = 0xD7,

#ifdef ENABLE_STRICT_SETSLOT
        op_setslot_str = 0xD8, // My own ...
        op_setslot_num = 0xD9, // My own ...
        op_setslot_uint = 0xDA, // My own ...
        op_setslot_sint = 0xDB, // My own ...
        op_setslot_bool = 0xDC, // My own ...
        op_setslot_value = 0xDD, // My own ...
        op_setslot_obj_as = 0xDE, // My own ...
        op_setslot_obj_cpp = 0xDF, // My own ...
#endif

        //op_abs_jump = 0xEE,
        op_debug = 0xEF,
        op_debugline = 0xF0,
        op_debugfile = 0xF1,
        op_0xF2 = 0xF2,
        //op_timestamp = 0xF3
    };

public:
    struct OpCodeInfo
    {
        signed char     operandCount:4;   // -1 for "invalid"
        unsigned char   canThrow:1;       // boolean
        unsigned char   pop:2;
        unsigned char   push:2;
        unsigned char   pop_args:1;       // boolean
        unsigned char   pop_mn:1;       // boolean
        SF_DEBUG_CODE(const char* name;)
    };

public:
    StringView GetCode() const
    {
        return code;
    }

    static const OpCodeInfo& GetOpCodeInfo(const OpCode opcode)
    {
        return opcode_info[opcode];
    }

    void Clear() { ; }

private:
    static const OpCodeInfo opcode_info[0x100];

    StringView  code;
};

///////////////////////////////////////////////////////////////////////////
#ifdef GFX_AS3_VERBOSE
#ifdef GFX_AS3_TRACE
inline
const char* AsString(const Code::OpCode opcode)
{
    return Code::GetOpCodeInfo(opcode).name;
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////

// MethodBodyInfo holds byte-code instructions associated with a particular
// Methods or function body, together with the associated traits, exception
// records and stack/local register constraints.
// Matches AVM2 4.11 "method_body_info".

class MethodBodyInfo : public HasTraits
{
    friend class Reader;
    friend class AS3::Tracer; // Because of ClearCode().

public:
    // Required for Array<MethodBodyInfo>
    MethodBodyInfo();
    ~MethodBodyInfo();

public:

    class ExceptionInfo
    {
        friend class Reader;

    public:
        ExceptionInfo();
        ExceptionInfo(UInd _from, UInd _to, UInd _target, UInd _exc_type_ind, UInt32 _var_name_ind);
        
    public:
        UInd GetFrom() const
        {
            return from;
        }
        UInd GetTo() const
        {
            return to;
        }
        UInd GetTargetPos() const
        {
            return target;
        }
        UInd GetExceptionTypeInd() const
        {
            return exc_type_ind;
        }
        UInt32 GetVariableNameInd() const
        {
            return var_name_ind;
        }
        
    private:
        UInd    from;
        UInd    to;
        UInd    target;
        UInd    exc_type_ind;
        UInt32  var_name_ind;
    };

    class Exception : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
    {
        friend class Reader;
        friend class AS3::Tracer;

    public:
        UPInt GetSize() const
        {
            return info.GetSize();
        }
        const ExceptionInfo& Get(UPInt num) const
        {
            SF_ASSERT(num < info.GetSize());
            return info[num];
        }
        // cp - code pointer.
        // handler_num - first handler to start from.
        bool FindExceptionInfo(Abc::TCodeOffset offset, UPInt& handler_num) const;

    private:
        ArrayLH_POD<ExceptionInfo, StatMV_VM_AbcFile_Mem> info;
    };

public:
    MiInd GetMethodInfoInd() const
    {
        return MiInd(method_info_ind);
    }
    const MethodInfo& GetMethodInfo(const MethodTable& m) const
    {
        return m.Get(MiInd(method_info_ind));
    }
    const Code& GetCode() const
    {
        return code;
    }

    const Exception& GetException() const
    {
        return exception;
    }

public:
    SInt32 GetMaxStackSize() const
    {
        return max_stack;
    }
    SInt32 GetMaxLocalRegisterIndex() const
    {
        return local_reg_count;
    }
    SInt32 GetMinScopeDepth() const
    {
        return init_scope_depth;
    }
    SInt32 GetMaxScopeDepth() const
    {
        return max_scope_depth;
    }

private:
    void ClearCode() { code.Clear(); }

private:
    SInd      method_info_ind;
    SInt32    max_stack;
    SInt32    local_reg_count;
    SInt32    init_scope_depth;
    SInt32    max_scope_depth;
    Code      code;
    Exception exception;
};

///////////////////////////////////////////////////////////////////////////
class MethodBodyTable : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
    friend class Reader;

public:
    ~MethodBodyTable();

public:
    UPInt GetSize() const
    {
        return Info.GetSize();
    }
    const MethodBodyInfo& Get(MbiInd ind) const
    {
        SF_ASSERT(ind.IsValid());
        SF_ASSERT(static_cast<UPInt>(ind.Get()) < Info.GetSize());
        SF_ASSERT(Info[ind.Get()]);
        return *Info[ind.Get()];
    }

public:
    void Clear()
    {
        Info.Clear();
    }

private:
    ArrayLH_POD<MethodBodyInfo*, StatMV_VM_AbcFile_Mem> Info;
};

inline
const MethodBodyInfo& MethodInfo::GetMethodBodyInfo(const MethodBodyTable& mbt) const
{
    SF_ASSERT(MethodBodyInfoInd >= 0);
    return mbt.Get(MbiInd(MethodBodyInfoInd));
}

///////////////////////////////////////////////////////////////////////////

// File represents all of data fully loaded from a .ABC file. 
// ABC file data includes the constant pool and Methods, class, Scripts,
// Metadata and other related tables.


class File : public NewOverrideBase<StatMV_VM_AbcFile_Mem>, public RefCountImpl
{
    friend class Reader;

public:
    File()
        : DataSize(0)
#ifdef SF_AMP_SERVER
        , FileHandle(0), SwfFileOffset(0)
#endif
    {
    }
    virtual ~File();

public:
    UInt32 GetDataSize() const
    {
        return DataSize;
    }
    void SetDataSize(UInt32 value)
    {
        DataSize = value;
    }

#ifdef SF_AMP_SERVER
    UInt32 GetFileHandle() const
    {
        return FileHandle;
    }
    void SetFileHandle(UInt32 swdHandle)
    {
        FileHandle = swdHandle;
    }
    UInt32 GetSwfFileOffset() const
    {
        return SwfFileOffset;
    }
    void SetSwfFileOffset(UInt32 swfFileOffset)
    {
        SwfFileOffset = swfFileOffset;
    }
#endif

public:
    UInt16 GetMinorVersion() const
    {
        return MinorVersion;
    }
    UInt16 GetMajorVersion() const
    {
        return MajorVersion;
    }
    const ConstPool& GetConstPool() const
    {
        return Const_Pool;
    }
    const MethodTable& GetMethods() const
    {
        return Methods;
    }
    const MetadataTable& GetMetadata() const
    {
        return Metadata;
    }
    const TraitTable& GetTraits() const
    {
        return Traits;
    }
    const ClassTable& GetClasses() const
    {
        return AS3_Classes;
    }
    const ScriptTable& GetScripts() const
    {
        return Scripts;
    }
    const MethodBodyTable& GetMethodBodyTable() const
    {
        return MethodBodies;
    }

public:
    void Clear();

public:
    const String& GetSource() const
    {
        return Source;
    }
    const String& GetName() const
    {
        // !!! TODO: That should be just a file name.
        return GetSource();
    }
    void SetSource(const String& src)
    {
        Source = src;
    }

private:
    File& operator =(const File&);
    File(const File&);

private:
    UInt32          DataSize; // temporarily for parsing ...
    String          Source; // File name of the source ABC file.

    SF_AMP_CODE(UInt32 FileHandle;)
    SF_AMP_CODE(int SwfFileOffset;)

    UInt16          MinorVersion;
    UInt16          MajorVersion;
    ConstPool       Const_Pool;
    MethodTable     Methods;
    MetadataTable   Metadata;
    TraitTable      Traits;
    ClassTable      AS3_Classes;
    ScriptTable     Scripts;
    MethodBodyTable MethodBodies;
};

///////////////////////////////////////////////////////////////////////////
inline
const Multiname& TraitInfo::GetTypeName(const File& f) const
{
    if (IsSlotOrConst())
        return f.GetConstPool().GetMultiname(Ind);

#if 0
    // For debugging.
    Type t = GetType();
    SF_UNUSED1(t);
    SF_ASSERT(t == tClass);
#else
    SF_ASSERT(GetType() == tClass);
#endif

    return f.GetClasses().Get(Ind).GetInstanceInfo().GetName(f.GetConstPool());
}

///////////////////////////////////////////////////////////////////////////
inline
bool Read8(const UInt8*& ptr, UInt8& v)
{
    v = static_cast<UInt8>(Abc::Read8(ptr));
    return true;
}

inline
bool Read8(const UInt8*& ptr, SInt8& v)
{
    v = static_cast<SInt8>(Abc::Read8(ptr));
    return true;
}

inline
bool Read(const UInt8*& ptr, UInt32& v)
{
    v = ReadU30(ptr);
    return true;
}

inline
bool Read(const UInt8*& ptr, SInt32& v)
{
    v = ReadU30(ptr);
    return true;
}

inline
bool Read(const UInt8*& ptr, UInt16& v)
{
    UInt32 i = ReadU30(ptr);
    SF_ASSERT(i <= 0xFFFF);
    v = static_cast<UInt16>(i);
    return true;
}

inline
bool Read(const UInt8*& ptr, SInt16& v)
{
    UInt32 i = ReadU30(ptr);
    SF_ASSERT(i <= 0x7FFF);
    v = static_cast<SInt16>(i);
    return true;
}

inline
bool Read(const UInt8*& ptr, UInt8& v)
{
    UInt32 i = ReadU30(ptr);
    SF_ASSERT(i <= 0xFF);
    v = (UInt8)i;
    return true;
}

inline
bool Read(const UInt8*& ptr, SInt8& v)
{
    UInt32 i = ReadU30(ptr);
    SF_ASSERT(i <= 0x7F);
    v = (SInt8)i;
    return true;
}

bool Read(const UInt8*& ptr, MethodInfo& obj);
bool Read(const UInt8*& ptr, ValueDetail& obj);

///////////////////////////////////////////////////////////////////////////

// Reader is a memory-buffer input stream for File. Reader is initialized
// with a memory buffer, while its Read(File&) Methods is then used to
// initialize the file further.

class Reader : public NewOverrideBase<StatMV_VM_AbcFile_Mem>
{
public:
    Reader(const UInt8* data, UPInt size)
    : Size(size)
    , CP(data)
    {
    }

public:
    void SetCP(UInt8* cp)
    {
        CP = cp;
    }

public:
    // Readers.

    //
    bool Read8(UInt8& v) { return Abc::Read8(CP, v); }
    bool Read8(SInt8& v) { return Abc::Read8(CP, v); }
    //
    bool Read16(UInt16& v)
    {
        v = static_cast<UInt16>(Abc::Read16(CP));
        return true;
    }
    bool Read16(SInt16& v)
    {
        v = static_cast<SInt16>(Abc::Read16(CP));
        return true;
    }

    // All Read() methods read 30-bit variable length integer
    // and convert it to a type of argument.

    //
    bool Read(UInt32& v) { return Abc::Read(CP, v); }
    bool Read(SInt32& v) { return Abc::Read(CP, v); }
    //
    bool Read(UInt16& v) { return Abc::Read(CP, v); }
    bool Read(SInt16& v) { return Abc::Read(CP, v); }
    bool Read(UInt8& v) { return Abc::Read(CP, v); }
    bool Read(SInt8& v) { return Abc::Read(CP, v); }

    bool Read(const ConstPool& cp, StringDataPtr& obj, const StringDataPtr& zero_val);
    bool Read(const ConstPool& cp, NamespaceInfo& obj);
    bool Read(NamespaceSetInfo& obj);
    bool Read(const ConstPool& cp, Multiname& obj);
    bool Read(ConstPool& obj);
    bool Read(ValueDetail& obj) { return Abc::Read(CP, obj); }
    bool Read(MethodInfo& obj) { return Abc::Read(CP, obj); }
    bool Read(MethodTable& obj);
    bool Read(const ConstPool& cp, MetadataInfo& obj);
    bool Read(MetadataInfo::Item& obj);
    bool Read(const ConstPool& cp, MetadataTable& obj);
    bool Read(TraitInfo& obj);
    bool Read(TraitInfo::MetaInfo& obj);
    bool Read(TraitTable& obj);
    bool ReadTraits(TraitTable& obj, HasTraits::ObjectTraits& obj_traits);
    bool Read(TraitTable& t, HasTraits& obj);
    bool Read(TraitTable& t, HasCode& obj);
    bool Read(TraitTable& t, Instance& obj);
    bool Read(Instance::Interfaces& obj);
    bool Read(TraitTable& t, StaticInfo& obj);
    bool Read(TraitTable& t, ClassTable& obj);
    bool Read(TraitTable& t, ScriptInfo& obj);
    bool Read(TraitTable& t, ScriptTable& obj);
    bool Read(Code& obj);
    bool Read(TraitTable& t, MethodBodyInfo& obj);
    bool Read(MethodBodyInfo::ExceptionInfo& obj);
    bool Read(MethodBodyInfo::Exception& obj);
    bool Read(TraitTable& t, MethodTable& mt, MethodBodyTable& obj);
    bool Read(File& obj);

private:
    Reader& operator =(const Reader&);

private:
    const UPInt     Size;
    const UInt8*    CP;
};

///////////////////////////////////////////////////////////////////////////
// Inline methods ...

///////////////////////////////////////////////////////////////////////////
inline
const MetadataInfo& TraitInfo::MetaInfo::Get(const MetadataTable& md, AbsoluteIndex ind) const
{
    return md.Get(info[ind.Get()]);
}

///////////////////////////////////////////////////////////////////////////
inline
const Multiname& Instance::Interfaces::GetName(const ConstPool& cp, AbsoluteIndex ind) const
{
    SF_ASSERT(ind.IsValid() && ind.Get() < info.GetSize());
    SF_ASSERT(info[ind.Get()] != 0);
    return cp.GetMultiname(info[ind.Get()]);
}

///////////////////////////////////////////////////////////////////////////
inline
const NamespaceInfo& Instance::GetProtectedNamespace(const ConstPool& cp) const
{
    return cp.GetNamespace(protected_namespace_ind);
}

inline
const Multiname& Instance::GetName(const ConstPool& cp) const
{
    // ??? May name_ind be zero ???
    const Multiname& result = cp.GetMultiname(name_ind);
    SF_ASSERT(result.GetKind() == MN_QName);

    return result;
}

inline
const Multiname& Instance::GetSuperClassName(const ConstPool& cp) const
{
    return cp.GetMultiname(super_name_ind);
}

///////////////////////////////////////////////////////////////////////////
inline
const Multiname& TraitInfo::GetName(const ConstPool& cp) const
{
    SF_ASSERT(name_ind != 0);
    const Multiname& result = cp.GetMultiname(name_ind);
    SF_ASSERT(result.GetKind() == MN_QName);

    return result;
}


}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

#endif // INC_AS3_Abc_H
