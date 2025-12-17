/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TessCurves.cpp
Content     :   Quadratic and Cubic cuves tessellation
Created     :
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TessCurves.h"

namespace Scaleform { namespace Render {

const CoordType AngleTolerance = 0.25f;
const CoordType CurveCollinearityEpsilon = 1e-10f;


//--------------------------------------------------------------------
bool TestQuadCollinearity(TessBase* con, const ToleranceParams& param, 
                          CoordType x1, CoordType y1,
                          CoordType x2, CoordType y2,
                          CoordType x3, CoordType y3)
{
    // Checking the curve for collinearity. 
    // We need to calculate the distance between p2 and the line p1-p3
    // (the length of the perpendicular). Essentially it is: 
    // d = cross_product(p1, p3, p2) / length(p1, p3);
    // To calculate the length we have to calculate a square root, 
    // but it is possible to avoid it, since we can compare the squares
    // of values instead of values themselves. 
    // 
    // If the three points are collinear we can do without any subdivision.
    // The subdivision function correctly processes the simple collinear
    // case p1-p2-p3, but fails in case when the second line segment
    // makes a U-turn, that is, p1-p3-p2 or p3-p1-p2 (where p2 is the curve 
    // control point). In the simple collinear case the result consists of 
    // only two points, but in the U-turn case we have to calculate the 
    // third point.
    //-----------------
    CoordType da = Math2D::SqDistance(x1, y1, x3, y3);
    CoordType d  = Math2D::AbsCrossProduct(x1, y1, x3, y3, x2, y2);
    CoordType collinearityTolerance = param.CollinearityTolerance / 4;
    if(d * d <= collinearityTolerance * collinearityTolerance * da)
    {
        // The expression below verifies if point p2 lies between p1 and p3.
        // If it does (0 <= d <= 1) we can omit the third point. Otherwise
        // it's necessary to calculate it.
        //---------------------
        if(da == 0) d = 2;
        else        d = ((x2 - x1)*(x3 - x1) + (y2 - y1)*(y3 - y1)) / da;
        if(d < 0 || d > 1)
        {
            // Calculate the ratio (parameter t) to subdivide the curve. 
            // This ratio corresponds to the point on the curve with the maximal 
            // curvature. The formula is the proportion between d1 and d2, 
            // that is, d1 / (d1 + d2), where d1 and d2 are the Euclidean 
            // distances p1-p2 and p2-p3 respectively.
            CoordType d1 = Math2D::Distance(x1, y1, x2, y2);
            CoordType d2 = Math2D::Distance(x2, y2, x3, y3) + d1;
            if(d2 != 0)
            {
                CoordType t    = d1 / d2;
                CoordType x12  = x1 + t*(x2 - x1);
                CoordType y12  = y1 + t*(y2 - y1);
                CoordType x23  = x2 + t*(x3 - x2);
                CoordType y23  = y2 + t*(y3 - y2);
                con->AddVertex(x12 + t*(x23 - x12), y12 + t*(y23 - y12));
            }
        }
        con->AddVertex(x3, y3);
        return true;
    }
    return false;
}



//--------------------------------------------------------------------
void TessellateQuadRecursively(TessBase* con, const ToleranceParams& param, 
                               CoordType toleranceSq, 
                               CoordType x1, CoordType y1,
                               CoordType x2, CoordType y2,
                               CoordType x3, CoordType y3,
                               int level)
{
    // Check the conditions to stop curve subdivision. 
    // We need to calculate the distance between p2 and the line p1-p3
    // (the length of the perpendicular). Essentially it is: 
    // d = cross_product(p1, p3, p2) / length(p1, p3);
    // To calculate the length we have to calculate a square root, 
    // but it is possible to avoid it, since we can compare the squares
    // of values instead of values themselves. 
    //-----------------
    CoordType d = Math2D::AbsCrossProduct(x1, y1, x3, y3, x2, y2);
    if(d == 0 ||
       d * d <= toleranceSq * Math2D::SqDistance(x1, y1, x3, y3) ||
       level >= (int)param.CurveRecursionLimit)
    {
        // The curve is flat enough, so that we add the point 
        // and stop subdivision.
        //---------------------
        con->AddVertex(x3, y3);
        return;
    }
    // Subdivide the curve at t=0.5.
    //-----------------
    CoordType x12  = (x1  + x2)  / 2;
    CoordType y12  = (y1  + y2)  / 2;
    CoordType x23  = (x2  + x3)  / 2;
    CoordType y23  = (y2  + y3)  / 2;
    CoordType x123 = (x12 + x23) / 2;
    CoordType y123 = (y12 + y23) / 2;
    TessellateQuadRecursively(con, param, toleranceSq, x1, y1, x12, y12, x123, y123, level+1);
    TessellateQuadRecursively(con, param, toleranceSq, x123, y123, x23, y23, x3, y3, level+1);
}




//--------------------------------------------------------------------
void TessellateQuadCurve(TessBase* con, const ToleranceParams& param, 
                         CoordType x2, CoordType y2,
                         CoordType x3, CoordType y3)
{
    CoordType x1 = con->GetLastX();
    CoordType y1 = con->GetLastY(); 
    if(!TestQuadCollinearity(con, param, x1, y1, x2, y2, x3, y3))
    {
        CoordType toleranceSq = (param.CurveTolerance/4) * (param.CurveTolerance/4);
        TessellateQuadRecursively(con, param, toleranceSq, x1, y1, x2, y2, x3, y3, 0);
    }
}



//----------------------------------------------------------------------
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.



void TessellateCubicRecursively(TessBase* con, const ToleranceParams& param,
                                CoordType toleranceSq, 
                                CoordType x1, CoordType y1,
                                CoordType x2, CoordType y2,
                                CoordType x3, CoordType y3,
                                CoordType x4, CoordType y4,
                                int level)
{
    if(level > (int)param.CurveRecursionLimit) 
    {
        return;
    }

    // Calculate all the mid-points of the line segments
    //----------------------
    CoordType x12   = (x1 + x2) / 2;
    CoordType y12   = (y1 + y2) / 2;
    CoordType x23   = (x2 + x3) / 2;
    CoordType y23   = (y2 + y3) / 2;
    CoordType x34   = (x3 + x4) / 2;
    CoordType y34   = (y3 + y4) / 2;
    CoordType x123  = (x12 + x23) / 2;
    CoordType y123  = (y12 + y23) / 2;
    CoordType x234  = (x23 + x34) / 2;
    CoordType y234  = (y23 + y34) / 2;
    CoordType x1234 = (x123 + x234) / 2;
    CoordType y1234 = (y123 + y234) / 2;


    // Try to approximate the full cubic curve by a single straight line
    //------------------
    CoordType dx = x4-x1;
    CoordType dy = y4-y1;

    CoordType d2 = fabsf(((x2 - x4) * dy - (y2 - y4) * dx));
    CoordType d3 = fabsf(((x3 - x4) * dy - (y3 - y4) * dx));
    CoordType da1, da2, k;

    switch((int(d2 > CurveCollinearityEpsilon) << 1) +
            int(d3 > CurveCollinearityEpsilon))
    {
    case 0:
        // All collinear OR p1==p4
        //----------------------
        k = dx*dx + dy*dy;
        if(k == 0)
        {
            d2 = Math2D::SqDistance(x1, y1, x2, y2);
            d3 = Math2D::SqDistance(x4, y4, x3, y3);
        }
        else
        {
            k   = 1 / k;
            da1 = x2 - x1;
            da2 = y2 - y1;
            d2  = k * (da1*dx + da2*dy);
            da1 = x3 - x1;
            da2 = y3 - y1;
            d3  = k * (da1*dx + da2*dy);
            if(d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
            {
                // Simple collinear case, 1---2---3---4
                con->AddVertex(x4, y4);
                return;
            }
                 if(d2 <= 0) d2 = Math2D::SqDistance(x2, y2, x1, y1);
            else if(d2 >= 1) d2 = Math2D::SqDistance(x2, y2, x4, y4);
            else             d2 = Math2D::SqDistance(x2, y2, x1 + d2*dx, y1 + d2*dy);

                 if(d3 <= 0) d3 = Math2D::SqDistance(x3, y3, x1, y1);
            else if(d3 >= 1) d3 = Math2D::SqDistance(x3, y3, x4, y4);
            else             d3 = Math2D::SqDistance(x3, y3, x1 + d3*dx, y1 + d3*dy);
        }

        if(d2 < toleranceSq || d3 < toleranceSq)
        {
            con->AddVertex(x4, y4);
            return;
        }

        break;

    case 1:
        // p1,p2,p4 are collinear, p3 is significant
        //----------------------
        if(d3 * d3 <= toleranceSq * (dx*dx + dy*dy))
        {
            // Angle Condition
            //----------------------
            da1 = fabsf(atan2f(y4 - y3, x4 - x3) - atan2f(y3 - y2, x3 - x2));
            if(da1 >= Math2D::Pi) da1 = 2*Math2D::Pi - da1;

            if(da1 < AngleTolerance)
            {
                con->AddVertex(x4, y4);
                return;
            }
        }
        break;

    case 2:
        // p1,p3,p4 are collinear, p2 is significant
        //----------------------
        if(d2 * d2 <= toleranceSq * (dx*dx + dy*dy))
        {
            // Angle Condition
            //----------------------
            da1 = fabsf(atan2f(y3 - y2, x3 - x2) - atan2f(y2 - y1, x2 - x1));
            if(da1 >= Math2D::Pi) da1 = 2*Math2D::Pi - da1;

            if(da1 < AngleTolerance)
            {
                con->AddVertex(x4, y4);
                return;
            }
        }
        break;

    case 3: 
        // Regular case
        //-----------------
        if((d2 + d3)*(d2 + d3) <= toleranceSq * (dx*dx + dy*dy))
        {
            // Angle Condition
            //----------------------
            k   = atan2f(y3 - y2, x3 - x2);
            da1 = fabsf(k - atan2f(y2 - y1, x2 - x1));
            da2 = fabsf(atan2f(y4 - y3, x4 - x3) - k);
            if(da1 >= Math2D::Pi) da1 = 2*Math2D::Pi - da1;
            if(da2 >= Math2D::Pi) da2 = 2*Math2D::Pi - da2;

            if(da1 + da2 < AngleTolerance)
            {
                // Finally we can stop the recursion
                //----------------------
                con->AddVertex(x4, y4);
                return;
            }
        }
        break;
    }

    // Continue subdivision
    //----------------------
    TessellateCubicRecursively(con, param, toleranceSq, x1, y1, x12, y12, x123, y123, x1234, y1234, level+1); 
    TessellateCubicRecursively(con, param, toleranceSq, x1234, y1234, x234, y234, x34, y34, x4, y4, level+1); 
}


//--------------------------------------------------------------------
void TessellateCubicCurve(TessBase* con, const ToleranceParams& param, 
                          CoordType x2, CoordType y2,
                          CoordType x3, CoordType y3,
                          CoordType x4, CoordType y4)
{
    CoordType x1 = con->GetLastX();
    CoordType y1 = con->GetLastY(); 
    CoordType toleranceSq = (param.CurveTolerance/4) * (param.CurveTolerance/4);
    TessellateCubicRecursively(con, param, toleranceSq, x1, y1, x2, y2, x3, y3, x4, y4, 0);

}



}} // Scaleform::Render

