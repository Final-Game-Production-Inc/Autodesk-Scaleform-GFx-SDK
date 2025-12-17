/**************************************************************************

Filename    :   GFx_FileOpener.cpp
Content     :   
Created     :   April 15, 2008
Authors     :   

Notes       :   Redesigned to use inherited state objects for GFx 2.2.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Loader.h"
#include "Kernel/SF_SysFile.h"
#include "GFx/GFx_Log.h"

namespace Scaleform { namespace GFx {

// ***** FileOpener implementation

// Default implementation - use SysFile.
File* FileOpener::OpenFile(const char *purl, int flags, int modes)
{
    // Buffered file wrapper is faster to use because it optimizes seeks.
    return new SysFile(purl, flags, modes);
}

SInt64  FileOpener::GetFileModifyTime(const char *purl)
{
    FileStat fileStat;
    if (SysFile::GetFileStat(&fileStat, purl))
        return fileStat.ModifyTime; 
    return -1;
}


// Implementation that allows us to override the log.
File*      FileOpener::OpenFileEx(const char *pfilename, Log *plog, 
                                  int  flags, int modes)
{   
    File*  pin = OpenFile(pfilename, flags, modes);
    int  errCode = 16;
    if (pin)
        errCode = pin->GetErrorCode();

    if (!pin || errCode)
    {
        if (plog)
            plog->LogError("Loader failed to open '%s'", pfilename);
        if (pin)
        {
            //plog->LogError("Code '%d'", errCode);
            pin->Release();
        }
        return 0;
    }
    return pin;
}

}} // namespace Scaleform { namespace GFx {

