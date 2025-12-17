/**************************************************************************

PublicHeader:   Render
Filename    :   Text_StyledText.h
Content     :   Styled text implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_TEXT_GFXSTYLEDTEXT_H
#define INC_Render_TEXT_GFXSTYLEDTEXT_H

#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_WString.h"

#include "Render/Text/Text_Core.h"
#include "Render/Text/Text_SGMLParser.h"

namespace Scaleform { 
namespace Render {
namespace Text {

//
// CSS style object
//
struct Style : public NewOverrideBase<StatRender_Text_Mem>
{
    TextFormat          mTextFormat;
    ParagraphFormat     mParagraphFormat;

    Style(MemoryHeap *pheap) : mTextFormat(pheap) { }

    void                Reset();
};

// interface to CSS StyleManager
class StyleManagerBase
{
public:
    enum KeyType
    {
        CSS_Tag   = 0,
        CSS_Class = 1,
        CSS_None  = 2 // Invalid initialization.
    };

    virtual ~StyleManagerBase() {}

    virtual const Style*     GetStyle(KeyType type, const String& name) const =0;
    virtual const Style*     GetStyle(KeyType type, const char* name, UPInt len = SF_MAX_UPINT) const =0;
    virtual const Style*     GetStyle(KeyType type, const wchar_t* name, UPInt len = SF_MAX_UPINT) const =0;
};

#define GFX_NBSP_CHAR_CODE          160

// Represents a paragraph of text. Paragraph - is the text line terminating by new line symbol or '\0'
// Paragraphs are stored and operated by the StyledText class.
class Paragraph : public NewOverrideBase<StatRender_Text_Mem>
{
    friend class StyledText;
    friend class CompositionString;

    typedef RangeDataArray<Ptr<TextFormat>, ArrayLH<RangeData<Ptr<TextFormat> > > > TextFormatArrayType;
    typedef RangeData<Ptr<TextFormat> > TextFormatRunType;

    class TextBuffer
    {
        wchar_t* pText;
        UPInt    Size;
        UPInt    Allocated;

        TextBuffer(const TextBuffer& ) { SF_ASSERT(0); }
    public:
        TextBuffer(): pText(0), Size(0), Allocated(0) {}
        TextBuffer(const TextBuffer& o, Allocator* pallocator);
        /*GFxWideStringBuffer(Allocator* pallocator):pAllocator(pallocator), pText(0), Size(0), Allocated(0)
        {
        SF_ASSERT(pAllocator);
        }*/
        ~TextBuffer();

        wchar_t* ToWStr() const
        {
            return pText;
        }
        UPInt GetSize() const { return Size; }
        UPInt GetLength() const;
        const wchar_t* GetCharPtrAt(UPInt pos) const;

        void SetString(Allocator* pallocator, const char* putf8Str, UPInt utf8length = SF_MAX_UPINT);
        void SetString(Allocator* pallocator, const wchar_t* pstr, UPInt length = SF_MAX_UPINT);
        wchar_t* CreatePosition(Allocator* pallocator, UPInt pos, UPInt length);
        void Remove(UPInt pos, UPInt length);
        void Clear() { Size = 0; } // doesn't free mem!
        void Free(Allocator* pallocator);

        SPInt StrChr(wchar_t c) { return StrChr(pText, GetLength(), c); }
        void StripTrailingNewLines();
        void StripTrailingNull();
        void AppendTrailingNull(Allocator* pallocator);

        static SPInt StrChr(const wchar_t* ptext, UPInt length, wchar_t c);
        static UPInt StrLen(const wchar_t* ptext);

        //Allocator* GetAllocator() const { return pAllocator; }
    };

    //Allocator*   GetAllocator() const  { return Text.GetAllocator(); }
