//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Array.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Array.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "Kernel/SF_MsgFormat.h"
#include "../AS3_AsString.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

namespace Impl
{

///////////////////////////////////////////////////////////////////////////////
int CompareAsString::Compare(const ASString& a, const ASString& b) const
{
    if (UseLocale)
    {
        if (Desc)
            return b.LocaleCompare_CaseCheck(a, !CaseInsensitive);

        return a.LocaleCompare_CaseCheck(b, !CaseInsensitive);
    }

    int result = 0;
    if (CaseInsensitive)
        result = String::CompareNoCase(a.ToCStr(), b.ToCStr());
    else
        result = SFstrcmp(a.ToCStr(), b.ToCStr());

    if (Desc)
        result = -result;

    return result;
}

int CompareValuePtr::Compare(const Value& a, const Value& b) const
{
    Value result;
    Value argv[2] = {a, b};

    GetVM().ExecuteUnsafe(Func, Value::GetUndefined(), result, 2, argv);

    if (GetVM().IsException())
        return 0;

//     if (!result.IsInt() && !result.ToInt32Value())
//         // exception
//         return 0;
// 
//     return result.AsInt();

    Value::Number n;
    if (!result.Convert2Number(n))
        // exception
        return 0;

    if (NumberUtil::IsNEGATIVE_INFINITY(n))
        return -1;

    if (NumberUtil::IsPOSITIVE_INFINITY(n))
        return 1;

    if (NumberUtil::IsNEGATIVE_ZERO(n))
        return -1;

    if (NumberUtil::IsPOSITIVE_ZERO(n))
        return 1;

    return (n == 0.0 ? 0 : (n < 0.0 ? -1 : 1));
}

Value::Number CompareOn::Compare(const Value& a, const Value& b) const
{
    Value::Number result = 0.0;

    const UPInt size = Fields.GetSize();
    for (UPInt i = 0; i < size && result == 0.0; ++i)
    {
        const Multiname name(GetVM().GetPublicNamespace(), Fields[i]);
        PropRef prop_a;
        PropRef prop_b;

        FindObjProperty(prop_a, GetVM(), a, name);
        FindObjProperty(prop_b, GetVM(), b, name);

        if (prop_a && prop_b)
        {
            Value value_a;
            Value value_b;

            // Read slot values.
            if (!prop_a.GetSlotValueUnsafe(GetVM(), value_a) || !prop_b.GetSlotValueUnsafe(GetVM(), value_b))
                // Exception.
                break;

            const UInt32 flags = Flags[i];
            const bool descending = (flags & Instances::fl::Array::SortFlags_Descending) != 0;

            if ((flags & Instances::fl::Array::SortFlags_Numeric) != 0)
            {
                // Compare as numerics.

                Value::Number num_a = 0;
                Value::Number num_b = 0;

                if (!value_a.Convert2Number(num_a) || !value_b.Convert2Number(num_b))
                    // Exception.
                    break;

                result = num_a - num_b;
            }
            else
            {
                // Compare as strings.

                // Convert slot values to string.
                StringManager& sm = GetVM().GetStringManager();
                ASString str_a = sm.CreateEmptyString();
                ASString str_b = str_a;

                if (!value_a.Convert2String(str_a) || !value_b.Convert2String(str_b))
                    // Exception.
                    break;

                // Compare string.
                const bool case_sensitive = (flags & Instances::fl::Array::SortFlags_CaseInsensitive) == 0;

                if ((flags & Instances::fl::Array::SortFlags_Locale) != 0)
                    result = str_a.LocaleCompare_CaseCheck(str_b, case_sensitive);
                else
                {
                    if (case_sensitive)
                        result = SFstrcmp(str_a.ToCStr(), str_b.ToCStr());
                    else
                        result = String::CompareNoCase(str_a.ToCStr(), str_b.ToCStr());
                }
            }

            if (descending)
                result = -result;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
SparseArray::SparseArray(MemoryHeap* heap)
: Length(0)
, ValueHLowInd(0)
, ValueHHighInd(0)
, ValueA(heap)
, ValueH(heap)
{

}

void SparseArray::MoveHashRight(UPInt first_ind, UPInt offset)
{
    Value v;

    for (UPInt i = ValueHHighInd + 1; i > first_ind; --i)
    {
        const UPInt ci = i - 1;

        if (ValueH.Get(ci, &v))
        {
            ValueH.Remove(ci);
            ValueH.Set(ci + offset, v);
        }
    }
}

// CutCutHash2 shifts elements.
void SparseArray::CutHash(UPInt pos, UPInt del_num, SparseArray* deleted)
{
    if (del_num == 0)
        return;

    const UPInt lastDelInd = pos + del_num - 1;
    const UPInt cutLowInd = Alg::Max(pos, ValueHLowInd);
    const UPInt cutHighInd = Alg::Min(lastDelInd, ValueHHighInd);

    Value v;

    // From cutLowInd to the end.
    // One part of this range will be cut, another part will be moved.
    for (UPInt i = cutLowInd; i <= ValueHHighInd; ++i)
    {
        if (ValueH.Get(i, &v))
        {
            ValueH.Remove(i);

            if (i > lastDelInd)
            {
                if (!v.IsUndefined())
                    // Move value.
                    ValueH.Set(i - del_num, v);
            }
            else if (deleted)
                deleted->PushBack(v);
        }
    }

    ValueHLowInd = Alg::Min(ValueHLowInd, cutLowInd);
    ValueHLowInd = (ValueHLowInd > del_num) ? ValueHLowInd - del_num : 0;

    ValueHHighInd = Alg::Max(ValueHHighInd, cutHighInd);
    ValueHHighInd = (ValueHHighInd > del_num) ? ValueHHighInd - del_num : 0;
}

void SparseArray::RemoveHash(UPInt first_ind, UPInt num)
{
    for (UInt32 i = 0; i < num; ++i)
    {
        const UPInt ind = i + first_ind;

        if (ValueH.Get(ind, NULL))
            ValueH.Remove(ind);
    }
}

void SparseArray::Resize(UPInt newLength)
{
    if (newLength == 0)
    {
        // ValueA
        ValueA.Clear();

        // ValueH
        ValueH.Clear();
        ValueHLowInd = 0;
        ValueHHighInd = 0;
    }
    else if (newLength <= ValueA.GetSize() && ValueA.GetSize() != 0)
    {
        // ValueA
        ValueA.Resize(newLength);

        // ValueH
        ValueH.Clear();
        ValueHLowInd = 0;
        ValueHHighInd = 0;
    }
    else if (newLength < GetValueHLowInd())
    {
        // ValueA is not affected.

        // ValueH
        // Just clear the hash table.
        ValueH.Clear();
        ValueHLowInd = 0;
        ValueHHighInd = 0;
    }
    else if (newLength < GetValueHHighInd())
    {
        // ValueA is not affected.

        // ValueH
        CutHash(newLength, GetValueHHighInd() - newLength + 1, NULL);
    }

    Length = newLength;
}

void SparseArray::Assign(const SparseArray& other)
{
    if (this != &other)
    {
        Length = other.Length;
        ValueHLowInd = other.ValueHLowInd;
        ValueHHighInd = other.ValueHHighInd;
        ValueH = other.ValueH;
        ValueA = other.ValueA;
    }
}

void SparseArray::PushBack(const Value& v)
{
    if (Length == ValueA.GetSize())
    {
        ValueA.PushBack(v);
    } else
    {
        ValueHHighInd = Length;
        ValueH.Set(ValueHHighInd, v);
    }

    ++Length;
}

void SparseArray::AppendN(UPInt n)
{
    Length += n;
}

void SparseArray::Append(const ValueArrayDH& x) 
{
    // Almost the same as PushBack().

    if (Length == ValueA.GetSize())
    {
        ValueA.Append(x);
        Length = ValueA.GetSize();
    } else
    {
        const UPInt size = x.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            // Keep ValueHHighInd synchronized.
            ValueHHighInd = Length;
            ValueH.Set(ValueHHighInd, x[i]);
            ++Length;
        }
    }
}

void SparseArray::Append(const ValueArray& x, UPInt ind, UPInt num) 
{
    // Almost the same as PushBack().

    const UPInt sizeX = x.GetSize();
    if (Length == ValueA.GetSize())
    {
        for (UPInt i = 0; i < num && (ind + i) < sizeX; ++i)
            ValueA.PushBack(x[ind + i]);

        Length = ValueA.GetSize();
    } else
    {
        for (UPInt i = 0; i < num && (ind + i) < sizeX; ++i)
        {
            // Keep ValueHHighInd synchronized.
            ValueHHighInd = Length;
            ValueH.Set(ValueHHighInd, x[ind + i]);
            ++Length;
        }
    }
}

void SparseArray::Pick(ValueStack& x, UPInt num)
{
    if (num == 0)
        return;

    if (Length == ValueA.GetSize())
    {
        const UPInt size = ValueA.GetSize();
        ValueA.Resize(size + num);
        memcpy(ValueA.GetDataPtr() + size, &x.TopN(static_cast<UInt16>(num - 1)), num * sizeof(Value));
        x.ReleaseN(num);

        Length = ValueA.GetSize();
    } else
    {
        UPInt ind = ValueHHighInd + num;
        for (UPInt i = 0; i < num; ++i, --ind)
        {
            ValueH.Set(ind, x.Top0());
            x.PopBack();
        }

        ValueHHighInd += num;
        Length += num;
    }
}

void SparseArray::Append(const SparseArray& other, UPInt ind, UPInt num)
{
    // Almost the same as PushBack().

    const UPInt sizeO = other.GetSize();
    if (Length == ValueA.GetSize())
    {
        for (UPInt i = 0; i < num && (ind + i) < sizeO; ++i)
            ValueA.PushBack(other.At(ind + i));

        Length = ValueA.GetSize();
    } else
    {
        for (UPInt i = 0; i < num && (ind + i) < sizeO; ++i)
        {
            // Keep ValueHHighInd synchronized.
            ValueHHighInd = Length;
            ValueH.Set(ValueHHighInd, other.At(ind + i));
            ++Length;
        }
    }
}

void SparseArray::Append(unsigned argc, const Value* argv) 
{
    // Almost the same as PushBack().

    if (Length == ValueA.GetSize())
    {
        for(unsigned i = 0; i < argc; ++i)
            ValueA.PushBack(argv[i]);

        Length = ValueA.GetSize();
    } else
    {
        for(unsigned i = 0; i < argc; ++i)
        {
            // Keep ValueHHighInd synchronized.
            ValueHHighInd = Length;
            ValueH.Set(ValueHHighInd, argv[i]);
            ++Length;
        }
    }
}

void SparseArray::Insert(UPInt pos, unsigned argc, const Value* argv) 
{
    if (pos < ValueA.GetSize())
    {
        for(unsigned i = 0; i < argc; ++i)
            ValueA.InsertAt(pos + i, argv[i]);

        if (GetValueHLowInd() > 0)
        {
            MoveHashRight(GetValueHLowInd(), argc);

            ValueHLowInd += argc;
            ValueHHighInd += argc;
        }
    } else if (pos == ValueA.GetSize())
    {
        // We can just append new values.
        for(unsigned i = 0; i < argc; ++i)
            ValueA.PushBack(argv[i]);

        if (!ValueH.IsEmpty())
        {
            MoveHashRight(GetValueHLowInd(), argc);

            ValueHLowInd += argc;
            ValueHHighInd += argc;
        }
    } else
    {
        // ValueA is not affected.

        // ValueH
        if (pos < GetValueHLowInd())
        {
            // ValueHLowInd IS affected.

            // Move old data right.
            MoveHashRight(GetValueHLowInd(), argc);

            // Recalculate.
            ValueHLowInd = pos;
            ValueHHighInd += argc;
        } else if (pos <= GetValueHHighInd())
        {
            // ValueHLowInd is NOT affected.

            // Move old data.
            MoveHashRight(pos, argc);

            // Recalculate.
            ValueHHighInd += argc;
        } else
        {
            // ValueHLowInd is NOT affected.
            ValueHHighInd = pos + argc - 1;
        }

        // Insert new data.
        for(unsigned i = 0; i < argc; ++i)
            ValueH.Set(pos + i, argv[i]);
    }

    Length += argc;
}

// Remove creates gaps.
void SparseArray::RemoveMultipleAt(UPInt pos, UPInt count, OP op)
{
    if (count == 0)
        return;

    if (pos < ValueA.GetSize())
    {
        // ValueA IS affected.
        const UPInt count_a = ValueA.GetSize() - pos;

        if (pos + count < ValueA.GetSize())
        {
            // there is a tail of ValueA. Put it into the hash table.
            const UPInt sizeA = ValueA.GetSize();
            for (UPInt i = pos + count; i < sizeA; ++i)
            {
                if (ValueHLowInd == 0)
                    ValueHLowInd = i;

                ValueH.Set(i, ValueA[i]);

                if (ValueHHighInd < i)
                    ValueHHighInd = i;
            }
        }

        ValueA.RemoveMultipleAt(pos, count_a);
#if 0
        // We must preserve size of ValueA or adjust ValueHLowInd, so, size of the Array doesn't change.
        const UPInt sizeA = pos + count_a;
        for (UPInt i = pos; i < sizeA; ++i)
            ValueA[i].SetUndefined();
#endif
    }

    // [pos, pos + count[ - range one.
    // [GetValueHLowInd(), GetValueHLowInd()] - range two.
    // If two ranges intersect.
    if (!ValueH.IsEmpty() && (GetValueHLowInd() < (pos + count) && GetValueHHighInd() >= pos))
    {
        // ValueH IS affected.

        // If the hash table is completely inside of [pos, pos + count].
        if (GetValueHLowInd() >= pos && GetValueHHighInd() < pos + count)
        {
            // Just clear whole ValueH.
            ValueH.Clear();
            ValueHLowInd = 0;
            ValueHHighInd = 0;
        }
        else
        {
            // Remove just a few elements.
            RemoveHash(Alg::Max(pos, GetValueHLowInd()), count);

            if (GetValueHHighInd() < pos + count)
                ValueHHighInd = GetLeftEqualInd(pos);

            if (GetValueHLowInd() >= pos)
                ValueHLowInd = GetRightEqualInd(pos + count, GetValueHHighInd());
        }
    }

    if (op == opCut)
    {
        if (Length <= pos + count)
            Length = pos;
    }
}       

UPInt SparseArray::GetLeftEqualInd(UPInt start_from) const
{
    SPInt i = (SPInt)start_from;

    for (; i >= 0; --i)
    {
        if (ValueH.Get(i, NULL))
            break;
    }

    return (UPInt)i;
}

UPInt SparseArray::GetRightEqualInd(UPInt start_from, UPInt end_ind) const
{
    UPInt i = start_from;

    for (; i <= end_ind; ++i)
    {
        if (ValueH.Get(i, NULL))
            break;
    }

    return i;
}

// Cut shifts elements.
void SparseArray::CutMultipleAt(UPInt pos, UPInt count, SparseArray* deleted)
{
    if (count == 0)
        return;

    if (pos < ValueA.GetSize())
    {
        // ValueA IS affected.
        const UPInt count_a = Alg::Min(count, ValueA.GetSize() - pos);

        if (deleted)
        {
            // Make a copy of deleted elements.
            for (UPInt i = pos; i < pos + count_a; ++i)
                deleted->PushBack(ValueA[i]);
        }

        ValueA.RemoveMultipleAt(pos, count_a);
    }

    CutHash(pos, count, deleted);

    Length = (GetValueHHighInd() ? GetValueHHighInd() + 1 : ValueA.GetSize());
}

const Value& SparseArray::At(UPInt ind) const
{
    if (ind < ValueA.GetSize())
        // ValueA
        return ValueA[ind];
    else if (ind < GetValueHLowInd() || ind > GetValueHHighInd())
        return DefaultValue;
    else 
    {
        const Value* vp;

        // ValueH

        if ((vp = ValueH.Get(ind)) != NULL)
            return *vp;
        else
            return DefaultValue;
    }
}

void SparseArray::Set(UPInt ind, const Value& v)
{
    if (ind >= GetSize())
        Resize(ind + 1);

    if (ind < ValueA.GetSize())
        // ValueA
        ValueA[ind] = v;
    else if (ind == ValueA.GetSize())
    {
        ValueA.PushBack(v);

        // This Optimize() call is critical here. It makes many apps run several times faster.
        Optimize();
    }
    else
    {
        ValueH.Set(ind, v);

        if (ind < GetValueHLowInd() || ValueH.GetSize() == 1)
            ValueHLowInd = ind;

        if (ind > GetValueHHighInd())
            ValueHHighInd = ind;
    }
}

void SparseArray::AdjustValueHLowInd()
{
    Value hv;

    // Try to move ValueHLowInd.
    while (ValueHLowInd <= ValueHHighInd)
    {
        if (!ValueH.Get(ValueHLowInd, &hv))
        {
            ++ValueHLowInd;
            continue;
        }

        if (hv.IsUndefined())
        {
            // Get rid of redundant undefined values.
            ValueH.Remove(ValueHLowInd);
            ++ValueHLowInd;
            continue;
        }

        // We found an element, which is not undefined.
        break;
    }
}

void SparseArray::Optimize()
{
    // Try to move data from hash table into array.
    Value hv;
    while (ValueA.GetSize() == ValueHLowInd && ValueHLowInd <= ValueHHighInd)
    {
        if (ValueH.Get(ValueHLowInd, &hv))
        {
            ValueH.Remove(ValueHLowInd);
            ++ValueHLowInd;

            if (hv.IsUndefined())
            {
                // Do not move undefined into array. Try to move ValueHLowInd instead.
                AdjustValueHLowInd();
                break;
            }
            else
                ValueA.PushBack(hv);
        }
        else
        {
            ++ValueHLowInd;

            // Try to move ValueHLowInd.
            AdjustValueHLowInd();
        }
    }

    if (ValueHLowInd > ValueHHighInd)
    {
        ValueH.Clear();
        ValueHLowInd = 0;
        ValueHHighInd = 0;
    }
}

void SparseArray::Reverse()
{
    /*
    // Generic reverse.
    UInt32 from = 0;
    UInt32 to   = GetSize() - 1;
    while(from < to)
    {
    const Value tmp = At(from);
    Set(from, At(to));
    Set(to, tmp);

    ++from;
    --to;
    }
    */

    if (GetSize() == ValueA.GetSize())
        // Just an array.
        Alg::ReverseArray(ValueA);
    else
    {
        // *from* and *to* are absolute indices.
        UInt32 from = 0;
        UPInt to   = GetSize() - 1;

        // ValueA part.
        const UPInt sizeA = ValueA.GetSize();
        for (; from < sizeA; ++from, --to)
        {
            const Value tmp = ValueA[from];
            Set(from, At(to));
            Set(to, tmp);
        }

        // Hash table only.
        Value value_from;
        Value value_to;
        bool range_changed = false;

        while(from < to)
        {
            if (ValueH.Get(from, &value_from))
            {
                // We have at least a *from* value.
                if (ValueH.Get(to, &value_to))
                {
                    // We have both *from* and *to*.
                    ValueH.Set(to, value_from);
                    ValueH.Set(from, value_to);
                }
                else
                {
                    range_changed = true;
                    ValueH.Remove(from);
                    ValueH.Set(to, value_from);
                }
            }
            else if (ValueH.Get(to, &value_to))
            {
                // We have only *to* value.
                range_changed = true;
                ValueH.Remove(to);
                ValueH.Set(from, value_to);
            }

            ++from;
            --to;
        }

        if (range_changed)
        {
            ValueHHighInd = GetLeftEqualInd(GetValueHHighInd());
            ValueHLowInd = GetRightEqualInd(0, GetValueHHighInd());
        }
    }
}

void SparseArray::ShiftLeft()
{
    if (GetSize() == 0)
        return;

    if (ValueA.GetSize() > 0)
        ValueA.RemoveAt(0);

    CutHash(0, 1, NULL);
    Optimize();

    --Length;
}

void SparseArray::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                                  SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
{
    AS3::ForEachChild_GC(prcc, ValueA, op SF_DEBUG_ARG(owner));

    ValueHashDH::ConstIterator it = ValueH.Begin();
    while (!it.IsEnd())
    {
        AS3::ForEachChild_GC(prcc, it->Second, op SF_DEBUG_ARG(owner));
        ++it;
    }
}

AbsoluteIndex SparseArray::GetNextArrayIndex(AbsoluteIndex ind) const
{
    if (!ind.IsValid())
    {
        // Get very first index.
        if (ValueA.GetSize() > 0)
            return AbsoluteIndex(0);
        else if (ValueH.GetSize() > 0)
            return AbsoluteIndex(GetValueHLowInd());
    }
    else
    {
        if (ind < ValueA.GetSize())
        {
            if (ind.Get() == ValueA.GetSize() - 1)
            {
                if (ValueH.GetSize() > 0)
                    // We do have ValueH indices.
                    return AbsoluteIndex(GetValueHLowInd());

                // Otherwise just return zero.
            }
            else
                return ind + 1;
        } if (ind.Get() >= GetValueHLowInd() && ind.Get() <= GetValueHHighInd())
        {
            for (UPInt i = ind.Get() + 1; i <= GetValueHHighInd(); ++i)
            {
                if (ValueH.Get(i, NULL))
                    return AbsoluteIndex(i);
            }
        }
    }

    return AbsoluteIndex(-1);
}

void SparseArray::ForEach(ArrayFunc& f) const
{
    ForEachDense(f);
    ForEachSparse(f);
}

void SparseArray::ForEachDense(ArrayFunc& f) const
{
    // ValueA
    const UPInt sizeA = ValueA.GetSize();
    for (UInt32 i = 0; i < sizeA; ++i)
        f(i, ValueA[i]);
}

void SparseArray::ForEachSparse(ArrayFunc& f) const
{
    // ValueH
    ValueHashDH::ConstIterator it = ValueH.Begin();
    while (!it.IsEnd())
    {
        f(static_cast<UInt32>(it->First), it->Second);
        ++it;
    }
}

///////////////////////////////////////////////////////////////////////////////
void Value2StrCollector::operator()(UInt32 ind, const Value& v)
{
    // Convert to string.
    ASString str = GetVM().GetStringManager().CreateEmptyString();
    if (!v.Convert2String(str))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(Impl::StringValueTriple(str, &v, ind));
}

void ValuePtrCollector::operator()(UInt32 ind, const Value& v)
{
    // Store a pair
    Pairs.PushBack(Impl::ValuePtrPair(&v, ind));
}

void Value2NumberCollector::operator()(UInt32 ind, const Value& v)
{
    Value::Number num;
    if (!v.Convert2Number(num))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(Impl::NumberValueTriple(num, &v, ind));
}

} // namespace Impl

//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3slice, 1, SInt32>(AS3::StringManager&)
    {
        return 16777215;
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3sortOn, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3lastIndexOf, 1, SInt32>(AS3::StringManager&)
    {
        return 0x7fffffff;
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3every, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3filter, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3forEach, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3map, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl::Array, Instances::fl::Array::mid_AS3some, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl::Array, Instances::fl::Array::mid_lengthGet, UInt32> TFunc_Instances_Array_lengthGet;
typedef ThunkFunc1<Instances::fl::Array, Instances::fl::Array::mid_lengthSet, const Value, UInt32> TFunc_Instances_Array_lengthSet;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3join, Value, unsigned, const Value*> TFunc_Instances_Array_AS3join;
typedef ThunkFunc0<Instances::fl::Array, Instances::fl::Array::mid_AS3pop, Value> TFunc_Instances_Array_AS3pop;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3push, Value, unsigned, const Value*> TFunc_Instances_Array_AS3push;
typedef ThunkFunc0<Instances::fl::Array, Instances::fl::Array::mid_AS3reverse, SPtr<Instances::fl::Array> > TFunc_Instances_Array_AS3reverse;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3concat, Value, unsigned, const Value*> TFunc_Instances_Array_AS3concat;
typedef ThunkFunc0<Instances::fl::Array, Instances::fl::Array::mid_AS3shift, Value> TFunc_Instances_Array_AS3shift;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3slice, SPtr<Instances::fl::Array>, SInt32, SInt32> TFunc_Instances_Array_AS3slice;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3unshift, Value, unsigned, const Value*> TFunc_Instances_Array_AS3unshift;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3splice, Value, unsigned, const Value*> TFunc_Instances_Array_AS3splice;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3sort, Value, unsigned, const Value*> TFunc_Instances_Array_AS3sort;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3sortOn, SPtr<Instances::fl::Array>, const Value&, const Value&> TFunc_Instances_Array_AS3sortOn;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3indexOf, SInt32, const Value&, SInt32> TFunc_Instances_Array_AS3indexOf;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3lastIndexOf, SInt32, const Value&, SInt32> TFunc_Instances_Array_AS3lastIndexOf;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3every, bool, const Value&, const Value&> TFunc_Instances_Array_AS3every;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3filter, SPtr<Instances::fl::Array>, const Value&, const Value&> TFunc_Instances_Array_AS3filter;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3forEach, const Value, const Value&, const Value&> TFunc_Instances_Array_AS3forEach;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3map, SPtr<Instances::fl::Array>, const Value&, const Value&> TFunc_Instances_Array_AS3map;
typedef ThunkFunc2<Instances::fl::Array, Instances::fl::Array::mid_AS3some, bool, const Value&, const Value&> TFunc_Instances_Array_AS3some;

template <> const TFunc_Instances_Array_lengthGet::TMethod TFunc_Instances_Array_lengthGet::Method = &Instances::fl::Array::lengthGet;
template <> const TFunc_Instances_Array_lengthSet::TMethod TFunc_Instances_Array_lengthSet::Method = &Instances::fl::Array::lengthSet;
template <> const TFunc_Instances_Array_AS3join::TMethod TFunc_Instances_Array_AS3join::Method = &Instances::fl::Array::AS3join;
template <> const TFunc_Instances_Array_AS3pop::TMethod TFunc_Instances_Array_AS3pop::Method = &Instances::fl::Array::AS3pop;
template <> const TFunc_Instances_Array_AS3push::TMethod TFunc_Instances_Array_AS3push::Method = &Instances::fl::Array::AS3push;
template <> const TFunc_Instances_Array_AS3reverse::TMethod TFunc_Instances_Array_AS3reverse::Method = &Instances::fl::Array::AS3reverse;
template <> const TFunc_Instances_Array_AS3concat::TMethod TFunc_Instances_Array_AS3concat::Method = &Instances::fl::Array::AS3concat;
template <> const TFunc_Instances_Array_AS3shift::TMethod TFunc_Instances_Array_AS3shift::Method = &Instances::fl::Array::AS3shift;
template <> const TFunc_Instances_Array_AS3slice::TMethod TFunc_Instances_Array_AS3slice::Method = &Instances::fl::Array::AS3slice;
template <> const TFunc_Instances_Array_AS3unshift::TMethod TFunc_Instances_Array_AS3unshift::Method = &Instances::fl::Array::AS3unshift;
template <> const TFunc_Instances_Array_AS3splice::TMethod TFunc_Instances_Array_AS3splice::Method = &Instances::fl::Array::AS3splice;
template <> const TFunc_Instances_Array_AS3sort::TMethod TFunc_Instances_Array_AS3sort::Method = &Instances::fl::Array::AS3sort;
template <> const TFunc_Instances_Array_AS3sortOn::TMethod TFunc_Instances_Array_AS3sortOn::Method = &Instances::fl::Array::AS3sortOn;
template <> const TFunc_Instances_Array_AS3indexOf::TMethod TFunc_Instances_Array_AS3indexOf::Method = &Instances::fl::Array::AS3indexOf;
template <> const TFunc_Instances_Array_AS3lastIndexOf::TMethod TFunc_Instances_Array_AS3lastIndexOf::Method = &Instances::fl::Array::AS3lastIndexOf;
template <> const TFunc_Instances_Array_AS3every::TMethod TFunc_Instances_Array_AS3every::Method = &Instances::fl::Array::AS3every;
template <> const TFunc_Instances_Array_AS3filter::TMethod TFunc_Instances_Array_AS3filter::Method = &Instances::fl::Array::AS3filter;
template <> const TFunc_Instances_Array_AS3forEach::TMethod TFunc_Instances_Array_AS3forEach::Method = &Instances::fl::Array::AS3forEach;
template <> const TFunc_Instances_Array_AS3map::TMethod TFunc_Instances_Array_AS3map::Method = &Instances::fl::Array::AS3map;
template <> const TFunc_Instances_Array_AS3some::TMethod TFunc_Instances_Array_AS3some::Method = &Instances::fl::Array::AS3some;

namespace Instances { namespace fl
{
    Array::Array(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Array::Array()$data"
    , SA(t.GetVM().GetMemoryHeap())
//##protect##"instance::Array::Array()$data"
    {
//##protect##"instance::Array::Array()$code"
//##protect##"instance::Array::Array()$code"
    }

    void Array::lengthGet(UInt32& result)
    {
//##protect##"instance::Array::lengthGet()"
        result = static_cast<UInt32>(GetSize());
//##protect##"instance::Array::lengthGet()"
    }
    void Array::lengthSet(const Value& result, UInt32 newLength)
    {
//##protect##"instance::Array::lengthSet()"
        SF_UNUSED1(result);
        Resize(newLength);
//##protect##"instance::Array::lengthSet()"
    }
    void Array::AS3join(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3join()"
        // !!!
        // NOTE The join function is intentionally generic; it does not require 
        // that its this value be an Array object. Therefore, it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the join function can be applied successfully to a host object
        // is implementation-dependent.

        StringManager& sm = GetStringManager();
        ASString sep = sm.GetBuiltin(AS3Builtin_comma);

        if (argc > 0)
            if (!argv[0].Convert2String(sep))
                // Exception.
                return;

        result = ToStringInternal(sep);
//##protect##"instance::Array::AS3join()"
    }
    void Array::AS3pop(Value& result)
    {
//##protect##"instance::Array::AS3pop()"
        // !!!
        // NOTE The pop function is intentionally generic; it does not require 
        // that its this value be an Array object. Therefore it can be
        // transferred to other kinds of objects for use as a method. Whether 
        // the pop function can be applied successfully to a host object
        // is implementation-dependent.

        if (GetSize() > 0)
        {
            result = Top();
            Pop();
        }
//##protect##"instance::Array::AS3pop()"
    }
    void Array::AS3push(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3push()"
        // !!!
        // NOTE The push function is intentionally generic; it does not require 
        // that its this value be an Array object. Therefore it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the push function can be applied successfully to a host object
        // is implementation-dependent.

        Append(argc, argv);
        result.SetUInt32(static_cast<UInt32>(GetSize()));
//##protect##"instance::Array::AS3push()"
    }
    void Array::AS3reverse(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::Array::AS3reverse()"
        // !!! 
        // NOTE The reverse function is intentionally generic; it does not 
        // require that its this value be an Array object. Therefore, it can
        // be transferred to other kinds of objects for use as a method. 
        // Whether the reverse function can be applied successfully to a
        // host object is implementation-dependent.

        Reverse();
        result = this;
//##protect##"instance::Array::AS3reverse()"
    }
    void Array::AS3concat(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3concat()"
        // Concatenates the elements specified in the parameters with the 
        // elements in an array and creates a new array. If the parameters 
        // specify an array, the elements of that array are concatenated.
        
        // !!!
        // NOTE The concat function is intentionally generic; it does not 
        // require that its this value be an Array object. Therefore it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the concat function can be applied successfully to a host
        // object is implementation-dependent.

        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetTraits());
        Pickable<Array> r = MakeInstance(itr);
        
        result.Pick(r);
        r.GetPtr()->Assign(*this);
        
        if (argc == 1)
        {
            const Traits& tr = GetVM().GetValueTraits(argv[0]);
            if (tr.GetTraitsType() == Traits_Array && tr.IsInstanceTraits())
                return r.GetPtr()->Append(static_cast<Array&>(*argv[0].GetObject()));
        }

        r.GetPtr()->Append(argc, argv);
//##protect##"instance::Array::AS3concat()"
    }
    void Array::AS3shift(Value& result)
    {
//##protect##"instance::Array::AS3shift()"
        // !!!
        // NOTE The shift function is intentionally generic; it does not 
        // require that its this value be an Array object. Therefore it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the shift function can be applied successfully to a host
        // object is implementation-dependent.

        if (GetSize() == 0)
            result.SetUndefined();
        else
        {
            result = At(0);
            ShiftLeft();
        }
//##protect##"instance::Array::AS3shift()"
    }
    void Array::AS3slice(SPtr<Instances::fl::Array>& result, SInt32 startIndex, SInt32 endIndex)
    {
//##protect##"instance::Array::AS3slice()"
        // !!!
        // NOTE The slice function is intentionally generic; it does not 
        // require that its this value be an Array object. Therefore it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the slice function can be applied successfully to a host
        // object is implementation-dependent.

        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetTraits());
        Pickable<Array> r = MakeInstance(itr);

        result.Pick(r);
        
        if (startIndex < 0)
            startIndex += static_cast<SInt32>(GetSize());

        startIndex = Alg::Max<SInt32>(0, startIndex);

        if (endIndex < 0)
            endIndex += static_cast<SInt32>(GetSize());

        endIndex = Alg::Min<SInt32>(static_cast<SInt32>(GetSize()), endIndex);

        for (SInt32 i = startIndex; i < endIndex; ++i)
            r.GetPtr()->PushBack(At(i));
//##protect##"instance::Array::AS3slice()"
    }
    void Array::AS3unshift(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3unshift()"
        // !!!
        // NOTE The unshift function is intentionally generic; it does not 
        // require that its this value be an Array object. Therefore it can
        // be transferred to other kinds of objects for use as a method. 
        // Whether the unshift function can be applied successfully to a
        // host object is implementation-dependent.

        Insert(0, argc, argv);
        result.SetUInt32(static_cast<UInt32>(GetSize()));
//##protect##"instance::Array::AS3unshift()"
    }
    void Array::AS3splice(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3splice()"
        // !!!
        // NOTE The splice function is intentionally generic; it does not require 
        // that its this value be an Array object. Therefore it can
        // be transferred to other kinds of objects for use as a method. 
        // Whether the splice function can be applied successfully to a host
        // object is implementation-dependent.

        SInt32 startIndex = 0;
        if (argc > 0)
            if (!argv[0].Convert2Int32(startIndex))
                // Exception.
                return;

        if (startIndex < 0)
            startIndex += static_cast<SInt32>(GetSize());

        startIndex = Alg::Max<SInt32>(0, startIndex);

        UInt32 deleteCount = static_cast<UInt32>(GetSize());
        
        if (argc > 1)
        {
            if (!argv[1].Convert2UInt32(deleteCount))
                // Exception.
                return;
        }
        else
            // If you do not specify a value for the deleteCount parameter, 
            // the method deletes all of the values from the startIndex  
            // element to the last element in the array.
            deleteCount -= startIndex;

        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetTraits());
        Pickable<Array> r = MakeInstance(itr);

        result.Pick(r);

        // Cut.
        CutMultipleAt(startIndex, deleteCount, r.GetPtr());
        
        // Insert the rest ...
        if (argc > 2)
        {
            startIndex = Alg::Min<SInt32>(static_cast<SInt32>(GetSize()), startIndex);
            Insert(startIndex, argc - 2, argv + 2);
        }
//##protect##"instance::Array::AS3splice()"
    }
    void Array::AS3sort(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Array::AS3sort()"
        // !!!
        // NOTE The sort function is intentionally generic; it does not require 
        // that its this value be an Array object. Therefore, it can be
        // transferred to other kinds of objects for use as a method. 
        // Whether the sort function can be applied successfully to a host object
        // is implementation-dependent.

        Value compareFunction;
        SInt32 flags = 0;

        if(argc > 0)
        {
            if(argv[0].IsCallable()) 
                compareFunction = argv[0];
            else 
            {
                if (!argv[0].Convert2Int32(flags))
                    return;
            }
        }

        if(argc > 1)
            if (!argv[1].Convert2Int32(flags))
                return;

        Impl::SparseArray newSA(GetVM().GetMemoryHeap());

        // There are two major branches: with compareFunction and without compareFunction.
        if (compareFunction.IsNullOrUndefined())
        {
            // We can compare as strings or as numbers.
            if (flags & Instances::fl::Array::SortFlags_Numeric)
            {
                // Compare as numbers.

                // Convert values to numbers and collect them into an array.
                ArrayDH<Impl::NumberValueTriple> pairs(GetVM().GetMemoryHeap());
                Impl::Value2NumberCollector vc(pairs);
                SA.ForEach(vc);

                // Choose a correct sorting function.
                Impl::CompareAsNumber functor((flags & Instances::fl::Array::SortFlags_Descending) != 0);

                // Sort.
                Alg::QuickSortSafe(pairs, functor);
                const UPInt size = pairs.GetSize();

                // Check for UNIQUESORT
                if (flags & Instances::fl::Array::SortFlags_UniqueSort)
                {
                    for (UInt32 i = 1; i < size; ++i)
                    {
                        if (functor.Equal(pairs[i - 1], pairs[i]))
                        {
                            // two or more elements being sorted have identical sort fields.
                            // Documentation says we should return ZERO.
                            //result.SetSInt32(0);
                            result.SetNull();
                            return;
                        }
                    }
                }

                // Initialize a new SparseArray.
                {
                    if (flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
                        for (UPInt i = 0; i < size; ++i)
                            newSA.PushBack(Value(pairs[i].Third));
                    else
                        for (UPInt i = 0; i < size; ++i)
                            newSA.PushBack(*pairs[i].Second);

                    // Preserve old size of Array.
                    if (size < GetSize())
                        newSA.Resize(GetSize());
                }
            }
            else
            {
                // Compare as strings.

                // Convert values to strings and collect them into an array.
                ArrayDH<Impl::StringValueTriple> pairs(GetVM().GetMemoryHeap());
                Impl::Value2StrCollector vc(GetVM(), pairs);
                SA.ForEach(vc);

                // Choose a correct sorting function.
                Impl::CompareAsString functor (
                    (flags & Instances::fl::Array::SortFlags_Descending) != 0,
                    (flags & Instances::fl::Array::SortFlags_CaseInsensitive) != 0,
                    (flags & Instances::fl::Array::SortFlags_Locale) != 0
                    );


                // Sort.
                Alg::QuickSortSafe(pairs, functor);
                const UPInt size = pairs.GetSize();

                // Check for UNIQUESORT
                if (flags & Instances::fl::Array::SortFlags_UniqueSort)
                {
                    for (UInt32 i = 1; i < size; ++i)
                    {
                        if (functor.Equal(pairs[i - 1], pairs[i]))
                        {
                            // two or more elements being sorted have identical sort fields.
                            // Documentation says we should return ZERO.
                            //result.SetSInt32(0);
                            result.SetNull();
                            return;
                        }
                    }
                }

                // Initialize new SparseArray.
                {
                    if (flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
                        for (UPInt i = 0; i < size; ++i)
                            newSA.PushBack(Value(pairs[i].Third));
                    else
                        for (UPInt i = 0; i < size; ++i)
                            newSA.PushBack(*pairs[i].Second);

                    // Preserve old size of Array.
                    if (size < GetSize())
                        newSA.Resize(GetSize());
                }
            }
        } else
        {
            // Compare using compareFunction.

            // Collect values into an array.
            ArrayDH<Impl::ValuePtrPair> pairs(GetVM().GetMemoryHeap());
            Impl::ValuePtrCollector vc(pairs);
            SA.ForEach(vc);

            // Sort.
            Impl::CompareValuePtr functor(
                GetVM(),
                compareFunction,
                (flags & Instances::fl::Array::SortFlags_Descending) != 0
                );
            Alg::QuickSortSafe(pairs, functor);
            const UPInt size = pairs.GetSize();

            // Check for UNIQUESORT (do we really need to do this?)
            if (flags & Instances::fl::Array::SortFlags_UniqueSort)
            {
                for (UInt32 i = 1; i < size; ++i)
                {
                    if (functor.Equal(pairs[i - 1], pairs[i]))
                    {
                        // two or more elements being sorted have identical sort fields.
                        // Documentation says we should return ZERO.
                        //result.SetSInt32(0);
                        result.SetNull();
                        return;
                    }
                }
            }

            // Initialize new SparseArray.
            {
                if (flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
                    for (UPInt i = 0; i < size; ++i)
                        newSA.PushBack(Value(pairs[i].Second));
                else
                    for (UPInt i = 0; i < size; ++i)
                        newSA.PushBack(*pairs[i].First);

                // Preserve old size of Array.
                if (size < GetSize())
                    newSA.Resize(GetSize());
            }
        }

        if (flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
        {
            Pickable<Instances::fl::Array> newArr = GetVM().MakeArray();
            // Assign first to prevent GC side effects.
            result = newArr;
            // TBD: !!! Not efficient.
            newArr->SA = newSA;
        }
        else
        {
            // TBD: !!! Not efficient.
            SA = newSA;
            result = this;
        }
//##protect##"instance::Array::AS3sort()"
    }
    void Array::AS3sortOn(SPtr<Instances::fl::Array>& result, const Value& fieldName, const Value& options)
    {
//##protect##"instance::Array::AS3sortOn()"
        MemoryHeap* heap = GetVM().GetMemoryHeap();
        ASString tmp = GetVM().GetStringManager().CreateEmptyString();
        // We use ValueArrayDH instead of ArrayCC<ASString> here because
        // ArrayCC is ArrayLH.
        ValueArrayDH fields(heap); 
        ArrayDH<UInt32> flags(heap);

        // Sort fields.
        const Traits& tr = GetVM().GetValueTraits(fieldName);
        if (tr.GetTraitsType() == Traits_Array && tr.IsInstanceTraits())
        {
            // Sort on an Array of values.
            Instances::fl::Array* arr = static_cast<Instances::fl::Array*>(fieldName.GetObject());
            SF_ASSERT(arr);

            for (UPInt i = 0; i < arr->GetSize(); ++i)
            {
                if (!arr->At(i).Convert2String(tmp))
                    return;

                fields.PushBack(tmp);
            }
        }
        else
        {
            // Convert to an ASString.
            if (!fieldName.Convert2String(tmp))
                return;

            fields.PushBack(tmp);
        }

        // Options.

        // By default use default options for each field.
        flags.Resize(fields.GetSize());
        for (UPInt i = 0; i < flags.GetSize(); ++i)
            flags[i] = 0;

        if (!options.IsNullOrUndefined())
        {
            const Traits& trs = GetVM().GetValueTraits(options);
            if (trs.GetTraitsType() == Traits_Array && trs.IsInstanceTraits())
            {
                Instances::fl::Array* arr = static_cast<Instances::fl::Array*>(options.GetObject());
                SF_ASSERT(arr);

                // Read flags;

                // Note: The fieldName and options arrays must have the same number 
                // of elements; otherwise, the options  array is ignored
                if (arr->GetSize() == fields.GetSize())
                {
                    for (UPInt i = 0; i < arr->GetSize() && i < flags.GetSize(); ++i)
                    {
                        UInt32 num;

                        if (!arr->At(i).Convert2UInt32(num))
                            return;

                        flags[i] = num;
                    }
                }
            }
            else
            {
                UInt32 flag_set;
                if (!options.Convert2UInt32(flag_set))
                    return;

                for (UPInt i = 0; i < flags.GetSize(); ++i)
                    flags[i] = flag_set;
            }
        } 

        // This block can be similar to one in the AS3sort() method.
        {
            // Compare using compareFunction.

            Impl::SparseArray newSA(heap);

            // Collect values into an array.
            ArrayDH<Impl::ValuePtrPair> pairs(GetVM().GetMemoryHeap());
            Impl::ValuePtrCollector vc(pairs);
            SA.ForEach(vc);

            // Sort.
            Impl::CompareOn functor(GetVM(), fields, flags);
            Alg::QuickSortSafe(pairs, functor);

            const UInt32 cur_flags = flags[0];
            const UPInt size = pairs.GetSize();

            // Check for UNIQUESORT (do we really need to do this?)
            if (cur_flags & Instances::fl::Array::SortFlags_UniqueSort)
            {
                for (UInt32 i = 1; i < size; ++i)
                {
                    if (functor.Equal(pairs[i - 1], pairs[i]))
                    {
                        // two or more elements being sorted have identical sort fields.
                        // Documentation says we should return ZERO.
                        //result.SetSInt32(0);
                        result = NULL;
                        return;
                    }
                }
            }

            // Initialize a new SparseArray.
            {
                if (cur_flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
                    for (UPInt i = 0; i < size; ++i)
                        newSA.PushBack(Value(pairs[i].Second));
                else
                    for (UPInt i = 0; i < size; ++i)
                        newSA.PushBack(*pairs[i].First);

                // Preserve old size of Array.
                if (size < GetSize())
                    newSA.Resize(GetSize());
            }

            if (cur_flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
            {
                Pickable<Instances::fl::Array> newArr = GetVM().MakeArray();
                // Assign first to prevent GC side effects.
                result = newArr;
                // TBD: !!! Not efficient.
                newArr->SA = newSA;
            }
            else
            {
                // TBD: !!! Not efficient.
                SA = newSA;
                result = this;
            }
        }
//##protect##"instance::Array::AS3sortOn()"
    }
    void Array::AS3indexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex)
    {
//##protect##"instance::Array::AS3indexOf()"
        if (fromIndex < 0)
            fromIndex = static_cast<SInt32>(GetSize()) + fromIndex;

        for(UPInt i = fromIndex; i < GetSize(); ++i)
        {
            if (StrictEqual(At(i), searchElement))
            {
                result = static_cast<SInt32>(i);
                return;
            }
        }
        
        result = -1;
//##protect##"instance::Array::AS3indexOf()"
    }
    void Array::AS3lastIndexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex)
    {
//##protect##"instance::Array::AS3lastIndexOf()"
        if (fromIndex < 0)
            fromIndex = static_cast<SInt32>(GetSize()) + fromIndex;

        for(SPInt i = Alg::Min<SPInt>(fromIndex, static_cast<SPInt>(GetSize()) - 1); i >= 0; --i)
        {
            if (StrictEqual(At(i), searchElement))
            {
                result = static_cast<SInt32>(i);
                return;
            }
        }
        
        result = -1;
//##protect##"instance::Array::AS3lastIndexOf()"
    }
    void Array::AS3every(bool& result, const Value& callback, const Value& thisObject)
    {
//##protect##"instance::Array::AS3every()"
        if (!callback.IsNullOrUndefined())
        {
            Value _this(thisObject.IsNullOrUndefined() ? callback : thisObject);
            
            for(UPInt i = 0; i < GetSize(); ++i)
            {
                const Value argv[3] = {At(i), Value(i), Value(this)};
                
                if (!callback.IsCallable())
                    break;

                Value resultValue;
                GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

                if (IsException())
                    break;
                
                if (!resultValue.IsBool() || resultValue.AsBool() == false)
                    break;
            }
        }
        
        result = false;
//##protect##"instance::Array::AS3every()"
    }
    void Array::AS3filter(SPtr<Instances::fl::Array>& result, const Value& callback, const Value& thisObject)
    {
//##protect##"instance::Array::AS3filter()"
        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetTraits());
        Pickable<Array> r = MakeInstance(itr);

        result.Pick(r);
        
        if (!callback.IsNullOrUndefined())
        {
            Value _this(thisObject.IsNullOrUndefined() ? callback : thisObject);
            
            for(UPInt i = 0; i < GetSize(); ++i)
            {
                const Value argv[3] = {At(i), Value(i), Value(this)};
                
                if (!callback.IsCallable())
                    break;

                Value resultValue;
                GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

                if (IsException())
                    break;
                
                if (!resultValue.IsBool() || resultValue.AsBool() == false)
                    continue;
                
                r->PushBack(At(i));
            }
        }
//##protect##"instance::Array::AS3filter()"
    }
    void Array::AS3forEach(const Value& result, const Value& callback, const Value& thisObject)
    {
//##protect##"instance::Array::AS3forEach()"
        if (!callback.IsNullOrUndefined())
        {
            Value _this(thisObject.IsNullOrUndefined() ? callback : thisObject);
            
            for(UPInt i = 0; i < GetSize(); ++i)
            {
                const Value argv[3] = {At(i), Value(i), Value(this)};
                
                if (!callback.IsCallable())
                    break;

                Value resultValue;
                GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

                if (IsException())
                    break;
            }
        }
        SF_UNUSED1(result);
//##protect##"instance::Array::AS3forEach()"
    }
    void Array::AS3map(SPtr<Instances::fl::Array>& result, const Value& callback, const Value& thisObject)
    {
//##protect##"instance::Array::AS3map()"
        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetTraits());
        Pickable<Array> r = MakeInstance(itr);

        result.Pick(r);
        
        if (!callback.IsNullOrUndefined())
        {
            Value _this(thisObject.IsNullOrUndefined() ? callback : thisObject);
            
            for(UPInt i = 0; i < GetSize(); ++i)
            {
                const Value argv[3] = {At(i), Value(i), Value(this)};
                
                if (!callback.IsCallable())
                    break;

                Value resultValue;
                GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

                if (IsException())
                    break;
                
                r->PushBack(resultValue);
            }
        }
//##protect##"instance::Array::AS3map()"
    }
    void Array::AS3some(bool& result, const Value& callback, const Value& thisObject)
    {
//##protect##"instance::Array::AS3some()"
        if (!callback.IsNullOrUndefined())
        {
            Value _this(thisObject.IsNullOrUndefined() ? callback : thisObject);
            
            for(UPInt i = 0; i < GetSize(); ++i)
            {
                const Value argv[3] = {At(i), Value(i), Value(this)};
                
                if (!callback.IsCallable())
                    break;

                Value r;
                GetVM().ExecuteUnsafe(callback, _this, r, 3, argv);

                if (IsException())
                    break;
                
                if (!r.IsBool() || r.AsBool() == true)
                {
                    result = true;
                    return;
                }
            }
        }

        result = false;
//##protect##"instance::Array::AS3some()"
    }

//##protect##"instance$methods"
    void Array::toLocaleStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED4(ti, vm, argc, argv);

        /*
        // This is not a generic method.
        if (!vm.GetClass(_this).IsOfType(vm.GetClassArray()))
        return vm.ThrowTypeError(VM::eInvokeOnIncompatibleObjectError);
        */
        result = static_cast<Instances::fl::Array*>(_this.GetObject())->ToLocaleStringInternal();
    }

    void Array::toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED3(ti, argc, argv);

        /*
        // This is not a generic method.
        if (!vm.GetClass(_this).IsOfType(vm.GetClassArray()))
            return vm.ThrowTypeError(VM::eInvokeOnIncompatibleObjectError);
        */
        const ASString& sep = vm.GetStringManager().GetBuiltin(AS3Builtin_comma);
        result = static_cast<Instances::fl::Array*>(_this.GetObject())->ToStringInternal(sep);
    }

    void Array::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        SA.ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));
    }

