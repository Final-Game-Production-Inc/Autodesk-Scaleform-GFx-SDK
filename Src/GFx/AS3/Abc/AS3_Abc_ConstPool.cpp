/**************************************************************************

Filename    :   AS3_Abc_ConstPool.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Abc_ConstPool.h"
#include "AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
const Multiname Multiname::AnyType = {0, -1, 0, MN_QName};


bool Multiname::IsBoolean(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("Boolean", 7));
    }

    return result;
}

bool Multiname::IsInt(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("int", 3));
    }

    return result;
}

bool Multiname::IsUInt(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("uint", 4));
    }

    return result;
}

bool Multiname::IsNumber(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("Number", 6));
    }

    return result;
}

bool Multiname::IsString(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("String", 6));
    }

    return result;
}


bool Multiname::IsFixedNumType(const ConstPool& cp) const
{
    bool result = false;

    if (IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("int", 3) || name == StringDataPtr("uint", 4));
    }

    return result;
}

bool Multiname::IsNumericType(const ConstPool& cp) const
{
    bool result = IsFixedNumType(cp);

    if (!result && IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("Number", 6));
    }

    return result;
}

bool Multiname::IsPrimitiveType(const ConstPool& cp) const
{
    bool result = IsNumericType(cp);

    if (!result && IsQName())
    {
        const StringDataPtr name = GetName(cp);
        const Abc::NamespaceInfo& ns = GetNamespace(cp);

        if (ns.IsPublic() && ns.GetNameURI().IsEmpty())
            result = (name == StringDataPtr("Boolean", 7) || name == StringDataPtr("String", 6));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
ConstPool::ConstPool()
: any_namespace(NS_Public, "")
{
    // We do not need to initialize zero records of all arrays in const_pool.
    // That will be done by Stream& operator >>(Stream& s, ConstPool& obj)
}


///////////////////////////////////////////////////////////////////////////
const UInt8 StringView::Empty = 0;

StringDataPtr StringView::ToStringDataPtr() const
{
    const UInt8* d = Data;
    //check if it's a NULL string;
    if (d == NULL)
        return "";
    const int size = ReadU30(d);
    return ReadStringPtr(d, size);
}

///////////////////////////////////////////////////////////////////////////
double ConstPool::GetDouble(UPInt ind) const
{
    SF_ASSERT(ind < DoubleCount);

    if (ind)
    {
        const UInt8* d = Doubles;

        --ind;
        d += ind * 8;

        return ReadDouble(d);
    }

    // if ind == 0.
    return 0.0;
}

UPInt NamespaceSetInfo::GetSize() const
{
    UInt32 result;
    const UInt8* ptr = Data;

    Read(ptr, result);

    return result;
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {


