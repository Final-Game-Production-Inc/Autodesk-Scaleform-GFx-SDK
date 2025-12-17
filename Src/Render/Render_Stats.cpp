/**************************************************************************

Filename    :   Render_Stats.cpp
Content     :   Rendering statistics declarations
Created     :   April 27, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Stats.h"
#include "Render/Render_HAL.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Random.h"

namespace Scaleform {

// ***** Renderer Stat Constants

// GRenderer Memory.
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatRender_Mem,    "Renderer", Stat_Mem)

SF_DECLARE_MEMORY_STAT(StatRender_MeshStaging_Mem,  "MeshStagingBuffer", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_RenderPipeline_Mem,"RenderPipelineManagement", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_MeshCacheMgmt_Mem,"MeshCacheManagement", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Context_Mem,      "Context", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_NodeData_Mem,     "NodeData", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_TreeCache_Mem,    "TreeCache", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_TextureManager_Mem, "TextureManager", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_MatrixPool_Mem,    "MatrixPool", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Text_Mem,          "Text", StatRender_Mem)
SF_DECLARE_MEMORY_STAT(StatRender_Font_Mem,          "Font", StatRender_Mem)

void Link_RenderStats()
{
}

} // Scaleform

