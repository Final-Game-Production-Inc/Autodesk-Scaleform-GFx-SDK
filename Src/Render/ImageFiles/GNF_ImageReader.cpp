/**********************************************************************

PublicHeader:   Render
Filename    :   GNF_ImageReader.cpp
Content     :   
Created     :   2013/05/08
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/ImageFiles/GNF_ImageFile.h"
#include "Render/ImageFiles/Render_ImageFileUtil.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

#include "gnf.h"

namespace Scaleform { namespace Render { namespace GNF {

bool FileReader::MatchFormat(File* file, UByte* header, UPInt headerSize) const
{
    FileHeaderReader<sizeof(sce::Gnf::Header)> headerReader(file, header, headerSize);
    if (!headerReader)
        return false;
    sce::Gnf::Header * headerInput = reinterpret_cast<sce::Gnf::Header *>(headerReader.GetPtr());
    return (headerInput->m_magicNumber == sce::Gnf::kMagic);
}

// Temporary GNF image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class GNFFileImageSource : public FileImageSource
{
public:
    GNFFileImageSource(File* file, sce::Gnm::Texture& texture, ImageFormat format);
    virtual             ~GNFFileImageSource(){}
    virtual bool        Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned    GetMipmapCount() const { return 1; }

private:
    sce::Gnm::Texture&  TextureDesc;
};

GNFFileImageSource::GNFFileImageSource(File* file, sce::Gnm::Texture& texture, ImageFormat format)
: FileImageSource(file,format), TextureDesc(texture)
{
    Size = ImageSize(TextureDesc.getWidth(), TextureDesc.getHeight());
    sce::Gnm::DataFormat gnfFormat = TextureDesc.getDataFormat();
    sce::Gnm::TileMode   gnfTile   = TextureDesc.getTileMode();

    // Only supports BCx textures.
    bool tiled = true;
    switch(gnfFormat.getSurfaceFormat())
    {
        default:                           SF_DEBUG_WARNING1(1, "GFx GNF only supports BCx and 8888 formats (provided=%d)", gnfFormat.getSurfaceFormat());
        case sce::Gnm::kSurfaceFormatBc1:  Format = Image_BC1; break;
        case sce::Gnm::kSurfaceFormatBc2:  Format = Image_BC2; break;
        case sce::Gnm::kSurfaceFormatBc3:  Format = Image_BC3; break;
        case sce::Gnm::kSurfaceFormatBc7:  Format = Image_BC7; break;
        case sce::Gnm::kSurfaceFormat8_8_8_8:
        {
            tiled = false;
            if (gnfFormat.getChannel(0) == sce::Gnm::kTextureChannelX &&
                gnfFormat.getChannel(1) == sce::Gnm::kTextureChannelY &&
                gnfFormat.getChannel(2) == sce::Gnm::kTextureChannelZ &&
                gnfFormat.getChannel(3) == sce::Gnm::kTextureChannelW)
            {
                Format = Image_R8G8B8A8;
            }
            else if (gnfFormat.getChannel(0) == sce::Gnm::kTextureChannelZ &&
                gnfFormat.getChannel(1) == sce::Gnm::kTextureChannelY &&
                gnfFormat.getChannel(2) == sce::Gnm::kTextureChannelX &&
                gnfFormat.getChannel(3) == sce::Gnm::kTextureChannelW)
            {
                Format = Image_B8G8R8A8;
            }
            else
            {
                SF_DEBUG_WARNING(1, "GFx GNF only supports R8G8B8A8 or B8G8R8A8 uncompressed textures.");
                Format = Image_R8G8B8A8;
            }
        }
    }

    if (tiled)
    {
        SF_DEBUG_WARNING1(gnfTile != sce::Gnm::kTileModeThin_1dThin,
            "GFx GNF only supports sce::Gnm::kTileModeThin_1dThin tiling (provided=%d)", gnfTile);
        Format = (ImageFormat)((UPInt)Format | ImageTarget_Orbis | ImageStorage_Tile);
    }
}

bool GNFFileImageSource::Decode( ImageData* pdest, CopyScanlineFunc copyScanline, void* arg ) const
{
    // File should already in the correct position to read the first image (done during FileReader::ReadImageSource).
    // This only supports 'direct' copy, no conversion between formats.
    unsigned mipWidth = Size.Width;
    unsigned mipHeight = Size.Height;
    for (unsigned m = 0; m < pdest->GetMipLevelCount(); m++)
    {
        ImagePlane mipPlane;
        if (pdest->HasSeparateMipmaps())
            pdest->GetMipLevelPlane(m, 0, &mipPlane);
        else
            pdest->GetPlaneRef().GetMipLevel(pdest->GetFormat(), m, &mipPlane);

        SF_DEBUG_ASSERT(Format == pdest->GetFormat(), "GNFFileImageSource formats must be equal.");

        // Just read the file directly into the buffer.
        if (pFile->Read(mipPlane.pData, mipPlane.DataSize) != mipPlane.DataSize)
            return false;

        mipWidth  = Alg::Max(1u, mipWidth / 2); 
        mipHeight = Alg::Max(1u, mipHeight / 2); 
    }
    return true;
}

ImageSource* FileReader::ReadImageSource(File* file,
    const ImageCreateArgs& args) const
{
    sce::Gnf::Header header;
    file->Read((UByte*)&header, sizeof(sce::Gnf::Header));
    SF_DEBUG_ASSERT(header.m_magicNumber == sce::Gnf::kMagic, "GNF does not having proper tag.");
    UByte contentsData[sce::Gnf::kMaxContents];
    sce::Gnf::Contents* contents = reinterpret_cast<sce::Gnf::Contents*>(contentsData);
    file->Read(contentsData, header.m_contentsSize);
    SF_DEBUG_WARNING1(contents->m_numTextures != 1, "GFx GNF supports exactly one texture per file (file has %d).", contents->m_numTextures);
    file->LSeek(sizeof(sce::Gnf::Header) + header.m_contentsSize + getTexturePixelsByteOffset(contents, 0) , File::Seek_Set);   
    return SF_NEW GNFFileImageSource(file, contents->m_textures[0], args.Format);
}

// Instance singleton.
FileReader FileReader::Instance;

}}}; // Scaleform::Render::GNF

