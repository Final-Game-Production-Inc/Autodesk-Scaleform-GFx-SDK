/**************************************************************************

Filename    :   Render_HitTest.cpp
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../Kernel/SF_Alg.h"
#include "../Kernel/SF_ArrayStaticBuff.h"
#include "Render_Math2D.h"
#include "Render_HitTest.h"

namespace Scaleform { namespace Render { 

namespace Math2D {


// This tolerance defines how to well the cubic curve will be approximated  
// with quadratic ones. The less this value is the better accuracy is achieved, 
// but the more quadratic curves will be generated. Value of 1/200 defines 
// accuracy of about 0.5 pixel
//--------------------------------------------------------------------
const float CubicToQuadTolerance = 1.0f/200.0f;



//--------------------------------------------------------------------
bool CheckMonoQuadIntersection(float x1, float y1, float x2, float y2,
                               float x3, float y3, float x,  float y)
{
    // Check the Y-monotone quadratic curve for the intersection 
    // with a horizontal ray from (x,y) to the left. First check Y.
    //----------------------
    if (y >= y1 && y < y3) // Conditions >= && < are IMPORTANT!
    {
        // Early out. Check if all tree edges (triangle) lie on the left 
        // or on the right. First means "definitely no intersection", 
        // second - "definitely there is an intersection".
        // It's OK to use bitwise expressions rather than logical 
        // as potentially more efficient.
        //-----------------------
        unsigned cp1 = CrossProduct(x1, y1, x2, y2, x, y) > 0;
        unsigned cp2 = CrossProduct(x2, y2, x3, y3, x, y) > 0;
        unsigned cp3 = CrossProduct(x1, y1, x3, y3, x, y) > 0;

        if (cp1 & cp2 & cp3)             // cp1>0  && cp2>0  && cp3>0, on the right
            return true;

        if ((cp1^1) & (cp2^1) & (cp3^1)) // cp1<=0 && cp2<=0 && cp3<=0, on the left
            return false;

        // Intermediate case, the point is inside the triangle. 
        // Calculate the real intersection point between the curve and 
        // the horizontal line at Y. Then check if the intersection point 
        // lies on the left of the given x.
        //-----------------------
        float den = y1 - 2*y2 + y3;
        float t = -1;
        if(den == 0)
        {
            den = y3 - y1;
            if (den != 0)
                t = (y - y1) / den;
        }
        else
        {
            // The initial expression is: 
            // t = (sqrtf(-y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3) + y1 - y2) / den;
            // Theoretically the expression under the sqrtf 
            // (-y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3) must never be negative. 
            // However, it may occur because of finite precision of 
            // float/double. So that, it's a good idea to clamp the value to ]0...oo]
            //------------------------
            t = -y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3;
            t = (t > 0) ? sqrtf(t) : 0;
            t = (t + y1 - y2) / den;
        }
        return CalcPointOnQuadCurve1D(x1, x2, x3, t) < x;
    }
    return false;
}

//--------------------------------------------------------------------
bool CheckQuadIntersection(float x1, float y1, float x2, float y2,
                            float x3, float y3, float x,  float y) 
{
    if(y2 >= y1 && y2 <= y3)
    {
        // A simple (Y-monotone) case
        //-----------------
        return CheckMonoQuadIntersection(x1, y1, x2, y2, x3, y3, x, y);
    }

    // The curve has a Y-extreme. Subdivide it at the 
    // extreme point and process separately.
    //--------------------
    float dy = (2*y2 - y1 - y3);
    float ty = (dy == 0) ? -1 : (y2 - y1) / dy;

    // Subdivide the curves they needs to be normalized 
    // (y1 must be less than y2). To do so it's theoretically
    // enough to check only one of (c1.y1, c1.y3) or (c2.y1, c2.y3),
    // but in practice it's better to check them both for the sake
    // of numerical stability.
    //--------------------
    QuadCoordType c1, c2;
    SubdivideQuadCurve(x1, y1, x2, y2, x3, y3, ty, &c1, &c2);
    if(c1.y1 > c1.y3)
    {
        Alg::Swap(c1.x1, c1.x3);
        Alg::Swap(c1.y1, c1.y3);
    }
    if(c2.y1 > c2.y3)
    {
        Alg::Swap(c2.x1, c2.x3);
        Alg::Swap(c2.y1, c2.y3);
    }

    // The results of these calls must differ. That is, if the hit-test point 
    // lies on the left or on the right of both sub-curves, it definitely
    // means that the curve is not contributing (0 or 2 intersections).
    // Different results mean that the point lies between the sub-curves
    // ("inside" the original curve).
    //-------------------
    return CheckMonoQuadIntersection(c1.x1, c1.y1, c1.x2, c1.y2, c1.x3, c1.y3, x, y) != 
           CheckMonoQuadIntersection(c2.x1, c2.y1, c2.x2, c2.y2, c2.x3, c2.y3, x, y);
}




// The code of Cubic to Quadratic approximation was taken from the 
// Anti-Grain Geometry research works and modified for the use by Scaleform. 
// Permission to use without restrictions is hereby granted to 
// Scaleform Corporation by the author of Anti-Grain Geometry Project.
// See http://antigtain.com for details.
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
template<class Path> 
void SubdivideCubicToQuads(float x1, float y1, float x2, float y2, 
                           float x3, float y3, float x4, float y4, Path& path)
{
    float tolerance = Distance(x1, y1, x2, y2) + 
                      Distance(x2, y2, x3, y3) + 
                      Distance(x3, y3, x4, y4);

    tolerance *= CubicToQuadTolerance;

    float xc;
    float yc;
    if(!Intersection(x1, y1, x2, y2, x3, y3, x4, y4, &xc, &yc, tolerance))
    {
        xc = (x2 + x3) / 2;
        yc = (y2 + y3) / 2;
    }

    float x12   = (x1 + xc ) / 2;
    float y12   = (y1 + yc ) / 2;
    float x23   = (xc + x4 ) / 2;
    float y23   = (yc + y4 ) / 2;
    float xq    = (x12+ x23) / 2;
    float yq    = (y12+ y23) / 2;

    x12   = (x1  + x2  ) / 2;
    y12   = (y1  + y2  ) / 2;
    x23   = (x2  + x3  ) / 2;
    y23   = (y2  + y3  ) / 2;
    float x34   = (x3  + x4  ) / 2;
    float y34   = (y3  + y4  ) / 2;
    float x123  = (x12 + x23 ) / 2;
    float y123  = (y12 + y23 ) / 2;
    float x234  = (x23 + x34 ) / 2;
    float y234  = (y23 + y34 ) / 2;
    float x1234 = (x123+ x234) / 2;
    float y1234 = (y123+ y234) / 2;

    float d;
    d = fabsf(fabsf(LinePointDistance(x1, y1, x4, y4, xq, yq)) - 
              fabsf(LinePointDistance(x1, y1, x4, y4, x1234, y1234))) +
              fabsf(LinePointDistance(x123, y123, x234, y234, xq, yq));

    if(d < tolerance)
    {
        path.QuadTo(xc, yc, x4, y4);
        return;
    }
    SubdivideCubicToQuads(x1, y1, x12, y12, x123, y123, x1234, y1234, path);
    SubdivideCubicToQuads(x1234, y1234, x234, y234, x34, y34, x4, y4, path);
}


//-----------------------------------------------------------------------
struct CubicCurveCoord
{
    float x1, y1, x2, y2, x3, y3, x4, y4;
};

//-----------------------------------------------------------------------
template<class CurveType>
void SubdivideCubic(const CurveType& c, float t, CurveType* c1, CurveType* c2)
{
    // Local variables are necessary in case of reuse curve c,
    // that is, when c1 or c2 points to the same physical c.
    //---------------------
    float x1    = c.x1;
    float y1    = c.y1;
    float x12   = c.x1 + t*(c.x2 - c.x1);
    float y12   = c.y1 + t*(c.y2 - c.y1);
    float x23   = c.x2 + t*(c.x3 - c.x2);
    float y23   = c.y2 + t*(c.y3 - c.y2);
    float x34   = c.x3 + t*(c.x4 - c.x3);
    float y34   = c.y3 + t*(c.y4 - c.y3);
    float x123  = x12  + t*(x23  - x12 );
    float y123  = y12  + t*(y23  - y12 );
    float x234  = x23  + t*(x34  - x23 );
    float y234  = y23  + t*(y34  - y23 );
    float x1234 = x123 + t*(x234 - x123);
    float y1234 = y123 + t*(y234 - y123);
    float x4    = c.x4;
    float y4    = c.y4;
    c1->x1 = x1; 
    c1->y1 = y1;
    c1->x2 = x12; 
    c1->y2 = y12; 
    c1->x3 = x123;
    c1->y3 = y123;
    c1->x4 = x1234;
    c1->y4 = y1234;
    c2->x1 = x1234;
    c2->y1 = y1234; 
    c2->x2 = x234;
    c2->y2 = y234; 
    c2->x3 = x34; 
    c2->y3 = y34;
    c2->x4 = x4;
    c2->y4 = y4;
}

//-----------------------------------------------------------------------
template<class Path> 
void CubicToQuadratic(float x1, float y1, float x2, float y2, 
                      float x3, float y3, float x4, float y4, Path& path)
{
    float ax  =   -x1 + 3*x2 - 3*x3 + x4;
    float ay  =   -y1 + 3*y2 - 3*y3 + y4;
    float bx  =  3*x1 - 6*x2 + 3*x3;
    float by  =  3*y1 - 6*y2 + 3*y3;
    float cx  = -3*x1 + 3*x2;
    float cy  = -3*y1 + 3*y2;
    float den = ay*bx - ax*by;

    float t1 = -1;
    float t2 = -1;

    if (den != 0)
    {
        float tc = -0.5f * (ay*cx - ax*cy) / den;
        float d  = sqrtf(tc*tc - (by*cx - bx*cy) / (3*den));
        t1 = tc - d;
        t2 = tc + d;
    }

    unsigned numSubcurves = 1;
    CubicCurveCoord cc;
    CubicCurveCoord sc[3];
    cc.x1 = x1; cc.y1 = y1;
    cc.x2 = x2; cc.y2 = y2;
    cc.x3 = x3; cc.y3 = y3;
    cc.x4 = x4; cc.y4 = y4;

    switch(int(t2 > 0 && t2 < 1) * 2 + int(t1 > 0 && t1 < 1))
    {
    case 0:
        sc[0] = cc;
        numSubcurves = 1;
        break;

    case 1:
        SubdivideCubic(cc, t1, &sc[0], &sc[1]);
        numSubcurves = 2;
        break;

    case 2:
        SubdivideCubic(cc, t2, &sc[0], &sc[1]);
        numSubcurves = 2;
        break;

    case 3:
        if(t2 < t1) 
        {
            float t = t1; t1 = t2; t2 = t;
        }
        SubdivideCubic(cc, t1, &sc[0], &sc[1]);
        SubdivideCubic(sc[1], (t2 - t1) / (1 - t1), &sc[1], &sc[2]);
        numSubcurves = 3;
        break;
    }

    unsigned i;
    for(i = 0; i < numSubcurves; ++i)
    {
        const CubicCurveCoord& c = sc[i];
        SubdivideCubicToQuads(c.x1, c.y1, c.x2, c.y2, c.x3, c.y3, c.x4, c.y4, path);
    }
}


//-----------------------------------------------------------------------
struct QuadCoord
{
    float cx, cy, ax, ay;
};

class QuadCurvePath
{
    enum { StaticBuff = 32 };

public:
    QuadCurvePath() : Quads(Memory::GetGlobalHeap()) {}

    void QuadTo(float cx, float cy, float ax, float ay)
    {
        QuadCoord c = { cx, cy, ax, ay };
        Quads.PushBack(c);
    }

    unsigned GetQuadCount() const { return (unsigned)Quads.GetSize(); }
    const QuadCoord& GetQuad(unsigned i) const { return Quads[i]; }

private:
    ArrayStaticBuffPOD<QuadCoord, StaticBuff> Quads;
};



//--------------------------------------------------------------------
int CheckQuadraticIntersection(int styleCount, float x1, float y1, float x2, float y2, 
                               float x3, float y3, float x, float y)
{
    if(y1 > y3)
    {
        Alg::Swap(x1, x3);
        Alg::Swap(y1, y3);
    }
    if (CheckQuadIntersection(x1, y1, x2, y2, x3, y3, x, y))
    {
        styleCount ^= 1;
    }
    return styleCount;
}


//--------------------------------------------------------------------
int CheckCubicIntersection(int styleCount, float x1, float y1, float x2, float y2, 
                           float x3, float y3, float x4, float y4, float x, float y)
{
    QuadCurvePath path;
    CubicToQuadratic(x1, y1, x2, y2, x3, y3, x4, y4, path);

    float lastX = x1;
    float lastY = y1;

    for(unsigned i = 0; i < path.GetQuadCount(); ++i)
    {
        const QuadCoord& q = path.GetQuad(i);
        styleCount = CheckQuadraticIntersection(styleCount, lastX, lastY, q.cx, q.cy, q.ax, q.ay, x, y);
        lastX = q.ax;
        lastY = q.ay;
    }
    return styleCount;
}



} // Math2D

//--------------------------------------------------------------------
bool HitTestRoundStroke(const VertexPath& path, float w, float x, float y)
{
    unsigned i, j;
    w /= 2;

    for(i = 0; i < path.GetNumPaths(); ++i)
    {
        const PathBasic& p = path.GetPath(i);
        for(j = 1; j < p.Count; ++j)
        {
            const VertexPath::VertexType& v1 = path.GetVertex(p.Start + j-1);
            const VertexPath::VertexType& v2 = path.GetVertex(p.Start + j);
            float dx = v2.x - v1.x;
            float dy = v2.y - v1.y;
            VertexPath::VertexType p1 = { v1.x - dy, v1.y + dx };
            VertexPath::VertexType p2 = { v2.x - dy, v2.y + dx };
            if(Math2D::CrossProduct(v1.x, v1.y, p1.x, p1.y, x, y) >= 0 &&
               Math2D::CrossProduct(v2.x, v2.y, p2.x, p2.y, x, y) <= 0 &&
               Math2D::LinePointDistanceAbs(v1.x, v1.y, v2.x, v2.y, x, y) <= w)
            {
                return true;
            }
        }
    }

    w *= w;
    for(i = 0; i < path.GetNumVertices(); ++i)
    {
        const VertexPath::VertexType& v1 = path.GetVertex(i);
        if (Math2D::SqDistance(v1.x, v1.y, x, y) <= w)
        {
            return true;
        }
    }
    return false;
}


//--------------------------------------------------------------------
bool HitTestGeneralStroke(const VertexPath& path, float x, float y)
{
    int styleCount = 0;
    unsigned i, j;

    for(i = 0; i < path.GetNumPaths(); ++i)
    {
        const PathBasic& p = path.GetPath(i);
        for(j = 1; j < p.Count; ++j)
        {
            VertexPath::VertexType p1 = path.GetVertex(p.Start + j-1);
            VertexPath::VertexType p2 = path.GetVertex(p.Start + j);
            if(p1.y != p2.y)
            {
                int dir = 1;
                if(p1.y > p2.y)
                {
                    Alg::Swap(p1, p2);
                   dir = -1;
                }
                if(y >= p1.y && y < p2.y &&
                   Math2D::CrossProduct(p1.x, p1.y, p2.x, p2.y, x, y) > 0)
                {
                    styleCount += dir;
                }
            }
        }
    }
    return styleCount != 0;
}



}} // Scaleform::Render

