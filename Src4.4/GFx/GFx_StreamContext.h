/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_StreamContext.h
Content     :   
Created     :   
Authors     :   Artem Bolgar, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_StreamContext_H
#define INC_SF_GFX_StreamContext_H

#include "Kernel/SF_Types.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_CxForm.h"
#include "Render/Render_Color.h"
#include "GFx/GFx_Types.h"

namespace Scaleform { namespace GFx {

struct StreamContext
{
    const UByte*    pData;
    UPInt           DataSize;
    UPInt           CurByteIndex;
    unsigned        CurBitIndex;

    StreamContext(const UByte* pdata, UPInt sz = SF_MAX_UPINT) : 
    pData(pdata), DataSize(sz), CurByteIndex(0), CurBitIndex(0) {}

    // Primitive types
    UInt32      ReadU32();
    UInt16      ReadU16();
    UInt8       ReadU8();
    SInt8       ReadS8();
    unsigned    ReadUInt4();
    unsigned    ReadUInt5();
    unsigned    ReadUInt(unsigned bitcount);                    // Defn in Stream.cpp
    int         ReadSInt(unsigned bitcount);
    bool        ReadUInt1();
    float       ReadFloat();

    // Complex types
    void        ReadRgb(Color *pc);
    void        ReadRgba(Color *pc);
    void        ReadMatrix(Render::Matrix2F *pm);          // Defn in Stream.cpp
    void        ReadCxformRgb(Render::Cxform *pcxform);  // "
    void        ReadCxformRgba(Render::Cxform *pcxform); // "

    // Utility
    void        Align();
    void        Skip(UInt32 d);

};

SF_INLINE void StreamContext::Align()
{
    if (CurBitIndex != 0)
        ++CurByteIndex;
    CurBitIndex = 0;
}

SF_INLINE UInt32 StreamContext::ReadU32()
{
    Align();

    SF_ASSERT(pData && CurByteIndex + sizeof(UInt32) <= DataSize);
    UInt32 v = (UInt32(pData[CurByteIndex]))       | (UInt32(pData[CurByteIndex+1])<<8) |
        (UInt32(pData[CurByteIndex+2])<<16) | (UInt32(pData[CurByteIndex+3])<<24);
    CurByteIndex += sizeof(UInt32);
    return v;
}

SF_INLINE float StreamContext::ReadFloat()
{
    Align();

    SF_ASSERT(pData && CurByteIndex + sizeof(UInt32) <= DataSize);
    union
    {
        UInt32 v;
        float  f;
    };
    v = (UInt32(pData[CurByteIndex]))       | (UInt32(pData[CurByteIndex+1])<<8) |
        (UInt32(pData[CurByteIndex+2])<<16) | (UInt32(pData[CurByteIndex+3])<<24);
    CurByteIndex += sizeof(UInt32);
    return f;
}


SF_INLINE UInt16 StreamContext::ReadU16()
{
    Align();

    SF_ASSERT(pData && CurByteIndex + sizeof(UInt16) <= DataSize);
    UInt16 v = (UInt16((pData[CurByteIndex]) | (UInt32(pData[CurByteIndex+1])<<8)));
    CurByteIndex += sizeof(UInt16);
    return v;
}

SF_INLINE UInt8 StreamContext::ReadU8()
{
    Align();

    SF_ASSERT(pData && CurByteIndex + 1 <= DataSize);
    return pData[CurByteIndex++];
}

SF_INLINE SInt8 StreamContext::ReadS8()
{
    return (SInt8) ReadU8();
}

SF_INLINE unsigned StreamContext::ReadUInt4()
{
    SF_ASSERT(pData && CurByteIndex < DataSize);
    unsigned v;
    switch(CurBitIndex)
    {
    case 0:
        v = ((pData[CurByteIndex] & 0xF0) >> 4);
        CurBitIndex += 4;
        break;
    case 1:
        v = ((pData[CurByteIndex] & 0x78) >> 3);
        CurBitIndex += 4;
        break;
    case 2:
        v = ((pData[CurByteIndex] & 0x3C) >> 2);
        CurBitIndex += 4;
        break;
    case 3:
        v = ((pData[CurByteIndex] & 0x1E) >> 1);
        CurBitIndex += 4;
        break;
    case 4:
        v = (pData[CurByteIndex] & 0x0F);
        CurBitIndex = 0;
        ++CurByteIndex;
        break;
    case 5:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x07) << 1) | (pData[CurByteIndex + 1] >> 7);
        CurBitIndex = 1;
        ++CurByteIndex;
        break;
    case 6:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x03) << 2) | (pData[CurByteIndex + 1] >> 6);
        CurBitIndex = 2;
        ++CurByteIndex;
        break;
    case 7:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x01) << 3) | (pData[CurByteIndex + 1] >> 5);
        CurBitIndex = 3;
        ++CurByteIndex;
        break;
    default: SF_ASSERT(0); v = 0;
    }
    return v;
}

