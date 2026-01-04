/**************************************************************************

PublicHeader:   Render
Filename    :   JPEG_ImageFile.h
Content     :   JPEG image file handler and supporting classes.
Created     :   June 24, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_JPEG_ImageFile_H
#define INC_SF_Render_JPEG_ImageFile_H

#include "Render_ImageFile.h"
#include "Render_ImageFileUtil.h"

namespace Scaleform { namespace Render { 

class MemoryBufferImage;

namespace JPEG {

// Declarations
class Input;
class Output;
struct ExtraData;

//--------------------------------------------------------------------
// ***** Abstract JPEG-Support interfaces
//
// AbstractReader adds extra methods that can be used to do
// piecewise reading of JPEG data, as needed for SWF loading.

class AbstractReader : public ImageFileReader
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_JPEG; }
    // 
    virtual ImageSource* CreateImageSource(File* file, const ImageCreateArgs& args, 
        ExtraData* hdr, UInt64 len, bool withHeaders = false) const =0;
    virtual Render::MemoryBufferImage* CreateMemoryBufferImage
        (File* file, const ImageCreateArgs& args, const ImageSize& sz, UInt64 length) =0;
    virtual ImageSource* CreateWrapperImageSource(Render::Image* memImage) const =0;
    virtual Input* CreateInput(File* pin) const = 0;
    virtual Input* CreateSwfJpeg2HeaderOnly(File* pin) const = 0;
    virtual Input* CreateSwfJpeg2HeaderOnly(const UByte* pbuffer, UPInt bufSize) const = 0;
};

class AbstractWriter : public ImageFileWriter
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_JPEG; }
    // 
    virtual Output* CreateOutput(File* pout, int width, int height, int quality) const = 0;
    virtual Output* CreateOutput(File* pout) const = 0;
};


// JPEG::ImageWriteArgs provides optional arguments for JPEG
// FileWriter::Write function.

struct ImageWriteArgs : Render::ImageWriteArgs
{
    ImageWriteArgs()
        : Render::ImageWriteArgs(ImageFile_JPEG), Quality(75)
    { }
    ImageWriteArgs(const ImageWriteArgs& src)
        : Render::ImageWriteArgs(src), Quality(src.Quality)
    { }

    // Quality for JPEG quantization as set by jpeg_set_quality,
    // ranges from 0 to 100.
    int Quality;
};

// A base class for extra data stored in jpeg (headers or alpha channel)

struct ExtraData : public RefCountBase<ExtraData, Stat_Default_Mem>
{
    UByte*  Data;
    UPInt   Size;

    ExtraData(): Data(0), Size(0) {}
    ExtraData(MemoryHeap* heap, UPInt sz) 
        : Size(sz)
    {
        Data = (UByte*)SF_HEAP_ALLOC(heap, sz, Stat_Default_Mem);
    }
    virtual ~ExtraData()
    {
        SF_FREE(Data);
    }

    void Allocate(MemoryHeap* heap, UPInt sz)
    {
        SF_ASSERT(!Data && !Size);
        Data = (UByte*)SF_HEAP_ALLOC(heap, sz, Stat_Default_Mem);
    }

    bool         IsEmpty() const { return Size == 0; }
    virtual bool IsTableHeader() const { return false; }
};


// JPEG::TablesHeader holds and shares JPEG tables segment. Is used if JPEG was 
// split to tables and data.

struct TablesHeader : public ExtraData
{
    TablesHeader(MemoryHeap* heap, UPInt sz) : ExtraData(heap, sz) {}

    virtual bool IsTableHeader() const { return true; }
};


#ifdef SF_ENABLE_LIBJPEG

// ***** Image File Handlers
//
// FileReader provides file format detection for JPEG and its data loading.

class FileReader : public ImageFileReader_Mixin<FileReader, AbstractReader>
{
public:
    virtual bool    MatchFormat(File* file, UByte* header, UPInt headerSize) const;
    virtual ImageSource* ReadImageSource(File* file, const ImageCreateArgs& args) const;

    virtual ImageSource* CreateImageSource(File* file, const ImageCreateArgs& args, ExtraData* hdr, UInt64 len, 
        bool withHeaders = false) const;
    virtual Render::MemoryBufferImage* CreateMemoryBufferImage
        (File* file, const ImageCreateArgs& args, const ImageSize& sz, UInt64 length);
    virtual ImageSource* CreateWrapperImageSource(Render::Image* memImage) const;
    virtual Input*  CreateInput(File* pin) const;
    virtual Input*  CreateSwfJpeg2HeaderOnly(File* pin) const;
    virtual Input*  CreateSwfJpeg2HeaderOnly(const UByte* pbuffer, UPInt bufSize) const;

    // Instance singleton.
    static FileReader Instance;
};

class FileWriter : public ImageFileWriter_Mixin<FileWriter, AbstractWriter>
{
public:
    virtual bool    Write(File* file, const ImageData& imageData,
                          const Render::ImageWriteArgs* args = 0) const;
    
    // Prepares for JPEG writing output. Quality is in 1 to 100 range.
    virtual Output* CreateOutput(File* pout, int width, int height, int quality) const;

    // CreateOutput requires CopyCriticalParams call afterwards!
    virtual Output* CreateOutput(File* pout) const;

    // Instance singleton.
  //  static FileWriter Instance;
};

#endif

//--------------------------------------------------------------------

class Input : public NewOverrideBase<Stat_Default_Mem>
{
public:
    virtual ~Input() { }

    virtual void     DiscardPartialBuffer() = 0;
    virtual bool     StartImage() = 0;
    virtual bool     StartRawImage() = 0;
    virtual bool     FinishImage() = 0;
    virtual bool     AbortImage() = 0;

    virtual ImageSize GetSize() const = 0;    
    virtual bool     ReadScanline(UByte* prgbData) = 0;
    virtual bool     ReadRawData(void** pprawData) = 0;

    // returns jpeg_decompress_struct*
    virtual void*    GetCInfo() = 0;
    virtual bool     HasError() const = 0;
};

// Helper object for writing jpeg image data.
class Output : public NewOverrideBase<Stat_Default_Mem>
{
public:
    virtual ~Output() { }

    // ...
    virtual void    WriteScanline(const UByte* prgbData) = 0;
    virtual void    WriteRawData(const void* prawData) = 0;
    virtual void    CopyCriticalParams(void* pSrcDecompressInfo)= 0;
    // returns jpeg_compress_struct*
    virtual void*   GetCInfo() = 0; 
};

// JPEG memory image, used for standalone jpeg files and built-in
// swf jpegs w/o alpha.
class MemoryBufferImage : public Render::MemoryBufferImage
{
    enum 
    { 
        Flag_Headers = 0x1,
        // 0x2
        Mask_Flags   = 0x3
    };
    union
    {
        ExtraData* pExtraData;
        UPInt      Flags; // only 2 low bits can be used!
    };

    ExtraData* GetExtraData() const;
    unsigned GetFlags() const { return unsigned(Flags) & Mask_Flags; } 
public:
    MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
                      ImageUpdateSync* sync, 
                      File* file, SInt64 filePos, UPInt length = 0, bool headers = false);
    MemoryBufferImage(ExtraData* exd, ImageFormat format, const ImageSize& size, unsigned use,
                      ImageUpdateSync* sync, 
                      File* file, SInt64 filePos, UPInt length = 0);
    MemoryBufferImage(ExtraData* exd, ImageFormat format, const ImageSize& size, unsigned use,
                      ImageUpdateSync* sync, const UByte* data, UPInt len);
    ~MemoryBufferImage();

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
};


}}} // namespace Scaleform::Render::JPEG

#endif // INC_SF_Render_JPEG_ImageFile_H
