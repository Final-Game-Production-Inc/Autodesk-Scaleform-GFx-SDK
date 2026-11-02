/**************************************************************************

    PublicHeader:   Render
    Filename    :   D3D12_Config.h
    Content     :   Configuration header for the D3D12 renderer.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_CONFIG_H
#define INC_SF_D3D12_CONFIG_H
#pragma once

// D3D12 renderer requires the Windows 10 SDK (d3d12.h) and is built on the
// Desktop (non-WinRT) path only.
#if !defined(SF_OS_WINMETRO)
    #include <d3dcommon.h>
    #include <d3d12.h>
    #include <dxgi.h>
#else
    #error D3D12 renderer is not supported on WinRT at this time.
#endif

#endif // INC_SF_D3D12_CONFIG_H
