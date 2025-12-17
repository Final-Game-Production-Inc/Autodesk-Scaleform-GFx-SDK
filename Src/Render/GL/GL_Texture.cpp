/**************************************************************************

Filename    :   GL_Texture.cpp
Content     :   GL Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GL_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"

#if defined(SF_USE_GLES)
#include "Render/GL/GLES11_ExtensionMacros.h"
#elif defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ExtensionMacros.h"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#endif

namespace Scaleform { namespace Render { namespace GL {


extern TextureFormat::Mapping TextureFormatMapping[];

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage) :   
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat), 
    LastMinFilter(0), LastAddress(0)
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

Texture::Texture(TextureManagerLocks* pmanagerLocks, GLuint texID, 
                 bool deleteTexture, const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0), 
    LastMinFilter(0), LastAddress(0)
{
    TextureFlags |= TF_UserAlloc;
    if ( !deleteTexture )
        TextureFlags |= TF_DoNotDelete;

    pTextures = &Texture0;
    pTextures[0].Size = size;
    pTextures[0].TexId = texID;
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
    if ( TextureFlags & TF_UserAlloc )
    {
        return Initialize(pTextures[0].TexId);
    }

    bool            resize  = false;
    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    if (State != State_Lost)
    {
        // Determine sizes of textures.
        for (itex = 0; itex < TextureCount; itex++)
        {
            HWTextureDesc& tdesc = pTextures[itex];
            tdesc.Size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
            if (!pmanager->IsNonPow2Supported(format, Use))
            {
                ImageSize roundedSize = ImageSize_RoundUpPow2(tdesc.Size);
                if (roundedSize != tdesc.Size)
                {
                    tdesc.Size = roundedSize;
                    resize = true;
                }
            }        
        }

        // XXX padding to power of 2 not supported in core yet
        if (resize /*&& (Use & ImageUse_Wrap)*/)
        {        
            if (ImageData::IsFormatCompressed(format))
            {        
                SF_DEBUG_ERROR(1, 
                               "CreateTexture failed - Can't rescale compressed Wrappable image to Pow2");
                if (State != State_Lost)
                    State = State_InitFailed;
                return false;
            }
            TextureFlags |= TF_Rescale;
        }
    }

    // Determine how many mipLevels we should have and whether we can
    // auto-generate them or not.
    unsigned allocMipLevels = MipLevels;
    bool genMipmaps = 0;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        if (!(pmanager->GetTextureUseCaps(format) & ImageUse_GenMipmaps))
        {            
            TextureFlags |= TF_SWMipGen;
            // If using SW MipGen, determine how many mip-levels we should have.
            allocMipLevels = 31;
            for (itex = 0; itex < TextureCount; itex++)
                allocMipLevels = Alg::Min(allocMipLevels, ImageSize_MipLevelCount(pTextures[itex].Size));
            MipLevels = (UByte)allocMipLevels;
        }
        else
        {
            genMipmaps = 1;
            allocMipLevels = 1;
        }
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        glGenTextures(1, &tdesc.TexId);
        glBindTexture(GL_TEXTURE_2D, tdesc.TexId);

#if defined(GL_TEXTURE_MAX_LEVEL)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels-1);
#elif defined(GL_APPLE_texture_max_level)
		if (pmanager->Caps & TextureManager::TC_UseAppleMaxLevel)
        {
			if (MipLevels > 1 )
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL_APPLE, MipLevels-1);
			else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL_APPLE, 1);
		}
