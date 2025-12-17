/**************************************************************************

PublicHeader:   Kernel
Filename    :   HeapPT_SysAllocMalloc.h
Content     :   Malloc System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   System Allocator that uses regular malloc/free

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_SysAllocMalloc_H
#define INC_SF_Kernel_SysAllocMalloc_H

#include "../SF_SysAlloc.h"

namespace Scaleform {

// ***** SysAllocPagedMalloc
//
//------------------------------------------------------------------------
class SysAllocPagedMalloc : public SysAllocBase_SingletonSupport<SysAllocPagedMalloc, SysAllocPaged>
{
public:
    enum { MinGranularity = 64*1024 };

    SF_EXPORT SysAllocPagedMalloc(UPInt granularity = MinGranularity);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual UPInt   GetFootprint() const { return Footprint; }
    virtual UPInt   GetUsedSpace() const { return Footprint; }

    virtual UPInt   GetBase() const; // DBG

private:
    UPInt   Granularity;
    UPInt   Footprint;
    UPInt   Base; // DBG
};

} // Scaleform

#endif
