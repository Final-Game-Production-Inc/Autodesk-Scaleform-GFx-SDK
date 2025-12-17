/**************************************************************************

PublicHeader:   Render
Filename    :   Text_Core.h
Content     :   Core text definitions
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_TEXT_TEXTCORE_H
#define INC_Render_TEXT_TEXTCORE_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Range.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_WString.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Stats.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_CxForm.h"
#include "Render/Render_Color.h"
#include "Render/Render_Image.h"
#include "Text_FontManager.h"

#define MAX_FONT_NAME_LEN           50
#define GFX_ACTIVE_SEL_BKCOLOR      0xFF000000u
#define GFX_INACTIVE_SEL_BKCOLOR    0xFF808080u
#define GFX_ACTIVE_SEL_TEXTCOLOR    0xFFFFFFFFu
#define GFX_INACTIVE_SEL_TEXTCOLOR  0xFFFFFFFFu

namespace Scaleform { 
namespace Render {
namespace Text {

template <typename T> T FixpToPixels(T x) { return T((x) / T(20.)); }
template <typename T> T PixelsToFixp(T x) { return T((x) * T(20.)); }

// Specializations to use mult instead of div op
template <> inline float  FixpToPixels(float x)  { return x * 0.05f; }
template <> inline double FixpToPixels(double x) { return x * 0.05;  }

inline RectF FixpToPixels(const RectF& x) 
{ 
    return RectF(FixpToPixels(x.x1),  FixpToPixels(x.y1), 
        FixpToPixels(x.x2),  FixpToPixels(x.y2));
}

inline RectF PixelsToFixp(const RectF& x) 
{ 
    return RectF(PixelsToFixp(x.x1),  PixelsToFixp(x.y1), 
        PixelsToFixp(x.x2),  PixelsToFixp(x.y2));
}

//template <typename Int> float FixpToPixels(Int x) { return float(float(x) * (1.0/32.0)); }
//template <typename Int, typename Fl> Int PixelsToFixp(Fl x) { return Int(x * 32.); }

class Paragraph;

#ifdef SF_BUILD_DEBUG
template <class T>
void _dump(RangeDataArray<T>& ranges)
{
    printf("-----\n");
    typename RangeDataArray<T>::Iterator it = ranges.Begin();
    for(; !it.IsFinished(); ++it)
    {
        RangeData<T>& r = *it;
        printf ("i = %d, ei = %d\n", r.Index, r.NextIndex());
    }
    printf("-----\n");
}
#endif

// Image descriptor that is used by LineBuffer to hold info about image (and image itself)
struct ImageDesc : public RefCountBaseNTS<ImageDesc, StatRender_Text_Mem>
{
    typedef Matrix2F   MatrixType;

    Ptr<Image>              pImage;
    float                   BaseLineX, BaseLineY; // in pixels of the image
    float                   ScreenWidth, ScreenHeight; // in screen units (twips)
    MatrixType              Matrix;                      

    ImageDesc():BaseLineX(0), BaseLineY(0), ScreenWidth(0), ScreenHeight(0) {}

    // in twips
    float GetScreenWidth()  const { return ScreenWidth; }
    float GetScreenHeight() const { return ScreenHeight; }

    float GetBaseLineX() const { return BaseLineX; }
    float GetBaseLineY() const { return BaseLineY; }

    float GetXScale() const { return Matrix.GetXScale(); }
    float GetYScale() const { return Matrix.GetYScale(); }
};

// A descriptor that is used by the TextFormat for holding
// images (<IMG> tag).
struct HTMLImageTagDesc : public ImageDesc
{
    StringLH    Url;
    StringLH    Id;
    int         VSpace, HSpace; // in twips
    unsigned    ParaId;
    enum
    {
        Align_BaseLine,
        Align_Right,
        Align_Left
    };
    UByte     Alignment;

    HTMLImageTagDesc() : VSpace(0), HSpace(0), 
        ParaId(~0u), Alignment(Align_BaseLine) {}
    
    UPInt GetHash() const;

    bool operator == (const HTMLImageTagDesc& f) const
    {
        return Url == f.Url && Id == f.Id && VSpace == f.VSpace && 
               HSpace == f.HSpace && ParaId == f.ParaId && Alignment == f.Alignment;
    }
};

// Format descriptor, applying to rich text
// TextFormat ref counting is Thread-Safe on purpose
class TextFormat : public NewOverrideBase<StatRender_Text_Mem>
{
    // Ranges:
    // fontSize         [0..2500] pts
    // letterSpacing    [-60..60] pts
public:
    friend class HashFunctor;
    struct HashFunctor
    {
        static UPInt CalcHash(const TextFormat& data);

        UPInt  operator()(const TextFormat& data) const { return CalcHash(data); }
        UPInt  operator()(const TextFormat* data) const { return CalcHash(*data); }
    };
private:
    class Allocator*                pAllocator;
    mutable unsigned                RefCount;
protected:
    StringDH                        FontList; // comma separated list of font names
    StringDH                        Url;
    Ptr<HTMLImageTagDesc>           pImageDesc;
    Ptr<FontHandle>                 pFontHandle; // required if font was referenced by id

    UInt32                          ColorV; // format AA RR GG BB.
    float                           LetterSpacing;
    UInt16                          FontSize;      // in 11.5 fixed point!

    enum
    {
        Format_Bold =       0x1,
        Format_Italic =     0x2,
        Format_Underline =  0x4,
        Format_Kerning =    0x8
    };
    UInt8       FormatFlags;

    enum
    {
        PresentMask_Color         = 1,
        PresentMask_LetterSpacing = 2,
        PresentMask_FontList      = 4,
        PresentMask_FontSize      = 8,
        PresentMask_Bold          = 0x10,
        PresentMask_Italic        = 0x20,
        PresentMask_Underline     = 0x40,
        PresentMask_Kerning       = 0x80,
        PresentMask_Url           = 0x100,
        PresentMask_ImageDesc     = 0x200,
        PresentMask_Alpha         = 0x400,
        PresentMask_FontHandle    = 0x800,
        PresentMask_SingleFontName= 0x1000 // indicating that the single font name is stored in FontList
    };
    UInt16       PresentMask;

    //    void InvalidateCachedFont() { pCachedFont = 0; }
public:

    TextFormat(MemoryHeap* pheap = NULL);
    TextFormat(Allocator* allocator);
    TextFormat(const TextFormat& srcfmt, Allocator* allocator = NULL);
    ~TextFormat();

    void AddRef() const  { ++RefCount; }
    void Release() const { if (--RefCount == 0) delete this; }
    unsigned GetRefCount() const { return RefCount; }

    MemoryHeap* GetHeap() const { return FontList.GetHeap(); }
    Allocator*  GetAllocator() const { return pAllocator; }

    void SetBold(bool bold = true);
    void ClearBold()    { FormatFlags &= ~Format_Bold; PresentMask &= ~PresentMask_Bold; }
    bool IsBold() const { return (FormatFlags & Format_Bold) != 0; }

    void SetItalic(bool italic = true);
    void ClearItalic()    { FormatFlags &= ~Format_Italic; PresentMask &= ~PresentMask_Italic; }
    bool IsItalic() const { return (FormatFlags & Format_Italic) != 0; }

    void SetUnderline(bool underline = true);
    void ClearUnderline()    { FormatFlags &= ~Format_Underline; PresentMask &= ~PresentMask_Underline; }
    bool IsUnderline() const { return (FormatFlags & Format_Underline) != 0; }

    void SetKerning(bool kerning = true);
    void ClearKerning()    { FormatFlags &= ~Format_Kerning; PresentMask &= ~PresentMask_Kerning; }
    bool IsKerning() const { return (FormatFlags & Format_Kerning) != 0; }

    // This method sets whole color, including the alpha
    void   SetColor(const Color& color) { ColorV = color.ToColor32(); PresentMask |= PresentMask_Color; }
    Color GetColor() const              { return Color(ColorV); }
    // This method sets only RGB values of color
    void SetColor32(UInt32 color) { ColorV = (color & 0xFFFFFF) | (ColorV & 0xFF000000u); PresentMask |= PresentMask_Color; }
    UInt32 GetColor32() const     { return ColorV; }
    void ClearColor()             { ColorV = 0xFF000000u; PresentMask &= ~PresentMask_Color; }

    void SetAlpha(UInt8 alpha) { ColorV = (ColorV & 0xFFFFFFu) | (UInt32(alpha)<<24); PresentMask |= PresentMask_Alpha; }
    UInt8 GetAlpha() const  { return UInt8((ColorV >> 24) & 0xFF); }
    void ClearAlpha()       { ColorV |= 0xFF000000u; PresentMask &= ~PresentMask_Alpha; }

    // letterSpacing is specified in pixels, e.g.: 2.6 pixels, but stored in twips
    void SetLetterSpacing(float letterSpacing) 
    {
        PresentMask |= PresentMask_LetterSpacing; 
        LetterSpacing = PixelsToFixp(letterSpacing);
    }
    float GetLetterSpacing() const             { return FixpToPixels(LetterSpacing); }
    void SetLetterSpacingInFixp(int letterSpacing) 
    { 
        SF_ASSERT(letterSpacing >= -32768 && letterSpacing < 32768); 
        LetterSpacing = (SInt16)letterSpacing; PresentMask |= PresentMask_LetterSpacing; 
    }
    SInt16 GetLetterSpacingInFixp() const        { return (SInt16)LetterSpacing; }
    void ClearLetterSpacing()                  { LetterSpacing = 0; PresentMask &= ~PresentMask_LetterSpacing; }

    void SetFontSizeInFixp(unsigned fontSize) 
    { 
        FontSize = (fontSize <= 65536) ? (UInt16)fontSize : (UInt16)65535u; 
        PresentMask |= PresentMask_FontSize; 
    }
    unsigned GetFontSizeInFixp() const { return FontSize; }
    void SetFontSize(float fontSize) 
    { 
        FontSize = (fontSize < FixpToPixels(65536.0f)) ? (UInt16)PixelsToFixp(fontSize) : (UInt16)65535u; 
        PresentMask |= PresentMask_FontSize; 
    }
    float GetFontSize(float scaleFactor) const { return FixpToPixels((float)FontSize)*scaleFactor; }
    float GetFontSize() const       { return FixpToPixels((float)FontSize); }
    void ClearFontSize()            { FontSize = 0; PresentMask &= ~PresentMask_FontSize; }

    void SetFontName(const String& fontName);
    void SetFontName(const char* pfontName, UPInt  fontNameSz = SF_MAX_UPINT);
    void SetFontName(const wchar_t* pfontName, UPInt fontNameSz = SF_MAX_UPINT);
    void SetFontList(const String& fontList);
    void SetFontList(const char* pfontList, UPInt  fontListSz = SF_MAX_UPINT);
    void SetFontList(const wchar_t* pfontList, UPInt  fontListSz = SF_MAX_UPINT);
    const String& GetFontList() const;
    void ClearFontList() { PresentMask &= ~(PresentMask_FontList|PresentMask_SingleFontName); }

    void SetFontHandle(FontHandle* pfontHandle); 
    FontHandle* GetFontHandle() const { return (IsFontHandleSet()) ? pFontHandle : NULL; }
    void ClearFontHandle() { pFontHandle = NULL; PresentMask &= ~PresentMask_FontHandle; }

    void SetImageDesc(HTMLImageTagDesc* pimage) { pImageDesc = pimage; PresentMask |= PresentMask_ImageDesc;}
    HTMLImageTagDesc* GetImageDesc() const { return (IsImageDescSet()) ? pImageDesc : NULL; }
    void ClearImageDesc() { pImageDesc = NULL; PresentMask &= ~PresentMask_ImageDesc; }

    void SetUrl(const char* purl, UPInt  urlSz = SF_MAX_UPINT);
    void SetUrl(const wchar_t* purl, UPInt  urlSz = SF_MAX_UPINT);
    void SetUrl(const String& url);
    const String& GetUrl() const    { return Url; }
    const char* GetUrlCStr() const     { return (IsUrlSet()) ? Url.ToCStr() : ""; }
    void ClearUrl()                    { Url.Clear(); PresentMask &= ~PresentMask_Url; }

    bool IsBoldSet() const      { return (PresentMask & PresentMask_Bold) != 0; }
    bool IsItalicSet() const    { return (PresentMask & PresentMask_Italic) != 0; }
    bool IsUnderlineSet() const { return (PresentMask & PresentMask_Underline) != 0; }
    bool IsKerningSet() const   { return (PresentMask & PresentMask_Kerning) != 0; }
    bool IsFontSizeSet() const  { return (PresentMask & PresentMask_FontSize) != 0; }
    bool IsFontListSet() const  { return (PresentMask & PresentMask_FontList) != 0; }
    bool IsSingleFontNameSet() const { return (PresentMask & PresentMask_SingleFontName) != 0; }
    bool IsFontHandleSet() const{ return (PresentMask & PresentMask_FontHandle) != 0; }
    bool IsImageDescSet() const { return (PresentMask & PresentMask_ImageDesc) != 0; }
    bool IsColorSet() const     { return (PresentMask & PresentMask_Color) != 0; }
    bool IsAlphaSet() const     { return (PresentMask & PresentMask_Alpha) != 0; }
    bool IsLetterSpacingSet() const { return (PresentMask & PresentMask_LetterSpacing) != 0; }
    bool IsUrlSet() const       { return (PresentMask & PresentMask_Url) != 0 && Url.GetLength() != 0; }
    bool IsUrlCleared() const   { return (PresentMask & PresentMask_Url) != 0 && Url.GetLength() == 0; }

    TextFormat Merge(const TextFormat& fmt) const;
    TextFormat Intersection(const TextFormat& fmt) const;

    bool operator == (const TextFormat& f) const
    {
        return (PresentMask == f.PresentMask && FormatFlags == f.FormatFlags && ColorV == f.ColorV && 
            FontSize == f.FontSize && 
            (IsFontListSet() == f.IsFontListSet() && (!IsFontListSet() || FontList.CompareNoCase(f.FontList) == 0)) && 
            LetterSpacing == f.LetterSpacing &&
            (IsFontHandleSet() == f.IsFontHandleSet() && (!IsFontHandleSet() || pFontHandle == f.pFontHandle || 
            (pFontHandle && f.pFontHandle && *pFontHandle == *f.pFontHandle))) &&
            (IsUrlSet() == f.IsUrlSet() && (!IsUrlSet() || Url.CompareNoCase(f.Url) == 0)) && 
            ((pImageDesc && f.pImageDesc && *pImageDesc == *f.pImageDesc) || (pImageDesc == f.pImageDesc)));
    }

    bool IsFontSame(const TextFormat& fmt) const; 
    bool IsHTMLFontTagSame(const TextFormat& fmt) const;
    void InitByDefaultValues();
};

// Format descriptor, applying to whole paragraph
class ParagraphFormat : public NewOverrideBase<StatRender_Text_Mem>
{
public:
    friend class HashFunctor;
    struct HashFunctor
    {
        static UPInt CalcHash(const ParagraphFormat& data);

        UPInt  operator()(const ParagraphFormat& data) const { return CalcHash(data); }
        UPInt  operator()(const ParagraphFormat* data) const { return CalcHash(*data); }
    };

    // ranges:
    // indent   [-720..720] px
    // leading  [-360..720] px
    // margins  [0..720] px
public:
    enum AlignType
    {
        Align_Left      = 0,
        Align_Right     = 1,
        Align_Justify   = 2,
        Align_Center    = 3
    };
    enum DisplayType
    {
        Display_Inline = 0,
        Display_Block  = 1,
        Display_None   = 2
    };
private:
    class Allocator* pAllocator;
    mutable unsigned RefCount;
protected:
    unsigned*  pTabStops;   // First elem is total num of tabstops, followed by tabstops in twips
    UInt16 BlockIndent;
    SInt16 Indent;
    SInt16 Leading;
    UInt16 LeftMargin;
    UInt16 RightMargin;
    enum
    {
        PresentMask_Alignment     = 0x01,
        PresentMask_BlockIndent   = 0x02,
        PresentMask_Indent        = 0x04,
        PresentMask_Leading       = 0x08,
        PresentMask_LeftMargin    = 0x10,
        PresentMask_RightMargin   = 0x20,
        PresentMask_TabStops      = 0x40,
        PresentMask_Bullet        = 0x80,
        PresentMask_Display       = 0x100,

        Mask_Align      = 0x600, // (0x400 | 0x200)
        Shift_Align     = 9,
        Mask_Display    = 0x1800,// (0x1000 | 0x800)
        Shift_Display   = 11,
        Mask_Bullet     = 0x8000
    };
    UInt16 PresentMask;
public:
    ParagraphFormat() : 
        pAllocator(NULL),
        RefCount(1), pTabStops(NULL), BlockIndent(0), Indent(0), Leading(0), 
        LeftMargin(0), RightMargin(0), PresentMask(0) 
    {}

    ParagraphFormat(const ParagraphFormat& src, Allocator* allocator = NULL) : 
        pAllocator(allocator),
        RefCount(1), pTabStops(NULL), 
        BlockIndent(src.BlockIndent), Indent(src.Indent), Leading(src.Leading), 
        LeftMargin(src.LeftMargin), RightMargin(src.RightMargin), 
        PresentMask(src.PresentMask)
    {
        CopyTabStops(src.pTabStops);
    }
    ~ParagraphFormat();

    void AddRef() const  { ++RefCount; }
    void Release() const { if (--RefCount == 0) delete this; }
    unsigned GetRefCount() const { return RefCount; }

    Allocator*  GetAllocator() const { return pAllocator; }

    void SetAlignment(AlignType align) 
    { 
        PresentMask = (UInt16)((PresentMask & (~Mask_Align)) | ((align << Shift_Align) & Mask_Align)); 
        PresentMask |= PresentMask_Alignment; 
    }
    void ClearAlignment()              { PresentMask &= ~Mask_Align; PresentMask &= ~PresentMask_Alignment; }
    bool IsAlignmentSet() const        { return (PresentMask & PresentMask_Alignment) != 0; }
    bool IsLeftAlignment() const       { return IsAlignmentSet() && GetAlignment() == Align_Left; }
    bool IsRightAlignment() const      { return IsAlignmentSet() && GetAlignment() == Align_Right; }
    bool IsCenterAlignment() const     { return IsAlignmentSet() && GetAlignment() == Align_Center; }
    bool IsJustifyAlignment() const    { return IsAlignmentSet() && GetAlignment() == Align_Justify; }
    AlignType GetAlignment() const     { return (AlignType)((PresentMask & Mask_Align) >> Shift_Align); }

    void SetBullet(bool bullet = true) 
    { 
        if (bullet)
            PresentMask |= Mask_Bullet; 
        else
            PresentMask &= ~Mask_Bullet; 
        PresentMask |= PresentMask_Bullet; 
    }
    void ClearBullet()                 { PresentMask &= ~Mask_Bullet; PresentMask &= ~PresentMask_Bullet; }
    bool IsBulletSet() const           { return (PresentMask & PresentMask_Bullet) != 0; }
    bool IsBullet() const              { return IsBulletSet() && (PresentMask & Mask_Bullet) != 0; }

    void SetBlockIndent(unsigned value)    { SF_ASSERT(value < 65536); BlockIndent = (UInt16)value; PresentMask |= PresentMask_BlockIndent; }
    unsigned GetBlockIndent() const        { return BlockIndent; }
    void ClearBlockIndent()            { BlockIndent = 0; PresentMask &= ~PresentMask_BlockIndent; }
    bool IsBlockIndentSet() const      { return (PresentMask & PresentMask_BlockIndent) != 0; }

    void SetIndent(int value)     { SF_ASSERT(value >= -32768 && value < 32768); Indent = (SInt16)value; PresentMask |= PresentMask_Indent; }
    int  GetIndent() const        { return Indent; }
    void ClearIndent()            { Indent = 0; PresentMask &= ~PresentMask_Indent; }
    bool IsIndentSet() const      { return (PresentMask & PresentMask_Indent) != 0; }

    void SetLeading(int value)    { SF_ASSERT(value >= -32768 && value < 32768); Leading = (SInt16)value; PresentMask |= PresentMask_Leading; }
    int  GetLeading() const        { return Leading; }
    void ClearLeading()            { Leading = 0; PresentMask &= ~PresentMask_Leading; }
    bool IsLeadingSet() const      { return (PresentMask & PresentMask_Leading) != 0; }

    void SetLeftMargin(unsigned value)    { SF_ASSERT(value < 65536); LeftMargin = (UInt16)value; PresentMask |= PresentMask_LeftMargin; }
    unsigned GetLeftMargin() const        { return LeftMargin; }
    void ClearLeftMargin()            { LeftMargin = 0; PresentMask &= ~PresentMask_LeftMargin; }
    bool IsLeftMarginSet() const      { return (PresentMask & PresentMask_LeftMargin) != 0; }

    void SetRightMargin(unsigned value)    { SF_ASSERT(value < 65536); RightMargin = (UInt16)value; PresentMask |= PresentMask_RightMargin; }
    unsigned GetRightMargin() const        { return RightMargin; }
    void ClearRightMargin()            { RightMargin = 0; PresentMask &= ~PresentMask_RightMargin; }
    bool IsRightMarginSet() const      { return (PresentMask & PresentMask_RightMargin) != 0; }

    void SetTabStops(unsigned num, ...);
    const unsigned* GetTabStops(unsigned* pnum) const;
    void  SetTabStops(const unsigned* psrcTabStops);
    const unsigned* GetTabStops() const { return pTabStops; }
    void SetTabStopsNum(unsigned num)   { AllocTabStops(num); PresentMask |= PresentMask_TabStops; }
    void SetTabStopsElement(unsigned idx, unsigned val);
    void ClearTabStops()            { FreeTabStops(); PresentMask &= ~PresentMask_TabStops; }
    bool IsTabStopsSet() const      { return (PresentMask & PresentMask_TabStops) != 0; }

    void SetDisplay(DisplayType display) 
    { 
        PresentMask = (UInt16)((PresentMask & (~Mask_Display)) | ((display << Shift_Display) & Mask_Display)); 
        PresentMask |= PresentMask_Display; 
    }
    void ClearDisplay()                  { PresentMask &= ~Mask_Display; PresentMask &= ~PresentMask_Display; }
    bool IsDisplaySet() const            { return (PresentMask & PresentMask_Display) != 0; }
    DisplayType GetDisplay() const       { return (DisplayType)((PresentMask & Mask_Display) >> Shift_Display); }

    ParagraphFormat Merge(const ParagraphFormat& fmt) const;
    ParagraphFormat Intersection(const ParagraphFormat& fmt) const;

    ParagraphFormat& operator=(const ParagraphFormat& src);

    bool operator == (const ParagraphFormat& f) const
    {
        return (PresentMask == f.PresentMask && BlockIndent == f.BlockIndent && Indent == f.Indent && 
            Leading == f.Leading && LeftMargin == f.LeftMargin &&
            RightMargin == f.RightMargin && TabStopsEqual(f.pTabStops));
    }

    void InitByDefaultValues();
protected:
    void AllocTabStops(unsigned num);
    void FreeTabStops();
    bool TabStopsEqual(const unsigned* psrcTabStops) const;
    void CopyTabStops(const unsigned* psrcTabStops);
};

// A special class that adds operator== to a pointer to use with Hash 
template <class T> 
class PtrCompare
{
	T P;
public:
	PtrCompare():P(NULL) {}
	PtrCompare(T p):P(p) {}
	PtrCompare(const PtrCompare& pp):P(pp.P) {}
	~PtrCompare() {}

	operator T() const   { return P; }
	T operator->() const { return P; }
	bool operator==(const T p) const 
	{
		return (p == P) || (P && p && *P == *p);
	}
	bool operator==(const PtrCompare<T>& p) const 
	{
		return (p.P == P) || (P && p.P && *P == *p.P);
	}
	PtrCompare& operator=(const PtrCompare& pp) { P = pp.P; return *this; }
	PtrCompare& operator=(T p) { P = p; return *this; }
};
template <class T>
bool operator==(const T* p, const PtrCompare<T>& p1)
{
	return p1 == p;
}

// Text allocator. Allocates text, text and paragraph formats.
class Allocator : public RefCountBaseNTS<Allocator, StatRender_Text_Mem>
{
    friend class TextFormat;
    friend class ParagraphFormat;
public:
    enum FlagsEnum
    {
        Flags_Global = 0x1 // global allocator, do not save font handles (since they might be destroyed)
    };
protected:
    enum 
    {
        InitialFormatCacheCap = 100,
        FormatCacheCapacityDelta = 10
    };
    typedef HashSetLH<PtrCompare<TextFormat*>, TextFormat::HashFunctor,
                       TextFormat::HashFunctor, StatRender_Text_Mem> TextFormatStorageType;

    typedef HashSetLH<PtrCompare<ParagraphFormat*>, ParagraphFormat::HashFunctor,
                       ParagraphFormat::HashFunctor, StatRender_Text_Mem> ParagraphFormatStorageType;

    TextFormatStorageType       TextFormatStorage;
    ParagraphFormatStorageType  ParagraphFormatStorage;
    unsigned                    TextFormatStorageCap;
    unsigned                    ParagraphFormatStorageCap;
    UInt32                      NewParagraphId;
    MemoryHeap*                 pHeap;
   
    // Default text format used for heap-correct format allocation.
    TextFormat                  EntryTextFormat;
    UInt8                       Flags;

protected:
    void Remove(TextFormat*);
    void Remove(ParagraphFormat*);
    Allocator* _GetThis() { return this; }

public:
    Allocator(MemoryHeap* pheap, UInt8 flags = 0)
      : TextFormatStorageCap(InitialFormatCacheCap),ParagraphFormatStorageCap(InitialFormatCacheCap), 
        NewParagraphId(1), pHeap(pheap), EntryTextFormat(pheap), Flags(flags) {}
    ~Allocator() { SF_ASSERT(1); }

    MemoryHeap* GetHeap() const { return pHeap; }

    wchar_t* AllocText(UPInt numChar)
    {
        return (wchar_t*)SF_HEAP_ALLOC(pHeap, numChar * sizeof(wchar_t), StatRender_Text_Mem);
    }
    wchar_t* ReallocText(wchar_t* pstr, UPInt oldLength, UPInt newLength)
    {
        SF_UNUSED(oldLength);
        if (!pstr) return AllocText(newLength);
        SF_ASSERT(Memory::GetHeapByAddress(pstr) == GetHeap());
        return (wchar_t*)SF_REALLOC(pstr, newLength*sizeof(wchar_t), StatRender_Text_Mem);
    }
    void FreeText(wchar_t* pstr)
    {
        SF_HEAP_FREE(pHeap, pstr);
    }
    void* AllocRaw(UPInt numBytes)
    {
        return SF_HEAP_ALLOC(pHeap, numBytes, StatRender_Text_Mem);
    }
    void FreeRaw(void* pstr)
    {
        SF_HEAP_FREE(pHeap, pstr);
    }

    TextFormat* AllocateTextFormat(const TextFormat& srcfmt);

    TextFormat* AllocateDefaultTextFormat()
    {
        return AllocateTextFormat(EntryTextFormat);
    }

    ParagraphFormat* AllocateParagraphFormat(const ParagraphFormat& srcfmt);

    // Flushes text format cache. Returns 'true' if cache was not empty.
    // Set 'noAllocationsAllowed' to true if need to prevent new allocations
    // (for example, when calling from OnExceedLimit handler).
    bool FlushTextFormatCache(bool noAllocationsAllowed = false);

    // Flushes paragraph format cache. Returns 'true' if cache was not empty.
    // Set 'noAllocationsAllowed' to true if need to prevent new allocations
    // (for example, when calling from OnExceedLimit handler).
    bool FlushParagraphFormatCache(bool noAllocationsAllowed = false);

    struct TextFormatVisitor
    {
        virtual ~TextFormatVisitor() {}
        // return 'true' to keep the entry in the cache;
        // 'false' otherwise.
        virtual bool Visit(const TextFormat&) = 0;
    };
    void VisitTextFormatCache(TextFormatVisitor&);

    Paragraph* AllocateParagraph();
    Paragraph* AllocateParagraph(const Paragraph& srcPara);

    UInt32  AllocateParagraphId() { return NewParagraphId++; }
    UInt32  GetNextParagraphId()  { return NewParagraphId; }
};

template <typename T>
struct CachedPrimValue
{
    mutable T       Value;
    mutable UInt16  FormatCounter;

    CachedPrimValue():FormatCounter(0) {}
    CachedPrimValue(T v):Value(v), FormatCounter(0) {}
    CachedPrimValue(T v, UInt16 counter):Value(v), FormatCounter(counter) {}

    void SetValue(T v, UInt16 counter)
    {
        Value = v; FormatCounter = counter;
    }
    operator T() const { return Value; }
    bool IsValid(UInt16 fmtCnt)
    {
        return FormatCounter == fmtCnt;
    }
};

template <typename T>
struct CachedValue
{
    mutable T       Value;
    mutable UInt16  FormatCounter;

    CachedValue():FormatCounter(0) {}
    CachedValue(const T& v):Value(v), FormatCounter(0) {}
    CachedValue(const T& v, UInt16 counter):Value(v), FormatCounter(counter) {}

    void SetValue(const T& v, UInt16 counter)
    {
        Value = v; FormatCounter = counter;
    }
    T& GetValue() const { return Value; }
    T& operator*() const { return Value; }
    T* operator->() const { return &Value; }
    //operator T&() const { return Value; }
    bool IsValid(UInt16 fmtCnt)
    {
        return FormatCounter == fmtCnt;
    }
    void Invalidate() { --FormatCounter; }
};
}}} // Scaleform::Render::Text

#endif //INC_Render_TEXT_TEXTCORE_H
