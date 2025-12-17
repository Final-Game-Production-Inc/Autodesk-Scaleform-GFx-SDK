/**************************************************************************

Filename    :   Render_Tessellator.cpp
Content     :   An optimal Flash compound shape tessellator & EdgeAA
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Alg.h"
#include "Render_Tessellator.h"
#include "Kernel/SF_AmpInterface.h"

namespace Scaleform { namespace Render {

Tessellator::Tessellator(LinearHeap* heap1, LinearHeap* heap2) :
    pHeap1(heap1),
    pHeap2(heap2),
    FillRule(FillNonZero),
    Epsilon(0),
    HasEpsilon(false),
    StrokerMode(false),
    InvasiveMerge(true),
    HasComplexFill(false),
    VertexLimit(65534), // Fit 16-bit indices by default
    ComplexFlags(heap2),
    MaxStyle(0),
    Paths(heap1),
    SrcVertices(heap1),
    LastVertex(0),
    Edges(heap1),
    MonoChains(heap1),
    MonoChainsSorted(heap1),
    Scanbeams(heap1),
    ActiveChains(heap1),
    InteriorChains(heap1),
    ValidChains(heap1),
    InteriorOrder(heap1),
    Intersections(heap1),
    StyleCounts(heap1),
    LastX(MinCoord),
    MinX(MaxCoord),
    MinY(MaxCoord),
    MaxX(MinCoord),
    MaxY(MinCoord),
    ChainsBelow(heap1),
    ChainsAbove(heap1),
    BaseLines(heap1),
    PendingEnds(heap1),
    Monotones(heap2),
    MonoVertices(heap2),
    MeshVertices(heap2),
    StyleMatrix(heap2),
    Meshes(heap2),
    MeshTriangles(heap1),

    MonoStyle(0),
    MeshIdx(~0U),
    FactorOneFlag(0),

#ifdef SF_RENDER_ENABLE_EDGEAA
    EdgeAAWidth(0.5f),
    EdgeAAFlag(true),
    IntersectionEpsilon(0.01f),
    LeftChain(heap1),
    RightChain(heap1),
    TmpEdgeFan(heap1),
    EdgeFans(heap1),
    StartFan(heap1),
    EndFan(heap1),
    OuterEdges(heap1),
    StrokerEdges(heap1),
    MonoStack(heap1),
    InnerQuads(heap1),
    StarVertices(heap1),
#else
    MonoStack(heap1),
#endif
    FakeVar(0)
{}

//-----------------------------------------------------------------------
void Tessellator::Clear()
{
    LastVertex = 0;

    HasComplexFill = false;
    MaxStyle = 0;
    LastX = MinCoord;
    MinX  = MaxCoord;
    MinY  = MaxCoord;
    MaxX  = MinCoord;
    MaxY  = MinCoord;
    ComplexFlags.ClearAndRelease();
    Paths.ClearAndRelease();
    SrcVertices.ClearAndRelease();
    Edges.ClearAndRelease();
    MonoChains.ClearAndRelease();
    MonoChainsSorted.ClearAndRelease();
    Scanbeams.ClearAndRelease();
    ActiveChains.ClearAndRelease();
    InteriorChains.ClearAndRelease();
    ValidChains.ClearAndRelease();
    InteriorOrder.ClearAndRelease();
    Intersections.ClearAndRelease();
    StyleCounts.ClearAndRelease();
    ChainsBelow.ClearAndRelease();
    ChainsAbove.ClearAndRelease();
    BaseLines.ClearAndRelease();
    PendingEnds.ClearAndRelease();
    Monotones.ClearAndRelease();
    MonoVertices.ClearAndRelease();
    MeshVertices.ClearAndRelease();
    StyleMatrix.ClearAndRelease();
    Meshes.ClearAndRelease();
    MeshTriangles.ClearAndRelease();

#ifdef SF_RENDER_ENABLE_EDGEAA
    LeftChain.ClearAndRelease();
    RightChain.ClearAndRelease();
    TmpEdgeFan.ClearAndRelease();
    EdgeFans.ClearAndRelease();
    StartFan.ClearAndRelease();
    EndFan.ClearAndRelease();
    OuterEdges.ClearAndRelease();
    StrokerEdges.ClearAndRelease();
    MonoStack.ClearAndRelease();
    InnerQuads.ClearAndRelease();
    StarVertices.ClearAndRelease();
#else
    MonoStack.ClearAndRelease();
#endif
    pHeap1->ClearAndRelease();
    pHeap2->ClearAndRelease();
}



//-----------------------------------------------------------------------
void Tessellator::clearHeap1()
{
    Paths.ClearAndRelease();
    SrcVertices.ClearAndRelease();
    Edges.ClearAndRelease();
    MonoChains.ClearAndRelease();
    MonoChainsSorted.ClearAndRelease();
    Scanbeams.ClearAndRelease();
    ActiveChains.ClearAndRelease();
    InteriorChains.ClearAndRelease();
    ValidChains.ClearAndRelease();
    InteriorOrder.ClearAndRelease();
    Intersections.ClearAndRelease();
    StyleCounts.ClearAndRelease();
    ChainsBelow.ClearAndRelease();
    ChainsAbove.ClearAndRelease();
    BaseLines.ClearAndRelease();
    PendingEnds.ClearAndRelease();
    MeshTriangles.ClearAndRelease();

#ifdef SF_RENDER_ENABLE_EDGEAA
    LeftChain.ClearAndRelease();
    RightChain.ClearAndRelease();
    EdgeFans.ClearAndRelease();
    StartFan.ClearAndRelease();
    TmpEdgeFan.ClearAndRelease();
    EndFan.ClearAndRelease();
    OuterEdges.ClearAndRelease();
    StrokerEdges.ClearAndRelease();
    MonoStack.ClearAndRelease();
    InnerQuads.ClearAndRelease();
    StarVertices.ClearAndRelease();
#else
    MonoStack.ClearAndRelease();
#endif
    pHeap1->Clear();
}

//-----------------------------------------------------------------------
void Tessellator::SetToleranceParam(const ToleranceParams& param)
{
    Epsilon = param.Epsilon;
    HasEpsilon = Epsilon > 0;
#ifdef SF_RENDER_ENABLE_EDGEAA
    IntersectionEpsilon = param.IntersectionEpsilon;
#endif
}

#ifdef SF_RENDER_ENABLE_EDGEAA
//-----------------------------------------------------------------------
void Tessellator::SetEdgeAAWidth(CoordType w) 
{ 
    EdgeAAWidth = w; 
    EdgeAAFlag = w > 0; 
}
#endif


//-----------------------------------------------------------------------
void Tessellator::SetFillRule(FillRuleType f) 
{ 
    if (f == FillStroker)
    {
        FillRule    = FillNonZero;
        StrokerMode = true;
    }
    else
    {
        FillRule    = f;
        StrokerMode = false;
    }
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpMonoChains(const MonoChainType* a, const MonoChainType* b)
{
    if (a->ySort != b->ySort) return a->ySort < b->ySort;
    if (a->xb != b->xb) return a->xb < b->xb;
    return a->xt < b->xt;
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpActiveChains(const MonoChainType* a, const MonoChainType* b)
{ 
    if(a->xb != b->xb) return a->xb < b->xb;
    return a->xt < b->xt;
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpIntersectionY(const IntersectionType& a, const IntersectionType& b)
{
    return a.y < b.y;
}

#ifdef SF_RENDER_ENABLE_EDGEAA
//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpEdgeAA(const TmpEdgeAAType& a, const TmpEdgeAAType& b)
{
    if (a.slope  != b.slope)  return a.slope  < b.slope;
    return a.style < b.style;
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpOuterEdges(const OuterEdgeType& a, const OuterEdgeType& b)
{
    if (a.edge->cntVer != b.edge->cntVer) return a.edge->cntVer < b.edge->cntVer;
    return a.edge->rayVer < b.edge->rayVer;
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpStrokerEdges(const StrokerEdgeType& a, const StrokerEdgeType& b)
{
    if(verIdx(a.node1) != verIdx(b.node1)) return verIdx(a.node1) < verIdx(b.node1);
    return verIdx(a.node2) < verIdx(b.node2);
}

//-----------------------------------------------------------------------
SF_INLINE bool 
Tessellator::cmpStrokerNode1(const StrokerEdgeType& e, unsigned node)
{
    return verIdx(e.node1) < verIdx(node);
}
#endif

//-----------------------------------------------------------------------
SF_INLINE bool Tessellator::forwardMin(int idx, int start) const
{
    CoordType y = SrcVertices[idx].y;
    if(idx <= start) return SrcVertices[idx+1].y > y;
    return SrcVertices[idx-1].y >= y && SrcVertices[idx+1].y > y;
}

//-----------------------------------------------------------------------
SF_INLINE bool Tessellator::reverseMin(int idx, int end) const
{
    CoordType y = SrcVertices[idx].y;
    if(idx >= end) return SrcVertices[idx-1].y > y;
    return SrcVertices[idx-1].y > y && SrcVertices[idx+1].y >= y;
}

//-----------------------------------------------------------------------
SF_INLINE bool Tessellator::forwardMax(int idx, int end) const
{
    if(idx >= end) return true;
    return SrcVertices[idx+1].y <= SrcVertices[idx].y;
}

//-----------------------------------------------------------------------
SF_INLINE bool Tessellator::reverseMax(int idx, int start) const
{
    if(idx <= start) return true;
    return SrcVertices[idx-1].y <= SrcVertices[idx].y;
}

//-----------------------------------------------------------------------
void Tessellator::buildEdgeList(unsigned start, unsigned numEdges, int step, 
                                unsigned leftStyle, unsigned rightStyle)
{
    EdgeType e;
    unsigned i;
    unsigned startEdgeIdx = (unsigned)Edges.GetSize();
    for(i = 0; i < numEdges; ++i)
    {
        e.lower = start;
        start  += step;
        const SrcVertexType& v1 = SrcVertices[e.lower];
        const SrcVertexType& v2 = SrcVertices[e.lower+step];
        e.slope = (v2.x - v1.x) / (v2.y - v1.y);
        Edges.PushBack(e);
    }
    MonoChainType mc;
    const SrcVertexType& v = SrcVertices[Edges[startEdgeIdx].lower];
    mc.edge        = startEdgeIdx;
    mc.end         = (unsigned)Edges.GetSize() - 1;
    mc.ySort       = v.y;
    mc.xb          = v.x;
    mc.xt          = Edges[startEdgeIdx].slope; // For initial sorting
    mc.dir         = (SInt16)step;
    mc.leftStyle   = (UInt16)leftStyle;
    mc.rightStyle  = (UInt16)rightStyle;
    mc.leftBelow   = 0;
    mc.leftAbove   = 0;
    mc.rightBelow  = 0;
    mc.rightAbove  = 0;
    mc.flags       = 0;
    mc.posScan     = 0;
    mc.posIntr     = ~0U;
    MonoChains.PushBack(mc);
}

//-----------------------------------------------------------------------
void Tessellator::addStyle(unsigned style, bool complex)
{
    if (style)
    {
        while (32 * ComplexFlags.GetSize() <= style)
        {
            ComplexFlags.Resize(ComplexFlags.GetSize() ? 2*ComplexFlags.GetSize() : 8);
        }

        if(complex)
        {
            ComplexFlags[style >> 5] |= 1 << (style & 31);
            HasComplexFill = true;
        }

        if (MaxStyle < style) 
        {
            MaxStyle = style;
        }
    }
}

//-----------------------------------------------------------------------
void Tessellator::ClosePath()
{
    if(SrcVertices.GetSize() > LastVertex + 2)
    {
        if (SrcVertices[LastVertex].x != SrcVertices.Back().x ||
            SrcVertices[LastVertex].y != SrcVertices.Back().y)
        {
            SrcVertices.PushBack(SrcVertices[LastVertex]);
        }
    }
}

//-----------------------------------------------------------------------
void Tessellator::FinalizePath(unsigned leftStyle, unsigned rightStyle, 
                               bool leftComplex, bool rightComplex)
{
    if(SrcVertices.GetSize() < LastVertex + 2 || leftStyle == rightStyle)
    {
        SrcVertices.CutAt(LastVertex);
        return;
    }

    if (StrokerMode)
    {
        // In the stroker mode we can discard all the paths that have
        // non-zero both, left and right fill styles.
        //------------------------
        if ((leftStyle == 0) == (rightStyle == 0))
        {
            SrcVertices.CutAt(LastVertex);
            return;
        }
        if (leftStyle)  leftStyle  = 1;
        if (rightStyle) rightStyle = 1;
        leftComplex = rightComplex = false;
    }

    addStyle(leftStyle,  leftComplex);
    addStyle(rightStyle, rightComplex);
    PathType path;
    path.start = LastVertex;
    path.end   = (unsigned)SrcVertices.GetSize() - 1;
    path.leftStyle  = leftStyle;
    path.rightStyle = rightStyle;
    Paths.PushBack(path);
    LastVertex = (unsigned)SrcVertices.GetSize();
}


//-----------------------------------------------------------------------
void Tessellator::GetSrcBounds(CoordType* x1, CoordType* y1, CoordType* x2, CoordType* y2) const
{
    CoordType tx1 = MaxCoord;
    CoordType ty1 = MaxCoord;
    CoordType tx2 = MinCoord;
    CoordType ty2 = MinCoord;
    for(UPInt i = 0; i < SrcVertices.GetSize(); ++i)
    {
        SrcVertexType v = SrcVertices[i];
        if(v.x < tx1) tx1 = v.x;
        if(v.y < ty1) ty1 = v.y;
        if(v.x > tx2) tx2 = v.x;
        if(v.y > ty2) ty2 = v.y;
    }
    *x1 = tx1; *y1 = ty1; *x2 = tx2; *y2 = ty2;
}

//-----------------------------------------------------------------------
void Tessellator::decomposePath(const PathType& path)
{
    int min, max;
    int numEdges;

    // Do the path forward pass
    for(min = path.start; min < (int)path.end; ++min)
    {
        // If a forward local minimum...
        if(forwardMin(min, path.start))
        {
            // Search for the next local maximum...
            numEdges = 1;
            max = min + 1;
            while(!forwardMax(max, path.end))
            {
                ++numEdges;
                ++max;
            }

            // Build the next edge list 
            buildEdgeList(min, numEdges, +1, path.leftStyle, path.rightStyle);
            min += numEdges - 1;
        }
    }

    // Do the path reverse pass
    for(min = path.end; min > (int)path.start; --min)
    {
        // If a reverse local minimum...
        if(reverseMin(min, path.end))
        {
            // Search for the previous local maximum...
            numEdges = 1;
            max = min - 1;
            while(!reverseMax(max, path.start))
            {
                ++numEdges;
                --max;
            }

            // Build the previous edge list
            buildEdgeList(min, numEdges, -1, path.leftStyle, path.rightStyle);
            min -= numEdges - 1;
        }
    }
}


//-----------------------------------------------------------------------
SF_INLINE void Tessellator::incStyles(const MonoChainType* mc)
{
    unsigned ls = mc->leftStyle;
    unsigned rs = mc->rightStyle;
    if(FillRule == FillNonZero)
    {
        StyleCounts[ls] += mc->dir;
        StyleCounts[rs] -= mc->dir;
    }
    else
    {
        StyleCounts[ls] ^= 1;
        StyleCounts[rs] ^= 1;
    }
}

//-----------------------------------------------------------------------
SF_INLINE UInt16 Tessellator::findElderStyle() const
{
    for(UPInt i = StyleCounts.GetSize(); i; --i)
    {
        if(StyleCounts[i-1]) return UInt16(i-1);
    }
    return 0;
}

//-----------------------------------------------------------------------
void Tessellator::perceiveStyles(const ChainPtrArray& aet)
{
    StyleCounts.Zero();
    UInt16 leftAbove = 0;
    for(UPInt i = 0; i < aet.GetSize(); ++i)
    {
        MonoChainType* mc = aet[i];
        mc->flags &= ~VisibleChain;
        if((mc->flags & EndChainFlag) == 0)
        {
            incStyles(mc);
            mc->rightAbove = findElderStyle();
            mc->leftAbove  = leftAbove;
            if(leftAbove != mc->rightAbove) 
            {
                mc->flags |= VisibleChain;
            }
            leftAbove = mc->rightAbove;
        }
    }
}

//-----------------------------------------------------------------------
void Tessellator::setupIntersections()
{
    InteriorChains.Clear();
    InteriorOrder.Clear();
    for(UPInt i = 0; i < ActiveChains.GetSize(); i++) 
    {
        ActiveChains[i]->posIntr = (unsigned)i;
        InteriorChains.PushBack(ActiveChains[i]);
        InteriorOrder.PushBack((unsigned)i);
    }
}

//-----------------------------------------------------------------------
SF_INLINE CoordType Tessellator::calcX(const EdgeType* edge, CoordType y) const
{
    const SrcVertexType& lower = SrcVertices[edge->lower];
    return lower.x + (y - lower.y) * edge->slope;
}

//-----------------------------------------------------------------------
unsigned Tessellator::nextScanbeam(CoordType yb, CoordType yt, 
                                   unsigned startMc, unsigned numMc)
{
    UPInt i, j;
    MonoChainType* mc;
    const SrcVertexType* lower;
    const SrcVertexType* upper;
    unsigned retFlags = 0;

    if(numMc) retFlags = InsertEdgesFlag;

    EdgeType* edge;
    ValidChains.Clear();
    for(i = 0; i < ActiveChains.GetSize(); ++i)
    {
        mc = ActiveChains[i];

        // Clear EventFlag, that is, assume the chain won't have 
        // an event point.
        //------------------------
        mc->flags &= ~EventFlag;

        edge = &Edges[mc->edge];
        if(SrcVertices[edge->lower+mc->dir].y == yb)
        {
            // There is an event point, that is, the yb scan-beam 
            // goes through the upper vertex of the edge
            //-----------------------
            if(mc->edge < mc->end)
            {
                // Next edge exists, the chain continues.
                // Switch to the next edge, initialize xb, calculate xt, 
                // and mark the chain as having an event point.
                //----------------
                edge     = &Edges[++mc->edge];
                lower    = &SrcVertices[edge->lower];
                upper    = &SrcVertices[edge->lower+mc->dir];
                mc->xb   = lower->x;
                mc->xt   = (upper->y == yt) ? upper->x : calcX(edge, yt);
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
            lower  = &SrcVertices[edge->lower];
            upper  = &SrcVertices[edge->lower+mc->dir];
            mc->xb = mc->xt;
            mc->xt = (upper->y == yt) ? upper->x : calcX(edge, yt);
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
            edge      = &Edges[mc->edge];
            lower     = &SrcVertices[edge->lower];
            upper     = &SrcVertices[edge->lower+mc->dir];
            mc->xb    = lower->x;
            mc->flags = EventFlag;
            mc->xt    = (upper->y == yt) ? upper->x : calcX(edge, yt);
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
    IntersectionType intr;
    for(i = 1; i < ValidChains.GetSize(); ++i)
    {
        SPInt jl;
        for(jl = i - 1; jl >= 0; --jl)
        {
            MonoChainType* mc1 = ActiveChains[ValidChains[jl]];
            MonoChainType* mc2 = ActiveChains[ValidChains[jl+1]];

            if(mc1->xt <= mc2->xt) break;

            if(Intersections.GetSize() == 0)
            {
                setupIntersections();
            }

            CoordType nom = mc1->xb - mc2->xb;
            CoordType den = mc2->xt - mc2->xb - mc1->xt + mc1->xb;

            intr.pos1 = mc1->posIntr;
            intr.pos2 = mc2->posIntr;
            intr.y    = (den == 0) ? yb : yb + height * nom / den;
            if(intr.y < yb)
            {
                intr.y = yb;
            }
            if(intr.y > yt)
            {
                intr.y = yt;
            }
            Intersections.PushBack(intr);
            Alg::Swap(ActiveChains[ValidChains[jl]], 
                      ActiveChains[ValidChains[jl+1]]);
        }
    }

    // If there are two or more intersections sort them
    //-------------------------
    if(Intersections.GetSize() > 1)
    {
        //Alg::QuickSort(Intersections, cmpIntersectionY);

        // Insertion sort works better because it's stable and does not
        // perform unnecessary swaps. In case there are two or more 
        // exactly coinciding intersection points it's highly desirable 
        // to keep the original order, while quick sort may swap equal 
        // elements. Since it's extremely unlikely that there are many
        // intersections within one slab, the performance of  
        // insertion sort is perfectly appropriate.
        //---------------------
        Alg::InsertionSort(Intersections, cmpIntersectionY);
        if (HasEpsilon)
        {
            CoordType y = yb;
            for (i = 0; i < Intersections.GetSize(); ++i)
            {
                if (Intersections[i].y - y < Epsilon * fabsf(y))
                    Intersections[i].y = y;
                y = Intersections[i].y;
            }
        }
    }

    return retFlags;
}

//-----------------------------------------------------------------------
SF_INLINE unsigned Tessellator::addEventVertex(const SrcVertexType& v1)
{
    if(v1.x > LastX)
    {
        TessVertex v2;
        v2.x = LastX = v1.x;
        v2.y = v1.y;
        v2.Idx = ~0U;
        v2.Styles[0] = v2.Styles[1] = 0xFFFFU;
        v2.Flags = TessStyleOpaque;
        v2.Mesh = 0;
        MeshVertices.PushBack(v2);
    }
    return (unsigned)MeshVertices.GetSize() - 1;
}

//-----------------------------------------------------------------------
unsigned Tessellator::addEventVertex(const MonoChainType* mc, 
                                     CoordType yb, bool enforceFlag)
{
    // (1) Check the condition if the left and right styles remain 
    //     the same and producing the vertex is not enforced. 
    //------------
    EdgeType* edge = &Edges[mc->edge];
    if(!enforceFlag && (mc->flags & HasIntersection) == 0 &&
        mc->leftBelow == mc->leftAbove && 
        mc->rightBelow == mc->rightAbove)
    {
        // (2) If the styles are the same, add the event vertex only 
        //     in case the current edge has changed (EventFlag) and its 
        //     lower vertex lies exactly on the scan-beam. The lastPool 
        //     condition is necessary to process self-intersections 
        //     correctly. When the horizontal band is divided into a 
        //     number of sub-bands in function processInterior(), 
        //     there should be a check for adding the same vertex 
        //     only once. The next sub-band will have a different 
        //     Y coordinate and the same vertex will be skipped.
        //     This branch is executed most frequently. Also only in
        //     this branch the function may return -1 ("No Event").
        //---------------
        if((mc->flags & EventFlag) != 0 &&
           SrcVertices[edge->lower].y == yb)
        {
            return addEventVertex(SrcVertices[edge->lower]);
        }
        else
        {
            return ~0U;
        }
    }
    else
    {
        // (3) The styles on the left and right are different, or 
        //     the vertex is enforced. It may also mean the edge
        //     intersects with another one. The algorithm does not 
        //     distinguish the cases when the edges do intersect, or
        //     the styles change for another reason (integrity violation).
        //     This generalization makes the algorithm extremely robust.
        //     Before calculating the intersection it's a good idea to 
        //     check for trivial cases, when the current scan-beam 
        //     coincides with the lower or upper edge vertex.
        //     This branch always returns a valid vertex, whether 
        //     a new or existing one.
        //--------------
        if(SrcVertices[edge->lower].y == yb) 
        {
            return addEventVertex(SrcVertices[edge->lower]);
        }

        if((mc->flags & EndChainFlag) != 0 && 
            SrcVertices[edge->lower+mc->dir].y == yb)
        {
            return addEventVertex(SrcVertices[edge->lower+mc->dir]);
        }

        // (4) The scan-bean differs from the lower or upper vertex,
        //     calculate the intersection point with function calcX().
        //     This code also checks for the same X coordinate with 
        //     certain Epsilon value. If the difference between the 
        //     lastPool X and the calculated X is within this epsilon, 
        //     the vertices merge.
        //--------------
        CoordType x = calcX(edge, yb);
        if(x - LastX > fabsf(yb) * Epsilon)
        {
            TessVertex v2;
            v2.x = LastX = x;
            v2.y = yb;
            v2.Idx = ~0U;
            v2.Styles[0] = v2.Styles[1] = 0xFFFFU;
            v2.Flags = TessStyleOpaque;
            v2.Mesh = 0;
            MeshVertices.PushBack(v2);
        }
    }
    return (unsigned)MeshVertices.GetSize() - 1;
}

//-----------------------------------------------------------------------
SF_INLINE void Tessellator::resetMonotone(MonotoneType* m, unsigned style)
{
    m->start            =  0;
    m->d.m.lastIdx   = ~0U;
    m->d.m.prevIdx1  = ~0U;
    m->d.m.prevIdx2  = ~0U;
    m->style            =  style;
    m->lowerBase        =  0;
}

//-----------------------------------------------------------------------
SF_INLINE void Tessellator::growMonotone(MonotoneType* m, unsigned vertex)
{
    MonoVertexType mv;
    mv.next   = 0;
    mv.srcVer = vertex;
#ifdef SF_RENDER_ENABLE_EDGEAA
    mv.aaVer  = vertex;
#endif
    if(m->start == 0)
    {
        MonoVertices.PushBack(mv);
        m->start    = &MonoVertices.Back();
        m->d.m.prevIdx2 = ~0U;
        m->d.m.prevIdx1 = ~0U;
        m->d.m.lastIdx  = (unsigned)MonoVertices.GetSize() - 1;
    }
    else
    {
        MonoVertexType& last = MonoVertices[m->d.m.lastIdx];
        if(last.srcVer != vertex)
        {
            MonoVertices.PushBack(mv);
            last.next   = &MonoVertices.Back();
            m->d.m.prevIdx2 = m->d.m.prevIdx1;
            m->d.m.prevIdx1 = m->d.m.lastIdx;
            m->d.m.lastIdx  = (unsigned)MonoVertices.GetSize() - 1;
        }
    }
}


//-----------------------------------------------------------------------
SF_INLINE unsigned Tessellator::lastMonoVertex(const MonotoneType* m) const
{
    return (m->d.m.lastIdx == ~0U) ? ~0U : MonoVertices[m->d.m.lastIdx].srcVer;
}

//-----------------------------------------------------------------------
void Tessellator::removeLastMonoVertex(MonotoneType* m)
{
    if(m->d.m.lastIdx != ~0U)
    {
        if(m->d.m.lastIdx == (unsigned)MonoVertices.GetSize() - 1) 
        {
            MonoVertices.PopBack();
        }
        m->d.m.lastIdx  = m->d.m.prevIdx1;
        m->d.m.prevIdx1 = m->d.m.prevIdx2;
        m->d.m.prevIdx2 = ~0U;
        if(m->d.m.lastIdx == ~0U)
        {
            m->start = 0;
        }
        else
        {
            MonoVertices[m->d.m.lastIdx].next = 0;
        }
    }
}



//-----------------------------------------------------------------------
SF_INLINE void Tessellator::growMonotone(MonotoneType* m, unsigned left, unsigned right)
{
    if(left  != ~0U) growMonotone(m, left  |  LeftFlag);
    if(right != ~0U) growMonotone(m, right & ~LeftFlag);
}


//-----------------------------------------------------------------------
SF_INLINE void Tessellator::growMonotone(ScanChainType* scan, unsigned vertex)
{
    if(scan && scan->monotone)
    {
        MonotoneType* m = scan->monotone;
        if(m->lowerBase)
        {
            if(m->lowerBase->y == MeshVertices[verIdx(vertex)].y)
            {
                m->lowerBase->vertexRight = verIdx(vertex);
            }
            else
            {
                if(vertex & LeftFlag) connectPendingToLeft (scan, vertex);
                else                  connectPendingToRight(scan, vertex);
            }
        }
        else
        {
            growMonotone(m, vertex);
        }
    }
}


//-----------------------------------------------------------------------
Tessellator::MonotoneType* Tessellator::startMonotone(unsigned style)
{
    MonotoneType m;
    resetMonotone(&m, style);
    Monotones.PushBack(m);
    return &Monotones.Back();
}

//-----------------------------------------------------------------------
void Tessellator::startMonotone(ScanChainType* scan, unsigned vertex)
{
    scan->monotone = 0;
    if(scan->chain->rightAbove)
    {
        scan->monotone = startMonotone(scan->chain->rightAbove);
        growMonotone(scan, vertex);
    }
}

//-----------------------------------------------------------------------
void Tessellator::replaceMonotone(ScanChainType* scan, unsigned style)
{
    if(style)
    {
        if(scan->monotone == 0)
        {
            scan->monotone = startMonotone(style);
            return;
        }
        if(scan->monotone->style == style || 
           scan->monotone->start == 0)
        {
            scan->monotone->style = style;
            return;
        }
        MonotoneType* monotone = startMonotone(style);
        *monotone = *scan->monotone;
        resetMonotone(scan->monotone, style);
    }
}

//-----------------------------------------------------------------------
void Tessellator::replaceMonotone(PendingEndType* pe, unsigned style)
{
    if(style)
    {
        if(pe->monotone == 0)
        {
            pe->monotone = startMonotone(style);
            return;
        }
        if(pe->monotone->style == style || 
           pe->monotone->start == 0)
        {
            pe->monotone->style = style;
            return;
        }
        MonotoneType* monotone = startMonotone(style);
        *monotone = *pe->monotone;
        resetMonotone(pe->monotone, style);
    }
}

//-----------------------------------------------------------------------
void Tessellator::addPendingEnd(ScanChainType* dst, 
                                ScanChainType* pending, 
                                CoordType  y)
{
    if(dst && dst->monotone && dst->monotone->style)
    {
        MonotoneType* m = dst->monotone;
        if(m->lowerBase == 0)
        {
            BaseLineType lowerBase;
            lowerBase.y           =  y;
            lowerBase.styleLeft   =  pending->chain->leftBelow;
            lowerBase.vertexLeft  =  dst->vertex;
            lowerBase.vertexRight = ~0U;
            lowerBase.firstChain  =  (unsigned)PendingEnds.GetSize();
            lowerBase.numChains   =  0;
            lowerBase.leftAbove   = ~0U;
            BaseLines.PushBack(lowerBase);
            m->lowerBase = &BaseLines.Back();
        }
        PendingEndType pe;
        pe.vertex   = pending->vertex;
        pe.monotone = pending->monotone;
        PendingEnds.PushBack(pe);
        ++m->lowerBase->numChains;
    }
}

//-----------------------------------------------------------------------
template<class ElementsArray> struct TessBaseLineIterator
{
    typedef typename ElementsArray::ValueType ScanChainType;

    template<class BaseLineType> 
    TessBaseLineIterator(ElementsArray& e,
                         BaseLineType* bl,
                         ScanChainType* scanLeft) :
        Elements(e),
        Index(bl->firstChain),
        Num(bl->numChains),
        VertexRightmost(bl->vertexRight),
        Chain(scanLeft),
        VertexLeft(bl->vertexLeft),
        VertexRight(e[bl->firstChain].vertex),
        Style(bl->styleLeft),
        FlagFirst(true)
    {}

    template<class StyleFunction> bool Next(StyleFunction style)
    {
        FlagFirst = false;
        if(Num)
        {
            --Num;
            VertexLeft  = VertexRight;
            Chain       = &Elements[Index++];
            VertexRight = Num ? Elements[Index].vertex : VertexRightmost;
            Style       = style(Chain);
            return true;
        }
        return false;
    }

    bool IsFirst() const { return FlagFirst; }
    bool IsLast()  const { return Num == 0; }

    ElementsArray& Elements;
    unsigned       Index;
    unsigned       Num;
    unsigned       VertexRightmost;
    ScanChainType* Chain;
    unsigned       VertexLeft;
    unsigned       VertexRight;
    unsigned       Style;
    bool           FlagFirst;

private:
    void operator = (const TessBaseLineIterator<ElementsArray>&);
};

//-----------------------------------------------------------------------
void Tessellator::connectPendingToLeft(ScanChainType* scan, unsigned targetVertex)
{
    BaseLineType* lowerBase = scan->monotone->lowerBase;
    unsigned styleAbove = scan->monotone->style;

    scan->monotone->lowerBase = 0;

    PendingEndType scanLeft;
    scanLeft.vertex   = lowerBase->vertexLeft;
    scanLeft.monotone = scan->monotone;

    TessBaseLineIterator<PendingEndArray> it(PendingEnds, lowerBase, &scanLeft);
    do
    {
        if(it.VertexLeft != it.VertexRight)
        {
            if(it.IsFirst())
            {
                growMonotone(scan->monotone, it.VertexRight);
                growMonotone(scan->monotone, targetVertex, targetVertex);
            }
            else
            {
                if(it.Style != styleAbove || it.Chain->monotone == 0)
                {
                    it.Chain->monotone = startMonotone(styleAbove);
                    growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
                }
                if(!it.IsLast())
                {
                    growMonotone(it.Chain->monotone, targetVertex, targetVertex);
                }
                else
                {
                    scan->monotone = it.Chain->monotone;
                    growMonotone(scan->monotone, targetVertex | LeftFlag);
                }
            }
        }
    }
    while(it.Next(pendingMonotoneStyle));

    if(lowerBase == &BaseLines.Back())
    {
        PendingEnds.CutAt(lowerBase->firstChain);
        BaseLines.PopBack();
    }
}

//-----------------------------------------------------------------------
void Tessellator::connectPendingToRight(ScanChainType* scan, unsigned targetVertex)
{
    BaseLineType* lowerBase = scan->monotone->lowerBase;
    unsigned styleAbove = scan->monotone->style;

    scan->monotone->lowerBase = 0;

    PendingEndType scanLeft;
    scanLeft.vertex   = lowerBase->vertexLeft;
    scanLeft.monotone = scan->monotone;

    TessBaseLineIterator<PendingEndArray> it(PendingEnds, lowerBase, &scanLeft);
    growMonotone(scan->monotone, it.VertexRight);
    growMonotone(scan->monotone, targetVertex);
    while(it.Next(pendingMonotoneStyle))
    {
        if(it.VertexLeft != it.VertexRight)
        {
            if(it.Style != styleAbove || it.Chain->monotone == 0)
            {
                it.Chain->monotone = startMonotone(styleAbove);
                growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
            }
            growMonotone(it.Chain->monotone, targetVertex, targetVertex);
        }
    }
    if(lowerBase == &BaseLines.Back())
    {
        PendingEnds.CutAt(lowerBase->firstChain);
        BaseLines.PopBack();
    }
}

//-----------------------------------------------------------------------
void Tessellator::connectStartingToLeft(ScanChainType* scan, 
                                        BaseLineType* upperBase, 
                                        unsigned targetVertex)
{
    ScanChainType* leftAbove = (upperBase->leftAbove == ~0U) ? 
                                scan :
                               &ChainsAbove[upperBase->leftAbove];

    TessBaseLineIterator<ScanChainArray> it(ChainsAbove, upperBase, scan);
    unsigned styleBelow = scan->monotone->style;
    MonotoneType* monotone = startMonotone(0);
    *monotone = *scan->monotone;
    resetMonotone(scan->monotone, styleBelow);

    do
    {
        if(!it.IsLast())
        {
            if(it.VertexLeft != it.VertexRight)
            {
                replaceMonotone(it.Chain, styleBelow);
                growMonotone(it.Chain->monotone, targetVertex, targetVertex);
                growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
            }
        }
        else
        {
            it.Chain->monotone = monotone;
            growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
        }
        if(it.Style != styleBelow || it.Chain->monotone == 0)
        {
            if(!it.Style)
            {
                it.Chain->monotone = 0;
            }
            else
            {
                if(it.IsFirst())
                {
                    it.Chain = leftAbove;
                }
                replaceMonotone(it.Chain, it.Style);
                growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
            }
        }
    }
    while(it.Next(startingMonotoneStyle));
    upperBase->numChains = 0;
}

//-----------------------------------------------------------------------
void Tessellator::connectStartingToRight(ScanChainType* scan, 
                                         BaseLineType* upperBase, 
                                         unsigned targetVertex)
{
    ScanChainType* leftAbove = (upperBase->leftAbove == ~0U) ? 
                                scan :
                               &ChainsAbove[upperBase->leftAbove];

    TessBaseLineIterator<ScanChainArray> it(ChainsAbove, upperBase, scan);
    unsigned styleBelow = scan->monotone->style;

    do
    {
        if(it.IsFirst())
        {
            growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
        }
        else
        {
            if(it.VertexLeft != it.VertexRight)
            {
                replaceMonotone(it.Chain, styleBelow);
                growMonotone(it.Chain->monotone, targetVertex, targetVertex);
                growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
            }
        }
        if(it.Style != styleBelow || it.Chain->monotone == 0)
        {
            if(!it.Style)
            {
                it.Chain->monotone = 0;
            }
            else
            {
                if(it.IsFirst())
                {
                    it.Chain = leftAbove;
                }
                replaceMonotone(it.Chain, it.Style);
                growMonotone(it.Chain->monotone, it.VertexLeft, it.VertexRight);
            }
        }
    }
    while(it.Next(startingMonotoneStyle));
    upperBase->numChains = 0;
}

//-----------------------------------------------------------------------
void Tessellator::connectStartingToPending(ScanChainType* scan, 
                                           BaseLineType* upperBase)
{
    BaseLineType* lowerBase = scan->monotone->lowerBase;
    unsigned styleBetween = scan->monotone->style;

    scan->monotone->lowerBase = 0;

    PendingEndType scanLeft;
    scanLeft.vertex   = lowerBase->vertexLeft;
    scanLeft.monotone = scan->monotone;

    TessBaseLineIterator<PendingEndArray> itLower(PendingEnds, lowerBase, &scanLeft);
    TessBaseLineIterator<ScanChainArray>  itUpper(ChainsAbove, upperBase, scan);
    unsigned lowerTarget = ~0U;
    unsigned stopNum = itLower.Num < itUpper.Num;

    for(;;)
    {
        bool invalid = 
            (itLower.VertexLeft == itLower.VertexRight &&
            (itUpper.VertexLeft == ~0U || itUpper.VertexRight == ~0U)) ||
            (itUpper.VertexLeft == itUpper.VertexRight &&
            (itLower.VertexLeft == ~0U || itLower.VertexRight == ~0U));

        if(!invalid && 
           (itLower.VertexLeft != itLower.VertexRight ||
            itUpper.VertexLeft != itUpper.VertexRight))
        {
            if(itLower.Style != styleBetween)
            {
                replaceMonotone(itLower.Chain, styleBetween);
                growMonotone(itLower.Chain->monotone, 
                             itLower.VertexLeft, 
                             itLower.VertexRight);
            }
            growMonotone(itLower.Chain->monotone, 
                         itUpper.VertexLeft, 
                         itUpper.VertexRight);
            itUpper.Chain->monotone = itLower.Chain->monotone;
        }
        if(itUpper.Style != styleBetween || itUpper.Chain->monotone == 0)
        {
            if(!itUpper.Style)
            {
                itUpper.Chain->monotone = 0;
            }
            else
            {
                replaceMonotone(itUpper.Chain, itUpper.Style);
                growMonotone(itUpper.Chain->monotone, 
                             itUpper.VertexLeft, 
                             itUpper.VertexRight);
            }
        }
        if(itLower.Num == stopNum) 
        {
            lowerTarget = itLower.VertexLeft;
            if(itLower.VertexRight != ~0U) 
            {
                lowerTarget = itLower.VertexRight;
            }
            break;
        }
        itLower.Next(pendingMonotoneStyle);
        itUpper.Next(startingMonotoneStyle);
    }

    if(itUpper.Num && lowerTarget != ~0U)
    {
        MonotoneType* monotone = 0;
        itLower.Next(pendingMonotoneStyle);
        itUpper.Next(startingMonotoneStyle);

        if(itLower.Chain->monotone && 
           itLower.Chain->monotone->style == styleBetween)
        {
            monotone = startMonotone(styleBetween);
            *monotone = *itLower.Chain->monotone;
            resetMonotone(itLower.Chain->monotone, styleBetween);
        }

        do
        {
            if(itUpper.IsLast())
            {
                itUpper.Chain->monotone = monotone;
                if(itUpper.Chain->monotone == 0)
                {
                    itUpper.Chain->monotone = startMonotone(styleBetween);
                    growMonotone(itUpper.Chain->monotone, 
                                 itLower.VertexLeft, 
                                 itLower.VertexRight);
                }
                growMonotone(itUpper.Chain->monotone, 
                             itUpper.VertexLeft, 
                             itUpper.VertexRight);
            }
            else
            {
                if(itUpper.VertexLeft != itUpper.VertexRight)
                {
                    replaceMonotone(itUpper.Chain, styleBetween);
                    growMonotone(itUpper.Chain->monotone, lowerTarget, lowerTarget);
                    growMonotone(itUpper.Chain->monotone, 
                                 itUpper.VertexLeft, 
                                 itUpper.VertexRight);
                }
            }
            if(itUpper.Style != styleBetween || itUpper.Chain->monotone == 0)
            {
                if(!itUpper.Style)
                {
                    itUpper.Chain->monotone = 0;
                }
                else
                {
                    replaceMonotone(itUpper.Chain, itUpper.Style);
                    growMonotone(itUpper.Chain->monotone, 
                                 itUpper.VertexLeft, 
                                 itUpper.VertexRight);
                }
            }
        }
        while(itUpper.Next(startingMonotoneStyle));
    }

    if(lowerBase == &BaseLines.Back())
    {
        PendingEnds.CutAt(lowerBase->firstChain);
        BaseLines.PopBack();
    }
    upperBase->numChains = 0;
}

//-----------------------------------------------------------------------
void Tessellator::connectStarting(ScanChainType* scan, 
                                  BaseLineType* upperBase)
{
    if(scan && scan->monotone)
    {
        unsigned i;
        unsigned targetVertex = lastMonoVertex(scan->monotone);

        // Retrieve the latest vertex below yb, removing vertices 
        // from the monotone vertex list. There can be at most 
        // two vertices that lie on the current scan line.
        //--------------------
        upperBase->vertexLeft  = ~0U;
        upperBase->vertexRight = ~0U;
        for(i = 0; i < 2; i++)
        {
            if(targetVertex == ~0U ||
               MeshVertices[verIdx(targetVertex)].y < upperBase->y)
            {
                break;
            }
            if(targetVertex & LeftFlag)
            {
                upperBase->vertexLeft  = verIdx(targetVertex);
            }
            else 
            {
                upperBase->vertexRight = targetVertex;
            }
            removeLastMonoVertex(scan->monotone);
            targetVertex = lastMonoVertex(scan->monotone);
        }

        if(scan->monotone->lowerBase)
        {
            connectStartingToPending(scan, upperBase);
            return;
        }

        if(targetVertex == ~0U)
        {
            // This should never happen but may, in some cases of broken 
            // data integrity. It means that there is no vertex to connect 
            // with the starting chains. In this case we just restore 
            // one of the retrieved vertices and continue processing.
            // Most probably degenerate monotones will appear (no harm).
            //--------------------
            if(upperBase->vertexRight != ~0U)
            {
                targetVertex = upperBase->vertexRight;
                upperBase->vertexRight = ~0U;
                growMonotone(scan->monotone, targetVertex);
            }
            else
            if(upperBase->vertexLeft != ~0U)
            {
                targetVertex = upperBase->vertexLeft;
                upperBase->vertexLeft = ~0U;
                growMonotone(scan->monotone, targetVertex);
            }
        }

        if(targetVertex & LeftFlag)
        {
            connectStartingToLeft(scan, upperBase, verIdx(targetVertex));
        }
        else
        {
            connectStartingToRight(scan, upperBase, targetVertex);
        }
    }
    upperBase->numChains = 0;
}








//-----------------------------------------------------------------------
SF_INLINE unsigned 
Tessellator::nextChainInBundle(unsigned below, unsigned above, unsigned vertex) const
{
    const ScanChainType* thisBelow = 0;
    const ScanChainType* thisAbove = 0;
    if(below < ChainsBelow.GetSize()) thisBelow = &ChainsBelow[below];
    if(above < ChainsAbove.GetSize()) thisAbove = &ChainsAbove[above];
    if(thisBelow && 
       thisAbove && 
       thisBelow->vertex == thisAbove->vertex)
    {
        return 0;
    }
    if(thisAbove && thisAbove->vertex == vertex)
    {
        return ChainStartsAtScanline;
    }
    if(thisBelow && thisBelow->vertex == vertex)
    {
        return ChainEndsAtScanline;
    }
    return 0;
}

//-----------------------------------------------------------------------
void Tessellator::sweepScanbeam(const ChainPtrArray& aet, CoordType yb)
{
    UPInt i;
    MonoChainType* mc;

    LastX = MinCoord;
    ChainsAbove.Clear();
    for(i = 0; i < aet.GetSize(); ++i)
    {
        mc = aet[i];
        mc->posScan = (unsigned)i;
        if(mc->flags & VisibleChain) 
        {
            ScanChainType scan;
            scan.chain    =  mc;
            scan.monotone =  0;
            scan.vertex   = ~0U;
            ChainsAbove.PushBack(scan);
        }
    }

    unsigned below = 0;
    unsigned above = 0;
    unsigned prevAbove = ~0U;
    unsigned vertex;

    ScanChainType* thisBelow;
    ScanChainType* thisAbove;
    ScanChainType* leftBelow = 0;
    ScanChainType* leftAbove = 0;
    BaseLineType   upperBase;

    upperBase.y         = yb;
    upperBase.numChains =  0;

    for(;;)
    {
        if(below < ChainsBelow.GetSize() && above < ChainsAbove.GetSize())
        {
            thisBelow = &ChainsBelow[below];
            thisAbove = &ChainsAbove[above];
            if(thisAbove->chain->posScan == thisBelow->chain->posScan)
            {
                thisBelow->vertex = 
                thisAbove->vertex = addEventVertex(thisAbove->chain, yb, false);
                ++below;
                ++above;
            }
            else
            if(thisAbove->chain->posScan < thisBelow->chain->posScan)
            {
                thisAbove->vertex = addEventVertex(thisAbove->chain, yb, true);
                ++above;
            }
            else
            {
                thisBelow->vertex = addEventVertex(thisBelow->chain, yb, true);
                ++below;
            }
        }
        else
        if(above < ChainsAbove.GetSize())
        {
            thisAbove = &ChainsAbove[above];
            thisAbove->vertex = addEventVertex(thisAbove->chain, yb, true);
            ++above;
        }
        else
        if(below < ChainsBelow.GetSize())
        {
            thisBelow = &ChainsBelow[below];
            thisBelow->vertex = addEventVertex(thisBelow->chain, yb, true);
            ++below;
        }
        else
        {
            break;
        }
    }


    below = 0;
    above = 0;
    for(;;)
    {
        if(below < ChainsBelow.GetSize() && above < ChainsAbove.GetSize())
        {
            thisBelow = &ChainsBelow[below];
            thisAbove = &ChainsAbove[above];

            // Define the processing branch
            //----------------
            unsigned chainFlag = ChainContinuesAtScanline;
            if(thisBelow->vertex != thisAbove->vertex)
            {
                chainFlag = 
                   (thisAbove->chain->posScan  < 
                    thisBelow->chain->posScan) ?
                        ChainStartsAtScanline : ChainEndsAtScanline;
            }

            switch(chainFlag)
            {
            case ChainContinuesAtScanline:              // Above  |
                                                        // Below  | continue chain
                vertex = thisBelow->vertex;
                if(vertex != ~0U)
                {
                    // Vertex at the continuing chain exists. 
                    // Grow the monotone polygon on the left of the chain.
                    // Most probably it is the same monotone, but may be
                    // a different one in case of intersecting with a 
                    // horizontal edge. In case the monotone polygon is the 
                    // same, function growMonotone() will filter out the latest 
                    // vertex.
                    //------------------
                    growMonotone(leftBelow, vertex);
                    growMonotone(leftAbove, vertex);

                    // If the upper base line contains starting chains,
                    // connect them with the latest visible vertex.
                    //------------------
                    if(upperBase.numChains)
                    {
                        connectStarting(leftBelow, &upperBase);
                    }

                    // Process a bundle, eliminating all degenerate cases.
                    //------------------
                    while((chainFlag = nextChainInBundle(below+1, above+1, vertex)) != 0)
                    {
                        if(chainFlag == ChainStartsAtScanline)
                        {
                            leftAbove = thisAbove;
                            thisAbove = &ChainsAbove[++above];
                            startMonotone(leftAbove, vertex | LeftFlag);
                            growMonotone(leftAbove, vertex);
                        }
                        else
                        {
                            leftBelow = thisBelow;
                            thisBelow = &ChainsBelow[++below];
                            growMonotone(leftBelow, vertex | LeftFlag);
                            growMonotone(leftBelow, vertex);
                        }
                    }

                    // Grow the monotone polygon on the right of the chain
                    // and if the style does not change, grow the polygon
                    // on the left and transfer it from thisBelow to thisAbove.
                    // In case the monotone polygon is the same function 
                    // growMonotone() will filter out the latest vertex,
                    // but it is necessary for correct processing of some 
                    // degenerate cases.
                    //---------------------
                    growMonotone(thisBelow, vertex | LeftFlag);
                    if(thisBelow->chain->rightBelow == thisAbove->chain->rightAbove)
                    {
                        growMonotone(leftAbove, vertex);
                        thisAbove->monotone = thisBelow->monotone;
                    }
                    else
                    {
                        // If the style changes start new monotone polygon.
                        //---------------------
                        startMonotone(thisAbove, vertex | LeftFlag);
                    }
                }
                else
                {
                    // The chain does not have a vertex at this scan-beam,
                    // but it is still necessary to check for the starting chains
                    // in this trapezoid. If there are any, connect them.
                    // Also, transfer the monotone polygon from thisBelow to 
                    // thisAbove.
                    //--------------------
                    if(upperBase.numChains)
                    {
                        connectStarting(leftBelow, &upperBase);
                    }
                    thisAbove->monotone = thisBelow->monotone;
                }

                prevAbove = above;
                leftBelow = thisBelow;
                leftAbove = thisAbove;
                ++below;
                ++above;
                break;
            
            case ChainStartsAtScanline:                 // Above  |  
                                                        // Below    | start chain (split)
                if(leftBelow && 
                   leftBelow->monotone && 
                   leftBelow->monotone->style)
                {
                    if(upperBase.numChains)
                    {
                        ++upperBase.numChains;
                    }
                    else
                    {
                        upperBase.styleLeft  = thisAbove->chain->leftAbove;
                        upperBase.firstChain = above;
                        upperBase.numChains  = 1;
                        upperBase.leftAbove  = prevAbove;
                    }
                    if(leftAbove &&
                       leftAbove->monotone &&
                       leftAbove->monotone->lowerBase &&
                       leftAbove->monotone->lowerBase->y == yb)
                    {
                        leftAbove->monotone->lowerBase->vertexRight = thisAbove->vertex;
                    }
                }
                else
                {
                    growMonotone(leftAbove, thisAbove->vertex);
                    startMonotone(thisAbove, thisAbove->vertex | LeftFlag);
                }
                leftAbove = thisAbove;
                ++above;
                break;

            case ChainEndsAtScanline:                   // Above    |
                                                        // Below  |   end chain (merge)
                growMonotone(leftBelow, thisBelow->vertex);
                growMonotone(thisBelow, thisBelow->vertex | LeftFlag);
                if(upperBase.numChains)
                {
                    connectStarting(leftBelow, &upperBase);
                }
                addPendingEnd(leftAbove, thisBelow, yb);
                prevAbove = ~0U;
                leftBelow = thisBelow;
                ++below;
                break;
            }
        }
        else
        if(above < ChainsAbove.GetSize())               // Above    | 
        {                                               // Below  |   start chain
            thisAbove = &ChainsAbove[above];
            growMonotone(leftAbove, thisAbove->vertex);
            startMonotone(thisAbove, thisAbove->vertex | LeftFlag);
            leftAbove = thisAbove;
            ++above;
        }
        else
        if(below < ChainsBelow.GetSize())               // Above  | 
        {                                               // Below    | end chain
            thisBelow = &ChainsBelow[below];
            growMonotone(leftBelow, thisBelow->vertex);
            growMonotone(thisBelow, thisBelow->vertex | LeftFlag);
            if(upperBase.numChains)
            {
                connectStarting(leftBelow, &upperBase);
            }
            addPendingEnd(leftAbove, thisBelow, yb);
            prevAbove = ~0U;
            leftBelow = thisBelow;
            ++below;
        }
        else
        {
            break;
        }
    }

    ChainsBelow.Clear();
    for(i = 0; i < ChainsAbove.GetSize(); ++i)
    {
        ChainsBelow.PushBack(ChainsAbove[i]);
    }
    for(i = 0; i < aet.GetSize(); ++i)
    {
        mc = aet[i];
        mc->leftBelow  = mc->leftAbove;
        mc->rightBelow = mc->rightAbove;
        mc->flags &= ~HasIntersection;
    }
}


//-----------------------------------------------------------------------
void Tessellator::swapChains(unsigned startIn, unsigned endIn)
{
    unsigned i;
    for(i = startIn; i < endIn; i++)
    {
        const IntersectionType& in = Intersections[i];
        unsigned i1 = InteriorOrder[in.pos1];
        unsigned i2 = InteriorOrder[in.pos2];
        InteriorChains[i1]->flags |= HasIntersection;
        InteriorChains[i2]->flags |= HasIntersection;
        Alg::Swap(InteriorChains[i1], InteriorChains[i2]);
        Alg::Swap(InteriorOrder[in.pos1], InteriorOrder[in.pos2]);
    }
}

//-----------------------------------------------------------------------
void Tessellator::processInterior(CoordType yb, CoordType yTop, unsigned perceiveFlag)
{
    unsigned startIn = 0;
    unsigned endIn   = 0;
    const IntersectionType* in;
    CoordType yt = yb;
    
    // Skip all intersections at yb
    while(endIn < Intersections.GetSize())
    {
        in = &Intersections[endIn];
        yt = in->y;
        if(yt > yb) break;
        perceiveFlag = 1;
        ++endIn;
    }
    swapChains(startIn, endIn);

    if(perceiveFlag) 
    {
        perceiveStyles(InteriorChains);
    }

    while(endIn < Intersections.GetSize())
    {
        CoordType yt2 = yt;
        startIn = endIn;
        while(endIn < Intersections.GetSize())
        {
            in = &Intersections[endIn];
            yt2 = in->y;
            if(yt2 > yt) break;
            ++endIn;
        }
        perceiveStyles(InteriorChains);
        sweepScanbeam(InteriorChains, yb);
        swapChains(startIn, endIn);

        yb = yt;
        yt = yt2;
    }

    perceiveStyles(ActiveChains);

    if(yt < yTop)
    {
        sweepScanbeam(ActiveChains, yt);
    }
}


// Monotonization phase
//-----------------------------------------------------------------------
void Tessellator::monotonize()
{
    UPInt i, j;
    if(SrcVertices.GetSize() == 0) return;

    StyleCounts.Resize(MaxStyle + 1);

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
        decomposePath(Paths[i]);

    // Fill MonoChainsSorted array and sort it by ySort
    //------------------
    MonoChainsSorted.Resize(MonoChains.GetSize());
    for(i = 0; i < MonoChains.GetSize(); ++i)
        MonoChainsSorted[i] = &MonoChains[i];

    Alg::QuickSort(MonoChainsSorted, cmpMonoChains);

    unsigned  sb = 0; // Scan beam index
    unsigned  mc = 0; // Monotone Chain index
    CoordType yb, yt;

    yb = yt = SrcVertices[Scanbeams[0]].y;

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

        unsigned flags = nextScanbeam(yb, yt, startMc, mc - startMc);

        if(Intersections.GetSize())
        {
            processInterior(yb, yt, flags);
        }
        else
        {
            if(flags) 
            {
                perceiveStyles(ActiveChains);
            }
            sweepScanbeam(ActiveChains, yb);
        }

        if(flags & RemoveEdgesFlag)
        {
            UPInt ik, pos;
            for(ik = pos = 0; ik < ActiveChains.GetSize(); ++ik)
            {
                MonoChainType* mec = ActiveChains[ik];
                if((mec->flags & EndChainFlag) == 0)
                {
                    ActiveChains[pos++] = mec;
                }
            }
            ActiveChains.CutAt(pos);
        }
        yb = yt; // Advance yb
    }
}



//-----------------------------------------------------------------------
unsigned Tessellator::setMesh(unsigned style)
{
    if (!HasComplexFill)
        return 0;

    if (StyleMatrix.ValueAt(style, style) == 0xFFFFU)
    {
        UInt32 cf = ComplexFlags[style >> 5] & (1 << (style & 31));
        if (cf)
        {
            StyleMatrix.At(style, style) = (UInt16)Meshes.GetSize();
            TessMesh mesh;
            mesh.MeshIdx = (unsigned)Meshes.GetSize();
            mesh.Style1  = 
            mesh.Style2  = style;
            mesh.Flags1  = 
            mesh.Flags2  = cf ? TessStyleComplex : 0;
            mesh.VertexCount = 0;
            mesh.StartVertex = 0;
            Meshes.PushBack(mesh);
            MeshTriangles.AddArray();
        }
        else
        {
            StyleMatrix.At(style, style) = 0;
        }
    }
    return StyleMatrix.ValueAt(style, style);
}


//-----------------------------------------------------------------------
SF_INLINE UInt32 Tessellator::isComplex(unsigned a, unsigned b) const
{
    return (ComplexFlags[a >> 5] & (1 << (a & 31)))|
           (ComplexFlags[b >> 5] & (1 << (b & 31)));
}

//-----------------------------------------------------------------------
unsigned Tessellator::setMesh(unsigned style1, unsigned style2)
{
    if (!HasComplexFill)
        return 0;

    if (StyleMatrix.ValueAt(style1, style2) == 0xFFFFU)
    {
        UInt32 cf1 = ComplexFlags[style1 >> 5] & (1 << (style1 & 31));
        UInt32 cf2 = ComplexFlags[style2 >> 5] & (1 << (style2 & 31));

        if (cf1 | cf2)
        {
            if (cf1) cf1 = TessStyleComplex;
            if (cf2) cf2 = TessStyleComplex;

            if (cf1 == 0)
            {
                Alg::Swap(style1, style2);
                Alg::Swap(cf1, cf2);
            }

            UInt16 meshIdx = (UInt16)Meshes.GetSize();
            UInt16 meshSt1 = StyleMatrix.ValueAt(style1, style1);

            if (cf2 == 0 && 
                meshSt1 != 0xFFFF && 
                TessStyleIsComplex(Meshes[meshSt1].Flags2) == 0)
            {
                // Only Style1 is complex; Style2 is solid. 
                // We can merge this pair (Style1, Style2) with meshSt1.
                //-----------------------
                SF_ASSERT(Meshes[meshSt1].Style1 == style1);
                meshIdx = meshSt1;
            }
            else
            {
                // Both styles are complex, or it's the first request for
                // merging complex->solid.
                //-----------------------
                TessMesh mesh;
                mesh.MeshIdx = (unsigned)Meshes.GetSize();
                mesh.Style1  =       style1;
                mesh.Style2  = cf2 ? style2:0;
                mesh.Flags1  = cf1;
                mesh.Flags2  = cf2;
                mesh.VertexCount = 0;
                mesh.StartVertex = 0;
                Meshes.PushBack(mesh);
                MeshTriangles.AddArray();
            }
            StyleMatrix.At(style1, style2) = meshIdx;
            StyleMatrix.At(style2, style1) = meshIdx;

            if (InvasiveMerge)
            {
                if (StyleMatrix.ValueAt(style1, style1) == 0xFFFFU)
                    StyleMatrix.At(style1, style1) = meshIdx;

                if (StyleMatrix.ValueAt(style2, style2) == 0xFFFFU)
                    StyleMatrix.At(style2, style2) = meshIdx;
            }
            else
            {
                if (StyleMatrix.ValueAt(style1, style1) == 0xFFFFU && cf1 && !cf2)
                    StyleMatrix.At(style1, style1) = meshIdx;
            }
        }
        else
        {
            StyleMatrix.At(style1, style1) = 0;
            StyleMatrix.At(style2, style2) = 0;
            StyleMatrix.At(style1, style2) = 0;
            StyleMatrix.At(style2, style1) = 0;
        }
    }
    return StyleMatrix.ValueAt(style1, style2);
}


//-----------------------------------------------------------------------
void Tessellator::GetMesh(unsigned meshIdx, TessMesh* mesh) const
{
    *mesh = Meshes[meshIdx];
    mesh->StartVertex = 0;
}

//-----------------------------------------------------------------------
unsigned Tessellator::GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const
{
    unsigned i = 0;
    for (i = 0; 
         i < num && mesh->StartVertex < MeshVertices.GetSize(); 
         ++mesh->StartVertex)
    {
        if (MeshVertices[mesh->StartVertex].Mesh == mesh->MeshIdx)
        {
            vertices[i++] = MeshVertices[mesh->StartVertex];
        }
    }
    return i;
}

//-----------------------------------------------------------------------
void Tessellator::GetTrianglesI16(unsigned meshIdx, UInt16* idx, unsigned start, unsigned num) const
{
    for(unsigned j = 0; j < num; ++j)
    {
        // TO DO: possible access optimization (MeshTriangles jagged array)
        const TriangleType& t = MeshTriangles.At(meshIdx, start+j);
        *idx++ = (UInt16)MeshVertices[t.d.t.v1].Idx;
        *idx++ = (UInt16)MeshVertices[t.d.t.v2].Idx;
        *idx++ = (UInt16)MeshVertices[t.d.t.v3].Idx;
    }
}

//-----------------------------------------------------------------------
unsigned Tessellator::emitVertex(unsigned meshIdx, unsigned ver, unsigned style, unsigned flags)
{
    ver = verIdx(ver);
    TessVertex* v1 = &MeshVertices[ver];
    if (v1->Idx == ~0U)
    {
        v1->Idx       = ver;
        v1->Styles[0] = 
        v1->Styles[1] = UInt16(style);
        v1->Flags     = UInt16(flags);
        v1->Mesh      = UInt16(meshIdx);
        return ver;
    }

    if (v1->Mesh == meshIdx && v1->Styles[0] == style)
    {
        return ver;
    }

    // Search in index list for next use.
    //-----------------
    while (v1->Idx != ver)
    {
        ver = v1->Idx;
        v1  = &MeshVertices[ver];
        if (v1->Mesh == meshIdx && v1->Styles[0] == style)
        {
            return ver;
        }
    }

    ver = v1->Idx = (unsigned)MeshVertices.GetSize();

    TessVertex v2 = *v1;
    v2.Idx       = ver;
    v2.Styles[0] = 
    v2.Styles[1] = UInt16(style);
    v2.Flags     = UInt16(flags);
    v2.Mesh      = UInt16(meshIdx);
    MeshVertices.PushBack(v2);
    return ver;
}


//-----------------------------------------------------------------------
void Tessellator::SortTrianglesByVertex1(unsigned meshIdx)
{
    TriangleArray::ArrayAdaptor adaptor = MeshTriangles.GetArrayAdaptor(meshIdx);
    CmpVer1 cmpVer1(MeshVertices);
    Alg::QuickSort(adaptor, cmpVer1);
}


//-----------------------------------------------------------------------
void Tessellator::splitMesh(TessMesh* mesh)
{
    UPInt i, j, n;
    unsigned limit = VertexLimit - VertexLimit/4; // Allow 25% for vertex copies
    unsigned newMeshes = (mesh->VertexCount + limit - 1) / limit;
    unsigned triangles = (unsigned)MeshTriangles.GetSize(mesh->MeshIdx) / newMeshes;
    unsigned triIdx    = mesh->MeshIdx;

    SortTrianglesByVertex1(triIdx);

    n = MeshTriangles.GetSize(mesh->MeshIdx);
    for (i = 0; i < n; ++i)
    {
        TriangleType& tri = MeshTriangles.At(triIdx, i);
        MeshVertices[tri.d.t.v1].Mesh = 0xFFFF;
        MeshVertices[tri.d.t.v2].Mesh = 0xFFFF;
        MeshVertices[tri.d.t.v3].Mesh = 0xFFFF;
    }

    for (i = 1; i < newMeshes; ++i)
    {
        if (!MeshTriangles.Split(triIdx, triangles))
            break;

        Meshes[triIdx].VertexCount = ~0U;
        TessMesh newMesh = Meshes[triIdx];
        newMesh.MeshIdx  = (unsigned)Meshes.GetSize();
        Meshes.PushBack(newMesh);
        triIdx = newMesh.MeshIdx;
    }

    for (i = 0; i < Meshes.GetSize(); ++i)
    {
        TessMesh& m = Meshes[i];
        if (m.VertexCount == ~0U)
        {
            n = MeshTriangles.GetSize(i);
            for (j = 0; j < n; ++j)
            {
                TriangleType& tri = MeshTriangles.At(i, j);
                TessVertex& v1 = MeshVertices[tri.d.t.v1];
                TessVertex& v2 = MeshVertices[tri.d.t.v2];
                TessVertex& v3 = MeshVertices[tri.d.t.v3];
                if (v1.Mesh == 0xFFFF) v1.Mesh = (UInt16)i;
                if (v2.Mesh == 0xFFFF) v2.Mesh = (UInt16)i;
                if (v3.Mesh == 0xFFFF) v3.Mesh = (UInt16)i;
                if (v1.Mesh != i) 
                {
                    tri.d.t.v1 = (unsigned)MeshVertices.GetSize();
                    MeshVertices.PushBack(v1);
                    MeshVertices.Back().Mesh = (UInt16)i;
                }
                if (v2.Mesh != i) 
                {
                    tri.d.t.v2 = (unsigned)MeshVertices.GetSize();
                    MeshVertices.PushBack(v2);
                    MeshVertices.Back().Mesh = (UInt16)i;
                }
                if (v3.Mesh != i) 
                {
                    tri.d.t.v3 = (unsigned)MeshVertices.GetSize();
                    MeshVertices.PushBack(v3);
                    MeshVertices.Back().Mesh = (UInt16)i;
                }
            }
        }
    }
}


//-----------------------------------------------------------------------
void Tessellator::SplitMeshes()
{
    for (;;)
    {
        bool done = true;
        UPInt i;
        UPInt n = Meshes.GetSize();
        for (i = 0; i < n; ++i)
        {
            if (Meshes[i].VertexCount > VertexLimit)
            {
                splitMesh(&Meshes[i]);
                done = false;
            }
        }
        if (done)
            break;

        for (i = 0; i < Meshes.GetSize(); ++i) Meshes[i].VertexCount = 0;
        for (i = 0; i < MeshVertices.GetSize(); ++i)
        {
            TessVertex& v = MeshVertices[i];
            if (v.Mesh != 0xFFFF) v.Idx = Meshes[v.Mesh].VertexCount++;
        }
    }
}




#ifdef SF_RENDER_ENABLE_EDGEAA

//-----------------------------------------------------------------------
unsigned Tessellator::emitVertex(unsigned meshIdx, unsigned ver, unsigned style1, unsigned style2, 
                                 unsigned flags, bool strictStyle)
{
    ver = verIdx(ver);
    TessVertex* v1 = &MeshVertices[ver];
    if (v1->Idx == ~0U)
    {
        v1->Idx       = ver;
        v1->Styles[0] = UInt16(style1);
        v1->Styles[1] = UInt16(style2);
        v1->Flags     = UInt16(flags);
        v1->Mesh      = UInt16(meshIdx);
        return ver;
    }

    if (v1->Mesh == meshIdx && 
        v1->Styles[0] == style1 && 
        v1->Styles[1] == style2 &&
        (!strictStyle || v1->Flags == flags))
    {
        return ver;
    }

    // Search in index list for next use.
    //-----------------
    while (v1->Idx != ver)
    {
        ver = v1->Idx;
        v1  = &MeshVertices[ver];
        if (v1->Mesh == meshIdx && 
            v1->Styles[0] == style1 && 
            v1->Styles[1] == style2 &&
            (!strictStyle || v1->Flags == flags))
        {
            return ver;
        }
    }

    ver = v1->Idx = (unsigned)MeshVertices.GetSize();

    TessVertex v2 = *v1;
    v2.Idx       = ver;
    v2.Styles[0] = UInt16(style1);
    v2.Styles[1] = UInt16(style2);
    v2.Flags     = UInt16(flags);
    v2.Mesh      = UInt16(meshIdx);
    MeshVertices.PushBack(v2);
    return ver;
}

//-----------------------------------------------------------------------
void Tessellator::collectFanEdges(const ChainVertexArray& chain, 
                                  const ChainVertexArray& oppos,
                                  unsigned style)
{
    UPInt i;

    MonoVertexType* prevVer = oppos.GetSize() ? oppos[0] : chain.Back();
    MonoVertexType* thisVer = chain[0];

    for(i = 0; i < chain.GetSize(); ++i)
    {
        MonoVertexType* nextVer = 
            (i+1 < chain.GetSize()) ? 
                chain[i+1] : (oppos.GetSize() ? oppos.Back() : chain[0]);

        TessVertex* cntVer = &MeshVertices[verIdx(thisVer->srcVer)];
        EdgeAAType* edgeAA = &EdgeFans[cntVer->Idx + cntVer->Mesh];
        edgeAA->cntVer = thisVer;
        edgeAA->rayVer = prevVer;
        edgeAA->slope  = 0;
        edgeAA->style  = UInt16(style);

        ++edgeAA;

        edgeAA->cntVer = thisVer;
        edgeAA->rayVer = nextVer;
        edgeAA->slope  = 0;
        edgeAA->style  = UInt16(style ^ 0x8000U);

        cntVer->Mesh += 2;
        prevVer = thisVer;
        thisVer = nextVer;
    }
}


//-----------------------------------------------------------------------
unsigned Tessellator::countFanEdges(MonotoneType* m)
{
    unsigned totalEdges = 0;
    MonoVertexType* prevVer = m->start;
    MonoVertexType* thisVer = prevVer ? prevVer->next : 0;
    if (thisVer == 0) 
    {
        m->start = 0;
        return 0;
    }
    MonoVertexType* nextVer = thisVer->next;

    // Skip all insignificant monotone vertices (the ones that 
    // will produce no triangles). It's necessary to preserve 
    // proper EdgeAA offset calculation in all cases.
    //---------------------
    while(nextVer)
    {
        unsigned v1 = verIdx(prevVer->srcVer);
        unsigned v2 = verIdx(thisVer->srcVer);
        unsigned v3 = verIdx(nextVer->srcVer);
        if (v1 != v2 && v2 != v3 && v3 != v1)
        {
            m->start = prevVer;
            break;
        }
        prevVer = thisVer;
        thisVer = nextVer;
        nextVer = nextVer->next;
    }

    if (nextVer == 0)
    {
        m->start = 0; // Monotone is empty
        return 0;
    }

    prevVer = 0;
    thisVer = m->start;
    while(thisVer)
    {
        // Ignore last srcVer in case if it coincides with the previous one.
        //---------------------
        if(prevVer && verIdx(prevVer->srcVer) == verIdx(thisVer->srcVer))
        {
            if (thisVer->next && thisVer->next->next)
            {
                // This condition means that the monotone continues and
                // has a touching point prevVer <--> thisVer. We split
                // the monotone at this point to correctly process EdgeAA.
                //---------------------
                MonotoneType m2 = *m;
                m2.start = thisVer;
                Monotones.PushBack(m2);
            }
            prevVer->next = 0;
            break;
        }
        SF_ASSERT(MeshVertices[verIdx(thisVer->srcVer)].Mesh < 0xFFFF-2);
        MeshVertices[verIdx(thisVer->srcVer)].Mesh += 2;
        totalEdges += 2;
        prevVer = thisVer;
        thisVer = thisVer->next;
    }
    return totalEdges;
}

//-----------------------------------------------------------------------
void Tessellator::collectFanEdges(MonotoneType* m)
{
    LeftChain.Clear();
    RightChain.Clear();

    MonoVertexType* thisVer = m->start;
    while(thisVer)
    {
        if(isLeft(thisVer->srcVer))
        {
            LeftChain.PushBack(thisVer);
        }
        else
        {
            RightChain.PushBack(thisVer);
        }
        thisVer = thisVer->next;
    }

    if (LeftChain.GetSize())
    {
        collectFanEdges(LeftChain, RightChain, m->style | 0x8000U);
    }

    if (RightChain.GetSize())
    {
        collectFanEdges(RightChain, LeftChain, m->style);
    }
}


//-----------------------------------------------------------------------
bool Tessellator::computeMiter(const TessVertex& v1,
                               const TessVertex& v2,
                               const TessVertex& v3,
                               TessVertex* newVer1,
                               TessVertex* newVer2)
{
    CoordType xi = v2.x;
    CoordType yi = v2.y;

    CoordType len1 = Math2D::Distance(v1, v2);
    CoordType len2 = Math2D::Distance(v2, v3);
    CoordType turn = Math2D::TurnRatio(v1, v2, v3, len1, len2);
    bool      left = turn < 0;
    CoordType epsilon = (len1 + len2) * IntersectionEpsilon;
    CoordType dx1 = EdgeAAWidth * (v2.y - v1.y) / len1;
    CoordType dy1 = EdgeAAWidth * (v1.x - v2.x) / len1;
    CoordType dx2 = EdgeAAWidth * (v3.y - v2.y) / len2;
    CoordType dy2 = EdgeAAWidth * (v2.x - v3.x) / len2;
    bool      bevel = false;

    turn = fabsf(turn);

    if (turn < 0.125f)
    {
        if(len1 > len2) { xi = v2.x + dx1; yi = v2.y + dy1; }
        else            { xi = v2.x + dx2; yi = v2.y + dy2; }
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
            if (left)
            {
                lim = EdgeAAWidth * 4.0f;
            }
            else
            {
                lim = ((len1 < len2) ? len1 : len2) / turn;
            }
            if(d1 > lim)
            {
                //// Version with bevel only on the outer joins. Generates
                //// less overlaps, but may lose outer AA in some cases.
                ////----------------------
                //if (left && newVer2)
                //{
                //    xi         = v2.x + dx1 - 2*dy1;
                //    yi         = v2.y + dy1 + 2*dx1;
                //    newVer2->x = v2.x + dx2 + 2*dy2;
                //    newVer2->y = v2.y + dy2 - 2*dx2;
                //    bevel = true;
                //}
                //else
                //{
                //    lim /= d1;
                //    xi = v2.x + (xi - v2.x) * lim;
                //    yi = v2.y + (yi - v2.y) * lim;
                //}


                // Version with bevel on all joins. Generates more
                // overlapping triangles, but more consistent outer AA.
                //----------------------
                if (newVer2)
                {
                    CoordType k = left ? 2.0f : 0.0f;
                    xi          = v2.x + dx1 - k*dy1;
                    yi          = v2.y + dy1 + k*dx1;
                    newVer2->x  = v2.x + dx2 + k*dy2;
                    newVer2->y  = v2.y + dy2 - k*dx2;
                    bevel = true;
                }
                else
                {
                    lim /= d1;
                    xi = v2.x + (xi - v2.x) * lim;
                    yi = v2.y + (yi - v2.y) * lim;
                }
            }
        }
        else
        {
            xi = v2.x;
            yi = v2.y;
        }
    }
    newVer1->x = xi;
    newVer1->y = yi;
    return bevel;
}

//-----------------------------------------------------------------------
void Tessellator::processFan(unsigned start, unsigned end)
{
    if (start == end) return;

    UPInt    i;
    unsigned i1;
    unsigned i2;
    unsigned nv = end - start;
    unsigned ev;
    TessVertex v1;
    EdgeAAType* e1;
    EdgeAAType* e2;

    i1 = end-1;
    StartFan.Clear();
    EndFan.Clear();
    for(i2 = start; i2 < end; ++i2)
    {
        e1 = &EdgeFans[i1];
        e2 = &EdgeFans[i2];
        if (verIdx(e1->rayVer->srcVer) == verIdx(e2->rayVer->srcVer))
        {
            if (styleIdx(e1->style) != styleIdx(e2->style))
            {
                StartFan.PushBack(i2);
                EndFan.PushBack(i1);
            }
        }
        else
        {
            if (e1->slope != e2->slope && 
                !styleIsLeft(e1->style) && styleIsLeft(e2->style))
            {
                StartFan.PushBack(i2);
                EndFan.PushBack(i1);
            }
        }
        i1 = i2;
    }

    if (StartFan.GetSize() == 0)
    {
        // In certain degenerate cases all the rays may have the same style.
        // Another case is incorrect order of the rays due to floating point 
        // numerical instability when calculating the "slopes".
        // If it happens EdgeAA won't help anyways, so that, we just
        // duplicate the reference cntVer for the entire fan and return.
        //-----------------------
        i2      = verIdx(EdgeFans[start].cntVer->srcVer);
        v1.x    = MeshVertices[i2].x;
        v1.y    = MeshVertices[i2].y;
        v1.Idx  = ~0U;
        v1.Styles[0] =
        v1.Styles[1] = styleIdx(EdgeFans[start].style);
        v1.Flags= TessStyleOpaque;
        v1.Mesh = 0xFFFFU;
        for(i = start; i < end; ++i)
        {
            EdgeFans[i].cntVer->aaVer = (unsigned)MeshVertices.GetSize();
        }
        MeshVertices.PushBack(v1);
        return;
    }

    EndFan.PushBack(EndFan[0]); // Copy first one

    unsigned endVer = (unsigned)StarVertices.GetSize();
    StarVertexType sv;
    sv.cntVer = verIdx(EdgeFans[start].cntVer->srcVer);

    for(i = 0; i < StartFan.GetSize(); ++i)
    {
        i1 = StartFan[i];
        i2 = EndFan[i+1];
        e1 = &EdgeFans[i1];
        e2 = &EdgeFans[i2];

        v1.Styles[0] = v1.Styles[1] = styleIdx(e1->style);
        if (verIdx(e1->rayVer->srcVer) != verIdx(e1->cntVer->srcVer) && 
            verIdx(e2->rayVer->srcVer) != verIdx(e1->cntVer->srcVer))
        {
            computeMiter(MeshVertices[verIdx(e1->rayVer->srcVer)], 
                         MeshVertices[verIdx(e1->cntVer->srcVer)],
                         MeshVertices[verIdx(e2->rayVer->srcVer)],
                         &v1, 0);
        }
        else
        {
            v1.x = MeshVertices[verIdx(e1->cntVer->srcVer)].x;
            v1.y = MeshVertices[verIdx(e1->cntVer->srcVer)].y;
        }

        ev = (unsigned)MeshVertices.GetSize();
        if(i2 < i1) i2 += nv;
        for(; i1 <= i2; ++i1)
        {
            EdgeFans[(i1 < end) ? i1 : i1 - nv].cntVer->aaVer = ev;
        }
        v1.Idx     = ~0U;
        v1.Styles[0] = 
        v1.Styles[1] = styleIdx(e1->style);
        v1.Flags   = TessStyleOpaque;
        v1.Mesh    = 0xFFFFU;
        sv.starVer = ev;
        MeshVertices.PushBack(v1);
        StarVertices.PushBack(sv);

        e2 = &EdgeFans[EndFan[i]];
        if(verIdx(e1->rayVer->srcVer) == verIdx(e2->rayVer->srcVer))
        {
            // Inner Quadrilaterals
            //---------------------
            if ((e2->rayVer->aaVer & VisitedFlag) == 0)
            {
                InnerQuadType q;
                q.e1 = e1;
                q.e2 = e2;
                InnerQuads.PushBack(q);
                e1->cntVer->aaVer |= VisitedFlag;
            }
        }
        else
        {
            // Outer Edges
            //---------------------
            TessVertex v2;
            bool bevel = false;

            if (verIdx(e1->rayVer->srcVer) != verIdx(e1->cntVer->srcVer) && 
                verIdx(e2->rayVer->srcVer) != verIdx(e1->cntVer->srcVer))
            {
                bevel = computeMiter(MeshVertices[verIdx(e2->rayVer->srcVer)], 
                                     MeshVertices[verIdx(e1->cntVer->srcVer)],
                                     MeshVertices[verIdx(e1->rayVer->srcVer)],
                                     &v1, &v2);
            }
            else
            {
                v1.x = MeshVertices[verIdx(e1->cntVer->srcVer)].x;
                v1.y = MeshVertices[verIdx(e1->cntVer->srcVer)].y;
            }

            unsigned vi1 = (unsigned)MeshVertices.GetSize();
            unsigned vi2 = vi1;
            v1.Idx       = 
            v2.Idx       = ~0U;
            v1.Styles[0] =
            v1.Styles[1] = styleIdx(e2->style);
            v2.Styles[0] =
            v2.Styles[1] = styleIdx(e1->style);
            v1.Flags     = 
            v2.Flags     = 0;
            v1.Mesh      = 
            v2.Mesh      = 0xFFFFU;

            if (v1.Styles[0] != v2.Styles[0])
            {
                setMesh(v1.Styles[0], v2.Styles[1]);
            }

            MeshVertices.PushBack(v1);
            if (bevel)
            {
                vi2 = (unsigned)MeshVertices.GetSize();
                MeshVertices.PushBack(v2);
            }

            // If there is an outer vertex (vertices), we have to insert it before
            // the last one, since the last vertex belongs to the 
            // filled area and it is supposed to be processed one step later.
            //----------------------
            StarVertices.PushBack(StarVertices.Back());
            if (bevel)
            {
                StarVertices.PushBack(StarVertices.Back());
                StarVertices[StarVertices.GetSize() - 3].starVer = vi1;
                StarVertices[StarVertices.GetSize() - 2].starVer = vi2;
            }
            else
            {
                StarVertices[StarVertices.GetSize() - 2].starVer = vi1;
            }

            OuterEdgeType oe;
            oe.edge   = e1;
            oe.outVer = vi2;
            OuterEdges.PushBack(oe);
            oe.edge   = e2;
            oe.outVer = vi1 | VisitedFlag;
            OuterEdges.PushBack(oe);
        }
    }

    if (endVer+3 > StarVertices.GetSize())
    {
        StarVertices.CutAt(endVer);
    }
}


//-----------------------------------------------------------------------
void Tessellator::emitTriangles()
{
    unsigned i;
    TriangleType tri;
    unsigned meshIdx;
    unsigned style;
    unsigned flags;
    unsigned s1=0, s2=0, s3=0, m1=0, m2=0, m3=0, f1=0, f2=0, f3=0;

    // Monotone Triangles
    //-----------------------
    for (i = 0; i < Monotones.GetSize(); ++i)
    {
        const MonotoneType& m = Monotones[i];
        if (m.d.t.numTriangles)
        {
            meshIdx = m.d.t.meshIdx;
            flags = TessStyleOpaque | 
                ((m.style == Meshes[meshIdx].Style1) ? TessStyleFactorOne : 0);
            for (unsigned j = 0; j < m.d.t.numTriangles; ++j)
            {
                TriangleType& trit = MeshTriangles.At(meshIdx, m.d.t.startTriangle+j);
                trit.d.m.v1->aaVer = emitVertex(meshIdx, trit.d.m.v1->aaVer, m.style, flags);
                trit.d.m.v2->aaVer = emitVertex(meshIdx, trit.d.m.v2->aaVer, m.style, flags);
                trit.d.m.v3->aaVer = emitVertex(meshIdx, trit.d.m.v3->aaVer, m.style, flags);
                trit.d.t.v1 = verIdx(trit.d.m.v1->aaVer);
                trit.d.t.v2 = verIdx(trit.d.m.v2->aaVer);
                trit.d.t.v3 = verIdx(trit.d.m.v3->aaVer);
            }
        }
    }

    // Inner Quadrilaterals
    //-----------------------
    for (i = 0; i < InnerQuads.GetSize(); ++i)
    {
        EdgeAAType* e1  = InnerQuads[i].e1;
        EdgeAAType* e2  = InnerQuads[i].e2;

        s1 = styleIdx(e1->style);
        s2 = styleIdx(e2->style);

        meshIdx = setMesh(s1, s2);

        const TessMesh& mesh = Meshes[meshIdx];

        if (mesh.Style1 && s1 != mesh.Style1)
        {
            Alg::Swap(e1, e2);
            Alg::Swap(s1, s2);
        }

        MonoVertexType* v1 = e1->cntVer;
        MonoVertexType* v2 = e1->rayVer;
        MonoVertexType* v3 = e2->cntVer;
        MonoVertexType* v4 = e2->rayVer;

        unsigned su3 = s1;
        unsigned fu3 = 0;

        if (TessStyleIsComplex(mesh.Flags1 ^ mesh.Flags2))
        {
            su3 = s2;
            fu3 = TessStyleUseColor2;
        }

        v1->aaVer = emitVertex(meshIdx, v1->aaVer, s1, su3, TessStyleOpaque | TessStyleFactorOne | fu3);
        v2->aaVer = emitVertex(meshIdx, v2->aaVer, s1, su3, TessStyleOpaque | TessStyleFactorOne | fu3);
        v3->aaVer = emitVertex(meshIdx, v3->aaVer, s2, TessStyleOpaque);
        v4->aaVer = emitVertex(meshIdx, v4->aaVer, s2, TessStyleOpaque);

        tri.d.t.v1 = verIdx(v1->aaVer);
        tri.d.t.v2 = verIdx(v2->aaVer);
        tri.d.t.v3 = verIdx(v3->aaVer);
        MeshTriangles.PushBack(meshIdx, tri);
        tri.d.t.v1 = verIdx(v4->aaVer);
        tri.d.t.v2 = verIdx(v3->aaVer);
        tri.d.t.v3 = verIdx(v2->aaVer);
        MeshTriangles.PushBack(meshIdx, tri);
    }

    // Outer Quadrilaterals
    //-----------------------
    Alg::QuickSort(OuterEdges, cmpOuterEdges);
    for (i = 0; i < OuterEdges.GetSize(); ++i)
    {
        OuterEdgeType* e1 = &OuterEdges[i];
        if ((e1->outVer & VisitedFlag) == 0)
        {
            EdgeAAType edgeAA;
            edgeAA.cntVer = e1->edge->rayVer;
            edgeAA.rayVer = e1->edge->cntVer;
            edgeAA.slope  = 0;
            edgeAA.style  = 0;

            OuterEdgeType oppos;
            oppos.edge = &edgeAA;
            oppos.outVer = ~0U;

            unsigned idx = (unsigned)Alg::LowerBound(OuterEdges, oppos, cmpOuterEdges);
            if (idx < OuterEdges.GetSize())
            {
                OuterEdgeType* e2 = &OuterEdges[idx];
                if (e2->edge->cntVer == e1->edge->rayVer)
                {
                    meshIdx = setMesh(style = styleIdx(e1->edge->style));
                    flags   = (style == Meshes[meshIdx].Style1) ? TessStyleFactorOne : 0;

                    MonoVertexType* mv1 = e1->edge->cntVer;
                    MonoVertexType* mv2 = e1->edge->rayVer;
                    unsigned i1 = emitVertex(meshIdx, mv1->aaVer, style, flags | TessStyleOpaque);
                    unsigned i2 = emitVertex(meshIdx, mv2->aaVer, style, flags | TessStyleOpaque);
                    e1->outVer  = emitVertex(meshIdx, e1->outVer, style, flags) | VisitedFlag;
                    e2->outVer  = emitVertex(meshIdx, e2->outVer, style, flags) | VisitedFlag;
                    unsigned i3 = verIdx(e1->outVer);
                    unsigned i4 = verIdx(e2->outVer);

                    tri.d.t.v1 = i1;
                    tri.d.t.v2 = i2;
                    tri.d.t.v3 = i3;
                    MeshTriangles.PushBack(meshIdx, tri);
                    tri.d.t.v1 = i4;
                    tri.d.t.v2 = i3;
                    tri.d.t.v3 = i2;
                    MeshTriangles.PushBack(meshIdx, tri);
                }
            }
        }
    }

    // Star Triangles
    //-----------------------
    for(i = 0; i < StarVertices.GetSize(); )
    {
        unsigned cntVer = StarVertices[i].cntVer;
        unsigned endStar = i;
        while(++endStar < StarVertices.GetSize() && 
              cntVer == StarVertices[endStar].cntVer) {}

        meshIdx = ~0U;
        if(i+3 == endStar)
        {
            m1 = StarVertices[i+0].starVer;
            m2 = StarVertices[i+1].starVer;
            m3 = StarVertices[i+2].starVer;
            s1 = MeshVertices[m1].Styles[0];
            s2 = MeshVertices[m2].Styles[0];
            s3 = MeshVertices[m3].Styles[0];
            f1 = MeshVertices[m1].Flags;
            f2 = MeshVertices[m2].Flags;
            f3 = MeshVertices[m3].Flags;
            m1 = setMesh(s1, s2); 
            m2 = setMesh(s2, s3); 
            m3 = setMesh(s3, s1); 

            if (m1 == m2 || m1 == m3)
            {
                meshIdx = m1;
            }
            else
            if (m2 == m3)
            {
                meshIdx = m2;
            }
        }

        if (meshIdx != ~0U)
        {
            m1 = StarVertices[i+0].starVer;
            m2 = StarVertices[i+1].starVer;
            m3 = StarVertices[i+2].starVer;
            tri.d.t.v1 = tri.d.t.v2 = tri.d.t.v3 = ~0U;

            // If one of the styles is complex, but not both...
            //--------------------
            if (TessStyleIsComplex(Meshes[meshIdx].Flags1 ^ Meshes[meshIdx].Flags2))
            {
                unsigned c1 = ComplexFlags[s1 >> 5] & (1 << (s1 & 31));
                unsigned c2 = ComplexFlags[s2 >> 5] & (1 << (s2 & 31));
                unsigned c3 = ComplexFlags[s3 >> 5] & (1 << (s3 & 31));
                if (c1) tri.d.t.v1 = emitVertex(meshIdx, m1, s1, c2 ? s3 : s2, f1 | TessStyleUseColor2);
                if (c2) tri.d.t.v2 = emitVertex(meshIdx, m2, s2, c3 ? s1 : s3, f2 | TessStyleUseColor2);
                if (c3) tri.d.t.v3 = emitVertex(meshIdx, m3, s3, c1 ? s2 : s1, f3 | TessStyleUseColor2);
            }
            if (tri.d.t.v1 == ~0U) tri.d.t.v1 = emitVertex(meshIdx, m1, s1, f1);
            if (tri.d.t.v2 == ~0U) tri.d.t.v2 = emitVertex(meshIdx, m2, s2, f2);
            if (tri.d.t.v3 == ~0U) tri.d.t.v3 = emitVertex(meshIdx, m3, s3, f3);
            MeshTriangles.PushBack(meshIdx, tri);
        }
        else
        {
            unsigned i1 = endStar - 1;
            unsigned i2 = i;

            for (; i2 < endStar; ++i2)
            {
                m1 = StarVertices[i1].starVer;
                m2 = StarVertices[i2].starVer;

                s1 = MeshVertices[m1].Styles[0];
                s2 = MeshVertices[m2].Styles[0];
                f1 = MeshVertices[m1].Flags & TessStyleOpaque;
                f2 = MeshVertices[m2].Flags & TessStyleOpaque;

                meshIdx = setMesh(s1, s2);
                const TessMesh& mesh = Meshes[meshIdx];

                if (mesh.Style1 && s1 != mesh.Style1)
                {
                    Alg::Swap(m1, m2);
                    Alg::Swap(s1, s2);
                    Alg::Swap(f1, f2);
                }

                s3 = s1;
                f3 = f1 | TessStyleFactorOne;
                if (TessStyleIsComplex(mesh.Flags1 ^ mesh.Flags2))
                {
                    s3  = s2;
                    f3 |= TessStyleUseColor2;
                }

                tri.d.t.v1 = emitVertex(meshIdx, m1, s1, s3, f3);
                tri.d.t.v2 = emitVertex(meshIdx, m2, s2, f2);

                f3 = ((f1 & f2) & TessStyleOpaque) ? TessStyleOpaque : TessStyleTranslucent;
                
                if (!TessStyleIsComplex(mesh.Flags1 | mesh.Flags2))
                {
                    s3  = s2;
                    f3 |= TessStyleMixColors;
                }
                else
                {
                    f3 |=  TessStyleFactorHalf | TessStyleUseColor2;
                    f3 &= ~TessStyleFactorOne;
                }
                tri.d.t.v3 = emitVertex(meshIdx, cntVer, s1, s3, f3, true);
                MeshTriangles.PushBack(meshIdx, tri);
                i1 = i2;
            }
        }
        i = endStar;
    }
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3, class V4>
SF_INLINE static bool TessIntersection(const V1& v1, const V2& v2,
                                       const V3& v3, const V4& v4,
                                       CoordType* x, CoordType* y,
                                       CoordType epsilon)
{
    CoordType nom = (v1.y-v3.y) * (v4.x-v3.x) - (v1.x-v3.x) * (v4.y-v3.y);
    CoordType den = (v2.x-v1.x) * (v4.y-v3.y) - (v2.y-v1.y) * (v4.x-v3.x);

    if(fabs(den) < epsilon) 
    {
        return false; 
    }

    CoordType k = nom / den;

    // The intersection must be within the segment (v1.x,v1.y) <-> (v2.x,v2.y)
    //-------------------
    if(k <= 0 || k >= 1)
    {
        return false;
    }
    *x = v1.x + k * (v2.x-v1.x);
    *y = v1.y + k * (v2.y-v1.y);

    return true;
}


//-----------------------------------------------------------------------
void Tessellator::moveVertexAA(const TessVertex& refVer, TessVertex* aaVer, 
                               const TessVertex& v2, const TessVertex& v3)
{
    CoordType x, y;
    CoordType epsilon = (fabsf(refVer.x - aaVer->x) + 
                         fabsf(refVer.y - aaVer->y) + 
                         fabsf(v2.x     - v3.x) + 
                         fabsf(v2.y     - v3.y)) * IntersectionEpsilon;

    if(TessIntersection(refVer, *aaVer, v2, v3, &x, &y, epsilon))
    {
        aaVer->x = x + (refVer.x - x) * 0.125f;
        aaVer->y = y + (refVer.y - y) * 0.125f;
    }
    else
    {
        aaVer->x = refVer.x;
        aaVer->y = refVer.y;
    }
}

//-----------------------------------------------------------------------
void Tessellator::addTriangleAA(MonoVertexType* v1,
                                MonoVertexType* v2,
                                MonoVertexType* v3,
                                CoordType cp)
{
#ifdef SF_RENDER_ENABLE_MOUNTAINS
    SF_UNUSED(cp);
#else
    // Old Monotone Triangulation
    //--------------------
    if(cp == 0)
    {
        return;
    }
    
    if (cp > 0)
    {
        Alg::Swap(v2, v3);
    }
#endif

    TriangleType tri;
    if (EdgeAAFlag)
    {
        const TessVertex& refV1 = MeshVertices[verIdx(v1->srcVer)];
        const TessVertex& refV2 = MeshVertices[verIdx(v2->srcVer)];
        const TessVertex& refV3 = MeshVertices[verIdx(v3->srcVer)];
        TessVertex* aaV1 = &MeshVertices[verIdx(v1->aaVer)];
        TessVertex* aaV2 = &MeshVertices[verIdx(v2->aaVer)];
        TessVertex* aaV3 = &MeshVertices[verIdx(v3->aaVer)];

        // TO DO: Subject to improve
        //------------------
        if (Math2D::CrossProduct(refV2, refV3, *aaV1) >= 0)
            moveVertexAA(refV1, aaV1, refV2, refV3);

        if (Math2D::CrossProduct(refV3, refV1, *aaV2) >= 0)
            moveVertexAA(refV2, aaV2, refV3, refV1);

        if (Math2D::CrossProduct(refV1, refV2, *aaV3) >= 0)
            moveVertexAA(refV3, aaV3, refV1, refV2);

        tri.d.m.v1 = v1;
        tri.d.m.v2 = v2;
        tri.d.m.v3 = v3;
    }
    else
    {
        tri.d.t.v1 = emitVertex(MeshIdx, verIdx(v1->srcVer), MonoStyle, FactorOneFlag);
        tri.d.t.v2 = emitVertex(MeshIdx, verIdx(v2->srcVer), MonoStyle, FactorOneFlag);
        tri.d.t.v3 = emitVertex(MeshIdx, verIdx(v3->srcVer), MonoStyle, FactorOneFlag);
    }
    MeshTriangles.PushBack(MeshIdx, tri);
}



#ifdef SF_RENDER_ENABLE_MOUNTAINS
//-----------------------------------------------------------------------
SF_INLINE CoordType 
Tessellator::triangleRatio(MonoVertexType* m1, MonoVertexType* m2, MonoVertexType* m3) const
{
    // Calculate the triangle ratio:
    // Distance between v3 and line segment v1->v2. 
    // The farther v3 is the better. Note that the ratio must be signed, 
    // otherwise flipped triangles can appear.
    //-------------------------
    return Math2D::LinePointDistance(MeshVertices[verIdx(m1->srcVer)],
                                     MeshVertices[verIdx(m2->srcVer)],
                                     MeshVertices[verIdx(m3->srcVer)]);
}

//-----------------------------------------------------------------------
void Tessellator::triangulateMountainAA()
{
    if (MonoStack.GetSize() > 2)
    {
        UPInt i;
        CoordType s = 0;
        unsigned ear = ~0U;

        const TessVertex& v1 = MeshVertices[verIdx(MonoStack[0]->srcVer)];
        const TessVertex& v2 = MeshVertices[verIdx(MonoStack.Back()->srcVer)];
        for(i = 1; i+1 < MonoStack.GetSize(); ++i)
        {
            const TessVertex& v = MeshVertices[verIdx(MonoStack[i]->srcVer)];
            s += Math2D::CrossProduct(v1, v2, v);
        }
        int      d = (s > 0) ? +1 : -1;
        unsigned n1 = 0;
        unsigned n2 = (unsigned)MonoStack.GetSize();

        while(n2 > n1+3)
        {
            CoordType s1, s2, s3, s4;
            s1 = triangleRatio(MonoStack[n1+2], MonoStack[n1+0], MonoStack[n1+1]);
            s2 = triangleRatio(MonoStack[n1+3], MonoStack[n1+1], MonoStack[n1+2]);
            s3 = triangleRatio(MonoStack[n2-1], MonoStack[n2-3], MonoStack[n2-2]);
            s4 = triangleRatio(MonoStack[n2-2], MonoStack[n2-4], MonoStack[n2-3]);

            ear = ~0U;
            if (d > 0)
            {
                // The mountain base is on the left
                //---------------------------------
                s = -EdgeAAWidth; // Restrict the minimal distance 
                if(s1 < 0 && s2 < 0)
                {
                    if(s1 < s) { s = s1; ear = n1+1; }
                    if(s2 < s) { s = s2; ear = n1+2; }
                }
                if(s2 < 0 && s4 < 0)
                {
                    if(s3 < s) { s = s3; ear = n2-2; }
                    if(s4 < s) { s = s4; ear = n2-3; }
                }
            }
            else
            {
                // The mountain base is on the right
                //---------------------------------
                s = +EdgeAAWidth; // Restrict the minimal distance
                if(s1 > 0 && s2 > 0)
                {
                    if(s1 > s) { s = s1; ear = n1+1; }
                    if(s2 > s) { s = s2; ear = n1+2; }
                }
                if(s3 > 0 && s4 > 0)
                {
                    if(s3 > s) { s = s3; ear = n2-2; }
                    if(s4 > s) { s = s4; ear = n2-3; }
                }
            }

            if(ear != ~0U)
            {
                addTriangleAA(MonoStack[ear-d], MonoStack[ear], MonoStack[ear+d], 0);
                if (ear == n1+1)
                {
                    MonoStack[n1+1] = MonoStack[n1+0]; ++n1;
                }
                else if(ear == n1+2)
                {
                    MonoStack[n1+2] = MonoStack[n1+1];
                    MonoStack[n1+1] = MonoStack[n1+0]; ++n1;
                }
                else if(ear == n2-2)
                {
                    MonoStack[n2-2] = MonoStack[n2-1]; --n2;
                }
                else//if(ear == n2-3)
                {
                    MonoStack[n2-3] = MonoStack[n2-2];
                    MonoStack[n2-2] = MonoStack[n2-1]; --n2;
                }
            }
            else
            {
                s = 0;
                for (i = n1+1; i+1 < n2; ++i)
                {
                    s1 = triangleRatio(MonoStack[i+1], MonoStack[i-1], MonoStack[i]);
                    if (d > 0) { if (s1 < s) { s = s1; ear = (unsigned)i; }}
                    else       { if (s1 > s) { s = s1; ear = (unsigned)i; }}
                }
                if(ear == ~0U) 
                {
                    return;
                }
                addTriangleAA(MonoStack[ear-d], MonoStack[ear], MonoStack[ear+d], 0);
                for (i = ear+1; i < n2; ++i)
                {
                    MonoStack[i-1] = MonoStack[i];
                }
                n2--;
            }
        }
        addTriangleAA(MonoStack[n1+1-d], MonoStack[n1+1], MonoStack[n1+1+d], 0);
    }
}

#else //SF_RENDER_ENABLE_MOUNTAINS

// Old Monotone Triangulation
//-----------------------------------------------------------------------
SF_INLINE CoordType 
Tessellator::triangleCrossProductAA(unsigned i1, unsigned i2, unsigned i3) const
{
    const TessVertex& a = MeshVertices[verIdx(i1)];
    const TessVertex& b = MeshVertices[verIdx(i2)];
    const TessVertex& c = MeshVertices[verIdx(i3)];
    return Math2D::CrossProduct(a, b, c);
}

//-----------------------------------------------------------------------
SF_INLINE void Tessellator::addTriangleAA(MonoVertexType* v1, 
                                       MonoVertexType* v2, 
                                       MonoVertexType* v3)
{
    addTriangleAA(v1, v2, v3, triangleCrossProductAA(v1->srcVer, v2->srcVer, v3->srcVer));
}

#endif //SF_RENDER_ENABLE_MOUNTAINS




//-----------------------------------------------------------------------
void Tessellator::triangulateMonotoneAA(MonotoneType* m)
{
    MonoVertexType* vertex = m->start;
    MonoStyle = m->style;

    MeshIdx = setMesh(MonoStyle);
    FactorOneFlag = TessStyleOpaque |
        ((MonoStyle == Meshes[MeshIdx].Style1) ? TessStyleFactorOne : 0);
    Meshes[MeshIdx].Flags1 |= FactorOneFlag & TessStyleFactorOne;

    m->d.t.startTriangle = ~0U;
    m->d.t.numTriangles  = 0;
    m->d.t.meshIdx = MeshIdx;

    if(vertex == 0 || vertex->next == 0 || vertex->next->next == 0) 
    {
        return;
    }

    m->d.t.startTriangle = (unsigned)MeshTriangles.GetSize(MeshIdx);

    MonoStack.Clear();
    MonoStack.PushBack(vertex); vertex = vertex->next;
    MonoStack.PushBack(vertex); vertex = vertex->next;

#ifdef SF_RENDER_ENABLE_MOUNTAINS
    // New Monotone Mountains Triangulation
    //------------------------
    while(vertex)
    {
        MonoVertexType* topStrip = vertex;
        MonoVertexType* topStack = MonoStack.Back();
        MonoStack.PushBack(topStrip);
        if(isLeft(topStrip->srcVer) != isLeft(topStack->srcVer))
        {
            triangulateMountainAA();
            MonoStack.Clear();
            MonoStack.PushBack(topStack);
            MonoStack.PushBack(topStrip);
        }
        vertex = vertex->next;
    }
    triangulateMountainAA();

#else //SF_RENDER_ENABLE_MOUNTAINS

    // Old Monotone Triangulation
    //------------------------
    MonoVertexType* p1;
    MonoVertexType* p2;
    while(vertex->next)
    {
        MonoVertexType* topStrip = vertex;
        MonoVertexType* topStack = MonoStack.Back();
    
        if(isLeft(topStrip->srcVer) != isLeft(topStack->srcVer))
        {
            while(MonoStack.GetSize() > 1)
            {
                p1 = MonoStack.Back();
                MonoStack.PopBack();
                addTriangleAA(topStrip, p1, MonoStack.Back());
            }
            MonoStack.PopBack();
            MonoStack.PushBack(topStack);
            MonoStack.PushBack(topStrip);
        }
        else
        {
            while(MonoStack.GetSize() > 1)
            {       
                p1 = MonoStack[MonoStack.GetSize() - 1];
                p2 = MonoStack[MonoStack.GetSize() - 2];
                CoordType cp = triangleCrossProductAA(topStrip->srcVer, p1->srcVer, p2->srcVer);
                if(cp == 0 || (cp < 0) != isLeft(p1->srcVer)) break;
                addTriangleAA(topStrip, p1, p2, cp);
                MonoStack.PopBack();
            }
            MonoStack.PushBack(topStrip); 
        }
        vertex = vertex->next;
    }
    
    while(MonoStack.GetSize() > 1)
    {
        p1 = MonoStack.Back();
        MonoStack.PopBack();
        addTriangleAA(vertex, p1, MonoStack.Back());
    }
#endif //SF_RENDER_ENABLE_MOUNTAINS

    m->d.t.numTriangles = (unsigned)MeshTriangles.GetSize(MeshIdx) - m->d.t.startTriangle;
}

//-----------------------------------------------------------------------
void Tessellator::unflipTriangles()
{
    // The idea of this function is that if the triangle is clockwise 
    // oriented, it's flipped. In this case we "fix" the triangles.
    // During this operation some other triangles may also flip, so that,
    // we have to perform several passes until all the triangles are 
    // correct. Theoretically it's possible to fall into an endless loop,
    // so, the number of passes must be limited.
    //-----------------------------
    bool     wasFlipped = true;
    unsigned pass;
    for(pass = 0; wasFlipped && pass < 8; ++pass)
    {
        wasFlipped = false;
        for(UPInt i = 0; i < MeshTriangles.GetArrayCount(); ++i)
        {
            UPInt nt = MeshTriangles.GetSize(i);
            for (UPInt j = 0; j < nt; ++j)
            {
                const TriangleType& t = MeshTriangles.At(i, j);
                const TessVertex& refV1 = MeshVertices[verIdx(t.d.m.v1->srcVer)];
                const TessVertex& refV2 = MeshVertices[verIdx(t.d.m.v2->srcVer)];
                const TessVertex& refV3 = MeshVertices[verIdx(t.d.m.v3->srcVer)];

                TessVertex& aaV1 = MeshVertices[verIdx(t.d.m.v1->aaVer)];
                TessVertex& aaV2 = MeshVertices[verIdx(t.d.m.v2->aaVer)];
                TessVertex& aaV3 = MeshVertices[verIdx(t.d.m.v3->aaVer)];

                if (Math2D::CrossProduct(aaV1, aaV2, aaV3) >= 0)
                {
                    CoordType d1 = Math2D::SqDistance(aaV1.x, aaV1.y, aaV2.x, aaV2.y);
                    CoordType d2 = Math2D::SqDistance(aaV2.x, aaV2.y, aaV3.x, aaV3.y);
                    CoordType d3 = Math2D::SqDistance(aaV3.x, aaV3.y, aaV1.x, aaV1.y);
                    CoordType epsilon = sqrtf(d1 + d2 + d3) * IntersectionEpsilon;
                    CoordType x, y;

                    // Select the longest triangle base and then try to move 
                    // the opposite vertex to this base, preventing the triangle 
                    // from degenerating -- leave 0.125 of the distance between the 
                    // reference vertex and the base. In case the segment 
                    // refVertex -> aaVertex does not intersect the base (function 
                    // TessIntersection fails) just move the aaVertex to
                    // refVertex by brute force.
                    //-------------------------
                    int edge = (d1 > d2) ? ((d1 > d3) ? 1 : 3) : ((d2 > d3) ? 2 : 3);
                    switch(edge)
                    {
                    case 1:
                        if (TessIntersection(aaV3, refV3, aaV1, aaV2, &x, &y, epsilon))
                        {
                            aaV3.x = x + (refV3.x - x) * 0.125f;
                            aaV3.y = y + (refV3.y - y) * 0.125f;
                        }
                        else
                        {
                            aaV3.x = refV3.x;
                            aaV3.y = refV3.y;
                        }
                        break;

                    case 2:
                        if (TessIntersection(aaV1, refV1, aaV2, aaV3, &x, &y, epsilon))
                        {
                            aaV1.x = x + (refV1.x - x) * 0.125f;
                            aaV1.y = y + (refV1.y - y) * 0.125f;
                        }
                        else
                        {
                            aaV1.x = refV1.x;
                            aaV1.y = refV1.y;
                        }
                        break;

                    case 3: 
                        if (TessIntersection(aaV2, refV2, aaV3, aaV1, &x, &y, epsilon))
                        {
                            aaV2.x = x + (refV2.x - x) * 0.125f;
                            aaV2.y = y + (refV2.y - y) * 0.125f;
                        }
                        else
                        {
                            aaV2.x = refV2.x;
                            aaV2.y = refV2.y;
                        }
                        break;
                    }

                    // If the triangle is still incorrect, just restore all its 
                    // original vertices by brute force.
                    //--------------------------
                    if (Math2D::CrossProduct(aaV1, aaV2, aaV3) >= 0)
                    {
                        aaV1.x = refV1.x;
                        aaV1.y = refV1.y;
                        aaV2.x = refV2.x;
                        aaV2.y = refV2.y;
                        aaV3.x = refV3.x;
                        aaV3.y = refV3.y;
                    }
                    wasFlipped = true;
                }
            }
        }
    }
}


//-----------------------------------------------------------------------
void Tessellator::addStrokerEdge(unsigned ia, unsigned ib)
{
    StrokerEdgeType e;
    e.node1 = verIdx(ia);
    e.node2 = verIdx(ib);
    if(e.node1 != e.node2)
    {
        StrokerEdges.PushBack(e);
    }
}


//-----------------------------------------------------------------------
void Tessellator::emitStrokerTrapezoid(const StrokerEdgeType* edge, 
                                       unsigned v1, unsigned v2)
{
    TriangleType tri;
    tri.d.t.v1 = verIdx(edge->node1);
    tri.d.t.v2 = v1;
    tri.d.t.v3 = v2;
    MeshTriangles.PushBack(0, tri);
    tri.d.t.v1 = verIdx(edge->node2);
    tri.d.t.v2 = verIdx(edge->node1);
    tri.d.t.v3 = v2;
    MeshTriangles.PushBack(0, tri);
}


//-----------------------------------------------------------------------
void Tessellator::emitStrokerVertex(CoordType x, CoordType y)
{
    TessVertex v;
    v.x = x;
    v.y = y;
    v.Idx = ~0U;
    v.Styles[0] = v.Styles[1] = 1;
    v.Flags = 0;
    v.Mesh = 0;
    MeshVertices.PushBack(v);
}

//-----------------------------------------------------------------------
unsigned Tessellator::addStrokerJoin(const StrokerEdgeType* e1, const StrokerEdgeType* e2)
{
    unsigned refVertex = verIdx(e1->node2);
    const TessVertex& v1 = MeshVertices[verIdx(e1->node1)];
    const TessVertex& v2 = MeshVertices[refVertex];
    const TessVertex& v3 = MeshVertices[verIdx(e2->node2)];

    CoordType width = -2*EdgeAAWidth;
    CoordType len1 = Math2D::Distance(v1, v2);
    CoordType len2 = Math2D::Distance(v2, v3);
    CoordType turn = Math2D::TurnRatio(v1, v2, v3, len1, len2);
    CoordType epsilon = (len1 + len2) * IntersectionEpsilon;
    CoordType dx1 = width * (v1.y - v2.y) / len1;
    CoordType dy1 = width * (v2.x - v1.x) / len1;
    CoordType dx2 = width * (v2.y - v3.y) / len2;
    CoordType dy2 = width * (v3.x - v2.x) / len2;
    CoordType xi = v2.x;
    CoordType yi = v2.y;

    unsigned nv = 1;
    if (fabsf(turn) < 0.125f)
    {
        if(len1 > len2) emitStrokerVertex(v2.x + dx1, v2.y + dy1);
        else            emitStrokerVertex(v2.x + dx2, v2.y + dy2);
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
                    emitStrokerVertex(v2.x + dx1, v2.y + dy1);
                    emitStrokerVertex(v2.x + dx2, v2.y + dy2);
                    nv = 2;
                }
                else
                {
                    emitStrokerVertex(xi, yi);
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
                    emitStrokerVertex(xi, yi);
                }
                else
                {
                    emitStrokerVertex(v2.x + dx1 - 2*dy1, v2.y + dy1 + 2*dx1);
                    emitStrokerVertex(v2.x + dx2 + 2*dy2, v2.y + dy2 - 2*dx2);
                    TriangleType tri;
                    tri.d.t.v1 = refVertex;
                    tri.d.t.v2 = (unsigned)MeshVertices.GetSize() - 2;
                    tri.d.t.v3 = (unsigned)MeshVertices.GetSize() - 1;
                    MeshTriangles.PushBack(0, tri);
                    nv = 2;
                }
            }
        }
        else
        {
            // Calculation of the intersection failed, most probably
            // the three points lie one straight line. 
            //----------------
            emitStrokerVertex(v2.x + dx1, v2.y + dy1);
            emitStrokerVertex(v2.x + dx2, v2.y + dy2);
            nv = 2;
        }
    }
    return nv;
}


//-----------------------------------------------------------------------
void Tessellator::processStrokerEdges()
{
    UPInt i;
    for(i = 0; i < Monotones.GetSize(); ++i)
    {
        const MonotoneType& monotone = Monotones[i];
        const MonoVertexType* vertex;
        
        vertex = monotone.start; 
        if(vertex == 0) continue;
        unsigned prevLeft = vertex->srcVer;

        vertex = vertex->next;
        if(vertex == 0) continue;
        unsigned prevRight = vertex->srcVer;

        addStrokerEdge(prevLeft, prevRight);
        while((vertex = vertex->next) != 0)
        {
            if(isLeft(vertex->srcVer))
            {
                addStrokerEdge(vertex->srcVer, prevLeft);
                prevLeft = vertex->srcVer;
            }
            else
            {
                addStrokerEdge(prevRight, vertex->srcVer);
                prevRight = vertex->srcVer;
            }
        }
        addStrokerEdge(prevRight, prevLeft);
    }

    if(StrokerEdges.GetSize() < 2) 
    {
        return;
    }

    Alg::QuickSort(StrokerEdges, cmpStrokerEdges);

    for(i = 0; i < StrokerEdges.GetSize(); ++i)
    {
        StrokerEdgeType& e1 = StrokerEdges[i];
        if((e1.node1 & VisitedFlag) == 0)
        {
            StrokerEdgeType es;
            es.node1 = e1.node2;
            es.node2 = e1.node1;
            UPInt pos = Alg::LowerBound(StrokerEdges, es, cmpStrokerEdges);
            if(pos < StrokerEdges.GetSize())
            {
                StrokerEdgeType& e2 = StrokerEdges[pos];
                if(e1.node1 == e2.node2 && e1.node2 == e2.node1)
                {
                    e1.node1 |= VisitedFlag;
                    e2.node1 |= VisitedFlag;
                }
            }
        }
    }

    for(i = 0; i < StrokerEdges.GetSize(); ++i)
    {
        StrokerEdgeType* strtEdge = &StrokerEdges[i];
        if((strtEdge->node1 & VisitedFlag) == 0)
        {
            StrokerEdgeType* thisEdge = strtEdge;
            unsigned strtVer = ~0U;
            unsigned prevVer = ~0U;
            for(;;)
            {
                UPInt pos = Alg::LowerBound(StrokerEdges, thisEdge->node2, cmpStrokerNode1);
                StrokerEdgeType* nextEdge = 0;
                while(pos < StrokerEdges.GetSize())
                {
                    StrokerEdgeType* e = &StrokerEdges[pos];
                    if((e->node1 &  VisitedFlag) == 0 && 
                        verIdx(e->node1) == verIdx(thisEdge->node2))
                    {
                        nextEdge = e;
                        break;
                    }
                    ++pos;
                }
                if(nextEdge == 0) break;

                unsigned nv = addStrokerJoin(thisEdge, nextEdge);
                if(prevVer != ~0U)
                {
                    emitStrokerTrapezoid(thisEdge, prevVer, (unsigned)MeshVertices.GetSize()-nv);
                    thisEdge->node1 |= VisitedFlag;
                }
                else
                {
                    strtVer = (unsigned)MeshVertices.GetSize() - nv;
                }
                if(nextEdge == strtEdge)
                {
                    emitStrokerTrapezoid(strtEdge, (unsigned)MeshVertices.GetSize()-1, strtVer);
                    strtEdge->node1 |= VisitedFlag;
                    break;
                }
                prevVer = (unsigned)MeshVertices.GetSize()-1;
                thisEdge = nextEdge;
            }
        }
    }
}



//-----------------------------------------------------------------------
void Tessellator::processEdgeAA()
{
    UPInt i;

    // Important: Monotones.GetSize() is NOT a loop invariant!
    //-----------------
    unsigned totalEdges = 0;
    for(i = 0; i < Monotones.GetSize(); ++i)
    {
        totalEdges += countFanEdges(&Monotones[i]);
    }

    EdgeFans.Resize(totalEdges);

    totalEdges = 0;
    TessVertex* ver;
    for(i = 0; i < MeshVertices.GetSize(); ++i)
    {
        ver = &MeshVertices[i];
        ver->Idx = totalEdges;
        totalEdges += ver->Mesh;
        ver->Mesh = 0;
    }

    for(i = 0; i < Monotones.GetSize(); ++i)
    {
        collectFanEdges(&Monotones[i]);
    }

    // Important: MeshVertices.GetSize() will increase in a loop.
    // Process the old vertices only!
    //---------------------
    UPInt numVertices = MeshVertices.GetSize();
    for(i = 0; i < numVertices; ++i)
    {
        ver = &MeshVertices[i];
        TmpEdgeFan.Clear();
        UPInt j;
        for (j = 0; j < ver->Mesh; ++j)
        {
            const EdgeAAType& e = EdgeFans[ver->Idx + j];
            TmpEdgeAAType te;
            te.cntVer = e.cntVer;
            te.rayVer = e.rayVer;
            te.style  = e.style;
            te.slope  = 
                Math2D::SlopeRatio(MeshVertices[verIdx(te.cntVer->srcVer)],
                                   MeshVertices[verIdx(te.rayVer->srcVer)]);
            TmpEdgeFan.PushBack(te);
        }
        Alg::QuickSort(TmpEdgeFan, cmpEdgeAA);
        unsigned  slopeIdx = 0;
        CoordType slope = MinCoord;
        for (j = 0; j < TmpEdgeFan.GetSize(); ++j)
        {
            EdgeAAType& e = EdgeFans[ver->Idx + j];
            const TmpEdgeAAType& te = TmpEdgeFan[j];
            e.cntVer = te.cntVer;
            e.rayVer = te.rayVer;
            e.style  = UInt16(te.style);
            e.slope  = UInt16(slopeIdx);
            if (te.slope != slope) 
            {
                ++slopeIdx;
                slope = te.slope;
            }
        }
        processFan(ver->Idx, ver->Idx + ver->Mesh);
        ver->Idx = ~0U;
        ver->Mesh = 0xFFFFU;
    }

    for(i = 0; i < Monotones.GetSize(); ++i)
    {
        triangulateMonotoneAA(&Monotones[i]);
    }

    unflipTriangles();
    emitTriangles();
}

//-----------------------------------------------------------------------
void Tessellator::Tessellate(bool autoSplitMeshes)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Tessellator::Tessellate", Amp_Native_Function_Id_Tessellate);
    UPInt i;

    MinX  = MaxCoord;
    MinY  = MaxCoord;
    MaxX  = MinCoord;
    MaxY  = MinCoord;
    monotonize();

    // Reserve Meshes[0] specifically for solid fills
    //-----------------
    TessMesh solidMesh;
    solidMesh.MeshIdx = 0;
    solidMesh.Style1 = solidMesh.Style2 = 0;
    solidMesh.Flags1 = solidMesh.Flags2 = 0;
    solidMesh.VertexCount = 0;
    solidMesh.StartVertex = 0;
    Meshes.PushBack(solidMesh);

    if (HasComplexFill)
    {
        StyleMatrix.Resize(MaxStyle + 1);
        for(i = 0; i < Paths.GetSize(); ++i)
        {
            const PathType& p = Paths[i];
            if (p.leftStyle && p.rightStyle) 
                setMesh(p.leftStyle, p.rightStyle);
        }
        for(i = 0; i < Paths.GetSize(); ++i)
        {
            const PathType& p = Paths[i];
            if (p.leftStyle)  setMesh(p.leftStyle);
            if (p.rightStyle) setMesh(p.rightStyle);
        }
    }

    clearHeap1();

    for(i = 0; i < Meshes.GetSize(); ++i)
        MeshTriangles.AddArray();

    if (EdgeAAFlag)
    {
        if (StrokerMode)
        {
            setMesh(1);
            EdgeAAFlag = false;
            for(i = 0; i < Monotones.GetSize(); ++i)
            {
                triangulateMonotoneAA(&Monotones[i]);
            }
            EdgeAAFlag = true;
            processStrokerEdges();
        }
        else
        {
            processEdgeAA();
        }
    }
    else
    {
        for(i = 0; i < Monotones.GetSize(); ++i)
        {
            triangulateMonotoneAA(&Monotones[i]);
        }
    }

    // Re-numerate vertices to match mesh triangles
    //-----------------------
    for (i = 0; i < MeshVertices.GetSize(); ++i)
    {
        TessVertex& v = MeshVertices[i];
        if (v.Mesh != 0xFFFF) v.Idx = Meshes[v.Mesh].VertexCount++;
    }

    if (autoSplitMeshes && VertexLimit && MeshVertices.GetSize() > VertexLimit)
    {
        SplitMeshes();
    }
}




#else // SF_RENDER_ENABLE_EDGEAA


//-----------------------------------------------------------------------
SF_INLINE void Tessellator::addTriangle(unsigned i1, unsigned i2, unsigned i3, CoordType cp)
{
    if(cp != 0)
    {
        TriangleType tri;
        tri.d.t.v1 = emitVertex(MeshIdx, verIdx(i1), MonoStyle, FactorOneFlag);
        tri.d.t.v2 = emitVertex(MeshIdx, verIdx(i2), MonoStyle, FactorOneFlag);
        tri.d.t.v3 = emitVertex(MeshIdx, verIdx(i3), MonoStyle, FactorOneFlag);
        if(cp > 0)
        {
            Alg::Swap(tri.d.t.v2, tri.d.t.v3);
        }
        MeshTriangles.PushBack(MeshIdx, tri);
    }
}

//-----------------------------------------------------------------------
SF_INLINE CoordType 
Tessellator::triangleCrossProduct(unsigned i1, unsigned i2, unsigned i3) const
{
    return Math2D::CrossProduct(MeshVertices[verIdx(i1)], 
                                MeshVertices[verIdx(i2)], 
                                MeshVertices[verIdx(i3)]);
}

//-----------------------------------------------------------------------
SF_INLINE void Tessellator::addTriangle(unsigned i1, unsigned i2, unsigned i3)
{
    addTriangle(i1, i2, i3, triangleCrossProduct(i1, i2, i3));
}

//-----------------------------------------------------------------------
void Tessellator::triangulateMonotone(const MonotoneType& monotone)
{
    MonoVertexType* vertex = monotone.start;

    if(vertex == 0 || vertex->next == 0 || vertex->next->next == 0) 
    {
        return; 
    }

    // Assign mesh index that corresponds to the given style.
    //------------------
    MonoStyle = monotone.style;
    MeshIdx   = setMesh(MonoStyle);
    FactorOneFlag = TessStyleOpaque |
        ((MonoStyle == Meshes[MeshIdx].Style1) ? TessStyleFactorOne : 0);
    Meshes[MeshIdx].Flags1 |= FactorOneFlag & TessStyleFactorOne;

    MonoStack.Clear();
    MonoStack.PushBack(vertex->srcVer); vertex = vertex->next;
    MonoStack.PushBack(vertex->srcVer); vertex = vertex->next;

    unsigned p1, p2;
    while(vertex->next)
    {
        unsigned topStrip = vertex->srcVer;
        unsigned topStack = MonoStack.Back();

        if(isLeft(topStrip) != isLeft(topStack))
        {
            while(MonoStack.GetSize() > 1)
            {
                p1 = MonoStack.Back();
                MonoStack.PopBack();
                addTriangle(topStrip, p1, MonoStack.Back());
            }
            MonoStack.PopBack();
            MonoStack.PushBack(topStack);
            MonoStack.PushBack(topStrip);
        }
        else
        {
            while(MonoStack.GetSize() > 1)
            {       
                p1 = MonoStack[MonoStack.GetSize() - 1];
                p2 = MonoStack[MonoStack.GetSize() - 2];
                CoordType cp = triangleCrossProduct(topStrip, p1, p2);
                if((cp < 0) != isLeft(p1)) break;
                addTriangle(topStrip, p1, p2, cp);
                MonoStack.PopBack();
            }
            MonoStack.PushBack(topStrip); 
        }
        vertex = vertex->next;
    }

    unsigned lastQueuePoint = vertex->srcVer;
    while(MonoStack.GetSize() > 1)
    {
        p1 = MonoStack.Back();
        MonoStack.PopBack();
        addTriangle(lastQueuePoint, p1, MonoStack.Back());
    }
}

//-----------------------------------------------------------------------
void Tessellator::Tessellate(bool autoSplitMeshes)
{
    unsigned i;

    SF_UNUSED(autoSplitMeshes);

    MinX  = MaxCoord;
    MinY  = MaxCoord;
    MaxX  = MinCoord;
    MaxY  = MinCoord;
    monotonize();

    // Reserve Meshes[0] specifically for solid fills
    //-----------------
    TessMesh solidMesh;
    solidMesh.MeshIdx = 0;
    solidMesh.Style1 = solidMesh.Style2 = 0;
    solidMesh.Flags1 = solidMesh.Flags2 = 0;
    solidMesh.VertexCount = 0;
    solidMesh.StartVertex = 0;
    Meshes.PushBack(solidMesh);

    if (HasComplexFill)
    {
        StyleMatrix.Resize(MaxStyle + 1);
        for(i = 0; i < Paths.GetSize(); ++i)
        {
            const PathType& p = Paths[i];
            if (p.leftStyle && p.rightStyle) 
                setMesh(p.leftStyle, p.rightStyle);
        }
        for(i = 0; i < Paths.GetSize(); ++i)
        {
            const PathType& p = Paths[i];
            if (p.leftStyle)  setMesh(p.leftStyle);
            if (p.rightStyle) setMesh(p.rightStyle);
        }
    }

    clearHeap1();

    for(i = 0; i < Meshes.GetSize(); ++i)
        MeshTriangles.AddArray();

    for (i = 0; i < Monotones.GetSize(); ++i)
    {
        triangulateMonotone(Monotones[i]);
    }

    // Re-numerate vertices to match mesh triangles
    //-----------------------
    for (i = 0; i < MeshVertices.GetSize(); ++i)
    {
        TessVertex& v = MeshVertices[i];
        v.Idx = (v.Mesh == 0xFFFF) ? ~0U : Meshes[v.Mesh].VertexCount++;
    }

    if (VertexLimit && MeshVertices.GetSize() > VertexLimit)
    {
        SplitMeshes();
    }
}

#endif // SF_RENDER_ENABLE_EDGEAA

//-----------------------------------------------------------------------
void Tessellator::Transform(const Matrix2F& m)
{
    UPInt i;
    MinX = MaxCoord;
    MinY = MaxCoord;
    MaxX = MinCoord;
    MaxY = MinCoord;
    for (i = 0; i < MeshVertices.GetSize(); ++i)
    {
        // TO DO: possible to optimize access
        TessVertex& v = MeshVertices[i];
        m.Transform(&v.x, &v.y);
        if (v.x < MinX) MinX = v.x;
        if (v.y < MinY) MinY = v.y;
        if (v.x > MaxX) MaxX = v.x;
        if (v.y > MaxY) MaxY = v.y;
    }
}

//-----------------------------------------------------------------------
Matrix2F Tessellator::StretchTo(float x1, float y1, float x2, float y2)
{
    Matrix2F m;
    UPInt i;
    if (MinX >= MaxX || MinY >= MaxY)
    {
        MinX = MaxCoord;
        MinY = MaxCoord;
        MaxX = MinCoord;
        MaxY = MinCoord;
        for (i = 0; i < MeshVertices.GetSize(); ++i)
        {
            // TO DO: possible to optimize access
            TessVertex v = MeshVertices[i];
            if (v.x < MinX) MinX = v.x;
            if (v.y < MinY) MinY = v.y;
            if (v.x > MaxX) MaxX = v.x;
            if (v.y > MaxY) MaxY = v.y;
        }
    }
    if (MinX < MaxX && MinY < MaxY)
    {
        m.SetRectToRect(MinX, MinY, MaxX, MaxY, x1, y1, x2, y2);
        for (i = 0; i < MeshVertices.GetSize(); ++i)
        {
            // TO DO: possible to optimize access
            TessVertex& v = MeshVertices[i];
            m.Transform(&v.x, &v.y);
        }
        m.Invert();
    }
    return m;
}

}} // Scaleform::Render

