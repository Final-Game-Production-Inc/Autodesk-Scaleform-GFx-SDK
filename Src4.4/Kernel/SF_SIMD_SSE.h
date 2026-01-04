/**************************************************************************

Filename    :   SF_SIMD_SSE.h
Content     :   SIMD instruction set(s) for SSE.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_SF_SIMD_SSE_H
#define INC_SF_SF_SIMD_SSE_H
#pragma once

#if defined(SF_ENABLE_SIMD) && defined(SF_CPU_SSE)

#include <emmintrin.h>

#if defined(_MSC_VER)
    // MSVC style alignment.
    #pragma warning (disable : 4324) // warning C4324: 'struct' : structure was padded due to __declspec(align())
    #define SF_SIMD_ALIGN(x)  __declspec(align(16)) x
    #ifndef _WIN64
        #include <intrin.h>
    #endif
#elif defined(__GNUC__)
    // GCC style alignment
    #define SF_SIMD_ALIGN(x) x __attribute__((aligned(16)))
#else
    #error Unexpected compiler supporting SSE instructions.
#endif

namespace Scaleform { namespace SIMD { 

typedef __m128 Vector4f;
typedef __m128i Vector4i;

// Separated, because there may be multiple instructions sets (but not currently).
namespace SSE { 

class InstructionSet
{
public:

    static bool HasSSE2Support()
    {
#if defined(_MSC_VER)
    #if defined(_WIN64)
            return true;
    #else
            int CPUInfo[4] = {-1};
            __cpuid(CPUInfo, 1);
            return ((CPUInfo[3] & (1 << 26)) != 0);        
    #endif
#elif defined(SF_CC_GNU)
    #if defined(__SSE2__) && __SSE2__
        // The compiler is only compiling for processors that have SSE2 support.
        return true;
    #else
        int CPUInfo[4] = {-1};
        asm("cpuid": "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (1));
        return ((CPUInfo[3] & (1 << 26)) != 0);
    #endif
#else
        return false;
#endif
    }

    // Any function overrides which use LoadAligned(Vector4i), or StreamAligned(Vector4i) Add16, Set1
    // must check the result of this function, to determine whether these instructions are supported.
    static bool SupportsIntegerIntrinsics()
    {
        static bool HasSSE2 = HasSSE2Support();
        return HasSSE2;
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4f LoadAligned( const float * p )
    {
        return _mm_load_ps(p);
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4i LoadAligned( const Vector4i * p )
    {
        return _mm_loadu_si128(p);
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, Vector4f v )
    {
        return _mm_store_ps(p, v);
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( Vector4i * p, Vector4i v )
    {
        return _mm_stream_si128(p, v);
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static Vector4f Constant( )
    {
#if defined(_MSC_VER)
        // When initializing unions which contain arrays (which __m128i is), the the initializers are cast to the first member in the union.
        static Vector4i v = { 
            ((i0&0x000000FF) >> 0 ), ((i0&0x0000FF00) >> 8), ((i0&0x00FF0000) >> 16), ((i0&0xFF000000) >> 24), 
            ((i1&0x000000FF) >> 0 ), ((i1&0x0000FF00) >> 8), ((i1&0x00FF0000) >> 16), ((i1&0xFF000000) >> 24), 
            ((i2&0x000000FF) >> 0 ), ((i2&0x0000FF00) >> 8), ((i2&0x00FF0000) >> 16), ((i2&0xFF000000) >> 24), 
            ((i3&0x000000FF) >> 0 ), ((i3&0x0000FF00) >> 8), ((i3&0x00FF0000) >> 16), ((i3&0xFF000000) >> 24) };
#else
        static const __m128i v = _mm_set_epi32(i3, i2, i1, i0);
#endif
        return *(Vector4f*)&v;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static Vector4i Set1(UInt16 v)
    {
        return _mm_set1_epi16(v);
    }

    // Splats 1 value to each word.
    template< int word >
    static Vector4f Splat(Vector4f r0)
    {
        return Shuffle<word,word,word,word>(r0,r0);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static Vector4f Shuffle( Vector4f r0, Vector4f r1 )
    {
        return _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(f3, f2, f1, f0));
    }

    // Interleaves the two low components of r0 and r1.
    static Vector4f UnpackLo( Vector4f r0, Vector4f r1 )
    {
        return _mm_unpacklo_ps(r0, r1);
    }

    // Interleaves the two high components of r0 and r1.
    static Vector4f UnpackHi( Vector4f r0, Vector4f r1 )
    {
        return _mm_unpackhi_ps(r0, r1);
    }

    // Multiplies two input registers and returns the result.
    static Vector4f Multiply( Vector4f r0, Vector4f r1 )
    {
        return _mm_mul_ps(r0, r1);
    }

    // Divides r0 by r1, and returns the result.
    static Vector4f Divide( Vector4f r0, Vector4f r1 )
    {
        return _mm_div_ps(r0, r1);
    }

    // Takes the dot product of the two input registers (first three elements)
    // and returns the result in every component
    static Vector4f Dot3( Vector4f r0, Vector4f r1 )
    {
        Vector4f r = Multiply( r0, r1 );
        return Add( Add( Splat<0>(r), Splat<1>(r)), Splat<2>(r));
    }

    // Adds to input registers and returns the result.
    static Vector4f Add( Vector4f r0, Vector4f r1 )
    {
        return _mm_add_ps(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static Vector4i Add16( Vector4i r0, Vector4i r1 )
    {
        return _mm_add_epi16(r0, r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static Vector4f MultiplyAdd( Vector4f r0, Vector4f r1, Vector4f r2 )
    {
        return Add( Multiply(r0, r1), r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static Vector4f Min( Vector4f r0, Vector4f r1 )
    {
        return _mm_min_ps(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static Vector4f Max( Vector4f r0, Vector4f r1 )
    {
        return _mm_max_ps(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static Vector4f And( Vector4f r0, Vector4f r1 )
    {
        return _mm_and_ps( r0, r1 );
    }

    // Returns the bitwise or of the two input registers.
    static Vector4f Or( Vector4f r0, Vector4f r1 )
    {
        return _mm_or_ps( r0, r1 );
    }

    // Element-wise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static Vector4f CompareGE( Vector4f r0, Vector4f r1 )
    {
        return _mm_cmpge_ps(r0, r1);
    }

    // Element-wise comparison of two registers (equal), returns true if any of the elements passes the comparison.
    static bool CompareEQ_Any( Vector4f r0, Vector4f r1 )
    {
        Vector4f result = _mm_cmpneq_ps(r0,r1);
        return (_mm_movemask_ps(result) & 0xF) != 0xF;
    }

    // Element-wise comparison of two registers, returns true if all of the elements passes the comparison.
    static bool CompareEQ_All( Vector4f r0, Vector4f r1 )
    {
        Vector4f result = _mm_cmpeq_ps(r0,r1);
        return (_mm_movemask_ps(result) & 0xF) == 0xF;
    }

    // Computes the reciprocal square root estimate of each input element.
    static Vector4f ReciprocalSqrt( Vector4f r0 )
    {
        return _mm_rsqrt_ps(r0);
    }

    // Computes the reciprocal estimate of each input element.
    static Vector4f Reciprocal( Vector4f r0 )
    {
        return _mm_rcp_ps(r0);
    }

    // Computes the element-wise difference between r0 and r1.
    static Vector4f Subtract( Vector4f r0, Vector4f r1 )
    {
        return _mm_sub_ps(r0, r1);
    }

    // Computes the element-wise absolute value of r0.
    static Vector4f Abs( Vector4f r0 )
    {
        Vector4f nosignbit = Constant<0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF>();
        return And( r0, nosignbit );
    }

    static Vector4f Select(Vector4f r0, Vector4f r1, Vector4f mask)
    {
        return _mm_xor_ps( r0, _mm_and_ps( mask, _mm_xor_ps( r1, r0 ) ) );
    }

    // Prefetches a single cache line of the given address.
    static void Prefetch(const void * p, int offset)
    {
        // Loads it only into the L2 cache, not L1 cache.
        return _mm_prefetch( (const char*)p + offset, _MM_HINT_T1 );
    }

    // Prefetches an entire object (does multiple prefetches, if the class is too large
    // for a single cache line.
    template<class T>
    static void PrefetchObj( const T * obj )
    {
        // On Win32, we will assume cache line size is 32 bytes. It should not be less than this,
        // if it is greater, we will wasting some prefetch instructions, however, that is better
        // than branching on the cache line size.
        static const int cacheLineSize = 32;
        switch((sizeof(T)-1)/cacheLineSize)
        {
            // Only handles objects up to 256 bytes.
            default:
            case 7: Prefetch(obj, 7*cacheLineSize);
            case 6: Prefetch(obj, 6*cacheLineSize);
            case 5: Prefetch(obj, 5*cacheLineSize);
            case 4: Prefetch(obj, 4*cacheLineSize);
            case 3: Prefetch(obj, 3*cacheLineSize);
            case 2: Prefetch(obj, 2*cacheLineSize);
            case 1: Prefetch(obj, 1*cacheLineSize);
            case 0: Prefetch(obj, 0*cacheLineSize);
        }
    }
};

} // SSE

// Define the default instruction set.
typedef SSE::InstructionSet IS;

}} // Scaleform::SIMD

#endif // SF_ENABLE_SIMD && SF_OS_WIN32

#endif // INC_SF_SF_SIMD_WIN32_H
