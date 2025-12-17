/**************************************************************************

Filename    :   GFx_PathDataStorage.h
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Compact path data storage

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFxPathDataStorage_H
#define INC_SF_GFxPathDataStorage_H

#include "Kernel/SF_Types.h"
#include "GFx_Types.h"

namespace Scaleform { namespace GFx {

//------------------------------------------------------------------------
// Data types. xxxx - payload, S - sign bit; Bit order in bytes: 76543210
//
//           Byte 0      Byte 1
// UInt15:  |xxxxxxx|0|            : 0...127
//          |xxxxxxx|1| |xxxxxxxx| : 128...32767
//
//           Byte 0      Byte 1
// SInt15:  |xxxxxxx|0|            : -64...63
//          |xxxxxxx|1| |Sxxxxxxx| : -16384...16383
//
//           Byte 0      Byte 1     Byte 2     Byte 3
// UInt30:  |xxxxxx|00|                                  : 0...63
//          |xxxxxx|01| |xxxxxxxx|                       : 128...32767
//          |xxxxxx|10| |xxxxxxxx| |xxxxxxxx|            : 32768...2^22-1
//          |xxxxxx|11| |xxxxxxxx| |xxxxxxxx| |xxxxxxxx| : 2^22...2^30-1
//
//           Byte 0      Byte 1     Byte 2     Byte 3
// SInt30:  |xxxxxx|00|                                  : -32...31
//          |xxxxxx|01| |xxxxxxxx|                       : -8192...8191
//          |xxxxxx|10| |xxxxxxxx| |xxxxxxxx|            : -2^21...2^21-1
//          |xxxxxx|11| |xxxxxxxx| |xxxxxxxx| |Sxxxxxxx| : -2^29...2^29-1
//
// Edges:
//          Data for the edges consists of 4-bit edge type and the payload in the 
//          4 bits of this byte plus 1...9 next bytes. 
//
//           Byte 0
//          |xxxx|eeee| |xxxxxxxx| ... where "eeee" - 4-bit edge type
//
// Edge types:
//          Edge_H12 = 0,  //  2 bytes - 12-bit horizontal line (4 + 12      = 16 = 2 bytes)
//          Edge_H20 = 1,  //  3 bytes - 20-bit horizontal line (4 + 20      = 24 = 3 bytes)
//          Edge_V12 = 2,  //  2 bytes - 12-bit vertical line   (4 + 12      = 16 = 2 bytes)
//          Edge_V20 = 3,  //  3 bytes - 20-bit vertical line   (4 + 20      = 24 = 3 bytes)
//          Edge_L6  = 4,  //  2 bytes -  6-bit general line    (4 + 6  + 6  = 16 = 2 bytes)
//          Edge_L10 = 5,  //  3 bytes - 10-bit general line    (4 + 10 + 10 = 24 = 3 bytes)
//          Edge_L14 = 6,  //  4 bytes - 14-bit general line    (4 + 14 + 14 = 32 = 4 bytes)
//          Edge_L18 = 7,  //  5 bytes - 18-bit general line    (4 + 18 + 18 = 40 = 5 bytes)
//          Edge_C5  = 8,  //  3 bytes -  5-bit quadratic curve (4 + 5  + 5  + 5  + 5  = 3 bytes)
//          Edge_C7  = 9,  //  4 bytes -  7-bit quadratic curve (4 + 7  + 7  + 7  + 7  = 4 bytes)
//          Edge_C9  = 10, //  5 bytes -  9-bit quadratic curve (4 + 9  + 9  + 9  + 9  = 5 bytes)
//          Edge_C11 = 11, //  6 bytes - 11-bit quadratic curve (4 + 11 + 11 + 11 + 11 = 6 bytes)
//          Edge_C13 = 12, //  7 bytes - 13-bit quadratic curve (4 + 13 + 13 + 13 + 13 = 7 bytes)
//          Edge_C15 = 13, //  8 bytes - 15-bit quadratic curve (4 + 15 + 15 + 15 + 15 = 8 bytes)
//          Edge_C17 = 14, //  9 bytes - 17-bit quadratic curve (4 + 17 + 17 + 17 + 17 = 9 bytes)
//          Edge_C19 = 15  // 10 bytes - 19-bit quadratic curve (4 + 19 + 19 + 19 + 19 = 10 bytes)
//
// Edge data: 
//          X,Y may mean absolute values as well as relative ones 
//          Horizontal and Vertical lines: X or Y respectively. 
//          General Lines:                 X, Y
//          Quadratic Curves:              CX, CY, AX, AY
//
// An example of Edge_C7:
//           Byte 0      Byte 1      Byte 2      Byte 3
//          |aaaa|1001| |bbbbb|Aaa| |cccccc|Bb| |Ddddddd|C| 
//
// Where:    Aaaaaaa - CX, Bbbbbbb - CY, Ccccccc - AX, Ddddddd - AY
//           (capital letter means sign bit.)
//
//------------------------------------------------------------------------
struct PathDataTypes
{
    enum RawEdgeType
    {
        Edge_H12 = 0,  //  2 bytes
        Edge_H20 = 1,  //  3 bytes
        Edge_V12 = 2,  //  2 bytes
        Edge_V20 = 3,  //  3 bytes
        Edge_L6  = 4,  //  2 bytes
        Edge_L10 = 5,  //  3 bytes
        Edge_L14 = 6,  //  4 bytes
        Edge_L18 = 7,  //  5 bytes
        Edge_C5  = 8,  //  3 bytes
        Edge_C7  = 9,  //  4 bytes
        Edge_C9  = 10, //  5 bytes
        Edge_C11 = 11, //  6 bytes
        Edge_C13 = 12, //  7 bytes
        Edge_C15 = 13, //  8 bytes
        Edge_C17 = 14, //  9 bytes
        Edge_C19 = 15  // 10 bytes
    };

    enum RangeType
    {
        MaxUInt6  =  (1 << 6)  - 1,
        MaxUInt7  =  (1 << 7)  - 1,
        MaxUInt14 =  (1 << 14) - 1,
        MaxUInt22 =  (1 << 22) - 1,
        MaxUInt30 =  (1 << 30) - 1,

        MinSInt5  = -(1 << 4),
        MaxSInt5  =  (1 << 4)  - 1,
        MinSInt6  = -(1 << 5),
        MaxSInt6  =  (1 << 5)  - 1,
        MinSInt7  = -(1 << 6),
        MaxSInt7  =  (1 << 6)  - 1,
        MinSInt8  = -(1 << 7),
        MaxSInt8  =  (1 << 7)  - 1,
        MinSInt9  = -(1 << 8),
        MaxSInt9  =  (1 << 8)  - 1,
        MinSInt10 = -(1 << 9),
        MaxSInt10 =  (1 << 9)  - 1,
        MinSInt11 = -(1 << 10),
        MaxSInt11 =  (1 << 10) - 1,
        MinSInt12 = -(1 << 11),
        MaxSInt12 =  (1 << 11) - 1,
        MinSInt13 = -(1 << 12),
        MaxSInt13 =  (1 << 12) - 1,
        MinSInt14 = -(1 << 13),
        MaxSInt14 =  (1 << 13) - 1,
        MinSInt15 = -(1 << 14),
        MaxSInt15 =  (1 << 14) - 1,
        MinSInt17 = -(1 << 16),
        MaxSInt17 =  (1 << 16) - 1,
        MinSInt18 = -(1 << 17),
        MaxSInt18 =  (1 << 17) - 1,
        MinSInt19 = -(1 << 18),
        MaxSInt19 =  (1 << 18) - 1,
        MinSInt20 = -(1 << 19),
        MaxSInt20 =  (1 << 19) - 1,
        MinSInt22 = -(1 << 21),
        MaxSInt22 =  (1 << 21) - 1,
    };

    enum EdgeType
    {
        Edge_HLine,
        Edge_VLine,
        Edge_Line,
        Edge_Quad
    };
};

//------------------------------------------------------------------------
template<class ContainerType> class PathDataEncoder : public PathDataTypes
{
public:
    PathDataEncoder(ContainerType& data);

    void Clear();

    void WriteChar(char v) { Data->PushBack(UInt8(v)); }

    void WriteUInt16fixlen(unsigned v);
    void WriteUInt32fixlen(unsigned v);

    void WriteSInt16fixlen(int v);
    void WriteSInt32fixlen(int v);

    void UpdateUInt16fixlen(unsigned pos, unsigned v);
    void UpdateUInt32fixlen(unsigned pos, unsigned v);

    void UpdateSInt16fixlen(unsigned pos, int v);
    void UpdateSInt32fixlen(unsigned pos, int v);

    unsigned WriteUInt15(unsigned v);
    unsigned WriteUInt30(unsigned v);

    unsigned WriteSInt15(int v);
    unsigned WriteSInt30(int v);

    unsigned WriteHLine(int x);
    unsigned WriteVLine(int y);
    unsigned WriteLine (int x, int y);
    unsigned WriteQuad (int cx, int cy, int ax, int ay);

    void CutAt(unsigned pos) { Data->CutAt(pos); }

    UPInt GetSize() const { return Data->GetSize(); }
    void Serialize(void* ptr, unsigned start, unsigned size) const;
    void Deserialize(const void* ptr, unsigned size);

private:
    ContainerType*  Data;
};

//------------------------------------------------------------------------
template<class ContainerType> class PathDataDecoder : public PathDataTypes
{
public:
    PathDataDecoder(const ContainerType& data);

    unsigned             GetSize() const { return (unsigned)Data->GetSize(); }
    const ContainerType& GetData() const { return *Data; }
 
    char     ReadChar(unsigned pos) const { return (char)Data->ValueAt(pos); }

    unsigned ReadUInt16fixlen(unsigned pos) const;
    unsigned ReadUInt32fixlen(unsigned pos) const;

    int      ReadSInt16fixlen(unsigned pos) const;
    int      ReadSInt32fixlen(unsigned pos) const;

    unsigned ReadSInt15(unsigned pos, int* v) const;
    unsigned ReadSInt30(unsigned pos, int* v) const;

    unsigned ReadUInt15(unsigned pos, unsigned* v) const;
    unsigned ReadUInt30(unsigned pos, unsigned* v) const;

    unsigned ReadRawEdge(unsigned pos, UInt8* data) const; // data must be at least UInt8[10];
    unsigned ReadEdge   (unsigned pos, int*  data) const; // data must be at least int[5];

private:
    const ContainerType*    Data;
    static const UInt8      Sizes[16];
};




//------------------------------------------------------------------------
template<class ContainerType>
PathDataEncoder<ContainerType>::PathDataEncoder(ContainerType& data) :
    Data(&data)
{
}

//------------------------------------------------------------------------
template<class ContainerType>
void PathDataEncoder<ContainerType>::Clear()
{
    Data->Clear();
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::WriteUInt16fixlen(unsigned v)
{
    Data->PushBack(UInt8(v));
    Data->PushBack(UInt8(v >> 8));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::WriteUInt32fixlen(unsigned v)
{
    Data->PushBack(UInt8(v));
    Data->PushBack(UInt8(v >> 8));
    Data->PushBack(UInt8(v >> 16));
    Data->PushBack(UInt8(v >> 24));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::WriteSInt16fixlen(int v)
{
    Data->PushBack(UInt8(v));
    Data->PushBack(UInt8(v >> 8));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::WriteSInt32fixlen(int v)
{
    Data->PushBack(UInt8(v));
    Data->PushBack(UInt8(v >> 8));
    Data->PushBack(UInt8(v >> 16));
    Data->PushBack(UInt8(v >> 24));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::UpdateUInt16fixlen(unsigned pos, unsigned v)
{
    Data->At(pos  ) = UInt8(v);
    Data->At(pos+1) = UInt8(v >> 8);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::UpdateUInt32fixlen(unsigned pos, unsigned v)
{
    Data->At(pos  ) = UInt8(v);
    Data->At(pos+1) = UInt8(v >> 8);
    Data->At(pos+2) = UInt8(v >> 16);
    Data->At(pos+3) = UInt8(v >> 24);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::UpdateSInt16fixlen(unsigned pos, int v)
{
    Data->At(pos  ) = UInt8(v);
    Data->At(pos+1) = UInt8(v >> 8);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline void PathDataEncoder<ContainerType>::UpdateSInt32fixlen(unsigned pos, int v)
{
    Data->At(pos  ) = UInt8(v);
    Data->At(pos+1) = UInt8(v >> 8);
    Data->At(pos+2) = UInt8(v >> 16);
    Data->At(pos+3) = UInt8(v >> 24);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataEncoder<ContainerType>::WriteUInt15(unsigned v)
{
    if (v <= MaxUInt7)
    {
        Data->PushBack(UInt8(v << 1));
        return 1;
    }
    Data->PushBack(UInt8((v << 1) | 1));
    Data->PushBack(UInt8 (v >> 7));
    return 2;
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataEncoder<ContainerType>::WriteSInt15(int v)
{
    if (v >= MinSInt7 && v <= MaxSInt7)
    {
        Data->PushBack(UInt8(v << 1));
        return 1;
    }
    Data->PushBack(UInt8((v << 1) | 1));
    Data->PushBack(UInt8 (v >> 7));
    return 2;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteUInt30(unsigned v)
{
    if (v <= MaxUInt6)
    {
        Data->PushBack(UInt8(v << 2));
        return 1;
    }
    if (v <= MaxUInt14)
    {
        Data->PushBack(UInt8((v << 2) | 1));
        Data->PushBack(UInt8 (v >> 6));
        return 2;
    }
    if (v <= MaxUInt22)
    {
        Data->PushBack(UInt8((v << 2) | 2));
        Data->PushBack(UInt8 (v >> 6));
        Data->PushBack(UInt8 (v >> 14));
        return 3;
    }
    Data->PushBack(UInt8((v << 2) | 3));
    Data->PushBack(UInt8 (v >> 6));
    Data->PushBack(UInt8 (v >> 14));
    Data->PushBack(UInt8 (v >> 22));
    return 4;
}


//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteSInt30(int v)
{
    if (v >= MinSInt6 && v <= MaxSInt6)
    {
        Data->PushBack(UInt8(v << 2));
        return 1;
    }
    if (v >= MinSInt14 && v <= MaxSInt14)
    {
        Data->PushBack(UInt8((v << 2) | 1));
        Data->PushBack(UInt8 (v >> 6));
        return 2;
    }
    if (v >= MinSInt22 && v <= MaxSInt22)
    {
        Data->PushBack(UInt8((v << 2) | 2));
        Data->PushBack(UInt8 (v >> 6));
        Data->PushBack(UInt8 (v >> 14));
        return 3;
    }
    Data->PushBack(UInt8((v << 2) | 3));
    Data->PushBack(UInt8 (v >> 6));
    Data->PushBack(UInt8 (v >> 14));
    Data->PushBack(UInt8 (v >> 22));
    return 4;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteHLine(int x)
{
    if (x >= MinSInt12 && x <= MaxSInt12)
    {
        Data->PushBack(UInt8((x << 4) | Edge_H12));
        Data->PushBack(UInt8 (x >> 4));
        return 2;
    }
    Data->PushBack(UInt8((x << 4) | Edge_H20));
    Data->PushBack(UInt8 (x >> 4));
    Data->PushBack(UInt8 (x >> 12));
    return 3;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteVLine(int y)
{
    if (y >= MinSInt12 && y <= MaxSInt12)
    {
        Data->PushBack(UInt8((y << 4) | Edge_V12));
        Data->PushBack(UInt8 (y >> 4));
        return 2;
    }
    Data->PushBack(UInt8((y << 4) | Edge_V20));
    Data->PushBack(UInt8 (y >> 4));
    Data->PushBack(UInt8 (y >> 12));
    return 3;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteLine(int x, int y)
{
    enum { m2=3, m6=0x3F };

    if (x >= MinSInt6 && x <= MaxSInt6 && y >= MinSInt6 && y <= MaxSInt6)
    {
        Data->PushBack(UInt8( (x << 4) | Edge_L6));
        Data->PushBack(UInt8(((x >> 4) & m2) | (y << 2)));
        return 2;
    }
    if (x >= MinSInt10 && x <= MaxSInt10 && y >= MinSInt10 && y <= MaxSInt10)
    {
        Data->PushBack(UInt8 ((x << 4) | Edge_L10));
        Data->PushBack(UInt8(((x >> 4) & m6) | (y << 6)));
        Data->PushBack(UInt8  (y >> 2));
        return 3;
    }
    if (x >= MinSInt14 && x <= MaxSInt14 && y >= MinSInt14 && y <= MaxSInt14)
    {
        Data->PushBack(UInt8 ((x << 4) | Edge_L14));
        Data->PushBack(UInt8  (x >> 4));
        Data->PushBack(UInt8(((x >> 12) & m2) | (y << 2)));
        Data->PushBack(UInt8  (y >> 6));
        return 4;
    }
    Data->PushBack(UInt8 ((x << 4) | Edge_L18));
    Data->PushBack(UInt8  (x >> 4));
    Data->PushBack(UInt8(((x >> 12) & m6) | (y << 6)));
    Data->PushBack(UInt8  (y >> 2));
    Data->PushBack(UInt8  (y >> 10));
    return 5;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataEncoder<ContainerType>::WriteQuad(int cx, int cy, int ax, int ay)
{
    int minV = cx;
    int maxV = cx;
    if (cy < minV) minV = cy;
    if (cy > maxV) maxV = cy;
    if (ax < minV) minV = ax;
    if (ax > maxV) maxV = ax;
    if (ay < minV) minV = ay;
    if (ay > maxV) maxV = ay;

    enum { m1=1, m2=3, m3=7, m4=0xF, m5=0x1F, m6=0x3F, m7=0x7F };

    if (minV >= MinSInt5 && maxV <= MaxSInt5)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C5                            ));
        Data->PushBack(UInt8( ((cx >> 4) & m1) | ((cy << 1) & m6) | (ax << 6) ));
        Data->PushBack(UInt8( ((ax >> 2) & m3) |  (ay << 3)                   ));
        return 3;
    }
    if (minV >= MinSInt7 && maxV <= MaxSInt7)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C7            ));
        Data->PushBack(UInt8( ((cx >> 4) & m3) | (cy << 3)    ));
        Data->PushBack(UInt8( ((cy >> 5) & m2) | (ax << 2)    ));
        Data->PushBack(UInt8( ((ax >> 6) & m1) | (ay << 1)    ));
        return 4;
    }
    if (minV >= MinSInt9 && maxV <= MaxSInt9)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C9            ));
        Data->PushBack(UInt8( ((cx >> 4) & m5) | (cy << 5)    ));
        Data->PushBack(UInt8( ((cy >> 3) & m6) | (ax << 6)    ));
        Data->PushBack(UInt8( ((ax >> 2) & m7) | (ay << 7)    ));
        Data->PushBack(UInt8(  (ay >> 1)                      ));
        return 5;
    }
    if (minV >= MinSInt11 && maxV <= MaxSInt11)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C11           ));
        Data->PushBack(UInt8( ((cx >> 4) & m7) | (cy << 7)    ));
        Data->PushBack(UInt8(  (cy >> 1)                      ));
        Data->PushBack(UInt8( ((cy >> 9) & m2) | (ax << 2)    ));
        Data->PushBack(UInt8( ((ax >> 6) & m5) | (ay << 5)    ));
        Data->PushBack(UInt8(  (ay >> 3)                      ));
        return 6;
    }
    if (minV >= MinSInt13 && maxV <= MaxSInt13)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C13           ));
        Data->PushBack(UInt8(  (cx >> 4)                      ));
        Data->PushBack(UInt8( ((cx >> 12) & m1) | (cy << 1)   ));
        Data->PushBack(UInt8( ((cy >> 7)  & m6) | (ax << 6)   ));
        Data->PushBack(UInt8(  (ax >> 2)                      ));
        Data->PushBack(UInt8( ((ax >> 10) & m3) | (ay << 3)   ));
        Data->PushBack(UInt8(  (ay >> 5)                      ));
        return 7;
    }
    if (minV >= MinSInt15 && maxV <= MaxSInt15)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C15           ));
        Data->PushBack(UInt8(  (cx >> 4)                      ));
        Data->PushBack(UInt8( ((cx >> 12) & m3) | (cy << 3)   ));
        Data->PushBack(UInt8(  (cy >> 5)                      ));
        Data->PushBack(UInt8( ((cy >> 13) & m2) | (ax << 2)   ));
        Data->PushBack(UInt8(  (ax >> 6)                      ));
        Data->PushBack(UInt8( ((ax >> 14) & m1) | (ay << 1)   ));
        Data->PushBack(UInt8(  (ay >> 7)                      ));
        return 8;
    }
    if (minV >= MinSInt17 && maxV <= MaxSInt17)
    {
        Data->PushBack(UInt8(  (cx << 4) | Edge_C17           ));
        Data->PushBack(UInt8(  (cx >> 4)                      ));
        Data->PushBack(UInt8( ((cx >> 12) & m5) | (cy << 5)   ));
        Data->PushBack(UInt8(  (cy >> 3)                      ));
        Data->PushBack(UInt8( ((cy >> 11) & m6) | (ax << 6)   ));
        Data->PushBack(UInt8(  (ax >> 2)                      ));
        Data->PushBack(UInt8( ((ax >> 10) & m7) | (ay << 7)   ));
        Data->PushBack(UInt8(  (ay >> 1)                      ));
        Data->PushBack(UInt8(  (ay >> 9)                      ));
        return 9;
    }
    Data->PushBack(UInt8(  (cx << 4) | Edge_C19               ));
    Data->PushBack(UInt8(  (cx >> 4)                          ));
    Data->PushBack(UInt8( ((cx >> 12) & m7) | (cy << 7)       ));
    Data->PushBack(UInt8(  (cy >> 1)                          ));
    Data->PushBack(UInt8(  (cy >> 9)                          ));
    Data->PushBack(UInt8( ((cy >> 17) & m2) | (ax << 2)       ));
    Data->PushBack(UInt8(  (ax >> 6)                          ));
    Data->PushBack(UInt8( ((ax >> 14) & m5) | (ay << 5)       ));
    Data->PushBack(UInt8(  (ay >> 3)                          ));
    Data->PushBack(UInt8(  (ay >> 11)                         ));
    return 10;
}

//------------------------------------------------------------------------
template<class ContainerType>
void PathDataEncoder<ContainerType>::Serialize(void* ptr, unsigned start, unsigned size) const
{
    UInt8* dst = (UInt8*)ptr;
    for(unsigned i = 0; i < size; ++i)
        *dst++ = Data->ValueAt(start + i);
}

//------------------------------------------------------------------------
template<class ContainerType>
void PathDataEncoder<ContainerType>::Deserialize(const void* ptr, unsigned size)
{
    const UInt8* src = (const UInt8*)ptr;
    for(unsigned i = 0; i < size; ++i)
        Data->PushBack(*src++);
}








//------------------------------------------------------------------------
template<class ContainerType>
const UInt8 PathDataDecoder<ContainerType>::Sizes[16] = 
{
    1,2,1,2,1,2,3,4,2,3,4,5,6,7,8,9
};

//------------------------------------------------------------------------
template<class ContainerType>
PathDataDecoder<ContainerType>::PathDataDecoder(const ContainerType& data) :
    Data(&data)
{
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataDecoder<ContainerType>::ReadUInt16fixlen(unsigned pos) const
{
    return Data->ValueAt(pos) | (Data->ValueAt(pos + 1) << 8);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataDecoder<ContainerType>::ReadUInt32fixlen(unsigned pos) const
{
    return Data->ValueAt(pos)| 
          (Data->ValueAt(pos + 1) << 8 )|
          (Data->ValueAt(pos + 2) << 16)|
          (Data->ValueAt(pos + 3) << 24);
}

//------------------------------------------------------------------------
template<class ContainerType>
inline int PathDataDecoder<ContainerType>::ReadSInt16fixlen(unsigned pos) const
{
    return SInt16(ReadUInt16fixlen(pos));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline int PathDataDecoder<ContainerType>::ReadSInt32fixlen(unsigned pos) const
{
    return int (ReadUInt32fixlen(pos));
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataDecoder<ContainerType>::ReadUInt15(unsigned pos, unsigned* v) const
{
    unsigned t = UInt8(Data->ValueAt(pos));
    if ((t & 1) == 0)
    {
        *v = t >> 1;
        return 1;
    }
    t  = unsigned(t >> 1);
    *v = unsigned(t | (UInt8(Data->ValueAt(pos+1)) << 7));
    return 2;
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataDecoder<ContainerType>::ReadSInt15(unsigned pos, int* v) const
{
    int t = SInt8(Data->ValueAt(pos));
    if ((t & 1) == 0)
    {
        *v = t >> 1;
        return 1;
    }
    t  = int ((t >> 1) & MaxUInt7);
    *v = int   (t | (SInt8(Data->ValueAt(pos+1)) << 7));
    return 2;
}

//------------------------------------------------------------------------
template<class ContainerType>
inline unsigned PathDataDecoder<ContainerType>::ReadRawEdge(unsigned pos, UInt8* data) const
{
    *data = Data->ValueAt(pos++);
    unsigned nb = Sizes[*data & 0xF];
    unsigned i;
    ++data;
    for (i = 0; i < nb; i++)
        *data++ = Data->ValueAt(pos++);

    return nb + 1;
}


//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataDecoder<ContainerType>::ReadUInt30(unsigned pos, unsigned* v) const
{
    unsigned t = UInt8(Data->ValueAt(pos));
    switch(t & 3)
    {
    case 0:
        *v = t >> 2;
        return 1;

    case 1:
        t  = t >> 2;
        *v = t | (Data->ValueAt(pos+1) << 6);
        return 2;

    case 2:
        t  = t >> 2;
        t  = t | (Data->ValueAt(pos+1) << 6);
        *v = t | (Data->ValueAt(pos+2) << 14);
        return 3;
    }
    t  = t >> 2;
    t  = t | (Data->ValueAt(pos+1) << 6);
    t  = t | (Data->ValueAt(pos+2) << 14);
    *v = t | (Data->ValueAt(pos+3) << 22);
    return 4;
}


//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataDecoder<ContainerType>::ReadSInt30(unsigned pos, int* v) const
{
    int t = SInt8(Data->ValueAt(pos));
    switch(t & 3)
    {
    case 0:
        *v = t >> 2;
        return 1;

    case 1:
        t  = int((t >> 2) & MaxUInt6);
        *v = int(t | (SInt8(Data->ValueAt(pos+1)) << 6));
        return 2;

    case 2:
        t  = int((t >> 2) & MaxUInt6);
        t  = int(t | (UInt8(Data->ValueAt(pos+1)) << 6));
        *v = int(t | (SInt8(Data->ValueAt(pos+2)) << 14));
        return 3;
    }
    t  = int((t >> 2) & MaxUInt6);
    t  = int(t | (UInt8(Data->ValueAt(pos+1)) << 6));
    t  = int(t | (UInt8(Data->ValueAt(pos+2)) << 14));
    *v = int(t | (SInt8(Data->ValueAt(pos+3)) << 22));
    return 4;
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned PathDataDecoder<ContainerType>::ReadEdge(unsigned pos, int* data) const
{
    UInt8 buff[10];
    unsigned  nb = ReadRawEdge(pos, buff);

    switch(buff[0] & 0xF)
    {
    case Edge_H12:
        data[0] = Edge_HLine;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1]) << 4);
        break;

    case Edge_H20:
        data[0] = Edge_HLine;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2]) << 12);
        break;

    case Edge_V12:
        data[0] = Edge_VLine;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1]) << 4);
        break;

    case Edge_V20:
        data[0] = Edge_VLine;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2]) << 12);
        break;

    case Edge_L6 :
        data[0] = Edge_Line;
        data[1] = int   (buff[0] >> 4) | (SInt8(buff[1] << 6) >> 2);
        data[2] = SInt8(buff[1]) >> 2;
        break;

    case Edge_L10:
        data[0] = Edge_Line;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1]  << 2) << 2);
        data[2] = int (buff[1] >> 6) | (SInt8(buff[2]) << 2);
        break;

    case Edge_L14:
        data[0] = Edge_Line;
        data[1] = int (buff[0] >> 4) |  int   (buff[1]  << 4) | (SInt8(buff[2] << 6) << 6);
        data[2] = int (buff[2] >> 2) | (SInt8(buff[3]) << 6);
        break;

    case Edge_L18:
        data[0] = Edge_Line;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2]  << 2) << 10);
        data[2] = int (buff[2] >> 6) | int (buff[3] << 2) | (SInt8(buff[4]) << 10);
        break;

    case Edge_C5 :
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1] << 7) >> 3);
        data[2] =                       SInt8(buff[1] << 2) >> 3;
        data[3] = int (buff[1] >> 6) | (SInt8(buff[2] << 5) >> 3);
        data[4] =                       SInt8(buff[2])      >> 3;
        break;

    case Edge_C7 :
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1] << 5) >> 1);
        data[2] = int (buff[1] >> 3) | (SInt8(buff[2] << 6) >> 1);
        data[3] = int (buff[2] >> 2) | (SInt8(buff[3] << 7) >> 1);
        data[4] =                       SInt8(buff[3])      >> 1;
        break;

    case Edge_C9 :
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | (SInt8(buff[1] << 3) << 1);
        data[2] = int (buff[1] >> 5) | (SInt8(buff[2] << 2) << 1);
        data[3] = int (buff[2] >> 6) | (SInt8(buff[3] << 1) << 1);
        data[4] = int (buff[3] >> 7) | (SInt8(buff[4]     ) << 1);
        break;

    case Edge_C11:
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) |                      (SInt8(buff[1] << 1) << 3);
        data[2] = int (buff[1] >> 7) | int (buff[2] << 1) | (SInt8(buff[3] << 6) << 3);
        data[3] = int (buff[3] >> 2) |                      (SInt8(buff[4] << 3) << 3);
        data[4] = int (buff[4] >> 5) |                      (SInt8(buff[5]     ) << 3);
        break;

    case Edge_C13:
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2] << 7) << 5);
        data[2] = int (buff[2] >> 1) |                      (SInt8(buff[3] << 2) << 5);
        data[3] = int (buff[3] >> 6) | int (buff[4] << 2) | (SInt8(buff[5] << 5) << 5);
        data[4] = int (buff[5] >> 3) |                      (SInt8(buff[6]     ) << 5);
        break;

    case Edge_C15:
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2] << 5) << 7);
        data[2] = int (buff[2] >> 3) | int (buff[3] << 5) | (SInt8(buff[4] << 6) << 7);
        data[3] = int (buff[4] >> 2) | int (buff[5] << 6) | (SInt8(buff[6] << 7) << 7);
        data[4] = int (buff[6] >> 1) |                      (SInt8(buff[7]     ) << 7);
        break;

    case Edge_C17:
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2] << 3) << 9);
        data[2] = int (buff[2] >> 5) | int (buff[3] << 3) | (SInt8(buff[4] << 2) << 9);
        data[3] = int (buff[4] >> 6) | int (buff[5] << 2) | (SInt8(buff[6] << 1) << 9);
        data[4] = int (buff[6] >> 7) | int (buff[7] << 1) | (SInt8(buff[8]     ) << 9);
        break;

    case Edge_C19:
        data[0] = Edge_Quad;
        data[1] = int (buff[0] >> 4) | int (buff[1] << 4) | (SInt8(buff[2] << 1) << 11);
        data[2] = int (buff[2] >> 7) | int (buff[3] << 1) |  int   (buff[4] << 9) | (SInt8(buff[5] << 6) << 11);
        data[3] = int (buff[5] >> 2) | int (buff[6] << 6) | (SInt8(buff[7] << 3) << 11);
        data[4] = int (buff[7] >> 5) | int (buff[8] << 3) | (SInt8(buff[9]     ) << 11);
        break;
    }
    return nb;
}

}} // Scaleform::GFx

#endif // INC_SF_GFxPathDataStorage_H
