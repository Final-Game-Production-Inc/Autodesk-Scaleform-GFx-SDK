/**************************************************************************

Filename    :   FxPlayerAlloc.h
Content     :   Sample system based GMemoryHeap allocator class that can
                be installed in FxPlayer to show overriding allocators.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Enable the memory tracking in the GFxPlayer.
// Comment this line out to prevent memory tracking.
//#define GFC_MEMORY_TRACKSIZES

#include "GMemoryHeap.h"

// Define this macro to enable the use of a custom sample
// SysAllocator class provided below.
#define  FXPLAYER_USE_SYSALLOCATOR

#ifndef FXPLAYER_USE_SYSALLOCATOR

    // If not using our allocator, memory tracking is only available
    // if GMemory was compiled with GFC_MEMORY_TRACKSIZES.
    #ifdef GFC_MEMORY_TRACKSIZES
        #define FXPLAYER_MEMORY_TRACKSIZES
    #endif

#else

// We want to track memory use.
#define FXPLAYER_MEMORY_TRACKSIZES


// ***** SysAllocator - Sample custom allocator for GFxPlayer

// Sample custom allocator. Delegates allocation to standard library
// malloc/free/realloc functions and adds memory counters on top.
class GSysMemoryHeap : public GMEMORY_HEAP_BASE
{
public:

    GAllocatorStats  Stats;

    SysAllocator()
    {
        GMemUtil::Set(&Stats, 0, sizeof(GAllocatorStats));
    }
    const GAllocatorStats* GetStats() const
    {
        return &Stats;
    }

    void    Free(void *pmem)
    {
        // System allocator & tracking option
#ifdef  FXPLAYER_MEMORY_TRACKSIZES
        // Have to check for null - by standard
        if (!pmem) return;
        Stats.FreeCount++;
        UPInt sizeFreed = *(((UPInt*)pmem)-1);
        Stats.FreeSize += sizeFreed;
        pmem = (((UPInt*)pmem)-1);
#endif
        free(pmem);
    }

    // align not needed
    void*   Alloc(UPInt size, UPInt align, UPInt extra)
    {
#ifdef FXPLAYER_MEMORY_TRACKSIZES
        void *pret;

        Stats.AllocCount++;
        pret = malloc(size+extra+sizeof(UPInt));
        if (pret)
        {
            Stats.AllocSize += size;
            *((UPInt*)pret) = size;
            return ((UPInt*)pret)+1;
        }
        return 0;

#else // NO MEMORY TRACKING
        return malloc(size+extra);
#endif
    }

    void*   Realloc(void *pmem, UPInt size)
    {
#ifdef  FXPLAYER_MEMORY_TRACKSIZES
        Stats.ReallocCount++;
        UPInt oldSize = pmem ? *((UPInt*)pmem-1) : 0;
        void *pret =  realloc(pmem ? ((UPInt*)pmem-1) : NULL,size + sizeof(UPInt));
        if (pret)
        {
            if (oldSize>size)
                Stats.FreeSize += (oldSize - size);
            else
                Stats.AllocSize += (size - oldSize);
            *((UPInt*)pret) = size;
            return ((UPInt*)pret) + 1;
        }
        return 0;
#else
        return realloc(pmem,size);
#endif
    }

};

#endif

