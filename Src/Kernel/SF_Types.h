/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Types.h
Content     :   Standard library defines and simple types
Created     :   June 28, 1998
Authors     :   Michael Antonov, Brendan Iribe, Maxim Shemanarev

Notes       :   This file represents all the standard types
                to be used. These types are defined for compiler
                independence from size of types.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Types_H
#define INC_SF_Kernel_Types_H

// Default version file
#ifndef SF_PRODUCT_VERSION
#define SF_PRODUCT_VERSION "GFxVersion.h"
#endif

#include SF_PRODUCT_VERSION


// ****** Operating System
//
//---------------------------------------------------------------------------
// Scaleform works on the following operating systems: (SF_OS_x)
//
//    WIN32    - Win32 (Windows 95/98/ME and Windows NT/2000/XP)
//    DARWIN   - Darwin OS (Mac OS X)
//    LINUX    - Linux
//    ANDROID  - Android
//    IPHONE   - iPhone
//    XBOX360  - Xbox 360 console
//    PS3      - Playstation 3 console
//    PSVITA   - PSVITA handheld console
//    WII      - Wii console
//    3DS      - 3DS handheld console

#if (defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))) || defined(__MACOS__)
#if (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) || defined(__IPHONE_OS_VERSION_MIN_REQUIRED))
#define SF_OS_IPHONE
#else
# define SF_OS_DARWIN
# define SF_OS_MAC
#endif
#elif defined(_XBOX)
# if defined(NOD3D) || defined(NONET)
#  error Cannot have NOD3D or NONET defined when including Scaleform GFx headers.
# endif
# include <xtl.h>
// Xbox360 and XBox both share _XBOX definition
#if (_XBOX_VER >= 200)
# define SF_OS_XBOX360
#endif
#elif (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
# define SF_OS_WIN32
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
# define SF_OS_WIN32
#elif defined(__MWERKS__) && defined(__INTEL__)
# define SF_OS_WIN32
#elif defined(__linux__) || defined(__linux)
# define SF_OS_LINUX
#elif defined(__PPU__)
# define SF_OS_PS3
#elif defined(__ORBIS__)
# define SF_OS_ORBIS
#elif defined(RVL)
# define SF_OS_WII
#elif defined(__psp2__)
# define SF_OS_PSVITA
#elif defined(NN_PLATFORM_CTR)
# define SF_OS_3DS
#elif defined(CAFE) && !defined (SF_OS_WIIU)
# define SF_OS_WIIU
#else
# define SF_OS_OTHER
#endif

#if defined(ANDROID)
 # define SF_OS_ANDROID
#endif


// ***** CPU Architecture
//
//---------------------------------------------------------------------------
// Scaleform supports the following CPUs: (SF_CPU_x)
//
//    X86        - x86 (IA-32)
//    X86_64     - x86_64 (amd64)
//    PPC        - PowerPC
//    PPC64      - PowerPC64
//    MIPS       - MIPS
//    ARM        - ARM based CPU
//    OTHER      - CPU for which no special support is present or needed


#if defined(__x86_64__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define SF_CPU_X86_64
#  define SF_64BIT_POINTERS
#elif defined(__i386__) || (defined(SF_OS_WIN32) && !defined(_M_ARM_FP)) || defined(SF_OS_XBOX)
#  define SF_CPU_X86
#elif defined(__powerpc64__) || defined(SF_OS_PS3) || defined(SF_OS_XBOX360) || defined(SF_OS_WII) || defined(SF_OS_WIIU)
#  define SF_CPU_PPC64
// Note: PS3, x360 and WII don't use 64-bit pointers.
#elif defined(__ppc__)
#  define SF_CPU_PPC
#elif defined(__mips__) || defined(__MIPSEL__)
#  define SF_CPU_MIPS
#elif defined(__arm__) || defined(_M_ARM_FP)
#  define SF_CPU_ARM
#else
#  define SF_CPU_OTHER
#endif

// ***** Coprocessor Architecture
//
//---------------------------------------------------------------------------
// Scaleform supports the following CPUs: (SF_CPU_x)
//
//    SSE        - Available on all modern x86 processors.
//    Altivec    - Available on all modern ppc processors (including PS3).
//                 X360 uses non-standard headers/defines, but does implement
//                 an extended Altivec feature set.
//    Neon       - Available on some armv7+ processors.

