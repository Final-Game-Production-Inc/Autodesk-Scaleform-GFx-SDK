/**************************************************************************

Filename    :   FxWin32DebugPopups.h
Content     :   Suppress Debug Popups in Windows
Created     :   Apr, 2009
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxWin32DebugPopups_H
#define INC_FxWin32DebugPopups_H
#include "Kernel/SF_Types.h"

#ifdef SF_OS_WIN32
#include <windows.h>
#include <errno.h>

// Redefine operator 'new' if necessary.
#if defined(GFC_DEFINE_NEW)
#define new GFC_DEFINE_NEW
#endif

// This header has to follow "#define new".
#include "Kernel/SF_HeapNew.h"

#include <crtdbg.h>


inline LONG CALLBACK ExceptionHandler(EXCEPTION_POINTERS* ep)
{
    SF_UNUSED(ep);
    return EXCEPTION_EXECUTE_HANDLER;
}

inline void SuppressDiagPopupMessages()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
        SEM_NOOPENFILEERRORBOX);

    /* Runtime library */
    _set_error_mode(_OUT_TO_STDERR);

#ifndef   SF_BUILD_DEBUGOPT
    /* Debug library */
    _CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
#endif
    /* Exceptions */
    SetUnhandledExceptionFilter(ExceptionHandler);
}

#endif
#endif 

