/**************************************************************************

Filename    :   Platform_D3D12.cpp
Content     :
Created     :
Authors     :   FINALZ-27980

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D12/D3D12_HAL.h"
#include "Platform.h"
#include <WinReg.h>
#include <ShlObj.h>

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Kernel/SF_SysFile.h"
#include <D3DCommon.h>