public:

    void SetStartIndex(UPInt i) { StartIndex = i; }

    // returns the actual pointer on text format at the specified position.
    // Will return NULL, if there is no text format at the "pos"
    TextFormat* GetTextFormatPtr(UPInt pos) const;
	void SetFormat(const ParagraphFormat* pfmt);

    void AppendTermNull(Allocator* pallocator, const TextFormat* pdefTextFmt);
    void RemoveTermNull();
    bool HasTermNull() const;
    bool HasNewLine() const;
    void SetTermNullFormat();

    // returns true if paragraph is empty (no chars or only termination null)
    bool IsEmpty() const { return GetLength() == 0; }

    void MarkToReformat() { ++ModCounter; }
public:
    // this struct is returned by FormatRunIterator::operator*
    struct StyledTextRun
    {
        const wchar_t*  pText;
        SPInt           Index;
        UPInt           Length;
        Ptr<TextFormat>  pFormat;

        StyledTextRun():pText(0), Index(0), Length(0) {}
        StyledTextRun(const wchar_t* ptext, SPInt index, UPInt len, TextFormat* pfmt)
        { Set(ptext, index, len, pfmt); }

        StyledTextRun& Set(const wchar_t* ptext, SPInt index, UPInt len, TextFormat* pfmt)
        {
            pText   = ptext;
            Index   = index;
            Length  = len;
            pFormat = pfmt;
            return *this;
        }
    };

    // Iterates through all format ranges in the paragraph, returning both
    // format structure and text chunks as StyledTextRun
    class FormatRunIterator
    {
        StyledTextRun                       PlaceHolder;
        const TextFormatArrayType*          pFormatInfo;
        TextFormatArrayType::ConstIterator  FormatIterator;
        const TextBuffer*                   pText;

        UPInt                               CurTextIndex;
    public:
        FormatRunIterator(const TextFormatArrayType& fmts, const TextBuffer& textHandle);
        FormatRunIterator(const TextFormatArrayType& fmts, const TextBuffer& textHandle, UPInt index);
        FormatRunIterator(const FormatRunIterator& orig);

        FormatRunIterator& operator=(const FormatRunIterator& orig);

        inline bool IsFinished() const  { return CurTextIndex >= pText->GetSize(); }

        const StyledTextRun& operator* ();
        const StyledTextRun* operator->() { return &operator*(); }
        void operator++();
        inline void operator++(int) { operator++(); }

        void SetTextPos(UPInt newTextPos);
    };

    struct CharacterInfo
    {
        Ptr<TextFormat>     pFormat;
        UPInt               Index;
        wchar_t             Character;

        CharacterInfo() : pFormat(NULL), Index(0), Character(0) {}
        CharacterInfo(wchar_t c, UPInt index, TextFormat* pf) : pFormat(pf), Index(index), Character(c) {}
    };
    class CharactersIterator
    {
        CharacterInfo                       PlaceHolder;
        const TextFormatArrayType*          pFormatInfo;
        TextFormatArrayType::ConstIterator  FormatIterator;
        const TextBuffer*                   pText;

        UPInt                               CurTextIndex;
    public:
        CharactersIterator() : pFormatInfo(NULL), pText(NULL), CurTextIndex(0) {}
        CharactersIterator(const TextBuffer& buf) : pFormatInfo(NULL), pText(&buf), CurTextIndex(0) {}
        CharactersIterator(const Paragraph* pparagraph);
        CharactersIterator(const Paragraph* pparagraph, UPInt index);

        inline bool IsFinished() const { return pText == NULL || CurTextIndex >= pText->GetSize(); }

        CharacterInfo&              operator*();
        const CharacterInfo&        operator*() const
        {
            return const_cast<CharactersIterator*>(this)->operator*();
        }
        inline CharacterInfo*       operator->() { return &operator*(); }
        inline const CharacterInfo* operator->() const { return &operator*(); }

        void operator++();
        void operator++(int) { operator++(); }
        void operator+=(UPInt n);

        const wchar_t* GetRemainingTextPtr(UPInt * plen) const;
        UPInt GetCurTextIndex() const { return CurTextIndex; }
    };
    CharactersIterator GetCharactersIterator() const            { return CharactersIterator(this); }
    CharactersIterator GetCharactersIterator(UPInt index) const { return CharactersIterator(this, index); }

