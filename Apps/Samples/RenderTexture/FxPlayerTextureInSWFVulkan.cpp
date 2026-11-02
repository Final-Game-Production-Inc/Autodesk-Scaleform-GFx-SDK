/**************************************************************************

Filename    :   FxPlayerTextureInSWFVulkan.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (Vulkan)
Created     :   2026
Authors     :   Scaleform Vulkan Backend

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerTextureInSWFVulkan.h"

#include "Render/Vulkan/Vulkan_Texture.h"

//------------------------------------------------------------------------
// ***** Cube shaders (SPIR-V, generated with glslangValidator from:)
//
// cube.vert:
//   #version 450
//   layout(location=0) in vec3 inPos;
//   layout(location=1) in vec4 inColor;
//   layout(binding=0) uniform UBO { mat4 WorldViewProj; } ubo;
//   layout(location=0) out vec4 outColor;
//   void main() { gl_Position = ubo.WorldViewProj * vec4(inPos, 1.0); outColor = inColor; }
//
// cube.frag:
//   #version 450
//   layout(location=0) in vec4 inColor;
//   layout(location=0) out vec4 outColor;
//   void main() { outColor = inColor; }
//------------------------------------------------------------------------

static const uint32_t gCubeVS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000027, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0009000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000019, 0x00000023,
    0x00000025, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b,
    0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67,
    0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002, 0x435f6c67, 0x4470696c,
    0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
    0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00030005, 0x00000011, 0x004f4255, 0x00070006,
    0x00000011, 0x00000000, 0x6c726f57, 0x65695664, 0x6f725077, 0x0000006a, 0x00030005, 0x00000013,
    0x006f6275, 0x00040005, 0x00000019, 0x6f506e69, 0x00000073, 0x00050005, 0x00000023, 0x4374756f,
    0x726f6c6f, 0x00000000, 0x00040005, 0x00000025, 0x6f436e69, 0x00726f6c, 0x00030047, 0x0000000b,
    0x00000002, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b,
    0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003,
    0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x00000011, 0x00000002,
    0x00040048, 0x00000011, 0x00000000, 0x00000005, 0x00050048, 0x00000011, 0x00000000, 0x00000007,
    0x00000010, 0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000013,
    0x00000021, 0x00000000, 0x00040047, 0x00000013, 0x00000022, 0x00000000, 0x00040047, 0x00000019,
    0x0000001e, 0x00000000, 0x00040047, 0x00000023, 0x0000001e, 0x00000000, 0x00040047, 0x00000025,
    0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008,
    0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a,
    0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a,
    0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003,
    0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
    0x00040018, 0x00000010, 0x00000007, 0x00000004, 0x0003001e, 0x00000011, 0x00000010, 0x00040020,
    0x00000012, 0x00000002, 0x00000011, 0x0004003b, 0x00000012, 0x00000013, 0x00000002, 0x00040020,
    0x00000014, 0x00000002, 0x00000010, 0x00040017, 0x00000017, 0x00000006, 0x00000003, 0x00040020,
    0x00000018, 0x00000001, 0x00000017, 0x0004003b, 0x00000018, 0x00000019, 0x00000001, 0x0004002b,
    0x00000006, 0x0000001b, 0x3f800000, 0x00040020, 0x00000021, 0x00000003, 0x00000007, 0x0004003b,
    0x00000021, 0x00000023, 0x00000003, 0x00040020, 0x00000024, 0x00000001, 0x00000007, 0x0004003b,
    0x00000024, 0x00000025, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x00050041, 0x00000014, 0x00000015, 0x00000013, 0x0000000f, 0x0004003d,
    0x00000010, 0x00000016, 0x00000015, 0x0004003d, 0x00000017, 0x0000001a, 0x00000019, 0x00050051,
    0x00000006, 0x0000001c, 0x0000001a, 0x00000000, 0x00050051, 0x00000006, 0x0000001d, 0x0000001a,
    0x00000001, 0x00050051, 0x00000006, 0x0000001e, 0x0000001a, 0x00000002, 0x00070050, 0x00000007,
    0x0000001f, 0x0000001c, 0x0000001d, 0x0000001e, 0x0000001b, 0x00050091, 0x00000007, 0x00000020,
    0x00000016, 0x0000001f, 0x00050041, 0x00000021, 0x00000022, 0x0000000d, 0x0000000f, 0x0003003e,
    0x00000022, 0x00000020, 0x0004003d, 0x00000007, 0x00000026, 0x00000025, 0x0003003e, 0x00000023,
    0x00000026, 0x000100fd, 0x00010038,
};

static const uint32_t gCubePS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000000d, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040005, 0x0000000b,
    0x6f436e69, 0x00726f6c, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000b,
    0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008,
    0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00050036, 0x00000002,
    0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c,
    0x0000000b, 0x0003003e, 0x00000009, 0x0000000c, 0x000100fd, 0x00010038,
};

//------------------------------------------------------------------------

FxPlayerTextureInSWFAppVulkan::FxPlayerTextureInSWFAppVulkan() :
    FxPlayerTextureInSWFApp(),
    pHAL(0),
    pCubeVertexBuffer(VK_NULL_HANDLE),
    pCubeVertexMemory(VK_NULL_HANDLE),
    pCubeUniformBuffer(VK_NULL_HANDLE),
    pCubeUniformMemory(VK_NULL_HANDLE),
    pCubeUniformData(0),
    pCubeDescriptorPool(VK_NULL_HANDLE),
    pCubeDescriptorSetLayout(VK_NULL_HANDLE),
    pCubeDescriptorSet(VK_NULL_HANDLE),
    pCubePipelineLayout(VK_NULL_HANDLE),
    pCubePipeline(VK_NULL_HANDLE),
    CubePipelineReady(false),
    pDepthImage(VK_NULL_HANDLE),
    pDepthMemory(VK_NULL_HANDLE),
    pDepthView(VK_NULL_HANDLE),
    pRenderFramebuffer(VK_NULL_HANDLE)
{
}

FxPlayerTextureInSWFAppVulkan::~FxPlayerTextureInSWFAppVulkan()
{
    ReleaseGraphicsResources();
}

void FxPlayerTextureInSWFAppVulkan::OnShutdown()
{
    ReleaseGraphicsResources();
    FxPlayerTextureInSWFAppBase::OnShutdown();
}

// Releases the Vulkan resources created by the sample. The rendered texture is
// owned by the GFx TextureManager (released with the Ptr), so only the
// resources created directly with vk* calls are freed here.
void FxPlayerTextureInSWFAppVulkan::ReleaseGraphicsResources()
{
    VkDevice device = pHAL ? pHAL->GetDevice() : VK_NULL_HANDLE;
    if (!device)
        return;

    if (pRenderFramebuffer != VK_NULL_HANDLE) { vkDestroyFramebuffer(device, pRenderFramebuffer, 0); pRenderFramebuffer = VK_NULL_HANDLE; }
    if (pDepthView != VK_NULL_HANDLE)         { vkDestroyImageView(device, pDepthView, 0); pDepthView = VK_NULL_HANDLE; }
    if (pDepthImage != VK_NULL_HANDLE)        { vkDestroyImage(device, pDepthImage, 0); pDepthImage = VK_NULL_HANDLE; }
    if (pDepthMemory != VK_NULL_HANDLE)       { vkFreeMemory(device, pDepthMemory, 0); pDepthMemory = VK_NULL_HANDLE; }
    if (pCubePipeline != VK_NULL_HANDLE)          { vkDestroyPipeline(device, pCubePipeline, 0); pCubePipeline = VK_NULL_HANDLE; }
    if (pCubePipelineLayout != VK_NULL_HANDLE)    { vkDestroyPipelineLayout(device, pCubePipelineLayout, 0); pCubePipelineLayout = VK_NULL_HANDLE; }
    if (pCubeDescriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, pCubeDescriptorSetLayout, 0); pCubeDescriptorSetLayout = VK_NULL_HANDLE; }
    if (pCubeDescriptorPool != VK_NULL_HANDLE)    { vkDestroyDescriptorPool(device, pCubeDescriptorPool, 0); pCubeDescriptorPool = VK_NULL_HANDLE; }
    if (pCubeUniformData)
        vkUnmapMemory(device, pCubeUniformMemory);
    pCubeUniformData = 0;
    if (pCubeUniformBuffer != VK_NULL_HANDLE)     { vkDestroyBuffer(device, pCubeUniformBuffer, 0); pCubeUniformBuffer = VK_NULL_HANDLE; }
    if (pCubeUniformMemory != VK_NULL_HANDLE)     { vkFreeMemory(device, pCubeUniformMemory, 0); pCubeUniformMemory = VK_NULL_HANDLE; }
    if (pCubeVertexBuffer != VK_NULL_HANDLE)      { vkDestroyBuffer(device, pCubeVertexBuffer, 0); pCubeVertexBuffer = VK_NULL_HANDLE; }
    if (pCubeVertexMemory != VK_NULL_HANDLE)      { vkFreeMemory(device, pCubeVertexMemory, 0); pCubeVertexMemory = VK_NULL_HANDLE; }
    CubePipelineReady = false;
}

// Selects a depth/stencil format supported by the physical device, matching
// the one used by the HAL for its offscreen render pass.
static VkFormat GetDepthStencilFormat(VkPhysicalDevice physDevice)
{
    VkFormat formats[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };
    for (unsigned i = 0; i < 2; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physDevice, formats[i], &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return formats[i];
    }
    return VK_FORMAT_D24_UNORM_S8_UINT;
}

// Creates the offscreen render-texture, the cube pipeline and all related
// Vulkan resources.
void FxPlayerTextureInSWFAppVulkan::InitGraphicsResources()
{
    pHAL = (Render::Vulkan::HAL*)pPlatformHAL;
    if (!pHAL || !pHAL->GetDevice())
        return;

    VkDevice device = pHAL->GetDevice();
    VkPhysicalDevice physDevice = pHAL->pPhysicalDevice;

    //------------------------------------------------------------------------
    // 1) Create the rendered texture (RGBA8, render-target + sampled).
    //------------------------------------------------------------------------
    VkImageCreateInfo imageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCI.imageType     = VK_IMAGE_TYPE_2D;
    imageCI.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imageCI.extent        = { OriginalResSize.Width, OriginalResSize.Height, 1 };
    imageCI.mipLevels     = 1;
    imageCI.arrayLayers   = 1;
    imageCI.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage        rtImage  = VK_NULL_HANDLE;
    VkDeviceMemory rtMemory = VK_NULL_HANDLE;
    VkImageView    rtView   = VK_NULL_HANDLE;
    if (vkCreateImage(device, &imageCI, 0, &rtImage) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the rendered texture image.");
        return;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, rtImage, &memReq);
    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(physDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &rtMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate rendered texture memory.");
        vkDestroyImage(device, rtImage, 0);
        return;
    }
    vkBindImageMemory(device, rtImage, rtMemory, 0);

    VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewCI.image            = rtImage;
    viewCI.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format           = VK_FORMAT_R8G8B8A8_UNORM;
    viewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(device, &viewCI, 0, &rtView) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the rendered texture view.");
        vkFreeMemory(device, rtMemory, 0);
        vkDestroyImage(device, rtImage, 0);
        return;
    }

    // Wrap the image with the GFx TextureManager (the texture then owns the
    // image/view/memory and releases them through the manager's kill lists).
    Render::Vulkan::TextureManager* pmanager = (Render::Vulkan::TextureManager*)pHAL->GetTextureManager();
    pMyHWTexture = *pmanager->CreateTexture(rtImage, rtView, OriginalResSize);
    if (!pMyHWTexture)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to wrap the rendered texture.");
        vkDestroyImageView(device, rtView, 0);
        vkFreeMemory(device, rtMemory, 0);
        vkDestroyImage(device, rtImage, 0);
        return;
    }

    //------------------------------------------------------------------------
    // 2) Depth/stencil surface for the offscreen cube rendering.
    //------------------------------------------------------------------------
    VkFormat dsFormat = GetDepthStencilFormat(physDevice);
    imageCI.format = dsFormat;
    imageCI.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (vkCreateImage(device, &imageCI, 0, &pDepthImage) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the depth image.");
        return;
    }
    vkGetImageMemoryRequirements(device, pDepthImage, &memReq);
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(physDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &pDepthMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate depth image memory.");
        vkDestroyImage(device, pDepthImage, 0); pDepthImage = VK_NULL_HANDLE;
        return;
    }
    vkBindImageMemory(device, pDepthImage, pDepthMemory, 0);
    viewCI.image            = pDepthImage;
    viewCI.format           = dsFormat;
    viewCI.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(device, &viewCI, 0, &pDepthView) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the depth image view.");
        return;
    }

    //------------------------------------------------------------------------
    // 3) Framebuffer (color + depth) using the HAL's offscreen render pass.
    //------------------------------------------------------------------------
    VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbCI.renderPass      = pHAL->pOffscreenRenderPassDS;
    VkImageView attachments[2] = { rtView, pDepthView };
    fbCI.attachmentCount = 2;
    fbCI.pAttachments    = attachments;
    fbCI.width           = OriginalResSize.Width;
    fbCI.height          = OriginalResSize.Height;
    fbCI.layers          = 1;
    if (vkCreateFramebuffer(device, &fbCI, 0, &pRenderFramebuffer) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the offscreen framebuffer.");
        return;
    }

    //------------------------------------------------------------------------
    // 4) Cube vertex buffer (host-visible, one-time upload).
    //------------------------------------------------------------------------
    VkBufferCreateInfo bufferCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCI.size        = sizeof(CubeVertices);
    bufferCI.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufferCI, 0, &pCubeVertexBuffer) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube vertex buffer.");
        return;
    }
    vkGetBufferMemoryRequirements(device, pCubeVertexBuffer, &memReq);
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(physDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &pCubeVertexMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the cube vertex buffer memory.");
        vkDestroyBuffer(device, pCubeVertexBuffer, 0); pCubeVertexBuffer = VK_NULL_HANDLE;
        return;
    }
    vkBindBufferMemory(device, pCubeVertexBuffer, pCubeVertexMemory, 0);
    void* pdata = 0;
    vkMapMemory(device, pCubeVertexMemory, 0, sizeof(CubeVertices), 0, &pdata);
    if (pdata)
    {
        memcpy(pdata, CubeVertices, sizeof(CubeVertices));
        vkUnmapMemory(device, pCubeVertexMemory);
    }

    //------------------------------------------------------------------------
    // 5) Cube uniform buffer (WorldViewProj matrix, updated per frame).
    //------------------------------------------------------------------------
    bufferCI.size  = 16 * sizeof(float);
    bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (vkCreateBuffer(device, &bufferCI, 0, &pCubeUniformBuffer) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube uniform buffer.");
        return;
    }
    vkGetBufferMemoryRequirements(device, pCubeUniformBuffer, &memReq);
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(physDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &pCubeUniformMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the cube uniform buffer memory.");
        vkDestroyBuffer(device, pCubeUniformBuffer, 0); pCubeUniformBuffer = VK_NULL_HANDLE;
        return;
    }
    vkBindBufferMemory(device, pCubeUniformBuffer, pCubeUniformMemory, 0);
    vkMapMemory(device, pCubeUniformMemory, 0, 16 * sizeof(float), 0, &pCubeUniformData);

    //------------------------------------------------------------------------
    // 6) Cube descriptor set layout: b0 = UBO (vertex). Independent from the
    //    HAL's own descriptor layout.
    //------------------------------------------------------------------------
    VkDescriptorSetLayoutBinding bindings[1] = {};
    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo dslCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dslCI.bindingCount = 1;
    dslCI.pBindings    = bindings;
    if (vkCreateDescriptorSetLayout(device, &dslCI, 0, &pCubeDescriptorSetLayout) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube descriptor set layout.");
        return;
    }

    VkDescriptorPoolSize poolSizes[1] = {};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    VkDescriptorPoolCreateInfo poolCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolCI.maxSets       = 1;
    poolCI.poolSizeCount = 1;
    poolCI.pPoolSizes    = poolSizes;
    if (vkCreateDescriptorPool(device, &poolCI, 0, &pCubeDescriptorPool) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube descriptor pool.");
        return;
    }

    VkDescriptorSetAllocateInfo setAI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    setAI.descriptorPool     = pCubeDescriptorPool;
    setAI.descriptorSetCount = 1;
    setAI.pSetLayouts        = &pCubeDescriptorSetLayout;
    if (vkAllocateDescriptorSets(device, &setAI, &pCubeDescriptorSet) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the cube descriptor set.");
        return;
    }

    VkDescriptorBufferInfo bufferInfo = { pCubeUniformBuffer, 0, 16 * sizeof(float) };
    VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstSet          = pCubeDescriptorSet;
    write.dstBinding      = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo     = &bufferInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, 0);

    //------------------------------------------------------------------------
    // 7) Cube pipeline layout + pipeline (against the offscreen render pass).
    //------------------------------------------------------------------------
    VkPipelineLayoutCreateInfo plCI = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plCI.setLayoutCount = 1;
    plCI.pSetLayouts    = &pCubeDescriptorSetLayout;
    if (vkCreatePipelineLayout(device, &plCI, 0, &pCubePipelineLayout) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube pipeline layout.");
        return;
    }

    // Vertex input: {float x,y,z; uchar r,g,b,a} - 16 bytes per vertex.
    VkVertexInputBindingDescription vbinding = {};
    vbinding.binding   = 0;
    vbinding.stride    = sizeof(Vertex);
    vbinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vattrs[2] = {};
    vattrs[0].binding  = 0;
    vattrs[0].location = 0;
    vattrs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vattrs[0].offset   = 0;
    vattrs[1].binding  = 0;
    vattrs[1].location = 1;
    vattrs[1].format   = VK_FORMAT_R8G8B8A8_UNORM;
    vattrs[1].offset   = 12;

    VkPipelineVertexInputStateCreateInfo viCI = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    viCI.vertexBindingDescriptionCount   = 1;
    viCI.pVertexBindingDescriptions      = &vbinding;
    viCI.vertexAttributeDescriptionCount = 2;
    viCI.pVertexAttributeDescriptions    = vattrs;

    VkPipelineInputAssemblyStateCreateInfo iaCI = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    iaCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    VkPipelineViewportStateCreateInfo vpCI = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vpCI.viewportCount = 1;
    vpCI.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rsCI = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rsCI.polygonMode = VK_POLYGON_MODE_FILL;
    rsCI.cullMode    = VK_CULL_MODE_NONE;
    rsCI.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo msCI = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    msCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blendAtt = {};
    blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cbCI = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cbCI.attachmentCount = 1;
    cbCI.pAttachments    = &blendAtt;

    VkPipelineDepthStencilStateCreateInfo dsCI = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    dsCI.depthTestEnable  = VK_TRUE;
    dsCI.depthWriteEnable = VK_TRUE;
    dsCI.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkShaderModule vsModule = VK_NULL_HANDLE, fsModule = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo smCI = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smCI.codeSize = sizeof(gCubeVS);
    smCI.pCode    = gCubeVS;
    if (vkCreateShaderModule(device, &smCI, 0, &vsModule) != VK_SUCCESS) return;
    smCI.codeSize = sizeof(gCubePS);
    smCI.pCode    = gCubePS;
    if (vkCreateShaderModule(device, &smCI, 0, &fsModule) != VK_SUCCESS)
    {
        vkDestroyShaderModule(device, vsModule, 0);
        return;
    }

    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vsModule;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fsModule;
    stages[1].pName  = "main";

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynCI = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynCI.dynamicStateCount = 2;
    dynCI.pDynamicStates    = dynamicStates;

    VkGraphicsPipelineCreateInfo gpCI = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    gpCI.stageCount          = 2;
    gpCI.pStages             = stages;
    gpCI.pVertexInputState   = &viCI;
    gpCI.pInputAssemblyState = &iaCI;
    gpCI.pViewportState      = &vpCI;
    gpCI.pRasterizationState = &rsCI;
    gpCI.pMultisampleState   = &msCI;
    gpCI.pDepthStencilState  = &dsCI;
    gpCI.pColorBlendState    = &cbCI;
    gpCI.pDynamicState       = &dynCI;
    gpCI.layout              = pCubePipelineLayout;
    gpCI.renderPass          = pHAL->pOffscreenRenderPassDS;
    gpCI.subpass             = 0;

    VkPipelineCacheCreateInfo cacheCI = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    VkPipelineCache cache = VK_NULL_HANDLE;
    if (vkCreatePipelineCache(device, &cacheCI, 0, &cache) != VK_SUCCESS)
    {
        vkDestroyShaderModule(device, vsModule, 0);
        vkDestroyShaderModule(device, fsModule, 0);
        return;
    }

    VkResult res = vkCreateGraphicsPipelines(device, cache, 1, &gpCI, 0, &pCubePipeline);
    vkDestroyPipelineCache(device, cache, 0);
    vkDestroyShaderModule(device, vsModule, 0);
    vkDestroyShaderModule(device, fsModule, 0);
    if (res != VK_SUCCESS || pCubePipeline == VK_NULL_HANDLE)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the cube graphics pipeline.");
        return;
    }

    CubePipelineReady = true;
    SF_DEBUG_MESSAGE(1, "Vulkan: TextureInSWF graphics resources initialized.");
}

// Renders the colored cube into the offscreen render-texture. This runs
// outside of the GFx frame command buffer (after the frame has been
// submitted), so a dedicated command buffer is recorded, submitted and
// waited on. The offscreen render pass leaves the texture in
// SHADER_READ_ONLY_OPTIMAL, ready to be sampled by the SWF movie.
void FxPlayerTextureInSWFAppVulkan::RenderMesh()
{
    if (!pHAL || !pMyHWTexture || !CubePipelineReady || !pRenderFramebuffer)
        return;

    VkDevice device = pHAL->GetDevice();

    // Update the WorldViewProj matrix (transposed, matching D3D convention).
    ComputeMatrices();
    if (pCubeUniformData)
        memcpy(pCubeUniformData, WorldViewProjMatrix, 16 * sizeof(float));

    // Allocate a temporary command buffer from the HAL's command pool.
    VkCommandBufferAllocateInfo allocCI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocCI.commandPool        = pHAL->pCommandPool;
    allocCI.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocCI.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &allocCI, &cmd) != VK_SUCCESS)
        return;

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, pHAL->pCommandPool, 1, &cmd);
        return;
    }

    // Begin the offscreen render pass (clear to dark blue, matching D3D12).
    VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rpBI.renderPass  = pHAL->pOffscreenRenderPassDS;
    rpBI.framebuffer = pRenderFramebuffer;
    rpBI.renderArea.offset = { 0, 0 };
    rpBI.renderArea.extent = { OriginalResSize.Width, OriginalResSize.Height };
    VkClearValue clears[2] = {};
    clears[0].color.float32[0] = 0.0f;
    clears[0].color.float32[1] = 0.0f;
    clears[0].color.float32[2] = 1.0f;
    clears[0].color.float32[3] = 0.35f;
    clears[1].depthStencil.depth   = 1.0f;
    clears[1].depthStencil.stencil = 0;
    rpBI.clearValueCount = 2;
    rpBI.pClearValues    = clears;
    vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.width  = (float)OriginalResSize.Width;
    viewport.height = (float)OriginalResSize.Height;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.extent = { OriginalResSize.Width, OriginalResSize.Height };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pCubePipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pCubePipelineLayout,
                            0, 1, &pCubeDescriptorSet, 0, 0);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &pCubeVertexBuffer, &offset);

    // The cube mesh is stored as 6 triangle-strips of 4 vertices each.
    for (unsigned i = 0; i < 6; i++)
        vkCmdDraw(cmd, 4, 1, i * 4, 0);

    vkCmdEndRenderPass(cmd);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        vkFreeCommandBuffers(device, pHAL->pCommandPool, 1, &cmd);
        return;
    }

    // Submit and wait, so the SWF movie can sample the texture next frame.
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, pHAL->GraphicsQueueFamily, 0, &queue);
    if (queue != VK_NULL_HANDLE)
    {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &cmd;
        if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS)
            vkQueueWaitIdle(queue);
    }

    vkFreeCommandBuffers(device, pHAL->pCommandPool, 1, &cmd);
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(TextureInSWF, Scaleform::GFx::System, FxPlayerTextureInSWFAppVulkan)
