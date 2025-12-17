/**************************************************************************

Filename    :   D3D9_Texture.cpp
Content     :   D3D9 Texture and TextureManager implementation
Created     :   January 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "D3D9_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace D3D9 {

extern TextureFormat::Mapping TextureFormatMapping[];

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat), 
    Type(Type_Managed)
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

Texture::Texture(TextureManagerLocks* pmanagerLocks, IDirect3DTexture9* ptexture, const ImageSize& size, ImageBase* pimage) :
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0), 
    Type(Type_Normal)
{
    TextureFlags |= TF_UserAlloc;
    ptexture->AddRef();
    pTextures = &Texture0;
    pTextures[0].pStagingTexture = 0;
    pTextures[0].pTexture = ptexture;
    pTextures[0].Size = size;
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

void Texture::GetUVGenMatrix(Matrix2F* mat) const
{
    // UV Scaling rules are as follows:
    //  - If image got scaled, divide by original size.
    //  - If image is padded, divide by new texture size.
    const ImageSize& sz = (TextureFlags & TF_Rescale) ? ImgSize : pTextures[0].Size;
    *mat = Matrix2F::Scaling(1.0f / (float)sz.Width, 1.0f / (float)sz.Height);
}


void    Texture::LoseManager()
{        
    Ptr<Texture> thisTexture = this;    // Guards against the texture being deleted inside Texture::LoseManager()
    // Need to unmap DrawableImage via LoseTextureData.
    Render::Texture::LoseTextureData();
    Render::Texture::LoseManager();
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
        // NOTE: If this is after a reset, pTexture will be NULL, and the Initialize call will fail.
        // The user must manually call Texture::Initialize with the new pointer to the new texture.
        // This can be done for example, during a HALNotify event.
        return Initialize( pTextures[0].pTexture );
    }

    bool            resize  = false;
    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    // Determine sizes of textures.
    if (State != State_Lost)
    {        
        for (itex = 0; itex < TextureCount; itex++)
        {
            HWTextureDesc& tdesc = pTextures[itex];
            tdesc.Size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
            if (!pmanager->Caps.NonPow2Supported(Use))
            {
                ImageSize roundedSize = ImageSize_RoundUpPow2(tdesc.Size);
                if (roundedSize != tdesc.Size)
                {
                    tdesc.Size = roundedSize;
                    resize = true;
                }
            }
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
    }
        
    // Determine how many mipLevels we should have and whether we can
    // auto-generate them or not.
    unsigned allocMipLevels = MipLevels;
    DWORD    d3dUsage = 0;
    D3DPOOL  d3dPool  = (Type == Type_Managed) ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        if (!GetTextureFormat()->CanAutoGenMipmaps())
        {            
            TextureFlags |= TF_SWMipGen;
            // If using SW MipGen, determine how many mip-levels we should have.
            allocMipLevels = 31;
            for (itex = 0; itex < TextureCount; itex++)
                allocMipLevels = Alg::Min(allocMipLevels,
                                          ImageSize_MipLevelCount(pTextures[itex].Size));
            MipLevels = (UByte)allocMipLevels;
        }
        else
        {
            d3dUsage |= D3DUSAGE_AUTOGENMIPMAP;
            allocMipLevels = 0;
        }
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

    // Only use Dynamic textures for updatable/mappable textures.
    // Also, since Dynamic textures can be lost, don't allow them if ImageUse_InitOnly
    // is not specified.
    bool    allowDynamicTexture = ((Use & ImageUse_InitOnly) != 0) &&
                                  ((Use & (ImageUse_PartialUpdate | ImageUse_Map_Mask)) != 0);
    if (!pmanager->Caps.DynamicTexAllowed())
        allowDynamicTexture = false;
    
    // If using D3D9Ex, force usage of dynamic default pool textures.
    if (pmanager->Caps.IsD3D9Ex())
        allowDynamicTexture = true;

    if (allowDynamicTexture)
    {
        d3dUsage |= D3DUSAGE_DYNAMIC;
        d3dPool  = D3DPOOL_DEFAULT;
        Type = Type_Dynamic;
    }

    if ( Use & ImageUse_RenderTarget)
    {
        d3dUsage = D3DUSAGE_RENDERTARGET;
        d3dPool = D3DPOOL_DEFAULT;
        Type = (Use & ImageUse_Map_Mask) ? Type_StagingBacked : Type_Dynamic;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        if (S_OK != pmanager->pDevice->CreateTexture(
                        tdesc.Size.Width, tdesc.Size.Height, allocMipLevels,
                        d3dUsage, GetTextureFormat()->GetD3DFormat(), d3dPool, &tdesc.pTexture, 0) )
        {
initialize_texture_fail_after_create0:
            SF_DEBUG_ERROR(1, "CreateTexture failed - IDirect3DTexture9::CreateTexture failed");
            // Texture creation failed, release all textures and fail.
initialize_texture_fail_after_create:
            ReleaseHWTextures();
            if (State != State_Lost)
                State = State_InitFailed;
            return false;
        }

        // Staging-backed textures get a staging buffer of the same size.
        if (Type == Type_StagingBacked)
        {
            // Only create the staging texture if we are initializing for the first time. Assume that
            // if we are in State_Lost, then the staging texture already exists.
            if (State == State_InitPending)
            {
                if (S_OK != pmanager->pDevice->CreateTexture(
                    tdesc.Size.Width, tdesc.Size.Height, allocMipLevels,
                    (d3dUsage & ~D3DUSAGE_RENDERTARGET),
                    GetTextureFormat()->GetD3DFormat(), D3DPOOL_SYSTEMMEM,
                    &tdesc.pStagingTexture, 0) )
                    goto initialize_texture_fail_after_create0;
            }
        }
    }

    // If we are initializing after being lost, update the render target with
    // the staging texture's contents, as the RT will have just been created.
    if ( State == State_Lost && Type == Type_StagingBacked )
        UpdateRenderTargetData(0);

    // Upload image content to texture, if any.
    // TODOBM: drawable images should not Update when being created. Pointless mapping and unmapping.
    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture");
        goto initialize_texture_fail_after_create;
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
    if ( pTextures[0].pTexture != ptexture )
    {
        ReleaseHWTextures();
        pTextures[0].pTexture = ptexture;
        pTextures[0].pTexture->AddRef();
    }

    MipLevels = (UByte)ptexture->GetLevelCount();

    D3DSURFACE_DESC surfaceDesc;
    ptexture->GetLevelDesc(0, &surfaceDesc);

    // Assume that all mip surfaces are allocated from the same pool.
    switch( surfaceDesc.Pool )
    {
    case D3DPOOL_MANAGED: Type = Type_Managed; break;
    case D3DPOOL_DEFAULT: Type = Type_Dynamic; break;
    default: SF_ASSERT(0); return false; // unexpected texture pool type (only Managed and Default are supported).
    }

    pFormat = 0;
    TextureManager* pmanager = GetManager();

    // If an image is provided, try to obtain the texture format from the image.
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        TextureFormat::Mapping* pmapping;
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->D3DFormat == surfaceDesc.Format )
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
    pTextures[0].Size.SetSize(surfaceDesc.Width, surfaceDesc.Height);

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = pTextures[0].Size;

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    rescaleBuffFromat = inputFormat;
    rescaleType = ResizeNone;
    
    if (rescale && !IsD3DFormatRescaleCompatible(GetTextureFormatMapping()->D3DFormat,
                                                 &rescaleBuffFromat, &rescaleType))
        convert = true;
    if (swMipGen && !IsD3DFormatMipGenCompatible(GetTextureFormatMapping()->D3DFormat))
        convert = true;
}

void Texture::ReleaseHWTextures(bool staging)
{
    Render::Texture::ReleaseHWTextures();

    TextureManager* pmanager = GetManager();
    bool useKillList = !pmanager->Caps.IsMultiThreaded() &&
                       (GetCurrentThreadId() != pmanager->RenderThreadId);

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        IDirect3DTexture9* ptexture = pTextures[itex].pTexture;
        IDirect3DTexture9* pstageTexture = staging ? pTextures[itex].pStagingTexture : 0;

        if (useKillList)
        {
            if (ptexture)
                pmanager->D3DTextureKillList.PushBack(ptexture);
            if (pstageTexture)
                pmanager->D3DTextureKillList.PushBack(pstageTexture);
        }
        else
        {
            if (ptexture)
                ptexture->Release();
            if (pstageTexture)
                pstageTexture->Release();
        }

        pTextures[itex].pTexture = 0;
        if ( staging )
            pTextures[itex].pStagingTexture = 0;
    }
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
void Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    Render::Texture::ApplyTexture(stageIndex, fm);

    D3DTEXTUREFILTERTYPE filter = 
        Texture_FilterLookup[fm.GetSampleMode() >> Sample_Shift];
    D3DTEXTUREADDRESS    address = Texture_AddressLookup[fm.GetWrapMode()];

    for (unsigned i = 0; i < TextureCount; i++, stageIndex++)
        GetManager()->SetSamplerState(stageIndex, pTextures[i].pTexture, filter, address);
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL*)
{
    unsigned textureCount = TextureCount;
    bool result = false;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = pTextures[itex];

        // TBD: What do we do for other texture types?
        // - Perhaps users will want to use this for other texture types (non-render-target)?        

        if (Type == Texture::Type_StagingBacked)
        {
            IDirect3DDevice9* device = ((TextureManager*)GetTextureManager())->GetDevice();
            device->UpdateTexture(tdesc.pStagingTexture, tdesc.pTexture);
            result = true;
        }
    }

    return result;
}

bool Texture::UpdateStagingData(RenderTargetData*)
{
    SF_DEBUG_WARNING(Type != Texture::Type_StagingBacked, "Attempt to UpdateStatingData of a non-system backed texture.");
    if (Type != Texture::Type_StagingBacked)
        return false;

    Ptr<IDirect3DSurface9> srcSurface, destSurface;
    IDirect3DDevice9* pdevice = GetManager()->GetDevice();    
    pTextures[0].pTexture->GetSurfaceLevel(0, &srcSurface.GetRawRef());
    pTextures[0].pStagingTexture->GetSurfaceLevel(0, &destSurface.GetRawRef());
    return SUCCEEDED( pdevice->GetRenderTargetData(srcSurface, destSurface) );
}


bool Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
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
        pDepthStencilSurface->Release();
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

    Texture* d3dTexture = reinterpret_cast<Texture*>(ptexture);
    pTexture            = ptexture;
    StartMipLevel       = mipLevel;
    LevelCount          = levelCount;


    bool     failedLock   = false;
    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        IDirect3DTexture9*      lockedTexture = (d3dTexture->Type == Texture::Type_StagingBacked) ?
                                                tdesc.pStagingTexture : tdesc.pTexture;
        ImagePlane              plane(tdesc.Size, 0);

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        for (unsigned level = 0; level < levelCount; level++)
        {
            D3DLOCKED_RECT lr;
            // TBD: D3DLOCK_DISCARD may be useful for full-screen DYNAMIC
            // texture updates.

            if (!failedLock &&
                (lockedTexture->LockRect(level + StartMipLevel, &lr, 0, 0) == D3D_OK))
            {   
                plane.Pitch    = lr.Pitch;
                plane.pData    = (UByte*)lr.pBits;
                plane.DataSize = ImageData::GetMipLevelSize(Data.GetFormat(), plane.GetSize(), level); 
                //lr.Pitch * plane.Height;
            }
            else
            {
                plane.Pitch    = 0;
                plane.pData    = 0;
                plane.DataSize = 0;
                failedLock = true;
            }

            Data.SetPlane(level * textureCount + itex, plane);
            // Prepare for next level.
            plane.SetNextMipSize();
        }
    }

    if (failedLock)
    {
        SF_DEBUG_ERROR(1, "D3D9::MappedTexture::Map failed - LockRect failure");
        Unmap();
        return false;
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool)
{
    unsigned textureCount = pTexture->TextureCount;
    Texture* d3dTexture = reinterpret_cast<Texture*>(pTexture);

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        IDirect3DTexture9*      lockedTexture = (d3dTexture->Type == Texture::Type_StagingBacked) ?
                                                tdesc.pStagingTexture : tdesc.pTexture;
        ImagePlane plane;

        for (int level = 0; level < LevelCount; level++)
        {
            Data.GetPlane(level * textureCount + itex, &plane);
            if (plane.pData)
            {
                lockedTexture->UnlockRect(level + StartMipLevel);
                plane.pData = 0;
            }
        }
    }
    MappedTextureBase::Unmap();
}

// ***** TextureManager

TextureManager::TextureManager(IDirect3DDevice9* pdevice,
                               D3DCapFlags capFlags,
                               ThreadId renderThreadId, ThreadCommandQueue* commandQueue,
                               TextureCache* texCache) :
    Render::TextureManager(renderThreadId, commandQueue, texCache), pDevice(pdevice), Caps(capFlags)
{
    initTextureFormats();
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

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    SF_ASSERT(TextureInitQueue.IsEmpty());
    processTextureKillList();

    pDevice = 0;
}


void TextureManager::PrepareForReset()
{
    Mutex::Locker lock(&pLocks->TextureMutex);

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    //SF_ASSERT(TextureInitQueue.IsEmpty());

    // Blast out all non-managed textures.
    Texture* p = reinterpret_cast<Texture*>(Textures.GetFirst());
    while (!Textures.IsNull(p))
    {
        Texture* next = reinterpret_cast<Texture*>(Textures.GetNext(p));
        if (p->Type != Texture::Type_Managed)
            p->LoseTextureData();
        p = next;
    }
    processTextureKillList();
}

void TextureManager::RestoreAfterReset()
{
    Mutex::Locker lock(&pLocks->TextureMutex);

    // Blast out all non-managed textures.
    Render::Texture* p= Textures.GetFirst();
    while (!Textures.IsNull(p))
    {
        // Try to restore textures.
        if (p->State == Texture::State_Lost)
            p->Initialize();
        p = Textures.GetNext(p);
    }

}


// ***** D3D9 Format mapping and conversion functions

void SF_STDCALL D3D9_CopyScanline8_Extend_A8_A8L8(UByte* pd, const UByte* ps, UPInt size,
                                                  Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0];
    }
}

void SF_STDCALL D3D9_CopyScanline8_Extend_A8_A4R4G4B4(UByte* pd, const UByte* ps, UPInt size,
                                                      Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0] | 0x0f; // Copy high 4 bits; low bits set to F.
    }
}

void SF_STDCALL D3D9_CopyScanline16_Retract_A8L8_A8(UByte* pd, const UByte* ps, UPInt size,
                                                    Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {        
        pd[0] = ps[1];
    }
}

void SF_STDCALL D3D9_CopyScanline16_Retract_A4R4G4B4_A8(UByte* pd, const UByte* ps, UPInt size,
                                        Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {        
        pd[0] = ps[1] & ~0x0f; // Copy high 4 bits; zero low bits.
    }
}


// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

TextureFormat::Mapping TextureFormatMapping[] = 
{
    // Warning: Different versions of the same ImageFormat must go right after each-other,
    // as initTextureFormats relies on that fact to skip them during detection.
    { Image_R8G8B8A8,   D3DFMT_A8B8G8R8, 4, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_R8G8B8A8,   D3DFMT_A8R8G8B8, 4, &Image_CopyScanline32_SwapBR,            &Image_CopyScanline32_SwapBR },

    { Image_B8G8R8A8,   D3DFMT_A8R8G8B8, 4, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,   D3DFMT_A8B8G8R8, 4, &Image_CopyScanline32_SwapBR,            &Image_CopyScanline32_SwapBR },

    { Image_R8G8B8,     D3DFMT_A8R8G8B8, 4, &Image_CopyScanline24_Extend_RGB_BGRA,   &Image_CopyScanline32_Retract_BGRA_RGB },
    { Image_B8G8R8,     D3DFMT_A8R8G8B8, 4, &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },

    { Image_A8,         D3DFMT_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_A8,         D3DFMT_A8L8,     2, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_A8,         D3DFMT_A4R4G4B4, 2, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_DXT1,       D3DFMT_DXT1,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT3,       D3DFMT_DXT3,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT5,       D3DFMT_DXT5,     0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},

    { Image_Y8_U2_V2,   D3DFMT_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2,   D3DFMT_A8L8,     2, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_Y8_U2_V2,   D3DFMT_A4R4G4B4, 2, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_Y8_U2_V2_A8,D3DFMT_A8,       1, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,D3DFMT_A8L8,     1, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_Y8_U2_V2_A8,D3DFMT_A4R4G4B4, 2, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_None,       D3DFMT_UNKNOWN,  0, 0, 0 }
};


void        TextureManager::initTextureFormats()
{
    Ptr<IDirect3D9>               pd3d9;
    Ptr<IDirect3DSurface9>        pbackSurface;
    D3DCAPS9                      caps;
    D3DDEVICE_CREATION_PARAMETERS cp;
    D3DDISPLAYMODE                dispMode;
    
    pDevice->GetDirect3D(&pd3d9.GetRawRef());    
    pDevice->GetCreationParameters(&cp);
    pDevice->GetDeviceCaps(&caps);
    pd3d9->GetAdapterDisplayMode(cp.AdapterOrdinal, &dispMode);
    
    TextureFormat::Mapping* pmapping;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        // See if format is supported.
        if (D3D_OK ==
            pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                     dispMode.Format, 0,
                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat))
        {
            TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping, 0);

            // And now check its capabilities to assign extra Usage.
            if ((caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) &&
                (D3D_OK ==  pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                                     dispMode.Format, D3DUSAGE_AUTOGENMIPMAP,
                                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat)))
            {
                tf->D3DUsage |= D3DUSAGE_AUTOGENMIPMAP;
            }

            if ((caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) &&
                (D3D_OK ==  pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                                     dispMode.Format, D3DUSAGE_DYNAMIC,
                                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat)))
            {
                tf->D3DUsage |= D3DUSAGE_DYNAMIC;
            }

            TextureFormats.PushBack(tf);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}

MappedTextureBase& TextureManager::getDefaultMappedTexture()
{
    return MappedTexture0;
}

MappedTextureBase* TextureManager::createMappedTexture()
{
    return SF_HEAP_AUTO_NEW(this) MappedTexture;
}

void TextureManager::processTextureKillList()
{
    for (unsigned i = 0; i<D3DTextureKillList.GetSize(); i++)
        D3DTextureKillList[i]->Release();

    for (unsigned i = 0; i<D3DDepthStencilKillList.GetSize(); i++)
        D3DDepthStencilKillList[i]->Release();

    D3DDepthStencilKillList.Clear();
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

void TextureManager::SetSamplerState( unsigned stage, IDirect3DTexture9* d3dtex, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address )
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
    if ( CurrentTextures[stage] != d3dtex )
    {
        pDevice->SetTexture(stage, d3dtex);
        CurrentTextures[stage] = d3dtex;
    }
}

bool TextureManager::CanCreateTextureCurrentThread() const
{
    // If using a multi-threaded device, we can create textures on any thread.
    return (RenderThreadId == 0 || GetCurrentThreadId() == RenderThreadId || Caps.IsMultiThreaded());
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
        SF_DEBUG_ERROR(1, "CreateTexture failed - TextueManager has been Reset");
        return 0;
    }

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Render::Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);

    return postCreateTexture(ptexture, use);
}

void TextureManager::BeginScene()
{
    for ( int i = 0; i < MaximumStages; i++ )
    {
        AddressMode[i] = D3DTADDRESS_FORCE_DWORD;
        FilterType[i] = D3DTEXF_FORCE_DWORD;
        CurrentTextures[i] = 0;
        pDevice->SetTexture(i, 0);

        // Set sampler parameters to known values, that GFx will not modify during its rendering.
        union 
        {
            float fbias;
            DWORD d;
        } bias;
        bias.fbias = -0.75f;
        pDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, bias.d );
        pDevice->SetSamplerState(i, D3DSAMP_ELEMENTINDEX, 0);
        pDevice->SetSamplerState(i, D3DSAMP_SRGBTEXTURE, 0);

        // Set texture coordinate indices to match their stages, as we will be using the programmable pipeline.
        pDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
    }    
}

Render::Texture* TextureManager::CreateTexture(IDirect3DTexture9* pd3dtexture, ImageSize imgSize, Image* image )
{
    if ( pd3dtexture == 0 )
        return 0;

    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, pd3dtexture, imgSize, image);

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

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size, MemoryManager* manager)
{
    SF_UNUSED(manager);
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

unsigned TextureManager::GetTextureFormatSupport() const
{
    //unsigned formats = ImageFormats_Standard;

    //for (unsigned i = 0; i< TextureFormats.GetSize(); i++)    
    //{
    //    unsigned format = TextureFormats[i].GetImageFormat();
    //    if (format == Image_DXT5)
    //        formats |= ImageFormats_DXT;
    //}
    return ImageFormats_DXT;
}

}}};  // namespace Scaleform::Render::D3D9

