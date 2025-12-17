/**************************************************************************

Filename    :   X360_Texture.cpp
Content     :   X360 Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Render/X360/X360_Texture.h"
#include "Render/X360/X360_HAL.h"
#include "Render/Render_TextureUtil.h"
#include "Render/X360/X360_Types.h"
#include <xgraphics.h>

namespace Scaleform { namespace Render { namespace X360 {

extern TextureFormat::Mapping X360TextureFormatMapping[];

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage, Render::MemoryManager* pmemManager) : 
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat), 
    pMemManager(pmemManager)
{
    SF_DEBUG_ASSERT(pFormat, "Creating texture with NULL format.");
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
        ImageSize sz = ImageData::GetFormatPlaneSize(pformat->GetImageFormat(), size, i );

        unsigned size1, sizeMips;
        pTextures[i].Tex = (D3DTexture*)SF_ALLOC(sizeof D3DTexture,Stat_Default_Mem);

        XGSetTextureHeaderEx(   sz.Width,                       // Width
                                sz.Height,                      // Height
                                mipLevels,                      // Levels
                                0,                              // Usage
                                GetTextureFormatMapping()->D3DFormat,   // Format
                                0,                              // ExpBias
                                XGHEADEREX_NONPACKED,           // Flags
                                0,                              // BaseOffset
                                XGHEADER_CONTIGUOUS_MIP_OFFSET, // MipOffset
                                0,                              // Pitch
                                pTextures[i].Tex,               // TexturePtr
                                &size1,                         // BaseSize
                                &sizeMips);                     // MipSize
    }
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, IDirect3DTexture9* ptexture, const ImageSize& size, 
                 ImageBase* pimage, Render::MemoryManager* manager, bool useRefCount) :
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0), 
    pMemManager(manager)
{
    TextureFlags |= TF_UserAlloc;
    TextureFlags |= (useRefCount ? 0 : TF_NoRefCount);
    if (ptexture && (TextureFlags & TF_NoRefCount) == 0)
        ptexture->AddRef();
    pTextures = &Texture0;
    pTextures[0].Tex = ptexture;
    pTextures[0].Size = size;
    pTextures[0].pTexData = 0;
    pTextures[0].Surface  = 0;
}

Texture::~Texture()
{
    //  pImage must be null, since ImageLost had to be called externally.
    SF_ASSERT(pImage == 0);
    
    Mutex::Locker  lock(&pManagerLocks->TextureMutex);
 
    if (State == State_Valid)
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

bool IsD3DFormatRescaleCompatible(D3DFORMAT format,
                                  ImageFormat *ptargetImageFormat,
                                  ResizeImageType* presizeType)
{
    switch(format)
    {
        case D3DFMT_A8R8G8B8:
            *ptargetImageFormat = Image_B8G8R8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case D3DFMT_A8B8G8R8:
            *ptargetImageFormat = Image_R8G8B8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case D3DFMT_A8:
            *ptargetImageFormat = Image_A8;
            *presizeType        = ResizeGray;
            return true;
        default:
            break;
    }
    return false;
}


bool IsD3DFormatMipGenCompatible(D3DFORMAT format)
{
    switch(format)
    {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_A8:
        return true;
    default:
        break;
    }
    return false;
}


bool Texture::Initialize()
{    
    SF_AMP_SCOPE_TIMER(GetManager()->RenderThreadId == GetCurrentThreadId() ? AmpServer::GetInstance().GetDisplayStats() : NULL, __FUNCTION__, Amp_Profile_Level_Medium);

    if ( TextureFlags & TF_UserAlloc )
    {
        return Initialize(pTextures[0].Tex);
    }

    bool            resize  = false;
    ImageFormat     format  = GetImageFormat();
    unsigned        itex;

    // Determine sizes of textures.
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        tdesc.Size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
    }

    if (resize && (Use & ImageUse_Wrap))
    {        
        if (ImageData::IsFormatCompressed(format))
        {        
            SF_DEBUG_ERROR(1, 
                "CreateTexture failed - Can't rescale compressed Wrappable image to Pow2");
            State = State_InitFailed;
            return false;
        }
        TextureFlags |= TF_Rescale;
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
                                          ImageSize_MipLevelCount(pTextures[itex].Size));
            MipLevels = (UByte)allocMipLevels;
        }

    // MA: Gamebryo-compatibility work-around from GFx 3.x, TBD whether still necessary.
    // For some reason we need to specify levelsNeeded-1, otherwise surface accesses
    // may crash (when running with Gamebryo). So, 256x256 texture has levelCount of 8 (not 9).
    // There is no problem with dropping one level unless user intends to Map it (not typical).
    if ((allocMipLevels > 1) && ((Use & ImageUse_Map_Mask) == 0))
    {
        allocMipLevels--;
        MipLevels--;
    }

    // TODO: support for packed mip chain allocations.
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        XGTEXTURE_DESC desc;
        XGGetTextureDesc(tdesc.Tex, 0, &desc);
        UINT baseSize, mipSize;
        XGGetTextureLayout(tdesc.Tex, NULL, &baseSize, NULL, NULL, 0, NULL, &mipSize, NULL, NULL, 0 );

        tdesc.pTexData = (UByte*)pMemManager->Alloc(baseSize + mipSize, 4096, Memory_X360_Physical);
        if (!tdesc.pTexData)
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            // Texture creation failed, release all textures and fail.
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }
        XGOffsetResourceAddress(tdesc.Tex, tdesc.pTexData);

        // Render target allocation.
        if ( Use & ImageUse_RenderTarget )
        {
            pTextures[itex].Surface = (D3DSurface*)SF_ALLOC(sizeof D3DSurface,Stat_Default_Mem);

            D3DSURFACE_PARAMETERS surfParams;
            memset(&surfParams, 0, sizeof surfParams);
            unsigned surfSize = XGSurfaceSize( tdesc.Size.Width, tdesc.Size.Height, GetTextureFormatMapping()->D3DFormat, D3DMULTISAMPLE_NONE );
            surfParams.Base = (DWORD)pMemManager->Alloc(surfSize, 0, Memory_X360_EDRAM);
            surfParams.HierarchicalZBase = 0xFFFFFFFF;
            XGSetSurfaceHeader( tdesc.Size.Width, tdesc.Size.Height, GetTextureFormatMapping()->D3DFormat, D3DMULTISAMPLE_NONE, 
                                &surfParams, pTextures[itex].Surface, 0 );
            XGOffsetSurfaceAddress(pTextures[itex].Surface, surfParams.Base, 0);
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
    return Render::Texture::Initialize();
}

bool Texture::Initialize(IDirect3DTexture9* ptexture)
{
    if ( !ptexture )
        return false;

    // If this is called explicitly (ie. not after the constructor), and the texture is being replaced,
    // A reference needs to be added to the new texture.
    if ( pTextures[0].Tex != ptexture )
    {
        ReleaseHWTextures();
        pTextures[0].Tex = ptexture;
        if ( (TextureFlags & TF_NoRefCount) == 0 )
            pTextures[0].Tex->AddRef();
    }

    MipLevels = (UByte)ptexture->GetLevelCount();

    D3DSURFACE_DESC surfaceDesc;
    ptexture->GetLevelDesc(0, &surfaceDesc);

    pFormat = 0;
    TextureManager* pmanager = GetManager();

    // If an image is provided, try to obtain the texture format from the image.
    if ( pImage )
        pFormat = pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        TextureFormat::Mapping* pmapping;
            for (pmapping = X360TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->D3DFormat == surfaceDesc.Format )
            {
                pFormat = pmanager->getTextureFormat(pmapping->Format);
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
    pTextures[0].Size.SetSize(surfaceDesc.Width, surfaceDesc.Height);
    HWTextureDesc& tdesc = pTextures[0];
    XGTEXTURE_DESC desc;
    XGGetTextureDesc(tdesc.Tex, 0, &desc);
    XGGetTextureLayout(tdesc.Tex, (UINT*)&pTextures[0].pTexData, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = pTextures[0].Size;

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat format, 
                                                ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_DEBUG_ASSERT(!rescale, "X360 does not support rescale (it should not be necessary).");
    SF_UNUSED4(rescale, swMipGen, rescaleType, rescaleBuffFromat);
    if (swMipGen && !(format == Image_R8G8B8A8 || format == Image_B8G8R8A8 || format == Image_A8))
        convert = true;
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData* pdata, Render::HAL* phal)
{
    RenderTargetData* phd = (RenderTargetData*)pdata;
    if (!phd || !phd->pResolveTarget || !phd->pRenderSurface)
        return false;

    // Need to copy back the results of the texture update to the surface.
    Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
    phal->DrawableCopyback(this, mvp, Matrix2F::Identity);
    return true;
}

bool Texture::UpdateStagingData(Render::RenderTargetData* pdata)
{
    RenderTargetData* phd = (RenderTargetData*)pdata;
    if (!phd || !phd->pResolveTarget)
        return false;
    TextureManager* pmgr = GetManager();

    Ptr<IDirect3DSurface9> pcurrentRT;
    pmgr->pDevice->GetRenderTarget(0, &pcurrentRT.GetRawRef());
    if (pcurrentRT == phd->pRenderSurface)
        pmgr->pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, 0, phd->pResolveTarget, 0, 0, 0, 0, 1.0f, 0, 0 );        
    return true;
}

void Texture::ReleaseHWTextures(bool)
{
    Render::Texture::ReleaseHWTextures();
    TextureManager* pmanager = GetManager();
    bool useKillList = pmanager->RenderThreadId && (GetCurrentThreadId() != pmanager->RenderThreadId);    

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        // User allocated textures go in a separate list, as our memory manager may not have allocated the
        // memory for the texture. If the user needs a specific free mechanism, they should hold the final
        // ref-count to the texture and free it themselves; except if the RefCount flag is not set, in which
        // case the texture is not released or destroyed, it is up to the user to manage its lifetime completely.
        if (( TextureFlags & TF_UserAlloc ) == 0 )
            pmanager->D3DTextureKillList.PushBack(pTextures[itex].Tex);
        else if ( (TextureFlags & TF_NoRefCount) == 0 )
            pmanager->D3DTextureReleaseList.PushBack(pTextures[itex].Tex);

        if ( Use & ImageUse_RenderTarget )
        {
            SF_ASSERT(pTextures[itex].Surface);
            pmanager->D3DSurfaceKillList.PushBack(pTextures[itex].Surface);
        }
        pTextures[itex].pTexData = NULL;
    }

    if ( !useKillList )
        pmanager->processTextureKillList();
}

static D3DTEXTUREFILTERTYPE Texture_FilterLookup[Sample_Count] = 
{
    D3DTEXF_POINT,    
    D3DTEXF_LINEAR
};
static D3DTEXTUREADDRESS  Texture_AddressLookup[Wrap_Count] = 
{
    D3DTADDRESS_WRAP,
    D3DTADDRESS_CLAMP
};

// Applies a texture to device starting at pstageIndex, advances index
void    Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    Render::Texture::ApplyTexture(stageIndex, fm);

    D3DTEXTUREFILTERTYPE filter = 
        Texture_FilterLookup[fm.GetSampleMode() >> Sample_Shift];
    D3DTEXTUREADDRESS    address = Texture_AddressLookup[fm.GetWrapMode()];

    for (unsigned i = 0; i < TextureCount; i++, stageIndex++)
        GetManager()->SetSamplerState(stageIndex, pTextures[i].Tex, filter, address);
}


bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    SF_AMP_SCOPE_TIMER(GetManager()->RenderThreadId == GetCurrentThreadId() ? AmpServer::GetInstance().GetDisplayStats() : NULL, __FUNCTION__, Amp_Profile_Level_Medium);
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
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex, GetTextureFormatMapping()->CopyFunc, 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pDepthStencilSurface(0)
{
}

DepthStencilSurface::~DepthStencilSurface()
{
    if (pDepthStencilSurface)
    {
        // Need to unset the DepthStencilSurface on the device because this may destroy it.
        TextureManager* pmanager = (TextureManager*)pManagerLocks->pManager;
        pmanager->pDevice->SetDepthStencilSurface(0);
        pDepthStencilSurface->Release();
    }
}

bool DepthStencilSurface::Initialize()
{
    TextureManager* pmanager= (TextureManager*)GetTextureManager();

    if ( FAILED(pmanager->pDevice->CreateDepthStencilSurface(Size.Width, Size.Height, 
        D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, FALSE, &pDepthStencilSurface, 0 )) )
    {
        State = Texture::State_InitFailed;
    }
    else
    {
        State = Texture::State_Valid;
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

    Texture* x360Texture = reinterpret_cast<Texture*>(ptexture);
    pTexture             = ptexture;
    StartMipLevel        = mipLevel;
    LevelCount           = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = x360Texture->pTextures[itex];
        UByte*                  pdata = tdesc.pTexData;

        for (unsigned level = 0; level < levelCount; level++)
        {
            XGTEXTURE_DESC          desc;
            XGGetTextureDesc(tdesc.Tex, StartMipLevel + level, &desc);

            ImagePlane              plane(desc.Width, desc.Height, desc.RowPitch);
            plane.pData = pdata;
            plane.DataSize = desc.SlicePitch;
            Data.SetPlane(level * textureCount + itex, plane);

            // Prepare for next level.
            pdata += desc.SlicePitch;
        }
    }

    pTexture->pMap = this;
    return true;
}

// ***** TextureManager

TextureManager::TextureManager(IDirect3DDevice9* pdevice,
                               Render::MemoryManager* pmemManager,
                               ThreadId renderThreadId, 
                               ThreadCommandQueue* commandQueue,
                               TextureCache* texCache) : 
    Render::TextureManager(renderThreadId, commandQueue, texCache),
    pDevice(pdevice), pMemoryManager(pmemManager)
{
    initTextureFormats();


    for ( int i = 0; i < MaximumStages; i++ )
    {
        AddressMode[i] = D3DTADDRESS_FORCE_DWORD;
        FilterType[i] = D3DTEXF_FORCE_DWORD;
    }
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
	processTextureKillList();

	// Notify all textures
	while (!Textures.IsEmpty())
		Textures.GetFirst()->LoseManager();

	pDevice = 0;
}

// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

TextureFormat::Mapping X360TextureFormatMapping[] = 
{
    // Tiled formats.
    { Image_X360_R8G8B8A8,  D3DFMT_A8R8G8B8,     4, &Image_CopyScanline32_RGBA_ARGB,         &Image_CopyScanline32_RGBA_ARGB },
    
    { Image_X360_A8,        D3DFMT_A8,           1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    { Image_X360_DXT1,      D3DFMT_DXT1,         0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_X360_DXT3,      D3DFMT_DXT3,         0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_X360_DXT5,      D3DFMT_DXT5,         0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    // Non-tiled formats.
    { Image_R8G8B8A8,       D3DFMT_LIN_A8R8G8B8, 4, &Image_CopyScanline32_RGBA_ARGB,         &Image_CopyScanline32_RGBA_ARGB },
    { Image_B8G8R8A8,       D3DFMT_LIN_A8B8G8R8, 4, &Image_CopyScanline32_RGBA_ARGB,         &Image_CopyScanline32_RGBA_ARGB },

    { Image_R8G8B8A8,       D3DFMT_LIN_A8R8G8B8, 4, &Image_CopyScanline32_RGBA_ARGB,         &Image_CopyScanline32_RGBA_ARGB },
    { Image_B8G8R8A8,       D3DFMT_LIN_A8B8G8R8, 4, &Image_CopyScanline32_RGBA_ARGB,         &Image_CopyScanline32_RGBA_ARGB },

    { Image_R8G8B8,         D3DFMT_LIN_X8R8G8B8, 4, &Image_CopyScanline24_Extend_RGB_ARGB,   &Image_CopyScanline32_Retract_ARGB_RGB },
    { Image_B8G8R8,         D3DFMT_LIN_X8B8G8R8, 4, &Image_CopyScanline24_Extend_RGB_ARGB,   &Image_CopyScanline32_Retract_ARGB_RGB },

    { Image_A8,             D3DFMT_LIN_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    { Image_DXT1,           D3DFMT_LIN_DXT1,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_DXT3,           D3DFMT_LIN_DXT3,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_DXT5,           D3DFMT_LIN_DXT5,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    { Image_Y8_U2_V2,       D3DFMT_LIN_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,    D3DFMT_LIN_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    { Image_None,           D3DFMT_UNKNOWN,      0, 0 }
};


void        TextureManager::initTextureFormats()
{
    Ptr<IDirect3D9>                 pd3d9;
    TextureFormat::Mapping*         pmapping;
    D3DDEVICE_CREATION_PARAMETERS   cp;
    D3DDISPLAYMODE                  modeDesc;

    pDevice->GetDisplayMode(0, &modeDesc);
    pDevice->GetCreationParameters(&cp);
    pDevice->GetDirect3D(&pd3d9.GetRawRef());    
  
    for (pmapping = X360TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping, 0);
        if ( SUCCEEDED( pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType, 
            modeDesc.Format, 0, D3DRTYPE_TEXTURE, pmapping->D3DFormat) ))
        {
            TextureFormats.PushBack(tf);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}

void TextureManager::unmapTexture(Render::Texture *ptexture, bool)
{
    Texture* x360Texture  = reinterpret_cast<Texture*>(ptexture);
    for ( int itex = 0; itex < ptexture->TextureCount; ++itex )
    {
        // If the image and the texture have different tile storage, then we must 
        // convert between them when unmapping.
        if ( x360Texture->pImage && 
            (x360Texture->GetTextureFormatMapping()->Format & ImageStorage_Tile) &&
            (x360Texture->pImage->GetFormat() & ImageStorage_Tile) == 0 )
        {
            D3DTexture* ptex = x360Texture->pTextures[itex].Tex;
            unsigned levelCount = ptex->GetLevelCount();
            for ( unsigned level = 0; level < levelCount; ++level )
            {
                XGTEXTURE_DESC          desc;
                XGGetTextureDesc(ptex, level, &desc);

                XGTileTextureLevel(desc.Width, desc.Height, level, 
                    XGGetGpuFormat(x360Texture->GetTextureFormat()->GetD3DFormat()), 
                    XGIsBorderTexture(ptex) ? XGTILE_BORDER : 0 | XGIsPackedTexture(ptex) ? 0 : XGTILE_NONPACKED,
                x360Texture->pTextures[itex].pTexData + XGGetMipLevelOffset(ptex, 0, level), 0, 
                x360Texture->pTextures[itex].pTexData + XGGetMipLevelOffset(ptex, 0, level),
                desc.RowPitch, 0 );
            }

        }

        pDevice->InvalidateResourceGpuCache(x360Texture->pTextures[itex].Tex, 0);
    }

    Render::TextureManager::unmapTexture(ptexture);
}

void    TextureManager::processTextureKillList()
{
	if ( !pDevice)
		return;

    // Unset all the texture (we could possibly use), just in case we're trying to delete one of them.
    for (int i = 0; i < 4; i++ )
        pDevice->SetTexture(i, 0);

    for (unsigned i = 0; i<D3DTextureKillList.GetSize(); i++)
    {
        // This shouldn't be necessary with a safe release method.
        IDirect3DTexture9 * ptexture = D3DTextureKillList[i];
        ptexture->BlockUntilNotBusy();

        // Free the physical memory associated with the texture, and unassociate the data.
        UINT pbase;
        XGGetTextureLayout(ptexture, &pbase, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, 0 );    
        GetMemoryManager()->Free(reinterpret_cast<LPVOID>(pbase), 0);
        XGOffsetResourceAddress(ptexture, NULL);
        SF_FREE(ptexture);
    }

    for (unsigned i = 0; i<D3DTextureReleaseList.GetSize(); i++)
    {
        IDirect3DTexture9 * ptexture = D3DTextureReleaseList[i];
        ptexture->Release();
    }

    for (unsigned i = 0; i<D3DSurfaceKillList.GetSize(); i++)
    {
        if (D3DSurfaceKillList[i]->IsSet(pDevice))
            pDevice->SetRenderTarget(0,0);
        SF_FREE(D3DSurfaceKillList[i]);
    }

    D3DTextureKillList.Clear();
    D3DTextureReleaseList.Clear();
    D3DSurfaceKillList.Clear();
}

void    TextureManager::processInitTextures()
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

void TextureManager::SetSamplerState( unsigned stage, IDirect3DTexture9* ptexture, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address )
{
    if ( AddressMode[stage] != address )
    {
        pDevice->SetSamplerState(stage, D3DSAMP_ADDRESSU, address);
        pDevice->SetSamplerState(stage, D3DSAMP_ADDRESSV, address);
        AddressMode[stage] = address;
    }
    if ( FilterType[stage] != filter )
    {
        pDevice->SetSamplerState(stage, D3DSAMP_MINFILTER, filter );
        pDevice->SetSamplerState(stage, D3DSAMP_MAGFILTER, filter );
        pDevice->SetSamplerState(stage, D3DSAMP_MIPFILTER, filter );
        FilterType[stage] = filter;
    }
    if ( CurrentTextures[stage] != ptexture )
    {
        pDevice->SetTexture(stage, ptexture);
        CurrentTextures[stage] = ptexture;
    }
}

void TextureManager::BeginScene()
{
    for ( int i = 0; i < MaximumStages; i++ )
    {
        AddressMode[i] = D3DTADDRESS_FORCE_DWORD;
        FilterType[i] = D3DTEXF_FORCE_DWORD;
        CurrentTextures[i] = 0;
        pDevice->SetTexture(i, 0);
    }    
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size,
                                               unsigned use, ImageBase* pimage,
                                               Render::MemoryManager* allocManager)
{
    SF_UNUSED(allocManager);

    // If an image format request is made for a texture that is not updateable, and there is a tiled
    // image format, create a texture of that format instead.
    if ( !(use & ImageUse_ReadOnly_Mask ) && mipLevels == 1 )
    {
        switch(format)
        {
        case Image_R8G8B8A8:     format = Image_X360_R8G8B8A8; break;
        case Image_A8:           format = Image_X360_A8; break;
        case Image_DXT1:         format = Image_X360_DXT1; break;
        case Image_DXT3:         format = Image_X360_DXT3; break;
        case Image_DXT5:         format = Image_X360_DXT5; break;
        }
    }

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage, 
                                       allocManager ? allocManager : pMemoryManager);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(IDirect3DTexture9* pd3dtexture, ImageSize imgSize, 
                                               ImageBase* pimage, Render::MemoryManager* manager,
                                               bool useRefCount)
{
    if ( pd3dtexture == 0 )
        return 0;

    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, pd3dtexture, imgSize, pimage, manager, useRefCount);
    return postCreateTexture(ptexture, 0);
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    // ImageUse_InitOnly (ImageUse_NoDataLoss alias) ok while textures are Managed
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
    if (!pDevice)
    {
        SF_DEBUG_ERROR(1, "CreateDepthStencilSurface failed - TextueManager has been Reset");
        return 0;
    }

    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, size);
    return postCreateDepthStencilSurface(pdss);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(IDirect3DSurface9* psurface)
{
    if ( !psurface ) 
        return 0;

    psurface->AddRef();
    D3DSURFACE_DESC desc;
    psurface->GetDesc(&desc);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, ImageSize(desc.Width, desc.Height) );
    pdss->pDepthStencilSurface = psurface;
    pdss->State = Texture::State_Valid;
    return pdss;
}

class ImageSwizzler : public Render::ImageSwizzler
{
public:
    virtual void Initialize(ImageSwizzlerContext& ctx)
    {
        ctx.pPlane = &ctx.pImage->pPlanes[0];
    }
    virtual void CacheScanline(ImageSwizzlerContext& ctx, unsigned y)
    {
        ctx.CachedBlockY = y;
    }

    virtual void SetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x, UInt32 c)
    {
        UINT addr = XGAddress2DTiledOffset(x, ctx.CachedBlockY, ctx.pPlane->Pitch/4, 4) *4;
        SF_DEBUG_WARNING4(addr >= ctx.pPlane->DataSize, "Unexpected pixel out-of-bounds (x=%d, y=%d, offset=%d, max=%d)\n",
            x, ctx.CachedBlockY, addr, ctx.pPlane->DataSize);
        *(UInt32*)(&ctx.pPlane->pData[addr]) = c;
    }

    virtual Color GetPixelInScanline(ImageSwizzlerContext& ctx, unsigned x)
    {
        UINT addr = XGAddress2DTiledOffset(x, ctx.CachedBlockY, ctx.pPlane->Pitch/4, 4) *4;
        SF_DEBUG_WARNING4(addr >= ctx.pPlane->DataSize, "Unexpected pixel out-of-bounds (x=%d, y=%d, offset=%d, max=%d)\n",
            x, ctx.CachedBlockY, addr, ctx.pPlane->DataSize);
        return *(UInt32*)(&ctx.pPlane->pData[addr]);
    }
};

Render::ImageSwizzler& TextureManager::GetImageSwizzler() const
{
    static X360::ImageSwizzler swizzler;
    return swizzler;
}

}}};  // namespace Scaleform::Render::D3D9

