/**************************************************************************

Filename    :   HeapPT_BitSet1.h
Content     :   
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Allocator bit-set maintanance

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_BitSet1_H
#define INC_SF_Kernel_HeapPT_BitSet1_H

#include <string.h>
#include "../SF_Types.h"

namespace Scaleform { namespace HeapPT {

// ***** BitSet1
//
//------------------------------------------------------------------------
class BitSet1
{
public:
    //--------------------------------------------------------------------
    static void     Clear(UInt32* buf, UPInt numWords)
    {
        memset(buf, 0, sizeof(UInt32) * numWords);
    }

    //--------------------------------------------------------------------
    static unsigned GetValue(const UInt32* buf, UPInt num)
    {
        return (buf[num >> 5] >> (num & 31)) & 1;
    }

    static unsigned SetBit(UInt32* buf, UPInt num)
    {
        return buf[num >> 5] |= UInt32(1) << (num & 31);
    }

    static unsigned ClrBit(UInt32* buf, UPInt num)
    {
        return buf[num >> 5] &= ~(UInt32(1) << (num & 31));
    }

    //--------------------------------------------------------------------
    static void SetUsed(UInt32* buf, UPInt start, UPInt num)
    {
        UPInt i;
        UPInt startWord =  start            >> 5;
        UPInt endWord   = (start + num - 1) >> 5;
        UPInt tail      = (start + num - 1) & 31;
        if (endWord > startWord)
        {
            buf[start >> 5] |= HeadUsedTable[start & 31];
            for(i = startWord + 1; i < endWord; ++i)
            {
                buf[i] = ~UInt32(0);
            }
            buf[endWord] |= TailUsedTable[tail];
        }
        else
        {
            buf[startWord] |= HeadUsedTable[start & 31] & TailUsedTable[tail];
        }
    }

    //--------------------------------------------------------------------
    static void SetFree(UInt32* buf, UPInt start, UPInt num)
    {
        UPInt i;
        UPInt startWord =  start            >> 5;
        UPInt endWord   = (start + num - 1) >> 5;
        UPInt tail      = (start + num - 1) & 31;
        if (endWord > startWord)
        {
            buf[start >> 5] &= HeadFreeTable[start & 31];
            for(i = startWord + 1; i < endWord; ++i)
            {
                buf[i] = 0;
            }
            buf[endWord] &= TailFreeTable[tail];
        }
        else
        {
            buf[startWord] &= HeadFreeTable[start & 31] | TailFreeTable[tail];
        }
    }

    //--------------------------------------------------------------------
    static UPInt FindLastUsedInWord(UInt32 bits)
    {
        if ((bits & 0xFFFF) != 0xFFFF)
        {
            return ((bits & 0xFF) == 0xFF) ?
                LastUsedBlock[(bits >> 8 ) & 0xFF] + 8:
                LastUsedBlock[ bits        & 0xFF] + 0;
        }
        return ((bits & 0xFFFFFF) == 0xFFFFFF) ?
            LastUsedBlock[(bits >> 24) & 0xFF] + 24:
            LastUsedBlock[(bits >> 16) & 0xFF] + 16;
    }

    //--------------------------------------------------------------------
    static UPInt FindLastFreeInWord(UInt32 bits)
    {
        if (bits & 0xFFFF)
        {
            return (bits & 0xFF) ?
                LastFreeBlock[ bits        & 0xFF] + 0:
                LastFreeBlock[(bits >> 8 ) & 0xFF] + 8;
        }
        return (bits & 0xFF0000) ?
            LastFreeBlock[(bits >> 16) & 0xFF] + 16:
            LastFreeBlock[(bits >> 24) & 0xFF] + 24;
    }

    //--------------------------------------------------------------------
    static UPInt FindUsedSize(const UInt32* buf, UPInt start, UPInt limit)
    {
        UPInt  startWord = start >> 5;
        UPInt  size = 0;
        UInt32 mask = HeadUsedTable[start & 31];
        UInt32 bits = buf[startWord] & mask;
        if (bits != mask)
        {
            return FindLastUsedInWord(bits >> (start & 31));
        }
        size += 32 - (start & 31);
        while ((startWord+1)*32 < limit && buf[++startWord] == ~UInt32(0))
        {
            size += 32;
        }
        return size + FindLastUsedInWord(buf[startWord]);
    }

    //--------------------------------------------------------------------
    static UPInt FindFreeSize(const UInt32* buf, UPInt start)
    {
        UPInt  startWord = start >> 5;
        UPInt  size = 0;
        UInt32 mask = HeadFreeTable[start & 31];
        UInt32 bits = buf[startWord] | mask;
        if (bits != mask)
        {
            return FindLastFreeInWord(bits >> (start & 31));
        }
        size += 32 - (start & 31);
        while (buf[++startWord] == 0)
        {
            size += 32;
        }
        return size + FindLastFreeInWord(buf[startWord]);
    }

private:
    //--------------------------------------------------------------------
    static const UInt32 HeadUsedTable[32];
    static const UInt32 TailUsedTable[32];
    static const UInt32 HeadFreeTable[32];
    static const UInt32 TailFreeTable[32];
    static const UByte  LastFreeBlock[256];
    static const UByte  LastUsedBlock[256];
};


}} // Scaleform::Heap

#endif
