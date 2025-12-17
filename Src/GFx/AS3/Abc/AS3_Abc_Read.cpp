/**************************************************************************

Filename    :   AS3_Abc_Read.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Abc.h"

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

///////////////////////////////////////////////////////////////////////////
bool Reader::Read(File& obj)
{
    bool result = true;

    obj.Clear();

    result = result &&
        Read16(obj.MinorVersion) && 
        Read16(obj.MajorVersion);

    SF_ASSERT(obj.MajorVersion == 46);
    SF_ASSERT(obj.MinorVersion == 16);
    result = result && (obj.MajorVersion == 46) && (obj.MinorVersion == 16);

    result = result &&
        Read(obj.Const_Pool) &&
        Read(obj.Methods) && 
        Read(obj.Const_Pool, obj.Metadata) && 
        Read(obj.Traits, obj.AS3_Classes) && 
        Read(obj.Traits, obj.Scripts) && 
        Read(obj.Traits, obj.Methods, obj.MethodBodies);

    return result;
}

bool Reader::Read(const ConstPool& cp, StringDataPtr& obj, const StringDataPtr& zero_val)
{
    SInt32 ind = 0;
    bool result = Read(ind);

    if (result)
    {
        if (ind == 0)
            obj = zero_val;
        else
            obj = cp.GetString(AbsoluteIndex(ind));
    }

    return result;
}

bool Reader::Read(const ConstPool& cp, NamespaceInfo& obj)
{
    bool result = true;

    UInt8 kind = 0;

    result = result &&
        Read8(kind) &&
        Read(cp, obj.NameURI, "");

    AbcKind abcKind = static_cast<AbcKind>(kind);

    switch(abcKind)
    {
    case AbcKind_Namespace:
    case AbcKind_PackageNamespace:
        obj.Kind = NS_Public;
        break;
    case AbcKind_PackageInternalNs:
        obj.Kind = NS_PackageInternal;
        break;
    case AbcKind_ProtectedNamespace:
        obj.Kind = NS_Protected;
        break;
    case AbcKind_ExplicitNamespace:
        obj.Kind = NS_Explicit;
        break;
    case AbcKind_StaticProtectedNs:
        obj.Kind = NS_StaticProtected;
        break;
    case AbcKind_PrivateNs:
        // Each class has its OWN private namespace ...
        obj.Kind = NS_Private;
        break;
    default:
        // obj.kind should be NS_Undefined.        
        SF_ASSERT(false);
        result = false;
        break;
    }

    return result;
}

bool Reader::Read(NamespaceSetInfo& obj)
{
    bool    result = true;

    UInd    entry = 0;
    UInt32  size = 0;

    obj.Data = CP;
    result = result && Read(size);

    for (UInt32 i = 0; result && (i < size); ++i)
    {
        result = result && Read(entry);

        SF_ASSERT(result && (entry > 0));
        if (!result || (entry <= 0))
            return false;
    }

    return result;
}

bool Reader::Read(const ConstPool& cp, Multiname& obj)
{
    bool result = true;
    UInt8 kind = 0;

    result = result && Read8(kind);

    // Decode AbcKind into flag-encoded multiname.
    AbcKind       abcKind = static_cast<AbcKind>(kind);
    MultinameKind mnKind;
    switch(abcKind)
    {
    case AbcKind_Qname:       mnKind = MN_QName; break;
    case AbcKind_QnameA:      mnKind = MN_QNameA; break;
    case AbcKind_RTQname:     mnKind = MN_RTQName; break;
    case AbcKind_RTQnameA:    mnKind = MN_RTQNameA; break;
    case AbcKind_RTQnameL:    mnKind = MN_RTQNameL; break;
    case AbcKind_RTQnameLA:   mnKind = MN_RTQNameLA; break;
    case AbcKind_Multiname:   mnKind = MN_Multiname; break;
    case AbcKind_MultinameL:  mnKind = MN_MultinameL; break;
    case AbcKind_MultinameA:  mnKind = MN_MultinameA; break;
    case AbcKind_MultinameLA: mnKind = MN_MultinameLA; break;
    case AbcKind_TypeName:    mnKind = MN_Typename; break;
    default:
        SF_ASSERT(false);
        mnKind = MN_Invalid;
        result = false;
        break;
    };
    obj.setKind(mnKind);

    switch (mnKind)
    {
    case MN_QName:
    case MN_QNameA:
        // Name and namespace are known at compile time ...
        result = result && 
            Read(obj.Ind) &&
            Read(obj.NameIndex);
        break;
    case MN_RTQName:
    case MN_RTQNameA:
        // Name is known at compile time ...
        // Namespace is resolved at runtime ...
        result = result && 
            Read(obj.NameIndex);
        break;
    case MN_RTQNameL:
    case MN_RTQNameLA:
        // Name and namespace are resolved at runtime ...
        break;
    case MN_Multiname:
    case MN_MultinameA:
        // Name and namespace set are known at compile time ...
        result = result && 
            Read(obj.NameIndex) &&
            Read(obj.Ind);
        break;
    case MN_MultinameL:
    case MN_MultinameLA:
        // Namespace set is known at compile time ...
        // Name is resolved at runtime ...
        result = result && 
            Read(obj.Ind);
        break;
    case MN_Typename:
        {
            SInt32 ind = 0;

            // Index of a type in the constant pool;
            result = result && Read(ind);
            if (!result)
                break;

            obj = cp.GetMultiname(ind);

            result = result && Read(ind);
            SF_ASSERT(ind == 1);
            result = result && (ind == 1);
            if (!result)
                break;

            result = result && Read(obj.NextIndex);
        }
        break;
    default:
        break;
    }

    if (!result)
    {
        obj.setKind(MN_Invalid);        
    }

    return result;
}

bool Reader::Read(ConstPool& obj)
{
    bool result = true;
    UInt32 number = 0;

    // S32
    result = result && Read(number);
    if (result)
    {
        obj.ConstInt.Reserve(number);
        obj.ConstInt.PushBack(0);

        for (UInt32 i = 1; i < number; ++i)
            obj.ConstInt.PushBack(ReadS32(CP));
    }

    // UInt32
    number = 0;
    result = result && Read(number);
    if (result)
    {
        obj.ConstUInt.Reserve(number);
        obj.ConstUInt.PushBack(0);

        for (UInt32 i = 1; i < number; ++i)
            obj.ConstUInt.PushBack(ReadU32(CP));
    }

    // Double
    obj.DoubleCount = 0;
    obj.Doubles = NULL;
    result = result && Read(obj.DoubleCount);
    if (result)
    {
        obj.Doubles = CP;
        if (obj.DoubleCount)
            CP += 8 * (obj.DoubleCount - 1);
    }

    // String
    number = 0;
    result = result && Read(number);
    if (result)
    {
        obj.ConstStr.Reserve(number);
        obj.ConstStr.PushBack(StringView());

        for (UInt32 i = 0; number > 0 && i < number - 1; ++i)
        {
            obj.ConstStr.PushBack(StringView(CP));
            const int size = ReadU30(CP);
            CP += size;
        }
    }

    // NamespaceInfo
    number = 0;
    result = result && Read(number);
    if (result)
    {
        obj.ConstNamespace.Reserve(number);
        obj.ConstNamespace.PushBack(NamespaceInfo(NS_Public, ""));

        for (UInt32 i = 1; i < number; ++i)
        {
            NamespaceInfo info(NS_Public, "");

            result = result && Read(obj, info);
            obj.ConstNamespace.PushBack(info);
        }
    }

    // NamespaceSetInfo
    number = 0;
    result = result && Read(number);
    if (result)
    {
        obj.const_ns_set.Reserve(number);
        obj.const_ns_set.PushBack(NamespaceSetInfo());

        obj.const_ns_set.Back().Data = NULL;

        NamespaceSetInfo info;
        for (UInt32 i = 1; i < number; ++i)
        {
            result = result && Read(info);
            obj.const_ns_set.PushBack(info);
        }
    }

    // Abc::Multiname
    number = 0;
    result = result && Read(number);
    if (result)
    {
        // For some reason number + 1 saves memory.
        obj.const_multiname.Reserve(number + 1);
        obj.const_multiname.PushBack(Multiname::GetAnyType());

        for (UInt32 i = 1; i < number; ++i)
        {
            Multiname info = {-1, -1, 0, MN_QName};

            result = result && Read(obj, info);
            obj.const_multiname.PushBack(info);
        }
    }

    return result;
}

bool Read(const UInt8*& ptr, ValueDetail& obj)
{
    bool result = true;

    UInt8 kind = 0;
    result = result &&
        Read(ptr, obj.ValueIndex) &&
        Read8(ptr, kind);

    SF_ASSERT(obj.ValueIndex >= 0);
    result = result && (obj.ValueIndex >= 0);

    obj.Kind = static_cast<ValueKind>(kind);

    return result;
}

bool Read(const UInt8*& ptr, MethodInfo& obj)
{
    bool result = true;

    SInt32 param_count = 0;
    result = result &&
        Read(ptr, param_count) &&
        Read(ptr, obj.RetTypeInd);

    SF_ASSERT(obj.RetTypeInd >= 0);
    result = result && (obj.RetTypeInd >= 0);
    if (!result)
        return result;

    obj.ParamTypes.Reserve(param_count);
    for (int i = 0; i < param_count; ++i)
        obj.ParamTypes.PushBack(ReadU30(ptr));

    // We ignore name because it is not used anywhere and it is not set properly
    // by a compiler.
    SInt32 name_ind;
    result = result &&
        Read(ptr, name_ind) &&
        Read8(ptr, obj.Flags);

    if (!result)
        return result;

    if (obj.HasOptionalParams())
    {
        SInt32 count = 0;
        result = result && Read(ptr, count);

        obj.OptionalParams.Reserve(count);
        for (int i = 0; result && (i < count); ++i)
        {
            ValueDetail detail;

            result = result && Read(ptr, detail);
            if (!result)
                return result;

            obj.OptionalParams.PushBack(detail);
        }
    }

    if (obj.HasParamNames())
    {
#ifdef SF_AS3_NO_PARAMNAMES
        SInt32 ind = 0;
        for (int i = 0; i < param_count; ++i)
            Read(ptr, ind);
#else
        obj.ParamNames.Reserve(param_count);
        for (int i = 0; i < param_count; ++i)
        {
            SInd ind = 0;
            result = result && Read(ptr, ind);

            if (result)
                obj.ParamNames.PushBack(ind);
            else
                break;

        }
#endif
    }

    return result;
}

bool Reader::Read(MethodTable& obj)
{
    bool result = true;

    SInt32 count = 0;
    result = result && Read(count);

    obj.Info.Reserve(count);
    for (int i = 0; result && (i < count); ++i)
    {
        // obj.Info stores pointers.
        obj.Info.PushBack(SF_HEAP_AUTO_NEW(this) MethodInfo());

        result = result && Read(*obj.Info.Back());
        if (!result)
        {
            delete obj.Info.Back();
            obj.Info.PopBack();
            break;
        }
    }

    return result;
}

bool Reader::Read(const ConstPool& cp, MetadataInfo& obj)
{
    bool result = true;

    SInt32 count = 0;

    result = result &&
        // Name MUST have a value.
        Read(cp, obj.Name, StringDataPtr()) &&
        Read(count);

    if (!result)
        return result;

    obj.Items.Reserve(count);

    // Read keys.
    for (int i = 0; i < count; ++i)
    {
        MetadataInfo::Item& item = obj.Items.PushDefault();

        result = result && Read(item.KeyInd);
    }

    // Read values.
    for (int i = 0; i < count; ++i)
    {
        MetadataInfo::Item& item = obj.Items[i];

        result = result && Read(item.ValueInd);
    }

    return result;
}

bool Reader::Read(MetadataInfo::Item& obj)
{
    bool result = true;

    result = result &&
        Read(obj.KeyInd) &&
        Read(obj.ValueInd);

    return result;
}

bool Reader::Read(const ConstPool& cp, MetadataTable& obj)
{
    bool result = true;

    SInt32 count = 0;

    result = result && Read(count);
    if (!result)
        return result;

    obj.Info.Reserve(count);
    for (int i = 0; i < count; ++i)
    {
        // obj.Info stores pointers.
        obj.Info.PushBack(SF_HEAP_AUTO_NEW(this) MetadataInfo());

        result = result && Read(cp, *obj.Info.Back());
        if (!result)
        {
            delete obj.Info.Back();
            obj.Info.PopBack();
            break;
        }
    }

    return result;
}

bool Reader::Read(TraitInfo& obj)
{
    bool result = true;

    result = result &&
        Read(obj.name_ind) &&
        Read8(obj.kind);

    SF_ASSERT(obj.name_ind >= 0);
    result = result && (obj.name_ind >= 0);
    if (!result)
        return result;

    // Lower four bits ...
    TraitInfo::Type type = obj.GetType();

    switch (type)
    {
    case TraitInfo::tSlot:
    case TraitInfo::tConst:
        // Read Slot.
        {
            SInd vindex = 0;

            result = result &&
                Read(obj.SlotId) &&
                Read(obj.Ind) &&
                Read(vindex);

            if (!result)
                return result;

            // This check is needed.
            if (vindex)
            {
                UInt8 kind = 0;
                result = result && Read8(kind);

                if (!IsValidValueKind(kind))
                {
                    // Incorrect ValueKind ...
                    SF_ASSERT(false);
                    result = false;
                }

                obj.default_value = ValueDetail(vindex, static_cast<ValueKind>(kind));
            }
        }

        break;
    case TraitInfo::tMethod:
    case TraitInfo::tGetter:
    case TraitInfo::tSetter:
        // Read MethodTable
        {
            result = result &&
                Read(obj.SlotId) &&
                Read(obj.Ind);

            SF_ASSERT(obj.SlotId >= 0);
            SF_ASSERT(obj.Ind >= 0);
            result = result && (obj.SlotId >= 0) && (obj.Ind >= 0);
        }
        break;
    case TraitInfo::tClass:
        // Read ClassTable
        {
            result = result &&
                Read(obj.SlotId) &&
                Read(obj.Ind);

            SF_ASSERT(obj.SlotId >= 0);
            SF_ASSERT(obj.Ind >= 0);
            result = result && (obj.SlotId >= 0) && (obj.Ind >= 0);
        }
        break;
    case TraitInfo::tFunction:
        {
            result = result &&
                Read(obj.SlotId) &&
                Read(obj.Ind);

            SF_ASSERT(obj.SlotId >= 0);
            SF_ASSERT(obj.Ind >= 0);
            result = result && (obj.SlotId >= 0) && (obj.Ind >= 0);
        }
        break;
    default:
        SF_ASSERT(false);
        result = false;
        break;
    }

    if (!result)
        return result;

    if (obj.HasMetaData(obj.GetKind()))
        result = result && Read(obj.meta_info);

    return result;
}

bool Reader::Read(TraitInfo::MetaInfo& obj)
{
    bool result = true;

    SInt32 count = 0;
    result = result && Read(count);

    SInd ind;
    obj.info.Reserve(count);
    for (int i = 0; i < count; ++i)
    {
        Read(ind);
        obj.info.PushBack(ind);
    }

    return result;
}

bool Reader::ReadTraits(TraitTable& t, HasTraits::ObjectTraits& obj_traits)
{
    bool result = true;

    SInd count = 0;
    result = result && Read(count);

    obj_traits.Reserve(count);
    t.TraitInfos.Reserve(t.TraitInfos.GetSize() + count);
    for (int i = 0; i < count; ++i)
    {
        obj_traits.PushBack(static_cast<SInd>(t.TraitInfos.GetSize()));
        // t.TraitInfos stores pointers.
        t.TraitInfos.PushBack(SF_HEAP_AUTO_NEW(this) TraitInfo());

        result = result && Read(*t.TraitInfos.Back());
        SF_ASSERT(result);
        if (!result)
        {
            obj_traits.PopBack();
            delete t.TraitInfos.Back();
            t.TraitInfos.PopBack();
            break;
        }
    }

    return result;
}

bool Reader::Read(TraitTable& t, HasTraits& obj)
{
    return ReadTraits(t, obj.GetObjectTraits());
}

bool Reader::Read(TraitTable& t, HasCode& obj)
{
    bool result = true;

    result = result &&
        Read(obj.method_info_ind) &&
        Read(t, static_cast<HasTraits&>(obj));

    SF_ASSERT(obj.method_info_ind >= 0);
    result = result && (obj.method_info_ind >= 0);

    return result;
}

bool Reader::Read(TraitTable& t, Instance& obj)
{
    bool result = true;

    result = result &&
        Read(obj.name_ind) &&
        Read(obj.super_name_ind) &&
        Read8(obj.flags);

    SF_ASSERT(obj.name_ind >= 0);
    SF_ASSERT(obj.super_name_ind >= 0);
    result = result && (obj.name_ind >= 0) && (obj.super_name_ind >= 0);

    if (obj.HasProtectedNamespace())
        result = result && Read(obj.protected_namespace_ind);

    result = result &&
        Read(obj.implemented_interfaces) &&
        Read(t, static_cast<HasCode&>(obj));

    return result;
}

bool Reader::Read(Instance::Interfaces& obj)
{
    bool result = true;

    SInt32 count = 0;
    result = result && Read(count);

    obj.info.Reserve(count);
    for (int i = 0; i < count; ++i)
        obj.info.PushBack(ReadU30(CP));

    return result;
}

bool Reader::Read(TraitTable& t, StaticInfo& obj)
{
    bool result = true;

    result = Read(t, static_cast<HasCode&>(obj));

    return result;
}

bool Reader::Read(TraitTable& t, ClassTable& obj)
{
    bool result = true;

    SInt32 count = 0;
    int i;

    result = result && Read(count);

    /*
    for (int i = 0; i < count; ++i)
    {
    Instance info(obj.GetMethod(), obj.GetTraits());

    s >> info;

    obj.instance.AddPick(info);
    }

    for (int i = 0; i < count; ++i)
    {
    ClassInfo info(obj.GetMethod(), obj.GetTraits());

    s >> info;

    obj.info.AddPick(info);
    }
    */

    obj.Info.Reserve(count);
    for (i = 0; result && (i < count); ++i)
    {
        // Create class info ...
        // obj.Info stores pointers.
        obj.Info.PushBack(SF_HEAP_AUTO_NEW(this) ClassInfo());

        Instance& inst_info = obj.Info.Back()->GetInstanceInfo();

        // Read Instance info ...
        result = result && Read(t, inst_info);
        if (!result)
        {
            delete obj.Info.Back();
            obj.Info.PopBack();
            break;
        }
    }

    for (i = 0; result && (i < count); ++i)
    {
        StaticInfo& stat_info = obj.Info[i]->GetStaticInfo();

        result = result && Read(t, stat_info);
    }

    return result;
}

