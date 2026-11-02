/**************************************************************************

Filename    :   FxPlayerSWFToTextureD3D12.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (D3D12)
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerSWFToTextureD3D12.h"

#include <d3dcompiler.h>

// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the quad, and the vertex buffer for the quad mesh.
void FxPlayerSWFToTextureAppD3D12::InitGraphicsResources()
{
    // Store the device pointer (for ease of access).
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
    clearValue.Color[2] = 0.0f; clearValue.Color[3] = 0.0f;

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

    // Create the shader-resource view for the rendered texture (GPU visible, used by the quad shader).
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSRVHeap.GetRawRef()))))
        return;
    pRenderTextureSV = pSRVHeap->GetGPUDescriptorHandleForHeapStart();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                  = texdesc.Format;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels     = 1;
    pDevice->CreateShaderResourceView(pRenderTexture, &srvDesc, pSRVHeap->GetCPUDescriptorHandleForHeapStart());

    // Create the linear-clamp sampler used by the quad shader.
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSamplerHeap.GetRawRef()))))
        return;
    pSampler = pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MaxLOD         = D3D12_FLOAT32_MAX;
    pDevice->CreateSampler(&samplerDesc, pSamplerHeap->GetCPUDescriptorHandleForHeapStart());

    // Now create the GFx texture wrapping our render texture, and assign the RTV so that
    // HAL::CreateRenderTarget can build the movie render target from it.
    Render::D3D12::TextureManager* pmanager = (Render::D3D12::TextureManager*)pPlatformHAL->GetTextureManager();
    pRenderTextureTex = *pmanager->CreateTexture(pRenderTexture.GetPtr(),
                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                 ImageSize(RTWidth, RTHeight));
    if (!pRenderTextureTex)
        return;
    ((Render::D3D12::Texture*)pRenderTextureTex.GetPtr())->Texture0.RTV = pRTV;

    // Now create the render target from our rendered texture (with depth/stencil).
    pRenderTarget = *phal->CreateRenderTarget(pRenderTextureTex.GetPtr(), true);

    // Create the quad-mesh (on which the rendered movie will be placed), and initialize it with the vertices.
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
    bufdesc.Width              = sizeof(QuadVertices);
    bufdesc.Height             = 1;
    bufdesc.DepthOrArraySize   = 1;
    bufdesc.MipLevels          = 1;
    bufdesc.Format             = DXGI_FORMAT_UNKNOWN;
    bufdesc.SampleDesc.Count   = 1;
    bufdesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufdesc.Flags              = D3D12_RESOURCE_FLAG_NONE;
    if (FAILED(pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufdesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ,
                                                0, IID_PPV_ARGS(&pQuadVertexBuffer.GetRawRef()))))
        return;
    void* pvbData = 0;
    D3D12_RANGE zeroRange = { 0, 0 };
    if (SUCCEEDED(pQuadVertexBuffer->Map(0, &zeroRange, &pvbData)))
    {
        memcpy(pvbData, QuadVertices, sizeof(QuadVertices));
        pQuadVertexBuffer->Unmap(0, 0);
    }

    // Create the constant buffer for the vertex shader (16 floats: WorldViewProj matrix).
    bufdesc.Width = 16 * sizeof(float);
    if (FAILED(pDevice->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE, &bufdesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ,
                                                0, IID_PPV_ARGS(&pVShaderUniforms.GetRawRef()))))
        return;

    // Create the root signature: one root constant-buffer view (b0) + one descriptor table (t0, s0).
    D3D12_DESCRIPTOR_RANGE ranges[2];
    ranges[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    ranges[0].NumDescriptors                    = 1;
    ranges[0].BaseShaderRegister                = 0;
    ranges[0].RegisterSpace                     = 0;
    ranges[0].OffsetInDescriptorsFromTableStart = 0;
    ranges[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    ranges[1].NumDescriptors                    = 1;
    ranges[1].BaseShaderRegister                = 0;
    ranges[1].RegisterSpace                     = 0;
    ranges[1].OffsetInDescriptorsFromTableStart = 1;
    D3D12_ROOT_PARAMETER rootParams[2];
    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParams[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 2;
    rootParams[1].DescriptorTable.pDescriptorRanges   = ranges;
    rootParams[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters     = 2;
    rsDesc.pParameters       = rootParams;
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

    // Compile the quad shaders (DXIL, shader model 6.0).
    Ptr<ID3DBlob> pVS, pPS, pErr;
    static const char* vsSource =
        "cbuffer WVP : register(b0) { float4x4 WorldViewProj; };\n"
        "struct VSIn { float3 pos : POSITION; float2 uv : TEXCOORD; };\n"
        "struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };\n"
        "VSOut main(VSIn i) { VSOut o; o.pos = mul(float4(i.pos, 1.0), WorldViewProj); o.uv = i.uv; return o; }\n";
    static const char* psSource =
        "Texture2D tex : register(t0);\n"
        "SamplerState smp : register(s0);\n"
        "struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };\n"
        "float4 main(VSOut i) : SV_Target { return tex.Sample(smp, i.uv); }\n";
    if (FAILED(D3DCompile(vsSource, strlen(vsSource), "quadVS", 0, 0, "main", "vs_6_0",
                          D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pVS.GetRawRef(), &pErr.GetRawRef())))
    {
        SF_DEBUG_MESSAGE(1, "Failed to compile the quad vertex shader.");
        return;
    }
    if (FAILED(D3DCompile(psSource, strlen(psSource), "quadPS", 0, 0, "main", "ps_6_0",
                          D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pPS.GetRawRef(), &pErr.GetRawRef())))
    {
        SF_DEBUG_MESSAGE(1, "Failed to compile the quad pixel shader.");
        return;
    }

    // Create the pipeline state object.
    static D3D12_INPUT_ELEMENT_DESC quadMeshElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature        = pRootSignature.GetPtr();
    psoDesc.VS.pShaderBytecode    = pVS->GetBufferPointer();
    psoDesc.VS.BytecodeLength     = pVS->GetBufferSize();
    psoDesc.PS.pShaderBytecode    = pPS->GetBufferPointer();
    psoDesc.PS.BytecodeLength     = pPS->GetBufferSize();
    psoDesc.InputLayout.pInputElementDescs = quadMeshElements;
    psoDesc.InputLayout.NumElements        = 2;
    psoDesc.RasterizerState.FillMode       = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode       = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable    = FALSE;
    psoDesc.DepthStencilState.StencilEnable  = FALSE;
    psoDesc.BlendState.RenderTarget[0].BlendEnable           = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask           = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets     = 1;
    psoDesc.RTVFormats[0]        = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count     = 1;
    if (FAILED(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPipelineState.GetRawRef()))))
    {
        SF_DEBUG_MESSAGE(1, "Failed to create the quad pipeline state.");
        return;
    }
}

// This function renders a quad on the screen, with the GFx movie rendered texture applied to it.
void FxPlayerSWFToTextureAppD3D12::RenderMovieTextureToQuad()
{
    Render::D3D12::HAL* phal = reinterpret_cast<Render::D3D12::HAL*>(pPlatformHAL);
    ID3D12GraphicsCommandList* pcl = phal->GetCommandList();
    if (!pcl || !pPipelineState || !pRootSignature || !pRenderTexture)
        return;

    // The movie was just rendered into the render texture; transition it to a shader resource.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = pRenderTexture.GetPtr();
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    pcl->ResourceBarrier(1, &barrier);

    // Bind our descriptor heaps (they are GPU visible).
    ID3D12DescriptorHeap* ppHeaps[] = { pSRVHeap.GetPtr(), pSamplerHeap.GetPtr() };
    pcl->SetDescriptorHeaps(2, ppHeaps);

    pcl->SetGraphicsRootSignature(pRootSignature.GetPtr());
    pcl->SetPipelineState(pPipelineState.GetPtr());
    pcl->SetGraphicsRootDescriptorTable(1,
        pSRVHeap->GetGPUDescriptorHandleForHeapStart());
    pcl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pQuadVertexBuffer->GetGPUVirtualAddress();
    vbView.SizeInBytes    = sizeof(Vertex) * 4;
    vbView.StrideInBytes  = sizeof(Vertex);
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

    // Now draw the quad.
    pcl->DrawInstanced(4, 1, 0, 0);

    // Restore the HAL's descriptor heaps.
    Render::D3D12::TextureManager* pmanager = (Render::D3D12::TextureManager*)pPlatformHAL->GetTextureManager();
    if (pmanager && pmanager->GetSRVHeap() && pmanager->GetSamplerHeap())
    {
        ID3D12DescriptorHeap* pHalHeaps[] = { pmanager->GetSRVHeap(), pmanager->GetSamplerHeap() };
        pcl->SetDescriptorHeaps(2, pHalHeaps);
    }
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(SWFToTexture, Scaleform::GFx::System, FxPlayerSWFToTextureAppD3D12)
