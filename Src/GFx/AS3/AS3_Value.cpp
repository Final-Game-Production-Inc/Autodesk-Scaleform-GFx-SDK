/**************************************************************************

Filename    :   AS3_Value.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Value.h"
#include "AS3_VM.h"
#include "Kernel/SF_MsgFormat.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_Function.h" // Because of Value::GetWeakBase()
#include "Obj/AS3_Obj_XML.h"
#ifdef GFX_ENABLE_XML
#include "Obj/AS3_Obj_XMLList.h"
#endif
#include "Obj/AS3_Obj_QName.h"

namespace Scaleform { namespace GFx { namespace AS3
{

namespace Impl
{
    bool ConvertTO(VM& vm, Value& to, const Value& from, const TypeInfo& ti)
    {
        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(ti);

        if (ctr == NULL)
        {
            vm.ThrowReferenceError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(ti.Name)
                ));

            return false;
        }

        if (!ctr->Coerce(from, to))
        {
            // A TypeError is thrown if value cannot be coerced to the specified type.
            vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(from)
                SF_DEBUG_ARG(ctr->GetName().ToCStr())
                ));

            return false;
        }

        return true;
    }
}

///////////////////////////////////////////////////////////////////////////
Value::Value(const ASString& v)
: Flags(kString)
, value(v.GetNode())
{
    if (v.IsNull())
    {
        value = (ASStringNode*)NULL;
        // null has type Object.
        SetKind(kObject);
    }
    else
        AddRefString();
}

Value::Value(ASStringNode* v)
: Flags(kString)
, value(v)
{
    if (v)
    {
        if (v->IsNull())
        {
            value = (ASStringNode*)NULL;
            // null has type Object.
            SetKind(kObject);
        }
        else
        {
            SF_ASSERT(v->pData);
            v->AddRef();
        }
    }
    else
        // null has type Object.
        SetKind(kObject);
}

Value::Value(const Value& other, StrongRefType)
{
    SIMD::IS::PrefetchObj(&other);
    memcpy(this, &other, sizeof(Value));
    other.AddRef();

    if (IsWeakRef())
        MakeStrongRef();
}

///////////////////////////////////////////////////////////////////////////
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
Value::Value(Traits& tr, TraceNullType isNull, SNode& n)
: Flags(tr.IsInstanceTraits() ? kSNodeIT : kSNodeCT)
{
    // We do not care about the real type because both types share same pointer
    value.VS._1.ITr = static_cast<InstanceTraits::Traits*>(&tr);
    value.VS._2.VSNode = &n;

#ifdef SF_AS3_REFCOUNTED_TRAITS
    AddRefTraits();
#endif
    SetTraceNullType(isNull);
    SetWith(false);
    AddRefSNode();
}

Value::Value(InstanceTraits::Traits& tr, TraceNullType isNull, SNode& n)
: Flags(kSNodeIT)
, value(tr, n)
{
#ifdef SF_AS3_REFCOUNTED_TRAITS
    AddRefTraits();
#endif
    SetTraceNullType(isNull);
    SetWith(false);
    AddRefSNode();
}

Value::Value(ClassTraits::Traits& tr, TraceNullType isNull, SNode& n)
: Flags(kSNodeCT)
, value(tr, n)
{
#ifdef SF_AS3_REFCOUNTED_TRAITS
    AddRefTraits();
#endif
    SetTraceNullType(isNull);
    SetWith(false);
    AddRefSNode();
}

void Value::UpdateTraits(InstanceTraits::Traits& tr)
{
    switch (GetKind())
    {
    case kInstanceTraits:
        break;
    case kClassTraits:
        SetKind(kInstanceTraits);
        break;
    case kSNodeIT:
        break;
    case kSNodeCT:
        SetKind(kSNodeIT);
        break;
    default:
        SF_ASSERT(false);
        break;
    }

#ifdef SF_AS3_REFCOUNTED_TRAITS
    ReleaseTraits();
    value.VS._1.ITr = &tr;
    AddRefTraits();
#else
    value.VS._1.ITr = &tr;
#endif
}

void Value::UpdateTraits(ClassTraits::Traits& tr)
{
    switch (GetKind())
    {
    case kInstanceTraits:
        SetKind(kClassTraits);
        break;
    case kClassTraits:
        break;
    case kSNodeIT:
        SetKind(kSNodeCT);
        break;
    case kSNodeCT:
        break;
    default:
        SF_ASSERT(false);
        break;
    }

#ifdef SF_AS3_REFCOUNTED_TRAITS
    ReleaseTraits();
    value.VS._1.CTr = &tr;
    AddRefTraits();
#else
    value.VS._1.CTr = &tr;
#endif
}

void Value::UpdateSNode(SNode& n)
{
    SF_ASSERT(IsSNode());
    ReleaseSNode();
    value.VS._2.VSNode = &n;
    AddRefSNode();
}

void Value::UpdateSNode(SNode& n, TraceNullType isNull)
{
    SF_ASSERT(IsSNode());
    ReleaseSNode();
    value.VS._2.VSNode = &n;
    AddRefSNode();
    SetTraceNullType(isNull);
}
#endif

void Value::Assign(const Value& other)
{
    if (&other != this)
    {
        Release();
        SIMD::IS::PrefetchObj(&other);
        memcpy(this, &other, sizeof(Value));
        AddRef();
    }
}

void Value::AssignUnsafe(const Value& other)
{
    if (&other != this)
    {
        SF_ASSERT(!IsRefCounted());

        SIMD::IS::PrefetchObj(&other);
        memcpy(this, &other, sizeof(Value));
        AddRef();
    }
}

void Value::Assign(const ASString& v)
{
    Release();
    if (v.IsNull())
    {
        value = (ASStringNode*)NULL;
        // null has type Object.
        SetKind(kObject);
    }
    else
    {
        SetKind(kString);
        value = v.GetNode();
        AddRefString();
    }
}

void Value::AssignUnsafe(const ASString& v)
{
    SF_ASSERT(!IsRefCounted());
    if (v.IsNull())
    {
        value = (ASStringNode*)NULL;
        // null has type Object.
        SetKind(kObject);
    }
    else
    {
        SetKind(kString);
        value = v.GetNode();
        AddRefString();
    }
}

void Value::Assign(ASStringNode* v)
{
    Release();
    value = v;
    if (v)
    {
        if (v->IsNull())
        {
            value = (ASStringNode*)NULL;
            // null has type Object.
            SetKind(kObject);
        }
        else
        {
            SF_ASSERT(v->pData);

            SetKind(kString);
            AddRefString();
        }
    }
    else
        // null has type Object.
        SetKind(kObject);
}

void Value::AssignUnsafe(ASStringNode* v)
{
    SF_ASSERT(!IsRefCounted());
    value = v;
    if (v)
    {
        if (v->IsNull())
        {
            value = (ASStringNode*)NULL;
            // null has type Object.
            SetKind(kObject);
        }
        else
        {
            SF_ASSERT(v->pData);

            SetKind(kString);
            AddRefString();
        }
    }
    else
        // null has type Object.
        SetKind(kObject);
}

void Value::Swap(Value& other)
{
    const UInt32 tmpFlags = other.Flags;
    const Extra tmpBonus = other.Bonus;
    const VU tmpValue = other.value;

    other.Flags = Flags;
    other.Bonus = Bonus;
    other.value = value;

    Flags = tmpFlags;
    Bonus = tmpBonus;
    value = tmpValue;
}

void Value::Pick(Value& other)
{
    Release();
    SIMD::IS::PrefetchObj(&other);
    memcpy(this, &other, sizeof(Value));
    //other.SetKind(kUndefined);
    other.SetFlags(0);
}

void Value::PickUnsafe(Value& other)
{
    SF_ASSERT(!IsRefCounted());
    SIMD::IS::PrefetchObj(&other);
    memcpy(this, &other, sizeof(Value));
    //other.SetKind(kUndefined);
    other.SetFlags(0);
}

const Value& Value::GetUndefined()
{
    static Value v;

    return v;
}

const Value& Value::GetNull()
{
    static Value v((Object*)NULL);

    return v;
}

Value::ObjectTag Value::GetObjectTag() const
{
    switch (GetKind())
    {
    case kFunction:
        return otFunction;
    case kObject:
        return otObject;
    case kClass:
        return otClass;
    case kNamespace:
        return otNamespace;
    default:
        break;
    }

    return otInvalid;
}

bool Value::Convert2Boolean() const
{
    bool result = false;;

    switch(GetKind())
    {
    case kUndefined:
        result = false;
        break;
    case kBoolean:
        result = value;
        break;
    case kInt:
        result = value.VS._1.VInt != 0;
        break;
    case kUInt:
        result = value.VS._1.VUInt != 0;
        break;
    case kNumber:
        if (IsNaN() || IsPOSITIVE_ZERO() || IsNEGATIVE_ZERO())
            result = false;
        else
            result = true;
        break;
    case kString:
        if (value.VS._1.VStr)
            // Not null.
            result = value.VS._1.VStr->Size != 0;
        else
            // null.
            result = false;
        break;
    case kNamespace:
        result = false;
        break;
    case kFunction:
    case kObject:
    case kClass:
    case kThunkFunction:
        if (IsNull())
            result = false;
        else
            result = true;
        break;
    case kThunkClosure:
    case kVTableIndClosure:
        // The same logic as for kFunction. False in case of null.
        // Because closure is never null, we should return true.
        // In case of ThunkClosure we always have a thunk.
        result = true;
        break;
    case kThunk:
    case kVTableInd:
        result = true;
        break;
    default:
        SF_ASSERT(false);
        break;
    }

    return result;
}

CheckResult Value::Convert2Number(Number& result) const
{
    return Convert2NumberInline(result);
}

CheckResult Value::ToNumberValue()
{
    CheckResult rc = true;
    Number result;

    if (Convert2Number(result))
        SetNumber(result);
    else
        rc = false;

    return rc;
}

CheckResult Value::Convert2NumberInternal(Number& result, KindType kind) const
{
    if (kind == kString)
    {
        const ASStringNode* sn = value.VS._1.VStr;
        if (sn)
        {
            // String is not null.

            const UInt32 len = sn->Size;
            if (len)
            {
                UInt32 offset;
                const char* str = sn->pData;
                Double d = NumberUtil::StringToDouble(str, len, &offset);
                if (NumberUtil::IsNaN(d) || d == 0.0)
                {
                    if (offset == len) // should be empty string, so result is actually 0;
                    {
                        result = NumberUtil::POSITIVE_ZERO();
                        return true;
                    }
                    else
                    {
                        if (d == 0.0)
                        {
                            // Check if all following characters are white space and/or line terminators
                            // Theoretically, index should never be out of bounds.
                            UInt32 index = static_cast<UInt32>(UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(str + offset), str + offset, len - offset));
                            if (index + offset >= len)
                            {
                                result = d;
                                return true;
                            }
                        }

                        d = NumberUtil::StringToInt(str, len, 0, &offset);
                    }
                }

                // Check if all following characters are white space and/or line terminators
                // Theoretically, index should never be out of bounds.
                UInt32 index = static_cast<UInt32>(UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(str + offset), str + offset, len - offset));
                if (index + offset >= len)
                {
                    result = d;
                    return true;
                }
            }
            else
            {
                result = NumberUtil::POSITIVE_ZERO();
                return true;
            }

            result = NumberUtil::NaN();
        }
        else
            // String is null.
            result = NumberUtil::POSITIVE_ZERO();
    } else
    {
        if (IsNull())
            result = NumberUtil::POSITIVE_ZERO();
        else
        {
            Value v;

            /*
            Apply the following steps:
            1. Call ToPrimitive(input argument, hint Number).
            2. Call ToNumber(Result(1)).
            3. Return Result(2).
            */
            if (!Convert2PrimitiveValueUnsafeHintNumber(v))
                return false;

            if (!v.Convert2Number(result))
                return false;
        }
    }

    return true;
}

