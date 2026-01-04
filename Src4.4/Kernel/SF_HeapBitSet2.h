/**************************************************************************

Filename    :   SF_HeapBitSet2.h
Content     :   
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Allocator bit-set maintanance

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Heap_BitSet2_H
#define INC_SF_Kernel_Heap_BitSet2_H

#include <string.h>
#include "SF_Types.h"

namespace Scaleform { namespace Heap {

// ***** BitSet2
//
// This class represents a number of static functions for manipulating
// with allocation bit-sets. It uses two bits per block, that is, for the 
// block size = 16 bytes, it will take 1K per every 64K of the payload. 
// In average the bookkeeping information is less than the losses for minimal 
// alignment itself. The idea is simple and straightforward. We have two 
// bits per block that mean the following:
//
// Variant without alignment shift:
//------------------
// 00             - empty,
// 01             - 1 block
// 10 01          - 2 blocks
// 11 00 01       - 3 blocks
// 11 01 xx 01    - 4 blocks
// 11 10 xx xx 01 - 5 blocks
//
// 11 11 0n nn nn . . . 01 - 6 to 37 blocks. "n nn nn" - is a 5-bit number
//                           of blocks minus 6. 0 means 6 blocks.
//
// 11 11 11 . . . . . . 01 - 38 or more blocks. Since we have a at least 
//                           68 bits (2*38-6-2), it's guaranteed we can place 
//                           a 32-bit number directly. And of course, 
//                           it's aligned to 32-bit.
//
// Variant with alignment shift:
//------------------
// 00             - empty,
// 01             - 1 block
// 10 aa          - 2 blocks, aa - alignment, 01 or 10
// 11 00 aa       - 3 blocks, aa - alignment, 01 or 10
// 11 01 xx aa    - 4 blocks, aa - alignment, 01, 10 or 11
// 11 10 xx xx aa - 5 blocks, aa - alignment, 01, 10 or 11
//
// 11 11 0n nn nn aa       - 6 blocks. "n nn nn" - is a 5-bit number
//                           of blocks minus 6. "0 00 00" means 6 blocks.
//                           aa - alignment: 01, 10 or 11.
//
// 11 11 0n nn nn xx aa    - 7 blocks. "n nn nn" equals "0 00 01" in this case.
//                           aa - alignment: 01, 10 or 11
//
// 11 11 0n nn nn aa aa a1 - 8 to 37 blocks. "aa aa a" - is a 5-bit alignment 
//                           shift value, that is, 0:1, 1:2, 2:4, 3:8, etc.
//
// 11 11 11 . . . aa aa a1 - 38 or more blocks. Since we have a at least 
//                           64 bits (2*38-6-6), it's guaranteed we can place 
//                           a 32-bit number directly. And of course, 
//                           it's aligned to 32-bit.
//------------------------------------------------------------------------
class BitSet2
{
public:
    // Calculate the number of 32-bit words needed for the 
    // bit-set with the particular data alignment. For example,
    // 16-byte blocks must have alignmentShift=4.
    //--------------------------------------------------------------------
    static UPInt GetBitSetSize(UPInt dataSize, UPInt alignmentShift)
    {
        UPInt alignmentMask = (UPInt(1) << alignmentShift) - 1;
        return (((dataSize + alignmentMask) >> alignmentShift) + 15) / 16;
    }

    static void Clear(UInt32* buf, UPInt numWords)
    {
        memset(buf, 0, sizeof(UInt32) * numWords);
    }

    // Get the two-bit value at the particular idx.
    //--------------------------------------------------------------------
    SF_INLINE static unsigned GetValue(const UInt32* buf, UPInt idx)
    {
        return (buf[idx >> 4] >> (2*idx & 30)) & 3;
    }

    // Clear the two-bit value at the particular idx.
    //--------------------------------------------------------------------
    SF_INLINE static void ClearValue(UInt32* buf, UPInt idx)
    {
        buf[idx >> 4] &= ~(UInt32(3) << (2*idx & 30));
    }

    // Set the two-bit value at the particular idx.
    //--------------------------------------------------------------------
    SF_INLINE static void SetValue(UInt32* buf, UPInt idx, unsigned val)
    {
        UPInt wrd = idx >> 4;
        UPInt rem = 2*idx & 30;
        buf[wrd]  = (buf[wrd] & ~(UInt32(3) << rem)) | (UInt32(val) << rem);
    }

    // Mark the array of blocks as empty. We need to mark the first
    // and the last values as 00. It's necessary for proper merging.
    //--------------------------------------------------------------------
    SF_INLINE static void MarkFree(UInt32* buf, UPInt start, UPInt num)
    {
        ClearValue(buf, start);
        ClearValue(buf, start+num-1);
    }

    // Mark the array of blocks as busy without alignment info. 
    // The first and the last values must be non-zero for proper merging. 
    // The other bits are set according to the described above encoding method.
    //--------------------------------------------------------------------
    static void MarkBusy(UInt32* buf, UPInt start, UPInt num)
    {
        switch(num)
        {
        case 0:
        case 1: 
            SetValue(buf, start, 1); 
            return;

        case 2:
            SetValue(buf, start, 2);
            break;

        case 3: 
        case 4: 
        case 5:
            SetValue(buf, start, 3);
            SetValue(buf, start+1, unsigned(num-3));
            break;

        default:
            if (num < 38)
            {
                unsigned n6 = unsigned(num-6);
                SetValue(buf, start,   3);
                SetValue(buf, start+1, 3);
                SetValue(buf, start+2,  n6 >> 4);
                SetValue(buf, start+3, (n6 >> 2) & 3);
                SetValue(buf, start+4,  n6       & 3);
            }
            else
            {
                SetValue(buf, start,   3);
                SetValue(buf, start+1, 3);
                SetValue(buf, start+2, 3);
                buf[(2*start + 6 + 31) >> 5] = UInt32(num);
            }
            break;
        }
        SetValue(buf, start+num-1, 1);
    }


    // Mark the array of blocks as busy with alignment info. 
    // The first and the last values must be non-zero for proper merging. 
    // The other bits are set according to the described above encoding method.
    //--------------------------------------------------------------------
    static void MarkBusy(UInt32* buf, UPInt start, UPInt num, UPInt alignShift)
    {
        switch(num)
        {
        case 0:
        case 1: 
            SetValue(buf, start, 1); 
            return;

        case 2:
            SetValue(buf, start, 2);
            SetValue(buf, start+1, unsigned(alignShift+1));
            return;

        case 3: 
        case 4: 
        case 5:
            SetValue(buf, start, 3);
            SetValue(buf, start+1, unsigned(num-3));
            SetValue(buf, start+num-1, unsigned(alignShift+1));
            return;

        case 6:
        case 7:
            SetValue(buf, start,   3);
            SetValue(buf, start+1, 3);
            SetValue(buf, start+2, 0);
            SetValue(buf, start+3, 0);
            SetValue(buf, start+4, unsigned(num-6));
            SetValue(buf, start+num-1, unsigned(alignShift+1));
            return;

        default:
            {
                if (num < 38)
                {
                    unsigned n6 = unsigned(num-6);
                    SetValue(buf, start,   3);
                    SetValue(buf, start+1, 3);
                    SetValue(buf, start+2,  n6 >> 4);
                    SetValue(buf, start+3, (n6 >> 2) & 3);
                    SetValue(buf, start+4,  n6       & 3);
                }
                else
                {
                    SetValue(buf, start,   3);
                    SetValue(buf, start+1, 3);
                    SetValue(buf, start+2, 3);
                    buf[(2*start + 6 + 31) >> 5] = UInt32(num);
                }
                unsigned a = unsigned(alignShift << 1) | 1;
                SetValue(buf, start+num-3,  a >> 4);
                SetValue(buf, start+num-2, (a >> 2) & 3);
                SetValue(buf, start+num-1,  a       & 3);
            }
            return;
        }
    }

    // Calculate the block size according to the described above
    // encoding method.
    //--------------------------------------------------------------------
    static UPInt GetBlockSize(const UInt32* buf, UPInt start)
    {
        unsigned size;
        size = GetValue(buf, start);
        if (size < 3)
        {
            return size;
        }
        size = GetValue(buf, start+1);
        if (size < 3)
        {
            return size+3;
        }
        size = GetValue(buf, start+2);
        if (size < 3)
        {
            return 6 + ((size << 4) | 
                (GetValue(buf, start+3) << 2) | 
                 GetValue(buf, start+4));
        }
        return buf[(2*start + 6 + 31) >> 5];
    }

    //--------------------------------------------------------------------
    static unsigned GetAlignShift(const UInt32* buf, UPInt start, UPInt num)
    {
        if (num < 8)
        {
            return GetValue(buf, start+num-1) - 1;
        }
        else
        {
            return (GetValue(buf, start+num-1) >> 1)|
                   (GetValue(buf, start+num-2) << 1)|
                   (GetValue(buf, start+num-3) << 3);
        }

    }
};


}} // Scaleform::Heap

#endif