#endif

        if (!ImageData::IsFormatCompressed(format))
        {
            const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
            glTexImage2D(GL_TEXTURE_2D, 0, pmapping->GLColors, tdesc.Size.Width, tdesc.Size.Height, 0, pmapping->GLFormat, pmapping->GLData, 0);

            unsigned width = tdesc.Size.Width;
            unsigned height = tdesc.Size.Height;
            for (unsigned i = 1; i < allocMipLevels; i++)
            {
                width >>= 1;
                height >>= 1;
                if (width < 1)
                    width = 1;
                if (height < 1)
                    height = 1;
                glTexImage2D(GL_TEXTURE_2D, i, pmapping->GLColors, width, height, 0, pmapping->GLFormat, pmapping->GLData, 0);
            }
        }
    }

    if (genMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // If we have lost the text, but have a backing image, the contents should be able to be restored.
    if (State == State_Lost && pBackingImage != 0)
    {
        UpdateRenderTargetData(0); // NOTE: RenderTargetData is not used in GL, so it can be NULL.

        // Reallocation of the backing buffer/update from pImage is not necessary, return now
        State = State_Valid;
        return Render::Texture::Initialize();
    }

    // If the image is mappable, we need to create a buffer in which the mapped data will exist.
    if ((Use & ImageUse_MapSimThread ||
        Use & ImageUse_MapRenderThread ))
    {
        pBackingImage = *RawImage::Create(format, MipLevels, pTextures[0].Size, 0);
    }


    // Upload image content to texture, if any.
    if (pImage && !Render::Texture::Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture contents");

        ReleaseHWTextures();
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

bool Texture::Initialize(GLuint texID)
{
    pFormat = 0;
    TextureManager* pmanager = GetManager();

    if ( pTextures[0].TexId != texID )
    {
        ReleaseHWTextures();
    }
    pTextures = &Texture0;
    Texture0.TexId = texID;

    int format;
#if defined(SF_USE_GLES) || defined(GL_ES_VERSION_2_0)
    // There is no way to query texture information in GLES. 
    format = GL_RGBA;
    SF_ASSERT(ImgSize != ImageSize(0));
    Texture0.Size.SetSize(ImgSize);
#else
    int width, height, level0, levelN;
    glBindTexture(GL_TEXTURE_2D, texID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, &level0);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &levelN);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &format);

    // 1000 is the default value for GL_TEXTURE_MAX_LEVEL
    // If we get this value, we can assume the texture is not mipmapped, and
    // the user simply failed to set the value properly.
    if (levelN == 1000)
    {
        levelN = 1;
    }

    Texture0.Size.SetSize(width, height);

    // Override the image size if it was not provided.
    if ( ImgSize == ImageSize(0) )
        ImgSize = pTextures[0].Size;

    MipLevels = Alg::Max<UByte>(1, (UByte)(levelN - level0));
#endif // GL_ES_VERSION_2_0


    // If an image is provided, try to obtain the texture format from the image.
    pFormat = 0;
    if ( pImage )
        pFormat = (TextureFormat*)pmanager->getTextureFormat(pImage->GetFormatNoConv());

    // Otherwise, figure out the texture format, based on the mapping table. Assume all levels have the same format.
    TextureFormat::Mapping* pmapping;
    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        if ( pmapping->GLFormat == TextureManager::getBaseTextureFormatFromInternal((GLenum)format) )
        {
            pFormat = (TextureFormat*)pmanager->getTextureFormat(pmapping->Format);
            break;
        }
    }

    if ( !pFormat )
    {
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return Render::Texture::Initialize();
}

bool Texture::Upload(unsigned itex, unsigned level, const ImagePlane& plane)
{
    glBindTexture(GL_TEXTURE_2D, pTextures[itex].TexId);

    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    if (ImageData::IsFormatCompressed(pmapping->Format))
    {
        // The plane's DataSize member contains the size of the buffer remaining, not the actual size of
        // the texture's mip level. Compute that, and pass it in.
        UPInt levelSize = ImageData::GetMipLevelSize(pmapping->ConvFormat, plane.GetSize(), itex);
        glCompressedTexImage2D(GL_TEXTURE_2D, level, pmapping->GLFormat, plane.Width, plane.Height, 0, (GLsizei)levelSize, plane.pData);
        
        // For certain formats (PVRTC), mip levels that are too small generate errors, and do not function correctly.
        // In this case, artibrarily reduce the miplevel count.
        GLenum mipError = glGetError();
        if (mipError != 0 && level > 0)
        {
            MipLevels = (UByte)level;
            SF_DEBUG_WARNING4(mipError != 0, "Uploading mip-level failed (level=%d, size=(%d x %d). Reducing number of mips to %d.", 
                              level, plane.Width, plane.Height, MipLevels);
#if defined(GL_APPLE_texture_max_level)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL_APPLE, MipLevels-1);
#endif            
            return true;
        }
        else if (mipError != 0)
            return false;
        return true;
    }

    if (plane.Width * pmapping->BytesPerPixel == plane.Pitch)
    {
        if (plane.Width == pTextures[itex].Size.Width && plane.Height == pTextures[itex].Size.Height)
            glTexImage2D(GL_TEXTURE_2D, level, GetTextureFormatMapping()->GLColors, plane.Width, plane.Height, 0, pmapping->GLFormat, pmapping->GLData, plane.pData);
        else
            glTexSubImage2D(GL_TEXTURE_2D, level, 0,0, plane.Width, plane.Height, pmapping->GLFormat, pmapping->GLData, plane.pData);
    }
#if (defined(GL_UNPACK_ROW_LENGTH) && defined(GL_UNPACK_ALIGNMENT))
    else if (plane.Pitch == ((3 + plane.Width * pmapping->BytesPerPixel) & ~3))
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, plane.Width);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        if (plane.Width == pTextures[itex].Size.Width && plane.Height == pTextures[itex].Size.Height)
            glTexImage2D(GL_TEXTURE_2D, level, GetTextureFormatMapping()->GLColors, plane.Width, plane.Height, 0, pmapping->GLFormat, pmapping->GLData, plane.pData);
        else
            glTexSubImage2D(GL_TEXTURE_2D, level, 0,0, plane.Width, plane.Height, pmapping->GLFormat, pmapping->GLData, plane.pData);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