static 
SInt32 ConvertDouble2SInt32(double n)
{
    /*
        9.5 ToInt32: (Signed 32 Bit Integer)
        The operator ToInt32 converts its argument to one of 2^32 integer values in the range -2^31 through 2^31-1, inclusive.
        This operator functions as follows:

        1. Call ToNumber on the input argument.
        2. If Result(1) is NaN, +0, ?0, +?, or -?, return +0.
        3. Compute sign(Result(1)) * floor(abs(Result(1))).
        4. Compute Result(3) modulo 2^32; that is, a finite integer value k of Number type with positive sign and less than
            2^32 in magnitude such the mathematical difference of Result(3) and k is mathematically an integer multiple of 2^32.
        5. If Result(4) is greater than or equal to 2^31, return Result(4)? 2^32, otherwise return Result(4).
    */

    // Step 2.
    if (NumberUtil::IsNaNOrInfinity(n) || NumberUtil::IsNEGATIVE_ZERO(n) || NumberUtil::IsPOSITIVE_ZERO(n))
        return 0;

    // Step 3.
    double result = n < 0.0 ? ::floor(-n) : ::floor(n);

    // Step 4.
    // (1ULL << 32) is equal to 4294967296.0
    if (result > 4294967295.0)
        result = ::fmod(result, 4294967296.0); // result %= (1ULL << 32)

    // Step 5.
    if (result >= (double)2147483648.0)
    {
        const SInt32 tmp = SInt32(result - 2147483648.0);

        if (n < 0.0)
            return 0x80000000 - tmp;
        else
            return 0x80000000 + tmp;
    }
    else
        return SInt32(n < 0.0 ? -result : result);
}


