/**************************************************************************

Filename    :   JPEG_ImageWriter.cpp
Content     :   JPEG file format writer implementaion
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

// jpeglib data source constructor, for using File* instead
// of stdio for jpeg IO.
void GJPEGUtil_SetupRwDest(jpeg_compress_struct* pcinfo, File* poutstream);



// ***** GPEG Output helper class

// A jpeglib destination manager that writes to a File.
// Paraphrased from IJG jpeglib jdatadst.c.
class JPEGRwDest : public NewOverrideBase<Stat_Default_Mem>
{
public:
    struct jpeg_destination_mgr DMgr;      // public fields

    Ptr<File>  pOutStream;                 // source stream
    JOCTET     Buffer[JPEG_BufferSize];    // start of Buffer


    // Constructor.  The caller is responsible for closing
    // the output stream after it's done using us.
    JPEGRwDest(File* pout)            
    {
        pOutStream = pout;
        // fill in function pointers...
        DMgr.init_destination       = InitDestination;
        DMgr.empty_output_buffer    = EmptyOutputBuffer;
        DMgr.term_destination       = TermDestination;

        DMgr.next_output_byte       = Buffer;
        DMgr.free_in_buffer         = JPEG_BufferSize;
    }

    static void InitDestination(j_compress_ptr cinfo)
    {
        JPEGRwDest*    pdest = (JPEGRwDest*) cinfo->dest;
    //  SF_ASSERT(dest);

        pdest->DMgr.next_output_byte    = pdest->Buffer;
        pdest->DMgr.free_in_buffer  = JPEG_BufferSize;
    }

    // Write the output Buffer into the stream.
    static boolean  EmptyOutputBuffer(j_compress_ptr cinfo)
    {
        JPEGRwDest*    pdest = (JPEGRwDest*) cinfo->dest;
    //  SF_ASSERT(dest);

        if (pdest->pOutStream->Write(pdest->Buffer, JPEG_BufferSize) != JPEG_BufferSize)
        {
            // Error.
            //throw "JPEGRwDest couldn't write data.";
        SF_DEBUG_WARNING(1, "JPEGRwDest couldn't write data.");
        return 0;
        }

        pdest->DMgr.next_output_byte    = pdest->Buffer;
        pdest->DMgr.free_in_buffer  = JPEG_BufferSize;

        return TRUE;
    }

    // Terminate the destination.  Flush any leftover
    // data, and make sure we get deleted.
    static void TermDestination(j_compress_ptr cinfo)
    {
        JPEGRwDest* pdest = (JPEGRwDest*) cinfo->dest;
    //  SF_ASSERT(dest);

        // Write any remaining data.
        SPInt   datacount = JPEG_BufferSize - pdest->DMgr.free_in_buffer;
        if (datacount > 0) {
            if (pdest->pOutStream->Write(pdest->Buffer, int(datacount)) != datacount)
            {
                // Error.
            //throw "JPEGRwDest::TermDestination couldn't write data.";
            SF_DEBUG_WARNING(1, "JPEGRwDest::TermDestination couldn't write data.");
        return;
            }
        }

        // Clean ourselves up.
        delete pdest;
        cinfo->dest = NULL;
    }
};


// Set up the given compress object to write to the given output stream.
void GJPEGUtil_SetupRwDest(j_compress_ptr cinfo, File* poutstream)
{
    cinfo->dest = (jpeg_destination_mgr*) SF_NEW JPEGRwDest(poutstream);
}


//--------------------------------------------------------------------
// Basically this is a thin wrapper around JpegCompress object. 
class JPEGOutputImpl_jpeglib : public Output
{
public:
    // State needed for JPEGOutput.
    struct jpeg_compress_struct CInfo;
    struct jpeg_error_mgr       JErr;

    // Constructor.  Read the header data from in, and
    // prepare to read data.
    JPEGOutputImpl_jpeglib(File* out, int width, int height, int quality)
    {
        CInfo.err = jpeg_std_error(&JErr);

        // Initialize decompression object.
        jpeg_create_compress(&CInfo);

        GJPEGUtil_SetupRwDest(&CInfo, out);
        CInfo.image_width       = width;
        CInfo.image_height      = height;
        CInfo.input_components  = 3;
        CInfo.in_color_space    = JCS_RGB;
        jpeg_set_defaults(&CInfo);
        jpeg_set_quality(&CInfo, quality, TRUE);

        jpeg_start_compress(&CInfo, TRUE);
    }

    // AB: to use with CopyCriticalParams only!
    JPEGOutputImpl_jpeglib(File* out)
    {
        CInfo.err = jpeg_std_error(&JErr);

        // Initialize decompression object.
        jpeg_create_compress(&CInfo);

        GJPEGUtil_SetupRwDest(&CInfo, out);
    }


    // Destructor.  Clean up our jpeg reader state.
    ~JPEGOutputImpl_jpeglib()
    {
        jpeg_finish_compress(&CInfo);
    /*
        RwDest* psrc = (RwSource*) CInfo.dest;
        delete dest;
        CInfo.dest = NULL;
    */
        jpeg_destroy_compress(&CInfo);
    }


    // Write out a single scanline.
    void    WriteScanline(const UByte* rgbData)
    {
        union {
            const UByte* rgbScanlinePtr;
            JSAMPROW     scanline;
        };
        rgbScanlinePtr = rgbData;

        jpeg_write_scanlines(&CInfo, &scanline, 1);
    }

    void    WriteRawData(const void* prawData)
    {
        jpeg_write_coefficients(&CInfo, (jvirt_barray_ptr*)prawData);
    }

    void CopyCriticalParams(void* pSrcDecompressInfo)
    {
        jpeg_copy_critical_parameters((struct jpeg_decompress_struct*)pSrcDecompressInfo, &CInfo);
    }

    void*   GetCInfo() { return &CInfo; }
};


//--------------------------------------------------------------------
// ***** FileWriter

bool FileWriter::Write(File* file, const ImageData& imageData,
                       const Render::ImageWriteArgs* argsVal) const
{
    if (imageData.Format != Image_R8G8B8)
    {
        // TBD: Update to use conversion below...
        SF_DEBUG_WARNING(1, "JPEG::FileWriter failed - Image_R8G8B8 format expected");
        return false;
    }

    const ImageWriteArgs* args = 0;
    int   quality = 75;
    
    if (argsVal && argsVal->FileFormat == ImageFile_JPEG)
    {
        args = (const ImageWriteArgs*)argsVal;
        quality = args->Quality;
    }

    Output* out = CreateOutput(file, 
                    imageData.GetWidth(), imageData.GetHeight(), quality);
    if (!out) return false;

    // TBD: Convert source image to desired format for writing.
    // ImageScanlineBuffer<1024*4> scanline(imageData.Format, Size.Width, Image_R8G8B8);
    // need to modify ImageScanlineBuffer..

    for (unsigned y = 0; y < imageData.GetHeight(); y++)
        out->WriteScanline(imageData.GetScanline(y));

    delete out;
    return true;
}

Output* FileWriter::CreateOutput(File* pout, int width, int height, int quality) const
{
    if (!pout || !pout->IsWritable()) return 0;
    return SF_NEW JPEGOutputImpl_jpeglib(pout, width, height, quality);
}
Output* FileWriter::CreateOutput(File* pout) const
{
    if (!pout || !pout->IsWritable()) return 0;
    return SF_NEW JPEGOutputImpl_jpeglib(pout);
}

//FileWriter FileWriter::Instance;


}}} // namespace Scaleform::Render::JPEG

#endif // SF_ENABLE_LIBJPEG

