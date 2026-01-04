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
#include "Render/GL/GL_HAL.h"

#include "Render/GL/GL_ExtensionMacros.h"

namespace Scaleform { namespace Render { namespace GL {


extern TextureFormat::Mapping TextureFormatMapping[];

Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 ImageBase* pimage) :   
    Render::Texture(pmanagerLocks, size, (UByte)mipLevels, (UInt16)use, pimage, pformat)
{
    memset(LastMinFilter,0, sizeof(LastMinFilter));
    memset(LastAddress, 0, sizeof(LastAddress));

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
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0)
{
    memset(LastMinFilter,0, sizeof(LastMinFilter));
    memset(LastAddress, 0, sizeof(LastAddress));

    TextureFlags |= TF_UserAlloc;
    if ( !deleteTexture )
        TextureFlags |= TF_DoNotDelete;

    pTextures = &Texture0;
    pTextures[0].Size = size;
    pTextures[0].TexId = *SF_NEW HALGLTexture(texID);
}

Texture::Texture(TextureManagerLocks* pmanagerLocks, HALGLTexture* texID,
                 const ImageSize& size, ImageBase* pimage) : 
    Render::Texture(pmanagerLocks, size, 0, 0, pimage, 0)
{
    memset(LastMinFilter,0, sizeof(LastMinFilter));
    memset(LastAddress, 0, sizeof(LastAddress));

    TextureFlags |= TF_UserAlloc;
    pTextures = &Texture0;
    pTextures[0].Size = size;
    pTextures[0].TexId = texID;
}

Texture::~Texture()
{
    SF_DEBUG_ASSERT(pImage == 0, "pImage must be null, since ImageLost had to be called externally.");
    Mutex::Locker  lock(&pManagerLocks->TextureMutex);
    
    if ((State == State_Valid) || (State == State_Lost))
    {
        SF_DEBUG_ASSERT(pManagerLocks->pManager, "pManagerLocks->pManager should still be valid for these states.");
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
                roundedSize.Width = Alg::Max<unsigned>(roundedSize.Width, 32);
                roundedSize.Height = Alg::Max<unsigned>(roundedSize.Height, 32);
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

    // Create textures. First, make a texture we know that will not be used as the active one, because otherwise
    // the binding of the current active texture in that slot could be broken. If the texture create is happening
    // in the middle of rendering, this could cause a problem.
    glActiveTexture(GL_TEXTURE0 + FragShaderDesc::MaxTextureSamplers);
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        tdesc.TexId = *SF_NEW HALGLTexture();
        glGenTextures(1, &tdesc.TexId.GetRawRef());
        glBindTexture(GL_TEXTURE_2D, tdesc.TexId);

#if defined(SF_RENDER_OPENGL)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels-1);
#elif defined(SF_RENDER_GLES)
		if (pmanager->Caps & TextureManager::TC_UseAppleMaxLevel)
        {
			if (MipLevels > 1 )
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels-1);
			else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
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

bool Texture::Initialize(HALGLTexture* texID)
{
    pFormat = 0;
    TextureManager* pmanager = GetManager();

    if ( pTextures[0].TexId && pTextures[0].TexId != texID )
    {
        ReleaseHWTextures();
    }
    pTextures = &Texture0;
    Texture0.TexId = texID;

    int format;
#if defined(SF_RENDER_GLES)
    // There is no way to query texture information in GLES. 
    format = GL_RGBA;
    SF_ASSERT(ImgSize != ImageSize(0));
    Texture0.Size.SetSize(ImgSize);
#else
    int width, height, level0, levelN;
    glActiveTexture(GL_TEXTURE0 + FragShaderDesc::MaxTextureSamplers);
    glBindTexture(GL_TEXTURE_2D, pTextures[0].TexId);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, &level0);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &levelN);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

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
#endif // SF_RENDER_GLES


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
    glActiveTexture(GL_TEXTURE0 + FragShaderDesc::MaxTextureSamplers);
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
            if (GetHAL()->CheckExtension(SF_GL_APPLE_texture_max_level))
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels-1);
            return true;
        }
        else if (mipError != 0)
            return false;
        return true;
    }


