/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_AllocInfo.h
Content     :   Allocator Debugging Support
Created     :   October 1, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_AllocInfo_H
#define INC_SF_Kernel_AllocInfo_H

#include "SF_Stats.h"

namespace Scaleform {


// ***** Allocator Debugging Support
//
// Debugging support is optionally stored with every allocation.
// This consists of the following pieces of info:
//  - StatId, used to identify and group allocation for statistic reporting.
//  - Line + FileName, used to report the memory leaks when heap
//    is released as a whole.
//------------------------------------------------------------------------
struct AllocInfo
{
    // User-specified identifier for allocation.
    unsigned    StatId;

#if defined(SF_BUILD_DEBUG)
    // Track location in code where allocation took place
    // so that it can be reported to debug leaks.
    unsigned    Line;
    const char* pFileName;

    AllocInfo()
        : StatId(Stat_Default_Mem), Line(0), pFileName("")
    { }
    AllocInfo(unsigned statId, const char* pfile, unsigned line)
        : StatId(statId), Line(line), pFileName(pfile)
    { }
    AllocInfo(const AllocInfo& src)
        : StatId(src.StatId), Line(src.Line), pFileName(src.pFileName)
    { }

#else

    AllocInfo() : StatId(Stat_Default_Mem)
    { }
    AllocInfo(unsigned statId, const char*, unsigned) : StatId(statId)
    { }
    AllocInfo(const AllocInfo& src) : StatId(src.StatId)
    { }

#endif

    // Note that we don't store the original Size here, since it is 
    // passed as a separate argument and we want to avoid such overhead.
};

} // Scaleform

#endif