#if defined(__SSE__) || (defined(SF_OS_WIN32) && !defined(_M_ARM_FP))
#  define  SF_CPU_SSE
#endif // __SSE__

#if defined( __ALTIVEC__ ) && !defined(SF_OS_WII)
#  define SF_CPU_ALTIVEC
#endif // __ALTIVEC__

#if defined(__ARM_NEON__)
#   define SF_CPU_ARM_NEON
#endif // __ARM_NEON__


// ***** Include Config.h
//
//---------------------------------------------------------------------------
#ifndef SF_CONFIG_H
#define SF_CONFIG_H "GFxConfig.h"
#endif

// Include conditional compilation options file
// Needs to be after OS detection code
#include SF_CONFIG_H


// ***** Compiler
//
//---------------------------------------------------------------------------
//  Scaleform is compatible with the following compilers: (SF_CC_x)
//
//     MSVC     - Microsoft Visual C/C++
//     INTEL    - Intel C++ for Linux / Windows
//     GNU      - GNU C++
//     SNC      - SN Systems ProDG C/C++ (EDG)
//     MWERKS   - Metrowerks CodeWarrior
//     BORLAND  - Borland C++ / C++ Builder
//     ARM      - ARM C/C++
//     LLVM-GCC - Low-Level Virtual Machine plugin for GCC.

#if defined(__INTEL_COMPILER)
// Intel 4.0                    = 400
// Intel 5.0                    = 500
// Intel 6.0                    = 600
// Intel 8.0                    = 800
// Intel 9.0                    = 900
# define SF_CC_INTEL       __INTEL_COMPILER

#elif defined(_MSC_VER)
// MSVC 5.0                     = 1100
// MSVC 6.0                     = 1200
// MSVC 7.0 (VC2002)            = 1300
// MSVC 7.1 (VC2003)            = 1310
// MSVC 8.0 (VC2005)            = 1400
// MSVC 9.0 (VC2008)            = 1500
// MSVC 10.0 (VC2010)           = 1600
// MSVC 11.0 (VC2012)           = 1700
// MSVC 12.0 (VC2013)           = 1800
# define SF_CC_MSVC        _MSC_VER

#elif defined(__MWERKS__)
// Metrowerks C/C++ 2.0         = 0x2000
// Metrowerks C/C++ 2.2         = 0x2200
# define SF_CC_MWERKS      __MWERKS__

#elif defined(__BORLANDC__) || defined(__TURBOC__)
// Borland C++ 5.0              = 0x500
// Borland C++ 5.02             = 0x520
// C++ Builder 1.0              = 0x520
// C++ Builder 3.0              = 0x530
// C++ Builder 4.0              = 0x540
// Borland C++ 5.5              = 0x550
// C++ Builder 5.0              = 0x550
// Borland C++ 5.51             = 0x551
# define SF_CC_BORLAND     __BORLANDC__

#elif defined(__ghs__)
# define SF_CC_GHS

#elif defined(__clang__)
# define SF_CC_CLANG

// SNC must come before GNUC because
// the SNC compiler defines GNUC as well
#elif defined(__SNC__)
# define SF_CC_SNC

#elif defined(__GNUC__)
# define SF_CC_GNU
# if defined(__llvm__) && __llvm__
#   define SF_CC_LLVMGCC
# endif

#elif defined(__CC_ARM)
# define SF_CC_ARM

#else
# error "SF does not support this Compiler - contact support@scaleform.com"
#endif


// Ignore compiler warnings specific to SN compiler
#if defined(SF_CC_SNC)
# pragma diag_suppress=68   // integer conversion resulted in a change of sign
# pragma diag_suppress=112  // statement is unreachable
// (possibly due to SN disliking modifying a mutable object in a const function)
# pragma diag_suppress=175  // expression has no effect
// (deliberately placed so it would NOT produce a warning...)
# pragma diag_suppress=178  // variable was declared but never referenced
# pragma diag_suppress=382  // extra ";" ignored
# pragma diag_suppress=552  // variable was set but never used
# pragma diag_suppress=613  // overloaded virtual function is only partially overridden in class
# pragma diag_suppress=999  // function funcA is hidden by funcB -- virtual function override intended?

# pragma diag_suppress=1011 // function funcA is hidden by funcB -- virtual function override intended?
# pragma diag_suppress=1421 // trigraphs (in comments)
# pragma diag_suppress=1787 // invalid value for enumeration type
# ifdef SF_OS_PSVITA
#  pragma diag_suppress=1783 // cast increases required alignment
# endif
#endif