#if defined(SF_RENDER_OPENGL)
    bool canUnpack = true;
#elif defined(SF_RENDER_GLES)
    bool canUnpack = GetHAL()->CheckExtension(SF_GL_EXT_unpack_subimage) || GetHAL()->CheckGLVersion(3,0);
#endif

    if (plane.Width * pmapping->BytesPerPixel == plane.Pitch)
    {
        if (plane.Width == pTextures[itex].Size.Width && plane.Height == pTextures[itex].Size.Height)
            glTexImage2D(GL_TEXTURE_2D, level, GetTextureFormatMapping()->GLColors, plane.Width, plane.Height, 0, pmapping->GLFormat, pmapping->GLData, plane.pData);
        else
            glTexSubImage2D(GL_TEXTURE_2D, level, 0,0, plane.Width, plane.Height, pmapping->GLFormat, pmapping->GLData, plane.pData);
    }
    else if (canUnpack && (plane.Pitch == ((3 + plane.Width * pmapping->BytesPerPixel) & ~3)))
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

    // Do not request the destruction if the TF_DoNotDelete flag is set.
    if ((TextureFlags & TF_DoNotDelete) == 0)
    {
        TextureManager::PendingTextureDestroyEntry entry(this);
        pmanager->addTextureResourceDestroyRequest(entry);
    }

    for (unsigned itex = 0; itex < TextureCount; itex++)
        pTextures[itex].TexId.Clear();

    memset(LastMinFilter,0, sizeof(LastMinFilter));
    memset(LastAddress, 0, sizeof(LastAddress));
}

void Texture::ApplyTexture(unsigned stage, const ImageFillMode& fillMode)
{
    Render::Texture::ApplyTexture(stage, fillMode);

    GLint minfilter = (fillMode.GetSampleMode() == Sample_Point) ? GL_NEAREST : (MipLevels>1 ? GL_LINEAR_MIPMAP_LINEAR  : GL_LINEAR);
    GLint magfilter = (fillMode.GetSampleMode() == Sample_Point) ? GL_NEAREST : GL_LINEAR;
    GLint address = (fillMode.GetWrapMode() == Wrap_Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    GL::TextureManager* pmanager = (GL::TextureManager*)GetManager();
    
    if (address == GL_REPEAT && (!Alg::IsPowerOfTwo(GetSize().Width) || !Alg::IsPowerOfTwo(GetSize().Height)) &&
        !pmanager->IsNonPow2Supported(GetImageFormat(), Use|ImageUse_Wrap))
    {
        HAL* hal = GetHAL();
        if (hal && (hal->GetHALState() & HAL::HS_InRenderTarget))
        {
            SF_DEBUG_WARNONCE(1, "Attempting to sample NPOT texture with GL_REPEAT, and the display FBO is not bound. "
                              "This is not supported by this device. Changing sampling to GL_CLAMP_TO_EDGE, artifacts may be present.\n");
            address = GL_CLAMP_TO_EDGE;
        }
    }
        
    for (unsigned plane = 0; plane < GetTextureStageCount() ; plane++)
    {
        int stageIndex = stage + plane;
        pmanager->ApplyTexture(stageIndex, pTextures[plane].TexId);
        if (LastMinFilter[stageIndex] != minfilter || LastAddress[stageIndex] != address)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);              
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, address);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, address);

            LastMinFilter[stageIndex] = minfilter;
            LastAddress[stageIndex] = address;
        }
    }
}

bool Texture::UpdateRenderTargetData(Render::RenderTargetData*, Render::HAL*)
{
    if (!pBackingImage)
        return false;

    ImageData BackingData;
    if (!pBackingImage->GetImageData(&BackingData))
        return false;

    SF_DEBUG_ASSERT(TextureCount == 1, "Can only update RTs with one texture.");
    glActiveTexture(GL_TEXTURE0 + FragShaderDesc::MaxTextureSamplers);
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
}

