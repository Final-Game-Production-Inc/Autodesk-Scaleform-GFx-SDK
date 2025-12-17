/**************************************************************************

Filename    :   PVRHelper.h
Content     :   Interface to image export libraries (DDS, PVR etc.) 
Created     :   Feb 2011
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_PVRHELPER_H
#define INC_PVRHELPER_H

// GFx Includes
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_ResizeImage.h"
#include "../ExportImageHelper.h"

using namespace Scaleform;
using namespace GFx;

class GPVRHelper : public ExportImageHelper
{
public:
    GPVRHelper()
    {
    }
    virtual ~GPVRHelper()
    {
    }
    virtual bool CompressToFile(const Render::Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger);
    virtual const char*  GetLastErrorString() {return NULL;}
};

struct PVRSettings : public CompressionSettinns
{
    enum FormatTypes
    {
        FormatPVRTC,
        FormatETC
    };
    int PVRn;
    FormatTypes Format;
};



#endif //INC_PVRHELPER_H
