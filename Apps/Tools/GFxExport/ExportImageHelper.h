/**************************************************************************

Filename    :   ExportImageHelper.h
Content     :   Interface to external compression libraries 
Created     :   September, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_ExportImageHelper_H
#define INC_ExportImageHelper_H

// GFx Includes
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_ResizeImage.h"
#include "Render/Render_Image.h"

using namespace Scaleform;
//using namespace GFx;

class GFxExportListener;
struct CompressionSettinns;

class ExportImageHelper : public RefCountBase<ExportImageHelper, Stat_Default_Mem>
{
public:
    ExportImageHelper() 
    {
        pRescaleFilterLut = 0;
    }
    virtual ~ExportImageHelper()
    {
        if(pRescaleFilterLut)
            delete pRescaleFilterLut;
    }
    enum QualitySetting 
    {
        QualityFastest,
        QualityNormal,
        QualityProduction,
        QualityHighest,
    };
    enum RescaleTypes
    {
        RescaleNone,                // no rescale
        RescaleNearestPower2,       // rescale to nearest power of two
        RescaleBiggestPower2,       // rescale to next bigger power of 2
        RescaleSmallestPower2,      // rescale to smaller power of 2 
        RescaleNextSmallestPower2,  // rescale to next smaller power of 2
        RescalePreScale,            // rescale to this size
        RescaleRelScale,            // relative rescale
    };
    enum MipFilterTypes
    {
        nvFilterBox,      
        nvFilterTriangle, 
        nvFilterKaiser,
    };
    enum RescaleFilterTypes
    { 
        FilterBox,      
        FilterTriangle, 
        FilterQuadratic,
        FilterCatrom,   
        FilterMitchell, 
        FilterGaussian, 
        FilterSinc,     
        FilterBessel,   
        FilterHanning,  
        FilterHamming,  
        FilterBlackman, 
        FilterKaiser,
        FilterBicubic,
        FilterSpline16,
        FilterSpline36,
        FilterHermite,
        FilterQuadric,
        FilterLanczos,
        FilterBilinear,
    };
    Render::ImageFilterLut * pRescaleFilterLut;

    //Calculates destination width and height, made separate function for Info mode 
    static void CalculateWH(UInt32 srcW, UInt32 srcH, UInt32 &destW, UInt32 &destH, ExportImageHelper::RescaleTypes rescale);
    //

    Render::RawImage* Rescale(const Render::Image* psrcimage, ExportImageHelper::RescaleTypes rescale, ExportImageHelper::RescaleFilterTypes rescaleFilter, Render::ImageFormat destImageFormat, bool square = false);

    virtual bool CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger) = 0;
    virtual const char*  GetLastErrorString() {return NULL;}
    void CalculateResizeFilter(RescaleFilterTypes FilterType);
    Render::RawImage* CreateResizedImage(const Render::ImageData *psrc, ExportImageHelper::RescaleTypes rescale, ExportImageHelper::RescaleFilterTypes filterType,
        bool square = false, unsigned minSize = 1 /*, unsigned targetWidth, unsigned targetHeight*/);
};

struct CompressionSettinns
{
    ExportImageHelper::RescaleTypes Rescale;
    ExportImageHelper::RescaleFilterTypes RescaleFilter;
    bool IsUncompressed;
    bool DoGenMipMap;
    bool bSquare;
};

#endif //INC_ExportImageHelper_H