    void Array::AS3Constructor(unsigned argc, const Value* argv)
    {
        // Calling Array() with no arguments returns an empty array.

        if (argc == 1)
        {
            // Calling Array() with a single integer argument returns an array 
            // of the specified length, but whose elements have undefined values.

            if (argv[0].IsNumeric())
            {
                if (argv[0].IsNumber())
                {
                    // Check that a number is actually an integer.
                    double intp; // We cannot use Value::Number here.
                    Value::Number fractp = modf(argv[0].AsNumber(), &intp);

                    if (fractp != 0)
                        return GetVM().ThrowRangeError(VM::Error(VM::eArrayIndexNotIntegerError, GetVM() SF_DEBUG_ARG((int)fractp)));
                }

                Value::Number num;
                if (!argv[0].Convert2Number(num))
                    return;

                if (num < 0)
                    return GetVM().ThrowRangeError(VM::Error(VM::eArrayIndexNotIntegerError, GetVM() SF_DEBUG_ARG((int)num)));

                // Reserve argv[0] ...
                Resize((UInt32)num);
                return;
            }

            // In case argv[0] is not a number, it will be added to the array.
        }

        // Calling Array() with a list of specific values returns an array 
        // with elements that contain each of the specified values.
        Append(argc, argv);
    }

    ASString Array::ToStringInternal(const ASString& sep) const
    {
        StringBuffer buff(GetVM().GetMemoryHeap());
        
        for(UPInt i = 0; i < GetSize(); ++i)
        {
            if (i > 0)
                buff += sep.ToCStr();

            const Value& v = At(i);

            if (v.IsNullOrUndefined())
                continue;

            if (!v.Convert2String(buff))
                // Exception.
                break;
        }

        return GetVM().GetStringManager().CreateString(buff.ToCStr(), buff.GetSize());
    }

