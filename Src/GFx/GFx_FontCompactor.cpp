/**************************************************************************

Filename    :   GFx_FontCompactor.cpp
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Notes       :   Compact font data storage

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Render/Render_Math2D.h"
#include "Kernel/SF_Alg.h"
#include "GFx/GFx_FontCompactor.h"

namespace Scaleform { namespace GFx {

#ifdef GFX_ENABLE_FONT_COMPACTOR

//------------------------------------------------------------------------
const int GFxFontCompactor_CollinearCurveEpsilon = 5;

//------------------------------------------------------------------------
FontCompactor::FontCompactor(ContainerType& data) : 
    Encoder(data),
    Decoder(data)
{
}

//------------------------------------------------------------------------
FontCompactor::~FontCompactor()
{
}

//------------------------------------------------------------------------
void FontCompactor::Clear()
{
    Encoder.Clear();
    TmpVertices.Clear();
    TmpContours.Clear();
    GlyphCodes.Clear();
    GlyphInfoTable.Clear();
    KerningTable.Clear();
}

//------------------------------------------------------------------------
void FontCompactor::normalizeLastContour()
{
    ContourType& c = TmpContours.Back();
    VertexType v2 = TmpVertices.Back();
    VertexType v1;
    if ((v2.x & 1) == 0)
    {
        v1 = TmpVertices[c.DataStart];
        if (v1.x == v2.x && v1.y == v2.y)
        {
            c.DataSize--;
            TmpVertices.PopBack();
        }
    }
    if (c.DataSize < 3)
    {
        TmpVertices.CutAt(c.DataStart);
        TmpContours.PopBack();
        return;
    }

    unsigned start = 0;
    int      minX = TmpVertices[c.DataStart].x >> 1;
    int      minY = TmpVertices[c.DataStart].y;
    unsigned i;
    for (i = 1; i < c.DataSize; i++)
    {
        v1 = TmpVertices[c.DataStart + i];
        if (v1.x & 1)
        {
            v1 = TmpVertices[++i + c.DataStart];
        }
        else
        {
            if (v1.y < minY)
            {
                minY = v1.y;
                start = i;
            }
            else
            if (v1.y == minY && (v1.x >> 1) < minX)
            {
                minX = v1.x;
                start = i;
            }
        }
    }

    if (start == 0)
        return;

    TmpContour.Clear();

    v1 = TmpVertices[c.DataStart + start];
    v1.x &= ~1;
    TmpContour.PushBack(v1);
    for (i = 1; i < c.DataSize; i++)
    {
        ++start;
        v1 = TmpVertices[c.DataStart + start % c.DataSize];
        if (v1.x & 1)
        {
            ++i;
            ++start;
            v2 = TmpVertices[c.DataStart + start % c.DataSize];
            TmpContour.PushBack(v1);
            TmpContour.PushBack(v2);
        }
        else
        {
            if ((v1.x >> 1) != (TmpContour.Back().x >> 1) ||
                 v1.y       !=  TmpContour.Back().y)
            {
                TmpContour.PushBack(v1);
            }
        }
    }

    TmpVertices.CutAt(c.DataStart);
    for (i = 0; i < TmpContour.GetSize(); i++)
        TmpVertices.PushBack(TmpContour[i]);

    c.DataSize = (unsigned)TmpContour.GetSize();
}

//------------------------------------------------------------------------
inline void FontCompactor::extendBounds(int* x1, int* y1, int* x2, int* y2, 
                                        int x, int y)
{
    if (x < *x1) *x1 = x;
    if (y < *y1) *y1 = y;
    if (x > *x2) *x2 = x;
    if (y > *y2) *y2 = y;
}

//------------------------------------------------------------------------
void FontCompactor::computeBounds(int* x1, int* y1, int* x2, int* y2) const
{
    *x1 =  16383;
    *y1 =  16383;
    *x2 = -16383;
    *y2 = -16383;

    unsigned i, j;
    for (i = 0; i < TmpContours.GetSize(); ++i)
    {
        const ContourType& c = TmpContours[i];
        VertexType v1 = TmpVertices[c.DataStart];
        v1.x >>= 1;
        extendBounds(x1, y1, x2, y2, v1.x, v1.y);
        for (j = 1; j < c.DataSize; ++j)
        {
            const VertexType& v2 = TmpVertices[c.DataStart + j];
            if(v2.x & 1)
            {
                const VertexType& v3 = TmpVertices[++j + c.DataStart];

                float t, x, y;
                t = Render::Math2D::QuadCurveExtremum(float(v1.x), float(v2.x >> 1), float(v3.x >> 1));
                if (t > 0 && t < 1)
                {
                    Render::Math2D::PointOnQuadCurve(float(v1.x),      float(v1.y),
                                                  float(v2.x >> 1), float(v2.y),
                                                  float(v3.x >> 1), float(v3.y),
                                                  t, &x, &y);
                    extendBounds(x1, y1, x2, y2, int(floorf(x) + 0.5f), int(floorf(y) + 0.5f));
                }
                t = Render::Math2D::QuadCurveExtremum(float(v1.y), float(v2.y), float(v3.y));
                if (t > 0 && t < 1)
                {
                    Render::Math2D::PointOnQuadCurve(float(v1.x),      float(v1.y),
                                                  float(v2.x >> 1), float(v2.y),
                                                  float(v3.x >> 1), float(v3.y),
                                                  t, &x, &y);
                    extendBounds(x1, y1, x2, y2, int(floorf(x) + 0.5f), int(floorf(y) + 0.5f));
                }
                v1 = v3;
            }
            else
                v1 = v2;

            v1.x >>= 1;
            extendBounds(x1, y1, x2, y2, v1.x, v1.y);
        }
    }
}

//------------------------------------------------------------------------
void FontCompactor::StartFont(const char* name, unsigned flags, unsigned nominalSize, 
                              int ascent, int descent, int leading)
{
    while(*name) 
        Encoder.WriteChar(*name++);
    Encoder.WriteChar(0);
    FontMetricsPos = (unsigned)Encoder.GetSize();
    Encoder.WriteUInt16fixlen(flags);
    Encoder.WriteUInt16fixlen(nominalSize);
    Encoder.WriteSInt16fixlen(ascent);
    Encoder.WriteSInt16fixlen(descent);
    Encoder.WriteSInt16fixlen(leading);
    FontNumGlyphs       = 0;
    FontTotalGlyphBytes = 0;
    FontStartGlyphs     = (unsigned)Encoder.GetSize();
    Encoder.WriteUInt32fixlen(0); // NumGlyphs
    Encoder.WriteUInt32fixlen(0); // TotalGlyphBytes

    GlyphCodes.Clear();
    GlyphInfoTable.Clear();
    KerningTable.Clear();
}

//------------------------------------------------------------------------
void FontCompactor::StartGlyph()
{
    TmpVertices.Clear();
    TmpContours.Clear();
}

//------------------------------------------------------------------------
void FontCompactor::MoveTo(SInt16 x, SInt16 y)
{
    if (TmpContours.GetSize())
        normalizeLastContour();

    ContourType c;
    c.DataStart = (unsigned)TmpVertices.GetSize();
    c.DataSize  = 1;
    TmpContours.PushBack(c);

    VertexType v;
    v.x = x << 1;
    v.y = y;
    TmpVertices.PushBack(v);
}

//------------------------------------------------------------------------
void FontCompactor::LineTo(SInt16 x, SInt16 y)
{
    ContourType& c = TmpContours.Back();
    VertexType v;
    if (c.DataSize)
    {
        v = TmpVertices.Back(); 
        if (x == (v.x >> 1) && y == v.y)
        {
            return;
        }
    }
    v.x = x << 1;
    v.y = y;
    TmpVertices.PushBack(v);
    TmpContours.Back().DataSize++;
}

//------------------------------------------------------------------------
void FontCompactor::QuadTo(SInt16 cx, SInt16 cy, SInt16 ax, SInt16 ay)
{
    ContourType& c = TmpContours.Back();
    VertexType v;
    if (c.DataSize)
    {
        v = TmpVertices.Back(); 
        v.x >>= 1;
        int cp = int(cx - ax) * int(ay - v.y) - int(cy - ay) * int(ax - v.x);
        if (cp < 0) cp = -cp;
        if (cp <= GFxFontCompactor_CollinearCurveEpsilon)
        {
            LineTo(ax, ay);
            return;
        }
    }
    v.x = (cx << 1) | 1;
    v.y = cy;
    TmpVertices.PushBack(v);
    v.x = (ax << 1) | 1;
    v.y = ay;
    TmpVertices.PushBack(v);
    TmpContours.Back().DataSize += 2;
}

//------------------------------------------------------------------------
unsigned FontCompactor::navigateToEndGlyph(unsigned pos) const
{
    int      t1;
    unsigned numContours;
    pos += Decoder.ReadSInt15(pos, &t1);
    pos += Decoder.ReadSInt15(pos, &t1);
    pos += Decoder.ReadSInt15(pos, &t1);
    pos += Decoder.ReadSInt15(pos, &t1);
    pos += Decoder.ReadUInt15(pos, &numContours);
    while (numContours--)
    {
        unsigned numEdges;
        pos += Decoder.ReadSInt15(pos, &t1);
        pos += Decoder.ReadSInt15(pos, &t1);
        pos += Decoder.ReadUInt30(pos, &numEdges);
        if ((numEdges & 1) == 0)
        {
            numEdges >>= 1;
            while(numEdges--)
            {
                UInt8 edge[10];
                pos += Decoder.ReadRawEdge(pos, edge);
            }
        }
    }
    return pos;
}

//------------------------------------------------------------------------
UInt32 FontCompactor::ComputeGlyphHash(unsigned pos) const
{
    UInt32   h = 0;
    unsigned end = navigateToEndGlyph(pos);

    for (; pos < end; ++pos)
        h = ((h << 5) + h) ^ UInt8(Decoder.ReadChar(pos));

    return h;
}

//------------------------------------------------------------------------
bool FontCompactor::GlyphsEqual(unsigned pos, const FontCompactor& cmpFont, unsigned cmpPos) const
{
    unsigned end1 =         navigateToEndGlyph(pos);
    unsigned end2 = cmpFont.navigateToEndGlyph(cmpPos);

    if (end1 - pos != end2 - cmpPos)
        return false;

    for (; pos < end1; ++pos, ++cmpPos)
    {
        if (UInt8(Decoder.ReadChar(pos)) != UInt8(cmpFont.Decoder.ReadChar(cmpPos)))
            return false;
    }
    return true;
}

//------------------------------------------------------------------------
bool FontCompactor::PathsEqual(unsigned pos, const FontCompactor& cmpPath, unsigned cmpPos) const
{
    unsigned size1, size2;
    unsigned pos1 = pos;
    unsigned pos2 = cmpPos;
    pos1 +=         Decoder.ReadUInt30(pos1, &size1);
    pos2 += cmpPath.Decoder.ReadUInt30(pos2, &size2);

    if (size1 != size2)
        return false;

    size1 >>= 1;
    size2 >>= 1;

    UInt8 edge1[10];
    UInt8 edge2[10];
    while (size1--)
    {
        unsigned nb1 =         Decoder.ReadRawEdge(pos1, edge1);
        unsigned nb2 = cmpPath.Decoder.ReadRawEdge(pos2, edge2);

        if (nb1 != nb2)
            return false;

        if (memcmp(edge1, edge2, nb1))
            return false;

        pos1 += nb1;
        pos2 += nb2;
    }
    return true;
}

//------------------------------------------------------------------------
UInt32 FontCompactor::ComputePathHash(unsigned pos) const
{
    unsigned size;
    UInt32 h = 0;
    UInt8 edge[10];

    pos += Decoder.ReadUInt30(pos, &size);
    size >>= 1;

    while (size--)
    {
        unsigned nb = Decoder.ReadRawEdge(pos, edge);
        pos += nb;
        for (unsigned i = 0; i < nb; ++i)
            h = ((h << 5) + h) ^ (UInt32)edge[i];
    }
    return h;
}


//------------------------------------------------------------------------
void FontCompactor::EndGlyph(bool mergeContours)
{
    GlyphInfoType glyphInfo;
    glyphInfo.GlyphCode     = UInt16(FontNumGlyphs);
    glyphInfo.AdvanceX      = 0;
    glyphInfo.GlobalOffset  = (unsigned)Encoder.GetSize();

    if (TmpContours.GetSize())
        normalizeLastContour();

    int x1, y1, x2, y2;
    computeBounds(&x1, &y1, &x2, &y2);
    Encoder.WriteSInt15(x1);
    Encoder.WriteSInt15(y1);
    Encoder.WriteSInt15(x2);
    Encoder.WriteSInt15(y2);

    Encoder.WriteUInt15((unsigned)TmpContours.GetSize());

    bool newShapesAdded = false;
    if (TmpContours.GetSize())
    {
        unsigned i, j;
        int  x, y, cx, cy, ax, ay;
        for (i = 0; i < TmpContours.GetSize(); ++i)
        {
            const ContourType& c = TmpContours[i];
            const VertexType* v1;

            unsigned numEdges = 0;
            for (j = 1; j < c.DataSize; ++j)
            {
                ++numEdges;
                v1 = &TmpVertices[c.DataStart + j];
                if(v1->x & 1)
                    ++j;
            }

            v1 = &TmpVertices[c.DataStart];
            x = v1->x >> 1;
            y = v1->y;
            Encoder.WriteSInt15(x);
            Encoder.WriteSInt15(y);
            unsigned startPath = (unsigned)Encoder.GetSize();
            Encoder.WriteUInt30(numEdges << 1);

            for (j = 1; j < c.DataSize; ++j)
            {
                v1 = &TmpVertices[c.DataStart + j];
                if(v1->x & 1)
                {
                    const VertexType* v2 = &TmpVertices[++j + c.DataStart];
                    cx = v1->x >> 1;
                    cy = v1->y;
                    ax = v2->x >> 1;
                    ay = v2->y;
                    Encoder.WriteQuad(cx-x, cy-y, ax-cx, ay-cy);
                    x = ax;
                    y = ay;
                }
                else
                {
                    ax = v1->x >> 1;
                    ay = v1->y;
                    if (ax == x)
                        Encoder.WriteVLine(ay-y);
                    else
                    if (ay == y)
                        Encoder.WriteHLine(ax-x);
                    else
                        Encoder.WriteLine(ax-x, ay-y);
                    x = ax;
                    y = ay;
                }
            }
            if (mergeContours)
            {
                UInt32 hash = ComputePathHash(startPath);
                ContourKeyType key(this, hash, startPath);
                const ContourKeyType* found = ContourHash.Get(key);
                if (found)
                {
                    Encoder.CutAt(startPath);
                    Encoder.WriteUInt30((found->DataStart << 1) | 1);
                }
                else
                {
                    ContourHash.Add(key);
                    newShapesAdded = true;
                }
            }
        }
    }
    ++FontNumGlyphs;

    unsigned startGlyph = glyphInfo.GlobalOffset;
    if (mergeContours && !newShapesAdded)
    {
        UInt32 hash = ComputeGlyphHash(glyphInfo.GlobalOffset);

        GlyphKeyType key(this, hash, glyphInfo.GlobalOffset);
        const GlyphKeyType* found = GlyphHash.Get(key);
        if (found)
        {
            Encoder.CutAt(glyphInfo.GlobalOffset);
            glyphInfo.GlobalOffset = found->DataStart;
        }
        else
        {
            GlyphHash.Add(key);
        }
    }

    FontTotalGlyphBytes += (unsigned)Encoder.GetSize() - startGlyph;
    Encoder.UpdateUInt32fixlen(FontStartGlyphs,   FontNumGlyphs);
    Encoder.UpdateUInt32fixlen(FontStartGlyphs+4, FontTotalGlyphBytes);

    GlyphInfoTable.PushBack(glyphInfo);
}

//------------------------------------------------------------------------
void FontCompactor::AssignGlyphInfo(unsigned glyphIndex, unsigned glyphCode, int advanceX)
{
    if (glyphIndex < GlyphInfoTable.GetSize())
    {
        GlyphInfoType& glyphInfo = GlyphInfoTable[glyphIndex];
        glyphInfo.GlyphCode = UInt16(glyphCode);
        glyphInfo.AdvanceX  = SInt16(advanceX);
        if (GlyphCodes.Get(UInt16(glyphCode)) == 0)
            GlyphCodes.Add(UInt16(glyphCode));
    }
}

//------------------------------------------------------------------------
void FontCompactor::AssignGlyphCode(unsigned glyphIndex, unsigned glyphCode)
{
    if (glyphIndex < GlyphInfoTable.GetSize())
    {
        GlyphInfoTable[glyphIndex].GlyphCode = UInt16(glyphCode);
        if (GlyphCodes.Get(UInt16(glyphCode)) == 0)
            GlyphCodes.Add(UInt16(glyphCode));
    }
}

//------------------------------------------------------------------------
void FontCompactor::AssignGlyphAdvance(unsigned glyphIndex, int advanceX)
{
    if (glyphIndex < GlyphInfoTable.GetSize())
    {
        GlyphInfoTable[glyphIndex].AdvanceX = SInt16(advanceX);
    }
}

//------------------------------------------------------------------------
void FontCompactor::EndGlyph(unsigned glyphCode, int advanceX, bool mergeContours)
{
    EndGlyph(mergeContours);
    AssignGlyphInfo((unsigned)GlyphInfoTable.GetSize() - 1, glyphCode, advanceX);
}

//------------------------------------------------------------------------
void FontCompactor::AddKerningPair(unsigned char1, unsigned char2, int adjustment)
{
    if (GlyphCodes.Get(UInt16(char1)) && GlyphCodes.Get(UInt16(char2)))
    {
        KerningPairType kp;
        kp.Char1        = UInt16(char1);
        kp.Char2        = UInt16(char2);
        kp.Adjustment   = adjustment;
        KerningTable.PushBack(kp);
    }
}

//------------------------------------------------------------------------
void FontCompactor::UpdateFlags(unsigned flags)
{
    Encoder.UpdateSInt16fixlen(FontMetricsPos + 0, flags);
}

//------------------------------------------------------------------------
void FontCompactor::UpdateMetrics(int ascent, int descent, int leading)
{
    Encoder.UpdateSInt16fixlen(FontMetricsPos + 4, ascent);
    Encoder.UpdateSInt16fixlen(FontMetricsPos + 6, descent);
    Encoder.UpdateSInt16fixlen(FontMetricsPos + 8, leading);
}

//------------------------------------------------------------------------
void FontCompactor::EndFont()
{
    unsigned i;

    //bool ordered = true;
    //for (i = 1; i < GlyphInfoTable.GetSize(); ++i)
    //{
    //    if (GlyphInfoTable[i-1].GlyphCode > GlyphInfoTable[i].GlyphCode)
    //    {
    //        ordered = false;
    //        break;
    //    }
    //}
    //
    //if (!ordered)
    //    Alg::QuickSort(GlyphInfoTable, cmpGlyphCodes);

    for (i = 0; i < GlyphInfoTable.GetSize(); ++i)
    {
        const GlyphInfoType& glyphInfo = GlyphInfoTable[i];
        Encoder.WriteUInt16fixlen(glyphInfo.GlyphCode);
        Encoder.WriteSInt16fixlen(glyphInfo.AdvanceX);
        Encoder.WriteUInt32fixlen(glyphInfo.GlobalOffset);
    }

    Alg::QuickSort(KerningTable, cmpKerningPairs);
    Encoder.WriteUInt30((unsigned)KerningTable.GetSize());
    for (i = 0; i < KerningTable.GetSize(); ++i)
    {
        const KerningPairType& kerningPair = KerningTable[i];
        Encoder.WriteUInt16fixlen(kerningPair.Char1);
        Encoder.WriteUInt16fixlen(kerningPair.Char2);
        Encoder.WriteSInt16fixlen(kerningPair.Adjustment);
    }
}

#endif //GFX_ENABLE_FONT_COMPACTOR

}} // Scaleform::GFx

