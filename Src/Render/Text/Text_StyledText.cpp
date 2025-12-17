/**************************************************************************

Filename    :   Text_StyledText.cpp
Content     :   Styled text implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Kernel/SF_AutoPtr.h"
#include "Kernel/SF_Alg.h"
#include "Render/Text/Text_StyledText.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { 
namespace Render {
namespace Text {

//////////////////////////////////////////////////////////////////////////
// Allocator
//
Paragraph* Allocator::AllocateParagraph()
{
    return SF_HEAP_NEW(pHeap) Paragraph(this);
}

Paragraph* Allocator::AllocateParagraph(const Paragraph& srcPara)
{
    return SF_HEAP_NEW(pHeap) Paragraph(srcPara, this);
}

//////////////////////////////////
// GFxWideStringBuffer
//
Paragraph::TextBuffer::TextBuffer(const TextBuffer& o, Allocator* pallocator)
{
    SF_ASSERT(pallocator);
    pText = pallocator->AllocText(o.Size);
    Allocated = Size = o.Size;
    memcpy(pText, o.pText, sizeof(wchar_t) * o.Size);
}

Paragraph::TextBuffer::~TextBuffer()
{
    // make sure text was freed
    SF_ASSERT(pText == NULL && Size == 0 && Allocated == 0);
}

void Paragraph::TextBuffer::Free(Allocator* pallocator)
{
    pallocator->FreeText(pText);
    pText = NULL;
    Size = Allocated = 0;
}

const wchar_t* Paragraph::TextBuffer::GetCharPtrAt(UPInt pos) const
{
    if (!pText || pos >= Size)
        return NULL;
    return &pText[pos];
}

void Paragraph::TextBuffer::SetString
(Allocator* pallocator, const char* putf8Str, UPInt utf8length)
{
    UPInt len;
    if (utf8length == SF_MAX_UPINT)
        len = UTF8Util::GetLength(putf8Str, SFstrlen(putf8Str));
    else
        len = utf8length;
    if (Allocated < len)
    {
        pText = pallocator->ReallocText(pText, Allocated, len);
        SF_ASSERT(pText);
        Allocated = len;
    }
    if (len > 0)
    {
        SF_ASSERT(pText);
        UTF8Util::DecodeStringSafe(pText, len, putf8Str, utf8length);
    }
    Size = len;
}

void Paragraph::TextBuffer::SetString
(Allocator* pallocator, const wchar_t* pstr, UPInt length)
{
    if (length == SF_MAX_UPINT)
        length = StrLen(pstr);
    if (Allocated < length)
    {
        pText = pallocator->ReallocText(pText, Allocated, length);
        SF_ASSERT(pText);
        Allocated = length;
    }
    if (length > 0)
    {
        SF_ASSERT(pText);
        memcpy(pText, pstr, length * sizeof(wchar_t));
    }
    Size = length;
}

wchar_t* Paragraph::TextBuffer::CreatePosition
(Allocator* pallocator, UPInt pos, UPInt length)
{
    SF_ASSERT(pos <= Size);

    if (Allocated < Size + length)        
    {
        pText = pallocator->ReallocText(pText, Allocated, Size + length);
        SF_ASSERT(pText);
        Allocated = Size + length;
    }
    if (Size - pos > 0)
    {
        SF_ASSERT(pText);
        memmove(pText + pos + length, pText + pos, (Size - pos)*sizeof(wchar_t));
    }
    Size += length;
    SF_ASSERT(pText != NULL || pos == 0);
    return pText + pos;
}

void Paragraph::TextBuffer::Remove(UPInt pos, UPInt length)
{
    if (pos < Size)
    {
        if (pos + length >= Size)
            Size = pos;
        else
        {
            memmove(pText + pos, pText + pos + length, (Size - (pos + length))*sizeof(wchar_t));
            Size -= length;
        }
    }
}

SPInt Paragraph::TextBuffer::StrChr(const wchar_t* ptext, UPInt length, wchar_t c)
{
    for (UPInt i = 0; i < length; ++i)
    {
        SF_ASSERT(ptext);
        if (ptext[i] == c)
            return (SPInt)i;
    }
    return -1;
}

UPInt Paragraph::TextBuffer::StrLen(const wchar_t* ptext)
{
    SF_ASSERT(ptext);
    UPInt i = 0;
    while (ptext[i] != 0)
        ++i;
    return i;
}

void Paragraph::TextBuffer::StripTrailingNewLines()
{
    int len = int(Size);
    // check, is the content already null terminated
    if (len > 0 && pText[len -1] == 0)
        --len; //if yes, skip the '\0'
    for (int i = len - 1; i >= 0 && (pText[i] == '\n' || pText[i] == '\r'); --i)
    {
        --Size;
        pText[i] = 0;
    }
}

void Paragraph::TextBuffer::StripTrailingNull()
{
    if (Size > 0)
    {
        SF_ASSERT(pText);
        if (pText[Size - 1] == '\0')
            --Size;
    }
}

void Paragraph::TextBuffer::AppendTrailingNull(Allocator* pallocator)
{
    if (Size > 0 && Size < Allocated)
    {
        SF_ASSERT(pText);
        pText[Size++] = '\0';
    }
    else
    {
        wchar_t* p = CreatePosition(pallocator, Size, 1); 
        SF_ASSERT(p);
        if (p)
            *p  = '\0';
    }
}

UPInt Paragraph::TextBuffer::GetLength() const
{
    int len = int(Size);
    // check, is the content already null terminated
    if (len > 0 && pText[len - 1] == '\0')
        return Size - 1;

    return Size;
}

//////////////////////////////////
// Paragraph
//
Paragraph::Paragraph(Allocator* ptextAllocator)
 : StartIndex(0), ModCounter(0) 
{
    SF_ASSERT(ptextAllocator);
    // Prevent stack allocation due to heaps.
    SF_ASSERT(Memory::GetHeapByAddress(this) != 0); 
    UniqueId = ptextAllocator->AllocateParagraphId();
}

Paragraph::Paragraph(const Paragraph& o, Allocator* ptextAllocator)
: Text(o.Text, ptextAllocator), 
  FormatInfo(o.FormatInfo), StartIndex(o.StartIndex), ModCounter(0)
{
    SF_ASSERT(ptextAllocator);
    SF_ASSERT(Memory::GetHeapByAddress(this) != 0); 
    UniqueId = ptextAllocator->AllocateParagraphId();
    pFormat = *ptextAllocator->AllocateParagraphFormat(*o.pFormat);
    
    // need to make copies of all textformats, to make sure they are allocated from
    // the correct textAllocator/heap.
    TextFormatArrayType::Iterator it = FormatInfo.Begin();
    for (; !it.IsFinished(); it++)
    {
        TextFormatArrayType::TypedRangeData& rangeData = *it;
        Ptr<TextFormat> pfmt = *ptextAllocator->AllocateTextFormat(*rangeData.GetData());
        rangeData.SetData(pfmt);
    }
}

void Paragraph::Clear()
{
    Text.Clear();
    FormatInfo.RemoveAll();
    ++ModCounter;
}

void Paragraph::SetText(Allocator* pallocator, const wchar_t* pstring, UPInt length)
{
    SF_ASSERT(length <= SF_MAX_UPINT);
    if (length != SF_MAX_UPINT)
    {
        // check if string contains '\0' inside. If yes - correct
        // the length.
        for (SPInt i = (SPInt)length - 1; i >= 0; --i)
        {
            if (pstring[i] == '\0')
            {
                length = (UPInt)i;
                break;
            }
        }
    }
    Text.SetString(pallocator, pstring, length);
    ++ModCounter;
}

void Paragraph::SetTextFormat(Allocator* pallocator, 
                                     const TextFormat& fmt, 
                                     UPInt startPos, 
                                     UPInt endPos)
{
    FormatRunIterator it = GetIteratorAt(startPos);
    if (endPos < startPos) endPos = startPos;
    SPInt length = (endPos == SF_MAX_UPINT) ? SF_MAX_SPINT : endPos - startPos;
    while(length > 0 && !it.IsFinished())
    {
        const StyledTextRun& run = *it;
        const UPInt runIndex = run.Index, runLength = run.Length;
        SPInt curIndex;
        if (startPos > runIndex)
            curIndex = startPos;
        else
            curIndex = runIndex;
        TextFormat format(pallocator->GetHeap());
        Ptr<TextFormat> pfmt;
        if (run.pFormat)
        {
            format = run.pFormat->Merge(fmt);
            pfmt = *pallocator->AllocateTextFormat(format);
        }
        else
        {
            pfmt = *pallocator->AllocateTextFormat(fmt);
        }

        UPInt newLen = Alg::PMin(UPInt(runLength - (curIndex - runIndex)), (UPInt)length);
        FormatInfo.SetRange(curIndex, newLen, pfmt);
        length -= newLen;
        //++it;
        it.SetTextPos(runIndex + runLength);
    }

    //_dump(FormatInfo);

    ++ModCounter;
}

TextFormat Paragraph::GetTextFormat(UPInt startPos, UPInt endPos) const
{
    FormatRunIterator it = GetIteratorAt(startPos);
    if (endPos < startPos) endPos = startPos;
    SPInt length = (endPos == SF_MAX_UPINT) ? SF_MAX_SPINT : endPos - startPos;
    TextFormat finalTextFmt(Memory::GetHeapByAddress(this));
    unsigned i = 0;
    while(length > 0 && !it.IsFinished())
    {
        const StyledTextRun& run = *it;

        if (run.pFormat)
        {
            if (i++ == 0)
                finalTextFmt = *run.pFormat;
            else
                finalTextFmt = run.pFormat->Intersection(finalTextFmt);
        }

        length -= (run.Length - (startPos - run.Index));
        startPos = run.Index + run.Length;
        ++it;
    }
    return finalTextFmt;
}

// returns the actual pointer on text format at the specified position.
// Will return NULL, if there is no text format at the "pos"
TextFormat* Paragraph::GetTextFormatPtr(UPInt startPos) const
{
    FormatRunIterator it = GetIteratorAt(startPos);
    TextFormat* pfmt = NULL;
    if (!it.IsFinished())
    {
        const StyledTextRun& run = *it;

        if (run.pFormat)
        {
            pfmt = run.pFormat;
        }
    }
    return pfmt;
}

void Paragraph::ClearTextFormat(UPInt startPos, UPInt endPos)
{
    FormatRunIterator it = GetIteratorAt(startPos);
    if (endPos < startPos) endPos = startPos;
    SPInt length = (endPos == SF_MAX_UPINT) ? SF_MAX_SPINT : endPos - startPos;
    while(length > 0 && !it.IsFinished())
    {
        const StyledTextRun& run = *it;
        const UPInt runIndex = run.Index, runLength = run.Length;
        SPInt curIndex;
        if (startPos > runIndex)
            curIndex = startPos;
        else
            curIndex = runIndex;

        UPInt newLen = Alg::PMin(UPInt(runLength - (curIndex - runIndex)), (UPInt)length);
        FormatInfo.ClearRange(curIndex, newLen);
        length -= newLen;
        //++it;
        it.SetTextPos(runIndex + runLength);
    }

    //_dump(FormatInfo);

    ++ModCounter;
}


void Paragraph::SetFormat(Allocator* pallocator, const ParagraphFormat& fmt)
{
    Ptr<ParagraphFormat> pfmt;
    if (pFormat)
        pfmt = *pallocator->AllocateParagraphFormat(pFormat->Merge(fmt));
    else
        pfmt = *pallocator->AllocateParagraphFormat(fmt);
    pFormat = pfmt;
    ++ModCounter;
}

void Paragraph::SetFormat(const ParagraphFormat* pfmt)
{
    pFormat = const_cast<ParagraphFormat*>(pfmt);
    ++ModCounter;
}

Paragraph::FormatRunIterator Paragraph::GetIterator() const
{
    return FormatRunIterator(FormatInfo, Text);
}

Paragraph::FormatRunIterator Paragraph::GetIteratorAt(UPInt index) const
{
    return FormatRunIterator(FormatInfo, Text, index);
}

wchar_t* Paragraph::CreatePosition(Allocator* pallocator, UPInt pos, UPInt length)
{
    if (length == 0)
        return NULL;
    wchar_t* p = Text.CreatePosition(pallocator, pos, length);
    SF_ASSERT(p);
    FormatInfo.ExpandRange(pos, length);
    ++ModCounter;
    return p;
}

void Paragraph::SetTermNullFormat()
{
    if (HasTermNull())
    {
        UPInt len = GetLength();
        FormatInfo.ExpandRange(len, 1);
        FormatInfo.RemoveRange(len + 1, 1);
    }
}

void Paragraph::InsertString(Allocator* pallocator, 
                                    const wchar_t* pstr, 
                                    UPInt pos, 
                                    UPInt length, 
                                    const TextFormat* pnewFmt)
{
    if (length > 0)
    {
        if (length == SF_MAX_UPINT)
            length = TextBuffer::StrLen(pstr);
        wchar_t* p = CreatePosition(pallocator, pos, length);
        SF_ASSERT(p);
        if (p)
        {
            memcpy(p, pstr, length * sizeof(wchar_t));
            if (pnewFmt)
            {
                //if (HasTermNull() && pos + length == GetLength())
                //    ++length; // need to expand format info to the termination null symbol
                FormatInfo.SetRange(pos, length, const_cast<TextFormat*>(pnewFmt));
            }

            SetTermNullFormat();
            ++ModCounter;
        }
        //_dump(FormatInfo);
    }
}

void Paragraph::AppendPlainText(Allocator* pallocator, const wchar_t* pstr, UPInt length)
{
    if (length > 0)
    {
        if (length == SF_MAX_UPINT)
            length = TextBuffer::StrLen(pstr);
        wchar_t* p = CreatePosition(pallocator, GetLength(), length);
        SF_ASSERT(p);

        if (p)
        {
            memcpy(p, pstr, length * sizeof(wchar_t));
            ++ModCounter;
        }
        //_dump(FormatInfo);
    }
}

void Paragraph::AppendPlainText(Allocator* pallocator, const char* putf8str, UPInt utf8StrSize)
{
    if (utf8StrSize > 0)
    {
        UPInt length;
        if (utf8StrSize == SF_MAX_UPINT)
            length = (UPInt)UTF8Util::GetLength(putf8str);
        else
            length = (UPInt)UTF8Util::GetLength(putf8str, (SPInt)utf8StrSize);
        wchar_t* p = CreatePosition(pallocator, GetLength(), length);
        SF_ASSERT(p);

        if (p)
        {
            UTF8Util::DecodeStringSafe(p, length, putf8str, utf8StrSize);
            ++ModCounter;
        }
        //_dump(FormatInfo);
    }
}

void Paragraph::AppendTermNull(Allocator* pallocator, const TextFormat* pdefTextFmt)
{
    if (!HasTermNull())
    {
        UPInt pos = GetLength();
        wchar_t* p = CreatePosition(pallocator, pos, 1);
        SF_ASSERT(p);
        if (p)
        {
            *p = '\0';
            SF_ASSERT(FormatInfo.Count() || pdefTextFmt);
            if (FormatInfo.Count() == 0 && pdefTextFmt)
            {
                Ptr<TextFormat> pfmt = *pallocator->AllocateTextFormat(*pdefTextFmt);
                FormatInfo.SetRange(pos, 1, pfmt);
            }
        }
    }
}

void Paragraph::RemoveTermNull()
{
    if (HasTermNull())
    {
        FormatInfo.RemoveRange(GetLength(), 1);
        Text.StripTrailingNull();
    }
}

bool Paragraph::HasTermNull() const
{
    UPInt l = Text.GetSize();
    return (l > 0 && *Text.GetCharPtrAt(l - 1) == '\0');
}

bool Paragraph::HasNewLine() const
{
    UPInt l = Text.GetSize();
    if (l > 0)
    {
        wchar_t c = *Text.GetCharPtrAt(l - 1);
        return (c == '\r' || c == '\n');
    }
    return false;
}

// returns true if paragraph is empty (no chars or only termination null)
UPInt Paragraph::GetLength() const
{
    UPInt len = GetSize();
    if (len > 0 && HasTermNull())
        --len;
    return len;
}

void Paragraph::Remove(UPInt startPos, UPInt endPos)
{
    SF_ASSERT(endPos >= startPos);
    UPInt length = (endPos == SF_MAX_UPINT) ? SF_MAX_UPINT : (UPInt)(endPos - startPos);
    if (length > 0)
    {
        Text.Remove(startPos, length);

        //_dump(FormatInfo);
        FormatInfo.RemoveRange(startPos, length);
        SetTermNullFormat();
        //_dump(FormatInfo);

        ++ModCounter;
    }
}

// copies text and formatting from psrcPara paragraph to this one, starting from startSrcIndex in
// source paragraph and startDestIndex in destination one. The "length" specifies the number
// of positions to copy.
void Paragraph::Copy(Allocator* pallocator, 
                            const Paragraph& psrcPara, 
                            UPInt startSrcIndex, 
                            UPInt startDestIndex, 
                            UPInt length)
{
    if (length > 0)
    {
        InsertString(pallocator, psrcPara.GetText() + startSrcIndex, startDestIndex, length);
        // copy format info
        FormatRunIterator fmtIt = psrcPara.GetIteratorAt(startSrcIndex);
        UPInt remainingLen = length;
        for(; !fmtIt.IsFinished() && remainingLen > 0; ++fmtIt)
        {
            const Paragraph::StyledTextRun& run = *fmtIt;

            SPInt idx;
            UPInt len;
            if (run.Index < SPInt(startSrcIndex))
            {
                idx = 0;
                len = run.Length - (startSrcIndex - run.Index);
            }
            else
            {
                idx = run.Index - startSrcIndex;
                len = run.Length;
            }
            len = Alg::PMin(len, remainingLen);
            if (run.pFormat)
            {
                // need to make a copy of pFormat to make sure it is allocated
                // from the correct textAllocator/heap
                Ptr<TextFormat> pfmt = *pallocator->AllocateTextFormat(*run.pFormat);
                FormatInfo.SetRange(startDestIndex + idx, len, pfmt);
            }
            remainingLen -= len;
        }
        SetTermNullFormat();
        ++ModCounter;
    }
}

// Shrinks the paragraph's length by the "delta" value.
void Paragraph::Shrink(UPInt delta)
{
    UPInt len = Text.GetSize();
    delta = Alg::PMin(delta, len);
    Remove(len - delta, len);
}

#ifdef SF_BUILD_DEBUG
void Paragraph::CheckIntegrity() const
{
    const_cast<Paragraph*>(this)->FormatInfo.CheckIntegrity();

    // check the size and length are correctly set
    if (GetLength() != GetSize())
        SF_ASSERT(HasTermNull());
    else
        SF_ASSERT(!HasTermNull());

    // check if the null or newline chars are inside of the paragraph. 
    // Should be only at the end of it.
    for(UPInt i = 0, n = GetLength(); i < n; ++i)
    {
        SF_ASSERT(Text.GetCharPtrAt(i));
        wchar_t c = *Text.GetCharPtrAt(i);
        SF_ASSERT(c != 0);
        if (i + 1 < n)
            SF_ASSERT(c != '\r' && c != '\n');
    }

    // also need to verify there are no term null and \n - \r symbols together
    for(UPInt i = 0, n = GetSize(), m = 0; i < n; ++i)
    {
        SF_ASSERT(Text.GetCharPtrAt(i));
        wchar_t c = *Text.GetCharPtrAt(i);
        if (c == '\r')      { SF_ASSERT(m == 0); m |= 1; }
        else if (c == '\n') { SF_ASSERT(m == 0); m |= 2; }
        else if (c == 0)    { SF_ASSERT(m == 0); m |= 4; }

        // also check, that every char is covered by format data
        SF_ASSERT(GetTextFormatPtr(i));
    }

    // ensure, format info range array does not exceed the text size
    TextFormatArrayType::Iterator it = const_cast<Paragraph*>(this)->FormatInfo.Last();
    if (!it.IsFinished())
    {
        TextFormatRunType& r = *it; 
        SPInt ni = r.NextIndex();
        UPInt sz = GetSize();
        SF_ASSERT(ni >= 0 && ((UPInt)ni) <= sz);
    }

}
#endif

//////////////////////////////////////////////////////////////////////////
// Paragraphs' iterators
Paragraph::FormatRunIterator::FormatRunIterator(const TextFormatArrayType& fmts, const TextBuffer& textHandle) 
:   pFormatInfo(&fmts), FormatIterator(fmts.Begin()), pText(&textHandle),  
    CurTextIndex(0)
{
}

Paragraph::FormatRunIterator::FormatRunIterator
    (const TextFormatArrayType& fmts, const TextBuffer& textHandle, UPInt index) 
:   pFormatInfo(&fmts), FormatIterator(pFormatInfo->GetIteratorByNearestIndex(index)),
    pText(&textHandle), CurTextIndex(0)
{
    if (!FormatIterator.IsFinished())
    {
        if (!FormatIterator->Contains((SPInt)index))
        {
            if ((SPInt)index > FormatIterator->Index)
            {
                CurTextIndex = (UPInt)FormatIterator->Index;
                CurTextIndex += FormatIterator->Length;
                ++FormatIterator;
            }
        }
        else
        {
            CurTextIndex = (UPInt)FormatIterator->Index;    
        }
    }
}
Paragraph::FormatRunIterator::FormatRunIterator(const FormatRunIterator& orig) 
:   pFormatInfo(orig.pFormatInfo), FormatIterator(orig.FormatIterator), pText(orig.pText),
    CurTextIndex(orig.CurTextIndex)
{
}

Paragraph::FormatRunIterator& 
Paragraph::FormatRunIterator::operator=(const Paragraph::FormatRunIterator& orig)
{
    FormatIterator    = orig.FormatIterator;
    pFormatInfo       = orig.pFormatInfo;
    pText             = orig.pText;
    CurTextIndex      = orig.CurTextIndex;
    return *this;
}

const Paragraph::StyledTextRun& Paragraph::FormatRunIterator::operator* () 
{ 
    if (!FormatIterator.IsFinished())
    {
        const TextFormatRunType& fmtRange = *FormatIterator; //(*pFormatInfo)[CurFormatIndex];
        SF_ASSERT(fmtRange.Index >= 0);
        if (CurTextIndex < (UPInt)fmtRange.Index)
        {
            // if index in text is lower than current range index - 
            // just return a text w/o format.

            // determine the length of text as fmtRange.Index - CurTextIndex
            return PlaceHolder.Set(pText->ToWStr() + CurTextIndex, 
                                   CurTextIndex, fmtRange.Index - CurTextIndex, NULL);
        }
        else
        {
            SF_ASSERT(fmtRange.Index >= 0 && UPInt(fmtRange.Index) == CurTextIndex);
            return PlaceHolder.Set(pText->ToWStr() + fmtRange.Index, 
                fmtRange.Index, fmtRange.Length, fmtRange.GetData());
        }
    }
    return PlaceHolder.Set(pText->ToWStr() + CurTextIndex, CurTextIndex, pText->GetSize() - CurTextIndex, NULL); 
}

void Paragraph::FormatRunIterator::operator++() 
{ 
    //if (CurFormatIndex < pFormatInfo->Count())
    if (!FormatIterator.IsFinished())
    {
        const TextFormatRunType& fmtRange = *FormatIterator;
        SF_ASSERT(fmtRange.Index >= 0);
        if (CurTextIndex < UPInt(fmtRange.Index))
        {
            CurTextIndex = fmtRange.Index;
        }
        else
        {
            SF_ASSERT(UPInt(fmtRange.Index) == CurTextIndex);
            CurTextIndex += fmtRange.Length;
            //++CurFormatIndex;
            ++FormatIterator;
        }
    }
    else
        CurTextIndex = pText->GetSize();
}

void Paragraph::FormatRunIterator::SetTextPos(UPInt newTextPos)
{
    while(!IsFinished())
    {
        const Paragraph::StyledTextRun& stRun = *(*this); 
        //SF_ASSERT(fmtRange.Index >= 0);
        if (stRun.Index >= (SPInt)newTextPos)
            break;
        operator++();
    }
}

//////////////////////////////////////////////////////////////////////////
Paragraph::CharactersIterator::CharactersIterator(const Paragraph* pparagraph) : 
pFormatInfo(&pparagraph->FormatInfo), FormatIterator(pFormatInfo->Begin()), pText(&pparagraph->Text),  
CurTextIndex(0)
{
}

Paragraph::CharactersIterator::CharactersIterator(const Paragraph* pparagraph, UPInt index) : 
pFormatInfo(&pparagraph->FormatInfo), FormatIterator(pFormatInfo->GetIteratorByNearestIndex(index)),
pText(&pparagraph->Text), CurTextIndex(index)
{
    if (!FormatIterator.IsFinished())
    {
        if (!FormatIterator->Contains(index) && (int)index > FormatIterator->Index)
        {
            ++FormatIterator;
        }
    }
}

Paragraph::CharacterInfo& Paragraph::CharactersIterator::operator*() 
{ 
    if (!IsFinished())
    {
        PlaceHolder.Character = *(pText->ToWStr() + CurTextIndex);
        PlaceHolder.Index     = CurTextIndex;
        if (!FormatIterator.IsFinished())
        {
            const TextFormatRunType& fmtRange = *FormatIterator; 
            SF_ASSERT(fmtRange.Index >= 0);
            if (CurTextIndex < (UPInt)fmtRange.Index)
            {
                // if index in text is lower than current range index - 
                // just return a text w/o format.
                PlaceHolder.pFormat = NULL;
            }
            else
            {
                SF_ASSERT(fmtRange.Contains(CurTextIndex));
                PlaceHolder.pFormat = fmtRange.GetData();
            }
        }
        else
            PlaceHolder.pFormat = NULL;
    }
    else
    {
        PlaceHolder.Character   = 0;
        PlaceHolder.Index       = CurTextIndex;
        PlaceHolder.pFormat     = NULL;
    }
    return PlaceHolder; 
}

void Paragraph::CharactersIterator::operator++() 
{ 
    if (!IsFinished())
    {
        ++CurTextIndex;
        if (!FormatIterator.IsFinished())
        {
            const TextFormatRunType& fmtRange = *FormatIterator; 
            SF_ASSERT(fmtRange.Index >= 0);
            if (CurTextIndex >= UPInt(fmtRange.NextIndex()))
            {
                ++FormatIterator;
            }
        }
    }
    else
        CurTextIndex = pText->GetSize();
}

void Paragraph::CharactersIterator::operator+=(UPInt n) 
{
    for(UPInt i = 0; i < n; ++i)
    {
        operator++();
    }
}

const wchar_t* Paragraph::CharactersIterator::GetRemainingTextPtr(UPInt* plen) const 
{ 
    if (!IsFinished())
    {
        if (plen) *plen = pText->GetSize() - CurTextIndex;
        return pText->ToWStr() + CurTextIndex;
    }
    *plen = 0;
    return NULL;
}

//////////////////////////////////
// StyledText
//
StyledText::StyledText():Paragraphs(), RTFlags(0) 
{
    MemoryHeap* pheap = Memory::GetHeapByAddress(this);
    pDefaultParagraphFormat = *SF_HEAP_NEW(pheap) ParagraphFormat;
    pDefaultTextFormat      = *SF_HEAP_NEW(pheap) TextFormat(pheap);
}

StyledText::StyledText(Allocator* pallocator)
    : pTextAllocator(pallocator),Paragraphs(), RTFlags(0)
{
    SF_ASSERT(pallocator);
    pDefaultParagraphFormat = *pallocator->AllocateParagraphFormat(ParagraphFormat());
    pDefaultTextFormat      = *pallocator->AllocateDefaultTextFormat();
}

Paragraph* StyledText::AppendNewParagraph(const ParagraphFormat* pdefParaFmt)
{
    UPInt sz = Paragraphs.GetSize();
    UPInt nextPos = 0;
    if (sz > 0)
    {
        Paragraph* ppara = Paragraphs[sz - 1];
        SF_ASSERT(ppara);
        nextPos = ppara->GetStartIndex() + ppara->GetLength();
    }

    Paragraphs.PushBack(GetAllocator()->AllocateParagraph());

    sz = Paragraphs.GetSize();
    Paragraph* ppara = Paragraphs[sz - 1];
    SF_ASSERT(ppara);
    ppara->SetFormat(pTextAllocator, (pdefParaFmt == NULL) ? *pDefaultParagraphFormat : *pdefParaFmt);
    ppara->SetStartIndex(nextPos);
    return ppara;
}

Paragraph* StyledText::AppendCopyOfParagraph(const Paragraph& srcPara)
{
    UPInt sz = Paragraphs.GetSize();
    UPInt nextPos = 0;
    if (sz > 0)
    {
        Paragraph* ppara = Paragraphs[sz - 1];
        SF_ASSERT(ppara);
        nextPos = ppara->GetStartIndex() + ppara->GetLength();
    }

    Paragraphs.PushBack(GetAllocator()->AllocateParagraph(srcPara));

    sz = Paragraphs.GetSize();
    Paragraph* ppara = Paragraphs[sz - 1];
    SF_ASSERT(ppara);
    ppara->SetStartIndex(nextPos);
    return ppara;
}

Paragraph* StyledText::InsertNewParagraph(StyledText::ParagraphsIterator& iter, 
                                                    const ParagraphFormat* pdefParaFmt)
{
    if (iter.IsFinished())
        return AppendNewParagraph(pdefParaFmt);
    else
    {
        UPInt index = UPInt(iter.GetIndex());
        UPInt nextPos = 0;
        if (index > 0)
        {
            Paragraph* ppara = Paragraphs[index - 1];
            SF_ASSERT(ppara);
            nextPos = ppara->GetStartIndex() + ppara->GetLength();
        }

        Paragraphs.InsertAt(index, GetAllocator()->AllocateParagraph());

        Paragraph* ppara = Paragraphs[index];
        SF_ASSERT(ppara);
        ppara->SetFormat(pTextAllocator, (pdefParaFmt == NULL) ? *pDefaultParagraphFormat : *pdefParaFmt);
        ppara->SetStartIndex(nextPos);
        return ppara;
    }
}

Paragraph* StyledText::InsertCopyOfParagraph(StyledText::ParagraphsIterator& iter, 
                                                       const Paragraph& srcPara)
{
    if (iter.IsFinished())
        return AppendCopyOfParagraph(srcPara);
    else
    {
        UPInt index = UPInt(iter.GetIndex());
        UPInt nextPos = 0;
        if (index > 0)
        {
            Paragraph* ppara = Paragraphs[index - 1];
            SF_ASSERT(ppara);
            nextPos = ppara->GetStartIndex() + ppara->GetLength();
        }

        Paragraphs.InsertAt(index, GetAllocator()->AllocateParagraph(srcPara));

        Paragraph* ppara = Paragraphs[index];
        SF_ASSERT(ppara);
        ppara->SetStartIndex(nextPos);
        return ppara;
    }
}

Paragraph* StyledText::GetLastParagraph()
{
    ParagraphsIterator paraIter = Paragraphs.Last();
    if (!paraIter.IsFinished())
    {
        return paraIter->GetPtr();
    }
    return NULL;
}

const Paragraph* StyledText::GetLastParagraph() const
{
    ParagraphsConstIterator paraIter = Paragraphs.Last();
    if (!paraIter.IsFinished())
    {
        return paraIter->GetPtr();
    }
    return NULL;
}

UPInt StyledText::AppendString(const char* putf8String, UPInt stringSize,
                                  NewLinePolicy newLinePolicy)
{
    SF_ASSERT(GetDefaultTextFormat() && GetDefaultParagraphFormat());
    return AppendString(putf8String, stringSize, newLinePolicy, 
        GetDefaultTextFormat(), GetDefaultParagraphFormat());
}

UPInt StyledText::AppendString(const char* putf8String, UPInt stringSize,
                                  NewLinePolicy newLinePolicy,
                                  const TextFormat* pdefTextFmt, 
                                  const ParagraphFormat* pdefParaFmt)
{
    SF_ASSERT(pdefTextFmt && pdefParaFmt);
    // parse text, and form paragraphs
    const char* pbegin = putf8String;
    if (stringSize == SF_MAX_UPINT)
        stringSize = SFstrlen(putf8String);
    const char* pend = pbegin + stringSize;

    // parse text, and form paragraphs
    UInt32 uniChar = 0;

    UPInt totalAppenededLen = 0;
    UPInt curOffset;
    Paragraph* ppara = GetLastParagraph();
    if (!ppara)
        curOffset = 0;
    else
        curOffset = ppara->GetStartIndex();

    OnTextInserting(curOffset, stringSize, putf8String);

    unsigned i = 0;
    do
    {
        UPInt posInPara;
        UPInt startOffset = curOffset;
        if (i++ > 0 || ppara == NULL)
        {
            // need to create new paragraph
            ppara = AppendNewParagraph(pdefParaFmt);
            posInPara = 0;
            ppara->SetStartIndex(startOffset);
        }
        else
        {
            // use existing paragraph
            ppara->RemoveTermNull();
            posInPara = ppara->GetLength();
            if (posInPara == 0 && pdefParaFmt)
            {
                // Paragraph is empty - we can apply pdefParaFmt
                ppara->SetFormat(pTextAllocator, *pdefParaFmt);
            }
        }
        SF_ASSERT(ppara);
        UPInt paraLength = 0;
        const char* pstr = pbegin;
        UInt32 prevUniChar = uniChar;
        for (uniChar = ~0u; pstr < pend && uniChar != 0; )
        {
            UInt32 newUniChar = UTF8Util::DecodeNextChar(&pstr);
            if (newLinePolicy == NLP_CompressCRLF && prevUniChar == '\r' && paraLength == 0)
            {
                prevUniChar = ~0u;
                if (newUniChar == '\n')
                {
                    ++pbegin;
                    continue; // skip extra newline
                }
            }
            uniChar = newUniChar;
            if (uniChar == '\n' || uniChar == '\r')
                break;
            ++paraLength;
        }
        if (uniChar == '\n' || uniChar == '\r')
            ++paraLength; 

        if (paraLength > 0)
        {
            wchar_t* pwstr = ppara->CreatePosition(pTextAllocator, posInPara, paraLength);

            pstr = pbegin;
            for (uniChar = ~0u; pstr < pend && uniChar != 0; )
            {
                uniChar = UTF8Util::DecodeNextChar(&pstr);
                if (uniChar == '\r' || uniChar == '\n')  // replace '\r' by '\n'
                    uniChar = NewLineChar();
                *pwstr++ = (wchar_t)uniChar;
                if (uniChar == NewLineChar())
                    break;
            }
            ppara->SetTextFormat(pTextAllocator, *pdefTextFmt, posInPara, SF_MAX_UPINT);
            pbegin = pstr;
            curOffset += paraLength + posInPara;
            totalAppenededLen += paraLength;
        }
    } while(pbegin < pend && uniChar != 0);

    // check the last char. If it is \n - create the empty paragraph
    if (uniChar == NewLineChar())
        ppara = AppendNewParagraph(pdefParaFmt);

    ppara->AppendTermNull(pTextAllocator, pdefTextFmt);

    if (pdefTextFmt->IsUrlSet())
        SetMayHaveUrl();

    CheckIntegrity();
    return totalAppenededLen;
}

UPInt StyledText::AppendString(const wchar_t* pstr, UPInt length, NewLinePolicy newLinePolicy)
{
    SF_ASSERT(GetDefaultTextFormat() && GetDefaultParagraphFormat());
    return AppendString(pstr, length, newLinePolicy, GetDefaultTextFormat(), GetDefaultParagraphFormat());
}

UPInt StyledText::AppendString(const wchar_t* pstr, UPInt length,
                                  NewLinePolicy newLinePolicy,
                                  const TextFormat* pdefTextFmt, 
                                  const ParagraphFormat* pdefParaFmt)
{
    SF_ASSERT(pdefTextFmt && pdefParaFmt);

    // parse text, and form paragraphs
    UInt32 uniChar = 0;
    if (length == SF_MAX_UPINT)
        length = SFwcslen(pstr);
    const wchar_t* pend = pstr + length;

    UPInt curOffset;
    Paragraph* ppara = GetLastParagraph();
    if (!ppara)
        curOffset = 0;
    else
        curOffset = ppara->GetStartIndex();

    OnTextInserting(curOffset, length, pstr);

    UPInt totalAppenededLen = 0;
    unsigned i = 0;
    do
    {
        UPInt posInPara;
        UPInt startOffset = curOffset;
        if (i++ > 0 || ppara == NULL)
        {
            // need to create new paragraph
            ppara = AppendNewParagraph(pdefParaFmt);
            posInPara = 0;
            ppara->SetStartIndex(startOffset);
        }
        else
        {
            // use existing paragraph
            ppara->RemoveTermNull();
            posInPara = ppara->GetLength();
            if (posInPara == 0 && pdefParaFmt)
            {
                // Paragraph is empty - we can apply pdefParaFmt
                ppara->SetFormat(pTextAllocator, *pdefParaFmt);
            }
        }
        UPInt paraLength = 0;
        // check, do we need to skip extra new line char in the case if "\r\n" EOL
        // is used.
        if (newLinePolicy == NLP_CompressCRLF && uniChar == '\r' && pstr[0] == '\n')
        {
            ++pstr;
            --length;
            if (length == 0)
                break; // it is possible that that was the last char.
        }
        for(; paraLength < length; ++paraLength)
        {
            uniChar = pstr[paraLength];
            if (uniChar == '\n' || uniChar == '\r' || uniChar == '\0')
                break;
        }
        if (uniChar == '\n' || uniChar == '\r')
            ++paraLength; // skip \n

        wchar_t* pwstr = ppara->CreatePosition(pTextAllocator, posInPara, paraLength);
        memcpy(pwstr, pstr, paraLength * sizeof(wchar_t));
        pstr += paraLength;
        length -= paraLength;
        SF_ASSERT(((SPInt)length) >= 0);

        if ((uniChar == '\n' || uniChar == '\r') && uniChar != NewLineChar())
        {
            // need to add '\n' instead of '\r'
            UPInt l = ppara->GetLength();
            if (l > 0)
            {
                wchar_t* p = ppara->GetText();
                p[l - 1] = NewLineChar();
            }
            uniChar = NewLineChar();
        }
        ppara->SetTextFormat(pTextAllocator, *pdefTextFmt, posInPara, SF_MAX_UPINT);
        curOffset += paraLength + posInPara;
        totalAppenededLen += paraLength;
    } while(pstr < pend && uniChar != '\0');

    // check the last char. If it is \n - create the empty paragraph
    if (uniChar == NewLineChar())
        ppara = AppendNewParagraph(pdefParaFmt);

    ppara->AppendTermNull(pTextAllocator, pdefTextFmt);

    if (pdefTextFmt->IsUrlSet())
        SetMayHaveUrl();

    CheckIntegrity();
    return totalAppenededLen;
}

void StyledText::SetText(const char* putf8String, UPInt stringSize)
{
    Clear();
    AppendString(putf8String, stringSize);
}

void StyledText::SetText(const wchar_t* pstr, UPInt length)
{
    Clear();
    AppendString(pstr, length);
}

StyledText::ParagraphsIterator StyledText::GetParagraphByIndex(UPInt index, UPInt* pindexInParagraph)
{
    UPInt i = Alg::LowerBound(Paragraphs, index, ParagraphComparator::Less);
    if (i < Paragraphs.GetSize() && ParagraphComparator::Compare(Paragraphs[i], index) == 0)
    {
        ParagraphsIterator it(Paragraphs.Begin() + (int)i);
        if (pindexInParagraph)
            *pindexInParagraph = index - it->GetPtr()->GetStartIndex();
        return it;    
    }
    return ParagraphsIterator();
}

StyledText::ParagraphsIterator StyledText::GetNearestParagraphByIndex(UPInt index, UPInt* pindexInParagraph)
{
    if (Paragraphs.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Paragraphs, index, ParagraphComparator::Less);
        if (i == Paragraphs.GetSize())
            --i; // make it point to the very last element
        ParagraphsIterator it(Paragraphs.Begin() + (int)i);
        if (pindexInParagraph)
        {
            *pindexInParagraph = index - (*it)->GetStartIndex();
        }
        return it;    
    }
    return ParagraphsIterator();
}

void StyledText::EnsureTermNull()
{
    Paragraph* ppara = GetLastParagraph();
    if (!ppara)
        ppara = AppendNewParagraph();
    if (ppara && !ppara->HasNewLine())
    {
        ppara->AppendTermNull(pTextAllocator, pDefaultTextFormat);
    }
}

void StyledText::SetTextFormat(const TextFormat& fmt, UPInt startPos, UPInt endPos)
{
    SF_ASSERT(endPos >= startPos);
    UPInt indexInPara, endIndexInPara;
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    UPInt runLen = endPos - startPos;
    while(!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        UPInt paraLength = ppara->GetLength();
        if (indexInPara + runLen > paraLength)
            endIndexInPara = indexInPara + paraLength - indexInPara;
        else
            endIndexInPara = indexInPara + runLen;

        if (endIndexInPara == paraLength && ppara->HasTermNull())
        {
            ++endIndexInPara; // include the termination null symbol
            if (runLen != SF_MAX_UPINT) // preventing overflowing
                ++runLen;
        }

        ppara->SetTextFormat(pTextAllocator, fmt, indexInPara, endIndexInPara);

        SF_ASSERT(runLen >= (endIndexInPara - indexInPara));
        runLen -= (endIndexInPara - indexInPara);
        indexInPara = 0;
        ++paraIter;
    }
    if (fmt.IsUrlSet())
        SetMayHaveUrl();
    CheckIntegrity();
}

void StyledText::SetParagraphFormat(const ParagraphFormat& fmt, UPInt startPos, UPInt endPos)
{
    SF_ASSERT(endPos >= startPos);
    UPInt indexInPara, endIndexInPara;
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    UPInt runLen = endPos - startPos;
    while(!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        if (indexInPara == 0)
        {
            ppara->SetFormat(pTextAllocator, fmt);
        }
        if (runLen == 0)
            break;
        UPInt paraLength = ppara->GetLength();
        if (runLen > paraLength)
            endIndexInPara = indexInPara + paraLength - indexInPara;
        else
            endIndexInPara = indexInPara + runLen;
        SF_ASSERT(runLen >= (endIndexInPara - indexInPara));
        runLen -= (endIndexInPara - indexInPara);
        indexInPara = 0;
        ++paraIter;
    }

    CheckIntegrity();
}

void StyledText::ClearTextFormat(UPInt startPos, UPInt endPos)
{
    SF_ASSERT(endPos >= startPos);
    UPInt indexInPara, endIndexInPara;
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    UPInt runLen = endPos - startPos;
    while(!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        UPInt paraLength = ppara->GetLength();
        if (indexInPara + runLen > paraLength)
            endIndexInPara = indexInPara + paraLength - indexInPara;
        else
            endIndexInPara = indexInPara + runLen;

        if (endIndexInPara == paraLength && ppara->HasTermNull())
        {
            ++endIndexInPara; // include the termination null symbol
            if (runLen != SF_MAX_UPINT) // preventing overflowing
                ++runLen;
        }

        ppara->ClearTextFormat(indexInPara, endIndexInPara);

        SF_ASSERT(runLen >= (endIndexInPara - indexInPara));
        runLen -= (endIndexInPara - indexInPara);
        indexInPara = 0;
        ++paraIter;
    }
}

void StyledText::GetTextAndParagraphFormat
        (TextFormat* pdestTextFmt, ParagraphFormat* pdestParaFmt, UPInt startPos, UPInt endPos)
{
    SF_ASSERT(endPos >= startPos);
    SF_ASSERT(pdestParaFmt || pdestTextFmt);

    UPInt indexInPara;
    UPInt runLen = endPos - startPos;
    
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    TextFormat         finalTextFmt(Memory::GetHeapByAddress(this));
    ParagraphFormat    finalParaFmt;

    int i = 0, pi = 0;
    while(runLen > 0 && !paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        UPInt lengthInPara = Alg::PMin(runLen, ppara->GetLength());
        if (lengthInPara == 0)
            break; // the only empty paragraph could be the last one, so, we good to break
        if (i++ == 0)
            finalTextFmt = ppara->GetTextFormat(indexInPara, indexInPara + lengthInPara);
        else
            finalTextFmt = ppara->GetTextFormat(indexInPara, indexInPara + lengthInPara).Intersection(finalTextFmt);
        if (indexInPara == 0)
        {
            const ParagraphFormat* ppfmt = ppara->GetFormat();
            if (ppfmt)
            {
                if (pi++ == 0)
                    finalParaFmt = *ppfmt;
                else
                    finalParaFmt = ppfmt->Intersection(finalParaFmt);
            }
        }
        SF_ASSERT(runLen >= lengthInPara);
        runLen -= lengthInPara;
        ++paraIter;
    }
    if (pdestTextFmt)
        *pdestTextFmt = finalTextFmt;
    if (pdestParaFmt)
        *pdestParaFmt = finalParaFmt;
}

bool StyledText::GetTextAndParagraphFormat
(const TextFormat** ppdestTextFmt, const ParagraphFormat** ppdestParaFmt, UPInt pos)
{
    SF_ASSERT(ppdestParaFmt || ppdestTextFmt);

    UPInt indexInPara;
    ParagraphsIterator paraIter = GetParagraphByIndex(pos, &indexInPara);
    const ParagraphFormat* pparaFmt = NULL;
    const TextFormat* ptextFmt = NULL;
    bool rv = false;
    if (!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        ptextFmt = ppara->GetTextFormatPtr(indexInPara);
        pparaFmt = ppara->GetFormat();
        rv = true;
    }
    if (ptextFmt == NULL)
        ptextFmt = pDefaultTextFormat;
    if (pparaFmt == NULL)
        pparaFmt = pDefaultParagraphFormat;
    if (ppdestTextFmt)
        *ppdestTextFmt = ptextFmt;
    if (ppdestParaFmt)
        *ppdestParaFmt = pparaFmt;
    return rv;
}

void StyledText::SetDefaultTextFormat(const TextFormat& defaultTextFmt)
{
    if (defaultTextFmt.GetImageDesc())
    {
        TextFormat textfmt = defaultTextFmt;
        textfmt.SetImageDesc(NULL);
        pDefaultTextFormat = *GetAllocator()->AllocateTextFormat(textfmt);
    }
    else 
        pDefaultTextFormat = *GetAllocator()->AllocateTextFormat(defaultTextFmt);
    SF_ASSERT(pDefaultTextFormat);
}

void StyledText::SetDefaultParagraphFormat(const ParagraphFormat& defaultParagraphFmt)
{
    pDefaultParagraphFormat = *GetAllocator()->AllocateParagraphFormat(defaultParagraphFmt);
    SF_ASSERT(pDefaultParagraphFormat);
}

void StyledText::SetDefaultTextFormat(const TextFormat* pdefaultTextFmt)
{
    SF_ASSERT(pdefaultTextFmt);
    if (pdefaultTextFmt->GetImageDesc())
    {
        SetDefaultTextFormat(*pdefaultTextFmt);
    }
    else
    {
        pDefaultTextFormat = const_cast<TextFormat*>(pdefaultTextFmt);
    }
}

void StyledText::SetDefaultParagraphFormat(const ParagraphFormat* pdefaultParagraphFmt)
{
    SF_ASSERT(pdefaultParagraphFmt);
    pDefaultParagraphFormat = const_cast<ParagraphFormat*>(pdefaultParagraphFmt);
}

String StyledText::GetHtml() const
{
    StringBuffer retStr;
    return String(GetHtml(&retStr));
}

String& StyledText::GetHtml(String* retStr) const
{
    SF_ASSERT(retStr);
    StringBuffer retStrBuf;
    (*retStr) = GetHtml(&retStrBuf);
    return *retStr;
}

StringBuffer& StyledText::GetHtml(StringBuffer* strBuf) const
{
    SF_ASSERT(strBuf);
    StringBuffer& retStr = *strBuf;
    retStr = "";
    ParagraphsIterator paraIter = GetParagraphIterator();
    for(UPInt np = 0, nn = Paragraphs.GetSize(); !paraIter.IsFinished(); ++paraIter, ++np)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        if (np + 1 == nn && ppara->GetLength() == 0)
        {
            // this is the last empty paragraph - ignore it
            continue; 
        }
        retStr += "<TEXTFORMAT";
        SF_ASSERT(ppara->GetFormat());
        const ParagraphFormat& paraFormat = *ppara->GetFormat();
        if (paraFormat.IsIndentSet())
            Format(retStr, " INDENT=\"{0}\"", paraFormat.GetIndent());
        if (paraFormat.IsBlockIndentSet())
            Format(retStr, " BLOCKINDENT=\"{0}\"", paraFormat.GetBlockIndent());
        if (paraFormat.IsLeftMarginSet())
            Format(retStr, " LEFTMARGIN=\"{0}\"", paraFormat.GetLeftMargin());
        if (paraFormat.IsRightMarginSet())
            Format(retStr, " RIGHTMARGIN=\"{0}\"", paraFormat.GetRightMargin());
        if (paraFormat.IsLeadingSet())
            Format(retStr, " LEADING=\"{0}\"", paraFormat.GetLeading());
        if (paraFormat.IsTabStopsSet())
        {
            retStr += " TABSTOPS=\"";
            unsigned n;
            const unsigned *ptabStops = paraFormat.GetTabStops(&n);
            for (unsigned i = 0; i < n; ++i)
                Format(retStr, "{0:sw:,:}{1}", i > 0, ptabStops[i]);
            retStr += "\"";
        }
        retStr += "><";
        if (!paraFormat.IsBullet())
            retStr += "P";
        else
            retStr += "LI";
        retStr += " ALIGN=\"";
        switch(paraFormat.GetAlignment())
        {
        case ParagraphFormat::Align_Left: retStr += "LEFT"; break;
        case ParagraphFormat::Align_Right: retStr += "RIGHT"; break;
        case ParagraphFormat::Align_Center: retStr += "CENTER"; break;
        case ParagraphFormat::Align_Justify: retStr += "JUSTIFY"; break;
        }
        retStr += "\">";
        Paragraph::FormatRunIterator it = ppara->GetIterator();
        Ptr<TextFormat> pprevFmt;
        bool fontTagOpened = false;
        for(; !it.IsFinished(); ++it)
        {
            const Paragraph::StyledTextRun& run = *it;
//            if (run.Length == 1 && run.pText[0] == NewLineChar())
  //              break; // a spec case - do not include format for newline char into HTML,
            // if this format is different from rest of the text.
            bool isImageTag = false;

            if (run.pFormat)
            {
                Ptr<HTMLImageTagDesc> pimage = run.pFormat->GetImageDesc();
                SF_ASSERT(!pimage || run.pFormat->IsImageDescSet());
                if (pimage)
                {
                    isImageTag = true;
                    if (run.pFormat->IsUrlSet())
                        Format(retStr, "<A HREF=\"{0}\">", run.pFormat->GetUrl());
                    retStr += "<IMG SRC=\"";

                    retStr += pimage->Url;
                    retStr += "\"";
                    if (pimage->ScreenWidth > 0)
                        Format(retStr, " WIDTH=\"{0}\"", (int)FixpToPixels(pimage->ScreenWidth));
                    if (pimage->ScreenHeight > 0)
                        Format(retStr, " HEIGHT=\"{0}\"", (int)FixpToPixels(pimage->ScreenHeight));
                    if (pimage->VSpace != 0)
                        Format(retStr, " VSPACE=\"{0}\"", (int)FixpToPixels(pimage->VSpace));
                    if (pimage->HSpace != 0)
                        Format(retStr, " HSPACE=\"{0}\"", (int)FixpToPixels(pimage->HSpace));
                    if (!pimage->Id.IsEmpty())
                        Format(retStr, " ID=\"{0}\"", pimage->Id);
                    retStr += " ALIGN=\"";
                    switch(pimage->Alignment)
                    {
                    case StyledText::HTMLImageTagInfo::Align_Left:
                        retStr += "left";
                        break;
                    case StyledText::HTMLImageTagInfo::Align_Right:
                        retStr += "right";
                        break;
                    case StyledText::HTMLImageTagInfo::Align_BaseLine:
                        retStr += "baseline";
                        break;
                    default: break;
                    }
                    retStr += "\">";

                    if (run.pFormat->IsUrlSet())
                        retStr += "</A>";
                }
                else
                {
                    if (fontTagOpened && pprevFmt && !pprevFmt->IsHTMLFontTagSame(*run.pFormat))
                    {
                        retStr += "</FONT>";
                        fontTagOpened = false;
                    }

                    if (!fontTagOpened)
                    {
                        retStr += "<FONT";
                        if (run.pFormat->IsFontListSet())
                            Format(retStr, " FACE=\"{0}\"", run.pFormat->GetFontList());
                        if (run.pFormat->IsFontSizeSet())
                            Format(retStr, " SIZE=\"{0}\"", (unsigned)run.pFormat->GetFontSize());
                        if (run.pFormat->IsColorSet())
                            Format(retStr, " COLOR=\"#{0:X:.6}\"", (run.pFormat->GetColor32() & 0xFFFFFFu));
                        if (run.pFormat->IsLetterSpacingSet())
                            Format(retStr, " LETTERSPACING=\"{0}\"", run.pFormat->GetLetterSpacing());
                        if (run.pFormat->IsAlphaSet())
                            Format(retStr, " ALPHA=\"#{0:X:.2}\"", (run.pFormat->GetAlpha() & 0xFFu));

                        Format(retStr, " KERNING=\"{0:sw:1:0}\"", run.pFormat->IsKerning());

                        retStr += ">";
                        fontTagOpened = true;
                    }

                    if (run.pFormat->IsUrlSet())
                        Format(retStr, "<A HREF=\"{0}\">", run.pFormat->GetUrl());

                    if (run.pFormat->IsBold())
                        retStr += "<B>";
                    if (run.pFormat->IsItalic())
                        retStr += "<I>";
                    if (run.pFormat->IsUnderline())
                        retStr += "<U>";
                    pprevFmt = run.pFormat;
                }
            }
            if (!isImageTag)
            {
                for (UPInt i = 0; i < run.Length; ++i)
                {
                    if (run.pText[i] == NewLineChar() || run.pText[i] == '\0')
                        continue;
                    switch(run.pText[i])
                    {
                    case '<': retStr += "&lt;"; break;
                    case '>': retStr += "&gt;"; break;
                    case '&': retStr += "&amp;"; break;
                    case '\"': retStr += "&quot;"; break;
                    case '\'': retStr += "&apos;"; break;
                    case GFX_NBSP_CHAR_CODE: retStr += "&nbsp;"; break;
                    default: retStr.AppendChar(run.pText[i]);
                    }
                }
                if (run.pFormat)
                {
                    if (run.pFormat->IsUnderline())
                        retStr += "</U>";
                    if (run.pFormat->IsItalic())
                        retStr += "</I>";
                    if (run.pFormat->IsBold())
                        retStr += "</B>";
                    if (run.pFormat->IsUrlSet())
                        retStr += "</A>";
                    //retStr += "</FONT>";
                }
            }
        }
        if (fontTagOpened)
        {
            retStr += "</FONT>";
            fontTagOpened = false;
        }

        retStr += "</";
        if (!paraFormat.IsBullet())
            retStr += "P";
        else
            retStr += "LI";
        retStr += "></TEXTFORMAT>";
    }
    return *strBuf;
}

UPInt StyledText::GetLength() const
{
    UPInt length = 0;
    ParagraphsIterator paraIter = GetParagraphIterator();
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        length += ppara->GetLength();
    }
    return length;
}

String StyledText::GetText() const
{
    String retStr;
    GetText(&retStr);
    return retStr;
}

String& StyledText::GetText(String* retStr) const
{
    SF_ASSERT(retStr);
    (*retStr) = "";
    ParagraphsIterator paraIter = GetParagraphIterator();
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        const wchar_t* pstr = ppara->GetText();
        retStr->AppendString(pstr, ppara->GetLength());
    }
    return *retStr;
}

void StyledText::GetText(WStringBuffer* pBuffer) const
{
    SF_ASSERT(pBuffer);

    pBuffer->Resize(GetLength() + 1);

    ParagraphsIterator paraIter = GetParagraphIterator();
    UPInt oldSz = 0;
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        const wchar_t* pstr = ppara->GetText();
        UPInt paraLen = ppara->GetLength();
        memcpy(pBuffer->GetBuffer() + oldSz, pstr, sizeof(wchar_t) * paraLen);
        oldSz += paraLen;
    }
    (*pBuffer)[oldSz] = '\0';
}

void StyledText::GetText(WStringBuffer* pBuffer,UPInt startPos, UPInt endPos) const
{
    SF_ASSERT(endPos >= startPos);
    SF_ASSERT(pBuffer);

    if (endPos == SF_MAX_UPINT)
        endPos = GetLength();
    UPInt len = endPos - startPos;

    pBuffer->Resize(len + 1);
    UPInt indexInPara = 0, remainedLen = len;
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    UPInt oldSize = 0;
    for(; !paraIter.IsFinished() && remainedLen > 0; ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        UPInt lenInPara = ppara->GetLength() - indexInPara;
        if (lenInPara > remainedLen)
            lenInPara = remainedLen;
        const wchar_t* pstr = ppara->GetText();

        memcpy(pBuffer->GetBuffer() + oldSize, pstr + indexInPara, sizeof(wchar_t) * lenInPara);
        oldSize += lenInPara;

        indexInPara = 0;
        remainedLen -= lenInPara;
        SF_ASSERT(((SPInt)remainedLen) >= 0);
    }
    (*pBuffer)[oldSize] = '\0';
}

StyledText* StyledText::CopyStyledText(UPInt startPos, UPInt endPos) const
{
    Allocator* pallocator = GetAllocator();
    StyledText* pstyledText = SF_HEAP_NEW(pallocator->GetHeap()) StyledText(pallocator);
    CopyStyledText(pstyledText, startPos, endPos);
    return pstyledText;
}

void StyledText::CopyStyledText(StyledText* pdest, UPInt startPos, UPInt endPos) const
{
    SF_ASSERT(endPos >= startPos);

    if (endPos == SF_MAX_UPINT)
        endPos = GetLength();
    UPInt len = endPos - startPos;
    UPInt indexInPara = 0, remainedLen = len;
    pdest->Clear();
    pdest->OnTextInserting(startPos, len, "");
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    if (!paraIter.IsFinished())
    {
        // if we need to take the only part of the first paragraph then
        // do Copy for this part. Otherwise, whole paragraph will be copied later.
        if (indexInPara != 0)
        {
            Paragraph* psrcPara = *paraIter;
            Paragraph* pdestPara = pdest->AppendNewParagraph(psrcPara->GetFormat());
            UPInt lenToCopy = psrcPara->GetLength() - indexInPara;
            lenToCopy = Alg::PMin(lenToCopy, len);
            pdestPara->Copy(pdest->GetAllocator(), *psrcPara, indexInPara, 0, lenToCopy);
            remainedLen -= lenToCopy;
            ++paraIter;
        }
        for(; !paraIter.IsFinished() && remainedLen > 0; ++paraIter)
        {
            Paragraph* ppara = *paraIter;
            SF_ASSERT(ppara);
            UPInt lenInPara = ppara->GetLength();
            if (lenInPara > remainedLen)
            {
                // copy the part of the last paragraph
                Paragraph* psrcPara = *paraIter;
                Paragraph* pdestPara = pdest->AppendNewParagraph(psrcPara->GetFormat());
                pdestPara->Copy(pdest->GetAllocator(), *psrcPara, 0, 0, remainedLen);
                break;
            }
            pdest->AppendCopyOfParagraph(*ppara);
            remainedLen -= lenInPara;
            SF_ASSERT(((SPInt)remainedLen) >= 0);
        }
    }
    // check the last char. If it is \n - create the empty paragraph
    Paragraph* plastPara = pdest->GetLastParagraph();
    if (plastPara && plastPara->HasNewLine())
    {
        pdest->AppendNewParagraph(plastPara->GetFormat());
    }

    pdest->EnsureTermNull();
    if (MayHaveUrl())
        pdest->SetMayHaveUrl();
    pdest->CheckIntegrity();
}

wchar_t* StyledText::CreatePosition(UPInt pos, UPInt length)
{
    SF_UNUSED2(pos, length);
    return 0;
}

UPInt StyledText::InsertString(const wchar_t* pstr, UPInt pos, UPInt length, NewLinePolicy newLinePolicy)
{
    return InsertString(pstr, pos, length, newLinePolicy, GetDefaultTextFormat(), GetDefaultParagraphFormat());
}

UPInt StyledText::InsertString(const wchar_t* pstr, UPInt pos, UPInt length, 
                               NewLinePolicy newLinePolicy,
                               const TextFormat* pdefTextFmt, 
                               const ParagraphFormat* pdefParaFmt)
{
    if (length == 0)
        return 0;
    if (pos > GetLength())
        pos = GetLength();

    // Insert plain string at the position. Style will be inherited from the current style
    // at the insertion position. If there is no style at the insertion position, then
    // text will remain plain. New paragraph will be created after each CR.
    if (length == SF_MAX_UPINT)
        length = Paragraph::TextBuffer::StrLen(pstr);

    OnTextInserting(pos, length, pstr);

    UPInt indexInPara = 0;
    UPInt remainingSrcStrLen = length;
    SPInt newLineIdx;
    ParagraphsIterator paraIter = GetNearestParagraphByIndex(pos, &indexInPara);

    UPInt insLineLen;
    UPInt nextParaStartingPos = (!paraIter.IsFinished()) ? (*paraIter)->GetStartIndex() : 0;
    wchar_t uniChar = 0;
    UPInt totalInsertedLen = 0;

    do
    {
        if (newLinePolicy == NLP_IgnoreCRLF)
        {
            // ignore all CR LF
            while (remainingSrcStrLen > 0 && (*pstr == '\r' || *pstr == '\n'))
            {
                ++pstr;
                --remainingSrcStrLen;
            }
            if (remainingSrcStrLen == 0)
                break; // it is possible that that was the last char.
        }
        if (paraIter.IsFinished())
        {
            // ok, there is no more (or none) paragraphs - create a new one
            AppendNewParagraph(pdefParaFmt);
            paraIter = GetParagraphIterator();
            indexInPara = 0;
        }
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        if (ppara->IsEmpty())
        {
            // if paragraph is empty we can apply the pdefParaFmt
            ppara->SetFormat(pTextAllocator, *pdefParaFmt);
        }

        insLineLen = 0;
        newLineIdx = -1;
        // check, do we need to skip extra new line char in the case if "\r\n" EOL
        // is used.
        if (newLinePolicy == NLP_CompressCRLF && uniChar == '\r' && pstr[insLineLen] == '\n')
        {
            ++pstr;
            --remainingSrcStrLen;
            if (remainingSrcStrLen == 0)
                break; // it is possible that that was the last char.
        }
        // look for a new-line char in the line being inserted.
        for(; insLineLen < remainingSrcStrLen; ++insLineLen)
        {
            uniChar = pstr[insLineLen];
            if (uniChar == '\n' || uniChar == '\r')
            {
                if (newLinePolicy != NLP_IgnoreCRLF)
                    newLineIdx = insLineLen;
                else
                    uniChar = 1; // just make it different from '\0' and CR LF
                break;
            }
            else if (uniChar == '\0')
                break;
        }
        if (uniChar == '\n' || uniChar == '\r')
            ++insLineLen; // skip \n

        // was there a new-line symbol?
        if (newLineIdx != -1)
        {
            // yes, make another paragraph first
            ParagraphsIterator newParaIter = paraIter;
            ++newParaIter;
            Paragraph& newPara = *InsertNewParagraph(newParaIter, pdefParaFmt);

            // new paragraph should inherit style from the previous ppara->
            newPara.SetFormat(ppara->GetFormat());
            SF_ASSERT(newPara.GetFormat());

            // now, copy the text to the new para
            newPara.Copy(pTextAllocator, *ppara, indexInPara, 0, ppara->GetSize() - indexInPara);

            UPInt remLen = ppara->GetSize();
            SF_ASSERT(remLen >= indexInPara);
            remLen -= indexInPara;

            // now, we can insert the sub-string in source para
            ppara->InsertString(pTextAllocator, pstr, indexInPara, insLineLen, pdefTextFmt);

            if (remLen > 0)
            {
                // now, make source paragraph shorter
                ppara->Shrink(remLen);
            }
        }
        else
        {
            // the string being inserted doesn't contain any '\n'
            // so, insert it as a substring
            ppara->InsertString(pTextAllocator, pstr, indexInPara, insLineLen, pdefTextFmt);
        }
        // do we need to replace the newline char?
        if ((uniChar == '\r' || uniChar == '\n') && uniChar != NewLineChar())
        {
            wchar_t* ptxt = ppara->GetText();
            SF_ASSERT(ptxt[indexInPara + insLineLen - 1] == '\r' || 
                ptxt[indexInPara + insLineLen - 1] == '\n');
            ptxt[indexInPara + insLineLen - 1] = NewLineChar();
        }

        SF_ASSERT(remainingSrcStrLen >= insLineLen);
        remainingSrcStrLen -= insLineLen;
        pstr += insLineLen;
        totalInsertedLen += insLineLen;
        ppara->SetStartIndex(nextParaStartingPos);

        // do not advance to the next paragraph if IgnoreCRLF is used
        if (newLinePolicy != NLP_IgnoreCRLF)
        {
            indexInPara = 0;
            nextParaStartingPos += ppara->GetSize();
            ++paraIter;
        }
        else
            indexInPara += insLineLen;
    } while(remainingSrcStrLen > 0 && uniChar != '\0');

    // correct the starting indices for remaining paragraphs
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        ppara->SetStartIndex(nextParaStartingPos);
        nextParaStartingPos += ppara->GetSize();
    }

    EnsureTermNull();

    if (pdefTextFmt->IsUrlSet())
        SetMayHaveUrl();
    CheckIntegrity();
    return totalInsertedLen;
}

UPInt StyledText::InsertStyledText(const StyledText& text, UPInt pos, UPInt length)
{
    UPInt textLen = text.GetLength();
    if (length == SF_MAX_UPINT || length > textLen)
        length = textLen;
    if (length == 0 || text.Paragraphs.GetSize() == 0)
        return 0;
    OnTextInserting(pos, length, ""); //??

    UPInt indexInPara = 0;
    ParagraphsIterator destParaIter = GetNearestParagraphByIndex(pos, &indexInPara);

    if (destParaIter.IsFinished())
    {
        // ok, there is no paragraphs - create a new one
        AppendNewParagraph();
        destParaIter = GetParagraphIterator();
        indexInPara = 0;
    }

    UPInt nextParaStartingPos = (!destParaIter.IsFinished()) ? (*destParaIter)->GetStartIndex() : 0;
    if (text.Paragraphs.GetSize() == 1)
    {
        // special case if there is only one para to insert
        Paragraph* pdestPara = *destParaIter;
        const Paragraph* psrcPara = text.Paragraphs[0];
        SF_ASSERT(psrcPara && pdestPara);
        pdestPara->Copy(pTextAllocator, *psrcPara, 0, indexInPara, psrcPara->GetLength());            
        if (indexInPara == 0)
            pdestPara->SetFormat(pTextAllocator, *psrcPara->GetFormat());

        nextParaStartingPos += pdestPara->GetSize();
        ++destParaIter;
    }
    else
    {   
        UPInt remainedLen = length;
        ParagraphsIterator srcParaIter = text.GetParagraphIterator();
        SF_ASSERT(!srcParaIter.IsFinished());

        // divide the target paragraph on two parts
        Paragraph* ppara = *destParaIter;
        ParagraphsIterator newParaIter = destParaIter;
        ++newParaIter;
        Paragraph& newPara = *InsertNewParagraph(newParaIter, ppara->GetFormat());

        // now, copy the text to the new para
        newPara.Copy(pTextAllocator, *ppara, indexInPara, 0, ppara->GetSize() - indexInPara);

        UPInt remLen = ppara->GetSize();
        SF_ASSERT(remLen >= indexInPara);
        remLen -= indexInPara;

        // now, we can insert the first source paragraph
        Paragraph* psrcPara = *srcParaIter;
        SF_ASSERT(psrcPara);
        ppara->Copy(pTextAllocator, *psrcPara, 0, indexInPara, psrcPara->GetLength());
        remainedLen -= psrcPara->GetLength();
        SF_ASSERT(((SPInt)remainedLen) >= 0);
        if (indexInPara == 0)
        {
            ppara->SetFormat(pTextAllocator, *psrcPara->GetFormat());
        }

        if (remLen > 0)
        {
            // now, make source paragraph shorter
            ppara->Shrink(remLen);
        }
        nextParaStartingPos += ppara->GetLength();

        ++destParaIter;
        ++srcParaIter;
        // now we can insert remaining paras
        for(; !srcParaIter.IsFinished() && remainedLen > 0; ++srcParaIter, ++destParaIter)
        {
            const Paragraph* psrcParaGraph = *srcParaIter;
            SF_ASSERT(psrcParaGraph);
            UPInt lenInPara = psrcParaGraph->GetLength();

            // check, do we need to copy the remaining text to the last
            // paragraph, or we still need to create a new paragraph. 
            // Even if length are equal but src para has newline - we
            // still just copy the whole paragraph. Another case, when length
            // are equal is when src paragraph contains term zero. In this case
            // we need to copy paragraph partially (w/o trailing zero).
            if (lenInPara > remainedLen || (lenInPara == remainedLen && !psrcParaGraph->HasNewLine()))
            {
                // copy the part of the last paragraph
                newPara.Copy(pTextAllocator, *psrcParaGraph, 0, 0, lenInPara);
                newPara.SetFormat(pTextAllocator, *psrcParaGraph->GetFormat());
                break;
            }
            InsertCopyOfParagraph(destParaIter, *psrcParaGraph);
            remainedLen -= lenInPara;
            nextParaStartingPos += lenInPara;
            SF_ASSERT(((SPInt)remainedLen) >= 0);
        }
        // update the starting pos for the newPara (the last affected paragraph)
        newPara.SetStartIndex(nextParaStartingPos);
        nextParaStartingPos += newPara.GetLength();
        ++destParaIter;
    }

    // correct the starting indices for remaining paragraphs
    for(; !destParaIter.IsFinished(); ++destParaIter)
    {
        Paragraph* ppara = *destParaIter;
        SF_ASSERT(ppara);
        if (ppara->GetStartIndex() == nextParaStartingPos)
            break;
        ppara->SetStartIndex(nextParaStartingPos);
        nextParaStartingPos += ppara->GetSize();
    }
    EnsureTermNull();
    if (text.MayHaveUrl())
        SetMayHaveUrl();
    CheckIntegrity();
    return length;
}

void StyledText::RemoveParagraph(ParagraphsIterator& paraIter, Paragraph* ppara)
{
    OnParagraphRemoving(*ppara);
    ppara->FreeText(pTextAllocator);
    paraIter.Remove();
}

void StyledText::Remove(UPInt startPos, UPInt length)
{
    if (length == SF_MAX_UPINT)
        length = GetLength();

    OnTextRemoving(startPos, length);

    UPInt indexInPara;
    ParagraphsIterator paraIter = GetParagraphByIndex(startPos, &indexInPara);
    Paragraph* pprevPara = NULL;

    UPInt remainingLen = length;
    bool needUniteParas = false;
    // first stage - remove part of this para
    if (!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);

        UPInt lengthInPara = Alg::PMin(remainingLen, ppara->GetSize() - indexInPara);
        if (lengthInPara <= ppara->GetSize())
        {
            needUniteParas = (indexInPara + lengthInPara >= ppara->GetSize());
            pprevPara = *paraIter;
            ppara->Remove(indexInPara, indexInPara + lengthInPara);
            remainingLen -= lengthInPara;
            ++paraIter;
        }
    }

    // second stage - remove all paragraphs we can remove completely
    while(!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        UPInt paraLen = ppara->GetSize();
        if (remainingLen >= paraLen)
        {
            RemoveParagraph(paraIter, ppara);
            remainingLen -= paraLen;
        }
        else if (pprevPara)
        {
            if (needUniteParas)
            {
                // 3rd stage: do a copy of remaining part of the last para
                UPInt insPos = pprevPara->GetSize();
                UPInt insLen = paraLen - remainingLen;
                pprevPara->Copy(pTextAllocator, *ppara, remainingLen, insPos, insLen);

                RemoveParagraph(paraIter, ppara);
                needUniteParas = false;
            }
            break;
        }
        else  
            break; //???
        if (remainingLen == 0) 
            break;
    }
    if (!paraIter.IsFinished())
    {
        Paragraph* ppara = *paraIter;
        if (ppara->GetSize() == 0)
        {
            RemoveParagraph(paraIter, ppara);
        }
        else if (pprevPara && needUniteParas)
        {
            // 3rd stage, if remainingLen == 0: do a copy of remaining part of the last para
            UPInt paraLen = ppara->GetSize();
            UPInt insPos = pprevPara->GetSize();
            UPInt insLen = paraLen;
            pprevPara->Copy(pTextAllocator, *ppara, 0, insPos, insLen);

            RemoveParagraph(paraIter, ppara);
        }
    }

    // correct the starting indices for remaining paragraphs
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        ppara->SetStartIndex(ppara->GetStartIndex() - length);
    }
    EnsureTermNull();
    CheckIntegrity();
}

void StyledText::OnTextInserting(UPInt startPos, UPInt length, const wchar_t* ptxt)
{
    SF_UNUSED3(startPos, length, ptxt);
}

void StyledText::OnTextRemoving(UPInt startPos, UPInt length)
{
    SF_UNUSED2(startPos, length);
}

/*
void StyledText::OnParagraphTextFormatChanging(const Paragraph& para, UPInt startPos, UPInt endPos, const TextFormat& formatToBeSet)
{
SF_UNUSED(para);
SF_UNUSED3(startPos, endPos, formatToBeSet);
}
void StyledText::OnParagraphTextFormatChanged(const Paragraph& para, UPInt startPos, UPInt endPos)
{
SF_UNUSED(para);
SF_UNUSED2(startPos, endPos);
}

void StyledText::OnParagraphFormatChanging(const Paragraph& para, const ParagraphFormat& formatToBeSet)
{
SF_UNUSED(para);
SF_UNUSED(formatToBeSet);
}
void StyledText::OnParagraphFormatChanged(const Paragraph& para)
{
SF_UNUSED(para);
}

void StyledText::OnParagraphTextInserting(const Paragraph& para, UPInt insertionPos, UPInt insertingLen)
{
SF_UNUSED(para);
SF_UNUSED2(insertionPos, insertingLen);
}

void StyledText::OnParagraphTextInserted(const Paragraph& para, UPInt startPos, UPInt endPos, const wchar_t* ptextInserted)
{
SF_UNUSED(para);
SF_UNUSED3(startPos, endPos, ptextInserted);
}

void StyledText::OnParagraphTextRemoving(const Paragraph& para, UPInt removingPos, UPInt removingLen)
{
SF_UNUSED(para);
SF_UNUSED2(removingPos, removingLen);
}

void StyledText::OnParagraphTextRemoved(const Paragraph& para, UPInt removedPos, UPInt removedLen)
{
SF_UNUSED(para);
SF_UNUSED2(removedPos, removedLen);
}
*/
void StyledText::OnParagraphRemoving(const Paragraph& para)
{
    SF_UNUSED(para);
}

