/**************************************************************************

Filename    :   Text_LineBuffer.cpp
Content     :   Text line buffer
Created     :   May, 2007
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Kernel/SF_UTF8Util.h"

#include "Render/Render_Image.h"
#include "Render/Text/Text_LineBuffer.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { 
namespace Render {
namespace Text {

LineBuffer::Line* LineBuffer::TextLineAllocator::AllocLine(unsigned size, LineBuffer::LineType lineType)
{
    //TODO: look for already allocated line in FreeLinesPool first
    Line* pline = (Line*)SF_HEAP_AUTO_ALLOC(this, size);// Was: GFxStatRender_Text_Mem);
    pline->SetMemSize(size);
    if (lineType == Line8)
        pline->InitLine8();
    else
        pline->InitLine32();
    return pline;
}

void LineBuffer::TextLineAllocator::FreeLine(LineBuffer::Line* ptr)
{
    //TODO: return line being freed in FreeLinesPool
    if (ptr)
    {
        ptr->Release();
        SF_FREE(ptr);
    }
}

void LineBuffer::ReleasePartOfLine(GlyphEntry* pglyphs, unsigned n, FormatDataEntry* pnextFormatData)
{
    for(unsigned i = 0; i < n; ++i, ++pglyphs)
    {
        GlyphEntry* pglyph = pglyphs;
        if (pglyph->IsNextFormat())
        {   
            if (pglyph->HasFmtFont())
            {
                FontHandle* pfont = pnextFormatData->pFont;
                pfont->Release();
                ++pnextFormatData;
            }
            if (pglyph->HasFmtColor())
            {
                ++pnextFormatData;
            }
            if (pglyph->HasFmtImage())
            {
                ImageDesc *pimage = pnextFormatData->pImage;
                pimage->Release();
                ++pnextFormatData;
            }
        }
    }
}

void LineBuffer::GlyphInserter::ResetTo(const GlyphInserter& savedPos)
{
    SF_ASSERT(savedPos.GlyphIndex <= GlyphIndex);
    if (savedPos.GlyphIndex < GlyphIndex && GlyphsCount > 0)
    {
        // first of all, we need to release all fonts and image descriptors
        // beyond the savedPos.GlyphIndex; otherwise memory leak is possible.
        
        GlyphEntry* pglyphs = savedPos.pGlyphs + savedPos.GlyphIndex;
        unsigned n = GlyphIndex - savedPos.GlyphIndex;
        FormatDataEntry* pnextFormatData 
            = reinterpret_cast<FormatDataEntry*>(savedPos.pNextFormatData + savedPos.FormatDataIndex);
        ReleasePartOfLine(pglyphs, n, pnextFormatData);
    }
    operator=(savedPos);
}

void LineBuffer::GlyphIterator::operator++()
{
    if (!IsFinished())
    {
        if (Delta == 0)
            Delta = pGlyphs->GetLength();
        ++pGlyphs;
        if (pGlyphs->GetLength() > 0 && Delta > 0 && !HighlighterIter.IsFinished())
        {
            HighlighterIter += Delta;
            Delta = 0;
        }
        UpdateDesc();
    }
}

void LineBuffer::GlyphIterator::UpdateDesc()
{
    pImage = NULL;
    if (!IsFinished())
    {
        if (pGlyphs->IsNextFormat())
        {   
            if (pGlyphs->HasFmtFont())
            {
                pFontHandle = pNextFormatData->pFont;
                ++pNextFormatData;
            }
            if (pGlyphs->HasFmtColor())
            {
                OrigColor = ColorV = pNextFormatData->ColorV;
                ++pNextFormatData;
            }
            if (pGlyphs->HasFmtImage())
            {
                pImage = pNextFormatData->pImage;
                ++pNextFormatData;
            }
        }
        if (pGlyphs->IsUnderline())
        {
            UnderlineStyle  = HighlightInfo::Underline_Single;
            UnderlineColor  = ColorV;
        }
        else
            UnderlineStyle = HighlightInfo::Underline_None;

        if (!HighlighterIter.IsFinished())
        {
            // get data from the highlighter
            const HighlightDesc& hd = *HighlighterIter;
            
            ColorV           = OrigColor;
            if ((pGlyphs->GetLength() > 0 || pGlyphs->IsWordWrapSeparator()))
            {
                if (hd.Info.HasTextColor())
                    ColorV = hd.Info.GetTextColor().ToColor32();

                if (hd.Info.HasUnderlineStyle())
                    UnderlineStyle = hd.Info.GetUnderlineStyle();
                if (hd.Info.HasUnderlineColor())
                    UnderlineColor = hd.Info.GetUnderlineColor().ToColor32();
                else
                    UnderlineColor  = ColorV;
                if (hd.Info.HasBackgroundColor())
                    SelectionColor = hd.Info.GetBackgroundColor().ToColor32();
                else
                    SelectionColor = 0;
            }
        }
        else
        {
            SelectionColor = 0;
            if (pGlyphs->IsUnderline())
            {
                UnderlineColor = ColorV;
                UnderlineStyle  = HighlightInfo::Underline_Single;
            }
        }
    }
}

LineBuffer::FormatDataEntry* LineBuffer::Line::GetFormatData() const 
{ 
    UByte* p = reinterpret_cast<UByte*>(GetGlyphs() + GetNumGlyphs());
    // align to boundary of FormatDataEntry
    UPInt pi = ((UPInt)p);
    UPInt delta = ((pi + sizeof(FormatDataEntry*) - 1) & ~(sizeof(FormatDataEntry*) - 1)) - pi;
    return reinterpret_cast<FormatDataEntry*>(p + delta);
}

void LineBuffer::Line::Release()
{
    if (!IsInitialized()) return;

    GlyphEntry* pglyphs = GetGlyphs();
    unsigned n = GetNumGlyphs();
    FormatDataEntry* pnextFormatData = GetFormatData();
    ReleasePartOfLine(pglyphs, n, pnextFormatData);
    SetNumGlyphs(0);
#ifdef SF_BUILD_DEBUG
    memset(((UByte*)this) + sizeof(MemSize), 0xf0, GetMemSize() - sizeof(MemSize));
#endif
}

bool LineBuffer::Line::HasNewLine() const
{
    unsigned n = GetNumGlyphs();
    if (n > 0)
    {
        GlyphEntry* pglyphs = GetGlyphs();
        return pglyphs[n - 1].IsNewLineChar() && !pglyphs[n - 1].IsEOFChar();
    }
    return false;
}

//////////////////////////////////
// LineBuffer
//
LineBuffer::LineBuffer()
{
    Geom.VisibleRect.Clear();
    LastHScrollOffset = ~0u;
}

LineBuffer::~LineBuffer()
{
    ClearLines();
}

LineBuffer::Line* LineBuffer::GetLine(unsigned lineIdx)
{
    if (lineIdx >= Lines.GetSize())
        return NULL;
    LineBuffer::Line* pline = Lines[lineIdx];
    InvalidateCache();
    return pline;
}

const LineBuffer::Line* LineBuffer::GetLine(unsigned lineIdx) const
{
    if (lineIdx >= Lines.GetSize())
        return NULL;
    const LineBuffer::Line* pline = Lines[lineIdx];
    return pline;
}

unsigned LineBuffer::CalcLineSize
    (unsigned glyphCount, unsigned formatDataElementsCount, LineBuffer::LineType lineType)
{
    unsigned sz;
    if (lineType == Line8)
        sz = LineData8::GetLineStructSize();
    else
        sz = LineData32::GetLineStructSize();
    sz += sizeof(UInt32); // MemSize
    sz += sizeof(GlyphEntry) * glyphCount;
    // add alignment here
    sz = (sz + sizeof(FormatDataEntry) - 1) & ~(sizeof(FormatDataEntry) - 1);
    sz += sizeof(FormatDataEntry) * formatDataElementsCount;
    return sz;
}


LineBuffer::Line* LineBuffer::InsertNewLine
    (unsigned lineIdx, unsigned glyphCount, unsigned formatDataElementsCount, LineBuffer::LineType lineType)
{
    unsigned sz = CalcLineSize(glyphCount, formatDataElementsCount, lineType);
    Line* pline = LineAllocator.AllocLine(sz, lineType);
    if (!pline) return NULL;

    pline->SetNumGlyphs(glyphCount);
    Lines.InsertAt(lineIdx, pline);
    return pline;
}

void LineBuffer::RemoveLines(unsigned lineIdx, unsigned num)
{
    Iterator it = Begin() + lineIdx;
    for(unsigned i = 0; i < num && !it.IsFinished(); ++it, ++i)
    {
        LineAllocator.FreeLine(it.GetPtr());
    }
    Lines.RemoveMultipleAt(lineIdx, num);
}

unsigned   LineBuffer::GetVScrollOffsetInFixp() const
{
    int yOffset = 0;
    if (Geom.FirstVisibleLinePos != 0)
    {
        // calculate y-offset of first visible line
        ConstIterator visIt   = BeginVisible(0);
        ConstIterator linesIt = Begin();
        if (!visIt.IsFinished() && !linesIt.IsFinished())
        {
            yOffset = visIt->GetOffsetY() - linesIt->GetOffsetY();
            SF_ASSERT(yOffset >= 0);
        }
    }
    return unsigned(yOffset);
}

void LineBuffer::ResetCache() 
{ 
}


template<class Matrix>
static float GFx_CalcHeightRatio(const Matrix& matrix)
{
    Matrix mat2(matrix);
    mat2.x(0);
    mat2.y(0);
    Render::PointF p01 = mat2.Transform(Render::PointF(0,1));
    Render::PointF p10 = mat2.Transform(Render::PointF(1,0));
    return fabsf(Render::Math2D::LinePointDistance(0, 0, p10.x, p10.y, p01.x, p01.y));
}

void    LineBuffer::SetFirstVisibleLine(unsigned line)
{
    Geom.FirstVisibleLinePos = line;
    InvalidateCache();
}

void    LineBuffer::SetHScrollOffset(unsigned offset)
{
    Geom.HScrollOffset = offset;
}

LineBuffer::Iterator LineBuffer::FindLineByTextPos(UPInt textPos)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, (unsigned)textPos, LineIndexComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        unsigned lineTextPos = pline->GetTextPos();
        if (textPos >= lineTextPos && textPos <= lineTextPos + pline->GetTextLength())
            return Iterator(*this, (unsigned)i);
    }
    return Iterator();
}

LineBuffer::Iterator LineBuffer::FindLineAtYOffset(float yoff)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, yoff, LineYOffsetComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        if (yoff >= pline->GetOffsetY() && yoff < pline->GetOffsetY() + pline->GetHeight() + pline->GetLeading())
        {
            return Iterator(*this, (unsigned)i);
        }
    }
    return Iterator();
}

LineBuffer::Iterator LineBuffer::FindLineAtOffset(const Render::PointF& p)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, p.y, LineYOffsetComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        int   yoff = pline->GetOffsetY();
        while (pline)
        {
            if ( (p.y >= pline->GetOffsetY() && p.y < pline->GetOffsetY() + pline->GetHeight() + pline->GetLeading()) )
            {
                if (p.x >= pline->GetOffsetX() && p.x < pline->GetOffsetX() + pline->GetWidth())
                {
                    return Iterator(*this, (unsigned)i);
                }
                if ( (++i) == Lines.GetSize() ) break;
                pline = Lines[(unsigned)i];
                if (pline->GetOffsetY() != yoff)
                    break;
            }
            else
                break;
        }
    }
    return Iterator();
}

bool LineBuffer::IsLineVisible(unsigned lineIndex, float yOffset) const
{
    const Line& line = *Lines[lineIndex];
    if (lineIndex == Geom.FirstVisibleLinePos)
    {
        // a special case for the first VISIBLE line: display it even if it is only
        // partially visible
        return (line.GetOffsetY() + yOffset <= Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2); 
    }
    // subtract leading of the line before checking its complete visibility
    return lineIndex > Geom.FirstVisibleLinePos  && 
        (line.GetOffsetY() + line.GetHeight() + yOffset <= Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2); 
}

bool LineBuffer::IsPartiallyVisible(float yOffset) const
{
    if (Geom.FirstVisibleLinePos < GetSize())
    {
        const Line& line = *Lines[Geom.FirstVisibleLinePos];
        if (line.GetWidth() != 0 && line.GetHeight() != 0)
        {
            // XBox360 VC9 compiler had a problem compiling original complex condition
            // (optimizer generated incorrect code). So, have splitted it by more
            // simple parts.
			float lh     = (float)line.GetHeight();
			float vrectH = Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2;
			float yf     = (float)line.GetOffsetY() + yOffset;
            if (yf <= vrectH && (yf + lh) > vrectH)
			{
// 				bool b1 = (yf <= vrectH);
// 				bool b2 = ((yf + lh) > vrectH);
// 				printf("Mask: %f > %f,  %d && %d, yoff %f, ly  %f, lhei %f, visrHei %f\n", 
// 					float(yf + line.GetHeight()), float(vrectH), 
// 					(int)b1, (int)b2,
// 					float(yOffset), float(yf), float(lh), float(Geom.VisibleRect.Height()));
                return true;
			}
        }
    }
    return false;
}

void LineBuffer::Scale(float scaleFactor)
{
    Iterator it = Begin();
    for (;!it.IsFinished(); ++it)
    {
        Line& line = *it;
        SF_ASSERT(line.IsData32());

        float newLeading    = float(line.GetLeading())*scaleFactor;
        float newW          = float(line.GetWidth())*scaleFactor;
        float newH          = float(line.GetHeight())*scaleFactor;
        line.SetLeading(int(newLeading));
        line.SetDimensions(int(newW), int(newH));
        line.SetBaseLineOffset(line.GetBaseLineOffset()*scaleFactor);
        line.SetOffsetX(int(float(line.GetOffsetX())*scaleFactor));
        line.SetOffsetY(int(float(line.GetOffsetY())*scaleFactor));

        GlyphIterator git = line.Begin();
        for(; !git.IsFinished(); ++git)
        {
            GlyphEntry& ge = git.GetGlyph();
            float newAdv = float(ge.GetAdvance())*scaleFactor;
            ge.SetAdvance(int(newAdv));
            ge.SetFontSize(ge.GetFontSize()*scaleFactor);
        }
    }
    InvalidateCache();
}

int LineBuffer::GetMinLineHeight() const
{
    if (GetSize() == 0)
        return 0;
    ConstIterator it = Begin();
    int minH = SF_MAX_SINT;
    for (;!it.IsFinished(); ++it)
    {
        const Line& line = *it;
        int h = line.GetHeight();
        if (h < minH)
            minH = h;
    }
    return minH;
}

#ifdef SF_BUILD_DEBUG
void LineBuffer::Dump() const
{
    printf("Dumping lines...\n");
    printf("VisibleRect: { %f, %f, {%f, %f}}\n", 
        Geom.VisibleRect.x1, Geom.VisibleRect.y1, Geom.VisibleRect.Width(), Geom.VisibleRect.Height());
    ConstIterator linesIt = Begin();
    unsigned i = 0;
    for(; !linesIt.IsFinished(); ++linesIt)
    {
        const Line& line = *linesIt;
        printf("Line[%d]\n", i++);
        printf("   TextPos = %d, NumGlyphs = %d, Len = %d\n", line.GetTextPos(), line.GetNumGlyphs(), line.GetTextLength());
        printf("   BaseLine = %f, Leading = %d\n", line.GetBaseLineOffset(), line.GetLeading());
        printf("   Bounds = { %d, %d, {%d, %d}}\n", line.GetOffsetX(), line.GetOffsetY(), line.GetWidth(), line.GetHeight());
    }
    printf("...end\n\n");
}

void LineBuffer::CheckIntegrity() const
{
    ConstIterator linesIt = Begin();
    unsigned nextpos = 0;
    for(; !linesIt.IsFinished(); ++linesIt)
    {
        const Line& line = *linesIt;
        unsigned l = 0;
        GlyphIterator git = const_cast<Line&>(line).Begin();
        for (; !git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& glyph = git.GetGlyph();
            l += glyph.GetLength();
        }
        if (!(line.GetTextPos() == nextpos || line.GetTextPos() == nextpos + 1) )
        {
            Dump();
            SF_ASSERT(0);
        }
        nextpos = line.GetTextPos() + l;
    }
}
#endif

void RecalculateRectToFit16Bit(Matrix2F& matrix, const RectF& srcRect, RectF* pdestRect)
{
    matrix.PrependTranslation(srcRect.x1, srcRect.y1);

    float xscale = 1, width = srcRect.Width();
    if (width > 32767)
    {
        xscale = width / 32767;
        width = 32767;
    }
    float yscale = 1, height = srcRect.Height();
    if (height > 32767)
    {
        yscale = height / 32767;
        height = 32767;
    }
    matrix.PrependScaling(xscale, yscale);
    SF_ASSERT(pdestRect);
    pdestRect->y1 = pdestRect->x1 = 0;
    pdestRect->SetWidth(width);
    pdestRect->SetHeight(height);
}

static TextUnderlineStyle ConvertUnderlineStyle(HighlightInfo::UnderlineStyle st)
{
    switch(st)
    {
    case HighlightInfo::Underline_Single:           return TextUnderline_Single; break;
    case HighlightInfo::Underline_Thick:            return TextUnderline_Thick; break;
    case HighlightInfo::Underline_Dotted:           return TextUnderline_Dotted; break;
    case HighlightInfo::Underline_DottedThick:      return TextUnderline_DottedThick; break;
    case HighlightInfo::Underline_DitheredSingle:   return TextUnderline_Dithered; break;
    case HighlightInfo::Underline_DitheredThick:    return TextUnderline_DitheredThick; break;
    default:;
    }
    SF_ASSERT(0); // shouldn't reach here!
    return TextUnderline_Single;
}

void LineBuffer::CreateVisibleTextLayout(TextLayout::Builder& bld, 
                                         const Highlighter* phighlighter, 
                                         const TextFieldParam& textFieldParam)
{
    float yOffset = -float(GetVScrollOffsetInFixp());

    Iterator linesIt = BeginVisible(yOffset);
    RectF glyphBounds;
    bool hasUnderline = false;
    if (phighlighter)
    {
        hasUnderline = phighlighter->HasUnderlineHighlight();
    }
    Color prevColor(0u);
    Font* prevFont = NULL;
    float prevFontSize = 0;
    bool  maskApplied;

    if (!IsStaticText() && !Geom.IsNoClipping() && IsPartiallyVisible(yOffset))
    {
        bld.SetClipBox(Geom.VisibleRect);
        maskApplied = true;
    }
    else
        maskApplied = false;
    for(; linesIt.IsVisible(); ++linesIt)
    {
        Line& line = *linesIt;
        Render::PointF offset;
        offset.x = (float)line.GetOffsetX();
        offset.y = (float)line.GetOffsetY();
        offset += Geom.VisibleRect.TopLeft();
        offset.y += yOffset + line.GetBaseLineOffset();
        GlyphIterator glyphIt = line.Begin(phighlighter);
        bool firstVisible = true;
       
        Color prevSelectionColor(0u);
        RectF selectionRect(0,0,0,0);

        HighlightInfo::UnderlineStyle prevUnderlineStyle = HighlightInfo::Underline_None;
        Color                         prevUnderlineColor(0u);
        
        int     underlineLen = 0;
        PointF  underlineBeginPt(0,0);
        float descent = float(line.GetHeight() - line.GetBaseLineOffset());

        for (int advance = 0; !glyphIt.IsFinished(); ++glyphIt, offset.x += advance)
        {
            const GlyphEntry& glyph = glyphIt.GetGlyph();

            advance = glyph.GetAdvance();
//             if ((glyph.IsCharInvisible() && !glyphIt.IsUnderline() && !glyphIt.HasImage()))
//                 continue;

            float       scale       = 1.0f;
            float       approxSymW  = 0;
            unsigned    index       = glyph.GetIndex();
            ImageDesc*  pimage      = NULL;
            Font*       pfont       = NULL;
            float       fontSize    = -1;
            Color       selectionColor = glyphIt.GetSelectionColor();

            if (glyphIt.HasImage())
            {
                pimage = glyphIt.GetImage();
                approxSymW = pimage->GetScreenWidth();
            }
            else
            {
                fontSize = (float)PixelsToFixp(glyph.GetFontSize());
                scale = fontSize / 1024.0f; // the EM square is 1024 x 1024   

                pfont = glyphIt.GetFont();
                SF_ASSERT(pfont);

                if (!IsStaticText()) // no mask or partial visibility test for static text
                {
                    if (index == ~0u)
                        approxSymW = float(advance) * scale;
                    else
                        approxSymW = pfont->GetGlyphBounds(index, &glyphBounds).x2 * scale;
                }
                else
                {
                    // For static text GetGlyphBounds is expensive, and since we do not 
                    // need it, just don't get it for static text.
                    approxSymW = 0;
                }
            }
            float adjox = offset.x - Geom.HScrollOffset;

            // check for invisibility/partial visibility of the glyph in order to determine
            // necessity of setting mask.
            if (!IsStaticText()) // no mask or partial visibility test for static text
            {
                // test for complete invisibility, left side
                if (/*index != ~0u &&*/ adjox + approxSymW <= Geom.VisibleRect.x1)
                    continue;

                // test for complete invisibility, right side
                if (int(adjox) >= int(Geom.VisibleRect.x2))
                    break; // we can finish here
            }
            hasUnderline = glyphIt.IsUnderline();

            if (firstVisible)
            {
                bld.SetNewLine(adjox, offset.y);
                firstVisible = false;
            }

            // test for partial visibility
            if (!glyph.IsCharInvisible() && ( // do not check if cur char is invisible
                !maskApplied && !IsStaticText() && !Geom.IsNoClipping() &&
                ((adjox < Geom.VisibleRect.x1 && adjox + approxSymW > Geom.VisibleRect.x1) || 
                (adjox < Geom.VisibleRect.x2 && adjox + approxSymW > Geom.VisibleRect.x2))))
            {
                RectF r(Geom.VisibleRect);
                // seems like we can expand the clipbox vertically to avoid clipping of
                // tall characters (common in Aisan languages). !AB
                r.Expand(0, GFX_TEXT_GUTTER);
                bld.SetClipBox(r);
                maskApplied = true;
            }

            if (pimage)
            {
                if (pimage->pImage)
                {
                    float yscale = pimage->GetYScale();
                    bld.AddImage(pimage->pImage, pimage->GetXScale(), yscale, 
                                 pimage->BaseLineY, float(advance));
                }
                else
                    LogDebugMessage(Log_Warning, "An image in TextLayout is NULL");
                continue;
            }

            Color color(glyphIt.GetColor());
            if (color != prevColor)
            {
                bld.ChangeColor(color.ToColor32());
            }
            SF_ASSERT(fontSize > 0);
            if (pfont != prevFont || fontSize != prevFontSize)
            {
                bld.ChangeFont(pfont, fontSize);

                // just need to hold FontHandle inside the layout to avoid releasing memory heap
                // while the layout uses the font. FontHandle may store a strong pointer to font
                // container.
                FontHandle* fh = glyphIt.GetFontHandle();
                bld.AddRefCntData(fh);
            }

            // handle underline
            if (!glyph.IsNewLineChar())
            {
                if (prevUnderlineColor != glyphIt.GetUnderlineColor() || prevUnderlineStyle != glyphIt.GetUnderlineStyle())
                {
                    if (prevUnderlineColor != 0)
                    {
                        bld.AddUnderline(underlineBeginPt.x - float(Geom.HScrollOffset), 
                            underlineBeginPt.y + descent/2, float(underlineLen), 
                            ConvertUnderlineStyle(prevUnderlineStyle), prevUnderlineColor.ToColor32());
                    }
                    // new underline, no underline before
                    underlineBeginPt = offset;
                    underlineLen     = advance;
                }
                else if (glyphIt.GetUnderlineColor() != 0)
                {
                    // extend
                    underlineLen += advance;
                }
            }
            else
            {
                if (prevUnderlineColor != 0)
                {
                    bld.AddUnderline(underlineBeginPt.x - float(Geom.HScrollOffset), 
                        underlineBeginPt.y + descent/2, float(underlineLen), 
                        ConvertUnderlineStyle(prevUnderlineStyle), prevUnderlineColor.ToColor32());
                }
                underlineLen = 0;
                hasUnderline = false;
            }

            // handle selection: accumulate or insert selection rect
            if (glyphIt.IsSelected() || prevSelectionColor != 0)
            {
                RectF glyphRect(0, 0,  float(advance), float(line.GetHeight() + line.GetNonNegLeading()));
                PointF rectOff(offset.x, offset.y - float(line.GetAscent()));
                glyphRect += rectOff;
                if (selectionColor != prevSelectionColor)
                {
                    if (prevSelectionColor != 0)
                    {
                        // previous selection existed, need to finish it first
                        RectF sr(selectionRect);
                        sr -= PointF(float(Geom.HScrollOffset), 0); // adj by HScrollOffset
                        bld.AddSelection(sr, prevSelectionColor.ToColor32());
                    }
                    selectionRect = glyphRect;
                }
                else if (selectionColor != 0)
                {
                    // just extend selection rect, color is the same
                    selectionRect.Union(glyphRect);
                }
            }

            bld.AddChar(index, float(advance), glyph.IsCharInvisible(), glyphIt.IsFauxBold(), glyphIt.IsFauxItalic());
            prevColor           = color;
            prevFont            = pfont;
            prevFontSize        = fontSize;
            prevSelectionColor  = selectionColor;
            if (hasUnderline)
            {
                prevUnderlineColor  = glyphIt.GetUnderlineColor();
                prevUnderlineStyle  = glyphIt.GetUnderlineStyle();
            }
            else
            {
                prevUnderlineColor = Color(0u);
                prevUnderlineStyle = HighlightInfo::Underline_None;
            }
        }
        if (prevUnderlineColor != 0)
        {
            // finalize underline
            bld.AddUnderline(underlineBeginPt.x - float(Geom.HScrollOffset), underlineBeginPt.y + descent/2, 
                float(underlineLen), ConvertUnderlineStyle(prevUnderlineStyle), prevUnderlineColor.ToColor32());
        }
        if (prevSelectionColor != 0)
        {
            // finalize selection
            selectionRect -= PointF(float(Geom.HScrollOffset), 0); // adj by HScrollOffset
            bld.AddSelection(selectionRect, prevSelectionColor.ToColor32());
        }
    }
    bld.SetParam(textFieldParam);
}

