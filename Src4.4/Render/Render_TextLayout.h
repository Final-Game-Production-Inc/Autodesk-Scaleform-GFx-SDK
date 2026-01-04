/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TextLayout.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TextLayout_H
#define INC_SF_Render_TextLayout_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayStaticBuff.h"
#include "Kernel/SF_RefCount.h"
#include "Render_Types2D.h"
#include "Render_GlyphParam.h"


namespace Scaleform { namespace Render {

class Font;
class Image;

// TextLayout class is used as an interface between the renderer and text subsystem.
// It keeps all the data as a stream and contains all necessary information
// about the visible layout of the text field.
//
// Creation:
//    TextLayout::Builder bld(heap);
//
//    // Then use functions to create the layout:
//    bld.ChangeColor(UInt32 color);
//    bld.ChangeFont(Font* f, float size);
//    bld.AddChar(unsigned glyphIndex, float advance);
//    bld.AddText(const char* utfStr);
//    bld.AddImage(Image* img, float scaleX, float scaleY, float baseLine, float advance);
//    bld.AddSelection(const RectF& r, UInt32 color);
//    bld.AddUnderline(float x, float y, float len, TextUnderlineStyle style, UInt32 color);
//    bld.AddCursor(const RectF& r, UInt32 color);
//    bld.SetBackground(UInt32 bkColor, UInt32 brColor);
//    bld.SetNewLine(float x, float y);
//
//    Ptr<TextLayout> textLayout = *SF_NEW TextLayout(bld);
//
//    . . .
//
//     Reading data:
// 
//     TextLayout::Record rec;
//     UPInt pos = 0;
//     while((pos = textLayout->ReadNext(pos, &rec)) != 0)
//     {
//         switch(textLayout->GetRecordType(rec))
//         {
//         case Record_Char:     // See "union Record" and NnnRecord structures
//         case Record_Color:
//         case Record_Background:
//         case Record_NewLine:
//         case Record_Font:
//         case Record_Selection:
//         case Record_Underline:
//         case Record_Cursor:
//         }
//     }
//
//------------------------------------------------------------------------
class TextLayout : public RefCountBase<TextLayout, StatRender_Text_Mem>
{
public:
    enum RecordType
    {
        Record_Char         = 0,
        Record_Color        = 1,
        Record_Background   = 2,
        Record_NewLine      = 3,
        Record_Font         = 4,
        Record_Selection    = 5,
        Record_Underline    = 6,
        Record_Cursor       = 7,
        Record_Image        = 8,
        Record_RefCntData   = 9
    };

    enum CharFlags
    {
        Flag_Invisible  = 0x01,
        Flag_FauxBold   = 0x02,
        Flag_FauxItalic = 0x04
    };

    // A special record for characters with big advance values.
    struct CharRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  GlyphIndex;
        float   Advance;
    };

    // Set/Change color record
    struct ColorRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        UInt32  mColor;
    };

    // Background of the text. If present, appears only once in the text layout.
    // The coordinates are taken from the bounding box.
    struct BackgroundRecord
    {
        UByte  Tag;
        UByte  Flags;
        UInt16 Filler;
        UInt32 BackgroundColor;
        UInt32 BorderColor;
    };

