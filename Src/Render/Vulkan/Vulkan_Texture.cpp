/**************************************************************************

Filename    :   Vulkan_Texture.cpp
Content     :   Vulkan texture implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Render/Vulkan/Vulkan_Texture.h"
#include "Render/Vulkan/Vulkan_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_TextureCacheGeneric.h"
#include "Render/Render_TextureUtil.h"

namespace Scaleform { namespace Render { namespace Vulkan {

// *** Format mapping table
static const TextureFormat::Mapping TextureFormatMapping[] =
{
    { Image_R8G8B8A8,   VK_FORMAT_R8G8B8A8_UNORM,          4, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,   VK_FORMAT_B8G8R8A8_UNORM,          4, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_R8G8B8,     VK_FORMAT_R8G8B8A8_UNORM,          4, &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,     VK_FORMAT_B8G8R8A8_UNORM,          4, &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_A8,         VK_FORMAT_R8_UNORM,                 1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2,  VK_FORMAT_R8_UNORM,                 1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,VK_FORMAT_R8_UNORM,                1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_DXT1,       VK_FORMAT_BC1_RGBA_UNORM_BLOCK,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_DXT3,       VK_FORMAT_BC2_UNORM_BLOCK,          0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_DXT5,       VK_FORMAT_BC3_UNORM_BLOCK,          0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_None,       VK_FORMAT_UNDEFINED,                0, 0, 0 }
};


// *** Texture

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use, ImageBase* pimage)
: Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat)
, pShadowData(0), ShadowSize(0)
{
    TextureCount = (UByte)ImageData::GetFormatPlaneCount(GetImageFormat());
    if (TextureCount > 1)
        pTextures = (HWTextureDesc*)SF_HEAP_AUTO_ALLOC(this, sizeof(HWTextureDesc)*TextureCount);
    else
        pTextures = &Texture0;
    memset(pTextures, 0, sizeof(HWTextureDesc) * TextureCount);
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, VkImage image, VkImageView view,
                 ImageSize imgSize, ImageBase* pimage)
: Render::Texture(pmanagerLocks, imgSize, 0, 0, pimage, 0)
, pShadowData(0), ShadowSize(0)
{
    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    memset(pTextures, 0, sizeof(HWTextureDesc));
    pTextures[0].Image = image;
    pTextures[0].View  = view;
    pTextures[0].Size  = imgSize;
}

Texture::~Texture()
{
    if ((State == State_Valid) || (State == State_Lost))
        ReleaseHWTextures();
    if (pTextures && pTextures != &Texture0)
        SF_FREE(pTextures);
    if (pShadowData)
        SF_FREE(pShadowData);
}

void Texture::ReleaseHWTextures(bool)
{
    TextureManager* pmgr = GetManager();
    bool userAlloc = (TextureFlags & TF_UserAlloc) != 0;

    for (unsigned i = 0; i < TextureCount; i++)
    {
        if (!userAlloc)
        {
            if (pTextures[i].View != VK_NULL_HANDLE)
            {
                pmgr->ViewKills[pmgr->KillSlot].PushBack(pTextures[i].View);
                pTextures[i].View = VK_NULL_HANDLE;
            }
            if (pTextures[i].Image != VK_NULL_HANDLE)
            {
                pmgr->ImageKills[pmgr->KillSlot].PushBack(pTextures[i].Image);
                pTextures[i].Image = VK_NULL_HANDLE;
            }
            if (pTextures[i].Memory != VK_NULL_HANDLE)
            {
                pmgr->MemoryKills[pmgr->KillSlot].PushBack(pTextures[i].Memory);
                pTextures[i].Memory = VK_NULL_HANDLE;
            }
        }
        else
        {
            pTextures[i].View   = VK_NULL_HANDLE;
            pTextures[i].Image  = VK_NULL_HANDLE;
            pTextures[i].Memory = VK_NULL_HANDLE;
        }
    }
}

bool Texture::Initialize()
{
    TextureManager* pmgr = GetManager();
    VkDevice device = pmgr->GetDevice();
    VkPhysicalDevice physDevice = pmgr->GetPhysicalDevice();

    if (TextureFlags & TF_UserAlloc)
    {
        State = State_Valid;
        return true;
    }

    const TextureFormat::Mapping* mapping = GetTextureFormatMapping();
    if (!mapping)
        return false;

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        ImageSize sz = ImageData::GetFormatPlaneSize(GetImageFormat(), ImgSize, itex);
        pTextures[itex].Size = sz;

        VkImageCreateInfo imageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCI.imageType   = VK_IMAGE_TYPE_2D;
        imageCI.format      = mapping->VkFmt;
        imageCI.extent      = { (uint32_t)sz.Width, (uint32_t)sz.Height, 1 };
        imageCI.mipLevels   = MipLevels > 0 ? MipLevels : 1;
        imageCI.arrayLayers = 1;
        imageCI.samples     = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling      = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (Use & ImageUse_RenderTarget)
            imageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        SF_VK_CHECK_RETURN(vkCreateImage(device, &imageCI, nullptr, &pTextures[itex].Image), false);

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, pTextures[itex].Image, &memReqs);

        VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (allocInfo.memoryTypeIndex == UINT32_MAX)
        {
            SF_DEBUG_WARNING(1, "Vulkan: No suitable memory type for texture");
            return false;
        }
        SF_VK_CHECK_RETURN(vkAllocateMemory(device, &allocInfo, nullptr, &pTextures[itex].Memory), false);
        SF_VK_CHECK_RETURN(vkBindImageMemory(device, pTextures[itex].Image, pTextures[itex].Memory, 0), false);

        VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewCI.image    = pTextures[itex].Image;
        viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format   = mapping->VkFmt;
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.levelCount = imageCI.mipLevels;
        viewCI.subresourceRange.layerCount = 1;

        SF_VK_CHECK_RETURN(vkCreateImageView(device, &viewCI, nullptr, &pTextures[itex].View), false);
    }

    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "Vulkan::Texture::Initialize - initial data upload failed");
        ReleaseHWTextures();
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return true;
}

bool Texture::Initialize(VkImage image, VkImageView view)
{
    pTextures[0].Image = image;
    pTextures[0].View  = view;
    State = State_Valid;
    return true;
}

void Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    TextureManager* pmgr = GetManager();
    VkSampler sampler = pmgr->GetSampler(fm);
    for (unsigned i = 0; i < TextureCount; i++)
    {
        pmgr->SetSamplerState(stageIndex + i, 1, &pTextures[i].View, sampler);
    }
}

void Texture::GetUVGenMatrix(Matrix2F* mat) const
{
    ImageSize sz = GetTextureSize(0);
    *mat = Matrix2F::Scaling(1.0f / (float)ImgSize.Width, 1.0f / (float)ImgSize.Height);
}

bool Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    if (!updates || !count)
        return false;

    TextureManager* pmgr = GetManager();
    VkDevice device = pmgr->GetDevice();
    VkPhysicalDevice physDevice = pmgr->GetPhysicalDevice();
    VkCommandPool cmdPool = pmgr->GetCommandPool();
    VkQueue queue = pmgr->GetQueue();

    const TextureFormat::Mapping* mapping = GetTextureFormatMapping();
    if (!mapping || mapping->BytesPerPixel == 0)
        return false;

    // Collect staging resources so we can batch all copies into a single
    // command buffer and wait only once instead of once per update.
    struct StagingEntry { VkBuffer buf; VkDeviceMemory mem; };
    StagingEntry* stagingEntries = (StagingEntry*)SF_ALLOC(count * sizeof(StagingEntry), StatRender_TextureManager_Mem);
    if (!stagingEntries)
        return false;
    memset(stagingEntries, 0, count * sizeof(StagingEntry));

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc& ud = updates[i];
        unsigned plane = 0;
        ImageSize sz = pTextures[plane].Size;
        unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> mipLevel);
        unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> mipLevel);

        UPInt rowBytes = (UPInt)mipW * mapping->BytesPerPixel;
        UPInt totalSize = rowBytes * mipH;

        VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCI.size = totalSize;
        bufCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bufCI, nullptr, &stagingEntries[i].buf) != VK_SUCCESS)
            goto cleanup_fail;

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, stagingEntries[i].buf, &memReqs);

        VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (allocInfo.memoryTypeIndex == UINT32_MAX)
            goto cleanup_fail;
        if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingEntries[i].mem) != VK_SUCCESS)
            goto cleanup_fail;
        vkBindBufferMemory(device, stagingEntries[i].buf, stagingEntries[i].mem, 0);

        void* mapped;
        vkMapMemory(device, stagingEntries[i].mem, 0, totalSize, 0, &mapped);
        const UByte* src = ud.SourcePlane.pData;
        UByte* dst = (UByte*)mapped;
        for (unsigned row = 0; row < mipH; row++)
        {
            memcpy(dst, src, rowBytes);
            dst += rowBytes;
            src += ud.SourcePlane.Pitch;
        }
        vkUnmapMemory(device, stagingEntries[i].mem);
    }

    // Single command buffer for all copies
    {
        VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cbAI.commandPool = cmdPool;
        cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAI.commandBufferCount = 1;
        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(device, &cbAI, &cmd);

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        for (unsigned i = 0; i < count; i++)
        {
            unsigned plane = 0;
            ImageSize sz = pTextures[plane].Size;
            unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> mipLevel);
            unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> mipLevel);

            pmgr->TransitionImageLayout(cmd, pTextures[plane].Image,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = { mipW, mipH, 1 };
            vkCmdCopyBufferToImage(cmd, stagingEntries[i].buf, pTextures[plane].Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            pmgr->TransitionImageLayout(cmd, pTextures[plane].Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
    }

    for (unsigned i = 0; i < count; i++)
    {
        if (stagingEntries[i].buf != VK_NULL_HANDLE) vkDestroyBuffer(device, stagingEntries[i].buf, nullptr);
        if (stagingEntries[i].mem != VK_NULL_HANDLE) vkFreeMemory(device, stagingEntries[i].mem, nullptr);
    }
    SF_FREE(stagingEntries);
    return true;

cleanup_fail:
    for (unsigned i = 0; i < count; i++)
    {
        if (stagingEntries[i].buf != VK_NULL_HANDLE) vkDestroyBuffer(device, stagingEntries[i].buf, nullptr);
        if (stagingEntries[i].mem != VK_NULL_HANDLE) vkFreeMemory(device, stagingEntries[i].mem, nullptr);
    }
    SF_FREE(stagingEntries);
    return false;
}

void Texture::computeUpdateConvertRescaleFlags(bool rescale, bool swMipGen, ImageFormat inputFormat,
                                                ImageRescaleType& rescaleType, ImageFormat& rescaleBuffFromat, bool& convert)
{
    SF_UNUSED2(rescale, swMipGen);
    rescaleType = ResizeNone;
    rescaleBuffFromat = inputFormat;
    convert = false;
}


// *** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size)
: Render::DepthStencilSurface(pmanagerLocks, size),
  pDepthStencilImage(VK_NULL_HANDLE), pDepthStencilView(VK_NULL_HANDLE),
  pDepthStencilMemory(VK_NULL_HANDLE), DepthStencilFormat(VK_FORMAT_D24_UNORM_S8_UINT)
{
}

DepthStencilSurface::~DepthStencilSurface()
{
    TextureManager* pmgr = (TextureManager*)pManagerLocks->pManager;
    VkDevice device = pmgr->GetDevice();
    if (pDepthStencilView != VK_NULL_HANDLE)
        vkDestroyImageView(device, pDepthStencilView, nullptr);
    if (pDepthStencilImage != VK_NULL_HANDLE)
        vkDestroyImage(device, pDepthStencilImage, nullptr);
    if (pDepthStencilMemory != VK_NULL_HANDLE)
        vkFreeMemory(device, pDepthStencilMemory, nullptr);
}

bool DepthStencilSurface::Initialize()
{
    TextureManager* pmgr = (TextureManager*)pManagerLocks->pManager;
    VkDevice device = pmgr->GetDevice();
    VkPhysicalDevice physDevice = pmgr->GetPhysicalDevice();

    // Try D24_UNORM_S8_UINT first, then D32_SFLOAT_S8_UINT
    VkFormat formats[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };
    VkFormat selectedFormat = VK_FORMAT_UNDEFINED;
    for (auto fmt : formats)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physDevice, fmt, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            selectedFormat = fmt;
            break;
        }
    }
    if (selectedFormat == VK_FORMAT_UNDEFINED)
        return false;

    DepthStencilFormat = selectedFormat;

    VkImageCreateInfo imageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCI.imageType   = VK_IMAGE_TYPE_2D;
    imageCI.format      = selectedFormat;
    imageCI.extent      = { (uint32_t)Size.Width, (uint32_t)Size.Height, 1 };
    imageCI.mipLevels   = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SF_VK_CHECK_RETURN(vkCreateImage(device, &imageCI, nullptr, &pDepthStencilImage), false);

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, pDepthStencilImage, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX)
    {
        SF_DEBUG_WARNING(1, "Vulkan: No suitable memory type for depth/stencil surface");
        return false;
    }
    SF_VK_CHECK_RETURN(vkAllocateMemory(device, &allocInfo, nullptr, &pDepthStencilMemory), false);
    SF_VK_CHECK_RETURN(vkBindImageMemory(device, pDepthStencilImage, pDepthStencilMemory, 0), false);

    VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewCI.image    = pDepthStencilImage;
    viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format   = selectedFormat;
    viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    viewCI.subresourceRange.levelCount = 1;
    viewCI.subresourceRange.layerCount = 1;

    SF_VK_CHECK_RETURN(vkCreateImageView(device, &viewCI, nullptr, &pDepthStencilView), false);

    State = Render::Texture::State_Valid;
    return true;
}


// *** MappedTexture

bool MappedTexture::Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(!IsMapped());
    SF_ASSERT((mipLevel + levelCount) <= ptexture->MipLevels);

    Texture* vktex = (Texture*)ptexture;
    TextureManager* pmgr = vktex->GetManager();
    VkDevice device = pmgr->GetDevice();
    VkPhysicalDevice physDevice = pmgr->GetPhysicalDevice();

    if (levelCount == 0)
        levelCount = 1;

    const TextureFormat::Mapping* mapping = vktex->GetTextureFormatMapping();
    if (!mapping || mapping->BytesPerPixel == 0)
        return false;

    unsigned texPlaneCount = vktex->TextureCount;
    if (texPlaneCount > PlaneReserveSize)
        return false;

    UPInt totalSize = 0;
    for (unsigned itex = 0; itex < texPlaneCount; itex++)
    {
        ImageSize sz = vktex->pTextures[itex].Size;
        for (unsigned level = mipLevel; level < mipLevel + levelCount; level++)
        {
            unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> level);
            unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> level);
            totalSize += (UPInt)mipW * mipH * mapping->BytesPerPixel;
        }
    }

    VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCI.size = totalSize;
    bufCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufCI, nullptr, &pStagingBuffer) != VK_SUCCESS)
        return false;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, pStagingBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX)
    {
        vkDestroyBuffer(device, pStagingBuffer, nullptr);
        pStagingBuffer = VK_NULL_HANDLE;
        return false;
    }

    if (vkAllocateMemory(device, &allocInfo, nullptr, &pStagingMemory) != VK_SUCCESS)
    {
        vkDestroyBuffer(device, pStagingBuffer, nullptr);
        pStagingBuffer = VK_NULL_HANDLE;
        return false;
    }
    vkBindBufferMemory(device, pStagingBuffer, pStagingMemory, 0);

    void* mapped = nullptr;
    vkMapMemory(device, pStagingMemory, 0, totalSize, 0, &mapped);

    if (vktex->pShadowData && vktex->ShadowSize == totalSize)
        memcpy(mapped, vktex->pShadowData, totalSize);
    else
        memset(mapped, 0, totalSize);

    pTexture    = ptexture;
    StartMipLevel = mipLevel;
    LevelCount    = levelCount;

    // Initialize Data once before the loop, using pre-allocated Planes array
    // (matches D3D11 pattern). Calling Initialize inside the loop would Clear()
    // previously set planes on each iteration, corrupting multi-plane textures (YUV).
    if (levelCount <= PlaneReserveSize)
        Data.Initialize(vktex->GetImageFormat(), levelCount, Planes, texPlaneCount * levelCount, true);
    else if (!Data.Initialize(vktex->GetImageFormat(), levelCount, true))
        return false;

    UByte* pdata = (UByte*)mapped;
    unsigned planeIdx = 0;
    for (unsigned itex = 0; itex < texPlaneCount; itex++)
    {
        ImageSize sz = vktex->pTextures[itex].Size;
        for (unsigned level = mipLevel; level < mipLevel + levelCount; level++)
        {
            unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> level);
            unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> level);
            UPInt pitch = (UPInt)mipW * mapping->BytesPerPixel;
            UPInt planeSize = pitch * mipH;

            Data.SetPlane(planeIdx, ImageSize(mipW, mipH), pitch, planeSize, pdata);
            pdata += planeSize;
            planeIdx++;
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool applyUpdate)
{
    if (!IsMapped())
        return;

    Texture* vktex = (Texture*)pTexture;
    TextureManager* pmgr = vktex->GetManager();
    VkDevice device = pmgr->GetDevice();
    VkCommandPool cmdPool = pmgr->GetCommandPool();
    VkQueue queue = pmgr->GetQueue();

    if (applyUpdate)
    {
        const TextureFormat::Mapping* mapping = vktex->GetTextureFormatMapping();
        UPInt totalSize = 0;
        for (unsigned itex = 0; itex < vktex->TextureCount; itex++)
        {
            ImageSize sz = vktex->pTextures[itex].Size;
            for (unsigned level = StartMipLevel; level < StartMipLevel + LevelCount; level++)
            {
                unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> level);
                unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> level);
                totalSize += (UPInt)mipW * mipH * mapping->BytesPerPixel;
            }
        }

        if (!vktex->pShadowData || vktex->ShadowSize != totalSize)
        {
            if (vktex->pShadowData)
                SF_FREE(vktex->pShadowData);
            vktex->pShadowData = (UByte*)SF_ALLOC(totalSize, StatRender_TextureManager_Mem);
            vktex->ShadowSize = totalSize;
        }
        if (vktex->pShadowData)
        {
            void* base = Data.GetPlaneRef(0).pData;
            memcpy(vktex->pShadowData, base, totalSize);
        }
    }

    vkUnmapMemory(device, pStagingMemory);

    if (applyUpdate)
    {
        const TextureFormat::Mapping* mapping = vktex->GetTextureFormatMapping();

        VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cbAI.commandPool = cmdPool;
        cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAI.commandBufferCount = 1;

        VkCommandBuffer cmd;
        VkResult vr = vkAllocateCommandBuffers(device, &cbAI, &cmd);

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkDeviceSize bufferOffset = 0;
        for (unsigned itex = 0; itex < vktex->TextureCount; itex++)
        {
            ImageSize sz = vktex->pTextures[itex].Size;
            VkImage image = vktex->pTextures[itex].Image;

            pmgr->TransitionImageLayout(cmd, image,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            for (unsigned level = StartMipLevel; level < StartMipLevel + LevelCount; level++)
            {
                unsigned mipW = Alg::Max<unsigned>(1u, sz.Width >> level);
                unsigned mipH = Alg::Max<unsigned>(1u, sz.Height >> level);

                VkBufferImageCopy region = {};
                region.bufferOffset = bufferOffset;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = level;
                region.imageSubresource.layerCount = 1;
                region.imageExtent = { mipW, mipH, 1 };

                vkCmdCopyBufferToImage(cmd, pStagingBuffer, image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                bufferOffset += (VkDeviceSize)mipW * mipH * mapping->BytesPerPixel;
            }

            pmgr->TransitionImageLayout(cmd, image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
    }

    vkDestroyBuffer(device, pStagingBuffer, nullptr);
    vkFreeMemory(device, pStagingMemory, nullptr);
    pStagingBuffer = VK_NULL_HANDLE;
    pStagingMemory = VK_NULL_HANDLE;
    pTexture->pMap = 0;
    pTexture = 0;
    StartMipLevel = 0;
    LevelCount = 0;
}


// *** TextureManager

TextureManager::TextureManager(VkDevice device, VkPhysicalDevice physDevice,
                               VkCommandPool cmdPool, VkQueue queue,
                               ThreadId renderThreadId,
                               ThreadCommandQueue* commandQueue,
                               TextureCache* texCache)
: Render::TextureManager(renderThreadId, commandQueue, texCache),
  pDevice(device), pPhysicalDevice(physDevice),
  pCommandPool(cmdPool), pQueue(queue),
  DummyImage(VK_NULL_HANDLE), DummyImageView(VK_NULL_HANDLE),
  DummyImageMemory(VK_NULL_HANDLE), DummySampler(VK_NULL_HANDLE),
  KillSlot(0),
  DeferKills(false)
{
    memset(Samplers, 0, sizeof(Samplers));
    memset(CurrentTextures, 0, sizeof(CurrentTextures));
    memset(CurrentSamplers, 0, sizeof(CurrentSamplers));
    initTextureFormats();
    createDummyTexture();
}

TextureManager::~TextureManager()
{
    Reset();
}

void TextureManager::Reset()
{
    DeferKills = false;
    destroyDummyTexture();

    for (unsigned i = 0; i < SamplerTypeCount; i++)
    {
        if (Samplers[i] != VK_NULL_HANDLE)
        {
            vkDestroySampler(pDevice, Samplers[i], nullptr);
            Samplers[i] = VK_NULL_HANDLE;
        }
    }
    // Drain all slots at shutdown
    for (unsigned s = 0; s < SF_VK_MAX_FRAMES_IN_FLIGHT; ++s)
    {
        KillSlot = s;
        processTextureKillList();
    }
}

void TextureManager::initTextureFormats()
{
    for (const TextureFormat::Mapping* m = TextureFormatMapping; m->Format != Image_None; m++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pPhysicalDevice, m->VkFmt, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {
            TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(m);
            TextureFormats.PushBack(tf);
        }
    }
}

void TextureManager::DrainDeferredKills()
{
    bool prev = DeferKills;
    DeferKills = false;
    processTextureKillList();
    DeferKills = prev;
}

void TextureManager::DrainAllDeferredKills()
{
    bool prev = DeferKills;
    DeferKills = false;
    for (unsigned s = 0; s < SF_VK_MAX_FRAMES_IN_FLIGHT; ++s)
    {
        KillSlot = s;
        processTextureKillList();
    }
    DeferKills = prev;
}

void TextureManager::processTextureKillList()
{
    // During rendering, Vulkan resources may still be referenced by in-flight
    // command buffers.  Defer destruction until the next BeginScene, after the
    // application has waited on the GPU fence for this slot.
    if (DeferKills)
        return;

    for (unsigned i = 0; i < ViewKills[KillSlot].GetSize(); i++)
        vkDestroyImageView(pDevice, ViewKills[KillSlot][i], nullptr);
    ViewKills[KillSlot].Clear();

    for (unsigned i = 0; i < ImageKills[KillSlot].GetSize(); i++)
        vkDestroyImage(pDevice, ImageKills[KillSlot][i], nullptr);
    ImageKills[KillSlot].Clear();

    for (unsigned i = 0; i < MemoryKills[KillSlot].GetSize(); i++)
        vkFreeMemory(pDevice, MemoryKills[KillSlot][i], nullptr);
    MemoryKills[KillSlot].Clear();
}

void TextureManager::processInitTextures()
{
    // Initialize pending textures that were created on a non-render thread.
    Render::TextureManager::processInitTextures();
}

void TextureManager::BeginScene()
{
    memset(CurrentTextures, 0, sizeof(CurrentTextures));
    memset(CurrentSamplers, 0, sizeof(CurrentSamplers));
}

VkSampler TextureManager::GetSampler(const ImageFillMode& fm)
{
    unsigned sampleIdx = fm.GetSampleMode();
    unsigned wrapIdx   = fm.GetWrapMode();
    unsigned idx = sampleIdx * Wrap_Count + wrapIdx;
    if (idx >= SamplerTypeCount) idx = 0;

    if (Samplers[idx] != VK_NULL_HANDLE)
        return Samplers[idx];

    // Create sampler on demand
    VkSamplerCreateInfo ci = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    ci.magFilter = (sampleIdx == Sample_Point) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    ci.minFilter = ci.magFilter;
    ci.mipmapMode = (sampleIdx == Sample_Point) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSamplerAddressMode addrMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (wrapIdx == Wrap_Repeat) addrMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    ci.addressModeU = addrMode;
    ci.addressModeV = addrMode;
    ci.addressModeW = addrMode;
    ci.maxLod = VK_LOD_CLAMP_NONE;

    vkCreateSampler(pDevice, &ci, nullptr, &Samplers[idx]);
    return Samplers[idx];
}

void TextureManager::SetSamplerState(unsigned stage, unsigned viewCount,
                                     VkImageView* views, VkSampler sampler)
{
    SF_UNUSED(viewCount);
    if (stage < MaximumStages)
    {
        CurrentTextures[stage] = views[0];
        CurrentSamplers[stage] = sampler;
    }
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                                const ImageSize& size, unsigned use,
                                                ImageBase* pimage, Render::MemoryManager* manager)
{
    SF_UNUSED(manager);
    const Render::TextureFormat* tf = getTextureFormat(format);
    if (!tf)
        return 0;

    Texture* ptex = SF_HEAP_AUTO_NEW(this)
        Texture(pLocks, (const TextureFormat*)tf, mipLevels, size, use, pimage);
    return postCreateTexture(ptex, use);
}

Render::Texture* TextureManager::CreateTexture(VkImage image, VkImageView view,
                                                ImageSize imgSize, ImageBase* pimage)
{
    Texture* ptex = SF_HEAP_AUTO_NEW(this)
        Texture(pLocks, image, view, imgSize, pimage);
    return postCreateTexture(ptex, 0);
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    unsigned use = ImageUse_Update;
    if (!ImageData::IsFormatCompressed(format))
        use |= ImageUse_PartialUpdate | ImageUse_GenMipmaps;
    const Render::TextureFormat* tf = getTextureFormat(format);
    if (tf)
        use |= ImageUse_InitOnly | ImageUse_MapRenderThread;
    return use;
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size,
                                                                        MemoryManager* manager)
{
    SF_UNUSED(manager);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, size);
    if (pdss && !pdss->Initialize())
    {
        pdss->Release();
        return 0;
    }
    return pdss;
}

bool TextureManager::createDummyTexture()
{
    VkImageCreateInfo imageCI = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCI.extent = { 1, 1, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(pDevice, &imageCI, nullptr, &DummyImage) != VK_SUCCESS)
        return false;

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(pDevice, DummyImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX)
        return false;
    if (vkAllocateMemory(pDevice, &allocInfo, nullptr, &DummyImageMemory) != VK_SUCCESS)
        return false;
    vkBindImageMemory(pDevice, DummyImage, DummyImageMemory, 0);

    VkImageViewCreateInfo viewCI = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewCI.image = DummyImage;
    viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCI.subresourceRange.levelCount = 1;
    viewCI.subresourceRange.layerCount = 1;
    if (vkCreateImageView(pDevice, &viewCI, nullptr, &DummyImageView) != VK_SUCCESS)
        return false;

    VkSamplerCreateInfo sampCI = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampCI.magFilter = VK_FILTER_NEAREST;
    sampCI.minFilter = VK_FILTER_NEAREST;
    sampCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (vkCreateSampler(pDevice, &sampCI, nullptr, &DummySampler) != VK_SUCCESS)
        return false;

    // Upload transparent black pixel and transition to shader-read-only layout
    // (avoids uninitialized VRAM showing as garbage color when dummy texture is sampled)
    const uint8_t pixelData[4] = { 0, 0, 0, 0 };

    VkBufferCreateInfo stagingBufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    stagingBufCI.size = 4;
    stagingBufCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stagingBuf = VK_NULL_HANDLE;
    vkCreateBuffer(pDevice, &stagingBufCI, nullptr, &stagingBuf);

    VkMemoryRequirements stagingMemReqs;
    vkGetBufferMemoryRequirements(pDevice, stagingBuf, &stagingMemReqs);
    VkMemoryAllocateInfo stagingAllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    stagingAllocInfo.allocationSize = stagingMemReqs.size;
    stagingAllocInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, stagingMemReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory stagingMem = VK_NULL_HANDLE;
    vkAllocateMemory(pDevice, &stagingAllocInfo, nullptr, &stagingMem);
    vkBindBufferMemory(pDevice, stagingBuf, stagingMem, 0);

    void* mapped = nullptr;
    vkMapMemory(pDevice, stagingMem, 0, 4, 0, &mapped);
    memcpy(mapped, pixelData, 4);
    vkUnmapMemory(pDevice, stagingMem);

    VkCommandBufferAllocateInfo cbAI = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cbAI.commandPool = pCommandPool;
    cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAI.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(pDevice, &cbAI, &cmd);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    TransitionImageLayout(cmd, DummyImage,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = { 1, 1, 1 };
    vkCmdCopyBufferToImage(cmd, stagingBuf, DummyImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    TransitionImageLayout(cmd, DummyImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(pQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(pQueue);
    vkFreeCommandBuffers(pDevice, pCommandPool, 1, &cmd);

    vkDestroyBuffer(pDevice, stagingBuf, nullptr);
    vkFreeMemory(pDevice, stagingMem, nullptr);

    return true;
}

void TextureManager::destroyDummyTexture()
{
    if (DummySampler != VK_NULL_HANDLE)
    { vkDestroySampler(pDevice, DummySampler, nullptr); DummySampler = VK_NULL_HANDLE; }
    if (DummyImageView != VK_NULL_HANDLE)
    { vkDestroyImageView(pDevice, DummyImageView, nullptr); DummyImageView = VK_NULL_HANDLE; }
    if (DummyImage != VK_NULL_HANDLE)
    { vkDestroyImage(pDevice, DummyImage, nullptr); DummyImage = VK_NULL_HANDLE; }
    if (DummyImageMemory != VK_NULL_HANDLE)
    { vkFreeMemory(pDevice, DummyImageMemory, nullptr); DummyImageMemory = VK_NULL_HANDLE; }
}

void TextureManager::TransitionImageLayout(VkCommandBuffer cmd, VkImage image,
                                           VkImageLayout oldLayout, VkImageLayout newLayout,
                                           VkImageAspectFlags aspect)
{
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

}}} // Scaleform::Render::Vulkan
