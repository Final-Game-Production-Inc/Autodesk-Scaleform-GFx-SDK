/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ASUtils.h
Content     :   AS Utility functinality
Created     :   Sep 21, 2009
Authors     :   Artyom Bolgar, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_ASUtilS_H
#define INC_SF_GFX_ASUtilS_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_String.h"

namespace Scaleform { namespace GFx {

// This namespace contains some static AS-related helper functions
namespace ASUtils
{
    // Escape functions - src length required.
    void EscapeWithMask(const char* psrc, UPInt length, String* pescapedStr, const unsigned* escapeMask);
    void EscapePath(const char* psrc, UPInt length, String* pescapedStr);
    void Escape(const char* psrc, UPInt length, String *pescapedStr);
    void Unescape(const char* psrc, UPInt length, String *punescapedStr);

    bool IsWhiteSpace(UInt32 c);
    // Used in AS3 parseInt, parseDouble and ConvertToNumber
    // Return *character index* of first not whitespace char.
    UInt32 SkipWhiteSpace(const String& str);
    // Return pointer to a first not whitespace char.
    const char* SkipWhiteSpace(const char* str, UPInt len);

    namespace AS3
    {
        // psrc must be an UTF8 encoded string.
        // useUtf8 param indicates that string will be encoded as UTF8 string first and then 
        // escaped; if the param is false then Unicode string will be used for escaping. The difference
        // between these two approaches will be something like "%D0%3e" vs "%u0420".
        void EscapeWithMask
            (const char* psrc, UPInt length, StringBuffer& b, const unsigned* escapeMask, bool useUtf8 = false);
        // psrc must be an UTF8 encoded string.
        void Escape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8 = false);
        void EncodeVar(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8 = false);
        // Return true in case of success.
        bool EncodeURI(const char* psrc, UPInt length, StringBuffer& escapedStr, bool isComp = false);
        void EncodeURIComponent(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8 = false);
        // psrc must be an UTF8 encoded string.
        // Return true in case of success.
        bool Unescape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8 = false);
        // Return true in case of success.
        bool DecodeURI(const char* psrc, UPInt length, StringBuffer& b, bool isComp = false);
    }
}

// This namespace contains some floating point number-related helper functions
namespace NumberUtil {

    // This function is necessary for platforms, which do not have hardware
    // support for double.
    float           ConvertDouble2Float(double v);

    GFx::Double SF_CDECL NaN();
    GFx::Double SF_CDECL POSITIVE_INFINITY();
    GFx::Double SF_CDECL NEGATIVE_INFINITY();
    GFx::Double SF_CDECL MIN_VALUE();
    GFx::Double SF_CDECL MAX_VALUE();
    GFx::Double SF_CDECL POSITIVE_ZERO();
    GFx::Double SF_CDECL NEGATIVE_ZERO();

    inline bool SF_STDCALL IsNaN(Double v)
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
    inline bool SF_STDCALL IsPOSITIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            double  D;
        } u;
        u.D = v;
        return (u.I == SF_UINT64(0x7FF0000000000000));
    }
    inline bool SF_STDCALL IsNEGATIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            double  D;
        } u;
        u.D = v;
        return (u.I == SF_UINT64(0xFFF0000000000000));
    }
    inline bool SF_STDCALL IsNaNOrInfinity(Double v)
    {
        SF_COMPILER_ASSERT(sizeof(double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            double  D;
        } u;
        u.D = v;
        return ((u.I & SF_UINT64(0x7FF0000000000000)) == SF_UINT64(0x7FF0000000000000));
    }

    bool SF_STDCALL IsPOSITIVE_ZERO(Double v);
    bool SF_STDCALL IsNEGATIVE_ZERO(Double v);

    enum 
    {
        TOSTRING_BUF_SIZE = 64
    };
    // radixPrecision: > 0  - radix
    //                 < 0  - precision (= -radixPrecision)
    // Returns: a pointer to the first character in the destStr buffer.
    // The recommended size for the destStr buffer is TOSTRING_BUF_SIZE.
    // IMPORTANT: use returned pointer rather than the buffer itself: ToString & IntToString
    // fill buffer from the end to the beginning and DO NOT move the resulting string to the
    // beginning of the buffer; thus, the resulting string may start not from the beginning of
    // the buffer.
    const char* SF_STDCALL ToString(Double value, char destStr[], size_t destStrSize, int radixPrecision = 10);
    // radix - 2, 8, 10, 16
    const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize, int radix);
    // radix is always 10
    const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize);

    //ECMA-262 implementations
    GFx::Double SF_STDCALL StringToInt(const char* str, UInt32 strLen, SInt32 radix, UInt32 *endIndex);
    GFx::Double SF_STDCALL StringToDouble(const char* str, UInt32 strLen,  UInt32 *endIndex);
} // NumberUtil

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_ASUtilS_H
