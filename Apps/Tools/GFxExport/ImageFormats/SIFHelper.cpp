/**************************************************************************

Filename    :   SIFHelper.cpp
Content     :   Interface to external compression libraries 
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#define NOMINMAX


#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_Stream.h"
#include "Render/Render_TextureUtil.h"
#include "Render/ImageFiles/SIF_ImageFile.h"

#include "../GFxExport.h"
#include "SIFHelper.h"
#include "Platform_export.h"
#include "Kernel/SF_MsgFormat.h"
#include "../ConsoleSupport.h"

using namespace Scaleform;
using namespace GFx;
using namespace Render;

bool GSIFHelper::CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger)
{
    SIFSettings *psifsettings = (SIFSettings*)psettings;
    GFxExportConsolePlugin* pexporter = FindConsole(psifsettings->Target);
    if (!pexporter)
        return false;
    ExportImageHelper::RescaleTypes rescale = psifsettings->Rescale;
    switch (rescale)
    {
    case RescaleNearestPower2:
    case RescaleBiggestPower2:
    case RescaleSmallestPower2:
    case RescaleNextSmallestPower2:
        break;
    default:
        if(!pexporter->IsNonPow2Supported())
            rescale = RescaleBiggestPower2;
    }
    
    ImageData srcimgData;
    Ptr<RawImage> ptempImage;
    ptempImage = *RawImage::Create(Image_R8G8B8A8, 1, psrcimage->GetSize(), 0);
    ptempImage->GetImageData(&srcimgData);
    psrcimage->Decode(&srcimgData, GetImageConvertFunc(Image_R8G8B8A8, psrcimage->GetFormat())); 
    Ptr<RawImage> pscaledImage;
    ImageFormat outFormat = pexporter->GetSIFFomat(Image_R8G8B8A8, *psifsettings, psrcimage->GetSize());
    
    pscaledImage =  *CreateResizedImage(&srcimgData, rescale, psifsettings->RescaleFilter, psifsettings->bSquare, pexporter->GetMinSize(outFormat));
    
    ImageData scaledImageData;
    
    if (!pscaledImage)
        pscaledImage = ptempImage;

    ptempImage = *RawImage::Create(psrcimage->GetFormat(), 1, pscaledImage->GetSize(), 0);
    ptempImage->GetImageData(&scaledImageData);
    
    pscaledImage->Decode(&scaledImageData, GetImageConvertFunc(psrcimage->GetFormat(), Image_R8G8B8A8));

    
    Ptr<RawImage> psifImage = *RawImage::Create(outFormat, 1, pscaledImage->GetSize(), 0);

    ImageData srcData;
    pscaledImage->GetImageData(&srcData);
    ImagePlane srcPlane;
    srcData.GetPlane(0, &srcPlane);

    ImageData destData;
    psifImage->GetImageData(&destData);
    ImagePlane destPlane;
    destData.GetPlane(0, &destPlane);

    pexporter->CreateSIFImage(Image_R8G8B8A8, *psifsettings, outFormat, srcPlane, destPlane);

    if(!Render::SIF::FileWriter::WriteImage(pdstFile, psifImage))
    {
        String errorMsg;
        Format(errorMsg, "Error: Can't write to '{0}' file", pdstFile->GetFilePath());
        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return false;
    }

    return true;
}
