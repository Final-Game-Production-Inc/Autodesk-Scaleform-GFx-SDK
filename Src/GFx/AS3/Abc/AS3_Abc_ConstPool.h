/**************************************************************************

Filename    :   AS3_Abc_ConstPool.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Abc_ConstPool_H
#define INC_AS3_Abc_ConstPool_H


#include "AS3_Abc_Type.h"
#include "../AS3_Index.h"

///////////////////////////////////////////////////////////////////////////////
// Forward declaration.
namespace Scaleform { namespace GFx { namespace AS3 
{
    class VM;
}}} // namespace Scaleform { namespace GFx { namespace AS3 {

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class ConstPool;

///////////////////////////////////////////////////////////////////////////
enum AbcKind {
    AbcKind_unused              = 0x00,
    AbcKind_Utf8                = 0x01,
    AbcKind_Int                 = 0x03,
    AbcKind_UInt                = 0x04,
    AbcKind_PrivateNs           = 0x05, // non-shared namespace
    AbcKind_Double              = 0x06,
    AbcKind_Qname               = 0x07, // o.ns::name, ct ns, ct name
    AbcKind_Namespace           = 0x08,
    AbcKind_Multiname           = 0x09, // o.name, ct nsset, ct name
    AbcKind_False               = 0x0A,
    AbcKind_True                = 0x0B,
    AbcKind_Null                = 0x0C,
    AbcKind_QnameA              = 0x0D, // o.@ns::name, ct ns, ct attr-name
    AbcKind_MultinameA          = 0x0E, // o.@name, ct attr-name
    AbcKind_RTQname             = 0x0F, // o.ns::name, rt ns, ct name
    AbcKind_RTQnameA            = 0x10, // o.@ns::name, rt ns, ct attr-name
    AbcKind_RTQnameL            = 0x11, // o.ns::[name], rt ns, rt name
    AbcKind_RTQnameLA           = 0x12, // o.@ns::[name], rt ns, rt attr-name
    AbcKind_NamespaceSet        = 0x15,
    AbcKind_PackageNamespace    = 0x16,
    AbcKind_PackageInternalNs   = 0x17,
    AbcKind_ProtectedNamespace  = 0x18,
    AbcKind_ExplicitNamespace   = 0x19,
    AbcKind_StaticProtectedNs   = 0x1A,
    AbcKind_MultinameL          = 0x1B,
    AbcKind_MultinameLA         = 0x1C,
    AbcKind_TypeName            = 0x1D
};

///////////////////////////////////////////////////////////////////////////

// NamespaceInfo describes a namespace entry in a constant pool, it combines a NamespaceKind 
// with URIString. Non-public namespaces receive a non-empty compiler-generated NameURI.
// Namespaces are compared based on {NamespaceKind, NameURI}, with exception of Private namespaces
// which must be unique across ABCs.

class NamespaceInfo
{
    friend class Reader;
    friend bool operator ==(const NamespaceInfo& l, const NamespaceInfo& r);
public:
    NamespaceInfo()    
    : Kind(NS_Undefined)
    {
    }
    NamespaceInfo(NamespaceKind k, const StringDataPtr& uri)    
    : Kind(k)
    , NameURI(uri)
    {
    }

public:

    bool HasKind() const
    {
        return Kind != NS_Undefined;
    }
    
    NamespaceKind GetKind() const
    {
        SF_ASSERT(IsValidKind());
        return Kind;
    }
    // User-defined namespaces have names. All other don't.
    bool IsUserDefined() const
    {
        return (GetKind() == NS_Public || GetKind() == NS_Explicit);
    }
    // This method is required for multiname resolution ...
    bool IsPublic() const
    {
        return IsUserDefined();
    }
    const StringDataPtr& GetNameURI() const
    {
        return NameURI;
    }
    
private:
    // For debugging ...
    bool IsValidKind() const
    {
        return Kind != NS_Undefined;
    }
    
private:    
    NamespaceKind   Kind;
    StringDataPtr   NameURI;
};

///////////////////////////////////////////////////////////////////////////
inline
bool operator ==(const NamespaceInfo& l, const NamespaceInfo& r)
{
    // MA TBD: This won't work for Privates
    return (l.Kind == r.Kind && l.NameURI == r.NameURI);
}

///////////////////////////////////////////////////////////////////////////
class NamespaceSetInfo
{
    friend class Reader;

public:
    const UInt8* GetData() const { return Data; }
    UPInt GetSize() const;

private:
    const UInt8*    Data;
};

///////////////////////////////////////////////////////////////////////////

// MultinameKindBits describe representation of MultinameKind.
// MultinameKind is broken down into Namespace, Name, and Attribute parts.
//  - Namespace can be qualified (Q), runtime-qualified (RTQ),
//    or a namespace-set (Multi).
//  - Name can be explicit (Name) or runtime/"late" (NameL).
//  - (A) suffix indicates that a name refers to an XML attribute,
//   as in "node.@attrib" access.
// TBD: MN_Typename is a separate value; need research what it's used for.
// Perhaps Typename describes parameterized type such as Vector.<int>?

enum MultinameKindBits
{
    MN_NS_Mask      = 0x03, // Namespace type bits.
    MN_NS_Qualified = 0x00,
    MN_NS_Runtime   = 0x01,
    MN_NS_Multi     = 0x02,
    MN_NameLate     = 0x04, // Late name binding flag.
    MN_Attr         = 0x08  // Attribute present in name flag.
};

// MultinameKind describes different types of Multiname.
enum MultinameKind
{
    MN_QName        = MN_NS_Qualified,
    MN_QNameA       = MN_NS_Qualified | MN_Attr,
    MN_RTQName      = MN_NS_Runtime,
    MN_RTQNameA     = MN_NS_Runtime | MN_Attr,
    MN_RTQNameL     = MN_NS_Runtime | MN_NameLate,
    MN_RTQNameLA    = MN_NS_Runtime | MN_NameLate | MN_Attr,
    MN_Multiname    = MN_NS_Multi,
    MN_MultinameA   = MN_NS_Multi | MN_Attr,
    MN_MultinameL   = MN_NS_Multi | MN_NameLate,
    MN_MultinameLA  = MN_NS_Multi | MN_NameLate | MN_Attr,
    MN_Typename     = 0x10,
    MN_Invalid      = 0x20 // Set on loading error.
};


// Multiname describes a name used for identifier lookup in the VM,
// combining Namespace/NamespaceSet and identifier name information.
//
// Information in Multiname may be not complete if it a Runtime Multiname
// (MN_NS_Runtime).

// This class is aggregate.
class Multiname
{
    friend class Reader;

public:
    // From HasMultinameKind.

    MultinameKind GetKind() const
    {
        SF_ASSERT(isValidKind());
        return Kind;
    }

    bool isValidKind() const
    {
        return Kind != MN_Invalid;
    }

    MultinameKindBits getNamespaceBits() const
    {
        return (MultinameKindBits)(Kind & MN_NS_Mask);
    }

    bool IsNameLate() const
    {
        return (GetKind() & MN_NameLate) != 0;
    }

    // Attributes are used with XML.
    bool IsAttr() const
    {
        return (GetKind() & MN_Attr) != 0;
    }

    bool IsQName() const
    {
        const MultinameKindBits bits = getNamespaceBits();
        // MN_NS_Runtime is also a qualified name.
        // Qualified name is an antonym to MN_NS_Multi.
        return bits == MN_NS_Qualified || bits == MN_NS_Runtime;
    }

    bool IsRunTime() const
    {
        return getNamespaceBits() == MN_NS_Runtime;
    }
    
    bool IsCompileTime() const
    {
        return !IsRunTime() || IsNameLate();
    }

    bool IsMultiname() const
    {
        return getNamespaceBits() == MN_NS_Multi;
    }

    bool IsTypename() const
    {
        return GetKind() == MN_Typename;
    }

public:
    // Name is not always available.
    SInd GetNameInd() const
    {
        // SF_ASSERT(NameIndex >= 0);
        return NameIndex;
    }
    StringDataPtr GetName(const ConstPool& cp) const;

    //
    const NamespaceInfo& GetNamespace(const ConstPool& cp) const;
    SInd GetNamespaceInd() const
    {
        return Ind;
    }

    //
    const NamespaceSetInfo& GetNamespaceSetInfo(const ConstPool& cp) const;
    SInd GetNamespaceSetInd() const
    {
        return Ind;
    }
    
    //
    const Multiname& GetNextMultiname(const ConstPool& cp) const;
    SInd GetNextMultinameIndex() const
    {
        return NextIndex;
    }

    //
    bool IsAnyType() const
    {
        return GetKind() == MN_QName && NameIndex == 0 && Ind == 0;
    }

    bool IsAnyNamespace() const
    {
        return Ind == 0;
    }

    static const Multiname& GetAnyType()
    {
        return AnyType;
    }

public:
    bool IsBoolean(const ConstPool& cp) const;
    bool IsInt(const ConstPool& cp) const;
    bool IsUInt(const ConstPool& cp) const;
    bool IsNumber(const ConstPool& cp) const;
    bool IsString(const ConstPool& cp) const;

    bool IsFixedNumType(const ConstPool& cp) const;
    bool IsNumericType(const ConstPool& cp) const;
    bool IsPrimitiveType(const ConstPool& cp) const;

private:
    // Accessor for Read()
    void setKind(MultinameKind kind)
    {
        Kind = kind;
    }

    static const Multiname  AnyType;

public:
    // Be careful when reordering members. This class is aggregate.
    // Do not forget to adjust code in Reader::Read(ConstPool& obj).

    // Namespace or NamespaceSet index.
    SInd            Ind;
    SInd            NextIndex;
    SInd            NameIndex;
    MultinameKind   Kind;
};

///////////////////////////////////////////////////////////////////////////
class StringView
{
public:
    StringView()
        : Data(&Empty)
    {
    }
    StringView(const UInt8* d)
        : Data(d)
    {
        SF_ASSERT(Data);
    }

public:
    /* Not safe.
    UInt32 GetSize() const 
    {
        UInt8* d = Data;
        return static_cast<UInt32>(ReadU30(d));
    }

    const char* ToCStr() const 
    {
        UInt8* d = Data;
        ReadU30(d);
        return d;
    }
    */

    StringDataPtr ToStringDataPtr() const;
    operator StringDataPtr () const { return ToStringDataPtr(); }

