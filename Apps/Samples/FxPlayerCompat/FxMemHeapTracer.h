/**************************************************************************

Filename    :   FxMemHeapTracer.h
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxHeapTracer_H
#define INC_FxHeapTracer_H

#include <stdio.h>
#include <string.h>
#include "GMemory.h"

class FxMemHeapTracer : public GMemoryHeap::HeapTracer
{
public:
    enum
    {
        CreateHeap    = 1,
        DestroyHeap   = 2,
        Alloc         = 3,
        AllocAlign    = 4,
        AllocSID      = Alloc      | 0x40,
        AllocAlignSID = AllocAlign | 0x40,
        Realloc       = 5,
        Free          = 6
    };

    FxMemHeapTracer(const char* fileName) : 
        File(fopen(fileName, "wb"))
    {
        UByte sizeofPtr;
        
        sizeofPtr = sizeof(void*);
        fwrite(&sizeofPtr, 1, 1, File);

        UPInt t1 = 1;
        UByte littleEndian = (*(const char*)&t1) != 0;
        fwrite(&littleEndian, 1, 1, File);

        UByte sizeofDesc = sizeof(GMemoryHeap::HeapDesc);
        fwrite(&sizeofDesc, 1, 1, File);

        GMemoryHeap::SetTracer(this);
    }

    ~FxMemHeapTracer()
    {
        GMemoryHeap::SetTracer(0);
        fclose(File);
    }

    virtual void OnCreateHeap(const GMemoryHeap* heap)
    {
        GMemoryHeap::HeapInfo info;
        heap->GetHeapInfo(&info);

        UByte t = CreateHeap;
        fwrite(&t, 1, 1, File);
        fwrite(&heap, sizeof(void*), 1, File);
        fwrite(&info.Desc, sizeof(GMemoryHeap::HeapDesc), 1, File);
        UPInt nameLen = strlen(heap->GetName()) + 1;
        fwrite(&nameLen, sizeof(UPInt), 1, File);
        fwrite(heap->GetName(), 1, nameLen, File);
    }

    virtual void OnDestroyHeap(const GMemoryHeap* heap)
    {
        UByte t = DestroyHeap;
        fwrite(&t, 1, 1, File);
        fwrite(&heap, sizeof(void*), 1, File);
    }

    virtual void OnAlloc(const GMemoryHeap* heap, UPInt size, UPInt align, UInt sid, const void* ptr)
    {
        char buf[1 + 5*sizeof(void*)];
        char* p = buf;
        *p++ = char(align ? (sid ? AllocAlignSID : AllocAlign) : (sid ? AllocSID : Alloc));
        memcpy(p, &heap,  sizeof(void*)); p += sizeof(void*);
        memcpy(p, &size,  sizeof(void*)); p += sizeof(void*);
        if (align)
        {
            memcpy(p, &align, sizeof(void*)); 
            p += sizeof(void*);
        }
        if (sid)
        {
            UPInt s2 = sid;
            memcpy(p, &s2, sizeof(void*)); 
            p += sizeof(void*);
        }
        memcpy(p, &ptr,   sizeof(void*)); p += sizeof(void*);
        fwrite(buf, 1, p - buf, File);
    }

    virtual void OnRealloc(const GMemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr)
    {
        char buf[1 + 4*sizeof(void*)];
        buf[0] = Realloc;
        memcpy(buf + 1 + 0*sizeof(void*), &heap,    sizeof(void*));
        memcpy(buf + 1 + 1*sizeof(void*), &oldPtr,  sizeof(void*));
        memcpy(buf + 1 + 2*sizeof(void*), &newSize, sizeof(void*));
        memcpy(buf + 1 + 3*sizeof(void*), &newPtr,  sizeof(void*));
        fwrite(buf, 1, sizeof(buf), File);
    }

    virtual void OnFree(const GMemoryHeap* heap, const void* ptr)
    {
        char buf[1 + 2*sizeof(void*)];
        buf[0] = Free;
        memcpy(buf + 1 + 0*sizeof(void*), &heap, sizeof(void*));
        memcpy(buf + 1 + 1*sizeof(void*), &ptr,  sizeof(void*));
        fwrite(buf, 1, sizeof(buf), File);
    }

    FILE* File;
};


#endif
