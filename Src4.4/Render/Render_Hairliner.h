/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Hairliner.h
Content     :   This class performs Intersection Perception and 
                creates a bidirectional graph
Created     :   2005-2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Hairliner_H
#define INC_SF_Render_Hairliner_H

#include "Kernel/SF_Types.h"

#ifdef SF_RENDER_ENABLE_HAIRLINER

#include "Render_Containers.h"
#include "Render_Math2D.h"
#include "Render_Matrix2x4.h"
#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
class Hairliner : public TessBase
{
public:
    struct SrcVertexType
    {
        CoordType x, y;
    };

    struct OutVertexType
    {
        CoordType x, y;
        unsigned  alpha;
    };

    struct TriangleType
    {
        unsigned v1, v2, v3;
    };

private:
    enum EdgeFlags_e
    {
        EndChainFlag        = 1 << 0,
        EventFlag           = 1 << 1,
        IntersectionFlag    = 1 << 2
    };

    enum ScanbeamFlags_e
    {
        InsertEdgesFlag = 1,
        RemoveEdgesFlag = 2
    };

    struct PathType
    {
        unsigned    start;
        unsigned    end;
    };

    struct SrcEdgeType
    {
        unsigned     lower;  // Edge lower point (global index)
        unsigned     upper;  // Edge upper point (global index)
        CoordType    slope;  // Change in X for a unit Y increase
        SrcEdgeType* next;   // Edge connected at the upper end
    };

    struct MonoChainType
    {
        SrcEdgeType* edge;       // Current edge during monotonization
        CoordType    ySort;      // Y coordinate to sort
        CoordType    xb;         // Scanbean bottom X during monotonization
        CoordType    xt;         // Scanbeam top X during monotonization
        unsigned     flags;
        unsigned     prevVertex; // Previous event vertex 
                                 // (chain vertex or intersection)
    };

    struct IntersectionType
    {
        MonoChainType* mc1;
        MonoChainType* mc2;
        CoordType y;
    };

    struct HorizontalEdgeType
    {
        CoordType x1;
        CoordType x2;
        CoordType y;
        unsigned  lv; // First left vertex
        unsigned  rv; // Last right vertex
    };

    struct FanEdgeType
    {
        enum { Visited = 0x80000000U };
        unsigned node1, node2;
        CoordType slope;
    };

public:
    //-----------------------------------
    Hairliner(LinearHeap* heap);

    // Setup (optional)
    //-----------------------------------
    void SetToleranceParam(const ToleranceParams& param);
    void SetWidth(CoordType w) { Width = 2*w; }

    // Processing
    //-----------------------------------
    virtual void Clear();

    virtual CoordType   GetLastX() const { return SrcVertices.Back().x; }
    virtual CoordType   GetLastY() const { return SrcVertices.Back().y; }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        ClosePath();
    virtual void        FinalizePath(unsigned, unsigned, bool, bool);

    void Tessellate();

    // Acquire mesh interface
    //----------------------------
    virtual void        Transform(const Matrix2F& m);
    virtual Matrix2F    StretchTo(float x1, float y1, float x2, float y2);

    virtual unsigned    GetVertexCount() const { return (unsigned)OutVertices.GetSize(); }
    virtual unsigned    GetMeshCount()   const { return Triangles.GetSize() != 0; }
    virtual unsigned    GetMeshVertexCount(unsigned)  const { return (unsigned)OutVertices.GetSize(); }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return (unsigned)Triangles.GetSize(); }
    virtual void        GetMesh(unsigned meshIdx, TessMesh* mesh) const;
    virtual unsigned    GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const;
    virtual void        GetTrianglesI16(unsigned meshIdx, UInt16* idx, unsigned start, unsigned num) const;

