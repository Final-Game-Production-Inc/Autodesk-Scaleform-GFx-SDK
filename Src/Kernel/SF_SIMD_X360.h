/**************************************************************************

Filename    :   SF_SIMD_X360.h
Content     :   SIMD instruction set for X360.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_SF_SIMD_X360_H
#define INC_SF_SF_SIMD_X360_H
#pragma once

#if defined(SF_ENABLE_SIMD) && defined(SF_OS_XBOX360)

#include <VectorIntrinsics.h>
#define SF_SIMD_ALIGN(x) _DECLSPEC_ALIGN_16_ x

//---------------------------------------------------------------------------------------
namespace Scaleform { namespace SIMD { 

typedef __vector4 Vector4f;
// __vector4i is defined as an array of UINT on X360, which is problematic for the
// instruction set functions. Redefine our own compatible structure type.
typedef _DECLSPEC_ALIGN_16_ struct { UINT _4i[4]; } Vector4i;

#define _PERMUTE_SINGLE_MASK(name, f0, f1, f2, f3) \
    static const __vector4i imask##name = {  \
    ((f0)*4+3) | ((f0)*4 +2) << 8 | ((f0)*4 +1) << 16 | ((f0)*4 +0) << 24, \
    ((f1)*4+3) | ((f1)*4 +2) << 8 | ((f1)*4 +1) << 16 | ((f1)*4 +0) << 24, \
    ((f2)*4+3) | ((f2)*4 +2) << 8 | ((f2)*4 +1) << 16 | ((f2)*4 +0) << 24, \
    ((f3)*4+3) | ((f3)*4 +2) << 8 | ((f3)*4 +1) << 16 | ((f3)*4 +0) << 24 }; \
    static const __vector4& ##name = (__vector4&)imask##name;

namespace X360 { 
class InstructionSet
{
public:

    static bool SupportsIntegerIntrinsics()
    {
        return true;
    }

    static const int AllPassMask = 0x80;
    static const int AllFailMask = 0x20;

    // Loads a value from aligned memory, and returns it.
    static Vector4f LoadAligned( const float * p )
    {
        return *(Vector4f*)p;
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4i LoadAligned( const Vector4i * p )
    {
        return *(p);
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, Vector4f v )
    {
        *(Vector4f*)p = v;
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( Vector4i * p, Vector4i v )
    {
        // Note: not actually streaming. __stvx uses the L2 cache.
        *p = v;
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static Vector4f Constant( )
    {
        static Vector4i c = { i0, i1, i2, i3 };
        return *(Vector4f*)&c;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static Vector4i Set1(UInt16 v)
    {
        // Better way?
        static const __vector4i mask = { 0x02030203, 0x02030203, 0x02030203, 0x02030203 };
        __vector4i t;
        t[0] = v;
        Vector4i r;
        *(Vector4f*)&r = __vperm(*(Vector4f*)t, __vzero(), *(Vector4f*)mask );
        return (r);
    }

    // Splats 1 value to each word.
    template< int word >
    static Vector4f Splat(Vector4f r0)
    {
        return __vspltw(r0, word);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static Vector4f Shuffle( Vector4f r0, Vector4f r1 )
    {
        // Counter intuitive reversal of order.
        _PERMUTE_SINGLE_MASK(mask, f0, f1, f2+4, f3+4);
        return __vperm(r0, r1, mask);
    }

    // Interleaves the two low components of r0 and r1.
    static Vector4f UnpackLo( Vector4f r0, Vector4f r1 )
    {
        _PERMUTE_SINGLE_MASK(mask, 0, 4, 1, 5);
        return __vperm(r0, r1, mask);
    }

    // Interleaves the two high components of r0 and r1.
    static Vector4f UnpackHi( Vector4f r0, Vector4f r1 )
    {
        _PERMUTE_SINGLE_MASK(mask, 2, 6, 3, 7 );
        return __vperm(r0, r1, mask);
    }

    // Multiplies two input registers and returns the result.
    static Vector4f Multiply( Vector4f r0, Vector4f r1 )
    {
        return __vmulfp(r0, r1);
    }

    // Divides r0 by r1, and returns the result.
    static Vector4f Divide( Vector4f r0, Vector4f r1 )
    {
        Vector4f recip = __vrefp(r1);
        return Multiply(r0, recip);
    }

    // Takes the dot product of the two input registers (first three elements)
    // and returns the result in every component
    static Vector4f Dot3( Vector4f r0, Vector4f r1 )
    {
        return __vmsum3fp(r0, r1);
    }

    // Adds to input registers and returns the result.
    static Vector4f Add( Vector4f r0, Vector4f r1 )
    {
        return __vaddfp(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static Vector4i Add16( Vector4i r0, Vector4i r1 )
    {
        __vector4 t = __vaddshs(*(Vector4f*)&r0, *(Vector4f*)&r1);
        return *(Vector4i*)&t;
    }

    // Multiplies r0 and r1 then adds r2.
    static Vector4f MultiplyAdd( Vector4f r0, Vector4f r1, Vector4f r2 )
    {
        return __vmaddfp(r0, r1, r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static Vector4f Min( Vector4f r0, Vector4f r1 )
    {
        return __vminfp(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static Vector4f Max( Vector4f r0, Vector4f r1 )
    {
        return __vmaxfp(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static Vector4f And( Vector4f r0, Vector4f r1 )
    {
        return __vand( r0, r1 );
    }

    // Returns the bitwise or of the two input registers.
    static Vector4f Or( Vector4f r0, Vector4f r1 )
    {
        return __vor( r0, r1 );
    }

    // Elementwise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static Vector4f CompareGE( Vector4f r0, Vector4f r1 )
    {
        return __vcmpgefp(r0, r1);
    }

    // Element-wise comparison of two registers (equal), returns true if any of the elements passes the comparison.
    static bool CompareEQ_Any( Vector4f r0, Vector4f r1 )
    {
        unsigned CR;
        __vcmpeqfpR(r0,r1,&CR);
        return (CR & AllFailMask) == 0;
    }

    // Element-wise comparison of two registers (equal), returns true if all of the elements passes the comparison.
    static bool CompareEQ_All( Vector4f r0, Vector4f r1 )
    {
        unsigned CR;
        __vcmpeqfpR(r0,r1,&CR);
        return (CR & AllPassMask) != 0;
    }

    // Computes the reciprocal square root estimate of each input element.
    static Vector4f ReciprocalSqrt( Vector4f r0 )
    {
        return __vrsqrtefp(r0);
    }

    // Computes the reciprocal estimate of each input element.
    static Vector4f Reciprocal( Vector4f r0 )
    {
        return __vrefp(r0);
    }

    // Computes the element-wise difference between r0 and r1.
    static Vector4f Subtract( Vector4f r0, Vector4f r1 )
    {
        return __vsubfp(r0, r1);
    }

    // Computes the element-wise absolute value of r0.
    static Vector4f Abs( Vector4f r0 )
    {
        Vector4f nosignbit = Constant<0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF>();
        return And( r0, nosignbit );
    }

    static Vector4f Select(Vector4f r0, Vector4f r1, Vector4f mask)
    {
        return __vsel( r0, r1, mask  );
    }

    // Prefetches a single cache line of the given address.
    static void Prefetch(const void * p, int offset)
    {
        return __dcbt( offset, p );
    }

    // Prefetches an entire object (does multiple prefetches, if the class is too large
    // for a single cache line.
    template<class T>
    static void PrefetchObj( const T * obj )
    {
        // On X360, cache line size is 32bytes.
        static const int CACHE_LINE_SIZE = 32;
        switch((sizeof(T)-1)/32)
        {
            // Only handles objects up to 256 bytes.
            default:
            case 7: Prefetch(obj, 7*CACHE_LINE_SIZE);
            case 6: Prefetch(obj, 6*CACHE_LINE_SIZE);
            case 5: Prefetch(obj, 5*CACHE_LINE_SIZE);
            case 4: Prefetch(obj, 4*CACHE_LINE_SIZE);
            case 3: Prefetch(obj, 3*CACHE_LINE_SIZE);
            case 2: Prefetch(obj, 2*CACHE_LINE_SIZE);
            case 1: Prefetch(obj, 1*CACHE_LINE_SIZE);
            case 0: Prefetch(obj, 0*CACHE_LINE_SIZE);
        }
    }
};

} // X360

// Define the default instruction set.
typedef X360::InstructionSet IS;

}} // Scaleform::SIMD.

#endif // SF_ENABLE_SIMD && SF_OS_XBOX360

#endif // INC_SF_SF_SIMD_X360_H
