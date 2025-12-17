/**************************************************************************

Filename    :   WiiU_Texture.cpp
Content     :   WiiU Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "WiiU_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_Debug.h"


namespace Scaleform { namespace Render { namespace WiiU {

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
    for (int i = 0; i < TextureCount; i++)
    {
        ImageSize size = ImageData::GetFormatPlaneSize(GetTextureFormatMapping()->Format, ImgSize, i);

        GX2Texture& tex = pTextures[i].Tex;

        tex.surface.width = size.Width;
        tex.surface.height = size.Height;
        tex.surface.depth = 1;
        tex.surface.format = GetTextureFormatMapping()->NativeFormat;
        tex.surface.dim = GX2_SURFACE_DIM_2D;
        tex.surface.numMips = MipLevels;
        tex.surface.aa = GX2_AA_MODE_1X;
        tex.surface.use = Use & ImageUse_RenderTarget ? GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE : GX2_SURFACE_USE_TEXTURE;

        if (Use & ImageUse_RenderTarget && !(Use & ImageUse_MapRenderThread))
            tex.surface.tileMode = GX2_TILE_MODE_DEFAULT;
        else
            tex.surface.tileMode = GetTextureFormatMapping()->NativeSwizzle;
        tex.viewFirstMip = 0;
        tex.viewNumMips = MipLevels;
        tex.viewFirstSlice = 0;
        tex.viewNumSlices = 1;
        tex.compSel = GetTextureFormatMapping()->NativeRemap;

        GX2CalcSurfaceSizeAndAlignment(&tex.surface);
        GX2InitTextureRegs(&tex);
    }
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, const GX2Texture& tex, const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0)
{
    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    pTextures[0].pTexBaseData = 0;
    pTextures[0].pTexMipData = 0;
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
                     ImageSize_MipLevelCount(ImageSize(pTextures[itex].Tex.surface.width, pTextures[itex].Tex.surface.height)));
        MipLevels = (UByte)allocMipLevels;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        tdesc.pTexBaseData = (UByte*)pmanager->pMemManager->Alloc(tdesc.Tex.surface.imageSize, tdesc.Tex.surface.alignment, Memory_Normal);
        if (!tdesc.pTexBaseData)
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            // Texture creation failed, release all textures and fail.
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }

        if (tdesc.Tex.surface.mipSize)
        {
            tdesc.pTexMipData = (UByte*)pmanager->pMemManager->Alloc(tdesc.Tex.surface.mipSize, tdesc.Tex.surface.alignment, Memory_Normal);
            if (!tdesc.pTexMipData)
            {
                SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
                // Texture creation failed, release all textures and fail.
                ReleaseHWTextures();
                State = State_InitFailed;
                return false;
            }
        }

        GX2InitTexturePtrs(&tdesc.Tex, tdesc.pTexBaseData, tdesc.pTexMipData);
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

bool Texture::Initialize(const GX2Texture& tex)
{
    TextureManager* pmanager = GetManager();
    TextureFormat::Mapping* pmapping;

    pTextures[0].Tex = tex;
    MipLevels = tex.surface.numMips;

    // If an image is provided, try to obtain the texture format from the image.
    pFormat = 0;
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->NativeFormat == tex.surface.format )
            {
                pFormat = (TextureFormat*)pmanager->getTextureFormat(pmapping->Format);
                break;
            }
        }
    }

    // Could not determine the format.
    if ( !pFormat)
    {
        SF_DEBUG_ERROR(1, "Texture::Initialize - couldn't determine ImageFormat of user supplied texture, using RGBA.");
        pFormat = (TextureFormat*)pmanager->getTextureFormat(TextureFormatMapping->Format);
    }

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = ImageSize(pTextures[0].Tex.surface.width, pTextures[0].Tex.surface.height);

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat format, 
    ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_DEBUG_ASSERT(!rescale, "WiiU does not support rescale (it should not be necessary).");
    SF_UNUSED4(rescale, swMipGen, rescaleType, rescaleBuffFromat);
    if (swMipGen && !(format == Image_R8G8B8A8 || format == Image_B8G8R8A8 || format == Image_A8))
        convert = true;
}   

void Texture::ReleaseHWTextures(bool)
{
    Render::Texture::ReleaseHWTextures();
    TextureManager* pmanager = GetManager();

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        if (pTextures[itex].pTexBaseData)
            pmanager->pMemManager->Free(pTextures[itex].pTexBaseData, pTextures[itex].Tex.surface.imageSize);
        pTextures[itex].pTexBaseData = 0;

        if (pTextures[itex].pTexMipData)
            pmanager->pMemManager->Free(pTextures[itex].pTexMipData, pTextures[itex].Tex.surface.mipSize);
        pTextures[itex].pTexMipData = 0;
    }
}

void Texture::ApplyTexture(unsigned stage, const ImageFillMode& fillMode)
{
    Render::Texture::ApplyTexture(stage, fillMode);
    for (unsigned plane = 0; plane < ImageData::GetFormatPlaneCount(GetImageFormat()); plane++)
    {
        int stageIndex = stage + plane;
        UByte fill = fillMode.Fill;
        if (pTextures[plane].Tex.surface.numMips > 1)
            fill |= Mipmap_2D;

        GX2SetPixelTexture(&pTextures[plane].Tex, stageIndex);
        GX2SetPixelSampler(&GetManager()->Samplers[fill], stageIndex);
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
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);
        
        pMap->Data.GetPlane(desc.PlaneIndex, &dplane);
        dplane.pData += desc.DestRect.y1 * dplane.Pitch +
                        desc.DestRect.x1 * pmapping->BytesPerPixel;

        splane.SetSize(desc.DestRect.GetSize());
        dplane.SetSize(desc.DestRect.GetSize());
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                          pmapping->CopyFunc, 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

void Texture::SyncTextureData(Render::RenderTargetData* prt)
{
    // Flush any pending draw calls
    GX2Flush();
    
    // Flush cache in case GPU or CPU touched dynamic data
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    unsigned size = Texture0.Tex.surface.height * Texture0.Tex.surface.pitch * pmapping->BytesPerPixel;
    GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, Texture0.pTexBaseData, size);
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData* prt, Render::HAL*)
{
    SyncTextureData(prt);
    return true;
}

bool Texture::UpdateStagingData(Render::RenderTargetData* prt)
{
    SyncTextureData(prt);
    return true;
}

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pMemManager(pmanager), 
    pSurfaceMemory(0)
{
    // If a MemoryManager isn't provided, assume that the TextureManager's MemoryManager should be used.
    if (!pMemManager)
        pMemManager = ((TextureManager*)pManagerLocks->pManager)->GetMemoryManager();
    memset(&DepthBuffer, 0, sizeof(GX2DepthBuffer));
}

DepthStencilSurface::~DepthStencilSurface()
{
    if ( pSurfaceMemory && pMemManager)
        pMemManager->Free(pSurfaceMemory, DepthBuffer.surface.imageSize);
}

bool DepthStencilSurface::Initialize()
{
    if ( !pMemManager )
        return false;

    pSurfaceMemory = pMemManager->Alloc(DepthBuffer.surface.imageSize, DepthBuffer.surface.alignment, Memory_Normal);
    if ( !pSurfaceMemory )
    {
        State = Texture::State_InitFailed;
    }
    else
    {
        State = Texture::State_Valid;
        GX2InitDepthBufferPtr(&DepthBuffer, pSurfaceMemory);
    }
    return State == Texture::State_Valid;
}

// ***** MappedTexture

bool MappedTexture::Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(!IsMapped());
    SF_ASSERT((mipLevel + levelCount) <= ptexture->MipLevels);
    SF_ASSERT(mipLevel || levelCount == 1);

    // Initialize Data as efficiently as possible.
    if (levelCount <= PlaneReserveSize)
        Data.Initialize(ptexture->GetImageFormat(), levelCount, Planes, ptexture->GetPlaneCount(), true);
    else if (!Data.Initialize(ptexture->GetImageFormat(), levelCount, true))
        return false;

    Texture* wiiuTexture = reinterpret_cast<Texture*>(ptexture);
    pTexture             = ptexture;
    StartMipLevel        = mipLevel;
    LevelCount           = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = wiiuTexture->pTextures[itex];
        ImagePlane              plane(tdesc.Tex.surface.width, tdesc.Tex.surface.height, tdesc.Tex.surface.pitch * wiiuTexture->GetTextureFormatMapping()->BytesPerPixel);

        for(unsigned i = 1; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        UByte*                  pdata = StartMipLevel ? tdesc.pTexMipData + tdesc.Tex.surface.mipOffset[StartMipLevel-1] : tdesc.pTexBaseData;

        for (unsigned level = 0; level < levelCount; level++)
        {
            plane.pData = pdata;
            plane.DataSize = plane.Height * plane.Pitch;
            Data.SetPlane(level * textureCount + itex, plane);

            // Prepare for next level.
            pdata = tdesc.pTexMipData + tdesc.Tex.surface.mipOffset[level-1];
            pdata += plane.DataSize;
            plane.SetNextMipSize();
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool)
{
    unsigned textureCount = pTexture->TextureCount;
    for (unsigned itex = 0; itex < textureCount; itex++)
        GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, Planes[itex].pData, Planes[itex].DataSize);

    MappedTextureBase::Unmap();
}


// ***** TextureManager

TextureManager::TextureManager(Render::MemoryManager* pmm, RenderSync* rsync, TextureCache* texCache) : 
    Render::TextureManager(texCache),
    pMemManager(pmm),
    RSync(rsync)
{
    initTextureFormats();

    GX2InitSampler        (&Samplers[Wrap_Clamp  | Sample_Point  | Mipmap_None], GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_POINT);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Clamp  | Sample_Point  | Mipmap_None], GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_NO_MIP);
    GX2InitSampler        (&Samplers[Wrap_Repeat | Sample_Point  | Mipmap_None], GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_POINT);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Repeat | Sample_Point  | Mipmap_None], GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_NO_MIP);
    GX2InitSampler        (&Samplers[Wrap_Clamp  | Sample_Linear | Mipmap_None], GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Clamp  | Sample_Linear | Mipmap_None], GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_NO_MIP);
    GX2InitSampler        (&Samplers[Wrap_Repeat | Sample_Linear | Mipmap_None], GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Repeat | Sample_Linear | Mipmap_None], GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_NO_MIP);

    GX2InitSampler        (&Samplers[Wrap_Clamp  | Sample_Point  | Mipmap_2D],   GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_POINT);
    GX2InitSampler        (&Samplers[Wrap_Repeat | Sample_Point  | Mipmap_2D],   GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_POINT);
    GX2InitSampler        (&Samplers[Wrap_Clamp  | Sample_Linear | Mipmap_2D],   GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Clamp  | Sample_Linear | Mipmap_2D],   GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_LINEAR);
    GX2InitSamplerLOD     (&Samplers[Wrap_Clamp  | Sample_Linear | Mipmap_2D],   0, 15.999f, -0.5f);
    GX2InitSampler        (&Samplers[Wrap_Repeat | Sample_Linear | Mipmap_2D],   GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitSamplerZMFilter(&Samplers[Wrap_Repeat | Sample_Linear | Mipmap_2D],   GX2_TEX_Z_FILTER_POINT, GX2_TEX_MIP_FILTER_LINEAR);
    GX2InitSamplerLOD     (&Samplers[Wrap_Repeat | Sample_Linear | Mipmap_2D],   0, 15.999f, -0.5f);

}

TextureManager::~TextureManager()
{   
    Mutex::Locker lock(&pLocks->TextureMutex);

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    pLocks->pManager = 0;
}

// ***** WiiU Format mapping and conversion functions

#define TEX(f)     GX2_SURFACE_FORMAT_##f##_UNORM
#define CS(f)      GX2_COMPONENT_##f
#define SZ_NONE    GX2_TILE_MODE_LINEAR_ALIGNED
#define SZ_TILE    GX2_TILE_MODE_DEFAULT
#define REMAP_RGBA GX2_COMP_SEL_XYZW
#define REMAP_BGRA GX2_GET_COMP_SEL( CS(Z_B), CS(Y_G), CS(X_R), CS(W_A))
#define REMAP_A    GX2_GET_COMP_SEL( CS(C_1), CS(C_1), CS(C_1), CS(X_R))

// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

TextureFormat::Mapping TextureFormatMapping[] = 
{
    { Image_R8G8B8A8,    Image_R8G8B8A8,   TEX(TCS_R8_G8_B8_A8), SZ_NONE, REMAP_RGBA, 4, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,    Image_B8G8R8A8,   TEX(TCS_R8_G8_B8_A8), SZ_NONE, REMAP_BGRA, 4, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

    { Image_R8G8B8,      Image_R8G8B8A8,   TEX(TCS_R8_G8_B8_A8), SZ_NONE, REMAP_RGBA, 4, &Image_CopyScanline24_Extend_RGB_RGBA, &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,      Image_B8G8R8A8,   TEX(TCS_R8_G8_B8_A8), SZ_NONE, REMAP_BGRA, 4, &Image_CopyScanline24_Extend_RGB_RGBA, &Image_CopyScanline32_Retract_RGBA_RGB },

    { Image_A8,          Image_A8,         TEX(TC_R8),           SZ_NONE, REMAP_A,    1, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

    { Image_Y8_U2_V2,    Image_A8,         TEX(TC_R8),           SZ_NONE, REMAP_A,    1, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8, Image_A8,         TEX(TC_R8),           SZ_NONE, REMAP_A,    1, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

	{ Image_WiiU_DXT1,   Image_DXT1,       TEX(T_BC1),           SZ_TILE, REMAP_RGBA, 0, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },
	{ Image_WiiU_DXT3,   Image_DXT3,       TEX(T_BC2),           SZ_TILE, REMAP_RGBA, 0, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },
	{ Image_WiiU_DXT5,   Image_DXT5,       TEX(T_BC3),           SZ_TILE, REMAP_RGBA, 0, &Image::CopyScanlineDefault,           &Image::CopyScanlineDefault },

    { Image_None,        Image_None,       TEX(TC_R8),           SZ_NONE, 0,          0, 0,                                     0 }
};

#undef TEX
#undef CS

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

Render::Texture* TextureManager::CreateTexture(const GX2Texture& tex, ImageSize size, ImageBase * pimage)
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
    GX2InitDepthBuffer(&pdss->DepthBuffer, size.Width, size.Height, GX2_SURFACE_FORMAT_D_D24_S8_UNORM, GX2_AA_MODE_1X);
    GX2Invalidate(GX2_INVALIDATE_CPU, pdss->DepthBuffer.surface.imagePtr, pdss->DepthBuffer.surface.imageSize);

    pdss->Initialize();
    return pdss;
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const GX2DepthBuffer& depth)
{
    // NOTE: MemManager is not passed to the new surface. Since its memory is already allocated, 
    // the allocator must deallocate the RSX memory; as it may not be the MemManager used by this TextureManager.
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, 0, ImageSize(depth.surface.width, depth.surface.height) );
    memcpy(&pdss->DepthBuffer, &depth, sizeof pdss->DepthBuffer);
    pdss->State = Texture::State_Valid;
    return pdss;
}

}}};  // namespace Scaleform::Render::WiiU

