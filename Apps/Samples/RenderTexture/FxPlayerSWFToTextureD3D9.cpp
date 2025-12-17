/**************************************************************************

Filename    :   FxPlayerSWFtoTextureD3D9.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerSWFtoTextureD3D9.h"
#include "Render/D3D9/D3D9_HAL.h"

// This file contains binary shaders, which were compiled offline using fxc.
#include "FxPlayerSWFToTextureD3D9_quadshader.h"

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the quad, and the vertex buffers for the quad mesh.
void FxPlayerSWFToTextureAppD3D9::InitGraphicsResources()
{
    // Store the device pointer (for ease of access), and add this to the HAL's notify list. This is required,
    // because on D3D9 device loss, we will need to destroy and recreate our rendered texture resources.
    D3D9::HAL* phal = reinterpret_cast<D3D9::HAL*>(pPlatformHAL);
    pDevice = phal->GetDevice();
    pPlatformHAL->AddNotify(this);

    // Create the rendered texture's buffers.
    CreateRenderTextureBuffers();

    // Create the vertex buffer used for the quad's mesh data.
    HRESULT hr;
    hr = pDevice->CreateVertexBuffer(sizeof(QuadVertices), 0, 0, D3DPOOL_MANAGED, &pQuadVertexBuffer.GetRawRef(), 0);
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create quad mesh vertex buffer (HRESULT=0x%08x).", hr);
        return;
    }

    // Copy the cube mesh vertex data to the vertex buffer.
    void *pVertices = NULL;
    pQuadVertexBuffer->Lock( 0, sizeof(QuadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, QuadVertices, sizeof(QuadVertices) );
    pQuadVertexBuffer->Unlock();

    // Create the shaders.
    hr = pDevice->CreateVertexShader((const DWORD*)QuadVertexShader, &pVertexShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex shader (HRESULT=0x%08x).", hr);
        return;
    }
    hr = pDevice->CreatePixelShader((const DWORD*)QuadPixelShader, &pPixelShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create pixel shader (HRESULT=0x%08x).", hr);
        return;
    }

    // Create the vertex input declaration.
    D3DVERTEXELEMENT9 quadMeshElements[] = {
        { 0, 0,               D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 3*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };
    hr = pDevice->CreateVertexDeclaration(quadMeshElements, &pVertexDecl.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex declaration (HRESULT=0x%08x).", hr);
        return;
    }
}

// This function renders a quad on the screen, with the GFx movie rendered texture applied to it.
void FxPlayerSWFToTextureAppD3D9::RenderMovieTextureToQuad()
{
    if (!pRenderTarget)
    {
        SF_DEBUG_WARNING(1, "Rendered target not initialized. Cannot render mesh.");
        return;
    }

    // Ensure that many D3D states are the way we expect (GFx may modify any of these states).
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    pDevice->SetRenderState(D3DRS_FILLMODE, CubeWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    pDevice->BeginScene();

    // Setup the shaders, and quad-mesh vertex buffer, and the rendered texture.
    pDevice->SetPixelShader(pPixelShader);
    pDevice->SetVertexShader(pVertexShader);
    pDevice->SetVertexDeclaration(pVertexDecl);
    pDevice->SetStreamSource(0, pQuadVertexBuffer, 0, sizeof(Vertex) );
    pDevice->SetIndices(0);
    pDevice->SetTexture(0, pRenderTexture);

    // Update the WxVxP matrix uniform.
    ComputeMatrices();
    pDevice->SetVertexShaderConstantF(0, WorldViewProjMatrix, 4 );

    // Now draw the sides of the cube.
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );

    pDevice->EndScene();
}

// Respond to device reset type notifications. We will need to release, and reallocate our
// non-managed pool rendering resources.
void FxPlayerSWFToTextureAppD3D9::OnHALEvent(HALNotifyType type)
{
    switch(type)
    {
    case HAL_PrepareForReset:
        // These are smart pointers, so setting them to zero should destroy the resources.
        pRenderTexture = 0;
        pStencilSurface = 0;
        pRenderTarget = 0;
        break;

    case HAL_RestoreAfterReset:
        // Recreate the buffers after the device has been reset.
        CreateRenderTextureBuffers();
        break;
    }
}

// This method is called by both InitGraphicsResources (to initially create the resources), and also
// in the HALNotify callback, after the device is reset, to reallocate the buffers.
bool FxPlayerSWFToTextureAppD3D9::CreateRenderTextureBuffers()
{
    HRESULT hr;
    hr = pDevice->CreateTexture(RTWidth,RTHeight,0, D3DUSAGE_RENDERTARGET|D3DUSAGE_AUTOGENMIPMAP, 
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pRenderTexture.GetRawRef(), 0);
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE(1, "Failed to create rendered texture.");
        return false;
    }

    hr = pDevice->CreateDepthStencilSurface( RTWidth,RTHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0,
        TRUE, &pStencilSurface.GetRawRef(), 0);
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE(1, "Failed to create depth/stencil target.");
        return false;
    }

    // Now create the render target from our render target and depth stencil surface.
    D3D9::HAL* phal = reinterpret_cast<D3D9::HAL*>(pPlatformHAL);
    Ptr<IDirect3DSurface9> prenderTargetSurface = 0;
    pRenderTexture->GetSurfaceLevel(0, &prenderTargetSurface.GetRawRef());
    pRenderTarget = *phal->CreateRenderTarget(prenderTargetSurface, pStencilSurface);

    return true;
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(TextureInSWF, Scaleform::GFx::System, FxPlayerSWFToTextureAppD3D9)
