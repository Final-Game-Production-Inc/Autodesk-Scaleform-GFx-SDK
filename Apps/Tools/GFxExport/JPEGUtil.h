/**************************************************************************

PublicHeader:   Render
Filename    :   JPEGUtil.h
Content     :   JPEG image I/O utility classes
Created     :   June 24, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_JPEGUtil_H
#define INC_SF_Render_JPEGUtil_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_RefCount.h"

namespace Scaleform { 

class File;

// ***** Declared Classes
class JPEGInput;
class JPEGOutput;

class JPEGSystem : public RefCountBase<JPEGSystem, Stat_Default_Mem>
{
public:
    static JPEGSystem* SF_STDCALL CreateDefaultSystem();

    virtual ~JPEGSystem();
    virtual JPEGInput* CreateInput(File* pin) = 0;
    virtual JPEGInput* CreateSwfJpeg2HeaderOnly(File* pin) = 0;
    virtual JPEGInput* CreateSwfJpeg2HeaderOnly(const UByte* pbuffer, UPInt bufSize) = 0;

    virtual JPEGOutput* CreateOutput(File* pout, int width, int height, int quality) = 0;
    virtual JPEGOutput* CreateOutput(File* pout) = 0;
};

class JPEGInput : public NewOverrideBase<Stat_Default_Mem>
{
public:

    virtual ~JPEGInput();

    virtual void     DiscardPartialBuffer()  = 0;
    virtual int      StartImage()            = 0;
    virtual int      StartRawImage()         = 0;
    virtual int      FinishImage()           = 0;

    virtual unsigned GetHeight() const       = 0;
    virtual unsigned GetWidth() const        = 0;
    virtual int      ReadScanline(unsigned char* prgbData) = 0;
    virtual int      ReadRawData(void** pprawData) = 0;

    virtual void*    GetCInfo()              = 0; // returns jpeg_decompress_struct*

    virtual bool     IsErrorOccurred() const = 0;
};


// Helper object for writing jpeg image data.
class JPEGOutput : public NewOverrideBase<Stat_Default_Mem>
{
public:

    virtual ~JPEGOutput();

    // ...
    virtual void    WriteScanline(unsigned char* prgbData)      = 0;
    virtual void    WriteRawData(const void* prawData)          = 0;
    virtual void    CopyCriticalParams(void* pSrcDecompressInfo)= 0;
    virtual void*   GetCInfo()           = 0;  // returns jpeg_compress_struct*
};

}

#endif // INC_SF_Render_JPEGUtil_H