    ASString Array::ToLocaleStringInternal() const
    {
        StringBuffer buff(GetVM().GetMemoryHeap());

        for(UPInt i = 0; i < GetSize(); ++i)
        {
            if (i > 0)
                buff += ",";

            const Value& v = At(i);

            if (v.IsNullOrUndefined())
                continue;

            const Multiname prop_name(GetVM().GetPublicNamespace(), GetStringManager().CreateConstString("toLocaleString"));

            Value r;
            if (!AS3::ExecutePropertyUnsafe(GetVM(), prop_name, v, r, 0, NULL))
                // Exception.
                break;

            if (r.IsString())
                buff += r.AsString().ToCStr();
            else
                buff += AS3::AsString(r, GetStringManager()).ToCStr();
        }

        return GetVM().GetStringManager().CreateString(buff.ToCStr(), buff.GetSize());
    }

    void Array::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        if (!ind.IsValid())
            return name.SetUndefined();

        if (ind.Get() <= GetSize())
            name.SetUInt32(static_cast<UInt32>(AbsoluteIndex(ind).Get()));
        else
            Object::GetNextPropertyName(name, ind - static_cast<UInt32>(GetSize()));
    }

    GlobalSlotIndex Array::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
        GlobalSlotIndex gind(0);

        if (ind.Get() <= GetSize())
        {
            const AbsoluteIndex aind = GetNextArrayIndex(AbsoluteIndex(ind));

            if (aind.IsValid())
            {
                gind = GlobalSlotIndex(aind);
                return gind;
            }
        }

        gind = Object::GetNextDynPropIndex(ind < static_cast<UInt32>(GetSize()) ? ind : ind - static_cast<UInt32>(GetSize()));

        if (gind.IsValid())
            gind += static_cast<UInt32>(GetSize());

        return gind;
    }

    void Array::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
    {
        if (!ind.IsValid())
            return value.SetUndefined();

        if (ind.Get() <= GetSize())
            value = At(AbsoluteIndex(ind).Get());
        else
            Object::GetNextPropertyValue(value, ind - static_cast<UInt32>(GetSize()));
    }

    CheckResult Array::SetProperty(const Multiname& prop_name, const Value& value)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
        {
            Set(ind, value);
            return true;;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::SetProperty(prop_name, value);
    }

    CheckResult Array::GetProperty(const Multiname& prop_name, Value& value)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
        {
            value = At(ind);
            return true;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::GetProperty(prop_name, value);
    }

    void Array::GetDynamicProperty(AbsoluteIndex ind, Value& value)
    {
        value = At(ind.Get());
    }

    CheckResult Array::DeleteProperty(const Multiname& prop_name)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
        {
            if (ind >= GetSize())
                return false;

            RemoveAt(ind);

            return true;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::DeleteProperty(prop_name);
    }

    bool Array::HasProperty(const Multiname& prop_name, bool check_prototype)
    {
        const Value& name = prop_name.GetName();

        if (name.IsInt())
        {
            // This case also handles UInt.
            const SInt32 ind = name.AsInt();
            return (ind >= 0 && static_cast<UPInt>(ind) < GetSize());
        }
        else if (name.IsString())
        {
            UInt32 ind = 0;

            if (GetArrayInd(name.GetStringNode(), ind))
                return (ind < GetSize());
        }

        return Instances::fl::Object::HasProperty(prop_name, check_prototype);
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 Array::tito[Array::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 18, 21, 24, 27, 30, 33, 36, 
    // };
    const TypeInfo* Array::tit[39] = {
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        NULL, 
        &AS3::fl::uintTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::ArrayTI, 
        NULL, 
        &AS3::fl::ArrayTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::uintTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::ArrayTI, &AS3::fl::ObjectTI, &AS3::fl::ObjectTI, 
        &AS3::fl::int_TI, NULL, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, NULL, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, &AS3::fl::FunctionTI, NULL, 
        &AS3::fl::ArrayTI, &AS3::fl::FunctionTI, NULL, 
        NULL, &AS3::fl::FunctionTI, NULL, 
        &AS3::fl::ArrayTI, &AS3::fl::FunctionTI, NULL, 
        &AS3::fl::BooleanTI, &AS3::fl::FunctionTI, NULL, 
    };
    const Abc::ConstValue Array::dva[7] = {
        {Abc::CONSTANT_Int, 2}, 
        {Abc::CONSTANT_Int, 1}, 
        {Abc::CONSTANT_Null, 0}, 
        {Abc::CONSTANT_Null, 0}, 
        {Abc::CONSTANT_Null, 0}, 
        {Abc::CONSTANT_Null, 0}, 
        {Abc::CONSTANT_Null, 0}, 
    };
    const ThunkInfo Array::ti[Array::ThunkInfoNum] = {
        {TFunc_Instances_Array_lengthGet::Func, &Array::tit[0], "length", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Array_lengthSet::Func, &Array::tit[1], "length", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Array_AS3join::Func, &Array::tit[3], "join", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_Array_AS3pop::Func, &Array::tit[4], "pop", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Array_AS3push::Func, &Array::tit[5], "push", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Array_AS3reverse::Func, &Array::tit[6], "reverse", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Array_AS3concat::Func, &Array::tit[7], "concat", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Array_AS3shift::Func, &Array::tit[8], "shift", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Array_AS3slice::Func, &Array::tit[9], "slice", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 0, 1, &Array::dva[0]},
        {TFunc_Instances_Array_AS3unshift::Func, &Array::tit[12], "unshift", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Array_AS3splice::Func, &Array::tit[13], "splice", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Array_AS3sort::Func, &Array::tit[14], "sort", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Array_AS3sortOn::Func, &Array::tit[15], "sortOn", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Array_AS3indexOf::Func, &Array::tit[18], "indexOf", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Array_AS3lastIndexOf::Func, &Array::tit[21], "lastIndexOf", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[1]},
        {TFunc_Instances_Array_AS3every::Func, &Array::tit[24], "every", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[2]},
        {TFunc_Instances_Array_AS3filter::Func, &Array::tit[27], "filter", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[3]},
        {TFunc_Instances_Array_AS3forEach::Func, &Array::tit[30], "forEach", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[4]},
        {TFunc_Instances_Array_AS3map::Func, &Array::tit[33], "map", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[5]},
        {TFunc_Instances_Array_AS3some::Func, &Array::tit[36], "some", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Array::dva[6]},
    };

    Array::Array(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Array::Array()"
        SetArrayLike();
        SetTraitsType(Traits_Array);
//##protect##"InstanceTraits::Array::Array()"

    }

    void Array::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Array&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    Array::Array(ClassTraits::Traits& t)
    : Class(t)
    , CASEINSENSITIVE(1)
    , DESCENDING(2)
    , UNIQUESORT(4)
    , RETURNINDEXEDARRAY(8)
    , NUMERIC(16)
    {
//##protect##"class_::Array::Array()"
//##protect##"class_::Array::Array()"
    }
//##protect##"class_$methods"
    void Array::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    Pickable<AS3::Object> Array::MakePrototype() const
    {
        InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetClassTraits().GetInstanceTraits());
        return MakeInstance(itr);
    }

    void Array::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        // !!! Below is a correct implementation.
        //InitPrototypeFromVTable(obj);
        // !!! In the end all methods should be generic and we shouldn't check type. See above.
        InitPrototypeFromVTableCheckType(obj);
        //return GetVM().GetClassFunction().MakeCheckTypeInstance(*this, v.AsThunk());

        Classes::Function& cf = GetVM().GetClassFunction();
        SF_DEBUG_CODE(InstanceTraits::Traits& itr = GetClassTraits().GetInstanceTraits();)

        // This trick with ThunkFunction is necessary to make toString.toString()
        // return "function Function() {}".
        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
        {
            obj.AddDynamicSlotValuePair(
                GetStringManager().CreateConstString(ti[i].Name), 
                cf.MakeCheckTypeInstance(*this, ti[i] SF_DEBUG_ARG(itr)), 
                SlotInfo::aDontEnum
                );
        }

        AddConstructor(obj);
    }

    const TypeInfo* Array::tit[2] = {
        &AS3::fl::StringTI,
        &AS3::fl::StringTI,
    };

    const ThunkInfo Array::ti[2] = {
        {&Instances::fl::Array::toLocaleStringProto, &Array::tit[0], "toLocaleString", NULL, Abc::NS_Public, CT_Method, 0, 0},
        {&Instances::fl::Array::toStringProto, &Array::tit[1], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0},
    };

