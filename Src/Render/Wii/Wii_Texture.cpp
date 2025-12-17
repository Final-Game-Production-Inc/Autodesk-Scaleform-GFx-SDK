/**************************************************************************

Filename    :   Wii_Texture.cpp
Content     :   Wii Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Wii_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"

#include <revolution/gx.h>

namespace Scaleform { namespace Render { namespace Wii {

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

Texture::Texture(TextureManagerLocks* pmanagerLocks, const GXTexObj* tex, const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0)
{
    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    pTextures[0].pTexData = 0;
    pTextures[0].TexSize = 0;
    pTextures[0].Tex = *tex;
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

bool Texture::Initialize()
{    
    if (TextureFlags & TF_UserAlloc )
    {
        return Initialize(&pTextures[0].Tex);
    }

    bool            resize  = false;
    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    // Determine sizes of textures.
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        ImageSize size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);

        tdesc.Width = size.Width;
        tdesc.Height = size.Height;

        if (!pmanager->IsNonPow2Supported(format, Use) || MipLevels > 1)
        {
            ImageSize roundedSize = ImageSize_RoundUpPow2(size);
            if (roundedSize != size)
            {
                tdesc.Width = roundedSize.Width;
                tdesc.Height = roundedSize.Height;
                resize = true;

                if (tdesc.Width > 512)
                    tdesc.Width = tdesc.Width >= 768 ? 1024 : 512;
                if (tdesc.Height > 512)
                    tdesc.Height = tdesc.Height >= 768 ? 1024 : 512;
            }
        }
        else
        {
            if (tdesc.Width > 1024)
                tdesc.Width = 1024;
            if (tdesc.Height > 1024)
                tdesc.Height = 1024;
        }
    }

    // XXX padding to power of 2 not supported in core yet
    if (resize /*&& (Use & ImageUse_Wrap)*/)
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

    // Determine how many mipLevels we should have and whether we can
    // auto-generate them or not.
    unsigned allocMipLevels = MipLevels;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        TextureFlags |= TF_SWMipGen;
        // If using SW MipGen, determine how many mip-levels we should have.
        allocMipLevels = 31;
        for (itex = 0; itex < TextureCount; itex++)
            allocMipLevels = Alg::Min(allocMipLevels,
                                      ImageSize_MipLevelCount(ImageSize(pTextures[itex].Width, pTextures[itex].Height)));
        MipLevels = (UByte)allocMipLevels;
    }

    if (Use & ImageUse_RenderTarget)
    {
        ImgSize.Width = pTextures[0].Width;
        ImgSize.Height = pTextures[0].Height;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        tdesc.TexSize = 0;

        unsigned h = tdesc.Height;
		unsigned w = tdesc.Width;

        tdesc.TexSize = GXGetTexBufferSize(w,h, GetTextureFormatMapping()->NativeFormat, MipLevels > 1, MipLevels);
        tdesc.pTexData = (UByte*)pmanager->pMemManager->Alloc(tdesc.TexSize, 32, Memory_Wii_MEM1);
        if (!tdesc.pTexData)
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            // Texture creation failed, release all textures and fail.
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }

        // set alpha, it won't be set in Update
        if (pFormat->GetImageFormat() == Image_R8G8B8 || pFormat->GetImageFormat() == Image_B8G8R8)
            memset(tdesc.pTexData, 255, tdesc.TexSize);

        if (MipLevels > 1)
        {
            GXInitTexObj(&tdesc.Tex, tdesc.pTexData, w, h, GetTextureFormatMapping()->NativeFormat, GX_CLAMP, GX_CLAMP, GX_TRUE);
            GXInitTexObjLOD(&tdesc.Tex, GX_LIN_MIP_LIN, GX_LINEAR, 0, MipLevels-1, -0.5f,
                GX_FALSE, GX_FALSE, GX_ANISO_1);
        }
        else
            GXInitTexObj(&tdesc.Tex, tdesc.pTexData, w, h, GetTextureFormatMapping()->NativeFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);
    }

    // Upload image content to texture, if any.
    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture");
        ReleaseHWTextures();
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

bool Texture::Initialize(const GXTexObj* tex)
{
    MipLevels = GXGetTexObjMipMap(tex) ? 2 : 1;

    // If an image is provided, try to obtain the texture format from the image.
    pFormat = 0;
    if ( pImage )
        pFormat = (TextureFormat*)GetManager()->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table.    
    if ( pFormat == 0 )
    {
        GXTexFmt fmt = GXGetTexObjFmt(tex);
        TextureFormat::Mapping* pmapping;
        for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
        {
            if ( pmapping->NativeFormat == fmt )
            {
                pFormat = (TextureFormat*)GetManager()->getTextureFormat(pmapping->Format);
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

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = ImageSize(pTextures[0].Width, pTextures[0].Height);

    pTextures[0].Tex = *tex;

    State = State_Valid;
    return Render::Texture::Initialize();
}


void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat format, ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    rescaleBuffFromat = pmapping->Format;
    rescaleType = ResizeNone;

    if (rescale)
    {
        if (pmapping->Format == Image_R8G8B8 || pmapping->Format == Image_B8G8R8)
            rescaleType = ResizeRgbToRgb;
        else if (pmapping->BytesPerPixel == 4)
            rescaleType = ResizeRgbaToRgba;
        else if (pmapping->BytesPerPixel == 1)
            rescaleType = ResizeGray;
        else
        {
            rescaleBuffFromat = Image_R8G8B8A8;
            convert = true;
        }
    }
    if (swMipGen && !(format == Image_R8G8B8A8 || format == Image_B8G8R8A8 || format == Image_A8))
        convert = true;
}


void Texture::ReleaseHWTextures(bool)
{
    Render::Texture::ReleaseHWTextures();
    TextureManager* pmanager = GetManager();

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        if (pTextures[itex].pTexData)
            pmanager->pMemManager->Free(pTextures[itex].pTexData, pTextures[itex].TexSize);
        pTextures[itex].pTexData = 0;
    }
}

void Texture::ApplyTexture(unsigned stage, const ImageFillMode& fillMode)
{
    Render::Texture::ApplyTexture(stage, fillMode);

    for (unsigned plane = 0; plane < GetTextureStageCount(); plane++)
    {
        int       stageIndex = stage + plane;
        GXTexObj* pwiitex = &pTextures[plane].Tex;

        if (fillMode.GetWrapMode() == Wrap_Clamp)
            GXInitTexObjWrapMode(pwiitex, GX_CLAMP, GX_CLAMP);
        else
            GXInitTexObjWrapMode(pwiitex, GX_REPEAT, GX_REPEAT);

        GXLoadTexObj(pwiitex, (GXTexMapID)(GX_TEXMAP0 + stageIndex));
    }
}

bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    ImageFormat format = GetImageFormat(); 
    ImagePlane  dplane;
    
    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);
        HWTextureDesc    &tex = pTextures[desc.PlaneIndex];
        int               level = 0; // XXX
        int               bpp = pmapping->BytesPerPixel;

        UByte *plevel = (UByte*)tex.pTexData;

        int w = tex.Width, h = tex.Height;
        int tw, th;
        int size;
        
        if (bpp)
        {
            tw = (bpp == 1 ? (w+7)&~7 : (w+3)&~3); th = (h+3)&~3;
            for (int i = 0; i < level; i++)
            {
                plevel += bpp * tw * th;
                w >>= 1;
                if (w < 1)
                    w = 1;
                h >>= 1;
                if (h < 1)
                    h = 1;
                tw = (bpp == 1 ? (w+7)&~7 : (w+3)&~3);
                th = (h+3)&~3;
            }
        }
        else
        {
            size = ((w+7)>>3) * ((h+7)>>3) * 32;

            for (int i = 0; i < level; i++)
            {
                plevel += size;
                w >>= 1;
                if (w < 1)
                    w = 1;
                h >>= 1;
                if (h < 1)
                    h = 1;
                size = ((w+7)>>3) * ((h+7)>>3) * 32;
            }
        }

        if (bpp == 4 && format == Image_R8G8B8A8)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw*4 + (x&~3)*16 + (y&3)*8 + (x&3)*2;

                    SF_ASSERT(ofs+33 < tw*th*bpp);
                    SF_ASSERT(plevel-tex.pTexData+ofs+33 <= tex.TexSize);

                    plevel[ofs+ 0] = splane.pData[splane.Pitch * j + 4 * k+3];
                    plevel[ofs+ 1] = splane.pData[splane.Pitch * j + 4 * k+0];
                    plevel[ofs+32] = splane.pData[splane.Pitch * j + 4 * k+1];
                    plevel[ofs+33] = splane.pData[splane.Pitch * j + 4 * k+2];
                }
        }
        else if (bpp == 4 && format == Image_R8G8B8)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw*4 + (x&~3)*16 + (y&3)*8 + (x&3)*2;

                    plevel[ofs+ 1] = splane.pData[splane.Pitch * j + 3 * k+0];
                    plevel[ofs+32] = splane.pData[splane.Pitch * j + 3 * k+1];
                    plevel[ofs+33] = splane.pData[splane.Pitch * j + 3 * k+2];
                }
        }
        else if (bpp == 4 && format == Image_B8G8R8A8)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw*4 + (x&~3)*16 + (y&3)*8 + (x&3)*2;

                    SF_ASSERT(ofs+33 < tw*th*bpp);
                    SF_ASSERT(plevel-tex.pTexData+ofs+33 <= tex.TexSize);

                    plevel[ofs+ 0] = splane.pData[splane.Pitch * j + 4 * k+3];
                    plevel[ofs+ 1] = splane.pData[splane.Pitch * j + 4 * k+2];
                    plevel[ofs+32] = splane.pData[splane.Pitch * j + 4 * k+1];
                    plevel[ofs+33] = splane.pData[splane.Pitch * j + 4 * k+0];
                }
        }
        else if (bpp == 4 && format == Image_B8G8R8)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw*4 + (x&~3)*16 + (y&3)*8 + (x&3)*2;

                    plevel[ofs+ 1] = splane.pData[splane.Pitch * j + 3 * k+2];
                    plevel[ofs+32] = splane.pData[splane.Pitch * j + 3 * k+1];
                    plevel[ofs+33] = splane.pData[splane.Pitch * j + 3 * k+0];
                }
        }
        else if (bpp == 2)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw*2 + (x&~3)*8 + (y&3)*8 + (x&3)*2;

                    plevel[ofs + 0] = splane.pData[splane.Pitch * j + 2 * k];
                    plevel[ofs + 1] = splane.pData[splane.Pitch * j + 2 * k + 1];
                }
        }
        else if (bpp == 1)
        {
            for (int j = 0; j < desc.DestRect.Height(); j++)
                for (int k = 0; k < desc.DestRect.Width(); k++)
                {
                    int x = k + desc.DestRect.x1;
                    int y = j + desc.DestRect.y1;
                    int ofs = (y&~3)*tw + (x&~7)*4 + (y&3)*8 + (x&7);

                    plevel[ofs] = splane.pData[splane.Pitch * j + k];
                }
        }
        else if (bpp == 0 && format == Image_DXT1 && desc.DestRect.Width() == tex.Width && desc.DestRect.Height() == tex.Height)
        {
            unsigned sw = (w + 3) >> 2;
            unsigned sh = (h + 3) >> 2;
            UByte*   pdest = plevel;

            for (int j = 0; j < sh; j += 2)
                for (int i = 0; i < sw; i += 2, pdest += 32)
                {
                    for (int jt = 0; jt < 2 && j + jt < sh; jt++)
                        for (int it = 0; it < 8 && i + it < sw; it += 4)
                        {
                            unsigned src = i*8+it*2+(j+jt)*sw*8;
                            unsigned dst = jt*16+it*2;

                            pdest[dst+0] = splane.pData[src+1];
                            pdest[dst+1] = splane.pData[src+0];
                            pdest[dst+2] = splane.pData[src+3];
                            pdest[dst+3] = splane.pData[src+2];

                            pdest[dst+ 8] = splane.pData[src+ 9];
                            pdest[dst+ 9] = splane.pData[src+ 8];
                            pdest[dst+10] = splane.pData[src+11];
                            pdest[dst+11] = splane.pData[src+10];

                            for (int c = 4; c < 8; c++)
                            {
                                int v = splane.pData[src+c];
                                pdest[dst+c] = ((v & 3) << 6) | ((v & 12) << 2) | ((v & 0x30) >> 2) | ((v & 0xc0) >> 6);

                                v = splane.pData[src+c+8];
                                pdest[dst+c+8] = ((v & 3) << 6) | ((v & 12) << 2) | ((v & 0x30) >> 2) | ((v & 0xc0) >> 6);
                            }
                        }
                }
        }
        else
            SF_ASSERT(0);

        if (bpp)
            DCFlushRange(plevel, bpp*tw*th);
        else
            DCFlushRange(plevel, size);
    }

    GXInvalidateTexAll();
    return true;
}

