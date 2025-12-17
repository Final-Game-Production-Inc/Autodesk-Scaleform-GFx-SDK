/**************************************************************************

PublicHeader:   Render
Filename    :   Render_MemoryManager.h
Content     :   Base MemoryManager used by console RenderHALs.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_MemoryManager_H
#define INC_SF_Render_MemoryManager_H

// Include Image.h since it defines TextureManager.
#include "Render_Image.h"

namespace Scaleform { namespace Render {


// MemoryType describes the type of memory allocation being requested.
// Memory_Normal should be implemented on all platforms, while platform-specific
// types only need to be handled on platforms for which they are defined.

enum MemoryType
{
    Memory_Normal               = 0,
    Memory_Uncached             = 0x0100,

    Memory_X360_Physical        = 0x1000,
    Memory_X360_EDRAM           = 0x1001,

    Memory_PS3_MainVideo        = 0x2000,
    Memory_PS3_RSX              = 0x2002,

    Memory_PSVita_Main          = 0x3000,
    Memory_PSVita_MainUncached  = 0x3100,
    Memory_PSVita_CDRAM         = 0x3002,
    Memory_PSVita_BlockMask     = 0xFF0F,
    Memory_PSVita_Vertex        = 0x3012,
    Memory_PSVita_Fragment      = 0x3022,
    Memory_PSVita_Shader        = 0x3032,

    Memory_3DS_Device           = 0x4000,
    Memory_3DS_VideoA           = 0x4001,
    Memory_3DS_VideoB           = 0x4002,

    Memory_Wii_MEM1             = 0x5000,

    Memory_Orbis_Start          = 0x6000,

    Memory_Orbis_WB_ONION_VOLATILE = Memory_Orbis_Start,    // Volatile/Nonvolatile are for 0.915 and below.
    Memory_Orbis_WB_ONION_NONVOLATILE,
    Memory_Orbis_WC_GARLIC_VOLATILE,
    Memory_Orbis_WC_GARLIC_NONVOLATILE,
    Memory_Orbis_WT_ONION_VOLATILE,
    Memory_Orbis_WT_ONION_NONVOLATILE,
    Memory_Orbis_WP_ONION_VOLATILE,
    Memory_Orbis_WP_ONION_NONVOLATILE,
    Memory_Orbis_UC_GARLIC_VOLATILE,
    Memory_Orbis_UC_GARLIC_NONVOLATILE,

    Memory_Orbis_WB_ONION,                                     // For 0.920 and above.
    Memory_Orbis_WC_GARLIC,
};

// ***** MemoryManager

// Renderer MemoryManager is responsible for allocating and freeing video memory,
// including textures. Implementation of this class will typically only be provided
// on consoles, as PC needs to allocate different resources separately.
// The only unified allocation interface is provided by TextureManager, which
// should work with all memory managers.

// TBD:
// In the future this API can be extended with additional functionality
// to support alternative memory management strategies. Possible functionality:
//  - Make allocations movable, requiring them to be locked for access.
//  - If so, report policy: QuickLock, ReadableBuffer, Movable
//  - Notify of Unload Complete (can be used to defrag).
//  - API to move buffer memory (in case of shrink).

class MemoryManager : public NewOverrideBase<Stat_Default_Mem>
{
public:
    virtual ~MemoryManager() {}

    // *** Alloc/Free

    // Allocates renderer memory of specified type.
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0) = 0;
    virtual void    Free(void* pmem, UPInt size) = 0;

};


}}; // namespace Scaleform::Render

#endif // INC_SF_Render_MemoryManager_H
