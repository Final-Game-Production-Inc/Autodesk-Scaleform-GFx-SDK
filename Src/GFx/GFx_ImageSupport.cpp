/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageSupport.cpp
Content     :   
Created     :   Sep, 2010
Authors     :   Artem Bolgar

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Types.h"
#include "GFx_ImageSupport.h"
#include "GFx_Loader.h"
#include "GFx/GFx_Stream.h"
#include "Kernel/SF_HeapNew.h"
#include "GFx/GFx_ZLibFile.h"

namespace Scaleform { namespace GFx {

#ifdef SF_ENABLE_ZLIB

#ifdef SF_ENABLE_LIBJPEG

using Render::JPEG::Input;
using Render::JPEG::FileReader;

#else

typedef void * Input;

#endif

using Render::Image;
using Render::ImageScanlineBuffer;
using Render::ImageSize;
using Render::ImageFormat;

static const UInt16 GFx_UndoPremultiplyTable[256] = 
{
    65280,65280,32640,21760,16320,13056,10880, 9325, 8160, 7253, 6528, 5934, 5440, 5021, 4662, 4352, 
     4080, 3840, 3626, 3435, 3264, 3108, 2967, 2838, 2720, 2611, 2510, 2417, 2331, 2251, 2176, 2105, 
     2040, 1978, 1920, 1865, 1813, 1764, 1717, 1673, 1632, 1592, 1554, 1518, 1483, 1450, 1419, 1388, 
     1360, 1332, 1305, 1280, 1255, 1231, 1208, 1186, 1165, 1145, 1125, 1106, 1088, 1070, 1052, 1036, 
     1020, 1004,  989,  974,  960,  946,  932,  919,  906,  894,  882,  870,  858,  847,  836,  826,  
      816,  805,  796,  786,  777,  768,  759,  750,  741,  733,  725,  717,  709,  701,  694,  687,  
      680,  672,  666,  659,  652,  646,  640,  633,  627,  621,  615,  610,  604,  598,  593,  588,  
      582,  577,  572,  567,  562,  557,  553,  548,  544,  539,  535,  530,  526,  522,  518,  514,  
      510,  506,  502,  498,  494,  490,  487,  483,  480,  476,  473,  469,  466,  462,  459,  456,  
      453,  450,  447,  444,  441,  438,  435,  432,  429,  426,  423,  421,  418,  415,  413,  410,  
      408,  405,  402,  400,  398,  395,  393,  390,  388,  386,  384,  381,  379,  377,  375,  373,  
      370,  368,  366,  364,  362,  360,  358,  356,  354,  352,  350,  349,  347,  345,  343,  341,  
      340,  338,  336,  334,  333,  331,  329,  328,  326,  324,  323,  321,  320,  318,  316,  315,  
      313,  312,  310,  309,  307,  306,  305,  303,  302,  300,  299,  298,  296,  295,  294,  292,  
      291,  290,  288,  287,  286,  285,  283,  282,  281,  280,  278,  277,  276,  275,  274,  273,  
      272,  270,  269,  268,  267,  266,  265,  264,  263,  262,  261,  260,  259,  258,  257,  256
};


// Helper routine to undo pre-multiply by alpha, stored in SWF files.
SF_INLINE static void    UndoPremultiplyAlpha(UByte *prgb, UByte a)
{
    //int undoVal = GFx_PremultiplyTable[a]; //GFx_GetPremultiplyAlphaUndoTable()[a];
    //// This can probably be optimized... (with binary ops and/or SSE/MMX)
    //prgb[0] =  UByte( G_Clamp<int>((undoVal * (int)prgb[0]) >> 8, 0, 255) );
    //prgb[1] =  UByte( G_Clamp<int>((undoVal * (int)prgb[1]) >> 8, 0, 255) );
    //prgb[2] =  UByte( G_Clamp<int>((undoVal * (int)prgb[2]) >> 8, 0, 255) );

    unsigned undoVal = GFx_UndoPremultiplyTable[a];
    prgb[0] = UByte((undoVal * ((prgb[0] <= a) ? prgb[0] : a)) >> 8);
    prgb[1] = UByte((undoVal * ((prgb[1] <= a) ? prgb[1] : a)) >> 8);
    prgb[2] = UByte((undoVal * ((prgb[2] <= a) ? prgb[2] : a)) >> 8);
}

namespace {
struct Params
{
    ImageScanlineBuffer<1024*2> SourceScanline;
    ImageScanlineBuffer<1024>   AlphaScanline;

    ImageScanlineBuffer<1024*2> ScanlineWithAlpha0;
    ImageScanlineBuffer<1024*2> ScanlineWithAlpha1;
    ImageScanlineBuffer<1024*2> ScanlineWithAlpha2;
    ImageScanlineBuffer<1024*2>* ScanlineWithAlphas[3];
    ImageScanlineBuffer<1024*4> FinalScanline;
    Ptr<File>                   ZlibFile;
    Input*                      Jin;
    unsigned                    Width;
    bool                        Success;

    Params(Input *jin, unsigned width, ImageFormat format) : 
        SourceScanline(Render::Image_R8G8B8, width, Render::Image_R8G8B8),
        AlphaScanline(Render::Image_A8, width, Render::Image_A8),
        ScanlineWithAlpha0(Render::Image_R8G8B8A8, width + 2, Render::Image_R8G8B8A8),
        ScanlineWithAlpha1(Render::Image_R8G8B8A8, width + 2, Render::Image_R8G8B8A8),
        ScanlineWithAlpha2(Render::Image_R8G8B8A8, width + 2, Render::Image_R8G8B8A8),
        FinalScanline(Render::Image_R8G8B8A8, width, format),
        Jin(jin),
        Width(width),
        Success(true)
    {
        ScanlineWithAlphas[0] = &ScanlineWithAlpha0; ScanlineWithAlphas[1] = &ScanlineWithAlpha1; ScanlineWithAlphas[2] = &ScanlineWithAlpha2; 
        memset(ScanlineWithAlpha0.GetReadBuffer(), 0, ScanlineWithAlpha0.GetReadSize());
    }
};
}

// A special filter to restore lost colors after the alpha-premultiplication operation per scanline.
static void UndoPremultiplyAlphaScanline(Params& params)
{
    UByte* imgSl = params.FinalScanline.GetReadBuffer();
    const UByte* line0 = params.ScanlineWithAlphas[0]->GetReadBuffer();
    const UByte* line1 = params.ScanlineWithAlphas[1]->GetReadBuffer();
    const UByte* line2 = params.ScanlineWithAlphas[2]->GetReadBuffer();
    memcpy(imgSl, line1 + 4, params.FinalScanline.GetReadSize());
    for (unsigned x = 0; x < params.Width; ++x)
    {
        const UByte* pl1 = line1 + x * 4 + 4; 
        const UByte* p5 = pl1;
        UByte* pi = imgSl + x * 4;

        if (pi[3] < 16)
        {
            const UByte* pl0 = line0 + x * 4 + 4; 
            const UByte* pl2 = line2 + x * 4 + 4; 
            const UByte* p1 = pl0 - 4;
            const UByte* p2 = pl0;
            const UByte* p3 = pl0 + 4;
            const UByte* p4 = pl1 - 4;
            const UByte* p6 = pl1 + 4;
            const UByte* p7 = pl2 - 4;
            const UByte* p8 = pl2;
            const UByte* p9 = pl2 + 4;

            // Calculate the weighted mean of 9 pixels. Sum(x[i] * w[i]) / Sum(w[i]);
            // Here x[i] is the color component, w[i] is alpha. So that, we calculate 
            // the denomonator once and the numerators for each channel. Note that we 
            // do not need to multiply by alpha because the components are already premultiplied.
            // We only need to scale the color values to the appropriate range (multiply by 256).
            unsigned den = unsigned(p1[3]) + p2[3] + p3[3] + p4[3] + p5[3] + p6[3] + p7[3] + p8[3] + p9[3];
            if (den)
            {
                unsigned r = ((p1[0] + p2[0] + p3[0] + p4[0] + p5[0] + p6[0] + p7[0] + p8[0] + p9[0]) << 8) / den;
                unsigned g = ((p1[1] + p2[1] + p3[1] + p4[1] + p5[1] + p6[1] + p7[1] + p8[1] + p9[1]) << 8) / den;
                unsigned b = ((p1[2] + p2[2] + p3[2] + p4[2] + p5[2] + p6[2] + p7[2] + p8[2] + p9[2]) << 8) / den;
                pi[0] = UByte((r <= 255) ? r : 255);
                pi[1] = UByte((g <= 255) ? g : 255);
                pi[2] = UByte((b <= 255) ? b : 255);
            }

            // An alternative method. Just fing the neighbor pixel with the maximal alpha
            //unsigned  a = p5[3];
            //if(p1[3] > a) { pi[0] = p1[0]; pi[1] = p1[1]; pi[2] = p1[2]; a = p1[3]; }
            //if(p2[3] > a) { pi[0] = p2[0]; pi[1] = p2[1]; pi[2] = p2[2]; a = p2[3]; }
            //if(p3[3] > a) { pi[0] = p3[0]; pi[1] = p3[1]; pi[2] = p3[2]; a = p3[3]; }
            //if(p4[3] > a) { pi[0] = p4[0]; pi[1] = p4[1]; pi[2] = p4[2]; a = p4[3]; }
            //if(p6[3] > a) { pi[0] = p6[0]; pi[1] = p6[1]; pi[2] = p6[2]; a = p6[3]; }
            //if(p7[3] > a) { pi[0] = p7[0]; pi[1] = p7[1]; pi[2] = p7[2]; a = p7[3]; }
            //if(p8[3] > a) { pi[0] = p8[0]; pi[1] = p8[1]; pi[2] = p8[2]; a = p8[3]; }
            //if(p9[3] > a) { pi[0] = p9[0]; pi[1] = p9[1]; pi[2] = p9[2]; a = p9[3]; }
        }
        else
        {
            UndoPremultiplyAlpha(pi, pi[3]);
        }
    }
}

#ifdef SF_ENABLE_LIBJPEG

static bool ReadAndUniteScanline(Params& params, UByte* fullData)
{
	UByte* jpegData = params.SourceScanline.GetReadBuffer();
	UByte* alphaData = params.AlphaScanline.GetReadBuffer();

	// read jpeg scanline...
	if (!params.Jin->ReadScanline(jpegData))
	{
		params.Success = false;
		return false; // read error!
	}
	// read zlib scanline
	if (params.ZlibFile->Read(alphaData, (int)params.AlphaScanline.GetReadSize()) <= 0)
	{
		params.Success = false;
		return false; // read error!
	}

	// unite them...
	for (unsigned i = 0, j = 0, a = 0; a < params.Width; i += 3, j += 4, ++a)
	{
		fullData[j]        = jpegData[i];
		fullData[j + 1]    = jpegData[i + 1];
		fullData[j + 2]    = jpegData[i + 2];
		fullData[j + 3]    = alphaData[a];
	}
	return true;
}

static bool JpegAlphaDecodeHelper(ImageFormat format, Input *jin, ZlibSupportBase* zlib, const UByte* alphaZlibData, UPInt alphaZlibDataSize,
                                  ImageData* pdest, Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageSize size = jin->GetSize();
    MemoryFile file("", alphaZlibData, (int)alphaZlibDataSize);

    Params params(jin, size.Width, format);
    params.ZlibFile = *zlib->CreateZlibFile(&file);

    if (!jin->HasError())
    {
        unsigned finalScanlineIdx = 1;
        unsigned linesRead = 0;
        // Read data by-scanline and convert it.
        for (unsigned y = 0; y < size.Height + 1; ++y)
        {
            UByte* fullData = params.ScanlineWithAlphas[finalScanlineIdx]->GetReadBuffer();
            reinterpret_cast<UInt32*>(fullData)[0] = 0;
            reinterpret_cast<UInt32*>(fullData)[size.Width + 1] = 0;
            if (y < size.Height)
            {
                if (!ReadAndUniteScanline(params, fullData + 4))
                    goto error_exit;
            }
            else
                memset(fullData, 0, params.ScanlineWithAlphas[finalScanlineIdx]->GetReadSize());
            ++linesRead;
            if (linesRead >= 2)
            {
                // so we can undo alpha pre-multiplication here.
                // always do it for line with index 1.
                UndoPremultiplyAlphaScanline(params);

                UByte* destScanline = pdest->GetScanline(y - 1); // we read one line ahead
                params.FinalScanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
            }
            if (finalScanlineIdx == 2)
            {
                Alg::Swap(params.ScanlineWithAlphas[0], params.ScanlineWithAlphas[2]);
                Alg::Swap(params.ScanlineWithAlphas[0], params.ScanlineWithAlphas[1]);
            }
            else
                ++finalScanlineIdx;
        }
    }
    else
        params.Success = false;

error_exit:
    delete jin;
    return params.Success;
}

MemoryBufferJpegImageWithZlibAlphas::MemoryBufferJpegImageWithZlibAlphas
                                   (ZlibSupportBase* zlib, Render::JPEG::AbstractReader* reader,
                                    unsigned alphaPos,
                                    ImageFormat format, const ImageSize& size, unsigned use,
                                    ImageUpdateSync* sync, 
                                    File* file, UPInt length)
: ZLib(zlib), JpegReader(reader), ZlibAlphaOffset(alphaPos), Format(format)
{ 
    Render::ImageCreateArgs args;
    args.Use            = use;
    args.pUpdateSync    = sync;
    args.Format         = Render::Image_R8G8B8;
    args.pHeap          = Memory::GetHeapByAddress(this);
    pImage = *reader->CreateMemoryBufferImage(file, args, size, length);
}

bool MemoryBufferJpegImageWithZlibAlphas::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    const UByte* fileData;
    UPInt        fileSz;
    static_cast<Render::MemoryBufferImage*>(pImage.GetPtr())->GetImageDataBits(&fileData, &fileSz);
    Input*     jin = JpegReader->CreateSwfJpeg2HeaderOnly(fileData, fileSz);
    if (!jin) return 0;
    // jin will be deleted by JpegAlphaDecodeHelper
    jin->StartImage();
    return JpegAlphaDecodeHelper(Format, jin, ZLib, fileData + ZlibAlphaOffset, 
        fileSz - ZlibAlphaOffset, pdest, copyScanline, arg);
}

Render::Texture* MemoryBufferJpegImageWithZlibAlphas::GetTexture(Render::TextureManager* pmanager)
{
    if (pTexture && pTexture->GetTextureManager() == pmanager)
    {
        return pTexture;
    }

    pTexture = 0;
    Render::Texture* ptexture = pmanager->CreateTexture(GetFormat(), 1, GetSize(), GetUse(), this);
    initTexture_NoAddRef(ptexture);
    return ptexture;
}

#endif

/// ZLIB ////
namespace
{
    struct ZlibDecodeParams
    {
        ImageFormat                             Format;
        ZlibImageSource::SourceBitmapDataFormat SrcFormat;
        ImageSize                               Size;
        Ptr<File>                               ZlibFile;
        unsigned                                ColorTableSize;
    };
}

static bool ZlibDecodeColorMapped(const ZlibDecodeParams& params, ImageData* pdest, 
                                  Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageScanlineBuffer<256*3> colorMap(Render::Image_R8G8B8, params.ColorTableSize, Render::Image_R8G8B8);
    ImageScanlineBuffer<1024> sourceScanline(Render::Image_A8,  params.Size.Width, Render::Image_A8);
    ImageScanlineBuffer<1024*2> finalScanline(Render::Image_R8G8B8,  params.Size.Width, params.Format);
    int colorMapSize = (int)params.ColorTableSize*3;

    const int bytesPerPixel = 1;
    const int pitch = (params.Size.Width * bytesPerPixel + 3) & ~3;
    // read color table first
    if (params.ZlibFile->Read(colorMap.GetReadBuffer(), colorMapSize) != colorMapSize)
        return false; // read error!
    UByte* inRow = sourceScanline.GetReadBuffer();
    UByte* outRow = finalScanline.GetReadBuffer();
    UByte* colorTable = colorMap.GetReadBuffer();

    for (unsigned y = 0; y < params.Size.Height; ++y)
    {
        // read zlib scanline
        if (params.ZlibFile->Read(inRow, pitch) != pitch)
            return false; // read error!

        for (unsigned x = 0; x < params.Size.Width; ++x)
        {
            UByte   pixel = inRow[x];
            outRow[x * 3 + 0] = colorTable[pixel * 3 + 0]; // r
            outRow[x * 3 + 1] = colorTable[pixel * 3 + 1]; // g
            outRow[x * 3 + 2] = colorTable[pixel * 3 + 2]; // b
        }
        UByte* destScanline = pdest->GetScanline(y); // we read one line ahead
        finalScanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
    }
    return true;
}

static bool ZlibDecodeColorMappedAlpha(const ZlibDecodeParams& params, ImageData* pdest, 
                                       Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageScanlineBuffer<256*4> colorMap(Render::Image_R8G8B8A8, params.ColorTableSize, Render::Image_R8G8B8A8);
    Params data(0, params.Size.Width, params.Format);
    unsigned finalScanlineIdx = 1;
    unsigned linesRead = 0;

    UByte* colorTable = colorMap.GetReadBuffer();
    int colorMapSize = (int)params.ColorTableSize*4;

    const int bytesPerPixel = 1;
    const int pitch = (params.Size.Width * bytesPerPixel + 3) & ~3;
    // read color table first
    if (params.ZlibFile->Read(colorMap.GetReadBuffer(), colorMapSize) != colorMapSize)
        return false; // read error!

    for (unsigned y = 0; y < params.Size.Height + 1; ++y)
    {
        UByte* fullData = data.ScanlineWithAlphas[finalScanlineIdx]->GetReadBuffer();
        reinterpret_cast<UInt32*>(fullData)[0] = 0;
        reinterpret_cast<UInt32*>(fullData)[params.Size.Width + 1] = 0;
        if (y < params.Size.Height)
        {
            UByte* scanline = data.AlphaScanline.GetReadBuffer();
            if (params.ZlibFile->Read(scanline, pitch) != pitch)
                return false;
            UByte* outRow = fullData + 4;
            for (unsigned x = 0; x < params.Size.Width; ++x)
            {
                UByte   pixel = scanline[x];
                outRow[x * 4 + 0] = colorTable[pixel * 4 + 0]; // r
                outRow[x * 4 + 1] = colorTable[pixel * 4 + 1]; // g
                outRow[x * 4 + 2] = colorTable[pixel * 4 + 2]; // b
                outRow[x * 4 + 3] = colorTable[pixel * 4 + 3]; // alpha
            }
        }
        else
            memset(fullData, 0, data.ScanlineWithAlphas[finalScanlineIdx]->GetReadSize());
        ++linesRead;
        if (linesRead >= 2)
        {
            // so we can undo alpha pre-multiplication here.
            // always do it for line with index 1.
            UndoPremultiplyAlphaScanline(data);

            UByte* destScanline = pdest->GetScanline(y - 1); // we read one line ahead
            data.FinalScanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
        }
        if (finalScanlineIdx == 2)
        {
            Alg::Swap(data.ScanlineWithAlphas[0], data.ScanlineWithAlphas[2]);
            Alg::Swap(data.ScanlineWithAlphas[0], data.ScanlineWithAlphas[1]);
        }
        else
            ++finalScanlineIdx;
    }
    return true;
}

static bool ZlibDecodeRGB16(const ZlibDecodeParams& params, ImageData* pdest, 
                            Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageScanlineBuffer<1024*2> sourceScanline(Render::Image_A8,  params.Size.Width*2, Render::Image_A8);
    ImageScanlineBuffer<1024*4> finalScanline(Render::Image_R8G8B8A8,  params.Size.Width, params.Format);

    UByte* inRow = sourceScanline.GetReadBuffer();
    const int bytesPerPixel = 2;
    const int pitch = (params.Size.Width * bytesPerPixel + 3) & ~3;
    UByte* outRow = finalScanline.GetReadBuffer();

    for (unsigned y = 0; y < params.Size.Height; ++y)
    {
        // read zlib scanline
        if (params.ZlibFile->Read(inRow, pitch) != pitch)
            return false; // read error!

        for (unsigned x = 0; x < params.Size.Width; ++x)
        {
            UInt16  pixel = inRow[x * 2] | (inRow[x * 2 + 1] << 8);

            // Format must be RGB 555.
            outRow[x * 4 + 0] = UByte( (pixel >> 7) & 0xF8 );  // red
            outRow[x * 4 + 1] = UByte( (pixel >> 2) & 0xF8 );  // green
            outRow[x * 4 + 2] = UByte( (pixel << 3) & 0xF8 );  // blue
            outRow[x * 4 + 3] = 0xff;                          // alpha
        }
        UByte* destScanline = pdest->GetScanline(y); // we read one line ahead
        finalScanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
    }
    return true;
}

static bool ZlibDecodeRGB24(const ZlibDecodeParams& params, ImageData* pdest, 
                            Image::CopyScanlineFunc copyScanline, void* arg)
{
    ImageScanlineBuffer<1024*4> scanline(Render::Image_R8G8B8A8,  params.Size.Width, params.Format);

    UByte* scanlinePtr = scanline.GetReadBuffer();
    const int bytesPerPixel = 4;
    const int pitch = (params.Size.Width * bytesPerPixel + 3) & ~3;

    for (unsigned y = 0; y < params.Size.Height; ++y)
    {
        // read zlib scanline
        if (params.ZlibFile->Read(scanlinePtr, pitch) != pitch)
            return false; // read error!

        for (unsigned x = 0; x < params.Size.Width; ++x)
        {
            // Need to re-arrange ARGB into RGB.
            //UByte   a = scanlinePtr[x * 4 + 0]; // not used, reserved
            UByte   r = scanlinePtr[x * 4 + 1];
            UByte   g = scanlinePtr[x * 4 + 2];
            UByte   b = scanlinePtr[x * 4 + 3];
            scanlinePtr[x * 4 + 0] = r;
            scanlinePtr[x * 4 + 1] = g;
            scanlinePtr[x * 4 + 2] = b;
            scanlinePtr[x * 4 + 3] = 0xff;
        }
        UByte* destScanline = pdest->GetScanline(y); // we read one line ahead
        scanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
    }
    return true;
}

static bool ZlibDecodeRGBA(const ZlibDecodeParams& params, ImageData* pdest, 
                           Image::CopyScanlineFunc copyScanline, void* arg)
{
    Params data(0, params.Size.Width, params.Format);
    unsigned finalScanlineIdx = 1;
    unsigned linesRead = 0;

    for (unsigned y = 0; y < params.Size.Height + 1; ++y)
    {
        UByte* fullData = data.ScanlineWithAlphas[finalScanlineIdx]->GetReadBuffer();
        reinterpret_cast<UInt32*>(fullData)[0] = 0;
        reinterpret_cast<UInt32*>(fullData)[params.Size.Width + 1] = 0;
        if (y < params.Size.Height)
        {
            int numBytes = params.Size.Width * 4;
            UByte* scanline = fullData + 4;
            if (params.ZlibFile->Read(scanline, numBytes) != numBytes)
                return false;
            for (unsigned x = 0; x < params.Size.Width; ++x)
            {
                // Need to re-arrange ARGB into RGBA.
                UByte   a = scanline[x * 4 + 0]; 
                UByte   r = scanline[x * 4 + 1];
                UByte   g = scanline[x * 4 + 2];
                UByte   b = scanline[x * 4 + 3];
                scanline[x * 4 + 0] = r;
                scanline[x * 4 + 1] = g;
                scanline[x * 4 + 2] = b;
                scanline[x * 4 + 3] = a;
            }
        }
        else
            memset(fullData, 0, data.ScanlineWithAlphas[finalScanlineIdx]->GetReadSize());
        ++linesRead;
        if (linesRead >= 2)
        {
            // so we can undo alpha pre-multiplication here.
            // always do it for line with index 1.
            UndoPremultiplyAlphaScanline(data);

            UByte* destScanline = pdest->GetScanline(y - 1); // we read one line ahead
            data.FinalScanline.ConvertReadBuffer(destScanline, 0, copyScanline, arg);
        }
        if (finalScanlineIdx == 2)
        {
            Alg::Swap(data.ScanlineWithAlphas[0], data.ScanlineWithAlphas[2]);
            Alg::Swap(data.ScanlineWithAlphas[0], data.ScanlineWithAlphas[1]);
        }
        else
            ++finalScanlineIdx;
    }
    return true;
}

static bool ZlibDecodeHelper(const ZlibDecodeParams& params, ImageData* pdest, Image::CopyScanlineFunc copyScanline, void* arg)
{
    //Params data(0, params.Size.Width, params.Format);
    //data.ZlibFile = params.ZlibFile;
    switch(params.SrcFormat)
    {
    case ZlibImageSource::ColorMappedRGB: // color mapped, no alpha
        return ZlibDecodeColorMapped(params, pdest, copyScanline, arg);
    case ZlibImageSource::ColorMappedRGBA: // color mapped with alpha
        return ZlibDecodeColorMappedAlpha(params, pdest, copyScanline, arg);
    case ZlibImageSource::RGB16:
        return ZlibDecodeRGB16(params, pdest, copyScanline, arg);
    case ZlibImageSource::RGB24:
        return ZlibDecodeRGB24(params, pdest, copyScanline, arg);
    case ZlibImageSource::RGBA: // rgba
        return ZlibDecodeRGBA(params, pdest, copyScanline, arg);
    default:
        SF_ASSERT(0);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
//
ZlibImageSource::ZlibImageSource(ZlibSupportBase* zlib, File* file, const ImageSize& size, SourceBitmapDataFormat bmpFormatId, 
                                 ImageFormat format, UInt16 colorTableSize, UInt64 uncompressedLen)
: 
    Render::FileImageSource(file, format, uncompressedLen), 
    Zlib(zlib),
    ColorTableSize(colorTableSize),
    BitmapFormatId(bmpFormatId)
{ 
    Size = size;
}

bool ZlibImageSource::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    ZlibDecodeParams params;
    params.Format     = Format;
    params.SrcFormat  = BitmapFormatId;
    params.Size       = Size;
    params.ZlibFile   = *Zlib->CreateZlibFile(pFile);
    params.ColorTableSize = ColorTableSize;
    return ZlibDecodeHelper(params, pdest, copyScanline, arg);
}

Render::Image* ZlibImageSource::CreateCompatibleImage(const Render::ImageCreateArgs& args)
{
    if (IsDecodeOnlyImageCompatible(args))
    {
        return SF_HEAP_NEW(args.GetHeap())
            MemoryBufferZlibImage(Zlib, GetFormat(), GetSize(), BitmapFormatId, ColorTableSize,
            args.Use, args.GetUpdateSync(),
            pFile, FilePos, (UPInt)FileLen);
    }
    return Render::ImageSource::CreateCompatibleImage(args);
}

//////////////////////////////////////////////////////////////////////////
bool MemoryBufferZlibImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    ZlibDecodeParams params;
    params.Format     = Format;
    params.SrcFormat  = BitmapFormatId;
    params.Size       = Size;
    MemoryFile mfile("", &FileData[0], (int)FileData.GetSize());
    params.ZlibFile   = *Zlib->CreateZlibFile(&mfile);
    params.ColorTableSize = ColorTableSize;
    bool rv = ZlibDecodeHelper(params, pdest, copyScanline, arg);
    params.ZlibFile   = NULL;
    return rv;
}

#endif

}} // Scaleform::GFx
