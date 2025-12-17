/**************************************************************************

Filename    :   DX11_export.cpp
Content     :   DX11 Texture native format export implementation
Created     :   
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.
**************************************************************************/

#include "Render/Render_Image.h"
#include "Platform_export.h"
#include "../ConsoleSupport.h"
#include <windows.h>

using namespace Scaleform;
using namespace Render;

class GFxExport_DX11 : public GFxExportConsolePlugin
{
public:
    // Images
    virtual bool        IsNonPow2Supported() const { return true; }
    virtual unsigned    GetMinSize(ImageFormat format) const { SF_UNUSED(format); return 1; }

    virtual ImageTarget GetImageTarget() const { return ImageTarget_DX11; }
    virtual int         GetImageFormats(unsigned bufSize, ImageFormat* formats) const
    {
        SF_ASSERT(0);
        fprintf(stderr, "Not supported on this platform\n");
        return 0;
    }

    virtual ImageFormat GetSIFFomat(ImageFormat format, const SIFSettings& settings, const ImageSize size) const
    {
        SF_ASSERT(0);
        fprintf(stderr, "Not supported on this platform\n");
        return Image_None;
    }

    virtual bool  CreateSIFImage(ImageFormat format, const SIFSettings& settings, ImageFormat destformat, const ImagePlane& srcPlane, ImagePlane& destPlane) const
    {
        SF_ASSERT(0);
        fprintf(stderr, "Not supported on this platform\n");
        return false;
    }

    virtual void        ConvertToRGBA(ImageFormat srcformat, const ImagePlane& srcPlane, ImagePlane& destPlane, ImageFormat destFormat) const
    {
        
    }

    virtual bool ConvertFile(const char* filename, const CompressionSettinns& settings) const;


};

bool GFxExport_DX11::ConvertFile( const char* filename, const CompressionSettinns& settings ) const
{
    SF_UNUSED(settings);

    char command[2048];
    sprintf_s(command, 2048, "texconv.exe %s.dds -f BC7_UNORM", filename);

    int code = system(command);
    if (code)
    {
        fprintf(stderr, "BC7 compression failed\n");
        return false;
    }
    return true;
}

static GFxExport_DX11 instance;

extern "C" __declspec(dllexport) GFxExportConsolePlugin* GetConsoleSupport(int version)
{
    return &instance;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