SF_INLINE
UInt32 ConvertDouble2UInt32(double n)
{
    /*
        9.6 ToUint32: (Unsigned 32 Bit Integer)
        The operator ToUint32 converts its argument to one of 2^32 integer values in the range 0 through 2^32?1, inclusive.
        This operator functions as follows:

        1. Call ToNumber on the input argument.
        2. If Result(1) is NaN, +0, ?0, +i, or -i, return +0.
        3. Compute sign(Result(1)) * floor(abs(Result(1))).
        4. Compute Result(3) modulo 2^32; that is, a finite integer value k of Number type with positive sign and less than
            2^32 in magnitude such the mathematical difference of Result(3) and k is mathematically an integer multiple of 2^32.
        5. Return Result(4).
    */

#if 0
    static const Value::Number UINT32MAX = Value::Number(~0u);

    if (NumberUtil::IsNaNOrInfinity(n) || n == 0)
        return 0;
    if (n >= 0 && n <= UINT32MAX)
        return (UInt32)n;

    Value::Number anv = floor(Alg::Abs(n));
    UInt32 uv = (UInt32)fmod(anv, UINT32MAX + 1);
    if (n < 0) uv = 0u-uv;
    return uv;
#else
    return (UInt32)ConvertDouble2SInt32(n);
#endif
}

CheckResult Value::Convert2Int32(SInt32& result) const
{
    switch (GetKind())
    {
    case kUndefined:
    case kThunk:
    case kThunkFunction:
        result = 0;
        break;
    case kInt:
    case kVTableInd:
        result = value;
        break;
    case kBoolean:
        result = (AsBool() ? 1 : 0);
        break;
    case kUInt:
        result = value.VS._1.VUInt;
        break;

    case kNumber:
        result = ConvertDouble2SInt32(AsNumber());
        break;

    case kString:
        if (GetStringNode())
        {
            // Not null.
            Number n;
            if (!Convert2Number(n))
                return false;

            result = ConvertDouble2SInt32(n);
        }
        else
            // null.
            result = 0;
        break;

    default:
        if (IsNull())
            // null.
            result = 0;
        else
        {
            // Not null.
            Value v;

            if (!Convert2PrimitiveValueUnsafeHintNumber(v))
                return false;

            if (!v.Convert2Int32(result))
                return false;
        }
    }

    return true;
}

CheckResult Value::Convert2UInt32(UInt32& result) const
{
    switch(GetKind())
    {
    case kUndefined:
    case kThunk:
    case kThunkFunction:
        result = 0;
        break;
    case kUInt:
        result = value.operator UInt32();
        break;
    case kInt:
        result = static_cast<UInt32>(AsInt());
        break;
    case kBoolean:
        result = (AsBool() ? 1 : 0);
        break;

    case kNumber:
        result = ConvertDouble2UInt32(AsNumber());

        break;
    case kString:
        if (GetStringNode())
        {
            // Not null.
            Number n;
            
            if (!Convert2Number(n))
                return false;

            result = ConvertDouble2UInt32(n);
        }
        else
            // null.
            result = 0;

        break;
    default:
        if (IsNull())
            result = 0;
        else
        {
            Value v;

            if (!Convert2PrimitiveValueUnsafeHintNumber(v))
                return false;

            if (!v.Convert2UInt32(result))
                return false;
        }
    }

    return true;
}

// Current implementation of double formatting, approximating ECMA 262-5
// section 9.8.1 (ToString rules). TBD: Move into utilities file.

