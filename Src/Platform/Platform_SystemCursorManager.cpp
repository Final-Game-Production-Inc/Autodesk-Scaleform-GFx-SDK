/**************************************************************************

Filename    :   Platform_SystemCursorManager.cpp
Content     :   
Created     :   Feb 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform/Platform_SystemCursorManager.h"
#include "Platform/Platform_RenderThread.h"

namespace Scaleform { namespace Platform { 

void SystemCursorManager::UpdateCursor(const Point<int>& mousePos, RenderHALThread* pRenderThread)
{
    if (pRenderThread)
        pRenderThread->UpdateCursor(mousePos, *this);
}

}} // Scaleform::Platform
