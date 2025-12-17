/**************************************************************************

Filename    :   PS3_Texture.cpp
Content     :   PS3 Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "PS3_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"


namespace Scaleform { namespace Render { namespace PS3 {

extern TextureFormat::Mapping TextureFormatMapping[];

// ***** PS3 Format mapping and conversion functions

#define REMAP_RGBA_2_ARGB  CELL_GCM_REMAP_MODE (CELL_GCM_TEXTURE_REMAP_ORDER_XYXY, \
    CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, \
    CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP)

#define REMAP_NONE  CELL_GCM_REMAP_MODE (CELL_GCM_TEXTURE_REMAP_ORDER_XYXY, \
    CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B, \
    CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP)

#define REMAP_BGRA_2_ARGB  CELL_GCM_REMAP_MODE (CELL_GCM_TEXTURE_REMAP_ORDER_XYXY, \
    CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_A, \
    CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP)

#define REMAP_A  CELL_GCM_REMAP_MODE (CELL_GCM_TEXTURE_REMAP_ORDER_XYXY, \
    CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B, \
    CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_ONE,    CELL_GCM_TEXTURE_REMAP_ONE,    CELL_GCM_TEXTURE_REMAP_ONE)

#define TEX(f) CELL_GCM_TEXTURE_##f | CELL_GCM_TEXTURE_NR

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat), 
    NeedsCacheFlush(true)
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
    for (int i = 0; i < TextureCount; i++)
    {
        pTextures[i].Tex.format = GetTextureFormatMapping()->NativeFormat;
        pTextures[i].Tex.remap = GetTextureFormatMapping()->NativeRemap;
        pTextures[i].Tex.cubemap = CELL_GCM_FALSE;
        pTextures[i].Tex.dimension = CELL_GCM_TEXTURE_DIMENSION_2;

        ImageSize size = ImageData::GetFormatPlaneSize(pformat->GetImageFormat(), ImgSize, i);
        pTextures[i].Tex.width = size.Width;
        pTextures[i].Tex.height = size.Height;
        pTextures[i].Tex.depth = 1;
        pTextures[i].Tex.pitch = ImageData::GetFormatPitch(GetTextureFormatMapping()->ConvFormat, size.Width, 0);        
        // Pitch must be a multiple of 64, if using a render target.
        if (use & ImageUse_RenderTarget)
            pTextures[i].Tex.pitch = Alg::Align<64>(pTextures[i].Tex.pitch); 
        
		if (pTextures[i].Tex.pitch)
			pTextures[i].Pitch = pTextures[i].Tex.pitch;
		else
			pTextures[i].Pitch = size.Width * GetTextureFormatMapping()->BytesPerPixel;
    }
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, const CellGcmTexture& tex, const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0), 
    NeedsCacheFlush(false)
{
    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    pTextures[0].pTexData = 0;
    pTextures[0].Size = pTextures[0].Pitch = 0;
    pTextures[0].Tex = tex;
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

bool Texture::Initialize()
{    
    if (TextureFlags & TF_UserAlloc )
    {
        return Initialize(pTextures[0].Tex);
    }

    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    // Determine sizes of textures.
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        ImageSize size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);

        tdesc.Tex.width = size.Width;
        tdesc.Tex.height = size.Height;
    }
        
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
            allocMipLevels = Alg::Min(allocMipLevels,
                                      ImageSize_MipLevelCount(ImageSize(pTextures[itex].Tex.width, pTextures[itex].Tex.height)));
        MipLevels = (UByte)allocMipLevels;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        tdesc.Size = 0;
        tdesc.Tex.mipmap = MipLevels;

        unsigned h = tdesc.Tex.height;
		unsigned w = tdesc.Tex.width;
        if (!ImageData::IsFormatCompressed(format))
        {
			for (int imip = 0; imip < MipLevels; imip++)
            {
                tdesc.Size += tdesc.Pitch * h;
                h >>= 1;
                if (h == 0)
                    h = 1;
            }
        }
        else // compressed
        {
            unsigned    ww = 1; while (ww < tdesc.Tex.width) { ww <<= 1; }
            unsigned    hh = 1; while (hh < tdesc.Tex.height) { hh <<= 1; }

            tdesc.Tex.format |= CELL_GCM_TEXTURE_LN;
            tdesc.Size = ImageData::GetMipLevelsSize(format, ImageSize(tdesc.Tex.width, tdesc.Tex.height), MipLevels);
        }

        tdesc.pTexData = (UByte*)pmanager->pMemManager->Alloc(tdesc.Size, (tdesc.Tex.pitch == 0) ? 128 : 64, Memory_PS3_RSX);
        if (UPInt(tdesc.pTexData) == ~UPInt(0))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            // Texture creation failed, release all textures and fail.
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }

        tdesc.Tex.location = CELL_GCM_LOCATION_LOCAL;
        cellGcmAddressToOffset(tdesc.pTexData, &tdesc.Tex.offset);
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
    return Render::Texture::Initialize();
}

bool Texture::Initialize(const CellGcmTexture& tex)
{
    TextureManager* pmanager = GetManager();
    TextureFormat::Mapping* pmapping;

    pTextures[0].Tex = tex;
    MipLevels = tex.mipmap;

    // If an image is provided, try to obtain the texture format from the image.
    pFormat = 0;
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->NativeFormat == tex.format )
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
        ImgSize = ImageSize(pTextures[0].Tex.width, pTextures[0].Tex.height);

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat format, 
                                                ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_DEBUG_ASSERT(!rescale, "PS3 does not support rescale (it should not be necessary).");
    SF_UNUSED4(rescale, swMipGen, rescaleType, rescaleBuffFromat);
    if (swMipGen && !(format == Image_R8G8B8A8 || format == Image_B8G8R8A8 || format == Image_A8))
        convert = true;
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL*)
{
    cellGcmSetInvalidateTextureCache(GetManager()->GcmCtx, CELL_GCM_INVALIDATE_TEXTURE);
    return true;
}

bool Texture::UpdateStagingData(Render::RenderTargetData*)
{
    cellGcmSetInvalidateTextureCache(GetManager()->GcmCtx, CELL_GCM_INVALIDATE_TEXTURE);
    return true;
}

void Texture::ReleaseHWTextures(bool)
{
    Render::Texture::ReleaseHWTextures();
    TextureManager* pmanager = GetManager();

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        if (pTextures[itex].pTexData)
            pmanager->pMemManager->Free(pTextures[itex].pTexData, pTextures[itex].Size);
        pTextures[itex].pTexData = 0;
        pTextures[itex].Size = 0;
    }
}

// Applies a texture to device starting at pstageIndex, advances index
void    Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
	Render::Texture::ApplyTexture(stageIndex, fm);

    int address = (fm.GetWrapMode() == Wrap_Clamp ? CELL_GCM_TEXTURE_CLAMP_TO_EDGE : CELL_GCM_TEXTURE_WRAP);
    int minfilter = (fm.GetSampleMode() == Sample_Point ? CELL_GCM_TEXTURE_NEAREST :
                     (MipLevels > 1 ? CELL_GCM_TEXTURE_LINEAR_LINEAR : CELL_GCM_TEXTURE_LINEAR));
    int magfilter = (fm.GetSampleMode() == Sample_Point ? CELL_GCM_TEXTURE_NEAREST : CELL_GCM_TEXTURE_LINEAR);

    CellGcmContextData* GcmCtx = GetManager()->GcmCtx;

    for (unsigned i = 0; i < TextureCount; i++, stageIndex++)
    {
        cellGcmSetTexture(GcmCtx, stageIndex, &pTextures[i].Tex);

        cellGcmSetTextureAddress(GcmCtx, stageIndex, address, address, CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
                                 CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_NEVER, 0);
        cellGcmSetTextureControl(GcmCtx, stageIndex, CELL_GCM_TRUE, 0, 12 << 8, CELL_GCM_TEXTURE_MAX_ANISO_1);

        // Lod bias is 1:4:8 signed fixed point; -0.5 = 0x1f80
        cellGcmSetTextureFilter(GcmCtx, stageIndex, MipLevels > 1 ? 0x1f80 : 0, minfilter, magfilter,
                                CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    }
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
        dplane.pData += desc.DestRect.y1 * dplane.Pitch +
                        desc.DestRect.x1 * GetTextureFormatMapping()->BytesPerPixel;

        splane.SetSize(desc.DestRect.GetSize());
        dplane.SetSize(desc.DestRect.GetSize());
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                          pFormat->GetScanlineCopyFn(), 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

Scaleform::UPInt Texture::GetBytes( int* memRegion ) const
{
    if (memRegion)
        *memRegion = 1;
    UPInt size = 0;
    for (int i = 0; i < TextureCount; i++)
        size += pTextures[0].Size;
    return size;
}

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pMemManager((PS3::MemoryManager*)pmanager), 
    pSurfaceMemory(0)
{
    // If a MemoryManager isn't provided, assume that the TextureManager's MemoryManager should be used.
    if (!pMemManager)
    {
        PS3::TextureManager* manager = (PS3::TextureManager*)GetTextureManager();
        pMemManager = (PS3::MemoryManager*)(manager->GetMemoryManager());
    }
    memset(&Surface, 0, sizeof(CellGcmSurface));
}

DepthStencilSurface::~DepthStencilSurface()
{
    if ( pSurfaceMemory && pMemManager)
    {
        UPInt surfaceSize = Surface.depthPitch * Surface.width;
        pMemManager->Free(pSurfaceMemory, surfaceSize);
    }
}

bool DepthStencilSurface::Initialize()
{
    if ( !pMemManager )
        return false;

    UPInt surfaceSize = Surface.depthPitch * Surface.width;
    pSurfaceMemory = pMemManager->Alloc(surfaceSize, 64, Memory_PS3_RSX);
    if (UPInt(pSurfaceMemory) == ~UPInt(0))
    {
        State = Texture::State_InitFailed;
    }
    else
    {
        State = Texture::State_Valid;
        cellGcmAddressToOffset(pSurfaceMemory, &Surface.depthOffset);
    }
    return State == Texture::State_Valid;
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

    Texture* ps3Texture  = reinterpret_cast<Texture*>(ptexture);
    pTexture             = ptexture;
    StartMipLevel        = mipLevel;
    LevelCount           = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = ps3Texture->pTextures[itex];
		ImagePlane              plane(tdesc.Tex.width, tdesc.Tex.height, tdesc.Pitch);
        UByte*                  pdata = tdesc.pTexData;

        for(unsigned i = 0; i < StartMipLevel; i++)
        {
            pdata += plane.Pitch * plane.Height;
            plane.SetNextMipSize();
        }

        for (unsigned level = 0; level < levelCount; level++)
        {
            plane.pData = pdata;
            plane.DataSize = plane.Height * plane.Pitch;
            Data.SetPlane(level * textureCount + itex, plane);

            // Prepare for next level.
            pdata += plane.DataSize;
            plane.SetNextMipSize();
        }
    }

    pTexture->pMap = this;
    return true;
}

// ***** TextureManager

TextureManager::TextureManager(CellGcmContextData* gcm, Render::MemoryManager* pmm, RenderSync* rsync, TextureCache* texCache) : 
    Render::TextureManager(0, 0, texCache),
    GcmCtx(gcm),
    pMemManager(pmm),
    RSync(rsync)
{
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
    { Image_PS3_R8G8B8A8_SZ, Image_None,     TEX(A8R8G8B8) | TEX(SZ),           REMAP_RGBA_2_ARGB, 4, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },
    { Image_PS3_A8_SZ,       Image_None,     TEX(B8)       | TEX(SZ),           REMAP_A,           1, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    // Used for render-targets.
    { Image_PS3_A8R8G8B8,    Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_NONE,        1, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    { Image_R8G8B8A8,        Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_RGBA_2_ARGB, 4, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,        Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_BGRA_2_ARGB, 4, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    { Image_R8G8B8,          Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_RGBA_2_ARGB, 4, &Image_CopyScanline24_Extend_RGB_RGBA,    &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,          Image_R8G8B8A8, TEX(A8R8G8B8) | TEX(LN),           REMAP_BGRA_2_ARGB, 4, &Image_CopyScanline24_Extend_RGB_RGBA,    &Image_CopyScanline32_Retract_RGBA_RGB },

    { Image_A8,              Image_A8,       TEX(B8)       | TEX(LN),           REMAP_A,           1, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault },

    { Image_PS3_DXT1,        Image_DXT1,     TEX(COMPRESSED_DXT1)  | TEX(SZ),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
    { Image_PS3_DXT3,        Image_DXT3,     TEX(COMPRESSED_DXT23) | TEX(SZ),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
    { Image_PS3_DXT5,        Image_DXT5,     TEX(COMPRESSED_DXT45) | TEX(SZ),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
                                                                                           
    { Image_DXT1,            Image_DXT1,     TEX(COMPRESSED_DXT1)  | TEX(LN),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
    { Image_DXT3,            Image_DXT3,     TEX(COMPRESSED_DXT23) | TEX(LN),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
    { Image_DXT5,            Image_DXT5,     TEX(COMPRESSED_DXT45) | TEX(LN),   REMAP_NONE,        0, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
                                                                                           
    { Image_Y8_U2_V2,        Image_A8,       TEX(B8) | TEX(LN),                 REMAP_A,           1, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
    { Image_Y8_U2_V2_A8,     Image_A8,       TEX(B8) | TEX(LN),                 REMAP_A,           1, &Image::CopyScanlineDefault,          &Image::CopyScanlineDefault  },
                                                                                           
    { Image_None,            Image_None,     0,                                 0,                 0, 0 }
};

#undef REMAP_RGBA_2_ARGB
#undef REMAP_BGRA_2_ARGB
#undef REMAP_A
#undef TEX

void TextureManager::initTextureFormats()
{
    TextureFormat::Mapping* pmapping;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping);
        TextureFormats.PushBack(tf);
    }
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

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(const CellGcmTexture& tex, ImageSize size, ImageBase * pimage)
{
    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, tex, size, pimage);
    return postCreateTexture(ptexture, 0);
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

    // Fill out some parameters required for allocation.
    pdss->Surface.width       = size.Width;
    pdss->Surface.height      = size.Height;
    pdss->Surface.depthFormat = CELL_GCM_SURFACE_Z24S8;
    pdss->Surface.depthPitch  = cellGcmGetTiledPitchSize(size.Width * 4);

    pdss->Initialize();
    return pdss;
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const CellGcmSurface& surface)
{
    // NOTE: MemManager is not passed to the new surface. Since its memory is already allocated, 
    // the allocator must deallocate the RSX memory; as it may not be the MemManager used by this TextureManager.
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, 0, ImageSize(surface.width, surface.height) );
    memcpy(&pdss->Surface, &surface, sizeof pdss->Surface);
    pdss->State = Texture::State_Valid;
    return pdss;
}

}}};  // namespace Scaleform::Render::PS3

