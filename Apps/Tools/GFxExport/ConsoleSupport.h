/**************************************************************************

Filename    :   ConsoleSupport.h
Content     :   GFxExport Console support plugin interface 
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFxExportConsolePlugin_H
#define INC_GFxExportConsolePlugin_H

#include "ImageFormats/SIFHelper.h"

using Scaleform::Render::ImageFormat;
using Scaleform::Render::ImageSize;
using Scaleform::Render::ImagePlane;

class GFxExportConsolePlugin
{
public:
    // Images
    virtual bool        IsNonPow2Supported() const = 0;
    virtual unsigned GetMinSize(ImageFormat format) const = 0;

    virtual ImageTarget GetImageTarget() const = 0;
    virtual int         GetImageFormats(unsigned bufSize, ImageFormat* formats) const = 0;

    virtual ImageFormat GetSIFFomat(ImageFormat format, const SIFSettings& settings, const ImageSize size) const = 0;
    virtual bool        CreateSIFImage(ImageFormat format, const SIFSettings& settings, ImageFormat destformat, const ImagePlane& srcPlane, ImagePlane& destPlane) const = 0;

    virtual void        ConvertToRGBA(ImageFormat srcformat, const ImagePlane& srcPlane, ImagePlane& destPlane, ImageFormat destFormat) const = 0;
    virtual bool        ConvertFile(const char* filename, const CompressionSettinns& settings) const { SF_UNUSED2(filename, settings); return false; } //use external tool to convert to platform format
};

typedef GFxExportConsolePlugin*(*GetConsoleSupportFunc)(int version);

struct ConsoleImageSupport
{
    const char*             Name;
    const char*             LongName;
    ImageTarget             ImgTarget;
    GFxExportConsolePlugin* pSupport;
};

extern ConsoleImageSupport Consoles[];

void DetectConsoles();
GFxExportConsolePlugin* FindConsole(ImageTarget target);

#endif
