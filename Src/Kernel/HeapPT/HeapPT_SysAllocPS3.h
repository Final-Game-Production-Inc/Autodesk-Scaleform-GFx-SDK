/**************************************************************************

PublicHeader:   Kernel
Filename    :   HeapPT_SysAllocPS3.h
Platform    :   PS3
Content     :   PS3 system allocator
Created     :   2009
Authors     :   Vlad Merker, Maxim Shemanarev

Notes       :   PS3 system allocator that uses sys_memory_* functions

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_SysAllocPS3_H
#define INC_SF_Kernel_HeapPT_SysAllocPS3_H

#include "../SF_SysAlloc.h"
#ifdef SF_OS_PS3

namespace Scaleform {

//------------------------------------------------------------------------
class SysAllocPS3: public SysAllocBase_SingletonSupport<SysAllocPS3, SysAllocPaged>
{
public:
    enum {
        MinPageSize = 0x10000U,  // 64K
        MaxPageSize = 0x100000U  // 1M
    };

    SysAllocPS3(UPInt granularity = MinPageSize);

    virtual void*   Alloc(UPInt size, UPInt);
    virtual bool    Free(void* ptr, UPInt size, UPInt);
    virtual void    GetInfo(Info* i) const;

private:
    UPInt   Granularity;
    UPInt   SysAlignment;
    UPInt   Footprint;
};

} // Scaleform

#endif      // SF_OS_PS3
#endif
