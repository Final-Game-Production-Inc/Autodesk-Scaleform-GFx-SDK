/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Tessellator.h
Content     :   An optimal Flash compound shape tessellator & EdgeAA
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Tessellator_H
#define INC_SF_Render_Tessellator_H

#ifdef SF_DEBUG_DRAW
#include "AggDraw.h"
extern AggDraw* DrawPtr;
#define private public
#endif

#include "Render_TessDefs.h"
#include "Render_Math2D.h"
#include "Render_Matrix2x4.h"
#include "Render_Containers.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
class Tessellator : public TessBase
{
public:
    enum FillRuleType
    {
        FillNonZero,
        FillEvenOdd,
        FillStroker
    };

    struct SrcVertexType
    {
        CoordType x, y;
        SrcVertexType() {}
        SrcVertexType(CoordType x_, CoordType y_) : x(x_), y(y_) {}
    };

    struct MonoVertexType
    {
        unsigned        srcVer;
#ifdef SF_RENDER_ENABLE_EDGEAA
        unsigned        aaVer;
#endif
        MonoVertexType* next;
    };

    struct TriangleType
    {
        union
        {
            struct { MonoVertexType *v1, *v2, *v3; } m;
            struct { unsigned v1, v2, v3; } t;
        } d;
    };

private:
    enum EdgeFlags_e
    {
        EndChainFlag    = 1 << 1,
        VisibleChain    = 1 << 2,
        EventFlag       = 1 << 3,
        HasIntersection = 1 << 4
    };

    enum ScanbeamFlags_e
    {
        InsertEdgesFlag = 1,
        RemoveEdgesFlag = 2
    };

    enum ChainFlagAtScanline_e
    {
        ChainContinuesAtScanline = 1,
        ChainStartsAtScanline    = 2,
        ChainEndsAtScanline      = 3
    };

    enum 
    { 
        LeftFlag    = 0x80000000U,
        VisitedFlag = 0x40000000U
    };

    static inline bool     isLeft(unsigned v)        { return (v & LeftFlag) != 0; }
    static inline bool     isRight(unsigned v)       { return (v & LeftFlag) == 0; }
    static inline unsigned verIdx(unsigned v)        { return v & 0x0FFFFFFF; }
    static inline UInt16   styleIdx(unsigned v)      { return UInt16(v & 0x7FFFU); }
    static inline unsigned styleIsLeft(unsigned v)   { return v & 0x8000; }
           inline UInt32   isComplex(unsigned a, unsigned b) const;

    struct PathType
    {
        unsigned  start;
        unsigned  end;
        unsigned  leftStyle;
        unsigned  rightStyle;
    };

    struct EdgeType
    {
        unsigned  lower;  // Edge lower point (global index)
        CoordType slope;  // Change in X for a unit Y increase
    };

    struct MonoChainType
    {
        unsigned    edge;       // Current edge index during monotonization
        unsigned    end;        // Ending edge in chain
        CoordType   ySort;      // Y coordinate to sort
        CoordType   xb;         // Scanbean bottom X during monotonization
        CoordType   xt;         // Scanbeam top X during monotonization

        SInt16      dir;        // Edge direction, +1=up, -1=down
        UInt16      flags;

        UInt16      leftStyle;  // Original left style
        UInt16      rightStyle; // Original right style
        UInt16      leftBelow;  // Left style below scan line
        UInt16      leftAbove;  // Left style above scan line
        UInt16      rightBelow; // Right style below scan line
        UInt16      rightAbove; // Right style above scan line

        unsigned    posIntr;    // Position in the ActiveChains table at 
                                // the bottom of the slab, before perceiving 
                                // the intersections

        unsigned    posScan;    // Position in the scan-beam
    };

    struct IntersectionType
    {
        unsigned  pos1;
        unsigned  pos2;
        CoordType y;
    };

