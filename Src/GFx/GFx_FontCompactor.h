/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_FontCompactor.h
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Compact font data storage

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFxFontCompactor_H
#define INC_SF_GFxFontCompactor_H

#include "GFxConfig.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Render/Render_Types2D.h"
#include "Kernel/SF_ArrayUnsafe.h"
#include "Kernel/SF_ArrayPaged.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Font.h"
#include "GFx_PathDataStorage.h"

namespace Scaleform { namespace GFx {

#if defined(GFX_ENABLE_COMPACTED_FONTS) || defined(GFX_ENABLE_FONT_COMPACTOR)

// Font collection data structure:
//------------------------------------------------------------------------
//
// Font Collection:
//
//      FontType            Font[];         // while(pos < size)
//
// FontType:
//      Char                Name[];         // Null-terminated, possibly UTF-8 encoded
//      UInt16fixlen        Flags;
//      UInt16fixlen        nominalSize;
//      SInt16fixlen        Ascent;
//      SInt16fixlen        Descent;
//      SInt16fixlen        Leading;
//      UInt32fixlen        NumGlyphs;
//      UInt32fixlen        TotalGlyphBytes;                // To quckly jump to the tables
//      GlyphType           Glyphs[NumGlyphs];
//      GlyphInfoType       GlyphInfoTable[NumGlyphs];      // Ordered by GlyphCode
//      UInt30              KerningTableSize;
//      KerningPairType     KerningPairs[KerningTableSize]; // Ordered by Code1,Code2
//      
// GlyphType:
//      SInt15              BoundingBox[4];
//      UInt15              NumContours;
//      ContourType         Contours[NumContours];
//      
// ContourType:
//      SInt15              MoveToX;
//      SInt15              MoveToY;
//      UInt30              NumEdges OR Reference;
//          if  (NumEdges & 1) Go To (NumEdges >> 1) and read edges
//          else NumEdges >>= 1
//      EdgeType            Edges[NumEdges];    // No edgess in case of reference.
//      
// EdgeType:
//      See GFxPathDataStorage.cpp, edge description
//
// GlyphInfoType:
//      UInt16fixlen        GlyphCode;
//      SInt16fixlen        AdvanceX;
//      UInt32fixlen        GlobalOffset;       // Starting from BoundingBox
//
// KerningPair:
//      UInt16fixlen        Char1;
//      UInt16fixlen        Char2;
//      SInt16fixlen        Adjustment;
//
//------------------------------------------------------------------------




//------------------------------------------------------------------------
template<class ContainerType> class GlyphPathIterator
{
public:
    typedef PathDataDecoder<ContainerType> PathDataDecoderType;

    GlyphPathIterator(const ContainerType& data) : Data(data) {}

    void ReadBounds(unsigned pos)
    {
        int t;
        Pos  = pos;
        Pos += Data.ReadSInt15(Pos, &t); XMin = SInt16(t);
        Pos += Data.ReadSInt15(Pos, &t); YMin = SInt16(t);
        Pos += Data.ReadSInt15(Pos, &t); XMax = SInt16(t);
        Pos += Data.ReadSInt15(Pos, &t); YMax = SInt16(t);
    }

    bool ValidBounds() const
    {
        return XMin < XMax && YMin < YMax;
    }

    void SetSpaceBounds(unsigned advanceX)
    {
        XMin = YMin = YMax = 0;
        XMax = (SInt16)advanceX;
    }

    void StartGlyph(unsigned pos)
    {
        ReadBounds(pos);
        Pos += Data.ReadUInt15(Pos, &NumContours);
        readPathHeader();
    }

    RectF& GetBounds(RectF* r) const { *r = RectF(XMin, YMin, XMax, YMax); return *r; }
    int     GetWidth()   const { return XMax - XMin; }
    int     GetHeight()  const { return YMax - YMin; }

    bool    IsFinished() const { return NumContours == 0; }
    void    AdvancePath() { --NumContours; readPathHeader(); }

    int     GetMoveX() const { return MoveX; }
    int     GetMoveY() const { return MoveY; }

    bool    IsPathFinished() const { return NumEdges == 0; }
    void    ReadEdge(int * edge);

private:
    void readPathHeader();

    PathDataDecoderType Data;
    unsigned   Pos;
    SInt16      XMin, YMin, XMax, YMax;
    int         MoveX, MoveY;
    unsigned   NumContours;
    unsigned   NumEdges;
    unsigned   EdgePos;
    bool   JumpToPos;
};

//------------------------------------------------------------------------
template<class ContainerType>
void GlyphPathIterator<ContainerType>::readPathHeader()
{
    if (NumContours)
    {
        Pos += Data.ReadSInt15(Pos, &MoveX);
        Pos += Data.ReadSInt15(Pos, &MoveY);
        Pos += Data.ReadUInt30(Pos, &NumEdges);

        EdgePos = Pos;
        JumpToPos = true;
        if (NumEdges & 1)
        {
            // Go to the referenced contour
            EdgePos   = NumEdges >> 1;
            EdgePos  += Data.ReadUInt30(EdgePos, &NumEdges);
            JumpToPos = false;
        }
        NumEdges >>= 1;
    }
}

//------------------------------------------------------------------------
template<class ContainerType>
void GlyphPathIterator<ContainerType>::ReadEdge(int * edge)
{
    EdgePos += Data.ReadEdge(EdgePos, edge);
    switch(edge[0])
    {
    case PathDataDecoderType::Edge_HLine:
        MoveX += edge[1];
        edge[0] = PathDataDecoderType::Edge_Line;
        edge[1] = MoveX;
        edge[2] = MoveY;
        break;

    case PathDataDecoderType::Edge_VLine:
        MoveY += edge[1];
        edge[0] = PathDataDecoderType::Edge_Line;
        edge[1] = MoveX;
        edge[2] = MoveY;
        break;

    case PathDataDecoderType::Edge_Line:
        MoveX += edge[1];
        MoveY += edge[2];
        edge[1] = MoveX;
        edge[2] = MoveY;
        break;

    case PathDataDecoderType::Edge_Quad:
        MoveX += edge[1];
        MoveY += edge[2];
        edge[1] = MoveX;
        edge[2] = MoveY;
        MoveX += edge[3];
        MoveY += edge[4];
        edge[3] = MoveX;
        edge[4] = MoveY;
        break;
    }
    if (NumEdges)
        --NumEdges;

    if(NumEdges == 0 && JumpToPos)
        Pos = EdgePos;
}




//------------------------------------------------------------------------
template<class ContainerType>
class CompactedFont : public RefCountBaseNTS<CompactedFont<ContainerType>, StatMD_Fonts_Mem >
{
public:
    typedef PathDataDecoder<ContainerType>   PathDataDecoderType;
    typedef GlyphPathIterator<ContainerType> GlyphPathIteratorType;

