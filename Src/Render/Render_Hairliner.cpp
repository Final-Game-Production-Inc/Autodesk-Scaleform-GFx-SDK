/**************************************************************************

Filename    :   Render_Hairliner.cpp
Content     :   This class performs Intersection Perception and 
                creates a bidirectional graph
Created     :   2005-2008
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Hairliner.h"

#ifdef SF_RENDER_ENABLE_HAIRLINER

#include "Kernel/SF_Alg.h"

#ifdef SF_DEBUG_DRAW
#include "AggDraw.h"
extern AggDraw* DrawPtr;
#endif

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
Hairliner::Hairliner(LinearHeap* heap) : 
    pHeap(heap),
    Epsilon(0),
    IntersectionEpsilon(0.01f),
    Width(0.5f),
    Paths(heap),
    SrcVertices(heap),
    OutVertices(heap),
    Triangles(heap),
    LastVertex(0),
    SrcEdges(heap),
    MonoChains(heap),
    MonoChainsSorted(heap),
    Scanbeams(heap),
    HorizontalEdges(heap),
    StartHorizontals(0),
    NumHorizontals(0),
    LastX(MinCoord),
    LastY(MinCoord),
    MinX(MaxCoord),
    MinY(MaxCoord),
    MaxX(MinCoord),
    MaxY(MinCoord),
    ActiveChains(heap),
    ChainsAtBottom(heap),
    ValidChains(heap),
    Intersections(heap),
    ContourNodes(heap),
    FanEdges(heap)
{}

//-----------------------------------------------------------------------
void Hairliner::Clear()
{
    Paths.ClearAndRelease();
    SrcVertices.ClearAndRelease();
    OutVertices.ClearAndRelease();
    Triangles.ClearAndRelease();
    LastVertex = 0;
    SrcEdges.ClearAndRelease();
    MonoChains.ClearAndRelease();
    MonoChainsSorted.ClearAndRelease();
    Scanbeams.ClearAndRelease();
    HorizontalEdges.ClearAndRelease();
    StartHorizontals = 0;
    NumHorizontals = 0;
    ActiveChains.ClearAndRelease();
    ChainsAtBottom.ClearAndRelease();
    ValidChains.ClearAndRelease();
    Intersections.ClearAndRelease();
    ContourNodes.ClearAndRelease();
    FanEdges.ClearAndRelease();
    MinX = MaxCoord;
    MinY = MaxCoord;
    MaxX = MinCoord;
    MaxY = MinCoord;
    pHeap->ClearAndRelease();
}


//-----------------------------------------------------------------------
void Hairliner::SetToleranceParam(const ToleranceParams& param)
{
    Epsilon = param.Epsilon;
    IntersectionEpsilon = param.IntersectionEpsilon;
}



//-----------------------------------------------------------------------
inline bool Hairliner::cmpMonoChains(const MonoChainType* a, 
                                     const MonoChainType* b)  
{
    if (a->ySort != b->ySort) return a->ySort < b->ySort;
    if (a->xb != b->xb) return a->xb < b->xb;
    return a->xt < b->xt;
}

//-----------------------------------------------------------------------
inline bool Hairliner::cmpActiveChains(const MonoChainType* a, 
                                       const MonoChainType* b)
{ 
    if(a->xb != b->xb) return a->xb < b->xb;
    return a->xt < b->xt;
}

//-----------------------------------------------------------------------
inline bool Hairliner::cmpIntersections(const IntersectionType& a, 
                                        const IntersectionType& b)
{
    return a.y < b.y;
}

//-----------------------------------------------------------------------
inline bool Hairliner::cmpHorizontalEdges(const HorizontalEdgeType& a, 
                                          const HorizontalEdgeType& b)
{
    return a.y < b.y;
}

//-----------------------------------------------------------------------
inline bool Hairliner::cmpEdges(const FanEdgeType& a, const FanEdgeType& b)
{
    if(a.node1 != b.node1) return a.node1 < b.node1;
    return a.slope < b.slope;
}

//-----------------------------------------------------------------------
inline bool Hairliner::cmpNode1(const FanEdgeType& a, unsigned b)
{
    return (a.node1 & ~FanEdgeType::Visited) < b;
}


//-----------------------------------------------------------------------
inline bool Hairliner::forwardMin(int idx, int start) const
{
    CoordType y = SrcVertices[idx].y;
    if(idx <= start) return SrcVertices[idx+1].y > y;
    return SrcVertices[idx-1].y >= y && SrcVertices[idx+1].y > y;
}

//-----------------------------------------------------------------------
inline bool Hairliner::reverseMin(int idx, int end) const
{
    CoordType y = SrcVertices[idx].y;
    if(idx >= end) return SrcVertices[idx-1].y > y;
    return SrcVertices[idx-1].y > y && SrcVertices[idx+1].y >= y;
}

//-----------------------------------------------------------------------
inline bool Hairliner::forwardMax(int idx, int end) const
{
    if(idx >= end) return true;
    return SrcVertices[idx+1].y <= SrcVertices[idx].y;
}

//-----------------------------------------------------------------------
inline bool Hairliner::reverseMax(int idx, int start) const
{
    if(idx <= start) return true;
    return SrcVertices[idx-1].y <= SrcVertices[idx].y;
}

//-----------------------------------------------------------------------
void Hairliner::buildEdgeList(unsigned start, unsigned numEdges, int step)
{
    SrcEdgeType e;
    unsigned i;
    unsigned startEdgeIdx = (unsigned)SrcEdges.GetSize();
    for(i = 0; i < numEdges; ++i)
    {
        e.lower = start;
        start  += step;
        e.upper = start;
        const SrcVertexType& v1 = SrcVertices[e.lower];
        const SrcVertexType& v2 = SrcVertices[e.upper];
        e.slope = (v2.x - v1.x) / (v2.y - v1.y);
        e.next  = 0;
        SrcEdges.PushBack(e);
        if(i) 
        {
            SrcEdges[SrcEdges.GetSize()-2].next = &SrcEdges.Back();
        }
    }
    MonoChainType mc;
    mc.edge        = &SrcEdges[startEdgeIdx];
    mc.ySort       = SrcVertices[mc.edge->lower].y;
    mc.xb          = SrcVertices[mc.edge->lower].x;
    mc.xt          = mc.edge->slope; // For initial sorting
    mc.flags       = 0;
    mc.prevVertex  = ~0U;
    MonoChains.PushBack(mc);
}


//-----------------------------------------------------------------------
void Hairliner::ClosePath()
{
    SrcVertices.PushBack(SrcVertices[LastVertex]);
}


//-----------------------------------------------------------------------
void Hairliner::FinalizePath(unsigned, unsigned, bool, bool)
{
    if((unsigned)SrcVertices.GetSize() < LastVertex + 2)
    {
        SrcVertices.CutAt(LastVertex);
        return;
    }

    PathType path;
    path.start = LastVertex;
    path.end   = (unsigned)SrcVertices.GetSize() - 1;
    Paths.PushBack(path);
    LastVertex = (unsigned)SrcVertices.GetSize();
}

//-----------------------------------------------------------------------
void Hairliner::decomposePath(const PathType& path)
{
    int start = path.start;
    int end   = path.end;
    unsigned i;
    CoordType y1 = SrcVertices[start].y;
    Scanbeams.PushBack(start);
    for(i = start+1; i <= (unsigned)end; ++i)
    {
        const SrcVertexType& v1 = SrcVertices[i-1];
        const SrcVertexType& v2 = SrcVertices[i];
        if(v2.y != y1)
        {
            Scanbeams.PushBack(i);
            y1 = v2.y;
        }
        else
        if(v1.x != v2.x)
        {
            HorizontalEdgeType he;
            he.x1 = v1.x;
            he.x2 = v2.x;
            he.y  = v2.y;
            he.lv = ~0U;
            he.rv = ~0U;
            if(he.x1 > he.x2) Alg::Swap(he.x1, he.x2);
            HorizontalEdges.PushBack(he);
        }
    }

    int min, max;
    int numEdges;

    // Do the path forward pass
    for(min = start; min < end; ++min)
    {
        // If a forward local minimum...
        if(forwardMin(min, start))
        {
            // Search for the next local maximum...
            numEdges = 1;
            max = min + 1;
            while(!forwardMax(max, end))
            {
                ++numEdges;
                ++max;
            }

            // Build the next edge list 
            buildEdgeList(min, numEdges, +1);
            min += numEdges - 1;
        }
    }

    // Do the path reverse pass
    for(min = end; min > start; --min)
    {
        // If a reverse local minimum...
        if(reverseMin(min, end))
        {
            // Search for the previous local maximum...
            numEdges = 1;
            max = min - 1;
            while(!reverseMax(max, start))
            {
                ++numEdges;
                --max;
            }

            // Build the previous edge list
            buildEdgeList(min, numEdges, -1);
            min -= numEdges - 1;
        }
    }
}

//-----------------------------------------------------------------------
inline CoordType Hairliner::calcX(const SrcEdgeType* edge, CoordType y) const
{
    const SrcVertexType& lower = SrcVertices[edge->lower];
    return lower.x + (y - lower.y) * edge->slope;
}

//-----------------------------------------------------------------------
unsigned Hairliner::nextScanbeam(CoordType yb, CoordType yt, 
                                 unsigned startMc, unsigned numMc)
{
    UPInt i, j;
    MonoChainType* mc;
    const SrcVertexType* lower;
    const SrcVertexType* upper;
    IntersectionType in;
    unsigned retFlags = 0;

    if(numMc) retFlags = InsertEdgesFlag;

    ValidChains.Clear();
    for(i = 0; i < ActiveChains.GetSize(); ++i)
    {
        mc = ActiveChains[i];

        // Clear EventFlag, that is, assume the chain won't have 
        // an event point.
        //------------------------
        mc->flags &= ~EventFlag;

        if(SrcVertices[mc->edge->upper].y == yb)
        {
            // There is an event point, that is, the yb scan-beam 
            // goes through the upper vertex of the edge
            //-----------------------
            if(mc->edge->next)
            {
                // Next edge exists, the chain continues.
                // Switch to the next edge, initialize xb, calculate xt, 
                // and mark the chain as having an event point.
                //----------------
                mc->edge = mc->edge->next;
                lower    = &SrcVertices[mc->edge->lower];
                upper    = &SrcVertices[mc->edge->upper];
                mc->xb   = lower->x;
                mc->xt   = (upper->y == yt) ? upper->x : calcX(mc->edge, yt);
                ValidChains.PushBack((unsigned)i);
            }
            else
            {
                // Chain ends at this scan-beam. Move xb to xt
                // and mark the chain as ending and having an event point.
                //------------------
                mc->xb     = mc->xt;
                mc->flags |= EndChainFlag;
                retFlags  |= RemoveEdgesFlag;
            }
            mc->flags |= EventFlag;
        }
        else
        {
            // The scan-beam yb crosses the edge somewhere in the middle.
            // No event points, just calculate new xb and xt.
            //------------------
            lower  = &SrcVertices[mc->edge->lower];
            upper  = &SrcVertices[mc->edge->upper];
            mc->xb = mc->xt;
            mc->xt = (upper->y == yt) ? upper->x : calcX(mc->edge, yt);
            ValidChains.PushBack((unsigned)i);
        }
    }

    // Add new monotone chains. First, the chains are added to the 
    // end of the array and then they are arranged in-order using
    // a modification of insertion sort. The new chains are already 
    // arranged, so that, it's possible to insert them properly 
    // in a single pass.
    //-----------------------
    if (numMc)
    {
        for(i = 0; i < numMc; ++i)
        {
            mc        = MonoChainsSorted[startMc+i];
            lower     = &SrcVertices[mc->edge->lower];
            upper     = &SrcVertices[mc->edge->lower];
            mc->xb    = lower->x;
            mc->flags = EventFlag;
            mc->xt    = (upper->y == yt) ? upper->x : calcX(mc->edge, yt);
            ActiveChains.PushBack(0);
        }

        i = ActiveChains.GetSize();
        j = ActiveChains.GetSize() - numMc;
        while(numMc)
        {
            if (j == 0 || 
                cmpActiveChains(ActiveChains[j-1], MonoChainsSorted[startMc+numMc-1]))
            {
                ActiveChains[--i] = MonoChainsSorted[startMc + (--numMc)];
            }
            else
            {
                ActiveChains[--i] = ActiveChains[--j];
            }
        }
    }

    Intersections.Clear();

    if(retFlags & InsertEdgesFlag)
    {
        // Re-create a list of valid chains to be sorted
        ValidChains.Clear();
        for(i = 0; i < ActiveChains.GetSize(); ++i)
        {
            if((ActiveChains[i]->flags & EndChainFlag) == 0) 
            {
                ValidChains.PushBack((unsigned)i);
            }
        }
    }

    // Perceive intersections.
    // To do that we just sort the edges by top X, using 
    // a O(N^2) sort method (insertion sort)
    // The necessity to swap the edges means they have an intersection
    // point within the yb...yt slab
    //---------------------
    CoordType height = yt - yb;
    for(i = 1; i < ValidChains.GetSize(); ++i)
    {
        int jc;
        for(jc = (int)i - 1; jc >= 0; --jc)
        {
            in.mc1 = ActiveChains[ValidChains[jc]];
            in.mc2 = ActiveChains[ValidChains[jc+1]];

            if(in.mc1->xt <= in.mc2->xt) break;

            if(Intersections.GetSize() == 0)
            {
                ChainsAtBottom.Clear();
                for (UPInt k = 0; k < ActiveChains.GetSize(); ++k)
                {
                    ChainsAtBottom.PushBack(ActiveChains[k]);
                }
            }

            CoordType nom = in.mc1->xb - in.mc2->xb;
            CoordType den = in.mc2->xt - in.mc2->xb - in.mc1->xt + in.mc1->xb;

            in.y    = (den == 0) ? yb : yb + height * nom / den;
            if(in.y < yb)
            {
                in.y = yb;
            }
            if(in.y > yt)
            {
                in.y = yt;
            }
            Intersections.PushBack(in);
            Alg::Swap(ActiveChains[ValidChains[jc  ]], 
                      ActiveChains[ValidChains[jc+1]]);
        }
    }

    return retFlags;
}



//-----------------------------------------------------------------------
inline unsigned Hairliner::addEventVertex(const SrcVertexType& v1)
{
    if(v1.y != LastY || v1.x != LastX)
    {
        LastX = v1.x;
        LastY = v1.y;
        OutVertexType v2 = { v1.x, v1.y, 1 };
        OutVertices.PushBack(v2);
    }
    return (unsigned)OutVertices.GetSize() - 1;
}


//-----------------------------------------------------------------------
inline unsigned Hairliner::addEventVertex(const MonoChainType* mc, 
                                          CoordType yb, bool enforce)
{
    // (1) Check the condition if the left and right styles remain 
    //     the same and producing the vertex is not enforced. 
    //------------
    if(!enforce)
    {
        // (2) If the styles are the same, add the event vertex only 
        //     in case the current edge has changed (EventFlag) and its 
        //     lower vertex lies exactly on the scan-beam. The last 
        //     condition is necessary to process self-intersections 
        //     correctly. When the horizontal band is divided into a 
        //     number of sub-bands in function processInterior(), 
        //     there should be a check for adding the same vertex 
        //     only once. The next sub-band will have a different 
        //     Y coordinate and the same vertex will be skipped.
        //     This branch is executed most frequently. Also only in
        //     this branch the function may return -1 ("No Event").
        //---------------
        if((mc->flags & EventFlag) != 0 && SrcVertices[mc->edge->lower].y == yb)
        {
            return addEventVertex(SrcVertices[mc->edge->lower]);
        }
        else
        {
            return ~0U;
        }
    }
    else
    {
        // (3) Intersection or chain-end.
        //     Before calculating the intersection it's a good idea to 
        //     check for trivial cases, when the current scan-beam 
        //     coincides with the lower or upper edge vertex.
        //     This branch always returns a valid vertex, whether 
        //     a new or existing one.
        //--------------
        if((mc->flags & EndChainFlag) != 0 && SrcVertices[mc->edge->upper].y == yb)
        {
            return addEventVertex(SrcVertices[mc->edge->upper]);
        }

        if(SrcVertices[mc->edge->lower].y == yb) 
        {
            return addEventVertex(SrcVertices[mc->edge->lower]);
        }

        // (4) The scan-bean differs from the lower or upper vertex,
        //     calculate the intersection point with function calcX().
        //     This code also checks for the same X coordinate with 
        //     certain Epsilon value. If the difference between the 
        //     last X and the calculated X is within this epsilon, 
        //     the vertices merge.
        //--------------
        CoordType x = calcX(mc->edge, yb);
        if(yb != LastY)
        {
            LastX = x;
            LastY = yb;
            OutVertexType v2 = { x, yb, 1 };
            OutVertices.PushBack(v2);
        }
        else
        if(fabsf(x - LastX) > Epsilon)
        {
            LastX = x;
            LastY = yb;
            OutVertexType v2 = { x, yb, 1 };
            OutVertices.PushBack(v2);
        }
    }
    return (unsigned)OutVertices.GetSize() - 1;
}


//-----------------------------------------------------------------------
inline void Hairliner::emitEdge(unsigned v1, unsigned v2)
{
    FanEdgeType e;
    e.node1 = v1;
    e.node2 = v2;
    e.slope = Math2D::SlopeRatio(OutVertices[v1], OutVertices[v2]);
    FanEdges.PushBack(e);

    e.node1 = v2;
    e.node2 = v1;
    e.slope -= 1;
    if(e.slope < -1.0f) e.slope += 2.0f;
    FanEdges.PushBack(e);
}

//-----------------------------------------------------------------------
inline void Hairliner::emitEdge(MonoChainType* mc, unsigned vertex)
{
    if(mc->prevVertex != ~0U && mc->prevVertex != vertex)
    {
        emitEdge(mc->prevVertex, vertex);
    }
    mc->prevVertex = vertex;
}

//-----------------------------------------------------------------------
inline void Hairliner::emitEdge(HorizontalEdgeType* he, unsigned vertex)
{
    if(he->rv != ~0U && he->rv != vertex)
    {
        emitEdge(he->rv, vertex);
    }
    he->rv = vertex;
}

//-----------------------------------------------------------------------
unsigned Hairliner::processHorizontalEdges(MonoChainType* mc, 
                                           unsigned vertex, 
                                           CoordType yb)
{
    unsigned  i;
    bool      xFlag = false;
    CoordType x = 0;
    for(i = 0; i < NumHorizontals; ++i)
    {
        HorizontalEdgeType* he = &HorizontalEdges[StartHorizontals + i];
        if(vertex != ~0U && he->rv == ~0U && he->x1 == OutVertices[vertex].x)
        {
            he->lv = vertex;
            he->rv = vertex;
        }
        else
        if(vertex != ~0U && he->x2 == OutVertices[vertex].x)
        {
            emitEdge(he, vertex);
            if(he->lv == ~0U) he->lv = vertex;
        }
        else
        {
            if(!xFlag)
            {
                const SrcVertexType& lower = SrcVertices[mc->edge->lower];
                const SrcVertexType& upper = SrcVertices[mc->edge->upper];
                     if(yb == lower.y) x = lower.x;
                else if(yb == upper.y) x = upper.x;
                else x = calcX(mc->edge, yb);
                xFlag = true;
            }
            if(x == he->x1)
            {
                SrcVertexType v = { x, yb };
                if(vertex == ~0U) vertex = addEventVertex(v);
                he->rv = vertex;
            }
            else
            if(x >= he->x1 && x <= he->x2)
            {
                SrcVertexType v = { x, yb };
                if(vertex == ~0U) vertex = addEventVertex(v);
                emitEdge(he, vertex);
                if(he->lv == ~0U) he->lv = vertex;
            }
        }
    }
    return vertex;
}

//-----------------------------------------------------------------------
void Hairliner::sweepScanbeam(const ChainPtrArray& aet, CoordType yb)
{
    UPInt i;
    for(i = 0; i < aet.GetSize(); ++i)
    {
        MonoChainType* mc = aet[i];
        unsigned thisVertex = addEventVertex(mc, yb, (mc->flags & EndChainFlag) != 0);
        if(NumHorizontals)
        {
            thisVertex = processHorizontalEdges(mc, thisVertex, yb);
        }
        if(thisVertex != ~0U) emitEdge(mc, thisVertex);
    }
    for(i = 0; i < NumHorizontals; ++i)
    {
        HorizontalEdgeType* he = &HorizontalEdges[StartHorizontals + i];
        if(he->lv != ~0U)
        {
            const OutVertexType& v = OutVertices[he->lv];
            if(v.x != he->x1)
            {
                SrcVertexType sv = { he->x1, yb };
                emitEdge(he->lv, addEventVertex(sv));
            }
        }
        if(he->rv != ~0U)
        {
            const OutVertexType& v = OutVertices[he->rv];
            if(v.x != he->x2)
            {
                SrcVertexType sv = { he->x2, yb };
                emitEdge(he->rv, addEventVertex(sv));
            }
        }
        if(he->lv == ~0U && he->rv == ~0U)
        {
            SrcVertexType v1 = { he->x1, yb };
            SrcVertexType v2 = { he->x2, yb };
            emitEdge(addEventVertex(v1), addEventVertex(v2));
        }
    }
}

//-----------------------------------------------------------------------
void Hairliner::processInterior(CoordType yb)
{
    UPInt i;
    sweepScanbeam(ChainsAtBottom, yb);
    for(i = 0; i < Intersections.GetSize(); ++i)
    {
        const IntersectionType& in = Intersections[i];
        if(in.y > yb)
        {
            emitEdge(in.mc1, addEventVertex(in.mc1, in.y, true));
            emitEdge(in.mc2, addEventVertex(in.mc2, in.y, true));
        }
    }
}


//-----------------------------------------------------------------------
void Hairliner::buildGraph()
{
    UPInt i, j;

    if(SrcVertices.GetSize() == 0) return;

    // Prepare and sort the Scanbeams array
    //------------------
    for(i = 0; i < SrcVertices.GetSize(); ++i)
    {
        Scanbeams.PushBack((unsigned)i);
    }
    CmpScanbeams cmpScanbeams(SrcVertices);
    Alg::QuickSort(Scanbeams, cmpScanbeams);

    // Clean up the coordinates. 
    // When two Y values are too close just make them exactly equal 
    // and remove the respective elements from Scanbeams.
    //-----------------
    CoordType y1 = MinCoord;
    for(i = j = 0; i < Scanbeams.GetSize(); ++i)
    {
        SrcVertexType& v2 = SrcVertices[Scanbeams[i]];
        if(v2.y - y1 > fabsf(v2.y) * Epsilon)
        {
            Scanbeams[j++] = Scanbeams[i];
            y1 = v2.y;
        }
        else
        {
            v2.y = y1;
        }
    }
    Scanbeams.CutAt(j);

    for(i = 0; i < Paths.GetSize(); ++i)
    {
        decomposePath(Paths[i]);
    }

    // Fill MonoChainsSorted array and sort it by ySort
    //------------------
    for(i = 0; i < MonoChains.GetSize(); ++i)
        MonoChainsSorted.PushBack(&MonoChains[i]);

    Alg::QuickSort(HorizontalEdges, cmpHorizontalEdges);
    Alg::QuickSort(MonoChainsSorted, cmpMonoChains);

    unsigned sb = 0; // Scan beam index
    unsigned mc = 0; // Monotone Chain index
    unsigned he = 0; // Horizontal edges
    CoordType yb, yt;

    yb = yt = SrcVertices[Scanbeams[0]].y;

    LastX = MinCoord;
    LastY = MinCoord;

    // Perform scanbeam processing loop
    //----------------
    while(sb < Scanbeams.GetSize())
    {
        if(++sb < Scanbeams.GetSize())
        {
            yt = SrcVertices[Scanbeams[sb]].y;
        }

        // While monotone chains corresponding to yb exist
        unsigned startMc = mc;
        while(mc < MonoChainsSorted.GetSize() && 
              MonoChainsSorted[mc]->ySort <= yb) ++mc;

        // While horizontal edges corresponding to yb exist
        StartHorizontals = he;
        while(he < HorizontalEdges.GetSize() && 
              HorizontalEdges[he].y <= yb) ++he;

        NumHorizontals = he - StartHorizontals;

        unsigned flags = nextScanbeam(yb, yt, startMc, mc - startMc);

        if(Intersections.GetSize())
        {
            processInterior(yb);
        }
        else
        {
            sweepScanbeam(ActiveChains, yb);
        }

        if(flags & RemoveEdgesFlag)
        {
            unsigned pi, pos;
            for(pi = pos = 0; pi < ActiveChains.GetSize(); ++pi)
            {
                MonoChainType* mct = ActiveChains[pi];
                if((mct->flags & EndChainFlag) == 0)
                {
                    ActiveChains[pos++] = mct;
                }
            }
            ActiveChains.CutAt(pos);
        }
        yb = yt; // Advance yb
    }
}


//-----------------------------------------------------------------------
void Hairliner::generateContourAA(unsigned startEdgeIdx)
{
    FanEdgeType* startEdge = &FanEdges[startEdgeIdx];
    FanEdgeType* edge = startEdge;
    CoordType prevX = MinCoord;
    CoordType prevY = MinCoord;

    ContourNodes.Clear();
    do
    {
        if(edge->node1 & FanEdgeType::Visited)
            break;

        unsigned thisNode = edge->node1 & ~FanEdgeType::Visited;

        const OutVertexType& v = OutVertices[thisNode];
        if(v.x != prevX || v.y != prevY)
        {
            ContourNodes.PushBack(thisNode);
            prevX = v.x;
            prevY = v.y;
        }
        edge->node1 |= FanEdgeType::Visited;

        UPInt startFan = Alg::LowerBound(FanEdges, edge->node2, cmpNode1);
        if(startFan >= FanEdges.GetSize()) break;

        UPInt fanSize = 0;
        while(startFan + fanSize < FanEdges.GetSize())
        {
            const FanEdgeType& e = FanEdges[startFan+fanSize];
            if((e.node1 & ~FanEdgeType::Visited) != edge->node2)
                break;
            ++fanSize;
        }

        if(fanSize == 1)
        {
            edge = &FanEdges[startFan];
        }
        else
        if(fanSize == 2)
        {
            FanEdgeType* e1 = &FanEdges[startFan];
            FanEdgeType* e2 = &FanEdges[startFan+1];
            if(e1->node2 == e2->node2)
            {
                edge = (e1->node1 & FanEdgeType::Visited) ? e2 : e1;
            }
            else
            {
                edge = (e1->node2 == thisNode) ? e2 : e1;
            }
        }
        else
        {
            unsigned i;
            for(i = 0; i < fanSize; ++i)
            {
                const FanEdgeType& e = FanEdges[startFan + i];
                if(e.node2 == thisNode)
                {
                    unsigned j;
                    for(j = 0; j < fanSize; ++j)
                    {
                        ++i;
                        if(i >= fanSize) i = 0;
                        edge = &FanEdges[startFan + i];
                        if(edge == startEdge || (edge->node1 & FanEdgeType::Visited) == 0)
                            break;
                    }
                    break;
                }
            }
        }
    }
    while(edge != startEdge);
    if(ContourNodes.GetSize())
    {
        const OutVertexType& v1 = OutVertices[ContourNodes[0]];
        const OutVertexType& v2 = OutVertices[ContourNodes.Back()];
        if(v1.x == v2.x && v1.y == v2.y)
        {
            ContourNodes.PopBack();
        }
    }
}


//-----------------------------------------------------------------------
unsigned Hairliner::addJoin(unsigned refVertex, 
                            const OutVertexType& v1, 
                            const OutVertexType& v2, 
                            const OutVertexType& v3, 
                            CoordType len1, CoordType len2, 
                            CoordType width)
{
    CoordType turn = Math2D::TurnRatio(v1, v2, v3, len1, len2);
    CoordType epsilon = (len1 + len2) * IntersectionEpsilon;
    CoordType dx1 = width * (v1.y - v2.y) / len1;
    CoordType dy1 = width * (v2.x - v1.x) / len1;
    CoordType dx2 = width * (v2.y - v3.y) / len2;
    CoordType dy2 = width * (v3.x - v2.x) / len2;
    CoordType xi = v2.x;
    CoordType yi = v2.y;
    OutVertexType o1, o2;
    TriangleType tri;
    unsigned nv = 1;

    o1.alpha = o2.alpha = 0;
    if (fabsf(turn) < 0.125f)
    {
        if(len1 > len2) { o1.x = v2.x + dx1; o1.y = v2.y + dy1; }
        else            { o1.x = v2.x + dx2; o1.y = v2.y + dy2; }
        OutVertices.PushBack(o1);
    }
    else
    {
        if (Math2D::Intersection(v1.x + dx1, v1.y + dy1,
                                 v2.x + dx1, v2.y + dy1,
                                 v2.x + dx2, v2.y + dy2,
                                 v3.x + dx2, v3.y + dy2,
                                 &xi, &yi, epsilon))
        {
            CoordType d1 = Math2D::Distance(v2.x, v2.y, xi, yi);
            CoordType lim;

            if(turn > 0)
            {
                // Inner Join
                //---------------------
                lim = ((len1 < len2) ? len1 : len2) / turn;

                // Clamp version. Reduces the EdgaAA triangles in complex cases
                //---------------------
                //if(d1 > lim)
                //{
                //    lim /= d1;
                //    xi = v2.x + (xi - v2.x) * lim;
                //    yi = v2.y + (yi - v2.y) * lim;
                //}
                //emitStrokerVertex(xi, yi);

                // Double vertex version. Generates more overlapped EdgeAA 
                // triangles, but in most cases looks better.
                //---------------------
                if(d1 > lim)
                {
                    o1.x = v2.x + dx1;
                    o1.y = v2.y + dy1;
                    o2.x = v2.x + dx2;
                    o2.y = v2.y + dy2;
                    OutVertices.PushBack(o1);
                    OutVertices.PushBack(o2);
                    nv = 2;
                }
                else
                {
                    o1.x = xi;
                    o1.y = yi;
                    OutVertices.PushBack(o1);
                }
            }
            else
            {
                // Outer Join
                //---------------------
                lim = -width * 4;
                if(d1 <= lim)
                {
                    // Inside the miter limit
                    //---------------------
                    o1.x = xi;
                    o1.y = yi;
                    OutVertices.PushBack(o1);
                }
                else
                {
                    o1.x = v2.x + dx1 - 2*dy1; 
                    o1.y = v2.y + dy1 + 2*dx1;
                    o2.x = v2.x + dx2 + 2*dy2;
                    o2.y = v2.y + dy2 - 2*dx2;
                    OutVertices.PushBack(o1);
                    OutVertices.PushBack(o2);
                    tri.v1 = refVertex;
                    tri.v2 = (unsigned)OutVertices.GetSize() - 2;
                    tri.v3 = (unsigned)OutVertices.GetSize() - 1;
                    Triangles.PushBack(tri);
                    nv = 2;
                }
            }
        }
        else
        {
            // Calculation of the intersection failed, most probably
            // the three points lie one straight line. 
            //----------------
            o1.x = v2.x + dx1 - dy1; 
            o1.y = v2.y + dy1 + dx1;
            o2.x = v2.x + dx2 + dy2; 
            o2.y = v2.y + dy2 - dx2;
            OutVertices.PushBack(o1);
            OutVertices.PushBack(o2);
            tri.v1 = refVertex;
            tri.v2 = (unsigned)OutVertices.GetSize() - 2;
            tri.v3 = (unsigned)OutVertices.GetSize() - 1;
            Triangles.PushBack(tri);
            nv = 2;
        }
    }
    return nv;
}



//-----------------------------------------------------------------------
void Hairliner::generateTriangles(CoordType width)
{
    if(ContourNodes.GetSize() < 2) return;

    TriangleType tri;
    unsigned i;
    unsigned thisRef = ContourNodes.Back();
    const OutVertexType* v1 = &OutVertices[ContourNodes[ContourNodes.GetSize() - 2]];
    const OutVertexType* v2 = &OutVertices[thisRef];
    const OutVertexType* v3;
    CoordType len1 = Math2D::Distance(*v1, *v2);
    CoordType len2;
    unsigned  prevRef  = ~0U;
    unsigned  prevAA   = ~0U;
    unsigned  startRef = ~0U;
    unsigned  startAA  = ~0U;
    for(i = 0; i < ContourNodes.GetSize(); ++i)
    {
        v3   = &OutVertices[ContourNodes[i]];
        len2 = Math2D::Distance(*v2, *v3);
        unsigned nv = addJoin(thisRef, *v1, *v2, *v3, len1, len2, width);
        if(prevRef == ~0U)
        {
            startRef = thisRef;
            startAA  = (unsigned)OutVertices.GetSize() - nv;
        }
        else
        {
            tri.v1 = prevRef;
            tri.v2 = prevAA;
            tri.v3 = (unsigned)OutVertices.GetSize() - nv;
            Triangles.PushBack(tri);
            tri.v1 = prevRef;
            tri.v2 = (unsigned)OutVertices.GetSize() - nv;
            tri.v3 = thisRef;
            Triangles.PushBack(tri);
        }
        v1   = v2;
        v2   = v3;
        len1 = len2;
        prevRef = thisRef;
        thisRef = ContourNodes[i];
        prevAA  = (unsigned)OutVertices.GetSize() - 1;
    }
    if(prevRef != ~0U)
    {
        tri.v1 = prevRef;
        tri.v2 = prevAA;
        tri.v3 = startAA;
        Triangles.PushBack(tri);
        tri.v1 = prevRef;
        tri.v2 = startAA;
        tri.v3 = startRef;
        Triangles.PushBack(tri);
    }
}

//-----------------------------------------------------------------------
void Hairliner::Tessellate()
{
    MinX = MaxCoord;
    MinY = MaxCoord;
    MaxX = MinCoord;
    MaxY = MinCoord;

    Triangles.Clear();

    buildGraph();

    if(FanEdges.GetSize() < 2) return;

    Alg::QuickSort(FanEdges, cmpEdges);

    UPInt i, j;
    // Remove duplicate edges to improve visual result
    // in some cases.
    //-------------------

    for(i = 1, j = 1; i < FanEdges.GetSize(); ++i)
    {
        const FanEdgeType& e1 = FanEdges[i-1];
        const FanEdgeType& e2 = FanEdges[i];
        if(e1.node1 != e2.node1 || e1.node2 != e2.node2)
        {
            FanEdges[j++] = e2;
        }
    }
    FanEdges.CutAt(j);

    for(i = 0; i < FanEdges.GetSize(); ++i)
    {
        FanEdgeType& e = FanEdges[i];
        if((e.node1 & FanEdgeType::Visited) == 0)
        {
            generateContourAA((unsigned)i);
            generateTriangles(-Width);
        }
    }
}


//-----------------------------------------------------------------------
void Hairliner::Transform(const Matrix2F& m)
{
    UPInt i;
    MinX = MaxCoord;
    MinY = MaxCoord;
    MaxX = MinCoord;
    MaxY = MinCoord;
    for (i = 0; i < OutVertices.GetSize(); ++i)
    {
        // TO DO: possible to optimize access
        OutVertexType& v = OutVertices[i];
        m.Transform(&v.x, &v.y);
        if (v.x < MinX) MinX = v.x;
        if (v.y < MinY) MinY = v.y;
        if (v.x > MaxX) MaxX = v.x;
        if (v.y > MaxY) MaxY = v.y;
    }
}

//-----------------------------------------------------------------------
Matrix2F Hairliner::StretchTo(float x1, float y1, float x2, float y2)
{
    Matrix2F m;
    UPInt i;
    if (MinX >= MaxX || MinY >= MaxY)
    {
        MinX = MaxCoord;
        MinY = MaxCoord;
        MaxX = MinCoord;
        MaxY = MinCoord;
        for (i = 0; i < OutVertices.GetSize(); ++i)
        {
            // TO DO: possible to optimize access
            OutVertexType v = OutVertices[i];
            if (v.x < MinX) MinX = v.x;
            if (v.y < MinY) MinY = v.y;
            if (v.x > MaxX) MaxX = v.x;
            if (v.y > MaxY) MaxY = v.y;
        }
    }
    if (MinX < MaxX && MinY < MaxY)
    {
        m.SetRectToRect(MinX, MinY, MaxX, MaxY, x1, y1, x2, y2);
        for (i = 0; i < OutVertices.GetSize(); ++i)
        {
            // TO DO: possible to optimize access
            OutVertexType& v = OutVertices[i];
            m.Transform(&v.x, &v.y);
        }
        m.Invert();
    }
    return m;
}

//-----------------------------------------------------------------------
void Hairliner::GetMesh(unsigned, TessMesh* mesh) const
{
    mesh->MeshIdx = 0;
    mesh->Style1  = 1;
    mesh->Style2  = 0;
    mesh->Flags1  = 0;
    mesh->Flags2  = 0;
    mesh->StartVertex = 0;
    mesh->VertexCount = (unsigned)OutVertices.GetSize();
}

//-----------------------------------------------------------------------
unsigned Hairliner::GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const
{
    unsigned i = 0;
    for (i = 0; i < num && mesh->StartVertex < OutVertices.GetSize(); ++mesh->StartVertex)
    {
        const OutVertexType& v = OutVertices[mesh->StartVertex];
        vertices->x = v.x;
        vertices->y = v.y;
        vertices->Idx = 0;
        vertices->Styles[0] = 1;
        vertices->Styles[1] = 0;
        vertices->Flags = UInt16(v.alpha ? TessStyleOpaque : 0);
        ++vertices;
        ++i;
    }
    return i;
}

//-----------------------------------------------------------------------
void Hairliner::GetTrianglesI16(unsigned, UInt16* idx, unsigned start, unsigned num) const
{
    for(unsigned j = 0; j < num; ++j)
    {
        const TriangleType& t = Triangles[start+j];
        *idx++ = (UInt16)t.v1;
        *idx++ = (UInt16)t.v2;
        *idx++ = (UInt16)t.v3;
    }
}


/*
//-----------------------------------------------------------------------
unsigned Hairliner::GetVertices(OutVertexType* vertices, unsigned start, unsigned num) const
{
    unsigned i = 0;
    for (i = 0; i < num && start < OutVertices.GetSize(); ++start)
    {
        vertices[i++] = OutVertices[start];
    }
    return i;
}
*/


}} // Scaleform::Render

#endif // SF_RENDER_ENABLE_HAIRLINER