#ifdef SF_BUILD_DEBUG
void StyledText::CheckIntegrity() const
{
    ParagraphsIterator paraIter = GetParagraphIterator();
    UPInt pos = 0;
    for(; !paraIter.IsFinished(); ++paraIter)
    {
        Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        SF_ASSERT(ppara->GetSize() != 0);
        SF_ASSERT(ppara->GetLength() != 0 || GetLastParagraph() == ppara);
        ppara->CheckIntegrity();
        SF_ASSERT(pos == ppara->GetStartIndex());
        pos += ppara->GetLength();

        // only the last paragraph MUST have the terminal null
        SF_ASSERT(GetLastParagraph() != ppara || ppara->HasTermNull());
    }
}
#endif

//////////////////////////////////////////////////////////////////////////
// StyledText's iterators
//
StyledText::CharactersIterator::CharactersIterator(StyledText* ptext) : 
Paragraphs(ptext->GetParagraphIterator()), pText(ptext), FirstCharInParagraphIndex(0)
{
    if (!Paragraphs.IsFinished())
    {
        Paragraph* ppara = *Paragraphs;
        SF_ASSERT(ppara);
        FirstCharInParagraphIndex = ppara->GetStartIndex();
        Characters = ppara->GetCharactersIterator();
    }
}
StyledText::CharactersIterator::CharactersIterator(StyledText* ptext, int index) : pText(ptext)
{
    UPInt indexInPara;
    Paragraphs = ptext->GetParagraphByIndex(index, &indexInPara);
    if (!Paragraphs.IsFinished())
    {
        Paragraph* ppara = *Paragraphs;
        SF_ASSERT(ppara);
        FirstCharInParagraphIndex = ppara->GetStartIndex();
        Characters = ppara->GetCharactersIterator(indexInPara);
    }
}