// *** Linux Unicode - must come before Standard Includes

#ifdef SF_OS_LINUX
/* Use glibc unicode functions on linux. */
#ifndef  _GNU_SOURCE
# define _GNU_SOURCE
#endif
#endif

// *** Symbian - fix defective header files

#ifdef SF_OS_SYMBIAN
#include <_ansi.h>
#undef _STRICT_ANSI
#include <stdlib.h>
#endif

#ifdef SF_OS_WII
#define Ptr PtrXXX
#include <revolution/types.h>
#undef Ptr
#include <types.h>
#endif

#ifdef SF_OS_3DS
#include <nn/types.h>
#endif

#ifdef SF_OS_WIIU
#include "SF_TypesWiiU.h"
#endif

// ***** Standard Includes
//
//---------------------------------------------------------------------------
#include    <stddef.h>
#include    <limits.h>
#include    <float.h>

// This macro needs to be defined if it is necessary to avoid the use of Double.
// In that case Double in defined as float and thus extra #ifdef checks on
// overloads need to be done. Useful for platforms with poor/unavailable
// Double support


// *****  Definitions
//
//---------------------------------------------------------------------------
// Byte order
#define SF_LITTLE_ENDIAN       1
#define SF_BIG_ENDIAN          2


namespace Scaleform {

// Compiler specific integer
//typedef int                 int ;
//typedef unsigned int        unsigned;

// Pointer-size integer
typedef size_t              UPInt;
typedef ptrdiff_t           SPInt;

} // namespace Scaleform {

// **** Win32 or XBox
//-----------------------
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)

// ** Miscellaneous compiler definitions

// DLL linking options
//-----------------------
#ifdef SF_BUILD_STATICLIB
// BUILD_STATICLIB means no DLL support, so disable symbol exports
# define    SF_IMPORT
# define    SF_EXPORT
#else
# define    SF_IMPORT   __declspec(dllimport)
# define    SF_EXPORT   __declspec(dllexport)
#endif // SF_BUILD_STATICLIB

#define SF_EXPORTC      extern "C" SF_EXPORT

// Build is used for importing (must be defined to nothing)
#define SF_BUILD

// Calling convention - goes after function return type but before function name
#ifdef __cplusplus_cli
    #define SF_FASTCALL         __stdcall
#else
    #define SF_FASTCALL         __fastcall
#endif

#define SF_STDCALL              __stdcall
#define SF_CDECL                __cdecl

// Byte order
#if defined(SF_OS_XBOX360)
#define SF_BYTE_ORDER          SF_BIG_ENDIAN
#else
#define SF_BYTE_ORDER          SF_LITTLE_ENDIAN
#endif

#if defined(SF_CC_MSVC)
// Disable "inconsistent dll linkage" warning
# pragma warning(disable : 4127)
// Disable "exception handling" warning
# pragma warning(disable : 4530)
# if (SF_CC_MSVC<1300)
// Disable "unreferenced inline function has been removed" warning
# pragma warning(disable : 4514)
// Disable "function not inlined" warning
# pragma warning(disable : 4710)
// Disable "_force_inline not inlined" warning
# pragma warning(disable : 4714)
// Disable "debug variable name longer than 255 chars" warning
# pragma warning(disable : 4786)
# endif // (SF_CC_MSVC<1300)
#endif // (SF_CC_MSVC)

// Assembly macros
#if defined(SF_CC_MSVC)
#define SF_ASM                _asm
#else
#define SF_ASM                asm
#endif // (SF_CC_MSVC)

// Inline substitute - goes before function declaration
#if defined(SF_CC_MSVC)
# define SF_INLINE          __forceinline
# define SF_NOINLINE        __declspec(noinline)
#elif defined(SF_CC_GNU)
# define SF_INLINE          __attribute__((always_inline))
# define SF_NOINLINE        __attribute__((noinline))
#else
# define SF_INLINE          inline
# define SF_NOINLINE
#endif  // SF_CC_MSVC


#ifdef UNICODE
#define SF_STR(str) L##str
#else
#define SF_STR(str) str
#endif // UNICODE

#else 

#if defined (SF_OS_ANDROID)
#define SF_BYTE_ORDER          SF_LITTLE_ENDIAN
#elif (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN))|| \
    (defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN))
