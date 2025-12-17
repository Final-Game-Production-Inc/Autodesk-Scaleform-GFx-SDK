/**************************************************************************

Filename    :   ExportImageHelper.h
Content     :   Interface to image export libraries (DDS, PVR etc.) 
Created     :   Feb 2011
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GDXTHELPER_H
#define INC_GDXTHELPER_H

// GFx Includes
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_ResizeImage.h"
#include "../ExportImageHelper.h"

using namespace Scaleform;
using namespace GFx;

class GDXTHelper : public ExportImageHelper
{
public:
    GDXTHelper()
    {
    }
    virtual ~GDXTHelper()
    {
    }

    static int LastError;
    //
    static UInt32 CalculateDDSSize( UInt32 srcW, UInt32 srcH, Render::ImageFormat format, 								int dxtN,
        bool IsUncompressed,
        bool doGenMipMap,
        bool isDxt1Allowed);
    
    virtual bool CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger);    
    virtual const char*  GetLastErrorString();
};

struct DXTSettings : public CompressionSettinns
{
    GDXTHelper::QualitySetting Quality;
    GDXTHelper::MipFilterTypes MipFilter;
    int DxtN; 
    bool IsDxt1Allowed;
};


#endif //INC_GDXTHELPER_H
