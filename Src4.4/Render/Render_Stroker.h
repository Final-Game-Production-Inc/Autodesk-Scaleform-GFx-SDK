/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Stroker.h
Content     :   Path-to-stroke converter
Created     :   2005
Authors     :   Maxim Shemanarev

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

#ifndef INC_SF_Render_Stroker_H
#define INC_SF_Render_Stroker_H

#include "Kernel/SF_Types.h"


#include "Render_Containers.h"
#include "Render_Math2D.h"
#include "Render_TessDefs.h"
#include "Render_ShapeDataDefs.h"
#include "Render_ToleranceParams.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
struct StrokeVertex
{
    CoordType x;
    CoordType y;
    CoordType dist;

    StrokeVertex() {}
    StrokeVertex(CoordType x_, CoordType y_) :
        x(x_), y(y_), dist(0) {}

    bool Distance(const StrokeVertex& val)
    {
        dist = Math2D::Distance(*this, val);
        return dist > 0;
    }
};

//-----------------------------------------------------------------------
// This is an internal class, a stroke path storage. It calculates
// the distance between the neighbor vertices and filters coinciding
// vertices. It's important for stroke calculations. Function ClosePath()
// checks if the first and last vertices coincide and closes the path,
// i.e., removes the extra vertex and sets Close=true
//-----------------------------------------------------------------------
class StrokePath
{
public:
    typedef Scaleform::Render::CoordType CoordType;
    typedef ArrayPaged<StrokeVertex, 4, 16> ContainerType;

    StrokePath(LinearHeap* heap) : Path(heap) {}

    // Add vertices
    //------------------
    void        Clear();
    void        ClearAndRelease();
    CoordType   GetLastX() const { return Path.Back().x; }
    CoordType   GetLastY() const { return Path.Back().y; }
    void        AddVertex(const StrokeVertex& v);
    bool        ClosePath();

    // Access
    //------------------
    UPInt GetVertexCount() const 
    { 
        return Path.GetSize(); 
    }

    const StrokeVertex& GetVertexPrev(UPInt i) const
    {
        if(i == 0) i += Path.GetSize();
        return Path[i - 1];
    }

    const StrokeVertex& GetVertex(UPInt i) const
    {
        return Path[i];
    }

    StrokeVertex& GetVertex(UPInt i)
    {
        return Path[i];
    }

    const StrokeVertex& GetVertexNext(UPInt i) const
    {
        if(++i >= Path.GetSize()) i -= Path.GetSize();
        return Path[i];
    }

    const StrokeVertex& GetVertexForw(UPInt i) const
    {
        i += 2;
        if(i >= Path.GetSize()) i -= Path.GetSize();
        return Path[i];
    }

private:
    ContainerType Path;

    StrokePath(const StrokePath&);
    void operator = (StrokePath&);
};



// Stroker types, separated so that they can also be used in GStrokerAA.
//-----------------------------------------------------------------------
class StrokerTypes
{
public:
    enum LineCapType
    {
        ButtCap,
        SquareCap,
        RoundCap
    };

    enum LineJoinType
    {
        MiterJoin,      // When exceeding miter limit cut at the limit
        MiterBevelJoin, // When exceeding miter limit use bevel join
        RoundJoin,
        BevelJoin
    };

    enum EquidistantDir
    {
        DirForward,
        DirBackward
    };
};


//-----------------------------------------------------------------------
class Stroker : public StrokerTypes, public TessBase
{
public:
    typedef Scaleform::Render::CoordType CoordType;

    Stroker(LinearHeap* heap);

    // Setup
    //---------------------
    void    SetToleranceParam(const ToleranceParams& param);
    void    SetWidth(CoordType v)           { Width = v/2;  }
    void    SetLineJoin(LineJoinType v)     { LineJoin = v; }
    void    SetLineCap(LineCapType v)       { StartLineCap = EndLineCap = v; }
    void    SetStartLineCap(LineCapType v)  { StartLineCap = v; }
    void    SetEndLineCap(LineCapType v)    { EndLineCap = v; }
    void    SetMiterLimit(CoordType v)      { MiterLimit = v; }


    // TessDef interface
    //---------------------
    virtual void        Clear();
    virtual CoordType   GetLastX() const { return Path.GetLastX(); }
    virtual CoordType   GetLastY() const { return Path.GetLastY(); }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        ClosePath();

    void                GenerateStroke(TessBase* tess);

