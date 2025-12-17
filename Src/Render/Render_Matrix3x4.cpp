/**************************************************************************

Filename    :   Render_Matrix3x4.cpp
Content     :
Created     :   Jan 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_Matrix4x4.h"
#include "Kernel/SF_SIMD.h"

#if defined(SF_OS_WIIU)
#include <ppc_ps.h>
#endif

namespace Scaleform { namespace Render {

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)

template<> void Matrix3F::MultiplyMatrix_Opt(const Matrix3F &m1, const Matrix3F &m2, float* mask)
{
    asm("psq_l       fp0, 0(r4), 0, 0");
    asm("psq_l       fp1, 8(r4), 0, 0");
    asm("psq_l       fp2, 16(r4), 0, 0");
    asm("psq_l       fp3, 24(r4), 0, 0");
    asm("psq_l       fp4, 32(r4), 0, 0");
    asm("psq_l       fp5, 40(r4), 0, 0");

    asm("psq_l       fp6, 0(r5), 0, 0");
    asm("psq_l       fp7, 8(r5), 0, 0");
    asm("psq_l       fp8, 16(r5), 0, 0");
    asm("psq_l       fp9, 24(r5), 0, 0");
    asm("psq_l       fp10, 32(r5), 0, 0");
    asm("psq_l       fp11, 40(r5), 0, 0");

    asm("ps_merge00  fp18, fp0, fp0");
    asm("ps_mul      fp12, fp18, fp6");
    asm("ps_mul      fp13, fp18, fp7");
    asm("ps_merge00  fp19, fp2, fp2");
    asm("ps_mul      fp14, fp19, fp6");
    asm("ps_mul      fp15, fp19, fp7");
    asm("ps_merge00  fp20, fp4, fp4");
    asm("ps_mul      fp16, fp20, fp6");
    asm("ps_mul      fp17, fp20, fp7");

    asm("ps_merge11  fp18, fp0, fp0");
    asm("ps_madd     fp12, fp18, fp8, fp12");
    asm("ps_madd     fp13, fp18, fp9, fp13");
    asm("ps_merge11  fp19, fp2, fp2");
    asm("ps_madd     fp14, fp19, fp8, fp14");
    asm("ps_madd     fp15, fp19, fp9, fp15");
    asm("ps_merge11  fp20, fp4, fp4");
    asm("ps_madd     fp16, fp20, fp8, fp16");
    asm("ps_madd     fp17, fp20, fp9, fp17");

    asm("ps_merge00  fp18, fp1, fp1");
    asm("ps_madd     fp12, fp18, fp10, fp12");
    asm("ps_madd     fp13, fp18, fp11, fp13");
    asm("ps_merge00  fp19, fp3, fp3");
    asm("ps_madd     fp14, fp19, fp10, fp14");
    asm("ps_madd     fp15, fp19, fp11, fp15");
    asm("ps_merge00  fp20, fp5, fp5");
    asm("ps_madd     fp16, fp20, fp10, fp16");
    asm("ps_madd     fp17, fp20, fp11, fp17");

    asm("psq_l       fp21, 0(r6), 0, 0");
    asm("ps_madd     fp13, fp1, fp21, fp13");
    asm("ps_madd     fp15, fp3, fp21, fp15");
    asm("ps_madd     fp17, fp5, fp21, fp17");

    asm("psq_st      fp12, 0(r3), 0, 0");
    asm("psq_st      fp13, 8(r3), 0, 0");
    asm("psq_st      fp14, 16(r3), 0, 0");
    asm("psq_st      fp15, 24(r3), 0, 0");
    asm("psq_st      fp16, 32(r3), 0, 0");
    asm("psq_st      fp17, 40(r3), 0, 0");
}

template<> void Matrix3F::MultiplyMatrix(const Matrix3F &m1, const Matrix3F &m2)
{
    float mask[] = { 0.0f, 1.0f };
    MultiplyMatrix_Opt(m1, m2, mask);
}

template<> void Matrix3F::MultiplyMatrix(const Matrix3F&m1, const Matrix2F &m2)
{
    asm("psq_l       fp0, 0(r4), 0, 0");    // m10.xy
    asm("psq_l       fp1, 8(r4), 0, 0");    // m10.zw
    asm("psq_l       fp2, 16(r4), 0, 0");   // m11.xy
    asm("psq_l       fp3, 24(r4), 0, 0");   // m11.zw
    asm("psq_l       fp4, 32(r4), 0, 0");   // m12.xy
    asm("psq_l       fp5, 40(r4), 0, 0");   // m12.zw

    asm("psq_l       fp6, 0(r5), 0, 0");    // m20.xy
    asm("psq_l       fp7, 8(r5), 0, 0");    // m20.zw
    asm("psq_l       fp8, 16(r5), 0, 0");   // m21.xy
    asm("psq_l       fp9, 24(r5), 0, 0");   // m21.zw

    asm("ps_merge00  fp16, fp0, fp0");      // Splat m10.x into fp16
    asm("ps_mul      fp10, fp16, fp6");     // r0.xy = m10.x * m20.xy
    asm("ps_mul      fp11, fp16, fp7");     // r0.zw = m10.x * m20.zw
    asm("ps_merge00  fp17, fp2, fp2");      // Splat m11.x into fp17
    asm("ps_mul      fp12, fp17, fp6");     // r1.xy = m11.x * m20.xy
    asm("ps_mul      fp13, fp17, fp7");     // r1.zw = m11.x * m20.zw
    asm("ps_merge00  fp18, fp4, fp4");      // Splat m12.x into fp18
    asm("ps_mul      fp14, fp18, fp6");     // r2.xy = m12.x * m20.xy
    asm("ps_mul      fp15, fp18, fp7");     // r2.zw = m12.x * m20.zw

    asm("ps_merge11  fp16, fp0, fp0");          // Splat m10.y into fp16
    asm("ps_madd     fp10, fp16, fp8, fp10");   // r0.xy += m10.y * m21.xy
    asm("ps_madd     fp11, fp16, fp9, fp11");   // r0.zw += m10.y * m21.zw
    asm("ps_merge11  fp17, fp2, fp2");          // Splat m11.y into fp17
    asm("ps_madd     fp12, fp17, fp8, fp12");   // r1.xy += m11.y * m21.xy
    asm("ps_madd     fp13, fp17, fp9, fp13");   // r1.zw += m11.y * m21.zw
    asm("ps_merge11  fp18, fp4, fp4");          // Splat m12.y into fp18
    asm("ps_madd     fp14, fp18, fp8, fp14");   // r2.xy += m12.y * m21.xy
    asm("ps_madd     fp15, fp18, fp9, fp15");   // r2.zw += m12.y * m21.zw

    asm("ps_add      fp11, fp11, fp1");         // r0.zw += m10.zw
    asm("ps_add      fp13, fp13, fp3");         // r1.zw += m11.zw
    asm("ps_add      fp15, fp15, fp5");         // r2.zw += m12.zw

    asm("psq_st      fp10, 0(r3), 0, 0");
    asm("psq_st      fp11, 8(r3), 0, 0");
    asm("psq_st      fp12, 16(r3), 0, 0");
    asm("psq_st      fp13, 24(r3), 0, 0");
    asm("psq_st      fp14, 32(r3), 0, 0");
    asm("psq_st      fp15, 40(r3), 0, 0");
}

template<> void Matrix3F::MultiplyMatrix_Opt(const Matrix2F&m1, const Matrix3F &m2, float* mask)
{
    asm("psq_l       fp0, 0(r4), 0, 0");    // m10.xy
    asm("psq_l       fp1, 8(r4), 0, 0");    // m10.zw
    asm("psq_l       fp2, 16(r4), 0, 0");   // m11.xy
    asm("psq_l       fp3, 24(r4), 0, 0");   // m11.zw

    asm("psq_l       fp4, 0(r5), 0, 0");    // m20.xy
    asm("psq_l       fp5, 8(r5), 0, 0");    // m20.zw
    asm("psq_l       fp6, 16(r5), 0, 0");   // m21.xy
    asm("psq_l       fp7, 24(r5), 0, 0");   // m21.zw
    asm("psq_l       fp8, 32(r5), 0, 0");   // m22.xy
    asm("psq_l       fp9, 40(r5), 0, 0");   // m22.zw

    asm("ps_merge00  fp14, fp0, fp0");      // Splat m10.x into fp14
    asm("ps_mul      fp10, fp14, fp4");     // r0.xy = m10.x * m20.xy
    asm("ps_mul      fp11, fp14, fp5");     // r0.zw = m10.x * m20.zw
    asm("ps_merge00  fp15, fp2, fp2");      // Splat m11.x into fp15
    asm("ps_mul      fp12, fp15, fp4");     // r1.xy = m11.x * m20.xy
    asm("ps_mul      fp13, fp15, fp5");     // r1.zw = m11.x * m20.zw

    asm("ps_merge11  fp14, fp0, fp0");          // Splat m10.y into fp14
    asm("ps_madd     fp10, fp14, fp6, fp10");   // r0.xy += m10.y * m21.xy
    asm("ps_madd     fp11, fp14, fp7, fp11");   // r0.zw += m10.y * m21.zw
    asm("ps_merge11  fp15, fp2, fp2");          // Splat m11.y into fp15
    asm("ps_madd     fp12, fp15, fp6, fp12");   // r1.xy += m11.y * m21.xy
    asm("ps_madd     fp13, fp15, fp7, fp13");   // r1.zw += m11.y * m21.zw

    asm("ps_merge00  fp14, fp1, fp1");          // Splat m10.z into fp14
    asm("ps_madd     fp10, fp14, fp8, fp10");   // r0.xy += m10.y * m22.xy
    asm("ps_madd     fp11, fp14, fp9, fp11");   // r0.zw += m10.y * m22.zw
    asm("ps_merge00  fp15, fp3, fp3");          // Splat m11.z into fp15
    asm("ps_madd     fp12, fp15, fp8, fp12");   // r1.xy += m11.y * m22.xy
    asm("ps_madd     fp13, fp15, fp9, fp13");   // r1.zw += m11.y * m22.zw

    asm("psq_l       fp16, 0(r6), 0, 0");
    asm("ps_madd     fp11, fp1, fp16, fp11");   // r0.w += m10.w
    asm("ps_madd     fp13, fp3, fp16, fp13");   // r1.w += m11.w

    asm("psq_st      fp10, 0(r3), 0, 0");       // M[0].xy = r0.xy
    asm("psq_st      fp11, 8(r3), 0, 0");       // M[0].zw = r0.zw
    asm("psq_st      fp12, 16(r3), 0, 0");      // M[1].xy = r0.xy
    asm("psq_st      fp13, 24(r3), 0, 0");      // M[1].zw = r0.zw
    asm("psq_st      fp8, 32(r3), 0, 0");       // M[2].xy = m22.xy
    asm("psq_st      fp9, 40(r3), 0, 0");       // M[2].zw = m22.zw
}

template<> void Matrix3F::MultiplyMatrix(const Matrix2F&m1, const Matrix3F &m2)
{
    float mask[] = { 0.0f, 1.0f };
    MultiplyMatrix_Opt(m1, m2, mask);
}

#elif defined(SF_ENABLE_SIMD)

// CUZ C++ 17
// template<> void Matrix3F::MultiplyMatrix(const register Matrix3F &m1, const register Matrix3F &m2)
template<> void Matrix3F::MultiplyMatrix(const Matrix3F& m1, const Matrix3F& m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);
    Vector4f m12 = IS::LoadAligned(m1.M[2]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);
    Vector4f m22 = IS::LoadAligned(m2.M[2]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2 = IS::Multiply   ( IS::Splat<0>(m12), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<2>(m12), m22, r2 );

    Vector4f addMask = IS::Constant<0,0,0,0xFFFFFFFF>();
    r0 =          IS::Add( r0, IS::And( m10, addMask ) );
    r1 =          IS::Add( r1, IS::And( m11, addMask ) );
    r2 =          IS::Add( r2, IS::And( m12, addMask ) );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
}

template<> void Matrix3F::MultiplyMatrix(const Matrix3F&m1, const Matrix2F &m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);
    Vector4f m12 = IS::LoadAligned(m1.M[2]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2 = IS::Multiply   ( IS::Splat<0>(m12), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );

    Vector4f addMask = IS::Constant<0,0,0xFFFFFFFF,0xFFFFFFFF>();
    r0 =          IS::Add( r0, IS::And( m10, addMask ) );
    r1 =          IS::Add( r1, IS::And( m11, addMask ) );
    r2 =          IS::Add( r2, IS::And( m12, addMask ) );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
}

template<> void Matrix3F::MultiplyMatrix(const Matrix2F&m1, const Matrix3F &m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);
    Vector4f m22 = IS::LoadAligned(m2.M[2]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );

    Vector4f addMask = IS::Constant<0,0,0,0xFFFFFFFF>();
    r0 =          IS::Add( r0, IS::And( m10, addMask ) );
    r1 =          IS::Add( r1, IS::And( m11, addMask ) );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], m22);
}

#endif // SF_ENABLE_SIMD

}} // Scaleform::Render
