/**************************************************************************

Filename    :   FxPlayerSWFtoTextureD3D1x.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerSWFToTextureD3D1x.h"

// This file contains binary shaders, which were compiled offline using fxc.
#include "FxPlayerSWFToTextureD3D1x_quadshader.h"

// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the cube, and the vertex buffers for the cube mesh.
void FxPlayerSWFToTextureAppD3D1x::InitGraphicsResources()
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

    // Create the shader-resource view for the rendered texture.
    pDevice->CreateShaderResourceView( pRenderTexture, 0, &pRenderTextureSV.GetRawRef());

    // Create the depth/stencil buffer that is used when rendering the cube. Most parameters remain the same,
    // except the format, and bind flags.
    texdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texdesc.BindFlags = D3D1x(BIND_DEPTH_STENCIL);
    pDevice->CreateTexture2D(&texdesc, NULL, &pRTDepthStencilBuf.GetRawRef());
    pDevice->CreateDepthStencilView( pRTDepthStencilBuf, NULL, &pRTDepthStencil.GetRawRef());

    // Now create the render target from our render target and depth stencil surface.
    pRenderTarget = *phal->CreateRenderTarget(pRenderTextureView.GetPtr(), pRTDepthStencil.GetPtr());

    // Create the quad-mesh (on which the rendered movie will be placed), and initialize it with the vertices
    D3D1x(BUFFER_DESC) bd;
    memset(&bd, 0, sizeof(D3D1x(BUFFER_DESC)));
    bd.Usage = D3D1x(USAGE_DEFAULT);
    bd.BindFlags = D3D1x(BIND_VERTEX_BUFFER);
    bd.ByteWidth = sizeof(QuadVertices);
    D3D1x(SUBRESOURCE_DATA) vd;
    vd.pSysMem = QuadVertices;
    vd.SysMemPitch = vd.SysMemSlicePitch = 0;
    pDevice->CreateBuffer(&bd, &vd, &pQuadVertexBuffer.GetRawRef());

    // Create the buffer for the vertex shader constants.
    memset(&bd, 0, sizeof(D3D1x(BUFFER_DESC)));
    bd.CPUAccessFlags = D3D1x(CPU_ACCESS_WRITE);
    bd.Usage = D3D1x(USAGE_DYNAMIC);
    bd.BindFlags = D3D1x(BIND_CONSTANT_BUFFER);
    bd.ByteWidth = sizeof(float) * 16;
    pDevice->CreateBuffer(&bd, 0, &pVShaderUniforms.GetRawRef());

    // Create the shaders
    Ptr<ID3D10Blob> pvertexShader, pfragmentShader;
    Ptr<ID3D10Blob> errorMessages;
    HRESULT hr;

    hr = D3D1xCreateVertexShader(pDevice, QuadVertexShader, sizeof(QuadVertexShader), &pVertexShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create vertex shader (HRESULT=0x%08x).", hr);
        return;
    }
    hr = D3D1xCreatePixelShader(pDevice, QuadPixelShader, sizeof(QuadPixelShader), &pPixelShader.GetRawRef());
    if (FAILED(hr))
    {
        SF_DEBUG_MESSAGE1(1, "Failed to create pixel shader (HRESULT=0x%08x).", hr);
        return;
    }


    // Create the input layout for the quad mesh.
    static D3D1x(INPUT_ELEMENT_DESC) quadMeshElements[] =
    {
        {"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D1x(INPUT_PER_VERTEX_DATA), 0},
        {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D1x(INPUT_PER_VERTEX_DATA), 0},
    };
    pDevice->CreateInputLayout(quadMeshElements, 2, QuadVertexShader, sizeof(QuadVertexShader), &pVertexLayout.GetRawRef());

    // Create the depth/stencil, rasterization, and blend states that are used when rendering the cube.
    D3D1x(DEPTH_STENCIL_DESC) ds;
    memset(&ds, 0, sizeof(D3D1x(DEPTH_STENCIL_DESC)));
    ds.DepthFunc = D3D1x(COMPARISON_ALWAYS);
    ds.DepthEnable = 0;
    ds.DepthWriteMask = D3D1x(DEPTH_WRITE_MASK_ZERO);
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
    rs.MultisampleEnable = 0;
    rs.AntialiasedLineEnable = 1;
    pDevice->CreateRasterizerState(&rs, &pRSFill.GetRawRef());

#if (SF_D3D_VERSION == 10)
#define BLEND(enable,srcC,dstC,Cop,srcA,dstA,Aop,mask) \
    { 0, {enable}, D3D1x(srcC), D3D1x(dstC), D3D1x(Cop), D3D1x(srcA), D3D1x(dstA), D3D1x(Aop), {mask} }

#else
#define BLEND(enable,srcC,dstC,Cop,srcA,dstA,Aop,mask) \
    { 0, 0, {{enable, D3D1x(srcC), D3D1x(dstC), D3D1x(Cop), D3D1x(srcA), D3D1x(dstA), D3D1x(Aop), mask}} }
#endif

    const D3D1x(BLEND_DESC) blend = BLEND(1, BLEND_ONE, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD,
        BLEND_ONE, BLEND_INV_SRC_ALPHA, BLEND_OP_ADD, D3D1x(COLOR_WRITE_ENABLE_ALL));
#undef BLEND

    pDevice->CreateBlendState(&blend, &pBlendState.GetRawRef());
}

// This function renders a quad on the screen, with the GFx movie rendered texture applied to it.
void FxPlayerSWFToTextureAppD3D1x::RenderMovieTextureToQuad()
{
    // Ensure that many D3D states are the way we expect (GFx may modify any of these states).
    const float one[] = {1,1,1,1};
    pDeviceContext->OMSetBlendState(pBlendState, one, 0xfffffff);
    pDeviceContext->RSSetState(CubeWireframe ? pRSLine : pRSFill);
    pDeviceContext->OMSetDepthStencilState(pDepthTest, 0);
    pDeviceContext->IASetPrimitiveTopology(D3D1x(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));

    // Setup the shaders, quad-mesh vertex buffer, and the rendered texture sampler
    D3D1xVSSetShader(pDeviceContext, pVertexShader);
    D3D1xPSSetShader(pDeviceContext, pPixelShader);
    pDeviceContext->IASetInputLayout(pVertexLayout);
    unsigned stride = sizeof(Vertex);
    unsigned offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pQuadVertexBuffer.GetRawRef(), &stride, &offset);
    pDeviceContext->IASetIndexBuffer(0, DXGI_FORMAT_UNKNOWN, 0);
    pDeviceContext->PSSetShaderResources(0, 1, &pRenderTextureSV.GetRawRef());

    // Setup the world, view, and projection matrices
    ComputeMatrices();
    D3D1x(MAPPED_BUFFER) mappedBuffer;
    D3D1xMapBuffer(pDeviceContext, pVShaderUniforms, 0, D3D1x(MAP_WRITE_DISCARD), 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, WorldViewProjMatrix, 16*sizeof(float));
    D3D1xUnmapBuffer(pDeviceContext, pVShaderUniforms, 0);
    ID3D1x(Buffer)* pConstantBuffers = pVShaderUniforms.GetPtr();
    pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffers);

    // Now draw the quad.
    pDeviceContext->Draw(4, 0);
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(SWFToTexture, Scaleform::GFx::System, FxPlayerSWFToTextureAppD3D1x)
