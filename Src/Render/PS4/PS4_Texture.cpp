/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Texture.cpp
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/PS4/PS4_Texture.h"
#include "Render/Render_MemoryManager.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"

#include "Render/PS4/PS4_MemoryManager.h"

#include <sdk_version.h>

namespace Scaleform { namespace Render { namespace PS4 {

extern TextureFormat::Mapping TextureFormatMapping[];

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat)
{
    TextureCount = (UByte) pformat->GetPlaneCount();
    if (TextureCount > 1)
    {
        pTextures = (HWTextureDesc*)
            SF_HEAP_AUTO_ALLOC(this, sizeof(HWTextureDesc) * TextureCount);
    }
    else
    {
        pTextures = &Texture0;
    }
    memset(pTextures, 0, sizeof(HWTextureDesc) * TextureCount);
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, const sce::Gnm::Texture& tex, const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0)
{
    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    pTextures[0].Tex = tex;
    // Needed only for deallocation, user is responsible for freeing.
    pTextures[0].pTexData = 0;
    pTextures[0].ByteSize = 0;
}

Texture::~Texture()
{
    //  pImage must be null, since ImageLost had to be called externally.
    SF_ASSERT(pImage == 0);
    
    Mutex::Locker  lock(&pManagerLocks->TextureMutex);
 
    if ((State == State_Valid) || (State == State_Lost))
    {
        // pManagerLocks->pManager should still be valid for these states.
        SF_ASSERT(pManagerLocks->pManager);
        RemoveNode();
        pNext = pPrev = 0;
        // If not on Render thread, add HW textures to queue.
        ReleaseHWTextures();
    }

    if ((pTextures != &Texture0) && pTextures)
        SF_FREE(pTextures);
}

void    Texture::LoseManager()
{        
    Ptr<Texture> thisTexture = this;    // Guards against the texture being deleted inside Texture::LoseManager()
    SF_ASSERT(pMap == 0);
    Render::Texture::LoseManager();
    pFormat = 0; // Users can't access Format any more ?
}

bool Texture::Initialize()
{    
    if (TextureFlags & TF_UserAlloc )
    {
        return Initialize(pTextures[0].Tex);
    }

    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    // Determine how many mipLevels we should have and whether we can
    // auto-generate them or not.
    unsigned allocMipLevels = MipLevels;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        TextureFlags |= TF_SWMipGen;
        // If using SW MipGen, determine how many mip-levels we should have.
        allocMipLevels = 31;
        for (itex = 0; itex < TextureCount; itex++)
        {
            allocMipLevels = Alg::Min(allocMipLevels, ImageSize_MipLevelCount(ImageSize(pTextures[itex].Tex.getWidth(), 
                                      pTextures[itex].Tex.getHeight())));
        }
        MipLevels = (UByte)allocMipLevels;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        sce::Gnm::SizeAlign allocSize;

        // If it is a render target as allocate the target, and then initialize the texture from that structure.
        ImageSize textureSize = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
        if (Use & ImageUse_RenderTarget)
        {
            sce::Gnm::TileMode tileMode;
            sce::GpuAddress::computeSurfaceTileMode(&tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, GetTextureFormatMapping()->DataFormat, 1);
            allocSize = tdesc.Surface.init(textureSize.Width, textureSize.Height, 1, GetTextureFormatMapping()->DataFormat, 
                tileMode, sce::Gnm::kNumSamples1, sce::Gnm::kNumFragments1, 0, 0);
        }
        else
        {
            sce::Gnm::TileMode tileMode = sce::Gnm::kTileModeDisplay_LinearAligned;

            // If the image format is marked as tiled, indicate that.
            if ((UPInt)GetImageFormat() & ImageStorage_Tile)
                tileMode = sce::Gnm::kTileModeThin_1dThin;

            allocSize = tdesc.Tex.initAs2d(textureSize.Width, textureSize.Height, MipLevels, GetTextureFormatMapping()->DataFormat, 
                tileMode, sce::Gnm::kNumFragments1 );
        }

        tdesc.ByteSize = allocSize.m_size;

        tdesc.pTexData = pmanager->pMemManager->Alloc(allocSize.m_size, allocSize.m_align, Memory_Orbis_UC_GARLIC_NONVOLATILE); 

        if (UPInt(tdesc.pTexData) == ~UPInt(0))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            // Texture creation failed, release all textures and fail.
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }

        if (Use & ImageUse_RenderTarget)
        {
            tdesc.Surface.setAddresses(tdesc.pTexData, 0, 0);
            tdesc.Tex.initFromRenderTarget(&tdesc.Surface, false);
        }
        else
        {
            tdesc.Tex.setBaseAddress256ByteBlocks(reinterpret_cast<UPInt>(tdesc.pTexData) >> 8);
        }
    }

    // Upload image content to texture, if any.
    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture");
        ReleaseHWTextures();
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return true;
}

bool Texture::Initialize(const sce::Gnm::Texture& tex)
{
    TextureManager* pmanager = GetManager();
    TextureFormat::Mapping* pmapping;

    pTextures[0].Tex = tex;
    MipLevels = tex.getLastMipLevel() - tex.getBaseMipLevel();

    // If an image is provided, try to obtain the texture format from the image.
    pFormat = 0;
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->DataFormat.getSurfaceFormat() == tex.getDataFormat().getSurfaceFormat())
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

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = ImageSize(pTextures[0].Tex.getWidth(), pTextures[0].Tex.getHeight());

    State = State_Valid;
    return true;
}

void Texture::ReleaseHWTextures(bool)
{
    TextureManager* pmanager = GetManager();

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        if (pTextures[itex].pTexData)
            pmanager->pMemManager->Free(reinterpret_cast<void*>(pTextures[itex].pTexData), pTextures[itex].ByteSize);
        pTextures[itex].pTexData = 0;
        pTextures[itex].ByteSize = 0;
    }
}

// Applies a texture to device starting at pstageIndex, advances index
void    Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    sceGnmxContextType* GnmxCtx = GetManager()->GnmxCtx;

    sce::Gnm::WrapMode   address   = (fm.GetWrapMode() == Wrap_Clamp ? sce::Gnm::kWrapModeClampLastTexel : sce::Gnm::kWrapModeWrap);
    sce::Gnm::FilterMode minfilter = (fm.GetSampleMode() == Sample_Point ? sce::Gnm::kFilterModePoint : sce::Gnm::kFilterModeBilinear);
    sce::Gnm::FilterMode magfilter = (fm.GetSampleMode() == Sample_Point ? sce::Gnm::kFilterModePoint : sce::Gnm::kFilterModeBilinear);
    sce::Gnm::MipFilterMode mipfilter = (MipLevels > 1 ? sce::Gnm::MipFilterMode::kMipFilterModeLinear : sce::Gnm::kMipFilterModeNone);

    sce::Gnm::Sampler samplerState;
    sce::Gnm::Texture textureArray[MaxTextureCount];

    samplerState.init();
    samplerState.setWrapMode(address, address, address);
    samplerState.setMipFilterMode(mipfilter);
    samplerState.setXyFilterMode(magfilter, minfilter);

    for (unsigned i = 0; i < TextureCount; i++)
    {
        textureArray[i] = pTextures[i].Tex;
    }
    GnmxCtx->setTextures(sce::Gnm::kShaderStagePs, stageIndex, TextureCount, textureArray);
    GnmxCtx->setSamplers(sce::Gnm::kShaderStagePs, 0, 1, &samplerState);
}


bool    Texture::Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT((Use & ImageUse_Map_Mask) != 0);
    SF_ASSERT(!pMap);

    if (levelCount == 0)
        levelCount = MipLevels - mipLevel;
    
    if (!GetManager()->mapTexture(this, mipLevel, levelCount))
    {
        SF_DEBUG_WARNING(1, "Texture::Map failed - couldn't map texture");
        return false;
    }

    pdata->Initialize(GetImageFormat(), levelCount,
                      pMap->Data.pPlanes, pMap->Data.RawPlaneCount, true);
    pdata->Use = Use;
    return true;
}