#endif
    else
    {
        if (!pBackingImage)
        {
            SF_DEBUG_WARNING(1, "GL::Texture - irregular size, cannot use upload, making texture mappable");
            MakeMappable();
        }

        unsigned width = pTextures[itex].Size.Width;
        unsigned height = pTextures[itex].Size.Height;

        for (unsigned i = 1; i < level; i++)
        {
            width >>= 1;
            height >>= 1;
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;
        }

        UpdateDesc upd;
        upd.DestRect = ImageRect(0,0, width, height);
        upd.PlaneIndex = itex;
        upd.SourcePlane = plane;

        Update(&upd, 1, level);
    }

    return true;
}

void Texture::uploadImage(ImageData* psource)
{
    for (unsigned itex = 0; itex < TextureCount; itex++)
        {
            for (unsigned level = 0; level < MipLevels; level++)
            {
                ImagePlane plane;
                psource->GetPlane(level * TextureCount + itex, &plane);
                Upload(itex, level, plane);
            }
    }
}


void Texture::ReleaseHWTextures(bool)
{
    Render::Texture::ReleaseHWTextures();

    TextureManager* pmanager = GetManager();
    bool useKillList = !(pmanager->IsMultiThreaded() || GetCurrentThreadId() == pmanager->RenderThreadId);

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        GLuint TexId = pTextures[itex].TexId;
        if (TexId && ((TextureFlags & TF_DoNotDelete) == 0))
        {
            if (useKillList)
                pmanager->GLTextureKillList.PushBack(TexId);
            else
                glDeleteTextures(1, &TexId);
        }        
        pTextures[itex].TexId = 0;
    }

    LastMinFilter = 0;
    LastAddress = 0;
}

void Texture::ApplyTexture(unsigned stage, const ImageFillMode& fillMode)
{
    Render::Texture::ApplyTexture(stage, fillMode);

    GLint minfilter = (fillMode.GetSampleMode() == Sample_Point) ? GL_NEAREST : (MipLevels>1 ? GL_LINEAR_MIPMAP_LINEAR  : GL_LINEAR);
    GLint magfilter = (fillMode.GetSampleMode() == Sample_Point) ? GL_NEAREST : GL_LINEAR;
    GLint address = (fillMode.GetWrapMode() == Wrap_Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    GL::TextureManager* pmanager = (GL::TextureManager*)GetManager();

    for (unsigned plane = 0; plane < GetTextureStageCount() ; plane++)
    {
        int stageIndex = stage + plane;
        pmanager->ApplyTexture(stageIndex, pTextures[plane].TexId);
        if (LastMinFilter != minfilter || LastAddress != address)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);              
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, address);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, address);

            LastMinFilter = minfilter;
            LastAddress = address;
        }
    }
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL*)
{
#if !defined(SF_USE_GLES)
    if (!pBackingImage)
        return false;

    ImageData BackingData;
    if (!pBackingImage->GetImageData(&BackingData))
        return false;

    SF_DEBUG_ASSERT(TextureCount == 1, "Can only update RTs with one texture.");
    glBindTexture(GL_TEXTURE_2D, pTextures[0].TexId );
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    for (unsigned mip = 0; mip < Alg::Min(BackingData.GetMipLevelCount(), GetMipmapCount()); ++mip)
    {
        ImagePlane plane;
        BackingData.GetPlane(mip, &plane);
        glTexImage2D(GL_TEXTURE_2D, mip, pmapping->GLFormat, pTextures[0].Size.Width,
            pTextures[0].Size.Height, 0, pmapping->GLFormat, GL_UNSIGNED_BYTE, plane.pData );
    }
    return true;
#else
    // GLES 1.1 doesn't have FBOs.
    return false;
#endif
}

bool Texture::UpdateStagingData(Render::RenderTargetData* prtData)
{
#if !defined(SF_USE_GLES)
    if (!pBackingImage)
        return false;

    ImageData BackingData;
    if (!pBackingImage->GetImageData(&BackingData))
        return false;

    RenderTargetData* glPRT = (RenderTargetData*)prtData;
    SF_DEBUG_ASSERT(TextureCount == 1, "Can only update RTs with one texture.");
    SF_DEBUG_ASSERT(MipLevels== 1, "Can only update RTs with one mip-level.");
    glBindFramebuffer(GL_FRAMEBUFFER, glPRT->FBOID);
    //glPixelStorei() ?
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    for (unsigned mip = 0; mip < Alg::Min(BackingData.GetMipLevelCount(), GetMipmapCount()); ++mip)
    {
        ImagePlane plane;
        BackingData.GetPlane(mip, &plane);
        glReadPixels(0, 0, GetSize().Width, GetSize().Height, pmapping->GLFormat, GL_UNSIGNED_BYTE, plane.pData );
    }

    // Set back to the default framebuffer
    RenderTarget* lastRT = GetHAL()->RenderTargetStack.Back().pRenderTarget;
    GL::RenderTargetData* defaultData = (GL::RenderTargetData*)lastRT->GetRenderTargetData();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultData->FBOID);
    return true;
#else
    // GLES 1.1 doesn't have FBOs.
    return false;
#endif
}

bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    // use map and update entire texture if necessary for performance
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    if (pBackingImage)
    {
domap:
        bool inUnmap = pMap != 0;

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
                desc.DestRect.x1 * pmapping->BytesPerPixel;

            splane.SetSize(desc.DestRect.GetSize());
            dplane.SetSize(desc.DestRect.GetSize());
            ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                pmapping->CopyFunc, 0);
        }

        if (!inUnmap)
            GetManager()->unmapTexture(this);
        return true;
    }

    // partial update
    bool convert = pmapping->CopyFunc != 0;

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);

#if (defined(GL_UNPACK_ROW_LENGTH) && defined(GL_UNPACK_ALIGNMENT))
        if (!convert && splane.Pitch == splane.Width * pmapping->BytesPerPixel)
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pmapping->GLFormat, pmapping->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
        else if (!convert && splane.Pitch == ((3 + splane.Width * pmapping->BytesPerPixel) & ~3))
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pmapping->GLFormat, pmapping->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else
#endif
        if (!convert && splane.Pitch == desc.DestRect.Width() * pmapping->BytesPerPixel)
        {
            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pmapping->GLFormat, pmapping->GLData, splane.pData);
        }
        else
        {
            SF_DEBUG_WARNING(1, "GL::Texture - irregular size, cannot use update, making texture mappable");

            MakeMappable();
            goto domap;
        }
    }

    return true;
}

void Texture::MakeMappable()
{
    pBackingImage = *RawImage::Create(pFormat->GetImageFormat(), MipLevels, pTextures[0].Size, 0);
}

// Copies the image data from the hardware.
#ifdef SF_AMP_SERVER
bool Texture::Copy(ImageData* pdata)
{
#if !defined(SF_USE_GLES_ANY)
    Image::CopyScanlineFunc puncopyFunc = pFormat->GetScanlineUncopyFn();
    if ( !GetManager() || pFormat->GetImageFormat() != pdata->Format || !puncopyFunc)
    {
        // - No texture manager, OR 
        // - Output format is different from the source input format of this texture (unexpected, because
        //   we should be copying back into the image's original source format) OR
        // - We don't know how to uncopy this format.
        return false;
    }


    // Try to map the texture. Only textures with backing images are actually mappable, which is not the usual case.
    bool alreadyMapped = (pMap != 0);
    unsigned mipCount = GetMipmapCount();
    bool mapped = alreadyMapped;
    if (!alreadyMapped )
        mapped = GetManager()->mapTexture(this, 0, mipCount) != 0;


    // Copy the planes into pdata, using the reverse copy function.
    SF_ASSERT( !mapped || pdata->GetPlaneCount() == pMap->Data.GetPlaneCount());
    Ptr<RawImage> imageDest = 0;
    int ip;
    for ( ip = 0; ip < pdata->RawPlaneCount; ip++ )
    {
        ImagePlane splane, dplane;
        pdata->GetPlane(ip, &dplane);

        if ( mapped )
        {
            pMap->Data.GetPlane(ip, &splane);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, pTextures[ip].TexId );
            imageDest = *RawImage::Create(GetImageFormat(), mipCount, pTextures[ip].Size, Use);
            ImageData pIdata;
            imageDest->GetImageData(&pIdata);

            for ( unsigned mip = 0; mip < mipCount; ++ mip)
            {
                pIdata.GetMipLevelPlane(mip, ip, &splane);

                // For safety.
                GLint w,h;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, mip, GL_TEXTURE_WIDTH, &w );
                glGetTexLevelParameteriv(GL_TEXTURE_2D, mip, GL_TEXTURE_HEIGHT, &h);
                SF_ASSERT((unsigned)w == splane.Width && (unsigned)h == splane.Height);

                // Get the image data from GL, to a temporary buffer.
                glGetTexImage(GL_TEXTURE_2D, mip, GetTextureFormatMapping()->GLColors, GetTextureFormatMapping()->GLData, splane.pData );
            }
        }
        ConvertImagePlane(dplane, splane, GetFormat(), ip, puncopyFunc, 0);
    }

    // Unmap the texture, if we mapped it.
    if ( mapped && !alreadyMapped )
        GetManager()->unmapTexture(this);

    return true;
#else
    // Image read-back not supported on GLES2.
    SF_UNUSED(pdata);
    return false;
#endif
}
#endif // SF_AMP_SERVER

void Texture::computeUpdateConvertRescaleFlags( bool rescale, bool swMipGen, ImageFormat inputFormat, 
                                                ImageRescaleType &rescaleType, ImageFormat &rescaleBuffFromat, bool &convert )
{
    SF_UNUSED(inputFormat);
    const TextureFormat::Mapping* pmapping = GetTextureFormatMapping();
    rescaleBuffFromat = pmapping->ConvFormat;
    rescaleType = ResizeNone;

    if (rescale)
    {
        if (pmapping->BytesPerPixel == 4)
            rescaleType = ResizeRgbaToRgba;
        else if (pmapping->BytesPerPixel == 1)
            rescaleType = ResizeGray;
        else
        {
            rescaleBuffFromat = Image_R8G8B8A8;
            convert = true;
        }
    }
    if (swMipGen && !(pmapping->BytesPerPixel == 4 || pmapping->BytesPerPixel == 1))
        convert = true;
}

