/**************************************************************************

Filename    :   Render_Matrix2x4.cpp
Content     :   SIMD optimized matrix operations.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Types2D.h"
#include "Kernel/SF_SIMD.h"

namespace Scaleform { namespace Render {

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)

template<> void Matrix2F::EncloseTransform(RectF *pr /*r4*/, const RectF& r /*r5*/) const
{
    // fp0:         rect.xy1, yxyx.xy
    // fp1:         rect.xy2, yxyx.zw
    // fp2:         M[0].xy, add_yxyx.xyzw
    // fp3:         M[0].zw,
    // fp4:         M[1].xy, result.xy
    // fp5:         M[1].zw, result.zw

    // fp6:         y1y1x1x1.xy, m11y1_m01y1_m10x1_m00x1.xy, shufMin.xy, min_yyxx.xy
    // fp7:         y1y1x1x1.zw, m11y1_m01y1_m10x1_m00x1.zw, shufMin.zw, min_yyxx.zw
    // fp8:         y2y2x2x2.xy, m11y2_m01y2_m10x2_m00x2.xy, shufMax.xy, max_yyxx.xy
    // fp9:         y2y2x2x2.zw, m11y2_m01y2_m10x2_m00x2.zw, shufMax.zw, max_yyxx.zw

    // fp10:        add_yyxx.xy
    // fp11:        add_yyxx.zw

    // fp12:        m11_01_10_00.xy, m10x2_m10x1_m00x2_m00x1.xy, yAyBxAxB.xy
    // fp13:        m11_01_10_00.zw, m10x2_m10x1_m00x2_m00x1.zw, yAyBxAxB.zw

    // fp14:        m11y2_m11y1_m01y2_m01y1.xy, min0_yyxx.xy
    // fp15:        m11y2_m11y1_m01y2_m01y1.zw, min0_yyxx.zw
    // fp16:        m10x1_m10x2_m00x1_m00x2.xy, yCyDxCxD.xy, max0_yyxx.xy
    // fp17:        m10x1_m10x2_m00x1_m00x2.zw, yCyDxCxD.zw, max0_yyxx.zw

    // fp18:        temp

    asm("psq_l      fp0, 0(r5), 0, 0");
    asm("psq_l      fp1, 8(r5), 0, 0");
    asm("psq_l      fp2, 0(r3), 0, 0");
    asm("psq_l      fp3, 8(r3), 0, 0");
    asm("psq_l      fp4, 16(r3), 0, 0");
    asm("psq_l      fp5, 24(r3), 0, 0");

    asm("ps_merge00 fp6, fp0, fp0");
    asm("ps_merge11 fp7, fp0, fp0");
    asm("ps_merge00 fp8, fp1, fp1");
    asm("ps_merge11 fp9, fp1, fp1");

    asm("ps_merge11 fp10, fp3, fp3");
    asm("ps_merge11 fp11, fp5, fp5");

    asm("ps_merge00 fp12, fp2, fp4");
    asm("ps_merge11 fp13, fp2, fp4");

    asm("ps_mul     fp6, fp6, fp12");
    asm("ps_mul     fp7, fp7, fp13");
    asm("ps_mul     fp8, fp8, fp12");
    asm("ps_mul     fp9, fp9, fp13");

    asm("ps_merge00 fp12, fp6, fp8");
    asm("ps_merge11 fp13, fp6, fp8");

    asm("ps_merge00 fp14, fp7, fp9");
    asm("ps_merge11 fp15, fp7, fp9");

    asm("ps_merge10 fp16, fp12, fp12");
    asm("ps_merge10 fp17, fp13, fp13");

    asm("ps_add     fp12, fp12, fp14");
    asm("ps_add     fp13, fp13, fp15");
    asm("ps_add     fp16, fp16, fp14");
    asm("ps_add     fp17, fp17, fp15");

    asm("ps_sub     fp18, fp12, fp16");
    asm("ps_sel     fp14, fp18, fp16, fp12");
    asm("ps_sub     fp18, fp13, fp17");
    asm("ps_sel     fp15, fp18, fp17, fp13");

    asm("ps_sub     fp18, fp12, fp16");
    asm("ps_sel     fp16, fp18, fp12, fp16");
    asm("ps_sub     fp18, fp13, fp17");
    asm("ps_sel     fp17, fp18, fp13, fp17");

    asm("ps_merge10 fp6, fp14, fp14");
    asm("ps_merge10 fp7, fp15, fp15");
    asm("ps_merge10 fp8, fp16, fp16");
    asm("ps_merge10 fp9, fp17, fp17");

    asm("ps_sub     fp18, fp14, fp6");
    asm("ps_sel     fp6, fp18, fp6, fp14");
    asm("ps_sub     fp18, fp15, fp7");
    asm("ps_sel     fp7, fp18, fp7, fp15");

    asm("ps_sub     fp18, fp16, fp8");
    asm("ps_sel     fp8, fp18, fp16, fp8");
    asm("ps_sub     fp18, fp17, fp9");
    asm("ps_sel     fp9, fp18, fp17, fp9");

    asm("ps_merge00 fp0, fp6, fp7");
    asm("ps_merge00 fp1, fp8, fp9");

    asm("ps_merge00 fp2, fp10, fp11");

    asm("ps_add     fp4, fp0, fp2");
    asm("ps_add     fp5, fp1, fp2");

    asm("psq_st     fp4, 0(r4), 0, 0");
    asm("psq_st     fp5, 8(r4), 0, 0");
}

