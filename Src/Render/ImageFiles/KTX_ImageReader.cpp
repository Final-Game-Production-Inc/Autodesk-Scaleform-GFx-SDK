/**************************************************************************

Filename    :   KTX_ImageReader.cpp
Content     :   KTX Image file format reader KTX implementation
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "KTX_ImageFile.h"
#include "Render/GL/GL_Common.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace KTX {

struct KTXHeaderInfo
{
    bool    EndianSwapped;          // Whether the image is the same endianness as the system.
    UInt32  GLType;                 // UNSIGNED_BYTE, etc. (0 for compressed images).
    UInt32  GLTypeSize;             // size in bytes of the data type (1 for compressed images).
    UInt32  GLFormat;               // RGB, etc. (0 for compressed images).
    UInt32  GLInternalFormat;       // Internal format representation of the glFormat.
    UInt32  GLBaseInternalFormat;   // Base internal format.
    UInt32  PixelWidth;             // Width of level 0.
    UInt32  PixelHeight;            // Height of level 0 (0 for 1D images).
    UInt32  PixelDepth;             // Depth of level 0 (0 for 1D/2D images).
    UInt32  ArrayElements;          // Number of array elements (0 for non-array textures).
    UInt32  Faces;                  // Number of faces (6 for cubemaps, 1 for non-cubemaps).
    UInt32  MipLevels;              // Number of mip-levels.
    UInt32  KeyValueSize;           // Size of key/value pairs in bytes.
    // Key/value pairs.
    UInt32  ImageSize;              // Sizes in bytes of pixel data.

    ImageFormat Format;

    bool IsCompressed() { return GLFormat == 0; }
};

// Temporary KTX image class used as a data source, allowing
// direct initialization of Texture with image data.

class KTXFileImageSource : public FileImageSource
{
    ImageFormat     SourceFormat;
    KTXHeaderInfo   HeaderInfo;

public:
    enum
    {
        KTX_IDSize = 12,                    // Size of identifier.
        KTX_HeaderStatic = 12 * sizeof(UInt32), // Size (in bytes) of initial static header, minus identifier and endian id.
        KTX_EndianComp = 0x04030201,            // File is same endianness of system comparison.
    };
    static UByte* GetKTXIdentifier()
    {
        static UByte id[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
        return id;
    }

    KTXFileImageSource(File* file, ImageFormat format)
    : FileImageSource(file,format),
      SourceFormat(Image_None)
    {
    }

    bool                ReadHeader();
    virtual bool        Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned    GetMipmapCount() const {return 1;}
};

static const UByte* ParseUInt32(const UByte* buf, UInt32* pval)
{
    *pval = Alg::ByteUtil::LEToSystem(*(UInt32*)buf);    
    return buf + 4;
}

bool KTXFileImageSource::ReadHeader()
{
    // Verify that the header is correct.
    UByte identifier[KTX_IDSize];
    if ( pFile->Read(identifier, KTX_IDSize) != KTX_IDSize ||
         memcmp( identifier, GetKTXIdentifier(), KTX_IDSize ) )
        return false;

    // Determine endianness
    UInt32 endian;
    if ( pFile->Read((UByte*)&endian, sizeof(UInt32)) != sizeof(UInt32))
        return false;
    HeaderInfo.EndianSwapped = (endian != KTX_EndianComp);

    // Parse the rest of the static header.
    UByte data[KTX_HeaderStatic];
    const UByte* buffer = data;
    if ( pFile->Read(data, KTX_HeaderStatic) != KTX_HeaderStatic ) 
        return false;

    buffer = ParseUInt32(buffer, &HeaderInfo.GLType);
    buffer = ParseUInt32(buffer, &HeaderInfo.GLTypeSize);
    buffer = ParseUInt32(buffer, &HeaderInfo.GLFormat);
    buffer = ParseUInt32(buffer, &HeaderInfo.GLInternalFormat);
    buffer = ParseUInt32(buffer, &HeaderInfo.GLBaseInternalFormat);
    buffer = ParseUInt32(buffer, &HeaderInfo.PixelWidth);
    buffer = ParseUInt32(buffer, &HeaderInfo.PixelHeight);
    buffer = ParseUInt32(buffer, &HeaderInfo.PixelDepth);
    buffer = ParseUInt32(buffer, &HeaderInfo.ArrayElements);
    buffer = ParseUInt32(buffer, &HeaderInfo.Faces);
    buffer = ParseUInt32(buffer, &HeaderInfo.MipLevels);
    buffer = ParseUInt32(buffer, &HeaderInfo.KeyValueSize);

    // Skip the key value pairs, and add the padding.
    pFile->Seek((HeaderInfo.KeyValueSize+3)&~4);

    // Image size is last.
    HeaderInfo.ImageSize = pFile->ReadUInt32();

    // Determine the image format.
    switch( HeaderInfo.GLInternalFormat )
    {
        // Really, you should only have ETC data in a KTX file.
#ifdef GL_OES_compressed_ETC1_RGB8_texture
        case GL_ETC1_RGB8_OES:      HeaderInfo.Format = Image_ETC1_RGB_4BPP; break;
#endif
        default:                    HeaderInfo.Format = Image_None; break;
    }

    return HeaderInfo.Format != Image_None;
}

bool KTXFileImageSource::Decode(ImageData* pdest,
                                CopyScanlineFunc copyScanline, void* arg) const
{
    if (!seekFileToDecodeStart())
        return false;

    unsigned mipWidth = Size.Width;
    unsigned mipHeight = Size.Height;
    for (unsigned m = 0; m < pdest->GetMipLevelCount(); m++)
    {
        ImagePlane mipPlane;
        if (pdest->HasSeparateMipmaps())
            pdest->GetMipLevelPlane(m, 0, &mipPlane);
        else
            pdest->GetPlaneRef().GetMipLevel(pdest->GetFormat(), m, &mipPlane);
        //For compressed formats copy mipmap data directly.
        if (Format == Image_ETC1_RGB_4BPP )
        {
            SF_ASSERT(pdest->Format == Format);
            if (pFile->Read(mipPlane.pData, (int)mipPlane.DataSize) != (int)mipPlane.DataSize)
                return false;
        }
        else
        {
            ImageScanlineBuffer<1024*4> scanline(HeaderInfo.Format, mipWidth, Format);
            int readSize = (int)scanline.GetReadSize();
            if (!scanline.IsValid())
                return false;
            for (unsigned y = 0; y < mipHeight; y++)
            {
                if (pFile->Read(scanline.GetReadBuffer(), readSize) != readSize)
                    return false;
                UByte* destScanline = mipPlane.GetScanline(y);
                scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
            }
        }
        mipWidth /= 2;
        mipHeight /= 2;
    }
    return true;
}



// ***** FileReader

bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<KTXFileImageSource::KTX_IDSize> header(file, headerArg, headerArgSize);
    if (!header)
        return false;

    return !memcmp(KTXFileImageSource::GetKTXIdentifier(), header.GetPtr(), KTXFileImageSource::KTX_IDSize);
}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;
    KTXFileImageSource* source = SF_NEW KTXFileImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        source = 0;
    }
    return source;
}

// Instance singleton.
FileReader FileReader::Instance;


}}}; // namespace Scaleform::Render::TGA