private:
    static const UInt8  Empty;
    const UInt8*        Data;
};

///////////////////////////////////////////////////////////////////////////
class ConstPool : public NewOverrideBase<StatMV_VM_AbcConstPool_Mem> // Not Moveable !!!
{
    friend class Reader;

public:
    typedef ArrayLH_POD<SInt32, StatMV_VM_AbcConstPool_Mem> IntType;
    typedef ArrayLH_POD<UInt32, StatMV_VM_AbcConstPool_Mem> UIntType;
    typedef ArrayLH<Multiname, StatMV_VM_AbcConstPool_Mem> MultinamesType;
    typedef ArrayLH<StringView, StatMV_VM_AbcConstPool_Mem> StringsType;
    // NamespaceInfo is not much bigger than original data.
    // Let's store parsed version of NamespaceInfo.
    typedef ArrayLH<NamespaceInfo, StatMV_VM_AbcConstPool_Mem> NamespaceType;
    typedef ArrayLH_POD<NamespaceSetInfo, StatMV_VM_AbcConstPool_Mem> NamespaceSetType;
    
public:
    ConstPool();
    
public:
    ConstPool& GetSelf()
    {
        return *this;
    }
    const ConstPool& GetSelf() const
    {
        return *this;
    }
    
    SInt32 GetInt(UPInt ind) const
    {
        // The "0" entry of the integer array is not present in the abcFile; 
        // it represents the zero value for the purposes of providing values for 
        // optional parameters and field initialization.
        SF_ASSERT(ind < ConstInt.GetSize());

        // ConstUInt[0] == 0
        return ConstInt[ind];
    }
    UPInt GetIntCount() const
    {
        return ConstInt.GetSize();
    }
    
