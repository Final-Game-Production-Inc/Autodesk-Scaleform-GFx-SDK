/**************************************************************************

Filename    :   X360_MemoryManager.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_X360_MemoryManager_H
#define INC_SF_Render_X360_MemoryManager_H

#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_AllocAddr.h"

namespace Scaleform { namespace Render { namespace X360 {

class MemoryManager : public Render::MemoryManager
{
public:
    // *** Alloc/Free

    // Allocates renderer memory of specified type.
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0);
    virtual void    Free(void* pmem, UPInt size);
};

}}}

#endif