Scaleform::UPInt Texture::GetBytes( int* memRegion ) const
{
    if (memRegion)
        *memRegion = 1;
    UPInt size = 0;
    for (int i = 0; i < TextureCount; i++)
        size += pTextures[0].TexSize;
    return size;
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

    Texture* wiiTexture = reinterpret_cast<Texture*>(ptexture);
    pTexture            = ptexture;
    StartMipLevel       = mipLevel;
    LevelCount          = levelCount;

    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = wiiTexture->pTextures[itex];
        ImagePlane              plane(tdesc.Width, tdesc.Height, tdesc.Width * wiiTexture->GetTextureFormatMapping()->BytesPerPixel);
        UByte*                  pdata = tdesc.pTexData;

        for(unsigned i = 0; i < StartMipLevel; i++)
        {
            pdata += plane.Pitch * plane.Height;
            plane.SetNextMipSize();
        }

        for (unsigned level = 0; level < levelCount; level++)
        {
            plane.pData = pdata;
            plane.DataSize = plane.Height * plane.Pitch;
            Data.SetPlane(level * textureCount + itex, plane);

            // Prepare for next level.
            pdata += plane.DataSize;
            plane.SetNextMipSize();
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool)
{
    MappedTextureBase::Unmap();
    GXInvalidateTexAll();
}


// ***** TextureManager

TextureManager::TextureManager(Render::MemoryManager* pmm, ThreadId renderThreadId, 
                               ThreadCommandQueue* commandQueue, TextureCache* texCache) : 
    Render::TextureManager(renderThreadId, commandQueue, texCache),
    pMemManager(pmm)
{
    initTextureFormats();
}

TextureManager::~TextureManager()
{   
    Mutex::Locker lock(&pLocks->TextureMutex);

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    pLocks->pManager = 0;
}

// ***** Wii Format mapping and conversion functions

static TextureFormat::Mapping TextureFormatMapping[] = 
{
    { Image_Wii_R8G8B8A8,    GX_TF_RGBA8,   false, 4, true,  &Image::CopyScanlineDefault },
    { Image_Wii_A8,          GX_TF_I8,      true,  1, true,  &Image::CopyScanlineDefault },

    { Image_R8G8B8A8,        GX_TF_RGBA8,   false, 4, false, &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,        GX_TF_RGBA8,   false, 4, false, &Image::CopyScanlineDefault },

    { Image_R8G8B8,          GX_TF_RGBA8,   false, 4, false, &Image::CopyScanlineDefault },
    { Image_B8G8R8,          GX_TF_RGBA8,   false, 4, false, &Image::CopyScanlineDefault },

    { Image_A8,              GX_TF_I8,      true,  1, false, &Image::CopyScanlineDefault },

    { Image_DXT1,            GX_TF_CMPR,    false, 0, false, &Image::CopyScanlineDefault },

    { Image_None,            GX_TF_I8,      false, 0, false, 0 }
};

void TextureManager::initTextureFormats()
{
    TextureFormat::Mapping* pmapping;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping);
        TextureFormats.PushBack(tf);
    }
}