private:
    Paragraph(const Paragraph& o);
public:
    Paragraph(Allocator* ptextAllocator);
    Paragraph(const Paragraph& o, Allocator* ptextAllocator);
    Paragraph() {  } // shouldn't be used! 
    ~Paragraph() {}

    FormatRunIterator GetIterator() const;
    FormatRunIterator GetIteratorAt(UPInt index) const;

    UPInt GetStartIndex() const { return StartIndex; }
    // returns length, not including terminal null (if exists)
    UPInt GetLength() const;
    // returns length, including terminal null (if exists)
    UPInt GetSize() const       { return Text.GetSize(); }
    UPInt GetNextIndex() const  { return StartIndex + GetLength(); }

    const wchar_t* GetText() const  { return Text.ToWStr(); }
    wchar_t*       GetText()        { return Text.ToWStr(); }

    wchar_t* CreatePosition(Allocator* pallocator, UPInt pos, UPInt length = 1);
    void InsertString(Allocator* pallocator, const wchar_t* pstr, UPInt pos, UPInt length, const TextFormat* pnewFmt);
    void InsertString(Allocator* pallocator, const wchar_t* pstr, UPInt pos, UPInt length = SF_MAX_UPINT)
    {
        InsertString(pallocator, pstr, pos, length, NULL);
    }
    // AppendPlainText methods doesn't expand format ranges
    void AppendPlainText(Allocator* pallocator, const wchar_t* pstr, UPInt length = SF_MAX_UPINT);
    void AppendPlainText(Allocator* pallocator, const char* putf8str, UPInt utf8StrSize = SF_MAX_UPINT);

    void Remove(UPInt startPos, UPInt endPos = SF_MAX_UPINT);
    void Clear();
    void FreeText(Allocator* pallocator) { Text.Free(pallocator); }

    void SetText(Allocator* pallocator, const wchar_t* pstring, UPInt length = SF_MAX_UPINT);
    void SetTextFormat(Allocator* pallocator, const TextFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);
    TextFormat GetTextFormat(UPInt startPos, UPInt endPos = SF_MAX_UPINT) const;

    void ClearTextFormat(UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);

    void SetFormat(Allocator* pallocator, const ParagraphFormat& fmt);
    const ParagraphFormat* GetFormat() const { return pFormat.GetPtr(); }

    UInt32 GetId() const        { return UniqueId; }
    UInt16 GetModCounter() const{ return ModCounter; }

    // copies text and formatting from psrcPara paragraph to this one, starting from startSrcIndex in
    // source paragraph and startDestIndex in destination one. The "length" specifies the number
    // of positions to copy.
    void Copy(Allocator* pallocator, const Paragraph& psrcPara, UPInt startSrcIndex, UPInt startDestIndex, UPInt length);

    // Shrinks the paragraph's length by the "delta" value.
    void Shrink(UPInt delta);

#ifdef SF_BUILD_DEBUG
    void CheckIntegrity() const;
#else
    void CheckIntegrity() const {}
#endif

private: // data
    TextBuffer              Text;
    Ptr<ParagraphFormat>    pFormat;
    TextFormatArrayType     FormatInfo;
    UPInt                   StartIndex;
    UInt32                  UniqueId;
    UInt16                  ModCounter;
};

// This class represents the storage for the styled rich text. It doesn't
// have any formatting or rendering functionality, it can just store and operate
// with rich text. DocView formats the styled text and the 
// GFxTextLineBuffer renders it.
class StyledText : public RefCountBaseNTS<StyledText, StatRender_Text_Mem>
{
    friend class DocView;
    friend class CompositionString;
//    friend class EditorKit;
public:
    class ParagraphPtrWrapper
    {
        mutable Paragraph* pPara;
    public:

