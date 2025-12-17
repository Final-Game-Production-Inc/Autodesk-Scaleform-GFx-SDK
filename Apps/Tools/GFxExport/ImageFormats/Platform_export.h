/**************************************************************************

Filename    :   Platform_export.h
Content     :   Native format export 
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.
**************************************************************************/

#ifndef INC_Platform_export_H
#define INC_Platform_export_H


#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "GFx/GFx_Stream.h"
#include "SIFHelper.h"

using Scaleform::Render::Image;
using Scaleform::Render::ImageTarget;
using Scaleform::Render::ImageData;



class SIFExporter : public RefCountBase<SIFExporter, Stat_Default_Mem>
{
public:
    static  SIFExporter* CreateExporter(ImageTarget format);
    virtual Image* CreateSIFImage(const SIFSettings& settings, const Image& image) = 0;
    virtual bool   IsNonPow2Supported() = 0;
    virtual unsigned   GetMinSize() {return 1;}
};

#endif //INC_Platform_export_H
