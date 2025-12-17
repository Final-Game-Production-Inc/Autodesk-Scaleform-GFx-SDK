/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Texture.h
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_Texture_H
#define INC_SF_PS4_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Kernel/SF_HeapNew.h"

#include "Render/PS4/PS4_Config.h"
#include "Render/PS4/PS4_Sync.h"

#include <gnm/texture.h>

namespace Scaleform { namespace Render { namespace PS4 {

// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        sce::Gnm::DataFormat     DataFormat;
        Image::CopyScanlineFunc  CopyFunc;
        Image::CopyScanlineFunc  UncopyFunc;
    };
    const Mapping*  pMapping;

    TextureFormat(const Mapping* mapping = 0) : pMapping(mapping) { }

    virtual ImageFormat     GetImageFormat() const              { return pMapping ? pMapping->Format : Image_None; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping ? pMapping->CopyFunc : 0; }
    virtual Image::CopyScanlineFunc GetScanlineUncopyFn() const { return pMapping ? pMapping->UncopyFunc : 0; }
};

class MappedTexture;
class TextureManager;

// Orbis Texture class implementation; it many actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    // Bits stored in TextureFlags.
    enum TextureFlagBits
    {
        TF_Rescale    = 0x01,
        TF_SWMipGen   = 0x02,
        TF_UserAlloc  = 0x04,
    };

    static const UByte      MaxTextureCount = 4;
    
    struct HWTextureDesc
    {        
        unsigned                ByteSize;
        sce::Gnm::Texture       Tex;
        sce::Gnm::RenderTarget  Surface;
        void*                   pTexData;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, const sce::Gnm::Texture& tex, const ImageSize& imgSize, ImageBase* pimage);
    ~Texture();

    ImageFormat             GetImageFormat() const                  { return GetTextureFormatMapping()->Format; }
    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return ImgSize; }
    TextureManager*         GetManager() const                      { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const                         { return pTextures != 0; }

    void                    LoseManager();
    bool                    Initialize();
    bool                    Initialize(const sce::Gnm::Texture& tex);
    void                    ReleaseHWTextures(bool staging = true);

    // Applies a texture to device starting at pstageIndex, advances index
    // TBD: Texture matrix may need to be adjusted if image scaling is done.
    void                    ApplyTexture(unsigned stageIndex, const ImageFillMode& fm);

    // *** Interface implementation

    virtual Image*                  GetImage() const                { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat             GetFormat() const               { return GetImageFormat(); }
    const TextureFormat*            GetTextureFormat() const        { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual bool            Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount);
    virtual bool            Unmap();

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );

};

// Orbis DepthStencilSurface implementation. 
class DepthStencilSurface : public Render::DepthStencilSurface
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, Render::MemoryManager* pmanager, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();

    sce::Gnm::DepthRenderTarget     DepthTarget;
    Render::MemoryManager*          pMemManager;
    Texture::CreateState            State;
    sce::Gnm::SizeAlign             DepthSize, StencilSize;
};

// MappedTexture object repents a Texture mapped into memory with Texture::Map() call;
// it is also used internally during updates.
// The key part of this class is the Data object, stored Locked texture level plains.

class MappedTexture : public MappedTextureBase
{
    friend class Texture;

public:
    MappedTexture() : MappedTextureBase() { }

    virtual bool Map(Render::Texture* ptexture, unsigned mipLevel, unsigned levelCount);
};




class TextureManager : public Render::TextureManager
{
    friend class Texture;

    sceGnmxContextType*     GnmxCtx;
    Render::MemoryManager*  pMemManager;
    MappedTexture           MappedTexture0;
   
    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }

    // Texture Memory-mapping support.
    virtual MappedTextureBase*  mapTexture(Render::Texture* p, unsigned mipLevel, unsigned levelCount);
    virtual void                unmapTexture(Render::Texture *ptexture, bool applyUpdate = true);    
    
public:
    TextureManager(sceGnmxContextType* context, Render::MemoryManager* pmm);
    ~TextureManager();

    virtual void            SetGfxContext(sceGnmxContextType* context) { GnmxCtx = context; }
    virtual void            Reset() { } 

    Render::MemoryManager* GetMemoryManager() { return pMemManager; }

    // *** TextureManager
    virtual unsigned         GetTextureFormatSupport() const { return ImageFormats_DXT; }

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(const sce::Gnm::Texture& tex,
                                           ImageSize size = ImageSize(0), ImageBase * pimage = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);

    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                                   Render::MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const sce::Gnm::DepthRenderTarget& surface);

    virtual ImageFormat     GetDrawableImageFormat() const { return Image_R8G8B8A8; }
    virtual bool			IsDrawableImageFormat(ImageFormat format) const { return (format == Image_R8G8B8A8); }

};

}}};  // namespace Scaleform::Render::D3D1x

#endif // INC_SF_Orbis_Texture_H