        ParagraphPtrWrapper():pPara(NULL) {}
        ParagraphPtrWrapper(Paragraph* ptr) : pPara(ptr) {}
        ParagraphPtrWrapper(const Paragraph* ptr) : pPara(const_cast<Paragraph*>(ptr)) {}

        ParagraphPtrWrapper(const ParagraphPtrWrapper& ptr) : pPara(ptr.pPara) { ptr.pPara = NULL; }
        ~ParagraphPtrWrapper() { delete pPara; }

        Paragraph& operator*() { return *pPara; }
        const Paragraph& operator*() const { return *pPara; }

        Paragraph* operator-> () { return pPara; }
        const Paragraph* operator-> () const { return pPara; }

        Paragraph* GetPtr() { return pPara; }
        const Paragraph* GetPtr() const { return pPara; }

        operator Paragraph*()                { return pPara; }
        operator const Paragraph*() const    { return pPara; }

        ParagraphPtrWrapper& operator=(const Paragraph* p)
        {
            if (pPara != const_cast<Paragraph*>(p))
                delete pPara;
            pPara = const_cast<Paragraph*>(p);
            return *this;
        }
        ParagraphPtrWrapper& operator=(Paragraph* p)
        {
            if (pPara != p)
                delete pPara;
            pPara = p;
            return *this;
        }
        ParagraphPtrWrapper& operator=(const ParagraphPtrWrapper& p)
        {
            if (pPara != p.pPara)
                delete pPara;
            pPara = p.pPara;
            p.pPara = NULL;
            return *this;
        }
        bool operator==(const ParagraphPtrWrapper& p) const
        {
            return pPara == p.pPara;
        }
        bool operator==(const Paragraph* p) const
        {
            return pPara == p;
        }
    };
    struct HTMLImageTagInfo : public NewOverrideBase<StatRender_Text_Mem>
    {
        Ptr<ImageDesc>  pTextImageDesc;
        StringDH        Url;
        StringDH        Id;
        unsigned        Width, Height; // in twips
        int             VSpace, HSpace; // in twips
        unsigned        ParaId;
        enum
        {
            Align_BaseLine,
            Align_Right,
            Align_Left
        };
        UByte        Alignment;

        HTMLImageTagInfo(MemoryHeap* pheap) : Url(pheap), Id(pheap), Width(0), Height(0), 
            VSpace(0), HSpace(0), ParaId(~0u), Alignment(Align_BaseLine) {}
    };

    typedef ArrayDH<HTMLImageTagInfo>           HTMLImageTagInfoArray;
    typedef ArrayLH<ParagraphPtrWrapper>        ParagraphArrayType;
    typedef ParagraphArrayType::Iterator        ParagraphsIterator;
    typedef ParagraphArrayType::ConstIterator   ParagraphsConstIterator;

    struct CharacterInfo
    {
        Ptr<TextFormat> pOriginalFormat;
        Paragraph*      pParagraph;
        UPInt           Index;
        wchar_t         Character;

        CharacterInfo() : Index(0), Character(0) {}
    };

    class CharactersIterator
    {
        ParagraphsIterator              Paragraphs;
        Paragraph::CharactersIterator   Characters;
        Ptr<StyledText>                 pText;
        UPInt                           FirstCharInParagraphIndex;
        CharacterInfo                   CharInfoPlaceHolder;
    public:
        CharactersIterator(StyledText* ptext);
        CharactersIterator(StyledText* ptext, int index);
        inline bool IsFinished() const { return Characters.IsFinished() && Paragraphs.IsFinished(); }