    CompactedFont(const ContainerType& fontData) : Decoder(fontData) {}
    unsigned    AcquireFont(unsigned startPos);

    unsigned    GetNumGlyphs() const    { return NumGlyphs; }
    int         GetGlyphIndex(UInt16 code) const;
    unsigned    GetGlyphCode(unsigned glyphIndex) const;
    unsigned    GetAdvanceInt(unsigned glyphIndex) const;
    float       GetAdvance(unsigned glyphIndex) const;
    float       GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;
    float       GetGlyphWidth(unsigned glyphIndex) const;
    float       GetGlyphHeight(unsigned glyphIndex) const;
    RectF&     GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;
    void        GetGlyphShape(unsigned glyphIndex, GlyphPathIteratorType* glyph) const;

    float       GetAscent()    const    { return Ascent;  }
    float       GetDescent()   const    { return Descent; }
    float       GetLeading()   const    { return Leading; }
    
    const char* GetName()        const  { return &Name[0]; }
    unsigned    GetFontFlags()   const  { return Flags; }
    unsigned    GetNominalSize() const  { return NominalSize; }

    bool        MatchFont(const char* name, unsigned flags) const;

private:
    CompactedFont(const CompactedFont<ContainerType>&);
    const CompactedFont<ContainerType>& operator = (const CompactedFont<ContainerType>&);

    unsigned getGlyphPos(unsigned glyphIndex) const
    {
        return Decoder.ReadUInt32fixlen(GlyphInfoTablePos + 
                                        glyphIndex * (2+2+4) + 2+2);
    }

    const PathDataDecoderType   Decoder;
    unsigned                    NumGlyphs;
    unsigned                    GlyphInfoTablePos;
    unsigned                    KerningTableSize;
    unsigned                    KerningTablePos;