    UInt32 GetUInt(UPInt ind) const
    {
        // The "0" entry of the uinteger array is not present in the abcFile; 
        // it represents the zero value for the purposes of providing values 
        // for optional parameters and field initialization.
        SF_ASSERT(ind < ConstUInt.GetSize());

        // ConstUInt[0] == 0
        return ConstUInt[ind];
    }
    UPInt GetUIntCount() const
    {
        return ConstUInt.GetSize();
    }
    
    double GetDouble(UPInt ind) const;

    UPInt GetDoubleCount() const
    {
        return DoubleCount;
    }
    
    StringDataPtr GetString(AbsoluteIndex ind) const
    {
        // Entry "0" of the string array is not present in the abcFile; it
        // represents the empty string in most contexts but is also used to 
        // represent the "any" name in others (known as "*" in ActionScript).
        SF_ASSERT(ind.IsValid() && ind.Get() < ConstStr.GetSize());

        return ConstStr[ind.Get()];
    }
    UPInt GetStringCount() const
    {
        return ConstStr.GetSize();
    }
    const StringsType& GetStrings() const
    {
        return ConstStr;
    }
    
    const NamespaceInfo& GetNamespace(UPInt ind) const
    {
        if (ind == 0)
            return GetAnyNamespace();
        
        // Entry "0" of the namespace array is not present in the abcFile; 
        // it represents the "any" namespace (known as "*" in ActionScript).
        SF_ASSERT(ind > 0 && ind < ConstNamespace.GetSize());
        return ConstNamespace[ind];
    }
    UPInt GetNamespaceCount() const
    {
        return ConstNamespace.GetSize();
    }
    
