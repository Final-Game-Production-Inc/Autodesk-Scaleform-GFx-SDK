/**************************************************************************

PublicHeader:   Kernel
Filename    :   HeapPT_SysAllocWii.h
Platform    :   Wii
Content     :   Wii System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_SysAllocWii_H
#define INC_SF_Kernel_HeapPT_SysAllocWii_H

#include "../SF_SysAlloc.h"
#ifdef SF_OS_WII

namespace Scaleform {

class SysAllocWii : public SysAllocBase_SingletonSupport<SysAllocWii, SysAllocPaged>
{
public:
    // amount of memory to reserve in MEM2 arena; 0 is all available space
    SysAllocWii(UPInt size = 0);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

private:
    UPInt PrivateData[(sizeof(SysAllocStatic) + sizeof(UPInt)) / sizeof(UPInt)];
    SysAllocStatic* GetAllocator() { return (SysAllocStatic*) PrivateData; }
};


} // Scaleform

#endif      // SF_OS_WII
#endif
