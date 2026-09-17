/**************************************************************************

Filename    :   Amp_ObjectsLog.h
Content     :   Log used by AMP to generate objects report
Created     :   May 2012
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INCLUDE_GFX_AMP_OBJECTS_LOG_H
#define INCLUDE_GFX_AMP_OBJECTS_LOG_H

#include "GFxConfig.h"
#ifdef SF_AMP_SERVER

#include "Kernel/SF_Log.h"
#include "Kernel/SF_String.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

class ObjectsLog : public Log
{
public:
    virtual void LogMessageVarg(LogMessageId messageType, const char* pfmt, va_list argList);
    const char* ToCStr() const;
private:
    StringBuffer Report;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif
