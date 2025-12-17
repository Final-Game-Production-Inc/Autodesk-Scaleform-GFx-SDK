/**************************************************************************

PublicHeader:   Render
Filename    :   Render_StrokerAA.h
Content     :   
Created     :   2005-2009
Authors     :   Maxim Shemanarev

Notes       :   Anti-Aliased stroke generator

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
**************************************************************************/

#ifndef INC_SF_Render_StrokerAA_H
#define INC_SF_Render_StrokerAA_H

#include "Kernel/SF_Types.h"


#include "Render_Stroker.h"
#include "Render_Matrix2x4.h"
#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
class StrokerAA : public StrokerTypes, public TessBase
{
public:
    struct VertexType
    {
        CoordType   x, y;
        UInt16      style;
        UInt16      alpha;

        VertexType() {}
        VertexType(CoordType x_, CoordType y_, unsigned style_, unsigned alpha_=0) : 
            x(x_), y(y_), style((UInt16)style_), alpha((UInt16)alpha_) 
        {}
    };

    struct TriangleType
    {
        unsigned v1, v2, v3;
    };

    StrokerAA(LinearHeap* heap);

    // Setup
    //----------------------------
    void        SetToleranceParam(const ToleranceParams& p);

    void        SetStyleLeft(int v)                 { StyleLeft = v; }
    void        SetStyleRight(int v)                { StyleRight = v; }

    void        SetSolidWidth(CoordType v)          { WidthLeft = WidthRight = v / 2; }
    void        SetSolidWidthLeft(CoordType v)      { WidthLeft = v; }
    void        SetSolidWidthRight(CoordType v)     { WidthRight = v; }

    void        SetEdgeAAWidth(CoordType v)         { AaWidthLeft = AaWidthRight = v*2; }
    void        SetEdgeAAWidthLeft(CoordType v)     { AaWidthLeft = v*2; }
    void        SetEdgeAAWidthRight(CoordType v)    { AaWidthRight = v*2; }

    void        SetLineJoin(LineJoinType v)         { LineJoin = v; }

    void        SetLineCap(LineCapType v)           { StartLineCap = EndLineCap = v; }
    void        SetStartLineCap(LineCapType v)      { StartLineCap = v; }
    void        SetEndLineCap(LineCapType v)        { EndLineCap = v; }

    void        SetMiterLimit(CoordType v)          { MiterLimit = v; }

    // Add vertices and tessellate
    //----------------------------
    virtual void        Clear();
    virtual CoordType   GetLastX() const { return Path.GetLastX(); }
    virtual CoordType   GetLastY() const { return Path.GetLastY(); }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        ClosePath();
    virtual void        FinalizePath(unsigned, unsigned, bool, bool);

    // Acquire mesh interface
    //----------------------------
    virtual void        Transform(const Matrix2F& m);
    virtual Matrix2F    StretchTo(float x1, float y1, float x2, float y2);

    virtual unsigned    GetVertexCount() const { return (unsigned)Vertices.GetSize(); }
    virtual unsigned    GetMeshCount()   const { return Triangles.GetSize() != 0; }
    virtual unsigned    GetMeshVertexCount(unsigned)  const { return (unsigned)Vertices.GetSize(); }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return (unsigned)Triangles.GetSize(); }
    virtual void        GetMesh(unsigned meshIdx, TessMesh* mesh) const;
    virtual unsigned    GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const;
    virtual void        GetTrianglesI16(unsigned meshIdx, UInt16* idx, unsigned start, unsigned num) const;

