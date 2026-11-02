/**************************************************************************

Filename    :   FxPlayerTextureInSWFD3D12.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (D3D12)
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerTextureInSWFD3D12_H
#define FX_PlayerTextureInSWFD3D12_H

#include "FxPlayerTextureInSWF.h"

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Render/D3D12/D3D12_HAL.h"

class FxPlayerTextureInSWFAppD3D12 : public FxPlayerTextureInSWFApp
{
public:
    virtual void                     RenderMesh();
    virtual void                     InitGraphicsResources();

protected:

    Ptr<ID3D12Device>                pDevice;               // The device (used for creating resources)

    Ptr<ID3D12Resource>              pRenderTexture;        // The render texture.
    Ptr<ID3D12DescriptorHeap>        pRTVHeap;              // RTV heap for the rendered texture.
    D3D12_CPU_DESCRIPTOR_HANDLE      pRTV;                  // RTV handle of the rendered texture.
    D3D12_CPU_DESCRIPTOR_HANDLE      pRenderTextureView;    // Alias for pRTV.
    Ptr<ID3D12Resource>              pRTDepthStencilBuf;    // The depth/stencil surface used when rendering the rendered texture.
    Ptr<ID3D12DescriptorHeap>        pDSVHeap;              // DSV heap for the depth/stencil surface.
    D3D12_CPU_DESCRIPTOR_HANDLE      pDSV;                  // DSV handle of the depth/stencil surface.

    Ptr<ID3D12Resource>              pCubeVertexBuffer;     // The vertex buffer, containing the cube mesh data.
    Ptr<ID3D12Resource>              pVShaderUniforms;      // The buffer containing the uniforms (shader constants), used with the vertex shader.
    Ptr<ID3D12RootSignature>         pRootSignature;        // The root signature.
    Ptr<ID3D12PipelineState>         pPipelineState;        // The pipeline state (PSO).
};

#endif
