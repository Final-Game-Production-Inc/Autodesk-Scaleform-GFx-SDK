/**************************************************************************

PublicHeader:   Render
Filename    :   Text_DocView.h
Content     :   Document-view implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_TEXT_TEXTDOCVIEW_H
#define INC_Render_TEXT_TEXTDOCVIEW_H

#include "Kernel/SF_Range.h"
#include "Text_StyledText.h"
#include "Text_FilterDesc.h"
#include "Text_LineBuffer.h"

#ifdef SF_OS_WIN32
#undef ReplaceText
#endif

namespace Scaleform { 

class Log;

namespace Render {
namespace Text {

struct HighlightDesc;
class Highlighter;

// An interface to IME composition string
class CompositionStringBase : public RefCountBase<CompositionStringBase, StatRender_Text_Mem>
{
public:
	virtual UPInt GetPosition() const =0;
	virtual UPInt GetLength() const   =0;
	virtual const wchar_t* GetText() const  =0;
	virtual TextFormat* GetTextFormat(UPInt pos) =0;
	virtual Allocator*   GetAllocator() const =0;
};

// An interface for editor kit
class EditorKitBase : public RefCountBase<EditorKitBase, StatRender_Text_Mem>
{
public:
    virtual ~EditorKitBase() {}

    virtual void AddDrawCursorInfo(TextLayout::Builder& bld) =0;
    virtual bool IsReadOnly() const = 0;
    virtual bool HasCompositionString() const = 0;
    virtual UPInt TextPos2GlyphOffset(UPInt textPos) =0;
    virtual UPInt TextPos2GlyphPos(UPInt textPos) =0;
    virtual UPInt GlyphPos2TextPos(UPInt glyphPos) =0;
    virtual CompositionStringBase* GetCompositionString() =0;
};

class StyleManagerBase;

#define GFX_EDIT_HSCROLL_DELTA PixelsToFixp(60)

// This class performs centralized management for rich text. It contains
// an instance of StyledText as a storage of rich text data, LineBuffer
// instance to perform rendering of the formatted text and the EditorKit
// to perform editing, GFxTextHighlight to perform highlighting. 
// This class contains functionality for formatting rich text.
class DocView : public RefCountBase<DocView, StatRender_Text_Mem>
{
    friend class EditorKitBase;
    friend class CompositionStringBase;
    friend class LineCursor;
    friend class ParagraphFormatter;

    typedef Matrix2F                     Matrix;
    typedef Render::Cxform               Cxform;
public:
    typedef HashIdentityDH<const TextFormat*, Ptr<FontHandle>, StatRender_Text_Mem> FontCache;
public:
    // editor's commands
    enum CommandType
    {
        Cmd_InsertChar,
        Cmd_InsertPlainText,
        Cmd_InsertStyledText,
        Cmd_DeleteChar,
        Cmd_DeleteText,
        Cmd_ReplaceTextByChar,
        Cmd_ReplaceTextByPlainText,
        Cmd_ReplaceTextByStyledText,
        Cmd_BackspaceChar
    };
    struct InsertCharCommand
    {
        UPInt   PosAt;
        wchar_t CharCode;

        InsertCharCommand(UPInt pos, wchar_t c):PosAt(pos), CharCode(c) {}
    };
    struct InsertPlainTextCommand
    {
        UPInt           PosAt;
        const wchar_t*  pStr;
        UPInt           Length;

        InsertPlainTextCommand(UPInt pos, const wchar_t* pstr, UPInt len):PosAt(pos), pStr(pstr), Length(len) {}
    };
    struct InsertStyledTextCommand
    {
        UPInt               PosAt;
        const StyledText*   pText;

        InsertStyledTextCommand(UPInt pos, const StyledText* pstr):PosAt(pos), pText(pstr) {}
    };
    struct DeleteCharCommand
    {
        UPInt   PosAt;

        DeleteCharCommand(UPInt pos):PosAt(pos) {}
    };
    struct DeleteTextCommand
    {
        UPInt   BeginPos;
        UPInt   EndPos;

        DeleteTextCommand(UPInt startPos, UPInt endPos):
        BeginPos(startPos), EndPos(endPos) {}
    };
    struct ReplaceTextByCharCommand
    {
        UPInt   BeginPos;
        UPInt   EndPos;
        wchar_t CharCode;

        ReplaceTextByCharCommand(UPInt startPos, UPInt endPos, wchar_t c):
        BeginPos(startPos), EndPos(endPos), CharCode(c) {}
    };
    struct ReplaceTextByPlainTextCommand
    {
        UPInt           BeginPos;
        UPInt           EndPos;
        const wchar_t*  pStr;
        UPInt           Length;

        ReplaceTextByPlainTextCommand(UPInt startPos, UPInt endPos,  const wchar_t* pstr, UPInt len):
        BeginPos(startPos), EndPos(endPos), pStr(pstr), Length(len) {}
    };
    struct ReplaceTextByStyledTextCommand
    {
        UPInt               BeginPos;
        UPInt               EndPos;
        const StyledText*   pText;

        ReplaceTextByStyledTextCommand(UPInt startPos, UPInt endPos, const StyledText* pstr):
        BeginPos(startPos), EndPos(endPos), pText(pstr) {}
    };

    // structure used for DocumentListener::Format_OnLineFormat. All widths are in twips.
    struct LineFormatDesc
    {
        const wchar_t*  pParaText;              // [in] paragraph text
        UPInt           ParaTextLen;            // [in] length, in chars
        float*          pWidths;                // [in] arr of line widths before char at the index, size = NumCharsInLine + 1
        UPInt           LineStartPos;           // [in] text position in paragraph of first char in line
        UPInt           NumCharsInLine;         // [in] count of chars currently in the line        
        float           VisibleRectWidth;       // [in] width of client rect
        float           CurrentLineWidth;       // [in] current line width
        float           LineWidthBeforeWordWrap;// [in] line width till ProposedWordWrapPoint
        float           DashSymbolWidth;        // [in] may be used to calculate hyphenation
        enum                                    
        {
            Align_Left      = 0,
            Align_Right     = 1,
            Align_Center    = 2,
            Align_Justify   = 3
        };
        UInt8           Alignment;              // [in] alignment of the line

        UPInt           ProposedWordWrapPoint;  // [in,out] text index of proposed word wrap pos,
        //          callback may change it to move wordwrap point
        bool            UseHyphenation;         // [out]    callback may set it to indicate to use hyphenation
    };

    // document listener to pass events outside of the docview
    class DocumentListener : public RefCountBaseNTS<DocumentListener, StatRender_Text_Mem>
    {
    protected:
        enum
        {
            Mask_OnLineFormat       = 0x01,
            Mask_OnScroll           = 0x02,
            Mask_OnMaxScrollChanged = 0x04,
            Mask_OnViewChanged      = 0x08,
            Mask_BidirectionalText  = 0x10,

            Mask_All = (Mask_OnLineFormat | Mask_OnScroll | Mask_OnMaxScrollChanged | Mask_OnViewChanged)
        };
        UInt8           HandlersMask;           
    public:
        DocumentListener(UInt8 handlersMask = 0):HandlersMask(handlersMask) {}
        bool DoesHandleLineFormat() const  { return (HandlersMask & Mask_OnLineFormat) != 0; }
        bool DoesHandleOnScroll() const    { return (HandlersMask & Mask_OnScroll) != 0; }
        bool DoesHandleOnMaxScrollChanged() const { return (HandlersMask & Mask_OnMaxScrollChanged) != 0; }
        bool DoesHandleOnViewChanged() const { return (HandlersMask & Mask_OnViewChanged) != 0; }
        bool DoesHandleBidirectionalText() const { return (HandlersMask & Mask_BidirectionalText) != 0; }

        // fired if Mask_OnLineFormat bit is set and line is formatted and 
        // ready to be committed. Might be used to implement custom word wrapping
        // or hyphenation.
        virtual bool    View_OnLineFormat(DocView&, LineFormatDesc&) { return false; }

        // A callback that is called on bidirectional text enabled textfields. This method
        // receives original text ('text'/'textLen' parameters) and pre-allocated buffers: 
        //   'newText'  - new re-ordered text should be put there, do not overrun the length (the length is the same as 'textLen')
        //   'indexMap' - a buffer for an array of unsigned integers, that should be filled with new indices of each char. For
        //                example, if original char at index 0 was relocated to index 10, then indexMap[0] = 10.
        //   'mirrorBits'- a buffer for an array of bools (length = 'textLen'), where index is an index of char in 'newText' and the value (true/false)
        //                indicates necessity of the glyph mirroring (for example, for integral sign).
        // Should return 'true' if method was successful and the core should use contents of the buffers; false, if no changes
        // to the textfield should be applied.
        virtual bool    View_PrepareBidiText(DocView&, const wchar_t* paraText, UPInt textLen, 
                                             wchar_t* newParaText, unsigned* indexMap, bool* mirroredBits)
        {
            SF_UNUSED5(paraText, textLen, newParaText, indexMap, mirroredBits);
            return false;
        }

        // view-related events
        virtual void    View_OnHScroll(DocView& , unsigned newScroll) { SF_UNUSED(newScroll); }
        virtual void    View_OnVScroll(DocView& , unsigned newScroll) { SF_UNUSED(newScroll); }
        virtual void    View_OnMaxScrollChanged(DocView&) {  }
        // fired, if view needs to be repainted, for example if formatting is changed,
        // selection truned on/off, etc. Fired only if Mask_OnViewChanged bit is set.
        virtual void    View_OnChanged(DocView&)                  {}

        // editor-related events
        virtual unsigned Editor_OnKey(EditorKitBase& , unsigned keyCode) { return keyCode; }
        virtual wchar_t  Editor_OnCharacter(EditorKitBase& , wchar_t srcChar) { return srcChar; }
        virtual void     Editor_OnChanged(EditorKitBase& ) {  }
        virtual void     Editor_OnCursorMoved(EditorKitBase& ) {  }
        virtual void     Editor_OnCursorBlink(EditorKitBase&, bool cursorState) { SF_UNUSED(cursorState); }
        
        virtual bool     Editor_OnInsertingText(EditorKitBase&, UPInt pos, UPInt len, const wchar_t*) { SF_UNUSED2(pos, len); return true; }
        virtual bool     Editor_OnRemovingText(EditorKitBase&, UPInt pos, UPInt len) { SF_UNUSED2(pos, len); return true; }

        virtual String GetCharacterPath() { return String(); }
    };

    // View's part
    enum ViewAlignment
    {
        Align_Left   = 0,
        Align_Right  = 1,
        Align_Center = 2,
    };
    enum ViewVAlignment
    {
        VAlign_None   = 0,
        VAlign_Top    = 1,
        VAlign_Bottom = 2,
        VAlign_Center = 3,
    };
    enum ViewTextAutoSize
    {
        TAS_None   = 0,
        TAS_Shrink = 1,
        TAS_Fit    = 2
    };
    enum 
    {
        Align_Mask   = 0x3,
        Align_Shift  = 0,

        VAlign_Mask   = 0x3,
        VAlign_Shift  = 2,

        TAS_Mask  = 0x3,
        TAS_Shift = 4
    };

    struct ImageSubstitutor : public NewOverrideBase<StatRender_Text_Mem>
    {
        struct Element
        {
            wchar_t                 SubString[20];
            Ptr<ImageDesc>  pImageDesc;
            UByte                   SubStringLen;
        };
        ArrayLH<Element>           Elements;

        ImageDesc* FindImageDesc(const wchar_t* pstr, UPInt  maxlen, UPInt * ptextLen = NULL);
        void AddImageDesc(const Element& elem);
        void RemoveImageDesc(ImageDesc*);
    };
    struct FindFontInfo
    {
        FontCache*          pFontCache;
        const TextFormat*   pCurrentFormat;
        const TextFormat*   pPrevFormat;
        Ptr<FontHandle>     pCurrentFont;

        explicit FindFontInfo(FontCache* pfc = NULL) : pFontCache(pfc) { init(); }

        inline void init() { pCurrentFormat = pPrevFormat = NULL; pCurrentFont = NULL; }
    };

    Allocator* GetAllocator() { return pDocument->GetAllocator(); }
protected:
    void ClearReformatReq()   { RTFlags &= (~(RTFlags_ReformatReq | RTFlags_CompleteReformatReq)); }

    void ClearCompleteReformatReq()   { RTFlags &= (~RTFlags_CompleteReformatReq); }
    bool IsCompleteReformatReq() const{ return (RTFlags & RTFlags_CompleteReformatReq) != 0; }

    // Finds font using pfontInfo. 'quietMode'==true allows to turn off error reporting
    // in the case if font is not found.
    // This method modifies the pfontInfo.
    FontHandle* FindFont(FindFontInfo* pfontInfo, bool quietMode);

    UPInt GetCursorPosInLineByOffset(unsigned lineIndex, float relativeOffsetX);

    StyledText::NewLinePolicy GetNewLinePolicy() const
    {
        return (DoesCompressCRLF()) ? StyledText::NLP_CompressCRLF: StyledText::NLP_ReplaceCRLF;
    }

    enum UseType { UseInternally, UseExternally };
    // if ut == UseInternally it will not try to reformat the text
    void SetViewRect(const RectF& rect, UseType ut);

    HighlightDesc* GetSelectionHighlighterDesc();
    HighlightDesc* GetSelectionHighlighterDesc() const;

    // convert glyph pos (taken from LineBuffer to-from text pos (as in StyledText)
    UPInt TextPos2GlyphOffset(UPInt textPos);
    UPInt TextPos2GlyphPos(UPInt textPos);
    UPInt GlyphPos2TextPos(UPInt glyphPos);

    // returns true, if at least one of the paragraphs contains non-left alignment.
    bool ContainsNonLeftAlignment() const;

    void SetBitmapFontFlag()     { RTFlags |= RTFlags_BitmapFontUsed; }
    void ClearBitmapFontFlag()   { RTFlags &= (~RTFlags_BitmapFontUsed); }
    bool IsBitmapFontFlagSet() const { return (RTFlags & RTFlags_BitmapFontUsed) != 0; }

    // run-time flag, set when a textfield is actually using bidirectional text
    void SetBidirectionalTextRTFlag()     { RTFlags |= RTFlags_HasBidirectionalText; }
    void ClearBidirectionalTextRTFlag()   { RTFlags &= (~RTFlags_HasBidirectionalText); }
    bool IsBidirectionalTextRTFlagSet() const { return (RTFlags & RTFlags_HasBidirectionalText) != 0; }
public:
    // types of view notifications, used for NotifyViewsChanged/OnDocumentChanged
    enum ViewNotificationMasks
    {
        ViewNotify_SignificantMask          = 0x100,
        ViewNotify_FormatChange             = 0x1,
        ViewNotify_TextChange               = 0x2,
        ViewNotify_ScrollingParamsChange    = 0x4,

        ViewNotify_SignificantFormatChange  = ViewNotify_SignificantMask | ViewNotify_FormatChange,
        ViewNotify_SignificantTextChange    = ViewNotify_SignificantMask | ViewNotify_TextChange,
    };


    DocView(Allocator* pallocator, FontManagerBase* pfontMgr, Log* plog);
    ~DocView();

    bool IsReformatReq() const{ return (RTFlags & (RTFlags_ReformatReq | RTFlags_CompleteReformatReq)) != 0; }

    MemoryHeap* GetHeap() const { return pDocument->GetHeap(); }

    UPInt   GetLength() const { return pDocument->GetLength(); }
    String  GetText() const;
    String& GetText(String*) const;

    String  GetHtml() const;
    String& GetHtml(String*) const;
    StringBuffer& GetHtml(StringBuffer*) const;

    StyledText* GetStyledText() const { return pDocument; }
    FontManagerBase* GetFontManager() const { return pFontManager; }

    void SetText(const char* putf8String, UPInt stringSize = SF_MAX_UPINT);
    void SetText(const wchar_t* pstring, UPInt length = SF_MAX_UPINT);

    void ParseHtml(const String& str, bool condenseWhite, 
        StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL)
    {
        ParseHtml(str.ToCStr(), str.GetLength(), condenseWhite, pimgInfoArr, pstyleMgr, defTxtFmt, defParaFmt);
    }
    void ParseHtml(const char* putf8Str, UPInt utf8Len, bool condenseWhite, 
        StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL);

    void ParseHtml(const char* putf8Str, UPInt utf8Len = SF_MAX_UPINT, 
        StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL)
    {
        ParseHtml(putf8Str, utf8Len, false, pimgInfoArr, pstyleMgr, defTxtFmt, defParaFmt);
    }
    void ParseHtml(const wchar_t* pwStr, UPInt strLen = SF_MAX_UPINT, bool condenseWhite = false, 
        StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL, const StyleManagerBase* pstyleMgr = NULL,
        const TextFormat* defTxtFmt = NULL, const ParagraphFormat* defParaFmt = NULL);

    bool MayHaveUrl() const { return pDocument->MayHaveUrl(); }

    // formatting
    void SetTextFormat(const TextFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);
    void SetParagraphFormat(const ParagraphFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT);
    void GetTextAndParagraphFormat
        (TextFormat* pdestTextFmt, 
        ParagraphFormat* pdestParaFmt, 
        UPInt startPos, UPInt endPos = SF_MAX_UPINT)
    {
        pDocument->GetTextAndParagraphFormat(pdestTextFmt, pdestParaFmt, startPos, endPos);
    }
    bool GetTextAndParagraphFormat(const TextFormat** ppdestTextFmt, const ParagraphFormat** pdestParaFmt, UPInt pos)
    {
        return pDocument->GetTextAndParagraphFormat(ppdestTextFmt, pdestParaFmt, pos);
    }
    void ClearAndSetTextFormat(const TextFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT)
    {
        pDocument->ClearTextFormat(startPos, endPos);
        pDocument->SetTextFormat(fmt, startPos, endPos);
    }

    // assign pdefaultTextFmt as a default text format
    void SetDefaultTextFormat(const TextFormat* pdefaultTextFmt)
    { pDocument->SetDefaultTextFormat(pdefaultTextFmt); }
    // makes a copy of text format and assign
    void SetDefaultTextFormat(const TextFormat& defaultTextFmt)
    { pDocument->SetDefaultTextFormat(defaultTextFmt); }
    // assign pdefaultTextFmt as a default text format
    void SetDefaultParagraphFormat(const ParagraphFormat* pdefaultParagraphFmt)
    { pDocument->SetDefaultParagraphFormat(pdefaultParagraphFmt); }
    // makes a copy of text format and assign
    void SetDefaultParagraphFormat(const ParagraphFormat& defaultParagraphFmt)
    { pDocument->SetDefaultParagraphFormat(defaultParagraphFmt); }

    const TextFormat*          GetDefaultTextFormat() const { return pDocument->GetDefaultTextFormat(); }
    const ParagraphFormat* GetDefaultParagraphFormat() const { return pDocument->GetDefaultParagraphFormat(); }

    void SetDocumentListener(DocumentListener* pdocumentListener) { pDocumentListener = pdocumentListener; }
    DocumentListener* GetDocumentListener() const { return pDocumentListener; }

    // text modification
    void AppendText(const wchar_t* pwStr, UPInt strLen = SF_MAX_UPINT);
    void AppendText(const char* putf8Str, UPInt utf8Len = SF_MAX_UPINT);

    void AppendHtml(const wchar_t* pwStr, UPInt strLen = SF_MAX_UPINT, bool condenseWhite = false, StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL);
    void AppendHtml(const char* putf8Str, UPInt utf8Len = SF_MAX_UPINT, bool condenseWhite = false, StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL);

    UPInt InsertText(const wchar_t* pstr, UPInt startPos, UPInt strLen = SF_MAX_UPINT);
    UPInt ReplaceText(const wchar_t* pstr, UPInt startPos, UPInt endPos, UPInt strLen = SF_MAX_UPINT);
    void  RemoveText(UPInt startPos, UPInt endPos)
    {
        UPInt len = (endPos >= startPos) ? endPos - startPos : 0;
        pDocument->Remove(startPos, len);
    }

    // insert styled text
    UPInt InsertStyledText(const StyledText& text, UPInt startPos, UPInt strLen = SF_MAX_UPINT)
    {
        return pDocument->InsertStyledText(text, startPos, strLen);
    }

    // edit
    // returns number of inserted or deleted chars
    UPInt EditCommand(CommandType cmdId, const void* command);

    // view
    ImageSubstitutor* CreateImageSubstitutor()
    {
        if (!pImageSubstitutor)
            pImageSubstitutor = new ImageSubstitutor;
        return pImageSubstitutor;
    }
    ImageSubstitutor* GetImageSubstitutor() const { return pImageSubstitutor; }
    void ClearImageSubstitutor() { delete pImageSubstitutor; pImageSubstitutor = NULL; }
    void Close();

    // notifications from document
    // Notifies the view the document was updated. "notifyMask" represents
    // the combination of DocView::ViewNotificationMasks
    virtual void OnDocumentChanged(unsigned notifyMask);
    virtual void OnDocumentParagraphRemoving(const Paragraph& para);

    void SetReformatReq()           { RTFlags |= RTFlags_ReformatReq; }
    void SetCompleteReformatReq()   { RTFlags |= RTFlags_CompleteReformatReq; }

    void Format();

    void CreateVisibleTextLayout(TextLayout::Builder& bld);

    void   SetBorderColor(UInt32 c)     { BorderColor = c; }
    UInt32 GetBorderColor() const       { return BorderColor; }
    void   SetBackgroundColor(UInt32 c) { BackgroundColor = c;}
    UInt32 GetBackgroundColor() const   { return BackgroundColor; }

    void SetAutoSizeX();
    void ClearAutoSizeX()   { Flags &= (~Flags_AutoSizeX); }
    bool IsAutoSizeX() const{ return (Flags & Flags_AutoSizeX); }

    void SetAutoSizeY();
    void ClearAutoSizeY()   { Flags &= (~Flags_AutoSizeY); }
    bool IsAutoSizeY() const{ return (Flags & Flags_AutoSizeY) != 0; }

    void SetMultiline()     { Flags |= Flags_Multiline; }
    void ClearMultiline()   { Flags &= (~Flags_Multiline); }
    bool IsMultiline() const{ return (Flags & Flags_Multiline) != 0; }

    void SetWordWrap();
    void ClearWordWrap();
    bool IsWordWrap() const { return (Flags & Flags_WordWrap) != 0 /*&& !IsAutoSizeX()*/; }

    void SetPasswordMode()     { Flags |= Flags_PasswordMode; }
    void ClearPasswordMode()   { Flags &= (~Flags_PasswordMode); }
    bool IsPasswordMode() const{ return (Flags & Flags_PasswordMode) != 0; }

    void SetUseDeviceFont()     { Flags |= Flags_UseDeviceFont; }
    void ClearUseDeviceFont()   { Flags &= (~Flags_UseDeviceFont); }
    bool DoesUseDeviceFont() const{ return (Flags & Flags_UseDeviceFont) != 0; }

    void SetAAForReadability()     { Flags |= Flags_AAReadability; }
    void ClearAAForReadability()   { Flags &= (~Flags_AAReadability); }
    bool IsAAForReadability() const{ return (Flags & Flags_AAReadability) != 0; }

    void SetAutoFit()     { Flags |= Flags_AutoFit; }
    void ClearAutoFit()   { Flags &= (~Flags_AutoFit); }
    bool IsAutoFit() const{ return (Flags & Flags_AutoFit) != 0; }

    bool IsBidirectionalTextEnabled() const
    { 
        return (FlagsEx & FlagsEx_BidiEnabled) && pDocumentListener && 
            pDocumentListener->DoesHandleBidirectionalText(); 
    }
    void EnableBidirectionalText() { FlagsEx |= FlagsEx_BidiEnabled; }
    void DisableBidirectionalText() { FlagsEx &= ~FlagsEx_BidiEnabled; }

    bool SetFilters(const TextFilter& f) { if ( Filter == f ) { return false; } Filter = f; return true; } 
    const TextFilter& GetFilters() const { return Filter; }
    void SetDefaultShadow()  { Filter.SetDefaultShadow(); }
    void EnableSoftShadow()  { Filter.ShadowFlags &= ~TextFieldParam::ShadowDisabled; }
    void DisableSoftShadow() { Filter.ShadowFlags |=  TextFieldParam::ShadowDisabled; }

    void  SetBlurX(float v) { Filter.BlurX = v; }
    float GetBlurX() const  { return Filter.BlurX; }

    void  SetBlurY(float v) { Filter.BlurY = v; }
    float GetBlurY() const  { return Filter.BlurY; }

    void  SetFauxBold(bool f) { if(f) FlagsEx |= FlagsEx_FauxBold; else FlagsEx &= ~FlagsEx_FauxBold; }
    bool  GetFauxBold() const { return (FlagsEx & FlagsEx_FauxBold) != 0; }

    void  SetFauxItalic(bool f) { if(f) FlagsEx |= FlagsEx_FauxItalic; else FlagsEx &= ~FlagsEx_FauxItalic; }
    bool  GetFauxItalic() const { return (FlagsEx & FlagsEx_FauxItalic) != 0; }

    void SetForceVector(bool f) { if(f) FlagsEx |= FlagsEx_ForceVector; else FlagsEx &= ~FlagsEx_ForceVector; }
    bool GetForceVector() const { return (FlagsEx & FlagsEx_ForceVector) != 0; }

    void  SetOutline(float v) { Outline = v; }
    float GetOutline() const  { return Outline; }

    void  SetBlurStrength(float v) { Filter.BlurStrength = v; }
    float GetBlurStrength() const  { return Filter.BlurStrength; }

    void  SetShadowBlurX(float v) { Filter.ShadowParams.BlurX = v; }
    float GetShadowBlurX() const  { return Filter.ShadowParams.BlurX; }

    void  SetShadowBlurY(float v) { Filter.ShadowParams.BlurY = v; }
    float GetShadowBlurY() const  { return Filter.ShadowParams.BlurY; }

    void  SetShadowStrength(float v) { Filter.ShadowParams.Strength = v; }
    float GetShadowStrength() const  { return Filter.ShadowParams.Strength; }

    void  SetKnockOut(bool f) { if(f) Filter.ShadowFlags |= GlyphParam::KnockOut; else Filter.ShadowFlags &= ~GlyphParam::KnockOut; }
    bool  IsKnockOut()  const { return (Filter.ShadowFlags & GlyphParam::KnockOut) != 0; }

    void  SetHideObject(bool f) { if(f) Filter.ShadowFlags |= GlyphParam::HideObject; else Filter.ShadowFlags &= ~GlyphParam::HideObject; }
    bool  IsHiddenObject()  const { return (Filter.ShadowFlags & GlyphParam::HideObject) != 0; }

    void  SetShadowQuality(unsigned v) { if(v > 1) Filter.ShadowFlags |= GlyphParam::FineBlur; else Filter.ShadowFlags &= ~GlyphParam::FineBlur; }
    unsigned GetShadowQuality() const { return (Filter.ShadowFlags & GlyphParam::FineBlur) ? 2 : 1; } 

    void  SetShadowAngle(float a) { Filter.ShadowAngle = a; Filter.UpdateShadowOffset(); }
    float GetShadowAngle() const  { return Filter.ShadowAngle; }

    void  SetShadowDistance(float d) { Filter.ShadowDistance = d; Filter.UpdateShadowOffset(); }
    float GetShadowDistance() const  { return Filter.ShadowDistance; }

    void  SetShadowColor(unsigned c) { Filter.ShadowParams.Colors[0] = (c & 0xFFFFFFu) | Filter.ShadowAlpha << 24; }
    unsigned GetShadowColor() const { return Filter.ShadowParams.Colors[0].ToColor32() & 0xFFFFFFu; }

    void  SetShadowAlpha(float a) { Filter.ShadowAlpha = UByte(Alg::Clamp(a * 255.0f, 0.0f, 255.0f)); Filter.ShadowParams.Colors[0].SetAlpha(Filter.ShadowAlpha); }
    float GetShadowAlpha() const  { return float(Filter.ShadowAlpha) / 255.0f; }

    void SetViewRect(const RectF& rect) { SetViewRect(rect, UseExternally); }
    const RectF& GetViewRect();

    // rectangle used for painting text (ViewRect - GUTTER)
    const RectF& GetTextRect() const { return mLineBuffer.Geom.VisibleRect; }

    bool     SetHScrollOffset(unsigned hscroll);    
    bool     SetVScrollOffset(unsigned vscroll);    

    unsigned GetHScrollOffset();
    unsigned GetVScrollOffset();

    float    GetTextWidth();
    float    GetTextHeight();
    unsigned  GetLinesCount();

    unsigned  GetMaxHScroll();
    unsigned  GetMaxVScroll();

    void  SetMaxLength(UPInt maxLen) { MaxLength = unsigned(maxLen); }
    UPInt GetMaxLength() const       { return MaxLength; }
    bool  HasMaxLength() const       { return MaxLength != 0; }

    // Returns true, if reformat actually occurred
    bool  ForceReformat();

    void SetAlignment(ViewAlignment alignment) 
    { 
        AlignProps = (UByte)((AlignProps & ~(Align_Mask << Align_Shift)) | ((alignment & Align_Mask) << Align_Shift));
        SetReformatReq();
    }
    ViewAlignment GetAlignment() const { return ViewAlignment((AlignProps >> Align_Shift) & Align_Mask); }

    void SetVAlignment(ViewVAlignment valignment)
    { 
        AlignProps = (UByte)((AlignProps & ~(VAlign_Mask << VAlign_Shift)) | ((valignment & VAlign_Mask) << VAlign_Shift));
        SetReformatReq();
    }
    ViewVAlignment GetVAlignment() const { return ViewVAlignment((AlignProps >> VAlign_Shift) & VAlign_Mask); }

    void SetTextAutoSize(ViewTextAutoSize valignment)
    { 
        AlignProps = (UByte)((AlignProps & ~(TAS_Mask << TAS_Shift)) | ((valignment & TAS_Mask) << TAS_Shift));
        SetReformatReq();
    }
    ViewTextAutoSize GetTextAutoSize() const { return ViewTextAutoSize((AlignProps >> TAS_Shift) & TAS_Mask); }

    bool IsUrlAtPoint(float x, float y, Range* purlPosRange = NULL);

    bool IsReadOnly() const;

    void  SetFontScaleFactor(float f); 
    float GetFontScaleFactor() const { return FixpToPixels((float)FontScaleFactor); }
    bool  HasFontScaleFactor() const { return (RTFlags & RTFlags_HasFontScaleFactor) != 0; }

    // Returns the zero-based index value of the character at the point specified by the x  and y parameters.
    UPInt GetCharIndexAtPoint(float x, float y);
    // Returns the zero-based index value of the cursor at the point specified by the x  and y parameters.
    UPInt GetCursorPosAtPoint(float x, float y);
    // Returns the zero-based index value of the cursor at the x-offset of line at lineIndex
    UPInt GetCursorPosInLine(unsigned lineIndex, float x);
    // Given a character index, returns the index of the first character in the same paragraph. 
    UPInt GetFirstCharInParagraph(UPInt indexOfChar);
    // Returns the zero-based index value of the line at the point specified by the x  and y parameters.
    unsigned GetLineIndexAtPoint(float x, float y);
    // Returns the zero-based index value of the line containing the character specified by the indexOfChar parameter.
    unsigned GetLineIndexOfChar(UPInt indexOfChar);
    // Returns the character index of the first character in the line that the lineIndex parameter specifies.
    UPInt GetLineOffset(unsigned lineIndex);
    // Returns the number of characters in a specific text line.
    UPInt GetLineLength(unsigned lineIndex, bool* phasNewLine = NULL);
    // Returns the text of the line specified by the lineIndex parameter.
    const wchar_t* GetLineText(unsigned lineIndex, UPInt* plen);
    // Returns the length of the paragraph containing the charIndex; returns -1, if indexOfChar is out of bound
    SPInt GetParagraphLength(UPInt charIndex);

    struct LineMetrics
    {   // everything is in twips
        unsigned Width, Height;
        unsigned Ascent, Descent;
        int      FirstCharXOff;
        int      Leading;
    };
    // Returns metrics information about a given text line.
    bool GetLineMetrics(unsigned lineIndex, LineMetrics* pmetrics);

    // Retrieve a rectangle (in twips) that is the bounding box of the character.
    // X and Y coordinates are in textfield's coordinate space and they does not reflect
    // current scrolling settings. Thus, if the character belongs to the line with large index
    // its Y-coordinate will be cumulative value of all prior line's heights, even if
    // it is scrolled and currently visible.
    bool GetCharBoundaries(RectF* pCharRect, UPInt indexOfChar);

    // Retrieve a rectangle (in twips) that is the bounding box of the character.
    // X and Y coordinates are in textfield's coordinate space and they does not reflect
    // current scrolling settings. Thus, if the character belongs to the line with large index
    // its Y-coordinate will be cumulative value of all prior line's heights, even if
    // it is scrolled and currently visible.
    // This method returns better rectangle than GetCharBoundaries does. It calculates
    // the width using the glyph's width rather than advance.
    bool GetExactCharBoundaries(RectF* pCharRect, UPInt indexOfChar);

    // returns the bottommost line index that is currently visible
    unsigned GetBottomVScroll();
    bool     SetBottomVScroll(unsigned newBottomMostLine);

    //EditorKitBase* CreateEditorKit();
    void           SetEditorKit(EditorKitBase* ek) { pEditorKit = ek; }
    EditorKitBase* GetEditorKit()             { return pEditorKit; };
    const EditorKitBase* GetEditorKit() const { return pEditorKit; };
    bool HasEditorKit() const                 { return (pEditorKit.GetPtr() != NULL); }

    // new line related
    unsigned char NewLineChar() const { return pDocument->NewLineChar(); }
    const char*   NewLineStr()  const { return pDocument->NewLineStr(); }
    void SetCompressCRLF()     { RTFlags |= RTFlags_CompressCRLF; }
    void ClearCompressCRLF()   { RTFlags &= (~RTFlags_CompressCRLF); }
    bool DoesCompressCRLF() const { return (RTFlags & RTFlags_CompressCRLF) != 0; }

    // highlight
    bool AddHighlight(HighlightDesc* pdesc);
    bool RemoveHighlight(int id);
    Highlighter* GetHighlighterManager() const { return (pHighlight) ? &pHighlight->HighlightManager : NULL; } //?
    void UpdateHighlight(const HighlightDesc& desc);

