/**************************************************************************

Filename    :   HeapPT_FreeBin.h
Content     :   
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Containers to store information about free memory areas

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_FreeBin_H
#define INC_SF_Kernel_HeapPT_FreeBin_H

#include "../SF_HeapTypes.h"

namespace Scaleform { 

namespace Heap { class SegVisitor; }

namespace HeapPT {

using namespace Heap;

// The structure of free zones.
//
// If the size is less than Limit (see below) it's stored in "ShortSize". 
// Otherwise, ShortSize equals to Limit, and the size is stored in UPInt, 
// right after sizeof(BinLNode). If the size is more than Heap_MinSize, 
// the Filler is not used. In the smallest possible free block it's used 
// as the size at the end of the block. So that, for sizes < Limit:
// +----------+----------+----------+-----+-----+- - -+-----+
// |   pPrev  |   pNext  | pSegment |Size |Filler ... |Size |
// +-One-UPInt+----------+----------+-----+-----+- - -+-----+
//
// For sizes >= Limit:
// +----------+----------+----------+-----+-----+----------+- - -+----------+-----+-----+
// |   pPrev  |   pNext  | pSegment |Limit|Filler   Size   | ... |   Size   Filler|Limit|
// +-One-UPInt+----------+----------+-----+-----+----------+- - -+----------+-----+-----+
//
// (pPrev and pNext belong to GListNode<BinLNode>).
//------------------------------------------------------------------------
struct BinLNode
{
    BinLNode*       pPrev;
    BinLNode*       pNext;
    HeapSegment*    pSegment;
    UPIntHalf       ShortSize;
    UPIntHalf       Filler;
};

//------------------------------------------------------------------------
struct BinTNode : BinLNode
{
    UPInt           Size;
    BinTNode*       Parent;
    BinTNode*       Child[2];
    UPInt           Index;
};


//------------------------------------------------------------------------
struct ListBin
{
    enum { BinSize = 8*sizeof(UPInt) }; // Assume Byte is 8 bits.

    ListBin();
    void Reset();

    //--------------------------------------------------------------------
    void        PushNode(UPInt idx, BinLNode* node);
    void        PullNode(UPInt idx, BinLNode* node);

    BinLNode*   PullBest(UPInt idx);
    BinLNode*   PullBest(UPInt idx, UPInt blocks, UPInt shift, UPInt alignMask);

    BinLNode*   FindAligned(BinLNode* root, UPInt blocks, UPInt shift, UPInt alignMask);

    static UByte* GetAlignedPtr(UByte* start, UPInt alignMask);
    static bool   AlignmentIsOK(const BinLNode* node, UPInt blocks,
                                UPInt shift, UPInt alignMask);

    //--------------------------------------------------------------------
    UPInt       Mask;
    BinLNode*   Roots[BinSize];
};


//------------------------------------------------------------------------
struct TreeBin
{
    enum 
    {
        BinSize   = ListBin::BinSize,
        SizeShift = (sizeof(void*) > 4) ? 6 : 5, // Must correspond to sizeof(void*)
        SizeLimit = (sizeof(void*) > 4) ? 0xFFFFFFFFU : 0xFFFFU
    };

    TreeBin();
    void Reset();

    //--------------------------------------------------------------------
    static UPInt ShiftForTreeIndex(UPInt i);
    static UPInt ComputeTreeIndex(UPInt blocks);

    void        PushNode(BinTNode* node);
    void        PullNode(BinTNode* node);
    BinTNode*   FindBest(UPInt size);
    BinTNode*   PullBest(UPInt size);

    const BinTNode* FindExactSize(UPInt size) const;

    //--------------------------------------------------------------------
    UPInt       Mask;
    BinTNode*   Roots[BinSize];
};



//------------------------------------------------------------------------
class FreeBin
{
public:
    enum { BinSize = ListBin::BinSize };

    FreeBin();
    void Reset();

    //--------------------------------------------------------------------
    SF_INLINE static
    UInt32* GetBitSet(const HeapSegment* seg)
    {
        return (UInt32*)(((UByte*)seg) + sizeof(HeapSegment));
    }

    //--------------------------------------------------------------------
    SF_INLINE static 
    UPInt GetSize(const void* node)
    {
        UPInt size = ((const BinLNode*)node)->ShortSize;
        if (size < BinSize+1)
        {
            return size;
        }
        return *(const UPInt*)((const UByte*)node + sizeof(BinLNode));
    }

    //--------------------------------------------------------------------
    SF_INLINE static 
    UPInt GetBytes(const void* node, UPInt shift)
    {
        return GetSize(node) << shift;
    }

    //--------------------------------------------------------------------
    SF_INLINE static 
    void SetSize(UByte* node, UPInt blocks, UPInt shift) 
    { 
        UPInt bytes = blocks << shift;
        BinLNode* listNode = (BinLNode*)node;
        if (blocks < BinSize+1)
        {
            listNode->ShortSize = 
            *(UPIntHalf*)(node + bytes - sizeof(UPIntHalf)) = UPIntHalf(blocks);
        }
        else
        {
            listNode->ShortSize = 
            *(UPIntHalf*)(node + bytes - sizeof(UPIntHalf)) = UPIntHalf(BinSize+1);

            *(UPInt*)(node + sizeof(BinLNode)) = 
            *(UPInt*)(node + bytes - 2*sizeof(UPInt)) = blocks;
        }
    }

    //--------------------------------------------------------------------
    SF_INLINE static void 
    MakeNode(HeapSegment* seg, UByte* data, UPInt blocks, UPInt shift)
    {
        SetSize(data, blocks, shift);
        ((BinLNode*)data)->pSegment = seg;
#ifdef SF_MEMORY_CHECK_CORRUPTION
        MemsetNode((BinLNode*)data, shift);
#endif
    }

    //--------------------------------------------------------------------
    UByte*  PullBest(UPInt blocks);
    UByte*  PullBest(UPInt blocks, UPInt shift, UPInt alignMask);

    //--------------------------------------------------------------------
              void Push(UByte* node); // Node must be initialized!
    SF_INLINE void Push(HeapSegment* seg, UByte* node, 
                        UPInt blocks, UPInt shift)
    {
        MakeNode(seg, node, blocks, shift);
        Push(node);
    }

    //--------------------------------------------------------------------
    void    Pull(UByte* node);
    void    Merge(UByte* node, UPInt shift, bool left, bool right);

    UPInt   GetTotalFreeSpace(UPInt shift) const
    {
        return FreeBlocks << shift;
    }

    //--------------------------------------------------------------------
    void    VisitMem(MemVisitor* visitor, UPInt shift,
                     MemVisitor::Category cat) const;

    void    VisitUnused(SegVisitor* visitor, UPInt shift, unsigned cat) const;

    void    CheckIntegrity(UPInt shift) const;

#ifdef SF_MEMORY_CHECK_CORRUPTION
    static void CheckArea(const void* area, UPInt bytes);
    static void MemsetNode(BinLNode* node, UPInt shift);
    static void CheckNode(const BinLNode* node, UPInt shift);
#endif


private:
    static void compilerAsserts(); // A fake function used for GCOMPILER_ASSERTS only

    static BinLNode* getPrevAdjacent(UByte* node, UPInt shift);
    static BinLNode* getNextAdjacent(UByte* node, UPInt shift);

    void visitTree(const BinTNode* root, MemVisitor* visitor,
                   UPInt shift, MemVisitor::Category cat) const;

    void visitUnusedNode(const BinLNode* node, 
                         SegVisitor* visitor, 
                         UPInt shift, unsigned cat) const;

    void visitUnusedInTree(const BinTNode* root, 
                           SegVisitor* visitor, 
                           UPInt shift, unsigned cat) const;

    void checkBlockIntegrity(const BinLNode* node, UPInt shift) const;
    void checkTreeIntegrity(const BinTNode* root, UPInt shift) const;

    ListBin ListBin1;
    ListBin ListBin2;
    TreeBin TreeBin1;
    UPInt   FreeBlocks;
};

}} // Scaleform::Heap

#endif
