/**************************************************************************

Filename    :   HeapPT_SysAllocWii.cpp
Platform    :   Wii
Content     :   Wii System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_SysAllocWii.h"
#include "../SF_Debug.h"

#include <revolution.h>
#include <revolution/os.h>
#include <new>

namespace Scaleform {

//------------------------------------------------------------------------
SysAllocWii::SysAllocWii(UPInt size)
{
    UByte *m2arenaLo = (UByte*) OSGetMEM2ArenaLo();
    UByte *m2arenaHi = (UByte*) OSGetMEM2ArenaHi();
    if (size > 0 && (m2arenaLo + size < m2arenaHi))
        m2arenaHi = m2arenaLo + size;
    OSSetMEM2ArenaLo(m2arenaHi);
    SF_DEBUG_WARNING(m2arenaHi-m2arenaLo < 65536, "Less than 64k memory available");

    ::new(PrivateData) SysAllocStatic(m2arenaLo, (UByte*)m2arenaHi-(UByte*)m2arenaLo);
}

void SysAllocWii::GetInfo(Info* i) const
{
    const_cast<SysAllocWii*>(this)->GetAllocator()->GetInfo(i);
}

void* SysAllocWii::Alloc(UPInt size, UPInt alignment)
{
    return GetAllocator()->Alloc(size, alignment);
}

bool SysAllocWii::ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt alignment)
{
    return GetAllocator()->ReallocInPlace(oldPtr, oldSize, newSize, alignment);
}

bool SysAllocWii::Free(void* ptr, UPInt size, UPInt alignment)
{
    return GetAllocator()->Free(ptr, size, alignment);
}


} // Scaleform