template<> void Matrix2F::SetToAppend_Opt( const Matrix2F& m0, const Matrix2F& m1, float* mask)
{
    // fp0:         m1v0.xy
    // fp1:         m1v0.zw
    // fp2:         m1v1.xy
    // fp3:         m1v1.zw

    // fp4:         m0v0.xy
    // fp5:         m0v0.zw
    // fp6:         m0v1.xy
    // fp7:         m0v1.zw

    // fp8:         mask

    // fp9:         res_v0.zw
    // fp10:        res_v1.zw

    // fp11:        shufm1v0.xy
    // fp12:        shufm1v1.xy

    // fp13:        t0.xy, t2.xy
    // fp14:        t0.zw, t2.zw
    // fp15:        t1.xy, t3.xy
    // fp16:        t1.zw, t3.zw

    asm("psq_l      fp0, 0(r5), 0, 0");
    asm("psq_l      fp1, 8(r5), 0, 0");
    asm("psq_l      fp2, 16(r5), 0, 0");
    asm("psq_l      fp3, 24(r5), 0, 0");

    asm("psq_l      fp4, 0(r4), 0, 0");
    asm("psq_l      fp5, 8(r4), 0, 0");
    asm("psq_l      fp6, 16(r4), 0, 0");
    asm("psq_l      fp7, 24(r4), 0, 0");

    asm("psq_l      fp8, 0(r6), 0, 0");

    asm("ps_mul     fp9, fp1, fp8");
    asm("ps_mul     fp10, fp3, fp8");

    asm("ps_merge00 fp11, fp0, fp0");
    asm("ps_merge00 fp12, fp2, fp2");

    asm("ps_mul     fp13, fp4, fp11");
    asm("ps_mul     fp14, fp5, fp11");
    asm("ps_mul     fp15, fp4, fp12");
    asm("ps_mul     fp16, fp5, fp12");

    asm("ps_merge11 fp11, fp0, fp0");
    asm("ps_merge11 fp12, fp2, fp2");

    asm("ps_madd    fp13, fp6, fp11, fp13");
    asm("ps_madd    fp14, fp7, fp11, fp14");
    asm("ps_madd    fp15, fp6, fp12, fp15");
    asm("ps_madd    fp16, fp7, fp12, fp16");

    asm("ps_add     fp14, fp14, fp9");
    asm("ps_add     fp16, fp16, fp10");

    asm("ps_mul     fp14, fp14, fp8");
    asm("ps_mul     fp16, fp16, fp8");

    asm("psq_st     fp13, 0(r3), 0, 0");
    asm("psq_st     fp14, 8(r3), 0, 0");
    asm("psq_st     fp15, 16(r3), 0, 0");
    asm("psq_st     fp16, 24(r3), 0, 0");
}

template<> void Matrix2F::SetToAppend( const Matrix2F& m0, const Matrix2F& m1 )
{
    float mask[] = { 0.0f, 1.0f };
    SetToAppend_Opt(m0, m1, mask);
}

