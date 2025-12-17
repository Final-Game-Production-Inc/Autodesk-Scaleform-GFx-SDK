/**************************************************************************

Filename    :   JPEG_ImageReader.cpp
Content     :   JPEG ImageReader implementation
Created     :   June 24, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "JPEG_ImageCommon.h"

#ifdef SF_ENABLE_LIBJPEG

namespace Scaleform { namespace Render { namespace JPEG {

// jpeglib data source constructors, for using File* instead
// of stdio for jpeg IO.
static void GJPEGUtil_SetupRwSource(jpeg_decompress_struct* pcinfo, File* pinstream);
static void GJPEGUtil_ReplaceRwSource(jpeg_decompress_struct* pcinfo, File* pinstream);

// ***** GPEG Input helper class

// A jpeglib source manager that reads from a File.  
// Paraphrased from IJG jpeglib jdatasrc.c
class JPEGRwSource : public NewOverrideBase<Stat_Default_Mem>
{
public:
    struct jpeg_source_mgr  SMgr;           // public fields

    Ptr<File> pInStream;                  // source stream
    bool        StartOfFile;                // have we gotten any data yet?
    JOCTET      Buffer[JPEG_BufferSize];    // start of Buffer


    // Constructor.  The caller is responsible for closing the input stream
    // after it's done using us.
    JPEGRwSource(File* pin)           
    {
        pInStream   = pin;
        StartOfFile = 1;
        
        // fill in function pointers...
        SMgr.init_source            = InitSource;
        SMgr.fill_input_buffer      = FillInputBuffer;
        SMgr.skip_input_data        = SkipInputData;
        SMgr.resync_to_restart      = jpeg_resync_to_restart;   // use default method
        SMgr.term_source            = TermSource;
        SMgr.bytes_in_buffer        = 0;
        SMgr.next_input_byte        = NULL;
    }

    JPEGRwSource(const struct jpeg_source_mgr&  smgr)
    {
        pInStream   = 0;
        StartOfFile = 1;
        SMgr = smgr;
    }

    static void InitSource(j_decompress_ptr cinfo)
    {
        JPEGRwSource*  psrc = (JPEGRwSource*) cinfo->src;
        psrc->StartOfFile = true;
    }

    // Read data into our input Buffer.  Client calls this
    // when it needs more data from the file.
    static boolean  FillInputBuffer(j_decompress_ptr cinfo)     
    {
        JPEGRwSource*  psrc = (JPEGRwSource*) cinfo->src;

        UPInt  bytesRead = psrc->pInStream->Read(psrc->Buffer, JPEG_BufferSize);

        if (bytesRead <= 0) {
            // Is the file completely empty?
            if (psrc->StartOfFile) {
                // Treat this as a fatal error.
            //throw "empty jpeg source stream.";
            SF_DEBUG_WARNING(1, "empty jpeg source stream.");
        return 0;
            }
            // Warn("jpeg end-of-stream");

            // Insert a fake EOI marker.
            psrc->Buffer[0] = (JOCTET) 0xFF;
            psrc->Buffer[1] = (JOCTET) JPEG_EOI;
            bytesRead = 2;
        }

        // Hack to work around SWF bug: sometimes data
        // starts with FFD9FFD8, when it should be
        // FFD8FFD9!
        if (psrc->StartOfFile && bytesRead >= 4)
        {
            if (psrc->Buffer[0] == 0xFF
                && psrc->Buffer[1] == 0xD9 
                && psrc->Buffer[2] == 0xFF
                && psrc->Buffer[3] == 0xD8)
            {
                psrc->Buffer[1] = 0xD8;
                psrc->Buffer[3] = 0xD9;
            }
        }

        // Expose Buffer state to clients.
        psrc->SMgr.next_input_byte = psrc->Buffer;
        psrc->SMgr.bytes_in_buffer = bytesRead;
        psrc->StartOfFile = 0;

        return TRUE;
    }

    // Called by client when it wants to advance past some
    // uninteresting data.
    static void SkipInputData(j_decompress_ptr cinfo, long numBytes)
    {
        JPEGRwSource*  psrc = (JPEGRwSource*) cinfo->src;

        // According to jpeg docs, large skips are
        // infrequent.  So let's just do it the simple
        // way.
        if (numBytes > 0)
        {
            while (numBytes > (long) psrc->SMgr.bytes_in_buffer)
            {
                numBytes -= (long) psrc->SMgr.bytes_in_buffer;
                FillInputBuffer(cinfo);
            }
            // Handle remainder.
            psrc->SMgr.next_input_byte += (UPInt) numBytes;
            psrc->SMgr.bytes_in_buffer -= (UPInt) numBytes;
        }
    }

    // Terminate the source.  Make sure we get deleted.
    static void TermSource(j_decompress_ptr cinfo)
    {
        SF_UNUSED(cinfo);

        /*JPEGRwSource*    psrc = (JPEGRwSource*) cinfo->src;
        SF_ASSERT(psrc);

        // @@ it's kind of bogus to be deleting here
        // -- TermSource happens at the end of
        // reading an image, but we're probably going
        // to want to init a source and use it to read
        // many images, without reallocating our Buffer.
        delete psrc;
        cinfo->src = NULL;*/
    }


    void    DiscardPartialBuffer()
    {
        // Discard existing bytes in our Buffer.
        SMgr.bytes_in_buffer = 0;
        SMgr.next_input_byte = NULL;
    }
};


