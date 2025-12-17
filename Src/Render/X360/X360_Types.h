/**************************************************************************

Filename    :   X360_Types.h
Content     :   Types and defines for X360.
Created     :   Nov 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_X360_TYPES_H
#define INC_SF_X360_TYPES_H
#pragma once

#define D3DUSAGE_AUTOGENMIPMAP    (0x00000000L)    // Not included in X360 (can always auto-gen).
#define D3DUSAGE_DYNAMIC        (0x00000000L)    // Not included in X360 (can always have dynamic).

// DirectX compatibility defines
#define IDirect3DDeviceX        IDirect3DDevice9
#define IDirect3DTextureX       IDirect3DTexture9
#define IDirect3DSurfaceX       IDirect3DSurface9
#define IDirect3DIndexBufferX   IDirect3DIndexBuffer9
#define IDirect3DVertexBufferX  IDirect3DVertexBuffer9
#define D3DCAPSx                D3DCAPS9
#define D3DVIEWPORTx            D3DVIEWPORT9
#define LPDIRECT3Dx             LPDIRECT3D9
#define LPDIRECT3DDEVICEx       LPDIRECT3DDEVICE9

#endif // INC_SF_X360_TYPES_H
