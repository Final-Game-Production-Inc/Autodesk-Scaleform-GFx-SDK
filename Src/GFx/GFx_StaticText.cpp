/**************************************************************************

Filename    :   GFx_StaticText.cpp
Content     :   Static text field character implementation
Created     :   May, 2007
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#include "Kernel/SF_UTF8Util.h"

#include "GFx/GFx_Shape.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_StaticText.h"
#include "Render/Render_TreeLog.h"
#include "Render/Render_TreeText.h"


// For Root()->GetRenderer
#include "GFx/GFx_PlayerImpl.h"
//#include "GFxSprite.h"

//#include "GFx/GFx_DisplayContext.h"
#include "GFx/GFx_LoadProcess.h"

#include "GFx/GFx_StaticText.h"
#include "Render/Render_Math2D.h"

namespace Scaleform {
namespace GFx {

//
// TextCharacter
//

//////////////////////////////////////////////////////////////////////////
// StaticTextRecord
//
StaticTextRecord::StaticTextRecord() :
    Offset(0.0f),       
    TextHeight(1.0f),
    FontId(0)
{
}

void    StaticTextRecord::Read(Stream* in, int glyphCount, int glyphBits, int advanceBits)
{
    Glyphs.Resize(glyphCount);
    for (int i = 0; i < glyphCount; i++)
    {
        Glyphs[i].GlyphIndex = in->ReadUInt(glyphBits);
        Glyphs[i].GlyphAdvance = (float) in->ReadSInt(advanceBits);
    }
}

// Obtains cumulative advance value.
float   StaticTextRecord::GetCumulativeAdvance() const
{
    float advance = 0;
    for (UPInt i = 0, glyphCount = Glyphs.GetSize(); i < glyphCount; i++)
        advance += Glyphs[i].GlyphAdvance;
    return advance;
}
//////////////////////////////////////////////////////////////////////////
// StaticTextRecordList
//
void StaticTextRecordList::Clear()
{
    for (UPInt i=0, sz = Records.GetSize(); i < sz; i++)
        if (Records[i]) delete Records[i];
    Records.Clear();
}

StaticTextRecord* StaticTextRecordList::AddRecord()
{
    StaticTextRecord* precord = SF_HEAP_AUTO_NEW(this) StaticTextRecord;
    if (precord)    
        Records.PushBack(precord);     
    return precord;
}

//////////////////////////////////////////////////////////////////////////
// StaticTextDef
//
StaticTextDef::StaticTextDef()
 : Flags(0)
{
}

void    StaticTextDef::Read(LoadProcess* p, TagType tagType)
{                
    SF_ASSERT(tagType == Tag_DefineText || tagType == Tag_DefineText2);
    Stream *in = p->GetStream();

    in->ReadRect(&TextRect);

    in->LogParse("  TextRect = { l: %f, t: %f, r: %f, b: %f }\n", 
        (float)TextRect.x1, (float)TextRect.y1, (float)TextRect.x2, (float)TextRect.y2);

    in->ReadMatrix(&MatrixPriv);

    in->LogParse("  mat:\n");
    in->LogParseClass(MatrixPriv);

    int GlyphBits = in->ReadU8();
    int AdvanceBits = in->ReadU8();

    in->LogParse("begin text records\n");

    bool    lastRecordWasStyleChange = false;
    
    Render::PointF offset;
    Color color;
    float textHeight    = 0;
    UInt16  fontId      = 0;
    ResourcePtr<FontResource> pfont;

    // Style offset starts at 0 and is overwritten when specified.
    offset.SetPoint(0.0f);

	float prevOffY = 0;
    for (;;)
    {
        int FirstByte = in->ReadU8();
        
        if (FirstByte == 0)
        {
            // This is the end of the text records.
            in->LogParse("end text records\n");
            break;
        }

        // Style changes and glyph records just alternate.
        // (Contrary to what most SWF references say!)
        if (lastRecordWasStyleChange == false)
        {
            // This is a style change.

            lastRecordWasStyleChange = true;

            bool    hasFont     = (FirstByte >> 3) & 1;
            bool    hasColor    = (FirstByte >> 2) & 1;
            bool    hasYOffset  = (FirstByte >> 1) & 1;
            bool    hasXOffset  = (FirstByte >> 0) & 1;

            in->LogParse("  text style change\n");

            if (hasFont)
            {
                fontId = in->ReadU16();
                in->LogParse("  HasFont: font id = %d\n", fontId);
                // Create a font resource handle.
                ResourceHandle hres;
                p->GetResourceHandle(&hres, ResourceId(fontId));
                pfont.SetFromHandle(hres);
            }
            if (hasColor)
            {
                if (tagType == 11)
                {
                    in->ReadRgb(&color);                          
                }
                else
                {
                    SF_ASSERT(tagType == 33);                         
                    in->ReadRgba(&color);
                }
                in->LogParse("  HasColor\n");
            }
            if (hasXOffset)
            {                   
                offset.x = in->ReadS16();
                in->LogParse("  XOffset = %g\n", offset.x);
            }
            else
            {
                // Leave carry-over from last record.                   
            }
            if (hasYOffset)
            {                   
                offset.y = in->ReadS16();
                in->LogParse("  YOffset = %g\n", offset.y);
            }
            else
            {
                // Leave carry-over from last record.
            }
            if (hasFont)
            {
                textHeight = in->ReadU16();
                in->LogParse("  TextHeight = %g\n", textHeight);
            }
        }
        else
        {
            // Read the glyph record.
            lastRecordWasStyleChange = false;

            int GlyphCount = FirstByte;
            // Don't mask the top GlyphCount bit; the first record is allowed to have > 127 glyphs.

			if (offset.y < 0 && prevOffY > 0)
			{
				in->LogParse("  Negative Y-offset, 16-bit value rollover, skipping the line...\n");
				// skipping glyph records
				for (int i = 0; i < GlyphCount; i++)
				{
					in->ReadUInt(GlyphBits);
					in->ReadSInt(AdvanceBits);
				}
			}
			else
			{
				StaticTextRecord* precord = TextRecords.AddRecord();
				if (precord)
				{
					prevOffY = offset.y;
					precord->Offset     = offset;
					precord->pFont      = pfont;
					precord->TextHeight = textHeight;
					precord->ColorV     = color;
					precord->FontId     = fontId;
					precord->Read(in, GlyphCount, GlyphBits, AdvanceBits);

					// Add up advances and adjust offset.
					offset.x += precord->GetCumulativeAdvance();
				}

				in->LogParse("  GlyphRecords: count = %d\n", GlyphCount);
			}
        }
    }
}

bool    StaticTextDef::DefPointTestLocal
    (const Render::PointF &pt, bool testShape, const DisplayObjectBase* pinst) const
{
    SF_UNUSED2(testShape, pinst);
    // Flash only uses the bounding box for text - even if shape flag is used
    return TextRect.Contains(pt);
}

StaticTextCharacter::StaticTextCharacter(StaticTextDef* pdef, 
                                         MovieDefImpl *pbindingDefImpl, 
                                         ASMovieRootBase* pasRoot,
                                         InteractiveObject* parent, 
                                         ResourceId id)
 : DisplayObject(pasRoot, parent, id), pDef(pdef), pHighlight(NULL), Flags(0)
{
//    if (id.GetIdIndex() != 286)
 //       return;
    Matrix invm = pdef->MatrixPriv;
    invm.Invert();
    RectF r = invm.EncloseTransform(pdef->TextRect);
    unsigned glyphAccum = 0;
    r.ExpandToPoint(Render::PointF(0,0));

    for (unsigned i = 0, n = pdef->TextRecords.GetSize(); i < n; ++i)
    {
        const StaticTextRecord* prec = pdef->TextRecords.Records[i];

        // resolve font
        SF_ASSERT(prec->FontId != 0);

        ResourceBindData fontData = pbindingDefImpl->GetResourceBinding().GetResourceData(prec->pFont);
        FontResource* pfont = (FontResource*)fontData.pResource.GetPtr();
        if (!pfont)
        {
            Log* plog = pbindingDefImpl->GetLog();
            if (plog)
                plog->LogError("Text style with undefined font; FontId = %d", prec->FontId);
            return;
        }

        if (!pdef->HasInstances())
        {
            // do this check only once per def
            MovieDataDef* pdataDef = pbindingDefImpl->GetDataDef();
            ImportData* pimport = pdataDef->GetFirstImport();
            for(; pimport != 0; pimport = pimport->pNext)
            {
                for (unsigned j = 0; j<pimport->Imports.GetSize(); j++)
                {
                    if (pimport->Imports[j].BindIndex == prec->pFont.GetBindIndex())
                    {
                        // found import table entry, display warning.
                        // It is highly not recommended to use imported fonts for static texts
                        // since imported font might be substituted by the localization stuff. 
                        // Unfortunately, Flash may use imported font implicitly, for example, 
                        // when static text uses font "Arial" and imported font "$Font" represents
                        // also the font "Arial" (see Properties window for imported font). In this 
                        // case, Flash won't create a local "Arial" font for the static text, it
                        // will use the "$Font" instead. Since, static text does not reformat
                        // the text for the different fonts, replacing the "$Font" by another 
                        // font may screw up the static text because of different advance values.
                        Log* plog = pbindingDefImpl->GetLog();
                        if (plog)
                            plog->LogWarning("Static text uses imported font! FontId = %d, import name = %s", 
                            prec->FontId, pimport->Imports[j].SymbolName.ToCStr());
                    }
                }
            }
        }

        // It could be more efficient to obtain FontHandle from manager, however,
        // this is a resource id based lookup (not font name based).
        Ptr<FontHandle> pfontHandle = *SF_NEW FontHandle(NULL, pfont->GetFont());
        SF_ASSERT(pfont->GetFont()->IsResolved());

        LineBuffer::Line* pline;
        unsigned glyphsCount = (unsigned)prec->Glyphs.GetSize();        
        if (glyphsCount <= 255 && !pfont->GetFont()->HasLayout())
        {
            // convert line to short form. Do not use short form if font has layout:
            // in this case we would need to set dimensions on lines and it is hard to
            // predict ranges of dimensions.
            pline = TextGlyphRecords.InsertNewLine(i, glyphsCount, 2, LineBuffer::Line8);
        }
        else
        {
            // Regular form. Used if there are more than 255 glyphs, or font
            // has layout.
            pline = TextGlyphRecords.InsertNewLine(i, glyphsCount, 2, LineBuffer::Line32);
        }
        SF_ASSERT(pline);

        pline->SetTextPos(glyphAccum);
        glyphAccum += glyphsCount;

        Render::PointF offset = prec->Offset;
        float lineHeight = prec->TextHeight;
        if (pfont->GetFont()->HasLayout())
        {
            // There is a layout in the font. This means there we have ascent and descent
            // members in the font and that is why we can set a baseline for the line 
            // (with appropriate correction of its offset.y). This might be necessary
            // to select the static text via TextSnapshot.
            // By default, static text font might have no ascent/descent. In this case
            // selection will not work.

            Double fontScale = prec->TextHeight / 1024;
 
            offset.y -= prec->TextHeight;
            offset -= r.TopLeft();

            // AB: seems to me, Flash uses the TextHeight as ascent value to calculate
            // height of the line. Height of the line is important for the selection
            // via TextSnapshot class.
            float baseLine = prec->TextHeight;

            pline->SetBaseLineOffset(baseLine);

            lineHeight = float(prec->TextHeight + pfont->GetFont()->GetDescent()* fontScale);
        }
        else
        {
            offset -= r.TopLeft();
            lineHeight = 0;
        }
        pline->SetOffset(Render::RoundTwips(offset.x), Render::RoundTwips(offset.y));
        pline->SetDimensions(0, 0);
        LineBuffer::GlyphInserter gins(pline->GetGlyphs(), glyphsCount, pline->GetFormatData());

        int  advanceAccum = 0;
        // C4456 Fixed.......
        for (unsigned i_Accum = 0; i_Accum < glyphsCount; ++i_Accum, ++gins)
        {
            LineBuffer::GlyphEntry& glyph = gins.GetGlyph();
            glyph.SetLength(1);
            glyph.SetIndex(prec->Glyphs[i_Accum].GlyphIndex);
            glyph.SetAdvance((int)prec->Glyphs[i_Accum].GlyphAdvance);
            advanceAccum += (int)prec->Glyphs[i_Accum].GlyphAdvance;
            glyph.SetFontSize((float)TwipsToPixels(prec->TextHeight));
            if (i_Accum == 0)
            {
                gins.AddFont(pfontHandle);
                gins.AddColor(prec->ColorV);
            }
        }

        if (pfont->GetFont()->HasLayout())
        {
            pline->SetDimensions(Alg::Max(0, advanceAccum), Alg::IRound(lineHeight));
            pline->SetTextLength(glyphsCount);

            //correct bottom-right corner of visible rectangle
            r.ExpandToPoint(Render::PointF(float(pline->GetOffsetX() + pline->GetWidth()),
                                    float(pline->GetOffsetY() + pline->GetHeight())));
        }
    }
    TextGlyphRecords.SetVisibleRect(r);
    TextGlyphRecords.SetStaticText();
    //TextGlyphRecords.Dump(); //@DBG
    Filter.SetDefaultShadow();
    pdef->SetHasInstances();  

    RecreateVisibleTextLayout();

#ifdef SF_AMP_SERVER
    if (pDef->IsAAForReadability())
    {
        AmpServer::GetInstance().IncrementFontOptRead();
    }
#endif
}

StaticTextCharacter::~StaticTextCharacter()
{
#ifdef SF_AMP_SERVER
    if (pDef->IsAAForReadability())
    {
        AmpServer::GetInstance().DecrementFontOptRead();
    }
#endif

    if (pHighlight)
        delete pHighlight;
}

//
// Creates the text highlighter
//
StaticTextCharacter::HighlightDesc*  StaticTextCharacter::CreateTextHighlighter()
{
    if (!pHighlight)
        pHighlight = SF_HEAP_NEW(FindMovieImpl()->GetHeap()) StaticTextCharacter::HighlightDesc;
    return pHighlight;
}

void StaticTextCharacter::SetMatrix(const Matrix& m)
{
    OrigMatrix = m;
    Matrix2F mtx(m);
    mtx.Prepend(pDef->MatrixPriv);
    DisplayObject::SetMatrix(mtx);
}

const Matrix& StaticTextCharacter::GetMatrix() const
{
    return OrigMatrix; 
}

void StaticTextCharacter::SetFilters(const FilterSet* filters)
{
    if (!filters)
        return;

    TextFilter tf;
    for (unsigned i = 0; i< filters->GetFilterCount(); i++)
    {
        const Render::Filter* filter = filters->GetFilter(i);
        tf.LoadFilterDesc(filter);
    }
    SetTextFilters(tf);
}

Ptr<Render::TreeNode> StaticTextCharacter::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeText> tshp = *context.CreateEntry<Render::TreeText>();
    // 使用static_cast明确指定转换路径，消除歧义
    return static_cast<Ptr<Render::TreeNode>>(tshp);
}

void StaticTextCharacter::RecreateVisibleTextLayout()
{
    Render::TreeText* textNode = static_cast<Render::TreeText*>(GetRenderNode());
    Render::TextLayout::Builder bld(Memory::GetGlobalHeap());
    if (pHighlight && !pHighlight->IsValid())
    {
        pHighlight->GetManager().UpdateGlyphIndices(NULL);
        pHighlight->Validate();
    }
    TextFieldParam param;
    LoadTextFieldParamFromTextFilter(param, Filter);

    if (pDef->IsAAForReadability())
    {
        param.TextParam.SetOptRead(true);
        param.TextParam.SetAutoFit(true);
        param.ShadowParam.SetOptRead(true);
        param.ShadowParam.SetAutoFit(true);
    }
    TextGlyphRecords.CreateVisibleTextLayout(bld, (pHighlight)? &pHighlight->GetManager() : NULL, param);
    bld.SetBounds(TextGlyphRecords.Geom.VisibleRect);
    textNode->SetLayout(bld);
}

// --------------------------------------------------------------------

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT

StaticTextSnapshotData::StaticTextSnapshotData()
{
    SelectColor = Color(255, 255, 0, 255); // Yellow
}

void    StaticTextSnapshotData::Visit(GlyphVisitor* pvisitor, UPInt start, UPInt end) const
{
    if (!pvisitor) return;
    UPInt glyphIdx = 0;
    for (UPInt i = 0, n = StaticTextCharRefs.GetSize(); i < n; ++i)
    {
        float glyphX = 0;
        float glyphY = 0;
        StaticTextCharacter* pstchar = StaticTextCharRefs[i].pChar;
        float charX = 0, charY = 0;
        const Render::Matrix2F& matChar = pstchar->GetMatrix();
        matChar.Transform(&charX, &charY);
        for (LineBuffer::Iterator iter = pstchar->TextGlyphRecords.Begin(); !iter.IsFinished(); ++iter)
        {
            LineBuffer::Line& line = (*iter);
            UPInt runIdx = 0;
            float xoff = (float)line.GetOffsetX();
            if (xoff < glyphX)
                glyphX = xoff;
            glyphY = float(line.GetOffsetY() + line.GetLeading());
            LineBuffer::GlyphIterator glyphIt = line.Begin();
            Font* pfont = glyphIt.GetFont();
            pvisitor->pFont = pfont; 
            pvisitor->ColorValue = glyphIt.GetColor();
            for (; !glyphIt.IsFinished(); ++glyphIt)
            {
                LineBuffer::GlyphEntry& glyph = glyphIt.GetGlyph();
                // Check range
                if (glyphIdx >= start && glyphIdx < end)
                {
                    // Setup state
                    pvisitor->RunIdx = runIdx;
                    pvisitor->Height = glyph.GetFontSize();
                    Render::Matrix2F mat;
                    mat.AppendRotation((float)matChar.GetRotation());  // Correct
                    mat.AppendTranslation(charX + glyphX, charY + glyphY);
                    pvisitor->Matrix = mat;
                    RectF corners;
                    pfont->GetGlyphBounds(glyph.GetIndex(), &corners);
                    pvisitor->Corners = mat.EncloseTransform(corners);
                    // Expensive..?
                    pvisitor->bSelected = IsSelected(glyphIdx, glyphIdx+1);
                    // Visit
                    pvisitor->OnVisit();

                    ++runIdx;
                }
                glyphX += glyph.GetAdvance();
                ++glyphIdx;
            }          
        }
    }
}

void    StaticTextSnapshotData::Add(StaticTextCharacter* pstChar)
{
    StaticTextSnapshotData::CharRef  cRef;
    cRef.pChar = pstChar;
    cRef.CharCount = 0;    
    float xoffset = 0;
    bool bfirst = true;

    // Iterate over line buffer to get string data and length
    for (LineBuffer::Iterator iter = pstChar->TextGlyphRecords.Begin(); !iter.IsFinished(); ++iter)
    {        
        LineBuffer::Line& line = (*iter);
        float lineXOffset = float(line.GetOffsetX());
        if (!bfirst && lineXOffset == xoffset)
            SnapshotString.AppendChar('\n');
        else if (bfirst)
            xoffset = lineXOffset;
        LineBuffer::GlyphIterator glyphIt = line.Begin();
        Font* pfont = glyphIt.GetFont();
        if (!pfont) continue;
        for (; !glyphIt.IsFinished(); ++glyphIt)
        {
            LineBuffer::GlyphEntry& glyph = glyphIt.GetGlyph();
            unsigned idx = glyph.GetIndex();
            // Get UCS2 char
            int ucs2 = pfont->GetCharValue(idx);
            if (ucs2 == -1) continue;
            // Store values
            cRef.CharCount++;
            SnapshotString.AppendChar(ucs2);
        }
        bfirst = false;
    }

    StaticTextCharRefs.PushBack(cRef);
}

UPInt   StaticTextSnapshotData::GetCharCount() const
{
    UPInt count = 0;
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        count += StaticTextCharRefs[i].CharCount;
    }
    return count;
}

String   StaticTextSnapshotData::GetSubString(UPInt start, UPInt end, bool binclNewLines) const
{
    // Strip out newlines if required
    String ret;
    const char* pchar = SnapshotString.ToCStr();
    UInt32 cval;
    while ( (start < end) && (cval = UTF8Util::DecodeNextChar(&pchar)) != '\0' )
    {
        if (cval != '\n')
        {
            ret.AppendChar(cval);
            start++;
        }
        else if (binclNewLines)
        {
            ret.AppendChar(cval);
        }
    }
    return ret;
}


String StaticTextSnapshotData::GetSelectedText(bool binclNewLines) const
{
    String ret;
    UPInt baseIdx = 0;
    UPInt lastTextIdx = 0;
    const char* pchar = SnapshotString.ToCStr();
    UInt32 c;
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        StaticTextCharacter* pstchar = StaticTextCharRefs[i].pChar;
        StaticTextCharacter::HighlightDesc* phighlight = pstchar->GetTextHighlighter();
        if (!phighlight)
            continue;

        HighlighterRangeIterator ranges = phighlight->GetManager().GetRangeIterator();
        for (; !ranges.IsFinished(); ++ranges)
        {
            HighlightDesc desc = (*ranges);
            // Correct range values
            desc.StartPos += baseIdx;
            // Should be added to string?
            UPInt& startIdx = desc.StartPos;
            UPInt   endIdx = startIdx + desc.Length;
            if (endIdx <= lastTextIdx)
                continue;   // Range has already been processed
            // Skip intermediate chars
            while (lastTextIdx < startIdx)
            {
                c = UTF8Util::DecodeNextChar(&pchar);
                if (c == '\n')
                    lastTextIdx--;
                lastTextIdx++;
            }
            for (UPInt i_Ti = lastTextIdx; i_Ti < endIdx; i_Ti++)
            {
                do 
                {
                    c = UTF8Util::DecodeNextChar(&pchar);                    
                    // Append newlines if required
                    if (binclNewLines && c == '\n')
                        ret.AppendChar(c);
                        
                } while (c == '\n');    // Skip newlines for actual processing
                ret.AppendChar(c);
            }
            lastTextIdx = endIdx;
        }
        baseIdx += StaticTextCharRefs[i].CharCount;
    }
    return ret;
}


//
// Special substring finder that supports case insensitive search. 
// Newlines in the source string are also skipped during comparison.
//
int   StaticTextSnapshotData::FindText(UPInt start, const char* query, bool bcaseSensitive) const
{
    int startidx = (int)start;
    UInt32 chr;
    UInt32 first = UTF8Util::DecodeNextChar(&query);
    const char* string = SnapshotString.ToCStr();

    for(int i = 0; 
        (chr = UTF8Util::DecodeNextChar(&string)) != 0;
        i++)
    {
        if (i >= startidx && 
            (( (bcaseSensitive && chr == first)) || (!bcaseSensitive && SFtoupper(chr) == SFtoupper(first))))
        {
            const char* s1 = string;
            const char* s2 = query;
            UInt32 c1, c2 = 0; 
            int  skipCount;
            for(;;)
            {
                skipCount = -1;

                c2 = UTF8Util::DecodeNextChar(&s2);

                // newline check. if found, skip over
                do 
                {
                    c1 = UTF8Util::DecodeNextChar(&s1);
                    ++skipCount;
                } 
                while (c1 == '\n');

                if (c1 == 0 || c2 == 0) break;

                if (bcaseSensitive)
                {
                    if (c1 != c2)
                        break;
                }
                else
                {
                    if (SFtoupper(c1) != SFtoupper(c2)) 
                        break;
                }
            }
            if (c2 == 0)
            {                
                return i;
            }
            if (c1 == 0)
            {
                return -1;
            }
            i -= skipCount;
        }
        // another newline check
        else if (chr == '\n')
            --i;
    }
    return -1;
}


//
// Return true if any char in input range is selected
//
bool    StaticTextSnapshotData::IsSelected(UPInt start, UPInt end) const
{
    String ret;
    UPInt baseIdx = 0, charStart, charEnd;
    UPInt const& selectStart = start, selectEnd = end;
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        StaticTextCharacter* pstchar = StaticTextCharRefs[i].pChar;
        StaticTextCharacter::HighlightDesc* phighlight = pstchar->GetTextHighlighter();
        if (!phighlight)
            continue;
        charStart = baseIdx;
        charEnd = baseIdx + StaticTextCharRefs[i].CharCount;
        // In range?
        if ( (selectStart >= charStart && selectStart < charEnd) ||
             (charStart >= selectStart && charStart < selectEnd) )
        {
            if (phighlight->GetManager().IsAnyCharSelected(selectStart - baseIdx, selectEnd - baseIdx))
                return true;
        }
        baseIdx += StaticTextCharRefs[i].CharCount;
    }
    return false;
}


void    StaticTextSnapshotData::SetSelected(UPInt start, UPInt end, bool bselect) const
{
    UPInt baseIdx = 0, lenUnprocessed = end - start, charStartIdx, charEndIdx; 
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        charStartIdx = baseIdx;
        charEndIdx = baseIdx + StaticTextCharRefs[i].CharCount;
        // Is static text char in range?
        if ( (charStartIdx >= start && charStartIdx < end) ||
             (start >= charStartIdx && start < charEndIdx) )
        {
            StaticTextCharacter* pstc = StaticTextCharRefs[i].pChar;
            // Is there already a highlighter?
            StaticTextCharacter::HighlightDesc* phighlight = pstc->GetTextHighlighter();
            if (!phighlight)
                phighlight = pstc->CreateTextHighlighter();
            // Create descriptor
            HighlightDesc desc;
            desc.Info.SetBackgroundColor(SelectColor);
            desc.StartPos = (baseIdx > start) ? 0 : (start - baseIdx);
            //desc.Length = lenUnprocessed < StaticTextCharRefs[i].CharCount ? lenUnprocessed : StaticTextCharRefs[i].CharCount;
            desc.Length = Alg::Min(lenUnprocessed, StaticTextCharRefs[i].CharCount - desc.StartPos);
            desc.GlyphNum = desc.Length;
            desc.AdjStartPos = desc.StartPos;
            if (bselect)
            {
                // Add highlight range
                phighlight->GetManager().Add(desc);
            }   
            else
            {
                // Remove highlight range
                phighlight->GetManager().Remove(desc);
            }
            lenUnprocessed -= desc.Length;
            pstc->RecreateVisibleTextLayout();
        }
        // Increment char position
        baseIdx = charEndIdx;        
    }    
}


void    StaticTextSnapshotData::SetSelectColor(const Color& color)
{
    SelectColor = color;
    // Set color and invalidate all highlights in all static chars
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        StaticTextCharacter* pstc = StaticTextCharRefs[i].pChar;
        // Is there already a highlighter?
        if (pstc->GetTextHighlighter())
            pstc->GetTextHighlighter()->SetSelectColor(SelectColor);
    }
}

static
Render::PointF ClosestPointOnRectangle(const RectF& r, const Render::PointF& p)
{    
    const Render::PointF& tl = r.TopLeft();
    const Render::PointF& tr = r.TopRight();
    const Render::PointF& bl = r.BottomLeft();
    const Render::PointF& br = r.BottomRight();

    enum RegionFlags
    {
        Reg_X0 = 0x00,
        Reg_X1 = 0x01,
        Reg_X2 = 0x02,
        Reg_Y0 = 0x00,
        Reg_Y1 = 0x10,
        Reg_Y2 = 0x20,
    };
    UInt32 flags = 0;

    if      (p.x <= tl.x)   flags |= Reg_X0;
    else if (p.x >= tr.x)   flags |= Reg_X2;
    else                    flags |= Reg_X1;
    if      (p.y <= tl.y)   flags |= Reg_Y0;
    else if (p.y >= bl.y)   flags |= Reg_Y2;
    else                    flags |= Reg_Y1;

    float t;
    Render::PointF ret(SF_MAX_FLOAT, SF_MAX_FLOAT);

    switch (flags)
    {
    case Reg_X0 | Reg_Y0:
        return tl;

    case Reg_X0 | Reg_Y1:
        t = Render::Math2D::PointToSegmentPos(tl, bl, p);
        SF_ASSERT(t >= 0 && t <= 1.0f);
        ret.x = tl.x + (bl.x - tl.x) * t;
        ret.y = tl.y + (bl.y - tl.y) * t;
        return ret;

    case Reg_X0 | Reg_Y2:
        return bl;

    case Reg_X1 | Reg_Y0:
        t = Render::Math2D::PointToSegmentPos(tl, tr, p);
        SF_ASSERT(t >= 0 && t <= 1.0f);
        ret.x = tl.x + (tr.x - tl.x) * t;
        ret.y = tl.y + (tr.y - tl.y) * t;
        return ret;

    case Reg_X1 | Reg_Y1:
        // Inside rectangle.. We don't care about this case
        // since this was checked already. x2?
        SF_ASSERT(0);

    case Reg_X1 | Reg_Y2:
        t = Render::Math2D::PointToSegmentPos(bl, br, p);
        SF_ASSERT(t >= 0 && t <= 1.0f);
        ret.x = bl.x + (br.x - bl.x) * t;
        ret.y = bl.y + (br.y - bl.y) * t;
        return ret;

    case Reg_X2 | Reg_Y0:
        return tr;

    case Reg_X2 | Reg_Y1:
        t = Render::Math2D::PointToSegmentPos(tr, br, p);
        SF_ASSERT(t >= 0 && t <= 1.0f);
        ret.x = tr.x + (br.x - tr.x) * t;
        ret.y = tr.y + (br.y - tr.y) * t;
        return ret;
    case Reg_X2 | Reg_Y2:
        return br;
    }

    SF_ASSERT(0); // Shouldn't be here
    return ret;
}

int     StaticTextSnapshotData::HitTestTextNearPos(float x, float y, float closedist) const
{
    StaticTextCharacter* pclosestChar = NULL;
    float minDist = SF_MAX_FLOAT;
    UPInt tempBaseIdx = 0, baseIdx = 0;
    float fixedX = 0, fixedY = 0;
    Render::PointF cpb(x, y);

    // First find closest Static Textfield
    for (UPInt i = 0; i < StaticTextCharRefs.GetSize(); i++)
    {
        StaticTextCharacter* pstc = StaticTextCharRefs[i].pChar;    
        Render::PointF ip = pstc->GetMatrix().TransformByInverse(cpb);
        const RectF& tf = pstc->TextGlyphRecords.Geom.VisibleRect;        
        // If point is inside textfield rect, we're done searching
        if (tf.Contains(ip))
        {
            pclosestChar = pstc;
            baseIdx = tempBaseIdx;
            fixedX = ip.x;
            fixedY = ip.y;
            break;
        }
        // Else keep track of closest textfield
        else
        {
            Render::PointF rp = ClosestPointOnRectangle(tf, ip);
            float dist2rect = Render::Math2D::Distance(ip, rp);
            if (dist2rect < closedist && dist2rect < minDist)
            {
                pclosestChar = pstc;
                minDist = dist2rect;
                baseIdx = tempBaseIdx;
                fixedX = rp.x;
                fixedY = rp.y;
            }            
        }
        tempBaseIdx += StaticTextCharRefs[i].CharCount;
    }

    if (!pclosestChar)
        return -1;

    fixedX -= pclosestChar->TextGlyphRecords.Geom.VisibleRect.x1;
    fixedY -= pclosestChar->TextGlyphRecords.Geom.VisibleRect.y1;

    LineBuffer::Iterator it = pclosestChar->TextGlyphRecords.FindLineAtOffset(Render::PointF(fixedX, fixedY));
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;

        float lineOffX = float(line.GetOffsetX());
        if (fixedX >= lineOffX && fixedX <= lineOffX + float(line.GetWidth()))
        {
            float xoffInLine = fixedX - lineOffX;
            int  xoffset = 0;

            // now need to find the glyph entry. Do the exhaustive search for now
            LineBuffer::GlyphIterator git = line.Begin();
            unsigned j = 0;
            for(; !git.IsFinished(); ++git)
            {
                const LineBuffer::GlyphEntry& ge = git.GetGlyph();
                xoffset += ge.GetAdvance();

                if (xoffset > xoffInLine)
                    break;

                j += ge.GetLength();
            }
            
            return (int)(line.GetTextPos() + j + baseIdx);
        }
    }

    return -1;
}

#endif  // SF_NO_FXPLAYER_AS_TEXTSNAPSHOT

void    SF_STDCALL GFx_DefineTextLoader(LoadProcess* p, const TagInfo& tagInfo)  // Read a DefineText tag.
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineText || tagInfo.TagType == Tag_DefineText2);

    UInt16  characterId = p->ReadU16();

    Ptr<GFx::StaticTextDef> pch =
        *SF_HEAP_NEW_ID(p->GetLoadHeap(),StatMD_CharDefs_Mem) GFx::StaticTextDef();
    p->LogParse("TextCharacter, id = %d\n", characterId);
    pch->Read(p, tagInfo.TagType);

    // Log print some parse stuff...

    p->AddResource(ResourceId(characterId), pch);
}

}} // namespace Scaleform::GFx