// Set up the given decompress object to read from the given stream.
void    GJPEGUtil_SetupRwSource(jpeg_decompress_struct* cinfo, File* pinstream)
{
    // SF_ASSERT(cinfo->src == NULL);
    cinfo->src = (jpeg_source_mgr*) SF_NEW JPEGRwSource(pinstream);
}

void    GJPEGUtil_ReplaceRwSource(jpeg_decompress_struct* cinfo, File* pinstream)
{
    // SF_ASSERT(cinfo->src == NULL);
    JPEGRwSource* psrc = (JPEGRwSource*) cinfo->src;
    delete psrc;
    cinfo->src = (jpeg_source_mgr*) SF_NEW JPEGRwSource(pinstream);
}


// Basically this is a thin wrapper around JpegDecompress object.   
class JPEGInputImpl_jpeglib : public Input
{
public:
    // State needed for JPEGInput.
    struct jpeg_decompress_struct   CInfo;
    struct JpegErrorHandler         JErr;
private:
    static int JpegCreateDecompress(jpeg_decompress_struct* pcinfo, JpegErrorHandler* pjerr);
    static int JpegReadHeader(jpeg_decompress_struct* pcinfo, JpegErrorHandler* pjerr, bool require_image);

    static void InitSource(j_decompress_ptr ) {}
    static void TermSource(j_decompress_ptr ) {}

public:
    bool    CompressorOpened :1;
    bool    ErrorOccurred    :1;
    bool    Initialized      :1;


    enum SWF_DEFINE_BITS_JPEG2 { SWF_JPEG2 };
    enum SWF_DEFINE_BITS_JPEG2_HEADER_ONLY { SWF_JPEG2_HEADER_ONLY };


    // Constructor.  Read the header data from in, and
    // prepare to read data.
    JPEGInputImpl_jpeglib(File* pin)      
    {
        Initialized = CompressorOpened = ErrorOccurred = false;

        CInfo.err = SetupJpegErr(&JErr);

        // Initialize decompression object.
        if (!JpegCreateDecompress(&CInfo, &JErr))
            return;

        GJPEGUtil_SetupRwSource(&CInfo, pin);

        if (!StartImage())
            return;
        Initialized = true;
    }


    // The SWF file format stores JPEG images with the
    // encoding tables separate from the image data.  This
    // constructor reads the encoding table only and keeps
    // them in this object.  You need to call
    // StartImage() and FinishImage() around any calls
    // to GetWidth/height/components and ReadScanline.
    JPEGInputImpl_jpeglib(SWF_DEFINE_BITS_JPEG2_HEADER_ONLY e, File* pin)
    {
        SF_UNUSED(e);

        Initialized = CompressorOpened = ErrorOccurred = false;

        CInfo.err = SetupJpegErr(&JErr);

        // Initialize decompression object.
        if (!JpegCreateDecompress(&CInfo, &JErr))
            return;

        GJPEGUtil_SetupRwSource(&CInfo, pin);

        // Read the encoding tables.
        if (!JpegReadHeader(&CInfo, &JErr, FALSE))
            return;

        // Don't start reading any image data!
        // App does that manually using StartImage.
        Initialized = true;
    }