    struct BaseLineType
    {
        CoordType y;
        unsigned  styleLeft;    // Style on the left of the trapezoid
        unsigned  vertexLeft;   // Opposite left srcVer of the trapezoid (if any)
        unsigned  vertexRight;  // Opposite right srcVer of the trapezoid (if any)
        unsigned  firstChain;   // Index of the first chain in the array
        unsigned  numChains;    // Number of chains in the array
        unsigned  leftAbove;    // Index of the chain on the left in the array (if any)
    };

    struct MonotoneType
    {
        MonoVertexType* start;
        union
        {
            struct 
            {
                unsigned    lastIdx;
                unsigned    prevIdx1;
                unsigned    prevIdx2;
            } m;
            struct 
            {
                unsigned    startTriangle;
                unsigned    numTriangles;
                unsigned    meshIdx;
            } t;
        }               d;
        unsigned        style;
        BaseLineType*   lowerBase;
    };

    struct ScanChainType
    {
        MonoChainType*  chain;
        MonotoneType*   monotone;
        unsigned        vertex;
    };

    struct PendingEndType
    {
        unsigned      vertex;
        MonotoneType* monotone;
    };

#ifdef SF_RENDER_ENABLE_EDGEAA
    struct EdgeAAType
    {
        MonoVertexType* cntVer; // Center vertex
        MonoVertexType* rayVer; // Ray vertex
        UInt16          style;
        UInt16          slope;
    };

    struct TmpEdgeAAType
    {
        MonoVertexType* cntVer; // Center vertex
        MonoVertexType* rayVer; // Ray vertex
        CoordType       slope;
        unsigned        style;
    };

    struct InnerQuadType
    {
        EdgeAAType* e1;
        EdgeAAType* e2;
    };

    struct OuterEdgeType
    {
        EdgeAAType* edge;
        unsigned    outVer;
    };

    struct StrokerEdgeType
    {
        unsigned node1, node2;
    };

    struct StarVertexType
    {
        unsigned cntVer;
        unsigned starVer;
    };
#endif


public:
    //-----------------------------------
    Tessellator(LinearHeap* heap1, LinearHeap* heap2);

    // Setup (optional)
    //-----------------------------------
    // FillRule = FillNonZero by default
    void        SetFillRule(FillRuleType f);
    void        SetToleranceParam(const ToleranceParams& param);

#ifdef SF_RENDER_ENABLE_EDGEAA
    void        SetEdgeAAWidth(CoordType w);
#endif

    void        EnableInvasiveMerge(bool m)     { InvasiveMerge = m; }
    void        SetVertexLimit(unsigned lim)    { VertexLimit = lim; }
    unsigned    GetVertexLimit() const          { return VertexLimit; }

    // Tessellation
    //-----------------------------------
    virtual void        Clear();

    virtual CoordType   GetLastX() const { return SrcVertices.Back().x; }
    virtual CoordType   GetLastY() const { return SrcVertices.Back().y; }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        ClosePath();
    virtual void        FinalizePath(unsigned leftStyle, unsigned rightStyle,
                                     bool leftComplex, bool rightComplex);

    void        GetSrcBounds(CoordType* x1, CoordType* y1, CoordType* x2, CoordType* y2) const;
    void        Tessellate(bool autoSplitMeshes=false);
    void        SplitMeshes();



    virtual void        Transform(const Matrix2F& m);
    virtual Matrix2F    StretchTo(float x1, float y1, float x2, float y2);

    virtual unsigned    GetMeshCount() const                        { return (unsigned)Meshes.GetSize(); }
    virtual unsigned    GetMeshVertexCount(unsigned meshIdx)  const { return Meshes[meshIdx].VertexCount; }
    virtual unsigned    GetMeshTriangleCount(unsigned meshIdx) const { return (unsigned)MeshTriangles.GetSize(meshIdx); }
    virtual void        GetMesh(unsigned meshIdx, TessMesh* mesh) const;
    virtual unsigned    GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const;
    virtual void        GetTrianglesI16(unsigned meshIdx, UInt16* idx, unsigned start, unsigned num) const;

