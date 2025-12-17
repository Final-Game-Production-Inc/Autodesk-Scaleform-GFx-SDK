/**************************************************************************

PublicHeader:   Render
Filename    :   SIF_ImageFile.h
Content     :   SIF Image file format handler header.
Created     :   July 2011
Authors     :   Dimtry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_SIF_ImageFile_H
#define INC_SF_Render_SIF_ImageFile_H

#include "Render_ImageFile.h"

namespace Scaleform { namespace Render { namespace SIF {

// SIF - Scaleform Image format is used to store platform specific (Swizzled/Packed) formats 

struct SIFHeaderInfo
{
    UByte   HeaderSize;
    ImageFormat Format; 	
    UInt32 Height;			
    UInt32 Width;				
    UInt16 RawPlaneCount;
    UByte  LevelCount;
	UByte Flags;
    //UInt32 DataSize;
};


struct ImageWriteArgs : Render::ImageWriteArgs
{
    ImageWriteArgs()
        : Render::ImageWriteArgs(Render::ImageFile_SIF), Format(Image_None)
    { }
    ImageWriteArgs(const ImageWriteArgs& src)
        : Render::ImageWriteArgs(src), Format(src.Format)
    { }

    ImageFormat Format;
};

//--------------------------------------------------------------------
// ***** Image File Handlers
//
// SIF::FileHandler provides file format detection for SIF and its data loading.
class FileReader : public ImageFileReader_Mixin<FileReader>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_SIF; }

    virtual bool    MatchFormat(File* file, UByte* header, UPInt headerSize) const;
    virtual ImageSource* ReadImageSource(File* file,
                                         const ImageCreateArgs& args = ImageCreateArgs()) const;

    // Instance singleton; to be used for accessing this functionality.
    static FileReader Instance;
};




class FileWriter : public ImageFileWriter_Mixin<FileWriter>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_SIF; }

    virtual bool    Write(File* file, const ImageData& imageData,
                          const Render::ImageWriteArgs* args = 0) const;

    // Instance singleton.
    static FileWriter Instance;
};


}}}; // namespace Scaleform::Render::SIF

#endif // INC_SF_Render_SIF_ImageFile_H
