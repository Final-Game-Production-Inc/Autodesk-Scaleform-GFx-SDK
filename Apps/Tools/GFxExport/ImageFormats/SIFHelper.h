/**************************************************************************

Filename    :   SIFHelper.h
Content     :   Interface to image export libraries (DDS, PVR etc.) 
Created     :   Feb 2011
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SIFHELPER_H
#define INC_SIFHELPER_H

// GFx Includes
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_ResizeImage.h"
#include "Render/Render_Image.h"
#include "../ExportImageHelper.h"

using namespace Scaleform;
//using namespace GFx;
using namespace Render;

struct SIFSettings : public CompressionSettinns
{
   ImageTarget Target;
};


class GSIFHelper : public ExportImageHelper
{
public:
    GSIFHelper()
    {
    }
    virtual ~GSIFHelper()
    {
    }
    virtual bool CompressToFile(const Image* psrcimage, File* pdstFile, CompressionSettinns* psettings, GFxExportListener *logger);
    virtual const char*  GetLastErrorString() {return NULL;}
};

#endif //INC_SIFHELPER_H