    void SortTrianglesByVertex1(unsigned meshIdx);

    unsigned            GetVertexCount()  const { return (unsigned)MeshVertices.GetSize(); }
    const TessVertex&   GetVertex(unsigned i) const { return MeshVertices[i]; }
    const TriangleType& GetTriangle(unsigned i, unsigned j) const { return MeshTriangles.At(i, j); }
          TriangleType& GetTriangle(unsigned i, unsigned j)       { return MeshTriangles.At(i, j); }

    unsigned                GetSrcVertexCount()  const { return (unsigned)SrcVertices.GetSize(); }
    const SrcVertexType&    GetSrcVertex(unsigned i) const { return SrcVertices[i]; }

private:
    typedef ArrayPaged<SrcVertexType, 4, 16>   SrcVertexArray;
    typedef ArrayPaged<TessVertex, 4, 16>      MeshVertexArray;
    typedef ArrayPaged<MonoChainType*, 4, 8>   ChainPtrArray;
    typedef ArrayPaged<ScanChainType, 4, 8>    ScanChainArray;
    typedef ArrayPaged<PendingEndType, 4, 4>   PendingEndArray;
    typedef ArrayPaged<MonoVertexType*, 4, 8>  ChainVertexArray;
    typedef ArrayJagged<TriangleType, 4, 16>   TriangleArray;
#ifdef SF_RENDER_ENABLE_EDGEAA
    typedef ArrayUnsafe<EdgeAAType>            EdgeFanArray;
#endif

    void clearHeap1();
    void monotonize();

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
    static bool cmpIntersectionY(const IntersectionType& a, const IntersectionType& b);
    struct CmpVer1
    {
        const MeshVertexArray& Ver;
        CmpVer1(const MeshVertexArray& v):Ver(v){}
        bool operator() (const TriangleType& a, const TriangleType& b) const
        {
            const TessVertex& av = Ver[a.d.t.v1];
            const TessVertex& bv = Ver[b.d.t.v1];
            if (av.y != bv.y) return av.y < bv.y;
            return av.x < bv.x;
        }
    private: void operator=(CmpVer1&);
    };

    bool forwardMin(int idx, int start) const;
    bool reverseMin(int idx, int end) const;
    bool forwardMax(int idx, int end) const;
    bool reverseMax(int idx, int start) const;

    void buildEdgeList(unsigned start, unsigned numEdges, int step,
                       unsigned leftStyle, unsigned rightStyle);

    void decomposePath(const PathType& path);
    void addStyle(unsigned style, bool complex);
    void incStyles(const MonoChainType* mc);
    UInt16 findElderStyle() const;
    void perceiveStyles(const ChainPtrArray& aet);

    CoordType calcX(const EdgeType* edge, CoordType yt) const;

    unsigned nextScanbeam(CoordType yb, CoordType yt, unsigned startMc, unsigned numMc);

    void setupIntersections();

    unsigned addEventVertex(const SrcVertexType& v);
    unsigned addEventVertex(const MonoChainType* mc, CoordType yb, bool enforceFlag);

    void addPendingEnd(ScanChainType* dst, ScanChainType* pending, CoordType y);

    unsigned lastMonoVertex(const MonotoneType* m) const;
    void removeLastMonoVertex(MonotoneType* m);

    static void resetMonotone(MonotoneType* m, unsigned style);
    MonotoneType* startMonotone(unsigned style);
    void startMonotone(ScanChainType* chain, unsigned vertex);
    void replaceMonotone(ScanChainType* chain, unsigned style);
    void replaceMonotone(PendingEndType* pe, unsigned style);

    static int pendingMonotoneStyle(const PendingEndType* pe)
    {
        return pe->monotone ? pe->monotone->style : 0;
    }

    static int startingMonotoneStyle(const ScanChainType* chain)
    {
        return chain->chain->rightAbove;
    }