// Shouldn't be static. Used in AS3_Obj_Number.
UPInt SF_ECMA_dtostr(char* buffer, int bufflen, Double val)
{   
    // Use fast code path for integers.
    int intVal = static_cast<int>(val);
    if (val == intVal)
    {
        SFitoa(intVal, buffer, bufflen, 10);
        return SFstrlen(buffer);
    }

    SF_ASSERT(bufflen >= 40);

    // Check for NaN/Infinity and format.
    if (NumberUtil::IsNaN(val))
    {
        memcpy(buffer, "NaN", 4);
        return 3;
    }
    if (NumberUtil::IsPOSITIVE_INFINITY(val))
    {
        memcpy(buffer, "Infinity", sizeof("Infinity"));
        return sizeof("Infinity")-1;
    }
    if (NumberUtil::IsNEGATIVE_INFINITY(val))
    {
        memcpy(buffer, "-Infinity", sizeof("-Infinity"));
        return sizeof("-Infinity")-1;
    }    

    char  temp[40] = { 0 };
    char  *d, *s;
    // For positive-exponent values, ECMA 9.8.1 (ToString) rules require
    // 21 digits of precision; this doesn't seem to apply for negative
    // exponent values. Limit negative exponent value to 16 digits, since
    // that is the maximum value precisely represented by double.
    // Higher then 16 decimal value can result in false representations such
    // as "0.0012345" printed as "0.001234499999999".
    char   format[] = "%.16g";
    Double absVal = fabs(val);

    if ((absVal >= 1.0e16) && (absVal < 1.0e21))
    {
        format[3] = '7';
        if (absVal >= 1.0e17)
        {
            format[3] = '8';
            if (absVal >= 1.0e18)
            {
                format[3] = '9';
                if (absVal >= 1.0e19)
                {
                    format[2] = '2';
                    format[3] = '0';
                    if (absVal >= 1.0e20)
                    {
                        format[2] = '2';
                        format[3] = '1';
                    }
                }
            }
        }
    }

    const int nc1 = (int)SFsprintf(temp, sizeof(temp), format, static_cast<double>(val));

    // Get rid of a trailing number.
    {
        // Check for exponent part.
        bool have_e = false;
        for (int i = 0; i < nc1; ++i)
        {
            if (temp[i] == 'e')
            {
                have_e = true;
                break;
            }
        }

        if (!have_e)
        {
            int dot_pos = 0;
            bool have_dot = false;

            // Look for dot.
            for (; temp[dot_pos] != 0; ++dot_pos)
            {
                char c = temp[dot_pos];
                if (c == '.' || c == ',')
                {
                    have_dot = true;
                    break;
                }
            }

            if (have_dot && temp[nc1 - 1] != '0')
            {
                int i = nc1 - 2;
                for (; i > dot_pos; --i)
                {
                    if (temp[i] != '0')
                        break;
                }

                // We must have at least one not zero digit after a dot.
                if (i < nc1 - 2 && i != dot_pos)
                    temp[i + 1] = 0;
            }
        }
    }

    // This code patches the resulting string to match ECMA:
    //  1) "," -> "."; possible due to locale settings
    //  2) "e-005" -> "e-5"; no leading exponent zeros generated by sprintf
    //  3) "1.2345E-006" -> "0.0000012345"; Up to 6 leading zeros are needed

    bool negativeExponent = false;
    int  exponentValue = 0;

    for (d = buffer, s = temp; *s != 0; s++)
    {
        if (*s == ',')
        {
            *d++ = '.';
            continue;
        }

        if (*s == 'e')
        {
            // Exponent, read it.
            *d++ = *s++;

            // Read sign
            if (*s == '-' || *s == '+')
            {
                if (*s == '-')
                    negativeExponent = true;
                *d++ = *s++;
            }

            // Skip 0s of exponent, then copy it
            while (*s == '0')
                s++;

            // Copy exponent.
            while((*s >='0') && (*s <= '9'))
            {
                exponentValue = exponentValue * 10 + (*s - '0');
                *d++ = *s++;
            }
            break;
        }

        *d++ = *s;
    }

    // See if we need to re-format small negative exponent with forward-0s
    // to match ECMA 262 section 9.8.1 item point 7, as follows:    
    // Given, s * 10 ^ (n-k), where k is the number of decimal digits
    // ... 
    // 7. "If 0 < n <= 21, return the String consisting of the most significant n digits of
    //    the decimal representation of s, followed by a decimal point '.', followed by the
    //    remaining k-n digits of the decimal representation of s.
    // 8. If -6 < n <= 0, return the String consisting of the character '0', followed by the
    //    decimal point '.', followed by -n occurrences

    if (negativeExponent && (exponentValue > 0) && (exponentValue <= 6))
    {
        // Start over, re-scanning string & re-assigning digits
        s = temp; d = buffer;
        if (*s == '-')
        {
            s++;
            d++;
        }

        // We should start with non-zero digit and a dot. This check is probably 
        // not necessary since exponents won't have multiple digits in front, but do it
        // for safety it just in case we have some strange printf implementation...
        if ((*s >= '1') && (*s <= '9') && (d[1] == '.'))
        {
            // Insert 0s
            *d++ = '0';
            *d++ = '.';
            for (int zeroCount = 1; zeroCount < exponentValue; zeroCount++)
                *d++ = '0';

            // Now, copy available digits while skipping '.'.
            while(*s != 'e')
            {
                SF_ASSERT(*s != 0);
                if ((*s >= '0') && (*s <= '9'))
                {
                    *d++ = *s;
                }
                s++;
            }
        }
    }

    // Terminate string.
    *d = 0;    
    return d - buffer;
}

CheckResult Value::Convert2String(ASString& result) const
{
    ASStringManager* sm = result.GetManager();

    switch(GetKind())
    {
    case kUndefined:
        result.AssignNode(sm->CreateConstStringNode("undefined", 9, 0));
        break;
    case kThunk:
    case kThunkClosure:
    case kVTableInd:
    case kVTableIndClosure:
        result.AssignNode(sm->CreateConstStringNode("function Function() {}", 22, 0));
        break;
    case kBoolean:
        {
            const bool b = value.VS._1.VBool;
            result.AssignNode(sm->CreateConstStringNode(b ? "true" : "false", b ? 4 : 5, 0));
        }
        break;
    case kInt:
        result = sm->CreateString(Scaleform::AsString(operator SInt32()));
        break;
    case kUInt:
        result = sm->CreateString(Scaleform::AsString(operator UInt32()));
        break;
    case kNumber:
        {
            char  buffer[40];
            UPInt length = SF_ECMA_dtostr(buffer, sizeof(buffer), value);
            result.AssignNode(sm->CreateStringNode(buffer, length));
        }      
        break;
    case kNamespace:
        result = AsNamespace().GetUri();
        break;
    case kFunction:
    case kClass:
    case kObject:
    case kThunkFunction:
        if (IsNull())
            result.AssignNode(sm->CreateConstStringNode("null", 4, 0));
        else
        {
            Value v;

            /*
            Apply the following steps:
            Call ToPrimitive(input argument, hint String).
            Call ToString(Result(1)).
            Return Result(2).
            */
            if (!Convert2PrimitiveValueUnsafe(*sm, v, hintString))
                return false;

            if (!v.IsPrimitive())
            {
                VM& vm = GetObject()->GetVM();

                vm.ThrowTypeError(VM::Error(VM::eConvertToPrimitiveError, vm
                    SF_DEBUG_ARG(vm.GetValueTraits(v).GetName().ToCStr())
                    ));

                return false;
            }

            if (!v.Convert2String(result))
                return false;
        }

        break;
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeIT:
    case Value::kSNodeCT:
#endif
    case kInstanceTraits:
    case kClassTraits:
        result.AssignNode(sm->GetEmptyStringNode());
        break;
    case kString:
        if (value.VS._1.VStr)
            // Not null.
            result.AssignNode(value.VS._1.VStr);
        else
            // Null.
            result.AssignNode(sm->CreateConstStringNode("null", 4, 0));

        break;

    case kMethodInd:
        break;
    }

    return true;
}

