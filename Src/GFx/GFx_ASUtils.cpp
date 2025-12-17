/**************************************************************************

Filename    :   GFx_ASUtils.cpp
Content     :   AS Utility functinality
Created     :   Sep 21, 2009
Authors     :   Artyom Bolgar, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_ASUtils.h"

namespace Scaleform { namespace GFx {

namespace ASUtils {

bool IsWhiteSpace(UInt32 c)
{
    return (c == ' ' ||  c == '\n' || c == '\r' || c =='\t' || c == '\f' || c == '\v' ||
        (c >= 0x2000 && c <=0x200b) || c == 0x2028 || c == 0x2029 || c == 0x205f || c == 0x3000 );
}

UInt32 SkipWhiteSpace(const String& str)
{
    UInt32 start = 0;
    UInt32 len = (UInt32)str.GetLength();
    while (start < len)
    {
        UInt32 c = str.GetCharAt(start);
        if (IsWhiteSpace(c))
            ++start;
        else
            break;
    }
    return start; 
}

const char* SkipWhiteSpace(const char* str, UPInt len)
{
    const char* end = str + len;
    const char* tmp;

    while (str < end)
    {
        tmp = str;
        UInt32 c = UTF8Util::DecodeNextChar_Advance0(&str);
        if (!IsWhiteSpace(c))
        {
            str = tmp;
            break;
        }
    }
    return str; 
}

void EscapeWithMask(const char* psrc, UPInt length, String* pescapedStr, const unsigned* escapeMask)
{   
    SF_ASSERT(pescapedStr);

    char            buf[256];
    char*           pbuf = buf;
    char* const     endp = pbuf + sizeof (buf) - 1;    

    for (unsigned i = 0; i < length; ++i)
    {
        int ch = (unsigned char)psrc[i];
        if (pbuf + 4 >= endp)
        {
            // flush
            *pbuf = '\0';
            *pescapedStr += buf;
            pbuf = buf;
        }
        if (ch < 128 && (escapeMask[ch/32]&(1u<<(ch%32)))) // isalnum (ch)
        {
            *pbuf++ = char(ch);
        }
        else 
        {
            *pbuf++ = '%';
            *pbuf++ = char(ch/16 + ((ch/16 <= 9) ? '0' : 'A'-10));
            *pbuf++ = char(ch%16 + ((ch%16 <= 9) ? '0' : 'A'-10));
        }
    }
    // flush
    *pbuf = '\0';
    *pescapedStr += buf;
}

void Unescape(const char* psrc, UPInt length, String* punescapedStr)
{   
    SF_ASSERT(punescapedStr);

    char                buf[256];
    const char* const   endcstr = psrc + length;
    char*               pbuf = buf;
    char* const         endp = pbuf + sizeof (buf) - 1;

    while (psrc < endcstr)
    {
        int ch = (unsigned char) *psrc++;
        if (pbuf + 1 >= endp)
        {
            // flush
            *pbuf = '\0';
            *punescapedStr += buf;
            pbuf = buf;
        }
        if (ch == '%')
        {
            int fd = SFtoupper((unsigned char)*psrc++);
            int sd = SFtoupper((unsigned char)*psrc++);
            fd = (fd - '0' > 9) ? fd - 'A' + 10 : fd - '0';
            sd = (sd - '0' > 9) ? sd - 'A' + 10 : sd - '0';
            if (fd < 16 && sd < 16)
            {
                *pbuf++ = char(fd*16 + sd);
            }
        }
        else 
        {
            *pbuf++ = char(ch);
        }
    }
    // flush
    *pbuf = '\0';
    *punescapedStr += buf;
}

void Escape(const char* psrc, UPInt length, String* pescapedStr)
{
    // each bit in mask represent the condition "isalnum(char) == 1"
    // mask positioning is as follows: mask[char/32]&(1<<(char%32))
    static const unsigned mask[] = { 0x00000000, 0x03FF0000, 0x07FFFFFE, 0x07FFFFFE };
    ASUtils::EscapeWithMask(psrc, length, pescapedStr, mask);
}

void EscapePath(const char* psrc, UPInt length, String* pescapedStr)
{
    // each bit in mask represent the condition "isalnum(char) || char == '\\' || char == '/' || char == '.' || char == ':'"
    // mask positioning is as follows: mask[char/32]&(1<<(char%32))
    static const unsigned mask[] = { 0x00000000, 0x07FFC000, 0x17FFFFFE, 0x07FFFFFE };
    ASUtils::EscapeWithMask(psrc, length, pescapedStr, mask);
}

namespace AS3
{
    class Formatter
    {
    public:
        static void EscapeWithMask(const char* psrc, UPInt length, StringBuffer& b, const unsigned* escapeMask,
            bool useUtf8 = false);
        // Return true on success.
        // ECMA 15.1.3
        static bool EscapeWithMaskURI(const char* psrc, UPInt length, StringBuffer& b, bool isComp);
        // Return true on success.
        static bool Unescape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8 = false);
        // Return true on success.
        static bool DecodeURI(const char* psrc, UPInt length, StringBuffer& b, bool isComp);

    private:
        static bool InMask(int ch, const unsigned* escapeMask)
        {
            return ch < 128 && (escapeMask[ch/32]&(1u<<(ch%32)));
        }

        // One byte
        static void WriteHexNibble(StringBuffer& b, UInt8 v);
        static UInt8 ReadHexNibble(const char*& pStr, const char* end);

        // Two bytes
        static void WriteHexByte(StringBuffer& b, UInt8 v)
        {
            WriteHexNibble(b, static_cast<UInt8>(v >> 4));
            WriteHexNibble(b, static_cast<UInt8>(v & 0x0F));
        }

        // Four bytes
        static void WriteHexWord(StringBuffer& b, UInt16 v)
        {
            WriteHexByte(b, static_cast<UInt8>(v >> 8));
            WriteHexByte(b, static_cast<UInt8>(v & 0xFF));
        }

        static bool IsHexDigit(char c)
        {
            return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
        }

        // This method will not advance position if format doesn't comply with the rules.
        // pStr has to be checked after a call to ReadHex().
        static UInt16 ReadHex(const char*& pStr, const char* end, UInt8 max_chars);
    };

    UInt16 Formatter::ReadHex(const char*& pStr, const char* end, UInt8 max_chars)
    {
        UInt16 result = 0;

        const char* prev_ptr = pStr;
        for (int i = 0; (i < max_chars) && (pStr < end) && IsHexDigit(*pStr); ++i)
            result = (result << 4) | ReadHexNibble(pStr, end);

        if ((pStr - prev_ptr) < max_chars)
            // Restore position.
            pStr = prev_ptr;

        return result;
    }

    void Formatter::WriteHexNibble(StringBuffer& b, UInt8 v)
    {
        if (v < 10)
            b += (char)(v + '0');
        else
            b += (char)(v + 'A' - 10);
    }

    UInt8 Formatter::ReadHexNibble(const char*& pStr, const char* end)
    {
        if (pStr < end)
        {
            const char c = *pStr++;

            if ('0' <= c && c <= '9') 
                return c - '0';

            if ('A' <= c && c <= 'F') 
                return c - 'A' + 10;

            if ('a' <= c && c <= 'f') 
                return c - 'a' + 10;

            // Error.
            --pStr;
        }

        return 0;
    }

    void Formatter::EscapeWithMask(
        const char* psrc, UPInt length, 
        StringBuffer& b, 
        const unsigned* escapeMask,
        bool useUtf8)
    {
        const char* end = psrc + length;

        while (psrc < end)
        {
            const char* psrc_saved = psrc;
            int ch = (int)UTF8Util::DecodeNextChar_Advance0(&psrc);

            if (InMask(ch, escapeMask)) // isalnum (ch)
                b += char(ch);
            else if (ch & 0xFF00)
            {
                if (!useUtf8)
                {
                    // encode as Unicode 16-bit value
                    // Totally six bytes.
                    b += '%';
                    b += 'u';
                    WriteHexWord(b, static_cast<UInt16>(ch));
                }
                else
                {
                    // encode as series of UTF-8 8-bit bytes
                    while (psrc_saved < psrc)
                    {
                        b += '%';
                        WriteHexByte(b, static_cast<UInt8>(*psrc_saved++));
                    }
                }
            }
            else 
            {
                b += '%';
                WriteHexByte(b, static_cast<UInt8>(ch));
            }
        }
    }

    static unsigned uriUnescaped[] =
    {
        0x00000000,
        0x03ff6782,
        0x87fffffe,
        0x47fffffe
    };
    
    static unsigned uriReservedPlusPound[] =
    {
        0x00000000,
        0xac009858,
        0x00000001,
        0x00000000
    };
    
    bool Formatter::EscapeWithMaskURI(
        const char* psrc, UPInt length,
        StringBuffer& b,
        bool isComp
        )
    {
        SF_UNUSED1(isComp);
        const char* end = psrc + length;
        // 1.
        UPInt strLen = UTF8Util::GetLength(psrc);
        // 3.
        UPInt k = 0;

        while (psrc < end)
        {
            int ch = (int)UTF8Util::DecodeNextChar_Advance0(&psrc);
            ++k;

            // 4.c.i
            if (InMask(ch, uriUnescaped) || (!isComp && InMask(ch, uriReservedPlusPound)))
            {
                // 4.c.ii
                b += char(ch);
            }
            else
            {
                if (ch >= 0xDC00 && ch <= 0xDFFF)
                    // 4.d.i
                    // URIError exception.
                    return false;

                UInt32 v;
                if (ch < 0xD800 || ch > 0xDBFF)
                    // 4.d.ii.1
                    v = ch;
                else
                {
                    // 4.d.iii.1
                    ++k;
                    if (k == strLen)
                        // 4.d.iii.2
                        // URIError exception.
                        return false;

                    // 4.d.iii.3
                    int kch = (int)UTF8Util::DecodeNextChar_Advance0(&psrc);
                    if (kch < 0xDC00 || kch > 0xDFFF)
                        // 4.d.iii.4
                        // URIError exception.
                        return false;

                    // 4.d.iii.5
                    // Let V be (((the code unit value of C) ?0xD800) * 0x400 + (kChar ?0xDC00) + 0x10000).
                    v = (ch - 0xD800) * 0x400 + (kch - 0xDC00) + 0x10000;
                }

                // 4.d.iv
                char Octets[6];
                SPInt l = 0;
                UTF8Util::EncodeCharSafe(Octets, 6, &l, v);

                if (l == 0)
                    // URIError exception.
                    return false;

                // 4.d.v
                // 4.d.vi
                for (SPInt j = 0; j < l; ++j)
                {
                    // 4.d.vi.1
                    UInt8 jOctet = Octets[j];
                    // 4.d.vi.2
                    b += '%';
                    WriteHexByte(b, jOctet);
                }
            }
        }

        return true;
    }

    bool Formatter::Unescape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8)
    {   
        const char* end = psrc + length;
        bool asUtf8;

        while (psrc < end)
        {
            int ch = *psrc++;

            if (ch == '%')
            {
                const char* prev_ptr = psrc;

                asUtf8 = useUtf8 && (*psrc != 'u');

                if (*psrc == 'u')
                    ch = ReadHex(psrc, end, 4);
                else
                    ch = ReadHex(psrc, end, 2);

                if (psrc == prev_ptr)
                    return false;

                if (asUtf8)
                    // It is a regular character.
                    b += static_cast<char>(ch);
                else
                    b.AppendChar(ch);
            }
            else
                // It is a regular character.
                b += static_cast<char>(ch);
        }

        return true;
    }

    bool Formatter::DecodeURI(const char* psrc, UPInt length, StringBuffer& b, bool isComp)
    {
        const char* end = psrc + length;
        const char* prev_ptr;
        // 3.
        UPInt k = 0;

        while (psrc < end)
        {
            // 4.b
            int ch = *psrc++;

            if (ch == '%')
            {
                // 4.d.i
                UPInt start = k;

#if 0
                // 4.d.i
                if (k + 2 >= length)
                    // URIError exception.
                    return false;
#endif

                prev_ptr = psrc;
                int v1 = ReadHexNibble(psrc, end);
                if (psrc == prev_ptr)
                    // 4.d.iii
                    return false;

                prev_ptr = psrc;
                int v2 = ReadHexNibble(psrc, end);
                if (psrc == prev_ptr)
                    // 4.d.iii
                    return false;

                // 4.d.iv
                UInt8 B = (UInt8)((v1 << 4) | v2);

                // 4.d.v
                k += 2;

                UInt32 V;

                if (!(B & 0x80))
                    // 4.d.vi
                    V = B;
                else
                {
                    // 4.d.vii

                    // 4.d.vii.1
                    // Let n be the smallest non-negative number such that (B << n) & 0x80 is equal to 0.
                    int n = 1;
                    while (((B << n) & 0x80) != 0)
                        ++n;

                    // 4.d.vii.2
                    // If n equals 1 or n is greater than 4, throw a URIError exception.
                    if (n == 1 || n > 4)
                        return false;

                    // 4.d.vii.3
                    char Octets[4]; 

                    // 4.d.vii.4
                    // Put B into Octets at position 0.
                    Octets[0] = B;

                    // 4.d.vii.5
                    // If k + (3 * (n ?1)) is greater than or equal to strLen, throw a URIError exception.
                    if (k + (3 * (n - 1)) >= length)
                        return false;

                    // 4.d.vii.6
                    // Let j be 1
                    // 4.d.vii.7
                    // Repeat, while j < n
                    for (int j = 1; j < n; ++j)
                    {
                        // 4.d.vii.7.a
                        // Increment k by 1.
                        ++k;
                        ch = *psrc++;

                        // 4.d.vii.7.b
                        // If the character at position k is not '%', throw a URIError exception.
                        if (ch != '%')
                            return false;

                        prev_ptr = psrc;
                        int v1_inner = ReadHexNibble(psrc, end);
                        if (psrc == prev_ptr)
                            // 7.c
                            return false;

                        prev_ptr = psrc;
                        int v2_inner = ReadHexNibble(psrc, end);
                        if (psrc == prev_ptr)
                            // 7.c
                            return false;

                        // 7.d
                        B = (UInt8)((v1_inner << 4) | v2_inner);

                        // 7.e
                        // If the two most significant bits in B are not 10, throw a URIError exception.
                        if ((B & 0xC0) != 0x80)
                            return false;

                        // 7.f
                        // Increment k by 2.
                        k += 2;

                        // 7.g
                        // Put B into Octets at position j.
                        Octets[j] = B;
                    }

                    // 4.d.vii.8
                    // Let V be the value obtained by applying the UTF-8 transformation to Octets,
                    // that is, from an array of octets into a 21-bit value. If Octets does not 
                    // contain a valid UTF-8 encoding of a Unicode code point throw an URIError exception.
                    prev_ptr = Octets;
                    const char* tmp_ptr = Octets;
                    V = UTF8Util::DecodeNextChar_Advance0(&tmp_ptr);
                    if (V == 0xFFFD)
                        return false;
                }

                if (V < 0x10000)
                {
                    // 4.d.vii.9
                    // If V is less than 0x10000, then

                    // Check for reserved set
                    if (!isComp)
                    {
                        if (InMask(V, uriReservedPlusPound))
                        {
                            while (start <= k)
                                b += psrc[start++];
                        } 
                        else
                            b.AppendChar(V);
                    }
                    else
                        b.AppendChar(V);
                } else {
                    // 4.d.vii.10
                    // Else, V is >= 0x10000

                    if (V > 0x10FFFF)
                        return false;

                    // 4.d.vii.10.a
                    // Let L be (((V - 0x10000) & 0x3FF) + 0xDC00).
                    UInt32 L = (((V - 0x10000) & 0x3FF) + 0xDC00);

                    // 4.d.vii.10.b
                    // Let H be ((((V - 0x10000) >> 10) & 0x3FF) + 0xD800).
                    UInt32 H = ((((V - 0x10000) >> 10) & 0x3FF) + 0xD800);

                    // 4.d.vii.10.c
                    // Let S be the String containing the two characters with code unit values H and L.
                    b.AppendChar(H);
                    b.AppendChar(L);
                } 
            }
            else
            {
                // 4.c.i
                // It is a regular character.
                b += static_cast<char>(ch);
            }

            // 4.f
            ++k;
        }

        return true;
    }

    // each bit in mask represent the condition "isalnum(char) == 1"
    // mask positioning is as follows: mask[char/32]&(1<<(char%32))
    static unsigned unescaped_mask[] = { 0x00000000, 0x03FFEC00, 0x87FFFFFF, 0x07FFFFFE };
    static unsigned unescaped_mask_VAR[] = { 0x00000000, 0x03FF0000, 0x87FFFFFF, 0x07FFFFFE };
    static unsigned unescaped_mask_URIComponent[] = { 0x00000000, 0x03FF6782, 0x87FFFFFE, 0x47FFFFFE };

    void Escape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8)
    {
        Formatter::EscapeWithMask(psrc, length, b, unescaped_mask, useUtf8);
    }

    void EncodeVar(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8)
    {
        Formatter::EscapeWithMask(psrc, length, b, unescaped_mask_VAR, useUtf8);
    }

    bool EncodeURI(const char* psrc, UPInt length, StringBuffer& escapedStr, bool isComp)
    {
        return Formatter::EscapeWithMaskURI(psrc, length, escapedStr, isComp);
    }

    void EncodeURIComponent(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8)
    {
        Formatter::EscapeWithMask(psrc, length, b, unescaped_mask_URIComponent, useUtf8);
    }

    void EscapeWithMask(const char* psrc, UPInt length, StringBuffer& b, const unsigned* escapeMask, bool useUtf8)
    {   
        Formatter::EscapeWithMask(psrc, length, b, escapeMask, useUtf8);
    }

    bool Unescape(const char* psrc, UPInt length, StringBuffer& b, bool useUtf8)
    {
        return Formatter::Unescape(psrc, length, b, useUtf8);
    }

    bool DecodeURI(const char* psrc, UPInt length, StringBuffer& b, bool isComp)
    {
        return Formatter::DecodeURI(psrc, length, b, isComp);
    }

} // namespace AS3

} // namespace ASUtils

//////////////////////////////////////////////////////////////////////////


#if SF_BYTE_ORDER == SF_LITTLE_ENDIAN

static const UByte GFxNaN_Bytes[]               = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF};
// static const UByte GFxQNaN_Bytes[]              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x7F};
static const UByte GFxPOSITIVE_INFINITY_Bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x7F};
static const UByte GFxNEGATIVE_INFINITY_Bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF};
static const UByte GFxMIN_VALUE_Bytes[]         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00};
static const UByte GFxMAX_VALUE_Bytes[]         = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0x7F};
static const UByte GFxNEGATIVE_ZERO_Bytes[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
static const UByte GFxPOSITIVE_ZERO_Bytes[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#else

static const UByte GFxNaN_Bytes[]               = {0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// static const UByte GFxQNaN_Bytes[]              = {0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UByte GFxPOSITIVE_INFINITY_Bytes[] = {0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UByte GFxNEGATIVE_INFINITY_Bytes[] = {0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UByte GFxMIN_VALUE_Bytes[]         = {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UByte GFxMAX_VALUE_Bytes[]         = {0x7F, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UByte GFxNEGATIVE_ZERO_Bytes[]     = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UByte GFxPOSITIVE_ZERO_Bytes[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#endif

namespace NumberUtil {

GFx::Double SF_CDECL NaN()               { Double v; memcpy(&v, GFxNaN_Bytes              , sizeof(v)); return v; }
GFx::Double SF_CDECL POSITIVE_INFINITY() { Double v; memcpy(&v, GFxPOSITIVE_INFINITY_Bytes, sizeof(v)); return v; }
GFx::Double SF_CDECL NEGATIVE_INFINITY() { Double v; memcpy(&v, GFxNEGATIVE_INFINITY_Bytes, sizeof(v)); return v; }
GFx::Double SF_CDECL POSITIVE_ZERO()     { Double v; memcpy(&v, GFxPOSITIVE_ZERO_Bytes    , sizeof(v)); return v; }
GFx::Double SF_CDECL NEGATIVE_ZERO()     { Double v; memcpy(&v, GFxNEGATIVE_ZERO_Bytes    , sizeof(v)); return v; }
GFx::Double SF_CDECL MIN_VALUE()         { Double v; memcpy(&v, GFxMIN_VALUE_Bytes        , sizeof(v)); return v; }
GFx::Double SF_CDECL MAX_VALUE()         { Double v; memcpy(&v, GFxMAX_VALUE_Bytes        , sizeof(v)); return v; }

bool SF_STDCALL IsPOSITIVE_ZERO(Double v)
{
    return (memcmp(&v, GFxPOSITIVE_ZERO_Bytes,  sizeof(v)) == 0);
}

bool SF_STDCALL IsNEGATIVE_ZERO(Double v)
{
    return (memcmp(&v, GFxNEGATIVE_ZERO_Bytes,  sizeof(v)) == 0);
}

float ConvertDouble2Float(double v)
{
    union 
    {
        double D;
        UInt64 I;
    } d;

    union
    {
        float  F;
        UInt32 I;
    } f;

    d.D = v;

    const int sign = static_cast<int>((d.I & 0x8000000000000000ULL) >> 63);
    int expo = static_cast<int>((d.I & 0x7FF0000000000000ULL) >> 52);
    int mant = static_cast<int>((d.I & 0x000FFFFFFFFFFFFFULL) >> 29);

    if (expo == 2047)
        expo = 255;
    else if (expo - 1023 > 127)
    {
        expo = 255;
        mant = 0;
    }
    else if (expo - 1023 < -126)
    {
        expo = 0;
        mant = 0;
    }
    else
        expo = expo - 1023 + 127;

    f.I = (sign << 31) + (expo << 23) + mant;

    return f.F;
}

// radixPrecision: > 10 - radix
//                 < 0  - precision (= -radixPrecision)
const char* SF_STDCALL ToString(Double value, char destStr[], size_t destStrSize, int radix)
{
    SF_ASSERT(destStrSize > 0);

#if 1
    // MA: not sure about precision, but "%.14g" 
    // makes test_rotation.swf display too much rounding            
    // MA: not sure about precision, but "%.14g" 
    // makes test_rotation.swf display too much rounding            
    static const char* const fmt[] = { 
        "%.1g", "%.2g", "%.3g", "%.4g", 
        "%.5g", "%.6g", "%.7g", "%.8g", 
        "%.9g", "%.10g", "%.11g", "%.12g", "%.13g", "%.14g" };
        const char* fmtStr = fmt[14 -1];

        if(radix <= 0)
        {
            fmtStr = fmt[((radix >= -14) ? -radix : 14) - 1];
            radix = 10;
        }

        if (NumberUtil::IsNaNOrInfinity(value))
        {
            if(NumberUtil::IsNaN(value))
            {
                SFstrcpy(destStr, destStrSize, "NaN");
            }
            else if(NumberUtil::IsPOSITIVE_INFINITY(value))
            {
                SFstrcpy(destStr, destStrSize, "Infinity");
            }
            else if(NumberUtil::IsNEGATIVE_INFINITY(value))
            {
                SFstrcpy(destStr, destStrSize, "-Infinity");
            }
        }
        else
        {
            if (radix == 10) 
            {
                SInt32 ival = (SInt32)value;
                if ((Double)ival == value)
                    return IntToString(ival, destStr, destStrSize);
                SFsprintf(destStr, destStrSize, fmtStr, (double)value);

#ifndef SF_OS_WINCE
                // Get rid of a possible comma ...
                for (char* s = destStr; *s != 0; ++s)
                {
                    if (*s == ',' || *s == '.')
                    {
                        *s = '.';
                        break;
                    }
                }
#endif
            }
            else
                return IntToString((int)value, destStr, destStrSize, radix);
        }

#else

    // MA: not sure about precision, but "%.14g" 
    // makes test_rotation.swf display too much rounding            
    unsigned precision = 14;

    if(radix <= 0)
    {
        precision = ((radix >= -14) ? -radix : 14);
        radix = 10;
    }

    if(NumberUtil::IsNaN(value))
    {
        destStr = "NaN";
    }
    else if(NumberUtil::IsPOSITIVE_INFINITY(value))
    {
        destStr = "Infinity";
    }
    else if(NumberUtil::IsNEGATIVE_INFINITY(value))
    {
        destStr = "-Infinity";
    }
    else
    {
        switch (radix) {
            case 10:
                {
                    //SInt32 ival = (SInt32)value;
                    //if ((double)ival == value)
                    //    return IntToString(ival, destStr, destStrSize);

                    GDoubleFormatter d((double)value);

                    d.SetPrecision(precision).SetType(GDoubleFormatter::FmtSignificant).Convert();
                    StringDataPtr result = d.GetResult();

                    size_t size = Alg::Min(destStrSize, result.GetSize());
                    memcpy(destStr, result.ToCStr(), size - 1);
                    destStr[size] = '\0';
                }
                break;
            case 16:  
            case 8:
            case 2:
                {
                    LongFormatter l((int)value);

                    l.SetBase(radix).Convert();
                    StringDataPtr result = l.GetResult();

                    size_t size = Alg::Min(destStrSize, result.GetSize());
                    memcpy(destStr, result.ToCStr(), size - 1);
                    destStr[size] = '\0';
                }
        }
    }

#endif

    return destStr;
}

// radixPrecision: > 10 - radix
//                 < 0  - precision (= -radixPrecision)
const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize, int radix)
{
    SF_ASSERT(destStrSize > 0);
    size_t      bufSize = destStrSize - 1;
    char*       pbuf  = &destStr[bufSize];
    *pbuf = '\0';

    switch (radix) {
        case 16:  
            {
                unsigned    ival = (unsigned)value;
                for (unsigned i = 0; i < bufSize; i++) 
                {
                    pbuf--;
                    UByte c = (UByte)(ival & 0xF);
                    *pbuf = (c <= 9) ? c + '0' : c - 10 + 'a';
                    ival >>= 4;
                    if (ival == 0)
                        break;
                }
            }
            break;
        case 8:
            {
                unsigned    ival = (unsigned)value;
                for (unsigned i = 0; i < bufSize; i++) 
                {
                    pbuf--;
                    UByte c = (UByte)(ival & 0x7);
                    *pbuf = c + '0';
                    ival >>= 3;
                    if (ival == 0)
                        break;
                }
            }
            break;
        case 2:
            {
                unsigned    ival = (unsigned)value;
                unsigned mask = 1;
                char* plastSignificantBit = 0;
                for (unsigned i = 0; i < sizeof (int)*8 && i < bufSize; i++) 
                {
                    pbuf--;
                    if (ival & mask) {
                        *pbuf = '1';
                        plastSignificantBit = pbuf;
                    }
                    else {
                        *pbuf = '0';
                    }
                    mask <<= 1;
                    if (mask == 0) mask = 1;
                }
                if (plastSignificantBit == 0)
                    pbuf = &destStr[destStrSize - 2];
                else
                    pbuf = plastSignificantBit;
            }
            break;
        default:
            return IntToString(value, destStr, destStrSize);
    }
    return pbuf;
}

const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize)
{
    SF_ASSERT(destStrSize > 0);
    size_t      bufSize = destStrSize - 1;
    char*       pbuf  = &destStr[bufSize];
    *pbuf = '\0';

    unsigned i, absValue = Alg::Abs(value);
    for (i = 0; i < bufSize; i++) 
    {
        pbuf--;
        UByte c = (UByte)(absValue % 10);
        *pbuf = c + '0';
        absValue /= 10;
        if (absValue == 0)
            break;
    }
    if (i < bufSize && value < 0)
        *(--pbuf) = '-';
    return pbuf;
}

SF_INLINE  SInt32 readDigit (char c)
{
    if (c >= '0' && c <= '9') 
        return (c - '0');
    else if (c >= 'a' && c <= 'z')
        return (c - 'a' + 10);
    else if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 10);
    else
        return -1;
}

GFx::Double SF_STDCALL StringToInt( const char* str, UInt32 strLen, SInt32 radix, UInt32 *endIndex )
{
    SInt32      sign = 1;
    UInt32      &offset = *endIndex;
    offset = 0;

    // ecma: Leading white space in string is ignored. If radix is undefined or 0, 
    //it is assumed to be 10 except when the number begins with the character pairs 0x or 0X, 
    //in which case a radix of 16 is assumed. If radix is 16, number may also optionally begin 
    //with the character pairs 0x or 0X.

    bool stripPrefix = true;
    if (radix == 0)
    {
        radix = 10;
    }
    else if ((radix < 2) || (radix > 36))
    {
        return NaN();
    }
    else if (radix != 16)
    {
        stripPrefix = false;
    }


    offset = (UInt32)UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(str), str, strLen);

    if(offset == strLen)
        return 0;

    if (str[offset] == '-')
    {
        sign = -1;
        ++offset;
    }
    else if (str[offset] == '+')
        ++offset;

    if (stripPrefix && (strLen - offset > 1))
    {
        if (str[offset] == '0' && (str[offset+1] == 'x' ||  str[offset+1] == 'X'))
        {
            radix = 16;
            offset+=2;
            if (strLen == offset)
                return NaN();
        }
    }

    UInt32 start = offset;
    Double n = 0;
    SInt32 digit = 0;
    while (offset < strLen)
    {
        digit = readDigit(str[offset]);
        if (digit >= radix || digit < 0)
            break;
        n = n * radix + digit;

        ++offset;
    }

    if (offset == start)
        return NaN();


    if (n >= 0x20000000000000LL && // more then 53 bits of mantissa 
        (radix == 2 || radix == 8 || radix == 16 ))  //special cases for radix 2, 8, 16 to avoid loss of precision            
    {
        SInt32 p2 = 1;
        switch (radix)
        {
        case 8:
            p2 = 3;
            break;
        case 16:
            p2 = 4;
            break;
        }

        //offset = start;
        while(start < strLen && str[start] =='0')
            ++start;

        SF_ASSERT (start < strLen);
        n = 0;
        UInt32 index;
        for (index = 0; start < strLen && index*p2 <= 52; index++)
        {
            digit = readDigit(str[start++]);
            if (digit >= radix || digit < 0)
            {
                digit = 0;
                break;
            }
            n = n * radix + digit;
        }

        if (index*p2 > 52) //Rounding for numbers more then 53 bit
        { 
            bool round = false;

            bool b53 = 0;
            bool b54 = 0;

            UInt32 shift = 0;

            switch(radix) 
            {
            case 16:  
                b53 = digit & 0x1;
                if (start < strLen)
                    digit = readDigit(str[start]);
                else
                    digit = -1;
                if (digit != -1 && digit < radix) 
                {
                    shift += p2;
                    b54 = (digit & 0x8) != 0;
                    round = (digit & 0x3) != 0;  
                } 
                else 
                {
                    round = b53 != 0;
                }
                break;

            case 8: 
                if (start < strLen)
                    digit = readDigit(str[start]);
                else
                    digit = -1;
                if (digit == -1 || digit >= radix) 
                    digit = 0;
                shift += p2;
                b53 = (digit & 0x2) != 0;
                b54 = (digit & 0x1) != 0;
                break;

            case 2: 
                b53 = (digit & 0x1) != 0;
                if (start < strLen)
                    digit = readDigit(str[start]);
                else
                    digit = -1;
                if (digit == -1 || digit >= radix) 
                    digit = 0;
                shift += p2;
                b54 = (digit & 0x1) != 0;
                break;
            }

            while(++start < strLen) 
            {
                digit = readDigit(str[start]);
                if (digit == -1 || digit >= radix) {
                    break;
                }
                round |= (digit != 0); 
                shift += p2;
            }
            round = b54 && (b53 || round);
            n += (round ? 1.0 : 0.0);
            n *= (1 << shift);
        }

    }
    return n * sign;
}

GFx::Double SF_STDCALL StringToDouble( const char* str, UInt32 strLen, UInt32 *endIndex )
{
    UInt32      &offset = *endIndex;
    offset = 0;
    offset = (UInt32)UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(str), str, strLen);

    strLen -= offset;

    // Unlike parseInt, parseFloat should return NaN for white spaces
    if (strLen == 0)
    {
        return NaN();
    }

    char *tail = 0;
    str += offset;

    if (strLen > 1)
    {
        switch (*str)
        {
        case 'I':
            if (!strncmp(str, "Infinity", 8))
            {
                offset += 8;
                return POSITIVE_INFINITY();
            }
            break;
        case '+':
            if (!strncmp(str, "+Infinity", 9))
            {
                offset += 9;
                return POSITIVE_INFINITY();
            }
            break;
        case '-':
            if (!strncmp(str, "-Infinity", 9))
            {
                offset += 9;
                return NEGATIVE_INFINITY();
            }
            break;
        default:
            break;
        }
    }

    double res = SFstrtod(str, &tail);
    offset += (UInt32)(tail - str);
    return (tail != str ? GFx::Double(res) : NaN());
}

} // namespace NumberUtil

}} //namespace Scaleform { namespace GFx {
