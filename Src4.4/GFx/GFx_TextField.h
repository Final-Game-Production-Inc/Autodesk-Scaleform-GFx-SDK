/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_TextField.h
Content     :   Dynamic and input text field character implementation
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_TEXTFIELD_H
#define INC_SF_GFX_TEXTFIELD_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_AutoPtr.h"

#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_Input.h"
#include "GFx/GFx_TextFieldDef.h"
#include "GFx/Text/Text_EditorKit.h"

namespace Scaleform {
namespace GFx {
class FontHandle;

namespace Text 
{
    class StyleManager;

    using namespace Render::Text;
}

// implementation of TextField character
class TextField : public InteractiveObject
{
public:
    // Shadow structure - used to keep track of shadow color and offsets, allocated only
    // if those were assigned through the shadowStyle and shadowColor properties.
    struct ShadowParams : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        Color              ShadowColor;
        ASString           ShadowStyleStr;
        ArrayLH<Render::PointF>   ShadowOffsets;
        ArrayLH<Render::PointF>   TextOffsets;

        ShadowParams(ASStringManager* psc)
            : ShadowStyleStr(psc->CreateEmptyString())
        {
            ShadowColor = Color(0,0,0,255);
        }
    };

    class TextDocumentListener : public Text::DocView::DocumentListener
    {
        TextField* GetTextField() const;
    public:
        TextDocumentListener();
        void TranslatorChanged();
        void OnScroll(Text::DocView&);
        // events
        virtual void    View_OnHScroll(Text::DocView& view, unsigned newScroll);
        virtual void    View_OnVScroll(Text::DocView& view, unsigned newScroll);
        virtual void    View_OnMaxScrollChanged(Text::DocView& view);
        virtual bool    View_OnLineFormat(Text::DocView&, Text::DocView::LineFormatDesc& desc);
        virtual void    View_OnChanged(Text::DocView&);
        // A callback that is called on bidirectional text enabled textfields. This method
        // receives original text ('text'/'textLen' parameters) and pre-allocated buffers: 
        //   'newText'  - new re-ordered text should be put there, do not overrun the length (the length is the same as 'textLen')
        //   'indexMap' - a buffer for an array of unsigned integers, that should be filled with new indices of each char. For
        //                example, if original char at index 0 was relocated to index 10, then indexMap[0] = 10.
        //   'mirrorBits'- a buffer for an array of bools (length = 'textLen'), where index is an index of char in 'newText' and the value (true/false)
        //                indicates necessity of the glyph mirroring (for example, for integral sign).
        // Should return 'true' if method was successful and the core should use contents of the buffers; false, if no changes
        // to the textfield should be applied.
        virtual bool    View_PrepareBidiText(Text::DocView&, const wchar_t* paraText, UPInt textLen, 
            wchar_t* newParaText, unsigned* indexMap, bool* mirroredBits);
        // editor events
        virtual void    Editor_OnChanged(Text::EditorKitBase& editor);
        virtual void    Editor_OnCursorMoved(Text::EditorKitBase& editor);
        virtual void    Editor_OnCursorBlink(Text::EditorKitBase& editor, bool cursorState);
        virtual bool    Editor_OnInsertingText(Text::EditorKitBase&, UPInt pos, UPInt len, const wchar_t*);
        virtual bool    Editor_OnRemovingText(Text::EditorKitBase&, UPInt pos, UPInt len);

        virtual String  GetCharacterPath();
    };

    enum LinkEvent
    {
        Link_press,
        Link_release,
        Link_rollover,
        Link_rollout
    };

protected:
    Ptr<TextFieldDef>   pDef;
    Ptr<Text::DocView>  pDocument;
    ResourceBinding*    pBinding; //?AB: is there any other way to get FontResource*?
    mutable Ptr<Render::FilterSet> pFilters;

private:
    enum
    {
        Flags_AutoSize      = 0x1,
        Flags_Html          = 0x2,
        Flags_Password      = 0x4,
        Flags_NoTranslate   = 0x8,
        Flags_CondenseWhite = 0x10,
        Flags_HandCursor    = 0x20,
        Flags_NextFrame     = 0x40,
        Flags_MouseWheelEnabled     = 0x80,
        Flags_UseRichClipboard      = 0x100,
        Flags_AlwaysShowSelection   = 0x200,
        Flags_NoAutoSelection       = 0x400,
        Flags_IMEDisabled           = 0x800,
        Flags_OriginalIsHtml        = 0x1000,
        Flags_NeedUpdateGeomData    = 0x2000,
        Flags_ForceOneTimeAdvance   = 0x4000,
        Flags_ForceAdvance          = 0x8000,