bool Texture::UpdateStagingData(Render::RenderTargetData* prtData)
{
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

#if defined(SF_RENDER_OPENGL)
    bool canUnpack = true;
#elif defined(SF_RENDER_GLES)
    bool canUnpack = GetHAL()->CheckExtension(SF_GL_EXT_unpack_subimage) || GetHAL()->CheckGLVersion(3,0);
#endif

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);

        if (canUnpack && !convert && splane.Pitch == splane.Width * pmapping->BytesPerPixel)
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pmapping->GLFormat, pmapping->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
        else if (canUnpack && !convert && splane.Pitch == ((3 + splane.Width * pmapping->BytesPerPixel) & ~3))
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pmapping->GLFormat, pmapping->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else if (!convert && splane.Pitch == desc.DestRect.Width() * pmapping->BytesPerPixel)
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
#if defined(SF_RENDER_OPENGL)
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
            glActiveTexture(GL_TEXTURE0 + FragShaderDesc::MaxTextureSamplers);
            glBindTexture(GL_TEXTURE_2D, pTextures[ip].TexId );
            imageDest = *RawImage::Create(GetImageFormat(), mipCount, pTextures[ip].Size, Use);
            ImageData pdata;
            imageDest->GetImageData(&pdata);

            for ( unsigned mip = 0; mip < mipCount; ++ mip)
            {
                pdata.GetMipLevelPlane(mip, ip, &splane);

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
DepthStencilSurface::StencilFormatEntry DepthStencilSurface::GLStencilFormats[] =
{
#if defined(SF_RENDER_GLES)
    {GL_DEPTH24_STENCIL8,   SF_GL_OES_packed_depth_stencil,    3, 0 },
    {GL_STENCIL_INDEX8,     0,                                 0, 0 },
#elif defined(SF_RENDER_OPENGL)
    {GL_DEPTH24_STENCIL8,   SF_GL_EXT_packed_depth_stencil,    3, 0 },
    {GL_STENCIL_INDEX8,     SF_GL_EXT_framebuffer_object,      3, 0 },
    {GL_STENCIL_INDEX,      0,                                 0, 0 },
#endif
    {0,                     0,                                 0, 0}
};

DepthStencilSurface::DepthStencilSurface(TextureManagerLocks* pmanagerLocks, const ImageSize& size) :
    Render::DepthStencilSurface(pmanagerLocks, size), RenderBufferID(0)
{

}

DepthStencilSurface::~DepthStencilSurface()
{
    TextureManager* pmanager = (TextureManager*)GetTextureManager();
    TextureManager::PendingTextureDestroyEntry entry(this);
    pmanager->addTextureResourceDestroyRequest(entry);
}

bool DepthStencilSurface::Initialize()
{
    RenderBufferID = *SF_NEW HALGLRenderbuffer();
    glGenRenderbuffers(1, &RenderBufferID.GetRawRef());
    glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferID);

    if ( GLFormatIndex < 0)
        GLFormatIndex = 0;

    while (GLStencilFormats[GLFormatIndex].SFGLExtension && 
           (!GetHAL()->CheckExtension((GLExtensionType)GLStencilFormats[GLFormatIndex].SFGLExtension) &&
            !GetHAL()->CheckGLVersion(GLStencilFormats[GLFormatIndex].GLMajorVersion, GLStencilFormats[GLFormatIndex].GLMinorVersion)))
    {
        if (!SetNextGLFormatIndex())
        {
            SF_DEBUG_MESSAGE(1, "Could not find suitable DepthStencilSurface format.");
            State = Texture::State_InitFailed;
            return false;
        }
    }
        

    glRenderbufferStorage(GL_RENDERBUFFER, GLStencilFormats[GLFormatIndex].Format, Size.Width, Size.Height);
    State = Texture::State_Valid;
    return true;
}

