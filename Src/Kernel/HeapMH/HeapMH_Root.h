/**************************************************************************

Filename    :   HeapMH_Root.h
Content     :   Heap root used for bootstrapping and bookkeeping.
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapMH_Root_H
#define INC_SF_Kernel_HeapMH_Root_H

#include "../SF_Atomic.h"
#include "../SF_SysAlloc.h"
#include "../SF_RadixTree.h"
#include "HeapMH_MemoryHeap.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
struct PageMH : public ListNode<PageMH>
{
    enum 
    { 
        PageSize    = 4096, 
        PageMask    = PageSize-1, 
        UnitShift   = 4,
        UnitSize    = 1 << UnitShift,
        UnitMask    = UnitSize - 1,
        BitSetBytes = PageSize/UnitSize/4,
        MaxBytes    = 512
    };

    MemoryHeapMH   *pHeap;     // Heap this page belongs to.
    UByte*         Start;
};


//------------------------------------------------------------------------
struct PageTableMH
{
    enum 
    {
        Index1Shift = 7,
        TableSize   = 1 << Index1Shift,
        Index0Mask  = TableSize - 1
    };

    struct Level0Entry
    {
        PageMH* FirstPage;
        UPInt   SizeMask;
    };

    Level0Entry Entries[TableSize];
};

struct DebugDataMH;

//------------------------------------------------------------------------
struct MagicHeader
{
    enum { MagicValue = 0x5FC0 }; // "SFCO" - Scaleform Corporation

    UInt16 Magic;       // Magic marker
    UInt16 UseCount;    // Existing allocation count
    UInt32 Index;       // 2-component index in page tables
    DebugDataMH* DebugHeader; 
#ifndef SF_64BIT_POINTERS
    UInt32 Filler;
#endif

    UInt32  GetIndex0() const { return Index &  PageTableMH::Index0Mask; }
    UInt32  GetIndex1() const { return Index >> PageTableMH::Index1Shift; }
};

//------------------------------------------------------------------------
struct NodeMH
{
    enum { Align4 = 0, Align8 = 1, Align16 = 2, MoreInfo = 3 };

    NodeMH* Parent;
    NodeMH* Child[2];
    UPInt   pHeap;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    DebugDataMH* DebugHeader;
#endif
    //------------------ Optional data
    UPInt   Align;

    static UPInt GetNodeSize(UPInt align)
    {
        UPInt size = 4*sizeof(void*);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        size += sizeof(void*);
#endif
        return (align <= 16) ? size : size + sizeof(UPInt);
    }

    MemoryHeapMH* GetHeap() const 
    { 
        return (MemoryHeapMH*)(pHeap & ~UPInt(3)); 
    }

    UPInt GetAlign() const
    {
        return ((pHeap & 3) < 3) ? (UPInt(1) << ((pHeap & 3) + 2)) : Align;
    }

    void SetHeap(MemoryHeapMH* heap, UPInt align)
    {
        switch(align)
        {
        case 1: case 2: case 4:
            pHeap = UPInt(heap);
            break;

        case 8:
            pHeap = UPInt(heap) | Align8;
            break;

        case 16:
            pHeap = UPInt(heap) | Align16;
            break;

        default:
            pHeap = UPInt(heap) | MoreInfo;
            Align = align;
            break;
        }
    }
};


//--------------------------------------------------------------------
struct TreeNodeAccessor
{
    static       UPInt    GetKey     (const NodeMH* n)          { return  UPInt(n); }
    static       NodeMH*  GetChild   (      NodeMH* n, UPInt i) { return  n->Child[i]; }
    static const NodeMH*  GetChild   (const NodeMH* n, UPInt i) { return  n->Child[i]; }
    static       NodeMH** GetChildPtr(      NodeMH* n, UPInt i) { return &n->Child[i]; }

    static       NodeMH*  GetParent  (      NodeMH* n)          { return n->Parent; }
    static const NodeMH*  GetParent  (const NodeMH* n)          { return n->Parent; }

    static void SetParent (NodeMH* n, NodeMH* p)                { n->Parent   = p; }
    static void SetChild  (NodeMH* n, UPInt i, NodeMH* c)       { n->Child[i] = c; }
    static void ClearLinks(NodeMH* n) { n->Parent = n->Child[0] = n->Child[1] = 0; }
};


//------------------------------------------------------------------------
struct MagicHeadersInfo
{
    MagicHeader* Header1; // Header on the left ptr or 0
    MagicHeader* Header2; // Header on the right ptr or 0
    UInt32*      BitSet;  // Pointer to the bitset
    UByte*       AlignedStart; // Pointer to the beginning of the page, aligned 
    UByte*       AlignedEnd;   // Pointer to the end of the page, aligned 
    UByte*       Bound;   // The bound between physical pages
    PageMH*      Page;    // Page, null after GetMagicHeaders()
};


//------------------------------------------------------------------------
struct PageInfoMH
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    DebugDataMH** DebugHeaders[3];
#endif
    PageMH* Page;
    NodeMH* Node;
    UPInt   UsableSize;
};

//------------------------------------------------------------------------
void GetMagicHeaders(UPInt pageStart, MagicHeadersInfo* headers);


//------------------------------------------------------------------------
class RootMH
{
    typedef RadixTree<NodeMH, TreeNodeAccessor> HeapTreeType;
public:
    RootMH(SysAlloc* sysAlloc);
    ~RootMH();

    void FreeTables();

    SysAlloc* GetSysAlloc() { return pSysAlloc; }

    MemoryHeapMH*  CreateHeap(const char*  name, 
                              MemoryHeapMH* parent,
                              const MemoryHeap::HeapDesc& desc);

    void           DestroyHeap(MemoryHeapMH* heap);

    LockSafe*      GetLock() { return &RootLock; }

    PageMH* AllocPage(MemoryHeapMH* heap);
    void    FreePage(PageMH* page);
    UInt32  GetPageIndex(const PageMH* page) const;

    PageMH* ResolveAddress(UPInt addr) const;

    NodeMH* AddToGlobalTree(UByte* ptr, UPInt size, UPInt align, MemoryHeapMH* heap)
    {
        NodeMH* node = (NodeMH*)(ptr + size);
        node->SetHeap(heap, align);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        node->DebugHeader = 0;
#endif
        HeapTree.Insert(node);
        return node;
    }

    NodeMH* FindNodeInGlobalTree(UByte* ptr)
    {
        return (NodeMH*)HeapTree.FindGrEq(UPInt(ptr));
    }

    void    RemoveFromGlobalTree(NodeMH* node)
    {
        HeapTree.Remove(node);
    }

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    void OnAlloc(const MemoryHeap* heap, UPInt size, UPInt align, unsigned sid, const void* ptr);
    void OnRealloc(const MemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr);
    void OnFree(const MemoryHeap* heap, const void* ptr);
#endif


private:
    bool allocPagePool();
    static void setMagic(UByte* pageStart, unsigned magicValue);
    static void clearMagic(UByte* pageStart);


    SysAlloc*          pSysAlloc;
    mutable LockSafe   RootLock;
    List<PageMH>       FreePages;
    unsigned           TableCount;
    HeapTreeType       HeapTree;
public:
    static MemoryHeap::HeapTracer* pTracer;
};

extern RootMH*      GlobalRootMH;
extern PageTableMH  GlobalPageTableMH;
extern PageMH       GlobalEmptyPage;

}} // Scaleform::HeapMH

#endif