    // TessDef compatibility
    //---------------------
    virtual void        FinalizePath(unsigned, unsigned, bool, bool) {}
    virtual void        Transform(const Matrix2F&) {}
    virtual Matrix2F    StretchTo(float, float, float, float) { return Matrix2F(); }
    virtual unsigned    GetVertexCount() const { return 0; }
    virtual unsigned    GetMeshCount() const { return 0; }
    virtual unsigned    GetMeshVertexCount(unsigned) const { return 0; }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return 0; }
    virtual void        GetMesh(unsigned, TessMesh*) const {}
    virtual unsigned    GetVertices(TessMesh*, TessVertex*, unsigned) const { return 0; }
    virtual void        GetTrianglesI16(unsigned, UInt16*, unsigned, unsigned) const {}

    void        CalcEquidistant(TessBase* tess, EquidistantDir dir);

private:
    // Copying is prohibited
    Stroker(const Stroker&);
    void operator = (Stroker&);

    void generateStroke(TessBase* tess, UPInt start, UPInt end, bool close);

    void calcArc(TessBase* tess,
                 CoordType x,   CoordType y,
                 CoordType dx1, CoordType dy1,
                 CoordType dx2, CoordType dy2);

    void calcMiter(TessBase* tess,
                   const StrokeVertex& v0,
                   const StrokeVertex& v1,
                   const StrokeVertex& v2,
                   CoordType dx1, CoordType dy1,
                   CoordType dx2, CoordType dy2,
                   LineJoinType lineJoin,
                   CoordType miterLimit,
                   CoordType epsilon,
                   CoordType dbevel);

    void calcCap(TessBase* tess,
                 const StrokeVertex& v0,
                 const StrokeVertex& v1,
                 CoordType len,
                 LineCapType cap);

    void calcJoin(TessBase* tess,
                  const StrokeVertex& v0,
                  const StrokeVertex& v1,
                  const StrokeVertex& v2,
                  CoordType len1,
                  CoordType len2);

private:
    LinearHeap*     pHeap;
    StrokePath      Path;
    CoordType       Width;
    LineJoinType    LineJoin;
    LineCapType     StartLineCap;
    LineCapType     EndLineCap;
    CoordType       MiterLimit;
    CoordType       CurveTolerance;
    CoordType       IntersectionEpsilon;
    bool            Closed;
};

//-----------------------------------------------------------------------
inline void Stroker::AddVertex(CoordType x, CoordType y)
{
    Path.AddVertex(StrokeVertex(x, y));
}


class StrokeScaler : public TessBase
{
    Stroker &Str;
    float ScaleX, ScaleY;
    float LastX, LastY;
public:
    StrokeScaler(Stroker& str, float scaleX, float scaleY) : 
      Str(str), ScaleX(scaleX), ScaleY(scaleY), LastX(0), LastY(0) {}

    // TessDef interface
    //---------------------
    virtual void        Clear() {}
    virtual CoordType   GetLastX() const { return LastX; }
    virtual CoordType   GetLastY() const { return LastY; }
    virtual void        AddVertex(CoordType x, CoordType y)
    {
        LastX = x;
        LastY = y;
        Str.AddVertex(x * ScaleX, y * ScaleY);
    }
    virtual void        ClosePath() {}

    // TessDef compatibility
    //---------------------
    virtual void        FinalizePath(unsigned, unsigned, bool, bool) {}
    virtual void        Transform(const Matrix2F&) {}
    virtual Matrix2F    StretchTo(float, float, float, float) { return Matrix2F(); }
    virtual unsigned    GetVertexCount() const { return 0; }
    virtual unsigned    GetMeshCount() const { return 0; }
    virtual unsigned    GetMeshVertexCount(unsigned) const { return 0; }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return 0; }
    virtual void        GetMesh(unsigned, TessMesh*) const {}
    virtual unsigned    GetVertices(TessMesh*, TessVertex*, unsigned) const { return 0; }
    virtual void        GetTrianglesI16(unsigned, UInt16*, unsigned, unsigned) const {}

private:
    void operator = (StrokeScaler&);
};



// Flash has a weird way of representing the strokes. The stroke
// may consist of a number of paths, that logically represent a 
// closed contour(s), but the paths may be unsorted. To construct 
// a closed contour it's necessary to sort the paths and sometimes
// even revert some of them. It's not a problem for the rasterizer,
// except for some minor slowdown, but critical for the tessellator.
//
// This class sorts the paths and constructs as long chains as 
// possible.
//-----------------------------------------------------------------------
class StrokeSorter : public TessBase
{
    enum
    {
        NumVerMask  = 0x0FFFFFFF,
        VisitedFlag = 0x40000000,
        ClosedFlag  = 0x20000000
    };

    struct PathType
    {
        unsigned start;
        unsigned numVer;
    };

