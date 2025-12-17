/**************************************************************************

Filename    :   Text_Core.cpp
Content     :   Core text definitions
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Render/Text/Text_Core.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { 
namespace Render {
namespace Text {

UPInt HTMLImageTagDesc::GetHash() const
{
    UPInt v[] = { 0, 0, 0, 0};
    v[0] = VSpace;
    v[1] = HSpace;
    v[2] = ParaId;
    v[3] = Alignment;
    UPInt hash = FixedSizeHash<int>::SDBM_Hash(&v, sizeof(v));
    hash ^= String::BernsteinHashFunctionCIS(Url, Url.GetSize());  
    hash ^= String::BernsteinHashFunctionCIS(Id.ToCStr(), Id.GetSize());
    return hash;
}

//////////////////////////////////
// TextFormat
//
TextFormat::TextFormat(MemoryHeap* pheap)
: pAllocator(NULL), RefCount(1), FontList((pheap)?pheap:Memory::GetGlobalHeap()), 
Url((pheap) ? pheap : Memory::GetGlobalHeap()),
ColorV(0xFF000000u), LetterSpacing(0), FontSize(0), FormatFlags(0), PresentMask(0) 
{ 
}

TextFormat::TextFormat(Allocator* allocator)
: pAllocator(allocator), RefCount(1), FontList((allocator)?allocator->GetHeap() : Memory::GetGlobalHeap()), 
Url((allocator)?allocator->GetHeap() : Memory::GetGlobalHeap()),
ColorV(0xFF000000u), LetterSpacing(0), FontSize(0), FormatFlags(0), PresentMask(0) 
{ 
}

TextFormat::TextFormat(const TextFormat& srcfmt, Allocator* allocator)
: pAllocator(allocator), RefCount(1), FontList(srcfmt.GetHeap(), srcfmt.FontList),
Url(srcfmt.GetHeap(), srcfmt.Url),
pImageDesc(srcfmt.pImageDesc), pFontHandle(srcfmt.pFontHandle),
ColorV(srcfmt.ColorV), LetterSpacing(srcfmt.LetterSpacing),
FontSize(srcfmt.FontSize), FormatFlags(srcfmt.FormatFlags), PresentMask(srcfmt.PresentMask)
{
}

TextFormat::~TextFormat() 
{
    //SF_ASSERT(!pFontHandle || !pFontHandle->GetFont() || pFontHandle->GetFont()->GetFont());
    if (pAllocator)
        pAllocator->Remove(this);
}

void TextFormat::SetFontList(const String& fontList)
{
    if (IsFontHandleSet())
    {
        // if font handle is set and name being set is different - reset the handle
        if (FontList.GetLength() != fontList.GetLength() || 
            String::CompareNoCase(FontList.ToCStr(), fontList.ToCStr()) != 0)
        {
            ClearFontHandle();
        }
    }
    FontList = fontList;
    PresentMask |= PresentMask_FontList; 

    if (!SFstrchr(FontList, ','))
        PresentMask |= PresentMask_SingleFontName; 
    else
        PresentMask &= ~PresentMask_SingleFontName; 
}

void TextFormat::SetFontList(const char* pfontList, UPInt  fontListSz)
{
    // CS5 may generate empty font names.
    SF_ASSERT(pfontList);
    SF_DEBUG_WARNING(fontListSz == 0, "Empty font name is set in TextFormat::SetFontList\n");

    if (fontListSz == SF_MAX_UPINT)
        fontListSz = SFstrlen(pfontList);

    if (IsFontHandleSet())
    {
        // if font handle is set and name being set is different - reset the handle
        if (FontList.GetLength() != fontListSz || 
            String::CompareNoCase(FontList.ToCStr(), pfontList, fontListSz) != 0)
        {
            ClearFontHandle();
        }
    }
    FontList.Clear();
    FontList.AppendString(pfontList, fontListSz);
    PresentMask |= PresentMask_FontList; 

    if (!SFstrchr(FontList, ','))
        PresentMask |= PresentMask_SingleFontName; 
    else
        PresentMask &= ~PresentMask_SingleFontName; 
}

void TextFormat::SetFontList(const wchar_t* pfontList, UPInt fontListSz)
{
    // CS5 may generate empty font names.
    SF_ASSERT(pfontList);
    SF_DEBUG_WARNING(fontListSz == 0, "Empty font name is set in TextFormat::SetFontList\n");

    if (fontListSz == SF_MAX_UPINT)
        fontListSz = SFwcslen(pfontList);

    if (IsFontHandleSet())
    {
        // if font handle is set and name being set is different - reset the handle
        if (FontList.GetLength() == fontListSz)
        {
            for (UPInt i = 0; i < fontListSz; ++i)
            {
                if (SFtowlower(FontList[i]) != SFtowlower(pfontList[i]))
                {
                    ClearFontHandle();
                    break;
                }
            }
        }
        else
            ClearFontHandle();
    }
    FontList.Clear();
    FontList.AppendString(pfontList, fontListSz);
    PresentMask |= PresentMask_FontList; 
    
    if (!SFstrchr(FontList, ','))
        PresentMask |= PresentMask_SingleFontName; 
    else
        PresentMask &= ~PresentMask_SingleFontName; 
}

const String& TextFormat::GetFontList() const    
{
    static const String emptyStr;
    return (IsFontListSet()) ? (const String&)FontList : emptyStr; 
}

void TextFormat::SetFontName(const String& fontName)
{
#ifdef SF_BUILD_DEBUG
    for (UPInt i = 0; i < fontName.GetSize(); i++)
        SF_ASSERT(fontName[i] != ',');
#endif
    SetFontList(fontName);
}

void TextFormat::SetFontName(const char* pfontName, UPInt  fontNameSz)
{
    // CS5 may generate empty font names.
    SF_ASSERT(pfontName);
    SF_DEBUG_WARNING(fontNameSz == 0, "Empty font name is set in TextFormat::SetFontName\n");

    if (fontNameSz == SF_MAX_UPINT)
        fontNameSz = SFstrlen(pfontName);
#ifdef SF_BUILD_DEBUG
    for (UPInt i = 0; i < fontNameSz; i++)
        SF_ASSERT(pfontName[i] != ',');
#endif
    SetFontList(pfontName, fontNameSz);
}

void TextFormat::SetFontName(const wchar_t* pfontName, UPInt  fontNameSz)
{
    // CS5 may generate empty font names.
    SF_ASSERT(pfontName);
    SF_DEBUG_WARNING(fontNameSz == 0, "Empty font name is set in TextFormat::SetFontName\n");

    if (fontNameSz == SF_MAX_UPINT)
        fontNameSz = SFwcslen(pfontName);
#ifdef SF_BUILD_DEBUG
    for (UPInt i = 0; i < fontNameSz; i++)
        SF_ASSERT(pfontName[i] != ',');
#endif
    SetFontList(pfontName, fontNameSz);
}

void TextFormat::SetFontHandle(FontHandle* pfontHandle) 
{ 
    pFontHandle = pfontHandle;
    PresentMask |= PresentMask_FontHandle; 
}

void TextFormat::SetUrl(const char* purl, UPInt urlSz)
{
    SF_ASSERT(purl && urlSz > 0);
    if (urlSz == SF_MAX_UPINT)
        urlSz = SFstrlen(purl);
    Url.Clear();
    Url.AppendString(purl, urlSz);
    PresentMask |= PresentMask_Url; 
}

void TextFormat::SetUrl(const wchar_t* purl, UPInt urlSz)
{
    SF_ASSERT(purl && urlSz > 0);
    if (urlSz == SF_MAX_UPINT)
        urlSz = SFwcslen(purl);
    Url.Clear();
    Url.AppendString(purl, urlSz);
    PresentMask |= PresentMask_Url; 
}

void TextFormat::SetUrl(const String& url)
{
    Url = url;
    PresentMask |= PresentMask_Url; 
}


void TextFormat::SetBold(bool bold)
{
    if (IsFontHandleSet())
    {
        // if font handle is set and boldness being set is different - reset the handle
        if (IsBold() != bold)
            ClearFontHandle();
    }
    if (bold) 
    {
        FormatFlags |= Format_Bold; 
    }
    else
    {
        FormatFlags &= ~Format_Bold; 
    }
    PresentMask |= PresentMask_Bold; 
}

void TextFormat::SetItalic(bool italic)
{
    if (IsFontHandleSet())
    {
        // if font handle is set and italicness being set is different - reset the handle
        if (IsItalic() != italic)
            ClearFontHandle();
    }
    if (italic) 
    {
        FormatFlags |= Format_Italic; 
    }
    else
    {
        FormatFlags &= ~Format_Italic; 
    }
    PresentMask |= PresentMask_Italic; 
}

void TextFormat::SetUnderline(bool underline)
{
    if (underline) 
    {
        FormatFlags |= Format_Underline; 
    }
    else
    {
        FormatFlags &= ~Format_Underline; 
    }
    PresentMask |= PresentMask_Underline; 
}

void TextFormat::SetKerning(bool kerning)
{
    if (kerning) 
    {
        FormatFlags |= Format_Kerning; 
    }
    else
    {
        FormatFlags &= ~Format_Kerning; 
    }
    PresentMask |= PresentMask_Kerning; 
}

bool TextFormat::IsFontSame(const TextFormat& fmt) const 
{ 
    return (((IsFontListSet() && fmt.IsFontListSet() && FontList.CompareNoCase(fmt.FontList) == 0) ||
        (IsFontHandleSet() && fmt.IsFontHandleSet() && pFontHandle == fmt.pFontHandle)) &&
        IsBold() == fmt.IsBold() && IsItalic() == fmt.IsItalic()); 
}

bool TextFormat::IsHTMLFontTagSame(const TextFormat& fmt) const 
{ 
    return (((IsFontListSet() && fmt.IsFontListSet() && FontList.CompareNoCase(fmt.FontList) == 0) ||
        (IsFontHandleSet() && fmt.IsFontHandleSet() && pFontHandle == fmt.pFontHandle)) && 
        (GetColor32() & 0xFFFFFFu) == (fmt.GetColor32() & 0xFFFFFFu) && GetAlpha() == fmt.GetAlpha() &&
        GetFontSize() == fmt.GetFontSize() && 
        IsKerning() == fmt.IsKerning() && GetLetterSpacing() == fmt.GetLetterSpacing()); 
}

TextFormat TextFormat::Merge(const TextFormat& fmt) const
{
    TextFormat result(*this);
    if (fmt.IsBoldSet())
        result.SetBold(fmt.IsBold());
    if (fmt.IsItalicSet())
        result.SetItalic(fmt.IsItalic());
    if (fmt.IsUnderlineSet())
        result.SetUnderline(fmt.IsUnderline());
    if (fmt.IsKerningSet())
        result.SetKerning(fmt.IsKerning());
    if (fmt.IsColorSet())
        result.SetColor(fmt.GetColor());
    if (fmt.IsAlphaSet())
        result.SetAlpha(fmt.GetAlpha());
    if (fmt.IsLetterSpacingSet())
        result.SetLetterSpacingInFixp(fmt.GetLetterSpacingInFixp()); // avoid extra conversions
    if (fmt.IsFontSizeSet())
        result.SetFontSizeInFixp(fmt.GetFontSizeInFixp());
    if (fmt.IsFontListSet())
        result.SetFontList(fmt.GetFontList());
    if (fmt.IsFontHandleSet())
        result.SetFontHandle(fmt.GetFontHandle());
    if (fmt.IsUrlCleared())
        result.ClearUrl();
    else if (fmt.IsUrlSet())
        result.SetUrl(fmt.GetUrl());
    if (fmt.IsImageDescSet())
        result.SetImageDesc(fmt.GetImageDesc());
    return result;
}

TextFormat TextFormat::Intersection(const TextFormat& fmt) const
{
    TextFormat result(fmt.GetHeap());
    if (IsBoldSet() && fmt.IsBoldSet() && IsBold() == fmt.IsBold())
        result.SetBold(fmt.IsBold());
    if (IsItalicSet() && fmt.IsItalicSet() && IsItalic() == fmt.IsItalic())
        result.SetItalic(fmt.IsItalic());
    if (IsUnderlineSet() && fmt.IsUnderlineSet() && IsUnderline() == fmt.IsUnderline())
        result.SetUnderline(fmt.IsUnderline());
    if (IsKerningSet() && fmt.IsKerningSet() && IsKerning() == fmt.IsKerning())
        result.SetKerning(fmt.IsKerning());
    if (IsColorSet() && fmt.IsColorSet() && GetColor() == fmt.GetColor())
        result.SetColor(fmt.GetColor());
    if (IsAlphaSet() && fmt.IsAlphaSet() && GetAlpha() == fmt.GetAlpha())
        result.SetAlpha(fmt.GetAlpha());
    if (IsLetterSpacingSet() && fmt.IsLetterSpacingSet() && GetLetterSpacing() == fmt.GetLetterSpacing())
        result.SetLetterSpacingInFixp(fmt.GetLetterSpacingInFixp()); // avoid extra conversions
    if (IsFontSizeSet() && fmt.IsFontSizeSet() && GetFontSizeInFixp() == fmt.GetFontSizeInFixp())
        result.SetFontSizeInFixp(fmt.GetFontSizeInFixp());
    if (IsFontListSet() && fmt.IsFontListSet() && FontList.CompareNoCase(fmt.FontList) == 0)
        result.SetFontList(fmt.GetFontList());
    if (IsFontHandleSet() && fmt.IsFontHandleSet() && GetFontHandle() == fmt.GetFontHandle())
        result.SetFontHandle(fmt.GetFontHandle());
    if (IsUrlSet() && fmt.IsUrlSet() && Url.CompareNoCase(fmt.Url) == 0)
        result.SetUrl(fmt.GetUrl());
    if (IsImageDescSet() && fmt.IsImageDescSet() && GetImageDesc() == fmt.GetImageDesc())
        result.SetImageDesc(fmt.GetImageDesc());
    return result;
}

void TextFormat::InitByDefaultValues()
{
    SetColor32(0);
    SetFontList("Times New Roman");
    SetFontSize(12);
    SetBold(false);
    SetItalic(false);
    SetUnderline(false);
    SetKerning(false);
    ClearAlpha();
    ClearLetterSpacing();
    ClearUrl();

}

UPInt TextFormat::HashFunctor::CalcHash(const TextFormat& data)
{
    union LetterSpacingUnion
    {
        UPInt AsInt;
        float AsFloat;
    };
    UPInt v[] = { 0, 0, 0, 0, 0};
    if (data.IsColorSet() || data.IsAlphaSet())
    {
        v[0] = data.ColorV;
    }
    LetterSpacingUnion letterSpacing;
    letterSpacing.AsInt = 0; // just in case float is smaller than UPInt
    letterSpacing.AsFloat = data.LetterSpacing;
    if (data.IsLetterSpacingSet())
    {
        v[1] = letterSpacing.AsInt;
    }
    v[2] = data.FormatFlags;
    if (data.IsFontSizeSet())
    {
        v[2] |= data.FontSize << 8;
    }
    //!AB: do not use font handle ptr for hash code since it might be different
    // for the same font. TextFormat::operator== compares internals of font handle.
    // We will just set a bit: 0 - if font handle is set, 1 - if not.
    //if (data.IsFontHandleSet())
    //    v[2] = (UPInt)data.pFontHandle.GetPtr();
    if (data.pFontHandle.GetPtr())
    {
        v[2] |= UInt32(1) << 24; 
    }

    v[3] = data.PresentMask;

    if (data.IsImageDescSet() && data.pImageDesc)
    {
        v[4] = data.pImageDesc->GetHash();
    }
    UPInt hash = FixedSizeHash<int>::SDBM_Hash(&v, sizeof(v));
    if (data.IsFontListSet())
    {
        hash ^= String::BernsteinHashFunctionCIS(data.FontList, data.FontList.GetSize());  
    }
    if (data.IsUrlSet())
    {
        hash ^= String::BernsteinHashFunctionCIS(data.Url.ToCStr(), data.Url.GetSize());
    }
    return hash;
}

//////////////////////////////////
// ParagraphFormat
//
ParagraphFormat::~ParagraphFormat()
{
    if (pAllocator)
        pAllocator->Remove(this);
    FreeTabStops();
}

ParagraphFormat ParagraphFormat::Merge(const ParagraphFormat& fmt) const
{
    ParagraphFormat result(*this);
    if (fmt.IsAlignmentSet())
        result.SetAlignment(fmt.GetAlignment());
    if (fmt.IsBulletSet())
        result.SetBullet(fmt.IsBullet());
    if (fmt.IsBlockIndentSet())
        result.SetBlockIndent(fmt.GetBlockIndent());
    if (fmt.IsIndentSet())
        result.SetIndent(fmt.GetIndent());
    if (fmt.IsLeadingSet())
        result.SetLeading(fmt.GetLeading());
    if (fmt.IsLeftMarginSet())
        result.SetLeftMargin(fmt.GetLeftMargin());
    if (fmt.IsRightMarginSet())
        result.SetRightMargin(fmt.GetRightMargin());
    if (fmt.IsTabStopsSet())
        result.SetTabStops(fmt.GetTabStops());
    if (fmt.IsDisplaySet())
        result.SetDisplay(fmt.GetDisplay());
    return result;
}

ParagraphFormat ParagraphFormat::Intersection(const ParagraphFormat& fmt) const
{
    ParagraphFormat result;
    if (IsAlignmentSet() && fmt.IsAlignmentSet() && GetAlignment() == fmt.GetAlignment())
        result.SetAlignment(fmt.GetAlignment());
    if (IsBulletSet() && fmt.IsBulletSet() && IsBullet() == fmt.IsBullet())
        result.SetBullet(fmt.IsBullet());
    if (IsBlockIndentSet() && fmt.IsBlockIndentSet() && GetBlockIndent() == fmt.GetBlockIndent())
        result.SetBlockIndent(fmt.GetBlockIndent());
    if (IsIndentSet() && fmt.IsIndentSet() && GetIndent() == fmt.GetIndent())
        result.SetIndent(fmt.GetIndent());
    if (IsLeadingSet() && fmt.IsLeadingSet() && GetLeading() == fmt.GetLeading())
        result.SetLeading(fmt.GetLeading());
    if (IsLeftMarginSet() && fmt.IsLeftMarginSet() && GetLeftMargin() == fmt.GetLeftMargin())
        result.SetLeftMargin(fmt.GetLeftMargin());
    if (IsRightMarginSet() && fmt.IsRightMarginSet() && GetRightMargin() == fmt.GetRightMargin())
        result.SetRightMargin(fmt.GetRightMargin());
    if (IsTabStopsSet() && fmt.IsTabStopsSet() && TabStopsEqual(fmt.GetTabStops()))
        result.SetTabStops(fmt.GetTabStops());
    if (IsDisplaySet() && fmt.IsDisplaySet() && GetDisplay() == fmt.GetDisplay())
        result.SetDisplay(fmt.GetDisplay());
    return result;
}

void ParagraphFormat::InitByDefaultValues()
{
    SetAlignment(Align_Left);
    ClearBlockIndent();
    ClearBullet();
    ClearIndent();
    ClearLeading();
    ClearLeftMargin();
    ClearRightMargin();
    ClearTabStops();
}

void ParagraphFormat::SetTabStops(unsigned num, ...)
{
    if (num > 0)
    {
        if (!pTabStops || pTabStops[0] != num)
        {
            FreeTabStops();
            AllocTabStops(num);
        }
        va_list vl;
        va_start(vl, num);

        for (unsigned i = 0; i < num; ++i)
        {
            unsigned arg = va_arg(vl, unsigned);
            pTabStops[i + 1] = arg;
        }
        va_end(vl);
        PresentMask |= PresentMask_TabStops;
    }
    else
        ClearTabStops();
}

void  ParagraphFormat::SetTabStops(const unsigned* psrcTabStops)
{
    if (psrcTabStops && psrcTabStops[0] > 0)
    {
        CopyTabStops(psrcTabStops);
        PresentMask |= PresentMask_TabStops;
    }
    else
        ClearTabStops();
}

const unsigned* ParagraphFormat::GetTabStops(unsigned* pnum) const
{
    if (pTabStops)
    {
        unsigned cnt = pTabStops[0];
        if (pnum)
            *pnum = cnt;
        return pTabStops + 1;
    }
    return NULL;
}

void ParagraphFormat::SetTabStopsElement(unsigned idx, unsigned val)
{
    if (pTabStops && idx < pTabStops[0])
    {
        pTabStops[idx + 1] = val;
    }
}

void ParagraphFormat::AllocTabStops(unsigned num)
{
    FreeTabStops();
    pTabStops = (unsigned*)SF_ALLOC((num + 1) * sizeof(unsigned), StatRender_Text_Mem);
    pTabStops[0] = num;
}

void ParagraphFormat::FreeTabStops()
{
    SF_FREE(pTabStops);
    pTabStops = NULL;
}

bool ParagraphFormat::TabStopsEqual(const unsigned* psrcTabStops) const
{
    if (pTabStops == psrcTabStops)
        return true;
    if (pTabStops && psrcTabStops)
    {
        unsigned c1 = pTabStops[0];
        unsigned c2 = psrcTabStops[0];
        if (c1 == c2)
        {
            return (memcmp(pTabStops + 1, psrcTabStops + 1, sizeof(unsigned) * c1) == 0);
        }
    }
    return false;
}

void ParagraphFormat::CopyTabStops(const unsigned* psrcTabStops)
{
    if (psrcTabStops)
    {
        unsigned n = psrcTabStops[0];
        if (!pTabStops || pTabStops[0] != n)
        {
            AllocTabStops(n);
        }
        memcpy(pTabStops + 1, psrcTabStops + 1, n * sizeof(unsigned));
    }
    else
        FreeTabStops();
}

ParagraphFormat& ParagraphFormat::operator=(const ParagraphFormat& src)
{
    BlockIndent     = src.BlockIndent;
    Indent          = src.Indent;
    Leading         = src.Leading;
    LeftMargin      = src.LeftMargin;
    RightMargin     = src.RightMargin;
    PresentMask     = src.PresentMask;
    CopyTabStops(src.pTabStops);
    return *this;
}

UPInt  ParagraphFormat::HashFunctor::CalcHash(const ParagraphFormat& data)
{
    UPInt hash = 0;
    if (data.IsTabStopsSet() && data.pTabStops)
        hash ^= FixedSizeHash<unsigned>::SDBM_Hash(data.pTabStops, (data.pTabStops[0] + 1) * sizeof(unsigned));
    if (data.IsBlockIndentSet())
        hash ^= data.GetBlockIndent();
    if (data.IsIndentSet())
        hash ^= ((UPInt)data.GetIndent()) << 8;
    if (data.IsLeadingSet())
        hash ^= ((UPInt)data.GetLeading()) << 12;
    if (data.IsLeftMarginSet())
        hash ^= ((UPInt)data.GetLeftMargin()) << 16;
    if (data.IsRightMarginSet())
        hash ^= ((UPInt)data.GetRightMargin()) << 18;
    hash ^= ((((UPInt)data.PresentMask) << 9) | (((UPInt)data.GetAlignment()) << 1) | (UPInt)data.IsBullet());
    hash ^= (((UPInt)data.GetDisplay()) << 10);
    return hash;
}

//////////////////////////////////////////////////////////////////////////
// Allocator
TextFormat* Allocator::AllocateTextFormat(const TextFormat& srcfmt)
{
    if (!srcfmt.IsImageDescSet())
    {
        PtrCompare<TextFormat*>* ppfmt = TextFormatStorage.Get(&srcfmt);
        if (ppfmt)
        {
            (*ppfmt)->AddRef();
            return (*ppfmt);
        }
        // check if we need to flush format cache
        if (TextFormatStorage.GetSize() >= TextFormatStorageCap)
            FlushTextFormatCache();
    }

    TextFormat* pfmt = SF_HEAP_NEW(pHeap) TextFormat(srcfmt, this);
    if (Flags & Flags_Global && pfmt->IsFontHandleSet())
    {
        // do not save font handles, since they might be destroyed eventually.
        pfmt->ClearFontHandle();
    }
    if (!srcfmt.IsImageDescSet())
        TextFormatStorage.Set(pfmt);
    return pfmt;
}

ParagraphFormat* Allocator::AllocateParagraphFormat(const ParagraphFormat& srcfmt)
{
    PtrCompare<ParagraphFormat*>* ppfmt = ParagraphFormatStorage.Get(&srcfmt);
    if (ppfmt)
    {
        (*ppfmt)->AddRef();
        return (*ppfmt);
    }
    // check if we need to flush format cache
    if (ParagraphFormatStorage.GetSize() >= ParagraphFormatStorageCap)
        FlushParagraphFormatCache();

    ParagraphFormat* pfmt = SF_HEAP_NEW(pHeap) ParagraphFormat(srcfmt, this);
    ParagraphFormatStorage.Set(pfmt);
    return pfmt;
}

void Allocator::VisitTextFormatCache(TextFormatVisitor& visitor)
{
    TextFormatStorageType::Iterator it = TextFormatStorage.Begin();
    for (;it != TextFormatStorage.End(); ++it)
    {
        PtrCompare<TextFormat*>& fmt = *it;
        if (!fmt || !visitor.Visit(*fmt))
            it.Remove();
    }
}

bool Allocator::FlushTextFormatCache(bool noAllocationsAllowed)
{
    UPInt tfSize = TextFormatStorage.GetSize();

    // SetCapacity may allocate new memory; if no new allocations are allowed
    // (for example, when Flush... is called from OnExceedLimit handler) then
    // don't set new capacity.
    if (!noAllocationsAllowed)
        TextFormatStorage.SetCapacity(TextFormatStorage.GetSize());
    
    // correct cache cap, if necessary
    if (TextFormatStorage.GetSize() >= TextFormatStorageCap)
        TextFormatStorageCap = unsigned(TextFormatStorage.GetSize() + FormatCacheCapacityDelta);
    else if (TextFormatStorage.GetSize() <= InitialFormatCacheCap)
        TextFormatStorageCap = InitialFormatCacheCap;

    return (tfSize != TextFormatStorage.GetSize());
}

bool Allocator::FlushParagraphFormatCache(bool noAllocationsAllowed)
{
    UPInt pfSize = ParagraphFormatStorage.GetSize();

    // SetCapacity may allocate new memory; if no new allocations are allowed
    // (for example, when Flush... is called from OnExceedLimit handler) then
    // don't set new capacity.
    if (!noAllocationsAllowed)
        ParagraphFormatStorage.SetCapacity(ParagraphFormatStorage.GetSize());

    // correct cache cap, if necessary
    if (ParagraphFormatStorage.GetSize() >= ParagraphFormatStorageCap)
        ParagraphFormatStorageCap = unsigned(ParagraphFormatStorage.GetSize() + FormatCacheCapacityDelta);
    else if (ParagraphFormatStorage.GetSize() <= InitialFormatCacheCap)
        ParagraphFormatStorageCap = InitialFormatCacheCap;

    return (pfSize != ParagraphFormatStorage.GetSize());
}

void Allocator::Remove(TextFormat* pfmt)
{
    TextFormatStorage.Remove(pfmt);
}
void Allocator::Remove(ParagraphFormat* pfmt)
{
    ParagraphFormatStorage.Remove(pfmt);
}

}}} // Scaleform::Render::Text
