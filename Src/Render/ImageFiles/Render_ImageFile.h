/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ImageFile.h
Content     :   Image file handlers and registry for their support.
Created     :   August 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_ImageFile_H
#define INC_SF_Render_ImageFile_H

#include "Kernel/SF_Array.h"
#include "Render/Render_Image.h"

namespace Scaleform { class File; };

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** ImageFileFormat

// ImageFileFormat enumeration defines values for most common
// supported file types.

enum ImageFileFormat
{
    ImageFile_Unopened,
    ImageFile_Unknown,

    ImageFile_JPEG,    
    ImageFile_DDS,
    ImageFile_PNG,
    ImageFile_TGA,
    ImageFile_PVR,
    ImageFile_KTX,
    ImageFile_GXT,
    ImageFile_GTX,
    ImageFile_GNF,

    ImageFile_SIF,

    ImageFile_Other
};



// Base class for image write arguments.
struct ImageWriteArgs
{    
    ImageFileFormat FileFormat;
    ImageWriteArgs(ImageFileFormat format = ImageFile_Unknown) : FileFormat(format) { }
    ImageWriteArgs(const ImageWriteArgs& src) : FileFormat(src.FileFormat) { }
};


//--------------------------------------------------------------------
// ***** ImageFileHandler

// ImageFileHandler is a base class for ImageFileReader and ImageFileWriter
// classes stored in ImageFileHandlerRegistry.
// Reader and Writer classes are defined and implemented separately to
// avoid linking to unused code.

class ImageFileHandler
{
public:
    virtual ~ImageFileHandler() {}
    virtual ImageFileFormat GetFormat() const       { return ImageFile_Unknown; }
    virtual bool            IsReader() const        { return false; }
    virtual bool            IsWriter() const        { return false; }
};

class ImageFileReader : public ImageFileHandler
{
public:
    virtual bool    IsReader() const { return true; }

    // Detects whether file data in the provided header, or beginning at the current
    // location File pointer, matched our supported file formats.
    // Leave internal File position unchanged.
    virtual bool    MatchFormat(File* file, UByte* header = 0, UPInt headerSize = 0) const = 0;
    
    // Reads image file creating ImageSource object from which data can be decoded.
    // ImageSource should be used as a temporary, as it maintains internal file pointer;
    // user is responsible for calling Release on it.
    virtual ImageSource* ReadImageSource(File* file, const ImageCreateArgs& args = ImageCreateArgs()) const = 0;

    // Creates an image by reading data from a file and applying its arguments.
    virtual Image*  Read(File* file, const ImageCreateArgs& args = ImageCreateArgs()) const;
};

class ImageFileWriter : public ImageFileHandler
{
    friend class ImageFileHandlerRegistry;
protected:
    static bool writeImage(File* file, const ImageFileWriter* writer,
                           Image* image, const ImageWriteArgs* args);

public:
    virtual bool    IsWriter() const { return true; }

    virtual bool    Write(File* file, const ImageData& imageData,
                          const ImageWriteArgs* args = 0) const = 0;
};


//--------------------------------------------------------------------
// ***** File Reader/Writer mixin helper functions

// ImageFileReader_Mixin implements ImageFileReader and injects ReadImage helper
// functions into the derived class.
template<class D, class B = ImageFileReader>
class ImageFileReader_Mixin : public B
{
public:
    static inline Image* ReadImage(File* file, const ImageCreateArgs& args = ImageCreateArgs())
    { return D::Instance.Read(file, args); }

    // ReadImage is used to read from a file created based on filename.
    // We use a template here to avoid direct dependency on SysFile, while also allowing
    // users to use their own file + filename implementation.
    // Used as:
    //     Ptr<Image> image = *Render::TGA::FileReader::ReadImage<SysFile>("fish.tga");
    template<class F, class A>
    static inline Image* ReadImage(const A& fileName,
                                   const ImageCreateArgs& args = ImageCreateArgs())
    {
        F file(fileName);
        return D::Instance.Read(&file, args);
    }
};

