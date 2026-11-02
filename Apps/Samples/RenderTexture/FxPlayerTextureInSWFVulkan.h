/**************************************************************************

Filename    :   FxPlayerTextureInSWFVulkan.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (Vulkan)
Created     :   2026
Authors     :   Scaleform Vulkan Backend

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerTextureInSWFVulkan_H
#define FX_PlayerTextureInSWFVulkan_H

#include "FxPlayerTextureInSWF.h"

#include <vulkan/vulkan.h>

#include "Render/Vulkan/Vulkan_HAL.h"

// TextureInSWF (Vulkan) sample application.
//
// Renders a colored cube mesh into an offscreen render-texture, then replaces
// a texture inside the SWF movie with that render-texture via
// GFxTextureManager::ReplaceTexture. The cube is rendered with a dedicated
// command buffer (submitted and waited on), since it runs outside of the
// GFx frame command buffer.
class FxPlayerTextureInSWFAppVulkan : public FxPlayerTextureInSWFApp
{
public:
    FxPlayerTextureInSWFAppVulkan();
    virtual ~FxPlayerTextureInSWFAppVulkan();

    virtual void                 RenderMesh();
    virtual void                 InitGraphicsResources();

protected:
    virtual void                 OnShutdown();

private:
    void                         ReleaseGraphicsResources();

    // HAL / device shortcuts.
    Render::Vulkan::HAL*         pHAL;

    // The rendered texture (RGBA8 offscreen image wrapped as a GFx texture).
    Ptr<Render::Texture>         pMyHWTexture;          // GFx texture wrapping pRenderTextureImage.

    // Cube mesh + uniform buffers (host-visible, one-time upload).
    VkBuffer                     pCubeVertexBuffer;     // CubeVertices (24 * 16 bytes).
    VkDeviceMemory               pCubeVertexMemory;
    VkBuffer                     pCubeUniformBuffer;    // WorldViewProj matrix (64 bytes).
    VkDeviceMemory               pCubeUniformMemory;
    void*                        pCubeUniformData;      // Persistent mapping of pCubeUniformBuffer.

    // Cube pipeline (independent descriptor layout / pipeline layout / pool).
    VkDescriptorPool             pCubeDescriptorPool;
    VkDescriptorSetLayout        pCubeDescriptorSetLayout;
    VkDescriptorSet              pCubeDescriptorSet;
    VkPipelineLayout             pCubePipelineLayout;
    VkPipeline                   pCubePipeline;
    bool                         CubePipelineReady;

    // Depth/stencil surface used while rendering the cube into the texture.
    VkImage                      pDepthImage;
    VkDeviceMemory               pDepthMemory;
    VkImageView                  pDepthView;

    // Framebuffer (color + depth) for rendering the cube offscreen.
    VkFramebuffer                pRenderFramebuffer;
};

#endif // FX_PlayerTextureInSWFVulkan_H