    JPEGInputImpl_jpeglib(SWF_DEFINE_BITS_JPEG2_HEADER_ONLY e, const UByte* pbuf, UPInt bufSize)
    {
        SF_UNUSED(e);
        struct jpeg_source_mgr smgr;
        memset(&smgr, 0, sizeof(smgr));
        smgr.bytes_in_buffer        = bufSize;
        smgr.next_input_byte        = pbuf;
        smgr.init_source            = InitSource;
        smgr.term_source            = TermSource;

        Initialized = CompressorOpened = ErrorOccurred = false;

        CInfo.err = SetupJpegErr(&JErr);

        // Initialize decompression object.
        if (!JpegCreateDecompress(&CInfo, &JErr))
            return;

        CInfo.src = (jpeg_source_mgr*) SF_NEW JPEGRwSource(smgr);

        // Read the encoding tables.
        if (!JpegReadHeader(&CInfo, &JErr, FALSE))
            return;

        // Don't start reading any image data!
        // App does that manually using StartImage.
        Initialized = true;
    }

    // Destructor.  Clean up our jpeg reader state.
    ~JPEGInputImpl_jpeglib()
    {
        FinishImage();

        JPEGRwSource* psrc = (JPEGRwSource*) CInfo.src;
        delete psrc;
        CInfo.src = NULL;


        jpeg_destroy_decompress(&CInfo);
    }

    bool HasError() const { return ErrorOccurred; }
    bool IsValid() const { return Initialized && !HasError(); }

    // Discard any data sitting in our JPEGInput Buffer.  Use
    // this before/after reading headers or partial image
    // data, to avoid screwing up future reads.
    void    DiscardPartialBuffer()
    {
        JPEGRwSource* psrc = (JPEGRwSource*) CInfo.src;

        // We only have to discard the JPEGInput Buffer after reading the tables.
        if (psrc)
            psrc->DiscardPartialBuffer();
    }


    // This is something you can do with "abbreviated"
    // streams; i.e. if you constructed this inputter
    // Using (SWFJPEG2HEADERONLY) to just load the
    // tables, or if you called FinishImage() and want to
    // load another image using the existing tables.
    bool    StartImage()
    {
        if (ErrorOccurred) return false;
        SF_DEBUG_WARNING(CompressorOpened != false, "JPEGInput::StartImage - image already started");

        SF_JPEG_SET_JMP1(this, false);

        // Now, read the image header.
        // MA: If already in READY state (202), don't call read_header. This makes
        // loading work for SW8. Need to reseatcj JPeg integration further.
        if (CInfo.global_state != 202) 
            jpeg_read_header(&CInfo, TRUE);

        // NOTE: Always request RGB images. 
        CInfo.out_color_space = JCS_RGB;
        jpeg_start_decompress(&CInfo);
        CompressorOpened = true;
        return true;
    }

    bool    StartRawImage()
    {
        if (ErrorOccurred) return false;
        SF_DEBUG_WARNING(CompressorOpened != false, "JPEGInput::StartRawImage - image already started");

        SF_JPEG_SET_JMP1(this, false);

        // Now, read the image header.
        // MA: If already in READY state (202), don't call read_header. This makes
        // loading work for SW8. Need to reseatcj JPeg integration further.
        if (CInfo.global_state != 202) 
            jpeg_read_header(&CInfo, TRUE);

        // NOTE: Always request RGB images. 
        CInfo.out_color_space = JCS_RGB;
        CompressorOpened = true;
        return true;
    }

    bool    FinishImage()
    {
        if (ErrorOccurred) return false;
        if (CompressorOpened)
        {
            SF_JPEG_SET_JMP1(this, false);
            jpeg_finish_decompress(&CInfo);
            CompressorOpened = false;
        }
        return true;
    }

    // Abort reading image if it wasn't finished yet.
    bool    AbortImage()
    {
        if (ErrorOccurred) return false;
        if (CompressorOpened)
        {
            SF_JPEG_SET_JMP1(this, false);
            jpeg_abort_decompress(&CInfo);
            CompressorOpened = false;
        }
        return true;
    }

    // Return the height of the image.  Take the data from our cinfo struct.
    ImageSize GetSize() const
    {
    //  SF_ASSERT(CompressorOpened);
        return ImageSize(CInfo.output_width, CInfo.output_height);
    }

    // Return number of Components (i.e. == 3 for RGB
    // data).  The size of the data for a scanline is
    // GetWidth() * GetComponents().
    int GetComponents() const
    {
    //  SF_ASSERT(CompressorOpened);
        return CInfo.output_components;
    }


