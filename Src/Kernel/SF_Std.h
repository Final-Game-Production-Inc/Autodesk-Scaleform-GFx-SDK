/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Std.h
Content     :   Standard C function interface
Created     :   2/25/2007
Authors     :   Ankur Mohan, Michael Antonov, Artem Bolgar, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Std_H
#define INC_SF_Kernel_Std_H

#include "SF_Types.h"
#include <stdarg.h> // for va_list args
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if !defined(SF_OS_WINCE) && defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
#define SF_MSVC_SAFESTRING
#include <errno.h>
#endif

// Wide-char funcs
#if !defined(SF_OS_SYMBIAN) && !defined(SF_CC_RENESAS) && !defined(SF_OS_PS2)
#include <wchar.h>
#endif

#if !defined(SF_OS_SYMBIAN) && !defined(SF_CC_RENESAS) && !defined(SF_OS_PS2) && !defined(SF_CC_SNC)
#include <wctype.h>
#endif

namespace Scaleform {

#if defined(SF_OS_WIN32)
inline char* SF_CDECL SFitoa(int val, char *dest, UPInt destsize, int radix)
{
#if defined(SF_MSVC_SAFESTRING)
    _itoa_s(val, dest, destsize, radix);
    return dest;
#else
    SF_UNUSED(destsize);
    return itoa(val, dest, radix);
#endif
}
#else // SF_OS_WIN32
inline char* SFitoa(int val, char* dest, unsigned int len, int radix)
{
    if (val == 0)
    {
        if (len > 1)
        {
            dest[0] = '0';
            dest[1] = '\0';  
        }
        return dest;
    }

    int cur = val;
    unsigned int i    = 0; 
    unsigned int sign = 0;

    if (val < 0)
    {
        val = -val;
        sign = 1;
    }

    while ((val != 0) && (i < (len - 1 - sign)))        
    {
        cur    = val % radix;
        val   /= radix;

        if (radix == 16)
        {
            switch(cur)
            {
            case 10:
                dest[i] = 'a';
                break;
            case 11:
                dest[i] = 'b';
                break;
            case 12:
                dest[i] = 'c';
                break;
            case 13:
                dest[i] = 'd';
                break;
            case 14:
                dest[i] = 'e';
                break;
            case 15:
                dest[i] = 'f';
                break;
            default:
                dest[i] = (char)('0' + cur);
                break;
            }
        } 
        else
        {
            dest[i] = (char)('0' + cur);
        }
        ++i;
    }

    if (sign)
    {
        dest[i++] = '-';
    }

    for (unsigned int j = 0; j < i / 2; ++j)
    {
        char tmp        = dest[j];
        dest[j]         = dest[i - 1 - j];
        dest[i - 1 - j] = tmp;
    }
    dest[i] = '\0';

    return dest;
}

#endif


// String functions

inline UPInt SF_CDECL SFstrlen(const char* str)
{
    return strlen(str);
}

inline char* SF_CDECL SFstrcpy(char* dest, UPInt destsize, const char* src)
{
#if defined(SF_MSVC_SAFESTRING)
    strcpy_s(dest, destsize, src);
    return dest;
#else
    SF_UNUSED(destsize);
    return strcpy(dest, src);
#endif
}

inline char* SF_CDECL SFstrncpy(char* dest, UPInt destsize, const char* src, UPInt count)
{
#if defined(SF_MSVC_SAFESTRING)
    strncpy_s(dest, destsize, src, count);
    return dest;
#else
    SF_UNUSED(destsize);
    return strncpy(dest, src, count);
#endif
}

inline char * SF_CDECL SFstrcat(char* dest, UPInt destsize, const char* src)
{
#if defined(SF_MSVC_SAFESTRING)
    strcat_s(dest, destsize, src);
    return dest;
#else
    SF_UNUSED(destsize);
    return strcat(dest, src);
#endif
}

inline int SF_CDECL SFstrcmp(const char* dest, const char* src)
{
    return strcmp(dest, src);
}

inline const char* SF_CDECL SFstrchr(const char* str, char c)
{
    return strchr(str, c);
}

inline char* SF_CDECL SFstrchr(char* str, char c)
{
    return strchr(str, c);
}

inline const char* SFstrrchr(const char* str, char c)
{
    UPInt len = SFstrlen(str);
    for (UPInt i=len; i>0; i--)     
        if (str[i]==c) 
            return str+i;
    return str[0] == c ? str : 0;
}

inline const UByte* SF_CDECL SFmemrchr(const UByte* str, UPInt size, UByte c)
{
    for (SPInt i = (SPInt)size - 1; i >= 0; i--)     
    {
        if (str[i] == c) 
            return str + i;
    }
    return str[0] == c ? str : 0;
}

inline char* SF_CDECL SFstrrchr(char* str, char c)
{
    UPInt len = SFstrlen(str);
    for (UPInt i=len; i>0; i--)     
        if (str[i]==c) 
            return str+i;
    return str[0] == c ? str : 0;
}


double SF_CDECL SFstrtod(const char* string, char** tailptr);

inline long SF_CDECL SFstrtol(const char* string, char** tailptr, int radix)
{
    return strtol(string, tailptr, radix);
}

inline unsigned long SF_CDECL SFstrtoul(const char* string, char** tailptr, int radix)
{
    return strtoul(string, tailptr, radix);
}

inline int SF_CDECL SFstrncmp(const char* ws1, const char* ws2, UPInt size)
{
    return strncmp(ws1, ws2, size);
}

inline UInt64 SF_CDECL SFstrtouq(const char *nptr, char **endptr, int base)
{
#if defined(SF_CC_MSVC) && !defined(SF_OS_WINCE)
    return _strtoui64(nptr, endptr, base);
#else
    return strtoull(nptr, endptr, base);
#endif
}

inline SInt64 SF_CDECL SFstrtoq(const char *nptr, char **endptr, int base)
{
#if defined(SF_CC_MSVC) && !defined(SF_OS_WINCE)
    return _strtoi64(nptr, endptr, base);
#else
    return strtoll(nptr, endptr, base);
#endif
}


inline SInt64 SF_CDECL SFatoq(const char* string)
{
#if defined(SF_CC_MSVC) && !defined(SF_OS_WINCE)
    return _atoi64(string);
#else
    return atoll(string);
#endif
}

inline UInt64 SF_CDECL SFatouq(const char* string)
{
  return SFstrtouq(string, NULL, 10);
}

inline double SF_CDECL SFatof(const char* string)
{
    return atof(string);
}

// Implemented in GStd.cpp in platform-specific manner.
int SF_CDECL SFstricmp(const char* dest, const char* src);
int SF_CDECL SFstrnicmp(const char* dest, const char* src, UPInt count);
#if defined(SF_CC_MSVC) && defined(SF_MSVC_SAFESTRING)
    #define SFsscanf sscanf_s
#else
    #define SFsscanf sscanf
#endif

inline UPInt SF_CDECL SFsprintf(char *dest, UPInt destsize, const char* format, ...)
{
    va_list argList;
    va_start(argList,format);
    UPInt ret;
#if defined(SF_CC_MSVC)
    #if defined(SF_MSVC_SAFESTRING)
        int rv = _vsnprintf_s(dest, destsize, _TRUNCATE, format, argList);
        if (rv == -1)
        {
            dest[destsize - 1] = '\0';
            ret = destsize - 1;
        }
        else
            ret = (UPInt)rv;
    #else
        SF_UNUSED(destsize);
        ret = _vsnprintf(dest, destsize - 1, format, argList); // -1 for space for the null character
        SF_ASSERT(ret != -1);
        dest[destsize-1] = 0;
    #endif
#else
    SF_UNUSED(destsize);
    ret = vsprintf(dest, format, argList);
    SF_ASSERT(ret < destsize);
#endif
    va_end(argList);
    return ret;
}

inline UPInt SF_CDECL SFvsprintf(char *dest, UPInt destsize, const char * format, va_list argList)
{
    UPInt ret;
#if defined(SF_CC_MSVC)
    #if defined(SF_MSVC_SAFESTRING)
        dest[0] = '\0';
        int rv = vsnprintf_s(dest, destsize, _TRUNCATE, format, argList);
        if (rv == -1)
        {
            dest[destsize - 1] = '\0';
            ret = destsize - 1;
        }
        else
            ret = (UPInt)rv;
    #else
        int rv = _vsnprintf(dest, destsize - 1, format, argList);
        ret = (UPInt)rv;
        dest[destsize-1] = 0;
    #endif
#else
    int rv = vsnprintf(dest, destsize, format, argList);
    ret = (UPInt)rv;
    dest[destsize-1] = 0;
#endif
    return ret;
}

wchar_t* SF_CDECL SFwcscpy(wchar_t* dest, UPInt destsize, const wchar_t* src);
wchar_t* SF_CDECL SFwcsncpy(wchar_t* dest, UPInt destsize, const wchar_t* src, UPInt count);
wchar_t* SF_CDECL SFwcscat(wchar_t* dest, UPInt destsize, const wchar_t* src);
UPInt    SF_CDECL SFwcslen(const wchar_t* str);
int      SF_CDECL SFwcscmp(const wchar_t* a, const wchar_t* b);
int      SF_CDECL SFwcsicmp(const wchar_t* a, const wchar_t* b);

inline int SF_CDECL SFwcsicoll(const wchar_t* a, const wchar_t* b)
{
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || defined(SF_OS_WII)
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
    return ::_wcsicoll(a, b);
#else
    return ::wcsicoll(a, b);
#endif
#else
    // not supported, use regular wcsicmp
    return SFwcsicmp(a, b);
#endif
}

inline int SF_CDECL SFwcscoll(const wchar_t* a, const wchar_t* b)
{
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || defined(SF_OS_PS3) || defined(SF_OS_WII) || defined(SF_OS_LINUX)
    return wcscoll(a, b);
#else
    // not supported, use regular wcscmp
    return SFwcscmp(a, b);
#endif
}

inline int SF_CDECL UnicodeCharIs(const UInt16* table, wchar_t charCode)
{
    unsigned offset = table[charCode >> 8];
    if (offset == 0) return 0;
    if (offset == 1) return 1;
    return (table[offset + ((charCode >> 4) & 15)] & (1 << (charCode & 15))) != 0;
}

#ifndef SF_NO_WCTYPE

extern const UInt16 UnicodeAlnumBits[];
extern const UInt16 UnicodeAlphaBits[];
extern const UInt16 UnicodeDigitBits[];
extern const UInt16 UnicodeSpaceBits[];
extern const UInt16 UnicodeXDigitBits[];

// Uncomment if necessary
//extern const UInt16 UnicodeCntrlBits[];
//extern const UInt16 UnicodeGraphBits[];
//extern const UInt16 UnicodeLowerBits[];
//extern const UInt16 UnicodePrintBits[];
//extern const UInt16 UnicodePunctBits[];
//extern const UInt16 UnicodeUpperBits[];

inline int SF_CDECL SFiswalnum (wchar_t charCode) { return UnicodeCharIs(UnicodeAlnumBits,  charCode); }
inline int SF_CDECL SFiswalpha (wchar_t charCode) { return UnicodeCharIs(UnicodeAlphaBits,  charCode); }
inline int SF_CDECL SFiswdigit (wchar_t charCode) { return UnicodeCharIs(UnicodeDigitBits,  charCode); }
inline int SF_CDECL SFiswspace (wchar_t charCode) { return UnicodeCharIs(UnicodeSpaceBits,  charCode); }
inline int SF_CDECL SFiswxdigit(wchar_t charCode) { return UnicodeCharIs(UnicodeXDigitBits, charCode); }

// Uncomment if necessary
//inline int SF_CDECL SFiswcntrl (wchar_t charCode) { return UnicodeCharIs(UnicodeCntrlBits,  charCode); }
//inline int SF_CDECL SFiswgraph (wchar_t charCode) { return UnicodeCharIs(UnicodeGraphBits,  charCode); }
//inline int SF_CDECL SFiswlower (wchar_t charCode) { return UnicodeCharIs(UnicodeLowerBits,  charCode); }
//inline int SF_CDECL SFiswprint (wchar_t charCode) { return UnicodeCharIs(UnicodePrintBits,  charCode); }
//inline int SF_CDECL SFiswpunct (wchar_t charCode) { return UnicodeCharIs(UnicodePunctBits,  charCode); }
//inline int SF_CDECL SFiswupper (wchar_t charCode) { return UnicodeCharIs(UnicodeUpperBits,  charCode); }

int SF_CDECL SFtowupper(wchar_t charCode);
int SF_CDECL SFtowlower(wchar_t charCode);

#else // SF_NO_WCTYPE

inline int SF_CDECL SFiswspace(wchar_t c)
{
#if defined(SF_OS_WII) || defined(SF_CC_SNC)
    return ((c) < 128 ? isspace((char)c) : 0);
#else
    return iswspace(c);
#endif
}

inline int SF_CDECL SFiswdigit(wchar_t c)
{
#if defined(SF_OS_WII) || defined(SF_CC_SNC)
    return ((c) < 128 ? isdigit((char)c) : 0);
#else
    return iswdigit(c);
#endif
}

inline int SF_CDECL SFiswxdigit(wchar_t c)
{
#if defined(SF_OS_WII) || defined(SF_CC_SNC)
    return ((c) < 128 ? isxdigit((char)c) : 0);
#else
    return iswxdigit(c);
#endif
}

inline int SF_CDECL SFiswalpha(wchar_t c)
{
#if defined(SF_OS_WII) || defined(SF_CC_SNC)
    return ((c) < 128 ? isalpha((char)c) : 0);
#else
    return iswalpha(c);
#endif
}

inline int SF_CDECL SFiswalnum(wchar_t c)
{
#if defined(SF_OS_WII) || defined(SF_CC_SNC)
    return ((c) < 128 ? isalnum((char)c) : 0);
#else
    return iswalnum(c);
#endif
}

inline wchar_t SF_CDECL SFtowlower(wchar_t c)
{
#if (defined(SF_OS_PS3) && defined(SF_CC_SNC))
    return (wchar_t)tolower((char)c);
#else
    return (wchar_t)towlower(c);
#endif
}

inline wchar_t SFtowupper(wchar_t c)
{
#if (defined(SF_OS_PS3) && defined(SF_CC_SNC))
    return (wchar_t)toupper((char)c);
#else
    return (wchar_t)towupper(c);
#endif
}

#endif // SF_NO_WCTYPE

// ASCII versions of tolower and toupper. Don't use "char"
inline int SF_CDECL SFtolower(int c)
{
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

inline int SF_CDECL SFtoupper(int c)
{
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}



inline double SF_CDECL SFwcstod(const wchar_t* string, wchar_t** tailptr)
{
#if defined(SF_OS_OTHER)
    SF_UNUSED(tailptr);
    char buffer[64];
    char* tp = NULL;
    UPInt max = SFwcslen(string);
    if (max > 63) max = 63;
    unsigned char c = 0;
    for (UPInt i=0; i < max; i++)
    {
        c = (unsigned char)string[i];
        buffer[i] = ((c) < 128 ? (char)c : '!');
    }
    buffer[max] = 0;
    return SFstrtod(buffer, &tp);
#else
    return wcstod(string, tailptr);
#endif
}

inline long SF_CDECL SFwcstol(const wchar_t* string, wchar_t** tailptr, int radix)
{
#if defined(SF_OS_OTHER)
    SF_UNUSED(tailptr);
    char buffer[64];
    char* tp = NULL;
    UPInt max = SFwcslen(string);
    if (max > 63) max = 63;
    unsigned char c = 0;
    for (UPInt i=0; i < max; i++)
    {
        c = (unsigned char)string[i];
        buffer[i] = ((c) < 128 ? (char)c : '!');
    }
    buffer[max] = 0;
    return strtol(buffer, &tp, radix);
#else
    return wcstol(string, tailptr, radix);
#endif
}

extern const UInt16 UnicodeNonSpacingMarksBits[];
extern const UInt16 UnicodeDirectionNeutralBits[];
extern const UInt16 UnicodeRightToLeftBits[];

// returns true, if char is a non-spacing mark ('Mn', see UnicodeData.txt)
inline int SF_CDECL SFisNonSpacingMark(wchar_t charCode) { return UnicodeCharIs(UnicodeNonSpacingMarksBits, charCode); }

// returns true, if char belongs to right-to-left language (Arabic/Hebrew). ('R' or 'AL' in UnicodeData.txt)
inline int SF_CDECL SFisRightToLeft(wchar_t charCode) { return UnicodeCharIs(UnicodeRightToLeftBits, charCode); }

// returns true, if char is directionally neutral ('B', 'S', 'WS' or 'ON' in UnicodeData.txt)
inline int SF_CDECL SFisDirNeutral(wchar_t charCode) { return UnicodeCharIs(UnicodeDirectionNeutralBits, charCode); }

} // Scaleform

#endif // INC_GSTD_H
