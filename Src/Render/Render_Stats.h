/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Stats.h
Content     :   Color transform for renderer.
Created     :   April 27, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Stats_H
#define INC_SF_Render_Stats_H

#include "Kernel/SF_Stats.h"

namespace Scaleform {

enum GStatRenderer
{
    StatRender_Default = StatGroup_Renderer,

    StatRender_Mem,
        StatRender_MeshStaging_Mem,
        StatRender_RenderPipeline_Mem,
        StatRender_MeshCacheMgmt_Mem,
        StatRender_Context_Mem,
        StatRender_NodeData_Mem,
        StatRender_TreeCache_Mem,
        StatRender_TextureManager_Mem,
        StatRender_MatrixPool_Mem,
        StatRender_Text_Mem,
        StatRender_Font_Mem
};

void    Link_RenderStats();

} // Scaleform


#endif
