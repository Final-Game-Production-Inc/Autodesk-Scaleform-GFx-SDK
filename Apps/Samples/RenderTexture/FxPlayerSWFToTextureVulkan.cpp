/**************************************************************************

Filename    :   FxPlayerSWFToTextureVulkan.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API (Vulkan)
Created     :   2026
Authors     :   Scaleform Vulkan Backend

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerSWFToTextureVulkan.h"

#include "Render/Vulkan/Vulkan_Texture.h"

//------------------------------------------------------------------------
// ***** Quad shaders (SPIR-V, generated with glslangValidator from:)
//
// quad.vert:
//   #version 450
//   layout(location=0) in vec3 inPos;
//   layout(location=1) in vec2 inUV;
//   layout(binding=0) uniform UBO { mat4 WorldViewProj; } ubo;
//   layout(location=0) out vec2 outUV;
//   void main() { gl_Position = ubo.WorldViewProj * vec4(inPos, 1.0); outUV = inUV; }
//
// quad.frag:
//   #version 450
//   layout(binding=1) uniform sampler2D texSampler;
//   layout(location=0) in vec2 inUV;
//   layout(location=0) out vec4 outColor;
//   void main() { outColor = texture(texSampler, inUV); }
//------------------------------------------------------------------------

static const uint32_t gQuadVS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000029, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0009000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000019, 0x00000025,
    0x00000027, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b,
    0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67,
    0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002, 0x435f6c67, 0x4470696c,
    0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
    0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00030005, 0x00000011, 0x004f4255, 0x00070006,
    0x00000011, 0x00000000, 0x6c726f57, 0x65695664, 0x6f725077, 0x0000006a, 0x00030005, 0x00000013,
    0x006f6275, 0x00040005, 0x00000019, 0x6f506e69, 0x00000073, 0x00040005, 0x00000025, 0x5574756f,
    0x00000056, 0x00040005, 0x00000027, 0x56556e69, 0x00000000, 0x00030047, 0x0000000b, 0x00000002,
    0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001,
    0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x00000011, 0x00000002, 0x00040048,
    0x00000011, 0x00000000, 0x00000005, 0x00050048, 0x00000011, 0x00000000, 0x00000007, 0x00000010,
    0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000013, 0x00000021,
    0x00000000, 0x00040047, 0x00000013, 0x00000022, 0x00000000, 0x00040047, 0x00000019, 0x0000001e,
    0x00000000, 0x00040047, 0x00000025, 0x0000001e, 0x00000000, 0x00040047, 0x00000027, 0x0000001e,
    0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020,
    0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006,
    0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020,
    0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015,
    0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040018,
    0x00000010, 0x00000007, 0x00000004, 0x0003001e, 0x00000011, 0x00000010, 0x00040020, 0x00000012,
    0x00000002, 0x00000011, 0x0004003b, 0x00000012, 0x00000013, 0x00000002, 0x00040020, 0x00000014,
    0x00000002, 0x00000010, 0x00040017, 0x00000017, 0x00000006, 0x00000003, 0x00040020, 0x00000018,
    0x00000001, 0x00000017, 0x0004003b, 0x00000018, 0x00000019, 0x00000001, 0x0004002b, 0x00000006,
    0x0000001b, 0x3f800000, 0x00040020, 0x00000021, 0x00000003, 0x00000007, 0x00040017, 0x00000023,
    0x00000006, 0x00000002, 0x00040020, 0x00000024, 0x00000003, 0x00000023, 0x0004003b, 0x00000024,
    0x00000025, 0x00000003, 0x00040020, 0x00000026, 0x00000001, 0x00000023, 0x0004003b, 0x00000026,
    0x00000027, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x00050041, 0x00000014, 0x00000015, 0x00000013, 0x0000000f, 0x0004003d, 0x00000010,
    0x00000016, 0x00000015, 0x0004003d, 0x00000017, 0x0000001a, 0x00000019, 0x00050051, 0x00000006,
    0x0000001c, 0x0000001a, 0x00000000, 0x00050051, 0x00000006, 0x0000001d, 0x0000001a, 0x00000001,
    0x00050051, 0x00000006, 0x0000001e, 0x0000001a, 0x00000002, 0x00070050, 0x00000007, 0x0000001f,
    0x0000001c, 0x0000001d, 0x0000001e, 0x0000001b, 0x00050091, 0x00000007, 0x00000020, 0x00000016,
    0x0000001f, 0x00050041, 0x00000021, 0x00000022, 0x0000000d, 0x0000000f, 0x0003003e, 0x00000022,
    0x00000020, 0x0004003d, 0x00000023, 0x00000028, 0x00000027, 0x0003003e, 0x00000025, 0x00000028,
    0x000100fd, 0x00010038,
};

static const uint32_t gQuadPS[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000014, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000011, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000d,
    0x53786574, 0x6c706d61, 0x00007265, 0x00040005, 0x00000011, 0x56556e69, 0x00000000, 0x00040047,
    0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x00000021, 0x00000001, 0x00040047,
    0x0000000d, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x0000001e, 0x00000000, 0x00020013,
    0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
    0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
    0x00000008, 0x00000009, 0x00000003, 0x00090019, 0x0000000a, 0x00000006, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020,
    0x0000000c, 0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040017,
    0x0000000f, 0x00000006, 0x00000002, 0x00040020, 0x00000010, 0x00000001, 0x0000000f, 0x0004003b,
    0x00000010, 0x00000011, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003d, 0x0000000b, 0x0000000e, 0x0000000d, 0x0004003d, 0x0000000f,
    0x00000012, 0x00000011, 0x00050057, 0x00000007, 0x00000013, 0x0000000e, 0x00000012, 0x0003003e,
    0x00000009, 0x00000013, 0x000100fd, 0x00010038,
};

//------------------------------------------------------------------------

FxPlayerSWFToTextureAppVulkan::FxPlayerSWFToTextureAppVulkan() :
    FxPlayerSWFToTextureApp(),
    pHAL(0),
    pQuadVertexBuffer(VK_NULL_HANDLE),
    pQuadVertexMemory(VK_NULL_HANDLE),
    pQuadUniformBuffer(VK_NULL_HANDLE),
    pQuadUniformMemory(VK_NULL_HANDLE),
    pQuadUniformData(0),
    pQuadDescriptorPool(VK_NULL_HANDLE),
    pQuadDescriptorSetLayout(VK_NULL_HANDLE),
    pQuadDescriptorSet(VK_NULL_HANDLE),
    pQuadPipelineLayout(VK_NULL_HANDLE),
    pQuadPipeline(VK_NULL_HANDLE),
    pQuadSampler(VK_NULL_HANDLE),
    QuadPipelineReady(false)
{
}

FxPlayerSWFToTextureAppVulkan::~FxPlayerSWFToTextureAppVulkan()
{
    ReleaseGraphicsResources();
}

void FxPlayerSWFToTextureAppVulkan::OnShutdown()
{
    ReleaseGraphicsResources();
    FxPlayerSWFToTextureAppBase::OnShutdown();
}

// Releases the Vulkan resources created by the sample. The render texture is
// owned by the GFx TextureManager (released with the Ptr), so only the quad
// pipeline resources created directly with vk* calls are freed here.
void FxPlayerSWFToTextureAppVulkan::ReleaseGraphicsResources()
{
    VkDevice device = pHAL ? pHAL->GetDevice() : VK_NULL_HANDLE;
    if (!device)
        return;

    if (pQuadPipeline != VK_NULL_HANDLE)          { vkDestroyPipeline(device, pQuadPipeline, 0); pQuadPipeline = VK_NULL_HANDLE; }
    if (pQuadPipelineLayout != VK_NULL_HANDLE)    { vkDestroyPipelineLayout(device, pQuadPipelineLayout, 0); pQuadPipelineLayout = VK_NULL_HANDLE; }
    if (pQuadDescriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, pQuadDescriptorSetLayout, 0); pQuadDescriptorSetLayout = VK_NULL_HANDLE; }
    if (pQuadDescriptorPool != VK_NULL_HANDLE)    { vkDestroyDescriptorPool(device, pQuadDescriptorPool, 0); pQuadDescriptorPool = VK_NULL_HANDLE; }
    if (pQuadUniformData)
        vkUnmapMemory(device, pQuadUniformMemory);
    pQuadUniformData = 0;
    if (pQuadUniformBuffer != VK_NULL_HANDLE)     { vkDestroyBuffer(device, pQuadUniformBuffer, 0); pQuadUniformBuffer = VK_NULL_HANDLE; }
    if (pQuadUniformMemory != VK_NULL_HANDLE)     { vkFreeMemory(device, pQuadUniformMemory, 0); pQuadUniformMemory = VK_NULL_HANDLE; }
    if (pQuadVertexBuffer != VK_NULL_HANDLE)      { vkDestroyBuffer(device, pQuadVertexBuffer, 0); pQuadVertexBuffer = VK_NULL_HANDLE; }
    if (pQuadVertexMemory != VK_NULL_HANDLE)      { vkFreeMemory(device, pQuadVertexMemory, 0); pQuadVertexMemory = VK_NULL_HANDLE; }
    QuadPipelineReady = false;
}

// Creates the offscreen render texture and the quad-mesh pipeline resources.
void FxPlayerSWFToTextureAppVulkan::InitGraphicsResources()
{
    pHAL = (Render::Vulkan::HAL*)pPlatformHAL;
    if (!pHAL || !pHAL->GetDevice())
        return;

    VkDevice device = pHAL->GetDevice();

    //------------------------------------------------------------------------
    // 1) Create the rendered texture (RGBA8, render-target + sampled).
    //------------------------------------------------------------------------
    VkImageCreateInfo imageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCI.imageType     = VK_IMAGE_TYPE_2D;
    imageCI.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imageCI.extent        = { RTWidth, RTHeight, 1 };
    imageCI.mipLevels     = 1;
    imageCI.arrayLayers   = 1;
    imageCI.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage         rtImage   = VK_NULL_HANDLE;
    VkDeviceMemory  rtMemory  = VK_NULL_HANDLE;
    VkImageView     rtView    = VK_NULL_HANDLE;
    if (vkCreateImage(device, &imageCI, 0, &rtImage) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the rendered texture image.");
        return;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, rtImage, &memReq);
    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(pHAL->pPhysicalDevice, memReq.memoryTypeBits,
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
    pRenderTextureTex = *pmanager->CreateTexture(rtImage, rtView, ImageSize(RTWidth, RTHeight));
    if (!pRenderTextureTex)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to wrap the rendered texture.");
        vkDestroyImageView(device, rtView, 0);
        vkFreeMemory(device, rtMemory, 0);
        vkDestroyImage(device, rtImage, 0);
        return;
    }

    // Create the GFx render target (with depth/stencil) from the texture.
    pRenderTarget = *pHAL->CreateRenderTarget(pRenderTextureTex.GetPtr(), true);
    if (!pRenderTarget)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the render target from the rendered texture.");
        pRenderTextureTex.Clear();
        return;
    }

    //------------------------------------------------------------------------
    // 2) Quad vertex buffer (host-visible, one-time upload).
    //------------------------------------------------------------------------
    VkBufferCreateInfo bufferCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCI.size        = sizeof(QuadVertices);
    bufferCI.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufferCI, 0, &pQuadVertexBuffer) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad vertex buffer.");
        return;
    }
    vkGetBufferMemoryRequirements(device, pQuadVertexBuffer, &memReq);
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(pHAL->pPhysicalDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &pQuadVertexMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the quad vertex buffer memory.");
        vkDestroyBuffer(device, pQuadVertexBuffer, 0); pQuadVertexBuffer = VK_NULL_HANDLE;
        return;
    }
    vkBindBufferMemory(device, pQuadVertexBuffer, pQuadVertexMemory, 0);
    void* pdata = 0;
    vkMapMemory(device, pQuadVertexMemory, 0, sizeof(QuadVertices), 0, &pdata);
    if (pdata)
    {
        memcpy(pdata, QuadVertices, sizeof(QuadVertices));
        vkUnmapMemory(device, pQuadVertexMemory);
    }

    //------------------------------------------------------------------------
    // 3) Quad uniform buffer (WorldViewProj matrix, updated per frame).
    //------------------------------------------------------------------------
    bufferCI.size  = 16 * sizeof(float);
    bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (vkCreateBuffer(device, &bufferCI, 0, &pQuadUniformBuffer) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad uniform buffer.");
        return;
    }
    vkGetBufferMemoryRequirements(device, pQuadUniformBuffer, &memReq);
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = Render::Vulkan::FindMemoryType(pHAL->pPhysicalDevice, memReq.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(device, &allocInfo, 0, &pQuadUniformMemory) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the quad uniform buffer memory.");
        vkDestroyBuffer(device, pQuadUniformBuffer, 0); pQuadUniformBuffer = VK_NULL_HANDLE;
        return;
    }
    vkBindBufferMemory(device, pQuadUniformBuffer, pQuadUniformMemory, 0);
    vkMapMemory(device, pQuadUniformMemory, 0, 16 * sizeof(float), 0, &pQuadUniformData);

    //------------------------------------------------------------------------
    // 4) Quad descriptor set layout: b0 = UBO (vertex), b1 = combined image
    //    sampler (fragment). Independent from the HAL's own descriptor layout.
    //------------------------------------------------------------------------
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo dslCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dslCI.bindingCount = 2;
    dslCI.pBindings    = bindings;
    if (vkCreateDescriptorSetLayout(device, &dslCI, 0, &pQuadDescriptorSetLayout) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad descriptor set layout.");
        return;
    }

    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;
    VkDescriptorPoolCreateInfo poolCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolCI.maxSets       = 1;
    poolCI.poolSizeCount = 2;
    poolCI.pPoolSizes    = poolSizes;
    if (vkCreateDescriptorPool(device, &poolCI, 0, &pQuadDescriptorPool) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad descriptor pool.");
        return;
    }

    VkDescriptorSetAllocateInfo setAI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    setAI.descriptorPool     = pQuadDescriptorPool;
    setAI.descriptorSetCount = 1;
    setAI.pSetLayouts        = &pQuadDescriptorSetLayout;
    if (vkAllocateDescriptorSets(device, &setAI, &pQuadDescriptorSet) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to allocate the quad descriptor set.");
        return;
    }

    // Sampler (shared, owned by the TextureManager).
    pQuadSampler = pmanager->GetSampler(ImageFillMode(Wrap_Clamp, Sample_Linear));

    // Bind the UBO + the rendered texture view to the descriptor set.
    VkImageView rtImageView = ((Render::Vulkan::Texture*)pRenderTextureTex.GetPtr())->pTextures[0].View;
    VkDescriptorBufferInfo bufferInfo = { pQuadUniformBuffer, 0, 16 * sizeof(float) };
    VkDescriptorImageInfo  imageInfo  = { pQuadSampler,
                                          rtImageView,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet           = pQuadDescriptorSet;
    writes[0].dstBinding       = 0;
    writes[0].descriptorCount  = 1;
    writes[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo      = &bufferInfo;
    writes[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet           = pQuadDescriptorSet;
    writes[1].dstBinding       = 1;
    writes[1].descriptorCount  = 1;
    writes[1].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo       = &imageInfo;
    vkUpdateDescriptorSets(device, 2, writes, 0, 0);

    //------------------------------------------------------------------------
    // 5) Quad pipeline layout. The pipeline itself is created lazily in
    //    RenderMovieTextureToQuad, because it must be created against the
    //    main (swapchain) render pass, which is only known once the platform
    //    has set it via HAL::SetMainRenderTarget during the first frame.
    //------------------------------------------------------------------------
    VkPipelineLayoutCreateInfo plCI = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plCI.setLayoutCount = 1;
    plCI.pSetLayouts    = &pQuadDescriptorSetLayout;
    if (vkCreatePipelineLayout(device, &plCI, 0, &pQuadPipelineLayout) != VK_SUCCESS)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad pipeline layout.");
        return;
    }

    SF_DEBUG_MESSAGE(1, "Vulkan: SWFToTexture graphics resources initialized.");
}

// Creates the quad graphics pipeline against the current main render pass.
// The main pass format is the swapchain format (B8G8R8A8_UNORM by default).
bool FxPlayerSWFToTextureAppVulkan::InitQuadPipeline()
{
    if (pQuadPipeline != VK_NULL_HANDLE)
        return true;
    if (!pHAL || pHAL->pMainRenderPass == VK_NULL_HANDLE || pQuadPipelineLayout == VK_NULL_HANDLE)
        return false;

    VkDevice device = pHAL->GetDevice();

    // Vertex input: {float x,y,z; float u,v} - 20 bytes per vertex.
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
    vattrs[1].format   = VK_FORMAT_R32G32_SFLOAT;
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
    blendAtt.blendEnable    = VK_TRUE;
    blendAtt.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAtt.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAtt.colorBlendOp        = VK_BLEND_OP_ADD;
    blendAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAtt.alphaBlendOp        = VK_BLEND_OP_ADD;
    blendAtt.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cbCI = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cbCI.attachmentCount = 1;
    cbCI.pAttachments    = &blendAtt;

    VkPipelineDepthStencilStateCreateInfo dsCI = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

    VkShaderModule vsModule = VK_NULL_HANDLE, fsModule = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo smCI = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smCI.codeSize = sizeof(gQuadVS);
    smCI.pCode    = gQuadVS;
    if (vkCreateShaderModule(device, &smCI, 0, &vsModule) != VK_SUCCESS) return false;
    smCI.codeSize = sizeof(gQuadPS);
    smCI.pCode    = gQuadPS;
    if (vkCreateShaderModule(device, &smCI, 0, &fsModule) != VK_SUCCESS)
    {
        vkDestroyShaderModule(device, vsModule, 0);
        return false;
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
    gpCI.layout              = pQuadPipelineLayout;
    // Created against the main (swapchain) render pass, B8G8R8A8_UNORM.
    gpCI.renderPass          = pHAL->pMainRenderPass;
    gpCI.subpass             = 0;

    VkPipelineCacheCreateInfo cacheCI = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    VkPipelineCache cache = VK_NULL_HANDLE;
    if (vkCreatePipelineCache(device, &cacheCI, 0, &cache) != VK_SUCCESS)
    {
        vkDestroyShaderModule(device, vsModule, 0);
        vkDestroyShaderModule(device, fsModule, 0);
        return false;
    }

    VkResult res = vkCreateGraphicsPipelines(device, cache, 1, &gpCI, 0, &pQuadPipeline);
    vkDestroyPipelineCache(device, cache, 0);
    vkDestroyShaderModule(device, vsModule, 0);
    vkDestroyShaderModule(device, fsModule, 0);
    if (res != VK_SUCCESS || pQuadPipeline == VK_NULL_HANDLE)
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: Failed to create the quad graphics pipeline.");
        return false;
    }

    QuadPipelineReady = true;
    return true;
}

// Draws a quad with the movie render texture applied, inside the current
// render pass (or by resuming the main pass when the HAL is not inside one).
void FxPlayerSWFToTextureAppVulkan::RenderMovieTextureToQuad()
{
    if (!pHAL || !pRenderTarget || !pRenderTextureTex)
        return;

    // The quad pipeline must be created against the main render pass, which
    // is set by the platform at the beginning of each frame.
    if (!InitQuadPipeline() || !QuadPipelineReady)
        return;

    VkCommandBuffer cmd = pHAL->GetCommandBuffer();
    if (cmd == VK_NULL_HANDLE)
        return;

    // If the HAL is not currently inside a render pass, resume the main pass
    // (LOAD_OP_LOAD) so the quad can be drawn; the platform ends it when the
    // frame is presented.
    bool resumed = false;
    if (!pHAL->InRenderPass)
    {
        if (pHAL->pMainResumeRenderPass == VK_NULL_HANDLE || pHAL->pMainFramebuffer == VK_NULL_HANDLE)
            return;
        VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpBI.renderPass  = pHAL->pMainResumeRenderPass;
        rpBI.framebuffer = pHAL->pMainFramebuffer;
        rpBI.renderArea.extent = pHAL->MainRenderExtent;
        vkCmdBeginRenderPass(cmd, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
        resumed = true;
    }

    // Update the WorldViewProj matrix (transposed, matching D3D convention).
    ComputeMatrices();
    if (pQuadUniformData)
        memcpy(pQuadUniformData, WorldViewProjMatrix, 16 * sizeof(float));

    // Dynamic viewport / scissor covering the whole swapchain.
    VkViewport viewport = {};
    viewport.width  = (float)pHAL->MainRenderExtent.width;
    viewport.height = (float)pHAL->MainRenderExtent.height;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.extent = pHAL->MainRenderExtent;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pQuadPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pQuadPipelineLayout,
                            0, 1, &pQuadDescriptorSet, 0, 0);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &pQuadVertexBuffer, &offset);
    vkCmdDraw(cmd, 4, 1, 0, 0);

    if (resumed)
        vkCmdEndRenderPass(cmd);
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(SWFToTexture, Scaleform::GFx::System, FxPlayerSWFToTextureAppVulkan)
