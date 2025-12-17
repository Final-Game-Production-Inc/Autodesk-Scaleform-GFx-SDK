/**************************************************************************

Filename    :   HeapPT_AllocLite.h
Content     :   Allocator based on binary search trees
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   The granulator requests the system allocator for 
                large memory blocks and granulates them as necessary,
                providing smaller granularity to the consumer.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_AllocLite_H
#define INC_SF_Kernel_HeapPT_AllocLite_H

#include "../SF_List.h"
#include "../SF_RadixTree.h"
#include "../SF_HeapTypes.h"

namespace Scaleform { 

namespace Heap { class SegVisitor; }
    
namespace HeapPT {

using namespace Heap;

//------------------------------------------------------------------------
struct HdrPage;
struct TreeSeg
{
    TreeSeg*    AddrParent;
    TreeSeg*    AddrChild[2];
    HdrPage*    Headers;
    UByte*      Buffer;
    UPInt       Size;
    UPInt       UseCount;
    UPIntHalf   AlignShift;
    UPIntHalf   HeadBytes;
};


//------------------------------------------------------------------------
struct DualTNode : ListNode<DualTNode>
{
    DualTNode* Parent;
    DualTNode* Child[2];
    DualTNode* AddrParent;
    DualTNode* AddrChild[2];
    TreeSeg*   ParentSeg;
    UPInt      Size;

    //--------------------------------------------------------------------
    static SF_INLINE UByte* GetAlignedPtr(UByte* start, UPInt alignMask)
    {
        UPInt aligned = (UPInt(start) + alignMask) & ~alignMask;
        UPInt head = aligned - UPInt(start);
        while (head && head < 16*sizeof(void*))
        {
            aligned += alignMask+1;
            head    += alignMask+1;
        }
        return (UByte*)aligned;
    }

    SF_INLINE bool AlignmentIsOK(UPInt blocks, UPInt shift, UPInt alignMask) const
    {
        UPInt  bytes   = blocks << shift;
        UPInt  aligned = UPInt(GetAlignedPtr((UByte*)this, alignMask));
        return aligned + bytes <= UPInt(this) + (Size << shift);
    }
};

//------------------------------------------------------------------------
class AllocLite
{
    typedef DualTNode FreeNode;

public:
    //--------------------------------------------------------------------
    struct SizeAccessor
    {
        static       UPInt      GetKey     (const FreeNode* n)          { return  n->Size;     }
        static       FreeNode*  GetChild   (      FreeNode* n, UPInt i) { return  n->Child[i]; }
        static const FreeNode*  GetChild   (const FreeNode* n, UPInt i) { return  n->Child[i]; }
        static       FreeNode** GetChildPtr(      FreeNode* n, UPInt i) { return &n->Child[i]; }

        static       FreeNode*  GetParent  (      FreeNode* n)          { return n->Parent; }
        static const FreeNode*  GetParent  (const FreeNode* n)          { return n->Parent; }

        static void SetParent (FreeNode* n, FreeNode* p)                { n->Parent   = p; }
        static void SetChild  (FreeNode* n, UPInt i, FreeNode* c)       { n->Child[i] = c; }
        static void ClearLinks(FreeNode* n) { n->Parent = n->Child[0] = n->Child[1] = 0; }
    };

    //--------------------------------------------------------------------
    struct AddrAccessor
    {
        static       UPInt      GetKey     (const FreeNode* n)          { return  UPInt(n); }
        static       FreeNode*  GetChild   (      FreeNode* n, UPInt i) { return  n->AddrChild[i]; }
        static const FreeNode*  GetChild   (const FreeNode* n, UPInt i) { return  n->AddrChild[i]; }
        static       FreeNode** GetChildPtr(      FreeNode* n, UPInt i) { return &n->AddrChild[i]; }

        static       FreeNode*  GetParent  (      FreeNode* n)          { return n->AddrParent; }
        static const FreeNode*  GetParent  (const FreeNode* n)          { return n->AddrParent; }

        static void SetParent (FreeNode* n, FreeNode* p)                { n->AddrParent   = p; }
        static void SetChild  (FreeNode* n, UPInt i, FreeNode* c)       { n->AddrChild[i] = c; }
        static void ClearLinks(FreeNode* n) { n->AddrParent = n->AddrChild[0] = n->AddrChild[1] = 0; }
    };

public:
    //--------------------------------------------------------------------
    enum ReallocResult
    {
        // Don't change these values because the check for successful
        // operation looks like (res < ReallocFailed).
        ReallocSucceeded        = 0,
        ReallocShrinkedAtTail   = 1,
        ReallocFailed           = 2,
        ReallocFailedAtTail     = 3
    };

    AllocLite(UPInt minSize);

    void  InitSegment(TreeSeg* seg);
    void  ReleaseSegment(TreeSeg* seg);

    void* Alloc(UPInt size, UPInt alignSize, TreeSeg** allocSeg);
    void  Free(TreeSeg* seg, void* ptr, UPInt size, UPInt alignSize);

    ReallocResult   ReallocInPlace(TreeSeg* seg, void* ptr, UPInt oldSize, 
                                   UPInt newSize, UPInt alignSize);

    void    Extend(TreeSeg* seg, UPInt incSize);
    bool    TrimAt(TreeSeg* seg, UByte* ptrAt);

    UPInt GetMinSize()   const { return MinSize; }
    UPInt GetFreeBytes() const { return FreeBlocks << MinShift; }
    void  VisitMem(MemVisitor* visitor, MemVisitor::Category cat) const;
    void  VisitUnused(SegVisitor* visitor, unsigned cat) const;

private:
    //--------------------------------------------------------------------
    typedef RadixTreeMulti<FreeNode, SizeAccessor>   SizeTreeType;
    typedef RadixTree     <FreeNode, AddrAccessor>   AddrTreeType;

    //--------------------------------------------------------------------
    void        pushNode(FreeNode* node, TreeSeg* seg, UPInt blocks);
    void        pullNode(FreeNode* node);
    FreeNode*   pullBest(UPInt blocks);
    FreeNode*   pullBest(UPInt blocks, UPInt alignMask);
    void        splitNode(FreeNode* node, UByte* start, UPInt size);

    void        visitTree(const FreeNode* root, 
                          HeapSegment* seg,
                          MemVisitor* visitor,
                          MemVisitor::Category cat) const;

    void        visitUnusedNode(const FreeNode* node, 
                                SegVisitor* visitor, 
                                unsigned cat) const;

    void        visitUnusedInTree(const FreeNode* root, 
                                  SegVisitor* visitor, 
                                  unsigned cat) const;

    //--------------------------------------------------------------------
    UPInt           MinShift;
    UPInt           MinSize;
    UPInt           MinMask;
    SizeTreeType    SizeTree;
    AddrTreeType    AddrTree;
    UPInt           FreeBlocks;
};

}} // Scaleform::Heap

#endif