CheckResult Value::Convert2String(StringBuffer& result) const
{
    switch(GetKind())
    {
    case kUndefined:
        result += "undefined";
        break;
    case kThunk:
    case kThunkClosure:
    case kVTableInd:
    case kVTableIndClosure:
        result += "function Function() {}";
        break;
    case kBoolean:
        result += (value.VS._1.VBool ? "true" : "false");
        break;
    case kInt:
        result += Scaleform::AsString(operator SInt32());
        break;
    case kUInt:
        result += Scaleform::AsString(operator UInt32());
        break;
    case kNumber:
        {
            char  buffer[40];
            UPInt length = SF_ECMA_dtostr(buffer, sizeof(buffer), value);
            result.AppendString(buffer, length);
        }      
        break;
    case kNamespace:
        result += AsNamespace().GetUri().ToCStr();
        break;
    case kFunction:
    case kClass:
    case kObject:
    case kThunkFunction:
        if (IsNull())
            result += "null";
        else
        {
            Value v;

            /*
            Apply the following steps:
            Call ToPrimitive(input argument, hint String).
            Call ToString(Result(1)).
            Return Result(2).
            */
            if (!Convert2PrimitiveValueUnsafe(*GetObject()->GetVM().GetStringManager().GetStringManager(), v, hintString))
                return false;

            if (!v.Convert2String(result))
                return false;
        }

        break;
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case kSNodeIT:
    case kSNodeCT:
#endif
    case kInstanceTraits:
    case kClassTraits:
        break;
    case kString:
        {
            ASStringNode* snode = value.VS._1.VStr;

            if (snode)
                result.AppendString(snode->pData, snode->Size);
            else
                result += "null";
        }
        break;
    default:
        SF_ASSERT(false);
        break;
    }

    return true;
}

GASRefCountBase** Value::AsGASRefCountBasePtrPtr() const
{
    switch (GetKind())
    {
    case kFunction:
    case kThunkFunction:
    case kObject:
    case kClass:
    case kNamespace:
        // Let's cheat on constness.
        return (GASRefCountBase**)(&value.VS._1.VObj);
    default:
        SF_ASSERT(false);
        break;
    }

    SF_ASSERT(false);
    return NULL;
}

GASRefCountBase** Value::AsClosurePtrPtr() const
{
    SF_ASSERT(IsClosure() || IsVTableIndClosure());
    // Let's cheat on constness.
    return (GASRefCountBase**)(&value.VS._2.VObj);
}

CheckResult Value::ToInt32Value()
{
    bool rc = true;
    SInt32 r;

    if (Convert2Int32(r))
        SetSInt32(r);
    else
        rc = false;

    return rc;
}

CheckResult Value::ToUInt32Value()
{
    CheckResult rc = true;
    UInt32 r;

    if (Convert2UInt32(r))
        SetUInt32(r);
    else
        rc = false;

    return rc;
}

CheckResult Value::ToStringValue(StringManager& sm)
{
    CheckResult rc = true;
    ASString r = sm.CreateEmptyString();

    if (Convert2String(r))
        *this = r;
    else
        rc = false;

    return rc;
}

GASRefCountBase* Value::GetWeakBase() const
{
    GASRefCountBase* result = NULL;

    switch(GetKind())
    {
    case kNamespace:
        result = value.VS._1.VNs;

        break;
    case kClass:
    case kFunction:
    case kThunkFunction:
    case kObject:
        // Object.
        result = value.VS._1.VObj;

        break;
    case kThunkClosure:
    case kVTableIndClosure:
        // Object.
        result = value.VS._2.VObj;

        break;
    default:
        SF_ASSERT(false);
        break;
    }

    return result;
}

bool Value::MakeWeakRef()
{
    bool rc = false;

    if (!IsWeakRef() && IsGarbageCollectable())
    {
        GASRefCountBase* wb = GetWeakBase();

        if (wb)
        {
            Bonus.pWeakProxy = wb->CreateWeakProxy().GetPtr();
            SetWeakRef();
            ReleaseInternal();
            rc = true;
        }
    }

    return rc;
}

bool Value::MakeStrongRef()
{
    bool rc = false;

    if (IsWeakRef())
    {
        if (IsValidWeakRef())
        {
            AddRefInternal();
            Bonus.pWeakProxy->Release();
            Bonus.pWeakProxy = NULL;
        }
        else
        {
            // Clean up for the safety sake.
            Bonus.pWeakProxy = NULL;
            value.VS._1.VObj = NULL;
            value.VS._2.VObj = NULL;

            SetKind(kUndefined);
        }

        SetWeakRef(false);

        rc = true;
    }

    return rc;
}

bool Value::IsValidWeakRef() const
{
    return IsWeakRef() ? Bonus.pWeakProxy->IsAlive() : true;
}

///////////////////////////////////////////////////////////////////////////
UPInt Value::HashFunctor::SDBM_Hash(const void* data_in, UPInt size, UPInt seed)     
{
    const UByte* data = (const UByte*) data_in;
    UPInt        h = seed;
    while (size > 0)
    {
        size--;
        h = (h << 16) + (h << 6) - h + (UPInt)data[size];
    }   
    return h;
}

