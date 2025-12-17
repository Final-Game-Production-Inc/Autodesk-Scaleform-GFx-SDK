/**************************************************************************

Filename    :   WiiU_Texture.h
Content     :   WiiU Texture and TextureManager header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_WiiU_Texture_H
#define INC_SF_Render_WiiU_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/WiiU/WiiU_Sync.h"
#include "Kernel/SF_HeapNew.h"

#include <cafe/gx2.h>

namespace Scaleform { namespace Render { namespace WiiU {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        ImageFormat              ConvFormat;
        GX2SurfaceFormat         NativeFormat;
        GX2TileMode              NativeSwizzle;
        GX2CompSel               NativeRemap;
        UByte                    BytesPerPixel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    const Mapping*  pMapping;

    TextureFormat(TextureFormat::Mapping* pmapping = 0) : pMapping(pmapping) { }

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }
};

class MappedTexture;
class TextureManager;


// WiiU Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    struct HWTextureDesc
    {        
        GX2Texture          Tex;
        UByte*              pTexBaseData;
        UByte*              pTexMipData;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks,
            const GX2Texture& tex, const ImageSize& size, ImageBase* pimage);
    ~Texture();

    TextureManager*         GetManager() const     { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const        { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(const GX2Texture& tex);
    void                    ReleaseHWTextures(bool staging = true);
    virtual void            ApplyTexture(unsigned stage, const ImageFillMode& fillMode);

    // *** Interface implementation
    virtual Image*                GetImage() const                        { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat           GetFormat() const                       { return GetImageFormat(); }
    virtual ImageSize             GetTextureSize(unsigned plane =0) const { return ImgSize; }
    const TextureFormat*          GetTextureFormat() const                { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping* GetTextureFormatMapping() const         { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual bool            UpdateStagingData(Render::RenderTargetData* prt);
    virtual bool            UpdateRenderTargetData(Render::RenderTargetData* prt, Render::HAL* =0);
    virtual void            SyncTextureData(Render::RenderTargetData* prt);

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
};

// WiiU DepthStencilSurface implementation. 
class DepthStencilSurface : public Render::DepthStencilSurface, public ListNode<DepthStencilSurface>
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();

    Render::MemoryManager*    pMemManager;
    GX2DepthBuffer            DepthBuffer;
    void*                     pSurfaceMemory;
};

// *** MappedTexture
class MappedTexture : public MappedTextureBase
{
    friend class Texture;

public:
    MappedTexture() : MappedTextureBase() { }

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
    virtual void Unmap(bool = true);
};


// WiiU Texture Manager.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;
    typedef List<Texture, Render::Texture>  TextureList;

    Render::MemoryManager*  pMemManager;
    RenderSync*             RSync;
    MappedTexture           MappedTexture0;

    // Lists protected by TextureManagerLocks::TextureMutex.
    TextureList             Textures;

    GX2Sampler          Samplers[SamplerStateObjectCount];

    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }
    
public:
    TextureManager(Render::MemoryManager* pmm, RenderSync* rsync, TextureCache* texCache = 0);
    ~TextureManager();

    Render::MemoryManager* GetMemoryManager() { return pMemManager; }

    // *** TextureManager
    virtual bool             CanCreateTextureCurrentThread() const { return true; }
    virtual unsigned         GetTextureFormatSupport() const { return ImageFormats_GTX; }

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(const GX2Texture& tex,
                                           ImageSize size = ImageSize(0), ImageBase * pimage = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
 
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                                   Render::MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const GX2DepthBuffer& surface);
};


}}};  // namespace Scaleform::Render::WiiU

#endif