template<> void Matrix2F::SetToAppend_Opt( const Matrix2F & m0, const Matrix2F & m1, const Matrix2F & m2, float* mask)
{
    // fp0:         m0v0.xy
    // fp1:         m0v0.zw
    // fp2:         m0v1.xy
    // fp3:         m0v1.zw

    // fp4:         m1v0.xy
    // fp5:         m1v0.zw
    // fp6:         m1v1.xy
    // fp7:         m1v1.zw

    // fp8:         m2v0.xy
    // fp9:         m2v0.zw
    // fp10:        m2v1.xy
    // fp11:        m2v1.zw

    // fp12:        mask

    // fp13:        res_v0.zw
    // fp14:        res_v1.zw

    // fp15:        shufm2v0.xy
    // fp16:        shufm2v1.xy

    // fp17:        t0.xy, t2.xy
    // fp18:        t0.zw, t2.zw
    // fp19:        t1.xy, t3.xy
    // fp20:        t1.zw, t3.zw

    // fp21:        m12v0.zw
    // fp22:        m12v1.zw
    // fp23:        temp0
    // fp24:        temp1

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp4, 0(r5), 0, 0");
    asm("psq_l      fp5, 8(r5), 0, 0");
    asm("psq_l      fp6, 16(r5), 0, 0");
    asm("psq_l      fp7, 24(r5), 0, 0");

    asm("psq_l      fp8, 0(r6), 0, 0");
    asm("psq_l      fp9, 8(r6), 0, 0");
    asm("psq_l      fp10, 16(r6), 0, 0");
    asm("psq_l      fp11, 24(r6), 0, 0");

    asm("psq_l      fp12, 0(r7), 0, 0");

    asm("ps_mul     fp13, fp9, fp12");
    asm("ps_mul     fp14, fp11, fp12");

    // First Prepend multiply: m1 * m2.
    asm("ps_merge00 fp15, fp8, fp8");
    asm("ps_merge00 fp16, fp10, fp10");

    asm("ps_mul     fp17, fp4, fp15");
    asm("ps_mul     fp18, fp5, fp15");
    asm("ps_mul     fp19, fp4, fp16");
    asm("ps_mul     fp20, fp5, fp16");

    asm("ps_merge11 fp15, fp8, fp8");
    asm("ps_merge11 fp16, fp10, fp10");

    asm("ps_madd    fp17, fp6, fp15, fp17");
    asm("ps_madd    fp18, fp7, fp15, fp18");
    asm("ps_madd    fp19, fp6, fp16, fp19");
    asm("ps_madd    fp20, fp7, fp16, fp20");

    asm("ps_add     fp21, fp13, fp18");
    asm("ps_add     fp22, fp14, fp20");

    // Second append multiply.
    asm("ps_mul     fp13, fp21, fp12");
    asm("ps_mul     fp14, fp22, fp12");

    asm("ps_merge00 fp15, fp17, fp17");
    asm("ps_merge00 fp16, fp19, fp19");

    asm("ps_merge11 fp23, fp17, fp17");
    asm("ps_merge11 fp24, fp19, fp19");

    asm("ps_mul     fp17, fp0, fp15");
    asm("ps_mul     fp18, fp1, fp15");
    asm("ps_mul     fp19, fp0, fp16");
    asm("ps_mul     fp20, fp1, fp16");

    asm("ps_madd    fp17, fp2, fp23, fp17");
    asm("ps_madd    fp18, fp3, fp23, fp18");
    asm("ps_madd    fp19, fp2, fp24, fp19");
    asm("ps_madd    fp20, fp3, fp24, fp20");

    asm("ps_add     fp18, fp18, fp13");
    asm("ps_add     fp20, fp20, fp14");

    asm("ps_mul     fp18, fp18, fp12");
    asm("ps_mul     fp20, fp20, fp12");

    asm("psq_st     fp17, 0(r3), 0, 0");
    asm("psq_st     fp18, 8(r3), 0, 0");
    asm("psq_st     fp19, 16(r3), 0, 0");
    asm("psq_st     fp20, 24(r3), 0, 0");
}

template<> void Matrix2F::SetToAppend( const Matrix2F & m0, const Matrix2F & m1, const Matrix2F & m2 )
{
    float mask[] = { 0.0f, 1.0f };
    SetToAppend_Opt(m0, m1, m2, mask);
}