    ArrayUnsafePOD<char>        Name;
    unsigned                    Flags;
    unsigned                    NominalSize;
    float                       Ascent;
    float                       Descent;
    float                       Leading;
};

//------------------------------------------------------------------------
template<class ContainerType> inline
unsigned CompactedFont<ContainerType>::GetGlyphCode(unsigned glyphIndex) const
{
    unsigned pos = GlyphInfoTablePos + glyphIndex * (2+2+4);
    return Decoder.ReadUInt16fixlen(pos);
}

//------------------------------------------------------------------------
template<class ContainerType> inline
int CompactedFont<ContainerType>::GetGlyphIndex(UInt16 code) const
{
    int end = (int)NumGlyphs - 1;
    int beg = 0;
    while(beg <= end)
    {
        int      mid = (end + beg) / 2;
        unsigned pos = GlyphInfoTablePos + mid * (2+2+4);
        unsigned chr = Decoder.ReadUInt16fixlen(pos);
        if (chr == code)
        {
            return mid;
        }
        else
        {
            if (code < chr) end = mid - 1;
            else            beg = mid + 1;
        }
    }
    return -1;
}

//------------------------------------------------------------------------
template<class ContainerType> inline
unsigned CompactedFont<ContainerType>::GetAdvanceInt(unsigned glyphIndex) const
{
    unsigned pos = GlyphInfoTablePos + glyphIndex * (2+2+4) + 2;
    return Decoder.ReadSInt16fixlen(pos);
}

//------------------------------------------------------------------------
template<class ContainerType> inline
float CompactedFont<ContainerType>::GetAdvance(unsigned glyphIndex) const
{
    return (float)GetAdvanceInt(glyphIndex);
}


//------------------------------------------------------------------------
template<class ContainerType> inline
float CompactedFont<ContainerType>::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    int end = (int)KerningTableSize - 1;
    int beg = 0;
    while(beg <= end)
    {
        int      mid   = (end + beg) / 2;
        unsigned pos   = KerningTablePos + mid * (2+2+2);
        unsigned char1 = Decoder.ReadUInt16fixlen(pos);
        unsigned char2 = Decoder.ReadUInt16fixlen(pos + 2);
        if (char1 == lastCode && char2 == thisCode)
        {
            return float(Decoder.ReadSInt16fixlen(pos + 4));
        }
        else
        {
            bool pairLess = (lastCode != char1) ? (lastCode < char1) : (thisCode < char2);
            if (pairLess) end = mid - 1;
            else          beg = mid + 1;
        }
    }
    return 0;
}

//------------------------------------------------------------------------
template<class ContainerType> inline
float CompactedFont<ContainerType>::GetGlyphWidth(unsigned glyphIndex) const
{
    GlyphPathIteratorType it(Decoder.GetData());
    it.ReadBounds(getGlyphPos(glyphIndex));
    if(!it.ValidBounds()) 
        it.SetSpaceBounds(GetAdvanceInt(glyphIndex));
    return float(it.GetWidth());
}

//------------------------------------------------------------------------
template<class ContainerType> inline
float CompactedFont<ContainerType>::GetGlyphHeight(unsigned glyphIndex) const
{
    GlyphPathIteratorType it(Decoder.GetData());
    it.ReadBounds(getGlyphPos(glyphIndex));
    if(!it.ValidBounds()) 
        it.SetSpaceBounds(GetAdvanceInt(glyphIndex));
    return float(it.GetHeight());
}

//------------------------------------------------------------------------
template<class ContainerType> inline
RectF& CompactedFont<ContainerType>::GetGlyphBounds(unsigned glyphIndex, 
                                                        RectF* prect) const
{
    GlyphPathIteratorType it(Decoder.GetData());
    it.ReadBounds(getGlyphPos(glyphIndex));
    if(!it.ValidBounds()) 
        it.SetSpaceBounds(GetAdvanceInt(glyphIndex));
    return it.GetBounds(prect);
}

//------------------------------------------------------------------------
template<class ContainerType>
void CompactedFont<ContainerType>::GetGlyphShape(unsigned glyphIndex, 
                                                    GlyphPathIteratorType* glyph) const
{
    glyph->StartGlyph(getGlyphPos(glyphIndex));
}

//------------------------------------------------------------------------
template<class ContainerType>
bool CompactedFont<ContainerType>::MatchFont(const char* name, unsigned flags) const
{
    return String::CompareNoCase(&Name[0], name) == 0 && 
        (flags & Render::Font::FF_Style_Mask) == (Flags & Render::Font::FF_Style_Mask);
}

//------------------------------------------------------------------------
template<class ContainerType>
unsigned CompactedFont<ContainerType>::AcquireFont(unsigned startPos)
{
    enum { MinFontDataSize = 15 };

    if (Decoder.GetSize() < startPos+MinFontDataSize)
        return 0;

    unsigned pos = startPos;
    unsigned i;
    // Read font name
    //--------------------
    i = 0;
    while(Decoder.ReadChar(pos + i)) 
        ++i;

    Name.Resize(i + 1);
    for(i = 0; i < Name.GetSize(); ++i)
        Name[i] = Decoder.ReadChar(pos + i);

    pos += (unsigned)Name.GetSize();
    //---------------------

    // Read attributes and sizes;
    Flags           =        Decoder.ReadUInt16fixlen(pos); pos += 2;
    NominalSize     =        Decoder.ReadUInt16fixlen(pos); pos += 2;
    Ascent          = (float)Decoder.ReadSInt16fixlen(pos); pos += 2;
    Descent         = (float)Decoder.ReadSInt16fixlen(pos); pos += 2;
    Leading         = (float)Decoder.ReadSInt16fixlen(pos); pos += 2;
    NumGlyphs       =        Decoder.ReadUInt32fixlen(pos); pos += 4;
    unsigned glyphBytes =    Decoder.ReadUInt32fixlen(pos); pos += 4;

    // Navigate to glyph info table (code, advanceX, globalOffset)
    pos += glyphBytes;
    GlyphInfoTablePos = pos;

    // Navigate to kerning table
    pos += NumGlyphs * (2+2+4);
    pos += Decoder.ReadUInt30(pos, &KerningTableSize);
    KerningTablePos = pos;

    // Navigate to next font
    pos += KerningTableSize * (2+2+2);

    return pos - startPos;
}

#ifdef GFX_ENABLE_FONT_COMPACTOR

//------------------------------------------------------------------------
class FontCompactor : public RefCountBaseNTS<FontCompactor, StatMD_Fonts_Mem>
{
public:
    enum { SID = StatMD_Fonts_Mem };
    typedef ArrayPagedLH_POD<UByte, 12, 256, SID>  ContainerType;
    typedef PathDataEncoder<ContainerType>       PathDataEncoderType;
    typedef PathDataDecoder<ContainerType>       PathDataDecoderType;
    typedef GlyphPathIterator<ContainerType>     GlyphPathIteratorType;
    typedef CompactedFont<ContainerType>         CompactedFontType;