void TextureManager::unmapTexture(Render::Texture *ptexture, bool)
{
    Render::TextureManager::unmapTexture(ptexture);
    Texture* wiiTexture  = reinterpret_cast<Texture*>(ptexture);

    DCFlushRange(wiiTexture->pTextures[0].pTexData, wiiTexture->pTextures[0].TexSize);
    GXInvalidateTexAll();
}

void TextureManager::processInitTextures()
{
    // TextureMutex lock expected externally.
    //Mutex::Locker lock(&TextureMutex);
   
    if (!TextureInitQueue.IsEmpty())
    {
        while (!TextureInitQueue.IsEmpty())
        {
            Render::Texture* ptexture = TextureInitQueue.GetFirst();
            ptexture->RemoveNode();
            ptexture->pPrev = ptexture->pNext = 0;
            if (ptexture->Initialize())
                Textures.PushBack(ptexture);
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

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(const GXTexObj* tex, ImageSize size, ImageBase * pimage)
{
    Texture* ptexture = SF_HEAP_AUTO_NEW(this) Texture(pLocks, tex, size, pimage);
    return postCreateTexture(ptexture, 0);
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    // ImageUse_InitOnly is ok while textures are Managed
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

bool TextureManager::IsNonPow2Supported(ImageFormat, UInt16 use)
{
    if (use & ImageUse_Wrap)
        return false;
    return true;
}

}}};  // namespace Scaleform::Render::Wii

