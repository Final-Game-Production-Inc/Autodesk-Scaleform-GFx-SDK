/**************************************************************************

Filename    :   D3D1x_Texture.h
Content     :   
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D1X_TEXTURE_H
#define INC_SF_D3D1X_TEXTURE_H
#pragma once

#include "Render/D3D1x/D3D1x_Config.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace D3D1x {


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
        D3D_FEATURE_LEVEL        MinFeatureLevel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    TextureFormat(const Mapping* mapping, DWORD d3dusage) : pMapping(mapping), D3DUsage(d3dusage) { }

    const Mapping*  pMapping;
    DWORD           D3DUsage;

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }

    // D3D1x Specific.
    DXGI_FORMAT             GetD3DFormat() const   { return pMapping->D3DFormat; }
};

class MappedTexture;
class TextureManager;

// D3D1x Texture class implementation; it many actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    static const UByte      MaxTextureCount = 4;

    struct HWTextureDesc
    {        
        ImageSize                   Size;
        ID3D1x(Texture2D)*          pTexture;
        ID3D1x(ShaderResourceView)* pView;
        ID3D1x(Texture2D)*          pStagingTexture;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, ID3D1x(Texture2D)* ptexture, ImageSize imgSize, ImageBase* pimage);
    ~Texture();

    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return pTextures[plane].Size; }
    TextureManager*         GetManager() const                      { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const                         { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(ID3D1x(Texture2D)* ptexture);
    void                    ReleaseHWTextures(bool staging = true);

    // Applies a texture to device starting at pstageIndex, advances index
    // TBD: Texture matrix may need to be adjusted if image scaling is done.
    void                    ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    // *** Interface implementation
    virtual Image*                  GetImage() const                { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat             GetFormat() const               { return GetImageFormat(); }
    const TextureFormat*            GetTextureFormat() const        { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual void            GetUVGenMatrix(Matrix2F* mat) const;
    
    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
};

// D3D9 DepthStencilSurface implementation. 
class DepthStencilSurface : public Render::DepthStencilSurface
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();

    ID3D1x(Texture2D)*        pDepthStencilSurface;
    ID3D1x(DepthStencilView)* pDepthStencilSurfaceView;
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


// D3D11 Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;
    friend class DepthStencilSurface;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<ID3D1x(Resource)*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DResourceArray;
    typedef ArrayLH<ID3D1x(View)*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DViewArray;
    
    ID3D1x(Device)*         pDevice;
    ID3D1x(DeviceContext)*  pDeviceContext;
    MappedTexture           MappedTexture0;    

    // Lists protected by TextureManagerLocks::TextureMutex.
    D3DResourceArray          D3DTextureKillList;
    D3DViewArray              D3DTexViewKillList;
    
    static const unsigned       SamplerTypeCount = (Sample_Count * Wrap_Count);
    ID3D1x(SamplerState)*       SamplerStates[SamplerTypeCount];

    // Detecting redundant sampler/address setting.
    static const int MaximumStages = 4;
    ID3D1x(SamplerState)*       CurrentSamplers[MaximumStages];
    ID3D1x(View)*               CurrentTextures[MaximumStages];

    // Detects supported D3DFormats and capabilities.
    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }
    
    virtual void    processTextureKillList();
    virtual void    processInitTextures();    


public:
    TextureManager(ID3D1x(Device)* pdevice,
                   ID3D1x(DeviceContext) * pcontext,
                   ThreadId renderThreadId, 
                   ThreadCommandQueue* commandQueue,
                   TextureCache* texCache = 0);
    ~TextureManager();

    // Used once texture manager is no longer necessary.
    void    Reset();

    // Does rendundancy checking on state setting.
    void            SetSamplerState( unsigned stage, unsigned viewCount, ID3D1x(ShaderResourceView)** views, ID3D1x(SamplerState)* state = 0);

    virtual void    BeginScene();

    // *** TextureManager
    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(ID3D1x(Texture2D)* pd3dtexture,
                                           ImageSize imgSize = ImageSize(0), ImageBase* image = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
    bool IsMultiThreaded() const    { return RenderThreadId != 0; }

    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                           MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(ID3D1x(Texture2D)* psurface);

	virtual bool			IsDrawableImageFormat(ImageFormat format) const { return (format == Image_B8G8R8A8) || (format == Image_R8G8B8A8); }

    ID3D1x(Device)*         GetDevice() const           { return pDevice; }
    ID3D1x(DeviceContext)*  GetDeviceContext() const    { return pDeviceContext; }
};


}}};  // namespace Scaleform::Render::D3D1x

#endif // INC_SF_D3D1X_TEXTURE_H