#define SF_BYTE_ORDER          SF_BIG_ENDIAN
#elif (defined(SF_OS_PS3) || defined(__ARMEB__) || defined(SF_CPU_PPC) || defined(SF_CPU_PPC64))
#define SF_BYTE_ORDER          SF_BIG_ENDIAN
#elif defined (SF_OS_WII) || defined(SF_OS_WIIU)
#define SF_BYTE_ORDER          SF_BIG_ENDIAN

#else
#define SF_BYTE_ORDER          SF_LITTLE_ENDIAN

#endif

// **** Standard systems

// ** Miscellaneous compiler definitions

// DLL linking options
#define SF_IMPORT
#define SF_EXPORT
#define SF_EXPORTC              extern "C" SF_EXPORT

// Build is used for importing, defined to nothing
#define SF_BUILD

// Assembly macros
#define SF_ASM                  __asm__
#define SF_ASM_PROC(procname)   SF_ASM
#define SF_ASM_END              SF_ASM

// Inline substitute - goes before function declaration
#define SF_INLINE               inline
#define SF_NOINLINE

// Calling convention - goes after function return type but before function name
#define SF_FASTCALL
#define SF_STDCALL
#define SF_CDECL

#endif // defined(SF_OS_WIN32) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360) || defined(SF_OS_WINCE)

// ***** Type definitions for Win32
//---------------------------------------------------------------------------
namespace Scaleform {

typedef char                Char;

// *** Type definitions for common systems

typedef char                Char;

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
// 8 bit Integer (Byte)
typedef char                SInt8;
typedef unsigned char       UInt8;
typedef SInt8               SByte;
typedef UInt8               UByte;

// 16 bit Integer (Word)
typedef short               SInt16;
typedef unsigned short      UInt16;

// 32 bit Integer (DWord)
// #if defined(SF_OS_LINUX) && defined(SF_CPU_X86_64)
// typedef int                 SInt32;
// typedef unsigned int        UInt32;
// #else
// typedef long                SInt32;
// typedef unsigned long       UInt32;
// #endif

typedef long                SInt32;
typedef unsigned long       UInt32;

// 64 bit Integer (QWord)
typedef __int64             SInt64;
typedef unsigned __int64    UInt64;

#elif defined(SF_OS_MAC) || defined(SF_OS_IPHONE)

typedef int          SInt8  __attribute__((__mode__ (__QI__)));
typedef unsigned int UInt8  __attribute__((__mode__ (__QI__)));
typedef int          SByte  __attribute__((__mode__ (__QI__)));
typedef unsigned int UByte  __attribute__((__mode__ (__QI__)));

typedef int          SInt16 __attribute__((__mode__ (__HI__)));
typedef unsigned int UInt16 __attribute__((__mode__ (__HI__)));

typedef int          SInt32 __attribute__((__mode__ (__SI__)));
typedef unsigned int UInt32 __attribute__((__mode__ (__SI__)));

typedef int          SInt64 __attribute__((__mode__ (__DI__)));
typedef unsigned int UInt64 __attribute__((__mode__ (__DI__)));

#elif defined(SF_CC_GNU) || defined(SF_CC_SNC) || defined(SF_OS_ORBIS)

typedef int          SInt8  __attribute__((__mode__ (__QI__)));
typedef unsigned int UInt8  __attribute__((__mode__ (__QI__)));
typedef int          SByte  __attribute__((__mode__ (__QI__)));
typedef unsigned int UByte  __attribute__((__mode__ (__QI__)));

typedef int          SInt16 __attribute__((__mode__ (__HI__)));
typedef unsigned int UInt16 __attribute__((__mode__ (__HI__)));

typedef int          SInt32 __attribute__((__mode__ (__SI__)));
typedef unsigned int UInt32 __attribute__((__mode__ (__SI__)));

typedef int          SInt64 __attribute__((__mode__ (__DI__)));
typedef unsigned int UInt64 __attribute__((__mode__ (__DI__)));


#elif defined(SF_OS_WII) || defined(SF_OS_3DS) || defined(SF_OS_WIIU)

// 8 bit Integer (Byte)
typedef s8              SInt8;
typedef u8              UInt8;
typedef s8              SByte;
typedef u8              UByte;

// 16 bit Integer
typedef s16             SInt16;
typedef u16             UInt16;

// 32 bit Integer
typedef s32             SInt32;
typedef u32             UInt32;

// 64 bit Integer
typedef s64             SInt64;
typedef u64             UInt64;

#else

} // Scaleform
#include <sys/types.h>
#ifdef SF_OS_LINUX
#include <cstdint>
#endif
namespace Scaleform {

// 8 bit Integer (Byte)
typedef int8_t              SInt8;
typedef uint8_t             UInt8;
typedef int8_t              SByte;
typedef uint8_t             UByte;

// 16 bit Integer
typedef int16_t             SInt16;
typedef uint16_t            UInt16;

// 32 bit Integer
typedef int32_t             SInt32;
typedef uint32_t            UInt32;

// 64 bit Integer
typedef int64_t             SInt64;
typedef uint64_t            UInt64;

#endif

//
// ***** BaseTypes Namespace

// BaseTypes namespace is explicitly declared to allow base types to be used
// by customers directly without other contents of Scaleform namespace.
//
// Its is expected that GFx samples will declare 'using namespace Scaleform::BaseTypes'
// to allow using these directly without polluting the target scope with other
// GFx/Scaleform declarations, such as Ptr<>, String or Mutex.
namespace BaseTypes
{
    //using Scaleform::unsigned;
    //using Scaleform::int ;
    using Scaleform::UPInt;
    using Scaleform::SPInt;
    using Scaleform::UByte;
    using Scaleform::SByte;
    using Scaleform::UInt16;
    using Scaleform::SInt16;
    using Scaleform::UInt32;
    using Scaleform::SInt32;
    using Scaleform::UInt64;
    using Scaleform::SInt64;
} // BaseTypes

} // Scaleform


