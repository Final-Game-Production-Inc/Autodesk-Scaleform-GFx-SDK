/**************************************************************************

Filename    :   Render_CxForm.cpp
Content     :   Color transform for renderer.
Created     :   August 17, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_CxForm.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

Cxform   Cxform::Identity;

// Initialize to identity transform.
Cxform::Cxform()
{
    SetIdentity();
}

// Prepend c's transform onto ours.  When
// transforming colors, c's transform is applied
// first, then ours.
void Cxform::Prepend_NonOpt(const Cxform& c)
{
    M[1][0] += M[0][0] * c.M[1][0];
    M[1][1] += M[0][1] * c.M[1][1];
    M[1][2] += M[0][2] * c.M[1][2];
    M[1][3] += M[0][3] * c.M[1][3];

    M[0][0] *= c.M[0][0];
    M[0][1] *= c.M[0][1];
    M[0][2] *= c.M[0][2];
    M[0][3] *= c.M[0][3];
}

void Cxform::Append_NonOpt(const Cxform& c)
{
    M[1][0] = c.M[1][0] + c.M[0][0] * M[1][0];
    M[1][1] = c.M[1][1] + c.M[0][1] * M[1][1];
    M[1][2] = c.M[1][2] + c.M[0][2] * M[1][2];
    M[1][3] = c.M[1][3] + c.M[0][3] * M[1][3];

    M[0][0] *= c.M[0][0];
    M[0][1] *= c.M[0][1];
    M[0][2] *= c.M[0][2];
    M[0][3] *= c.M[0][3];
}

void Cxform::SetToAppend_NonOpt(const Cxform& c0, const Cxform& c1)
{
    M[1][0] = c1.M[1][0] + c1.M[0][0] * c0.M[1][0];
    M[1][1] = c1.M[1][1] + c1.M[0][1] * c0.M[1][1];
    M[1][2] = c1.M[1][2] + c1.M[0][2] * c0.M[1][2];
    M[1][3] = c1.M[1][3] + c1.M[0][3] * c0.M[1][3];

    M[0][0] = c0.M[0][0] * c1.M[0][0];
    M[0][1] = c0.M[0][1] * c1.M[0][1];
    M[0][2] = c0.M[0][2] * c1.M[0][2];
    M[0][3] = c0.M[0][3] * c1.M[0][3];
}

void Cxform::Normalize_NonOpt()
{
    M[1][0] *= (1.f/255.f);
    M[1][1] *= (1.f/255.f);
    M[1][2] *= (1.f/255.f);
    M[1][3] *= (1.f/255.f);
}

void    Cxform::SetIdentity()
{
    M[0][0] = 1;
    M[0][1] = 1;
    M[0][2] = 1;
    M[0][3] = 1;
    M[1][0] = 0;
    M[1][1] = 0;
    M[1][2] = 0;
    M[1][3] = 0;
}

// Apply our transform to the given color; return the result.
Color  Cxform::Transform(const Color in) const
{
    Color  result(
        (UByte)Alg::Clamp<float>(in.GetRed()   * M[0][0] + 255.0f * M[1][0], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetGreen() * M[0][1] + 255.0f * M[1][1], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetBlue()  * M[0][2] + 255.0f * M[1][2], 0, 255),
        (UByte)Alg::Clamp<float>(in.GetAlpha() * M[0][3] + 255.0f * M[1][3], 0, 255) );

    return result;
}

void Cxform::GetAsFloat2x4( float (*rows)[4] ) const
{
    rows[0][0] = M[0][0];
    rows[0][1] = M[0][1];
    rows[0][2] = M[0][2];
    rows[0][3] = M[0][3];

    rows[1][0] = M[1][0];
    rows[1][1] = M[1][1];
    rows[1][2] = M[1][2];
    rows[1][3] = M[1][3];
}

void Cxform::GetAsFloat2x4Aligned_NonOpt( float (*rows)[4] ) const
{
    rows[0][0] = M[0][0];
    rows[0][1] = M[0][1];
    rows[0][2] = M[0][2];
    rows[0][3] = M[0][3];

    rows[1][0] = M[1][0];
    rows[1][1] = M[1][1];
    rows[1][2] = M[1][2];
    rows[1][3] = M[1][3];
}

bool    Cxform::IsIdentity() const
{
    return (M[0][0] == 1 &&  M[0][1] == 1 && M[0][2] == 1 && M[0][3] == 1 &&
        M[1][0] == 0 &&  M[1][1] == 0 && M[1][2] == 0 && M[1][3] == 0);
}

bool Cxform::operator==( const Cxform& x ) const
{
    return M[0][0] == x.M[0][0] && M[0][1] == x.M[0][1] && M[0][2] == x.M[0][2] && M[0][3] == x.M[0][3] &&
        M[1][0] == x.M[1][0] && M[1][1] == x.M[1][1] && M[1][2] == x.M[1][2] && M[1][3] == x.M[1][3];
}

bool Cxform::operator != (const Cxform& x) const
{
    return !(x == *this);
}

#if (defined(SF_OS_WIIU) || defined(SF_OS_WII)) && !defined(SF_BUILD_DEBUG)

void Cxform::Prepend(const Cxform& c)
{
    asm("psq_l      fp0, 0(r4), 0, 0");     // c0
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");    // c1
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp4, 0(r3), 0, 0");     // m0
    asm("psq_l      fp5, 8(r3), 0, 0");
    asm("psq_l      fp6, 16(r3), 0, 0");    // m1
    asm("psq_l      fp7, 24(r3), 0, 0");

    asm("ps_mul     fp0, fp0, fp4");        // m0result = m0 * c0
    asm("ps_mul     fp1, fp1, fp5");
    asm("ps_madd    fp4, fp4, fp2, fp6");   // m1result = m0 * c1 + m1
    asm("ps_madd    fp5, fp5, fp3, fp7");

    asm("psq_st     fp0, 0(r3), 0, 0");
    asm("psq_st     fp1, 8(r3), 0, 0");
    asm("psq_st     fp4, 16(r3), 0, 0");
    asm("psq_st     fp5, 24(r3), 0, 0");
}

void Cxform::Append(const Cxform& c)
{
    asm("psq_l      fp0, 0(r4), 0, 0");     // c0
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");    // c1
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp4, 0(r3), 0, 0");     // m0
    asm("psq_l      fp5, 8(r3), 0, 0");
    asm("psq_l      fp6, 16(r3), 0, 0");    // m1
    asm("psq_l      fp7, 24(r3), 0, 0");

    asm("ps_mul     fp4, fp4, fp0");        // m0result = m0 * c0
    asm("ps_mul     fp5, fp5, fp1");
    asm("ps_madd    fp0, fp0, fp6, fp2");   // m1result = c0 * m1 + c1
    asm("ps_madd    fp1, fp1, fp7, fp3");

    asm("psq_st     fp4, 0(r3), 0, 0");
    asm("psq_st     fp5, 8(r3), 0, 0");
    asm("psq_st     fp0, 16(r3), 0, 0");
    asm("psq_st     fp1, 24(r3), 0, 0");
}

void Cxform::SetToAppend(const Cxform& c0, const Cxform& c1)
{
    asm("psq_l      fp0, 0(r4), 0, 0");     // c00
    asm("psq_l      fp1, 8(r4), 0, 0");
    asm("psq_l      fp2, 16(r4), 0, 0");    // c01
    asm("psq_l      fp3, 24(r4), 0, 0");

    asm("psq_l      fp4, 0(r5), 0, 0");     // c10
    asm("psq_l      fp5, 8(r5), 0, 0");
    asm("psq_l      fp6, 16(r5), 0, 0");    // c11
    asm("psq_l      fp7, 24(r5), 0, 0");

    asm("ps_mul     fp0, fp0, fp4");        // m0result = c00 * c10
    asm("ps_mul     fp1, fp1, fp5");
    asm("ps_madd    fp4, fp4, fp2, fp6");   // m1result = c10 * c01 + c11
    asm("ps_madd    fp5, fp5, fp3, fp7");

    asm("psq_st     fp0, 0(r3), 0, 0");
    asm("psq_st     fp1, 8(r3), 0, 0");
    asm("psq_st     fp4, 16(r3), 0, 0");
    asm("psq_st     fp5, 24(r3), 0, 0");
}

void Cxform::Normalize_Opt(float* invScale)
{
    asm("psq_l      fp0, 16(r3), 0, 0");    // m1
    asm("psq_l      fp1, 24(r3), 0, 0");
    asm("psq_l      fp2, 0(r4), 0, 0");     // tffinv

    asm("ps_mul     fp0, fp0, fp2");
    asm("ps_mul     fp1, fp1, fp2");

    asm("psq_st     fp0, 16(r3), 0, 0");
    asm("psq_st     fp1, 24(r3), 0, 0");
}

void Cxform::Normalize()
{
    float invScale[] = { 1.0f / 255.0f, 1.0f / 255.0f };
    Normalize_Opt(invScale);
}

void Cxform::GetAsFloat2x4Aligned( float (*rows)[4] ) const
{
    asm("psq_l      fp0, 0(r3), 0, 0");     // m0
    asm("psq_l      fp1, 8(r3), 0, 0");
    asm("psq_l      fp2, 16(r3), 0, 0");    // m1
    asm("psq_l      fp3, 24(r3), 0, 0");

    asm("psq_st     fp0, 0(r4), 0, 0");     // m0
    asm("psq_st     fp1, 8(r4), 0, 0");
    asm("psq_st     fp2, 16(r4), 0, 0");    // m1
    asm("psq_st     fp3, 24(r4), 0, 0");
}

#elif defined(SF_ENABLE_SIMD)

void Cxform::Prepend(const Cxform& c)
{
    using namespace SIMD;
    Vector4f c0 = IS::LoadAligned(c.M[0]);
    Vector4f m0 = IS::LoadAligned(M[0]);
    Vector4f c1 = IS::LoadAligned(c.M[1]);
    Vector4f m1 = IS::LoadAligned(M[1]);

    Vector4f m0result = IS::Multiply(m0, c0);
    Vector4f m1result = IS::MultiplyAdd(m0, c1, m1);

    IS::StoreAligned(M[0], m0result);
    IS::StoreAligned(M[1], m1result);
}

void Cxform::Append(const Cxform& c)
{
    using namespace SIMD;
    Vector4f c0 = IS::LoadAligned(c.M[0]);
    Vector4f m0 = IS::LoadAligned(M[0]);
    Vector4f c1 = IS::LoadAligned(c.M[1]);
    Vector4f m1 = IS::LoadAligned(M[1]);

    Vector4f m0result = IS::Multiply(m0, c0);
    Vector4f m1result = IS::MultiplyAdd(c0, m1, c1);

    IS::StoreAligned(M[0], m0result);
    IS::StoreAligned(M[1], m1result);
}

void Cxform::SetToAppend(const Cxform& c0, const Cxform& c1)
{
    using namespace SIMD;
    Vector4f c00 = IS::LoadAligned(c0.M[0]);
    Vector4f c10 = IS::LoadAligned(c1.M[0]);
    Vector4f c01 = IS::LoadAligned(c0.M[1]);
    Vector4f c11 = IS::LoadAligned(c1.M[1]);

    Vector4f m0result = IS::Multiply(c00, c10);
    Vector4f m1result = IS::MultiplyAdd(c10, c01, c11);

    IS::StoreAligned(M[0], m0result);
    IS::StoreAligned(M[1], m1result);
}

void Cxform::Normalize()
{
    using namespace SIMD;
    static const Vector4f tffinv = { (1.f/255.f), (1.f/255.f), (1.f/255.f), (1.f/255.f) };
    Vector4f m1 = IS::LoadAligned( M[1] );
    m1 = IS::Multiply( m1, tffinv );
    IS::StoreAligned( M[1], m1 );
}

void Cxform::GetAsFloat2x4Aligned( float (*rows)[4] ) const
{
    using namespace SIMD;
    Vector4f m0 = IS::LoadAligned(M[0]);
    Vector4f m1 = IS::LoadAligned(M[1]);
    IS::StoreAligned(rows[0], m0);
    IS::StoreAligned(rows[1], m1);
}

#else

void Cxform::Prepend(const Cxform& c)
{
    Prepend_NonOpt(c);
}

void Cxform::Append(const Cxform& c)
{
    Append_NonOpt(c);
}

void Cxform::SetToAppend(const Cxform& c0, const Cxform& c1)
{
    SetToAppend_NonOpt(c0, c1);
}

void Cxform::Normalize()
{
    Normalize_NonOpt();
}

void Cxform::GetAsFloat2x4Aligned( float (*rows)[4] ) const
{
    GetAsFloat2x4Aligned_NonOpt(rows);
}

#endif // SF_ENABLE_SIMD.

}} // Scaleform::Render