#ifdef SF_BUILD_DEBUG
    void Dump() { ForceReformat(); mLineBuffer.Dump(); }
#endif
public: 
    // primitive selection support, should be used by editor kit
    void SetSelection(UPInt startPos, UPInt endPos, bool highlightSelection = true);
    void ClearSelection() { SetSelection(0, 0); }
    //void GetSelection(UPInt* pstartPos, UPInt* pendPos);
    UPInt GetBeginSelection() const      { return Alg::PMin(BeginSelection, EndSelection); }
    UPInt GetEndSelection()   const      { return Alg::PMax(BeginSelection, EndSelection); }
    void SetBeginSelection(UPInt begSel) { SetSelection(begSel, EndSelection); }
    void SetEndSelection(UPInt endSel)   { SetSelection(BeginSelection, endSel); }
    void SetSelectionTextColor(UInt32 color);
    void SetSelectionBackgroundColor(UInt32 color);
    UInt32 GetSelectionTextColor() const;
    UInt32 GetSelectionBackgroundColor() const;
    const LineBuffer& GetLineBuffer() const { return mLineBuffer; }
    void  SetDefaultTextAndParaFormat(UPInt cursorPos);
    // Return current values of maxHScroll/maxVScroll values without
    // recalculations of them.
    unsigned GetMaxHScrollValue() const;
    unsigned GetMaxVScrollValue() const;

    unsigned GetHScrollOffsetValue() const { return mLineBuffer.GetHScrollOffset(); }    
    unsigned GetVScrollOffsetValue() const { return mLineBuffer.GetFirstVisibleLineIndex(); }    
    unsigned GetVScrollOffsetValueInFixp() const { return mLineBuffer.GetVScrollOffsetInFixp(); }    

    // returns a counter of Format calls, required by some validation logic (in EditorKit)
    UInt16   GetFormatCounter() const { return FormatCounter; }

    // Document's part
    class DocumentText : public StyledText
    {
        DocView* pDocument;
    public:
        DocumentText(DocView* pdoc, Allocator* pallocator) : 
          StyledText(pallocator), pDocument(pdoc) {}
          virtual void OnParagraphRemoving(const Paragraph& para);
          virtual void OnTextInserting(UPInt startPos, UPInt length, const wchar_t* ptxt);
          virtual void OnTextInserting(UPInt startPos, UPInt length, const char* ptxt);
          virtual void OnTextRemoving(UPInt startPos, UPInt length);
          //virtual void OnParagraphTextInserted(const Paragraph& para, UPInt startPos, UPInt endPos, const wchar_t* ptextInserted);
          //virtual void OnParagraphTextRemoved(const Paragraph& para, UPInt removedPos, UPInt removedLen);
    };

    Highlighter* CreateHighlighterManager();

