/**********************************************************************

Filename    :   GTX_ImageReader.cpp
Content     :   
Created     :   Jun 2011
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/ImageFiles/GTX_ImageFile.h"
#include "Render/ImageFiles/Render_ImageFileUtil.h"
#include "Render/WiiU/WiiU_Texture.h"
#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include <cafe/gfd.h>

namespace Scaleform { namespace Render { namespace GTX {

// Temporary GTX image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class GTXFileImageSource : public FileImageSource
{
    UByte               ImageDesc; 
    ImageData           Data;
    GX2Texture          TexHeader;
    UInt32              TexDataOffset, MipDataOffset;
    int                 Status;
    
public:
    GTXFileImageSource(File* file, ImageFormat format)
        : FileImageSource(file,format), ImageDesc(0), Status(0), TexDataOffset(0), MipDataOffset(0)
    {
    }
    virtual             ~GTXFileImageSource(){}
    bool                ReadHeader();
    virtual bool        Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const { return false; }
    virtual unsigned    GetMipmapCount() const 
    { 
        return TexHeader.surface.numMips; 
    }

    virtual Image* CreateCompatibleImage(const ImageCreateArgs& args);

    SF_AMP_CODE(virtual UPInt GetBytes() const { return (Data.GetBitsPerPixel() * Data.GetSize().Area()) / 8; })
};

bool GTXFileImageSource::ReadHeader()
{
    Status = 0;
    GFDHeader header;
    if ( pFile->Read((UByte*)&header, sizeof(header)) != sizeof(header)
        || header.size != sizeof(header))
    {
        SF_ASSERT(0);
        return false;
    }
    while (1)
    {
        GFDBlockHeader block;
        if ( pFile->Read((UByte*)&block, sizeof(block)) != sizeof(block)
            || block.size != sizeof(block))
            return (Status & 3) == 3;

        switch (block.type)
        {
        case GFD_BLOCK_TYPE_GX2_TEX_HEADER:
            if (pFile->Read((UByte*)&TexHeader, sizeof(GX2Texture)) != sizeof(GX2Texture))
                return false;
            Status |= 1;
            break;

        case GFD_BLOCK_TYPE_GX2_TEX_IMAGE:
            TexDataOffset = pFile->Tell();
            pFile->Seek(block.size, File::Seek_Cur);
            Status |= 2;
            break;

        case GFD_BLOCK_TYPE_GX2_TEX_MIP_IMAGE:
            MipDataOffset = pFile->Tell();
            pFile->Seek(block.size, File::Seek_Cur);
            break;

        case GFD_BLOCK_TYPE_END:
            return (Status & 3) == 3;

        default:
            pFile->Seek(block.size, File::Seek_Cur);
            break;
        }
    }
    
    return true;
}

Image* GTXFileImageSource::CreateCompatibleImage(const ImageCreateArgs& args)
{
    if (!args.pManager)
    {
        SF_DEBUG_ERROR(1, "GTXFileImageSource: must have a TextureManager");
        return 0;
    }
    if (Status != 3)
    {
        SF_DEBUG_ERROR(1, "GTXFileImageSource: header read failed");
        return 0;
    }

    MemoryHeap*  heap = args.pHeap ? args.pHeap : Memory::GetGlobalHeap();
    WiiU::TextureManager* pManager = (WiiU::TextureManager*) args.pManager;
    Ptr<WiiU::Texture> ptexture = *(WiiU::Texture*)pManager->CreateTexture(TexHeader);
    if (ptexture)
    {
        WiiU::Texture::HWTextureDesc& tdesc = ptexture->pTextures[0];
        tdesc.pTexBaseData = (UByte*)pManager->GetMemoryManager()->Alloc(tdesc.Tex.surface.imageSize, tdesc.Tex.surface.alignment, Memory_Normal);
        if (!tdesc.pTexBaseData)
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
            return 0;
        }
        pFile->Seek(TexDataOffset);
        if (pFile->Read(tdesc.pTexBaseData, tdesc.Tex.surface.imageSize) != tdesc.Tex.surface.imageSize)
        {
            SF_DEBUG_ERROR(1, "GTXFileImageSource: read from file failed");
            return 0;
        }
        GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, tdesc.pTexBaseData, tdesc.Tex.surface.imageSize);

        if (tdesc.Tex.surface.mipSize)
        {
            tdesc.pTexMipData = (UByte*)pManager->GetMemoryManager()->Alloc(tdesc.Tex.surface.mipSize, tdesc.Tex.surface.alignment, Memory_Normal);
            if (!tdesc.pTexMipData)
            {
                SF_DEBUG_ERROR(1, "CreateTexture failed - memory allocation failed");
                return 0;
            }
            pFile->Seek(MipDataOffset);
            if (pFile->Read(tdesc.pTexMipData, tdesc.Tex.surface.mipSize) != tdesc.Tex.surface.mipSize)
            {
                SF_DEBUG_ERROR(1, "GTXFileImageSource: read from file failed");
                return 0;
            }
            GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, tdesc.pTexMipData, tdesc.Tex.surface.mipSize);
        }

        GX2InitTexturePtrs(&tdesc.Tex, tdesc.pTexBaseData, tdesc.pTexMipData);

        return SF_HEAP_NEW(heap)
            TextureImage(ptexture->GetFormat(), ptexture->GetSize(), args.pManager->GetTextureUseCaps(ptexture->GetFormat()),
                         ptexture, args.pUpdateSync);
    }
    else
        return 0;
}


bool GTX::FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerSize) const
{
    FileHeaderReader<4> header(file, headerArg, headerSize);
    if (!header)
        return false;
    if ((header[0] != 'G') || (header[1] != 'f') || (header[2] != 'x') || header[3] != '2')
        return false;
    return true;
}

ImageSource* GTX::FileReader::ReadImageSource(File* file, const ImageCreateArgs& args ) const
{
    if (!file || !file->IsValid())
        return 0;

    GTXFileImageSource* source = SF_NEW GTXFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        return 0;
    }
    return source; 
}

// Instance singleton.
FileReader FileReader::Instance;

}}} // Scaleform::Render::GTX