        Flags_NeedUpdateLayout  = 0x10000
    };

    TextFieldDef::alignment Alignment;

    UInt32                  Flags;

protected:
    StringLH                OriginalTextValue;

#ifdef GFX_ENABLE_CSS
public: // for Wii compiler 4.3 145
    struct CSSHolderBase : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        virtual ~CSSHolderBase() {}

        struct UrlZone
        {
            Ptr<Text::StyledText>   SavedFmt;
            unsigned                HitCount;   // hit counter
            unsigned                OverCount;  // over counter

            UrlZone():HitCount(0),OverCount(0) {}

            bool operator==(const UrlZone& r) const 
            { 
                return SavedFmt == r.SavedFmt && HitCount == r.HitCount && OverCount == r.OverCount; 
            }
            bool operator!=(const UrlZone& r) const { return !operator==(r); }
        };
        RangeDataArray<UrlZone>    UrlZones;
        struct MouseStateType 
        {
            unsigned UrlZoneIndex;
            bool OverBit;
            bool HitBit;

            MouseStateType() : UrlZoneIndex(0), OverBit(false), HitBit(false) {}
        }                           MouseState[GFX_MAX_MICE_SUPPORTED];

        virtual bool HasASStyleSheet() const =0;
        virtual const Text::StyleManagerBase* GetTextStyleManager() const =0;
    };
private:
    AutoPtr<CSSHolderBase> pCSSData;
#else
    typedef void CSSHolderBase;
#endif //GFX_ENABLE_CSS

    ShadowParams *                      pShadow;

    StringHashLH<Ptr<Text::ImageDesc> >* pImageDescAssoc;

    unsigned                            FocusedControllerIdx;
    TextDocumentListener                TextDocListener;
public:
    void SetDirtyFlag();

    void SetAutoSize(bool v = true) { (v) ? Flags |= Flags_AutoSize : Flags &= (~Flags_AutoSize); }
    void ClearAutoSize()            { SetAutoSize(false); }
    bool IsAutoSize() const         { return (Flags & Flags_AutoSize) != 0; }

    void SetHtml(bool v = true) { (v) ? Flags |= Flags_Html : Flags &= (~Flags_Html); }
    void ClearHtml()            { SetHtml(false); }
    bool IsHtml() const         { return (Flags & Flags_Html) != 0; }

    void SetOriginalHtml(bool v = true) { (v) ? Flags |= Flags_OriginalIsHtml : Flags &= (~Flags_OriginalIsHtml); }
    void ClearOriginalHtml()            { SetOriginalHtml(false); }
    bool IsOriginalHtml() const         { return (Flags & Flags_OriginalIsHtml) != 0; }

    void SetPassword(bool v = true) { (v) ? Flags |= Flags_Password : Flags &= (~Flags_Password); }
    void ClearPassword()            { SetPassword(false); }
    bool IsPassword() const         { return (Flags & Flags_Password) != 0; }

    void SetCondenseWhite(bool v = true) { (v) ? Flags |= Flags_CondenseWhite : Flags &= (~Flags_CondenseWhite); }
    void ClearCondenseWhite()            { SetCondenseWhite(false); }
    bool IsCondenseWhite() const         { return (Flags & Flags_CondenseWhite) != 0; }

    void SetHandCursor(bool v = true) { (v) ? Flags |= Flags_HandCursor : Flags &= (~Flags_HandCursor); }
    void ClearHandCursor()            { SetHandCursor(false); }
    bool IsHandCursor() const         { return (Flags & Flags_HandCursor) != 0; }

    void SetMouseWheelEnabled(bool v = true) { (v) ? Flags |= Flags_MouseWheelEnabled : Flags &= (~Flags_MouseWheelEnabled); }
    void ClearMouseWheelEnabled()            { SetMouseWheelEnabled(false); }
    bool IsMouseWheelEnabled() const         { return (Flags & Flags_MouseWheelEnabled) != 0; }

    void SetSelectable(bool v = true);
    void ClearSelectable()            { SetSelectable(false); }
    bool IsSelectable() const;         

    void SetNoTranslate(bool v = true) { (v) ? Flags |= Flags_NoTranslate : Flags &= (~Flags_NoTranslate); }
    void ClearNoTranslate()            { SetNoTranslate(false); }
    bool IsNoTranslate() const         { return (Flags & Flags_NoTranslate) != 0; }

    bool IsReadOnly() const; 