UPInt Value::HashFunctor::operator()(const Value& v) const
{
    const Value::KindType kind = v.GetKind();
    UPInt result = 0;

    switch (kind)
    {
    case Value::kUndefined:
        break;
    case Value::kBoolean:
        result = v.AsBool() ? 1 : 0;
        break;
    case Value::kInt:
    case Value::kUInt:
        {
            SInt32 lv = v.AsInt();
            result = SDBM_Hash(&lv, sizeof(lv));
        }
        break;
    case Value::kNumber:
        {
            Value::Number lv = v.AsNumber();
            result = SDBM_Hash(&lv, sizeof(lv));
        }
        break;
    case Value::kString:
        if (v.GetStringNode())
            result = v.GetStringNode()->GetHashCode();
        else
            result = 0;
        break;
    case Value::kVTableInd:
        {
            SInt32 lv = v.GetVTableInd();
            const Traits* tr = &v.GetTraits();
            result = SDBM_Hash(&lv, sizeof(lv)) ^ SDBM_Hash(&tr, sizeof(tr));
        }
        break;
    case Value::kVTableIndClosure:
        {
            SInt32 lv = v.GetVTableInd();
            const Object* cv = v.GetClosure();
            result = SDBM_Hash(&lv, sizeof(lv)) ^ SDBM_Hash(&cv, sizeof(cv));
        }
        break;
    case Value::kThunk:
        /* New version
        {
            const ThunkInfo* lv = &v.AsThunk();
            result = SDBM_Hash(&lv, sizeof(lv));
        }
        */
        // Old version.
        result = SDBM_Hash(&v.AsThunk(), sizeof(ThunkInfo));
        break;
    case Value::kFunction:
    case Value::kObject:
    case Value::kClass:
    case Value::kNamespace:
    case Value::kThunkFunction:
        if (v.GetObject())
        {
            const Object* lv = v.GetObject();
            result = SDBM_Hash(&lv, sizeof(lv));
        }
        else
            result = 0;
        break;
    case Value::kThunkClosure:
        /* New version.
        {
            const ThunkInfo* lv = &v.AsThunk();
            const Object* cv = v.GetClosure();
            result = SDBM_Hash(&lv, sizeof(lv)) ^ SDBM_Hash(&cv, sizeof(cv));
        }
        */
        // Old version.
        {
            const Object* cv = v.GetClosure();
            result = SDBM_Hash(&v.AsThunk(), sizeof(ThunkInfo)) ^ SDBM_Hash(&cv, sizeof(cv));
        }
        break;
    default:
        SF_ASSERT(false);
        break;
    }

    result += kind;
    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult AbstractLessThan(Boolean3& result, const Value& l, const Value& r)
{
    Value _1;
    Value _2;

    if (!l.Convert2PrimitiveValueUnsafeHintNumber(_1))
        return false;

    if (!r.Convert2PrimitiveValueUnsafeHintNumber(_2))
        return false;

    Value::KindType _1k = _1.GetKind();
    Value::KindType _2k = _2.GetKind();

    if (_1k == Value::kInt && _2k == Value::kInt)
    {
        result = _1.AsInt() < _2.AsInt() ? true3 : false3;
        return true;
    }

    if (_1k == Value::kUInt && _2k == Value::kUInt)
    {
        result = _1.AsUInt() < _2.AsUInt() ? true3 : false3;
        return true;
    }

    if (_1k == Value::kString && _2k == Value::kString &&
        _1.GetStringNode() != NULL && _2.AsStringNode() != NULL)
    {
        const ASString str1 = _1;
        const ASString str2 = _2;

        // Our ASString < operator matches ECMA-262 section 11.8.5.16-20.
        // We don't want to use strcmp() here since it improperly handles \0s in string.
        result = str1 < str2 ? true3 : false3;
        return true;
    }

    // (ECMA-262 section 11.8.5.4)
    if (!_1.ToNumberValueInline())
        return false;
    // (ECMA-262 section 11.8.5.5)
    if (!_2.ToNumberValueInline())
        return false;

    // (ECMA-262 section 11.8.5.6 - 7)
    if (_1.IsNaN() || _2.IsNaN())
    {
        result = undefined3;
        return true;
    }

    result = _1.AsNumber() < _2.AsNumber() ? true3 : false3;

    return true;
}

///////////////////////////////////////////////////////////////////////////
bool StrictEqual(const Value& x, const Value& y)
{
    // (ECMA-262 section 11.9.6.1)
    if (x.GetKind() != y.GetKind())
    {
        if (x.IsNumeric() && y.IsNumeric())
        {
            if (x.IsUIntStrict())
            {
                switch (y.GetKind())
                {
                case Value::kInt:
                    if (y.AsInt() < 0)
                        return false;

                    return x.AsUInt() == static_cast<UInt32>(y.AsInt());
                case Value::kNumber:
                    return x.AsUInt() == y.AsNumber();
                default:
                    break;
                }
            }
            else if(x.IsIntStrict())
            {
                switch (y.GetKind())
                {
                case Value::kUInt:
                    if (x.AsInt() < 0)
                        return false;

                    return static_cast<UInt32>(x.AsInt()) == y.AsUInt();
                case Value::kNumber:
                    return x.AsInt() == y.AsNumber();
                default:
                    break;
                }
            }
            else if(x.IsNumber())
            {
                switch (y.GetKind())
                {
                case Value::kInt:
                    return x.AsNumber() == y.AsInt();
                case Value::kUInt:
                    return x.AsNumber() == y.AsUInt();
                default:
                    break;
                }
            }
        } 
        else
            return false;
    }

    switch(x.GetKind())
    {
    case Value::kUndefined:
        // (ECMA-262 section 11.9.6.2)
        return true;
    case Value::kBoolean:
        // (ECMA-262 section 11.9.6.12)
        return x.AsBool() == y.AsBool();
    case Value::kInt:
        return x.AsInt() == y.AsInt();
    case Value::kUInt:
        return x.AsUInt() == y.AsUInt();
    case Value::kNumber:
        return x.AsNumber() == y.AsNumber();
    case Value::kString:
        // (ECMA-262 section 11.9.6.11)
        return x.AsStringNode() == y.AsStringNode();
    case Value::kNamespace:
        return &x.AsNamespace() == &y.AsNamespace();
    case Value::kFunction:
    case Value::kObject:
    case Value::kClass:
    case Value::kThunkFunction:
#if 0
        // (ECMA-262 section 11.9.6.3)
        if (x.IsNull() || y.IsNull())
            // This is strange.
            return true;
#endif

        if (x.GetObject() == y.GetObject())
            return true;

        break;
    case Value::kThunkClosure:
        if (x.GetClosure() == y.GetClosure())
        {
            if (&x.AsThunk() == &y.AsThunk())
                return true;
        }

        break;
    case Value::kVTableIndClosure:
        if (x.GetClosure() == y.GetClosure())
        {
            if (x.GetVTableInd() == y.GetVTableInd())
                return true;
        }

        break;
    case Value::kVTableInd:
        return x.GetVTableInd() == y.GetVTableInd() && &x.GetTraits() == &y.GetTraits();
    case Value::kThunk:
        return &x.AsThunk() == &y.AsThunk();
    default:
        SF_ASSERT(false);
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
bool IsQNameObject(const Value& v)
{
    bool result = false;

    if (v.IsObject() && v.GetObject())
    {
        Traits& tr = v.GetObject()->GetTraits();

        if (tr.GetTraitsType() == Traits_QName && tr.IsInstanceTraits())
            result = true;
    }

    return result;
}

bool IsXMLObject(Object* obj)
{
    bool result = false;

    if (obj)
    {
        Traits& tr = obj->GetTraits();

        if (tr.GetTraitsType() == Traits_XML && tr.IsInstanceTraits())
            result = true;
    }

    return result;
}

bool IsXMLListObject(Object* obj)
{
    bool result = false;

    if (obj)
    {
        Traits& tr = obj->GetTraits();

        if (tr.GetTraitsType() == Traits_XMLList && tr.IsInstanceTraits())
            result = true;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult AbstractEqual(bool& result, const Value& l, const Value& r)
{
#ifdef GFX_ENABLE_XML
    // See ECMA-357 11.5.1.
    // Step 1
    if (IsXMLListObject(l))
    {
        Object* obj = l.GetObject();
        const XMLSupport& xmls = obj->GetVM().GetXMLSupport();

        return xmls.EqualsXMLList(result, static_cast<Instances::fl::XMLList&>(*obj), r);
    }
    // Step 2
    else if (IsXMLListObject(r))
    {
        Object* obj = r.GetObject();
        const XMLSupport& xmls = obj->GetVM().GetXMLSupport();

        return xmls.EqualsXMLList(result, static_cast<Instances::fl::XMLList&>(*obj), l);
    }
#endif

    Value::KindType lk = l.GetKind();
    Value::KindType rk = r.GetKind();

    // Normalize kObject, kClass, kFunction, and kThunkFunction.
    // They all get converted to kObject.
    {
        if (l.IsObject())
            lk = Value::kObject;

        if (r.IsObject())
            rk = Value::kObject;
    }

    if (lk == rk)
    {
        switch(lk)
        {
        case Value::kUndefined:
            result = true;
            return true;
        case Value::kNumber:
            result = l.AsNumber() == r.AsNumber();
            return true;
        case Value::kString:
            result = l.AsStringNode() == r.AsStringNode();
            return true;
        case Value::kNamespace:
            {
                Instances::fl::Namespace* ln = l.GetNamespace();
                Instances::fl::Namespace* rn = r.GetNamespace();

                if (ln == NULL || rn == NULL)
                    result = false;
                else
                    result = (*ln == *rn);
            }
            return true;
        case Value::kBoolean:
            {
                /* For some reason this doesn't work with MSVC90. true == true returns false ...
                const bool lv = l.AsBool();
                const bool rv = r.AsBool();
                const bool res(lv == rv);
                */
                const int lv = static_cast<int>(l.AsBool());
                const int rv = static_cast<int>(r.AsBool());
                const bool res((lv == 0 && rv == 0) || (lv != 0 && rv != 0));
                result = res;
            }
            return true;
        case Value::kInt:
            result = l.AsInt() == r.AsInt();
            return true;
        case Value::kUInt:
            result = l.AsUInt() == r.AsUInt();
            return true;
        case Value::kThunk:
            result = &l.AsThunk() == &r.AsThunk();
            return true;
        case Value::kVTableInd:
            result = l.GetVTableInd() == r.GetVTableInd() && &l.GetTraits() == &r.GetTraits();
            return true;
        case Value::kThunkClosure:
            result = l.GetClosure() == r.GetClosure() && &l.AsThunk() == &r.AsThunk();
            return true;
        case Value::kVTableIndClosure:
            result = l.GetClosure() == r.GetClosure() && l.GetVTableInd() == r.GetVTableInd();
            return true;
        default:
            break;
        }

        // Object.
        result = (l.GetObject() == r.GetObject());

#ifdef GFX_ENABLE_XML
        if (result == false)
        {
            //  ECMA-357 11.5.1.
            //  3.a.
            if (IsXMLObject(l) && IsXMLObject(r))
            {
                Object* obj = r.GetObject();
                const XMLSupport& xmls = obj->GetVM().GetXMLSupport();

                Instances::fl::XML& lxml = static_cast<Instances::fl::XML&>(*l.GetObject());
                Instances::fl::XML& rxml = static_cast<Instances::fl::XML&>(*r.GetObject());

                return xmls.EqualsXML(result, lxml, rxml);
            }
            //  ECMA-357 11.5.1.
            //  3.b.
            else if (IsQNameObject(l) && IsQNameObject(r))
            {
                Object* obj = r.GetObject();
                const XMLSupport& xmls = obj->GetVM().GetXMLSupport();

                Instances::fl::QName& lqn = static_cast<Instances::fl::QName&>(*l.GetObject());
                Instances::fl::QName& rqn = static_cast<Instances::fl::QName&>(*r.GetObject());

                return xmls.EqualsQName(result, lqn, rqn);
            }
        }
#endif
    }
    else
    {
#ifdef GFX_ENABLE_XML
        // XML (ECMA-357 11.5.1 step 4)
        if (IsXMLObject(l))
        {
            Object* obj = l.GetObject();
            const XMLSupport& xmls = obj->GetVM().GetXMLSupport();
            Instances::fl::XML& lxml = static_cast<Instances::fl::XML&>(*obj);
            bool stop = true;

            if (!xmls.EqualsXML(stop, result, lxml, r))
                // Exception
                return false;

            if (stop)
                return true;
        }
        else if (IsXMLObject(r))
        {
            Object* obj = r.GetObject();
            const XMLSupport& xmls = obj->GetVM().GetXMLSupport();
            Instances::fl::XML& rxml = static_cast<Instances::fl::XML&>(*obj);
            bool stop = true;

            if (!xmls.EqualsXML(stop, result, rxml, l))
                // Exception
                return false;

            if (stop)
                return true;
        }
#endif

        const bool r_IsNull = r.IsNull();
        const bool l_IsNull = l.IsNull();

        // Combination: Undefined + Null (ECMA-357 11.5.1 steps 5 and 6)
        // (ECMA-262 11.9.3 steps 2 and 3)
        // Actually, this is a more generic case.
        // Combination: Undefined + Object
        {
            if (Value::IsUndefined(lk) && Value::IsObject(rk))
            {
                result = r_IsNull;
                return true;
            }

            if (Value::IsObject(lk) && Value::IsUndefined(rk))
            {
                result = l_IsNull;
                return true;
            }
        }

        // Combination: Number + String (ECMA-357 11.5.1 steps 7 and 8)
        // (ECMA-262 11.9.3 steps 4 and 5)
        {
            if (Value::IsNumber(lk) && Value::IsString(rk))
            {
                Value::Number v;
                if (!r.Convert2NumberInline(v))
                    // Exception.
                    return false;

                result = (l.AsNumber() == v);
                return true;
            }

            if (Value::IsString(lk) && Value::IsNumber(rk))
            {
                Value::Number v;
                if (!l.Convert2NumberInline(v))
                    // Exception.
                    return false;

                result = (v == r.AsNumber());
                return true;
            }
        }

        // Combination: Boolean/int/uint + something else.
        // (ECMA-262 11.9.3 steps 6 and 7)
        // Also handles kInt and kUInt.
        {
            if (Value::IsBool(lk) || Value::IsInt(lk))
            {
                Value::Number v;
                if (!l.Convert2NumberInline(v))
                    return false;

                return AbstractEqual(result, Value(v), r);
            }

            if (Value::IsBool(rk) || Value::IsInt(rk))
            {
                Value::Number v;
                if (!r.Convert2NumberInline(v))
                    return false;

                return AbstractEqual(result, l, Value(v));
            }
        }

        // Combination: Number/String + Object
        // (ECMA-262 11.9.3 steps 8 and 9)
        {
            if ((Value::IsString(lk) || Value::IsNumber(lk)) && Value::IsObject(rk))
            {
                if (r_IsNull)
                {
                    result = false;
                    return true;
                }

                Value v;
                if (!r.Convert2PrimitiveValueUnsafe(*r.GetObject()->GetStringManager().GetStringManager(), v))
                    // Exception.
                    return false;

                return AbstractEqual(result, l, v);
            }

            if (Value::IsObject(lk) && (Value::IsString(rk) || Value::IsNumber(rk)))
            {
                if (l_IsNull)
                {
                    result = false;
                    return true;
                }

                Value v;
                if (!l.Convert2PrimitiveValueUnsafe(*l.GetObject()->GetStringManager().GetStringManager(), v))
                    // Exception.
                    return false;

                return AbstractEqual(result, v, r);
            }
        }

        // (ECMA-262 11.9.3 step 10)
        result = false;
    }

    // No exceptions so far.
    return true;
}

Value GetAbsObject(UPInt addr)
{
    Value result;
    Value::ObjectTag tag = GetObjectTag(addr);

    addr -= tag;
    switch (tag)
    {
    case Value::otObject:
        result.AssignUnsafe(reinterpret_cast<Object*>(addr));
        break;
    case Value::otClass:
        result.AssignUnsafe(reinterpret_cast<Class*>(addr));
        break;
    case Value::otFunction:
        result.AssignUnsafe(reinterpret_cast<Instances::Function*>(addr));
        break;
    case Value::otNamespace:
        result.AssignUnsafe(reinterpret_cast<Instances::fl::Namespace*>(addr));
        break;
    default:
        break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
void STPtr::AddRef() const
{
    if (pObject)
        GetRawAddr()->AddRef();
}

void STPtr::Release() const
{
    if (pObject)
    {
        ObjType* addr = GetAddrNoTag();

        if ((Scaleform::UPInt)addr & 1)
        {
            Value::ObjectTag tag = GetObjectTag((Scaleform::UPInt)pObject);
            pObject = (ObjType*)((Scaleform::UPInt)addr - 1 + tag);
        }
        else
            addr->Release();
    }
}

void STPtr::GetValue(Value& v)
{
    if (pObject)
    {
        Value::ObjectTag tag = GetObjectTag((Scaleform::UPInt)pObject);
        ObjType* addr = GetRawAddr();

        switch (tag)
        {
        case Value::otObject:
            v.Assign((Object*)addr);
            return;
        case Value::otClass:
            v.Assign((Class*)addr);
            return;
        case Value::otFunction:
            v.Assign((Instances::Function*)addr);
            return;
        case Value::otNamespace:
            v.Assign((Instances::fl::Namespace*)addr);
            return;
        default:
            SF_ASSERT(false);
            break;
        }
    }

    v.SetNull();
}

void STPtr::GetValueUnsafe(Value& v)
{
    if (pObject)
    {
        Value::ObjectTag tag = GetObjectTag((Scaleform::UPInt)pObject);
        ObjType* addr = GetRawAddr();

        switch (tag)
        {
        case Value::otObject:
            v.AssignUnsafe((Object*)addr);
            return;
        case Value::otClass:
            v.AssignUnsafe((Class*)addr);
            return;
        case Value::otFunction:
            v.AssignUnsafe((Instances::Function*)addr);
            return;
        case Value::otNamespace:
            v.AssignUnsafe((Instances::fl::Namespace*)addr);
            return;
        default:
            SF_ASSERT(false);
            break;
        }
    }

    v.SetNull();
}

STPtr::SelfType& STPtr::SetValue(const Value& v)
{
    ObjType* addr = MakeAddr(v);

    if (addr != pObject)
    {
        Release();
        pObject = addr;
        AddRef();
    }

    return *this;
}

STPtr::SelfType& STPtr::Set(const STPtr::SelfType& other)
{
    if (&other != this)
    {
        Release();
        other.AddRef();

        pObject = other.pObject;
    }

    return *this;
}

STPtr::ObjType* STPtr::MakeAddr(const Value& v)
{
    ObjType* addr = NULL;

    if (v.IsObject() || v.IsNamespace())
    {
        GASRefCountBase* vv = v.GetGASRefCountBase();

        if (vv)
            addr = (ObjType*)((UPInt)vv | v.GetObjectTag());
    }

    return addr;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

