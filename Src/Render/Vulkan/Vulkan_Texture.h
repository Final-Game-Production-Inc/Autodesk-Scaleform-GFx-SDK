/**************************************************************************

Filename    :   Vulkan_Texture.h
Content     :   Vulkan texture and TextureManager implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_VULKAN_TEXTURE_H
#define INC_SF_VULKAN_TEXTURE_H
#pragma once

#include "Render/Vulkan/Vulkan_Config.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace Vulkan {

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        VkFormat                 VkFmt;
        UByte                    BytesPerPixel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    TextureFormat(const Mapping* mapping) : pMapping(mapping) {}

    const Mapping* pMapping;

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }

    VkFormat GetVkFormat() const { return pMapping->VkFmt; }
};

class MappedTexture;
class TextureManager;

class Texture : public Render::Texture
{
public:
    static const UByte MaxTextureCount = 4;

    struct HWTextureDesc
    {
        ImageSize       Size;
        VkImage         Image;
        VkImageView     View;
        VkDeviceMemory  Memory;
    };

    HWTextureDesc*  pTextures;
    HWTextureDesc   Texture0;

    UByte*          pShadowData;
    UPInt           ShadowSize;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, VkImage image, VkImageView view,
            ImageSize imgSize, ImageBase* pimage);
    ~Texture();

    virtual ImageSize       GetTextureSize(unsigned plane = 0) const { return pTextures[plane].Size; }
    TextureManager*         GetManager() const     { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const        { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(VkImage image, VkImageView view);
    void                    ReleaseHWTextures(bool staging = true);

    void                    ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    virtual Image*                  GetImage() const        { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat             GetFormat() const       { return GetImageFormat(); }
    const TextureFormat*            GetTextureFormat() const { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual void            GetUVGenMatrix(Matrix2F* mat) const;
    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);

protected:
    virtual void            computeUpdateConvertRescaleFlags(bool rescale, bool swMipGen, ImageFormat inputFormat,
                                                             ImageRescaleType& rescaleType, ImageFormat& rescaleBuffFromat, bool& convert);
};


class DepthStencilSurface : public Render::DepthStencilSurface
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size);
    ~DepthStencilSurface();

    bool Initialize();

    VkImage         pDepthStencilImage;
    VkImageView     pDepthStencilView;
    VkDeviceMemory  pDepthStencilMemory;
    VkFormat        DepthStencilFormat;
};


class MappedTexture : public MappedTextureBase
{
    friend class Texture;
public:
    MappedTexture() : MappedTextureBase(), pStagingBuffer(VK_NULL_HANDLE), pStagingMemory(VK_NULL_HANDLE) {}

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
    virtual void Unmap(bool applyUpdate = true);

private:
    VkBuffer        pStagingBuffer;
    VkDeviceMemory  pStagingMemory;
};


class TextureManager : public Render::TextureManager
{
    friend class HAL;
    friend class Texture;
    friend class DepthStencilSurface;
    friend class ShaderInterface;

    typedef ArrayConstPolicy<8, 8, false> KillListArrayPolicy;
    typedef ArrayLH<VkImage, StatRender_TextureManager_Mem, KillListArrayPolicy>        ImageKillList;
    typedef ArrayLH<VkImageView, StatRender_TextureManager_Mem, KillListArrayPolicy>    ViewKillList;
    typedef ArrayLH<VkDeviceMemory, StatRender_TextureManager_Mem, KillListArrayPolicy> MemoryKillList;
    typedef ArrayLH<VkSampler, StatRender_TextureManager_Mem, KillListArrayPolicy>      SamplerKillList;

    VkDevice                pDevice;
    VkPhysicalDevice        pPhysicalDevice;
    VkCommandPool           pCommandPool;
    VkQueue                 pQueue;
    MappedTexture           MappedTexture0;

    VkImage                 DummyImage;
    VkImageView             DummyImageView;
    VkDeviceMemory          DummyImageMemory;
    VkSampler               DummySampler;
    bool                    createDummyTexture();
    void                    destroyDummyTexture();

    // Per-slot deferred kill lists, indexed by frame slot.  Each slot is only
    // drained when the corresponding in-flight fence has been waited on.
    ImageKillList           ImageKills[SF_VK_MAX_FRAMES_IN_FLIGHT];
    ViewKillList            ViewKills[SF_VK_MAX_FRAMES_IN_FLIGHT];
    MemoryKillList          MemoryKills[SF_VK_MAX_FRAMES_IN_FLIGHT];
    unsigned                KillSlot;

    static const unsigned   SamplerTypeCount = (Sample_Count * Wrap_Count);
    VkSampler               Samplers[SamplerTypeCount];

    static const int        MaximumStages = SF_VK_MAX_TEXTURE_STAGES;
    VkImageView             CurrentTextures[MaximumStages];
    VkSampler               CurrentSamplers[MaximumStages];

    void                    initTextureFormats();
    virtual MappedTextureBase&  getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*  createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }

    virtual void    processTextureKillList();
    virtual void    processInitTextures();

    // When true, processTextureKillList() is a no-op: VkImageView/VkImage/
    // VkDeviceMemory kills accumulate until the next BeginScene, after the
    // GPU fence ensures no command buffer still references them.
    bool            DeferKills;

public:
    void            SetKillSlot(unsigned frameIndex) { KillSlot = frameIndex % SF_VK_MAX_FRAMES_IN_FLIGHT; }
    // Drain accumulated texture kills for the current slot (call after GPU fence).
    void            DrainDeferredKills();
    // Drain ALL slots (call at shutdown after vkDeviceWaitIdle).
    void            DrainAllDeferredKills();
    TextureManager(VkDevice device, VkPhysicalDevice physDevice,
                   VkCommandPool cmdPool, VkQueue queue,
                   ThreadId renderThreadId,
                   ThreadCommandQueue* commandQueue = 0,
                   TextureCache* texCache = 0);
    ~TextureManager();

    void    Reset();
    void    SetSamplerState(unsigned stage, unsigned viewCount, VkImageView* views, VkSampler sampler);

    virtual void    BeginScene();

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size, unsigned use,
                                           ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(VkImage image, VkImageView view,
                                           ImageSize imgSize = ImageSize(0), ImageBase* pimage = 0);

    virtual unsigned GetTextureUseCaps(ImageFormat format);
    bool IsMultiThreaded() const { return RenderThreadId != 0; }

    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                                    MemoryManager* manager = 0);
    virtual bool IsDrawableImageFormat(ImageFormat format) const
    {
        return (format == Image_B8G8R8A8) || (format == Image_R8G8B8A8);
    }

    VkDevice            GetDevice() const        { return pDevice; }
    VkPhysicalDevice    GetPhysicalDevice() const { return pPhysicalDevice; }
    VkCommandPool       GetCommandPool() const   { return pCommandPool; }
    VkQueue             GetQueue() const         { return pQueue; }
    VkSampler           GetSampler(const ImageFillMode& fm);
    VkImageView         GetDummyImageView() const { return DummyImageView; }
    VkSampler           GetDummySampler() const   { return DummySampler; }

    void TransitionImageLayout(VkCommandBuffer cmd, VkImage image,
                               VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT);
};


}}} // Scaleform::Render::Vulkan

#endif // INC_SF_VULKAN_TEXTURE_H
