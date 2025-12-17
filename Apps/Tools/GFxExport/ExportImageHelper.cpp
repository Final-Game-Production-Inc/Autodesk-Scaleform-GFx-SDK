/**************************************************************************

Filename    :   ExportImageHelper.cpp
Content     :   Interface to external compression libraries 
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#define NOMINMAX


//#include <nvtt/nvtt.h>
//#include "PVRTexLib.h"


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

#include "ExportImageHelper.h"

using namespace Scaleform;
using namespace GFx;
using namespace Render;


void ExportImageHelper::CalculateResizeFilter(RescaleFilterTypes FilterType)
{
    bool norm = true;
    if (pRescaleFilterLut)
    {
        SF_ASSERT(0);
    }
    pRescaleFilterLut= new ImageFilterLut;
    switch(FilterType)
    {
    case FilterBicubic : pRescaleFilterLut->Calculate(ImageFilterBicubic (), norm); break;
    case FilterSpline16: pRescaleFilterLut->Calculate(ImageFilterSpline16(), norm); break;
    case FilterSpline36: pRescaleFilterLut->Calculate(ImageFilterSpline36(), norm); break;
    case FilterHanning : pRescaleFilterLut->Calculate(ImageFilterHanning (), norm); break;
    case FilterHamming : pRescaleFilterLut->Calculate(ImageFilterHamming (), norm); break;
    case FilterHermite : pRescaleFilterLut->Calculate(ImageFilterHermite (), norm); break;
    case FilterKaiser  : pRescaleFilterLut->Calculate(ImageFilterKaiser  (), norm); break;
    case FilterQuadric : pRescaleFilterLut->Calculate(ImageFilterQuadric (), norm); break;
    case FilterCatrom  : pRescaleFilterLut->Calculate(ImageFilterCatrom  (), norm); break;
    case FilterGaussian: pRescaleFilterLut->Calculate(ImageFilterGaussian(), norm); break;
    case FilterBessel  : pRescaleFilterLut->Calculate(ImageFilterBessel  (), norm); break;
    case FilterMitchell: pRescaleFilterLut->Calculate(ImageFilterMitchell(), norm); break;
    case FilterSinc    : pRescaleFilterLut->Calculate(ImageFilterSinc    (), norm); break;
    case FilterLanczos : pRescaleFilterLut->Calculate(ImageFilterLanczos (), norm); break;
    case FilterBlackman: pRescaleFilterLut->Calculate(ImageFilterBlackman(), norm); break;
    case FilterBox:
    case FilterBilinear:
        break;
    default:
        SF_ASSERT(0);

    }
}

void ExportImageHelper::CalculateWH( UInt32 srcW, UInt32 srcH, UInt32 &destW, UInt32 &destH, ExportImageHelper::RescaleTypes rescale )
{
    //calculation of the next higher or lower power or 2 can be implemented without using log, if necessary
    double wp=log(double(srcW))/log((double)2.0), hp=log(double(srcH))/log((double)2.0);
    switch (rescale)
    {
    case ExportImageHelper::RescaleNearestPower2 :
        destW=1<<(int)floor(wp+0.5); destH=1<<(int)floor(hp+0.5);
        break;

    case ExportImageHelper::RescaleBiggestPower2 :
        destW=1<<(int)ceil(wp); destH=1<<(int)ceil(hp);
        break;

    case ExportImageHelper::RescaleSmallestPower2 :
        destW=1<<(int)floor(wp); destH=1<<(int)floor(hp);
        break;
    case ExportImageHelper::RescaleNextSmallestPower2 :
        if (wp > 1)
            destW = 1<<(int)floor(wp-1);
        else
            destW = 1;
        if (hp > 1)
            destH = 1<<(int)floor(hp-1);
        else
            destH = 1;
        break;

    case ExportImageHelper::RescalePreScale :
        destW=int((srcW + 3)&(~3)); destH=int((srcH + 3)&(~3));
        break;

    case ExportImageHelper::RescaleNone :
        destW=srcW; destH=srcH;
        break;
    default:
        SF_ASSERT(0);
    }
}

Render::RawImage* ExportImageHelper::CreateResizedImage( const Render::ImageData *psrc, ExportImageHelper::RescaleTypes rescale, ExportImageHelper::RescaleFilterTypes filterType, bool square, unsigned minSize /*, unsigned targetWidth, unsigned targetHeight*/ )
{

	if (rescale == ExportImageHelper::RescaleNone)
		return 0;
    SF_ASSERT(psrc->GetFormat() == Image_R8G8B8A8);
    UInt32 w = 0, h = 0; //destination width and height  

    CalculateWH( psrc->GetWidth(), psrc->GetHeight(), w, h, rescale );
    w = Alg::Max<unsigned>(w, minSize);
    h = Alg::Max<unsigned>(h, minSize);
    if (square)
        w = h = Alg::Max<unsigned>(w, h);

    

	if (w == psrc->GetWidth() && h == psrc->GetHeight())
		return 0;
	
	RawImage* pdstImage = RawImage::Create(Image_R8G8B8A8, psrc->GetMipLevelCount(), ImageSize(w,h), 0);
	ImageData dstImageData; 
	pdstImage->GetImageData(&dstImageData);
	
    switch(filterType)
    {
    case ExportImageHelper::FilterBox:      
        ResizeImageBox(dstImageData.GetDataPtr(), (int)w, (int)h, (int)dstImageData.GetPitch(), 
                        psrc->GetDataPtr(), (int)psrc->GetWidth(), (int) psrc->GetHeight(), (int)psrc->GetPitch(), ResizeRgbaToRgba); 
        break;
    case ExportImageHelper::FilterBilinear: 
        ResizeImageBilinear(dstImageData.GetDataPtr(), (int)w, (int)h, (int)dstImageData.GetPitch(), 
                        psrc->GetDataPtr(), (int)psrc->GetWidth(), (int) psrc->GetHeight(), (int)psrc->GetPitch(), ResizeRgbaToRgba); 
        break;
    default:       
        ResizeImage(dstImageData.GetDataPtr(), (int)w, (int)h, (int)dstImageData.GetPitch(), 
                        psrc->GetDataPtr(), (int)psrc->GetWidth(), (int) psrc->GetHeight(), (int)psrc->GetPitch(), ResizeRgbaToRgba,
                     *pRescaleFilterLut); 
        break;
    }
	return pdstImage;
}

Render::RawImage* ExportImageHelper::Rescale( const Render::Image* psrcimage, ExportImageHelper::RescaleTypes rescale, ExportImageHelper::RescaleFilterTypes rescaleFilter, Render::ImageFormat destImageFormat, bool square /*= false*/ )
{
    SF_UNUSED(destImageFormat);
    //Ptr<RawImage> pimage = *new RawImage(*psrcimage);
    //Ptr<RawImage> pargbImage = *pimage->ConvertImage(Image_R8G8B8A8);
    ImageData srcimgData;
    RawImage* ptempImage;
    ptempImage = RawImage::Create(Image_R8G8B8A8, 1, psrcimage->GetSize(), 0);
    ptempImage->GetImageData(&srcimgData);
    psrcimage->Decode(&srcimgData, GetImageConvertFunc(Image_R8G8B8A8, psrcimage->GetFormat()));
    RawImage* prescaledImage = CreateResizedImage(&srcimgData, rescale, rescaleFilter, square);
    if (prescaledImage)
    {
        ptempImage->Release();
        return prescaledImage;
    }
    else
    {
        return ptempImage;
    }
}

//
//bool GSIFHelper::CompressToFile( const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings )
//{
//    SF_UNUSED3(psrcimage, pdstFile, psettings);
//    return true;
//}
