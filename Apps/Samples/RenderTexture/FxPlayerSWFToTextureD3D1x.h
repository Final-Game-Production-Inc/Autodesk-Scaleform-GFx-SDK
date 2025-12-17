/**************************************************************************

Filename    :   FxPlayerSWFtoTextureD3D1x.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerSWFtoTextureD3D1x_H
#define FX_PlayerSWFtoTextureD3D1x_H

#include "FxPlayerSWFToTexture.h"

// NOTE: the appropriate D3D10 or D3D11 headers will be included D3D1x_Config.h. This includes
// creating macros which replace the function names with appropriate ones, depending on which one
// is being compiled (for example, the ID3D1x macros used below).
#include "Render/D3D1x/D3D1x_Config.h"

#include "Render/D3D1x/D3D1x_HAL.h"

class FxPlayerSWFToTextureAppD3D1x : public FxPlayerSWFToTextureApp
{
public:
    virtual void    InitGraphicsResources();

protected:
    virtual void    RenderMovieTextureToQuad();

    Ptr<ID3D1x(Device)>              pDevice;               // The device (used for creating resources)
    Ptr<ID3D1x(DeviceContext)>       pDeviceContext;        // The device context (used for rendering, maps to 'pDevice' in D3D10).

    Ptr<ID3D1x(Texture2D)>           pRenderTexture;        // The render texture.
    Ptr<ID3D1x(RenderTargetView)>    pRenderTextureView;    // The render texture view.
    Ptr<ID3D1x(Texture2D)>           pRTDepthStencilBuf;    // The depth/stencil surface used when rendering the rendered texture.
    Ptr<ID3D1x(DepthStencilView)>    pRTDepthStencil;       // The depth/stencil surface view.

    Ptr<ID3D1x(Buffer)>              pQuadVertexBuffer;     // The vertex buffer, containing the quad mesh data.
    Ptr<ID3D1x(InputLayout)>         pVertexLayout;         // The vertex format layout.
    Ptr<ID3D1x(VertexShader)>        pVertexShader;         // The vertex shader.
    Ptr<ID3D1x(PixelShader)>         pPixelShader;          // The fragment (pixel) shader.
    Ptr<ID3D1x(Buffer)>              pVShaderUniforms;      // The buffer containing the uniforms (shader constants), used with the vertex shader.
    Ptr<ID3D1x(ShaderResourceView)>  pRenderTextureSV;      // The shader resource view of the rendered texture

    Ptr<ID3D1x(DepthStencilState)>   pDepthTest;            // The depth/stencil state object
    Ptr<ID3D1x(RasterizerState)>     pRSFill, pRSLine;      // The rasterizer state objects (solid, and wireframe fills)
    Ptr<ID3D1x(BlendState)>          pBlendState;           // The blend state object.
};

#endif