    // Set/Change font
    struct FontRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        float   mSize;
        Font*   pFont;
    };

    // Start new line. In fact, just initial positioning.
    struct LineRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        float   x, y;
    };

    struct SelectionRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        UInt32  mColor;
        float   x1,y1,x2,y2; // Can't use RectF
    };

    struct UnderlineRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Style; // See UnderlineStyle
        float   x, y;
        float   Len;
        UInt32  mColor;
    };

    struct CursorRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        UInt32  mColor;
        float   x1,y1,x2,y2; // Can't use RectF
    };

    struct ImageRecord
    {
        UByte   Tag;
        UByte   Flags;
        UInt16  Filler;
        Image*  pImage;
        float   ScaleX;
        float   ScaleY;
        float   BaseLine;
        float   Advance;
    };

    struct RefCntDataRecord
    {
        UByte           Tag;
        UByte           Flags;
        RefCountImpl*   pData;
    };
    enum RecordSizes
    {
        Size_Char           = sizeof(CharRecord),
        Size_Color          = sizeof(ColorRecord),
        Size_Background     = sizeof(BackgroundRecord),
        Size_Font           = sizeof(FontRecord),
        Size_Line           = sizeof(LineRecord),
        Size_Selection      = sizeof(SelectionRecord),
        Size_Underline      = sizeof(UnderlineRecord),
        Size_Cursor         = sizeof(CursorRecord),
        Size_Image          = sizeof(ImageRecord),
        Size_RefCntData     = sizeof(RefCntDataRecord)
    };

    union Record
    {
        CharRecord              mChar;
        BackgroundRecord        mBackground;
        FontRecord              mFont;
        ColorRecord             mColor;
        LineRecord              mLine;
        SelectionRecord         mSelection;
        UnderlineRecord         mUnderline;
        CursorRecord            mCursor;
        ImageRecord             mImage;
        RefCntDataRecord        mRefCntData;
    };


    class Builder
    {
        enum { StaticDataSize = 1024, StaticFontSize = 32, StaticImageSize = 32, StaticRefCntSize = StaticFontSize };

    public:
        Builder(MemoryHeap* heap);

        void SetBounds(const RectF& b)         { Bounds = b; }
        void SetClipBox(const RectF& b)        { ClipBox = b; }
        void SetParam(const TextFieldParam& p) { Param = p; }

        void AddChar(unsigned glyphIndex, float advance, bool invisible, bool fauxBold, bool fauxItalic);
        void AddText(const char* utfStr);
        void AddImage(Image* img, float scaleX, float scaleY, float baseLine, float advance);
        void AddSelection(const RectF& r, UInt32 color);
        void AddUnderline(float x, float y, float len, TextUnderlineStyle style, UInt32 color);
        void AddCursor(const RectF& r, UInt32 color);
        void AddRefCntData(RefCountImpl*);
        void ChangeColor(UInt32 color);
        void SetBackground(UInt32 bkColor, UInt32 brColor);
        void ChangeFont(Font* f, float size);
        void SetNewLine(float x, float y);

    private:
        Builder(const Builder&);
        void operator = (const Builder&);

        void recordData(const UByte* data);

        TextFieldParam  Param;
        RectF           Bounds;
        RectF           ClipBox;
        ArrayStaticBuffPOD<UByte,   StaticDataSize>     Data;
        ArrayStaticBuffPOD<Font*,   StaticFontSize>     Fonts;
        ArrayStaticBuffPOD<Image*,  StaticImageSize>    Images;
        ArrayStaticBuffPOD<RefCountImpl*, StaticRefCntSize> RefCntData;
        Font*                                           LastFont;
        float                                           LastScale;
        friend class TextLayout;
    };

    //---------------------------------------------
    TextLayout();
    TextLayout(const Builder& bulder);
    ~TextLayout();

    void Clear();
    void Create(const Builder& bulder);

    void SetBounds(const RectF& b)         { Bounds = b; }
    void SetParam(const TextFieldParam& p) { Param = p; }

    const RectF& GetBounds()         const { return Bounds; }
    const RectF& GetClipBox()        const { return ClipBox; }
    const TextFieldParam& GetParam() const { return Param; }

    RecordType GetRecordType(const Record& rec) const
    {
        return RecordType(rec.mChar.Tag);
    }

    // Reading data:
    //
    //TextLayout::Record rec;
    //UPInt pos = 0;
    //while((pos = textLayout.ReadNext(pos, &rec)) != 0)
    //{
    //    switch(textLayout.GetRecordType(rec))
    //    {
    //    case Record_Char:
    //    case Record_Color:
    //    case Record_Background:
    //    case Record_NewLine:
    //    case Record_Font:
    //    case Record_Selection:
    //    case Record_Underline:
    //    case Record_Cursor:
    //    }
    //}
    //----------------------------------------------
    UPInt ReadNext(UPInt pos, Record* rec) const;

private:
    TextLayout(const TextLayout&);
    void operator = (const TextLayout&);

    TextFieldParam                          Param;
    RectF                                   Bounds;
    RectF                                   ClipBox;
    ArrayLH_POD<UByte, StatRender_Text_Mem> Data;
    UPInt                                   DataSize;
    Font**                                  pFonts;
    UPInt                                   FontCount;
    Image**                                 pImages;
    UPInt                                   ImageCount;
    RefCountImpl**                          pRefCntData;
    UPInt                                   RefCntCount;
};







}} // Scaleform::Render

#endif
