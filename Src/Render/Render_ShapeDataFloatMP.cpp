/**************************************************************************

Filename    :   Render_ShapeDataFloatMP.cpp
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_ShapeDataFloatMP.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

ShapeDataFloatMP::ShapeDataFloatMP() : 
    ShapeMeshProvider()
{
    pData = *SF_HEAP_AUTO_NEW(this) ShapeDataFloat();
}

void ShapeDataFloatMP::CountLayers()
{
    pData->EndShape();
    AttachShape(pData);
}

}} // Scaleform::Render
