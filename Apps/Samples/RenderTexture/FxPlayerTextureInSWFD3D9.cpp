/**************************************************************************

Filename    :   FxPlayerTextureInSWFD3D9.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerTextureInSWFD3D9.h"
#include "Render/D3D9/D3D9_HAL.h"

// This file contains binary shaders, which were compiled offline using fxc.
#include "FxPlayerTextureInSWFD3D9_cubeshader.h"

// This function renders the cube-mesh that can be seen in the rendered texture. It should
// be executed on the render thread (in multi-threaded mode).
void FxPlayerTextureInSWFAppD3D9::RenderMesh()
{
    if (!pRenderTexture)
    {
        SF_DEBUG_WARNING(1, "Rendered texture not initialized. Cannot render mesh.");
        return;
    }

    IDirect3DSurface9 *poldSurface      = 0;
    IDirect3DSurface9 *poldDepthSurface = 0;
    IDirect3DSurface9 *psurface         = 0;

    // Remember the current RenderTarget and DepthStencil surface.
    pDevice->GetRenderTarget(0, &poldSurface);
    pDevice->GetDepthStencilSurface(&poldDepthSurface);

    // Set rendered texture as current render target
    pRenderTexture->GetSurfaceLevel(0, &psurface);
    if (!FAILED(pDevice->SetRenderTarget(0, psurface )))
    {
        // Set stencil; this will disable it if not available.
        pDevice->SetDepthStencilSurface(pStencilSurface);
    }

    // Set the viewport to the entire RenderTarget.
    ImageSize textureSize = pMyHWTexture->GetSize();
    D3DVIEWPORT9 vp = { 0, 0, textureSize.Width, textureSize.Height, 0.0f, 1.0f };
    pDevice->SetViewport(&vp);

    // Clear the render target to Blue (alpha = 35%). Also clear the depth and stencil.
    static D3DCOLOR clearColor = D3DCOLOR_ARGB(90,0,0,255);
    pDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, clearColor, 1.0f, 0 );

    // Ensure that many D3D states are the way we expect (GFx may modify any of these states).
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    pDevice->SetRenderState(D3DRS_FILLMODE, CubeWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

    pDevice->BeginScene();

    // Setup the shaders, and cube-mesh vertex buffer
    pDevice->SetPixelShader(pPixelShader);
    pDevice->SetVertexShader(pVertexShader);
    pDevice->SetVertexDeclaration(pVertexDecl);
    pDevice->SetStreamSource(0, pCubeVertexBuffer, 0, sizeof(Vertex) );
    pDevice->SetIndices(0);

    // Update the WxVxP matrix uniform.
    ComputeMatrices();
    pDevice->SetVertexShaderConstantF(0, WorldViewProjMatrix, 4 );

    // Now draw the sides of the cube.
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );

    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    pDevice->EndScene();

    // Restore the render target
    pDevice->SetRenderTarget(0, poldSurface);
    pDevice->SetDepthStencilSurface(poldDepthSurface);

    if (psurface)
        psurface->Release();
    if (poldSurface)
        poldSurface->Release();
    if (poldDepthSurface)
        poldDepthSurface->Release();
}

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the cube, and the vertex buffers for the cube mesh.
void FxPlayerTextureInSWFAppD3D9::InitGraphicsResources()
{
    // Store the device pointer (for ease of access), and add this to the HAL's notify list. This is required,
    // because on D3D9 device loss, we will need to destroy and recreate our rendered texture resources.
    D3D9::HAL* phal = reinterpret_cast<D3D9::HAL*>(pPlatformHAL);
    pDevice = phal->GetDevice();
    pPlatformHAL->AddNotify(this);

    // Create the rendered texture's buffers.
    CreateRenderTextureBuffers();

    // Create the vertex buffer used for the cube's mesh data.
    HRESULT hr;
    hr = pDevice->CreateVertexBuffer(sizeof(CubeVertices), 0, 0, D3DPOOL_MANAGED, &pCubeVertexBuffer.GetRawRef(), 0);
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create cube mesh vertex buffer (HRESULT=0x%08x).", hr);
        return;
    }

    // Copy the cube mesh vertex data to the vertex buffer.
    void *pVertices = NULL;
    pCubeVertexBuffer->Lock( 0, sizeof(CubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, CubeVertices, sizeof(CubeVertices) );
    pCubeVertexBuffer->Unlock();

    // Create the shaders.
    hr = pDevice->CreateVertexShader((const DWORD*)CubeVertexShader, &pVertexShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex shader (HRESULT=0x%08x).", hr);
        return;
    }
    hr = pDevice->CreatePixelShader((const DWORD*)CubePixelShader, &pPixelShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create pixel shader (HRESULT=0x%08x).", hr);
        return;
    }

    // Create the vertex input declaration.
    D3DVERTEXELEMENT9 cubeMeshElements[] = {
        { 0, 0,               D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 3*sizeof(float), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    };
    hr = pDevice->CreateVertexDeclaration(cubeMeshElements, &pVertexDecl.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex declaration (HRESULT=0x%08x).", hr);
        return;
    }
}

// Respond to device reset type notifications. We will need to release, and reallocate our
// non-managed pool rendering resources.
void FxPlayerTextureInSWFAppD3D9::OnHALEvent(HALNotifyType type)
{
    switch(type)
    {
    case HAL_PrepareForReset:
        // These are smart pointers, so setting them to zero should destroy the resources.
        pRenderTexture = 0;
        pStencilSurface = 0;
        pMyHWTexture = 0;
        break;

    case HAL_RestoreAfterReset:
        // Recreate the buffers after the device has been reset.
        CreateRenderTextureBuffers();

        // Now call ReplaceTexture, so that our new pMyHWTexture will be applied to the resource.
        ReplaceTexture();
        break;
    }
}

// This method is called by both InitGraphicsResources (to initially create the resources), and also
// in the HALNotify callback, after the device is reset, to reallocate the buffers.
bool FxPlayerTextureInSWFAppD3D9::CreateRenderTextureBuffers()
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

    // Now create the GFx texture, that will actually be applied, from our D3D targets.
    D3D9::TextureManager * pmanager = (D3D9::TextureManager*)pPlatformHAL->GetTextureManager(); 
    pMyHWTexture = *pmanager->CreateTexture( pRenderTexture, ImageSize(RTWidth, RTHeight));

    return pMyHWTexture != 0;
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(TextureInSWF, Scaleform::GFx::System, FxPlayerTextureInSWFAppD3D9)