    void SetUseRichClipboard(bool v = true) 
    { 
        (v) ? Flags |= Flags_UseRichClipboard : Flags &= (~Flags_UseRichClipboard); 
    }
    void ClearUseRichClipboard()            { SetUseRichClipboard(false); }
    bool DoesUseRichClipboard() const       { return (Flags & Flags_UseRichClipboard) != 0; }

    void SetAlwaysShowSelection(bool v = true) 
    { 
        (v) ? Flags |= Flags_AlwaysShowSelection : Flags &= (~Flags_AlwaysShowSelection); 
    }
    void ClearAlwaysShowSelection()            { SetAlwaysShowSelection(false); }
    bool DoesAlwaysShowSelection() const       { return (Flags & Flags_AlwaysShowSelection) != 0; }

    void SetNoAutoSelection(bool v = true) { (v) ? Flags |= Flags_NoAutoSelection : Flags &= (~Flags_NoAutoSelection); }
    void ClearNoAutoSelection()            { SetNoAutoSelection(false); }
    bool IsNoAutoSelection() const         { return (Flags & Flags_NoAutoSelection) != 0; }

    void SetIMEDisabledFlag(bool v = true) { (v) ? Flags |= Flags_IMEDisabled : Flags &= (~Flags_IMEDisabled); }
    void ClearIMEDisabledFlag()            { SetIMEDisabledFlag(false); }
    bool IsIMEDisabledFlag() const         { return (Flags & Flags_IMEDisabled) != 0; }

    void SetForceOneTimeAdvance(bool v = true) { (v) ? Flags |= Flags_ForceOneTimeAdvance : Flags &= (~Flags_ForceOneTimeAdvance); }
    void ClearForceOneTimeAdvance()            { SetForceOneTimeAdvance(false); }
    bool IsForceOneTimeAdvance() const         { return (Flags & Flags_ForceOneTimeAdvance) != 0; }

    void SetForceAdvance(bool v = true) { (v) ? Flags |= Flags_ForceAdvance : Flags &= (~Flags_ForceAdvance); }
    void ClearForceAdvance()            { SetForceAdvance(false); }
    bool IsForceAdvance() const         { return (Flags & Flags_ForceAdvance) != 0; }

    void SetNeedUpdateLayoutFlag(bool v = true) { (v) ? Flags |= Flags_NeedUpdateLayout : Flags &= (~Flags_NeedUpdateLayout); }
    void ClearNeedUpdateLayoutFlag()            { SetNeedUpdateLayoutFlag(false); }
    bool IsNeedUpdateLayoutFlag() const         { return (Flags & Flags_NeedUpdateLayout) != 0; }

    Ptr<Text::EditorKit> CreateEditorKit();

    SF_INLINE const AvmTextFieldBase*  GetAvmTextField() const
    {
        return GetAvmObjImpl()->ToAvmTextFieldBase();
    }
    SF_INLINE AvmTextFieldBase*  GetAvmTextField()
    {
        return GetAvmObjImpl()->ToAvmTextFieldBase();
    }
public:
    TextField(TextFieldDef* def, 
              MovieDefImpl *pbindingDefImpl, 
              ASMovieRootBase* pasRoot,
              InteractiveObject* parent, 
              ResourceId id);

    ~TextField();

    void           SetCSSData(CSSHolderBase*);
    CSSHolderBase* GetCSSData() const;
 
    // *** overloaded methods of DisplayObject
    // Override AdvanceFrame so that variable dependencies can be updated.
    virtual void            UpdateTransform3D();
    virtual void            AdvanceFrame(bool nextFrame, float framePos);
    virtual RectF           GetBounds(const Matrix &t) const;
    virtual RectF           GetRectBounds(const Matrix &t) const { return GetBounds(t); }
    virtual CharacterDef*   GetCharacterDef() const { return pDef; }
    virtual unsigned        GetCursorType() const;
    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::TextField; }
    virtual Double          GetX() const;
    virtual Double          GetY() const;
    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
    virtual void            OnEventLoad();
    virtual void            OnEventUnload();
    virtual bool            PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;
    // sets rotation, in degrees [-180..0..180]
    virtual void            SetRotation(Double rotation);
    virtual void            SetStateChangeFlags(UInt8 flags);
    // sets height, in pixels
    virtual void            SetHeight(Double height);
    // sets width, in pixels
    virtual void            SetWidth(Double width);
    // sets coordinate X, specified in pixels.
    virtual void            SetX(Double x);
    // sets coordinate Y, specified in pixels.
    virtual void            SetY(Double y);
    // sets X scale, in percents [0..100%...].
    virtual void            SetXScale(Double xscale);
    // sets Y scale, in percents [0..100%...].
    virtual void            SetYScale(Double yscale);

    virtual void            SetFilters(const FilterSet* filters);
    virtual const FilterSet*GetFilters() const;

    // *** overloaded methods of InteractiveObject
    // Returns 0 if nothing to do
    // 1 - if need to add to optimized play list
    // -1 - if need to remove from optimized play list
    virtual int             CheckAdvanceStatus(bool playingNow);
    // Return the topmost entity that the given point covers.  NULL if none.
    // I.E. check against ourself.
    virtual TopMostResult   GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);
    virtual bool            IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const; 
    virtual bool            IsFocusRectEnabled() const { return false; }
    virtual bool            IsTabable() const;
    virtual bool            OnMouseEvent(const EventId& event);