template<> Matrix2F& Matrix2F::Append_Opt( const Matrix2F & m, float* mask )
{
    // FIXME: SIMD causes transform issues on this one
    return Append_NonOpt(m);

#if 0
    // fp0:         m1v0.xy
    // fp1:         m1v0.zw
    // fp2:         m1v1.xy
    // fp3:         m1v1.zw

    // fp4:         m0v0.xy
    // fp5:         m0v0.zw
    // fp6:         m0v1.xy
    // fp7:         m0v1.zw

    // fp8:         mask

    // fp9:         res_v0.zw
    // fp10:        res_v1.zw

    // fp11:        shufm1v0.xy
    // fp12:        shufm1v1.xy

    // fp13:        t0.xy, t2.xy
    // fp14:        t0.zw, t2.zw
    // fp15:        t1.xy, t3.xy
    // fp16:        t1.zw, t3.zw

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp4, 0(r3), 0, 0");
    asm("psq_l      fp5, 8(r3), 0, 0");
    asm("psq_l      fp6, 16(r3), 0, 0");
    asm("psq_l      fp7, 24(r3), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");

    asm("ps_mul     fp9, fp1, fp8");
    asm("ps_mul     fp10, fp3, fp8");

    asm("ps_merge00 fp11, fp0, fp0");
    asm("ps_merge00 fp12, fp2, fp2");

    asm("ps_mul     fp13, fp4, fp11");
    asm("ps_mul     fp14, fp5, fp11");
    asm("ps_mul     fp15, fp4, fp12");
    asm("ps_mul     fp16, fp5, fp12");

    asm("ps_merge11 fp11, fp0, fp0");
    asm("ps_merge11 fp12, fp2, fp2");

    asm("ps_madd    fp13, fp6, fp11, fp13");
    asm("ps_madd    fp14, fp7, fp11, fp14");
    asm("ps_madd    fp15, fp6, fp12, fp15");
    asm("ps_madd    fp16, fp7, fp12, fp16");

    asm("ps_add     fp14, fp14, fp9");
    asm("ps_add     fp16, fp16, fp10");

    asm("ps_mul     fp14, fp14, fp8");
    asm("ps_mul     fp16, fp16, fp8");

    asm("psq_st     fp13, 0(r3), 0, 0");
    asm("psq_st     fp14, 8(r3), 0, 0");
    asm("psq_st     fp15, 16(r3), 0, 0");
    asm("psq_st     fp16, 24(r3), 0, 0");

    return *this;
#endif
}

template<> Matrix2F& Matrix2F::Append( const Matrix2F & m )
{
    float mask[] = { 0.0f, 1.0f };
    return Append_Opt(m, mask);
}

#elif defined(SF_ENABLE_SIMD)

template<>
void Matrix2F::EncloseTransform(RectF *pr, const RectF& r) const
{
    using namespace Scaleform::SIMD;

    Vector4f rect   = IS::LoadAligned(&r.x1);
    Vector4f m0     = IS::LoadAligned(M[0]);
    Vector4f m1     = IS::LoadAligned(M[1]);

    Vector4f y1y1x1x1 = IS::UnpackLo(rect, rect);
    Vector4f y2y2x2x2 = IS::UnpackHi(rect, rect);
    Vector4f add_yyxx = IS::Shuffle<3, 3, 3, 3>(m0, m1);

    Vector4f m11_01_10_00 = IS::UnpackLo(m0, m1);

    Vector4f m11y1_m01y1_m10x1_m00x1 = IS::Multiply(y1y1x1x1, m11_01_10_00);
    Vector4f m11y2_m01y2_m10x2_m00x2 = IS::Multiply(y2y2x2x2, m11_01_10_00);

    Vector4f m10x2_m10x1_m00x2_m00x1 = IS::UnpackLo(m11y1_m01y1_m10x1_m00x1, m11y2_m01y2_m10x2_m00x2);
    Vector4f m11y2_m11y1_m01y2_m01y1 = IS::UnpackHi(m11y1_m01y1_m10x1_m00x1, m11y2_m01y2_m10x2_m00x2);
    Vector4f m10x1_m10x2_m00x1_m00x2 = IS::Shuffle<1,0,3,2>(m10x2_m10x1_m00x2_m00x1, m10x2_m10x1_m00x2_m00x1);

    Vector4f yAyBxAxB = IS::Add(m10x2_m10x1_m00x2_m00x1, m11y2_m11y1_m01y2_m01y1);
    Vector4f yCyDxCxD = IS::Add(m10x1_m10x2_m00x1_m00x2, m11y2_m11y1_m01y2_m01y1);

    Vector4f min0_yyxx = IS::Min(yAyBxAxB, yCyDxCxD);
    Vector4f max0_yyxx = IS::Max(yAyBxAxB, yCyDxCxD);

    Vector4f min_yyxx = IS::Min(min0_yyxx, IS::Shuffle<1,0,3,2>(min0_yyxx, min0_yyxx));
    Vector4f max_yyxx = IS::Max(max0_yyxx, IS::Shuffle<1,0,3,2>(max0_yyxx, max0_yyxx));

    Vector4f yxyx       = IS::Shuffle<0,2,0,2>(min_yyxx, max_yyxx);
    Vector4f add_yxyx   = IS::Shuffle<0,2,0,2>(add_yyxx, add_yyxx);
    Vector4f result     = IS::Add(yxyx, add_yxyx);

    IS::StoreAligned(&pr->x1, result);
}

