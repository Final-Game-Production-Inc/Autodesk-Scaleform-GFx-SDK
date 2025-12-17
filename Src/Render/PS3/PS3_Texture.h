/**************************************************************************

Filename    :   PS3_Texture.h
Content     :   PS3 Texture and TextureManager header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_PS3_Texture_H
#define INC_SF_Render_PS3_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/PS3/PS3_MemoryManager.h"
#include "Render/PS3/PS3_Sync.h"
#include "Kernel/SF_HeapNew.h"

#include <cell/gcm.h>

namespace Scaleform { namespace Render { namespace PS3 {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        ImageFormat              ConvFormat;
        UInt32                   NativeFormat;
        UInt32                   NativeRemap;
        UByte                    BytesPerPixel;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };

    const Mapping*  pMapping;

    TextureFormat(const Mapping* mapping = 0) : pMapping(mapping) { }

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping->UncopyFunc; }
};

class MappedTexture;
class TextureManager;

// PS3 Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    struct HWTextureDesc
    {        
        unsigned            Size;
        UByte*              pTexData;
        CellGcmTexture      Tex;
		unsigned			Pitch; // for swizzle formats
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;
    bool                    NeedsCacheFlush;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks,
            const CellGcmTexture& tex, const ImageSize& size, ImageBase* pimage);
    ~Texture();

    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return ImgSize; }
    TextureManager*         GetManager() const                      { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const                         { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(const CellGcmTexture& tex);
    void                    ReleaseHWTextures(bool staging = true);
    virtual void            ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    // *** Interface implementation
    virtual Image*                GetImage() const                { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat           GetFormat() const               { return GetImageFormat(); }
    const TextureFormat*          GetTextureFormat() const        { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping* GetTextureFormatMapping() const { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

    virtual bool            UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL* =0);
    virtual bool            UpdateStagingData(Render::RenderTargetData*);

    virtual UPInt   GetBytes(int* memRegion) const;

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
};

// PS3 DepthStencilSurface implementation. 
class DepthStencilSurface : public Render::DepthStencilSurface, public ListNode<DepthStencilSurface>
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();

    MemoryManager*            pMemManager;
    CellGcmSurface            Surface;       // Holds only depth target information, not color buffer info.
    void*                     pSurfaceMemory;
};

// *** MappedTexture
class MappedTexture : public MappedTextureBase
{
    friend class Texture;

public:
    MappedTexture() : MappedTextureBase() { }

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
};

class ImageSwizzler : public Render::ImageSwizzler
{
public:
    ImageSwizzler(ImageData *d);

    virtual void CacheScanline(unsigned y);

    virtual void SetPixelInScanline(unsigned x, Color c)
    {
        SetPixelInScanline(x, c.ToColor32());
    }

    virtual void SetPixelInScanline(unsigned x, UInt32 c);

    virtual Color GetPixelInScanline(unsigned x);

protected:
    unsigned CachedBlockY;
    const int *CachedBlockLine;
    ImagePlane *pPlane;
};


// PS3 Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;
    typedef List<Texture, Render::Texture>  TextureList;

    CellGcmContextData*     GcmCtx;
    Render::MemoryManager*  pMemManager;
    RenderSync*             RSync;
    MappedTexture           MappedTexture0;

    // Lists protected by TextureManagerLocks::TextureMutex.
    TextureList             Textures;

    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }
    
public:
    TextureManager(CellGcmContextData* gcm, Render::MemoryManager* pmm, RenderSync* rsync, TextureCache* texCache = 0);
    ~TextureManager();

    Render::MemoryManager* GetMemoryManager() { return pMemManager; }

    // *** TextureManager
    virtual bool             CanCreateTextureCurrentThread() const { return true; }
    virtual unsigned         GetTextureFormatSupport() const { return ImageFormats_DXT; }

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(const CellGcmTexture& tex,
                                           ImageSize size = ImageSize(0), ImageBase * pimage = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
 
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                                   Render::MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const CellGcmSurface& surface);

    virtual ImageFormat     GetDrawableImageFormat() const { return Image_PS3_A8R8G8B8; }
    virtual bool			IsDrawableImageFormat(ImageFormat format) const { return (format == Image_PS3_A8R8G8B8); }

};


}}};  // namespace Scaleform::Render::PS3

#endif