    // Read a scanline's worth of image data into the
    // given Buffer.  The amount of data read is
    // GetWidth() * GetComponents().
    bool    ReadScanline(unsigned char* prgbData)
    {
        if (ErrorOccurred) return false;
        SF_DEBUG_ERROR(CompressorOpened != true, "JPEGInput::ReadScanline - image not yet started");
        SF_JPEG_SET_JMP1(this, false);
            
    //  SF_ASSERT(cinfo.OutputScanline < cinfo.OutputHeight);
        int LinesRead = jpeg_read_scanlines(&CInfo, &prgbData, 1);
    //  SF_ASSERT(LinesRead == 1);
        SF_UNUSED(LinesRead);  // avoid warning in NDEBUG
        return true;
    }

    bool    ReadRawData(void** pprawData)
    {
        if (ErrorOccurred) return false;
        SF_JPEG_SET_JMP1(this, false);
        jvirt_barray_ptr * src_coef_arrays = jpeg_read_coefficients(&CInfo);
        *pprawData = src_coef_arrays;
        return true;
    }

    void*   GetCInfo() { return &CInfo; }

};

int JPEGInputImpl_jpeglib::JpegCreateDecompress(jpeg_decompress_struct* pcinfo,
                                                JpegErrorHandler* pjerr)
{
    SF_UNUSED(pjerr); // in case if SF_NO_LONGJMP is defined.
    SF_JPEG_SET_JMP(pcinfo, pjerr, 0);

    // Initialize decompression object.
    jpeg_create_decompress(pcinfo);

    return 1;
}

int JPEGInputImpl_jpeglib::JpegReadHeader(jpeg_decompress_struct* pcinfo,
                                          JpegErrorHandler* pjerr, bool require_image)
{
    SF_UNUSED(pjerr); // in case if SF_NO_LONGJMP is defined.
    SF_JPEG_SET_JMP(pcinfo, pjerr, 0);

    jpeg_read_header(pcinfo, require_image);
    return 1;
}




static bool DecodeHelper(ImageFormat format, Input *jin,
                         ImageData* pdest, Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageSize                   size = jin->GetSize();
    bool                        success = false;
    ImageScanlineBuffer<1024*4> scanline(Image_R8G8B8, size.Width, format);

    if (scanline.IsValid() && !jin->HasError())
    {
        // Read data by-scanline and convert it.
        success = true;
        for (unsigned y = 0; y < size.Height; y++)
        {
            if (!jin->ReadScanline(scanline.GetReadBuffer()))
            {
                success = false;
                break; // read error!
            }
            UByte* destScanline = pdest->GetScanline(y);
            scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
        }
    }

    delete jin;
    return success;
}


//--------------------------------------------------------------------
// ***** JPEG::ImageSource

// Create either from stand-along file (save for file length)
// or from file buffer (user must provide size).

class ImageSource : public FileImageSource
{    
    mutable Input*      pOriginalInput;
    Ptr<ExtraData>      pExtraData;
    bool                WithHeaders;

public:
    ImageSource(File* file, ImageFormat format = Image_None, ExtraData* exd = 0, UInt64 len = 0, bool withHeaders = false)
        : FileImageSource(file, format, len), pOriginalInput(0), pExtraData(exd), WithHeaders(withHeaders)
    { 
    }

    ~ImageSource()
    {
        if (pOriginalInput)
        {
            // Since data wasn't processed, abort JPEG.
            pOriginalInput->AbortImage();
            delete pOriginalInput;
        }
    }
    
    bool ReadHeader();

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    // Creates an image out of source that is compatible with argument Use,
    // updateSync and other settings.
    virtual Image*      CreateCompatibleImage(const ImageCreateArgs& args);
    virtual unsigned    GetMipmapCount() const { return 1;}
};

bool ImageSource::ReadHeader()
{
    SF_ASSERT(pOriginalInput == 0);
    if (!pExtraData || !pExtraData->IsTableHeader())
    {
        if (!WithHeaders) // regular jpeg from file
            pOriginalInput = FileReader::Instance.CreateInput(pFile);
        else
        {
            // used by swf jpeg w/o alpha
            pOriginalInput = FileReader::Instance.CreateSwfJpeg2HeaderOnly(pFile);
            pOriginalInput->StartImage();
        }
    }
    else
    {
        // used by swf jpeg w/splitted header
        pOriginalInput = FileReader::Instance.CreateSwfJpeg2HeaderOnly(pExtraData->Data, pExtraData->Size);
        if (!pOriginalInput)
            return false;
        struct jpeg_decompress_struct* cinfo = static_cast<struct jpeg_decompress_struct*>(pOriginalInput->GetCInfo());
        // replace source by file.
        GJPEGUtil_ReplaceRwSource(cinfo, pFile);
        pOriginalInput->StartImage();
    }
    if (!pOriginalInput)
        return false;
    Size = pOriginalInput->GetSize();
    if (Format == Image_None)
        Format = Image_R8G8B8;
    return true;
}