    // Create the container as:
    // FontCompactor::ContainerType container(GfxFontCompactor::NumBlocksInc);

     FontCompactor(ContainerType& data);
    ~FontCompactor();

    void Clear();

    // Font creation and packing
    //-----------------------
    void StartFont(const char* name, unsigned flags, unsigned nominalSize, 
                   int ascent, int descent, int leading);

    void StartGlyph();
    void MoveTo(SInt16 x, SInt16 y);
    void LineTo(SInt16 x, SInt16 y);
    void QuadTo(SInt16 cx, SInt16 cy, SInt16 ax, SInt16 ay);
    void EndGlyph(bool mergeContours);
    void EndGlyph(unsigned glyphCode, int advanceX, bool mergeContours);

    void AssignGlyphInfo(unsigned glyphIndex, unsigned glyphCode, int advanceX);
    void AssignGlyphCode(unsigned glyphIndex, unsigned glyphCode);
    void AssignGlyphAdvance(unsigned glyphIndex, int advanceX);

    void AddKerningPair(unsigned char1, unsigned char2, int adjustment);

    void UpdateFlags(unsigned flags);
    void UpdateMetrics(int ascent, int descent, int leading);
    void EndFont();
    //-----------------------

    // Serialization
    //-----------------------
    UPInt GetDataSize() const { return Encoder.GetSize(); }
    void  Serialize(void* ptr, unsigned start, unsigned size) const
    {
        Encoder.Serialize(ptr, start, size);
    }

    UInt32 ComputePathHash(unsigned pos) const;
    bool   PathsEqual(unsigned pos, const FontCompactor& cmpPath, unsigned cmpPos) const;