        CharacterInfo& operator*();
        void operator++();
        inline void operator++(int) { operator++(); }
    };
private:
    struct ParagraphComparator
    {
        static int Compare(const Paragraph* pp1, UPInt index)
        {
            UPInt si1 = pp1->GetStartIndex();
            if (index >= si1 && index < (si1 + pp1->GetSize()))
                return 0; 
            return (int)(si1 - index);
        }
        static int Less(const Paragraph* pp1, UPInt index)
        {
            return Compare(pp1, index) < 0;
        }
    };
    void EnsureTermNull();
public:
    StyledText();
    StyledText(Allocator* pallocator);
    ~StyledText() { Clear(); }

    void Clear();

    UPInt GetLength() const;

    void SetText(const char* putf8String, UPInt stringSize = SF_MAX_UPINT);
    void SetText(const wchar_t* pstring, UPInt length = SF_MAX_UPINT);

    String GetText() const;
    String& GetText(String*) const;
    void GetText(WStringBuffer* pBuffer) const;
    void GetText(WStringBuffer* pBuffer,UPInt startPos, UPInt endPos) const;

    void CopyStyledText(StyledText* pdest, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT) const;
    StyledText* CopyStyledText(UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT) const;

    void SetHtml(const String&);
    void SetHtml(const wchar_t* pstring, UPInt length = SF_MAX_UPINT);
    String GetHtml() const;
    String& GetHtml(String*) const;
    StringBuffer& GetHtml(StringBuffer*) const;

    void GetTextAndParagraphFormat(TextFormat* pdestTextFmt, ParagraphFormat* pdestParaFmt, UPInt startPos, UPInt endPos = SF_MAX_UPINT);
    bool GetTextAndParagraphFormat(const TextFormat** ppdestTextFmt, const ParagraphFormat** pdestParaFmt, UPInt pos);
    void SetTextFormat(const TextFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);
    void SetParagraphFormat(const ParagraphFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);

    // wipe out text format for the specified range. Might be useful, if need to set
    // format without merging with existing one.
    void ClearTextFormat(UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);

    // assign pdefaultTextFmt as a default text format
    void SetDefaultTextFormat(const TextFormat* pdefaultTextFmt);
    // makes a copy of text format and assign
    void SetDefaultTextFormat(const TextFormat& defaultTextFmt);
    // assign pdefaultTextFmt as a default text format
    void SetDefaultParagraphFormat(const ParagraphFormat* pdefaultParagraphFmt);
    // makes a copy of text format and assign
    void SetDefaultParagraphFormat(const ParagraphFormat& defaultParagraphFmt);

    const TextFormat*          GetDefaultTextFormat() const 
    { SF_ASSERT(pDefaultTextFormat); return pDefaultTextFormat; }
    const ParagraphFormat* GetDefaultParagraphFormat() const 
    { SF_ASSERT(pDefaultParagraphFormat); return pDefaultParagraphFormat; }

    const Paragraph* GetParagraph(unsigned paragraphIndex) const
    {
        return (paragraphIndex < GetParagraphsCount()) ? Paragraphs[paragraphIndex].GetPtr() : NULL;
    }
    unsigned GetParagraphsCount() const { return (unsigned)Paragraphs.GetSize(); }

    wchar_t* CreatePosition(UPInt pos, UPInt length = 1);

    // Insert/append funcs
    enum NewLinePolicy
    {
        NLP_CompressCRLF, // CR LF will be compressed into one EOL
        NLP_ReplaceCRLF,  // each CR and/or LF will be replaced by EOL
        NLP_IgnoreCRLF    // CR LF will be ignored (for single line textfields)
    };
    UPInt AppendString(const char* putf8String, UPInt stringSize = SF_MAX_UPINT, 
        NewLinePolicy newLinePolicy = NLP_ReplaceCRLF);
    UPInt AppendString(const char* putf8String, UPInt stringSize, 
        NewLinePolicy newLinePolicy,  
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt);
    UPInt AppendString(const char* putf8String, UPInt stringSize, 
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt)
    {
        return AppendString(putf8String, stringSize, NLP_ReplaceCRLF, pdefTextFmt, pdefParaFmt);
    }
    UPInt AppendString(const wchar_t* pstr, UPInt length = ~0u,
        NewLinePolicy newLinePolicy = NLP_ReplaceCRLF);
    UPInt AppendString(const wchar_t* pstr, UPInt length,
        NewLinePolicy newLinePolicy,
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt);
    UPInt AppendString(const wchar_t* pstr, UPInt length,
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt)
    {
        return AppendString(pstr, length, NLP_ReplaceCRLF, pdefTextFmt, pdefParaFmt);
    }