// ***** SF_DEBUG_BREAK, SF_ASSERT
//
//---------------------------------------------------------------------------
// If not in debug build, macros do nothing
#ifndef SF_BUILD_DEBUG

    #define SF_DEBUG_BREAK  ((void)0)
    #define SF_ASSERT(p)    ((void)0)

#else 

// Microsoft Win32 specific debugging support
//#if (defined(SF_OS_WIN32) && !defined(__cplusplus_cli)) || defined(SF_OS_XBOX)
#if defined(SF_OS_WIN32)
#   ifdef SF_CPU_X86
#       if defined(__cplusplus_cli)
            #define SF_DEBUG_BREAK do { __debugbreak(); } while(0)
#       elif defined(SF_CC_GNU)
            #define SF_DEBUG_BREAK do { SF_ASM("int $3\n\t"); } while(0)
#       else
            #define SF_DEBUG_BREAK do { SF_ASM int 3 } while (0)
#       endif
#   else
        #define SF_DEBUG_BREAK     do { __debugbreak(); } while(0)
#   endif
// Xbox360 specific debugging
#elif defined(SF_OS_XBOX360)    
    #define SF_DEBUG_BREAK         DebugBreak()
    // Special assert for X360 static code analysis.
    #if defined(_PREFAST_)
        #define SF_ASSERT(p)       do { BOOL res = (BOOL)((p) != 0); __analysis_assume(res); if (!(res))  { SF_DEBUG_BREAK; } } while(0)
    #endif
#elif defined(SF_OS_ORBIS)
    #include <libdbg.h>
    #define SF_DEBUG_BREAK     do { SCE_BREAK(); } while(0)
#elif defined(SF_CC_CLANG)
    #define SF_DEBUG_BREAK     do { __builtin_trap(); } while(0)
#elif defined(SF_CPU_X86)
    #define SF_DEBUG_BREAK     do { SF_ASM("int $3\n\t"); } while(0)
#elif defined(SF_OS_3DS)
    #include <nn/dbg.h>
    #define SF_DEBUG_BREAK     do { nn::dbg::Break(nn::dbg::BREAK_REASON_USER); } while(0)
#elif defined(SF_OS_WIIU)
    #define SF_DEBUG_BREAK     do { OSDebug(); } while(0)
#else
    #define SF_DEBUG_BREAK     do { *((int *) 0) = 1; } while(0)
#endif

// This will cause compiler breakpoint
#ifndef SF_ASSERT
    #define SF_ASSERT(p)       do { if (!(p))  { SF_DEBUG_BREAK; } } while(0)
#endif

#endif // SF_BUILD_DEBUG