bool    Texture::Unmap()
{
    if (!pMap) return false;
    GetManager()->unmapTexture(this);
    return true;
}


bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
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
        // TODOBM:
        //dplane.pData += desc.DestRect.y1 * dplane.Pitch +
        //                desc.DestRect.x1 * pFormat->BytesPerPixel;

        splane.SetSize(desc.DestRect.GetSize());
        dplane.SetSize(desc.DestRect.GetSize());
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                          pFormat->GetScanlineCopyFn(), 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat format, 
                                                ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_DEBUG_ASSERT(!rescale, "Orbis does not support rescale (it should not be necessary).");
    SF_UNUSED4(rescale, swMipGen, rescaleType, rescaleBuffFromat);
    if (swMipGen && !(format == Image_R8G8B8A8 || format == Image_B8G8R8A8 || format == Image_A8))
        convert = true;
}

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pMemManager((PS4::MemoryManager*)pmanager), State(Texture::State_InitPending)
{
    // If a MemoryManager isn't provided, assume that the TextureManager's MemoryManager should be used.
    if (!pMemManager)
    {
        PS4::TextureManager* manager = (PS4::TextureManager*)GetTextureManager();
        pMemManager = (PS4::MemoryManager*)(manager->GetMemoryManager());
    }
    memset(&DepthTarget, 0, sizeof(DepthTarget));
}

DepthStencilSurface::~DepthStencilSurface()
{
    if ( pMemManager)
    {
        if (StencilSize.m_size > 0)
            pMemManager->Free(reinterpret_cast<void*>(DepthTarget.getStencilReadAddress()), StencilSize.m_size);
        if (DepthSize.m_size > 0 )
            pMemManager->Free(reinterpret_cast<void*>(DepthTarget.getZReadAddress()), DepthSize.m_size);
    }
}

bool DepthStencilSurface::Initialize()
{
    if ( !pMemManager )
        return false;

    sce::GpuAddress::SurfaceType surfaceType;
    if (DepthSize.m_size > 0 && StencilSize.m_size > 0 )
        surfaceType = sce::GpuAddress::kSurfaceTypeDepthOnlyTarget;
    else if (DepthSize.m_size > 0)
        surfaceType = sce::GpuAddress::kSurfaceTypeDepthOnlyTarget;
    else 
    {
        SF_DEBUG_ASSERT(StencilSize.m_size > 0, "Attempting to allocate DepthStencilSurface with no depth or stencil data.");
        surfaceType = sce::GpuAddress::kSurfaceTypeDepthOnlyTarget;
    }

    sce::Gnm::TileMode depthTileMode;
    const sce::Gnm::DataFormat depthFormat = sce::Gnm::DataFormat::build(sce::Gnm::kZFormat32Float);
    const sce::Gnm::StencilFormat stencilFormat = StencilSize.m_size > 0 ? sce::Gnm::kStencil8 : sce::Gnm::kStencilInvalid;

    sce::GpuAddress::computeSurfaceTileMode(&depthTileMode, surfaceType, depthFormat, 1);

    DepthSize = DepthTarget.init( Size.Width, Size.Height, 
        depthFormat.getZFormat(), stencilFormat, depthTileMode, sce::Gnm::kNumFragments1, &StencilSize, 0 );

    void* pdepthMemory = 0;
    void* pstencilMemory = 0;
    if (DepthSize.m_size > 0)
        pdepthMemory = pMemManager->Alloc(DepthSize.m_size, DepthSize.m_align, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    if (StencilSize.m_size > 0)
        pstencilMemory = pMemManager->Alloc(StencilSize.m_size, StencilSize.m_align, Memory_Orbis_UC_GARLIC_NONVOLATILE);

    if (UPInt(pdepthMemory) == ~UPInt(0) || UPInt(pstencilMemory) == ~UPInt(0))
    {
        State = Texture::State_InitFailed;
    }
    else
    {
        State = Texture::State_Valid;
        DepthTarget.setAddresses(pdepthMemory, pstencilMemory);
    }
    return State == Texture::State_Valid;
}

// ***** MappedTexture

bool MappedTexture::Map(Render::Texture* pbaseTexture, unsigned mipLevel, unsigned levelCount)
{
    PS4::Texture* ptexture = reinterpret_cast<PS4::Texture*>(pbaseTexture);
    SF_ASSERT(!IsMapped());
    SF_ASSERT((mipLevel + levelCount) <= ptexture->MipLevels);

    // Initialize Data as efficiently as possible.
    if (levelCount <= PlaneReserveSize)
        Data.Initialize(ptexture->GetImageFormat(), levelCount, Planes, ptexture->GetPlaneCount(), true);
    else if (!Data.Initialize(ptexture->GetImageFormat(), levelCount, true))
        return false;

    pTexture      = ptexture;
    StartMipLevel = mipLevel;
    LevelCount    = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = ptexture->pTextures[itex];
        unsigned                pitchInBytes = (tdesc.Tex.getPitch() * tdesc.Tex.getDataFormat().getBitsPerElement()) / 8;
		ImagePlane              plane(tdesc.Tex.getWidth(), tdesc.Tex.getHeight(), pitchInBytes);
        UPInt                   baseAddr = reinterpret_cast<UPInt>(tdesc.Tex.getBaseAddress());

        for (unsigned level = 0; level < levelCount; level++)
        {
            uint64_t mipOffset, surfaceSize;
            sce::GpuAddress::computeTextureSurfaceOffsetAndSize(&mipOffset, &surfaceSize, &tdesc.Tex, StartMipLevel, 0);

            plane.pData = reinterpret_cast<UByte*>(baseAddr+mipOffset);
            plane.DataSize = surfaceSize;
            Data.SetPlane(level * textureCount + itex, plane);

            // Prepare for next level.
            plane.SetNextMipSize();
        }
    }

    pTexture->pMap = this;
    return true;
}

