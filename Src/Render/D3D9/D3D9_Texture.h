/**************************************************************************

Filename    :   D3D9_Texture.h
Content     :   D3D9 Texture and TextureManager header
Created     :   January 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_D3D9_Texture_H
#define INC_SF_Render_D3D9_Texture_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_HeapNew.h"
#include <d3d9.h>

namespace Scaleform { namespace Render { 

class ThreadCommandQueue;

namespace D3D9 {


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

    // D3D9 Specific.
    D3DFORMAT               GetD3DFormat() const        { return pMapping->D3DFormat; }  
    virtual bool            CanAutoGenMipmaps() const   { return (D3DUsage & D3DUSAGE_AUTOGENMIPMAP) != 0; }
    virtual bool            CanBeDynamic() const        { return (D3DUsage & D3DUSAGE_DYNAMIC) != 0; }
};

class MappedTexture;
class TextureManager;

// D3D9 Texture class implementation; it many actually include several HW 
// textures (one for each ImageFormat plane).

class Texture : public Render::Texture
{
public:
    enum AllocType
    {
        Type_Normal, // Not used yet.
        Type_Managed,
        Type_Dynamic,
        Type_StagingBacked
    };
    AllocType               Type;
    
    struct HWTextureDesc
    {        
        ImageSize           Size;
        IDirect3DTexture9*  pTexture;
        // Same-size D3DPOOL_SYSTEMMEM texture allocated only for RenderTarget
        IDirect3DTexture9*  pStagingTexture; 
    };

    // TextureDesc array is allocated if more then one is needed.
    HWTextureDesc*          pTextures;
    HWTextureDesc           Texture0;

    Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat, unsigned mipLevels,
            const ImageSize& size, unsigned use, ImageBase* pimage);
    Texture(TextureManagerLocks* pmanagerLocks, IDirect3DTexture9* ptexture, 
            const ImageSize& size, ImageBase* pimage);
    ~Texture();

    virtual ImageSize       GetTextureSize(unsigned plane =0) const { return pTextures[plane].Size; }
    TextureManager*         GetManager() const                      { return (TextureManager*)pManagerLocks->pManager; }
    bool                    IsValid() const                         { return pTextures != 0; }

    void                    LoseManager();
    bool                    Initialize();
    bool                    Initialize(IDirect3DTexture9* ptexture);
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
    
    virtual bool            UpdateRenderTargetData(Render::RenderTargetData* prt, Render::HAL* =0);
    virtual bool            UpdateStagingData(Render::RenderTargetData* prt);

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

    IDirect3DSurface9 *       pDepthStencilSurface;
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


// D3D configuration flags, including both user configuration settings
// and available capabilities.

struct  D3DCapFlags
{
    UInt32 Flags;

    enum 
    {
        Cap_MultiThreaded    = 0x00000001,
        Cap_Managed          = 0x00000002,
        Cap_DynamicTex       = 0x00000004,
        Cap_AutoGenMipmaps   = 0x00000010,
        Cap_NonPow2Tex       = 0x00000040,
        Cap_NonPow2TexWrap   = 0x00000080,
        Cap_D3D9Ex           = 0x00000100,  // Using a D3D9Ex device.
    };

    bool IsMultiThreaded() const    { return (Flags & Cap_MultiThreaded) != 0; }
    bool IsManaged() const          { return (Flags & Cap_Managed) != 0; }
    bool DynamicTexAllowed() const  { return (Flags & Cap_DynamicTex) != 0; }
    bool IsD3D9Ex() const           { return (Flags & Cap_D3D9Ex) != 0; }

    bool NonPow2Supported(unsigned use)
    {
        UInt32 bit =(use & ImageUse_Wrap) ? Cap_NonPow2TexWrap : Cap_NonPow2Tex;
        return (Flags & bit) != 0;
    }

    // TBD: Add Multi-Threaded and other options in the future..
    //  We also need to combine HW capabilities with user-specified settings
    void InitFromHWCaps(const D3DCAPS9& caps, const D3DDEVICE_CREATION_PARAMETERS& cparams)
    {
        Flags = 0;
        if (!(caps.TextureCaps & D3DPTEXTURECAPS_POW2))
        {
            Flags |= Cap_NonPow2Tex;
            if (!(caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
                Flags |= Cap_NonPow2TexWrap;
        }
        else  if (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
        {
            Flags |= Cap_NonPow2Tex;
        }
        if (cparams.BehaviorFlags & D3DCREATE_MULTITHREADED)
        {
            Flags |= Cap_MultiThreaded;
        }
        if (caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES)
        {
            Flags |= Cap_DynamicTex;
        }
    }
};


// D3D9 Texture Manger.
// This class is responsible for creating textures and keeping track of them
// in the list.
// 

class TextureManager : public Render::TextureManager
{
    friend class Texture;
    friend class DepthStencilSurface;

    typedef ArrayConstPolicy<8, 8, false>   KillListArrayPolicy;
    typedef ArrayLH<IDirect3DTexture9*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DTextureArray;
    typedef ArrayLH<IDirect3DSurface9*,
                    StatRender_TextureManager_Mem,
                    KillListArrayPolicy>    D3DDepthStencilArray;

    IDirect3DDevice9*   pDevice;
    D3DCapFlags         Caps;
    MappedTexture       MappedTexture0;    

    // Lists protected by TextureManagerLocks::TextureMutex.
    D3DTextureArray         D3DTextureKillList;
    D3DDepthStencilArray    D3DDepthStencilKillList;

    // Detecting redundant sampler/address setting.
    static const int MaximumStages = 4;
    D3DTEXTUREADDRESS    AddressMode[MaximumStages];
    D3DTEXTUREFILTERTYPE FilterType[MaximumStages];
    IDirect3DTexture9*   CurrentTextures[MaximumStages];

    // Detects supported D3DFormats and capabilities.
    void                         initTextureFormats();
    virtual MappedTextureBase&   getDefaultMappedTexture();
    virtual MappedTextureBase*   createMappedTexture();

    virtual void    processTextureKillList();
    virtual void    processInitTextures();    
    
public:
    TextureManager(IDirect3DDevice9* pdevice,
                   D3DCapFlags caps,
                   ThreadId renderThreadId, 
                   ThreadCommandQueue* commandQueue,
                   TextureCache* texCache = 0);
    ~TextureManager();

    void    PrepareForReset();
    void    RestoreAfterReset();
    // Used once texture manager is no longer necessary.
    void    Reset();

    IDirect3DDevice9* GetDevice() const { return pDevice; }
    const D3DCapFlags& GetCaps() const { return Caps; }

    void            SetSamplerState( unsigned stage, IDirect3DTexture9* d3dtex, 
                                     D3DTEXTUREFILTERTYPE filter = D3DTEXF_POINT, 
                                     D3DTEXTUREADDRESS address = D3DTADDRESS_CLAMP);

    virtual void    BeginScene();

    // *** TextureManager
    virtual Render::Texture* CreateTexture(ImageFormat format, unsigned mipLevels,
                                           const ImageSize& size,
                                           unsigned use, ImageBase* pimage,
                                           Render::MemoryManager* manager = 0);
    virtual Render::Texture* CreateTexture(IDirect3DTexture9* pd3dtexture,
                                           ImageSize imgSize = ImageSize(0), Image* pimage = 0);

    virtual unsigned        GetTextureUseCaps(ImageFormat format);
 
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(const ImageSize& size,
                                                           MemoryManager* manager = 0);
    virtual Render::DepthStencilSurface* CreateDepthStencilSurface(IDirect3DSurface9* psurface);

    virtual bool            CanCreateTextureCurrentThread() const;

    // In D3D9, R8G8B8A8 is not mappable, so use B8G8R8A8 which is.
    virtual ImageFormat     GetDrawableImageFormat() const { return Image_B8G8R8A8; }

    virtual unsigned		GetTextureFormatSupport() const;

	virtual bool			IsDrawableImageFormat(ImageFormat format) const { return (format == Image_B8G8R8A8); }
};


}}};  // namespace Scaleform::Render::D3D9

#endif