// Handy macro to quiet compiler warnings about unused parameters/variables.
#if defined(SF_CC_GNU) || defined(SF_CC_CLANG)
#define     SF_UNUSED_ATTR        __attribute__ ((unused))
#define     SF_UNUSED(a)          do {__typeof__ (&a) __attribute__ ((unused)) __tmp = &a; } while(0)
#define     SF_UNUSED2(a,b)       SF_UNUSED(a); SF_UNUSED(b)
#define     SF_UNUSED3(a,b,c)     SF_UNUSED2(a,c); SF_UNUSED(b)
#define     SF_UNUSED4(a,b,c,d)   SF_UNUSED3(a,c,d); SF_UNUSED(b)
#else
#define     SF_UNUSED_ATTR
#define     SF_UNUSED(a)          (a)
#define     SF_UNUSED2(a,b)       (a);(b)
#define     SF_UNUSED3(a,b,c)     (a);(b);(c)
#define     SF_UNUSED4(a,b,c,d)   (a);(b);(c);(d)
#endif

#define     SF_UNUSED1(a) SF_UNUSED(a)
#define     SF_UNUSED5(a1,a2,a3,a4,a5) SF_UNUSED4(a1,a2,a3,a4); SF_UNUSED(a5)
#define     SF_UNUSED6(a1,a2,a3,a4,a5,a6) SF_UNUSED4(a1,a2,a3,a4); SF_UNUSED2(a5,a6)
#define     SF_UNUSED7(a1,a2,a3,a4,a5,a6,a7) SF_UNUSED4(a1,a2,a3,a4); SF_UNUSED3(a5,a6,a7)
#define     SF_UNUSED8(a1,a2,a3,a4,a5,a6,a7,a8) SF_UNUSED4(a1,a2,a3,a4); SF_UNUSED4(a5,a6,a7,a8)
#define     SF_UNUSED9(a1,a2,a3,a4,a5,a6,a7,a8,a9) SF_UNUSED4(a1,a2,a3,a4); SF_UNUSED5(a5,a6,a7,a8,a9)


// ***** Compiler Assert
//
//---------------------------------------------------------------------------
// Compile-time assert.  Thanks to Jon Jagger (http://www.jaggersoft.com) for this trick.
// Improved from stack overflow (http://stackoverflow.com/questions/807244/c-compiler-asserts-how-to-implement)
#define SF_COMPILER_ASSERT(predicate) _impl_CASSERT_LINE(predicate,__LINE__,__FILE__)
#define _impl_PASTE(a,b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1] SF_UNUSED_ATTR;

// ******** Variable range definitions
//
//---------------------------------------------------------------------------
#if defined(SF_CC_MSVC)
#define SF_UINT64(x) x
#else
#define SF_UINT64(x) x##LL
#endif

// 8 bit Integer ranges (Byte)
#define SF_MAX_SINT8           Scaleform::SInt8(0x7F)                   //  127
#define SF_MIN_SINT8           Scaleform::SInt8(0x80)                   // -128
#define SF_MAX_UINT8           Scaleform::UInt8(0xFF)                   //  255
#define SF_MIN_UINT8           Scaleform::UInt8(0x00)                   //  0

#define SF_MAX_SBYTE           SF_MAX_SINT8                             //  127
#define SF_MIN_SBYTE           SF_MIN_SINT8                             // -128
#define SF_MAX_UBYTE           SF_MAX_UINT8                             //  255
#define SF_MIN_UBYTE           SF_MIN_UINT8                             //  0

// 16 bit Integer ranges (Word)
#define SF_MAX_SINT16          Scaleform::SInt16(SHRT_MAX)              //  32767
#define SF_MIN_SINT16          Scaleform::SInt16(SHRT_MIN)              // -32768
#define SF_MAX_UINT16          Scaleform::UInt16(USHRT_MAX)             //  65535
#define SF_MIN_UINT16          Scaleform::UInt16(0)                     //  0

// 32 bit Integer ranges (Int)
#define SF_MAX_SINT32          Scaleform::SInt32(INT_MAX)               //  2147483647
#define SF_MIN_SINT32          Scaleform::SInt32(INT_MIN)               // -2147483648
#define SF_MAX_UINT32          Scaleform::UInt32(UINT_MAX)              //  4294967295
#define SF_MIN_UINT32          Scaleform::UInt32(0)                     //  0

// 64 bit Integer ranges (Long)
#define SF_MAX_SINT64          Scaleform::SInt64(0x7FFFFFFFFFFFFFFF)    // -9223372036854775808
#define SF_MIN_SINT64          Scaleform::SInt64(0x8000000000000000)    //  9223372036854775807
#define SF_MAX_UINT64          Scaleform::UInt64(0xFFFFFFFFFFFFFFFF)    //  18446744073709551615
#define SF_MIN_UINT64          Scaleform::UInt64(0)                     //  0