    UInt32 ComputeGlyphHash(unsigned pos) const;
    bool   GlyphsEqual(unsigned pos, const FontCompactor& cmpFont, unsigned cmpPos) const;

private:
    FontCompactor(const FontCompactor&);
    const FontCompactor& operator = (FontCompactor&);

    struct VertexType
    {
        SInt16 x,y;
    };

    struct ContourType
    {
        unsigned DataStart;
        unsigned DataSize;
    };

    struct GlyphInfoType
    {
        UInt16   GlyphCode;
        SInt16   AdvanceX;
        unsigned GlobalOffset;
    };

    struct ContourKeyType
    {
        const FontCompactor* pCoord;
        UInt32               HashValue;
        unsigned             DataStart;

        ContourKeyType() : pCoord(0), HashValue(0), DataStart(0) {}
        ContourKeyType(const FontCompactor* coord, UInt32 hash, unsigned start) :
            pCoord(coord), HashValue(hash), DataStart(start) {}

        UPInt operator()(const ContourKeyType& data) const
        {
            return (UPInt)data.HashValue;
        }

        bool operator== (const ContourKeyType& cmpWith) const 
        {
            return pCoord->PathsEqual(DataStart, *cmpWith.pCoord, cmpWith.DataStart);
        }
    };

    struct GlyphKeyType
    {
        const FontCompactor* pFont;
        UInt32                  HashValue;
        unsigned                DataStart;

        GlyphKeyType() : pFont(0), HashValue(0), DataStart(0) {}
        GlyphKeyType(const FontCompactor* font, UInt32 hash, unsigned start) :
            pFont(font), HashValue(hash), DataStart(start) {}

        UPInt operator()(const GlyphKeyType& data) const
        {
            return (UPInt)data.HashValue;
        }

        bool operator== (const GlyphKeyType& cmpWith) const 
        {
            return pFont->GlyphsEqual(DataStart, *cmpWith.pFont, cmpWith.DataStart);
        }
    };

    struct KerningPairType
    {
        UInt16 Char1, Char2;
        int    Adjustment;
    };

    static bool cmpGlyphCodes(const GlyphInfoType& a, const GlyphInfoType& b)
    {
        return a.GlyphCode < b.GlyphCode;
    }

    static bool cmpKerningPairs(const KerningPairType& a, const KerningPairType& b)
    {
        if (a.Char1 != b.Char1) return a.Char1 < b.Char1;
        return a.Char2 < b.Char2;
    }

    void        normalizeLastContour();
    static void extendBounds(int * x1, int* y1, int* x2, int* y2, int x, int y);
    void        computeBounds(int * x1, int* y1, int* x2, int* y2) const;
    unsigned    navigateToEndGlyph(unsigned pos) const;


    // NOTE; MD_Fonts_Mem is not fully correct here
    typedef AllocatorGH<ContourKeyType, SID>                                                   ContainerGlobalAllocator;
    typedef HashSet<ContourKeyType, ContourKeyType, ContourKeyType, ContainerGlobalAllocator>  ContourHashType;
    typedef HashSet<GlyphKeyType,   GlyphKeyType,   GlyphKeyType, ContainerGlobalAllocator>    GlyphHashType;

    PathDataEncoderType                             Encoder;
    PathDataDecoderType                             Decoder;
    ContourHashType                                 ContourHash;
    GlyphHashType                                   GlyphHash;
    ArrayPagedPOD<VertexType, 6, 64, SID>           TmpVertices;
    ArrayPagedPOD<ContourType, 6, 64, SID>          TmpContours;
    ArrayPagedPOD<VertexType, 6, 64, SID>           TmpContour;
 
    HashSet<UInt16>                                 GlyphCodes;
    ArrayPagedPOD<GlyphInfoType, 6, 64, SID>        GlyphInfoTable;
    ArrayPagedPOD<KerningPairType, 6, 64, SID>      KerningTable;

    unsigned                                        FontMetricsPos;
    unsigned                                        FontNumGlyphs;
    unsigned                                        FontTotalGlyphBytes;
    unsigned                                        FontStartGlyphs;
};

#endif // GFX_ENABLE_FONT_COMPACTOR
#endif // defined(GFX_ENABLE_COMPACTED_FONTS) || defined(GFX_ENABLE_FONT_COMPACTOR)

}} // Scaleform::GFx

#endif // INC_SF_GFxFontCompactor_H


