/**************************************************************************

Filename    :   Render_TextLayout.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_UTF8Util.h"
#include "Render_TextLayout.h"
#include "Render_Font.h"
#include "Render_Image.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
static const unsigned TextLayout_RecordSizes[] = 
{ 
    TextLayout::Size_Char,              // Record_Char
    TextLayout::Size_Color,             // Record_Color
    TextLayout::Size_Background,        // Record_Background
    TextLayout::Size_Line,              // Record_Line
    TextLayout::Size_Font,              // Record_Font
    TextLayout::Size_Selection,         // Record_Selection
    TextLayout::Size_Underline,         // Record_Underline
    TextLayout::Size_Cursor,            // Record_Cursor
    TextLayout::Size_Image,             // Record_Image
    TextLayout::Size_RefCntData         // Record_RefCntData
};




//------------------------------------------------------------------------
TextLayout::TextLayout() :
    DataSize(0), pFonts(0), FontCount(0), pImages(0), ImageCount(0), pRefCntData(0), RefCntCount(0)
{}

TextLayout::TextLayout(const Builder& builder) :
    pFonts(0), FontCount(0), pImages(0), ImageCount(0), pRefCntData(0), RefCntCount(0)
{
    Create(builder);
}

TextLayout::~TextLayout()
{
    Clear();
}

void TextLayout::Clear()
{
    UPInt i;
    for(i = 0; i < FontCount; ++i)
        pFonts[i]->Release();
    for(i = 0; i < ImageCount; ++i)
        pImages[i]->Release();
    for(i = 0; i < RefCntCount; ++i)
        pRefCntData[i]->Release();
    Data.Clear();
}

//------------------------------------------------------------------------
void TextLayout::Create(const Builder& builder)
{
    pFonts = 0; FontCount = 0;
    pImages = 0; ImageCount = 0;
    pRefCntData = 0; RefCntCount = 0;

    Bounds  = builder.Bounds;
    ClipBox = builder.ClipBox;
    Param   = builder.Param;

    DataSize = builder.Data.GetSize();
    UPInt fonts = (DataSize + sizeof(void*)-1) & ~(sizeof(void*)-1);
    UPInt total = fonts + (builder.Fonts.GetSize() + builder.Images.GetSize() + builder.RefCntData.GetSize()) * sizeof(void*);
    Data.Resize(total);
    UPInt off = fonts;
    if (builder.Fonts.GetSize())
    {
        pFonts = (Font**)(&Data[off]);
        FontCount = builder.Fonts.GetSize();
        off += FontCount * sizeof(void*);
    }
    if (builder.Images.GetSize())
    {
        pImages = (Image**)(&Data[off]);
        ImageCount = builder.Images.GetSize();
        off += ImageCount * sizeof(void*);
    }
    if (builder.RefCntData.GetSize())
    {
        pRefCntData = (RefCountImpl**)(&Data[off]);
        RefCntCount = builder.RefCntData.GetSize();
        off += RefCntCount * sizeof(void*);
    }
    
    if (builder.Data.GetSize())
        memcpy(&Data[0], &builder.Data[0], DataSize);

    if (pFonts)
        memcpy(pFonts, &builder.Fonts[0], FontCount * sizeof(void*));

    if (pImages)
        memcpy(pImages, &builder.Images[0], ImageCount * sizeof(void*));

    if (pRefCntData)
        memcpy(pRefCntData, &builder.RefCntData[0], RefCntCount * sizeof(void*));

    UPInt i;
    for(i = 0; i < FontCount; ++i)
        pFonts[i]->AddRef();

    for(i = 0; i < ImageCount; ++i)
        pImages[i]->AddRef();

    for(i = 0; i < RefCntCount; ++i)
        pRefCntData[i]->AddRef();
}

//------------------------------------------------------------------------
UPInt TextLayout::ReadNext(UPInt pos, Record* rec) const
{
    if (pos >= DataSize) return 0;
    SF_ASSERT(pos + 4 <= DataSize);
    UByte* data = (UByte*)rec;
    *data++ = Data[pos++];
    unsigned size = TextLayout_RecordSizes[rec->mChar.Tag];
    while(--size)
        *data++ = Data[pos++];
    return pos;
}


//------------------------------------------------------------------------
TextLayout::Builder::Builder(MemoryHeap* heap) :
    Param(), Bounds(0,0,0,0), ClipBox(0,0,0,0),
    Data(heap), Fonts(heap), Images(heap), RefCntData(heap), LastFont(0), LastScale(1)
{}




//------------------------------------------------------------------------
SF_INLINE void TextLayout::Builder::recordData(const UByte* data)
{
    unsigned size = TextLayout_RecordSizes[data[0]];
    while(size--)
        Data.PushBack(*data++);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddChar(unsigned glyphIndex, float advance, bool invisible, bool fauxBold, bool fauxItalic)
{
    UByte flags = 0;
    if (invisible)
        flags |= Flag_Invisible;

    if (fauxBold)
        flags |= Flag_FauxBold;

    if (fauxItalic)
        flags |= Flag_FauxItalic;

    CharRecord rec = { Record_Char, flags, UInt16(glyphIndex), advance };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
// A simple automation to add text with last set font. Does not process
// kerning pairs.
void TextLayout::Builder::AddText(const char* utfStr)
{
    SF_ASSERT(LastFont);
    UInt32 c;
    while((c = UTF8Util::DecodeNextChar(&utfStr)) != 0)
    {
        unsigned glyphIndex = LastFont->GetGlyphIndex((UInt16)c);
        float advance = LastFont->GetAdvance(glyphIndex) * LastScale;
        AddChar(glyphIndex, advance, false, false, false);
    }
}

//------------------------------------------------------------------------
void TextLayout::Builder::ChangeColor(UInt32 color)
{
    ColorRecord rec = { Record_Color, 0, 0, color };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::SetBackground(UInt32 bkColor, UInt32 brColor)
{
    BackgroundRecord rec = { Record_Background, 0, 0, bkColor, brColor };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::ChangeFont(Font* f, float size)
{
    FontRecord rec = { Record_Font, 0, 0, size, f };
    recordData((const UByte*)&rec);
    LastFont = f;
    LastScale = size / f->GetNominalGlyphHeight();
    for(UPInt i = 0; i < Fonts.GetSize(); ++i)
    {
        if (f == Fonts[i])
            return;
    }
    Fonts.PushBack(f);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddRefCntData(RefCountImpl* p)
{
    RefCntDataRecord rec = { Record_RefCntData, 0, p };
    recordData((const UByte*)&rec);
    for(UPInt i = 0; i < RefCntData.GetSize(); ++i)
    {
        if (p == RefCntData[i])
            return;
    }
    RefCntData.PushBack(p);
}

//------------------------------------------------------------------------
void TextLayout::Builder::SetNewLine(float x, float y)
{
    LineRecord rec = { Record_NewLine, 0, 0, x, y };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddSelection(const RectF& r, UInt32 color)
{
    SelectionRecord rec = { Record_Selection, 0, 0, color, r.x1, r.y1, r.x2, r.y2 };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddUnderline(float x, float y, float len, TextUnderlineStyle style, UInt32 color)
{
    UnderlineRecord rec = { Record_Underline, 0, (UInt16)style, x, y, len, color };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddCursor(const RectF& r, UInt32 color)
{
    CursorRecord rec = { Record_Cursor, 0, 0, color, r.x1, r.y1, r.x2, r.y2 };
    recordData((const UByte*)&rec);
}

//------------------------------------------------------------------------
void TextLayout::Builder::AddImage(Image* img, float scaleX, float scaleY, float baseLine, float advance)
{
    ImageRecord rec = { Record_Image, 0, 0, img, scaleX, scaleY, baseLine, advance }; 
    recordData((const UByte*)&rec);
    for(UPInt i = 0; i < Images.GetSize(); ++i)
    {
        if (img == Images[i])
            return;
    }
    Images.PushBack(img);
}

}} // Scaleform::Render
