/**************************************************************************

PublicHeader:   None
Filename    :   GFx_Types.h
Content     :   General GFx Types.
Created     :   August 1, 2010
Authors     :   Artem, Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_TYPES_H
#define INC_SF_GFX_TYPES_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Stats.h"
#include "Kernel/SF_Alg.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_Event.h"
#include "GFx/GFx_ASString.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Color.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Matrix3x4.h"
#include "Render/Render_Matrix4x4.h"
#include "Render/Render_Twips.h"
#include "Render/Render_CxForm.h"

#define GFX_UNLOAD_TRACE_ENABLED 1

namespace Scaleform { 

namespace Render {
    class GCompoundShape;
    class GRasterizer;
}

namespace GFx {

using Render::Cxform;
using Render::Color;
using Render::Rect;
using Render::Point;
using Render::RectF;
using Render::SizeF;
using Render::Size;
using Render::PointF;
using Render::Matrix2F;
using Render::Matrix3F;
using Render::Matrix4F;
using Render::TwipsToPixels;
using Render::PixelsToTwips;
using Render::GCompoundShape;
using Render::GRasterizer;

// 3-way bool type - Undefined, True, False states
class Bool3W
{
    enum
    {
        B3W_Undefined, B3W_True, B3W_False
    };
    UByte Value;
public:
    inline Bool3W() : Value(B3W_Undefined) {}
    inline Bool3W(bool v) : Value(UByte((v)?B3W_True:B3W_False)) {}
    inline Bool3W(const Bool3W& v): Value(v.Value) {}

    inline Bool3W& operator=(const Bool3W& o)
    {
        Value = o.Value;
        return *this;
    }
    inline Bool3W& operator=(bool o)
    {
        Value = UByte((o) ? B3W_True : B3W_False);
        return *this;
    }
    inline bool operator==(const Bool3W& o) const
    {
        return (Value == B3W_Undefined || o.Value == B3W_Undefined) ? false : Value == o.Value;
    }
    inline bool operator==(bool o) const
    {
        return (Value == B3W_Undefined) ? false : Value == ((o) ? B3W_True : B3W_False);
    }
    inline bool operator!=(const Bool3W& o) const
    {
        return (Value == B3W_Undefined || o.Value == B3W_Undefined) ? false : Value != o.Value;
    }
    inline bool operator!=(bool o) const
    {
        return (Value == B3W_Undefined) ? false : Value != ((o) ? B3W_True : B3W_False);
    }
    inline bool IsDefined() const { return Value != B3W_Undefined; }
    inline bool IsTrue() const    { return Value == B3W_True; }
    inline bool IsFalse() const   { return Value == B3W_False; }
};

enum PlayState
{
    State_Playing,
    State_Stopped
};

// Used to indicate how focus was transfered
enum FocusMovedType
{
    GFx_FocusMovedByMouse     = 1,
    GFx_FocusMovedByKeyboard  = 2,
    GFx_FocusMovedByAS        = 3
};

// Utility functions
template <UInt32 mask>
inline void G_SetFlag(UInt32& pvalue, bool state)
{
    (state) ? (pvalue |= mask) : (pvalue &= (~mask));
}


template <UInt32 mask>
inline bool G_IsFlagSet(UInt32 value)
{
    return (value & mask) != 0;
}


// set 3 way flag: 0 - not set, -1 - false, 1 - true
template <UInt32 shift>
inline void G_Set3WayFlag(UInt32& pvalue, int state)
{
    pvalue = (pvalue & (~(3U<<shift))) | UInt32((state & 3) << shift);
}

template <UInt32 shift>
inline int G_Get3WayFlag(UInt32 value)
{
    UInt32 v = ((value >> shift) & 3);
    return (v == 3) ? -1 : int(v);
}

template <UInt32 shift>
inline bool G_Is3WayFlagTrue(UInt32 value)
{
    return G_Get3WayFlag<shift>(value) == 1;
}

template <UInt32 shift>
inline bool G_Is3WayFlagSet(UInt32 value)
{
    return G_Get3WayFlag<shift>(value) != 0;
}

template <UInt32 shift>
inline bool G_Is3WayFlagFalse(UInt32 value)
{
    return G_Get3WayFlag<shift>(value) != 1;
}

///////////////////////////////////////////////////////////////////////////
#if defined(SF_SAFE_DOUBLE)

// Forward declaration.
struct Double;

namespace AS3 {
    class Value;
}

namespace NumberUtil {
//     bool SF_STDCALL IsNaN(Double v);
    GFx::Double SF_CDECL NaN();
}

struct Double
{
    typedef double ValueType;

    //
    Double() : value() {}
#ifdef SF_CC_MSVC
    Double(int v) : value(v) {}
    Double(unsigned int v) : value(v) {}
#endif
    Double(SInt32 v) : value(v) {}
    Double(UInt32 v) : value(v) {}
    Double(long long v) : value(static_cast<ValueType>(v)) {}
    Double(unsigned long long v) : value(static_cast<ValueType>(v)) {}
    Double(float v) : value(v) {}
    Double(double v) : value(v) {}
    Double(const AS3::Value& v);
    Double(const Double& other) : value(other.value) {}

    bool IsNaN() const { return IsNaN(value); }

    //
    operator ValueType() const { return value; }

    //
    template <typename T> Double& operator =(const T& v);
    Double& operator =(const Double& v);

    //
    Double operator +() const;

    //
    template <typename T> Double operator +(T v) const;
    Double operator +(Double v) const;
    Double operator +(double v) const;

    //
    template <typename T> void operator +=(const T& v);
    void operator +=(Double v);
    void operator +=(double v);

    //
    Double operator -() const;

    //
    template <typename T> Double operator -(T v) const;
    Double operator -(Double v) const;
    Double operator -(double v) const;

    //
    template <typename T> void operator -=(const T& v);
    void operator -=(Double v);
    void operator -=(double v);

    //
    template <typename T> Double operator *(T v) const;
    Double operator *(Double v) const;
    Double operator *(double v) const;
    Double operator *(int v) const;

    //
    template <typename T> void operator *=(const T& v);
    void operator *=(Double v);
    void operator *=(double v);

    //
    template <typename T> Double operator /(T v) const;
    Double operator /(Double v) const;
    Double operator /(double v) const;
    Double operator /(int v) const;

    //
    template <typename T> void operator /=(const T& v);
    void operator /=(Double v);
    void operator /=(double v);

    //
    template <typename T> bool operator ==(const T& r) const;
    bool operator ==(Double r) const;
    bool operator ==(double r) const;
#ifdef SF_CC_MSVC
    friend bool operator ==(int l, Double r);
    friend bool operator ==(unsigned int l, Double r);
#endif
    friend bool operator ==(SInt32 l, Double r);
    friend bool operator ==(UInt32 l, Double r);

    //
    template <typename T> bool operator !=(const T& v) const { return !operator ==(v); }

    //
    template <typename T> bool operator <(const T& v) const;
    bool operator <(Double v) const;
    bool operator <(double v) const;

    //
    template <typename T> bool operator <=(const T& v) const;
    bool operator <=(Double v) const;
    bool operator <=(double v) const;

    //
    template <typename T> bool operator >(const T& v) const;
    bool operator >(Double v) const;
    bool operator >(double v) const;

    //
    template <typename T> bool operator >=(const T& v) const;
    bool operator >=(Double v) const;
    bool operator >=(double v) const;

private:
#if 1
    // No dependency on NumberUtil.
    static bool SF_STDCALL IsNaN(Double v)
    {
        SF_COMPILER_ASSERT(sizeof(double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            double  D;
        } u;
        u.D = v;
        return ((u.I & SF_UINT64(0x7FF0000000000000)) == SF_UINT64(0x7FF0000000000000) && (u.I & SF_UINT64(0xFFFFFFFFFFFFF)));
    }

    static double SF_CDECL NaN()
    {
        return NumberUtil::NaN();
    }
#else
    static bool SF_STDCALL IsNaN(double v)
    {
        return NumberUtil::IsNaN(v);
    }

    static double SF_CDECL NaN()
    {
        return NumberUtil::NaN();
    }
#endif

private:
    ValueType   value;
}; // struct Double

template <typename T>
inline Double& Double::operator =(const T& v)
{
    value = static_cast<ValueType>(v);
    return *this;
}

inline Double& Double::operator =(const Double& v)
{
    value = v.value;
    return *this;
}

inline Double Double::operator +() const
{
    return Double(value);
}

template <typename T>
inline Double Double::operator +(T v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value + v;
#else
    rv = value + v;
#endif
    return Double(rv);
}

inline Double Double::operator +(Double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || v.IsNaN())
        rv = NaN();
    else
        rv = value + v.value;
#else
    rv = value + v.value;
#endif
    return Double(rv);
}

inline Double Double::operator +(double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || IsNaN(v))
        rv = NaN();
    else
        rv = value + v;
#else
    rv = value + v;
#endif
    return Double(rv);
}

template <typename T>
inline void Double::operator +=(const T& v)
{
#if 0
    if (!IsNaN())
        value += v;
#else
    value += v;
#endif
}

inline void Double::operator +=(Double v)
{
#if 0
    if (IsNaN() || v.IsNaN())
        value = NaN();
    else
        value += v.value;
#else
    value += v.value;
#endif
}

inline void Double::operator +=(double v)
{
#if 0
    if (IsNaN() || IsNaN(v))
        value = NaN();
    else
        value += v;
#else
    value += v;
#endif
}

inline Double Double::operator -() const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = -value;
#else
    rv = -value;
#endif
    return Double(rv);
}

template <typename T>
inline Double Double::operator -(T v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value - v;
#else
    rv = value - v;
#endif
    return Double(rv);
}

inline Double Double::operator -(Double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || v.IsNaN())
        rv = NaN();
    else
        rv = value - v.value;
#else
    rv = value - v.value;
#endif
    return Double(rv);
}

inline Double Double::operator -(double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || IsNaN(v))
        rv = NaN();
    else
        rv = value - v;
#else
    rv = value - v;
#endif
    return Double(rv);
}

template <typename T>
inline void Double::operator -=(const T& v)
{
#if 0
    if (!IsNaN())
        value -= v;
#else
    value -= v;
#endif
}

inline void Double::operator -=(Double v)
{
#if 0
    if (IsNaN() || v.IsNaN())
        value = NaN();
    else
        value -= v.value;
#else
    value -= v.value;
#endif
}

inline void Double::operator -=(double v)
{
#if 0
    if (IsNaN() || IsNaN(v))
        value = NaN();
    else
        value -= v;
#else
    value -= v;
#endif
}

template <typename T>
inline Double Double::operator *(T v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value * v;
#else
    rv = value * v;
#endif
    return Double(rv);
}

inline Double Double::operator *(Double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || v.IsNaN())
        rv = NaN();
    else
        rv = value * v.value;
#else
    rv = value * v.value;
#endif
    return Double(rv);
}

inline Double Double::operator *(double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || IsNaN(v))
        rv = NaN();
    else
        rv = value * v;
#else
    rv = value * v;
#endif
    return Double(rv);
}

inline Double Double::operator *(int v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value * v;
#else
    rv = value * v;
#endif
    return Double(rv);
}

template <typename T>
inline void Double::operator *=(const T& v)
{
#if 0
    if (!IsNaN())
        value *= v;
#else
    value *= v;
#endif
}

inline void Double::operator *=(Double v)
{
#if 0
    if (IsNaN() || v.IsNaN())
        value = NaN();
    else
        value *= v.value;
#else
    value *= v.value;
#endif
}

inline void Double::operator *=(double v)
{
#if 0
    if (IsNaN() || IsNaN(v))
        value = NaN();
    else
        value *= v;
#else
    value *= v;
#endif
}

template <typename T>
inline Double Double::operator /(T v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value / v;
#else
    rv = value / v;
#endif
    return Double(rv);
}

inline Double Double::operator /(Double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || v.IsNaN())
        rv = NaN();
    else
        rv = value / v.value;
#else
    rv = value / v.value;
#endif
    return Double(rv);
}

inline Double Double::operator /(double v) const
{
    ValueType rv;
#if 0
    if (IsNaN() || IsNaN(v))
        rv = NaN();
    else
        rv = value / v;
#else
    rv = value / v;
#endif
    return Double(rv);
}

inline Double Double::operator /(int v) const
{
    ValueType rv;
#if 0
    if (IsNaN())
        rv = value;
    else
        rv = value / v;
#else
    rv = value / v;
#endif
    return Double(rv);
}

template <typename T>
inline void Double::operator /=(const T& v)
{
#if 0
    if (!IsNaN())
        value /= v;
#else
    value /= v;
#endif
}

inline void Double::operator /=(Double v)
{
#if 0
    if (IsNaN() || v.IsNaN())
        value = NaN();
    else
        value /= v.value;
#else
    value /= v.value;
#endif
}

inline void Double::operator /=(double v)
{
#if 0
    if (IsNaN() || IsNaN(v))
        value = NaN();
    else
        value /= v;
#else
    value /= v;
#endif
}

template <typename T>
inline bool Double::operator ==(const T& r) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN())
        return false;
#endif

    return (value == r);
}

inline bool Double::operator ==(Double r) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || r.IsNaN())
        return false;
#endif

    return (value == r.value);
}

inline bool Double::operator ==(double r) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || IsNaN(r))
        return false;
#endif

    return (value == r);
}

#ifdef SF_CC_MSVC
inline bool operator ==(int l, Double r)
{
#ifdef SF_FLOATING_POINT_FAST
    if (r.IsNaN())
        return false;
#endif

    return l == r.value;
}

inline bool operator ==(unsigned int l, Double r)
{
#ifdef SF_FLOATING_POINT_FAST
    if (r.IsNaN())
        return false;
#endif

    return l == r.value;
}
#endif

inline bool operator ==(SInt32 l, Double r)
{
#ifdef SF_FLOATING_POINT_FAST
    if (r.IsNaN())
        return false;
#endif

    return l == r.value;
}

inline bool operator ==(UInt32 l, Double r)
{
#ifdef SF_FLOATING_POINT_FAST
    if (r.IsNaN())
        return false;
#endif

    return l == r.value;
}

template <typename T>
inline bool Double::operator <(const T& v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN())
        return false;
#endif

    return value < static_cast<ValueType>(v);
}

inline bool Double::operator <(Double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || v.IsNaN())
        return false;
#endif

    return value < v.value;
}

inline bool Double::operator <(double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || IsNaN(v))
        return false;
#endif

    return value < v;
}

template <typename T>
inline bool Double::operator <=(const T& v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN())
        return false;
#endif

    return value <= static_cast<ValueType>(v);
}

inline bool Double::operator <=(Double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || v.IsNaN())
        return false;
#endif

    return value <= v.value;
}

inline bool Double::operator <=(double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || IsNaN(v))
        return false;
#endif

    return value <= v;
}

template <typename T>
inline bool Double::operator >(const T& v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN())
        return false;
#endif

    return value > static_cast<ValueType>(v);
}

inline bool Double::operator >(Double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || v.IsNaN())
        return false;
#endif

    return value > v.value;
}

inline bool Double::operator >(double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || IsNaN(v))
        return false;
#endif

    return value > v;
}

template <typename T>
inline bool Double::operator >=(const T& v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN())
        return false;
#endif

    return value >= static_cast<ValueType>(v);
}

inline bool Double::operator >=(Double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || v.IsNaN())
        return false;
#endif

    return value >= v.value;
}

inline bool Double::operator >=(double v) const
{
#ifdef SF_FLOATING_POINT_FAST
    if (IsNaN() || IsNaN(v))
        return false;
#endif

    return value >= v;
}

#else
    typedef Scaleform::Double Double;
#endif // SF_SAFE_DOUBLE

}} // Scaleform::GFx


#if defined(SF_SAFE_DOUBLE)

namespace Scaleform {

    template <typename T> struct FmtInfo;
    class DoubleFormatter;

    template <>
    struct FmtInfo<GFx::Double>
    {
        typedef DoubleFormatter formatter;
    };

} // namespace Scaleform

#endif // SF_SAFE_DOUBLE

#endif // INC_SF_GFX_TYPES_H