StyledText::CharacterInfo& StyledText::CharactersIterator::operator*() 
{
    Paragraph::CharacterInfo& chInfo = *Characters;
    CharInfoPlaceHolder.Character = chInfo.Character;
    CharInfoPlaceHolder.Index     = chInfo.Index + FirstCharInParagraphIndex;
    CharInfoPlaceHolder.pOriginalFormat = chInfo.pFormat;
    CharInfoPlaceHolder.pParagraph = *Paragraphs;
    return CharInfoPlaceHolder;
}

void StyledText::CharactersIterator::operator++() 
{ 
    ++Characters;
    if (Characters.IsFinished())
    {
        ++Paragraphs;
        if (!Paragraphs.IsFinished())
        {
            Paragraph* ppara = *Paragraphs;
            SF_ASSERT(ppara);
            FirstCharInParagraphIndex = ppara->GetStartIndex();
            Characters = ppara->GetCharactersIterator();
        }
    }
}

void StyledText::Clear()
{
    for (UPInt i = 0, n = Paragraphs.GetSize(); i < n; ++i)
        Paragraphs[i]->FreeText(GetAllocator());
    Paragraphs.Resize(0);
    ClearMayHaveUrl();
}

//
// Reset all style properties
//
void Style::Reset()
{
    mTextFormat = TextFormat(mTextFormat.GetHeap());
    mParagraphFormat = ParagraphFormat();
}

}}} // Scaleform::Render::Text
