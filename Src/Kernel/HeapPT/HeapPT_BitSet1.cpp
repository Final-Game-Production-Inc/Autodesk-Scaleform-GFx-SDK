/**************************************************************************

Filename    :   HeapPT_BitSet1.cpp
Content     :   Static tables bor BitSet1
            :
Created     :   March, 2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_BitSet1.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
const UInt32 BitSet1::HeadUsedTable[32] = 
{
    0xFFFFFFFFU,0xFFFFFFFEU,0xFFFFFFFCU,0xFFFFFFF8U,
    0xFFFFFFF0U,0xFFFFFFE0U,0xFFFFFFC0U,0xFFFFFF80U,
    0xFFFFFF00U,0xFFFFFE00U,0xFFFFFC00U,0xFFFFF800U,
    0xFFFFF000U,0xFFFFE000U,0xFFFFC000U,0xFFFF8000U,
    0xFFFF0000U,0xFFFE0000U,0xFFFC0000U,0xFFF80000U,
    0xFFF00000U,0xFFE00000U,0xFFC00000U,0xFF800000U,
    0xFF000000U,0xFE000000U,0xFC000000U,0xF8000000U,
    0xF0000000U,0xE0000000U,0xC0000000U,0x80000000U
};

//------------------------------------------------------------------------
const UInt32 BitSet1::TailUsedTable[32] = 
{
    0x00000001U,0x00000003U,0x00000007U,0x0000000FU,
    0x0000001FU,0x0000003FU,0x0000007FU,0x000000FFU,
    0x000001FFU,0x000003FFU,0x000007FFU,0x00000FFFU,
    0x00001FFFU,0x00003FFFU,0x00007FFFU,0x0000FFFFU,
    0x0001FFFFU,0x0003FFFFU,0x0007FFFFU,0x000FFFFFU,
    0x001FFFFFU,0x003FFFFFU,0x007FFFFFU,0x00FFFFFFU,
    0x01FFFFFFU,0x03FFFFFFU,0x07FFFFFFU,0x0FFFFFFFU,
    0x1FFFFFFFU,0x3FFFFFFFU,0x7FFFFFFFU,0xFFFFFFFFU
};

//------------------------------------------------------------------------
const UInt32 BitSet1::HeadFreeTable[32] = 
{
    0x00000000U,0x00000001U,0x00000003U,0x00000007U,
    0x0000000FU,0x0000001FU,0x0000003FU,0x0000007FU,
    0x000000FFU,0x000001FFU,0x000003FFU,0x000007FFU,
    0x00000FFFU,0x00001FFFU,0x00003FFFU,0x00007FFFU,
    0x0000FFFFU,0x0001FFFFU,0x0003FFFFU,0x0007FFFFU,
    0x000FFFFFU,0x001FFFFFU,0x003FFFFFU,0x007FFFFFU,
    0x00FFFFFFU,0x01FFFFFFU,0x03FFFFFFU,0x07FFFFFFU,
    0x0FFFFFFFU,0x1FFFFFFFU,0x3FFFFFFFU,0x7FFFFFFFU
};

//------------------------------------------------------------------------
const UInt32 BitSet1::TailFreeTable[32] = 
{
    0xFFFFFFFEU,0xFFFFFFFCU,0xFFFFFFF8U,0xFFFFFFF0U,
    0xFFFFFFE0U,0xFFFFFFC0U,0xFFFFFF80U,0xFFFFFF00U,
    0xFFFFFE00U,0xFFFFFC00U,0xFFFFF800U,0xFFFFF000U,
    0xFFFFE000U,0xFFFFC000U,0xFFFF8000U,0xFFFF0000U,
    0xFFFE0000U,0xFFFC0000U,0xFFF80000U,0xFFF00000U,
    0xFFE00000U,0xFFC00000U,0xFF800000U,0xFF000000U,
    0xFE000000U,0xFC000000U,0xF8000000U,0xF0000000U,
    0xE0000000U,0xC0000000U,0x80000000U,0x00000000U
};

//------------------------------------------------------------------------
const UByte BitSet1::LastFreeBlock[256] = 
{
    8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
    5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0
};

//------------------------------------------------------------------------
const UByte BitSet1::LastUsedBlock[256] = 
{
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,8
};

}} // Scaleform::Heap

