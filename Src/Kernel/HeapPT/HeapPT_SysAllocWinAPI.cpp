/**************************************************************************

Filename    :   HeapPT_SysAllocWinAPI.cpp
Platform    :   WinAPI, XBox360
Content     :   Win32 System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Win32 System Allocator that uses VirtualAlloc
                and VirualFree.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

//#define SF_MEMORY_LOG_SYSALLOC

#include "HeapPT_SysAllocWinAPI.h"
#include "HeapPT_SysAllocMapper.h"
#include "../SF_HeapNew.h"
#ifndef SF_OS_XBOX360
#include <windows.h>
#include <string.h>
#endif

#ifdef SF_MEMORY_FORCE_SYSALLOC
#include <stdio.h>
#endif


namespace Scaleform {


#ifdef SF_MEMORY_LOG_SYSALLOC

enum { SysAllocPoolSize = 8192 };

struct SysAllocInfo
{
    char  Type; // A=Alloc, F=Free, R=ReallocInPlace
    UPInt Addr; 
    UPInt Size;
    UPInt OldSize;
};

static FILE* SysAllocFile = 0;
static SysAllocInfo SysAllocPool[SysAllocPoolSize];
static int SysAllocCount = 0;


//------------------------------------------------------------------------
static void WriteSysAllocPool()
{
    UByte stream[64];
    for(int i = 0; i < SysAllocCount; ++i)
    {
        const SysAllocInfo info = SysAllocPool[i];
        stream[0] = info.Type;
        memcpy(stream+1,                 &info.Addr,    sizeof(UPInt));
        memcpy(stream+1+1*sizeof(UPInt), &info.Size,    sizeof(UPInt));
        memcpy(stream+1+2*sizeof(UPInt), &info.OldSize, sizeof(UPInt));
        fwrite(stream, 1, (info.Type == 'R') ? 1+3*sizeof(UPInt) : 1+2*sizeof(UPInt), SysAllocFile);
    }
    SysAllocCount = 0;
}

static void WriteSysAllocEntry(char type, const void* ptr, UPInt size, UPInt oldSize)
{
    if (SysAllocCount >= SysAllocPoolSize)
    {
        WriteSysAllocPool();
    }
    SysAllocPool[SysAllocCount].Type = type;
    SysAllocPool[SysAllocCount].Addr = (UPInt)ptr;
    SysAllocPool[SysAllocCount].Size = size;
    SysAllocPool[SysAllocCount].OldSize = oldSize;
    SysAllocCount++;
}
#endif



//------------------------------------------------------------------------
UPInt SysMemMapperWinAPI::GetPageSize() const
{
#ifdef SF_OS_XBOX360
    return 4096;
#else
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return info.dwPageSize;
#endif
}

//------------------------------------------------------------------------
void* SysMemMapperWinAPI::ReserveAddrSpace(UPInt size)
{
    return ::VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

//------------------------------------------------------------------------
bool SysMemMapperWinAPI::ReleaseAddrSpace(void* ptr, UPInt)
{
    return ::VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

//------------------------------------------------------------------------
void* SysMemMapperWinAPI::MapPages(void* ptr, UPInt size)
{
    return ::VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

//------------------------------------------------------------------------
bool SysMemMapperWinAPI::UnmapPages(void* ptr, UPInt size)
{
    return ::VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

//------------------------------------------------------------------------
SysAllocWinAPI::SysAllocWinAPI(UPInt granularity, UPInt segSize) :
    Mapper(),
    pAllocator(::new(PrivateData) SysAllocMapper(&Mapper, segSize, granularity))
{
    SF_COMPILER_ASSERT(sizeof(PrivateData) >= sizeof(SysAllocMapper));

#ifdef SF_MEMORY_LOG_SYSALLOC
    SysAllocFile = fopen("SysAllocLog", "wb");
    memset(SysAllocPool, 0, sizeof(SysAllocPool));
    SysAllocCount = 0;
#endif
}

//------------------------------------------------------------------------
SysAllocWinAPI::~SysAllocWinAPI()
{
#ifdef SF_MEMORY_LOG_SYSALLOC
    WriteSysAllocPool();
    fclose(SysAllocFile);
#endif
}

//------------------------------------------------------------------------
void SysAllocWinAPI::GetInfo(Info* i) const
{
    pAllocator->GetInfo(i);
}

//------------------------------------------------------------------------
void* SysAllocWinAPI::Alloc(UPInt size, UPInt align)
{
#ifdef SF_MEMORY_LOG_SYSALLOC
    void* ptr = pAllocator->Alloc(size, align);
    WriteSysAllocEntry('A', ptr, size, 0);
    return ptr;
#else
    return pAllocator->Alloc(size, align);
#endif
}

//------------------------------------------------------------------------
bool SysAllocWinAPI::ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
{
#ifdef SF_MEMORY_LOG_SYSALLOC
    bool ok = pAllocator->ReallocInPlace(oldPtr, oldSize, newSize, align);
    if (ok)
    {
        WriteSysAllocEntry('R', oldPtr, newSize, oldSize);
    }
    return ok;
#else
    return pAllocator->ReallocInPlace(oldPtr, oldSize, newSize, align);
#endif
}

//------------------------------------------------------------------------
bool SysAllocWinAPI::Free(void* ptr, UPInt size, UPInt align)
{
#ifdef SF_MEMORY_LOG_SYSALLOC
    bool ok = pAllocator->Free(ptr, size, align);
    WriteSysAllocEntry('F', ptr, size, 0);
    return ok;
#else
    return pAllocator->Free(ptr, size, align);
#endif
}

//------------------------------------------------------------------------
UPInt SysAllocWinAPI::GetFootprint() const 
{ 
    return pAllocator->GetFootprint(); 
}

//------------------------------------------------------------------------
UPInt SysAllocWinAPI::GetUsedSpace() const 
{ 
    return pAllocator->GetUsedSpace(); 
}

//------------------------------------------------------------------------
const UInt32* SysAllocWinAPI::GetBitSet(UPInt seg) const // DBG
{
    return pAllocator->GetBitSet(seg);
}

UPInt SysAllocWinAPI::GetBase() const
{
    return pAllocator->GetBase();
}










#ifdef SF_MEMORY_FORCE_SYSALLOC


//------------------------------------------------------------------------
const UInt32 HeapPT_BitSet::HeadBusyTable[16] = 
{
    0xFFFFFFFFU,
    0xFFFFFFFCU,
    0xFFFFFFF0U,
    0xFFFFFFC0U,
    0xFFFFFF00U,
    0xFFFFFC00U,
    0xFFFFF000U,
    0xFFFFC000U,
    0xFFFF0000U,
    0xFFFC0000U,
    0xFFF00000U,
    0xFFC00000U,
    0xFF000000U,
    0xFC000000U,
    0xF0000000U,
    0xC0000000U
};

//------------------------------------------------------------------------
const UInt32 HeapPT_BitSet::TailBusyTable[16] = 
{
    0x00000001U,
    0x00000007U,
    0x0000001FU,
    0x0000007FU,
    0x000001FFU,
    0x000007FFU,
    0x00001FFFU,
    0x00007FFFU,
    0x0001FFFFU,
    0x0007FFFFU,
    0x001FFFFFU,
    0x007FFFFFU,
    0x01FFFFFFU,
    0x07FFFFFFU,
    0x1FFFFFFFU,
    0x7FFFFFFFU
};

//------------------------------------------------------------------------
const UInt32 HeapPT_BitSet::HeadFreeTable[16] = 
{
    0x00000000U,
    0x00000003U,
    0x0000000FU,
    0x0000003FU,
    0x000000FFU,
    0x000003FFU,
    0x00000FFFU,
    0x00003FFFU,
    0x0000FFFFU,
    0x0003FFFFU,
    0x000FFFFFU,
    0x003FFFFFU,
    0x00FFFFFFU,
    0x03FFFFFFU,
    0x0FFFFFFFU,
    0x3FFFFFFFU
};

//------------------------------------------------------------------------
const UInt32 HeapPT_BitSet::TailFreeTable[16] = 
{
    0xFFFFFFFCU,
    0xFFFFFFF0U,
    0xFFFFFFC0U,
    0xFFFFFF00U,
    0xFFFFFC00U,
    0xFFFFF000U,
    0xFFFFC000U,
    0xFFFF0000U,
    0xFFFC0000U,
    0xFFF00000U,
    0xFFC00000U,
    0xFF000000U,
    0xFC000000U,
    0xF0000000U,
    0xC0000000U,
    0x00000000U
};


//------------------------------------------------------------------------
const UByte HeapPT_BitSet::LastBlock[256] = 
{
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 4,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 4,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 4,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 3,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 4
};


//------------------------------------------------------------------------
void HeapPT_BitSet::Clear(UInt32* buf, UPInt numWords)
{
    memset(buf, 0, sizeof(UInt32) * numWords);
}

//------------------------------------------------------------------------
void HeapPT_BitSet::SetBusy(UInt32* buf, UPInt start, UPInt num)
{
    UPInt i;
    UPInt startWord =  start            >> 4;
    UPInt endWord   = (start + num - 1) >> 4;
    UPInt tail      = (start + num - 1) & 15;
    if (endWord > startWord)
    {
        buf[start >> 4] |= HeadBusyTable[start & 15];
        for(i = startWord + 1; i < endWord; ++i)
        {
            buf[i] = ~0U;
        }
        buf[endWord] |= TailBusyTable[tail];
    }
    else
    {
        buf[startWord] |= HeadBusyTable[start & 15] & TailBusyTable[tail];
    }
}


//------------------------------------------------------------------------
void HeapPT_BitSet::SetFree(UInt32* buf, UPInt start, UPInt num)
{
    UPInt i;
    UPInt startWord =  start            >> 4;
    UPInt endWord   = (start + num - 1) >> 4;
    UPInt tail      = (start + num - 1) & 15;
    if (endWord > startWord)
    {
        buf[start >> 4] &= HeadFreeTable[start & 15];
        for(i = startWord + 1; i < endWord; ++i)
        {
            buf[i] = 0;
        }
        buf[endWord] &= TailFreeTable[tail];
    }
    else
    {
        buf[startWord] &= HeadFreeTable[start & 15] | TailFreeTable[tail];
    }
}


//------------------------------------------------------------------------
UPInt HeapPT_BitSet::FindBlockSize(const UInt32* buf, UPInt start)
{
    UPInt  startWord = start >> 4;
    UPInt  size = 0;
    UInt32 mask = HeadBusyTable[start & 15];
    UInt32 bits = buf[startWord] & mask;
    if (bits != mask)
    {
        return FindLastBlockInWord(bits >> (2*start & 30));
    }
    size += 16 - (start & 15);
    while (buf[++startWord] == ~0U)
    {
        size += 16;
    }
    return size + FindLastBlockInWord(buf[startWord]);
}


//------------------------------------------------------------------------
UPInt HeapPT_BitSet::CheckFree(const UInt32* buf, UPInt start, UPInt size)
{
    for(UPInt i = 0; i < size; ++i)
    {
        if (GetValue(buf, start+i) != 0)
            return i;
    }
    return ~UPInt(0);
}


//------------------------------------------------------------------------
UPInt HeapPT_BitSet::FindFreeBlock(const UInt32* buf, UPInt size, UPInt lim)
{
    UPInt start = 0;
    for(;;)
    {
        if (GetValue(buf, start) == 0)
        {
            UPInt free = CheckFree(buf, start, size);
            if (free == ~UPInt(0))
                return start;
            start += free;
        }
        else
        {
            start += FindBlockSize(buf, start);
            if (start >= lim)
                break;
        }
    }
    return 0;
}


#ifdef SF_64BIT_POINTERS
#define HeapPT_Reserve 4294967296LL
#else
#define HeapPT_Reserve 1073741824
#endif
enum 
{ 
    HeapPT_BitSize = HeapPT_Reserve / (4096 * 4)
};

//------------------------------------------------------------------------
HeapPT_SysAlloc::HeapPT_SysAlloc() :
    pBitSet(0),
    pMemory(0),
    Mapper()
{
}


//------------------------------------------------------------------------
void HeapPT_SysAlloc::Init()
{
    Lock::Locker lock(&mLock);
    pBitSet = (UInt32*)Mapper.ReserveAddrSpace(HeapPT_BitSize);
    pBitSet = (UInt32*)Mapper.MapPages(pBitSet, HeapPT_BitSize);
    memset(pBitSet, 0, HeapPT_BitSize);
    HeapPT_BitSet::SetBusy(pBitSet, HeapPT_BitSize*4-1, 1);
    pMemory = (UByte*)Mapper.ReserveAddrSpace(HeapPT_Reserve);
}

//------------------------------------------------------------------------
HeapPT_SysAlloc::~HeapPT_SysAlloc()
{
    Lock::Locker lock(&mLock);
    Mapper.ReleaseAddrSpace(pMemory, HeapPT_Reserve);
    Mapper.UnmapPages(pBitSet, HeapPT_BitSize);
    Mapper.ReleaseAddrSpace(pBitSet, HeapPT_BitSize);
}


//------------------------------------------------------------------------
void* HeapPT_SysAlloc::Alloc(UPInt size, UPInt align)
{
    if (size == 0)
        size = 1;

    size = (size + align-1) & ~(align-1);
    UPInt size4k = (size + 4095) & ~UPInt(4095);

    Lock::Locker lock(&mLock);
    UPInt start = 0;
    {
        start = HeapPT_BitSet::FindFreeBlock(pBitSet, (size4k / 4096) + 1, HeapPT_BitSize*4-1);
        start++; // Empty guard page
        HeapPT_BitSet::SetBusy(pBitSet, start, size4k / 4096);
    }

    void* ptr = Mapper.MapPages(pMemory + start * 4096, size4k);
    memset(ptr, 0xAA, size4k);

    // Move ptr so that the allocation will be at the end of the block
    UPInt addr = UPInt(ptr) + size4k - size;
    SF_ASSERT((addr & (align-1)) == 0);
    SF_ASSERT((addr & ~UPInt(4095)) == UPInt(ptr));

    return (void*)addr;
}

//------------------------------------------------------------------------
void  HeapPT_SysAlloc::Free(void* ptr)
{
    if (ptr == 0)
        return;

    ptr = (void*)(UPInt(ptr) & ~UPInt(4095));

    Lock::Locker lock(&mLock);
    UByte* pb = (UByte*)ptr;
    UPInt start = 0;
    UPInt size = 0;
    start = (pb - pMemory) / 4096;
    size  = HeapPT_BitSet::FindBlockSize(pBitSet, start);
    HeapPT_BitSet::SetFree(pBitSet, start, size);
    Mapper.UnmapPages(ptr, size * 4096);
}

//------------------------------------------------------------------------
void* HeapPT_SysAlloc::Realloc(void* oldPtr, UPInt newSize)
{
    if (oldPtr == 0)
        return Alloc(newSize, 4096);

    void* oldPtr4k = (void*)(UPInt(oldPtr) & ~UPInt(4095));
    void* newPtr = 0;

    {
        Lock::Locker lock(&mLock);
        UPInt bitStart = ((UByte*)oldPtr4k - pMemory) / 4096;
        UPInt oldSize  = HeapPT_BitSet::FindBlockSize(pBitSet, bitStart);

        newPtr = Alloc(newSize, 8);

        oldSize *= 4096;
        UPInt copySize = (newSize < oldSize) ? newSize : oldSize;
        UPInt sizeLim = UPInt(oldPtr4k) + oldSize - UPInt(oldPtr);
        if (copySize > sizeLim)
            copySize = sizeLim;
        memcpy(newPtr, oldPtr, copySize);
    }
    Free(oldPtr);
    return newPtr;
}

#endif //SF_MEMORY_FORCE_SYSALLOC




} // Scaleform
