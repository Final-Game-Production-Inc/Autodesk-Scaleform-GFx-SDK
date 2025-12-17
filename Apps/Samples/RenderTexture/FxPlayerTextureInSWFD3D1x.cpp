/**************************************************************************

Filename    :   FxPlayerTextureInSWFD3D1x.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerTextureInSWFD3D1x.h"

// This file contains binary shaders, which were compiled offline using fxc.
#include "FxPlayerTextureInSWFD3D1x_cubeshader.h"

// This function renders the cube-mesh that can be seen in the rendered texture. It should
// be executed on the render thread (in multi-threaded mode).
void FxPlayerTextureInSWFAppD3D1x::RenderMesh()
{
    if (!pRenderTexture)
    {
        SF_DEBUG_WARNING(1, "Rendered texture not initialized. Cannot render mesh.");
        return;
    }

    Ptr<ID3D1x(RenderTargetView) > poldRT;
    Ptr<ID3D1x(DepthStencilView) > poldDS;

    // Remember the current RenderTarget and DepthStencil surface.
    pDeviceContext->OMGetRenderTargets(1, &poldRT.GetRawRef(), &poldDS.GetRawRef() );

    // Set rendered texture as current render target
    pDeviceContext->OMSetRenderTargets(1, &pRenderTextureView.GetRawRef(), pRTDepthStencil);

    // Set the viewport to the entire RenderTarget.
    D3D1x(VIEWPORT) vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width    = D3D10_11((UINT),(FLOAT))RTWidth;
    vp.Height   = D3D10_11((UINT),(FLOAT))RTHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pDeviceContext->RSSetViewports(1, &vp);

    // Clear the render target to Blue (alpha = 35%). Also clear the depth and stencil.
    const float clear[] = {0,0,255/255.0f,90/255.0f};
    pDeviceContext->ClearRenderTargetView(pRenderTextureView, clear);
    pDeviceContext->ClearDepthStencilView(pRTDepthStencil, D3D1x(CLEAR_DEPTH) | D3D1x(CLEAR_STENCIL), 1.0f, 0);

    // Ensure that many D3D states are the way we expect (GFx may modify any of these states).
    const float one[] = {1,1,1,1};
    pDeviceContext->OMSetBlendState(pBlendState, one, 0xfffffff);
    pDeviceContext->RSSetState(CubeWireframe ? pRSLine : pRSFill);
    pDeviceContext->OMSetDepthStencilState(pDepthTest, 0);
    pDeviceContext->IASetPrimitiveTopology(D3D1x(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));

    // Setup the shaders, and cube-mesh vertex buffer
    D3D1xVSSetShader(pDeviceContext, pVertexShader);
    D3D1xPSSetShader(pDeviceContext, pPixelShader);
    pDeviceContext->IASetInputLayout(pVertexLayout);
    unsigned stride = sizeof(Vertex);
    unsigned offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pCubeVertexBuffer.GetRawRef(), &stride, &offset);
    pDeviceContext->IASetIndexBuffer(0, DXGI_FORMAT_UNKNOWN, 0);

    // Setup the world, view, and projection matrices
    ComputeMatrices();
    D3D1x(MAPPED_BUFFER) mappedBuffer;
    D3D1xMapBuffer(pDeviceContext, pVShaderUniforms, 0, D3D1x(MAP_WRITE_DISCARD), 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, WorldViewProjMatrix, 16*sizeof(float));
    D3D1xUnmapBuffer(pDeviceContext, pVShaderUniforms, 0);
    ID3D1x(Buffer)* pConstantBuffers = pVShaderUniforms.GetPtr();
    pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffers);

    // Now draw the sides of the cube.
    pDeviceContext->Draw(4, 0);
    pDeviceContext->Draw(4, 4);
    pDeviceContext->Draw(4, 8);
    pDeviceContext->Draw(4, 12);
    pDeviceContext->Draw(4, 16);
    pDeviceContext->Draw(4, 20);

    // Restore the render target
    pDeviceContext->OMSetRenderTargets(1, &poldRT.GetRawRef(), poldDS);
}

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the cube, and the vertex buffers for the cube mesh.
void FxPlayerTextureInSWFAppD3D1x::InitGraphicsResources()
{
    // Store the device pointer (for ease of access).
    D3D1x::HAL* phal = reinterpret_cast<D3D1x::HAL*>(pPlatformHAL);
    pDevice = phal->GetDevice();
    pDeviceContext = phal->pDeviceContext;

    // Create the rendered texture.
    D3D1x(TEXTURE2D_DESC) texdesc;
    memset(&texdesc, 0, sizeof(texdesc));
    texdesc.Width = RTWidth;
    texdesc.Height = RTHeight;
    texdesc.MipLevels = 1;
    texdesc.ArraySize = 1;
    texdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texdesc.SampleDesc.Count = 1;
    texdesc.SampleDesc.Quality = 0;
    texdesc.Usage = D3D1x(USAGE_DEFAULT);
    texdesc.BindFlags = D3D1x(BIND_RENDER_TARGET) | D3D1x(BIND_SHADER_RESOURCE);
    texdesc.CPUAccessFlags = 0;
    texdesc.MiscFlags = 0;
    pDevice->CreateTexture2D(&texdesc, NULL, &pRenderTexture.GetRawRef());
    pDevice->CreateRenderTargetView(pRenderTexture, NULL, &pRenderTextureView.GetRawRef());

    // Create the depth/stencil buffer that is used when rendering the cube. Most parameters remain the same,
    // except the format, and bind flags.
    texdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texdesc.BindFlags = D3D1x(BIND_DEPTH_STENCIL);
    pDevice->CreateTexture2D(&texdesc, NULL, &pRTDepthStencilBuf.GetRawRef());
    pDevice->CreateDepthStencilView( pRTDepthStencilBuf, NULL, &pRTDepthStencil.GetRawRef());

    // Now create the GFx texture, that will actually be applied, from our FBO.
    D3D1x::TextureManager * pmanager = (D3D1x::TextureManager*)pPlatformHAL->GetTextureManager(); 
    pMyHWTexture = *pmanager->CreateTexture( pRenderTexture, ImageSize(RTWidth, RTHeight));

    // Create the vertex buffer used for the cube's mesh data, and initialize it with the cube-mesh vertices
    D3D1x(BUFFER_DESC) bd;
    memset(&bd, 0, sizeof(D3D1x(BUFFER_DESC)));
    bd.Usage = D3D1x(USAGE_DEFAULT);
    bd.BindFlags = D3D1x(BIND_VERTEX_BUFFER);
    bd.ByteWidth = sizeof(CubeVertices);
    D3D1x(SUBRESOURCE_DATA) vd;
    vd.pSysMem = CubeVertices;
    vd.SysMemPitch = vd.SysMemSlicePitch = 0;
    pDevice->CreateBuffer(&bd, &vd, &pCubeVertexBuffer.GetRawRef());

    // Create the buffer for the vertex shader constants.
    memset(&bd, 0, sizeof(D3D1x(BUFFER_DESC)));
    bd.CPUAccessFlags = D3D1x(CPU_ACCESS_WRITE);
    bd.Usage = D3D1x(USAGE_DYNAMIC);
    bd.BindFlags = D3D1x(BIND_CONSTANT_BUFFER);
    bd.ByteWidth = sizeof(float) * 16;
    pDevice->CreateBuffer(&bd, 0, &pVShaderUniforms.GetRawRef());

    // Create the shaders
    HRESULT hr;
    hr = D3D1xCreateVertexShader(pDevice, CubeVertexShader, sizeof(CubeVertexShader), &pVertexShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex shader (HRESULT=0x%08x).", hr);
        return;
    }
    hr = D3D1xCreatePixelShader(pDevice, CubePixelShader, sizeof(CubePixelShader), &pPixelShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create pixel shader (HRESULT=0x%08x).", hr);
        return;
    }

    // Create the input layout for the cube mesh.
    static D3D1x(INPUT_ELEMENT_DESC) cubeMeshElements[] =
    {
        {"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D1x(INPUT_PER_VERTEX_DATA), 0},
        {"COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,   0, 12, D3D1x(INPUT_PER_VERTEX_DATA), 0},
    };
    pDevice->CreateInputLayout(cubeMeshElements, 2, CubeVertexShader, sizeof(CubeVertexShader), &pVertexLayout.GetRawRef());

    // Create the depth/stencil, rasterization, and blend states that are used when rendering the cube.
    D3D1x(DEPTH_STENCIL_DESC) ds;
    memset(&ds, 0, sizeof(D3D1x(DEPTH_STENCIL_DESC)));
    ds.DepthFunc = D3D1x(COMPARISON_LESS_EQUAL);
    ds.DepthEnable = 1;
    ds.DepthWriteMask = D3D1x(DEPTH_WRITE_MASK_ALL);
    ds.FrontFace.StencilDepthFailOp = ds.BackFace.StencilDepthFailOp = D3D1x(STENCIL_OP_KEEP);
    ds.FrontFace.StencilFailOp = ds.BackFace.StencilFailOp = D3D1x(STENCIL_OP_KEEP);
    ds.FrontFace.StencilPassOp = ds.BackFace.StencilPassOp = D3D1x(STENCIL_OP_KEEP);
    ds.FrontFace.StencilFunc = ds.BackFace.StencilFunc = D3D1x(COMPARISON_ALWAYS);
    pDevice->CreateDepthStencilState(&ds, &pDepthTest.GetRawRef());

    D3D1x(RASTERIZER_DESC) rs;
    memset(&rs, 0, sizeof(D3D1x(RASTERIZER_DESC)));
    rs.FillMode = D3D1x(FILL_SOLID);
    rs.CullMode = D3D1x(CULL_NONE);
    rs.ScissorEnable = 0;
    rs.MultisampleEnable = 1;
    rs.AntialiasedLineEnable = 1;
    pDevice->CreateRasterizerState(&rs, &pRSFill.GetRawRef());
    rs.FillMode = D3D1x(FILL_WIREFRAME);
    pDevice->CreateRasterizerState(&rs, &pRSLine.GetRawRef());

#if (SF_D3D_VERSION == 10)
#define BLEND(enable,srcC,dstC,Cop,srcA,dstA,Aop,mask) \
    { 0, {enable}, D3D1x(srcC), D3D1x(dstC), D3D1x(Cop), D3D1x(srcA), D3D1x(dstA), D3D1x(Aop), {mask} }

#else
#define BLEND(enable,srcC,dstC,Cop,srcA,dstA,Aop,mask) \
    { 0, 0, {{enable, D3D1x(srcC), D3D1x(dstC), D3D1x(Cop), D3D1x(srcA), D3D1x(dstA), D3D1x(Aop), mask}} }
#endif

    const D3D1x(BLEND_DESC) blend = BLEND(1, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD,
        BLEND_ONE, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD, D3D1x(COLOR_WRITE_ENABLE_ALL));
#undef BLEND

    pDevice->CreateBlendState(&blend, &pBlendState.GetRawRef());
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(TextureInSWF, Scaleform::GFx::System, FxPlayerTextureInSWFAppD3D1x)
