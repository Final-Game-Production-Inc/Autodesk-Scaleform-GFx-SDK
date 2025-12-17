/**************************************************************************

Filename    :   GDXTHelper.cpp
Content     :   Interface to external compression libraries 
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#define NOMINMAX


#include <nvtt/nvtt.h>

#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_Stream.h"

#include "Render/Render_ResizeImage.h"
#include "Render/Render_TextureUtil.h"

#include "DXTHelper.h"

using namespace Scaleform;
using namespace GFx;
using namespace Render;
/*
union nvColor32 {
    struct {
    UInt8 b, g, r, a;
    };
    UInt32 u;
};
*/
struct nvColor32 {
    UInt8 b, g, r, a;
};

int GDXTHelper::LastError = nvtt::Error_Unknown;


UInt32 GDXTHelper::CalculateDDSSize( UInt32 srcW, UInt32 srcH, ImageFormat format,                              int dxtN,
                                  bool isUncompressedDDS,
                                  bool doGenMipMap,
                                  bool isDxt1Allowed )
{
    nvtt::InputOptions       inputOptions;
    nvtt::CompressionOptions compressionOptions;
    nvtt::Compressor         compressor;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, srcW, srcH);
    inputOptions.setWrapMode(nvtt::WrapMode_Clamp);
    inputOptions.setNormalMap(false);
    inputOptions.setConvertToNormalMap(false);
    inputOptions.setNormalizeMipmaps(false);
    if (doGenMipMap)
        inputOptions.setMipmapGeneration(true);
    else
        inputOptions.setMipmapGeneration(false);

    switch (format)
    {
    case Image_R8G8B8A8:
        if (isUncompressedDDS)
            compressionOptions.setFormat(nvtt::Format_RGBA);
        else
        {
            if (dxtN == 1)
                compressionOptions.setFormat(nvtt::Format_DXT1a);
            else if (dxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_R8G8B8:
        if (isUncompressedDDS)
        {
            if (!doGenMipMap)
                compressionOptions.setFormat(nvtt::Format_RGB); //for current nvtt implementation, same as RGBA
            else
                compressionOptions.setFormat(nvtt::Format_RGBA); // mipmaps should be ALWAYS generated as 32-bit and with dimension power of 2
            // to avoid any resampling in the renderer! (AB)
        }
        else
        {
            if (isDxt1Allowed)
                compressionOptions.setFormat(nvtt::Format_DXT1);
            else if (dxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;
    case Image_A8:
        compressionOptions.setFormat(nvtt::Format_RGBA); //no A_8 format in current implementation of nvtt
        break;

    default:
        break;
    }
    //int tmp=nvtt::estimateSize(inputOptions, compressionOptions);
    return compressor.estimateSize(inputOptions, compressionOptions);
}

//OutputHandler is used by nvtt::compress to write output DDS file
struct GOutputHandler : public nvtt::OutputHandler
{
    GOutputHandler() : Total(0), Progress(0), Percentage(0), pOutFile(NULL) {}
    GOutputHandler(File *pFile ) : Total(0), Progress(0), Percentage(0) 
    {
        pOutFile = pFile;
    }
    virtual ~GOutputHandler() { }

    virtual void setTotal(int t)
    {
        Total = t;
    }

    virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel)
    {
        SF_UNUSED4(size, width, height, depth);
        SF_UNUSED2(face, miplevel);
        // ignore.
    }

    // Output data.
    virtual bool writeData(const void * data, int size)
    {
        return ((int)size == pOutFile->Write((const UByte*)data, (int)size));
        
    }

    int     Total;
    int     Progress;
    int     Percentage;
    File * pOutFile;
};

//OutputHandler is used by nvtt::compress
struct GErrorHandler : public nvtt::ErrorHandler
{
    virtual void error(nvtt::Error e)
    {
        GDXTHelper::LastError=e;    
        SF_ASSERT(0);//
    }
};

static void GImage2NvColor32Buff(const ImageData& srcImg, nvColor32* nvImage)
{
    SF_ASSERT(nvImage);
    for (unsigned y = 0; y < srcImg.GetSize().Height; y++)
    {
        const UByte* pscanline = srcImg.GetScanline(y);
        switch(srcImg.Format)
        {
        case Image_R8G8B8:
            {
                for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 3)
                {
                    nvColor32 & pix = nvImage[y*srcImg.GetWidth()+x];
                    pix.r = pscanline[0];
                    pix.g = pscanline[1];
                    pix.b = pscanline[2];
                    pix.a = 0xFF;
                }
            }
            break;

        case Image_R8G8B8A8:
            {
                for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 4)
                {
                    nvColor32 & pix = nvImage[y*srcImg.GetWidth()+x];
                    pix.r = pscanline[0];
                    pix.g = pscanline[1];
                    pix.b = pscanline[2];
                    pix.a = pscanline[3];

                }

            }
            break;

        case Image_A8:
            {
                for (unsigned x = 0; x < srcImg.GetWidth(); x++, pscanline += 1)
                {
                    nvColor32 & pix = nvImage[y*srcImg.GetWidth()+x];

                    pix.r = pscanline[0];
                    pix.g = pscanline[0];
                    pix.b = pscanline[0];
                    pix.a = pscanline[0];

                }
            }
            break;

                default:
                    SF_ASSERT(0);
        }
    }
}

static nvtt::MipmapFilter NvFilterConst(GDXTHelper::MipFilterTypes filter)
{
    switch(filter)
    {
    case GDXTHelper::nvFilterBox:         return nvtt::MipmapFilter_Box;    
    case GDXTHelper::nvFilterTriangle:    return nvtt::MipmapFilter_Triangle;
    case GDXTHelper::nvFilterKaiser:      return nvtt::MipmapFilter_Kaiser;
    }
    return nvtt::MipmapFilter_Box;
}