bool DepthStencilSurface::SetNextGLFormatIndex()
{
    if ( GLFormatIndex >= 0 && GLStencilFormats[GLFormatIndex].Format == 0 )
        return false;
    GLFormatIndex++;
    SF_ASSERT(GLFormatIndex >= 0);
    return GLStencilFormats[GLFormatIndex].Format > 0;
}

void DepthStencilSurface::ResetGLFormatIndex()
{
    GLFormatIndex = -1;
}

bool DepthStencilSurface::CurrentFormatHasDepth()
{
    switch(GLStencilFormats[GLFormatIndex].Format)
    {
        case GL_DEPTH24_STENCIL8:
        case GL_UNSIGNED_INT_24_8:
        case GL_DEPTH_STENCIL:
            return true;

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
    Caps(0),
    MaximumTextureSize(0)
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

    SF_DEBUG_ASSERT(TextureInitQueue.IsEmpty(), "InitTextureQueue MUST be empty, or there was a thread service problem.");

    // Notify all textures
    Render::Texture* p= Textures.GetFirst();
    while (!Textures.IsNull(p))
    {
        p->LoseTextureData();
        p = Textures.GetNext(p);
    }

    // Clear out all the lists, without actually 'deleteing' them. The resources are lost, and some GPUs throw
    // GL errors when trying to delete resources when there is no active content.
    TextureResourceDestroyList.Clear();
}

void TextureManager::ApplyTexture(unsigned stageIndex, HALGLTexture* texture)
{
    glActiveTexture(GL_TEXTURE0 + stageIndex);
    glBindTexture(GL_TEXTURE_2D, texture );
}

void TextureManager::DestroyFBO(HALGLFramebuffer* fboid)
{
    TextureManager::PendingTextureDestroyEntry entry(fboid);
    addTextureResourceDestroyRequest(entry);
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

#if defined(GL_ES_VERSION_2_0)
#define BGRA_EXT SF_GL_EXT_texture_format_BGRA8888
#elif !defined(GL_BGRA)
#define BGRA_EXT SF_GL_EXT_bgra
#else
#define BGRA_EXT SF_GL_NONE
#endif

TextureFormat::Mapping TextureFormatMapping[] = 
{
    // Standard formats.
    { Image_R8G8B8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,        GL_UNSIGNED_BYTE, 4, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
    { Image_R8G8B8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_BGRA,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline32_SwapBR,           &Image_CopyScanline32_SwapBR},
    { Image_B8G8R8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_BGRA,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
    { Image_B8G8R8A8,   Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,        GL_UNSIGNED_BYTE, 4, SF_GL_NONE,  &Image_CopyScanline32_SwapBR,           &Image_CopyScanline32_SwapBR},

#if !defined(SF_RENDER_GLES) || defined(SF_RENDER_GLES30_SUPPORT)
    { Image_R8G8B8,     Image_R8G8B8,       GL_RGB,             GL_RGB,         GL_UNSIGNED_BYTE, 3, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
    { Image_R8G8B8,     Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,        GL_UNSIGNED_BYTE, 4, SF_GL_NONE,  &Image_CopyScanline24_Extend_RGB_RGBA,  &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_R8G8B8,     Image_R8G8B8A8,     GL_RGBA,            GL_BGRA,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline24_Extend_RGB_BGRA,  &Image_CopyScanline32_Retract_BGRA_RGB },

#if defined(GL_BGR_EXT)
    { Image_B8G8R8,     Image_R8G8B8,       GL_RGB,             GL_BGR,         GL_UNSIGNED_BYTE, 3, BGRA_EXT,    &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault},
#endif
    { Image_B8G8R8,     Image_R8G8B8A8,     GL_RGBA,            GL_BGRA,        GL_UNSIGNED_BYTE, 4, BGRA_EXT,    &Image_CopyScanline24_Extend_RGB_RGBA,  &Image_CopyScanline32_Retract_RGBA_RGB },
    { Image_B8G8R8,     Image_R8G8B8A8,     GL_RGBA,            GL_RGBA,        GL_UNSIGNED_BYTE, 4, SF_GL_NONE,  &Image_CopyScanline24_Extend_RGB_BGRA,  &Image_CopyScanline32_Retract_BGRA_RGB },

    // GL_ALPHA is not usable in GL3.0+ contexts. Use GL_RED instead, shaders will change to reflect.
    { Image_A8,         Image_A8,           GL_ALPHA,           GL_ALPHA,       GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_RENDER_GLES) || defined(SF_RENDER_GLES30_SUPPORT)
    { Image_A8,         Image_A8,           GL_RED,             GL_RED,         GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
#if defined(GL_LUMINANCE) // Removed in GL 3.0.
    { Image_A8,         Image_A8,           GL_LUMINANCE,       GL_LUMINANCE,   GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif

    // Video formats.
    // GL_ALPHA is not usable in GL3.0+ contexts. Use GL_RED instead, shaders will change to reflect.
    { Image_Y8_U2_V2,   Image_Y8_U2_V2,     GL_ALPHA,           GL_ALPHA,       GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_RENDER_GLES) || defined(SF_RENDER_GLES30_SUPPORT)
    { Image_Y8_U2_V2,   Image_Y8_U2_V2,     GL_RED,             GL_RED,         GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif
    { Image_Y8_U2_V2_A8,Image_Y8_U2_V2_A8,  GL_ALPHA,           GL_ALPHA,       GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#if !defined(SF_RENDER_GLES) || defined(SF_RENDER_GLES30_SUPPORT)
    { Image_Y8_U2_V2_A8,Image_Y8_U2_V2_A8,  GL_RED,             GL_RED,         GL_UNSIGNED_BYTE, 1, SF_GL_NONE,  &Image::CopyScanlineDefault,            &Image::CopyScanlineDefault },
#endif

    // Compressed formats.
    { Image_DXT1,     Image_DXT1,     GL_COMPRESSED_RGBA_S3TC_DXT1, GL_COMPRESSED_RGBA_S3TC_DXT1, GL_UNSIGNED_BYTE, 0, SF_GL_EXT_texture_compression_s3tc,   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },
    { Image_DXT3,     Image_DXT3,     GL_COMPRESSED_RGBA_S3TC_DXT3, GL_COMPRESSED_RGBA_S3TC_DXT3, GL_UNSIGNED_BYTE, 0, SF_GL_EXT_texture_compression_s3tc,   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },
    { Image_DXT5,     Image_DXT5,     GL_COMPRESSED_RGBA_S3TC_DXT5, GL_COMPRESSED_RGBA_S3TC_DXT5, GL_UNSIGNED_BYTE, 0, SF_GL_EXT_texture_compression_s3tc,   &Image::CopyScanlineDefault, &Image::CopyScanlineDefault },

    { Image_PVRTC_RGB_4BPP,   Image_PVRTC_RGB_4BPP,  GL_COMPRESSED_RGB_PVRTC_4BPPV1,  GL_COMPRESSED_RGB_PVRTC_4BPPV1,  GL_UNSIGNED_BYTE, 0, SF_GL_IMG_texture_compression_pvrtc, &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGB_2BPP,   Image_PVRTC_RGB_2BPP,  GL_COMPRESSED_RGB_PVRTC_2BPPV1,  GL_COMPRESSED_RGB_PVRTC_2BPPV1,  GL_UNSIGNED_BYTE, 0, SF_GL_IMG_texture_compression_pvrtc, &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGBA_4BPP,  Image_PVRTC_RGBA_4BPP, GL_COMPRESSED_RGBA_PVRTC_4BPPV1, GL_COMPRESSED_RGBA_PVRTC_4BPPV1, GL_UNSIGNED_BYTE, 0, SF_GL_IMG_texture_compression_pvrtc, &Image::CopyScanlineDefault, 0 },
    { Image_PVRTC_RGBA_2BPP,  Image_PVRTC_RGBA_2BPP, GL_COMPRESSED_RGBA_PVRTC_2BPPV1, GL_COMPRESSED_RGBA_PVRTC_2BPPV1, GL_UNSIGNED_BYTE, 0, SF_GL_IMG_texture_compression_pvrtc, &Image::CopyScanlineDefault, 0 },

    { Image_ETC1_RGB_4BPP,    Image_ETC1_RGB_4BPP,   GL_ETC1_RGB8,  GL_ETC1_RGB8, GL_UNSIGNED_BYTE, 0, SF_GL_OES_compressed_ETC1_RGB8_texture, &Image::CopyScanlineDefault, 0 },
    { Image_ETC1_RGB_4BPP,    Image_ETC1_RGB_4BPP,   GL_ETC1_RGB8,  GL_ETC1_RGB8, GL_UNSIGNED_BYTE, 0, SF_GL_ARB_ES2_compatibility,            &Image::CopyScanlineDefault, 0 },
    { Image_ETC1A_RGBA_4BPP,  Image_ETC1A_RGBA_4BPP, GL_ETC1_RGB8,  GL_ETC1_RGB8, GL_UNSIGNED_BYTE, 0, SF_GL_OES_compressed_ETC1_RGB8_texture, &Image::CopyScanlineDefault, 0 },
    { Image_ETC1A_RGBA_4BPP,  Image_ETC1A_RGBA_4BPP, GL_ETC1_RGB8,  GL_ETC1_RGB8, GL_UNSIGNED_BYTE, 0, SF_GL_ARB_ES2_compatibility,            &Image::CopyScanlineDefault, 0 },



    { Image_ATCIC,    Image_ATCIC,    GL_ATC_RGB,                     GL_ATC_RGB,                     GL_UNSIGNED_BYTE, 0, SF_GL_AMD_compressed_ATC_texture, &Image::CopyScanlineDefault, 0 },
    { Image_ATCICA,   Image_ATCICA,   GL_ATC_RGBA_EXPLICIT_ALPHA,     GL_ATC_RGBA_EXPLICIT_ALPHA,     GL_UNSIGNED_BYTE, 0, SF_GL_AMD_compressed_ATC_texture, &Image::CopyScanlineDefault, 0 },
    { Image_ATCICI,   Image_ATCICI,   GL_ATC_RGBA_INTERPOLATED_ALPHA, GL_ATC_RGBA_INTERPOLATED_ALPHA, GL_UNSIGNED_BYTE, 0, SF_GL_AMD_compressed_ATC_texture, &Image::CopyScanlineDefault, 0 },

    { Image_BC7,      Image_BC7,      GL_COMPRESSED_RGBA_BPTC_UNORM,  GL_COMPRESSED_RGBA_BPTC_UNORM,  GL_UNSIGNED_BYTE, 0, SF_GL_ARB_texture_compression_bptc, &Image::CopyScanlineDefault, 0 },

    { Image_None, Image_None,      0,  0, 0, 0 }
};

// Returns whether the pixel format is valid for the current GL version.
bool CheckPixelFormat(HAL* pHal, GLenum intFmt)
{
#if defined(SF_RENDER_GLES)
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
#ifdef SF_RENDER_GLES
    return intfmt;
#else
    switch( intfmt )
    {
#if defined(GL_ALPHA)
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            return GL_ALPHA;
#endif
#if defined(GL_LUMINANCE)
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
    
#if defined(SF_RENDER_GLES)
    Caps |= TC_NonPower2Limited;
    if (pHal->CheckGLVersion(3,0) || pHal->CheckExtension(SF_GL_OES_texture_npot))
        Caps |= TC_NonPower2Full;

    // GLES2 devices generally do not work with NPOT render targets. Depending on the device, they may either 
    // report glCheckFramebufferStatus failure, a failure during glRenderBufferStorage, or just a black texture.
    // Since the behavior is unpredicable, disable it. GLES3 devices seem to have good support for NPOT targets.
    if (pHal->CheckGLVersion(3,0))
        Caps |= TC_NonPower2RT;

    // Device support for BGRA appears to be flaky, so just disable it.
    // PowerVR texture support is different between Android 2.0-2.2 and 2.3
    // 2.2 doesn't support RGBA but supports BGRA
    // 2.3 supports RGBA, but not BGRA, but still reports that BGRA is supported
    //if (pHal->CheckExtension(SF_GL_OES_required_internalformat) &&
    //    pHal->CheckExtension(SF_GL_EXT_texture_format_BGRA8888) &&
    //    !pHal->CheckExtension(SF_GL_IMG_multisampled_render_to_texture))
    //    Caps |= TC_UseBgra;
#else
    // XXX
    Caps |= TC_NonPower2Full|TC_NonPower2Limited|TC_NonPower2RT;
#endif

	// Check to see if Apple's texture_max_level ext is actually present.
    if (pHal->CheckExtension(SF_GL_APPLE_texture_max_level))
        Caps |= TC_UseAppleMaxLevel;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaximumTextureSize);

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
#if defined(GL_ES_VERSION_2_0)
        if (Caps & TC_UseBgra)
        {
            if (pmapping->GLFormat == GL_RGBA)
                continue;
        }
        else if (pmapping->GLFormat == GL_BGRA)
            continue;
#endif
        if (CheckPixelFormat(pHal, pmapping->GLColors) && (pmapping->Extension == SF_GL_NONE || pHal->CheckExtension(pmapping->Extension)))
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
    Mutex::Locker lock(&pLocks->TextureMutex);
    TextureResourceDestroyList.ProcessDestructionList();
}

TextureManager::PendingTextureDestroyEntry::PendingTextureDestroyEntry( Texture* ptexture ) : StageCount(0)
{
    SF_DEBUG_MESSAGE(!ptexture, "Trying to destroy NULL Texture.");
    if (!ptexture)
        return;

    GPUFence = ptexture->GetFence();
    StageCount = ptexture->GetTextureStageCount();
    SF_DEBUG_ASSERT2(StageCount <= FragShaderDesc::MaxTextureSamplers, "Texture has more stages than any fragment shader (%d vs %d).", 
        ptexture->GetTextureStageCount(), FragShaderDesc::MaxTextureSamplers);
    for (unsigned i = 0; i < StageCount; ++i)
    {
        pTexture[i] = ptexture->pTextures[i].TexId;
        pRenderBuffer[i] = 0;
        pFramebuffer[i] = 0;
    }
}


TextureManager::PendingTextureDestroyEntry::PendingTextureDestroyEntry( DepthStencilSurface* pds ) : StageCount(0)
{
    SF_DEBUG_MESSAGE(!pds, "Trying to destroy NULL DepthStencilSurface.");
    if (!pds)
        return;

    GPUFence = pds->GetFence();

    StageCount = 1;
    pRenderBuffer[0] = pds->RenderBufferID;
    pFramebuffer[0] = 0;
    pTexture[0] = 0;
}

TextureManager::PendingTextureDestroyEntry::PendingTextureDestroyEntry(HALGLFramebuffer* pFBO) : StageCount(0)
{
    SF_DEBUG_MESSAGE(!pFBO, "Trying to destroy NULL FBO.");
    if (!pFBO)
        return;

    // TBD: no fence is available. It FBO destruction should really be handled by GL internally,
    // so likely this is not required.
    // GPUFence = pFBO->GetFence();

    StageCount = 1;
    pFramebuffer[0] = pFBO;
    pRenderBuffer[0] = 0;
    pTexture[0] = 0;
}

void TextureManager::PendingTextureDestroyEntry::FreeResource()
{
    if (pTexture[0])
    {
        HALGLTexture* a0[MaximumStages];
        for (unsigned i = 0; i < StageCount; ++i)
            a0[i] = pTexture[i];
        glDeleteTextures((GLsizei)StageCount, a0);
    }

    if (pRenderBuffer[0])
    {
        HALGLRenderbuffer* a0[MaximumStages];
        for (unsigned i = 0; i < StageCount; ++i)
            a0[i] = pRenderBuffer[i];
        glDeleteRenderbuffers((GLsizei)StageCount, a0);
    }

    if (pFramebuffer[0])
    {
        HALGLFramebuffer* a0[MaximumStages];
        for (unsigned i = 0; i < StageCount; ++i)
            a0[i] = pFramebuffer[i];
        glDeleteFramebuffers((GLsizei)StageCount, a0);
    }
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

void TextureManager::addTextureResourceDestroyRequest( const PendingTextureDestroyEntry& e )
{
    Mutex::Locker lock(&pLocks->TextureMutex);

    e.pHAL = pHal; // Add the HAL.
    TextureResourceDestroyList.PushBack(e);
}

// Should be called before each frame on RenderThread.
void TextureManager::BeginFrame( )   
{ 
    Render::TextureManager::BeginFrame();
    memset(CurrentTextures, -1, sizeof(CurrentTextures) );
}

ImageSize clampImageSizeAndWarn(const ImageSize& size, const ImageSize& maxSize)
{
    SF_DEBUG_MESSAGE4(size.Width > maxSize.Width || size.Height > maxSize.Height,
        "Attempting to create a texture that is too large (requested = %d x %d, maximum = %d x %d). Clamping to maximum.",
        size.Width, size.Height, maxSize.Width, maxSize.Height);
    return ImageSize(Alg::Clamp<int>(size.Width, 0, maxSize.Width), Alg::Clamp<int>(size.Height, 0, maxSize.Height));
}
Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size, unsigned use, 
                                               ImageBase* pimage, Render::MemoryManager* allocManager)    
{
    SF_UNUSED(allocManager);

    TextureFormat* ptformat = (TextureFormat*)precreateTexture(format, use, pimage);
    if ( !ptformat )
        return 0;

    ImageSize actualSize = clampImageSizeAndWarn(size, ImageSize(MaximumTextureSize, MaximumTextureSize));

    Render::Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, actualSize, use, pimage);

    return postCreateTexture(ptexture, use);
}

Render::Texture* TextureManager::CreateTexture(GLuint texID, bool deleteTexture, 
                                               ImageSize imgSize, ImageBase* pimage )
{
    ImageSize actualSize = clampImageSizeAndWarn(imgSize, ImageSize(MaximumTextureSize, MaximumTextureSize));
    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, texID, deleteTexture, actualSize, pimage);

    return postCreateTexture(ptexture, 0);
}

Render::Texture* TextureManager::CreateTexture(HALGLTexture* texID, ImageSize imgSize, ImageBase* pimage)
{
    ImageSize actualSize = clampImageSizeAndWarn(imgSize, ImageSize(MaximumTextureSize, MaximumTextureSize));
    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, texID, actualSize, pimage);

    return postCreateTexture(ptexture, 0);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(const ImageSize& size, MemoryManager* manager )
{
    SF_UNUSED(manager);
    DepthStencilSurface* pdss = SF_HEAP_AUTO_NEW(this) DepthStencilSurface(pLocks, size);
    return postCreateDepthStencilSurface(pdss);
}

Render::DepthStencilSurface* TextureManager::CreateDepthStencilSurface(HALGLRenderbuffer* id)
{
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
        case Image_ETC1_RGB_4BPP:     formats |= ImageFormats_ETC; break;       // assume that if one ETC type is supported, they all are.
        case Image_ETC1A_RGBA_4BPP:   formats |= ImageFormats_ETC1A; break;

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