// ***** DepthStencilSurface
int DepthStencilSurface::GLFormatIndex = -1;
#if !defined(SF_USE_GLES)
unsigned DepthStencilSurface::GLStencilFormats[] =
{
#if defined(SF_USE_GLES2)
    GL_DEPTH24_STENCIL8_OES,    // iOS requires this, because it's first, it will always succeed.
    GL_UNSIGNED_INT_24_8_OES,
    GL_STENCIL_INDEX8,
    GL_DEPTH_STENCIL_OES,
    GL_STENCIL_INDEX,
#endif
#if defined(GL_OES_stencil4)
    GL_STENCIL_INDEX4_OES,
#endif
#if defined(GL_ARB_framebuffer_object)
    GL_DEPTH24_STENCIL8,
    GL_STENCIL_INDEX8,
    GL_STENCIL_INDEX4,
    GL_STENCIL_INDEX16,
#endif
    0
};
#else
unsigned DepthStencilSurface::GLStencilFormats[] =
{
    0
};
#endif

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), RenderBufferID(0)
{

}

DepthStencilSurface::~DepthStencilSurface()
{
    TextureManager* pmanager = (TextureManager*)GetTextureManager();
    bool useKillList = !(pmanager->IsMultiThreaded() || GetCurrentThreadId() == pmanager->RenderThreadId);
    pmanager->GLDepthStencilKillList.PushBack(RenderBufferID);
    if ( !useKillList )
    {
        pmanager->processTextureKillList();
    }
}

bool DepthStencilSurface::Initialize()
{
#if !defined(SF_USE_GLES)
    glGenRenderbuffers(1, &RenderBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferID);

    // Clear glError
    GLenum glError = glGetError();
    if ( GLFormatIndex < 0 )
        GLFormatIndex = 0;

    do 
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GLStencilFormats[GLFormatIndex], Size.Width, Size.Height);
        glError = glGetError();
    } while ( glError != GL_NO_ERROR && SetNextGLFormatIndex());

    if ( glGetError() != 0 )
        State = Texture::State_InitFailed;
    else
        State = Texture::State_Valid;
#else
    // No renderbuffer extension on GLES1.1
    State = Texture::State_InitFailed;
#endif // !SF_USE_GLES
    return State == Texture::State_Valid;
}

bool DepthStencilSurface::SetNextGLFormatIndex()
{
    if ( GLFormatIndex >= 0 && GLStencilFormats[GLFormatIndex] == 0 )
        return false;
    GLFormatIndex++;
    SF_ASSERT(GLFormatIndex >= 0);
    return GLStencilFormats[GLFormatIndex] > 0;
}

bool DepthStencilSurface::CurrentFormatHasDepth()
{
    switch(GLStencilFormats[GLFormatIndex])
    {
#if defined(SF_USE_GLES2)
        case GL_DEPTH24_STENCIL8_OES:
        case GL_UNSIGNED_INT_24_8_OES:
        case GL_DEPTH_STENCIL_OES:
            return true;
#endif
#if defined(GL_ARB_framebuffer_object)
        case GL_DEPTH24_STENCIL8:
            return true;
#endif

        default: 
            return false;
    }
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

    Texture* glTexture  = reinterpret_cast<Texture*>(ptexture);
    if (!glTexture->pBackingImage)
        return false;

    pTexture      = ptexture;
    StartMipLevel = mipLevel;
    LevelCount    = levelCount;


    unsigned textureCount = ptexture->TextureCount;
    ImageData BackingData;
    glTexture->pBackingImage->GetImageData(&BackingData);

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        ImagePlane              plane;

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        for (unsigned level = 0; level < levelCount; level++)
        {
            BackingData.GetMipLevelPlane(level + StartMipLevel, itex, &plane);
            Data.SetPlane(level * textureCount + itex, plane);
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap(bool)
{
    unsigned textureCount = pTexture->TextureCount;
    Texture* glTexture  = reinterpret_cast<Texture*>(pTexture);

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        ImagePlane plane;

        for (int level = 0; level < LevelCount; level++)
        {
            Data.GetPlane(level * textureCount + itex, &plane);

            if (plane.pData)
                glTexture->Upload(itex, level + StartMipLevel, plane);
        }
    }
    MappedTextureBase::Unmap();
}

// ***** TextureManager

TextureManager::TextureManager(ThreadId renderThreadId, ThreadCommandQueue* commandQueue, TextureCache* texCache) :
    Render::TextureManager(renderThreadId, commandQueue, texCache),
    pHal(0),
    Caps(0)
{
}

TextureManager::~TextureManager()
{   
    Mutex::Locker lock(&pLocks->TextureMutex);

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    SF_ASSERT(TextureInitQueue.IsEmpty());
    processTextureKillList();    

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    pLocks->pManager = 0;
}

void TextureManager::NotifyLostContext()
{
    Mutex::Locker lock(&pLocks->TextureMutex);

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    SF_ASSERT(TextureInitQueue.IsEmpty());
    //processTextureKillList();
    GLTextureKillList.Clear();

    // Notify all textures
    Render::Texture* p= Textures.GetFirst();
    while (!Textures.IsNull(p))
    {
        p->LoseTextureData();
        p = Textures.GetNext(p);
    }
}

void TextureManager::ApplyTexture(unsigned stageIndex, GLint texture)
{
    // Detect redundant texture changes.
    if ( CurrentTextures[stageIndex] == texture )
        return;
    glActiveTexture(GL_TEXTURE0 + stageIndex);
    glBindTexture(GL_TEXTURE_2D, texture );
}

void TextureManager::DestroyFBO(GLuint fboid)
{
#if !defined(SF_USE_GLES)
    if (CanCreateTextureCurrentThread())
        glDeleteFramebuffers(1, &fboid);
    else
        GLFrameBufferKillList.PushBack(fboid);
#endif
}

// ***** GL Format mapping and conversion functions

void SF_STDCALL GL_CopyScanline8_Extend_A_LA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[1] = ps[0];
        pd[0] = 255;
    }
}