// ***** TextureManager
TextureManager::TextureManager(sceGnmxContextType* context, Render::MemoryManager* pmm) : 
    Render::TextureManager(),
    GnmxCtx(0),
    pMemManager(pmm)
{
    SetGfxContext(context);
    initTextureFormats();
}

TextureManager::~TextureManager()
{   
    Mutex::Locker lock(&pLocks->TextureMutex);

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    pLocks->pManager = 0;
}

// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

TextureFormat::Mapping TextureFormatMapping[] = 
{
    //{ Image_Orbis_R8G8B8A8_SZ, Image_None,     TEX(A8R8G8B8) | TEX(SZ),           REMAP_RGBA_2_ARGB, 4, true,  &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },
    //{ Image_Orbis_A8_SZ,       Image_None,     TEX(B8)       | TEX(SZ),           REMAP_A,           1, true,  &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    // Used for render-targets.
    //{ Image_Orbis_A8R8G8B8,    Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_NONE,        1, true,  &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    { Image_R8G8B8A8,        sce::Gnm::kDataFormatR8G8B8A8Unorm, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,        sce::Gnm::kDataFormatR8G8B8A8Unorm, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

    { Image_R8G8B8,          sce::Gnm::kDataFormatR8G8B8A8Unorm, &Image_CopyScanline24_Extend_RGB_RGBA, &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,          sce::Gnm::kDataFormatR8G8B8A8Unorm, &Image_CopyScanline24_Extend_RGB_RGBA, &Image_CopyScanline32_Retract_RGBA_RGB },

    { Image_A8,              sce::Gnm::kDataFormatA8Unorm,       &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

    // Tiled BCx formats. Note that non-tiled versions are not supported, because the GPU will not render them.
    { Image_Orbis_BC1,       sce::Gnm::kDataFormatBc1Unorm,      &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },
    { Image_Orbis_BC2,       sce::Gnm::kDataFormatBc2Unorm,      &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },
    { Image_Orbis_BC3,       sce::Gnm::kDataFormatBc3Unorm,      &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },
    { Image_Orbis_BC7,       sce::Gnm::kDataFormatBc7Unorm,      &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },

    { Image_Y8_U2_V2,        sce::Gnm::kDataFormatA8Unorm,       &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },
    { Image_Y8_U2_V2_A8,     sce::Gnm::kDataFormatA8Unorm,       &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault  },

    { Image_None,            sce::Gnm::kDataFormatInvalid,       0,                                     0 }
};

void TextureManager::initTextureFormats()
{
    TextureFormat::Mapping* pmapping;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping);
        TextureFormats.PushBack(tf);
    }
}

