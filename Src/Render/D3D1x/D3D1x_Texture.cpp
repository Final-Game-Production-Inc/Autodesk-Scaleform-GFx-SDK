/**************************************************************************

Filename    :   D3D1x_Texture.cpp
Content     :   
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "D3D1x_Texture.h"
#include "Render\Render_TextureUtil.h"
#include "Kernel\SF_Debug.h"

namespace Scaleform { namespace Render { namespace D3D1x {

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

Texture::Texture(TextureManagerLocks* pmanagerLocks, ID3D1x(Texture2D)* ptexture, ImageSize size, ImageBase* image) :
    Render::Texture(pmanagerLocks, size, 0, 0, image, 0 )
{
    TextureFlags |= TF_UserAlloc;
    ptexture->AddRef();
    pTextures = &Texture0;
    pTextures[0].pTexture = ptexture;
    pTextures[0].pView    = 0;
    pTextures[0].Size     = size;
    pTextures[0].pStagingTexture = 0;
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

bool IsD3DFormatRescaleCompatible(DXGI_FORMAT format,
                                  ImageFormat *ptargetImageFormat,
                                  ResizeImageType* presizeType)
{
    switch(format)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            *ptargetImageFormat = Image_R8G8B8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            *ptargetImageFormat = Image_R8G8B8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case DXGI_FORMAT_A8_UNORM:
            *ptargetImageFormat = Image_A8;
            *presizeType        = ResizeGray;
            return true;
        default:
            break;
    }
    return false;
}


bool IsD3DFormatMipGenCompatible(DXGI_FORMAT format)
{
    switch(format)
    {
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return true;
        default:
            return false;
    }
}


bool Texture::Initialize()
{    
    SF_AMP_SCOPE_TIMER(GetManager()->RenderThreadId == GetCurrentThreadId() ? AmpServer::GetInstance().GetDisplayStats() : NULL, __FUNCTION__, Amp_Profile_Level_Medium);

    if ( TextureFlags & TF_UserAlloc )
    {
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

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        if (IsD3DFormatMipGenCompatible(GetTextureFormat()->GetD3DFormat()))
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
            allocMipLevels = 0;
        }
    }

    // Only use Dynamic textures for updatable/mappable textures.
    // Also, since Dynamic textures can be lost, don't allow them if ImageUse_InitOnly
    // is not specified.
    bool    allowDynamicTexture = ((Use & ImageUse_InitOnly) != 0) &&
                                  ((Use & (ImageUse_PartialUpdate | ImageUse_Map_Mask)) != 0);
    bool    renderTarget = (Use & ImageUse_RenderTarget) != 0;

    D3D1x(USAGE) usage  = D3D1x(USAGE_DEFAULT);
    UINT cpu            = 0;
    UINT bindFlags      = D3D1x(BIND_SHADER_RESOURCE);
    if (allowDynamicTexture)
    {
        usage = D3D1x(USAGE_DYNAMIC);
        cpu   |= D3D1x(CPU_ACCESS_WRITE);
    }

    if (renderTarget)
        bindFlags |= D3D1x(BIND_RENDER_TARGET);

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        D3D1x(TEXTURE2D_DESC) desc;
        memset(&desc, 0, sizeof desc);
        desc.Width              = tdesc.Size.Width;
        desc.Height             = tdesc.Size.Height;
        desc.MipLevels          = MipLevels;
        desc.ArraySize          = 1;
        desc.Format             = GetTextureFormat()->GetD3DFormat();
        desc.Usage              = usage;
        desc.BindFlags          = bindFlags;
        desc.CPUAccessFlags     = cpu;
        desc.SampleDesc.Count   = 1;

        if (FAILED(pmanager->pDevice->CreateTexture2D( &desc, 0, &tdesc.pTexture) ) ||
            FAILED(pmanager->pDevice->CreateShaderResourceView( tdesc.pTexture, 0, &tdesc.pView)))
        {
            SF_DEBUG_ERROR(1, "ID3D1x(Device)::CreateTexture2D failed");
            // Texture creation failed, release all textures and fail.
initialize_texture_fail_after_create:
            ReleaseHWTextures();
            if (State != State_Lost)
                State = State_InitFailed;
            return false;
        }
    }

    // Upload image content to texture, if any.
    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture");
        goto initialize_texture_fail_after_create;
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

bool Texture::Initialize(ID3D1x(Texture2D)* ptexture)
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

    D3D1x(TEXTURE2D_DESC) texDesc;
    ptexture->GetDesc(&texDesc);
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
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
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
    pTextures[0].Size.SetSize(texDesc.Width, texDesc.Height);

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = pTextures[0].Size;

    // Create the view.
    if ( FAILED(GetManager()->pDevice->CreateShaderResourceView( pTextures[0].pTexture, 0, &pTextures[0].pView )))
    {
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, ImageRescaleType &rescaleType, 
                                                ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_UNUSED(inputFormat);
    if (rescale && !IsD3DFormatRescaleCompatible(GetTextureFormat()->GetD3DFormat(),
                                                 &rescaleBuffFromat, &rescaleType))
        convert = true;
    if (swMipGen && !IsD3DFormatMipGenCompatible(GetTextureFormat()->GetD3DFormat()))
        convert = true;
}

void Texture::ReleaseHWTextures(bool staging)
{
    Render::Texture::ReleaseHWTextures(staging);

    TextureManager* pmanager = GetManager();
    bool useKillList = (pmanager->RenderThreadId == 0 || 
        GetCurrentThreadId() != pmanager->RenderThreadId);

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        ID3D1x(Texture2D)* ptexture = pTextures[itex].pTexture;
        ID3D1x(ShaderResourceView)* pview = pTextures[itex].pView;
        ID3D1x(Texture2D)* pstaging = staging ? pTextures[itex].pStagingTexture : 0;

        if (ptexture)
        {
            if (useKillList)
            {
                pmanager->D3DTextureKillList.PushBack(ptexture);
                pmanager->D3DTexViewKillList.PushBack(pview);
                if (pstaging)
                    pmanager->D3DTextureKillList.PushBack(pstaging);
            }
            else
            {
                ptexture->Release();
                pview->Release();
                if (pstaging)
                    pstaging->Release();
            }
        }
        pTextures[itex].pTexture = 0;
        pTextures[itex].pView    = 0;
        if (staging)
            pTextures[itex].pStagingTexture = 0;
    }
}

// Applies a texture to device starting at pstageIndex, advances index
void    Texture::ApplyTexture(unsigned stageIndex, const ImageFillMode& fm)
{
    Render::Texture::ApplyTexture(stageIndex, fm);

    TextureManager* pmanager = GetManager();
    ID3D1x(SamplerState)* sampler = pmanager->SamplerStates[fm.Fill];
    ID3D1x(ShaderResourceView) * views[MaxTextureCount];
    for ( unsigned view = 0; view < TextureCount; ++view )
        views[view] = pTextures[view].pView;

    // Assumption: all textures in a stage use the same sampler.
    pmanager->SetSamplerState(stageIndex, TextureCount, views, sampler);
}

bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
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

// ***** DepthStencilSurface

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), pDepthStencilSurface(0), pDepthStencilSurfaceView(0)
{
}

DepthStencilSurface::~DepthStencilSurface()
{
    if (pDepthStencilSurface)
        pDepthStencilSurface->Release();
    if (pDepthStencilSurfaceView)
        pDepthStencilSurfaceView->Release();
}

bool DepthStencilSurface::Initialize()
{
    TextureManager* pmanager= (TextureManager*)GetTextureManager();

    D3D1x(TEXTURE2D_DESC) desc;
    memset(&desc, 0, sizeof desc);
    desc.Width              = Size.Width;
    desc.Height             = Size.Height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.Usage              = D3D1x(USAGE_DEFAULT);
    desc.BindFlags          = D3D1x(BIND_DEPTH_STENCIL);
    desc.CPUAccessFlags     = 0;
    desc.SampleDesc.Count   = 1;

    if ( FAILED(pmanager->pDevice->CreateTexture2D(&desc, 0, (ID3D1x(Texture2D)**)&pDepthStencilSurface)) ||
         FAILED(pmanager->pDevice->CreateDepthStencilView(pDepthStencilSurface, 0,&pDepthStencilSurfaceView )))
        State = Texture::State_InitFailed;
    else
        State = Texture::State_Valid;
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

    D3D1x::Texture* d3dTexture                  = reinterpret_cast<Texture*>(ptexture);
    TextureManager* pmgr                        = d3dTexture->GetManager();
    Ptr<ID3D1x(Device)> pdevice                 = pmgr->GetDevice();
    Ptr<ID3D1x(DeviceContext)> pdeviceContext   = pmgr->GetDeviceContext();
    pTexture                                    = ptexture;
    StartMipLevel                               = mipLevel;
    LevelCount                                  = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        ImagePlane              plane(tdesc.Size, 0);

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        D3D1x(TEXTURE2D_DESC) inputDesc;
        tdesc.pTexture->GetDesc(&inputDesc);
        D3D1x(TEXTURE2D_DESC) stagingDesc;
        memcpy(&stagingDesc, &inputDesc, sizeof inputDesc);
        stagingDesc.CPUAccessFlags  = D3D1x(CPU_ACCESS_WRITE);
        stagingDesc.Usage           = D3D1x(USAGE_STAGING);
        stagingDesc.BindFlags       = 0;

        if (!tdesc.pStagingTexture)
        {
            if ( FAILED(pdevice->CreateTexture2D(&stagingDesc, 0, &tdesc.pStagingTexture)))
                return false;
        }

        // The only case where we need to copy the GPU resource to the CPU is in the case of RenderTargets.
        // RenderTargets (presumably) will be updated the by the GPU, and therefore the CPU needs the updated
        // contents. Things that will only be updated by the CPU (eg. GlyphCache) will always have the most
        // updated results in the staging texture, and therefore do not need the copyback. Note that on feature
        // levels lower than 10_0, CopyResource GPU->CPU is not supported, so give a warning in this case and skip it.
        if ((ptexture->Use & (ImageUse_MapRenderThread|ImageUse_RenderTarget)) == (ImageUse_MapRenderThread|ImageUse_RenderTarget))
        {
            static bool checkHandleCopyResource = false;
            static bool canHandleCopyResource = true;       // assume capability

            // Now check, based on feature level of the device.
            if (!checkHandleCopyResource)
            {
                D3D10
                ( 
                    Ptr<ID3D1x(Device1)> d3d10Device1;
                    if ( SUCCEEDED(pdevice->QueryInterface(IID_ID3D10Device1, (void**)&d3d10Device1.GetRawRef())) && d3d10Device1)
                        canHandleCopyResource = (d3d10Device1->GetFeatureLevel() >= D3D10_FEATURE_LEVEL_10_0);
                );
                D3D11
                ( 
                    canHandleCopyResource = pdevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_0;
                )
                checkHandleCopyResource = true;
            }

            SF_DEBUG_WARNONCE(!canHandleCopyResource, "Attempt to Map a texture with ImageUse_RenderTarget, but device feature level "
                "does not support CopyResource, skipping copy (GPU operations may be invalidated).");
            if (canHandleCopyResource)
                pdeviceContext->CopyResource(tdesc.pStagingTexture, tdesc.pTexture);
        }

        for (unsigned level = 0; level < levelCount; level++)
        {
            D3D1x(MAPPED_TEXTURE2D) mappedResource;
            if ( SUCCEEDED( D3D1xMapTexture2D(pdeviceContext, tdesc.pStagingTexture, level, D3D1x(MAP_WRITE), 0, &mappedResource)))
            {
                plane.Pitch    = mappedResource.RowPitch;
                plane.pData    = (UByte*)mappedResource.pData;
                plane.DataSize = ImageData::GetMipLevelSize(Data.GetFormat(), plane.GetSize(), itex); 
            }
            else
            {
                plane.Pitch    = 0;
                plane.pData    = 0;
                plane.DataSize = 0;
                return false;
            }

            Data.SetPlane(level * textureCount + itex, plane);
            // Prepare for next level.
            plane.SetNextMipSize();
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool applyUpdate)
{
    Texture* d3dTexture                         = reinterpret_cast<Texture*>(pTexture);
    TextureManager* pmgr                        = d3dTexture->GetManager();
    Ptr<ID3D1x(DeviceContext)> pdeviceContext   = pmgr->GetDeviceContext();

    unsigned textureCount = pTexture->TextureCount;
    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = d3dTexture->pTextures[itex];
        ImagePlane plane;

        // Unmap the entire resource first.
        for (int level = 0; level < LevelCount; level++)
            D3D1xUnmapTexture2D(pdeviceContext, tdesc.pStagingTexture, level);

        if (applyUpdate)
        {
            for (int level = 0; level < LevelCount; level++)
            {
                Data.GetPlane(level * textureCount + itex, &plane);
                if (plane.pData)
                {
                    pdeviceContext->CopySubresourceRegion(tdesc.pTexture, level + StartMipLevel, 0, 0, 0, tdesc.pStagingTexture, level, 0);
                    plane.pData = 0;
                }
            }
        }
    }

    pTexture->pMap = 0;
    pTexture       = 0;
    StartMipLevel  = 0;
    LevelCount     = 0;    
}

// ***** TextureManager
TextureManager::TextureManager(ID3D1x(Device)* pdevice,
                               ID3D1x(DeviceContext)* pcontext,
                               ThreadId renderThreadId, 
                               ThreadCommandQueue* commandQueue,
                               TextureCache* texCache) : 
    Render::TextureManager(renderThreadId, commandQueue, texCache),
    pDevice(pdevice), pDeviceContext(pcontext)
{
    initTextureFormats();

    // Precreate the sampler states.
    memset(SamplerStates, 0, sizeof SamplerStates);
    for ( unsigned wrap = 0; wrap < Wrap_Count; wrap++ )
    {
        for ( unsigned filter = 0; filter < Sample_Count; ++filter )
        {
            D3D1x(SAMPLER_DESC) samplerDesc;
            memset(&samplerDesc, 0, sizeof samplerDesc );
            switch( filter << Sample_Shift )
            {
            default:
            case Sample_Point:  samplerDesc.Filter = D3D1x(FILTER_MIN_MAG_MIP_POINT); break;
            case Sample_Linear: samplerDesc.Filter = D3D1x(FILTER_MIN_MAG_MIP_LINEAR); break;
            }

            switch( wrap )
            {
            default:
            case Wrap_Clamp:    samplerDesc.AddressU = samplerDesc.AddressV = D3D1x(TEXTURE_ADDRESS_CLAMP); break;
            case Wrap_Repeat:   samplerDesc.AddressU = samplerDesc.AddressV = D3D1x(TEXTURE_ADDRESS_WRAP); break;
            }
            samplerDesc.AddressW        = D3D1x(TEXTURE_ADDRESS_CLAMP);
            samplerDesc.ComparisonFunc  = D3D1x(COMPARISON_ALWAYS);
            samplerDesc.MaxLOD          = D3D1x(FLOAT32_MAX);
            samplerDesc.MipLODBias      = -0.75f;
            samplerDesc.MaxAnisotropy   = 1;

            ImageFillMode fm((WrapMode)wrap, (SampleMode)(filter << Sample_Shift));
            SF_ASSERT( fm.Fill < SamplerTypeCount );
            HRESULT hr = pDevice->CreateSamplerState( &samplerDesc, &SamplerStates[fm.Fill] );        
            SF_DEBUG_MESSAGE1( FAILED(hr), "Failed CreateSamplerState (hr=0x%x)", hr ); SF_UNUSED(hr);
        }
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

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    processTextureKillList();

    // Destroy sampler states.
    for ( unsigned wrap = 0; wrap < Wrap_Count; wrap++ )
    {
        for ( unsigned filter = 0; filter < Sample_Count; ++filter )
        {
            ImageFillMode fm((WrapMode)wrap, (SampleMode)(filter << Sample_Shift));
            SF_ASSERT( fm.Fill < SamplerTypeCount );
            if ( SamplerStates[fm.Fill] )
                SamplerStates[fm.Fill]->Release();
        }
    }
    memset(SamplerStates, 0, sizeof SamplerStates);

    pDevice = 0;
}

void TextureManager::SetSamplerState( unsigned stage, unsigned viewCount, ID3D1x(ShaderResourceView)** views, ID3D1x(SamplerState)* state)
{
    bool loadSamplers = false;
    bool loadTextures = false;
    ID3D1x(SamplerState)* states[16];
    for ( unsigned i = 0; i < viewCount; ++i)
    {
        states[i] = state;
        if ( CurrentSamplers[i+stage] != state )
        {
            loadSamplers = true;
        }

        if ( CurrentTextures[i+stage] != views[i] )
        {
            loadTextures = true;
        }
    }
    if ( loadSamplers )
    {
        pDeviceContext->PSSetSamplers( stage, viewCount, states );
        memcpy(&CurrentSamplers[stage], states, viewCount*sizeof(ID3D1x(SamplerState)*));
    }
    if ( loadTextures )
    {
        pDeviceContext->PSSetShaderResources( stage, viewCount, views );
        memcpy(&CurrentTextures[stage], views, viewCount*sizeof(ID3D1x(ShaderResourceView)*));
    }
}

void TextureManager::BeginScene()
{
    memset(CurrentSamplers, 0, sizeof(CurrentSamplers));
    memset(CurrentTextures, 0, sizeof(CurrentTextures));
    ID3D1x(ShaderResourceView)* views[MaximumStages];
    ID3D1x(SamplerState)* states[MaximumStages];
    memset(views, 0, sizeof(views));
    memset(states, 0, sizeof(states));
    pDeviceContext->PSSetSamplers( 0, MaximumStages, states);
    pDeviceContext->PSSetShaderResources( 0, MaximumStages, views);
}


// ***** D3D1x Format mapping and conversion functions

void SF_STDCALL D3D1x(CopyScanline8_Extend_A8_A8L8)(UByte* pd, const UByte* ps, UPInt size,
                                                  Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0];
    }
}

void SF_STDCALL D3D1x(CopyScanline8_Extend_A8_A4R4G4B4)(UByte* pd, const UByte* ps, UPInt size,
                                                      Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0] | 0x0f; // Copy high 4 bits; low bits set to F.
    }
}

void SF_STDCALL D3D1x(CopyScanline16_Retract_A8L8_A8)(UByte* pd, const UByte* ps, UPInt size,
                                                    Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {        
        pd[0] = ps[1];
    }
}

void SF_STDCALL D3D1x(CopyScanline16_Retract_A4R4G4B4_A8)(UByte* pd, const UByte* ps, UPInt size,
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
    { Image_R8G8B8A8,   DXGI_FORMAT_R8G8B8A8_UNORM,  4, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,   DXGI_FORMAT_B8G8R8A8_UNORM,  4, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_R8G8B8,     DXGI_FORMAT_R8G8B8A8_UNORM,  4, D3D_FEATURE_LEVEL_9_1,  &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,     DXGI_FORMAT_B8G8R8A8_UNORM,  4, D3D_FEATURE_LEVEL_9_1,  &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_A8,         DXGI_FORMAT_R8_UNORM,        1, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

    // Compressed formats.
    { Image_DXT1,       DXGI_FORMAT_BC1_UNORM,       0, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT3,       DXGI_FORMAT_BC2_UNORM,       0, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT5,       DXGI_FORMAT_BC3_UNORM,       0, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_BC7,        DXGI_FORMAT_BC7_UNORM,       0, D3D_FEATURE_LEVEL_11_0, &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},

    // Video formats.
    { Image_Y8_U2_V2,   DXGI_FORMAT_R8_UNORM,        1, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,DXGI_FORMAT_R8_UNORM,        1, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },

#ifdef _DURANGO
    // Format used by the ColorFrameReader of the Xbox One Kinect. Not used directly by GFx itself, but it is
    // included here so that users can easily replace textures within .SWFs using the Kinect color output.
    { Image_Y4_U2_V2,    DXGI_FORMAT_G8R8_G8B8_UNORM, 1, D3D_FEATURE_LEVEL_9_1,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
#endif

    { Image_None,       DXGI_FORMAT_UNKNOWN,         0, D3D_FEATURE_LEVEL_9_1,  0,                                       0 }
};


void        TextureManager::initTextureFormats()
{
    // Obtain the feature level, so we can check if the TextureFormat is unsupported by the current feature level.
    D3D_FEATURE_LEVEL currentFeatureLevel = D3D_FEATURE_LEVEL_10_0;
#if (SF_D3D_VERSION == 11)
    currentFeatureLevel = pDevice->GetFeatureLevel();
#elif (SF_D3D_VERSION == 10 )
    Ptr<ID3D1x(Device1)> d3d10Device1;
    if ( SUCCEEDED(pDevice->QueryInterface(IID_ID3D10Device1, (void**)&d3d10Device1.GetRawRef())) && d3d10Device1)
    {
        currentFeatureLevel = (D3D_FEATURE_LEVEL)d3d10Device1->GetFeatureLevel();
    }
#endif

    TextureFormat::Mapping* pmapping = 0;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        // Must exclude non-supported image formats before checking the support, because they will
        // cause the D3D11 debug runtime to assert.
        if (currentFeatureLevel < pmapping->MinFeatureLevel)
            continue;

        UINT formatSupport;
        // See if format is supported.        
        if (SUCCEEDED( pDevice->CheckFormatSupport(pmapping->D3DFormat, &formatSupport) ) &&
            formatSupport & D3D1x(FORMAT_SUPPORT_TEXTURE2D) )
        {
            TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping, 0);
            // And now check its capabilities to assign extra Usage.
            if (formatSupport & D3D1x(FORMAT_SUPPORT_MIP_AUTOGEN) )
            {
                // tf.D3DUsage |= D3DUSAGE_AUTOGENMIPMAP;
            }
            TextureFormats.PushBack(tf);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}


void    TextureManager::processTextureKillList()
{
    for (unsigned i = 0; i<D3DTexViewKillList.GetSize(); i++)
        D3DTexViewKillList[i]->Release();
    for (unsigned i = 0; i<D3DTextureKillList.GetSize(); i++)
        D3DTextureKillList[i]->Release();
    D3DTexViewKillList.Clear();
    D3DTextureKillList.Clear();
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

    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(ID3D1x(Texture2D)* pd3dtexture, ImageSize imgSize, ImageBase* image )
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

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(ID3D1x(Texture2D)* psurface)
{
    if ( !psurface ) 
        return 0;

    psurface->AddRef();
    D3D1x(TEXTURE2D_DESC) desc;
    psurface->GetDesc(&desc);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, ImageSize(desc.Width, desc.Height) );
    pdss->pDepthStencilSurface = psurface;
    pdss->State = Texture::State_Valid;
    return pdss;
}

}}};  // namespace Scaleform::Render::D3D1x

