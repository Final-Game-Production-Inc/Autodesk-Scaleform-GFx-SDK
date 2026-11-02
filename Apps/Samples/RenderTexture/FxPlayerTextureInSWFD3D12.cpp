/**************************************************************************

Filename    :   FxPlayerTextureInSWFD3D12.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (D3D12)
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerTextureInSWFD3D12.h"

#include <d3dcompiler.h>

// This function renders the cube-mesh that can be seen in the rendered texture. It should
// be executed on the render thread (in multi-threaded mode).
void FxPlayerTextureInSWFAppD3D12::RenderMesh()
{
    if (!pRenderTexture)
    {
        SF_DEBUG_WARNING(1, "Rendered texture not initialized. Cannot render mesh.");
        return;
    }

    Render::D3D12::HAL* phal = reinterpret_cast<Render::D3D12::HAL*>(pPlatformHAL);
    ID3D12GraphicsCommandList* pcl = phal->GetCommandList();
    if (!pcl || !pPipelineState || !pRootSignature)
        return;

    // Transition the rendered texture from shader-resource to render-target.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pRenderTexture.GetPtr();
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    pcl->ResourceBarrier(1, &barrier);

    // Set the rendered texture as current render target and clear it.
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = pRenderTextureView.ptr ? pRenderTextureView : pRTV;
    pcl->OMSetRenderTargets(1, &rtv, TRUE, &pDSV);

    const float clear[] = {0, 0, 255/255.0f, 90/255.0f};
    pcl->ClearRenderTargetView(rtv, clear, 0, 0);
    pcl->ClearDepthStencilView(pDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, 0);

    // Set the viewport to the entire RenderTarget.
    D3D12_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width    = (float)RTWidth;
    vp.Height   = (float)RTHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    D3D12_RECT scissorRect = { 0, 0, (LONG)RTWidth, (LONG)RTHeight };
    pcl->RSSetViewports(1, &vp);
    pcl->RSSetScissorRects(1, &scissorRect);

    // Setup the shaders, and cube-mesh vertex buffer.
    pcl->SetGraphicsRootSignature(pRootSignature.GetPtr());
    pcl->SetPipelineState(pPipelineState.GetPtr());
    pcl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    UINT stride = sizeof(Vertex);
    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pCubeVertexBuffer->GetGPUVirtualAddress();
    vbView.SizeInBytes    = stride * 24;
    vbView.StrideInBytes  = stride;
    pcl->IASetVertexBuffers(0, 1, &vbView);

    // Setup the world, view, and projection matrices, and update the constant buffer.
    ComputeMatrices();
    void* pdata = 0;
    D3D12_RANGE range = { 0, 0 };
    if (SUCCEEDED(pVShaderUniforms->Map(0, &range, &pdata)))
    {
        memcpy(pdata, WorldViewProjMatrix, 16 * sizeof(float));
        pVShaderUniforms->Unmap(0, 0);
    }
    pcl->SetGraphicsRootConstantBufferView(0, pVShaderUniforms->GetGPUVirtualAddress());

    // Now draw the sides of the cube.
    for (unsigned i = 0; i < 6; i++)
        pcl->DrawInstanced(4, 1, i * 4, 0);

    // Restore the main render target and transition the rendered texture back to a shader resource.
    if (phal->MainRT.Set)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE mainRTV = phal->MainRT.RTV;
        pcl->OMSetRenderTargets(1, &mainRTV, FALSE,
            phal->MainRT.DSV.ptr ? &phal->MainRT.DSV : 0);
    }
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    pcl->ResourceBarrier(1, &barrier);
}

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the cube, and the vertex buffers for the cube mesh.
void FxPlayerTextureInSWFAppD3D12::InitGraphicsResources()
{
    Render::D3D12::HAL* phal = reinterpret_cast<Render::D3D12::HAL*>(pPlatformHAL);
    pDevice = phal->GetDevice();
    if (!pDevice)
        return;

    // Create the rendered texture.
    D3D12_RESOURCE_DESC texdesc = {};
    texdesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texdesc.Alignment          = 0;
    texdesc.Width              = RTWidth;
    texdesc.Height             = RTHeight;
    texdesc.DepthOrArraySize   = 1;
    texdesc.MipLevels          = 1;
    texdesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    texdesc.SampleDesc.Count   = 1;
    texdesc.SampleDesc.Quality = 0;
    texdesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texdesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = texdesc.Format;
    clearValue.Color[0] = 0.0f; clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 1.0f; clearValue.Color[3] = 0.35f;

    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;
    if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texdesc,
                                                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                &clearValue, IID_PPV_ARGS(&pRenderTexture.GetRawRef()))))
    {
        SF_DEBUG_MESSAGE(1, "Failed to create the rendered texture.");
        return;
    }

    // Create the render-target view for the rendered texture.
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = 2;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRTVHeap.GetRawRef()))))
        return;
    pRTV = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    pDevice->CreateRenderTargetView(pRenderTexture, 0, pRTV);
    pRenderTextureView = pRTV;

    // Create the depth/stencil buffer and view.
    texdesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texdesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE dsClear = {};
    dsClear.Format                = texdesc.Format;
    dsClear.DepthStencil.Depth    = 1.0f;
    dsClear.DepthStencil.Stencil  = 0;
    if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texdesc,
                                                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &dsClear, IID_PPV_ARGS(&pRTDepthStencilBuf.GetRawRef()))))
        return;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.NumDescriptors = 1;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pDSVHeap.GetRawRef()))))
        return;
    pDSV = pDSVHeap->GetCPUDescriptorHandleForHeapStart();
    pDevice->CreateDepthStencilView(pRTDepthStencilBuf, 0, pDSV);

    // Now create the GFx texture, that will actually be applied, from our render texture.
    Render::D3D12::TextureManager* pmanager = (Render::D3D12::TextureManager*)pPlatformHAL->GetTextureManager();
    pMyHWTexture = *pmanager->CreateTexture(pRenderTexture.GetPtr(),
                                            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                            ImageSize(RTWidth, RTHeight));

    // Create the vertex buffer used for the cube's mesh data, and initialize it with the cube-mesh vertices.
    D3D12_HEAP_PROPERTIES uploadProps;
    memset(&uploadProps, 0, sizeof(uploadProps));
    uploadProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    uploadProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadProps.CreationNodeMask     = 1;
    uploadProps.VisibleNodeMask      = 1;
    D3D12_RESOURCE_DESC bufdesc = {};
    bufdesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufdesc.Alignment          = 0;
    bufdesc.Width              = sizeof(CubeVertices);
    bufdesc.Height             = 1;
    bufdesc.DepthOrArraySize   = 1;
    bufdesc.MipLevels          = 1;
    bufdesc.Format             = DXGI_FORMAT_UNKNOWN;
    bufdesc.SampleDesc.Count   = 1;
    bufdesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufdesc.Flags              = D3D12_RESOURCE_FLAG_NONE;
    if (FAILED(pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufdesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ,
                                                0, IID_PPV_ARGS(&pCubeVertexBuffer.GetRawRef()))))
        return;
    void* pvbData = 0;
    D3D12_RANGE zeroRange = { 0, 0 };
    if (SUCCEEDED(pCubeVertexBuffer->Map(0, &zeroRange, &pvbData)))
    {
        memcpy(pvbData, CubeVertices, sizeof(CubeVertices));
        pCubeVertexBuffer->Unmap(0, 0);
    }

    // Create the constant buffer for the vertex shader (16 floats: WorldViewProj matrix).
    bufdesc.Width = 16 * sizeof(float);
    if (FAILED(pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufdesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ,
                                                0, IID_PPV_ARGS(&pVShaderUniforms.GetRawRef()))))
        return;

    // Create the root signature: one root constant-buffer view (b0).
    D3D12_ROOT_PARAMETER rootParam = {};
    rootParam.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.Descriptor.ShaderRegister = 0;
    rootParam.Descriptor.RegisterSpace  = 0;
    rootParam.ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters     = 1;
    rsDesc.pParameters       = &rootParam;
    rsDesc.NumStaticSamplers = 0;
    rsDesc.pStaticSamplers   = 0;
    rsDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    Ptr<ID3DBlob> pSigBlob, pErrBlob;
    if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                           &pSigBlob.GetRawRef(), &pErrBlob.GetRawRef())))
    {
        SF_DEBUG_MESSAGE(1, "Failed to serialize root signature.");
        return;
    }
    if (FAILED(pDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(),
                                            IID_PPV_ARGS(&pRootSignature.GetRawRef()))))
        return;

    // Compile the cube shaders (DXIL, shader model 6.0).
    Ptr<ID3DBlob> pVS, pPS, pErr;
    static const char* vsSource =
        "cbuffer WVP : register(b0) { float4x4 WorldViewProj; };\n"
        "struct VSIn { float3 pos : POSITION; float4 color : COLOR; };\n"
        "struct VSOut { float4 pos : SV_Position; float4 color : COLOR; };\n"
        "VSOut main(VSIn i) { VSOut o; o.pos = mul(float4(i.pos, 1.0), WorldViewProj); o.color = i.color; return o; }\n";
    static const char* psSource =
        "struct VSOut { float4 pos : SV_Position; float4 color : COLOR; };\n"
        "float4 main(VSOut i) : SV_Target { return i.color; }\n";
    if (FAILED(D3DCompile(vsSource, strlen(vsSource), "cubeVS", 0, 0, "main", "vs_6_0",
                          D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pVS.GetRawRef(), &pErr.GetRawRef())))
    {
        SF_DEBUG_MESSAGE(1, "Failed to compile the cube vertex shader.");
        return;
    }
    if (FAILED(D3DCompile(psSource, strlen(psSource), "cubePS", 0, 0, "main", "ps_6_0",
                          D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pPS.GetRawRef(), &pErr.GetRawRef())))
    {
        SF_DEBUG_MESSAGE(1, "Failed to compile the cube pixel shader.");
        return;
    }

    // Create the pipeline state object.
    static D3D12_INPUT_ELEMENT_DESC cubeMeshElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature        = pRootSignature.GetPtr();
    psoDesc.VS.pShaderBytecode    = pVS->GetBufferPointer();
    psoDesc.VS.BytecodeLength     = pVS->GetBufferSize();
    psoDesc.PS.pShaderBytecode    = pPS->GetBufferPointer();
    psoDesc.PS.BytecodeLength     = pPS->GetBufferSize();
    psoDesc.InputLayout.pInputElementDescs = cubeMeshElements;
    psoDesc.InputLayout.NumElements        = 2;
    psoDesc.RasterizerState.FillMode       = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode       = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable    = TRUE;
    psoDesc.DepthStencilState.DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable  = FALSE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask           = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets     = 1;
    psoDesc.RTVFormats[0]        = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat            = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count     = 1;
    if (FAILED(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipelineState.GetRawRef()))))
    {
        SF_DEBUG_MESSAGE(1, "Failed to create the cube pipeline state.");
        return;
    }
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(TextureInSWF, Scaleform::GFx::System, FxPlayerTextureInSWFAppD3D12)