// Compiler specific
#define SF_MAX_SINT            int(SF_MAX_SINT32)                       //  2147483647
#define SF_MIN_SINT            int(SF_MIN_SINT32)                       // -2147483648
#define SF_MAX_UINT            unsigned(SF_MAX_UINT32)                  //  4294967295
#define SF_MIN_UINT            unsigned(SF_MIN_UINT32)                  //  0


#if defined(SF_64BIT_POINTERS)
#define SF_MAX_SPINT           SF_MAX_SINT64
#define SF_MIN_SPINT           SF_MIN_SINT64
#define SF_MAX_UPINT           SF_MAX_UINT64
#define SF_MIN_UPINT           SF_MIN_UINT64
#else
#define SF_MAX_SPINT           SF_MAX_SINT
#define SF_MIN_SPINT           SF_MIN_SINT
#define SF_MAX_UPINT           SF_MAX_UINT
#define SF_MIN_UPINT           SF_MIN_UINT
#endif

// ***** Floating point ranges
//
//---------------------------------------------------------------------------
// Minimum and maximum (positive) float values
#define SF_MIN_FLOAT           float(FLT_MIN)               // 1.175494351e-38F
#define SF_MAX_FLOAT           float(FLT_MAX)               // 3.402823466e+38F

// Minimum and maximum (positive) Double values
#define SF_MIN_DOUBLE          double(DBL_MIN)   // 2.2250738585072014e-308
#define SF_MAX_DOUBLE          double(DBL_MAX)   // 1.7976931348623158e+308

// ***** Double
namespace Scaleform {

    typedef double  Double;

namespace BaseTypes {
    using Scaleform::Double;
}

} // namespace Scaleform {


// ***** Flags
//
//---------------------------------------------------------------------------
#define SF_FLAG32(value)       (Scaleform::UInt32(value))
#define SF_FLAG64(value)       (Scaleform::UInt64(value))


// ***** Replacement new overrides
//
//---------------------------------------------------------------------------
// Calls constructor on own memory created with "new(ptr) type"
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

#if defined(SF_CC_MSVC)
SF_INLINE void*   operator new        (Scaleform::UPInt n, void *ptr)   { return ptr; SF_UNUSED(n); }
SF_INLINE void    operator delete     (void *ptr, void *ptr2)           { return; SF_UNUSED2(ptr,ptr2); }
#else
// Needed for placement on many platforms including PSP.
#include <new>
#endif

#endif // __PLACEMENT_NEW_INLINE


// ***** Special Documentation Set
//---------------------------------------------------------------------------
// SF Build type
#ifdef SF_BUILD_DEBUG
  #ifdef SF_BUILD_DEBUGOPT
    #define SF_BUILD_STRING         "DebugOpt"
  #else
    #define SF_BUILD_STRING         "Debug"
  #endif
#else
  #define SF_BUILD_STRING           "Release"
#endif


// Questinable
//#ifdef SF_DOM_INCLUDE
//
//// Enables Windows MFC Compatibility
//# define SF_BUILD_MFC
//
//// STL configuration
//// - default is STLSTD if not set
//# define SF_BUILD_STLPORT
//# define SF_BUILD_STLSGI
//# define SF_BUILD_STLSTD
//
//// Enables SF Debugging information
//# define SF_BUILD_DEBUG
//
//// Enables SF Static library build
//# define SF_BUILD_STATICLIB
//
//// Causes the SF Memory allocator
//// to be used for all new and delete's
//# define SF_BUILD_NEW_OVERRIDE
//
//// Prevents SF from defining new within
//// type macros, so developers can override
//// new using the #define new new(...) trick
//// - used with SF_DEFINE_NEW macro
//# define SF_BUILD_DEFINE_NEW
//
//// Enables SF Purify customized build
//# define SF_BUILD_PURIFY
//// Enables SF Evaluation build with time locking
//# define SF_BUILD_EVAL
//
//#endif

namespace Scaleform { 

template <typename T1, typename T2>
struct Pair
{
    typedef T1 FirstType;
    typedef T2 SecondType;

    Pair(const FirstType& f, const SecondType& s) : First(f), Second(s) {}

    T1 First;
    T2 Second;
};

} //namespace Scaleform { 

#endif  // INC_GTYPES_H