private:
    typedef ArrayPaged<MonoChainType*, 4, 8>   ChainPtrArray;
    typedef ArrayPaged<SrcVertexType, 4, 16>   SrcVertexArray;

    struct CmpScanbeams
    {
        const SrcVertexArray& Ver;
        CmpScanbeams(const SrcVertexArray& v):Ver(v){}
        bool operator() (unsigned a, unsigned b) const
        {
            return Ver[a].y < Ver[b].y;
        }
    private: void operator=(CmpScanbeams&);
    };

    static bool cmpMonoChains(const MonoChainType* a, const MonoChainType* b);
    static bool cmpActiveChains(const MonoChainType* a, const MonoChainType* b);
    static bool cmpIntersections(const IntersectionType& a, const IntersectionType& b);
    static bool cmpHorizontalEdges(const HorizontalEdgeType& a, const HorizontalEdgeType& b);
    static bool cmpEdges(const FanEdgeType& a, const FanEdgeType& b);
    static bool cmpNode1(const FanEdgeType& a, unsigned b);

    bool forwardMin(int idx, int start) const;
    bool reverseMin(int idx, int end) const;
    bool forwardMax(int idx, int end) const;
    bool reverseMax(int idx, int start) const;

    void decomposePath(const PathType& path);
    void buildEdgeList(unsigned start, unsigned numEdges, int step);

    CoordType calcX(const SrcEdgeType* edge, CoordType yt) const;

    unsigned nextScanbeam(CoordType yb, CoordType yt, unsigned startMc, unsigned numMc);

    void setupIntersections();

    unsigned addEventVertex(const SrcVertexType& v);
    unsigned addEventVertex(const MonoChainType* mc, CoordType yb, bool enforce);

    unsigned processHorizontalEdges(MonoChainType* mc, unsigned vertex, CoordType yb);
    void     sweepScanbeam(const ChainPtrArray& aet, CoordType yb);
    void     buildGraph();

    void emitEdge(unsigned v1, unsigned v2);
    void emitEdge(MonoChainType* mc, unsigned vertex);
    void emitEdge(HorizontalEdgeType* he, unsigned vertex);
    void processInterior(CoordType yb);

    unsigned addJoin(unsigned refVertex, 
                     const OutVertexType& v1, 
                     const OutVertexType& v2, 
                     const OutVertexType& v3, 
                     CoordType len1, CoordType len2, 
                     CoordType width);

    void generateTriangles(CoordType width);
    void generateContourAA(unsigned edge);

    //-------------------------------------------------------------------
    LinearHeap*                             pHeap;
    CoordType                               Epsilon;
    CoordType                               IntersectionEpsilon;
    CoordType                               Width;

    ArrayPaged<PathType, 4, 4>              Paths;
    SrcVertexArray                          SrcVertices;
    ArrayPaged<OutVertexType, 4, 16>        OutVertices;
    ArrayPaged<TriangleType, 4, 16>         Triangles;
    unsigned                                LastVertex;
    ArrayPaged<SrcEdgeType, 4, 16>          SrcEdges;
    ArrayPaged<MonoChainType, 4, 8>         MonoChains;
    ChainPtrArray                           MonoChainsSorted;
    ArrayPaged<unsigned, 4, 16>             Scanbeams;
    ArrayPaged<HorizontalEdgeType, 2, 4>    HorizontalEdges;
    unsigned                                StartHorizontals;
    unsigned                                NumHorizontals;
    CoordType                               LastX;
    CoordType                               LastY;
    CoordType                               MinX;
    CoordType                               MinY;
    CoordType                               MaxX;
    CoordType                               MaxY;

    ChainPtrArray                           ActiveChains;
    ChainPtrArray                           ChainsAtBottom;
    ArrayPaged<unsigned, 4, 16>             ValidChains;
    ArrayPaged<IntersectionType, 4, 4>      Intersections;
    ArrayPaged<unsigned, 4, 16>             ContourNodes;
    ArrayPaged<FanEdgeType, 4, 16>          FanEdges;
};

//-----------------------------------------------------------------------
inline void Hairliner::AddVertex(CoordType x, CoordType y)
{
    SrcVertexType v = { x, y };
    SrcVertices.PushBack(v);
}

}} // Scaleform::Render

#endif // SF_RENDER_ENABLE_HAIRLINER

#endif
