/**************************************************************************

Filename    :   Amp_ObjectsLog.cpp
Content     :   Log used by AMP to generate objects report
Created     :   May 2012
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_ObjectsLog.h"

#ifdef SF_AMP_SERVER

namespace Scaleform {
namespace GFx {
namespace AMP {

void ObjectsLog::LogMessageVarg(LogMessageId messageType, const char* pfmt, va_list argList)
{
    char buffer[MaxLogBufferMessageSize];
    FormatLog(buffer, sizeof(buffer), messageType, pfmt, argList);
    Report.AppendString(buffer);
}

const char* ObjectsLog::ToCStr() const
{
    return Report.ToCStr();
}


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif