/**************************************************************************

Filename    :   TGA_ImageReader.cpp
Content     :   TGA Image file format reader TGA implementation
Created     :   February 2010
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "TGA_ImageFile.h"
#include "Render_ImageFileUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace TGA {


// Temporary TGA image class used as a data source, allowing
// direct initialization of RawImage or Texture with image data.

class TGAFileImageSource : public FileImageSource
{
    ImageFormat     SourceFormat;
    UByte           ImageDesc;
    Ptr<Palette>    pColorMap;

public:
    TGAFileImageSource(File* file, ImageFormat format)
    : FileImageSource(file,format),
      SourceFormat(Image_None), ImageDesc(0)
    {
    }

    bool         ReadHeader(MemoryHeap *pheap);
    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    virtual unsigned        GetMipmapCount() const {return 1;}
};


bool TGAFileImageSource::ReadHeader(MemoryHeap *pheap)
{
    UByte           idLen        = pFile->ReadUByte();
    TGAColorMapType colorMapType = (TGAColorMapType) pFile->ReadUByte();
    TGAImageType    imageType    = (TGAImageType)pFile->ReadUByte();

    // Color Map Specification 
    unsigned colorMapStartIndex, // Not used now
             colorMapSize,
             colorMapEntrySize;
    colorMapStartIndex = pFile->ReadUInt16();   // first entry index   
    colorMapSize       = pFile->ReadUInt16();   // color map length (in entries)
    colorMapEntrySize  = pFile->ReadUByte();    // color map entry size (in bits)

    if (colorMapEntrySize > 0 &&
        colorMapEntrySize != 24 && colorMapEntrySize != 32)
    {
        SF_DEBUG_WARNING(1, "TGA::FileReader filed - only 24/32 bit color maps supported");
        return false;
    }

    pFile->ReadUInt16(); // X origin
    pFile->ReadUInt16(); // Y origin

    UInt16 width       = pFile->ReadUInt16();
    UInt16 height      = pFile->ReadUInt16();
    UByte  bitsPerPixel= pFile->ReadUByte();
    Size.SetSize(width, height);

    if (!((colorMapType == TGAColorMap_NotIncluded && imageType == TGAImage_TrueColor) ||
          (colorMapType == TGAColorMap_Included && imageType == TGAImage_ColorMapped)))
    {
        // only uncompressed RGB and color map formats are supported now
        SF_DEBUG_WARNING(1, "TGA::FileReader filed - unsupported image type");
        return false;
    }

    ImageDesc = pFile->ReadUByte();
    if (idLen > 0)
        pFile->SkipBytes(idLen);

    // Determine source format and scanline type.
    switch(bitsPerPixel)
    {
    case 8:  SourceFormat = Image_P8;       break;
    case 24: SourceFormat = Image_B8G8R8;   break;
    case 32: SourceFormat = Image_B8G8R8A8; break;
    default:
        // only 8/24/32-bits TrueType format supported
        SF_DEBUG_WARNING(1, "TGA::FileReader filed - unsupported image bpp");
        return false; 
    }
    
    if (Format == Image_None)
    {
#if (!defined(GFC_OS_PSP) && !defined(GFC_OS_PS2))
        if (SourceFormat == Image_P8)
        {
            // By default, convert 256-colors TGA to RGB_888 if no alpha in palette
            if (colorMapEntrySize < 32)
                Format = Image_B8G8R8;
            else // or, to ARGB, if palette is 32 bit 
                Format = Image_B8G8R8A8;
        }
        else
#endif
        {
            Format = SourceFormat;
        }
    }    

    if (colorMapType == TGAColorMap_Included)
    {
        SF_ASSERT(imageType == TGAImage_ColorMapped);

        // Load color map
        bool colorMapHasAlpha = (colorMapEntrySize == 32);
        pColorMap = *Palette::Create(colorMapSize, colorMapHasAlpha, pheap);
        if (!pColorMap)
        {
            SF_DEBUG_WARNING(1, "TGA::FileReader filed - couldn't allocate color map");
            return false;
        }

        // Palette entries are BGR ordered.
        for (unsigned i = 0; i < colorMapSize; ++i)
        {
            Color& c = (*pColorMap)[i];
            UByte  b = pFile->ReadUByte(),
                   g = pFile->ReadUByte(),
                   r = pFile->ReadUByte();
            c.SetRGBA(r,g,b, 0xFF);
            
            if (colorMapHasAlpha)
                c.SetAlpha(pFile->ReadUByte());
        }
    }
    FilePos = pFile->LTell();
    return true;
}


bool TGAFileImageSource::Decode(ImageData* pdest,
                                CopyScanlineFunc copyScanline, void* arg) const
{
    if (!seekFileToDecodeStart())
        return false;

    ImageScanlineBuffer<1024*4> scanline(SourceFormat, Size.Width, Format);
    if (!scanline.IsValid())
        return false;

    unsigned yindex, ydelta, y;
    bool     success = true;

    if (ImageDesc & TGAImageDesc_TopToBottom)
    {
        yindex = 0;
        ydelta = 1;
    }
    else
    {
        yindex = Size.Height-1;
        ydelta = (unsigned)-1;
    }

    int readSize = (int)scanline.GetReadSize();
    
    for (y = 0; y < Size.Height; y++, yindex += ydelta)
    {
        if (pFile->Read(scanline.GetReadBuffer(), readSize) != readSize)
        {
            success = false;
            break; // read error!
        }
        UByte* destScanline = pdest->GetScanline(yindex);
        scanline.ConvertReadBuffer(destScanline, pColorMap.GetPtr(), copyScanline, arg);
    }

    // Assign color map if necessary.
    if (Format == Image_P8)
        pdest->pPalette = pColorMap;

    return success;
}



// ***** FileReader

bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{
    FileHeaderReader<18> header(file, headerArg, headerArgSize);
    if (!header)
        return false;

    // TGA doesn't have a header tag, so we just verify header-value compatibility.
    // Due to this it is best if TGA is handled last.
    TGAColorMapType colorMapType = (TGAColorMapType) header[1];
    TGAImageType    imageType    = (TGAImageType)header[2];
    UByte           colorMapEntrySize = header[7];
    int             width  = header[12] | (((int)header[13])<<8);
    int             height = header[14] | (((int)header[15])<<8);
    UByte           bitsPerPixel = header[16];
    UByte           imageDesc    = header[17];
    
    // Only uncompressed RGB and color map formats are supported now.
    if (!((colorMapType == TGAColorMap_NotIncluded && imageType == TGAImage_TrueColor) ||
          (colorMapType == TGAColorMap_Included && imageType == TGAImage_ColorMapped)))
        return false;
    
    if ((colorMapEntrySize != 0) && (colorMapEntrySize != 24) && (colorMapEntrySize != 32))
        return false;
    if ((bitsPerPixel != 8) && (bitsPerPixel != 24) && (bitsPerPixel != 32))
        return false;
    if ((imageDesc & TGAImageDesc_ZeroMaskBits) != 0)
        return false;

    // There must be enough data in the file for non-RLE.
    if (file->GetLength() < (width*height*(bitsPerPixel>>3) + 18))
        return false;
    return true;
}

ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;
    TGAFileImageSource* source = SF_NEW TGAFileImageSource(file, args.Format);
    if (source && !source->ReadHeader(args.GetHeap()))
    {
        source->Release();
        source = 0;
    }
    return source;    
}

// Instance singleton.
FileReader FileReader::Instance;


}}}; // namespace Scaleform::Render::TGA