bool ImageSource::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    Input* jin = pOriginalInput;

    // Small optimization - use original input to avoid it
    // being created/destroyed an extra time on file loading.
    if (jin)
    {
        pOriginalInput = 0;
    }
    else
    {
        if (!seekFileToDecodeStart())
            return false;
        jin = FileReader::Instance.CreateInput(pFile);
        if (!jin) return 0;
    }

    return DecodeHelper(Format, jin, pdest, copyScanline, arg);
}


// ***** JPEG MemoryBufferImage
MemoryBufferImage::MemoryBufferImage(ImageFormat format, const ImageSize& size, unsigned use,
                                     ImageUpdateSync* sync, 
                                     File* file, SInt64 filePos, UPInt length, bool headers)
 : Render::MemoryBufferImage(format,size, use, sync,
                             file, filePos, length), pExtraData(0)
{
    if (headers)
        Flags |= Flag_Headers;
}

MemoryBufferImage::MemoryBufferImage(ExtraData* exd, ImageFormat format, const ImageSize& size, unsigned use,
                                     ImageUpdateSync* sync, 
                                     File* file, SInt64 filePos, UPInt length)
 : Render::MemoryBufferImage(format,size, use, sync,
                             file, filePos, length), pExtraData(exd)
{ 
    if (pExtraData) 
    {
        pExtraData->AddRef(); 
        if (pExtraData->IsTableHeader())
            Flags |= Flag_Headers;
    }
}

MemoryBufferImage::MemoryBufferImage(ExtraData* exd, ImageFormat format, const ImageSize& size, unsigned use,
                                     ImageUpdateSync* sync, 
                                     const UByte* data, UPInt len)
 : Render::MemoryBufferImage(format,size, use, sync, data, len),
   pExtraData(exd)
{ 
    if (pExtraData) 
    {
        pExtraData->AddRef(); 
        if (pExtraData->IsTableHeader())
            Flags |= Flag_Headers;
    }
}

MemoryBufferImage::~MemoryBufferImage()
{
    if ((Flags & Mask_Flags) && GetExtraData())
        GetExtraData()->Release();
}

ExtraData* MemoryBufferImage::GetExtraData() const 
{ 
    union
    {
        ExtraData* pExtraData;
        UPInt      Flags; // only 2 low bits can be used!
    } u;
    u.Flags = (Flags & (~Mask_Flags));
    return u.pExtraData;
}

bool MemoryBufferImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    MemoryFile file(FilePath, &FileData[0], (int)FileData.GetSize());
    Input*     jin;
    if (GetExtraData() == 0)
    {
        // swf jpeg no alpha
        jin = FileReader::Instance.CreateSwfJpeg2HeaderOnly(&file);
        if (jin)
            jin->StartImage();
    }
    else
    {
        // used by swf jpeg w/splitted header
        jin = FileReader::Instance.CreateSwfJpeg2HeaderOnly(GetExtraData()->Data, GetExtraData()->Size);
        struct jpeg_decompress_struct* cinfo = static_cast<struct jpeg_decompress_struct*>(jin->GetCInfo());
        // replace source by file.
        GJPEGUtil_ReplaceRwSource(cinfo, &file);
        if (jin)
            jin->StartImage();
    }

    if (!jin) return 0;

    return DecodeHelper(Format, jin, pdest, copyScanline, arg);
}

Image* ImageSource::CreateCompatibleImage(const ImageCreateArgs& args)
{
    if (IsDecodeOnlyImageCompatible(args))
    {
        if (pExtraData)
            return SF_HEAP_NEW(args.GetHeap())
                MemoryBufferImage(pExtraData, GetFormat(), GetSize(),
                                  args.Use, args.GetUpdateSync(),
                                  pFile, FilePos, (UPInt)FileLen);
        else
            return SF_HEAP_NEW(args.GetHeap())
                MemoryBufferImage(GetFormat(), GetSize(),
                                  args.Use, args.GetUpdateSync(),
                                  pFile, FilePos, (UPInt)FileLen, WithHeaders);
    }
    return Render::ImageSource::CreateCompatibleImage(args);
}

