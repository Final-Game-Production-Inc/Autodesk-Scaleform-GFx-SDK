/**************************************************************************

Filename    :   Wii_Texture.h
Content     :   Wii Texture and TextureManager header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Wii_Texture_H
#define INC_SF_Render_Wii_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Wii/Wii_MemoryManager.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace Wii {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format;
        GXTexFmt                 NativeFormat;
        bool                     IsAlpha;
        UByte                    BytesPerPixel;
        bool                     ScanlineCompatible;
        Image::CopyScanlineFunc  CopyFunc;
    };

    const Mapping*  pMapping;

    TextureFormat(TextureFormat::Mapping* pmapping = 0) : pMapping(pmapping) { }

    virtual ImageFormat             GetImageFormat() const      { return pMapping->Format; }
    virtual Image::CopyScanlineFunc GetScanlineCopyFn() const   { return pMapping->CopyFunc; }
};

class MappedTexture;
class TextureManager;


// Wii Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    struct HWTextureDesc
    {
        unsigned            Width, Height;
        UPInt               TexSize; // size of texture
        GXTexObj            Tex;
        UByte*              pTexData;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks,
            const GXTexObj* tex, const ImageSize& size, ImageBase* pimage);
    ~Texture();

    TextureManager*         GetManager() const     { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const        { return pTextures != 0; }

    bool                    Initialize();
    bool                    Initialize(const GXTexObj* tex);
    void                    ReleaseHWTextures(bool staging = true);
    virtual void            ApplyTexture(unsigned stage, const ImageFillMode& fillMode);

    void                    BindTexture(unsigned stageIndex, ImageFillMode fm);

    // *** Interface implementation
    virtual Image*                GetImage() const                        { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat           GetFormat() const                       { return GetImageFormat(); }
    virtual ImageSize             GetTextureSize(unsigned plane =0) const { return ImgSize; }
    const TextureFormat*          GetTextureFormat() const                { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping* GetTextureFormatMapping() const         { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }
    
    virtual bool                  Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    
    virtual UPInt                 GetBytes(int* memRegion) const;

public:
    virtual void computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                   ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
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

// Wii Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;

    Render::MemoryManager*    pMemManager;
    MappedTexture             MappedTexture0;
    
    virtual void                 processInitTextures();

    void                         initTextureFormats();
    virtual void                 unmapTexture(Render::Texture *ptexture, bool applyUpdate = true);
    virtual bool                 isScanlineCompatible(const Render::TextureFormat* ptformat) { return ((TextureFormat*)ptformat)->pMapping->ScanlineCompatible; }
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }
    
public:
    TextureManager(Render::MemoryManager* pmm, 
                   ThreadId renderThreadId = 0, 
                   ThreadCommandQueue* commandQueue = 0, 
                   TextureCache* texCache = 0);
    ~TextureManager();

    bool IsMultiThreaded() const    { return RenderThreadId != 0; }

    Render::MemoryManager* GetMemoryManager() { return pMemManager; }

    // *** TextureManager
    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(const GXTexObj* tex,
                                           ImageSize size = ImageSize(0), ImageBase * pimage = 0);

    virtual DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                            Render::MemoryManager* manager = 0) 
    { SF_UNUSED2(size, manager); return 0; }

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
    bool                    IsNonPow2Supported(ImageFormat format, UInt16 use);
};


}}};  // namespace Scaleform::Render::Wii

#endif
