/**************************************************************************

    Filename    :   D3D12_Texture.h
    Content     :   D3D12 texture manager, texture and depth-stencil surface.
                    Textures are committed resources on the DEFAULT heap with
                    a shader-resource descriptor allocated from an internal
                    CBV_SRV_UAV descriptor heap. Render targets also receive a
                    render-target descriptor from an internal RTV heap.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_TEXTURE_H
#define INC_SF_D3D12_TEXTURE_H
#pragma once

#include "Render/D3D12/D3D12_Config.h"
#include "Render/D3D12/D3D12_Sync.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Render/Render_HAL.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace D3D12 {

class HAL;

// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        DXGI_FORMAT              D3DFormat;
        UByte                    BytesPerPixel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    TextureFormat(const Mapping* mapping) : pMapping(mapping) { }

    const Mapping*  pMapping;

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }

    // D3D12 Specific.
    DXGI_FORMAT             GetD3DFormat() const   { return pMapping->D3DFormat; }
};

class MappedTexture;
class TextureManager;

// D3D12 Texture class implementation; it may actually include several HW
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    static const UByte      MaxTextureCount = 4;

    struct HWTextureDesc
    {
        ImageSize                   Size;
        Ptr<ID3D12Resource>         pTexture;
        Ptr<ID3D12Resource>         pStagingTexture;
        D3D12_CPU_DESCRIPTOR_HANDLE SRV;
        D3D12_CPU_DESCRIPTOR_HANDLE RTV;
        D3D12_GPU_DESCRIPTOR_HANDLE GPU;
        unsigned                    SRVIndex;
        unsigned                    RTVIndex;
    };

    // TextureDesc array is allocated if more than one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;
    // Set for image-backed textures created before the frame command list is
    // open; the upload is performed lazily on first ApplyTexture.
    bool                    NeedsUpload;
    bool                    UploadTried;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, ID3D12Resource* ptexture, D3D12_RESOURCE_STATES usage,
            ImageSize imgSize, ImageBase* pimage);
    ~Texture();

    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return pTextures[plane].Size; }
    TextureManager*         GetManager() const                      { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const                         { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(ID3D12Resource* ptexture);
    void                    ReleaseHWTextures(bool staging = true);

    // Applies a texture to device starting at pstageIndex, advances index
    // TBD: Texture matrix may need to be adjusted if image scaling is done.
    void                    ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    // Uploads simple 8/32-bit image-backed textures directly from ImageData,
    // bypassing the generic convert/rescale path (which is incomplete here).
    // Prepare fills the staging buffer while pImage is still alive; Commit
    // records the GPU copy when the frame command list is recording.
    bool                    PrepareImageUpload();
    void                    CommitImageUpload();

    // *** Interface implementation
    virtual Image*                  GetImage() const                { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat             GetFormat() const               { return GetImageFormat(); }
    const TextureFormat*            GetTextureFormat() const        { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual void            GetUVGenMatrix(Matrix2F* mat) const;

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);
    virtual bool            UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL* = 0);
    virtual bool            UpdateStagingData(Render::RenderTargetData*);

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat,
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
};

// D3D12 DepthStencilSurface implementation.
class DepthStencilSurface : public Render::DepthStencilSurface
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();

    Ptr<ID3D12Resource>             pDepthStencilSurface;
    D3D12_CPU_DESCRIPTOR_HANDLE     DSV;
    unsigned                        DSVIndex;
};

// *** MappedTexture
class MappedTexture : public MappedTextureBase
{
    friend class Texture;

public:
    MappedTexture() : MappedTextureBase() { }

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
    virtual void Unmap(bool applyUpdate = true);
};


// D3D12 Texture Manager.
// This class is responsible for creating textures and keeping track of them
// in the list.
//
class TextureManager : public Render::TextureManager
{
    friend class Texture;
    friend class DepthStencilSurface;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<Ptr<ID3D12Resource>,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DResourceArray;

public:
    ID3D12Device*           pDevice;
    Ptr<ID3D12GraphicsCommandList> pCommandList;
    HAL*                    pOwnerHAL;
    MappedTexture           MappedTexture0;

    // Lists protected by TextureManagerLocks::TextureMutex.
    D3DResourceArray        D3DTextureKillList;

    // Descriptor heaps owned by the texture manager.
    Ptr<ID3D12DescriptorHeap> pSRVHeap;     // CBV_SRV_UAV, GPU visible; texture SRVs.
    Ptr<ID3D12DescriptorHeap> pSamplerHeap; // SAMPLER, GPU visible; sampler states.
    Ptr<ID3D12DescriptorHeap> pRTVHeap;     // RTV, CPU only.
    Ptr<ID3D12DescriptorHeap> pDSVHeap;     // DSV, CPU only.

    // Allocator cursors (prototype: linear allocation, no per-descriptor free).
    UPInt                   SRVHeapCursor;
    UPInt                   SamplerHeapCursor;
    UPInt                   RTVHeapCursor;
    UPInt                   DSVHeapCursor;

    static const unsigned   SamplerTypeCount = (static_cast<unsigned>(Sample_Count) * static_cast<unsigned>(Wrap_Count));
    // Number of SRV/sampler descriptors a draw's descriptor tables are made of.
    // This must match the descriptor ranges declared by HAL::createRootSignature
    // (t0..t7 and s0..s7): a table that is declared 8 wide but only partially
    // written feeds the shader uninitialized descriptors, which is undefined
    // behavior on the GPU (and was what hung the device).
    static const unsigned   TextureStageCount = 8;
    // One group of TextureStageCount identical samplers per fill mode; the whole
    // group is bound as the s0..s7 descriptor table, so no per-draw sampler
    // copies (and no sampler scratch area) are needed.
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerStates[SamplerTypeCount];
    D3D12_GPU_DESCRIPTOR_HANDLE SamplerStateGPUs[SamplerTypeCount];

    // Detects supported D3D12 formats and capabilities.
    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }

    virtual void    processTextureKillList();
    virtual void    processInitTextures();

    // Allocates a descriptor from one of the internal heaps.
    D3D12_CPU_DESCRIPTOR_HANDLE allocSRVDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE* pGPU, unsigned* pIndex = 0);
    D3D12_CPU_DESCRIPTOR_HANDLE allocRTVDescriptor(unsigned* pIndex = 0);
    D3D12_CPU_DESCRIPTOR_HANDLE allocDSVDescriptor(unsigned* pIndex = 0);
    void freeSRVDescriptor(unsigned index);
    void freeRTVDescriptor(unsigned index);
    void freeDSVDescriptor(unsigned index);
    Array<unsigned>         FreeSRVDescriptors;
    Array<unsigned>         FreeRTVDescriptors;
    Array<unsigned>         FreeDSVDescriptors;
    Array<unsigned>         PendingSRVDescriptors;
    Array<unsigned>         PendingRTVDescriptors;
    Array<unsigned>         PendingDSVDescriptors;
    // Upload staging buffers released after the next frame fence.
    Array<Ptr<ID3D12Resource> > PendingStagingRelease;

public:
    TextureManager(ID3D12Device* pdevice,
                   RenderSync* rsync,
                   ThreadId renderThreadId,
                   ThreadCommandQueue* commandQueue,
                   TextureCache* texCache = 0);
    ~TextureManager();

    // Used once texture manager is no longer necessary.
    void    Reset();

    // Called once per frame by HAL::BeginFrame, after the platform has waited
    // for the previous frame's GPU work. Descriptors and staging buffers that
    // were released during the previous frame can only be recycled here: doing
    // it in BeginScene() released upload staging buffers while the (still open)
    // command list referenced them in CopyTextureRegion, which made the debug
    // layer report "resource ... was deleted prior to closing the command list"
    // and hung the GPU.
    virtual void    BeginFrame();
    virtual void    BeginScene();

    // *** TextureManager
    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(ID3D12Resource* pd3dtexture,
                                           D3D12_RESOURCE_STATES usage,
                                           ImageSize imgSize = ImageSize(0), ImageBase* image = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
    bool IsMultiThreaded() const    { return RenderThreadId != 0; }

    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                           MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(ID3D12Resource* psurface,
                                                           D3D12_RESOURCE_STATES usage);

    virtual bool            IsDrawableImageFormat(ImageFormat format) const { return (format == Image_B8G8R8A8) || (format == Image_R8G8B8A8); }

    // Sets the command list used for GPU uploads/copies.
    void            SetCommandList(ID3D12GraphicsCommandList* commandList);
    ID3D12GraphicsCommandList* GetCommandList() const { return pCommandList; }
    // Sets the HAL that owns this manager (used to apply stage textures).
    void            SetOwnerHAL(HAL* phal) { pOwnerHAL = phal; }
    HAL*            GetOwnerHAL() const    { return pOwnerHAL; }
    // Returns the sampler descriptor (CPU handle) of the s0..s7 group for the
    // given fill mode.
    D3D12_CPU_DESCRIPTOR_HANDLE GetSamplerDescriptor(const ImageFillMode& fm) const;
    // Returns the GPU handle of the same group, ready to be bound as a descriptor
    // table. The group holds TextureStageCount copies of the sampler.
    D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerDescriptorGPU(const ImageFillMode& fm) const;
    // Returns the shader-resource descriptor heap (GPU visible).
    ID3D12DescriptorHeap* GetSRVHeap() const { return pSRVHeap; }
    // Returns the sampler descriptor heap (GPU visible).
    ID3D12DescriptorHeap* GetSamplerHeap() const { return pSamplerHeap; }

    // Scratch descriptor area used by the HAL to stage the per-draw SRV table.
    // It is carved out of the END of the SRV heap, so the persistent texture SRVs
    // (allocated from the bottom) must stay below
    // (D3D12_SRVHeapDescriptorCount - SRVScratchDescriptorCount).
    static const unsigned   SRVScratchDescriptorCount   = 4096;
    // Returns the GPU handle of the scratch region start in the SRV heap.
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVScratchBase() const;

    // Swap-chain resize only invalidates the back buffer, not the texture
    // heaps; keep glyph/font atlases alive across PrepareForReset.
    virtual void    OnHALEvent(HALNotifyType type) { if (type == HAL_Shutdown) Reset(); }

    ID3D12Device*         GetDevice() const           { return pDevice; }
};

}}} // namespace Scaleform::Render::D3D12

#endif // INC_SF_D3D12_TEXTURE_H
