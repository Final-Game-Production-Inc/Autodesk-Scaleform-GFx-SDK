/**************************************************************************

Filename    :   SF_SIMD_ARMNeon.h
Content     :   
Created     :   Feb 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_SF_SIMD_ARMNEON_H
#define INC_SF_SF_SIMD_ARMNEON_H
#pragma once

#include "Kernel/SF_Types.h"

#if defined(SF_ENABLE_SIMD) && defined(SF_CPU_ARM_NEON)

#define SF_SIMD_ALIGN(x) x __attribute__((aligned(16)))

// The iPhone SDK for Xcode3 apparently reverses some of the from/to in vreinterpretq calls.
#if defined ( SF_OS_IPHONE ) && !defined(SF_CC_LLVMGCC)
#define vreinterpretq_f32_u32 vreinterpretq_u32_f32
#define vreinterpretq_u32_f32 vreinterpretq_f32_u32
#endif

#if defined( SF_OS_IPHONE ) && defined( SF_CC_LLVMGCC )
    // This allows initialization of types without extra braces.
    // Must be defined before #include <arm_neon.h>
#   define ARM_NEON_GCC_COMPATIBILITY
#endif

#include <arm_neon.h>

//---------------------------------------------------------------------------------------
namespace Scaleform { namespace SIMD { 
    
    typedef float32x4_t Vector4f;
    typedef uint32x4_t  Vector4i;
    
    namespace ARMNeon { 
        
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
                return vld1q_f32( (float32_t const*)p );
            }
            
            // Loads a value from aligned memory, and returns it.
            static Vector4i LoadAligned( const Vector4i * p )
            {
                return vld1q_u32( (uint32_t const*)p );
            }
            
            // Stores a value to aligned memory.
            static void StoreAligned( float * p, Vector4f v )
            {
                vst1q_f32( (float32_t*)p, v );
            }
            
            // Stores a values to aligned memory, without using the cache.
            static void StreamAligned( Vector4i * p, Vector4i v )
            {
                // Note: not actually streaming.
                vst1q_u32( (uint32_t*)p, v );
            }
            
            // Creates a 4 single-precision floating point constant from 4 unsigned integers.
            template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
            static Vector4f Constant( )
            {
                static Vector4i c = { i0, i1, i2, i3 };
                return vreinterpretq_f32_u32(c);
            }
            
            // Sets 8 16-bit integer values in the register to the input.
            static Vector4i Set1(UInt16 v)
            {
                uint16x8_t r = vdupq_n_u16(v);
                return vreinterpretq_u32_u16(r);
            }
            
            // Splats 1 value to each word.
            template< int word >
            static Vector4f Splat(Vector4f r0)
            {
#if defined(SF_CC_LLVMGCC) || defined(SF_CC_CLANG)
                // LLVM doesn't support %e/%f, and vdup needs a doubleword register.
                Vector4f out;
                float32x2_t r1;
                switch( word )
                {
                    default: 
                    case 0:
                        r1 = vget_low_f32(r0);
                        asm volatile("vdup.32  %q[out], %[r1][0]" : [out]"=&w"(out) : [r1]"w"(r1)); 
                        break;
                        
                    case 1: 
                        r1 = vget_low_f32(r0);
                        asm volatile("vdup.32  %q[out], %[r1][1]" : [out]"=&w"(out) : [r1]"w"(r1)); 
                        break;
                        
                    case 2:
                        r1 = vget_high_f32(r0);
                        asm volatile("vdup.32  %q[out], %[r1][0]" : [out]"=&w"(out) : [r1]"w"(r1)); 
                        break;
                        
                    case 3: 
                        r1 = vget_high_f32(r0);
                        asm volatile("vdup.32  %q[out], %[r1][1]" : [out]"=&w"(out) : [r1]"w"(r1)); 
                        break;
                }
                return out;                
#else
                Vector4f out;
                // This looks very ugly, but it should optimize down to one instruction in release.
                // Unfortunatley, VDUP can only load from a doubleword register.
                switch( word )
                {
                    default: 
                    case 0: asm volatile("vdup.32  %q[out], %e[r0][0]" : [out]"=&w"(out) : [r0]"w"(r0)); break;
                    case 1: asm volatile("vdup.32  %q[out], %e[r0][1]" : [out]"=&w"(out) : [r0]"w"(r0)); break;
                    case 2: asm volatile("vdup.32  %q[out], %f[r0][0]" : [out]"=&w"(out) : [r0]"w"(r0)); break;
                    case 3: asm volatile("vdup.32  %q[out], %f[r0][1]" : [out]"=&w"(out) : [r0]"w"(r0)); break;
                }
                return out;
#endif
            }
            
            // Takes two registers, selects two elements from the first, and two from the second.
            // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
            // f2 and f3 refer to the elements in r1.
            template <int f0, int f1, int f2, int f3>
            static Vector4f Shuffle( Vector4f r0, Vector4f r1 )
            {
                Vector4f vo;
                
#if defined(SF_CC_LLVMGCC) || defined(SF_CC_CLANG)
                static const uint8x8_t vi0 = { f0*4, f0*4+1, f0*4+2, f0*4+3, f1*4, f1*4+1, f1*4+2, f1*4+3 };
                static const uint8x8_t vi1 = { f2*4, f2*4+1, f2*4+2, f2*4+3, f3*4, f3*4+1, f3*4+2, f3*4+3 };

                uint8x8x2_t r0ext, r1ext;
                r0ext.val[0] = vreinterpret_u8_f32( vget_low_f32(r0) );
                r0ext.val[1] = vreinterpret_u8_f32( vget_high_f32(r0) );
                r1ext.val[0] = vreinterpret_u8_f32( vget_low_f32(r1) );
                r1ext.val[1] = vreinterpret_u8_f32( vget_high_f32(r1) );
                float32x2_t vo0 = vreinterpret_f32_u8( vtbl2_u8(r0ext, vi0) );
                float32x2_t vo1 = vreinterpret_f32_u8( vtbl2_u8(r1ext, vi1) );
                vo = vcombine_f32(vo0, vo1);
#else
                static const uint8x16_t vi = { f0*4, f0*4+1, f0*4+2, f0*4+3, 
                                               f1*4, f1*4+1, f1*4+2, f1*4+3, 
                                               f2*4, f2*4+1, f2*4+2, f2*4+3, 
                                               f3*4, f3*4+1, f3*4+2, f3*4+3 };

                // The intrinsics don't allow the proper combination of input parameters
                asm volatile
                ("vtbl.8  %e[vo],{%q[r0]},%e[vi]   \n\t"
                 "vtbl.8  %f[vo],{%q[r1]},%f[vi]   \n\t"
                 : [vo]"=&w"(vo)
                 : [r0]"w"(r0), [r1]"w"(r1), [vi]"w"(vi) );
#endif
                
                return vo;
            }
            
            // Interleaves the two low components of r0 and r1.
            static Vector4f UnpackLo( Vector4f r0, Vector4f r1 )
            {
                float32x4x2_t r = vzipq_f32( r0, r1 );
                return r.val[0];
            }
            
            // Interleaves the two high components of r0 and r1.
            static Vector4f UnpackHi( Vector4f r0, Vector4f r1 )
            {
                float32x4x2_t r = vzipq_f32( r0, r1 );
                return r.val[1];
            }
            
            // Multiplies two input registers and returns the result.
            static Vector4f Multiply( Vector4f r0, Vector4f r1 )
            {
                return vmulq_f32(r0, r1);
            }
            
            // Divides r0 by r1, and returns the result.
            static Vector4f Divide( Vector4f r0, Vector4f r1 )
            {
                Vector4f recip = vrecpeq_f32(r1);
                return Multiply(r0, recip);
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
                return vaddq_f32(r0, r1);
            }
            
            // Adds 16-bit integer elements in two registers together
            static Vector4i Add16( Vector4i r0, Vector4i r1 )
            {
                uint16x8_t r0_16 = vreinterpretq_u16_u32(r0);
                uint16x8_t r1_16 = vreinterpretq_u16_u32(r1);
                uint16x8_t r     = vaddq_u16(r0_16, r1_16);
                return vreinterpretq_u32_u16(r);
            }
            
            // Multiplies r0 and r1 then adds r2.
            static Vector4f MultiplyAdd( Vector4f r0, Vector4f r1, Vector4f r2 )
            {
                // Note: intrinsic operands in different order than on other platforms.
                return vmlaq_f32(r2, r0, r1 );
            }
            
            // Computes minimum component-wise values between r0 and r1.
            static Vector4f Min( Vector4f r0, Vector4f r1 )
            {
                return vminq_f32(r0, r1);
            }
            
            // Computes maximum component-wise values between r0 and r1.
            static Vector4f Max( Vector4f r0, Vector4f r1 )
            {
                return vmaxq_f32(r0, r1);
            }
            
            // Returns the bitwise and of the two input registers.
            static Vector4f And( Vector4f r0, Vector4f r1 )
            {
                return vreinterpretq_f32_u32( vandq_u32( vreinterpretq_u32_f32(r0), vreinterpretq_u32_f32(r1) ) );
            }

            // Returns the bitwise or of the two input registers.
            static Vector4f Or( Vector4f r0, Vector4f r1 )
            {
                return vreinterpretq_f32_u32( vorrq_u32( vreinterpretq_u32_f32(r0), vreinterpretq_u32_f32(r1) ) );
            }

            // Elementwise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
            static Vector4f CompareGE( Vector4f r0, Vector4f r1 )
            {
                return vreinterpretq_f32_u32( vcgeq_f32(r0, r1) );
            }
            
            // Element-wise comparison of two registers (equal), returns true if any of the elements passes the comparison.
            static bool CompareEQ_Any( Vector4f r0, Vector4f r1 )
            {
                uint64_t    eql;
                uint32x4_t  cmp = vceqq_f32(r0, r1);
                 uint64x1_t lsb = vreinterpret_u64_u16(vmovn_u32(cmp));
                vst1_u64(&eql, lsb);
                return eql > 0;
            }
            
            // Element-wise comparison of two registers (equal), returns true if all of the elements passes the comparison.
            static bool CompareEQ_All( Vector4f r0, Vector4f r1 )
            {
                uint64_t    eql;
                uint32x4_t  cmp = vceqq_f32(r0, r1);
                 uint64x1_t lsb = vreinterpret_u64_u16(vmovn_u32(cmp));
                vst1_u64(&eql, lsb);
                return eql == 0xFFFFFFFFFFFFFFFFll;
            }
            
            // Computes the reciprocal square root estimate of each input element.
            static Vector4f ReciprocalSqrt( Vector4f r0 )
            {
                return vrsqrteq_f32(r0);
            }
            
            // Computes the reciprocal estimate of each input element.
            static Vector4f Reciprocal( Vector4f r0 )
            {
                return vrecpeq_f32(r0);
            }
            
            // Computes the element-wise difference between r0 and r1.
            static Vector4f Subtract( Vector4f r0, Vector4f r1 )
            {
                return vsubq_f32(r0, r1);
            }
            
            // Computes the element-wise absolute value of r0.
            static Vector4f Abs( Vector4f r0 )
            {
                return vabsq_f32(r0);
            }
            
            static Vector4f Select(Vector4f r0, Vector4f r1, Vector4f mask)
            {
                return vbslq_f32( vreinterpretq_u32_f32( mask ), r1, r0 );
            }
            
            // Prefetches a single cache line of the given address.
            static void Prefetch(const void * p, int offset)
            {
                // can't find definition of the __pld intrinsic, and inline assembly doesn't seem to work.
                // __pld( (const char*)p + offset );
            }
            
            // Prefetches an entire object (does multiple prefetches, if the class is too large
            // for a single cache line.
            template<class T>
            static void PrefetchObj( const T * obj )
            {
                // TODOBM: determine cache line size.
                static const int CACHE_LINE = 32;
                switch((sizeof(T)-1)/32)
                {
                        // Only handles objects up to 256 bytes.
                    default:
                    case 7: Prefetch(obj, 7*CACHE_LINE);
                    case 6: Prefetch(obj, 6*CACHE_LINE);
                    case 5: Prefetch(obj, 5*CACHE_LINE);
                    case 4: Prefetch(obj, 4*CACHE_LINE);
                    case 3: Prefetch(obj, 3*CACHE_LINE);
                    case 2: Prefetch(obj, 2*CACHE_LINE);
                    case 1: Prefetch(obj, 1*CACHE_LINE);
                    case 0: Prefetch(obj, 0*CACHE_LINE);
                }
            }
        };
        
    } // ARMNeon

    
    // Define the default instruction set.
    typedef ARMNeon::InstructionSet IS;
        
}} // Scaleform::SIMD.


#endif // SF_ENABLE_SIMD && SF_CPU_ARM_NEON

#endif // INC_SF_SF_SIMD_ARMNEON_H