#if defined(SF_USE_GLES)
#define BGRA_EXT "GL_APPLE_texture_format_BGRA8888"
#elif defined(GL_ES_VERSION_2_0)
#define BGRA_EXT "GL_IMG_texture_format_BGRA8888"
#elif !defined(GL_BGRA)
#define BGRA_EXT "EXT_bgra"
#else
#define BGRA_EXT NULL
#endif

TextureFormat::Mapping TextureFormatMapping[] = 
{
    // Standard formats.
    { Image_R8G8B8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, NULL,        &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
    { Image_R8G8B8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline32_SwapBR,           &Image_CopyScanline32_SwapBR},
    { Image_B8G8R8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
    { Image_B8G8R8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, NULL,        &Image_CopyScanline32_SwapBR,           &Image_CopyScanline32_SwapBR},

#if !defined(SF_USE_GLES_ANY)
    { Image_R8G8B8,     Image_R8G8B8,       GL_RGB,             GL_RGB,             GL_UNSIGNED_BYTE, 3, NULL,        &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
    { Image_R8G8B8,     Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, NULL,        &Image_CopyScanline24_Extend_RGB_RGBA,  &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_R8G8B8,     Image_R8G8B8A8,     GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline24_Extend_RGB_BGRA,  &Image_CopyScanline32_Retract_BGRA_RGB },

#if defined(GL_BGR_EXT)
    { Image_B8G8R8,     Image_R8G8B8,       GL_RGB,             GL_BGR_EXT,         GL_UNSIGNED_BYTE, 3, BGRA_EXT,    &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
#endif
    { Image_B8G8R8,     Image_R8G8B8A8,     GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline24_Extend_RGB_RGBA,  &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,     Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, NULL,        &Image_CopyScanline24_Extend_RGB_BGRA,  &Image_CopyScanline32_Retract_BGRA_RGB },

    // GL_ALPHA is not usable in GL3.0+ contexts. Use GL_RED instead, shaders will change to reflect.
    { Image_A8,         Image_A8,           GL_ALPHA,           GL_ALPHA,           GL_UNSIGNED_BYTE, 1, NULL,        &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_USE_GLES_ANY)
    { Image_A8,         Image_A8,           GL_RED,             GL_RED,             GL_UNSIGNED_BYTE, 1, NULL,        &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
#if defined(GL_LUMINANCE) // Removed in GL 3.0.
    { Image_A8,         Image_A8,           GL_LUMINANCE,       GL_LUMINANCE,       GL_UNSIGNED_BYTE, 1, NULL,        &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif

    // Video formats.
    // GL_ALPHA is not usable in GL3.0+ contexts. Use GL_RED instead, shaders will change to reflect.
    { Image_Y8_U2_V2,   Image_Y8_U2_V2,     GL_ALPHA,           GL_ALPHA,           GL_UNSIGNED_BYTE, 1, 0,          &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_USE_GLES_ANY)
    { Image_Y8_U2_V2,   Image_Y8_U2_V2,     GL_RED,             GL_RED,             GL_UNSIGNED_BYTE, 1, 0,          &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
    { Image_Y8_U2_V2_A8,Image_Y8_U2_V2_A8,  GL_ALPHA,           GL_ALPHA,           GL_UNSIGNED_BYTE, 1, 0,          &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_USE_GLES_ANY)
    { Image_Y8_U2_V2_A8,Image_Y8_U2_V2_A8,  GL_RED,             GL_RED,             GL_UNSIGNED_BYTE, 1, 0,          &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif

    // Compressed formats.
#if defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
    { Image_DXT1,     Image_DXT1,     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_UNSIGNED_BYTE, 0, "EXT_texture_compression_s3tc",   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },
    { Image_DXT3,     Image_DXT3,     GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_UNSIGNED_BYTE, 0, "EXT_texture_compression_s3tc",   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },
    { Image_DXT5,     Image_DXT5,     GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_UNSIGNED_BYTE, 0, "EXT_texture_compression_s3tc",   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },
#endif

#if defined(GL_IMG_texture_compression_pvrtc)
    { Image_PVRTC_RGB_4BPP,   Image_PVRTC_RGB_4BPP,  GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,  GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,  GL_UNSIGNED_BYTE, 0, "IMG_texture_compression_pvrtc", &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGB_2BPP,   Image_PVRTC_RGB_2BPP,  GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,  GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,  GL_UNSIGNED_BYTE, 0, "IMG_texture_compression_pvrtc", &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGBA_4BPP,  Image_PVRTC_RGBA_4BPP, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_UNSIGNED_BYTE, 0, "IMG_texture_compression_pvrtc", &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGBA_2BPP,  Image_PVRTC_RGBA_2BPP, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GL_UNSIGNED_BYTE, 0, "IMG_texture_compression_pvrtc", &Image::CopyScanlineDefault, 0 },
#endif // GL_IMG_texture_compression_pvrtc

#if defined(GL_OES_compressed_ETC1_RGB8_texture)
    { Image_ETC1_RGB_4BPP,    Image_ETC1_RGB_4BPP,   GL_ETC1_RGB8_OES,  GL_ETC1_RGB8_OES, GL_UNSIGNED_BYTE, 0, "OES_compressed_ETC1_RGB8_texture", &Image::CopyScanlineDefault, 0 },
#endif // GL_OES_compressed_ETC1_RGB8_texture

#if defined(GL_AMD_compressed_ATC_texture)
    { Image_ATCIC,    Image_ATCIC,    GL_ATC_RGB_AMD,                     GL_ATC_RGB_AMD,                     GL_UNSIGNED_BYTE, 0, "AMD_compressed_ATC_texture", &Image::CopyScanlineDefault, 0 },
    { Image_ATCICA,   Image_ATCICA,   GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,     GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,     GL_UNSIGNED_BYTE, 0, "AMD_compressed_ATC_texture", &Image::CopyScanlineDefault, 0 },
    { Image_ATCICI,   Image_ATCICI,   GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD, GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD, GL_UNSIGNED_BYTE, 0, "AMD_compressed_ATC_texture", &Image::CopyScanlineDefault, 0 },
#endif // GL_AMD_compressed_ATC_texture

#if defined(GL_ARB_texture_compression_bptc)
    { Image_BC7,      Image_BC7,      GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,  GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,  GL_UNSIGNED_BYTE, 0, "ARB_texture_compression_bptc", &Image::CopyScanlineDefault, 0 },
#endif

    { Image_None, Image_None,      0,  0, 0, 0 }
};

// Returns whether the pixel format is valid for the current GL version.
bool CheckPixelFormat(HAL* pHal, GLenum intFmt)
{
#if defined(SF_USE_GLES_ANY)
    return true;
#else
    switch(intFmt)
    {
        // Alpha and luminance are not valid in 3.0+
#if defined(GL_LUMINANCE)
        case GL_LUMINANCE: return !pHal->CheckGLVersion(3,0);
#endif
        case GL_ALPHA:     return !pHal->CheckGLVersion(3,0);
        default:           return true;
    }
#endif
}

GLenum TextureManager::getBaseTextureFormatFromInternal(GLenum intfmt)
{
#ifdef SF_USE_GLES_ANY
    return intfmt;
#else
    switch( intfmt )
    {
#if defined(GL_ALPHA4)
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            return GL_ALPHA;
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            return GL_LUMINANCE;
#endif
            
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            return GL_RGB;

        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
            return GL_RGBA;
        default:
            return intfmt;
    }
#endif
}

void TextureManager::Initialize(HAL* phal)
{
    RenderThreadId = GetCurrentThreadId();
    pHal = phal;
    Caps = 0;
    
#if defined(SF_USE_GLES)
    // XXX
    if (pHal->CheckExtension(BGRA_EXT))
        Caps |= TC_UseBgra;
#elif defined(GL_ES_VERSION_2_0)
    Caps |= TC_NonPower2Limited;
    if (pHal->CheckExtension("GL_OES_texture_npot"))
        Caps |= TC_NonPower2Full;
    
    // PowerVR devices have some issues rendering to NPOT framebuffers,
    // although these do not report framebuffer incompleteness. This appears
    // to only manifest itself in DrawableImage operations, so we specifically force those
    // to POT for iOS and Android, otherwise, we allow NPOT framebuffers.
    Caps |= TC_NonPower2RT;
    
// PowerVR texture support is different between Android 2.0-2.2 and 2.3
// 2.2 doesn't support RGBA but supports BGRA
// 2.3 supports RGBA, but not BGRA, but still reports that BGRA is supported
if (pHal->CheckExtension("GL_OES_required_internalformat") &&
    pHal->CheckExtension("GL_IMG_texture_format_BGRA8888") &&
    !pHal->CheckExtension("GL_IMG_multisampled_render_to_texture"))
    Caps |= TC_UseBgra;
#else
    // XXX
    Caps |= TC_NonPower2Full|TC_NonPower2Limited|TC_NonPower2RT;
#endif

	// Check to see if Apple's texture_max_level ext is actually present.
    if (pHal->CheckExtension("APPLE_texture_max_level"))
        Caps |= TC_UseAppleMaxLevel;

    initTextureFormats();
}

void TextureManager::initTextureFormats()
{
    // Do not initialize if the TextureManager has not been initialized. 
    if (pHal == 0)
        return;

    TextureFormat::Mapping* pmapping;

    for (pmapping = TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        // See if format is supported.
#if defined(SF_USE_GLES) || defined(GL_ES_VERSION_2_0)
        if (Caps & TC_UseBgra)
        {
            if (pmapping->GLFormat == GL_RGBA)
                continue;
        }
        else if (pmapping->GLFormat == GL_BGRA_EXT)
            continue;
#endif
        if (CheckPixelFormat(pHal, pmapping->GLColors) && (pmapping->Extension == NULL || pHal->CheckExtension(pmapping->Extension)))
        {
            TextureFormat* tf = SF_HEAP_AUTO_NEW(this) TextureFormat(pmapping);
            TextureFormats.PushBack(tf);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}


void    TextureManager::processTextureKillList()
{
    if ( GLTextureKillList.GetSize() > 0 )
    {
        glDeleteTextures((GLsizei)GLTextureKillList.GetSize(), GLTextureKillList.GetDataPtr());
        GLTextureKillList.Clear();
    }

#if !defined(SF_USE_GLES)
    if ( GLDepthStencilKillList.GetSize() > 0 )
    {
        glDeleteRenderbuffers((GLsizei)GLDepthStencilKillList.GetSize(), GLDepthStencilKillList.GetDataPtr());
        GLDepthStencilKillList.Clear();
    }
    if ( GLFrameBufferKillList.GetSize() > 0 )
    {
        glDeleteFramebuffers((GLsizei)GLFrameBufferKillList.GetSize(), GLFrameBufferKillList.GetDataPtr());
        GLFrameBufferKillList.Clear();
    }
#endif
}

void    TextureManager::processInitTextures()
{
    // TextureMutex lock expected externally.
    //Mutex::Locker lock(&TextureMutex);

    if (!TextureInitQueue.IsEmpty())
    {
        do {
            Render::Texture* ptexture = TextureInitQueue.GetFirst();
            ptexture->RemoveNode();
            ptexture->pPrev = ptexture->pNext = 0;
            if (ptexture->Initialize())
                Textures.PushBack(ptexture);

        } while (!TextureInitQueue.IsEmpty());
        pLocks->TextureInitWC.NotifyAll();
    }
}

// Should be called before each frame on RenderThread.
void TextureManager::BeginFrame( )   
{ 
    Render::TextureManager::BeginFrame();
    memset(CurrentTextures, -1, sizeof(CurrentTextures) );
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size, unsigned use, 
                                               ImageBase* pimage, Render::MemoryManager* allocManager)    
{
    SF_UNUSED(allocManager);

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    Render::Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, pimage);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(GLuint texID, bool deleteTexture, 
                                               ImageSize imgSize, ImageBase* pimage )
{
    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, texID, deleteTexture, imgSize, pimage);

    return postCreateTexture(ptexture, 0);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size, MemoryManager* manager )
{
    SF_UNUSED(manager);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, size);
    return postCreateDepthStencilSurface(pdss);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(GLuint id)
{
#if !defined(SF_USE_GLES)
    if ( !glIsRenderbuffer(id) )
        return 0;

    GLint width, height;
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &height);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, ImageSize(width, height) );
    pdss->RenderBufferID = id;
    pdss->State = Texture::State_Valid;
    return pdss;
#else
    // Not supported on GLES 1.1
    SF_UNUSED(id);
    return 0;
#endif
}

unsigned TextureManager::GetTextureFormatSupport() const
{
    unsigned formats = ImageFormats_Standard;

    for (unsigned i = 0; i< TextureFormats.GetSize(); i++)    
    {
        switch(TextureFormats[i]->GetImageFormat())
        {
        case Image_DXT5:              formats |= ImageFormats_DXT; break;
        case Image_PVRTC_RGBA_4BPP:   formats |= ImageFormats_PVR; break;
        case Image_ATCICA:            formats |= ImageFormats_ATITC; break;
        case Image_ETC1_RGBA_8BPP:    formats |= ImageFormats_ETC; break;

        default: break;
        }
    }

    return formats;
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    unsigned use = ImageUse_Update;
#if !defined(SF_OS_MAC) && !defined(SF_OS_ANDROID)
    use |= ImageUse_NoDataLoss;
#endif
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
    unsigned need = TC_NonPower2Limited;
    if (use & (ImageUse_Wrap|ImageUse_GenMipmaps))
        need |= TC_NonPower2Full;

    if (use & ImageUse_RenderTarget)
        need |= TC_NonPower2RT;   
    
    return ((Caps & need) == need) != 0;
}

}}};  // namespace Scaleform::Render::GL

