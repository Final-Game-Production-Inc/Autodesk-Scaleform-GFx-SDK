/**********************************************************************

Filename    :   GXT_ImageReader.cpp
Content     :   
Created     :   Jun 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/ImageFiles/GXT_ImageFile.h"
#include "Render/ImageFiles/Render_ImageFileUtil.h"
#include "Render/Render_MemoryManager.h"
#include "Render/PSVita/PSVita_Texture.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include <gxt.h>

namespace Scaleform { namespace Render { namespace GXT {

// Temporary GXT image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class GXTFileImageSource : public FileImageSource
{
    UByte               ImageDesc; 
    SceGxtHeader        HeaderInfo;
    SceGxtTextureInfo   TextureInfo;
    ImageData           Data;
    
public:
    GXTFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format), ImageDesc(0)
    {
    }
    virtual             ~GXTFileImageSource(){}
    bool                ReadHeader();
    virtual bool        Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const { return false; }
    virtual unsigned    GetMipmapCount() const 
    { 
        // Assuption: only one texture in the GXT.
        return TextureInfo.mipCount; 
    }

    virtual Image* CreateCompatibleImage(const ImageCreateArgs& args);

    SF_AMP_CODE(virtual UPInt GetBytes() const { return (Data.GetBitsPerPixel() * Data.GetSize().Area()) / 8; })
};

bool GXTFileImageSource::ReadHeader()
{   
    if ( pFile->Read((UByte*)&HeaderInfo, sizeof(SceGxtHeader)) != sizeof(SceGxtHeader) || 
         HeaderInfo.numTextures != 1 )
    {
        // Only one texture per-GTX file is currently supported.
        SF_ASSERT(0);
        return false;
    }
    if ( pFile->Read((UByte*)&TextureInfo, sizeof(SceGxtTextureInfo)) != sizeof(SceGxtTextureInfo) )
        return false;
    
    if (Format == Image_None)
    {
        const unsigned SCE_GXM_TEXTURE_BASE_MASK = 0xFF000000U;
        const unsigned SCE_GXM_SWIZZLE_MASK      = 0x0000F000U;
        switch (TextureInfo.format & SCE_GXM_TEXTURE_BASE_MASK)
        {
	        case SCE_GXM_TEXTURE_BASE_FORMAT_PVRT2BPP:      
            case SCE_GXM_TEXTURE_BASE_FORMAT_PVRTII2BPP:
            {
                if ( (TextureInfo.format & SCE_GXM_SWIZZLE_MASK) == SCE_GXM_TEXTURE_SWIZZLE4_ABGR)
                    Format = Image_PVRTC_RGBA_2BPP; 
                else
                    Format = Image_PVRTC_RGB_2BPP; 
                break;
            }
	        case SCE_GXM_TEXTURE_BASE_FORMAT_PVRT4BPP:      
            case SCE_GXM_TEXTURE_BASE_FORMAT_PVRTII4BPP:
            {
                if ( (TextureInfo.format & SCE_GXM_SWIZZLE_MASK) == SCE_GXM_TEXTURE_SWIZZLE4_ABGR)
                    Format = Image_PVRTC_RGBA_4BPP; 
                else
                    Format = Image_PVRTC_RGB_4BPP; 
                break;
            }
            case SCE_GXM_TEXTURE_BASE_FORMAT_UBC1:          Format = Image_PSVita_DXT1; break;
	        case SCE_GXM_TEXTURE_BASE_FORMAT_UBC2:          Format = Image_PSVita_DXT3; break;
	        case SCE_GXM_TEXTURE_BASE_FORMAT_UBC3:          Format = Image_PSVita_DXT5; break;

            // Uncompressed formats.
            case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8U8:      
                switch (TextureInfo.format & SCE_GXM_SWIZZLE_MASK)
                {
                case SCE_GXM_TEXTURE_SWIZZLE4_ABGR: Format = Image_R8G8B8A8; break;
                case SCE_GXM_TEXTURE_SWIZZLE4_ARGB: Format = Image_B8G8R8A8; break;
                default: return false; // Unsupported swizzle.
                }
                break;
            case SCE_GXM_TEXTURE_BASE_FORMAT_U8:            Format = Image_A8; break;
            case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8:
                switch (TextureInfo.format & SCE_GXM_SWIZZLE_MASK)
                {
                case SCE_GXM_TEXTURE_SWIZZLE3_RGB:          Format = Image_B8G8R8; break;
                case SCE_GXM_TEXTURE_SWIZZLE3_BGR:          Format = Image_R8G8B8; break;
                default: return false; // Unsupported swizzle.
                }
                break;

            default: return false;
        }
    }
    Size = ImageSize(TextureInfo.width, TextureInfo.height);

    pFile->Seek(HeaderInfo.dataOffset);
    FilePos = pFile->LTell();
    return true;
}

Image* GXTFileImageSource::CreateCompatibleImage(const ImageCreateArgs& args)
{
    if (!args.pManager)
    {
        SF_DEBUG_ERROR(1, "GXTFileImageSource: must have a TextureManager");
        return 0;
    }
    if (!TextureInfo.dataSize || !seekFileToDecodeStart())
    {
        SF_DEBUG_ERROR(1, "GXTFileImageSource: header read failed");
        return 0;
    }

    PSVita::TextureManager* pManager = (PSVita::TextureManager*) args.pManager;

    SceGxmTexture TexHeader;
    UByte* pTexData = (UByte*)pManager->GetMemoryManager()->Alloc(TextureInfo.dataSize, 16, Memory_PSVita_CDRAM);
    if (!pTexData)
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
        return 0;
    }

    if (pFile->Read(pTexData, TextureInfo.dataSize) != TextureInfo.dataSize)
    {
        SF_DEBUG_ERROR(1, "GXTFileImageSource: read from file failed");
        return 0;
    }

    SceGxmErrorCode error;
    switch (TextureInfo.type)
    {
    case SCE_GXM_TEXTURE_SWIZZLED:
        error = sceGxmTextureInitSwizzled(&TexHeader, pTexData, (SceGxmTextureFormat)TextureInfo.format, TextureInfo.width, TextureInfo.height, TextureInfo.mipCount);
        break;
    case SCE_GXM_TEXTURE_SWIZZLED_ARBITRARY:
        error = sceGxmTextureInitSwizzledArbitrary(&TexHeader, pTexData,(SceGxmTextureFormat) TextureInfo.format, TextureInfo.width, TextureInfo.height, TextureInfo.mipCount);
        break;
    case SCE_GXM_TEXTURE_TILED:
        error = sceGxmTextureInitTiled(&TexHeader, pTexData, (SceGxmTextureFormat)TextureInfo.format, TextureInfo.width, TextureInfo.height, TextureInfo.mipCount);
        break;
    case SCE_GXM_TEXTURE_LINEAR:
        error = sceGxmTextureInitLinear(&TexHeader, pTexData, (SceGxmTextureFormat)TextureInfo.format, TextureInfo.width, TextureInfo.height, TextureInfo.mipCount);
        break;
    case SCE_GXM_TEXTURE_LINEAR_STRIDED:
        error = sceGxmTextureInitLinearStrided(&TexHeader, pTexData,(SceGxmTextureFormat) TextureInfo.format, TextureInfo.width, TextureInfo.height, TextureInfo.mipCount);
        break;

    default:
        SF_DEBUG_ERROR(1, "GXTFileImageSource: unknown texture storage type");
        return 0;
    }
    SF_ASSERT(error == SCE_OK);

    MemoryHeap*  heap = args.pHeap ? args.pHeap : Memory::GetGlobalHeap();
    Ptr<PSVita::Texture> ptexture = *(PSVita::Texture*)pManager->CreateTexture(TexHeader);
    if (ptexture)
    {
        // free data
        ptexture->pTextures[0].pTexData = pTexData;
        ptexture->pTextures[0].Size = TextureInfo.dataSize;

        return SF_HEAP_NEW(heap)
            TextureImage(ptexture->GetFormat(), ptexture->GetSize(), args.pManager->GetTextureUseCaps(ptexture->GetFormat()),
            ptexture, args.pUpdateSync);
    }
    else
        return 0;
}

bool GXT::FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerSize) const
{
    FileHeaderReader<4> header(file, headerArg, headerSize);
    if (!header)
        return false;
    if ((header[0] != 'G') || (header[1] != 'X') || (header[2] != 'T'))
        return false;
    return true;
}

ImageSource* GXT::FileReader::ReadImageSource(File* file, const ImageCreateArgs& args ) const
{
    if (!file || !file->IsValid())
        return 0;

    GXTFileImageSource* source = SF_NEW GXTFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
}

// Instance singleton.
FileReader FileReader::Instance;

}}} // Scaleform::Render::GXT
