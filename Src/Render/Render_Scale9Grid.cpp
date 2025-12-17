/**************************************************************************

Filename    :   Render_Scale9Grid.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Math2D.h"
#include "Render_Scale9Grid.h"
#include "Render_Primitive.h"
#include "Render_BoundBox.h"
#include "Render_MeshKey.h"

namespace Scaleform { namespace Render {




//------------------------------------------------------------------------
VertexElement Image9GridVertex::Elements[] =
{
    { 0, {VET_XY32f} },
    { 8, {VET_UV32f} },
    { 0, {VET_None} }
};
VertexFormat Image9GridVertex::Format = { sizeof(Image9GridVertex), Image9GridVertex::Elements };



void Scale9GridData::MakeMeshKey(float* keyData) const
{
    *keyData++ = S9Rect.x1;
    *keyData++ = S9Rect.y1;
    *keyData++ = S9Rect.x2;
    *keyData++ = S9Rect.y2;
    *keyData++ = Bounds.x1;
    *keyData++ = Bounds.y1;
    *keyData++ = Bounds.x2;
    *keyData++ = Bounds.y2;
    MeshKey::CalcMatrixKey(ViewMtx, keyData, 0);
    keyData += 3;
    *keyData++ = ShapeMtx.Tx();
    *keyData++ = ShapeMtx.Ty();
}



//------------------------------------------------------------------------
Scale9GridInfo::Scale9GridInfo(const RectF& s9Rect, 
                               const Matrix2F& s9gMtx, 
                               const Matrix2F& shapeMtx,
                               const RectF& bounds)
{
    Scale9 = s9Rect;
    S9gMatrix  = s9gMtx;
    ShapeMatrix = shapeMtx;
    Bounds = bounds;

    InverseMatrix = S9gMatrix.GetInverse();
    InverseMatrix.Append(ShapeMatrix.GetInverse());

    Compute();
}

//------------------------------------------------------------------------
Scale9GridInfo::Scale9GridInfo(const Scale9GridData* s9g, const Matrix2F& viewMtx)
{
    Scale9       = s9g->S9Rect;
    S9gMatrix    = s9g->Scale9Mtx;
    ShapeMatrix  = s9g->ShapeMtx;
    Bounds       = s9g->Bounds;
    InverseMatrix = S9gMatrix.GetInverse();
    InverseMatrix.Append(ShapeMatrix.GetInverse());
    InverseMatrix.Append(viewMtx);
    Compute();
}

void Scale9GridInfo::Compute()
{
    // Scaling Grid
    float gx1 = Scale9.x1;
    float gy1 = Scale9.y1;
    float gx2 = Scale9.x2;
    float gy2 = Scale9.y2;

    // Sprite Bounds
    float bx1 = Bounds.x1;
    float by1 = Bounds.y1;
    float bx2 = Bounds.x2;
    float by2 = Bounds.y2;

    // Prevent from having degenerate matrices. 
    // The bounding box must be bigger than the scaling grid
    // box. A value of 1/200 of a pixel (one TWIP/10) is quite enough for 
    // correct calculating of the matrices.
    if (bx1 >= gx1) bx1 = gx1-0.9f;
    if (by1 >= gy1) by1 = gy1-0.9f;
    if (bx2 <= gx2) bx2 = gx1+0.9f;
    if (by2 <= gy2) by2 = gy1+0.9f;

    // Transformed Bounds
    float tx1 = bx1;
    float ty1 = by1;
    float tx2 = bx2;
    float ty2 = by1;
    float tx3 = bx2;
    float ty3 = by2;
    float tx4 = bx1;
    float ty4 = by2;

    // Transform bounds (Y axis goes up)
    //         4                           3 
    //         *----+-----------------+----*
    //        /    /                 /    /
    //       +----+-----------------+----+ 
    //      /    /                 /    /
    //     /    /                 /    /
    //    /    /                 /    /
    //   +----+-----------------+----+ 
    //  /    /                 /    /
    // *----+-----------------+----*
    // 1                           2
    //------------------------------------------
    S9gMatrix.Transform(&tx1, &ty1);
    S9gMatrix.Transform(&tx2, &ty2);
    S9gMatrix.Transform(&tx3, &ty3);
    S9gMatrix.Transform(&tx4, &ty4);

    // Compute pseudo width and height of the bounds
    float w = Math2D::Distance(tx1, ty1, tx2, ty2);
    float h = Math2D::Distance(tx2, ty2, tx3, ty3);

    if (w == 0) w = 0.001f;
    if (h == 0) h = 0.001f;

    // Coefficients for the margins
    float kx1 = (gx1 - bx1) / w;
    float ky1 = (gy1 - by1) / h;
    float kx2 = (bx2 - gx2) / w;
    float ky2 = (by2 - gy2) / h;

    // Prevent from "overlapping"
    float d = kx1 + kx2;
    if (d > 1)
    {
        d += 0.05f;
        kx1 /= d;
        kx2 /= d;
    }

    d = ky1 + ky2;
    if (d > 1)
    {
        d += 0.05f;
        ky1 /= d;
        ky2 /= d;
    }

    // Compute the outer points (Y axis goes up)
    //              6                 5
    //         +----*-----------------*----+
    //        /    /                 /    /
    //     7 *----+-----------------+----* 4
    //      /    /                 /    /
    //     /    /                 /    /
    //    /    /                 /    /
    // 8 *----+-----------------+----* 3
    //  /    /                 /    /
    // +----*-----------------*----+
    //      1                 2
    //------------------------------------------
    float ox1 = tx1 + (tx2-tx1) * kx1;
    float oy1 = ty1 + (ty2-ty1) * kx1;
    float ox2 = tx2 - (tx2-tx1) * kx2;
    float oy2 = ty2 - (ty2-ty1) * kx2;
    float ox3 = tx2 + (tx3-tx2) * ky1;
    float oy3 = ty2 + (ty3-ty2) * ky1;
    float ox4 = tx3 - (tx3-tx2) * ky2;
    float oy4 = ty3 - (ty3-ty2) * ky2;
    float ox5 = tx3 - (tx3-tx4) * kx2;
    float oy5 = ty3 - (ty3-ty4) * kx2;
    float ox6 = tx4 + (tx3-tx4) * kx1;
    float oy6 = ty4 + (ty3-ty4) * kx1;
    float ox7 = tx4 - (tx4-tx1) * ky2;
    float oy7 = ty4 - (ty4-ty1) * ky2;
    float ox8 = tx1 + (tx4-tx1) * ky1;
    float oy8 = ty1 + (ty4-ty1) * ky1;

    // Compute the inner points (Y axis goes up)
    // 
    //         +----+-----------------+----+
    //        /    /                 /    /
    //       +----*-----------------*----+
    //      /    / 4             3 /    /
    //     /    /                 /    /
    //    /    / 1             2 /    /
    //   +----*-----------------*----+
    //  /    /                 /    /
    // +----+-----------------+----+
    // 
    //------------------------------------------
    float ix1 = ox8 + (tx2-tx1) * kx1;
    float iy1 = oy8 + (ty2-ty1) * kx1;
    float ix2 = ox3 - (tx2-tx1) * kx2;
    float iy2 = oy3 - (ty2-ty1) * kx2;
    float ix3 = ox4 - (tx3-tx4) * kx2;
    float iy3 = oy4 - (ty3-ty4) * kx2;
    float ix4 = ox7 + (tx3-tx4) * kx1;
    float iy4 = oy7 + (ty3-ty4) * kx1;

// DBG
//GFx_DrawQuad(tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4, 0x7FFF0000, context.GetRenderer());
//GFx_DrawQuad(ox1, oy1, ox2, oy2, ox5, oy5, ox6, oy6, 0x7F00FF00, context.GetRenderer());
//GFx_DrawQuad(ox8, oy8, ox3, oy3, ox4, oy4, ox7, oy7, 0x7F0000FF, context.GetRenderer());
//GFx_DrawQuad(ix1, iy1, ix2, iy2, ix3, iy3, ix4, iy4, 0x7F000000, context.GetRenderer());


    // Area Matrices (Y-axis goes up)
    //        |        |
    //   m6   |   m7   |  m8
    //        |        |
    // -------+--------+--------
    //        |        |
    //   m3   |   m4   |  m5
    //        |        |
    // -------+--------+--------
    //        |        |
    //   m0   |   m1   |  m2
    //        |        |
    //------------------------------
    float pr[6];
    #define GFX_PARL(a,b,c,d,e,f) { pr[0]=a; pr[1]=b; pr[2]=c; pr[3]=d; pr[4]=e; pr[5]=f; }

    GFX_PARL(tx1, ty1, ox1, oy1, ix1, iy1); ResultingMatrices[0].SetRectToParl(bx1, by1, gx1, gy1, pr);
    GFX_PARL(ox1, oy1, ox2, oy2, ix2, iy2); ResultingMatrices[1].SetRectToParl(gx1, by1, gx2, gy1, pr);
    GFX_PARL(ox2, oy2, tx2, ty2, ox3, oy3); ResultingMatrices[2].SetRectToParl(gx2, by1, bx2, gy1, pr);
    GFX_PARL(ox8, oy8, ix1, iy1, ix4, iy4); ResultingMatrices[3].SetRectToParl(bx1, gy1, gx1, gy2, pr);
    GFX_PARL(ix1, iy1, ix2, iy2, ix3, iy3); ResultingMatrices[4].SetRectToParl(gx1, gy1, gx2, gy2, pr);
    GFX_PARL(ix2, iy2, ox3, oy3, ox4, oy4); ResultingMatrices[5].SetRectToParl(gx2, gy1, bx2, gy2, pr);
    GFX_PARL(ox7, oy7, ix4, iy4, ox6, oy6); ResultingMatrices[6].SetRectToParl(bx1, gy2, gx1, by2, pr);
    GFX_PARL(ix4, iy4, ix3, iy3, ox5, oy5); ResultingMatrices[7].SetRectToParl(gx1, gy2, gx2, by2, pr);
    GFX_PARL(ix3, iy3, ox4, oy4, tx3, ty3); ResultingMatrices[8].SetRectToParl(gx2, gy2, bx2, by2, pr);
    #undef GFX_PARL

    ResultingGrid.x1 = gx1;
    ResultingGrid.y1 = gy1;
    ResultingGrid.x2 = gx2;
    ResultingGrid.y2 = gy2;
}

//------------------------------------------------------------------------
unsigned Scale9GridInfo::Transform(float* x, float* y) const
{
    // Area Codes (Y-axis goes up)
    //        |        |
    // 0110   |  0010  | 0011
    // 6->m6  |  2->m7 | 3->m8
    // -------+--------+-------- y2
    //        |        |
    // 0100   |  0000  | 0001
    // 4->m3  |  0->m4 | 1->m5
    // -------+--------+-------- y1
    //        |        |
    // 1100   |  1000  | 1001
    // 12->m0 |  8->m1 | 9->m2
    //        x1       x2
    //------------------------------
    enum {X=0};
    static const UInt8 codeToMtx[] = { 4, 5, 7, 8, 3, X, 6, X, 1, 2, X, X, 0 };
    //                                 0  1  2  3  4  5  6  7  8  9  10 11 12

    ShapeMatrix.Transform(x, y);
    unsigned areaCode = (*x > ResultingGrid.x2) |       // See "Area Codes"
                       ((*y > ResultingGrid.y2) << 1) | 
                       ((*x < ResultingGrid.x1) << 2) | 
                       ((*y < ResultingGrid.y1) << 3);
    ResultingMatrices[codeToMtx[areaCode]].Transform(x, y);
    InverseMatrix.Transform(x, y);
    return areaCode;
}

//------------------------------------------------------------------------
float Scale9GridInfo::GetScale() const
{
    float scale = 0;
    for(int i = 0; i < 9; ++i)
    {
        scale += ResultingMatrices[i].GetScale();
    }
    return scale / 9.0f;
}

float Scale9GridInfo::GetXScale() const
{
    float scale = 0;
    for(int i = 0; i < 3; ++i)
    {
        scale += ResultingMatrices[i].GetXScale();
    }
    return scale / 3.0f;
}

float Scale9GridInfo::GetYScale() const
{
    float scale = 0;
    for(int i = 0; i < 9; i += 3)
    {
        scale += ResultingMatrices[i].GetYScale();
    }
    return scale / 3.0f;
}

//------------------------------------------------------------------------
unsigned Scale9GridInfo::GetAreaCode(float x, float y) const
{
    return  (x > ResultingGrid.x2) |       // See "Area Codes" above
           ((y > ResultingGrid.y2) << 1) | 
           ((x < ResultingGrid.x1) << 2) | 
           ((y < ResultingGrid.y1) << 3);
}


//------------------------------------------------------------------------
RectF Scale9GridInfo::AdjustBounds(const RectF& bounds) const
{
    float x1 = bounds.x1;
    float y1 = bounds.y1;
    float x2 = bounds.x2;
    float y2 = bounds.y1;
    float x3 = bounds.x2;
    float y3 = bounds.y2;
    float x4 = bounds.x1;
    float y4 = bounds.y2;
    Transform(&x1, &y1);
    Transform(&x2, &y2);
    Transform(&x3, &y3);
    Transform(&x4, &y4);

    float xb1=x1, yb1=y1, xb2=x1, yb2=y1;
    if (x2 < xb1) xb1 = x2; if (y2 < yb1) yb1 = y2;
    if (x2 > xb2) xb2 = x2; if (y2 > yb2) yb2 = y2;
    if (x3 < xb1) xb1 = x3; if (y3 < yb1) yb1 = y3;
    if (x3 > xb2) xb2 = x3; if (y3 > yb2) yb2 = y3;
    if (x4 < xb1) xb1 = x4; if (y4 < yb1) yb1 = y4;
    if (x4 > xb2) xb2 = x4; if (y4 > yb2) yb2 = y4;

    return RectF(xb1, yb1, xb2, yb2);
}



//-----------------------------------------------------------------------
inline bool SegmentLineIntersection(float ax, float ay, float bx, float by,
                                    float cx, float cy, float dx, float dy,
                                    float* x, float* y, float epsilon)
{
    float na  = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
  //float nb  = (bx-ax) * (ay-cy) - (by-ay) * (ax-cx);
    float den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);

    if(fabsf(den) < epsilon) 
        return false;

    float ua = na / den;
  //float ub = nb / den;

    if(ua < -0.00001f || ua > 1.00001f)
        return false;

    *x = ax + ua * (bx-ax);
    *y = ay + ua * (by-ay);
    return true;
}



//------------------------------------------------------------------------
void Scale9GridTess::addVertex(TmpVertices& ver, float x, float y, float u, float v, unsigned areaCode)
{
    TmpVertexType tmpVer = { areaCode, VerCount, 0 };
    ver.PushBack(tmpVer);
    Vertices[VerCount].Set(x, y, u, v);
    VerCount++;
}

//------------------------------------------------------------------------
void Scale9GridTess::addVertices(TmpVertices& ver, const Matrix2F& toUV, float x, float y, unsigned code1, unsigned code2)
{
    float u = x;
    float v = y;
    toUV.Transform(&u, &v);
    TmpVertexType tmpVer1 = { code1, VerCount, 0 };
    TmpVertexType tmpVer2 = { code2, VerCount, 0 };
    ver.PushBack(tmpVer1);
    ver.PushBack(tmpVer2);
    Vertices[VerCount].Set(x, y, u, v);
    VerCount++;
}


//------------------------------------------------------------------------
unsigned Scale9GridTess::getAreaCode(const RectF& r, float x, float y) const
{
    return  (x > r.x2) |       // See "Area Codes" above
           ((y > r.y2) << 1) | 
           ((x < r.x1) << 2) | 
           ((y < r.y1) << 3);
}


//------------------------------------------------------------------------
void Scale9GridTess::addCorner(TmpVertices& ver, const float* c, float x, float y, 
                               const Matrix2F& toUV, 
                               unsigned code1, unsigned code2, unsigned code3, unsigned code4)
{
    float c1 = Math2D::CrossProduct(c[0], c[1], c[2], c[3], x, y);
    float c2 = Math2D::CrossProduct(c[2], c[3], c[4], c[5], x, y);
    float c3 = Math2D::CrossProduct(c[4], c[5], c[6], c[7], x, y);
    float c4 = Math2D::CrossProduct(c[6], c[7], c[0], c[1], x, y);

    bool cp1 = c1 <= 0;
    bool cp2 = c2 <= 0;
    bool cp3 = c3 <= 0;
    bool cp4 = c4 <= 0;

    bool cp5 = c1 >= 0;
    bool cp6 = c2 >= 0;
    bool cp7 = c3 >= 0;
    bool cp8 = c4 >= 0;

    if ((cp1 == cp2 && cp2 == cp3 && cp3 == cp4) || 
        (cp5 == cp6 && cp6 == cp7 && cp7 == cp8))
    {
        // The corner is inside the image. Add vertices
        float u = x;
        float v = y;
        toUV.Transform(&u, &v);
        TmpVertexType tmpVer1 = { code1, VerCount, 0 };
        TmpVertexType tmpVer2 = { code2, VerCount, 0 };
        TmpVertexType tmpVer3 = { code3, VerCount, 0 };
        TmpVertexType tmpVer4 = { code4, VerCount, 0 };
        ver.PushBack(tmpVer1);
        ver.PushBack(tmpVer2);
        ver.PushBack(tmpVer3);
        ver.PushBack(tmpVer4);
        Vertices[VerCount].Set(x, y, u, v);
        VerCount++;
    }
}



//------------------------------------------------------------------------
void Scale9GridTess::transformVertex(const Scale9GridInfo& s9g, Image9GridVertex& v) const
{
    // Area Codes (Y-axis goes up)
    //        |        |
    // 0110   |  0010  | 0011
    // 6->m6  |  2->m7 | 3->m8
    // -------+--------+-------- y2
    //        |        |
    // 0100   |  0000  | 0001
    // 4->m3  |  0->m4 | 1->m5
    // -------+--------+-------- y1
    //        |        |
    // 1100   |  1000  | 1001
    // 12->m0 |  8->m1 | 9->m2
    //        x1       x2
    //------------------------------
    enum {X=0};
    static const UInt8 codeToMtx[] = { 4, 5, 7, 8, 3, X, 6, X, 1, 2, X, X, 0 };
    //                                 0  1  2  3  4  5  6  7  8  9  10 11 12

    unsigned areaCode = (v.x > s9g.ResultingGrid.x2) |       // See "Area Codes"
                       ((v.y > s9g.ResultingGrid.y2) << 1) | 
                       ((v.x < s9g.ResultingGrid.x1) << 2) | 
                       ((v.y < s9g.ResultingGrid.y1) << 3);
    s9g.ResultingMatrices[codeToMtx[areaCode]].Transform(&v.x, &v.y);
    s9g.InverseMatrix.Transform(&v.x, &v.y);
}


//------------------------------------------------------------------------
void Scale9GridTess::tessellateArea(TmpVertices& ver, unsigned i1, unsigned i2)
{
    if (i1+3 <= i2)
    {
        unsigned i;
        float cx = 0;
        float cy = 0;

        // Compute the center
        for (i = i1; i < i2; ++i)
        {
            TmpVertexType& v = ver[i];
            cx += Vertices[v.VerIdx].x;
            cy += Vertices[v.VerIdx].y;
        }
        cx /= (i2-i1);
        cy /= (i2-i1);

        // Compute the slope ratios
        for (i = i1; i < i2; ++i)
        {
            TmpVertexType& v = ver[i];
            v.Slope = Math2D::SlopeRatio(cx, cy, Vertices[v.VerIdx].x, Vertices[v.VerIdx].y);
        }

        // Sort the vertices in the counter-clockwise direction
        Alg::QuickSortSliced(ver, i1, i2, cmpSlopes);

        // Add indices so they form a triangle fan
        for(i = i1+2; i < i2; ++i)
        {
            Indices.PushBack((UInt16)ver[i1].VerIdx);
            Indices.PushBack((UInt16)ver[i-1].VerIdx);
            Indices.PushBack((UInt16)ver[i].VerIdx);
        }
    }
}


//------------------------------------------------------------------------
Scale9GridTess::Scale9GridTess(MemoryHeap* heap, const Scale9GridInfo& s9g, const RectF& imgRect, 
                               const Matrix2F& uvMatrix, const Matrix2F& fillMatrix) :
    VerCount(0), Indices(heap)
{
    float c[8] = { imgRect.x1, imgRect.y1, 
                   imgRect.x2, imgRect.y1, 
                   imgRect.x2, imgRect.y2, 
                   imgRect.x1, imgRect.y2 };

    s9g.ShapeMatrix.Transform(&c[0], &c[1]);
    s9g.ShapeMatrix.Transform(&c[2], &c[3]);
    s9g.ShapeMatrix.Transform(&c[4], &c[5]);
    s9g.ShapeMatrix.Transform(&c[6], &c[7]);

    Matrix2F uv = uvMatrix;

    // Limited support for fill matrix transformations in Scale9Grid. In particular, 
    // it allows Flash artists to use only 3 small images for the window corners and borders 
    // instead of one big image for the entire window. By flipping and rotating the images
    // one can design a window. 
    // "Limited" means that only horizontal/vertical flipping and 90, 180, and 270 degree 
    // rotation is supported. In case this feature is not desirable, just comment out:
    //----------------------------- from here
    Matrix2F fm1 = fillMatrix;
    Matrix2F fm2;

    float em = 0.00001f;
    if (fm1.Sx()  <= -em) fm1.Sx()  = -1;
    if (fm1.Sx()  >= +em) fm1.Sx()  = +1;
    if (fm1.Sy()  <= -em) fm1.Sy()  = -1;
    if (fm1.Sy()  >= +em) fm1.Sy()  = +1;
    if (fm1.Shx() <= -em) fm1.Shx() = -1;
    if (fm1.Shx() >= +em) fm1.Shx() = +1;
    if (fm1.Shy() <= -em) fm1.Shy() = -1;
    if (fm1.Shy() >= +em) fm1.Shy() = +1;
    fm1.Tx() = 0;
    fm1.Ty() = 0;
    fm2.AppendTranslation(-0.5f, -0.5f);
    fm2.Append(fm1);
    fm2.AppendTranslation(0.5f, 0.5f);
    uv.Prepend(fm2);
    //----------------------------- to here

    Matrix2F toUV;
    toUV.SetParlToRect(c, 0, 0, 1, 1);
    toUV.Append(uv);

    RectF r = s9g.ResultingGrid;
    TmpVertices tmpVer(heap);

    // Add 4 vertices of the transformed image.
    //--------------------------------
    float x1 = 0;
    float y1 = 0;
    float x2 = 1;
    float y2 = 0;
    float x3 = 1;
    float y3 = 1;
    float x4 = 0;
    float y4 = 1;
    uv.Transform(&x1, &y1);
    uv.Transform(&x2, &y2);
    uv.Transform(&x3, &y3);
    uv.Transform(&x4, &y4);

    addVertex(tmpVer, c[0], c[1], x1, y1, getAreaCode(r, c[0], c[1]));
    addVertex(tmpVer, c[2], c[3], x2, y2, getAreaCode(r, c[2], c[3]));
    addVertex(tmpVer, c[4], c[5], x3, y3, getAreaCode(r, c[4], c[5]));
    addVertex(tmpVer, c[6], c[7], x4, y4, getAreaCode(r, c[6], c[7]));

    float x, y;
    float eps = 0.001f;
    float ex = (r.x2 - r.x1) / 2;
    float ey = (r.y2 - r.y1) / 2;

    // Add intersections with the grid lines
    // 00 -> 10
    //--------------------------------
    if (SegmentLineIntersection(c[0], c[1], c[2], c[3], r.x1, r.y1, r.x2, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[0], c[1], c[2], c[3], r.x2, r.y1, r.x2, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    if (SegmentLineIntersection(c[0], c[1], c[2], c[3], r.x2, r.y2, r.x1, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[0], c[1], c[2], c[3], r.x1, r.y2, r.x1, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    // 10 -> 11
    //--------------------------------
    if (SegmentLineIntersection(c[2], c[3], c[4], c[5], r.x1, r.y1, r.x2, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[2], c[3], c[4], c[5], r.x2, r.y1, r.x2, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    if (SegmentLineIntersection(c[2], c[3], c[4], c[5], r.x2, r.y2, r.x1, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[2], c[3], c[4], c[5], r.x1, r.y2, r.x1, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    // 11 -> 01
    //--------------------------------
    if (SegmentLineIntersection(c[4], c[5], c[6], c[7], r.x1, r.y1, r.x2, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[4], c[5], c[6], c[7], r.x2, r.y1, r.x2, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    if (SegmentLineIntersection(c[4], c[5], c[6], c[7], r.x2, r.y2, r.x1, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[4], c[5], c[6], c[7], r.x1, r.y2, r.x1, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    // 01 -> 00
    //--------------------------------
    if (SegmentLineIntersection(c[6], c[7], c[0], c[1], r.x1, r.y1, r.x2, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[6], c[7], c[0], c[1], r.x2, r.y1, r.x2, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    if (SegmentLineIntersection(c[6], c[7], c[0], c[1], r.x2, r.y2, r.x1, r.y2, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x, y+ey), getAreaCode(r, x, y-ey));

    if (SegmentLineIntersection(c[6], c[7], c[0], c[1], r.x1, r.y2, r.x1, r.y1, &x, &y, eps))
        addVertices(tmpVer, toUV, x, y, getAreaCode(r, x+ex, y), getAreaCode(r, x-ex, y));

    // Add grid corners if they are inside the image
    //--------------------------------
    addCorner(tmpVer, c, r.x1, r.y1, toUV, 0, 4, 12,8);
    addCorner(tmpVer, c, r.x2, r.y1, toUV, 1, 0, 8, 9);
    addCorner(tmpVer, c, r.x2, r.y2, toUV, 3, 2, 0, 1);
    addCorner(tmpVer, c, r.x1, r.y2, toUV, 2, 6, 4, 0);

    Alg::QuickSort(tmpVer, cmpCodes);

    unsigned i1 = 0;
    unsigned i2 = 0;
    for (i2 = 1; i2 < tmpVer.GetSize(); ++i2)
    {
        if (tmpVer[i2].AreaCode != tmpVer[i1].AreaCode)
        {
            tessellateArea(tmpVer, i1, i2);
            i1 = i2;
        }
    }
    tessellateArea(tmpVer, i1, i2);

    for (i1 = 0; i1 < VerCount; ++i1)
    {
        transformVertex(s9g, Vertices[i1]);
    }

}



}} // Scaleform::Render