class WrapperImageSource : public Render::WrapperImageSource
{
    mutable Input*      pOriginalInput;
public:
    WrapperImageSource(Image* pdelegate) : Render::WrapperImageSource(pdelegate), pOriginalInput(0) {}
    ~WrapperImageSource()
    {
        if (pOriginalInput)
        {
            // Since data wasn't processed, abort JPEG.
            pOriginalInput->AbortImage();
            delete pOriginalInput;
        }
    }

    bool ReadHeader();
};

bool WrapperImageSource::ReadHeader()
{
    SF_ASSERT(pOriginalInput == 0);

    Render::MemoryBufferImage* img = pDelegate->GetAsMemoryImage();
    if (img)
    {
        const UByte* fileData;
        UPInt        fileSz;
        img->GetImageDataBits(&fileData, &fileSz);

        pOriginalInput = FileReader::Instance.CreateSwfJpeg2HeaderOnly(fileData, fileSz);
        if (!pOriginalInput)
            return false;

        pOriginalInput->StartImage();
        img->SetImageSize(pOriginalInput->GetSize());
        if (img->GetFormat() == Image_None)
            img->SetFormat(Image_R8G8B8);
        return true;
    }
    return false;
}


//--------------------------------------------------------------------
// ***** FileReader

bool FileReader::MatchFormat(File* file, UByte* headerArg, UPInt headerArgSize) const
{    
    FileHeaderReader<2> header(file, headerArg, headerArgSize);
    if (!header || (header[0] != 0xFF) || (header[1] != 0xD8))
        return false;
    return true;
}

Render::ImageSource* FileReader::CreateImageSource(File* file, const ImageCreateArgs& args, 
                                                   ExtraData* exd, UInt64 len, bool withHeaders) const
{
    if (!file || !file->IsValid())
        return 0;
    JPEG::ImageSource* source = SF_NEW JPEG::ImageSource(file, args.Format, exd, len, withHeaders);
    if (source && !source->ReadHeader())
    {
        source->Release();
        source = 0;
    }
    return source;
}

Render::MemoryBufferImage* FileReader::CreateMemoryBufferImage
    (File* file, const ImageCreateArgs& args, const ImageSize& sz, UInt64 length)
{
    return SF_HEAP_NEW(args.GetHeap())
        MemoryBufferImage(Image_None, sz,
                          args.Use, args.GetUpdateSync(),
                          file, file->LTell(), (UPInt)length);
}

Render::ImageSource* FileReader::CreateWrapperImageSource(Render::Image* memImage) const
{
    JPEG::WrapperImageSource* source = SF_NEW JPEG::WrapperImageSource(memImage);
    if (source && !source->ReadHeader())
    {
        source->Release();
        source = 0;
    }
    return source;
}


Render::ImageSource* FileReader::ReadImageSource(File* file, const ImageCreateArgs& args) const
{
    if (!file || !file->IsValid())
        return 0;
    JPEG::ImageSource* source = SF_NEW JPEG::ImageSource(file, args.Format);
    if (source && !source->ReadHeader())
    {
        source->Release();
        source = 0;
    }
    return source;
}

Input* FileReader::CreateInput(File* pin) const
{
    if (!pin || !pin->IsValid()) return 0;
    JPEGInputImpl_jpeglib* jpegInput = SF_NEW JPEGInputImpl_jpeglib(pin);
    if (jpegInput && jpegInput->IsValid())
        return jpegInput;
    delete jpegInput;
    return NULL;
}
Input* FileReader::CreateSwfJpeg2HeaderOnly(File* pin) const
{
    if (!pin || !pin->IsValid()) return 0;
    JPEGInputImpl_jpeglib* jpegInput = SF_NEW JPEGInputImpl_jpeglib(JPEGInputImpl_jpeglib::SWF_JPEG2_HEADER_ONLY, pin);
    if (jpegInput && jpegInput->IsValid())
        return jpegInput;
    delete jpegInput;
    return NULL;
}
Input* FileReader::CreateSwfJpeg2HeaderOnly(const UByte* pbuffer, UPInt bufSize) const
{
    JPEGInputImpl_jpeglib* jpegInput = SF_NEW JPEGInputImpl_jpeglib(JPEGInputImpl_jpeglib::SWF_JPEG2_HEADER_ONLY,
                                                                    pbuffer, bufSize);
    if (jpegInput && jpegInput->IsValid())
        return jpegInput;
    delete jpegInput;
    return NULL;
}

// Singleton instance.
FileReader FileReader::Instance;


}}} // namespace Scaleform::Render::JPEG

#endif // SF_ENABLE_LIBJPEG

