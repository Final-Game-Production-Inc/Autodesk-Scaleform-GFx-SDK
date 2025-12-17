/**************************************************************************

Filename    :   FxPlayerSWFtoTextureD3D9.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerSWFtoTextureD3D9_H
#define FX_PlayerSWFtoTextureD3D9_H

#include "FxPlayerSWFToTexture.h"
#include <d3d9.h>

// The D3D9-platform specific application class.  Note that we derive from HALNotify, because
// we want to be notified of the device reset events.
class   FxPlayerSWFToTextureAppD3D9 : public FxPlayerSWFToTextureApp, public HALNotify
{
public:
    virtual void    InitGraphicsResources();

    // Overridden from HALNotify
    virtual void             OnHALEvent(HALNotifyType type);

protected:
    void            RenderMovieTextureToQuad();

    // Helper functions
    bool                     CreateRenderTextureBuffers();

    Ptr<IDirect3DDevice9>               pDevice;            // The Direct3D device.
    Ptr<IDirect3DTexture9>              pRenderTexture;     // The render texture.
    Ptr<IDirect3DSurface9>              pStencilSurface;    // The depth/stencil surface used when rendering the rendered texture.
    Ptr<IDirect3DVertexBuffer9>         pQuadVertexBuffer;  // The vertex buffer, containing the quad mesh data.
    Ptr<IDirect3DVertexDeclaration9>    pVertexDecl;        // The vertex format declaration.
    Ptr<IDirect3DVertexShader9>         pVertexShader;      // The vertex shader.
    Ptr<IDirect3DPixelShader9>          pPixelShader;       // The fragment (pixel) shader.
};



#endif
