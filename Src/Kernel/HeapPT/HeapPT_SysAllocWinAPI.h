/**************************************************************************

PublicHeader:   Kernel
Filename    :   HeapPT_SysAllocWinAPI.h
Platform    :   WinAPI, XBox360
Content     :   Win32/XBox360 System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Win32 System Allocator that uses VirtualAlloc
                and VirualFree.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_SysAllocWinAPI_H
#define INC_SF_Kernel_HeapPT_SysAllocWinAPI_H

#include "../SF_SysAlloc.h"
#include "../SF_Threads.h"
#if (defined SF_OS_XBOX360 || defined SF_OS_WIN32)
namespace Scaleform
{

// ***** SysMemMapperWinAPI
//
// Memory mapper for Windows API. Uses VirtualAlloc/VirtualFree.
//------------------------------------------------------------------------
class SysMemMapperWinAPI : public SysMemMapper
{
public:
    virtual UPInt   GetPageSize() const;

    // Reserve and release address space. The functions must not allocate
    // any actual memory. The size is typically very large, such as 128 
    // megabytes. ReserveAddrSpace() returns a pointer to the reserved
    // space; there is no usable memory is allocated.
    //--------------------------------------------------------------------
    virtual void*   ReserveAddrSpace(UPInt size);
    virtual bool    ReleaseAddrSpace(void* ptr, UPInt size);

    // Map and unmap memory pages to allocate actual memory. The caller 
    // guarantees the integrity of the calls. That is, the the size is 
    // always a multiple of GetPageSize() and the ptr...ptr+size is always 
    // within the reserved address space. Also it's guaranteed that there 
    // will be no MapPages call for already mapped memory and no UnmapPages 
    // for not mapped memory.
    //--------------------------------------------------------------------
    virtual void*   MapPages(void* ptr, UPInt size);
    virtual bool    UnmapPages(void* ptr, UPInt size);
};


// ***** SysAllocWinAPI
//------------------------------------------------------------------------

// SysAllocWinAPI provides a default system allocator implementation for Windows
// and Xbox360 platforms, relying on VirtualAlloc and VirtualFree APIs.
//
// If possible, we recommend that developers use this interface directly instead
// of providing their own SysAlloc implementation. Since SysAllocWinAPI relies on
// the low-level OS functions, it intelligently considers both system page size and
// granularity, allowing it to be particularly alignment friendly. Due to this alignment
// efficiency, SysAllocWinAPI does not lose any memory on 4K alignment required
// internally by MemoryHeap. No alignment overhead means that SysAllocWinAPI uses
// memory and address space with maximum efficiency, making it the best choice for
// the GFx system allocator on Microsoft platforms.

class SysAllocWinAPI : public SysAllocBase_SingletonSupport<SysAllocWinAPI, SysAllocPaged>
{
    // Default segment size to reserve address space. Actual segments
    // can have different size, for example, if a larger continuous
    // block is requested. Or, in case of bad fragmentation, when 
    // ReserveAddrSpace() fails, the mapper can reduce segment size.
    enum { SegSize = 128*1024*1024 };
public:

    // Initializes system allocator.
    SF_EXPORT       SysAllocWinAPI(UPInt granularity = 64*1024, UPInt segSize = SegSize);
    virtual        ~SysAllocWinAPI();

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetUsedSpace() const;

    SF_EXPORT const UInt32* GetBitSet(UPInt seg) const; // DBG
    virtual UPInt           GetBase() const;            // DBG

private:
    SysMemMapperWinAPI      Mapper;
    class SysAllocMapper*   pAllocator;
    UPInt                   PrivateData[128];

};








#ifdef SF_MEMORY_FORCE_SYSALLOC

//------------------------------------------------------------------------
class HeapPT_BitSet
{
public:
    static void Clear(UInt32* buf, UPInt numWords);

    SF_INLINE static unsigned GetValue(const UInt32* buf, UPInt num)
    {
        return (buf[num >> 4] >> (2*num & 30)) & 3;
    }

    SF_INLINE static UPInt FindLastBlockInWord(UInt32 bits)
    {
        if ((bits & 0xFFFF) != 0xFFFF)
        {
            return ((bits & 0xFF) == 0xFF) ?
                LastBlock[(bits >> 8) & 0xFF] + 4 :
                LastBlock[ bits       & 0xFF];
        }
        return ((bits & 0xFFFFFF) == 0xFFFFFF) ?
            LastBlock[(bits >> 24) & 0xFF] + 12 :
            LastBlock[(bits >> 16) & 0xFF] + 8;
    }

    static void  SetBusy(UInt32* buf, UPInt start, UPInt num);
    static void  SetFree(UInt32* buf, UPInt start, UPInt num);
    static UPInt FindBlockSize(const UInt32* buf, UPInt start);
    static UPInt FindFreeBlock(const UInt32* buf, UPInt size, UPInt lim);
    static UPInt CheckFree(const UInt32* buf, UPInt start, UPInt size);


private:
    static const UInt32 HeadBusyTable[16];
    static const UInt32 TailBusyTable[16];
    static const UInt32 HeadFreeTable[16];
    static const UInt32 TailFreeTable[16];
    static const UByte  LastBlock[256];
};




//------------------------------------------------------------------------
class HeapPT_SysAlloc
{
public:
    HeapPT_SysAlloc();
    ~HeapPT_SysAlloc();

    void Init();

    void* Alloc(UPInt size, UPInt align);
    void  Free(void* ptr);
    void* Realloc(void* ptr, UPInt newSize);

private:
    UInt32*             pBitSet;
    UByte*              pMemory;
    SysMemMapperWinAPI  Mapper;
    Lock                mLock;
};

#endif //SF_MEMORY_FORCE_SYSALLOC





} // Scaleform

#endif      // (defined SF_OS_XBOX360 || defined SF_OS_WIN32)
#endif
