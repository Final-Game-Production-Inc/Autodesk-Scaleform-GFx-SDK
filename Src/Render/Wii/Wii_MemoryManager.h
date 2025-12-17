/**************************************************************************

Filename    :   Wii_MemoryManager.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Wii_MemoryManager_H
#define INC_SF_Render_Wii_MemoryManager_H

#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_AllocAddr.h"

namespace Scaleform { namespace Render { namespace Wii {

class MemoryManager : public Render::MemoryManager
{
    AllocAddr Mem1;
    void*     Mem1Start;
    void*     Mem1End;
    enum {
        Shift = 5
    };

    UPInt allocShifted(UPInt size, UPInt align)
    {
        SF_ASSERT(align <= (1 << Shift));
        UPInt addr = Mem1.Alloc((size + (1 << Shift) - 1) >> Shift);
        
        // If the allocation fails, return zero. This should be valid on Wii, because
        // its memory address space shouldn't begin at zero. Alloc fail checks in Wii
        // check for 0.
        if ( addr == ~UPInt(0))
        	return 0;
        return addr << Shift;
        	
    }
    void freeShifted(UPInt mem, UPInt size)
    {
        Mem1.Free(mem >> Shift, (size + (1 << Shift) - 1) >> Shift);
    }

public:
    MemoryManager(MemoryHeap* pheap, void* mem1Start, void* mem1End);
    ~MemoryManager();

    // *** Alloc/Free

    // Allocates renderer memory of specified type.
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0);
    virtual void    Free(void* pmem, UPInt size);
};

}}}

#endif