    void connectPendingToLeft    (ScanChainType* chain, unsigned targetVertex);
    void connectPendingToRight   (ScanChainType* chain, unsigned targetVertex);
    void connectStartingToPending(ScanChainType* chain, BaseLineType* upperBase);
    void connectStartingToLeft   (ScanChainType* chain, BaseLineType* upperBase, unsigned targetVertex);
    void connectStartingToRight  (ScanChainType* chain, BaseLineType* upperBase, unsigned targetVertex);
    void connectStarting         (ScanChainType* chain, BaseLineType* upperBase);

    void growMonotone(MonotoneType* m, unsigned vertex);
    void growMonotone(MonotoneType* m, unsigned left, unsigned right);
    void growMonotone(ScanChainType* chain, unsigned vertex);

    unsigned nextChainInBundle(unsigned below, unsigned above, unsigned vertex) const;
    void     sweepScanbeam(const ChainPtrArray& aet, CoordType yb);

    void swapChains(unsigned startIn, unsigned endIn);
    void processInterior(CoordType yb, CoordType yTop, unsigned perceiveFlag);

    unsigned setMesh(unsigned style);
    unsigned setMesh(unsigned style1, unsigned style2);
    unsigned emitVertex(unsigned meshIdx, unsigned ver, unsigned style, unsigned flags);

    void splitMesh(TessMesh* mesh);

#ifdef SF_RENDER_ENABLE_EDGEAA
    static bool cmpEdgeAA(const TmpEdgeAAType& a, const TmpEdgeAAType& b);
    static bool cmpOuterEdges(const OuterEdgeType& a, const OuterEdgeType& b);
    static bool cmpStrokerEdges(const StrokerEdgeType& a, const StrokerEdgeType& b);
    static bool cmpStrokerNode1(const StrokerEdgeType& e, unsigned node);

    void collectFanEdges(const ChainVertexArray& chain, 
                         const ChainVertexArray& oppos, unsigned style);

    unsigned countFanEdges(MonotoneType* m);
    void     collectFanEdges(MonotoneType* m);

    bool computeMiter(const TessVertex& v1, 
                      const TessVertex& v2, 
                      const TessVertex& v3,
                      TessVertex* newVer1,
                      TessVertex* newVer2);

    void     processFan(unsigned start, unsigned end);
    unsigned emitVertex(unsigned meshIdx, unsigned ver, unsigned style1, unsigned style2, 
                        unsigned flags, bool strictStyle=false);

    void emitTriangles();

    unsigned addStrokerJoin(const StrokerEdgeType* e1, const StrokerEdgeType* e2);
    void     emitStrokerVertex(CoordType x, CoordType y);
    void     emitStrokerTrapezoid(const StrokerEdgeType* edge, unsigned v1, unsigned v2);
    void     addStrokerEdge(unsigned v1, unsigned v2);
    void     processStrokerEdges();
    void     processEdgeAA();

    void moveVertexAA(const TessVertex& refVer, TessVertex* aaVer, 
                      const TessVertex& v2, const TessVertex& v3);

    void      addTriangleAA(MonoVertexType* v1, MonoVertexType* v2, MonoVertexType* v3, CoordType cp);

#ifdef SF_RENDER_ENABLE_MOUNTAINS
    CoordType   triangleRatio(MonoVertexType* v1, MonoVertexType* v2, MonoVertexType* v3) const;
    void        triangulateMountainAA();
#else
    CoordType   triangleCrossProductAA(unsigned i1, unsigned i2, unsigned i3) const;
    void        addTriangleAA(MonoVertexType* v1, MonoVertexType* v2, MonoVertexType* v3);
#endif
    void triangulateMonotoneAA(MonotoneType* m);
    void unflipTriangles();

#else
    CoordType triangleCrossProduct(unsigned i1, unsigned i2, unsigned i3) const;
    void      addTriangle(unsigned i1, unsigned i2, unsigned i3, CoordType cp);
    void      addTriangle(unsigned i1, unsigned i2, unsigned i3);
    void      triangulateMonotone(const MonotoneType& monotone);
#endif