protected:
    Ptr<DocumentText>       pDocument;
    Ptr<FontManagerBase>    pFontManager;
    Ptr<DocumentListener>   pDocumentListener;

    struct HighlightDescLoc : public NewOverrideBase<StatRender_Text_Mem>
    {
        Highlighter         HighlightManager;
        float               HScrollOffset;
        float               VScrollOffset;
        UInt16              FormatCounter; 

        HighlightDescLoc() : HScrollOffset(-1), VScrollOffset(-1), 
            FormatCounter(0) {}

        void Invalidate()       { HighlightManager.Invalidate(); }
        void Validate()         { HighlightManager.Validate(); }
        bool IsValid() const    { return HighlightManager.IsValid(); }
    } *pHighlight;

    // View's part
    ImageSubstitutor*       pImageSubstitutor;
    UPInt                   BeginSelection, EndSelection;
    Text::LineBuffer        mLineBuffer;
    RectF                   ViewRect; // total rectangle occupied by the view
    unsigned                TextWidth;  // in twips
    unsigned                TextHeight; // in twips
    unsigned                MaxLength;
    CachedPrimValue<unsigned> MaxVScroll; 

    Ptr<EditorKitBase>      pEditorKit;
    TextFilter              Filter; 
    Ptr<Log>                pLog; 
    UInt32                  BorderColor, BackgroundColor;
    UInt16                  FormatCounter; // being incremented each Format call
    UInt16                  FontScaleFactor; // in twips, 0 .. 1000.0
    float                   Outline;

    enum
    {
        Flags_AutoSizeX     = 0x1,
        Flags_AutoSizeY     = 0x2,
        Flags_Multiline     = 0x4,
        Flags_WordWrap      = 0x8,
        Flags_PasswordMode  = 0x10,
        Flags_UseDeviceFont = 0x20,
        Flags_AAReadability = 0x40,
        Flags_AutoFit       = 0x80
    };
    UInt8                   Flags;

    UInt8                   AlignProps; // combined H- and V- alignments and TAS_<>
    enum
    {
        FlagsEx_FauxBold    = 0x1,
        FlagsEx_FauxItalic  = 0x2,
        FlagsEx_BidiEnabled = 0x4,
        FlagsEx_ForceVector = 0x8,
    };
    UInt8                           FlagsEx;

    enum
    {
        RTFlags_ReformatReq         = 0x1,
        RTFlags_CompleteReformatReq = 0x2,
        RTFlags_HasFontScaleFactor  = 0x4,
        RTFlags_CompressCRLF        = 0x8,
        RTFlags_FontErrorDetected   = 0x10,
        RTFlags_BitmapFontUsed      = 0x20,
        RTFlags_HasBidirectionalText= 0x40
    };
    UInt8                           RTFlags; // run-time flags
};

}}} // Scaleform::Render::Text

#endif //INC_TEXT_GFXTEXTDOCVIEW_H