static nvtt::Quality NvQualityConst(GDXTHelper::QualitySetting qual)
{
    switch(qual)
    {
    case GDXTHelper::QualityFastest:    return nvtt::Quality_Fastest;
    case GDXTHelper::QualityNormal:     return nvtt::Quality_Normal;
    case GDXTHelper::QualityProduction: return nvtt::Quality_Production;
    case GDXTHelper::QualityHighest:    return nvtt::Quality_Highest;
    }
    SF_ASSERT(0);
    return nvtt::Quality_Fastest;
}


bool GDXTHelper::CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger)
{
    SF_UNUSED(logger);

    DXTSettings* ps = (DXTSettings*) psettings;
    ExportImageHelper::RescaleTypes rescale = ps->Rescale;
    bool isUncompressedDDS = ps->IsUncompressed;
    // If we compress dds or generate mipmaps image must be rescaled
    if (rescale == RescaleNone && (!isUncompressedDDS || ps->DoGenMipMap ))
        rescale = RescaleBiggestPower2;
    //Ptr<RawImage> pimage     = *new RawImage(*psrcimage);
    //Ptr<RawImage> pargbImage = *pimage->ConvertImage(RawImage::Image_ARGB_8888);
	ImageData srcimgData;
	Ptr<RawImage> ptempImage;
	ptempImage = *RawImage::Create(Image_R8G8B8A8, 1, psrcimage->GetSize(), 0);
	ptempImage->GetImageData(&srcimgData);
	psrcimage->Decode(&srcimgData, GetImageConvertFunc(Image_R8G8B8A8, psrcimage->GetFormat())); 
	Ptr<RawImage> scaledImage;
	scaledImage =  *CreateResizedImage(&srcimgData, rescale, ps->RescaleFilter, psettings->bSquare);
	if (!scaledImage)
		scaledImage = ptempImage;

    // Do not compress DDS for images smaller then 4x4
    if ((scaledImage->GetSize().Width < 4) || (scaledImage->GetSize().Height < 4))
        isUncompressedDDS = true;

    nvtt::InputOptions       inputOptions;
    nvtt::CompressionOptions compressionOptions;
    nvtt::Compressor         compressor;
    
    nvtt::MipmapFilter  nvMipFilter = NvFilterConst(ps->MipFilter);
    nvtt::Quality       nvQuality   = NvQualityConst(ps->Quality);

	ImageData scaledImageData;
	scaledImage->GetImageData(&scaledImageData);
    nvColor32 *pimgBuff = (nvColor32*) SF_ALLOC(scaledImageData.GetWidth() * scaledImageData.GetHeight() * sizeof(nvColor32),
        Stat_Image_Mem);
    GImage2NvColor32Buff(scaledImageData, pimgBuff);

    inputOptions.setTextureLayout(nvtt::TextureType_2D, scaledImageData.GetWidth(), scaledImageData.GetHeight());
    inputOptions.setMipmapData(pimgBuff, scaledImageData.GetWidth(), scaledImageData.GetHeight());
    SF_FREE(pimgBuff);

    inputOptions.setWrapMode(nvtt::WrapMode_Clamp);

    inputOptions.setNormalMap(false);
    inputOptions.setConvertToNormalMap(false);
    inputOptions.setGamma(2.2f, 2.2f); //values from nvidia code
    inputOptions.setNormalizeMipmaps(false);

    compressionOptions.setQuality(nvQuality);
    compressor.enableCudaAcceleration(false); //no cuda
    compressionOptions.setColorWeights(1, 1, 1);

    if (ps->DoGenMipMap)
    {
        inputOptions.setMipmapGeneration(true);
        inputOptions.setMipmapFilter(nvMipFilter);
    }
    else
        inputOptions.setMipmapGeneration(false);

    switch (psrcimage->GetFormat())
    {
	case Image_R8G8B8A8:
        if (isUncompressedDDS)
            compressionOptions.setFormat(nvtt::Format_RGBA);
        else
        {
            if (ps->DxtN == 1)
                compressionOptions.setFormat(nvtt::Format_DXT1a);
            else if (ps->DxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;

    case Image_R8G8B8:
        if (isUncompressedDDS)
        {
            if (!ps->DoGenMipMap)
                compressionOptions.setFormat(nvtt::Format_RGB); //for current nvtt implementation, same as RGBA
            else
                compressionOptions.setFormat(nvtt::Format_RGBA); // mipmaps should be ALWAYS generated as 32-bit and with dimension power of 2
            // to avoid any resampling in the renderer! (AB)
        }
        else
        {
            if (ps->IsDxt1Allowed)
                compressionOptions.setFormat(nvtt::Format_DXT1);
            else if (ps->DxtN == 3)
                compressionOptions.setFormat(nvtt::Format_DXT3);
            else
                compressionOptions.setFormat(nvtt::Format_DXT5);
        }
        break;
    case Image_A8:
        compressionOptions.setFormat(nvtt::Format_A8); 
        break;

        default:
            break;
    }

    GOutputHandler outputHandler(pdstFile);
    GErrorHandler  errorHandler;
    outputHandler.setTotal(compressor.estimateSize(inputOptions, compressionOptions));
    
    nvtt::OutputOptions outputOptions;
    outputOptions.setOutputHandler(&outputHandler);
    outputOptions.setErrorHandler(&errorHandler);

    compressor.process(inputOptions, compressionOptions, outputOptions);

    return true;    
}


const char* GDXTHelper::GetLastErrorString()
{
    return nvtt::errorString( (nvtt::Error)GDXTHelper::LastError);
}
