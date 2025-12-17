/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_MemoryManager.cpp
Content     :   
Created     :   2012/09/21
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Kernel/SF_Debug.h"

#include "Render/PS4/PS4_MemoryManager.h"

#include <scebase_common.h>
#include <gnm.h>

namespace Scaleform { namespace Render { namespace PS4 {

MemoryManager::MemoryManager() :
    FrameCounter(0)
{
}

MemoryManager::~MemoryManager()
{
    // GPU flush, just to make sure all memory has been already accessed.
    sce::Gnm::flushGarlic();

    // Simulate a bunch of end-frames, which will clear all remaining allocated kernel memory.
    for (unsigned frame = 0; frame < FreeFrameDelay; ++frame)
        EndFrame();

    SF_DEBUG_ASSERT(FreeList.GetSize() == 0, "Expected no pending memory blocks to be freed.");
}

void* MemoryManager::Alloc(sce::Gnm::SizeAlign sizeAlign, MemoryType type, unsigned arena)
{
    return Alloc(sizeAlign.m_size, sizeAlign.m_align, type, arena);
}

void* MemoryManager::Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena)
{
    Lock::Locker scopedLock(&MemoryManagerLock);

    // Allocate the new memory block.
    off_t newMemoryBlock = 0;
    size_t newBlockSize = Alg::Align(size, 16 * 1024); // Must be a multiple of 16kb.


#if defined(SCE_ORBIS_SDK_VERSION) && (SCE_ORBIS_SDK_VERSION < 0x00930020u) // deprecated in 0.930.
    int returnValue = sceKernelAllocateDirectMemory(SCE_KERNEL_MAIN_DMEM_OFFSET, 
                        (SCE_KERNEL_MAIN_DMEM_OFFSET + SCE_KERNEL_MAIN_DMEM_SIZE),
                        newBlockSize,
                        Alg::Max<size_t>(16 * 1024, align),
                        getSceMemoryType(type),
                        &newMemoryBlock);
#else
    int returnValue = sceKernelAllocateDirectMemory(0, 
        (SCE_KERNEL_MAIN_DMEM_SIZE),
        newBlockSize,
        Alg::Max<size_t>(16 * 1024, align),
        getSceMemoryType(type),
        &newMemoryBlock);
#endif

    SF_DEBUG_ASSERT1(returnValue == SCE_OK, "sceKernelAllocateDirectMemory error (%d)", returnValue);
    if (returnValue != SCE_OK)
        return (void*)(~UPInt(0));

    // Now map it.
    void* mapAddress = 0;
    returnValue = sceKernelMapDirectMemory(&reinterpret_cast<void*&>(mapAddress),
        newBlockSize,
        SCE_KERNEL_PROT_CPU_READ|SCE_KERNEL_PROT_CPU_WRITE|SCE_KERNEL_PROT_GPU_ALL,
        0,
        newMemoryBlock,
        Alg::Max<size_t>(16*1024, align));
    SF_DEBUG_ASSERT1(returnValue == SCE_OK, "sceKernelMapDirectMemory error (%d)", returnValue);
    if (returnValue != SCE_OK)
    {
        sceKernelReleaseDirectMemory(newMemoryBlock, size);
        return (void*)(~UPInt(0));
    }

    // Add it to our mapping hash, so that we know where to deallocate it from when it is released.
    DirectMemoryMappingHash.Add(reinterpret_cast<UPInt>(mapAddress), newMemoryBlock);

    return mapAddress;        
}

void MemoryManager::Free(void* pmem, UPInt size)
{
    Lock::Locker scopedLock(&MemoryManagerLock);
    MemoryManagerFreeBlock freeBlock = { FrameCounter, size, pmem };
    FreeList.PushBack(freeBlock);
}

void MemoryManager::EndFrame()
{
    Lock::Locker scopedLock(&MemoryManagerLock);
    FrameCounter++;
    unsigned entry;
    for (entry = 0; entry < FreeList.GetSize(); )
    {
        if (FreeList[entry].FreeFrame < FrameCounter - FreeFrameDelay)
        {
            // Try to find it in the hash.
            UPInt directAddress = 0;
            UPInt size = FreeList[entry].Size;
            void* pmem = FreeList[entry].Address;
            if (!DirectMemoryMappingHash.Get(reinterpret_cast<UPInt>(pmem), &directAddress))
            {
                SF_DEBUG_ASSERT2(0, "Could not find memory in direct address -> mapping hash (mem=%016lx, size=%ld)", 
                    reinterpret_cast<UPInt>(pmem), size);
                return;
            }

            DirectMemoryMappingHash.Remove(reinterpret_cast<UPInt>(pmem));
            size_t freeSize = Alg::Align(size, 16*1024);
            sceKernelReleaseDirectMemory(directAddress, freeSize);

            FreeList.RemoveAt(entry);
        }
        else
        {
            ++entry;
        }
    }
}

SceKernelMemoryType MemoryManager::getSceMemoryType(MemoryType type)
{
#if defined(SCE_ORBIS_SDK_VERSION) && (SCE_ORBIS_SDK_VERSION < 0x00930020u) // deprecated in 0.930.
    SceKernelMemoryType sceType = (SceKernelMemoryType)(type - Memory_Orbis_Start);
    SF_DEBUG_ASSERT1(sceType >= SCE_KERNEL_WB_ONION_VOLATILE && sceType < SCE_KERNEL_MEMORY_TYPE_END,
        "Invalid memory type specified (%d)", type);
    return sceType;
#else
    // Map 0.915 memory types to 0.920+ ones.
    switch(type)
    {
        default:
        case Memory_Orbis_WB_ONION_VOLATILE:
        case Memory_Orbis_WB_ONION_NONVOLATILE:
        case Memory_Orbis_WT_ONION_VOLATILE:
        case Memory_Orbis_WT_ONION_NONVOLATILE:
        case Memory_Orbis_WP_ONION_VOLATILE:
        case Memory_Orbis_WP_ONION_NONVOLATILE:
            return SCE_KERNEL_WB_ONION;
        case Memory_Orbis_WC_GARLIC_VOLATILE:
        case Memory_Orbis_WC_GARLIC_NONVOLATILE:
        case Memory_Orbis_UC_GARLIC_VOLATILE:
        case Memory_Orbis_UC_GARLIC_NONVOLATILE:
            return SCE_KERNEL_WC_GARLIC;
    }
#endif
}

}}}; // Scaleform::Render::Orbis