FontHandle* LineBuffer::FindFirstFontInfo() const
{
    for (UPInt l = 0, nl = Lines.GetSize(); l < nl; ++l)
    {
        const Line* line = Lines[l];
        LineBuffer::GlyphIterator git = line->Begin();
        for (;!git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& ge = git.GetGlyph();
            if (ge.HasFmtFont())
            {
                return git.GetFontHandle();
            }
        }
    }
    return NULL;
}

void LoadTextFieldParamFromTextFilter(TextFieldParam& params, const TextFilter& filter)
{
    ///////////////////////////////
    params.TextParam.SetBlurX(filter.BlurX);
    params.TextParam.SetBlurY(filter.BlurY);
    params.TextParam.Flags = GlyphParam::FineBlur;
    params.TextParam.SetBlurStrength(filter.BlurStrength);

    if ((filter.ShadowFlags & TextFieldParam::ShadowDisabled) == 0)
    {
        params.ShadowParam.Flags = UInt16(filter.ShadowFlags & ~TextFieldParam::ShadowDisabled);
        params.ShadowParam.SetBlurX(filter.ShadowParams.BlurX);
        params.ShadowParam.SetBlurY(filter.ShadowParams.BlurX);
        params.ShadowParam.SetBlurStrength(filter.ShadowParams.Strength);
        params.ShadowColor   = filter.ShadowParams.Colors[0].ToColor32();
        params.ShadowOffsetX = filter.ShadowParams.Offset.x;
        params.ShadowOffsetY = filter.ShadowParams.Offset.y;

    }
}

}}} // Scaleform::Render::Text