    struct SortedPathType
    {
        CoordType   x, y;
        PathType*   thisPath;
    };

public:
    typedef Scaleform::Render::CoordType CoordType;

    struct VertexType
    {
        CoordType x, y;
        CoordType Dist;
        UInt8     segType;
        bool      snapX;
        bool      snapY;
    };

    typedef ArrayPaged<VertexType, 4, 16> VertexArrayType;
    typedef ArrayPaged<PathType,   4, 16> PathArrayType;


    //---------------------
    StrokeSorter(LinearHeap* heap);

    // TessDef interface
    //---------------------
    virtual void        Clear();
    virtual CoordType   GetLastX() const { return SrcVertices.Back().x; }
    virtual CoordType   GetLastY() const { return SrcVertices.Back().y; }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        FinalizePath(unsigned closeFlag, unsigned, bool, bool);

    // TessDef compatibility
    //---------------------
    virtual void        ClosePath() {}
    virtual void        Transform(const Matrix2F&) {}
    virtual Matrix2F    StretchTo(float, float, float, float) { return Matrix2F(); }
    virtual unsigned    GetVertexCount() const { return 0; }
    virtual unsigned    GetMeshCount() const { return 0; }
    virtual unsigned    GetMeshVertexCount(unsigned) const { return 0; }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return 0; }
    virtual void        GetMesh(unsigned, TessMesh*) const {}
    virtual unsigned    GetVertices(TessMesh*, TessVertex*, unsigned) const { return 0; }
    virtual void        GetTrianglesI16(unsigned, UInt16*, unsigned, unsigned) const {}

    void AddVertexNV(CoordType x, CoordType y, unsigned segType=Math2D::Seg_LineTo);
    void AddQuad(CoordType x2, CoordType y2, CoordType x3, CoordType y3);
    void AddCubic(CoordType x2, CoordType y2, CoordType x3, CoordType y3, CoordType x4, CoordType y4);

    void Sort();
    void AddOffset(CoordType offsetX, CoordType offsetY);
    void Transform(const TransformerBase* tr);
    void Snap(CoordType offsetX, CoordType offsetY);
    void GenerateDashes(const DashArray* da, const ToleranceParams& param, float scale);

    unsigned GetPathCount() const 
    { 
        return (unsigned)OutPaths.GetSize(); 
    }

    bool IsClosed(unsigned pathIdx) const
    {
        return (OutPaths[pathIdx].numVer & ClosedFlag) != 0;
    }

    unsigned GetVertexCount(unsigned pathIdx) const
    {
        return OutPaths[pathIdx].numVer & NumVerMask;
    }

    const VertexType& GetVertex(unsigned pathIdx, unsigned verIdx) const
    {
        const PathType& p = OutPaths[pathIdx];
        unsigned n = p.numVer & NumVerMask;
        return OutVertices[p.start + ((verIdx < n) ? verIdx : verIdx-n)];
    }

private:
    static bool cmpPaths(const SortedPathType& a, const SortedPathType& b)
    {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }

    void  appendPath(PathType* dst, PathType* src);

    unsigned    findNext(const PathType& outPath) const;
    VertexType* getVertex(unsigned pathIdx, unsigned verIdx)
    {
        const PathType& p = OutPaths[pathIdx];
        unsigned n = p.numVer & NumVerMask;
        return &OutVertices[p.start + ((verIdx < n) ? verIdx : verIdx-n)];
    }

private:
    LinearHeap*                     pHeap;
    VertexArrayType                 SrcVertices;
    PathArrayType                   SrcPaths;
    ArrayUnsafe<SortedPathType>     SortedPaths;
    VertexArrayType                 OutVertices;
    PathArrayType                   OutPaths;
    unsigned                        LastVertex;
};



//-----------------------------------------------------------------------
class DashGenerator
{
    enum DashStatus
    {
        Status_Ready,
        Status_Polyline,
        Status_Stop
    };

public:
    DashGenerator(const float* dashArray, unsigned dashCount, float dashStart, 
                  StrokeSorter::VertexType* ver, unsigned verCount, bool closed);


    unsigned GetVertex(float* x, float* y);

private:
    DashGenerator(const DashGenerator&);
    const DashGenerator& operator = (const DashGenerator&);

    const float* pDashArray;
    unsigned     DashCount;
    float        DashStart;
    unsigned     CurrDash;
    float        CurrRest;
    float        CurrDashStart;
    StrokeSorter::VertexType* Vertices;
    unsigned     VerCount;

    const StrokeSorter::VertexType* Ver1;
    const StrokeSorter::VertexType* Ver2;

    bool         Closed;
    DashStatus   Status;
    unsigned     SrcVertex;
};



}} // Scaleform::Render


#endif