#ifdef GFX_ENABLE_KEYBOARD
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask);
    virtual bool            OnCharEvent(UInt32 wcharCode, unsigned controllerIdx);
#endif
    virtual bool            IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const;
    virtual bool            IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx);
    virtual void            OnFocus(FocusEventType event, InteractiveObject* oldOrNewFocusCh, unsigned controllerIdx, FocusMovedType fmt);
    virtual bool            OnLosingKeyboardFocus(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt);
    // Special event handler; mouse wheel support
    virtual bool            OnMouseWheelEvent(int mwDelta);
    virtual void            PropagateMouseEvent(const EventId& id);

    //********** native methods *******************
    void                    AddIdImageDescAssoc(const char* idStr, Text::ImageDesc* pdesc);
    void                    AppendText(const wchar_t* pwStr, UPInt strLen = SF_MAX_UPINT)
    {
        pDocument->AppendText(pwStr, strLen);
        SetNeedUpdateLayoutFlag();
    }
    void                    AppendText(const char* putf8Str, UPInt utf8Len = SF_MAX_UPINT)
    {
        pDocument->AppendText(putf8Str, utf8Len);
        SetNeedUpdateLayoutFlag();
    }

    void                    AppendHtml(
        const wchar_t* pwStr, UPInt strLen = SF_MAX_UPINT, 
        bool condenseWhite = false, Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL)
    {
        pDocument->AppendHtml(pwStr, strLen, condenseWhite, pimgInfoArr);
        SetNeedUpdateLayoutFlag();
    }
    void                    AppendHtml(
        const char* putf8Str, UPInt utf8Len = SF_MAX_UPINT, 
        bool condenseWhite = false, Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL)
    {
        pDocument->AppendHtml(putf8Str, utf8Len, condenseWhite, pimgInfoArr);
        SetNeedUpdateLayoutFlag();
    }
    // Changes format of link according to event. Returns 'true' if changes were done
    // and NotifyChanged should be called.
    bool                    ChangeUrlFormat(LinkEvent event, unsigned mouseIndex, const Range* purlRange = NULL);
    void                    ClearAAForReadability() { pDocument->ClearAAForReadability(); }
    void                    ClearAutoFit() { pDocument->ClearAutoFit(); }
    void                    ClearAutoSizeX() { pDocument->ClearAutoSizeX(); }
    void                    ClearAutoSizeY() { pDocument->ClearAutoSizeY(); }
    void                    ClearIdImageDescAssoc();
    void                    ClearImageSubstitutor() { pDocument->ClearImageSubstitutor(); }
    void                    ClearMultiline() { pDocument->ClearMultiline(); }
    void                    ClearPasswordMode() { pDocument->ClearPasswordMode(); }
    void                    ClearUseDeviceFont() { pDocument->ClearUseDeviceFont(); }
    void                    ClearWordWrap() { pDocument->ClearWordWrap(); }
    void                    ClearUrlZones();
    void                    CollectUrlZones();
    Text::DocView::ImageSubstitutor* CreateImageSubstitutor() { return pDocument->CreateImageSubstitutor(); }
    bool                    DoesUseDeviceFont() const { return pDocument->DoesUseDeviceFont(); }
    void                    EnableSoftShadow() { pDocument->EnableSoftShadow(); }
    void                    EnableBidirectionalText(bool en = true) { (en)?pDocument->EnableBidirectionalText():pDocument->DisableBidirectionalText(); }
    void                    ForceCompleteReformat() { pDocument->SetCompleteReformatReq(); }
    void                    ForceReformat() { pDocument->SetReformatReq(); }
    Text::DocView::ViewAlignment GetAlignment() const { return pDocument->GetAlignment(); }
    RectF                   GetCursorBounds(UPInt cursorPos, float* phscroll, float* pvscroll);
    SPInt                   GetCaretIndex() const;
    Color                   GetBackgroundColor() const { return Color(pDocument->GetBackgroundColor()); }
    UInt32                  GetBackgroundColor24() const { return pDocument->GetBackgroundColor() & 0xFFFFFFu; }
    UInt32                  GetBackgroundColor32() const { return pDocument->GetBackgroundColor(); }
    float                   GetBlurX() const { return pDocument->GetBlurX(); }
    float                   GetBlurY() const { return pDocument->GetBlurY(); }
    float                   GetBlurStrength() const { return pDocument->GetBlurStrength(); }
    float                   GetOutline() const { return pDocument->GetOutline(); }
    Color                   GetBorderColor() const { return Color(pDocument->GetBorderColor()); }
    UInt32                  GetBorderColor24() const { return pDocument->GetBorderColor() & 0xFFFFFFu; }
    UInt32                  GetBorderColor32() const { return pDocument->GetBorderColor(); }
    unsigned                GetBottomVScroll() const { return pDocument->GetBottomVScroll(); }
    UPInt                   GetBeginIndex() const;
    bool                    GetCharBoundaries(RectF* pCharRect, UPInt indexOfChar) 
    {
        return pDocument->GetCharBoundaries(pCharRect, indexOfChar);
    }
    // Returns the zero-based index value of the character at the point specified by the x  and y parameters.
    UPInt                   GetCharIndexAtPoint(float x, float y) { return pDocument->GetCharIndexAtPoint(x, y); }
    SPInt                   GetCursorPos() const;
    const Text::TextFormat* GetDefaultTextFormat() const { return pDocument->GetDefaultTextFormat(); }
    const Text::ParagraphFormat* GetDefaultParagraphFormat() const { return pDocument->GetDefaultParagraphFormat(); }
    Text::EditorKit*        GetEditorKit() const { return static_cast<Text::EditorKit*>(pDocument->GetEditorKit()); }
    UPInt                   GetEndIndex() const;
    bool                    GetExactCharBoundaries(RectF* pCharRect, UPInt indexOfChar) 
    {
        return pDocument->GetExactCharBoundaries(pCharRect, indexOfChar);
    }
    bool                    GetFauxBold() const { return pDocument->GetFauxBold(); }
    bool                    GetFauxItalic() const { return pDocument->GetFauxItalic(); }
    bool                    GetForceVector() const { return pDocument->GetForceVector(); }
    UPInt                   GetFirstCharInParagraph(UPInt indexOfChar) 
    { 
        return pDocument->GetFirstCharInParagraph(indexOfChar);
    }
    unsigned                GetFocusedControllerIdx() const { return FocusedControllerIdx; }
    float                   GetFontScaleFactor() const { return pDocument->GetFontScaleFactor(); }
    StringHashLH<Ptr<Text::ImageDesc> >* GetImageDescAssoc() const { return pImageDescAssoc; }
    
    // in pixels
    Double                  GetHScrollOffset() const 
    { 
        return TwipsToPixels(Double(pDocument->GetHScrollOffset())); 
    }
    Double                  GetMaxHScroll() const 
    { 
        return TwipsToPixels(Double(pDocument->GetMaxHScroll())); 
    }
    unsigned                GetLineIndexAtPoint(float x, float y) { return pDocument->GetLineIndexAtPoint(x, y); }
    unsigned                GetLineIndexOfChar(UPInt indexOfChar) { return pDocument->GetLineIndexOfChar(indexOfChar); }
    UPInt                   GetLineLength(unsigned lineIndex, bool* phasNewLine = NULL)
    {
        return pDocument->GetLineLength(lineIndex, phasNewLine);
    }
    bool                    GetLineMetrics(unsigned lineIndex, Text::DocView::LineMetrics* pmetrics)
    {
        return pDocument->GetLineMetrics(lineIndex, pmetrics);
    }
    UPInt                   GetLineOffset(unsigned lineIndex) { return pDocument->GetLineOffset(lineIndex); }
    const wchar_t*          GetLineText(unsigned lineIndex, UPInt* plen)
    {
        return pDocument->GetLineText(lineIndex, plen);
    }
    unsigned                GetLinesCount() const { return pDocument->GetLinesCount(); }
    unsigned                GetMaxLength() const { return (unsigned)pDocument->GetMaxLength(); }
    unsigned                GetMaxVScroll() const { return pDocument->GetMaxVScroll(); }
    const StringLH&         GetOriginalTextValue() const { return OriginalTextValue; }
    // Returns text or html. If reqHtml = true but the textfield is not HTML-enabled
    // it still will return plain text (default behavior in AS2). 
    ASString                GetText(bool reqHtml) const; 
    // Returns html. Unlike GetText(true), this method will return HTML regardless.
    ASString                GetHtml() const ;
    Text::DocView::ViewTextAutoSize GetTextAutoSize() const { return pDocument->GetTextAutoSize(); }
    UPInt                   GetTextLength() const { return pDocument->GetLength(); }
    SF_INLINE TextFieldDef* GetTextFieldDef() const { return pDef; }
    void                    GetInitialFormats(Text::TextFormat* ptextFmt, Text::ParagraphFormat* pparaFmt);
    // returns -1, if indexOfChar is out of bound
    SPInt                   GetParagraphLength(UPInt indexOfChar)
    {
        return pDocument->GetParagraphLength(indexOfChar);
    }
    // Special code for complex objects API (see Value::ObjectInterface::SetPositionInfo)
    void                    GetPosition(Value::DisplayInfo* pinfo);
    UInt32                  GetSelectedBackgroundColor() const;
    UInt32                  GetSelectedTextColor() const;
    float                   GetShadowAlpha() const  { return pDocument->GetShadowAlpha(); }
    float                   GetShadowAngle() const  { return pDocument->GetShadowAngle(); }
    float                   GetShadowBlurX() const { return pDocument->GetShadowBlurX(); }
    float                   GetShadowBlurY() const { return pDocument->GetShadowBlurY(); }
    Color                   GetShadowColor() const { return pDocument->GetShadowColor(); }
    UInt32                  GetShadowColor32() const { return GetShadowColor().ToColor32(); }
    float                   GetShadowDistance() const { return pDocument->GetShadowDistance(); }
    unsigned                GetShadowQuality() const { return pDocument->GetShadowQuality(); }
    float                   GetShadowStrength() const { return pDocument->GetShadowStrength(); }
    ASString                GetShadowStyle() const;
    const Text::StyleManager*  GetStyleSheet() const; 
    void                    GetTextAndParagraphFormat(
                                Text::TextFormat* pdestTextFmt, 
                                Text::ParagraphFormat* pdestParaFmt, 
                                UPInt startPos, UPInt endPos = SF_MAX_UPINT)
    {
        pDocument->GetTextAndParagraphFormat(pdestTextFmt, pdestParaFmt, startPos, endPos);
    }
    bool                    GetTextAndParagraphFormat(
                                const Text::TextFormat** ppdestTextFmt, 
                                const Text::ParagraphFormat** pdestParaFmt, 
                                UPInt pos)
    {
        return pDocument->GetTextAndParagraphFormat(ppdestTextFmt, pdestParaFmt, pos);
    }
    Color                   GetTextColor() const;
    UInt32                  GetTextColor32() const;
    // returns width of text in pixels
    Double                  GetTextWidth() const;
    // returns height of text in pixels
    Double                  GetTextHeight() const;
    Text::DocView::ViewVAlignment GetVAlignment() const { return pDocument->GetVAlignment(); }
    unsigned                GetVScrollOffset() const { return pDocument->GetVScrollOffset(); }
    bool                    HasBackground() const { return GetBackgroundColor().GetAlpha() != 0; }
    bool                    HasBorder() const { return GetBorderColor().GetAlpha() != 0; }
    bool                    HasEditorKit() const { return pDocument->HasEditorKit(); }
    bool                    HasMaxLength() const { return pDocument->HasMaxLength(); }
    bool                    HasStyleSheet() const;
    bool                    IsAAForReadability() const { return pDocument->IsAAForReadability(); }
    bool                    IsAutoFit() const { return pDocument->IsAutoFit(); }
    bool                    IsAutoSizeX() const { return pDocument->IsAutoSizeX(); }
    bool                    IsAutoSizeY() const { return pDocument->IsAutoSizeY(); }
    bool                    IsBidirectionalTextEnabled() const { return pDocument->IsBidirectionalTextEnabled(); }
    bool                    IsMultiline() const { return pDocument->IsMultiline(); }
    bool                    IsOverwriteMode() const;
    bool                    IsShadowHiddenObject()  const { return pDocument->IsHiddenObject(); }
    bool                    IsShadowKnockOut()  const { return pDocument->IsKnockOut(); }
    bool                    IsUrlUnderMouseCursor(unsigned mouseIndex, Render::PointF* pPnt = NULL, Range* purlRangePos = NULL);
    bool                    IsUrlTheSame(unsigned mouseIndex, const Range& urlRange);
    bool                    IsWordWrap() const { return pDocument->IsWordWrap(); }
    bool                    MayHaveUrl() const { return pDocument->MayHaveUrl(); }
    void                    NotifyChanged();
    bool                    SetRestrict(const ASString& restrStr);
    const String*           GetRestrict() const;
    void                    ClearRestrict();

    void                    ProcessImageTags(Text::StyledText::HTMLImageTagInfoArray& imageInfoArray);
    void                    RemoveIdImageDescAssoc(const char* idStr);
    void                    ReplaceText(const wchar_t* pstr, UPInt startPos, UPInt endPos, UPInt strLen = SF_MAX_UPINT);
    void                    ResetBlink(bool state = 1, bool blocked = 0);

    void                    SetAAForReadability() { pDocument->SetAAForReadability(); }
    void                    SetAlignment(Text::DocView::ViewAlignment alignment) 
    {
        pDocument->SetAlignment(alignment);
    }
    void                    SetAutoFit() { pDocument->SetAutoFit(); }
    void                    SetAutoSizeX() { pDocument->SetAutoSizeX(); }
    void                    SetAutoSizeY() { pDocument->SetAutoSizeY(); }
    void                    SetBackground(bool b = true);
    void                    SetBackgroundColor(UInt32 rgb);
    void                    SetBackgroundColor(const Color& c);
    void                    SetBlurStrength(float v) { pDocument->SetBlurStrength(v); }
    void                    SetBlurX(float v) { pDocument->SetBlurX(v); }
    void                    SetBlurY(float v) { pDocument->SetBlurY(v); }
    void                    SetBorder(bool b = true);
    void                    SetBorderColor(UInt32 rgb);
    void                    SetBorderColor(const Color& c);
    void                    SetDefaultParagraphFormat(const Text::ParagraphFormat& defaultParagraphFmt)
    { 
        pDocument->SetDefaultParagraphFormat(defaultParagraphFmt); 
        SetNeedUpdateLayoutFlag();
    }
    void                    SetDefaultTextFormat(const Text::TextFormat& defaultTextFmt)
    { 
        pDocument->SetDefaultTextFormat(defaultTextFmt); 
        SetNeedUpdateLayoutFlag();
    }
    void                    SetFauxBold(bool v) { pDocument->SetFauxBold(v); }
    void                    SetFauxItalic(bool v) { pDocument->SetFauxItalic(v); }
    void                    SetForceVector(bool v) { pDocument->SetForceVector(v); }

    // see also SetFilters(FilterSet* filters)
    void                    SetTextFilters(const TextFilter& f);    

    void                    SetFontScaleFactor(float f) { pDocument->SetFontScaleFactor(f); }
    // hs is in pixels
    void                    SetHScrollOffset(Double hs) { pDocument->SetHScrollOffset((unsigned)PixelsToTwips(hs)); }
    void                    SetInitialFormatsAsDefault();
    void                    SetMaxLength(UPInt maxL) { pDocument->SetMaxLength(maxL); }
    void                    SetMultiline() { pDocument->SetMultiline(); }
    // Exposed for the complex objects API (see Value::ObjectInterface::SetDisplayInfo)
    void                    SetNeedUpdateGeomData()
    {
        Flags |= Flags_NeedUpdateGeomData;
        SetDirtyFlag();
    }
    void                    SetOverwriteMode(bool overwMode = true);
    void                    SetParagraphFormat(const Text::ParagraphFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT)
    {
        pDocument->SetParagraphFormat(fmt, startPos, endPos);
    }
    void                    SetPasswordMode() { pDocument->SetPasswordMode(); }
    void                    SetSelection(SPInt beginIndex, SPInt endIndex);
    void                    SetShadowAlpha(float v) { pDocument->SetShadowAlpha(v); }
    void                    SetShadowAngle(float v) { pDocument->SetShadowAngle(v); }
    void                    SetShadowBlurX(float v) { pDocument->SetShadowBlurX(v); }
    void                    SetShadowBlurY(float v) { pDocument->SetShadowBlurY(v); }
    void                    SetShadowColor(UInt32 rgb);
    void                    SetShadowColor(const Color& c) { SetShadowColor(c.ToColor32()); }
    void                    SetShadowDistance(float v) { pDocument->SetShadowDistance(v); }
    void                    SetShadowHideObject(bool v) { pDocument->SetHideObject(v); }
    void                    SetShadowKnockOut(bool v) { pDocument->SetKnockOut(v); }
    void                    SetShadowQuality(unsigned v) { pDocument->SetShadowQuality(v); }
    void                    SetShadowStrength(float v) { pDocument->SetShadowStrength(v); }
    void                    SetOutline(float v) const { pDocument->SetOutline(v); }
    bool                    SetShadowStyle(const char *pstr);
    SF_INLINE void          SetText(const char* pnewText, bool reqHtml)
    {
        bool currHtml = IsHtml();
        if (reqHtml && !currHtml)        { SetHtml(true); }
        else if (!reqHtml && currHtml)   { SetHtml(false); }
        SetTextValue(pnewText, reqHtml);
    }
    void                    SetText(const wchar_t* pnewText, bool reqHtml);
    void                    SetTextAutoSize(Text::DocView::ViewTextAutoSize va) 
    { 
        pDocument->SetTextAutoSize(va); 
    }
    void                    SetTextColor(UInt32 rgb);
    void                    SetTextColor(const Color& c) { SetTextColor(c.ToColor32()); }
    void                    SetTextFormat(const Text::TextFormat& fmt, UPInt startPos = 0, UPInt endPos = SF_MAX_UPINT)
    {
        pDocument->SetTextFormat(fmt, startPos, endPos);
    }
    // Set our text to the given string. pnewText can have HTML or
    // regular formatting based on argument.
    bool                    SetTextValue(const char* pnewText, bool html, bool notifyVariable = true);
    void                    SetUseDeviceFont() { pDocument->SetUseDeviceFont(); }
    void                    SetVAlignment(Text::DocView::ViewVAlignment va) { pDocument->SetVAlignment(va); }
    void                    SetVScrollOffset(unsigned vs) { pDocument->SetVScrollOffset(vs); }
    void                    SetWideCursor(bool wideCursor = true);
    void                    SetWordWrap() { pDocument->SetWordWrap(); }
    // Special method for complex objects API (see Value::ObjectInterface::SetDisplayInfo)
    Render::PointF          TransformToTextRectSpace(const Value::DisplayInfo& info) const;
    GeomDataType&           UpdateAndGetGeomData(GeomDataType* pgeomData, bool force = false);
    // Updates styles of links after new style sheet is installed.
    void                    UpdateUrlStyles();