    UPInt InsertString(const wchar_t* pstr, UPInt pos, UPInt length = SF_MAX_UPINT,
        NewLinePolicy newLinePolicy = NLP_ReplaceCRLF);
    UPInt InsertString(const wchar_t* pstr, UPInt pos, UPInt length, 
        NewLinePolicy newLinePolicy,
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt);
    UPInt InsertString(const wchar_t* pstr, UPInt pos, UPInt length, 
        const TextFormat* pdefTextFmt, const ParagraphFormat* pdefParaFmt)
    {
        return InsertString(pstr, pos, length, NLP_ReplaceCRLF, pdefTextFmt, pdefParaFmt);
    }
    // insert styled text
    UPInt InsertStyledText(const StyledText& text, UPInt pos, UPInt length = SF_MAX_UPINT);

    void Remove(UPInt startPos, UPInt length);

    CharactersIterator GetCharactersIterator() { return CharactersIterator(this); }

    void SetNewLine0D()        { RTFlags |= RTFlags_NewLine0D; }
    void ClearNewLine0D()      { RTFlags &= (~(RTFlags_NewLine0D)); }
    bool IsNewLine0D() const   { return (RTFlags & (RTFlags_NewLine0D)) != 0; }
    unsigned char NewLineChar() const { return IsNewLine0D() ? '\r' : '\n'; }
    const char*   NewLineStr()  const { return IsNewLine0D() ? "\r" : "\n"; }

#ifdef SF_BUILD_DEBUG
    void CheckIntegrity() const;
#else
    void CheckIntegrity() const {}
#endif

private:
    template <class Char>
    bool ParseHtmlImpl(const Char* phtml, UPInt  htmlSize, HTMLImageTagInfoArray* pimgInfoArr = NULL, 
        bool multiline = true, bool condenseWhite = false, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL);

public:
    ParagraphsIterator GetParagraphByIndex(UPInt index, UPInt* pindexInParagraph = NULL);
    ParagraphsIterator GetParagraphByIndex(UPInt index, UPInt* pindexInParagraph = NULL) const
    {
        return const_cast<StyledText*>(this)->GetParagraphByIndex(index, pindexInParagraph);
    }
protected:
    bool ParseHtml(const char* phtml, UPInt  htmlSize, HTMLImageTagInfoArray* pimgInfoArr = NULL, 
        bool multiline = true, bool condenseWhite = false, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL);
    bool ParseHtml(const wchar_t* phtml, UPInt  htmlSize, HTMLImageTagInfoArray* pimgInfoArr = NULL, 
        bool multiline = true, bool condenseWhite = false, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL);

    void RemoveParagraph(ParagraphsIterator& paraIter, Paragraph* ppara);
    MemoryHeap* GetHeap() const
    {
        if (!pTextAllocator)
            return Memory::GetHeapByAddress(this);
        return pTextAllocator->GetHeap();
    }
    Allocator* GetAllocator() const
    {
        if (!pTextAllocator)
        {
            MemoryHeap* pheap = Memory::GetHeapByAddress(this);
            pTextAllocator = *SF_HEAP_NEW(pheap) Allocator(pheap);
        }
        return pTextAllocator;
    }

