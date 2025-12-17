/**************************************************************************

Filename    :   TGA_ImageWriter.cpp
Content     :   TGA Image writer implementation
Created     :   February 2010
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "TGA_ImageFile.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
//#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace TGA {


// Write a 32-bit Targa format bitmap.  Dead simple, no compression.
bool FileWriter::Write(File* out, const ImageData& imageData,
                       const ImageWriteArgs*) const
{
    if (!out || !out->IsValid() || !out->IsWritable())
        return false;

    UByte    bitsPerPixel = 0;
    UByte    imageDescr   = TGAImageDesc_TopToBottom;
    // Byte offsets in source memory of pixel.
    unsigned redIndex = 0, greenIndex = 1, blueIndex = 2;

    switch(imageData.GetFormat())
    {
    case Image_B8G8R8A8:
        redIndex = 2;
        blueIndex= 0;
    case Image_R8G8B8A8:
        bitsPerPixel = 32;
        imageDescr |= 8;
        break;

    case Image_B8G8R8:
        redIndex = 2;
        blueIndex= 0;
    case Image_R8G8B8:
        bitsPerPixel = 24;
        break;

    case Image_A8:
        bitsPerPixel = 8;
    default:
        break;
    }

    if ((bitsPerPixel == 0) ||
        (imageData.GetWidth() > 0xFFFF) ||
        (imageData.GetHeight() > 0xFFFF))
    {
        SF_DEBUG_WARNING(1, "TGA::WriteImage failed - unsupported ImageData format/size");
        return false;
    }

    out->WriteUByte(0);    // ID length 

    // ColorMapType, ImageType, Color Map Specification
    if (imageData.GetFormat() == Image_A8)
    {
        out->WriteUByte(TGAColorMap_Included);
        out->WriteUByte(TGAImage_ColorMapped);
        out->WriteUInt16(0);   // first entry index   
        out->WriteUInt16(256); // color map length (in entries)
        out->WriteUByte(24);   // color map entry size (in bits)
    }
    else
    {
        out->WriteUByte(TGAColorMap_NotIncluded);
        out->WriteUByte(TGAImage_TrueColor);
        out->WriteUInt16(0);
        out->WriteUInt16(0);
        out->WriteUByte(0);
    }

    out->WriteUInt16(0);   // X origin
    out->WriteUInt16(0);   // Y origin
    out->WriteUInt16((UInt16)imageData.GetWidth());
    out->WriteUInt16((UInt16)imageData.GetHeight());
    out->WriteUByte(bitsPerPixel);
    out->WriteUByte(imageDescr);

   
    //  Image Descriptor: 
    //  Bits:    7   6   5   4   3   2   1   0
    //          |0   0| |r   r| |a   a   a   a|
    //  where bits 3-0: These bits specify the number of attribute bits per
    //                  pixel. In the case of the TrueVista, these bits indicate
    //                  the number of bits per pixel which are designated as
    //                  Alpha Channel bits.
    //          bits 5-4: These bits are used to indicate the order in which
    //                  pixel data is transferred from the file to the screen.
    //                  Bit 4 is for left-to-right ordering and bit 5 is for topto-
    //                  bottom ordering as shown below:
    //                  bits:   5   4
    //                          0   0   - bottom left
    //                          0   1   - bottom right
    //                          1   0   - top left
    //                          1   1   - top right
    if (imageData.GetFormat() == Image_A8)
    {
        // color map
        for (unsigned i = 0; i < 256; ++i)
        {
            out->WriteUByte(UByte(i));
            out->WriteUByte(UByte(i));
            out->WriteUByte(UByte(i));
        }
    }

    unsigned height= imageData.GetHeight();
    unsigned width = imageData.GetWidth();

    for (unsigned y = 0; y < height; y++)
    {
        const UByte* p = imageData.GetScanline(y);
        
        if (bitsPerPixel == 24)
        {
            // write 24-bit scanline
            for (unsigned x = 0, wx = width*3; x < wx; x += 3)
            {
                out->WriteUByte(p[x + blueIndex]);   // B
                out->WriteUByte(p[x + greenIndex]);  // G
                out->WriteUByte(p[x + redIndex]);    // R
            }
        }

        else if (bitsPerPixel == 32)
        {
            // write 32-bit scanline
            for (unsigned x = 0, wx = width*4; x < wx; x += 4)
            {
                out->WriteUByte(p[x + blueIndex]);  // B
                out->WriteUByte(p[x + greenIndex]); // G
                out->WriteUByte(p[x + redIndex]);   // R
                out->WriteUByte(p[x + 3]);          // A
            }
        }

        else if (bitsPerPixel == 8)
        {
            // write indices 
            for (unsigned x = 0; x < width; ++x)
            {
                out->WriteUByte(p[x]);
            }
        }

        else
        {
            // Unsupported format of scanline
            SF_ASSERT(0);
        }
    }

    if (!out->IsWritable())
        return false;

    // Error code ?
    return true;
}

FileWriter FileWriter::Instance;

}}}; // namespace Scaleform::Render::TGA

