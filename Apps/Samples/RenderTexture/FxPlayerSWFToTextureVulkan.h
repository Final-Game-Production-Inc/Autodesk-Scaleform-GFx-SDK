/**************************************************************************

Filename    :   FxPlayerSWFToTextureVulkan.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (Vulkan)
Created     :   2026
Authors     :   Scaleform Vulkan Backend

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerSWFToTextureVulkan_H
#define FX_PlayerSWFToTextureVulkan_H

#include "FxPlayerSWFToTexture.h"

#include <vulkan/vulkan.h>

#include "Render/Vulkan/Vulkan_HAL.h"

// SWFToTexture (Vulkan) sample application.
//
// Renders a GFx movie into an offscreen render-texture (using the HAL's
// PushRenderTarget / PopRenderTarget mechanism), then draws a quad with that
// texture applied on top of the scene. All Vulkan resources are created
// directly on the HAL's device/queue so they can be mixed with the GFx
// command buffer each frame.
class FxPlayerSWFToTextureAppVulkan : public FxPlayerSWFToTextureApp
{
public:
    FxPlayerSWFToTextureAppVulkan();
    virtual ~FxPlayerSWFToTextureAppVulkan();

    virtual void                 InitGraphicsResources();

protected:
    virtual void                 OnShutdown();
    virtual void                 RenderMovieTextureToQuad();

private:
    // Creates the offscreen image + GFx texture + quad pipeline resources.
    bool                         InitQuadPipeline();
    void                         ReleaseGraphicsResources();

    // HAL / device shortcuts.
    Render::Vulkan::HAL*         pHAL;                  // The Vulkan HAL.

    // The rendered texture (RGBA8 offscreen image wrapped as a GFx texture).
    Ptr<Render::Texture>         pRenderTextureTex;     // GFx texture wrapping pRenderTextureImage.

    // Quad mesh + uniform buffers (host-visible, one-time upload).
    VkBuffer                     pQuadVertexBuffer;     // QuadVertices (4 * 20 bytes).
    VkDeviceMemory               pQuadVertexMemory;
    VkBuffer                     pQuadUniformBuffer;    // WorldViewProj matrix (64 bytes).
    VkDeviceMemory               pQuadUniformMemory;
    void*                        pQuadUniformData;      // Persistent mapping of pQuadUniformBuffer.

    // Quad pipeline (independent descriptor layout / pipeline layout / pool).
    VkDescriptorPool             pQuadDescriptorPool;
    VkDescriptorSetLayout        pQuadDescriptorSetLayout;
    VkDescriptorSet              pQuadDescriptorSet;
    VkPipelineLayout             pQuadPipelineLayout;
    VkPipeline                   pQuadPipeline;
    VkSampler                    pQuadSampler;          // Shared sampler owned by the TextureManager.
    bool                         QuadPipelineReady;
};

#endif // FX_PlayerSWFToTextureVulkan_H
