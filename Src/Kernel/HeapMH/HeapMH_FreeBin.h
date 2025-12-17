/**************************************************************************

Filename    :   HeapMH_FreeBin.h
Content     :   
Created     :   2010
Authors     :   Maxim Shemanarev

Notes       :   Containers to store information about free memory areas

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapMH_FreeBin_H
#define INC_SF_Kernel_HeapMH_FreeBin_H

#include "HeapMH_Root.h"

namespace Scaleform { namespace HeapMH {

//UInt32 Magic;
//UInt32 Index;
//UByte  UseCount;
//SByte  BitSet;
//UByte  DataStart;
//UByte  DataEnd;


#ifdef SF_64BIT_POINTERS

struct BinNodeMH
{
    enum { MinBlocks = 2 };
    UPInt   Prev;
    UPInt   Next;
    PageMH* Page;  

    UPInt GetBlocks() const
    { 
        return (Prev & 0xF) | ((Next & 0xF) << 4);
    }

    UPInt GetBytes() const 
    { 
        return GetBlocks() << PageMH::UnitShift; 
    }

    UPInt GetPrevBlocks() const
    { 
        const UPInt* tail  = ((const UPInt*)this) - 2;
        return (tail[0] & 0xF) | ((tail[1] & 0xF) << 4);
    }

    UPInt GetPrevBytes() const
    { 
        return GetPrevBlocks() << PageMH::UnitShift; 
    }

    void SetBlocks(UPInt blocks)
    { 
        UPInt bytes = blocks << PageMH::UnitShift;
        UPInt* tail  = ((UPInt*)this) + bytes / sizeof(UPInt) - 2;
        tail[0] = Prev = (Prev & ~UPInt(0xF)) | (blocks & 0xF);
        tail[1] = Next = (Next & ~UPInt(0xF)) | (blocks >> 4);
    }

    void SetBytes(UPInt bytes)
    { 
        UPInt blocks = bytes >> PageMH::UnitShift;
        UPInt* tail  = ((UPInt*)this) + bytes / sizeof(UPInt) - 2;
        tail[0] = Prev = (Prev & ~UPInt(0xF)) | (blocks & 0xF);
        tail[1] = Next = (Next & ~UPInt(0xF)) | (blocks >> 4);
    }

    PageMH* GetPage()
    {
        return (GetBlocks() > MinBlocks) ? Page : 0;
    }

    void SetPage(PageMH* page)
    {
        if (GetBlocks() > MinBlocks)
            Page = page;
    }

    BinNodeMH*  GetPrev() const { return (BinNodeMH*)(Prev & ~UPInt(0xF)); }
    BinNodeMH*  GetNext() const { return (BinNodeMH*)(Next & ~UPInt(0xF)); }

    void        SetPrev(BinNodeMH* prev) { Prev = UPInt(prev) | (Prev & 0xF); }
    void        SetNext(BinNodeMH* next) { Next = UPInt(next) | (Next & 0xF); }

    static BinNodeMH* MakeNode(UByte* start, UPInt bytes, PageMH* page)
    {
        BinNodeMH* node = (BinNodeMH*)start;
        node->SetBytes(bytes);
        node->SetPage(page);
        return node;
    }
};

#else // SF_64BIT_POINTERS

struct BinNodeMH
{
    UPInt   Prev;
    UPInt   Next;
    PageMH* Page;  

    UPInt GetBlocks() const
    { 
        return ((const UByte*)this)[sizeof(UPInt) * 3];
    }

    UPInt GetBytes() const 
    { 
        return GetBlocks() << PageMH::UnitShift; 
    }

    UPInt GetPrevBlocks() const
    { 
        const UByte* tail  = ((const UByte*)this) - 1;
        return *tail;
    }

    UPInt GetPrevBytes() const
    { 
        return GetPrevBlocks() << PageMH::UnitShift; 
    }

    void SetBlocks(UPInt blocks)
    { 
        UPInt bytes = blocks << PageMH::UnitShift;
        UByte* head  = ((UByte*)this) + sizeof(UPInt) * 3;
        UByte* tail  = ((UByte*)this) + bytes - 1;
        *head = *tail = (UByte)blocks;
    }

    void SetBytes(UPInt bytes)
    { 
        UPInt blocks = bytes >> PageMH::UnitShift;
        UByte* head  = ((UByte*)this) + sizeof(UPInt) * 3;
        UByte* tail  = ((UByte*)this) + bytes - 1;
        *head = *tail = (UByte)blocks;
    }

    PageMH* GetPage()
    {
        return Page;
    }

    void SetPage(PageMH* page)
    {
        Page = page;
    }

    BinNodeMH*  GetPrev() const { return (BinNodeMH*)Prev; }
    BinNodeMH*  GetNext() const { return (BinNodeMH*)Next; }

    void        SetPrev(BinNodeMH* prev) { Prev = UPInt(prev); }
    void        SetNext(BinNodeMH* next) { Next = UPInt(next); }

    static BinNodeMH* MakeNode(UByte* start, UPInt bytes, PageMH* page)
    {
        BinNodeMH* node = (BinNodeMH*)start;
        node->SetBytes(bytes);
        node->SetPage(page);
        return node;
    }
};

#endif // SF_64BIT_POINTERS


//------------------------------------------------------------------------
struct ListBinMH
{
    enum { BinSize = 8*sizeof(UPInt) }; // Assume Byte is 8 bits.

    ListBinMH();
    void Reset();

    //--------------------------------------------------------------------
    void        Push(UByte* node);
    void        Pull(UByte* node);

    BinNodeMH*  PullBest(UPInt blocks);
    BinNodeMH*  PullBest(UPInt blocks, UPInt alignMask);

    void        Merge(UByte* node, UPInt bytes, bool left, bool right, PageMH* page);

    static UByte* GetAlignedPtr(UByte* start, UPInt alignMask);
    static bool   AlignmentIsOK(const BinNodeMH* node, UPInt blocks, UPInt alignMask);

private:
    void        pushNode(UPInt idx, BinNodeMH* node);
    void        pullNode(UPInt idx, BinNodeMH* node);

    BinNodeMH*  findAligned(BinNodeMH* root, UPInt blocks, UPInt alignMask);


    BinNodeMH*  getPrevAdjacent(UByte* node) const;
    BinNodeMH*  getNextAdjacent(UByte* node) const;

    //--------------------------------------------------------------------
    UPInt       Mask;
    BinNodeMH*  Roots[BinSize];
};



}} // Scaleform::HeapMH

#endif
