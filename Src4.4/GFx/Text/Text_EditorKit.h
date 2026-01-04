
/**************************************************************************

PublicHeader:   GFx
Filename    :   Text_EditorKit.h
Content     :   Editor implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_TEXT_GFXEDITORKIT_H
#define INC_TEXT_GFXEDITORKIT_H

#include "Render/Text/Text_DocView.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_Loader.h"
#include "Kernel/SF_AutoPtr.h"

namespace Scaleform { 
namespace GFx {
namespace Text {

using Render::TextLayout;
using namespace Render::Text;
// This struct contains styles for IME composition string. Used
// by the CompositionString.
struct IMEStyle : public NewOverrideBase<StatMV_Text_Mem>
{
	enum Category
	{
		SC_CompositionSegment   = 0,
		SC_ClauseSegment        = 1,
		SC_ConvertedSegment     = 2,
		SC_PhraseLengthAdj      = 3,
		SC_LowConfSegment       = 4,

		SC_MaxNum
	};
	HighlightInfo   HighlightStyles[SC_MaxNum];
	UInt8           PresenceMask;

	IMEStyle() : PresenceMask(0) {}

	const HighlightInfo& GetElement(UPInt n) const
	{
		return HighlightStyles[n];
	}
	bool HasElement(UPInt n) const 
	{ 
		SF_ASSERT(n < sizeof(PresenceMask)*8); 
		return (PresenceMask & (1 << n)) != 0; 
	}

	void SetElement(UPInt n, const HighlightInfo& hinfo) 
	{ 
		SF_ASSERT(n < SC_MaxNum); 
		PresenceMask |= (1 << n);
		HighlightStyles[n] = hinfo;
	}
	void Unite(const IMEStyle& st)
	{
		for (unsigned i = 0; i < SC_MaxNum; ++i)
		{
			if (st.HasElement(i))
				SetElement(i, st.GetElement(i));
		}
	}
};

class EditorKit;
#ifndef SF_NO_IME_SUPPORT
// A composition string implementation to be used within EditorKit.
//class GFx::Text::EditorKit;

class CompositionString : public CompositionStringBase
{
public:
    Ptr<TextFormat> pDefaultFormat;
	EditorKit*      pEditorKit;
    Ptr<Allocator>  pAllocator;
    Paragraph       String;
    UPInt           CursorPos;
    bool            HasHighlightingFlag;
    IMEStyle        Styles;
    unsigned        HighlightIds[IMEStyle::SC_MaxNum*2];
    UInt8           HighlightIdsUsed; // number of highlight ids used

    Paragraph*  GetSourceParagraph();
    void        Reformat();
	
//	CompositionString(Render::Text::EditorKit* peditorKit);
	CompositionString(GFx::Text::EditorKit* const peditorKit);
    ~CompositionString();

    Allocator*   GetAllocator() const { return pAllocator; }

    void SetText(const wchar_t*, UPInt nchars = SF_MAX_UPINT);
    const wchar_t* GetText() const { return String.GetText(); }

    void SetPosition(UPInt pos);
    UPInt GetPosition() const { return String.GetStartIndex(); }

    UPInt GetLength() const { return String.GetLength(); }

    Paragraph& GetParagraph() { return String; }
    TextFormat* GetTextFormat(UPInt pos) { return String.GetTextFormatPtr(pos); }
    TextFormat* GetDefaultTextFormat() const { return pDefaultFormat; }

    void  SetCursorPosition(UPInt pos);
    UPInt GetCursorPosition() const { return CursorPos; }

    void  HighlightText(UPInt pos, UPInt len, IMEStyle::Category styleCategory);

    void  UseStyles(const IMEStyle&);

    void  ClearHighlighting();
    bool  HasHighlighting() const { return HasHighlightingFlag; }

    static IMEStyle GetDefaultStyles();
};
#endif //#ifdef SF_NO_IME_SUPPORT

#ifdef GFX_ENABLE_TEXT_INPUT
// This class provides functionality for editing and selecting rich text.
// It is used in conjunction with DocView.
class EditorKit : public Render::Text::EditorKitBase
{
protected:
    typedef Render::Matrix2F    Matrix;
    typedef Render::Cxform      Cxform;

    enum 
    {
        Flags_ReadOnly              = 1,
        Flags_Selectable            = 2,
        Flags_UseRichClipboard      = 4,
        Flags_CursorBlink           = 8,
        Flags_CursorTimerBlocked    = 0x10,
        Flags_MouseCaptured         = 0x20,
        Flags_ShiftPressed          = 0x40,
        Flags_OverwriteMode         = 0x80,
        Flags_WideCursor            = 0x100,
        Flags_LastDoubleClick       = 0x200,
        Flags_Focused               = 0x400
    };
    Ptr<DocView>        pDocView;
    Ptr<Clipboard>  pClipboard;
    Ptr<TextKeyMap>     pKeyMap;
#ifndef SF_NO_IME_SUPPORT
    Ptr<CompositionString> pComposStr;
#endif //#ifdef SF_NO_IME_SUPPORT
    struct RestrictParams : public NewOverrideBase<StatMV_Text_Mem>
    {
        typedef RangeDataArray<void*, ArrayLH<RangeData<void*> > > RestrictRangesArrayType;

        RestrictRangesArrayType RestrictRanges;
        StringLH                RestrictString;                  
    };
    AutoPtr<RestrictParams> pRestrict;

    UPInt               CursorPos;
    Color               CursorColor;
    int                 SelectPos;
    CachedValue<RectF>  CursorRect;
    Double              CursorTimer;
    Double              LastAdvanceTime;
    float               LastHorizCursorPos;
    Render::PointF      LastMousePos;
    UInt32              LastClickTime;
    UInt32              ActiveSelectionBkColor;
    UInt32              ActiveSelectionTextColor;
    UInt32              InactiveSelectionBkColor;
    UInt32              InactiveSelectionTextColor;
    UInt16              Flags;

    void SetCursorBlink()     { Flags |= Flags_CursorBlink; }
    void ClearCursorBlink()   { Flags &= (~Flags_CursorBlink); }
    bool IsCursorBlink() const{ return (Flags & Flags_CursorBlink) != 0; }

    void SetCursorTimerBlocked()     { Flags |= Flags_CursorTimerBlocked; }
    void ClearCursorTimerBlocked()   { Flags &= (~Flags_CursorTimerBlocked); }
    bool IsCursorTimerBlocked() const{ return (Flags & Flags_CursorTimerBlocked) != 0; }

    void SetMouseCaptured()     { Flags |= Flags_MouseCaptured; }
    void ClearMouseCaptured()   { Flags &= (~Flags_MouseCaptured); }

    void SetShiftPressed()     { Flags |= Flags_ShiftPressed; }
    void ClearShiftPressed()   { Flags &= (~Flags_ShiftPressed); }
    bool IsShiftPressed() const{ return (Flags & Flags_ShiftPressed) != 0; }

    void SetLastDoubleClickFlag()     { Flags |= Flags_LastDoubleClick; }
    void ClearLastDoubleClickFlag()   { Flags &= (~Flags_LastDoubleClick); }
    bool IsLastDoubleClickFlagSet() const{ return (Flags & Flags_LastDoubleClick) != 0; }

    // "notifyMask" represents
    // the combination of DocView::ViewNotificationMasks
    virtual void OnDocumentChanged(unsigned notifyMask);

    const LineBuffer::GlyphEntry* GetGlyphEntryAtIndex(UPInt charIndex, UPInt* ptextPos);

    void  InvalidateSelectionColors();
    bool  ParseRestrict(const char* restrStr, UPInt restrStrLen);
public:
    EditorKit(DocView* pdocview);
    ~EditorKit();

    // impl of EditorKitBase
    virtual void AddDrawCursorInfo(TextLayout::Builder& bld);
    virtual bool IsReadOnly() const{ return (Flags & Flags_ReadOnly) != 0; }
    // for HasCompositionString() and GetCompositionString() look below
    // convert glyph pos (taken from LineBuffer to-from text pos (as in StyledText)
    virtual UPInt TextPos2GlyphOffset(UPInt textPos);
    virtual UPInt TextPos2GlyphPos(UPInt textPos);
    virtual UPInt GlyphPos2TextPos(UPInt glyphPos);


    void SetReadOnly()     { Flags |= Flags_ReadOnly; }
    void ClearReadOnly()   { Flags &= (~Flags_ReadOnly); }

    void SetSelectable()     { Flags |= Flags_Selectable; }
    void ClearSelectable()   { Flags &= (~Flags_Selectable); }
    bool IsSelectable() const{ return (Flags & Flags_Selectable) != 0; }

    void SetUseRichClipboard()     { Flags |= Flags_UseRichClipboard; }
    void ClearUseRichClipboard()   { Flags &= (~Flags_UseRichClipboard); }
    bool DoesUseRichClipboard() const{ return (Flags & Flags_UseRichClipboard) != 0; }

    void SetOverwriteMode()     { Flags |= Flags_OverwriteMode; }
    void ClearOverwriteMode()   { Flags &= (~Flags_OverwriteMode); }
    bool IsOverwriteMode() const{ return (Flags & Flags_OverwriteMode) != 0; }

    void SetWideCursor()     { Flags |= Flags_WideCursor; }
    void ClearWideCursor();
    bool IsWideCursor() const{ return (Flags & Flags_WideCursor) != 0; }

    void SetFocused()     { Flags |= Flags_Focused; }
    void ClearFocused()   { Flags &= (~Flags_Focused); }
    bool IsFocused() const{ return (Flags & Flags_Focused) != 0; }

    bool IsMouseCaptured() const{ return (Flags & Flags_MouseCaptured) != 0; }

    void Advance(Double timer);

    void UpdateWideCursor();

    void ResetBlink(bool state = 1, bool blocked = 0);

    bool HasCursor() const { return !IsReadOnly(); }

    void SetCursorPos(UPInt pos, bool selectionAllowed);
    void SetCursorPos(UPInt pos) { SetCursorPos(pos, IsSelectable()); }
    UPInt GetCursorPos() const;
	CachedValue<RectF> GetCursorRect()const { return CursorRect; };
    // scrolls the text to the position. If wideCursor is true, it will scroll
    // to make whole wide cursor visible.
    bool ScrollToPosition(UPInt pos, bool avoidComposStr, bool wideCursor = false);
    bool ScrollToCursor()
    {
        return ScrollToPosition(CursorPos, true, IsWideCursor());
    }

    // event handlers
    void OnMouseDown(float x, float y, int buttons);
    void OnMouseUp(float x, float y, int buttons);
    void OnMouseMove(float x, float y);
    bool OnKeyDown(int code, const KeyModifiers& specKeysState);
    bool OnKeyUp(int code, const KeyModifiers& specKeysState);
    bool OnChar(UInt32 wcharCode);
    void OnSetFocus();
    void OnKillFocus();

    DocView* GetDocument() const { return pDocView; }
    void SetClipboard(Clipboard* pclipbrd) { pClipboard = pclipbrd; }
    void SetKeyMap(TextKeyMap* pkeymap)        { pKeyMap = pkeymap; }

    bool CalcCursorRectInLineBuffer
        (UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex = NULL, unsigned* pglyphIndex = NULL, bool avoidComposStr = true,
         LineBuffer::Line::Alignment* plineAlignment = NULL);
    bool CalcAbsCursorRectInLineBuffer(UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex = NULL, unsigned* pglyphIndex = NULL)
    {
        return CalcCursorRectInLineBuffer(charIndex, pcursorRect, plineIndex, pglyphIndex, false);
    }
    bool CalcCursorRectOnScreen
        (UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex = NULL, unsigned* pglyphIndex = NULL, bool avoidComposStr = true,
         LineBuffer::Line::Alignment* plineAlignment = NULL);
    bool CalcAbsCursorRectOnScreen(UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex = NULL, unsigned* pglyphIndex = NULL)
    {
        return CalcCursorRectOnScreen(charIndex, pcursorRect, plineIndex, pglyphIndex, false);
    }

    // Selection, public API
    void SetSelection(UPInt startPos, UPInt endPos);
    void ClearSelection()                                  { pDocView->ClearSelection(); }
    UPInt GetBeginSelection() const                        { return pDocView->GetBeginSelection(); }
    UPInt GetEndSelection()   const                        { return pDocView->GetEndSelection(); }
    void SetBeginSelection(UPInt begSel)                   { SetSelection(begSel, GetEndSelection()); }
    void SetEndSelection(UPInt endSel)                     { SetSelection(GetBeginSelection(), endSel); }
    void SetActiveSelectionTextColor(UInt32 color)         { ActiveSelectionTextColor = color; InvalidateSelectionColors(); }
    void SetActiveSelectionBackgroundColor(UInt32 color)   { ActiveSelectionBkColor   = color; InvalidateSelectionColors(); }
    void SetInactiveSelectionTextColor(UInt32 color)       { InactiveSelectionTextColor = color; InvalidateSelectionColors(); }
    void SetInactiveSelectionBackgroundColor(UInt32 color) { InactiveSelectionBkColor   = color; InvalidateSelectionColors(); }
    UInt32 GetActiveSelectionTextColor() const             { return ActiveSelectionTextColor; }
    UInt32 GetActiveSelectionBackgroundColor() const       { return ActiveSelectionBkColor; }
    UInt32 GetInactiveSelectionTextColor() const           { return InactiveSelectionTextColor; }
    UInt32 GetInactiveSelectionBackgroundColor() const     { return InactiveSelectionBkColor; }

    // clipboard operations
    void CopyToClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard);
    void CutToClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard);
    // if successful, returns new projected cursor position; SF_MAX_UPINT otherwise.
    UPInt PasteFromClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard);

    bool          SetRestrict(const String& restrStr);
    const String* GetRestrict() const;
    void          ClearRestrict();
    // returns true, if wcharCode is ALLOWED (not restricted); false otherwise.
    bool          CheckRestrict(wchar_t wcharCode) const;
    bool          HasRestrict() const { return !!pRestrict; }

#ifndef SF_NO_IME_SUPPORT
    // Composition string
    CompositionString* CreateCompositionString();
    void ReleaseCompositionString();
    virtual bool HasCompositionString() const { return (pComposStr && pComposStr->GetLength() > 0); }
    virtual CompositionString* GetCompositionString() { return pComposStr; }
#else
    virtual bool HasCompositionString() const { return false; }
    virtual CompositionStringBase* GetCompositionString() { return NULL; }
#endif //#ifdef SF_NO_IME_SUPPORT
};

#else

// A stub class used if SF_NO_TEXT_INPUT_SUPPORT macro is defined.
class EditorKit : public Render::Text::EditorKitBase          // public RefCountBaseNTS<EditorKit, StatMV_Text_Mem>
{
public:
    typedef Render::Matrix2F    Matrix;
    typedef Render::Cxform      Cxform;

    UPInt TextPos2GlyphOffset(UPInt ) { return 0; }
    UPInt TextPos2GlyphPos(UPInt )    { return 0; }
    UPInt GlyphPos2TextPos(UPInt )    { return 0; }

    void SetReadOnly()     { }
    void ClearReadOnly()   { }
    bool IsReadOnly() const{ return false; }

    void SetSelectable()     {  }
    void ClearSelectable()   {  }
    bool IsSelectable() const{ return false; }

    void SetUseRichClipboard()     {  }
    void ClearUseRichClipboard()   {  }
    bool DoesUseRichClipboard() const{ return false; }

    void SetOverwriteMode()     {  }
    void ClearOverwriteMode()   {  }
    bool IsOverwriteMode() const{ return false; }

    void SetWideCursor()     {  }
    void ClearWideCursor()   {  }
    bool IsWideCursor() const{ return false; }

    bool IsMouseCaptured() const{ return false; }

    void Advance(Double) {}
//     void PreDisplay(DisplayContext &, const Matrix&, const Cxform&) {}
//     void Display(DisplayContext &, const Matrix&, const Cxform&) {}
//     void PostDisplay(DisplayContext &, const Matrix&, const Cxform&) { }
    void ResetBlink(bool = 1, bool = 0) {}

    bool HasCursor() const { return !IsReadOnly(); }

    void SetCursorPos(UPInt, bool) { }
    void SetCursorPos(UPInt)       { }
    UPInt GetCursorPos() const     { return 0; }
    // scrolls the text to the position. If wideCursor is true, it will scroll
    // to make whole wide cursor visible.
    bool ScrollToPosition(UPInt, bool , bool = false) { return false; }
    bool ScrollToCursor() { return false; }

    // event handlers
    void OnMouseDown(float , float , int ) {}
    void OnMouseUp(float , float , int ) {}
    void OnMouseMove(float , float ) {}
    bool OnKeyDown(int, const KeyModifiers&) { return false; }
    bool OnKeyUp(int, const KeyModifiers&)   { return false; }
    bool OnChar(UInt32) { return false; }
    void OnSetFocus() {}
    void OnKillFocus() {}

    DocView* GetDocument() const { return 0; }
    void SetClipboard(Clipboard*) { }
    void SetKeyMap(TextKeyMap*)       { }

    bool CalcAbsCursorRectOnScreen(UPInt , RectF* , unsigned* = NULL, unsigned* = NULL)
    {
        return false;
    }

    // Selection, public API
    void SetSelection(UPInt , UPInt )                      {}
    void ClearSelection()                                  {}
    UPInt GetBeginSelection() const                        { return false; }
    UPInt GetEndSelection()   const                        { return false; }
    void SetBeginSelection(UPInt )                         { }
    void SetEndSelection(UPInt )                           { }
    void SetActiveSelectionTextColor(UInt32 )              { }
    void SetActiveSelectionBackgroundColor(UInt32 )        { }
    void SetInactiveSelectionTextColor(UInt32 )            { }
    void SetInactiveSelectionBackgroundColor(UInt32 )      { }
    UInt32 GetActiveSelectionTextColor() const             { return 0; }
    UInt32 GetActiveSelectionBackgroundColor() const       { return 0; }
    UInt32 GetInactiveSelectionTextColor() const           { return 0; }
    UInt32 GetInactiveSelectionBackgroundColor() const     { return 0; }

    // clipboard operations
    void CopyToClipboard(UPInt , UPInt , bool ) {}
    void CutToClipboard(UPInt , UPInt , bool )  {}
    // if successful, returns new projected cursor position; SF_MAX_UPINT otherwise.
    UPInt PasteFromClipboard(UPInt , UPInt , bool ) { return 0; }

    bool          SetRestrict(const String&) { return false; }
    const String* GetRestrict() const { return NULL; }
    void          ClearRestrict() {}

#ifndef SF_NO_IME_SUPPORT
    // Composition string
    CompositionString* CreateCompositionString() { return NULL; }
    void ReleaseCompositionString() { }
    virtual bool HasCompositionString() const { return false; }
    virtual CompositionString* GetCompositionString() { return NULL; }
#else
    virtual bool HasCompositionString() const { return false; }
    virtual CompositionStringBase* GetCompositionString() { return NULL; }
#endif //#ifdef SF_NO_IME_SUPPORT
};
#endif //SF_NO_TEXT_INPUT_SUPPORT

}}} // Scaleform::GFx::Text

#endif //INC_TEXT_GFXEDITORKIT_H