private:
    enum VertexMarkers
    {
        MarkerSolidL = 0xFFFFFFFC,
        MarkerSolidR = 0xFFFFFFFD,
        MarkerTotalL = 0xFFFFFFFE,
        MarkerTotalR = 0xFFFFFFFF
    };

    struct WidthsType
    {
        CoordType solidWidthL;
        CoordType solidWidthR;
        CoordType solidWidth;  // Average:  (L + R) / 2
        CoordType totalWidthL;
        CoordType totalWidthR;
        CoordType totalWidth;  // Average:  (L + R) / 2
        CoordType widthCoeff;  // Unsigned: (R > L) ? L/R : R/L
        CoordType solidCoeffL; // Unsigned: SolidL/TotalL
        CoordType solidCoeffR; // Unsigned: SolidR/TotalR
        CoordType solidLimitL;
        CoordType solidLimitR;
        CoordType totalLimitL;
        CoordType totalLimitR;
        bool      solidFlagL;
        bool      solidFlagR;
        bool      aaFlagL;
        bool      aaFlagR;
        bool      solidFlag;
        bool      rightSideCalc;

        WidthsType() : 
            solidWidthL(0), solidWidthR(0), solidWidth(0),
            totalWidthL(0), totalWidthR(0), totalWidth(0),
            widthCoeff(0), 
            solidCoeffL(0), solidCoeffR(0),
            solidLimitL(0), solidLimitR(0),
            totalLimitL(0), totalLimitR(0),
            solidFlagL(0),  solidFlagR(0),
            aaFlagL(0),     aaFlagR(0),
            solidFlag(0),   rightSideCalc(0)
        {}
    };

    struct JoinParamType
    {
        CoordType dx1SolidL, dy1SolidL, dx1TotalL, dy1TotalL;
        CoordType dx2SolidL, dy2SolidL, dx2TotalL, dy2TotalL;
        CoordType dx3SolidL, dy3SolidL, dx3TotalL, dy3TotalL;
        CoordType dx1SolidR, dy1SolidR, dx1TotalR, dy1TotalR;
        CoordType dx2SolidR, dy2SolidR, dx2TotalR, dy2TotalR;
        CoordType dx3SolidR, dy3SolidR, dx3TotalR, dy3TotalR;

        CoordType dbSolidL, dbTotalL, dbSolidR, dbTotalR;

        CoordType xMiterPrevL, yMiterPrevL;
        CoordType xMiterPrevR, yMiterPrevR; 
        CoordType dMiterPrevL, dMiterPrevR; 

        CoordType xMiterThisL, yMiterThisL;
        CoordType xMiterThisR, yMiterThisR;
        CoordType dMiterThisL, dMiterThisR; 

        CoordType xMiterNextL, yMiterNextL;
        CoordType xMiterNextR, yMiterNextR;
        CoordType dMiterNextL, dMiterNextR;

        CoordType xSolidMiterL, ySolidMiterL;
        CoordType xSolidMiterR, ySolidMiterR;
        CoordType dSolidMiterL, dSolidMiterR;

        bool badMiterPrevL, badMiterPrevR;
        bool badMiterThisL, badMiterThisR;
        bool badMiterNextL, badMiterNextR;
        bool rightTurnPrev, rightTurnThis, rightTurnNext;
        bool overlapPrev, overlapThis;

        JoinParamType() : 
            dx1SolidL(0), dy1SolidL(0), dx1TotalL(0), dy1TotalL(0),
            dx2SolidL(0), dy2SolidL(0), dx2TotalL(0), dy2TotalL(0),
            dx3SolidL(0), dy3SolidL(0), dx3TotalL(0), dy3TotalL(0),
            dx1SolidR(0), dy1SolidR(0), dx1TotalR(0), dy1TotalR(0),
            dx2SolidR(0), dy2SolidR(0), dx2TotalR(0), dy2TotalR(0),
            dx3SolidR(0), dy3SolidR(0), dx3TotalR(0), dy3TotalR(0),
            dbSolidL(0), dbTotalL(0), dbSolidR(0), dbTotalR(0),
            xMiterPrevL(0), yMiterPrevL(0),
            xMiterPrevR(0), yMiterPrevR(0), 
            dMiterPrevL(0), dMiterPrevR(0), 
            xMiterThisL(0), yMiterThisL(0),
            xMiterThisR(0), yMiterThisR(0),
            dMiterThisL(0), dMiterThisR(0), 
            xMiterNextL(0), yMiterNextL(0),
            xMiterNextR(0), yMiterNextR(0),
            dMiterNextL(0), dMiterNextR(0),
            xSolidMiterL(0), ySolidMiterL(0),
            xSolidMiterR(0), ySolidMiterR(0),
            dSolidMiterL(0), dSolidMiterR(0),
            badMiterPrevL(0), badMiterPrevR(0),
            badMiterThisL(0), badMiterThisR(0),
            badMiterNextL(0), badMiterNextR(0),
            rightTurnPrev(0), rightTurnThis(0), rightTurnNext(0),
            overlapPrev(0), overlapThis(0) {}
    };

    unsigned addVertex(CoordType x, CoordType y, unsigned style, unsigned alpha=1)
    {
        Vertices.PushBack(VertexType(x, y, style, alpha));
        return (unsigned)Vertices.GetSize() - 1;
    }

    void addTriangle(unsigned v1, unsigned v2, unsigned v3)
    {
        TriangleType tri;
        tri.v1 = v1; tri.v2 = v2; tri.v3 = v3;
        Triangles.PushBack(tri);
    }

    void calcWidths(WidthsType& w) const;

    void calcButtCap(const StrokeVertex& v1, 
                     const StrokeVertex& v2, 
                     CoordType len,
                     const WidthsType& w,
                     bool endFlag);

    void calcRoundCap(const StrokeVertex& v1, 
                      const StrokeVertex& v2, 
                      CoordType len,
                      const WidthsType& w,
                      bool endFlag);

    void calcCap(const StrokeVertex& v1, 
                 const StrokeVertex& v2, 
                 CoordType len,
                 const WidthsType& w,
                 bool endFlag);

    void calcRoundJoin(const StrokeVertex& v1, 
                       const WidthsType& w,
                       const JoinParamType& p);

    void calcBevelJoin(const StrokeVertex& v1, 
                       const WidthsType& w,
                       const JoinParamType& p,
                       LineJoinType lineJoin);

    void calcMiterJoin(const StrokeVertex& v1, 
                       const WidthsType& w,
                       const JoinParamType& p,
                       LineJoinType lineJoin);

    static
    bool MitersIntersect(CoordType ax, CoordType ay, 
                         CoordType bx, CoordType by,
                         CoordType cx, CoordType cy, 
                         CoordType dx, CoordType dy,
                         CoordType epsilon);

    void calcJoinParam(const StrokeVertex& v1, 
                       const StrokeVertex& v2,
                       const StrokeVertex& v3,
                       const WidthsType& w,
                       JoinParamType& p);

    void calcInitialJoinParam(const StrokeVertex& v1, 
                              const StrokeVertex& v2,
                              const WidthsType& w,
                              JoinParamType& p);

    void calcJoin(const StrokeVertex& v1, 
                  const StrokeVertex& v2, 
                  const StrokeVertex& v3,
                  const WidthsType& w,
                  JoinParamType& p);

    void calcButtJoin(const StrokeVertex& v1, 
                      const StrokeVertex& v2, 
                      CoordType len, 
                      const WidthsType& w);

    LineJoinType                    LineJoin;
    LineCapType                     StartLineCap;
    LineCapType                     EndLineCap;
    CoordType                       MiterLimit;
    unsigned                        StyleLeft;
    unsigned                        StyleRight;
    CoordType                       WidthLeft; 
    CoordType                       WidthRight;
    CoordType                       AaWidthLeft;
    CoordType                       AaWidthRight;
    CoordType                       Tolerance;
    CoordType                       IntersectionEpsilon;
    bool                            Closed;
    StrokePath                      Path;
    ArrayPaged<VertexType, 4, 16>   Vertices;
    ArrayPaged<TriangleType, 4, 16> Triangles;
    unsigned                        SolidL;
    unsigned                        SolidR;
    unsigned                        TotalL;
    unsigned                        TotalR;
    CoordType                       MinX;
    CoordType                       MinY;
    CoordType                       MaxX;
    CoordType                       MaxY;
};

//-----------------------------------------------------------------------
inline void StrokerAA::AddVertex(CoordType x, CoordType y)
{
    Path.AddVertex(StrokeVertex(x, y));
}

}} // Scaleform::Render


#endif