template<>
void Matrix2F::SetToAppend( const Matrix2F& m0, const Matrix2F& m1 )
{
    using namespace Scaleform::SIMD;

    Vector4f c0001 = IS::Constant<0,0,0,0xFFFFFFFF>();
    Vector4f c1101 = IS::Constant<0xFFFFFFFF,0xFFFFFFFF,0,0xFFFFFFFF>();
    Vector4f m1v0  = IS::LoadAligned(m1.M[0]);
    Vector4f m1v1  = IS::LoadAligned(m1.M[1]);
    Vector4f m0v0  = IS::LoadAligned(m0.M[0]);
    Vector4f m0v1  = IS::LoadAligned(m0.M[1]);

    Vector4f res_v0 = IS::And( m1v0, c0001 );
    Vector4f res_v1 = IS::And( m1v1, c0001 );

    Vector4f t0 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v0, m1v0));
    Vector4f t1 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v1, m1v1));

    Vector4f t2 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v0, m1v0), t0);
    Vector4f t3 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v1, m1v1), t1);

    IS::StoreAligned(&M[0][0], IS::And( IS::Add(res_v0, t2), c1101) );
    IS::StoreAligned(&M[1][0], IS::And( IS::Add(res_v1, t3), c1101) );
}

template<> void Matrix2F::SetToAppend( const Matrix2F & m0, const Matrix2F & m1, const Matrix2F & m2 )
{
    using namespace SIMD;

    Vector4f c0001 = IS::Constant<0,0,0,0xFFFFFFFF>();
    Vector4f c1101 = IS::Constant<0xFFFFFFFF,0xFFFFFFFF,0,0xFFFFFFFF>();
    Vector4f m0v0  = IS::LoadAligned(m0.M[0]);
    Vector4f m0v1  = IS::LoadAligned(m0.M[1]);
    Vector4f m1v0  = IS::LoadAligned(m1.M[0]);
    Vector4f m1v1  = IS::LoadAligned(m1.M[1]);
    Vector4f m2v0  = IS::LoadAligned(m2.M[0]);
    Vector4f m2v1  = IS::LoadAligned(m2.M[1]);

    Vector4f res_v0 = IS::And( m2v0, c0001 );
    Vector4f res_v1 = IS::And( m2v1, c0001 );

    // First Prepend multiply: m1 * m2.
    Vector4f t0 = IS::Multiply( m1v0, IS::Shuffle<0,0,0,0>(m2v0, m2v0));
    Vector4f t1 = IS::Multiply( m1v0, IS::Shuffle<0,0,0,0>(m2v1, m2v1));

    Vector4f t2 = IS::MultiplyAdd( m1v1, IS::Shuffle<1,1,1,1>(m2v0, m2v0), t0);
    Vector4f t3 = IS::MultiplyAdd( m1v1, IS::Shuffle<1,1,1,1>(m2v1, m2v1), t1);

    Vector4f m12v0 = IS::Add(res_v0, t2);
    Vector4f m12v1 = IS::Add(res_v1, t3);

    // Second append multiply.
    res_v0 = IS::And( m12v0, c0001 );
    res_v1 = IS::And( m12v1, c0001 );

    t0 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m12v0, m12v0));
    t1 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m12v1, m12v1));

    t2 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m12v0, m12v0), t0);
    t3 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m12v1, m12v1), t1);

    IS::StoreAligned(&M[0][0], IS::And( IS::Add(res_v0, t2), c1101) );
    IS::StoreAligned(&M[1][0], IS::And( IS::Add(res_v1, t3), c1101) );
}

template<>
Matrix2F& Matrix2F::Append( const Matrix2F & m )
{
    using namespace Scaleform::SIMD;

    Vector4f c0001 = IS::Constant<0,0,0,0xFFFFFFFF>();
    Vector4f c1101 = IS::Constant<0xFFFFFFFF,0xFFFFFFFF,0,0xFFFFFFFF>();
    Vector4f m1v0  = IS::LoadAligned(m.M[0]);
    Vector4f m1v1  = IS::LoadAligned(m.M[1]);
    Vector4f m0v0  = IS::LoadAligned(M[0]);
    Vector4f m0v1  = IS::LoadAligned(M[1]);

    Vector4f res_v0 = IS::And( m1v0, c0001 );
    Vector4f res_v1 = IS::And( m1v1, c0001 );

    Vector4f t0 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v0, m1v0));
    Vector4f t1 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v1, m1v1));

    Vector4f t2 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v0, m1v0), t0);
    Vector4f t3 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v1, m1v1), t1);

    IS::StoreAligned(&M[0][0], IS::And( IS::Add(res_v0, t2), c1101) );
    IS::StoreAligned(&M[1][0], IS::And( IS::Add(res_v1, t3), c1101) );
    return *this;
}

#endif // SF_ENABLE_SIMD

}}
