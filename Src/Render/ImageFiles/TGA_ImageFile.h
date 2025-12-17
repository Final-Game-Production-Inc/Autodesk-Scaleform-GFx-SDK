/**************************************************************************

PublicHeader:   Render
Filename    :   TGA_ImageFile.h
Content     :   TGA Image file format handler header.
Created     :   February 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TGA_ImageFile_H
#define INC_SF_Render_TGA_ImageFile_H

#include "Render_ImageFile.h"

namespace Scaleform { namespace Render { namespace TGA {

//--------------------------------------------------------------------
// ***** Image File Handlers
//
// TGA::FileHandler provides file format detection for TGA and its data loading.
class FileReader : public ImageFileReader_Mixin<FileReader>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_TGA; }

    virtual bool    MatchFormat(File* file, UByte* header, UPInt headerSize) const;
    virtual ImageSource* ReadImageSource(File* file,
                                         const ImageCreateArgs& args = ImageCreateArgs()) const;

    // Instance singleton; to be used for accessing this functionality.
    static FileReader Instance;
};

class FileWriter : public ImageFileWriter_Mixin<FileWriter>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_TGA; }

    virtual bool    Write(File* file, const ImageData& imageData,
                          const ImageWriteArgs* args = 0) const;

    // Instance singleton.
    static FileWriter Instance;
};


//--------------------------------------------------------------------
// TGA file format constant value/bit definitions.

enum TGAColorMapType
{
    TGAColorMap_NotIncluded  = 0,
    TGAColorMap_Included     = 1,
};

enum TGAImageType
{
    TGAImage_NoData             = 0,
    TGAImage_ColorMapped        = 1,
    TGAImage_TrueColor          = 2,
    TGAImage_BlackAndWhite      = 3,
    TGAImage_RLE_ColorMapped    = 9,
    TGAImage_RLE_TrueColor      = 10,
    TGAImage_RLE_BlackAndWhite  = 11,
};

enum TGAImageDescBits
{
    TGAImageDesc_TopToBottom = 0x20,
    TGAImageDesc_LeftToRight = 0x10,
    TGAImageDesc_ZeroMaskBits= 0xC0 // Top high bits are 0.
};

}}}; // namespace Scaleform::Render::TGA

#endif // INC_SF_Render_TGA_ImageFile_H
