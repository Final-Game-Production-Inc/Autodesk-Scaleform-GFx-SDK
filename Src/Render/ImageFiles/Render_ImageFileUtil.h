/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ImageFileUtil.h
Content     :   Helper classes for ImageFile format loading
Created     :   August 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_ImageFileUtil_H
#define INC_SF_Render_ImageFileUtil_H

#include "Render_ImageFile.h"
#include "Kernel/SF_String.h"

namespace Scaleform { class File; };

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** FileImageSource

// FileImageSource is a partial implementation of ImageSource used for reading
// files; to that end it stores file pointer and its starting position.

class FileImageSource : public ImageSource
{
public:
    FileImageSource(File* file, ImageFormat format = Image_None, UInt64 len = 0);
    ~FileImageSource();
    
    virtual ImageFormat GetFormat() const { return Format; }
    virtual unsigned    GetUse() const    { return Use; }
    virtual ImageSize   GetSize() const   { return Size; }

    virtual Render::Image* GetAsImage() { return NULL; }
    SF_AMP_CODE(
        virtual UInt32  GetImageId() const { return ImageId; }
        virtual UInt32  GetBaseImageId() const { return 0; }
    )

protected:    
    bool            seekFileToDecodeStart() const;

    ImageFormat     Format;
    ImageSize       Size;
    unsigned        Use;
    Ptr<File>       pFile;
    SInt64          FilePos;
    UInt64          FileLen;
    SF_AMP_CODE(UInt32 ImageId;)
};

// MemoryBufferImage is a partial implementation of Image that stores raw
// file data in a buffer, assuming that it can be read through a MemoryFile
// by its overridden Decode implementation.
//  - Used by JPEG implementation.

class MemoryBufferImage : public Render::Image
{
public:
    MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
                      ImageUpdateSync* sync,
                      File* file, SInt64 filePos, UPInt length = 0);

    MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
                      ImageUpdateSync* sync, const UByte* data, UPInt dataSize);

    virtual ImageFormat GetFormat() const { return Format; }
    virtual unsigned    GetUse() const    { return Use; }
    virtual ImageSize   GetSize() const   { return Size; }
    virtual Texture*    GetTexture(TextureManager* pmanager);
    virtual unsigned    GetMipmapCount() const {return 1;}

    // Decode must be implemented in derived class.
    virtual bool Decode(ImageData* pdest,
                        CopyScanlineFunc copyScanline, void* arg) const = 0;

    virtual Image*              GetAsImage() { return this; }
    virtual MemoryBufferImage*  GetAsMemoryImage() { return this; }

    bool GetImageDataBits(const UByte** ppdata, UPInt* pdataLen)
    {
        *ppdata = &FileData[0];
        *pdataLen= FileData.GetSize();
        return true;
    }
    void         SetImageSize(ImageSize isz)  { Size = isz; }
    void         SetFormat(ImageFormat f)     { Format = f; }


    SF_AMP_CODE(
        virtual UInt32  GetImageId() const { return ImageId; }
        virtual UInt32  GetBaseImageId() const { return 0; }
    )
protected:
    ImageFormat     Format;
    ImageSize       Size;
    unsigned        Use;    
    ArrayLH<UByte>  FileData;
    StringLH        FilePath;
    SF_AMP_CODE(UInt32 ImageId;)
};


//--------------------------------------------------------------------
// ***** FileHeaderReader

class FileHeaderReaderImpl
{
protected:
    UByte* pHeader;
public:
    FileHeaderReaderImpl(File* file, UByte* header, UPInt headerSize,
                         UByte* tempBuffer, UByte sizeNeeded);
    UByte* GetPtr() const { return pHeader; }
};

// FileHeaderReader is a helpers for MatchFormat implementation.
// Verifies that file is valid and that there are specified number of bytes pointed
// to by header. Expects tempBiffer to have specified size.

template<int SZ>
class FileHeaderReader : public FileHeaderReaderImpl
{    
    UByte Buffer[SZ];
public:
    FileHeaderReader(File* file, UByte* header = 0, UPInt headerSize = 0)
        : FileHeaderReaderImpl(file, header, headerSize, Buffer, SZ)
    { }

    UByte operator[] (unsigned i) const { SF_ASSERT(i < SZ); return pHeader[i]; }
    operator bool  () const             { return pHeader != 0; }
    bool operator ! () const            { return pHeader == 0; }
};


//--------------------------------------------------------------------
// ***** ImageScanlineBuffer

class ImageScanlineBufferImpl
{
    typedef Image::CopyScanlineFunc CopyFunc;

    ImageFormat ReadFormat, ConvertSourceFormat;
    unsigned    Width;
    UByte      *pReadScanline, *pConvertScanline;
    UPInt       ReadScanlineSize, ConvertScanlineSize;
    CopyFunc    pConvertFunc;
    bool        BuffersAllocated;
public:

    ImageScanlineBufferImpl(ImageFormat readFormat,
                            unsigned width, ImageFormat convertSourceFormat,
                            UByte* tempBuffer = 0, UPInt tempBufferSize = 0);
    ~ImageScanlineBufferImpl();

    // After class is created, IsValid should be called to check
    // successful initialization.
    bool IsValid() const
    { return (ReadFormat != Image_None) && Width && pReadScanline; }

    // Return address of reserved scanline buffer for reading,
    // it should be a destination for file Read.
    UByte* GetReadBuffer() const    { return pReadScanline; }
    UPInt  GetReadSize() const      { return ReadScanlineSize; }

    // Copies read buffer to destination with scan-line conversion;
    // note that up to two conversions are possible internally.
    //  - copyScanline will receive data in ConvertSourceFormat.
    void   ConvertReadBuffer(UByte* dest, Palette* pal,
                             CopyFunc copyScanline = &Image::CopyScanlineDefault,
                             void* scanlineArg = 0);
};

// ImageScanlineBuffer is a memory buffer used for storing Image scanlines 
// read from file and converting them to an a desired format. It is intended 
// to be allocated on stack, reserving a pre-defined amount of space and 
// dynamically allocation more if necessary.
// 
// ImageScanlineBuffer is used by Image::Decode implementations.
// Two passes are possible during conversion:
//   1. Convert from image-file format to ConvertSourceFormat (optional)
//   2. Convert to Decode destination format with users copyScanline function.
// First step is necessary if raw image-file scanline doesn't match
// ImageFormat requested during image creation.

template<int SZ>
class ImageScanlineBuffer : public ImageScanlineBufferImpl
{
    UByte   TempBuffer[SZ];

public:
    ImageScanlineBuffer(ImageFormat readFormat, unsigned width,
        ImageFormat convertSourceFormat)
        : ImageScanlineBufferImpl(readFormat, width, convertSourceFormat,
        TempBuffer, SZ)
    { }
};


}}; // namespace Scaleform::Render

#endif // INC_SF_Render_ImageFileUtil_H
