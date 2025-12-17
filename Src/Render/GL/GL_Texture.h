/**************************************************************************

Filename    :   GL_Texture.h
Content     :   GL Texture and TextureManager header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GL_Texture_H
#define INC_SF_Render_GL_Texture_H

#include "Render/GL/GL_Common.h"

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { 

class ThreadCommandQueue;

namespace GL {


// TextureFormat describes format of the texture and its caps.
// Format includes allowed usage capabilities and ImageFormat
// from which texture is supposed to be initialized.

struct TextureFormat : public Render::TextureFormat
{
    struct Mapping
    {
        ImageFormat              Format, ConvFormat;
        GLenum                   GLColors, GLFormat, GLData;
        UByte                    BytesPerPixel;
        const char*              Extension;
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
class HAL;


// GL Texture class implementation; it man actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    Ptr<RawImage>            pBackingImage;
    
    struct HWTextureDesc
    {
        ImageSize           Size;
        GLuint              TexId;
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    GLint                   LastMinFilter, LastAddress;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, GLuint texID, bool deleteTexture,
            const ImageSize& size, ImageBase* pimage);
    ~Texture();

    TextureManager*         GetManager() const     { return (TextureManager*)pManagerLocks->pManager; }
    inline  HAL*            GetHAL() const;
    virtual bool            IsValid() const        { return pTextures != 0; }

    virtual bool            Initialize();
    bool                    Initialize(GLuint texID);
    virtual void            ReleaseHWTextures(bool staging = true);
    virtual void            ApplyTexture(unsigned stage, const ImageFillMode& fillMode);

    // *** Interface implementation

    virtual Image*                  GetImage() const                        { SF_ASSERT(!pImage || (pImage->GetImageType() != Image::Type_ImageBase)); return (Image*)pImage; }
    virtual ImageFormat             GetFormat() const                       { return GetImageFormat(); }
    virtual ImageSize               GetTextureSize(unsigned plane =0) const { return pTextures[plane].Size; }
    const TextureFormat*            GetTextureFormat() const                { return reinterpret_cast<const TextureFormat*>(pFormat); }
    const TextureFormat::Mapping*   GetTextureFormatMapping() const         { return pFormat ? reinterpret_cast<const TextureFormat*>(pFormat)->pMapping : 0; }

    virtual bool            UpdateRenderTargetData(Render::RenderTargetData* prtData, Render::HAL* =0);
    virtual bool            UpdateStagingData(Render::RenderTargetData* prtData);

    virtual bool            Update(const UpdateDesc* updates, unsigned count = 1, unsigned mipLevel = 0);    

    virtual bool            Upload(unsigned itex, unsigned level, const ImagePlane& plane);
    virtual void            MakeMappable();

    // Copies the image data from the hardware.
    SF_AMP_CODE( virtual bool Copy(ImageData* pdata); )

protected:
    virtual void            computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                              ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert );
    virtual void            uploadImage(ImageData* psource);
};

// GL DepthStencilSurface implementation. 
class DepthStencilSurface : public Render::DepthStencilSurface
{
public:
    DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size);
    ~DepthStencilSurface();

    bool                            Initialize();
    inline HAL*                     GetHAL() const;

    GLuint                    RenderBufferID;

    // We can't query ahead of time which stencil format is supported. So, we have to attempt
    // creating them, and if they fail, try the next format. Returns false once there are no
    // more formats to try.
    static bool               SetNextGLFormatIndex();

    // Returns true of the current format has a packed depth component (and is not just stencil data).
    static bool               CurrentFormatHasDepth();

    // The list of GL formats we will try to use to allocate stencil buffers (in order of preference).
    static int                GLFormatIndex;
    static unsigned           GLStencilFormats[];
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


// GL Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;
    friend class DepthStencilSurface;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<GLuint,
        StatRender_TextureManager_Mem,
        KillListArrayPolicy>    GLTextureArray;

    enum TextureCaps
    {
        TC_NonPower2Limited = 0x01,
        TC_NonPower2Full    = 0x02,
        TC_NonPower2RT      = 0x04,
        TC_UseBgra          = 0x08,
		TC_UseAppleMaxLevel = 0x10,
    };

    MappedTexture       MappedTexture0;

    HAL*                     pHal;
    unsigned                 Caps;

    // Lists protected by TextureManagerLocks::TextureMutex.
    GLTextureArray            GLTextureKillList;
    GLTextureArray            GLDepthStencilKillList;
    GLTextureArray            GLFrameBufferKillList;

    // Detecting redundant texture bindings.
    static const int MaximumStages = 4;
    GLint                    CurrentTextures[MaximumStages];

    HAL* GetHAL() const { return pHal; }

    void                         initTextureFormats();
    static GLenum                getBaseTextureFormatFromInternal(GLenum intfmt);
    virtual MappedTextureBase&   getDefaultMappedTexture() { return MappedTexture0; }
    virtual MappedTextureBase*   createMappedTexture()     { return SF_HEAP_AUTO_NEW(this) MappedTexture; }

    virtual void    processInitTextures();
    virtual void    processTextureKillList();

public:
    TextureManager(ThreadId renderThreadId, ThreadCommandQueue* commandQueue, TextureCache* texCache = 0);
    ~TextureManager();

    // XXX - use Extensions instead if needed
    void            Initialize(HAL* phal);
    void            NotifyLostContext();
    void            RestoreAfterLoss();

    // Applies a texture to a stage.
    void            ApplyTexture(unsigned stageIndex, GLint texture);

    // Adds a FrameBuffer to the FBO kill list, or deletes it immediately if in RT.
    void            DestroyFBO(GLuint fboid);

    // *** TextureManager
    virtual unsigned         GetTextureFormatSupport() const;

    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size, unsigned use, 
                                           ImageBase* pimage = 0,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(GLuint texID, bool deleteTexture, 
                                           ImageSize imgSize = ImageSize(0),
                                           ImageBase* pimage = 0);

    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                           MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(GLuint id);

	virtual bool	IsDrawableImageFormat(ImageFormat format) const { return (format == Image_B8G8R8A8) || (format == Image_R8G8B8A8); }

	bool            IsMultiThreaded() { return false; }
    unsigned        GetTextureUseCaps(ImageFormat format);
    bool            IsNonPow2Supported(ImageFormat format, UInt16 use);

    // Should be called before each frame on RenderThread.
    virtual void    BeginFrame( );
};

GL::HAL* Texture::GetHAL() const
{
    return ((GL::TextureManager*)pManagerLocks->pManager)->pHal;
}

GL::HAL* DepthStencilSurface::GetHAL() const
{
    return ((GL::TextureManager*)pManagerLocks->pManager)->pHal;
}

}}};  // namespace Scaleform::Render::GL

#endif
