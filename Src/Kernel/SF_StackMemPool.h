/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_StackMemPool.h
Content     :   Policy-based memory pool, which tries to allocate memory
                on stack first.
Created     :   January 22, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_StackMemPool_H
#define INC_SF_Kernel_StackMemPool_H

#include "SF_Memory.h"
#include "SF_Array.h"

namespace Scaleform {

////////////////////////////////////////////////////////////////////////////////
// StackMemPool Alloc/Free policies.

// Regular Alloc/Free logic.
struct MemPoolImmediateFree
{
    MemPoolImmediateFree(MemoryHeap* pheap) : pHeap(pheap) {}

    void* Alloc(UPInt nbytes, UPInt align)
    {
        return pHeap ? Memory::AllocInHeap(pHeap, nbytes, align) : Memory::Alloc(nbytes, align);
    }
    void Free(void* p)
    {
        Memory::Free(p);
    }

private:
    MemoryHeap*    pHeap;
};

// Postpone freeing of memory till the very end.
struct MemPoolPostponeFree
{
    MemPoolPostponeFree(MemoryHeap* pheap) : pHeap(pheap) {}
    ~MemPoolPostponeFree()
    {
        for (UPInt i = 0; i < AllocatedMem.GetSize(); ++i)
        {
            Memory::Free(AllocatedMem[i]);
        }
    }

    void* Alloc(UPInt nbytes, UPInt align)
    {
        void* mem = pHeap ? Memory::AllocInHeap(pHeap, nbytes, align) : Memory::Alloc(nbytes, align);

        if (mem)
        {
            AllocatedMem.PushBack(mem);
        }

        return mem;
    }
    static void Free(void* /*p*/)
    {
        ; // Do nothing.
    }

private:
    MemoryHeap*    pHeap;
    Array<void*>   AllocatedMem;
};

////////////////////////////////////////////////////////////////////////////////
// Memory pool, which tries to allocate memory on stack.
// It can be used as a more flexible version of a stack memory buffer, or as 
// a faster memory allocator for in-place constructors.
template <UPInt N = 512, UPInt A = sizeof(void*), class AFP = MemPoolImmediateFree>
class StackMemPool : AFP
{
public:
    StackMemPool(MemoryHeap* pheap = NULL)
    : AFP(pheap), BuffPtr(AlignMem(Buff)), BuffSize(buff_max_size - (BuffPtr - Buff))
    {
    }

public:
    void* Alloc(UPInt nbytes)
    {
        void* tmpBuffPtr = NULL;

        if (nbytes <= BuffSize)
        {
            // Allocate in Buffer.
            tmpBuffPtr = BuffPtr;

            BuffPtr = AlignMem(BuffPtr + nbytes);
            const UPInt curSize = BuffPtr - Buff;
            BuffSize = buff_max_size > curSize ? buff_max_size - curSize : 0;
        } else
        {
            // Allocate in heap.
            tmpBuffPtr = AFP::Alloc(nbytes, align);
        }

        return tmpBuffPtr;
    }
    void Free(void* p)
    {
        if (!(p >= Buff && p < Buff + buff_max_size))
        {
            // Pointer is not inside of the Buffer.
            AFP::Free(p);
        }
    }

private:
    // Return new aligned offset. Align must be power of two.
    static
    char* AlignMem(char* offset)
    {
        return reinterpret_cast<char*>(A + ((reinterpret_cast<UPInt>(offset) - 1) & ~(A - 1)));
    }

private:
    // align must be power of two.
    enum { buff_max_size = N, align = A };

    char  Buff[buff_max_size];
    char* BuffPtr;
    UPInt BuffSize;
};

} // Scaleform

#undef new

template <Scaleform::UPInt N, Scaleform::UPInt A>
inline
void* operator new (Scaleform::UPInt nbytes, Scaleform::StackMemPool<N, A>& pool)
{
    return pool.Alloc(nbytes);
}

template <Scaleform::UPInt N, Scaleform::UPInt A>
inline
void operator delete (void* p, Scaleform::StackMemPool<N, A>& pool)
{
    pool.Free(p);
}

// Redefine operator 'new' if necessary.
#if defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

// This header has to follow "#define new".
#include "SF_HeapNew.h"

#endif // INC_GMEMSTACKPOOL_H