//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{
    const MemberInfo Array::mi[Array::MemberInfoNum] = {
        {"CASEINSENSITIVE", NULL, OFFSETOF(Classes::fl::Array, CASEINSENSITIVE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"DESCENDING", NULL, OFFSETOF(Classes::fl::Array, DESCENDING), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"UNIQUESORT", NULL, OFFSETOF(Classes::fl::Array, UNIQUESORT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"RETURNINDEXEDARRAY", NULL, OFFSETOF(Classes::fl::Array, RETURNINDEXEDARRAY), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMERIC", NULL, OFFSETOF(Classes::fl::Array, NUMERIC), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    Array::Array(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Array::Array()"
        SetTraitsType(Traits_Array);
//##protect##"ClassTraits::Array::Array()"

    }

    Pickable<Traits> Array::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Array(vm, AS3::fl::ArrayCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::ArrayCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
#if 0
    // Implementation is correct.
    bool Array::Coerce(const Value& value, Value& result) const
    {
        bool rc = true;
        VM& vm = GetVM();

        if (vm.IsOfType(value, vm.GetClassTraitsArray()))
            result = value;
        else if (!value.IsNull())
        {
            // Construct a new array from a value.
            GetInstanceTraits().GetClass().Construct(result, 1, &value);
            rc = !vm.IsException();
        }
        else
            result.SetNull();

        return rc;
    }
#endif
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo ArrayTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl::Array::InstanceType),
        0,
        ClassTraits::fl::Array::MemberInfoNum,
        InstanceTraits::fl::Array::ThunkInfoNum,
        0,
        "Array", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ArrayCI = {
        &ArrayTI,
        ClassTraits::fl::Array::MakeClassTraits,
        NULL,
        ClassTraits::fl::Array::mi,
        InstanceTraits::fl::Array::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

