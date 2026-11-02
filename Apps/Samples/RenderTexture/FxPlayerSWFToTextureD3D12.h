/**************************************************************************

Filename    :   FxPlayerSWFToTextureD3D12.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (D3D12)
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerSWFToTextureD3D12_H
#define FX_PlayerSWFToTextureD3D12_H

#include "FxPlayerSWFToTexture.h"

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Render/D3D12/D3D12_HAL.h"

class FxPlayerSWFToTextureAppD3D12 : public FxPlayerSWFToTextureApp
{
public:
    virtual void    InitGraphicsResources();

protected:
    virtual void    RenderMovieTextureToQuad();

    Ptr<ID3D12Device>                pDevice;               // The device (used for creating resources)

    Ptr<ID3D12Resource>              pRenderTexture;        // The render texture (the movie renders into this).
    Ptr<Render::Texture>             pRenderTextureTex;     // The GFx texture wrapping the render texture.
    Ptr<ID3D12DescriptorHeap>        pRTVHeap;              // RTV heap for the render texture.
    D3D12_CPU_DESCRIPTOR_HANDLE      pRTV;                  // RTV handle of the render texture.
    Ptr<ID3D12DescriptorHeap>        pSRVHeap;              // GPU-visible SRV heap (sampled by the quad).
    D3D12_GPU_DESCRIPTOR_HANDLE      pRenderTextureSV;      // GPU handle of the render texture SRV.
    Ptr<ID3D12DescriptorHeap>        pSamplerHeap;          // GPU-visible sampler heap.
    D3D12_GPU_DESCRIPTOR_HANDLE      pSampler;              // GPU handle of the linear-clamp sampler.

    Ptr<ID3D12Resource>              pQuadVertexBuffer;     // The vertex buffer, containing the quad mesh data.
    Ptr<ID3D12Resource>              pVShaderUniforms;      // The buffer containing the uniforms (shader constants), used with the vertex shader.
    Ptr<ID3D12RootSignature>         pRootSignature;        // The root signature.
    Ptr<ID3D12PipelineState>         pPipelineState;        // The pipeline state (PSO).
};

#endif