    const NamespaceSetInfo& GetNamespaceSet(UPInt ind) const
    {
        // The "0" entry of the ns_set array is not
        // present in the abcFile.
        SF_ASSERT(ind > 0 && ind < const_ns_set.GetSize());
        return const_ns_set[ind];
    }
    UPInt GetNamespaceSetCount() const
    {
        return const_ns_set.GetSize();
    }
    
    const MultinamesType& GetMultinames() const 
    {
        return const_multiname;
    }
    const Multiname& GetMultiname(UPInt ind) const
    {
        return const_multiname[ind];
    }
    UPInt GetMultinameCount() const
    {
        return const_multiname.GetSize();
    }
    
    const NamespaceInfo& GetAnyNamespace() const 
    {
        return any_namespace;
    }

public:
    void Clear()
    {
        ConstInt.Clear();
        ConstUInt.Clear();
        ConstStr.Clear();
        ConstNamespace.Clear();
        const_ns_set.Clear();
        const_multiname.Clear();
    }

private:
    ConstPool& operator =(const ConstPool&);

private:
    UInt32              DoubleCount;
    const UInt8*        Doubles;
    IntType             ConstInt;
    UIntType            ConstUInt;
    StringsType         ConstStr;
    NamespaceType       ConstNamespace;
    NamespaceSetType    const_ns_set;
    MultinamesType      const_multiname;
    const NamespaceInfo any_namespace;
};

///////////////////////////////////////////////////////////////////////////
// Inline methods ...

inline
StringDataPtr Multiname::GetName(const ConstPool& cp) const
{
    return cp.GetString(AbsoluteIndex(NameIndex));
}

inline
const NamespaceInfo& Multiname::GetNamespace(const ConstPool& cp) const
{
    SF_ASSERT(IsQName());
    
    //if (!ns_ind)
    //  return any_name;

    return cp.GetNamespace(Ind);
}

inline
const NamespaceSetInfo& Multiname::GetNamespaceSetInfo(const ConstPool& cp) const
{
    SF_ASSERT(IsMultiname());
    SF_ASSERT(Ind > 0);
    return cp.GetNamespaceSet(Ind);
}

inline
const Multiname& Multiname::GetNextMultiname(const ConstPool& cp) const
{
    SF_ASSERT(IsQName());
    SF_ASSERT(GetNextMultinameIndex() >= 0);

    return cp.GetMultiname(GetNextMultinameIndex());
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

#endif // INC_AS3_Abc_ConstPool_H