bool Reader::Read(TraitTable& t, ScriptInfo& obj)
{
    bool result = true;

    result = Read(t, static_cast<HasCode&>(obj));

    return result;
}

bool Reader::Read(TraitTable& t, ScriptTable& obj)
{
    bool result = true;

    SInt32 count = 0;
    result = result && Read(count);

    obj.Info.Reserve(count);
    for (int i = 0; i < count; ++i)
    {
        // obj.Info stores pointers.
        obj.Info.PushBack(SF_HEAP_AUTO_NEW(this) ScriptInfo());

        result = result && Read(t, *obj.Info.Back());
        if (!result)
        {
            delete obj.Info.Back();
            obj.Info.PopBack();
            break;
        }
    }

    return result;
}

bool Reader::Read(Code& obj)
{
    bool result = true;

    obj.code = StringView(CP);

    SInt32 count = 0;
    result = result && Read(count);
    CP += count;

    return result;
}

bool Reader::Read(TraitTable& t, MethodBodyInfo& obj)
{
    bool result = true;

    result = result &&
        Read(obj.method_info_ind) &&
        Read(obj.max_stack) &&
        Read(obj.local_reg_count) &&
        Read(obj.init_scope_depth) &&
        Read(obj.max_scope_depth) &&
        Read(obj.code) &&
        Read(obj.exception) &&
        Read(t, static_cast<HasTraits&>(obj));

    SF_ASSERT(obj.method_info_ind >= 0);
    result = result && (obj.method_info_ind >= 0);

    return result;
}

