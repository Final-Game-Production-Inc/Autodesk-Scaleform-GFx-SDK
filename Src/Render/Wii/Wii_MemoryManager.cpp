/**************************************************************************

Filename    :   Wii_MemoryManager.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Wii_MemoryManager.h"
#include "Kernel/SF_Debug.h"


namespace Scaleform { namespace Render { namespace Wii {


MemoryManager::MemoryManager(MemoryHeap* pheap, void* mem1Start, void* mem1End) : Mem1(pheap), Mem1Start(mem1Start), Mem1End(mem1End)
{
    UPInt mem1Size32 = ((UByte*)mem1End - (UByte*)mem1Start) >> Shift;
    Mem1.AddSegment(UPInt(mem1Start) >> Shift, mem1Size32);
}

MemoryManager::~MemoryManager()
{
}

void*   MemoryManager::Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena)
{
    switch (type)
    {
    default:
        SF_DEBUG_WARNING1(1, "Wii::MemoryManager, unknown memory type %x, using main", type);
        //return SF_MEMALIGN(size, align, Stat_Default_Mem);

    case Memory_Wii_MEM1:
        return (void*)allocShifted(size, align);
        //return OSAlloc(size);
    }
}

void    MemoryManager::Free(void* pmem, UPInt size)
{
    //OSFree(pmem);
    if (pmem >= Mem1Start && pmem < Mem1End)
        freeShifted((UPInt)pmem, size);
    else
        SF_FREE(pmem);
}

}}}
