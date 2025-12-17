/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_AllocAddr.h
Content     :   Abstract address space allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   An abstract address space allocator based on binary 
                trees. Used when the memory itself is not available
                (address space, video memory, etc).

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Memory_AllocAddr_H
#define INC_SF_Kernel_Memory_AllocAddr_H

#include "SF_List.h"
#include "SF_RadixTree.h"
#include "SF_Types.h"

namespace Scaleform {

//------------------------------------------------------------------------
struct AllocAddrNode : ListNode<AllocAddrNode>
{
    AllocAddrNode*  AddrParent;
    AllocAddrNode*  AddrChild[2];
    AllocAddrNode*  SizeParent;
    AllocAddrNode*  SizeChild[2];
    UPInt           Addr;
    UPInt           Size;
};

class MemoryHeap;

//------------------------------------------------------------------------

// AllocAddr is an allocator for blocks of address space that doesn't store
// any header management information in the managed memory; it uses the provided
// memory heap for descriptor nodes instead. Intended for vertex/index buffer allocation
// and video memory management.

class AllocAddr
{
    typedef AllocAddrNode FreeNode;

public:
    //--------------------------------------------------------------------
    struct SizeAccessor
    {
        static       UPInt      GetKey     (const FreeNode* n)          { return  n->Size; }
        static       FreeNode*  GetChild   (      FreeNode* n, UPInt i) { return  n->SizeChild[i]; }
        static const FreeNode*  GetChild   (const FreeNode* n, UPInt i) { return  n->SizeChild[i]; }
        static       FreeNode** GetChildPtr(      FreeNode* n, UPInt i) { return &n->SizeChild[i]; }

        static       FreeNode*  GetParent  (      FreeNode* n)          { return n->SizeParent; }
        static const FreeNode*  GetParent  (const FreeNode* n)          { return n->SizeParent; }

        static void SetParent (FreeNode* n, FreeNode* p)                { n->SizeParent   = p; }
        static void SetChild  (FreeNode* n, UPInt i, FreeNode* c)       { n->SizeChild[i] = c; }
        static void ClearLinks(FreeNode* n) { n->SizeParent = n->SizeChild[0] = n->SizeChild[1] = 0; }
    };

    //--------------------------------------------------------------------
    struct AddrAccessor
    {
        static       UPInt      GetKey     (const FreeNode* n)          { return  n->Addr; }
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
    AllocAddr(MemoryHeap* nodeHeap);
    AllocAddr(MemoryHeap* nodeHeap, UPInt start, UPInt size);
    ~AllocAddr();

    // Add/remove segments. The new segments will be used in the common
    // tree, so that, the allocation may occur in any segment. If the 
    // segments are coalesced they will be merged. Technically it's OK
    // to remove a part of the segment, in case it is empty. If not, an
    // assertion will fail in debug mode.
    void    AddSegment(UPInt addr, UPInt size);
    void    RemoveSegment(UPInt addr, UPInt size);

    // Allocates a block of address space of specified size.
    // Returns ~UPInt(0) on fail. Important! Aligned alloc aligns
    // only the address, NOT the size. So, Alloc(1, 65536) will return 
    // only one byte of the usable address space.
    UPInt   Alloc(UPInt size);
    UPInt   Alloc(UPInt size, UPInt align);

    // Frees an allocation at address. Must pass the original allocated size.
    // Returns the size of the merged block that becomes available after
    // free, this will be >= size.
    UPInt   Free(UPInt addr, UPInt size);

    // Obtain the size of the largest block available for allocation.
    UPInt   GetLargestAvailable()
    {
        const FreeNode* node = SizeTree.FindBestLeEq(~UPInt(0));
        return node ? node->Size : 0;
    }

    UPInt   GetFreeSize() const;

private:
    //--------------------------------------------------------------------
    typedef RadixTreeMulti<FreeNode, SizeAccessor>   SizeTreeType;
    typedef RadixTree     <FreeNode, AddrAccessor>   AddrTreeType;

    //--------------------------------------------------------------------
    void        destroyAll();
    void        pushNode(FreeNode* node, UPInt addr, UPInt size);
    void        pullNode(FreeNode* node);
    FreeNode*   pullBest(UPInt size);
    void        splitNode(FreeNode* node, UPInt addr, UPInt size);
    UPInt       mergeNodes(FreeNode* prev, FreeNode* next, UPInt addr, UPInt size);

    //--------------------------------------------------------------------
    MemoryHeap*     pNodeHeap;
    SizeTreeType    SizeTree;
    AddrTreeType    AddrTree;
};

} // Scaleform

#endif
