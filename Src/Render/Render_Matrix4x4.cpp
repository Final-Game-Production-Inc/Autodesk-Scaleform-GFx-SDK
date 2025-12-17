/**************************************************************************

Filename    :   Render_Matrix4x4.cpp
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

namespace Scaleform { namespace Render {

#if (defined(SF_OS_WII) || defined(SF_OS_WIIU)) && !defined(SF_BUILD_DEBUG)

template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix4F &m2)
{
    // fp0-fp7:     m1
    // fp8-fp15:    m2
    // fp16-fp23:   result
    // fp24+        temporaries

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");
    asm("psq_l      fp4, 32(r4), 0, 0");
    asm("psq_l      fp5, 40(r4), 0, 0");
    asm("psq_l      fp6, 48(r4), 0, 0");
    asm("psq_l      fp7, 56(r4), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");
    asm("psq_l      fp9, 8(r5), 0, 0");
    asm("psq_l      fp10, 16(r5), 0, 0");
    asm("psq_l      fp11, 24(r5), 0, 0");
    asm("psq_l      fp12, 32(r5), 0, 0");
    asm("psq_l      fp13, 40(r5), 0, 0");
    asm("psq_l      fp14, 48(r5), 0, 0");
    asm("psq_l      fp15, 56(r5), 0, 0");

    asm("ps_merge00 fp24, fp0, fp0");
    asm("ps_merge00 fp25, fp2, fp2");
    asm("ps_merge00 fp26, fp4, fp4");
    asm("ps_merge00 fp27, fp6, fp6");

    asm("ps_mul     fp16, fp24, fp8");
    asm("ps_mul     fp17, fp24, fp9");
    asm("ps_mul     fp18, fp25, fp8");
    asm("ps_mul     fp19, fp25, fp9");
    asm("ps_mul     fp20, fp26, fp8");
    asm("ps_mul     fp21, fp26, fp9");
    asm("ps_mul     fp22, fp27, fp8");
    asm("ps_mul     fp23, fp27, fp9");

    asm("ps_merge11 fp24, fp0, fp0");
    asm("ps_merge11 fp25, fp2, fp2");
    asm("ps_merge11 fp26, fp4, fp4");
    asm("ps_merge11 fp27, fp6, fp6");

    asm("ps_madd    fp16, fp24, fp10, fp16");
    asm("ps_madd    fp17, fp24, fp11, fp17");
    asm("ps_madd    fp18, fp25, fp10, fp18");
    asm("ps_madd    fp19, fp25, fp11, fp19");
    asm("ps_madd    fp20, fp26, fp10, fp20");
    asm("ps_madd    fp21, fp26, fp11, fp21");
    asm("ps_madd    fp22, fp27, fp10, fp22");
    asm("ps_madd    fp23, fp27, fp11, fp23");

    asm("ps_merge00 fp24, fp1, fp1");
    asm("ps_merge00 fp25, fp3, fp3");
    asm("ps_merge00 fp26, fp5, fp5");
    asm("ps_merge00 fp27, fp7, fp7");

    asm("ps_madd    fp16, fp24, fp12, fp16");
    asm("ps_madd    fp17, fp24, fp13, fp17");
    asm("ps_madd    fp18, fp25, fp12, fp18");
    asm("ps_madd    fp19, fp25, fp13, fp19");
    asm("ps_madd    fp20, fp26, fp12, fp20");
    asm("ps_madd    fp21, fp26, fp13, fp21");
    asm("ps_madd    fp22, fp27, fp12, fp22");
    asm("ps_madd    fp23, fp27, fp13, fp23");

    asm("ps_merge11 fp24, fp1, fp1");
    asm("ps_merge11 fp25, fp3, fp3");
    asm("ps_merge11 fp26, fp5, fp5");
    asm("ps_merge11 fp27, fp7, fp7");

    asm("ps_madd    fp16, fp24, fp14, fp16");
    asm("ps_madd    fp17, fp24, fp15, fp17");
    asm("ps_madd    fp18, fp25, fp14, fp18");
    asm("ps_madd    fp19, fp25, fp15, fp19");
    asm("ps_madd    fp20, fp26, fp14, fp20");
    asm("ps_madd    fp21, fp26, fp15, fp21");
    asm("ps_madd    fp22, fp27, fp14, fp22");
    asm("ps_madd    fp23, fp27, fp15, fp23");

    asm("psq_st      fp16, 0(r3), 0, 0");
    asm("psq_st      fp17, 8(r3), 0, 0");
    asm("psq_st      fp18, 16(r3), 0, 0");
    asm("psq_st      fp19, 24(r3), 0, 0");
    asm("psq_st      fp20, 32(r3), 0, 0");
    asm("psq_st      fp21, 40(r3), 0, 0");
    asm("psq_st      fp22, 48(r3), 0, 0");
    asm("psq_st      fp23, 56(r3), 0, 0");
}

//specialize for 3x4 * 4x4
template<> void Matrix4F::MultiplyMatrix(const Matrix3F &m1, const Matrix4F &m2)
{
    // fp0-fp5:     m1
    // fp8-fp15:    m2
    // fp16-fp23:   result
    // fp24+        temporaries

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");
    asm("psq_l      fp4, 32(r4), 0, 0");
    asm("psq_l      fp5, 40(r4), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");
    asm("psq_l      fp9, 8(r5), 0, 0");
    asm("psq_l      fp10, 16(r5), 0, 0");
    asm("psq_l      fp11, 24(r5), 0, 0");
    asm("psq_l      fp12, 32(r5), 0, 0");
    asm("psq_l      fp13, 40(r5), 0, 0");
    asm("psq_l      fp14, 48(r5), 0, 0");
    asm("psq_l      fp15, 56(r5), 0, 0");

    asm("ps_merge00 fp24, fp0, fp0");
    asm("ps_merge00 fp25, fp2, fp2");
    asm("ps_merge00 fp26, fp4, fp4");

    asm("ps_mul     fp16, fp24, fp8");
    asm("ps_mul     fp17, fp24, fp9");
    asm("ps_mul     fp18, fp25, fp8");
    asm("ps_mul     fp19, fp25, fp9");
    asm("ps_mul     fp20, fp26, fp8");
    asm("ps_mul     fp21, fp26, fp9");

    asm("ps_merge11 fp24, fp0, fp0");
    asm("ps_merge11 fp25, fp2, fp2");
    asm("ps_merge11 fp26, fp4, fp4");

    asm("ps_madd    fp16, fp24, fp10, fp16");
    asm("ps_madd    fp17, fp24, fp11, fp17");
    asm("ps_madd    fp18, fp25, fp10, fp18");
    asm("ps_madd    fp19, fp25, fp11, fp19");
    asm("ps_madd    fp20, fp26, fp10, fp20");
    asm("ps_madd    fp21, fp26, fp11, fp21");

    asm("ps_merge00 fp24, fp1, fp1");
    asm("ps_merge00 fp25, fp3, fp3");
    asm("ps_merge00 fp26, fp5, fp5");

    asm("ps_madd    fp16, fp24, fp12, fp16");
    asm("ps_madd    fp17, fp24, fp13, fp17");
    asm("ps_madd    fp18, fp25, fp12, fp18");
    asm("ps_madd    fp19, fp25, fp13, fp19");
    asm("ps_madd    fp20, fp26, fp12, fp20");
    asm("ps_madd    fp21, fp26, fp13, fp21");

    asm("ps_merge11 fp24, fp1, fp1");
    asm("ps_merge11 fp25, fp3, fp3");
    asm("ps_merge11 fp26, fp5, fp5");

    asm("ps_madd    fp16, fp24, fp14, fp16");
    asm("ps_madd    fp17, fp24, fp15, fp17");
    asm("ps_madd    fp18, fp25, fp14, fp18");
    asm("ps_madd    fp19, fp25, fp15, fp19");
    asm("ps_madd    fp20, fp26, fp14, fp20");
    asm("ps_madd    fp21, fp26, fp15, fp21");

    asm("psq_st      fp16, 0(r3), 0, 0");
    asm("psq_st      fp17, 8(r3), 0, 0");
    asm("psq_st      fp18, 16(r3), 0, 0");
    asm("psq_st      fp19, 24(r3), 0, 0");
    asm("psq_st      fp20, 32(r3), 0, 0");
    asm("psq_st      fp21, 40(r3), 0, 0");
    asm("psq_st      fp14, 48(r3), 0, 0");
    asm("psq_st      fp15, 56(r3), 0, 0");
}

template<> void Matrix4F::MultiplyMatrix_Opt(const Matrix4F &m1, const Matrix3F &m2, float* mask)
{
    // fp0-fp7:     m1
    // fp8-fp13:    m2
    // fp14:        mask
    // fp16-fp23:   result
    // fp24+        temporaries

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");
    asm("psq_l      fp4, 32(r4), 0, 0");
    asm("psq_l      fp5, 40(r4), 0, 0");
    asm("psq_l      fp6, 48(r4), 0, 0");
    asm("psq_l      fp7, 56(r4), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");
    asm("psq_l      fp9, 8(r5), 0, 0");
    asm("psq_l      fp10, 16(r5), 0, 0");
    asm("psq_l      fp11, 24(r5), 0, 0");
    asm("psq_l      fp12, 32(r5), 0, 0");
    asm("psq_l      fp13, 40(r5), 0, 0");

    asm("psq_l      fp14, 0(r6), 0, 0");

    asm("ps_merge00 fp24, fp0, fp0");
    asm("ps_merge00 fp25, fp2, fp2");
    asm("ps_merge00 fp26, fp4, fp4");
    asm("ps_merge00 fp27, fp6, fp6");

    asm("ps_mul     fp16, fp24, fp8");
    asm("ps_mul     fp17, fp24, fp9");
    asm("ps_mul     fp18, fp25, fp8");
    asm("ps_mul     fp19, fp25, fp9");
    asm("ps_mul     fp20, fp26, fp8");
    asm("ps_mul     fp21, fp26, fp9");
    asm("ps_mul     fp22, fp27, fp8");
    asm("ps_mul     fp23, fp27, fp9");

    asm("ps_merge11 fp24, fp0, fp0");
    asm("ps_merge11 fp25, fp2, fp2");
    asm("ps_merge11 fp26, fp4, fp4");
    asm("ps_merge11 fp27, fp6, fp6");

    asm("ps_madd    fp16, fp24, fp10, fp16");
    asm("ps_madd    fp17, fp24, fp11, fp17");
    asm("ps_madd    fp18, fp25, fp10, fp18");
    asm("ps_madd    fp19, fp25, fp11, fp19");
    asm("ps_madd    fp20, fp26, fp10, fp20");
    asm("ps_madd    fp21, fp26, fp11, fp21");
    asm("ps_madd    fp22, fp27, fp10, fp22");
    asm("ps_madd    fp23, fp27, fp11, fp23");

    asm("ps_merge00 fp24, fp1, fp1");
    asm("ps_merge00 fp25, fp3, fp3");
    asm("ps_merge00 fp26, fp5, fp5");
    asm("ps_merge00 fp27, fp7, fp7");

    asm("ps_madd    fp16, fp24, fp12, fp16");
    asm("ps_madd    fp17, fp24, fp13, fp17");
    asm("ps_madd    fp18, fp25, fp12, fp18");
    asm("ps_madd    fp19, fp25, fp13, fp19");
    asm("ps_madd    fp20, fp26, fp12, fp20");
    asm("ps_madd    fp21, fp26, fp13, fp21");
    asm("ps_madd    fp22, fp27, fp12, fp22");
    asm("ps_madd    fp23, fp27, fp13, fp23");

    asm("ps_madd    fp17, fp1, fp14, fp17");
    asm("ps_madd    fp19, fp3, fp14, fp19");
    asm("ps_madd    fp21, fp5, fp14, fp21");
    asm("ps_madd    fp23, fp7, fp14, fp23");

    asm("psq_st      fp16, 0(r3), 0, 0");
    asm("psq_st      fp17, 8(r3), 0, 0");
    asm("psq_st      fp18, 16(r3), 0, 0");
    asm("psq_st      fp19, 24(r3), 0, 0");
    asm("psq_st      fp20, 32(r3), 0, 0");
    asm("psq_st      fp21, 40(r3), 0, 0");
    asm("psq_st      fp22, 48(r3), 0, 0");
    asm("psq_st      fp23, 56(r3), 0, 0");
}

template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix3F &m2)
{
    float mask[] = { 0.0f, 1.0f };
    MultiplyMatrix_Opt(m1, m2, mask);
}

//specialize for 2x4 * 4x4
template<> void Matrix4F::MultiplyMatrix(const Matrix2F &m1, const Matrix4F &m2)
{
    // fp0-fp3:     m1
    // fp8-fp15:    m2
    // fp16-fp23:   result
    // fp24+        temporaries

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");
    asm("psq_l      fp9, 8(r5), 0, 0");
    asm("psq_l      fp10, 16(r5), 0, 0");
    asm("psq_l      fp11, 24(r5), 0, 0");
    asm("psq_l      fp12, 32(r5), 0, 0");
    asm("psq_l      fp13, 40(r5), 0, 0");
    asm("psq_l      fp14, 48(r5), 0, 0");
    asm("psq_l      fp15, 56(r5), 0, 0");

    asm("ps_merge00 fp24, fp0, fp0");
    asm("ps_merge00 fp25, fp2, fp2");

    asm("ps_mul     fp16, fp24, fp8");
    asm("ps_mul     fp17, fp24, fp9");
    asm("ps_mul     fp18, fp25, fp8");
    asm("ps_mul     fp19, fp25, fp9");

    asm("ps_merge11 fp24, fp0, fp0");
    asm("ps_merge11 fp25, fp2, fp2");

    asm("ps_madd    fp16, fp24, fp10, fp16");
    asm("ps_madd    fp17, fp24, fp11, fp17");
    asm("ps_madd    fp18, fp25, fp10, fp18");
    asm("ps_madd    fp19, fp25, fp11, fp19");

    asm("ps_merge00 fp24, fp1, fp1");
    asm("ps_merge00 fp25, fp3, fp3");

    asm("ps_madd    fp16, fp24, fp12, fp16");
    asm("ps_madd    fp17, fp24, fp13, fp17");
    asm("ps_madd    fp18, fp25, fp12, fp18");
    asm("ps_madd    fp19, fp25, fp13, fp19");

    asm("ps_merge11 fp24, fp1, fp1");
    asm("ps_merge11 fp25, fp3, fp3");

    asm("ps_madd    fp16, fp24, fp14, fp16");
    asm("ps_madd    fp17, fp24, fp15, fp17");
    asm("ps_madd    fp18, fp25, fp14, fp18");
    asm("ps_madd    fp19, fp25, fp15, fp19");

    asm("psq_st      fp16, 0(r3), 0, 0");
    asm("psq_st      fp17, 8(r3), 0, 0");
    asm("psq_st      fp18, 16(r3), 0, 0");
    asm("psq_st      fp19, 24(r3), 0, 0");
    asm("psq_st      fp12, 32(r3), 0, 0");
    asm("psq_st      fp13, 40(r3), 0, 0");
    asm("psq_st      fp14, 48(r3), 0, 0");
    asm("psq_st      fp15, 56(r3), 0, 0");
}

// specialize for 4x4 * 2x4
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix2F &m2)
{
    // fp0-fp7:     m1
    // fp8-fp11:    m2
    // fp16-fp23:   result
    // fp24+        temporaries

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");
    asm("psq_l      fp3, 24(r4), 0, 0");
    asm("psq_l      fp4, 32(r4), 0, 0");
    asm("psq_l      fp5, 40(r4), 0, 0");
    asm("psq_l      fp6, 48(r4), 0, 0");
    asm("psq_l      fp7, 56(r4), 0, 0");

    asm("psq_l      fp8, 0(r5), 0, 0");
    asm("psq_l      fp9, 8(r5), 0, 0");
    asm("psq_l      fp10, 16(r5), 0, 0");
    asm("psq_l      fp11, 24(r5), 0, 0");

    asm("ps_merge00 fp24, fp0, fp0");
    asm("ps_merge00 fp25, fp2, fp2");
    asm("ps_merge00 fp26, fp4, fp4");
    asm("ps_merge00 fp27, fp6, fp6");

    asm("ps_mul     fp16, fp24, fp8");
    asm("ps_mul     fp17, fp24, fp9");
    asm("ps_mul     fp18, fp25, fp8");
    asm("ps_mul     fp19, fp25, fp9");
    asm("ps_mul     fp20, fp26, fp8");
    asm("ps_mul     fp21, fp26, fp9");
    asm("ps_mul     fp22, fp27, fp8");
    asm("ps_mul     fp23, fp27, fp9");

    asm("ps_merge11 fp24, fp0, fp0");
    asm("ps_merge11 fp25, fp2, fp2");
    asm("ps_merge11 fp26, fp4, fp4");
    asm("ps_merge11 fp27, fp6, fp6");

    asm("ps_madd    fp16, fp24, fp10, fp16");
    asm("ps_madd    fp17, fp24, fp11, fp17");
    asm("ps_madd    fp18, fp25, fp10, fp18");
    asm("ps_madd    fp19, fp25, fp11, fp19");
    asm("ps_madd    fp20, fp26, fp10, fp20");
    asm("ps_madd    fp21, fp26, fp11, fp21");
    asm("ps_madd    fp22, fp27, fp10, fp22");
    asm("ps_madd    fp23, fp27, fp11, fp23");

    asm("ps_add    fp17, fp1, fp17");
    asm("ps_add    fp19, fp3, fp19");
    asm("ps_add    fp21, fp5, fp21");
    asm("ps_add    fp23, fp7, fp23");

    asm("psq_st      fp16, 0(r3), 0, 0");
    asm("psq_st      fp17, 8(r3), 0, 0");
    asm("psq_st      fp18, 16(r3), 0, 0");
    asm("psq_st      fp19, 24(r3), 0, 0");
    asm("psq_st      fp20, 32(r3), 0, 0");
    asm("psq_st      fp21, 40(r3), 0, 0");
    asm("psq_st      fp22, 48(r3), 0, 0");
    asm("psq_st      fp23, 56(r3), 0, 0");
}

#endif

#if 0 // Causes culling issues, to be fixed later

template<> void Matrix4F::EncloseTransformHomogeneous(RectF *pr, const RectF& r) const
{
    // fp0-fp1:     r0
    // fp2-fp9:     M[0]-M[3]
    // fp10-fp11:   t0
    // fp12-fp13:   t1
    // fp14-fp15:   mc3
    // fp16:        tl
    // fp17:        tr
    // fp18:        br
    // fp19:        bl
    // fp20-fp23:   temporaries
    // fp24-fp27:   p0-p3
    // fp28:        min
    // fp29:        max
    // fp30:        Splat<3>(mc3)

    asm("psq_l      fp0, 0(r5), 0, 0");
    asm("psq_l      fp1, 8(r5), 0, 0");

    asm("psq_l      fp2, 0(r3), 0, 0");
    asm("psq_l      fp3, 8(r3), 0, 0");
    asm("psq_l      fp4, 16(r3), 0, 0");
    asm("psq_l      fp5, 24(r3), 0, 0");
    asm("psq_l      fp6, 32(r3), 0, 0");
    asm("psq_l      fp7, 40(r3), 0, 0");
    asm("psq_l      fp8, 48(r3), 0, 0");
    asm("psq_l      fp9, 56(r3), 0, 0");

    asm("ps_merge11 fp10, fp3, fp3");
    asm("ps_merge11 fp11, fp5, fp5");
    asm("ps_merge11 fp12, fp7, fp7");
    asm("ps_merge11 fp13, fp9, fp9");

    asm("ps_merge00 fp14, fp10, fp11");
    asm("ps_merge00 fp15, fp12, fp13");
    asm("ps_merge11 fp30, fp15, fp15");

    // Make the corners of the rectangle.
    asm("ps_merge01 fp16, fp0, fp0");
    asm("ps_merge01 fp17, fp1, fp0");
    asm("ps_merge01 fp18, fp1, fp1");
    asm("ps_merge01 fp19, fp0, fp1");

    // Transform each corner by the matrix, divide the result by W.
    asm("ps_mul     fp20, fp16, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp16, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp16, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp24, fp20, fp21"); // p0 = IS::UnpackLo( x, y );
    asm("ps_add     fp24, fp24, fp14"); // p0 = IS::Add(p0, mc3);
    asm("ps_div     fp24, fp24, fp22"); // p0 = IS::Divide( p0, w );

    asm("ps_mul     fp20, fp17, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp17, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp17, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp25, fp20, fp21"); // p1 = IS::UnpackLo( x, y );
    asm("ps_add     fp25, fp25, fp14"); // p1 = IS::Add(p1, mc3);
    asm("ps_div     fp25, fp25, fp22"); // p1 = IS::Divide(p1, w);

    asm("ps_mul     fp20, fp18, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp18, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp18, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp26, fp20, fp21"); // p2 = IS::UnpackLo( x, y );
    asm("ps_add     fp26, fp26, fp14"); // p2 = IS::Add(p2, mc3);
    asm("ps_div     fp26, fp26, fp22"); // p2 = IS::Divide(p2, w);

    asm("ps_mul     fp20, fp19, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp19, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp19, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp27, fp20, fp21"); // p3 = IS::UnpackLo( x, y );
    asm("ps_add     fp27, fp27, fp14"); // p3 = IS::Add(p3, mc3);
    asm("ps_div     fp27, fp27, fp22"); // p3 = IS::Divide(p3, w);

    // Find the minima of the bounds
    asm("ps_sub     fp20, fp24, fp25");
    asm("ps_sel     fp28, fp20, fp25, fp24");
    asm("ps_sel     fp29, fp20, fp24, fp25");
    asm("ps_sub     fp20, fp28, fp26");
    asm("ps_sel     fp28, fp20, fp26, fp28");
    asm("ps_sub     fp20, fp28, fp27");
    asm("ps_sel     fp28, fp20, fp27, fp28");

    // Find the maxima of the bounds
    asm("ps_sub     fp20, fp29, fp26");
    asm("ps_sel     fp29, fp20, fp29, fp26");
    asm("ps_sub     fp20, fp29, fp27");
    asm("ps_sel     fp29, fp20, fp29, fp27");

    asm("ps_merge01 fp28, fp28, fp28");
    asm("ps_merge01 fp29, fp29, fp29");

    // IS::StoreAligned( (float*)pr, result );
    asm("psq_st      fp28, 0(r4), 0, 0");
    asm("psq_st      fp29, 8(r4), 0, 0");
}

template<> void Matrix4F::TransformHomogeneousAndScaleCorners(const RectF& bounds, float sx, float sy, float* dest) const
{
    static f32x2 c11 = (f32x2){ 1.0f,  1.0f };
    static f32x2 c1N = (f32x2){ 1.0f, -1.0f };
    static f32x2 cHALF = (f32x2){ 0.5f,  0.5f };
    f32x2 sxsy = (f32x2){ sx, sy };

    __SETREG(8, (unsigned int)&c11);
    __SETREG(9, (unsigned int)&c1N);
    __SETREG(10, (unsigned int)&cHALF);
    __SETREG(11, (unsigned int)&sxsy);

    // fp0-fp1:     r0
    // fp2-fp9:     M[0]-M[3]
    // fp10-fp11:   t0
    // fp12-fp13:   t1
    // fp14-fp15:   mc3
    // fp16:        tl
    // fp17:        tr
    // fp18:        br
    // fp19-fp23:   temporaries
    // fp24-fp26:   p0-p2
    // fp30:        Splat<3>(mc3)

    asm("psq_l      fp0, 0(r4), 0, 0");
    asm("psq_l      fp1, 8(r4), 0, 0");

    asm("psq_l      fp2, 0(r3), 0, 0");
    asm("psq_l      fp3, 8(r3), 0, 0");
    asm("psq_l      fp4, 16(r3), 0, 0");
    asm("psq_l      fp5, 24(r3), 0, 0");
    asm("psq_l      fp6, 32(r3), 0, 0");
    asm("psq_l      fp7, 40(r3), 0, 0");
    asm("psq_l      fp8, 48(r3), 0, 0");
    asm("psq_l      fp9, 56(r3), 0, 0");

    asm("ps_merge11 fp10, fp3, fp3");
    asm("ps_merge11 fp11, fp5, fp5");
    asm("ps_merge11 fp12, fp7, fp7");
    asm("ps_merge11 fp13, fp9, fp9");

    asm("ps_merge00 fp14, fp10, fp11");
    asm("ps_merge00 fp15, fp12, fp13");
    asm("ps_merge11 fp30, fp15, fp15");

    // Make three corners of the rectangle.
    asm("ps_merge01 fp16, fp0, fp0");
    asm("ps_merge01 fp17, fp1, fp0");
    asm("ps_merge01 fp18, fp1, fp1");

    // Transform each corner by the matrix, divide the result by W.
    asm("ps_mul     fp20, fp16, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp16, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp16, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp24, fp20, fp21"); // p0 = IS::UnpackLo( x, y );
    asm("ps_add     fp24, fp24, fp14"); // p0 = IS::Add(p0, mc3);
    asm("ps_div     fp24, fp24, fp22"); // p0 = IS::Divide( p0, w );

    asm("ps_mul     fp20, fp17, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp17, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp17, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp25, fp20, fp21"); // p1 = IS::UnpackLo( x, y );
    asm("ps_add     fp25, fp25, fp14"); // p1 = IS::Add(p1, mc3);
    asm("ps_div     fp25, fp25, fp22"); // p1 = IS::Divide(p1, w);

    asm("ps_mul     fp20, fp18, fp2");
    asm("ps_merge00 fp28, fp20, fp20");
    asm("ps_merge11 fp29, fp20, fp20");
    asm("ps_add     fp20, fp28, fp29"); // fp20 now contains xx
    asm("ps_mul     fp21, fp18, fp4");
    asm("ps_merge00 fp28, fp21, fp21");
    asm("ps_merge11 fp29, fp21, fp21");
    asm("ps_add     fp21, fp28, fp29"); // fp21 now contains yy
    asm("ps_mul     fp22, fp18, fp8");
    asm("ps_merge00 fp28, fp22, fp22");
    asm("ps_merge11 fp29, fp22, fp22");
    asm("ps_add     fp22, fp28, fp29"); // fp22 now contains ww
    asm("ps_add     fp22, fp22, fp30");
    asm("ps_merge00 fp26, fp20, fp21"); // p2 = IS::UnpackLo( x, y );
    asm("ps_add     fp26, fp26, fp14"); // p2 = IS::Add(p2, mc3);
    asm("ps_div     fp26, fp26, fp22"); // p2 = IS::Divide(p2, w);

    // At this point, p0-p2 are in fp24-fp26, and we can re-use registers as necessary
    asm("psq_l      fp0, 0(r8), 0, 0");
    asm("psq_l      fp1, 0(r9), 0, 0");
    asm("psq_l      fp2, 0(r10), 0, 0");
    asm("psq_l      fp3, 0(r11), 0, 0");

    asm("ps_mul     fp3, fp3, fp2");

    asm("ps_merge01 fp4, fp24, fp24");
    asm("ps_merge01 fp5, fp25, fp25");

    asm("ps_madd    fp4, fp4, fp1, fp0");
    asm("ps_madd    fp5, fp5, fp1, fp0");
    asm("ps_mul     fp4, fp4, fp3");
    asm("ps_mul     fp5, fp5, fp3");

    asm("ps_madd    fp6, fp26, fp1, fp0");
    asm("ps_mul     fp6, fp6, fp3");

    asm("psq_st      fp4, 0(r5), 0, 0");
    asm("psq_st      fp5, 8(r5), 0, 0");
    asm("psq_st      fp6, 16(r5), 0, 0");
}
#endif

#if defined(SF_ENABLE_SIMD)

template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix4F &m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);
    Vector4f m12 = IS::LoadAligned(m1.M[2]);
    Vector4f m13 = IS::LoadAligned(m1.M[3]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);
    Vector4f m22 = IS::LoadAligned(m2.M[2]);
    Vector4f m23 = IS::LoadAligned(m2.M[3]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2 = IS::Multiply   ( IS::Splat<0>(m12), m20 );
    Vector4f r3 = IS::Multiply   ( IS::Splat<0>(m13), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<1>(m13), m21, r3 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<2>(m12), m22, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<2>(m13), m22, r3 );

    r0 =          IS::MultiplyAdd( IS::Splat<3>(m10), m23, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<3>(m11), m23, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<3>(m12), m23, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<3>(m13), m23, r3 );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
    IS::StoreAligned(M[3], r3);
}

//specialize for 3x4 * 4x4
template<> void Matrix4F::MultiplyMatrix(const Matrix3F &m1, const Matrix4F &m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);
    Vector4f m12 = IS::LoadAligned(m1.M[2]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);
    Vector4f m22 = IS::LoadAligned(m2.M[2]);
    Vector4f m23 = IS::LoadAligned(m2.M[3]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2 = IS::Multiply   ( IS::Splat<0>(m12), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<2>(m12), m22, r2 );

    r0 =          IS::MultiplyAdd( IS::Splat<3>(m10), m23, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<3>(m11), m23, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<3>(m12), m23, r2 );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
    IS::StoreAligned(M[3], m23);
}

// specialize for 4x4 * 3x4
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix3F &m2)
{
    using namespace SIMD;

    Vector4f c0001 = IS::Constant<0,0,0,0xFFFFFFFF>();

    Vector4f m10   = IS::LoadAligned(m1.M[0]);
    Vector4f m11   = IS::LoadAligned(m1.M[1]);
    Vector4f m12   = IS::LoadAligned(m1.M[2]);
    Vector4f m13   = IS::LoadAligned(m1.M[3]);

    Vector4f m20   = IS::LoadAligned(m2.M[0]);
    Vector4f m21   = IS::LoadAligned(m2.M[1]);
    Vector4f m22   = IS::LoadAligned(m2.M[2]);

    Vector4f r0    = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1    = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2    = IS::Multiply   ( IS::Splat<0>(m12), m20 );
    Vector4f r3    = IS::Multiply   ( IS::Splat<0>(m13), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<1>(m13), m21, r3 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<2>(m12), m22, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<2>(m13), m22, r3 );

    r0 =          IS::Add( r0, IS::And( m10, c0001 ) );
    r1 =          IS::Add( r1, IS::And( m11, c0001 ) );
    r2 =          IS::Add( r2, IS::And( m12, c0001 ) );
    r3 =          IS::Add( r3, IS::And( m13, c0001 ) );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
    IS::StoreAligned(M[3], r3);
}

//specialize for 2x4 * 4x4
template<> void Matrix4F::MultiplyMatrix(const Matrix2F &m1, const Matrix4F &m2)
{
    using namespace SIMD;

    Vector4f m10 = IS::LoadAligned(m1.M[0]);
    Vector4f m11 = IS::LoadAligned(m1.M[1]);

    Vector4f m20 = IS::LoadAligned(m2.M[0]);
    Vector4f m21 = IS::LoadAligned(m2.M[1]);
    Vector4f m22 = IS::LoadAligned(m2.M[2]);
    Vector4f m23 = IS::LoadAligned(m2.M[3]);

    Vector4f r0 = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1 = IS::Multiply   ( IS::Splat<0>(m11), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );

    r0 =          IS::MultiplyAdd( IS::Splat<2>(m10), m22, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<2>(m11), m22, r1 );

    r0 =          IS::MultiplyAdd( IS::Splat<3>(m10), m23, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<3>(m11), m23, r1 );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], m22);
    IS::StoreAligned(M[3], m23);
}

// specialize for 4x4 * 2x4
template<> void Matrix4F::MultiplyMatrix(const Matrix4F &m1, const Matrix2F &m2)
{
    using namespace SIMD;

    Vector4f c0011 = IS::Constant<0,0,0xFFFFFFFF,0xFFFFFFFF>();

    Vector4f m10   = IS::LoadAligned(m1.M[0]);
    Vector4f m11   = IS::LoadAligned(m1.M[1]);
    Vector4f m12   = IS::LoadAligned(m1.M[2]);
    Vector4f m13   = IS::LoadAligned(m1.M[3]);

    Vector4f m20   = IS::LoadAligned(m2.M[0]);
    Vector4f m21   = IS::LoadAligned(m2.M[1]);

    Vector4f r0    = IS::Multiply   ( IS::Splat<0>(m10), m20 );
    Vector4f r1    = IS::Multiply   ( IS::Splat<0>(m11), m20 );
    Vector4f r2    = IS::Multiply   ( IS::Splat<0>(m12), m20 );
    Vector4f r3    = IS::Multiply   ( IS::Splat<0>(m13), m20 );

    r0 =          IS::MultiplyAdd( IS::Splat<1>(m10), m21, r0 );
    r1 =          IS::MultiplyAdd( IS::Splat<1>(m11), m21, r1 );
    r2 =          IS::MultiplyAdd( IS::Splat<1>(m12), m21, r2 );
    r3 =          IS::MultiplyAdd( IS::Splat<1>(m13), m21, r3 );

    r0 =          IS::Add( r0, IS::And( m10, c0011 ) );
    r1 =          IS::Add( r1, IS::And( m11, c0011 ) );
    r2 =          IS::Add( r2, IS::And( m12, c0011 ) );
    r3 =          IS::Add( r3, IS::And( m13, c0011 ) );

    IS::StoreAligned(M[0], r0);
    IS::StoreAligned(M[1], r1);
    IS::StoreAligned(M[2], r2);
    IS::StoreAligned(M[3], r3);
}

template<> void Matrix4F::EncloseTransformHomogeneous(RectF *pr, const RectF& r) const
{
    using namespace SIMD;

    Vector4f c0000 = IS::Constant<0,0,0,0>();

    Vector4f r0 = IS::LoadAligned(&r.x1);
    Vector4f m0 = IS::LoadAligned(M[0]);
    Vector4f m1 = IS::LoadAligned(M[1]);
    Vector4f m2 = IS::LoadAligned(M[2]);
    Vector4f m3 = IS::LoadAligned(M[3]);

    Vector4f t0  = IS::Shuffle<3,3,3,3>( m0, m1);
    Vector4f t1  = IS::Shuffle<3,3,3,3>( m2, m3);
    Vector4f mc3 = IS::Shuffle<0,2,0,2>( t0, t1 );

    // Make the corners of the rectangle.
    Vector4f tl = IS::Shuffle<0, 1, 0, 0>(r0, c0000);
    Vector4f tr = IS::Shuffle<2, 1, 0, 0>(r0, c0000);
    Vector4f br = IS::Shuffle<2, 3, 0, 0>(r0, c0000);
    Vector4f bl = IS::Shuffle<0, 3, 0, 0>(r0, c0000);

    Vector4f x,y,z,w;
    Vector4f p0,p1,p2,p3;

    // Transform each corner by the matrix, divide the result by W.
    x = IS::Dot3( tl, m0 );
    y = IS::Dot3( tl, m1 );
    z = IS::Dot3( tl, m2 );
    w = IS::Dot3( tl, m3 );
    w = IS::Add( w, IS::Splat<3>(mc3));
    p0 = IS::UnpackLo( x, y );
    p0 = IS::Shuffle<0,1,0,0>( p0, z);
    p0 = IS::Add(p0, mc3);
    p0 = IS::Divide( p0, w );

    x = IS::Dot3( tr, m0 );
    y = IS::Dot3( tr, m1 );
    z = IS::Dot3( tr, m2 );
    w = IS::Dot3( tr, m3 );
    w = IS::Add( w, IS::Splat<3>(mc3));
    p1 = IS::UnpackLo( x, y );
    p1 = IS::Shuffle<0,1,0,0>( p1, z);
    p1 = IS::Add(p1, mc3);
    p1 = IS::Divide( p1, w );

    x = IS::Dot3( br, m0 );
    y = IS::Dot3( br, m1 );
    z = IS::Dot3( br, m2 );
    w = IS::Dot3( br, m3 );
    w = IS::Add( w, IS::Splat<3>(mc3));
    p2 = IS::UnpackLo( x, y );
    p2 = IS::Shuffle<0,1,0,0>( p2, z);
    p2 = IS::Add(p2, mc3);
    p2 = IS::Divide( p2, w );

    x = IS::Dot3( bl, m0 );
    y = IS::Dot3( bl, m1 );
    z = IS::Dot3( bl, m2 );
    w = IS::Dot3( bl, m3 );
    w = IS::Add( w, IS::Splat<3>(mc3));
    p3 = IS::UnpackLo( x, y );
    p3 = IS::Shuffle<0,1,0,0>( p3, z);
    p3 = IS::Add(p3, mc3);
    p3 = IS::Divide( p3, w );

    // Find the maxima and minima of the bounds.
    Vector4f min, max;
    min = IS::Min(p0, p1);
    min = IS::Min(min, p2);
    min = IS::Min(min, p3);
    max = IS::Max(p0, p1);
    max = IS::Max(max, p2);
    max = IS::Max(max, p3);

    Vector4f result;
    result = IS::Shuffle<0,1,0,1>(min,max);
    IS::StoreAligned( (float*)pr, result );
}

template<> void Matrix4F::TransformHomogeneousAndScaleCorners(const RectF& bounds, float sx, float sy, float* dest) const
{
    using namespace SIMD;

    SF_SIMD_ALIGN(float sxsy_scalar[4]);
    sxsy_scalar[0] = sx;
    sxsy_scalar[1] = sy;
    Vector4f sxsy = IS::LoadAligned(sxsy_scalar);
    sxsy = IS::Shuffle<0,1,0,1>(sxsy, sxsy);

    // Assume IEEE754.
    Vector4f c0000 = IS::Constant<0,0,0,0>();
    Vector4f c1N1N = IS::Constant<0x3F800000,0xBF800000,0x3F800000,0xBF800000>();
    Vector4f c1111 = IS::Constant<0x3F800000,0x3F800000,0x3F800000,0x3F800000>();
    Vector4f cHALF = IS::Constant<0x3F000000,0x3F000000,0x3F000000,0x3F000000>();

    Vector4f r0 = IS::LoadAligned(&bounds.x1);
    Vector4f m0 = IS::LoadAligned(M[0]);
    Vector4f m1 = IS::LoadAligned(M[1]);
    Vector4f m2 = IS::LoadAligned(M[2]);
    Vector4f m3 = IS::LoadAligned(M[3]);

    Vector4f t0  = IS::Shuffle<3,3,3,3>( m0, m1);
    Vector4f t1  = IS::Shuffle<3,3,3,3>( m2, m3);
    Vector4f mc3 = IS::Shuffle<0,2,0,2>( t0, t1 );

    // Make three corners of the rectangle.
    Vector4f tl = IS::Shuffle<0, 1, 0, 0>(r0, c0000);
    Vector4f tr = IS::Shuffle<2, 1, 0, 0>(r0, c0000);
    Vector4f br = IS::Shuffle<2, 3, 0, 0>(r0, c0000);

    Vector4f x,y,z,w;
    Vector4f p0,p1,p2;

    Vector4f mc3_4 = IS::Splat<3>(mc3);
    // Transform each corner by the matrix, divide the result by W.
    x = IS::Dot3( tl, m0 );
    y = IS::Dot3( tl, m1 );
    z = IS::Dot3( tl, m2 );
    w = IS::Dot3( tl, m3 );
    w = IS::Add( w, mc3_4 );
    p0 = IS::UnpackLo( x, y );
    p0 = IS::Shuffle<0,1,0,0>( p0, z);
    p0 = IS::Add(p0, mc3);
    p0 = IS::Divide( p0, w );

    x = IS::Dot3( tr, m0 );
    y = IS::Dot3( tr, m1 );
    z = IS::Dot3( tr, m2 );
    w = IS::Dot3( tr, m3 );
    w = IS::Add( w, mc3_4);
    p1 = IS::UnpackLo( x, y );
    p1 = IS::Shuffle<0,1,0,0>( p1, z);
    p1 = IS::Add(p1, mc3);
    p1 = IS::Divide( p1, w );

    x = IS::Dot3( br, m0 );
    y = IS::Dot3( br, m1 );
    z = IS::Dot3( br, m2 );
    w = IS::Dot3( br, m3 );
    w = IS::Add( w, mc3_4);
    p2 = IS::UnpackLo( x, y );
    p2 = IS::Shuffle<0,1,0,0>( p2, z);
    p2 = IS::Add(p2, mc3);
    p2 = IS::Divide( p2, w );

    Vector4f sxsy_2 = IS::Multiply( sxsy, cHALF);
    Vector4f out0 = IS::Multiply( IS::Add( IS::Multiply(IS::Shuffle<0,1,0,1>(p0,p1), c1N1N), c1111), sxsy_2 );
    Vector4f out1 = IS::Multiply( IS::Add( IS::Multiply(p2, c1N1N), c1111), sxsy_2 );

    IS::StoreAligned(dest, out0);
    IS::StoreAligned(dest+4, out1);
}

#endif // SF_ENABLE_SIMD


}} // Scaleform::Render