// ImageFileWriter_Mixin implements ImageFileWriter and injects WriteImage helper
// functions into the derived class.
template<class D, class B = ImageFileWriter>
class ImageFileWriter_Mixin : public B
{
public:
    static inline bool WriteImage(File* file, const ImageData& imageData,
                                  const ImageWriteArgs* args = 0)
    { return D::Instance.Write(file, imageData, args); }

    template<class F, class A>
    static inline bool WriteImage(const A& fileName, const ImageData& imageData,
                                  const ImageWriteArgs* args = 0)
    {
        F file(fileName, F::Open_Write|F::Open_Create|F::Open_Truncate);
        return D::Instance.Write(&file, imageData, args);
    }

    // Writes image content to file, accessing necessary ImageData either through
    // Image::Map or by decoding to a temporary buffer.
    template<class F, class A>
    static inline bool WriteImage(const A& fileName, Image* image,
                                  const ImageWriteArgs* args = 0)
    {
        F file(fileName, F::Open_Write|F::Open_Create|F::Open_Truncate);
        return B::writeImage(&file, &D::Instance, image, args);
    }

	static inline bool WriteImage(File* file, Image* image,
		const ImageWriteArgs* args = 0)
    { return B::writeImage(file, &D::Instance, image, args); }
};


//--------------------------------------------------------------------
// ***** ImageFileHandlerRegistry

// ImageFileHandlerRegistry maintains a list of ImageFileHandlers, which can
// be used to automatically detect file formats and load image files.
// Registry needs to be initialized with handlers before use, either through
// constructor or the AddHandler function.

/*
Planned use:

SF::Ptr<GFx::ImageFileHandlerRegistry> registry = 
   *SF_NEW GFx::ImageFileHandlerRegistry(5, &SF::Render::JPEG::FileReader::Instance,
                                            &SF::Render::TGA::FileReader::Instance,
                                            &SF::Render::TGA::FileWriter::Instance,
                                            &SF::Render::DDS::FileReader::Instance,
                                            &SF::Render::PNG::FileReader::Instance);
Loader.SetImageFileHandlerRegistry(registry);

*/


class ImageFileHandlerRegistry
{
    Array<ImageFileHandler*> Handlers;
public:

    ImageFileHandlerRegistry(unsigned handlerCount, ...);
    virtual ~ImageFileHandlerRegistry() {}
    
    void            AddHandler(ImageFileHandler* handler);

    // Looks up ImageFileHandler based on format constant.
    ImageFileReader* GetReader(ImageFileFormat format);
    ImageFileWriter* GetWriter(ImageFileFormat format);

    // Detects image file format and returns its type, also filling in a phandler
    // interface that can be used to load it.
    //  - If ImageFile_Unknown is returned, file data isn't supported by any
    //    one of the specified handlers.
    ImageFileFormat DetectFormat(ImageFileReader** preader,
                                 File* file, UByte* header = 0, UPInt headerSize = 0);

    // Attempts to read an image by first detecting its format and then using
    // the appropriate handler. Returns 0 if failed.
    virtual Image*  ReadImage(File* file,
                              const ImageCreateArgs& args = ImageCreateArgs()) const;

    // ReadImage is used to read from a file created based on filename.
    // We use a template here to avoid direct dependency on SysFile, while also allowing
    // users to use their own file + filename implementation.
    // Used as:
    //     Ptr<Image> image = *Registry.ReadImage<SysFile>("fish.tga");
    template<class F, class S>
    inline Image* ReadImage(const S& fileName, const ImageCreateArgs& args = ImageCreateArgs())
    {
        F file(fileName);
        return ReadImage(&file, args);
    }
};


}}; // namespace Scaleform::Render

#endif // INC_SF_Render_ImageFile_H
