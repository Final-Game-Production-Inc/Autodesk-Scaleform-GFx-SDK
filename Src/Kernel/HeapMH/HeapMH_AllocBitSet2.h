/**************************************************************************

Filename    :   HeapMH_AllocBitSet2.h
Content     :   "Magic-header based" Bit-set based allocator, 2 bits 
                per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapMH_AllocBitSet2_H
#define INC_SF_Kernel_HeapMH_AllocBitSet2_H

#include "../SF_HeapBitSet2.h"
#include "HeapMH_FreeBin.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
class AllocBitSet2MH
{
public:
    AllocBitSet2MH();

    void Reset() { Bin.Reset(); }

    void  InitPage(PageMH* page, UInt32 index);
    void  ReleasePage(UByte* start);

    void* Alloc(UPInt size, MagicHeadersInfo* headers);
    void* Alloc(UPInt size, UPInt alignSize, MagicHeadersInfo* headers);
    void* ReallocInPlace(PageMH* page, void* oldPtr, UPInt newSize, UPInt* oldSize, MagicHeadersInfo* headers);
    void  Free(PageMH* page, void* ptr, MagicHeadersInfo* headers, UPInt* oldBytes);

    UPInt GetUsableSize(const PageMH* page, const void* ptr) const;

private:
    ListBinMH Bin;
};

}} // Scaleform::HeapMH


#endif