MappedTextureBase* TextureManager::mapTexture(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    MappedTexture* pmap;

    if (MappedTexture0.Reserve())
        pmap = &MappedTexture0;
    else
    {
        pmap = SF_HEAP_AUTO_NEW(this) MappedTexture;
        if (!pmap) return 0;
    }
    
    if (pmap->Map(ptexture, mipLevel, levelCount))
        return pmap;
    if (pmap != &MappedTexture0)
        delete pmap;
    return 0;  
}

void TextureManager::unmapTexture(Render::Texture *ptexture, bool )
{
    MappedTextureBase *pmap = ptexture->pMap;
    pmap->Unmap();
    if (pmap != &MappedTexture0)
        delete pmap;

//    ptexture->NeedsCacheFlush = true;
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size,
                                               unsigned use, ImageBase* pimage,
                                               Render::MemoryManager* allocManager)
{
    SF_UNUSED(allocManager);
    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);
    if (!ptexture)
        return 0;
    if (!ptexture->IsValid())
    {
        ptexture->Release();
        return 0;
    }

    Mutex::Locker lock(&pLocks->TextureMutex);

    if (ptexture->Initialize())
        Textures.PushBack(ptexture);

    // Clear out 'pImage' reference if it's not supposed to be kept. It is safe to do this
    // without ImageLock because texture hasn't been returned yet, so this is the only
    // thread which has access to it. Also free the data if it is a RawImage.
    if (use & ImageUse_InitOnly)
    {
        if ( ptexture->pImage && ptexture->pImage->GetImageType() == Image::Type_RawImage )
            ((Render::RawImage*)ptexture->pImage)->freeData();
        ptexture->pImage = 0;
    }

    // If texture was properly initialized, it would've been added to list.
    if (ptexture->State == Texture::State_InitFailed)
    {
        ptexture->Release();
        return 0;
    }
    return ptexture;
}

Render::Texture* TextureManager::CreateTexture(const sce::Gnm::Texture& tex, ImageSize size, ImageBase * pimage)
{
    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, tex, size, pimage);
    if (!ptexture)
        return 0;
    if (!ptexture->IsValid())
    {
        ptexture->Release();
        return 0;
    }

    Mutex::Locker lock(&pLocks->TextureMutex);

    if (ptexture->Initialize())
        Textures.PushBack(ptexture);

    // If texture was properly initialized, it would've been added to list.
    if (ptexture->State == Texture::State_InitFailed)
    {
        ptexture->Release();
        return 0;
    }
    return ptexture;
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    // ImageUse_InitOnly is ok while textures are Managed
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

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size, Render::MemoryManager* manager)
{
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, manager, size);
    if (!pdss)
        return 0;

    // Fill out some parameters to indicate which sort of depthstencil target is required.
    // The width and height go into the Size, and we set the stored sizes if depth and/or stencil are required.
    pdss->Size               = size;
    pdss->DepthSize.m_size   = 0;
    pdss->StencilSize.m_size = 1;

    pdss->Initialize();
    return pdss;
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const sce::Gnm::DepthRenderTarget& surface)
{
    // NOTE: MemManager is not passed to the new surface. Since its memory is already allocated, 
    // the allocator must deallocate the memory; as it may not be the MemManager used by this TextureManager.
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, 0, ImageSize(surface.getWidth(), surface.getHeight()) );
    memcpy(&pdss->DepthTarget, &surface, sizeof(pdss->DepthTarget));
    pdss->State = Texture::State_Valid;
    return pdss;
}

}}};  // namespace Scaleform::Render::Orbis

