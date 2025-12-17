/**************************************************************************

Filename    :   X360_Texture.h
Content     :   D3D9 Texture and TextureManager header
Created     :   January 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_X360_Texture_H
#define INC_SF_Render_X360_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Kernel/SF_HeapNew.h"
#include "X360_MemoryManager.h"
#include "X360_Types.h"

namespace Scaleform { namespace Render { namespace X360 {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        D3DFORMAT                D3DFormat;
        UByte                    BytesPerPixel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    const Mapping*  pMapping;
    DWORD           D3DUsage;

    TextureFormat(const Mapping* mapping, DWORD d3dusage) : pMapping(mapping), D3DUsage(d3dusage) { }

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }

    D3DFORMAT       GetD3DFormat() const   { return pMapping->D3DFormat; }
};

class MappedTexture;
class TextureManager;


// X360 Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    struct HWTextureDesc
    {        
        ImageSize           Size;
        UByte*              pTexData;
        D3DTexture *        Tex;
        D3DSurface *        Surface;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Render::MemoryManager*  pMemManager;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
        const ImageSize& size, unsigned use, ImageBase* pimage, Render::MemoryManager* pmemManager = 0);
    Texture(TextureManagerLocks* pmanagerLocks, IDirect3DTexture9* ptexture, const ImageSize& size, ImageBase* pimage,
        Render::MemoryManager* pmemManager, bool useRefCount = true);
    ~Texture();

    TextureManager*         GetManager() const     { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const        { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(IDirect3DTexture9* ptexture);
    void                    ReleaseHWTextures(bool staging = true);

    // Applies a texture to device starting at pstageIndex, advances index
    // TBD: Texture matrix may need to be adjusted if image scaling is done.
    virtual void            ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    // *** Interface implementation
    virtual Image*                  GetImage() const                        { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat     GetFormat() const                       { return GetImageFormat(); }
    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return pTextures[plane].Size; }
    const TextureFormat*    GetTextureFormat() const    { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const         { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

    virtual bool            UpdateRenderTargetData(Render::RenderTargetData* pdata, Render::HAL* phal = 0);
    virtual bool            UpdateStagingData(Render::RenderTargetData* pdata);

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

    IDirect3DSurface9 *       pDepthStencilSurface;
};

// *** MappedTexture
class MappedTexture : public MappedTextureBase
{
    friend class Texture;

public:
    MappedTexture() : MappedTextureBase() { }

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
};


// X360 Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
protected:
    friend class Texture;
    friend class DepthStencilSurface;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<IDirect3DTexture9*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DTextureArray;
    typedef ArrayLH<IDirect3DSurface9*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DSurfaceArray;
    typedef List<DepthStencilSurface, Render::DepthStencilSurface> DepthStencilList;

    IDirect3DDevice9*   pDevice;
    Render::MemoryManager*      pMemoryManager;
    MappedTexture       MappedTexture0;

    // Lists protected by TextureManagerLocks::TextureMutex.
    DepthStencilList          DepthStencilInitQueue;
    D3DTextureArray           D3DTextureKillList;
    D3DTextureArray           D3DTextureReleaseList;
    D3DSurfaceArray           D3DSurfaceKillList;

    // Detecting redundant sampler/address setting.
public:
    static const int MaximumStages = 4;
protected:
    D3DTEXTUREADDRESS    AddressMode[MaximumStages];
    D3DTEXTUREFILTERTYPE FilterType[MaximumStages];
    IDirect3DTexture9*   CurrentTextures[MaximumStages];

    // Detects supported D3DFormats and capabilities.
    void                         initTextureFormats();
    virtual void                 unmapTexture(Render::Texture *ptexture, bool = true);    
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }
    
    virtual void    processTextureKillList();
    virtual void    processInitTextures();    

public:
    TextureManager(IDirect3DDevice9* pdevice, Render::MemoryManager* pmemmanager,
                   ThreadId renderThreadId, ThreadCommandQueue* commandQueue, TextureCache* texCache = 0 );
    ~TextureManager();

    Render::MemoryManager* GetMemoryManager() { return pMemoryManager; }

	// Used once texture manager is no longer necessary.
	void    Reset();

    bool            IsMultiThreaded() const    { return RenderThreadId != 0; }
    void            SetSamplerState(unsigned stage, IDirect3DTexture9* ptexture,
                                    D3DTEXTUREFILTERTYPE filter = D3DTEXF_POINT, 
                                    D3DTEXTUREADDRESS address = D3DTADDRESS_CLAMP);

    virtual void    BeginScene();

    // *** TextureManager
    virtual unsigned         GetTextureFormatSupport() const { return ImageFormats_DXT; }

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(IDirect3DTexture9* ptexture,
                                           ImageSize imgSize = ImageSize(0), ImageBase* pimage = 0,
                                           Render::MemoryManager* manager = 0,
                                           bool useRefCount = true);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
 
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                                   Render::MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(IDirect3DSurface9* psurface);

    // In D3D9, R8G8B8A8 is not mappable, so use B8G8R8A8 which is.
    virtual ImageFormat     GetDrawableImageFormat() const { return Image_X360_R8G8B8A8; }
    virtual bool			IsDrawableImageFormat(ImageFormat format) const { return (format == Image_X360_R8G8B8A8); }

    virtual Render::ImageSwizzler& GetImageSwizzler() const;
};


}}};  // namespace Scaleform::Render::X360

#endif