    //-------------------------------------------------------------------
    LinearHeap*                                 pHeap1;
    LinearHeap*                                 pHeap2;
    FillRuleType                                FillRule;
    CoordType                                   Epsilon;
    bool                                        HasEpsilon;
    bool                                        StrokerMode;
    bool                                        InvasiveMerge;
    bool                                        HasComplexFill;
    unsigned                                    VertexLimit;

    ArrayUnsafe<UInt32>                         ComplexFlags;
    unsigned                                    MaxStyle;
    ArrayPaged<PathType, 4, 4>                  Paths;
    SrcVertexArray                              SrcVertices;
    unsigned                                    LastVertex;
    ArrayPaged<EdgeType, 4, 16>                 Edges;
    ArrayPaged<MonoChainType, 4, 16>            MonoChains;
    ArrayUnsafe<MonoChainType*>                 MonoChainsSorted;
    ArrayPaged<unsigned, 4, 16>                 Scanbeams;

    ChainPtrArray                               ActiveChains;
    ChainPtrArray                               InteriorChains;
    ArrayPaged<unsigned, 4, 16>                 ValidChains;
    ArrayPaged<unsigned, 4, 16>                 InteriorOrder;
    ArrayPaged<IntersectionType, 4, 4>          Intersections;

    ArrayUnsafe<int>                            StyleCounts;

    CoordType                                   LastX;
    CoordType                                   MinX;
    CoordType                                   MinY;
    CoordType                                   MaxX;
    CoordType                                   MaxY;

    ScanChainArray                              ChainsBelow;
    ScanChainArray                              ChainsAbove;
    ArrayPaged<BaseLineType, 4, 4>              BaseLines;   // Base lines of the pending ends
    PendingEndArray                             PendingEnds; // An array of all pending monotones

    ArrayPaged<MonotoneType, 4, 16>             Monotones;
    ArrayPaged<MonoVertexType, 4, 16>           MonoVertices;
    MeshVertexArray                             MeshVertices;

    ArrayMatrix<UInt16>                         StyleMatrix;
    ArrayPaged<TessMesh, 4, 4>                  Meshes;
    TriangleArray                               MeshTriangles;

    unsigned                                    MonoStyle;
    unsigned                                    MeshIdx;
    unsigned                                    FactorOneFlag;

#ifdef SF_RENDER_ENABLE_EDGEAA
    CoordType                                   EdgeAAWidth;
    bool                                        EdgeAAFlag;
    CoordType                                   IntersectionEpsilon;
    ChainVertexArray                            LeftChain;
    ChainVertexArray                            RightChain;
    ArrayPaged<TmpEdgeAAType, 3, 4>             TmpEdgeFan;
    EdgeFanArray                                EdgeFans;
    ArrayPaged<unsigned, 3, 4>                  StartFan;
    ArrayPaged<unsigned, 3, 4>                  EndFan;
    ArrayPaged<OuterEdgeType, 4, 16>            OuterEdges;
    ArrayPaged<StrokerEdgeType, 4, 16>          StrokerEdges;

    ArrayPaged<MonoVertexType*, 4, 2>           MonoStack;
    ArrayPaged<InnerQuadType, 4, 16>            InnerQuads;

    ArrayPaged<StarVertexType, 4, 16>           StarVertices;

#else
    ArrayPaged<unsigned, 4, 2>                  MonoStack;
#endif
    int                                         FakeVar;
};

//-----------------------------------------------------------------------
inline void Tessellator::AddVertex(CoordType x, CoordType y)
{
    SrcVertices.PushBack(SrcVertexType(x, y));
}

#ifdef SF_DEBUG_DRAW
#undef private
#endif

}} // Scaleform::Render

#endif