bool Reader::Read(MethodBodyInfo::ExceptionInfo& obj)
{
    bool result = true;

    result = result &&
        Read(obj.from) &&
        Read(obj.to) &&
        Read(obj.target) &&
        Read(obj.exc_type_ind) &&
        Read(obj.var_name_ind);

    return result;
}

bool Reader::Read(MethodBodyInfo::Exception& obj)
{
    bool result = true;

    SInt32 count = 0;
    result = result && Read(count);

    obj.info.Reserve(count);
    for (int i = 0; result && (i < count); ++i)
    {
        MethodBodyInfo::ExceptionInfo info;

        result = result && Read(info);
        if (!result)
            break;

        obj.info.PushBack(info);
    }

    return result;
}

bool Reader::Read(TraitTable& t, MethodTable& mt, MethodBodyTable& obj)
{
    bool result = true;

    SInd count = 0;
    result = result && Read(count);

    obj.Info.Reserve(count);
    for (SInd i = 0; i < count; ++i)
    {
        // obj.Info stores pointers.
        obj.Info.PushBack(SF_HEAP_AUTO_NEW(this) MethodBodyInfo());
        MethodBodyInfo& mbi = *obj.Info.Back();

        result = result && Read(t, mbi);
        if (result)
        {
            // Establish connection between MethodBodyInfo and MethodInfo.
            mt.Info[mbi.GetMethodInfoInd().Get()]->SetMethodBodyInfoInd(MbiInd(i));
        }
        else
        {
            delete obj.Info.Back();
            obj.Info.PopBack();
            break;
        }
    }

    return result;
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {

