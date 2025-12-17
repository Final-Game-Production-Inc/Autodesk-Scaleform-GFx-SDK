/**************************************************************************

Filename    :   PVRHelper.cpp
Content     :   Interface to external compression libraries 
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#define NOMINMAX

#include "PVRTextureUtilities.h"

#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_Stream.h"

#include "Render/Render_ResizeImage.h"
#include "Render/Render_TextureUtil.h"
// Standard includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <zlib.h>

#include "../GFxExport.h"
#include "PVRHelper.h"
#include "Kernel/SF_MsgFormat.h"
using namespace Scaleform;
using namespace GFx;
using namespace Render;

using namespace pvrtexture;

static PixelType GetPvrFormat(ImageFormat format)
{
    switch (format)
    {
    case Image_A8:
        return PixelType('a',0,0,0, 8,0,0,0);
    case Image_R8G8B8:
        return PixelType('r','g','b',0,8,8,8,0);
    }
    return PixelType('r','g','b','a',8,8,8,8);
}


bool GPVRHelper::CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger)
{
    PVRSettings* ps = (PVRSettings*) psettings;
    ExportImageHelper::RescaleTypes rescale = ps->Rescale;
    String  errorMsg;

    if (rescale == RescaleNone || rescale == RescalePreScale) //looks like PVRTC formats require power of 2
        rescale = RescaleBiggestPower2;
    ImageData srcimgData;
    Ptr<RawImage> ptempImage;
    ptempImage = *RawImage::Create(Image_R8G8B8A8, 1, psrcimage->GetSize(), 0);
    ptempImage->GetImageData(&srcimgData);
    psrcimage->Decode(&srcimgData, GetImageConvertFunc(Image_R8G8B8A8, psrcimage->GetFormat())); 
    Ptr<RawImage> scaledImage;
    scaledImage =  *CreateResizedImage(&srcimgData, rescale, ps->RescaleFilter, true);
    if (!scaledImage)
        scaledImage = ptempImage;

    ImageData scaledImageData;
    scaledImage->GetImageData(&scaledImageData);
    ImageFormat srcFormat = psrcimage->GetFormat();
    bool hasAlpha = (srcFormat == Image_R8G8B8A8 || srcFormat == Image_A8);
    PixelType format;
    unsigned minMipSize = 1;
    if (ps->IsUncompressed || 
        scaledImageData.GetWidth() < 8 || scaledImageData.GetHeight() < 8 ||
        (ps->PVRn == 2 && (scaledImageData.GetWidth() < 16 || scaledImageData.GetHeight() < 16)))
    {
        format = GetPvrFormat(srcFormat);
    }
    else if (ps->Format == PVRSettings::FormatETC)
    {
        if  (hasAlpha)//currently ETC does not support alpha
        {
            format = GetPvrFormat(srcFormat);
            logger->OnMessage(GFxExportListener::MSG_Warning, "ETC compression does not support alpha. Saving uncompressed texture.");
        }
        else
        {
            format = ePVRTPF_ETC1;
            minMipSize = 8;
        }
    }
    else
    {
        if (ps->PVRn == 2)
        {
            minMipSize = 16;
            format = hasAlpha ? PixelType(ePVRTPF_PVRTCI_2bpp_RGBA) : PixelType(ePVRTPF_PVRTCI_2bpp_RGB);
        }
        else
        {
            minMipSize = 8;
            format = hasAlpha ? PixelType(ePVRTPF_PVRTCI_4bpp_RGBA) : PixelType(ePVRTPF_PVRTCI_4bpp_RGB);
        }
    }

    UInt32 mipcount = 0;
    if (ps->DoGenMipMap)
    {
        UInt32 size = Alg::Max<UInt32>(scaledImageData.GetWidth(), scaledImageData.GetHeight());
        while ( (size >>= 1) >= minMipSize )
            ++mipcount;
    }

    CPVRTextureHeader header(
        PixelType('r','g','b','a',8,8,8,8).PixelTypeID,//format.PixelTypeID,
        scaledImageData.GetHeight(),
        scaledImageData.GetWidth()
        );
    // write to file specified
    CPVRTexture cTexture(header, scaledImageData.GetDataPtr());
    // Generate MIP-map chain
    if (mipcount)
        GenerateMIPMaps(cTexture, eResizeCubic, mipcount);

    if (!Transcode(cTexture, format, ePVRTVarTypeUnsignedInteger, ePVRTCSpacelRGB))
        return false;
    String opath = pdstFile->GetFilePath();
    String path = opath;
    bool shareImages = path.GetExtension() == ".$$$";
    if (shareImages)
        path += ".pvr";
    pdstFile->Close();       
    cTexture.saveFile(path.ToCStr());
    if (shareImages)
    {
        remove(opath.ToCStr());
        rename(path.ToCStr(), opath.ToCStr());
    }

    return true;
}
