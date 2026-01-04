/**************************************************************************

PublicHeader:   Render
Filename    :   PNG_ImageFile.h
Content     :   PNG Image file format handler header.
Created     :   February 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_PNG_ImageFile_H
#define INC_SF_Render_PNG_ImageFile_H

#include "Render_ImageFile.h"

namespace Scaleform { namespace Render { namespace PNG {

//--------------------------------------------------------------------
// ***** Image File Handlers
//
// PNG::FileHandler provides file format detection for PNG and its data loading.

class Input : public NewOverrideBase<Stat_Default_Mem>
{
public:

    virtual ~Input() { }
    virtual int ReadData(void** ppData) =0;
    virtual int ReadScanline (UByte* prgbData) =0;
    virtual bool Decode(ImageFormat format, ImageData* pdest, Image::CopyScanlineFunc copyScanline, void* arg) =0;
    virtual ImageSize GetSize() const =0;

};

class FileReader : public ImageFileReader_Mixin<FileReader>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_PNG; }
    virtual Input*  CreateInput(File* pin) const;
    virtual bool    MatchFormat(File* file, UByte* header, UPInt headerSize) const;
    virtual ImageSource* ReadImageSource(File* file,
                                         const ImageCreateArgs& args = ImageCreateArgs()) const;

    // Instance singleton; to be used for accessing this functionality.
    static FileReader Instance;
};

class FileWriter : public ImageFileWriter_Mixin<FileWriter>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_PNG; }

    virtual bool    Write(File* file, const ImageData& imageData,
                          const ImageWriteArgs* args = 0) const;

    // Instance singleton.
    static FileWriter Instance;
};


}}}; // namespace Scaleform::Render::PNG

#endif // INC_SF_Render_PNG_ImageFile_H
