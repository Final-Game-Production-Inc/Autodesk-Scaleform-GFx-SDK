/**************************************************************************

    Filename    :   D3D12_Texture.cpp
    Content     :   D3D12 texture manager, texture and depth-stencil surface.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D12/D3D12_Texture.h"
#include "Render/D3D12/D3D12_HAL.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_DebugEnv.h"
#include <stdlib.h>
#include <stdio.h>

//#define SF_RENDER_LOG_TEXTURES

namespace Scaleform { namespace Render { namespace D3D12 {

// Descriptor heap sizes used by the TextureManager (prototype allocation).
enum
{
    D3D12_SRVHeapDescriptorCount     = 8192,
    D3D12_SamplerHeapDescriptorCount = 1024,
    D3D12_RTVHeapDescriptorCount     = 4096,
    D3D12_DSVHeapDescriptorCount     = 4096
};

// Creates a committed 2D texture resource on the DEFAULT heap.
static bool CreateCommittedTexture2D(ID3D12Device* pdevice,
                                     unsigned width, unsigned height,
                                     DXGI_FORMAT format, unsigned mipLevels,
                                     D3D12_RESOURCE_FLAGS flags,
                                     D3D12_RESOURCE_STATES initialState,
                                     ID3D12Resource** ppResource)
{
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Alignment          = 0;
    resDesc.Width              = width;
    resDesc.Height             = height;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = (UINT16)mipLevels;
    resDesc.Format             = format;
    resDesc.SampleDesc.Count   = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags              = flags;

    return SUCCEEDED( pdevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                       &resDesc, initialState, NULL,
                                                       IID_PPV_ARGS(ppResource)) );
}

// Creates a committed buffer resource on the specified heap type.
static bool CreateCommittedBuffer(ID3D12Device* pdevice,
                                  D3D12_HEAP_TYPE heapType,
                                  UPInt size, D3D12_RESOURCE_STATES initialState,
                                  ID3D12Resource** ppResource)
{
    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = heapType;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment          = 0;
    resDesc.Width              = size;
    resDesc.Height             = 1;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = 1;
    resDesc.Format             = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count   = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    return SUCCEEDED( pdevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                       &resDesc, initialState, NULL,
                                                       IID_PPV_ARGS(ppResource)) );
}

// Records a resource barrier on the given command list.
static void RecordResourceBarrier(ID3D12GraphicsCommandList* pCommandList,
                                  ID3D12Resource* pResource,
                                  D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    if (!pCommandList || !pResource || before == after)
        return;
    D3D12_RESOURCE_BARRIER barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter  = after;
    pCommandList->ResourceBarrier(1, &barrier);
}

// Row pitch for a D3D12 placed footprint must be a multiple of 256 bytes.
static UINT ComputeAlignedRowPitch(unsigned width, unsigned bytesPerPixel)
{
    return (UINT)Alg::Align((UPInt)width * bytesPerPixel, 256u);
}

// Creates a staging buffer (UPLOAD or READBACK) large enough to hold one
// mip level of a texture.
static bool CreateStagingBufferForTexture(ID3D12Device* pdevice,
                                          const Texture::HWTextureDesc& tdesc,
                                          D3D12_HEAP_TYPE heapType,
                                          unsigned mipLevel,
                                          ID3D12Resource** ppResource)
{
    ImageSize levelSize = tdesc.Size;
    for (unsigned i = 0; i < mipLevel; i++)
    {
        levelSize.Width  = Alg::Max<UInt32>(1, levelSize.Width >> 1);
        levelSize.Height = Alg::Max<UInt32>(1, levelSize.Height >> 1);
    }
    // Allocate generously (4 bytes per pixel) to cover all 32-bit formats,
    // with a 256-byte aligned row pitch as required for placed footprints.
    UPInt rowPitch = Alg::Align((UPInt)levelSize.Width * 4, 256u);
    UPInt size = rowPitch * levelSize.Height;
    if (size == 0)
        size = rowPitch;
    return CreateCommittedBuffer(pdevice, heapType, size,
                                 heapType == D3D12_HEAP_TYPE_READBACK ?
                                     D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ,
                                 ppResource);
}

// ***** CopyScanline conversion functions

void SF_STDCALL D3D12_CopyScanline8_Extend_A8_A8L8(UByte* pd, const UByte* ps, UPInt size,
                                                   Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {
        pd[0] = 255;
        pd[1] = ps[0];
    }
}

void SF_STDCALL D3D12_CopyScanline16_Retract_A8L8_A8(UByte* pd, const UByte* ps, UPInt size,
                                                     Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {
        pd[0] = ps[1];
    }
}

// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

TextureFormat::Mapping D3D12TextureFormatMapping[] =
{
    // Warning: Different versions of the same ImageFormat must go right after each-other,
    // as initTextureFormats relies on that fact to skip them during detection.
    { Image_R8G8B8A8,   DXGI_FORMAT_R8G8B8A8_UNORM,  4,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,   DXGI_FORMAT_B8G8R8A8_UNORM,  4,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_R8G8B8,     DXGI_FORMAT_R8G8B8A8_UNORM,  4,  &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,     DXGI_FORMAT_B8G8R8A8_UNORM,  4,  &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_A8,         DXGI_FORMAT_R8_UNORM,        1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    // Compressed formats.
    { Image_DXT1,       DXGI_FORMAT_BC1_UNORM,       0,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT3,       DXGI_FORMAT_BC2_UNORM,       0,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT5,       DXGI_FORMAT_BC3_UNORM,       0,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_BC7,        DXGI_FORMAT_BC7_UNORM,       0,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},

    // Video formats.
    { Image_Y8_U2_V2,   DXGI_FORMAT_R8_UNORM,        1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,DXGI_FORMAT_R8_UNORM,        1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    { Image_None,       DXGI_FORMAT_UNKNOWN,         0,  0,                                      0 }
};


// ***** Texture

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
                 const ImageSize& size, unsigned use, ImageBase* pimage) :
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat),
    pTextures(0), NeedsUpload(false), UploadTried(false)
{
    // YUV video textures consist of several planes (Y, U, V).
    TextureCount = (UByte)pformat->GetPlaneCount();

    memset(&Texture0, 0, sizeof(Texture0));
    Texture0.SRV.ptr = 0;
    Texture0.RTV.ptr = 0;
    Texture0.GPU.ptr = 0;
    Texture0.SRVIndex = Texture0.RTVIndex = ~0u;
    // If we are storing multiple textures, allocate storage for all of them.
    if (TextureCount > 1)
    {
        pTextures = (HWTextureDesc*)SF_HEAP_AUTO_ALLOC(this, sizeof(HWTextureDesc) * TextureCount);
        if (!pTextures)
            State = State_InitFailed;
        memset(pTextures, 0, sizeof(HWTextureDesc) * TextureCount);
        for (unsigned i = 0; i < TextureCount; i++)
        {
            pTextures[i].SRV.ptr = 0;
            pTextures[i].RTV.ptr = 0;
            pTextures[i].GPU.ptr = 0;
            pTextures[i].SRVIndex = pTextures[i].RTVIndex = ~0u;
        }
    }
    else
    {
        pTextures = &Texture0;
    }
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, ID3D12Resource* ptexture, D3D12_RESOURCE_STATES usage,
                 ImageSize imgSize, ImageBase* pimage) :
    Render::Texture(pmanagerLocks, imgSize, 1, 0, pimage, 0),
    pTextures(0), NeedsUpload(false), UploadTried(false)
{
    memset(&Texture0, 0, sizeof(Texture0));
    Texture0.SRV.ptr = 0;
    Texture0.RTV.ptr = 0;
    Texture0.GPU.ptr = 0;
    Texture0.SRVIndex = Texture0.RTVIndex = ~0u;
    pTextures = &Texture0;
    pTextures[0].pTexture = ptexture;
    pTextures[0].Size = imgSize;
    SF_UNUSED(usage);
}

Texture::~Texture()
{
    // Unlink from TextureManager::Textures and release the D3D resources, like
    // D3D1x/GL do. Without this every destroyed Texture stayed linked in the
    // manager intrusive list, so the next Textures.PushBack() walked freed nodes
    // (crash in List<Texture>::PushBack via postCreateTexture) and D3D resources leaked.
    if (pManagerLocks && (State == State_Valid || State == State_Lost))
    {
        Mutex::Locker lock(&pManagerLocks->TextureMutex);
        RemoveNode();
        pNext = pPrev = 0;

        // D3D12 has no runtime-side reference tracking: releasing the resource
        // here destroys it immediately, even though the frame's command list may
        // still reference it. The render-buffer manager evicts temporary targets
        // *during* a frame (reserveSpace -> evictUntilAvailable), which used to
        // produce "resource deleted prior to closing the command list" (id=921)
        // and eventually a GPU hang on the 9-1-B feedback banner. Park the
        // resources in the kill list; BeginFrame drains it after the frame fence.
        if (pManagerLocks->pManager && pTextures)
        {
            TextureManager* pmgr = (TextureManager*)pManagerLocks->pManager;
            for (unsigned i = 0; i < TextureCount; i++)
            {
                if (pTextures[i].pTexture)
                {
                    if (IsDebugEnvSet("GFX_TEXREL"))
                    {
                        static int sLog = 0;
                        if (sLog++ < 4000)
                            fprintf(stderr, "[TEXDTOR] %p %ux%u -> killlist\n", this,
                                    pTextures[i].Size.Width, pTextures[i].Size.Height);
                    }
                    pmgr->D3DTextureKillList.PushBack(pTextures[i].pTexture);
                    pTextures[i].pTexture.Clear();
                }
                if (pTextures[i].pStagingTexture)
                {
                    pmgr->D3DTextureKillList.PushBack(pTextures[i].pStagingTexture);
                    pTextures[i].pStagingTexture.Clear();
                }

                // Return the descriptor slots as well. This destructor is the
                // normal path for evicted temporary render targets and it used
                // to leak the SRV and RTV index on every destroyed texture.
                // After a few thousand targets (the 9-1-B banner animates its
                // scale, so new temp sizes are created continuously) the RTV
                // heap cursor ran past D3D12_RTVHeapDescriptorCount and
                // OMSetRenderTargets was called with a descriptor outside the
                // heap - the NVIDIA driver then dereferenced garbage and the
                // process died inside nvwgf2umx (AV at 0x28).
                if (pTextures[i].SRVIndex != ~0u)
                {
                    pmgr->freeSRVDescriptor(pTextures[i].SRVIndex);
                    pTextures[i].SRVIndex = ~0u;
                }
                if (pTextures[i].RTVIndex != ~0u)
                {
                    pmgr->freeRTVDescriptor(pTextures[i].RTVIndex);
                    pTextures[i].RTVIndex = ~0u;
                }
                pTextures[i].SRV.ptr = 0;
                pTextures[i].RTV.ptr = 0;
                pTextures[i].GPU.ptr = 0;
            }
        }
    }

    if (pTextures && pTextures != &Texture0)
        SF_HEAP_FREE(this, pTextures);
    pTextures = 0;
}

bool Texture::Initialize()
{
    SF_AMP_SCOPE_TIMER(GetManager()->RenderThreadId == GetCurrentThreadId() ? AmpServer::GetInstance().GetDisplayStats() : NULL, __FUNCTION__, Amp_Profile_Level_Medium);

    if (TextureFlags & TF_UserAlloc)
    {
        return Initialize(pTextures[0].pTexture);
    }

    ImageFormat     format = GetImageFormat();
    TextureManager* pmanager = GetManager();
    unsigned        itex;

    // Determine sizes of textures.
    if (State != State_Lost)
    {
        for (itex = 0; itex < TextureCount; itex++)
        {
            HWTextureDesc& tdesc = pTextures[itex];
            tdesc.Size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
        }
    }

    // Determine how many mipLevels we should have. D3D12 does not provide
    // hardware mip-map generation, so software mip-map generation is used
    // when requested.
    unsigned allocMipLevels = MipLevels;
    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        TextureFlags |= TF_SWMipGen;
        allocMipLevels = 31;
        for (itex = 0; itex < TextureCount; itex++)
            allocMipLevels = Alg::Min(allocMipLevels,
                ImageSize_MipLevelCount(pTextures[itex].Size));
        MipLevels = (UByte)allocMipLevels;
    }

    bool renderTarget = (Use & ImageUse_RenderTarget) != 0;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    if (renderTarget)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // Create textures.
    bool createSuccess = true;
    for (itex = 0; itex < TextureCount && createSuccess; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        D3D12_RESOURCE_STATES initialState = renderTarget ?
            D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        if (!CreateCommittedTexture2D(pmanager->pDevice, tdesc.Size.Width, tdesc.Size.Height,
                                      GetTextureFormat()->GetD3DFormat(), allocMipLevels,
                                      flags, initialState, &tdesc.pTexture.GetRawRef()))
        {
            SF_DEBUG_ERROR(1, "ID3D12Device::CreateCommittedResource failed (texture)");
            createSuccess = false;
            break;
        }
        if (tdesc.pTexture)
        {
            wchar_t wname[96];
            swprintf_s(wname, L"GFx_Tex_%ux%u_%s", tdesc.Size.Width, tdesc.Size.Height,
                       renderTarget ? L"RT" : L"Srv");
            tdesc.pTexture->SetName(wname);
        }

        // Create a shader resource view descriptor.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof(srvDesc));
        srvDesc.Format                  = GetTextureFormat()->GetD3DFormat();
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels     = allocMipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        tdesc.SRV = pmanager->allocSRVDescriptor(&tdesc.GPU, &tdesc.SRVIndex);
        pmanager->pDevice->CreateShaderResourceView(tdesc.pTexture, &srvDesc, tdesc.SRV);

        // Create a render target view descriptor if requested.
        if (renderTarget)
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
            memset(&rtvDesc, 0, sizeof(rtvDesc));
            rtvDesc.Format             = GetTextureFormat()->GetD3DFormat();
            rtvDesc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = 0;
            tdesc.RTV = pmanager->allocRTVDescriptor(&tdesc.RTVIndex);
            pmanager->pDevice->CreateRenderTargetView(tdesc.pTexture, &rtvDesc, tdesc.RTV);
        }
    }

    if (!createSuccess)
    {
        ReleaseHWTextures();
        if (State != State_Lost)
            State = State_InitFailed;
        return false;
    }

    // Decode image content into the CPU staging buffer now; pImage is cleared
    // by the base class when ImageUse_NoDataLoss is set. The GPU copy is
    // committed on first ApplyTexture, when the frame command list is open.
    if (pImage)
    {
        UploadTried = true;
        NeedsUpload = PrepareImageUpload();
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

bool Texture::Initialize(ID3D12Resource* ptexture)
{
    if ( !ptexture )
        return false;

    // If this is called explicitly (ie. not after the constructor), and the texture is being replaced,
    // store the new texture.
    if ( pTextures[0].pTexture != ptexture )
    {
        ReleaseHWTextures();
        pTextures[0].pTexture = ptexture;
    }

    D3D12_RESOURCE_DESC texDesc = ptexture->GetDesc();
    MipLevels = (UByte)texDesc.MipLevels;
    pFormat = 0;
    TextureManager* pmanager = GetManager();

    // If an image is provided, try to obtain the texture format from the image.
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.
    if ( pFormat == 0 )
    {
        TextureFormat::Mapping* pmapping;
        for (pmapping = D3D12TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->D3DFormat == texDesc.Format )
            {
                pFormat = (TextureFormat*)pmanager->getTextureFormat(pmapping->Format);
                break;
            }
        }
    }

    // Could not determine the format.
    if ( !pFormat)
    {
        SF_DEBUG_ERROR(1, "Texture::Initialize failed - couldn't determine ImageFormat of user supplied texture.");
        State = State_InitFailed;
        return false;
    }

    // Fill out the HW description.
    pTextures[0].Size.SetSize((unsigned)texDesc.Width, (unsigned)texDesc.Height);

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = pTextures[0].Size;

    // Create the shader resource view.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format                  = texDesc.Format;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels     = MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    pTextures[0].SRV = pmanager->allocSRVDescriptor(&pTextures[0].GPU, &pTextures[0].SRVIndex);
    pmanager->pDevice->CreateShaderResourceView(pTextures[0].pTexture, &srvDesc, pTextures[0].SRV);

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat,
                                                ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_UNUSED4(rescale, swMipGen, inputFormat, rescaleType);
    SF_UNUSED(rescaleBuffFromat);
    // D3D12 does not support hardware rescaling or mip-map generation;
    // conversion is handled by the generic image pipeline.
    convert = true;
}

void Texture::GetUVGenMatrix(Matrix2F* mat) const
{
    // UV Scaling rules are as follows:
    //  - If image got scaled, divide by original size.
    //  - If image is padded, divide by new texture size.
    const ImageSize& sz = (TextureFlags & TF_Rescale) ? ImgSize : pTextures[0].Size;
    *mat = Matrix2F::Scaling(1.0f / (float)sz.Width, 1.0f / (float)sz.Height);
}

void Texture::ReleaseHWTextures(bool staging)
{
    Render::Texture::ReleaseHWTextures(staging);

    TextureManager* pmanager = GetManager();
    if (!pmanager)
        return;

    // D3D12 has no runtime-side reference tracking, so *every* texture release
    // has to be deferred until the frame fence: the texture may still be
    // referenced by the command list that is being recorded. Always queue into
    // the kill list (BeginFrame drains it after the fence). The previous code
    // released immediately when called on the render thread, which produced
    // "deleted prior to closing the command list" (id=921) and GPU hangs.
    Mutex::Locker lock(&pManagerLocks->TextureMutex);

    if (IsDebugEnvSet("GFX_TEXREL"))
    {
        static int sLog = 0;
        if (sLog++ < 4000)
            fprintf(stderr, "[TEXREL] %p %ux%u deferred thread=%u\n", this,
                    ImgSize.Width, ImgSize.Height, (unsigned)GetCurrentThreadId());
    }

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        ID3D12Resource* ptexture = pTextures[itex].pTexture;
        ID3D12Resource* pstaging = staging ? pTextures[itex].pStagingTexture : 0;

        if (ptexture)
        {
            pmanager->D3DTextureKillList.PushBack(ptexture);
            if (pstaging)
                pmanager->D3DTextureKillList.PushBack(pstaging);
        }
        pTextures[itex].pTexture = 0;
        if (staging)
            pTextures[itex].pStagingTexture = 0;
        if (pTextures[itex].SRVIndex != ~0u)
        {
            pmanager->freeSRVDescriptor(pTextures[itex].SRVIndex);
            pTextures[itex].SRVIndex = ~0u;
        }
        if (pTextures[itex].RTVIndex != ~0u)
        {
            pmanager->freeRTVDescriptor(pTextures[itex].RTVIndex);
            pTextures[itex].RTVIndex = ~0u;
        }
        pTextures[itex].SRV.ptr = 0;
        pTextures[itex].RTV.ptr = 0;
        pTextures[itex].GPU.ptr = 0;
    }
}

// Decodes an image-backed texture into its CPU staging buffer while the
// source image is still alive (ImageUse_NoDataLoss clears pImage right after
// initialization). The GPU copy is deferred to CommitImageUpload.
bool Texture::PrepareImageUpload()
{
    TextureManager* pmgr = GetManager();
    if (!pImage || !pFormat || !pmgr)
        return false;

    ImageFormat fmt = GetImageFormat();
    // Only formats with a byte-compatible GPU representation (possibly after
    // the standard 24->32-bit expand) are handled by the direct uploader.
    const TextureFormat::Mapping* pmap = GetTextureFormat()->pMapping;
    ImageData data;
    const bool expand24 = (fmt == Image_R8G8B8 || fmt == Image_B8G8R8);
    // Decode into the source format first; for 24-bit images expand each row
    // explicitly with the mapping's scanline copier, which avoids format
    // mismatch inside Image::Decode.
    Ptr<RawImage> pdecode = *RawImage::Create(expand24 ? fmt : pmap->Format, 1,
                                              pTextures[0].Size, 0);
    if (!pdecode || !pdecode->GetImageData(&data))
        return false;
    if (!pImage->Decode(&data, expand24 ? Image::CopyScanlineDefault : pmap->CopyFunc, 0))
        return false;

    unsigned bpp = pmap->BytesPerPixel;
    unsigned planes = Alg::Min<unsigned>(TextureCount, data.GetPlaneCount());
    for (unsigned i = 0; i < planes; i++)
    {
        HWTextureDesc& tdesc = pTextures[i];
        if (!tdesc.pStagingTexture &&
            !CreateStagingBufferForTexture(pmgr->GetDevice(), tdesc,
                                           D3D12_HEAP_TYPE_UPLOAD, 0,
                                           &tdesc.pStagingTexture.GetRawRef()))
            return false;

        ImagePlane splane;
        data.GetPlane(i, &splane);
        if (!splane.pData)
            return false;

        UINT rowPitch = ComputeAlignedRowPitch(tdesc.Size.Width, bpp);
        D3D12_RANGE range = { 0, (SIZE_T)rowPitch * tdesc.Size.Height };
        void* pData = 0;
        if (FAILED(tdesc.pStagingTexture->Map(0, &range, &pData)))
            return false;

        unsigned rows = Alg::Min<unsigned>(tdesc.Size.Height, splane.GetSize().Height);
        unsigned rowPixels = Alg::Min<unsigned>(tdesc.Size.Width, splane.GetSize().Width);
        if (expand24)
        {
            UByte* pRow = (UByte*)malloc((size_t)rowPixels * bpp);
            if (!pRow)
            {
                tdesc.pStagingTexture->Unmap(0, 0);
                return false;
            }
            for (unsigned y = 0; y < rows; y++)
            {
                pmap->CopyFunc(pRow, splane.pData + (size_t)y * splane.Pitch,
                               (UPInt)rowPixels * 3, 0, 0);
                memcpy((UByte*)pData + (size_t)y * rowPitch, pRow,
                       (size_t)rowPixels * bpp);
            }
            free(pRow);
        }
        else
        {
            unsigned rowBytes = Alg::Min<unsigned>(tdesc.Size.Width * bpp,
                                                   splane.GetSize().Width * bpp);
            for (unsigned y = 0; y < rows; y++)
                memcpy((UByte*)pData + (size_t)y * rowPitch,
                       splane.pData + (size_t)y * splane.Pitch, rowBytes);
        }
        tdesc.pStagingTexture->Unmap(0, 0);
    }
    return true;
}

void Texture::CommitImageUpload()
{
    TextureManager* pmgr = GetManager();
    ID3D12GraphicsCommandList* pcl = pmgr ? pmgr->GetCommandList() : 0;
    if (!pcl)
        return;

    unsigned bpp = GetTextureFormat()->pMapping->BytesPerPixel;
    for (unsigned i = 0; i < TextureCount; i++)
    {
        HWTextureDesc& tdesc = pTextures[i];
        if (!tdesc.pStagingTexture || !tdesc.pTexture)
            continue;

        UINT rowPitch = ComputeAlignedRowPitch(tdesc.Size.Width, bpp);
        D3D12_TEXTURE_COPY_LOCATION src, dst;
        memset(&src, 0, sizeof(src));
        memset(&dst, 0, sizeof(dst));
        src.pResource = tdesc.pStagingTexture;
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint.Footprint.Width  = tdesc.Size.Width;
        src.PlacedFootprint.Footprint.Height = tdesc.Size.Height;
        src.PlacedFootprint.Footprint.Depth  = 1;
        src.PlacedFootprint.Footprint.Format = GetTextureFormat()->GetD3DFormat();
        src.PlacedFootprint.Footprint.RowPitch = rowPitch;
        dst.pResource = tdesc.pTexture;
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        RecordResourceBarrier(pcl, tdesc.pTexture,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                              D3D12_RESOURCE_STATE_COPY_DEST);
        pcl->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
        RecordResourceBarrier(pcl, tdesc.pTexture,
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Keep the staging buffer alive only until the next frame fence; large
        // image sets would otherwise exhaust the upload heap.
        pmgr->PendingStagingRelease.PushBack(tdesc.pStagingTexture);
        tdesc.pStagingTexture.Clear();
    }
}

void Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    // YUV video textures present a new frame on every bind; everything else
    // (font atlases, UI images) only needs the deferred one-shot upload.
    const ImageFormat bindFmt = GetImageFormat();
    if (pImage && (bindFmt == Image_Y8_U2_V2 || bindFmt == Image_Y8_U2_V2_A8))
        NeedsUpload = PrepareImageUpload();
    else if (!NeedsUpload && !UploadTried && pImage)
    {
        UploadTried = true;
        NeedsUpload = PrepareImageUpload();
    }

    if (NeedsUpload)
    {
        NeedsUpload = false;
        CommitImageUpload();
    }

    Render::Texture::ApplyTexture(stageIndex, fm);

    TextureManager* pmanager = GetManager();
    HAL* hal = (HAL*)pmanager->GetOwnerHAL();
    if (!hal)
        return;

    for ( unsigned view = 0; view < TextureCount; ++view )
        hal->SetStageTexture((unsigned)(stageIndex + view), pTextures[view].SRV, pTextures[view].GPU,
                             pmanager->GetSamplerDescriptor(fm),
                             pmanager->GetSamplerDescriptorGPU(fm));
}

bool Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_Medium);
    if (!GetManager()->mapTexture(this, mipLevel, 1))
    {
        SF_DEBUG_WARNING(1, "Texture::Update failed - couldn't map texture");
        return false;
    }

    ImageFormat format = GetImageFormat();
    ImagePlane  dplane;

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);

        pMap->Data.GetPlane(desc.PlaneIndex, &dplane);
        dplane.pData += desc.DestRect.y1 * dplane.Pitch +
                        desc.DestRect.x1 * GetTextureFormat()->pMapping->BytesPerPixel;

        splane.SetSize(desc.DestRect.GetSize());
        dplane.SetSize(desc.DestRect.GetSize());
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                          pFormat->GetScanlineCopyFn(), 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL*)
{
    // Render target content is read back through the mapped-texture path
    // (MappedTexture::Map issues a GPU->CPU copy); no separate implementation
    // is required in the prototype.
    return true;
}

bool Texture::UpdateStagingData(Render::RenderTargetData*)
{
    return true;
}

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pDepthStencilSurface(0)
{
    DSV.ptr = 0;
    DSVIndex = ~0u;
}

DepthStencilSurface::~DepthStencilSurface()
{
    // Same deferred-release rule as Texture: a depth/stencil resource may be
    // referenced by the command list that is still being recorded, so it is
    // parked in the kill list (drained after the frame fence) instead of being
    // destroyed here. The DSV descriptor slot is already deferred separately via
    // freeDSVDescriptor -> PendingDSVDescriptors.
    if (pDepthStencilSurface)
    {
        TextureManager* pmgr = (TextureManager*)GetTextureManager();
        if (pmgr)
        {
            if (IsDebugEnvSet("GFX_TEXREL"))
            {
                static int sLog = 0;
                if (sLog++ < 4000)
                    fprintf(stderr, "[DSDTOR] %p %ux%u -> killlist\n", this, Size.Width, Size.Height);
            }
            pmgr->D3DTextureKillList.PushBack(pDepthStencilSurface);
            pDepthStencilSurface.Clear();
        }
    }
    if (DSVIndex != ~0u)
    {
        TextureManager* pmgr = (TextureManager*)GetTextureManager();
        if (pmgr)
            pmgr->freeDSVDescriptor(DSVIndex);
        DSVIndex = ~0u;
    }
}

bool DepthStencilSurface::Initialize()
{
    TextureManager* pmanager = (TextureManager*)GetTextureManager();

    if (!CreateCommittedTexture2D(pmanager->pDevice, Size.Width, Size.Height,
                                  DXGI_FORMAT_D24_UNORM_S8_UINT, 1,
                                  D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
                                  D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                  &pDepthStencilSurface.GetRawRef()))
    {
        State = Texture::State_InitFailed;
        return false;
    }
    if (pDepthStencilSurface)
    {
        wchar_t wname[96];
        swprintf_s(wname, L"GFx_DS_%ux%u", Size.Width, Size.Height);
        pDepthStencilSurface->SetName(wname);
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    memset(&dsvDesc, 0, sizeof(dsvDesc));
    dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;
    DSV = pmanager->allocDSVDescriptor(&DSVIndex);
    pmanager->pDevice->CreateDepthStencilView(pDepthStencilSurface, &dsvDesc, DSV);

    State = Texture::State_Valid;
    return true;
}

// ***** MappedTexture

bool MappedTexture::Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(!IsMapped());
    SF_ASSERT((mipLevel + levelCount) <= ptexture->MipLevels);

    // Initialize Data as efficiently as possible.
    if (levelCount <= PlaneReserveSize)
        Data.Initialize(ptexture->GetImageFormat(), levelCount, Planes, ptexture->GetPlaneCount(), true);
    else if (!Data.Initialize(ptexture->GetImageFormat(), levelCount, true))
        return false;

    D3D12::Texture* d3dTexture = reinterpret_cast<Texture*>(ptexture);
    TextureManager* pmgr       = d3dTexture->GetManager();
    Ptr<ID3D12Device> pdevice  = pmgr->GetDevice();
    pTexture                   = ptexture;
    StartMipLevel              = mipLevel;
    LevelCount                 = levelCount;

    unsigned textureCount = ptexture->TextureCount;
    unsigned planeCount   = ptexture->GetPlaneCount();
    bool readBack         = (ptexture->Use & (ImageUse_MapRenderThread|ImageUse_RenderTarget)) ==
                            (ImageUse_MapRenderThread|ImageUse_RenderTarget);

    for (unsigned pindex = 0; pindex < planeCount; pindex++)
    {
        unsigned itex = pindex % textureCount;
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        ImagePlane              plane(tdesc.Size, 0);

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        // Create a staging buffer large enough for the mapped region.
        if (!tdesc.pStagingTexture)
        {
            if (!CreateStagingBufferForTexture(pdevice, tdesc,
                                               readBack ? D3D12_HEAP_TYPE_READBACK
                                                        : D3D12_HEAP_TYPE_UPLOAD,
                                               StartMipLevel, &tdesc.pStagingTexture.GetRawRef()))
                return false;
        }

        // For render targets, copy the GPU content into the staging buffer.
        if (readBack && pmgr->pCommandList)
        {
            D3D12_TEXTURE_COPY_LOCATION src, dst;
            memset(&src, 0, sizeof(src));
            memset(&dst, 0, sizeof(dst));
            src.pResource = tdesc.pTexture;
            src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            src.SubresourceIndex = StartMipLevel;
            dst.pResource = tdesc.pStagingTexture;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dst.PlacedFootprint.Offset = 0;
            dst.PlacedFootprint.Footprint.Width = tdesc.Size.Width;
            dst.PlacedFootprint.Footprint.Height = tdesc.Size.Height;
            dst.PlacedFootprint.Footprint.Depth = 1;
            dst.PlacedFootprint.Footprint.Format = d3dTexture->GetTextureFormat()->GetD3DFormat();
            dst.PlacedFootprint.Footprint.RowPitch =
                ComputeAlignedRowPitch(tdesc.Size.Width, d3dTexture->GetTextureFormat()->pMapping->BytesPerPixel);

            RecordResourceBarrier(pmgr->pCommandList, tdesc.pTexture,
                                  D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
            pmgr->pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
            RecordResourceBarrier(pmgr->pCommandList, tdesc.pTexture,
                                  D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }

        D3D12_RANGE range;
        range.Begin = 0;
        // Only map the bytes that actually exist in the staging buffer. Using
        // width*height*4 unconditionally fails for 1-byte formats (A8 font /
        // alpha textures), which makes text and many UI images disappear.
        UINT rowPitch = ComputeAlignedRowPitch(plane.GetSize().Width,
                                               d3dTexture->GetTextureFormat()->pMapping->BytesPerPixel);
        range.End   = (SIZE_T)rowPitch * plane.GetSize().Height;
        void* pData = 0;
        if (FAILED(tdesc.pStagingTexture->Map(0, &range, &pData)))
            return false;

        plane.Pitch    = rowPitch;
        plane.pData    = (UByte*)pData;
        plane.DataSize = ImageData::GetMipLevelSize(Data.GetFormat(), plane.GetSize(), pindex);

        Data.SetPlane(pindex, plane);
        // Prepare for next level.
        plane.SetNextMipSize();
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool applyUpdate)
{
    Texture* d3dTexture = reinterpret_cast<Texture*>(pTexture);
    TextureManager* pmgr = d3dTexture->GetManager();

    unsigned textureCount = pTexture->TextureCount;
    bool readBack         = (pTexture->Use & (ImageUse_MapRenderThread|ImageUse_RenderTarget)) ==
                            (ImageUse_MapRenderThread|ImageUse_RenderTarget);

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        ImagePlane plane;

        tdesc.pStagingTexture->Unmap(0, 0);

        // Upload staging content into the texture (not needed for read-back maps).
        if (applyUpdate && !readBack && pmgr->pCommandList)
        {
            D3D12_TEXTURE_COPY_LOCATION src, dst;
            memset(&src, 0, sizeof(src));
            memset(&dst, 0, sizeof(dst));
            src.pResource = tdesc.pStagingTexture;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint.Offset = 0;
            src.PlacedFootprint.Footprint.Width = tdesc.Size.Width;
            src.PlacedFootprint.Footprint.Height = tdesc.Size.Height;
            src.PlacedFootprint.Footprint.Depth = 1;
            src.PlacedFootprint.Footprint.Format = d3dTexture->GetTextureFormat()->GetD3DFormat();
            src.PlacedFootprint.Footprint.RowPitch =
                ComputeAlignedRowPitch(tdesc.Size.Width, d3dTexture->GetTextureFormat()->pMapping->BytesPerPixel);
            dst.pResource = tdesc.pTexture;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = StartMipLevel;

            RecordResourceBarrier(pmgr->pCommandList, tdesc.pTexture,
                                  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
            pmgr->pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
            RecordResourceBarrier(pmgr->pCommandList, tdesc.pTexture,
                                  D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }

        for (unsigned level = 0; level < (unsigned)LevelCount; level++)
        {
            Data.GetPlane(level * textureCount + itex, &plane);
            plane.pData = 0;
        }
    }

    pTexture->pMap = 0;
    pTexture       = 0;
    StartMipLevel  = 0;
    LevelCount     = 0;
}

// ***** TextureManager

TextureManager::TextureManager(ID3D12Device* pdevice,
                               RenderSync* rsync,
                               ThreadId renderThreadId,
                               ThreadCommandQueue* commandQueue,
                               TextureCache* texCache) :
    Render::TextureManager(renderThreadId, commandQueue, texCache),
    pDevice(pdevice), pCommandList(0), pOwnerHAL(0),
    SRVHeapCursor(0), SamplerHeapCursor(0), RTVHeapCursor(0), DSVHeapCursor(0)
{
    SF_UNUSED(rsync);

    // Create descriptor heaps.
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.NumDescriptors = D3D12_SRVHeapDescriptorCount;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSRVHeap.GetRawRef()))))
        SF_DEBUG_ERROR(1, "Failed to create D3D12 SRV descriptor heap");

    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.NumDescriptors = D3D12_SamplerHeapDescriptorCount;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSamplerHeap.GetRawRef()))))
        SF_DEBUG_ERROR(1, "Failed to create D3D12 sampler descriptor heap");

    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.NumDescriptors = D3D12_RTVHeapDescriptorCount;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRTVHeap.GetRawRef()))))
        SF_DEBUG_ERROR(1, "Failed to create D3D12 RTV descriptor heap");

    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.NumDescriptors = D3D12_DSVHeapDescriptorCount;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pDSVHeap.GetRawRef()))))
        SF_DEBUG_ERROR(1, "Failed to create D3D12 DSV descriptor heap");

    initTextureFormats();

    // Precreate the sampler states. One group of TextureStageCount identical
    // samplers per fill mode: the s0..s7 descriptor table declared by the root
    // signature is bound to the whole group, so every sampler the shader can
    // reach is a valid, initialized descriptor.
    memset(SamplerStates, 0, sizeof(SamplerStates));
    memset(SamplerStateGPUs, 0, sizeof(SamplerStateGPUs));
    SamplerHeapCursor = 0;
    UINT samplerIncrement = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    D3D12_CPU_DESCRIPTOR_HANDLE samplerHeapCPU = pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE samplerHeapGPU = pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
    for ( unsigned wrap = 0; wrap < Wrap_Count; wrap++ )
    {
        for ( unsigned filter = 0; filter < Sample_Count; ++filter )
        {
            D3D12_SAMPLER_DESC samplerDesc;
            memset(&samplerDesc, 0, sizeof(samplerDesc));
            switch( filter << Sample_Shift )
            {
            default:
            case Sample_Point:  samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; break;
            case Sample_Linear: samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; break;
            }

            switch( wrap )
            {
            default:
            case Wrap_Clamp:    samplerDesc.AddressU = samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP; break;
            case Wrap_Repeat:   samplerDesc.AddressU = samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; break;
            }
            samplerDesc.AddressW           = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            samplerDesc.ComparisonFunc     = D3D12_COMPARISON_FUNC_ALWAYS;
            samplerDesc.MaxLOD             = D3D12_FLOAT32_MAX;
            samplerDesc.MinLOD             = 0;
            samplerDesc.MipLODBias         = -0.75f;
            samplerDesc.MaxAnisotropy      = 1;

            ImageFillMode fm((WrapMode)wrap, (SampleMode)(filter << Sample_Shift));
            SF_ASSERT( fm.Fill < SamplerTypeCount );

            SamplerStates[fm.Fill] = samplerHeapCPU;
            SamplerStates[fm.Fill].ptr += SamplerHeapCursor * samplerIncrement;
            SamplerStateGPUs[fm.Fill] = samplerHeapGPU;
            SamplerStateGPUs[fm.Fill].ptr += SamplerHeapCursor * samplerIncrement;

            for ( unsigned stage = 0; stage < TextureStageCount; ++stage )
            {
                D3D12_CPU_DESCRIPTOR_HANDLE cpu = SamplerStates[fm.Fill];
                cpu.ptr += stage * samplerIncrement;
                pDevice->CreateSampler(&samplerDesc, cpu);
                SamplerHeapCursor++;
            }
        }
    }

    // The persistent sampler groups and the (unused) upper heap area must not
    // overlap; keep this assert so a future addition cannot silently overrun.
    SF_ASSERT(SamplerHeapCursor <= (UPInt)D3D12_SamplerHeapDescriptorCount);
}

TextureManager::~TextureManager()
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    Reset();
    pLocks->pManager = 0;
}

void TextureManager::Reset()
{
    Mutex::Locker lock(&pLocks->TextureMutex);

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    SF_ASSERT(TextureInitQueue.IsEmpty());

    // Drain pending image updates while the video players are still alive.
    ImageUpdates.ProcessUpdates(this);

    // Do not call Texture::LoseManager() here. For glyph-cache textures it
    // invokes TextureLost(), which can release the texture/image while the
    // intrusive list is being walked and corrupt the list sentinel. The HAL
    // and device are being torn down, so detach the list and let resources
    // die with the manager.
    Textures.Clear();

    processTextureKillList();

    memset(SamplerStates, 0, sizeof(SamplerStates));
    memset(SamplerStateGPUs, 0, sizeof(SamplerStateGPUs));

    pSRVHeap.Clear();
    pSamplerHeap.Clear();
    pRTVHeap.Clear();
    pDSVHeap.Clear();
    FreeSRVDescriptors.Clear();
    FreeRTVDescriptors.Clear();
    FreeDSVDescriptors.Clear();
    PendingSRVDescriptors.Clear();
    PendingRTVDescriptors.Clear();
    PendingDSVDescriptors.Clear();
    PendingStagingRelease.Clear();
    SRVHeapCursor = SamplerHeapCursor = RTVHeapCursor = DSVHeapCursor = 0;
    pCommandList.Clear();
    pDevice = 0;
}

void TextureManager::SetCommandList(ID3D12GraphicsCommandList* commandList)
{
    pCommandList = commandList;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetSamplerDescriptor(const ImageFillMode& fm) const
{
    SF_ASSERT(fm.Fill < SamplerTypeCount);
    return SamplerStates[fm.Fill];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSamplerDescriptorGPU(const ImageFillMode& fm) const
{
    SF_ASSERT(fm.Fill < SamplerTypeCount);
    return SamplerStateGPUs[fm.Fill];
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRVScratchBase() const
{
    D3D12_GPU_DESCRIPTOR_HANDLE gpu = pSRVHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += (D3D12_SRVHeapDescriptorCount - SRVScratchDescriptorCount) *
               pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    return gpu;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::allocSRVDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE* pGPU, unsigned* pIndex)
{
    SF_ASSERT(pSRVHeap);
    UINT increment = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    unsigned index;
    if (FreeSRVDescriptors.GetSize() > 0)
    {
        index = FreeSRVDescriptors.Back();
        FreeSRVDescriptors.PopBack();
    }
    else
        index = static_cast<unsigned int>(SRVHeapCursor++);
    if (pIndex)
        *pIndex = index;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = pSRVHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += index * increment;
    if (pGPU)
    {
        *pGPU = pSRVHeap->GetGPUDescriptorHandleForHeapStart();
        pGPU->ptr += index * increment;
    }
    if (index >= D3D12_SRVHeapDescriptorCount)
    {
        static int sWarned = 0;
        if (sWarned++ < 8)
        {
            fprintf(stderr, "[D3D12] SRV heap exhausted (index=%u/%u)\n",
                    index, D3D12_SRVHeapDescriptorCount);
            fflush(stderr);
        }
        index = D3D12_SRVHeapDescriptorCount - 1;
        cpu = pSRVHeap->GetCPUDescriptorHandleForHeapStart();
        cpu.ptr += index * increment;
        if (pGPU)
        {
            *pGPU = pSRVHeap->GetGPUDescriptorHandleForHeapStart();
            pGPU->ptr += index * increment;
        }
        if (pIndex)
            *pIndex = index;
    }
    return cpu;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::allocRTVDescriptor(unsigned* pIndex)
{
    SF_ASSERT(pRTVHeap);
    UINT increment = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    unsigned index;
    if (FreeRTVDescriptors.GetSize() > 0)
    {
        index = FreeRTVDescriptors.Back();
        FreeRTVDescriptors.PopBack();
    }
    else
        index = static_cast<unsigned int>(RTVHeapCursor++);
    if (pIndex)
        *pIndex = index;
    if (index >= D3D12_RTVHeapDescriptorCount)
    {
        static int sWarned = 0;
        if (sWarned++ < 8)
        {
            fprintf(stderr, "[D3D12] RTV heap exhausted (index=%u/%u)\n",
                    index, D3D12_RTVHeapDescriptorCount);
            fflush(stderr);
        }
        index = D3D12_RTVHeapDescriptorCount - 1;
        if (pIndex)
            *pIndex = index;
    }
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += index * increment;
    return cpu;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::allocDSVDescriptor(unsigned* pIndex)
{
    SF_ASSERT(pDSVHeap);
    UINT increment = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    unsigned index;
    if (FreeDSVDescriptors.GetSize() > 0)
    {
        index = FreeDSVDescriptors.Back();
        FreeDSVDescriptors.PopBack();
    }
    else
        index = static_cast<unsigned int>(DSVHeapCursor++);
    if (pIndex)
        *pIndex = index;
    if (index >= D3D12_DSVHeapDescriptorCount)
    {
        static int sWarned = 0;
        if (sWarned++ < 8)
        {
            fprintf(stderr, "[D3D12] DSV heap exhausted (index=%u/%u)\n",
                    index, D3D12_DSVHeapDescriptorCount);
            fflush(stderr);
        }
        index = D3D12_DSVHeapDescriptorCount - 1;
        if (pIndex)
            *pIndex = index;
    }
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = pDSVHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += index * increment;
    return cpu;
}

void TextureManager::freeSRVDescriptor(unsigned index)  { PendingSRVDescriptors.PushBack(index); }
void TextureManager::freeRTVDescriptor(unsigned index)  { PendingRTVDescriptors.PushBack(index); }
void TextureManager::freeDSVDescriptor(unsigned index)  { PendingDSVDescriptors.PushBack(index); }

void TextureManager::BeginFrame()
{
    // The platform waits for the previous frame's frame fence before
    // HAL::BeginFrame, so everything released during that frame can be recycled
    // (or handed back to the GPU) safely now.
    for (UPInt i = 0; i < PendingSRVDescriptors.GetSize(); i++)
        FreeSRVDescriptors.PushBack(PendingSRVDescriptors[i]);
    for (UPInt i = 0; i < PendingRTVDescriptors.GetSize(); i++)
        FreeRTVDescriptors.PushBack(PendingRTVDescriptors[i]);
    for (UPInt i = 0; i < PendingDSVDescriptors.GetSize(); i++)
        FreeDSVDescriptors.PushBack(PendingDSVDescriptors[i]);
    PendingSRVDescriptors.Clear();
    PendingRTVDescriptors.Clear();
    PendingDSVDescriptors.Clear();
    PendingStagingRelease.Clear();

    processTextureKillList();
}

void TextureManager::BeginScene()
{
    // Per-frame recycling happens in BeginFrame(); a scene boundary is *not* a
    // safe point to release GPU resources, because the frame's command list is
    // still open (and may already reference them).
}

void TextureManager::initTextureFormats()
{
    TextureFormat::Mapping* pmapping = 0;
    for (pmapping = D3D12TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
        memset(&formatSupport, 0, sizeof(formatSupport));
        formatSupport.Format = pmapping->D3DFormat;
        if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                                &formatSupport, sizeof(formatSupport))))
            continue;
        if (!(formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D))
            continue;

        TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping);
        TextureFormats.PushBack(tf);

        // If format added, skip additional mappings for it.
        while ((pmapping+1)->Format == pmapping->Format)
            pmapping++;
    }
}

void TextureManager::processTextureKillList()
{
    // The kill list stores Ptr<ID3D12Resource>, so Clear() releases each entry
    // exactly once. The previous code called Release() explicitly *and* then
    // cleared the Ptr array, which released every queued resource twice.
    // Entries are queued from the render thread (deferred depth/stencil release)
    // and from off-thread texture destruction; BeginFrame drains the list only
    // after the platform has waited for the previous frame's fence.
    D3DTextureKillList.Clear();
}

void TextureManager::processInitTextures()
{
    // TextureMutex lock expected externally.
    //Mutex::Locker lock(&TextureMutex);

    if (!TextureInitQueue.IsEmpty() || !DepthStencilInitQueue.IsEmpty())
    {
        while (!TextureInitQueue.IsEmpty())
        {
            Render::Texture* ptexture = TextureInitQueue.GetFirst();
            ptexture->RemoveNode();
            ptexture->pPrev = ptexture->pNext = 0;
            if (ptexture->Initialize())
                Textures.PushBack(ptexture);
        }
        while (!DepthStencilInitQueue.IsEmpty())
        {
            Render::DepthStencilSurface* pdss = DepthStencilInitQueue.GetFirst();
            pdss->RemoveNode();
            pdss->pPrev = pdss->pNext = 0;
            pdss->Initialize();
        }
        pLocks->TextureInitWC.NotifyAll();
    }
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size,
                                               unsigned use, ImageBase* pimage,
                                               Render::MemoryManager* allocManager)
{
    SF_UNUSED(allocManager);
    SF_AMP_SCOPE_TIMER(RenderThreadId == GetCurrentThreadId() ? AmpServer::GetInstance().GetDisplayStats() : NULL, __FUNCTION__, Amp_Profile_Level_Medium);
    if (!pDevice)
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - TextureManager has been Reset");
        return 0;
    }

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Texture* ptexture =
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(ID3D12Resource* pd3dtexture,
                                               D3D12_RESOURCE_STATES usage,
                                               ImageSize imgSize, ImageBase* image )
{
    SF_UNUSED(usage);
    if ( pd3dtexture == 0 )
        return 0;

    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, pd3dtexture, usage, imgSize, image);

    return postCreateTexture(ptexture, 0);
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    // ImageUse_InitOnly (ImageUse_NoDataLoss alias) ok while textures are managed.
    unsigned use = ImageUse_InitOnly | ImageUse_Update;
    if (!ImageData::IsFormatCompressed(format))
        use |= ImageUse_PartialUpdate | ImageUse_GenMipmaps;

    const Render::TextureFormat* ptformat = getTextureFormat(format);
    if (!ptformat)
        return 0;
    if (isScanlineCompatible(ptformat))
        use |= ImageUse_MapRenderThread;
    return use;
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size, MemoryManager* manager)
{
    SF_UNUSED(manager);
    if (!pDevice)
    {
        SF_DEBUG_ERROR(1, "CreateDepthStencilSurface failed - TextureManager has been Reset");
        return 0;
    }

    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, size);
    return postCreateDepthStencilSurface(pdss);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(ID3D12Resource* psurface,
                                                                       D3D12_RESOURCE_STATES usage)
{
    SF_UNUSED(usage);
    if ( !psurface )
        return 0;

    D3D12_RESOURCE_DESC desc = psurface->GetDesc();
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, ImageSize((unsigned)desc.Width, (unsigned)desc.Height) );
    pdss->pDepthStencilSurface = psurface;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    memset(&dsvDesc, 0, sizeof(dsvDesc));
    dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;
    pdss->DSV = allocDSVDescriptor(&pdss->DSVIndex);
    pDevice->CreateDepthStencilView(psurface, &dsvDesc, pdss->DSV);
    pdss->State = Texture::State_Valid;
    return pdss;
}

}}} // namespace Scaleform::Render::D3D12