    void SetMayHaveUrl()        { RTFlags |= RTFlags_MayHaveUrl; }
    void ClearMayHaveUrl()      { RTFlags &= (~(RTFlags_MayHaveUrl)); }
    bool MayHaveUrl() const     { return (RTFlags & (RTFlags_MayHaveUrl)) != 0; }

    ParagraphsIterator GetNearestParagraphByIndex(UPInt index, UPInt* pindexInParagraph = NULL);
    ParagraphsIterator GetNearestParagraphByIndex(UPInt index, UPInt* pindexInParagraph = NULL) const
    {
        return const_cast<StyledText*>(this)->GetNearestParagraphByIndex(index, pindexInParagraph);
    }
    ParagraphsIterator GetParagraphIterator() { return Paragraphs.Begin(); }
    ParagraphsIterator GetParagraphIterator() const { return const_cast<StyledText*>(this)->Paragraphs.Begin(); }

    Paragraph* AppendNewParagraph(const ParagraphFormat* pdefParaFmt = NULL);
    Paragraph* InsertNewParagraph(StyledText::ParagraphsIterator& iter, 
        const ParagraphFormat* pdefParaFmt = NULL);
    Paragraph* AppendCopyOfParagraph(const Paragraph& srcPara);
    Paragraph* InsertCopyOfParagraph(StyledText::ParagraphsIterator& iter, 
        const Paragraph& srcPara);

    Paragraph*       GetLastParagraph();
    const Paragraph* GetLastParagraph() const;

    // atomic callbacks. These methods might be invoked multiple times 
    // from inside of the InsertString or Remove methods. Client SHOULD NOT do
    // anything "heavy" in these handlers, like re-formatting (though, the flag 
    // "re-formatting is neeeded" might be set).
    //virtual void OnParagraphTextFormatChanging(const Paragraph& para, UPInt startPos, UPInt endPos, const TextFormat& formatToBeSet);
    //virtual void OnParagraphTextFormatChanged(const Paragraph& para, UPInt startPos, UPInt endPos);
    //virtual void OnParagraphFormatChanging(const Paragraph& para, const ParagraphFormat& formatToBeSet);
    //virtual void OnParagraphFormatChanged(const Paragraph& para);
    //virtual void OnParagraphTextInserting(const Paragraph& para, UPInt insertionPos, UPInt insertingLen);
    //virtual void OnParagraphTextInserted(const Paragraph& para, UPInt startPos, UPInt endPos, const wchar_t* ptextInserted);
    //virtual void OnParagraphTextRemoving(const Paragraph& para, UPInt removingPos, UPInt removingLen);
    //virtual void OnParagraphTextRemoved(const Paragraph& para, UPInt removedPos, UPInt removedLen);
    virtual void OnTextInserting(UPInt startPos, UPInt length, const wchar_t* ptxt);
    virtual void OnTextInserting(UPInt startPos, UPInt length, const char* ptxt)
    { SF_UNUSED3(startPos, length, ptxt); };
    virtual void OnTextRemoving(UPInt startPos, UPInt length);
    virtual void OnTextInserted(UPInt startPos, UPInt length, const wchar_t* ptxt) 
    { SF_UNUSED3(startPos, length, ptxt); };
    virtual void OnParagraphRemoving(const Paragraph& para);

    // This callback will be fired at the end of big change, like InsertString, Remove, etc
    virtual void OnDocumentChanged() {}
protected: //data
    mutable Ptr<Allocator>  pTextAllocator;
    ParagraphArrayType      Paragraphs;
    Ptr<ParagraphFormat>    pDefaultParagraphFormat;
    Ptr<TextFormat>         pDefaultTextFormat;

    enum 
    {
        RTFlags_MayHaveUrl = 0x1,
        RTFlags_NewLine0D  = 0x2 // use '\r' as internal new-line char
    };
    UByte                           RTFlags;
};

}}} // Scaleform::Render::Text

#endif // INC_TEXT_GFXSTYLEDTEXT_H
