/**************************************************************************

Filename    :   SF_Debug.cpp
Content     :   General purpose debugging support
Created     :   July 18, 2001
Authors     :   Brendan Iribe, Michael Antonov

Notes       :   GFC Debug warning functionality is enabled
                if and only if SF_BUILD_DEBUG is defined.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Debug.h"

namespace Scaleform {

void LogDebugMessage(LogMessageId id, const char* fmt, ...)
{
    Log* log = Log::GetGlobalLog();
    va_list argList;
    va_start(argList, fmt);
    if (log)
        log->LogMessageVarg(id, fmt, argList);
    else
        Log::DefaultLogMessageVarg(id, fmt, argList);
    va_end(argList);
}
    

} // Scaleform
