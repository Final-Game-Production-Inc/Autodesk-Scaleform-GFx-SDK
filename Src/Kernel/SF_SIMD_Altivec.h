/**************************************************************************

Filename    :   SF_SIMD_Altivec.h
Content     :   SIMD instruction set for processors with the Altivec extensions.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_SF_SIMD_Altivec_H
#define INC_SF_SF_SIMD_Altivec_H
#pragma once

#if defined(SF_ENABLE_SIMD) && defined(SF_CPU_ALTIVEC)

#include "altivec.h"
#include "ppu_intrinsics.h"

#define SF_SIMD_ALIGN(x) x __attribute__((aligned(16)))

#define _PERMUTE_SINGLE_MASK(f0, f1, f2, f3)       \
    ((const __vector unsigned char){                     \
    ((f0)*4+0), ((f0)*4 +1), ((f0)*4 +2), ((f0)*4 +3),   \
    ((f1)*4+0), ((f1)*4 +1), ((f1)*4 +2), ((f1)*4 +3),   \
    ((f2)*4+0), ((f2)*4 +1), ((f2)*4 +2), ((f2)*4 +3),   \
    ((f3)*4+0), ((f3)*4 +1), ((f3)*4 +2), ((f3)*4 +3) }) \
    

namespace Scaleform { namespace SIMD { 
    
typedef __vector float Vector4f;
typedef __vector unsigned int Vector4i;
    
namespace Altivec { 

//---------------------------------------------------------------------------------------
class InstructionSet
{
public:
    static bool SupportsIntegerIntrinsics()
    {
        return true;
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4f LoadAligned( const float * p )
    {
        return vec_lvx( 0, (Vector4f*)p );
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4i LoadAligned( const Vector4i * p )
    {
        return vec_ld( 0, p );
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, Vector4f v )
    {
        vec_st(v, 0, p);
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( Vector4i * p, Vector4i v )
    {
        // Note: not actually streaming.
        vec_st(v, 0, p);
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static const Vector4f Constant( )
    {
        return (const Vector4f)((const Vector4i){ i0, i1, i2, i3 });
    }

    // Sets 8 16-bit integer values in the register to the input.
    static Vector4i Set1(UInt16 v)
    {
        return (Vector4i)vec_splats(v);
    }

    // Splats 1 value to each word.
    template< int word >
    static Vector4f Splat(Vector4f r0)
    {
        return vec_splat(r0, word);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static Vector4f Shuffle( Vector4f r0, Vector4f r1 )
    {
        // Counter intuitive reversal of order.
        return vec_perm(r0, r1, _PERMUTE_SINGLE_MASK(f0, f1, f2+4, f3+4));
    }

    // Interleaves the two low components of r0 and r1.
    static Vector4f UnpackLo( Vector4f r0, Vector4f r1 )
    {
        // NOTE: Altivec defines element orders opposite to SSE, thus this instruction
        // had to be reversed to provide the same functionality.
        return vec_mergeh(r0, r1);
    }

    // Interleaves the two high components of r0 and r1.
    static Vector4f UnpackHi( Vector4f r0, Vector4f r1 )
    {
        // NOTE: Altive defines element orders opposite to SSE, thus this instruction
        // had to be reversed to provide the same functionality.
        return vec_mergel(r0, r1);
    }

    // Multiplies two input registers and returns the result.
    static Vector4f Multiply( Vector4f r0, Vector4f r1 )
    {
        // No muliply instruction - use madd, w/addend = 0.
        return vec_madd(r0, r1, (Vector4f)(0,0,0,0));
    }

    // Adds to input registers and returns the result.
    static Vector4f Add( Vector4f r0, Vector4f r1 )
    {
        return vec_add(r0, r1);
    }

    // Divides r0 by r1, and returns the result.
    static Vector4f Divide( Vector4f r0, Vector4f r1 )
    {
        Vector4f recip = vec_re(r1);
        return Multiply(r0, recip);
    }

    // Takes the dot product of the two input registers (first three elements)
    // and returns the result in every component
    static Vector4f Dot3( Vector4f r0, Vector4f r1 )
    {
        Vector4f r = Multiply( r0, r1 );
        return Add( Add( Splat<0>(r), Splat<1>(r)), Splat<2>(r));
    }

    // Adds 16-bit integer elements in two registers together
    static Vector4i Add16( Vector4i r0, Vector4i r1 )
    {
        return (Vector4i)vec_add((__vector signed short)r0, (__vector signed short)r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static Vector4f MultiplyAdd( Vector4f r0, Vector4f r1, Vector4f r2 )
    {
        return vec_madd(r0, r1, r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static Vector4f Min( Vector4f r0, Vector4f r1 )
    {
        return vec_min(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static Vector4f Max( Vector4f r0, Vector4f r1 )
    {
        return vec_max(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static Vector4f And( Vector4f r0, Vector4f r1 )
    {
        return vec_and( r0, r1 );
    }

    // Returns the bitwise or of the two input registers.
    static Vector4f Or( Vector4f r0, Vector4f r1 )
    {
        return vec_or( r0, r1 );
    }

    // Elementwise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static Vector4f CompareGE( Vector4f r0, Vector4f r1 )
    {
        return (Vector4f)vec_cmpge(r0, r1);
    }

    // Element-wise comparison of two registers (equal), returns true if any of the elements passes the comparison.
    static bool CompareEQ_Any( Vector4f r0, Vector4f r1 )
    {
        return vec_any_eq(r0,r1);
    }

    // Element-wise comparison of two registers, returns true if all of the elements passes the comparison.
    static bool CompareEQ_All( Vector4f r0, Vector4f r1 )
    {
        return vec_all_eq(r0,r1);
    }

    // Computes the reciprocal square root estimate of each input element.
    static Vector4f ReciprocalSqrt( Vector4f r0 )
    {
        return vec_rsqrte(r0);
    }

    // Computes the reciprocal estimate of each input element.
    static Vector4f Reciprocal( Vector4f r0 )
    {
        return vec_re(r0);
    }

    // Computes the element-wise difference between r0 and r1.
    static Vector4f Subtract( Vector4f r0, Vector4f r1 )
    {
        return vec_sub(r0, r1);
    }

    // Computes the element-wise absolute value of r0.
    static Vector4f Abs( Vector4f r0 )
    {
        Vector4f nosignbit = Constant<0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF>();
        return And( r0, nosignbit );
    }

    static Vector4f Select(Vector4f r0, Vector4f r1, Vector4f mask)
    {
        return vec_sel( r0, r1, (Vector4i) mask  );
    }

    // Prefetches a single cache line of the given address.
    static void Prefetch(const void * p, int offset)
    {
        return __dcbt( (const unsigned char*)p + offset );
    }

    // Prefetches an entire object (does multiple prefetches, if the class is too large
    // for a single cache line.
    template<class T>
    static void PrefetchObj( const T * obj )
    {
        static const int cacheLineSize = 128;
        switch((sizeof(T)-1)/cacheLineSize)
        {
            // Only handles objects up to 256 bytes.
        default:
        case 1: Prefetch(obj, 1*cacheLineSize);
        case 0: Prefetch(obj, 0*cacheLineSize);
        }
    }
};

} // Altivec

// Define the default instruction set.
typedef Altivec::InstructionSet IS;

}} // Scaleform::SIMD

#endif // SF_ENABLE_SIMD && SF_CPU_Altivec

#endif // INC_SF_SF_SIMD_Altivec_H