#ifndef SF_NO_IME_SUPPORT
    //******** IME related methods
    void                    ClearCompositionString();
    void                    CommitCompositionString(const wchar_t* pstr, UPInt len);
	const wchar_t*			GetCompositionString();
    void                    CreateCompositionString();
    FontResource*           GetFontResource();
	void					SetCandidateListFont(Sprite* psprite);
    void                    HighlightCompositionStringText(UPInt posInCompStr, UPInt len, Text::IMEStyle::Category hcategory);
    bool                    IsIMEEnabled() const { return !IsIMEDisabledFlag() && !IsReadOnly() && !IsPassword(); }
    void                    ReleaseCompositionString();
    void                    SetCompositionStringText(const wchar_t* pstr, UPInt len);
    void                    SetCompositionStringPosition(UPInt pos);
    UPInt                   GetCompositionStringPosition() const;
    UPInt                   GetCompositionStringLength() const;
    void                    SetCursorInCompositionString(UPInt pos);
#endif //#ifdef SF_NO_IME_SUPPORT

};


class AvmTextFieldBase : public AvmInteractiveObjBase
{
public:
    virtual const Text::StyleManager* GetStyleSheet() const =0;
    virtual bool HasStyleSheet() const =0;
    virtual void NotifyChanged() =0;
    virtual void OnLinkEventEx(TextField::LinkEvent event, unsigned pos, 
                                unsigned controllerIndex) { SF_UNUSED3(event, pos, controllerIndex); }
    virtual bool OnMouseEvent(const EventId& event) =0;
    // returns false, if default action should be prevented
    virtual bool OnCharEvent(wchar_t wcharCode, unsigned controllerIdx) =0;
    virtual void OnScroll() =0;

    virtual bool OnEditorInsertingText(UPInt pos, UPInt len, const wchar_t* wstr, unsigned controllerIdx = 0) =0;
    virtual bool OnEditorRemovingText(UPInt pos, UPInt len, unsigned controllerIdx = 0) =0;

    virtual bool UpdateTextFromVariable() =0;
    virtual void UpdateVariable() =0;
};


}} // namespace Scaleform::GFx

#endif //INC_SF_GFX_TEXTFIELD_H

