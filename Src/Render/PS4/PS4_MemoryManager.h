/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_MemoryManager.h
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_MemoryManager_H
#define INC_SF_PS4_MemoryManager_H

#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_Hash.h"

#include <gnm.h>

namespace Scaleform { namespace Render { namespace PS4 {

class MemoryManager : public Render::MemoryManager
{
public:
    MemoryManager();
    virtual ~MemoryManager();

    // *** Alloc/Free
    // Allocates renderer memory of specified type.
    virtual void*   Alloc(sce::Gnm::SizeAlign sizeAlign, MemoryType type, unsigned arena = 0);
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0);
    virtual void    Free(void* pmem, UPInt size);
    virtual void    EndFrame();

protected:
    SceKernelMemoryType getSceMemoryType(MemoryType type);
    Hash<UPInt, UPInt>  DirectMemoryMappingHash;
    Lock                MemoryManagerLock;
    UPInt               FrameCounter;

    struct MemoryManagerFreeBlock
    {
    public:
        UPInt   FreeFrame;
        UPInt   Size;
        void*   Address;
    };
    typedef ArrayConstPolicy<0, 8, true> NeverShrinkPolicy;
    typedef ArrayLH<MemoryManagerFreeBlock, Stat_Default_Mem, NeverShrinkPolicy> MemoryManagerFreeListType;
    static const int FreeFrameDelay = 10;

    MemoryManagerFreeListType   FreeList;
};

}}}

#endif // INC_SF_Orbis_MemoryManager_H