SF_INLINE unsigned StreamContext::ReadUInt5()
{
    SF_ASSERT(pData && CurByteIndex < DataSize);
    unsigned v;
    switch(CurBitIndex)
    {
    case 0:
        v = ((pData[CurByteIndex] & 0xF8) >> 3);
        CurBitIndex += 5;
        break;
    case 1:
        v = ((pData[CurByteIndex] & 0x7C) >> 2);
        CurBitIndex += 5;
        break;
    case 2:
        v = ((pData[CurByteIndex] & 0x3E) >> 1);
        CurBitIndex += 5;
        break;
    case 3:
        v = (pData[CurByteIndex] & 0x1F);
        CurBitIndex = 0;
        ++CurByteIndex;
        break;
    case 4:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x0F) << 1) | (pData[CurByteIndex + 1] >> 7);
        CurBitIndex = 1;
        ++CurByteIndex;
        break;
    case 5:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x07) << 2) | (pData[CurByteIndex + 1] >> 6);
        CurBitIndex = 2;
        ++CurByteIndex;
        break;
    case 6:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x03) << 3) | (pData[CurByteIndex + 1] >> 5);
        CurBitIndex = 3;
        ++CurByteIndex;
        break;
    case 7:
        SF_ASSERT(CurByteIndex + 1 < DataSize);
        v = ((pData[CurByteIndex] & 0x01) << 4) | (pData[CurByteIndex + 1] >> 4);
        CurBitIndex = 4;
        ++CurByteIndex;
        break;
    default: SF_ASSERT(0); v = 0;
    }
    return v;
}

SF_INLINE int  StreamContext::ReadSInt(unsigned bitcount)
{
    SF_ASSERT(bitcount <= 32);

    SInt32  value = (SInt32) ReadUInt(bitcount);

    // Sign extend...
    if (value & (1 << (bitcount - 1)))
    {
        value |= -1 << bitcount;
    }

    return value;
}

SF_INLINE bool StreamContext::ReadUInt1()
{
    // Seems like the following code is faster than the commented one,
    // at least on PC....
    unsigned v = pData[CurByteIndex] & (1 << (7 - CurBitIndex));
    ++CurBitIndex;
    if (CurBitIndex >= 8) 
    {
        CurBitIndex = 0;
        ++CurByteIndex;
    }
    //unsigned v;
    //switch(CurBitIndex)
    //{
    //case 0: v = pData[CurByteIndex] & 0x80; ++CurBitIndex; break;
    //case 1: v = pData[CurByteIndex] & 0x40; ++CurBitIndex; break;
    //case 2: v = pData[CurByteIndex] & 0x20; ++CurBitIndex; break;
    //case 3: v = pData[CurByteIndex] & 0x10; ++CurBitIndex; break;
    //case 4: v = pData[CurByteIndex] & 0x08; ++CurBitIndex; break;
    //case 5: v = pData[CurByteIndex] & 0x04; ++CurBitIndex; break;
    //case 6: v = pData[CurByteIndex] & 0x02; ++CurBitIndex; break;
    //case 7: v = pData[CurByteIndex] & 0x01; CurBitIndex = 0; ++CurByteIndex; break;
    //default: SF_ASSERT(0); v = 0;
    //}
    return v != 0;
}


SF_INLINE void StreamContext::Skip(UInt32 d)
{
    Align();

    SF_ASSERT(CurByteIndex + d <= DataSize);
    CurByteIndex += d;
}


SF_INLINE void StreamContext::ReadRgb(Color *pc)
{
    // Read: R, G, B
    pc->SetRed(ReadU8());
    pc->SetGreen(ReadU8());
    pc->SetBlue(ReadU8());
    pc->SetAlpha(0xFF); 
}

SF_INLINE void StreamContext::ReadRgba(Color *pc)
{
    // Read: RGB, A
    ReadRgb(pc);
    pc->SetAlpha(ReadU8()); 
}
}} // Scaleform::GFx

#endif // INC_SF_GFX_StreamContext_H
