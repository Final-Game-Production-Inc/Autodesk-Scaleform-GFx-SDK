/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_HeapNew.h
Content     :   Defines heap allocation macros such as SF_HEAP_NEW, etc.
Created     :   September 9, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Notes       :   This file should be included in internal headers and
                source files but NOT the public headers because it
                #undefines operator new. It is separated from Memory
                for this reason.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Memory.h"

// Operator 'new' must NOT be defined, or the macros below will not compile
// correctly (compiler would complain about the number of macro arguments).
#undef new

#ifndef INC_SF_Kernel_HeapNew_H
#define INC_SF_Kernel_HeapNew_H


// Define heap macros
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

#define SF_HEAP_NEW(heap)        new(heap,__FILE__,__LINE__)
#define SF_HEAP_NEW_ID(heap,id)  new(heap, id, __FILE__,__LINE__)
#define SF_HEAP_AUTO_NEW(addr)   new((Scaleform::MemAddressPtr)((void*)addr),__FILE__,__LINE__)
// Global new
#define SF_NEW                    new(__FILE__,__LINE__)

#else

#define SF_HEAP_NEW(pheap)        new(pheap)
#define SF_HEAP_NEW_ID(pheap,id)  new(pheap,id)
#define SF_HEAP_AUTO_NEW(paddr)   new((Scaleform::MemAddressPtr)((void*)paddr))
#define SF_NEW                    new

#endif 


#endif // INC_SF_HEAPNEW_H
