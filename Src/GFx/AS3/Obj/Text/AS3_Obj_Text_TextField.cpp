//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextField.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextField.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Text_TextFormat.h"
#include "AS3_Obj_Text_StyleSheet.h"
#include "AS3_Obj_Text_TextLineMetrics.h"
#include "GFx/GFx_TextField.h"
#include "GFx/AS3/AS3_AvmTextField.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Obj/Geom/AS3_Obj_Geom_Rectangle.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class NativeMenu;
}
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getTextFormat, 0, SInt32>(AS3::StringManager&)
    {
        return -1;
    }

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getTextFormat, 1, SInt32>(AS3::StringManager&)
    {
        return -1;
    }

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_setTextFormat, 1, SInt32>(AS3::StringManager&)
    {
        return -1;
    }

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_setTextFormat, 2, SInt32>(AS3::StringManager&)
    {
        return -1;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_alwaysShowSelectionGet, bool> TFunc_Instances_TextField_alwaysShowSelectionGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_alwaysShowSelectionSet, const Value, bool> TFunc_Instances_TextField_alwaysShowSelectionSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_antiAliasTypeGet, ASString> TFunc_Instances_TextField_antiAliasTypeGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_antiAliasTypeSet, const Value, const ASString&> TFunc_Instances_TextField_antiAliasTypeSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_autoSizeGet, ASString> TFunc_Instances_TextField_autoSizeGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_autoSizeSet, const Value, const ASString&> TFunc_Instances_TextField_autoSizeSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_backgroundGet, bool> TFunc_Instances_TextField_backgroundGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_backgroundSet, const Value, bool> TFunc_Instances_TextField_backgroundSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_backgroundColorGet, UInt32> TFunc_Instances_TextField_backgroundColorGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_backgroundColorSet, const Value, UInt32> TFunc_Instances_TextField_backgroundColorSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_borderGet, bool> TFunc_Instances_TextField_borderGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_borderSet, const Value, bool> TFunc_Instances_TextField_borderSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_borderColorGet, UInt32> TFunc_Instances_TextField_borderColorGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_borderColorSet, const Value, UInt32> TFunc_Instances_TextField_borderColorSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_bottomScrollVGet, SInt32> TFunc_Instances_TextField_bottomScrollVGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_caretIndexGet, SInt32> TFunc_Instances_TextField_caretIndexGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_condenseWhiteGet, bool> TFunc_Instances_TextField_condenseWhiteGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_condenseWhiteSet, const Value, bool> TFunc_Instances_TextField_condenseWhiteSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_contextMenuGet, SPtr<Instances::fl_display::NativeMenu> > TFunc_Instances_TextField_contextMenuGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_defaultTextFormatGet, SPtr<Instances::fl_text::TextFormat> > TFunc_Instances_TextField_defaultTextFormatGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_defaultTextFormatSet, const Value, Instances::fl_text::TextFormat*> TFunc_Instances_TextField_defaultTextFormatSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_displayAsPasswordGet, bool> TFunc_Instances_TextField_displayAsPasswordGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_displayAsPasswordSet, const Value, bool> TFunc_Instances_TextField_displayAsPasswordSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_embedFontsGet, bool> TFunc_Instances_TextField_embedFontsGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_embedFontsSet, const Value, bool> TFunc_Instances_TextField_embedFontsSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_gridFitTypeGet, ASString> TFunc_Instances_TextField_gridFitTypeGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_gridFitTypeSet, const Value, const ASString&> TFunc_Instances_TextField_gridFitTypeSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_htmlTextGet, ASString> TFunc_Instances_TextField_htmlTextGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_htmlTextSet, const Value, const ASString&> TFunc_Instances_TextField_htmlTextSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_lengthGet, SInt32> TFunc_Instances_TextField_lengthGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_maxCharsGet, SInt32> TFunc_Instances_TextField_maxCharsGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_maxCharsSet, const Value, SInt32> TFunc_Instances_TextField_maxCharsSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_maxScrollHGet, SInt32> TFunc_Instances_TextField_maxScrollHGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_maxScrollVGet, SInt32> TFunc_Instances_TextField_maxScrollVGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_mouseWheelEnabledGet, bool> TFunc_Instances_TextField_mouseWheelEnabledGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_mouseWheelEnabledSet, const Value, bool> TFunc_Instances_TextField_mouseWheelEnabledSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_multilineGet, bool> TFunc_Instances_TextField_multilineGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_multilineSet, const Value, bool> TFunc_Instances_TextField_multilineSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_numLinesGet, SInt32> TFunc_Instances_TextField_numLinesGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_restrictGet, ASString> TFunc_Instances_TextField_restrictGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_restrictSet, const Value, const ASString&> TFunc_Instances_TextField_restrictSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_scrollHGet, SInt32> TFunc_Instances_TextField_scrollHGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_scrollHSet, const Value, SInt32> TFunc_Instances_TextField_scrollHSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_scrollVGet, SInt32> TFunc_Instances_TextField_scrollVGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_scrollVSet, const Value, SInt32> TFunc_Instances_TextField_scrollVSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_selectableGet, bool> TFunc_Instances_TextField_selectableGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_selectableSet, const Value, bool> TFunc_Instances_TextField_selectableSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_selectionBeginIndexGet, SInt32> TFunc_Instances_TextField_selectionBeginIndexGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_selectionEndIndexGet, SInt32> TFunc_Instances_TextField_selectionEndIndexGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_sharpnessGet, Value::Number> TFunc_Instances_TextField_sharpnessGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_sharpnessSet, const Value, Value::Number> TFunc_Instances_TextField_sharpnessSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_styleSheetGet, SPtr<Instances::fl_text::StyleSheet> > TFunc_Instances_TextField_styleSheetGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_styleSheetSet, const Value, Instances::fl_text::StyleSheet*> TFunc_Instances_TextField_styleSheetSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textGet, ASString> TFunc_Instances_TextField_textGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textSet, const Value, const ASString&> TFunc_Instances_TextField_textSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textColorGet, UInt32> TFunc_Instances_TextField_textColorGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textColorSet, const Value, UInt32> TFunc_Instances_TextField_textColorSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textHeightGet, Value::Number> TFunc_Instances_TextField_textHeightGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_textWidthGet, Value::Number> TFunc_Instances_TextField_textWidthGet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_thicknessGet, Value::Number> TFunc_Instances_TextField_thicknessGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_thicknessSet, const Value, Value::Number> TFunc_Instances_TextField_thicknessSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_typeGet, ASString> TFunc_Instances_TextField_typeGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_typeSet, const Value, const ASString&> TFunc_Instances_TextField_typeSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_useRichTextClipboardGet, bool> TFunc_Instances_TextField_useRichTextClipboardGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_useRichTextClipboardSet, const Value, bool> TFunc_Instances_TextField_useRichTextClipboardSet;
typedef ThunkFunc0<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_wordWrapGet, bool> TFunc_Instances_TextField_wordWrapGet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_wordWrapSet, const Value, bool> TFunc_Instances_TextField_wordWrapSet;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_appendText, const Value, const ASString&> TFunc_Instances_TextField_appendText;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getCharBoundaries, SPtr<Instances::fl_geom::Rectangle>, SInt32> TFunc_Instances_TextField_getCharBoundaries;
typedef ThunkFunc2<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getCharIndexAtPoint, SInt32, Value::Number, Value::Number> TFunc_Instances_TextField_getCharIndexAtPoint;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getFirstCharInParagraph, SInt32, SInt32> TFunc_Instances_TextField_getFirstCharInParagraph;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getImageReference, SPtr<Instances::fl_display::DisplayObject>, const ASString&> TFunc_Instances_TextField_getImageReference;
typedef ThunkFunc2<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineIndexAtPoint, SInt32, Value::Number, Value::Number> TFunc_Instances_TextField_getLineIndexAtPoint;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineIndexOfChar, SInt32, SInt32> TFunc_Instances_TextField_getLineIndexOfChar;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineLength, SInt32, SInt32> TFunc_Instances_TextField_getLineLength;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineMetrics, SPtr<Instances::fl_text::TextLineMetrics>, SInt32> TFunc_Instances_TextField_getLineMetrics;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineOffset, SInt32, SInt32> TFunc_Instances_TextField_getLineOffset;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getLineText, ASString, SInt32> TFunc_Instances_TextField_getLineText;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getParagraphLength, SInt32, SInt32> TFunc_Instances_TextField_getParagraphLength;
typedef ThunkFunc2<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_getTextFormat, SPtr<Instances::fl_text::TextFormat>, SInt32, SInt32> TFunc_Instances_TextField_getTextFormat;
typedef ThunkFunc1<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_replaceSelectedText, const Value, const ASString&> TFunc_Instances_TextField_replaceSelectedText;
typedef ThunkFunc3<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_replaceText, const Value, SInt32, SInt32, const ASString&> TFunc_Instances_TextField_replaceText;
typedef ThunkFunc2<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_setSelection, const Value, SInt32, SInt32> TFunc_Instances_TextField_setSelection;
typedef ThunkFunc3<Instances::fl_text::TextField, Instances::fl_text::TextField::mid_setTextFormat, const Value, Instances::fl_text::TextFormat*, SInt32, SInt32> TFunc_Instances_TextField_setTextFormat;

template <> const TFunc_Instances_TextField_alwaysShowSelectionGet::TMethod TFunc_Instances_TextField_alwaysShowSelectionGet::Method = &Instances::fl_text::TextField::alwaysShowSelectionGet;
template <> const TFunc_Instances_TextField_alwaysShowSelectionSet::TMethod TFunc_Instances_TextField_alwaysShowSelectionSet::Method = &Instances::fl_text::TextField::alwaysShowSelectionSet;
template <> const TFunc_Instances_TextField_antiAliasTypeGet::TMethod TFunc_Instances_TextField_antiAliasTypeGet::Method = &Instances::fl_text::TextField::antiAliasTypeGet;
template <> const TFunc_Instances_TextField_antiAliasTypeSet::TMethod TFunc_Instances_TextField_antiAliasTypeSet::Method = &Instances::fl_text::TextField::antiAliasTypeSet;
template <> const TFunc_Instances_TextField_autoSizeGet::TMethod TFunc_Instances_TextField_autoSizeGet::Method = &Instances::fl_text::TextField::autoSizeGet;
template <> const TFunc_Instances_TextField_autoSizeSet::TMethod TFunc_Instances_TextField_autoSizeSet::Method = &Instances::fl_text::TextField::autoSizeSet;
template <> const TFunc_Instances_TextField_backgroundGet::TMethod TFunc_Instances_TextField_backgroundGet::Method = &Instances::fl_text::TextField::backgroundGet;
template <> const TFunc_Instances_TextField_backgroundSet::TMethod TFunc_Instances_TextField_backgroundSet::Method = &Instances::fl_text::TextField::backgroundSet;
template <> const TFunc_Instances_TextField_backgroundColorGet::TMethod TFunc_Instances_TextField_backgroundColorGet::Method = &Instances::fl_text::TextField::backgroundColorGet;
template <> const TFunc_Instances_TextField_backgroundColorSet::TMethod TFunc_Instances_TextField_backgroundColorSet::Method = &Instances::fl_text::TextField::backgroundColorSet;
template <> const TFunc_Instances_TextField_borderGet::TMethod TFunc_Instances_TextField_borderGet::Method = &Instances::fl_text::TextField::borderGet;
template <> const TFunc_Instances_TextField_borderSet::TMethod TFunc_Instances_TextField_borderSet::Method = &Instances::fl_text::TextField::borderSet;
template <> const TFunc_Instances_TextField_borderColorGet::TMethod TFunc_Instances_TextField_borderColorGet::Method = &Instances::fl_text::TextField::borderColorGet;
template <> const TFunc_Instances_TextField_borderColorSet::TMethod TFunc_Instances_TextField_borderColorSet::Method = &Instances::fl_text::TextField::borderColorSet;
template <> const TFunc_Instances_TextField_bottomScrollVGet::TMethod TFunc_Instances_TextField_bottomScrollVGet::Method = &Instances::fl_text::TextField::bottomScrollVGet;
template <> const TFunc_Instances_TextField_caretIndexGet::TMethod TFunc_Instances_TextField_caretIndexGet::Method = &Instances::fl_text::TextField::caretIndexGet;
template <> const TFunc_Instances_TextField_condenseWhiteGet::TMethod TFunc_Instances_TextField_condenseWhiteGet::Method = &Instances::fl_text::TextField::condenseWhiteGet;
template <> const TFunc_Instances_TextField_condenseWhiteSet::TMethod TFunc_Instances_TextField_condenseWhiteSet::Method = &Instances::fl_text::TextField::condenseWhiteSet;
template <> const TFunc_Instances_TextField_contextMenuGet::TMethod TFunc_Instances_TextField_contextMenuGet::Method = &Instances::fl_text::TextField::contextMenuGet;
template <> const TFunc_Instances_TextField_defaultTextFormatGet::TMethod TFunc_Instances_TextField_defaultTextFormatGet::Method = &Instances::fl_text::TextField::defaultTextFormatGet;
template <> const TFunc_Instances_TextField_defaultTextFormatSet::TMethod TFunc_Instances_TextField_defaultTextFormatSet::Method = &Instances::fl_text::TextField::defaultTextFormatSet;
template <> const TFunc_Instances_TextField_displayAsPasswordGet::TMethod TFunc_Instances_TextField_displayAsPasswordGet::Method = &Instances::fl_text::TextField::displayAsPasswordGet;
template <> const TFunc_Instances_TextField_displayAsPasswordSet::TMethod TFunc_Instances_TextField_displayAsPasswordSet::Method = &Instances::fl_text::TextField::displayAsPasswordSet;
template <> const TFunc_Instances_TextField_embedFontsGet::TMethod TFunc_Instances_TextField_embedFontsGet::Method = &Instances::fl_text::TextField::embedFontsGet;
template <> const TFunc_Instances_TextField_embedFontsSet::TMethod TFunc_Instances_TextField_embedFontsSet::Method = &Instances::fl_text::TextField::embedFontsSet;
template <> const TFunc_Instances_TextField_gridFitTypeGet::TMethod TFunc_Instances_TextField_gridFitTypeGet::Method = &Instances::fl_text::TextField::gridFitTypeGet;
template <> const TFunc_Instances_TextField_gridFitTypeSet::TMethod TFunc_Instances_TextField_gridFitTypeSet::Method = &Instances::fl_text::TextField::gridFitTypeSet;
template <> const TFunc_Instances_TextField_htmlTextGet::TMethod TFunc_Instances_TextField_htmlTextGet::Method = &Instances::fl_text::TextField::htmlTextGet;
template <> const TFunc_Instances_TextField_htmlTextSet::TMethod TFunc_Instances_TextField_htmlTextSet::Method = &Instances::fl_text::TextField::htmlTextSet;
template <> const TFunc_Instances_TextField_lengthGet::TMethod TFunc_Instances_TextField_lengthGet::Method = &Instances::fl_text::TextField::lengthGet;
template <> const TFunc_Instances_TextField_maxCharsGet::TMethod TFunc_Instances_TextField_maxCharsGet::Method = &Instances::fl_text::TextField::maxCharsGet;
template <> const TFunc_Instances_TextField_maxCharsSet::TMethod TFunc_Instances_TextField_maxCharsSet::Method = &Instances::fl_text::TextField::maxCharsSet;
template <> const TFunc_Instances_TextField_maxScrollHGet::TMethod TFunc_Instances_TextField_maxScrollHGet::Method = &Instances::fl_text::TextField::maxScrollHGet;
template <> const TFunc_Instances_TextField_maxScrollVGet::TMethod TFunc_Instances_TextField_maxScrollVGet::Method = &Instances::fl_text::TextField::maxScrollVGet;
template <> const TFunc_Instances_TextField_mouseWheelEnabledGet::TMethod TFunc_Instances_TextField_mouseWheelEnabledGet::Method = &Instances::fl_text::TextField::mouseWheelEnabledGet;
template <> const TFunc_Instances_TextField_mouseWheelEnabledSet::TMethod TFunc_Instances_TextField_mouseWheelEnabledSet::Method = &Instances::fl_text::TextField::mouseWheelEnabledSet;
template <> const TFunc_Instances_TextField_multilineGet::TMethod TFunc_Instances_TextField_multilineGet::Method = &Instances::fl_text::TextField::multilineGet;
template <> const TFunc_Instances_TextField_multilineSet::TMethod TFunc_Instances_TextField_multilineSet::Method = &Instances::fl_text::TextField::multilineSet;
template <> const TFunc_Instances_TextField_numLinesGet::TMethod TFunc_Instances_TextField_numLinesGet::Method = &Instances::fl_text::TextField::numLinesGet;
template <> const TFunc_Instances_TextField_restrictGet::TMethod TFunc_Instances_TextField_restrictGet::Method = &Instances::fl_text::TextField::restrictGet;
template <> const TFunc_Instances_TextField_restrictSet::TMethod TFunc_Instances_TextField_restrictSet::Method = &Instances::fl_text::TextField::restrictSet;
template <> const TFunc_Instances_TextField_scrollHGet::TMethod TFunc_Instances_TextField_scrollHGet::Method = &Instances::fl_text::TextField::scrollHGet;
template <> const TFunc_Instances_TextField_scrollHSet::TMethod TFunc_Instances_TextField_scrollHSet::Method = &Instances::fl_text::TextField::scrollHSet;
template <> const TFunc_Instances_TextField_scrollVGet::TMethod TFunc_Instances_TextField_scrollVGet::Method = &Instances::fl_text::TextField::scrollVGet;
template <> const TFunc_Instances_TextField_scrollVSet::TMethod TFunc_Instances_TextField_scrollVSet::Method = &Instances::fl_text::TextField::scrollVSet;
template <> const TFunc_Instances_TextField_selectableGet::TMethod TFunc_Instances_TextField_selectableGet::Method = &Instances::fl_text::TextField::selectableGet;
template <> const TFunc_Instances_TextField_selectableSet::TMethod TFunc_Instances_TextField_selectableSet::Method = &Instances::fl_text::TextField::selectableSet;
template <> const TFunc_Instances_TextField_selectionBeginIndexGet::TMethod TFunc_Instances_TextField_selectionBeginIndexGet::Method = &Instances::fl_text::TextField::selectionBeginIndexGet;
template <> const TFunc_Instances_TextField_selectionEndIndexGet::TMethod TFunc_Instances_TextField_selectionEndIndexGet::Method = &Instances::fl_text::TextField::selectionEndIndexGet;
template <> const TFunc_Instances_TextField_sharpnessGet::TMethod TFunc_Instances_TextField_sharpnessGet::Method = &Instances::fl_text::TextField::sharpnessGet;
template <> const TFunc_Instances_TextField_sharpnessSet::TMethod TFunc_Instances_TextField_sharpnessSet::Method = &Instances::fl_text::TextField::sharpnessSet;
template <> const TFunc_Instances_TextField_styleSheetGet::TMethod TFunc_Instances_TextField_styleSheetGet::Method = &Instances::fl_text::TextField::styleSheetGet;
template <> const TFunc_Instances_TextField_styleSheetSet::TMethod TFunc_Instances_TextField_styleSheetSet::Method = &Instances::fl_text::TextField::styleSheetSet;
template <> const TFunc_Instances_TextField_textGet::TMethod TFunc_Instances_TextField_textGet::Method = &Instances::fl_text::TextField::textGet;
template <> const TFunc_Instances_TextField_textSet::TMethod TFunc_Instances_TextField_textSet::Method = &Instances::fl_text::TextField::textSet;
template <> const TFunc_Instances_TextField_textColorGet::TMethod TFunc_Instances_TextField_textColorGet::Method = &Instances::fl_text::TextField::textColorGet;
template <> const TFunc_Instances_TextField_textColorSet::TMethod TFunc_Instances_TextField_textColorSet::Method = &Instances::fl_text::TextField::textColorSet;
template <> const TFunc_Instances_TextField_textHeightGet::TMethod TFunc_Instances_TextField_textHeightGet::Method = &Instances::fl_text::TextField::textHeightGet;
template <> const TFunc_Instances_TextField_textWidthGet::TMethod TFunc_Instances_TextField_textWidthGet::Method = &Instances::fl_text::TextField::textWidthGet;
template <> const TFunc_Instances_TextField_thicknessGet::TMethod TFunc_Instances_TextField_thicknessGet::Method = &Instances::fl_text::TextField::thicknessGet;
template <> const TFunc_Instances_TextField_thicknessSet::TMethod TFunc_Instances_TextField_thicknessSet::Method = &Instances::fl_text::TextField::thicknessSet;
template <> const TFunc_Instances_TextField_typeGet::TMethod TFunc_Instances_TextField_typeGet::Method = &Instances::fl_text::TextField::typeGet;
template <> const TFunc_Instances_TextField_typeSet::TMethod TFunc_Instances_TextField_typeSet::Method = &Instances::fl_text::TextField::typeSet;
template <> const TFunc_Instances_TextField_useRichTextClipboardGet::TMethod TFunc_Instances_TextField_useRichTextClipboardGet::Method = &Instances::fl_text::TextField::useRichTextClipboardGet;
template <> const TFunc_Instances_TextField_useRichTextClipboardSet::TMethod TFunc_Instances_TextField_useRichTextClipboardSet::Method = &Instances::fl_text::TextField::useRichTextClipboardSet;
template <> const TFunc_Instances_TextField_wordWrapGet::TMethod TFunc_Instances_TextField_wordWrapGet::Method = &Instances::fl_text::TextField::wordWrapGet;
template <> const TFunc_Instances_TextField_wordWrapSet::TMethod TFunc_Instances_TextField_wordWrapSet::Method = &Instances::fl_text::TextField::wordWrapSet;
template <> const TFunc_Instances_TextField_appendText::TMethod TFunc_Instances_TextField_appendText::Method = &Instances::fl_text::TextField::appendText;
template <> const TFunc_Instances_TextField_getCharBoundaries::TMethod TFunc_Instances_TextField_getCharBoundaries::Method = &Instances::fl_text::TextField::getCharBoundaries;
template <> const TFunc_Instances_TextField_getCharIndexAtPoint::TMethod TFunc_Instances_TextField_getCharIndexAtPoint::Method = &Instances::fl_text::TextField::getCharIndexAtPoint;
template <> const TFunc_Instances_TextField_getFirstCharInParagraph::TMethod TFunc_Instances_TextField_getFirstCharInParagraph::Method = &Instances::fl_text::TextField::getFirstCharInParagraph;
template <> const TFunc_Instances_TextField_getImageReference::TMethod TFunc_Instances_TextField_getImageReference::Method = &Instances::fl_text::TextField::getImageReference;
template <> const TFunc_Instances_TextField_getLineIndexAtPoint::TMethod TFunc_Instances_TextField_getLineIndexAtPoint::Method = &Instances::fl_text::TextField::getLineIndexAtPoint;
template <> const TFunc_Instances_TextField_getLineIndexOfChar::TMethod TFunc_Instances_TextField_getLineIndexOfChar::Method = &Instances::fl_text::TextField::getLineIndexOfChar;
template <> const TFunc_Instances_TextField_getLineLength::TMethod TFunc_Instances_TextField_getLineLength::Method = &Instances::fl_text::TextField::getLineLength;
template <> const TFunc_Instances_TextField_getLineMetrics::TMethod TFunc_Instances_TextField_getLineMetrics::Method = &Instances::fl_text::TextField::getLineMetrics;
template <> const TFunc_Instances_TextField_getLineOffset::TMethod TFunc_Instances_TextField_getLineOffset::Method = &Instances::fl_text::TextField::getLineOffset;
template <> const TFunc_Instances_TextField_getLineText::TMethod TFunc_Instances_TextField_getLineText::Method = &Instances::fl_text::TextField::getLineText;
template <> const TFunc_Instances_TextField_getParagraphLength::TMethod TFunc_Instances_TextField_getParagraphLength::Method = &Instances::fl_text::TextField::getParagraphLength;
template <> const TFunc_Instances_TextField_getTextFormat::TMethod TFunc_Instances_TextField_getTextFormat::Method = &Instances::fl_text::TextField::getTextFormat;
template <> const TFunc_Instances_TextField_replaceSelectedText::TMethod TFunc_Instances_TextField_replaceSelectedText::Method = &Instances::fl_text::TextField::replaceSelectedText;
template <> const TFunc_Instances_TextField_replaceText::TMethod TFunc_Instances_TextField_replaceText::Method = &Instances::fl_text::TextField::replaceText;
template <> const TFunc_Instances_TextField_setSelection::TMethod TFunc_Instances_TextField_setSelection::Method = &Instances::fl_text::TextField::setSelection;
template <> const TFunc_Instances_TextField_setTextFormat::TMethod TFunc_Instances_TextField_setTextFormat::Method = &Instances::fl_text::TextField::setTextFormat;

namespace Instances { namespace fl_text
{
    TextField::TextField(InstanceTraits::Traits& t)
    : Instances::fl_display::InteractiveObject(t)
//##protect##"instance::TextField::TextField()$data"
//##protect##"instance::TextField::TextField()$data"
    {
//##protect##"instance::TextField::TextField()$code"
//##protect##"instance::TextField::TextField()$code"
    }

    void TextField::alwaysShowSelectionGet(bool& result)
    {
//##protect##"instance::TextField::alwaysShowSelectionGet()"
        result = GetTextField()->DoesAlwaysShowSelection();
//##protect##"instance::TextField::alwaysShowSelectionGet()"
    }
    void TextField::alwaysShowSelectionSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::alwaysShowSelectionSet()"
        SF_UNUSED1(result);
        GetTextField()->SetAlwaysShowSelection(value);
//##protect##"instance::TextField::alwaysShowSelectionSet()"
    }
    void TextField::antiAliasTypeGet(ASString& result)
    {
//##protect##"instance::TextField::antiAliasTypeGet()"
        result = GetVM().GetStringManager().CreateConstString(
            GetTextField()->IsAAForReadability() ? "advanced" : "normal");
//##protect##"instance::TextField::antiAliasTypeGet()"
    }
    void TextField::antiAliasTypeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::antiAliasTypeSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        if (value == "normal")
            ptxtDisp->ClearAAForReadability();
        else if (value == "advanced")
            ptxtDisp->SetAAForReadability();
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::antiAliasTypeSet()"
    }
    void TextField::autoSizeGet(ASString& result)
    {
//##protect##"instance::TextField::autoSizeGet()"
        GFx::TextField* ptxtDisp = GetTextField();
        if (!ptxtDisp->IsAutoSize())
            result = GetVM().GetStringManager().CreateConstString("none");
        else
        {
            switch(ptxtDisp->GetAlignment())
            {
            case Text::DocView::Align_Left:
                result = GetVM().GetStringManager().CreateConstString("left");
                break;
            case Text::DocView::Align_Right:
                result = GetVM().GetStringManager().CreateConstString("right");
                break;
            case Text::DocView::Align_Center:
                result = GetVM().GetStringManager().CreateConstString("center");
                break;
            default:  SF_ASSERT(0); // unknown
            }
        }
//##protect##"instance::TextField::autoSizeGet()"
    }
    void TextField::autoSizeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::autoSizeSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        Text::DocView::ViewAlignment oldAlignment = ptxtDisp->GetAlignment();
        bool oldAutoSize = ptxtDisp->IsAutoSize();
        if (value == "none")
        {
            ptxtDisp->ClearAutoSize();
            ptxtDisp->SetAlignment(Text::DocView::Align_Left);
        }
        else 
        {
            ptxtDisp->SetAutoSize();
            if (value == "left")
                ptxtDisp->SetAlignment(Text::DocView::Align_Left);
            else if (value == "right")
                ptxtDisp->SetAlignment(Text::DocView::Align_Right);
            else if (value == "center")
                ptxtDisp->SetAlignment(Text::DocView::Align_Center);
        }
        if ((oldAlignment != ptxtDisp->GetAlignment()) || (oldAutoSize != ptxtDisp->IsAutoSize()))
            UpdateAutosizeSettings();
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::autoSizeSet()"
    }
    void TextField::backgroundGet(bool& result)
    {
//##protect##"instance::TextField::backgroundGet()"
        result = GetTextField()->HasBackground();
//##protect##"instance::TextField::backgroundGet()"
    }
    void TextField::backgroundSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::backgroundSet()"
        SF_UNUSED1(result);
        GetTextField()->SetBackground(value);
//##protect##"instance::TextField::backgroundSet()"
    }
    void TextField::backgroundColorGet(UInt32& result)
    {
//##protect##"instance::TextField::backgroundColorGet()"
        result = GetTextField()->GetBackgroundColor24();
//##protect##"instance::TextField::backgroundColorGet()"
    }
    void TextField::backgroundColorSet(const Value& result, UInt32 value)
    {
//##protect##"instance::TextField::backgroundColorSet()"
        SF_UNUSED1(result);
        GetTextField()->SetBackgroundColor(value);
//##protect##"instance::TextField::backgroundColorSet()"
    }
    void TextField::borderGet(bool& result)
    {
//##protect##"instance::TextField::borderGet()"
        result = GetTextField()->HasBorder();
//##protect##"instance::TextField::borderGet()"
    }
    void TextField::borderSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::borderSet()"
        SF_UNUSED1(result);
        GetTextField()->SetBorder(value);
//##protect##"instance::TextField::borderSet()"
    }
    void TextField::borderColorGet(UInt32& result)
    {
//##protect##"instance::TextField::borderColorGet()"
        result = GetTextField()->GetBorderColor24();
//##protect##"instance::TextField::borderColorGet()"
    }
    void TextField::borderColorSet(const Value& result, UInt32 value)
    {
//##protect##"instance::TextField::borderColorSet()"
        SF_UNUSED1(result);
        GetTextField()->SetBorderColor(value);
//##protect##"instance::TextField::borderColorSet()"
    }
    void TextField::bottomScrollVGet(SInt32& result)
    {
//##protect##"instance::TextField::bottomScrollVGet()"
        result = GetTextField()->GetBottomVScroll() + 1;
//##protect##"instance::TextField::bottomScrollVGet()"
    }
    void TextField::caretIndexGet(SInt32& result)
    {
//##protect##"instance::TextField::caretIndexGet()"
        result = static_cast<SInt32>(GetTextField()->GetCaretIndex());
//##protect##"instance::TextField::caretIndexGet()"
    }
    void TextField::condenseWhiteGet(bool& result)
    {
//##protect##"instance::TextField::condenseWhiteGet()"
        result = GetTextField()->IsCondenseWhite();
//##protect##"instance::TextField::condenseWhiteGet()"
    }
    void TextField::condenseWhiteSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::condenseWhiteSet()"
        SF_UNUSED1(result);
        GetTextField()->SetCondenseWhite(value);
//##protect##"instance::TextField::condenseWhiteSet()"
    }
    void TextField::contextMenuGet(SPtr<Instances::fl_display::NativeMenu>& result)
    {
//##protect##"instance::TextField::contextMenuGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::TextField::contextMenuGet()");
//##protect##"instance::TextField::contextMenuGet()"
    }
    void TextField::defaultTextFormatGet(SPtr<Instances::fl_text::TextFormat>& result)
    {
//##protect##"instance::TextField::defaultTextFormatGet()"
        GFx::TextField* pthis = GetTextField();
        const Text::TextFormat*      ptextFmt = pthis->GetDefaultTextFormat();
        const Text::ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(result, asvm.TextFormatClass, 0);
        SF_ASSERT(result);

        SF_ASSERT(ptextFmt && pparaFmt);
        result->SetTextFormat(*pparaFmt, *ptextFmt);
//##protect##"instance::TextField::defaultTextFormatGet()"
    }
    void TextField::defaultTextFormatSet(const Value& result, Instances::fl_text::TextFormat* value)
    {
//##protect##"instance::TextField::defaultTextFormatSet()"
        SF_UNUSED1(result);
        GFx::TextField* pthis = GetTextField();
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Text::TextFormat fmt(asvm.GetMovieImpl()->GetHeap());
        Text::ParagraphFormat paraFmt;

        value->GetTextFormat(&paraFmt, &fmt);

        const Text::TextFormat*      ptextFmt = pthis->GetDefaultTextFormat();
        const Text::ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();
        pthis->SetDefaultTextFormat(ptextFmt->Merge(fmt));
        pthis->SetDefaultParagraphFormat(pparaFmt->Merge(paraFmt));
//##protect##"instance::TextField::defaultTextFormatSet()"
    }
    void TextField::displayAsPasswordGet(bool& result)
    {
//##protect##"instance::TextField::displayAsPasswordGet()"
        result = GetTextField()->IsPassword();
//##protect##"instance::TextField::displayAsPasswordGet()"
    }
    void TextField::displayAsPasswordSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::displayAsPasswordSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        bool oldPasswd = ptxtDisp->IsPassword();
        if (oldPasswd != value)
        {
            ptxtDisp->SetPassword(value);
            if (value)
                ptxtDisp->SetPasswordMode();
            else
                ptxtDisp->ClearPasswordMode();
            ptxtDisp->ForceCompleteReformat();
        }
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::displayAsPasswordSet()"
    }
    void TextField::embedFontsGet(bool& result)
    {
//##protect##"instance::TextField::embedFontsGet()"
        result = !GetTextField()->DoesUseDeviceFont();
//##protect##"instance::TextField::embedFontsGet()"
    }
    void TextField::embedFontsSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::embedFontsSet()"
        SF_UNUSED1(result);
        if (value)
            GetTextField()->ClearUseDeviceFont();
        else
            GetTextField()->SetUseDeviceFont();
//##protect##"instance::TextField::embedFontsSet()"
    }
    void TextField::gridFitTypeGet(ASString& result)
    {
//##protect##"instance::TextField::gridFitTypeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("TextField::gridFitTypeGet()");
//##protect##"instance::TextField::gridFitTypeGet()"
    }
    void TextField::gridFitTypeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::gridFitTypeSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("TextField::gridFitTypeSet()");
//##protect##"instance::TextField::gridFitTypeSet()"
    }
    void TextField::htmlTextGet(ASString& result)
    {
//##protect##"instance::TextField::htmlTextGet()"
        result = GetTextField()->GetHtml();
//##protect##"instance::TextField::htmlTextGet()"
    }
    void TextField::htmlTextSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::htmlTextSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        ptxtDisp->SetHtml(true);
        ptxtDisp->SetTextValue(value.ToCStr(), true);
//##protect##"instance::TextField::htmlTextSet()"
    }
    void TextField::lengthGet(SInt32& result)
    {
//##protect##"instance::TextField::lengthGet()"
        result = static_cast<SInt32>(GetTextField()->GetTextLength());
//##protect##"instance::TextField::lengthGet()"
    }
    void TextField::maxCharsGet(SInt32& result)
    {
//##protect##"instance::TextField::maxCharsGet()"
        GFx::TextField* ptxtDisp = GetTextField();
        result = 0;
        if (ptxtDisp->HasMaxLength())
            result = ptxtDisp->GetMaxLength();
//##protect##"instance::TextField::maxCharsGet()"
    }
    void TextField::maxCharsSet(const Value& result, SInt32 value)
    {
//##protect##"instance::TextField::maxCharsSet()"
        SF_UNUSED1(result);
        if (value >= 0)
            GetTextField()->SetMaxLength(value);
//##protect##"instance::TextField::maxCharsSet()"
    }
    void TextField::maxScrollHGet(SInt32& result)
    {
//##protect##"instance::TextField::maxScrollHGet()"
        result = (SInt32)GetTextField()->GetMaxHScroll();
//##protect##"instance::TextField::maxScrollHGet()"
    }
    void TextField::maxScrollVGet(SInt32& result)
    {
//##protect##"instance::TextField::maxScrollVGet()"
        result = GetTextField()->GetMaxVScroll() + 1;
//##protect##"instance::TextField::maxScrollVGet()"
    }
    void TextField::mouseWheelEnabledGet(bool& result)
    {
//##protect##"instance::TextField::mouseWheelEnabledGet()"
        result = GetTextField()->IsMouseWheelEnabled();
//##protect##"instance::TextField::mouseWheelEnabledGet()"
    }
    void TextField::mouseWheelEnabledSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::mouseWheelEnabledSet()"
        SF_UNUSED1(result);
        GetTextField()->SetMouseWheelEnabled(value);
//##protect##"instance::TextField::mouseWheelEnabledSet()"
    }
    void TextField::multilineGet(bool& result)
    {
//##protect##"instance::TextField::multilineGet()"
        result = GetTextField()->IsMultiline();
//##protect##"instance::TextField::multilineGet()"
    }
    void TextField::multilineSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::multilineSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        bool oldMultiline = ptxtDisp->IsMultiline();
        if (oldMultiline != value)
        {
            if (value)
                ptxtDisp->SetMultiline();
            else
                ptxtDisp->ClearMultiline();
            UpdateAutosizeSettings();
        }
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::multilineSet()"
    }
    void TextField::numLinesGet(SInt32& result)
    {
//##protect##"instance::TextField::numLinesGet()"
        result = GetTextField()->GetLinesCount();
//##protect##"instance::TextField::numLinesGet()"
    }
    void TextField::restrictGet(ASString& result)
    {
//##protect##"instance::TextField::restrictGet()"
        const String* restr = GetTextField()->GetRestrict();
        if (restr)
            result = GetVM().GetStringManager().CreateString(*restr);
        else
            result.SetNull();
//##protect##"instance::TextField::restrictGet()"
    }
    void TextField::restrictSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::restrictSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        if (value.IsNull())
            ptxtDisp->ClearRestrict();
        else
            ptxtDisp->SetRestrict(value);
//##protect##"instance::TextField::restrictSet()"
    }
    void TextField::scrollHGet(SInt32& result)
    {
//##protect##"instance::TextField::scrollHGet()"
        result = (SInt32)GetTextField()->GetHScrollOffset();
//##protect##"instance::TextField::scrollHGet()"
    }
    void TextField::scrollHSet(const Value& result, SInt32 value)
    {
//##protect##"instance::TextField::scrollHSet()"
        SF_UNUSED1(result);
        if (value < 0) value = 0;
        GFx::TextField* ptxtDisp = GetTextField();
        ptxtDisp->SetHScrollOffset((Double)value);
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::scrollHSet()"
    }
    void TextField::scrollVGet(SInt32& result)
    {
//##protect##"instance::TextField::scrollVGet()"
        result = GetTextField()->GetVScrollOffset() + 1;
//##protect##"instance::TextField::scrollVGet()"
    }
    void TextField::scrollVSet(const Value& result, SInt32 value)
    {
//##protect##"instance::TextField::scrollVSet()"
        SF_UNUSED(result);
        if (value < 1) value = 1;
        GFx::TextField* ptxtDisp = GetTextField();
        ptxtDisp->SetVScrollOffset((unsigned)(value - 1));
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::scrollVSet()"
    }
    void TextField::selectableGet(bool& result)
    {
//##protect##"instance::TextField::selectableGet()"
        result = GetTextField()->IsSelectable();
//##protect##"instance::TextField::selectableGet()"
    }
    void TextField::selectableSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::selectableSet()"
        SF_UNUSED1(result);
        GetTextField()->SetSelectable(value);
//##protect##"instance::TextField::selectableSet()"
    }
    void TextField::selectionBeginIndexGet(SInt32& result)
    {
//##protect##"instance::TextField::selectionBeginIndexGet()"
        GFx::TextField* ptxtDisp = GetTextField();
        result = static_cast<SInt32>(ptxtDisp->GetBeginIndex());
//##protect##"instance::TextField::selectionBeginIndexGet()"
    }
    void TextField::selectionEndIndexGet(SInt32& result)
    {
//##protect##"instance::TextField::selectionEndIndexGet()"
        GFx::TextField* ptxtDisp = GetTextField();
        result = static_cast<SInt32>(ptxtDisp->GetEndIndex());
//##protect##"instance::TextField::selectionEndIndexGet()"
    }
    void TextField::sharpnessGet(Value::Number& result)
    {
//##protect##"instance::TextField::sharpnessGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("TextField::sharpnessGet()");
//##protect##"instance::TextField::sharpnessGet()"
    }
    void TextField::sharpnessSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TextField::sharpnessSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("TextField::sharpnessSet()");
//##protect##"instance::TextField::sharpnessSet()"
    }
    void TextField::styleSheetGet(SPtr<Instances::fl_text::StyleSheet>& result)
    {
//##protect##"instance::TextField::styleSheetGet()"
//         result.SetUndefined();

#ifdef GFX_ENABLE_CSS

        AvmTextField* patf = ToAvmTextField(GetTextField());
        if (patf->GetCSSData() && patf->GetCSSData()->pASStyleSheet.GetPtr())
            result = patf->GetCSSData()->pASStyleSheet;

#else

        SF_UNUSED1(result);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("TextField::styleSheetGet()");

#endif

//##protect##"instance::TextField::styleSheetGet()"
    }
    void TextField::styleSheetSet(const Value& result, Instances::fl_text::StyleSheet* value)
    {
//##protect##"instance::TextField::styleSheetSet()"

#ifdef GFX_ENABLE_CSS
        SF_UNUSED1(result);

        GFx::TextField* ptf = GetTextField();
        AvmTextField* patf = ToAvmTextField(GetTextField());

        if (value)
        {
            if (!ptf->GetCSSData())
                ptf->SetCSSData(new AvmTextField::CSSHolder);
            patf->GetCSSData()->pASStyleSheet = value;

            // Make the textfield read-only
            if (ptf->HasEditorKit())
            {
                ptf->GetEditorKit()->SetReadOnly();
            }

            patf->SetDirtyFlag();
        }
        else if (patf->GetCSSData())
            patf->GetCSSData()->pASStyleSheet = NULL;

        ptf->CollectUrlZones();
        ptf->UpdateUrlStyles();
        ptf->SetNeedUpdateLayoutFlag();

#else

        SF_UNUSED2(result, value);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("TextField::styleSheetSet()");

#endif

//##protect##"instance::TextField::styleSheetSet()"
    }
    void TextField::textGet(ASString& result)
    {
//##protect##"instance::TextField::textGet()"
        result = GetTextField()->GetText(false);
//##protect##"instance::TextField::textGet()"
    }
    void TextField::textSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::textSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        ptxtDisp->SetHtml(false);
        ptxtDisp->SetText(ASString(value).ToCStr(), false);
//##protect##"instance::TextField::textSet()"
    }
    void TextField::textColorGet(UInt32& result)
    {
//##protect##"instance::TextField::textColorGet()"
        result = GetTextField()->GetTextColor32();
//##protect##"instance::TextField::textColorGet()"
    }
    void TextField::textColorSet(const Value& result, UInt32 value)
    {
//##protect##"instance::TextField::textColorSet()"
        SF_UNUSED1(result);
        GetTextField()->SetTextColor(value);
//##protect##"instance::TextField::textColorSet()"
    }
    void TextField::textHeightGet(Value::Number& result)
    {
//##protect##"instance::TextField::textHeightGet()"
        result = GetTextField()->GetTextHeight();
//##protect##"instance::TextField::textHeightGet()"
    }
    void TextField::textWidthGet(Value::Number& result)
    {
//##protect##"instance::TextField::textWidthGet()"
        result = GetTextField()->GetTextWidth();
//##protect##"instance::TextField::textWidthGet()"
    }
    void TextField::thicknessGet(Value::Number& result)
    {
//##protect##"instance::TextField::thicknessGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("TextField::thicknessGet()");
//##protect##"instance::TextField::thicknessGet()"
    }
    void TextField::thicknessSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TextField::thicknessSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("TextField::thicknessSet()");
//##protect##"instance::TextField::thicknessSet()"
    }
    void TextField::typeGet(ASString& result)
    {
//##protect##"instance::TextField::typeGet()"
        result = GetVM().GetStringManager().CreateConstString(
            GetTextField()->IsReadOnly() ? "dynamic" : "input");
//##protect##"instance::TextField::typeGet()"
    }
    void TextField::typeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::typeSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        if (value == "dynamic")
        {
            if (ptxtDisp->HasEditorKit())
                ptxtDisp->GetEditorKit()->SetReadOnly();
        }
        else if (value == "input" && !ptxtDisp->HasStyleSheet())
        {
            Text::EditorKit* peditor = ptxtDisp->CreateEditorKit();
            peditor->ClearReadOnly();
        }
        ptxtDisp->ForceReformat();
//##protect##"instance::TextField::typeSet()"
    }
    void TextField::useRichTextClipboardGet(bool& result)
    {
//##protect##"instance::TextField::useRichTextClipboardGet()"
        result = GetTextField()->DoesUseRichClipboard();
//##protect##"instance::TextField::useRichTextClipboardGet()"
    }
    void TextField::useRichTextClipboardSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::useRichTextClipboardSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        ptxtDisp->SetUseRichClipboard(value);
        if (ptxtDisp->HasEditorKit())
        {
            if (ptxtDisp->DoesUseRichClipboard())
                ptxtDisp->GetEditorKit()->SetUseRichClipboard();
            else
                ptxtDisp->GetEditorKit()->ClearUseRichClipboard();
        }
//##protect##"instance::TextField::useRichTextClipboardSet()"
    }
    void TextField::wordWrapGet(bool& result)
    {
//##protect##"instance::TextField::wordWrapGet()"
        result = GetTextField()->IsWordWrap();
//##protect##"instance::TextField::wordWrapGet()"
    }
    void TextField::wordWrapSet(const Value& result, bool value)
    {
//##protect##"instance::TextField::wordWrapSet()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        bool oldWordWrap = ptxtDisp->IsWordWrap();
        if (oldWordWrap != value)
        {
            if (value)
                ptxtDisp->SetWordWrap();
            else
                ptxtDisp->ClearWordWrap();
			UpdateAutosizeSettings();
        }
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::wordWrapSet()"
    }
    void TextField::appendText(const Value& result, const ASString& newText)
    {
//##protect##"instance::TextField::appendText()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();
        if (ptxtDisp->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        ptxtDisp->AppendText(newText.ToCStr());
        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::appendText()"
    }
    void TextField::getCharBoundaries(SPtr<Instances::fl_geom::Rectangle>& result, SInt32 charIndex)
    {
//##protect##"instance::TextField::getCharBoundaries()"
        RectF charBounds;
        charBounds.Clear();
        if (GetTextField()->GetCharBoundaries(&charBounds, charIndex))
        {
            Value argv[4];
#if 0
            // Old code.
            argv[0].SetNumber(Alg::IRound(TwipsToPixels(charBounds.x1)));
            argv[1].SetNumber(Alg::IRound(TwipsToPixels(charBounds.y1)));
            argv[2].SetNumber(Alg::IRound(TwipsToPixels(charBounds.Width())));
            argv[3].SetNumber(Alg::IRound(TwipsToPixels(charBounds.Height())));
#else
            argv[0].SetNumber(TwipsToPixels(charBounds.x1));
            argv[1].SetNumber(TwipsToPixels(charBounds.y1));
            argv[2].SetNumber(TwipsToPixels(charBounds.Width()));
            argv[3].SetNumber(TwipsToPixels(charBounds.Height()));
#endif

            ASVM& asvm = static_cast<ASVM&>(GetVM());
            asvm.ConstructInstance(result, asvm.RectangleClass, 4, argv);
            SF_ASSERT(result);
        }
//##protect##"instance::TextField::getCharBoundaries()"
    }
    void TextField::getCharIndexAtPoint(SInt32& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::TextField::getCharIndexAtPoint()"
        UPInt pos = GetTextField()->GetCharIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
        if (!Alg::IsMax(pos))
            result = static_cast<SInt32>(pos);
        else
            result = -1;
//##protect##"instance::TextField::getCharIndexAtPoint()"
    }
    void TextField::getFirstCharInParagraph(SInt32& result, SInt32 charIndex)
    {
//##protect##"instance::TextField::getFirstCharInParagraph()"
        if (charIndex < 0)
            result = -1;
        else {
            UPInt off = GetTextField()->GetFirstCharInParagraph(unsigned(charIndex));
            if (!Alg::IsMax(off))
                result = static_cast<SInt32>(off);
            else
                result = -1;
        }
//##protect##"instance::TextField::getFirstCharInParagraph()"
    }
    void TextField::getImageReference(SPtr<Instances::fl_display::DisplayObject>& result, const ASString& id)
    {
//##protect##"instance::TextField::getImageReference()"
        SF_UNUSED2(result, id);
        WARN_NOT_IMPLEMENTED("TextField::getImageReference()");
//##protect##"instance::TextField::getImageReference()"
    }
    void TextField::getLineIndexAtPoint(SInt32& result, Value::Number x, Value::Number y)
    {
//##protect##"instance::TextField::getLineIndexAtPoint()"
        unsigned  pos = GetTextField()->GetLineIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
        if (!Alg::IsMax(pos))
            result = pos;
        else
            result = -1;
//##protect##"instance::TextField::getLineIndexAtPoint()"
    }
    void TextField::getLineIndexOfChar(SInt32& result, SInt32 charIndex)
    {
//##protect##"instance::TextField::getLineIndexOfChar()"
        if (charIndex < 0)
            result = -1;
        else {
            unsigned off = GetTextField()->GetLineIndexOfChar(unsigned(charIndex));
            if (!Alg::IsMax(off))
                result = off;
            else
                result = -1;
        }
//##protect##"instance::TextField::getLineIndexOfChar()"
    }
    void TextField::getLineLength(SInt32& result, SInt32 lineIndex)
    {
//##protect##"instance::TextField::getLineLength()"
        if (lineIndex < 0)
            result = -1;
        else {
            UPInt len = GetTextField()->GetLineLength(unsigned(lineIndex));
            if (!Alg::IsMax(len))
                result = static_cast<SInt32>(len);
            else
                result = -1;
        }
//##protect##"instance::TextField::getLineLength()"
    }
    void TextField::getLineMetrics(SPtr<Instances::fl_text::TextLineMetrics>& result, SInt32 lineIndex)
    {
//##protect##"instance::TextField::getLineMetrics()"
        Text::DocView::LineMetrics metrics;
        if (GetTextField()->GetLineMetrics(unsigned(lineIndex), &metrics))
        {
            Value argv[6];
#if 0
            // Old code.
            argv[0].SetNumber(Alg::IRound(TwipsToPixels(metrics.Ascent)));
            argv[1].SetNumber(Alg::IRound(TwipsToPixels(metrics.Descent)));
            argv[2].SetNumber(Alg::IRound(TwipsToPixels(metrics.Height)));
            argv[3].SetNumber(Alg::IRound(TwipsToPixels(metrics.Leading)));
            argv[4].SetNumber(Alg::IRound(TwipsToPixels(metrics.Width)));
            argv[5].SetNumber(Alg::IRound(TwipsToPixels(metrics.FirstCharXOff)));
#else
            argv[0].SetNumber(TwipsToPixels(metrics.Ascent));
            argv[1].SetNumber(TwipsToPixels(metrics.Descent));
            argv[2].SetNumber(TwipsToPixels(metrics.Height));
            argv[3].SetNumber(TwipsToPixels(metrics.Leading));
            argv[4].SetNumber(TwipsToPixels(metrics.Width));
            argv[5].SetNumber(TwipsToPixels(metrics.FirstCharXOff));
#endif

            ASVM& asvm = static_cast<ASVM&>(GetVM());

            SPtr<AS3::Object> obj = asvm.GetClass("flash.text.TextLineMetrics", asvm.GetCurrentAppDomain());
            asvm.ConstructInstance(result, obj, 6, argv);
            SF_ASSERT(result);
        }
//##protect##"instance::TextField::getLineMetrics()"
    }
    void TextField::getLineOffset(SInt32& result, SInt32 lineIndex)
    {
//##protect##"instance::TextField::getLineOffset()"
        if (lineIndex < 0)
            result= -1;
        else {
            UPInt off = GetTextField()->GetLineOffset(unsigned(lineIndex));
            if (!Alg::IsMax(off))
                result = static_cast<SInt32>(off);
            else
                result = -1;
        }
//##protect##"instance::TextField::getLineOffset()"
    }
    void TextField::getLineText(ASString& result, SInt32 lineIndex)
    {
//##protect##"instance::TextField::getLineText()"
        UPInt len = 0;
        const wchar_t* ptext = GetTextField()->GetLineText(unsigned(lineIndex), &len);
        if (ptext)
        {
            String str;
            str.AppendString(ptext, (SPInt)len);
            result = GetVM().GetStringManager().CreateString(str);
        }
        else
            result = GetVM().GetStringManager().CreateConstString("");

//##protect##"instance::TextField::getLineText()"
    }
    void TextField::getParagraphLength(SInt32& result, SInt32 charIndex)
    {
//##protect##"instance::TextField::getParagraphLength()"
        result = static_cast<SInt32>(GetTextField()->GetParagraphLength(charIndex));
//##protect##"instance::TextField::getParagraphLength()"
    }
    void TextField::getTextFormat(SPtr<Instances::fl_text::TextFormat>& result, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::TextField::getTextFormat()"
        if (beginIndex == -1)
            beginIndex = 0;
        if (endIndex == -1)
            endIndex = SF_MAX_SINT32;
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_text::TextFormat> tfmt;
        asvm.ConstructInstance(tfmt, asvm.TextFormatClass, 0);
        SF_ASSERT(tfmt);

        if (beginIndex <= endIndex)
        {
            Text::TextFormat textFmt(asvm.GetMovieImpl()->GetHeap());
            Text::ParagraphFormat paraFmt;
            GetTextField()->GetTextAndParagraphFormat(&textFmt, &paraFmt, beginIndex, endIndex);

            tfmt->SetTextFormat(paraFmt, textFmt);
        }
        result = tfmt;
//##protect##"instance::TextField::getTextFormat()"
    }
    void TextField::replaceSelectedText(const Value& result, const ASString& value)
    {
//##protect##"instance::TextField::replaceSelectedText()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();

        if (ptxtDisp->HasStyleSheet())
            return;

        ConstPtr<Text::TextFormat> ptextFmt = ptxtDisp->GetDefaultTextFormat();
        ConstPtr<Text::ParagraphFormat> pparaFmt = ptxtDisp->GetDefaultParagraphFormat();

        unsigned len = value.GetLength();
        UPInt startPos = ptxtDisp->GetEditorKit()->GetBeginSelection();
        UPInt endPos   = ptxtDisp->GetEditorKit()->GetEndSelection();

        wchar_t buf[1024];
        if (len < sizeof(buf)/sizeof(buf[0]))
        {
            UTF8Util::DecodeStringSafe(buf, 1024, value.ToCStr());
            ptxtDisp->ReplaceText(buf, startPos, endPos);
        }
        else {
            wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
            UTF8Util::DecodeStringSafe(pbuf, len + 1, value.ToCStr());
            ptxtDisp->ReplaceText(pbuf, startPos, endPos);
            SF_FREE(pbuf);
        }
        ptxtDisp->GetEditorKit()->SetCursorPos(startPos + len, false);

        if (pparaFmt)
            ptxtDisp->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
        if (ptextFmt)
            ptxtDisp->SetTextFormat(*ptextFmt, startPos, startPos + len);

        ptxtDisp->SetDirtyFlag();
//##protect##"instance::TextField::replaceSelectedText()"
    }
    void TextField::replaceText(const Value& result, SInt32 beginIndex, SInt32 endIndex, const ASString& newText)
    {
//##protect##"instance::TextField::replaceText()"
        SF_UNUSED1(result);
        GFx::TextField* ptxtDisp = GetTextField();

        if (ptxtDisp->HasStyleSheet())
            return;

        unsigned len = newText.GetLength();
        unsigned startPos = unsigned(beginIndex);
        unsigned endPos   = unsigned(endIndex);
        if (beginIndex >= 0 && endIndex >= 0 && startPos <= endPos)
        {
            ConstPtr<Text::TextFormat> ptextFmt;
            ConstPtr<Text::ParagraphFormat> pparaFmt;
            UPInt prevLen = ptxtDisp->GetTextLength();
            UPInt newLen  = prevLen - (endPos - startPos) + len;
            if (startPos >= prevLen)
            {
                ptextFmt = ptxtDisp->GetDefaultTextFormat();
                pparaFmt = ptxtDisp->GetDefaultParagraphFormat();
            }
            else
            {
                const Text::TextFormat* _ptextFmt;
                const Text::ParagraphFormat* _pparaFmt;
                ptxtDisp->GetTextAndParagraphFormat(&_ptextFmt, &_pparaFmt, startPos);
                ptextFmt = _ptextFmt;
                pparaFmt = _pparaFmt;
            }

            wchar_t buf[1024];
            if (len < sizeof(buf)/sizeof(buf[0]))
            {
                UTF8Util::DecodeStringSafe(buf, 1024, newText.ToCStr());
                ptxtDisp->ReplaceText(buf, startPos, endPos);
            }
            else {
                wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
                UTF8Util::DecodeStringSafe(pbuf, len + 1, newText.ToCStr());
                ptxtDisp->ReplaceText(pbuf, startPos, endPos);
                SF_FREE(pbuf);
            }

            if (ptxtDisp->HasEditorKit())
            {
                if (ptxtDisp->GetEditorKit()->GetCursorPos() > newLen)
                    ptxtDisp->GetEditorKit()->SetCursorPos(newLen, false);
            }

            if (pparaFmt)
                ptxtDisp->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
            if (ptextFmt)	
                ptxtDisp->SetTextFormat(*ptextFmt, startPos, startPos + len);
			
            ptxtDisp->SetDirtyFlag();
        }
//##protect##"instance::TextField::replaceText()"
    }
    void TextField::setSelection(const Value& result, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::TextField::setSelection()"
        SF_UNUSED1(result);
        GetTextField()->SetSelection(beginIndex, endIndex);
//##protect##"instance::TextField::setSelection()"
    }
    void TextField::setTextFormat(const Value& result, Instances::fl_text::TextFormat* format, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::TextField::setTextFormat()"
        SF_UNUSED1(result);
        if (!format)
            return; // ?exception?
        if (beginIndex == -1)
            beginIndex = 0;
        if (endIndex == -1)
            endIndex = SF_MAX_SINT32;
        if (beginIndex <= endIndex)
        {
            GFx::TextField* pthis = GetTextField();
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            Text::TextFormat fmt(asvm.GetMovieImpl()->GetHeap());
            Text::ParagraphFormat paraFmt;
            
            format->GetTextFormat(&paraFmt, &fmt);

            pthis->SetTextFormat(fmt, (unsigned)beginIndex, (unsigned)endIndex);
            pthis->SetParagraphFormat(paraFmt, (unsigned)beginIndex, (unsigned)endIndex);
            pthis->SetDirtyFlag();
        }
//##protect##"instance::TextField::setTextFormat()"
    }

    SPtr<Instances::fl_text::TextFormat> TextField::defaultTextFormatGet()
    {
        SPtr<Instances::fl_text::TextFormat> result;
        defaultTextFormatGet(result);
        return result;
    }
    SPtr<Instances::fl_text::StyleSheet> TextField::styleSheetGet()
    {
        SPtr<Instances::fl_text::StyleSheet> result;
        styleSheetGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> TextField::getCharBoundaries(SInt32 charIndex)
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        getCharBoundaries(result, charIndex);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> TextField::getImageReference(const ASString& id)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        getImageReference(result, id);
        return result;
    }
    SPtr<Instances::fl_text::TextLineMetrics> TextField::getLineMetrics(SInt32 lineIndex)
    {
        SPtr<Instances::fl_text::TextLineMetrics> result;
        getLineMetrics(result, lineIndex);
        return result;
    }
    SPtr<Instances::fl_text::TextFormat> TextField::getTextFormat(SInt32 beginIndex, SInt32 endIndex)
    {
        SPtr<Instances::fl_text::TextFormat> result;
        getTextFormat(result, beginIndex, endIndex);
        return result;
    }
//##protect##"instance$methods"
    void TextField::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            TextField::CreateStageObject();
            GFx::TextField* tf = GetTextField();
            tf->SetWidth(100);
            tf->SetHeight(100);
            tf->SetTextValue("", true);
        }
    }

    GFx::DisplayObject* TextField::CreateStageObject()
    {
        if (!pDispObj)
        {
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();

            CharacterCreateInfo ccinfo = 
                proot->GetMovieDefImpl()->GetCharacterCreateInfo(
                    ResourceId(CharacterDef::CharId_EmptyTextField));
            SF_ASSERT(ccinfo.pCharDef);

            // figure out MovieDefImpl from current call frame.
            if (GetVM().GetCallStack().GetSize() > 0)
            {
                const VMAbcFile& abcFile = GetVM().GetCurrCallFrame().GetFile();
                ccinfo.pBindDefImpl = static_cast<const ASVM::AbcFileWithMovieDef&>(abcFile.GetAbcFile()).pDefImpl;
            }

            pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->
                CreateCharacterInstance(
                    proot->GetMovieImpl(), ccinfo, NULL, 
                    ResourceId(CharacterDef::CharId_EmptyTextField)));
            AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
            pAvmObj->AssignAS3Obj(this);
            pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());
        }
        return pDispObj;
    }

    void TextField::UpdateAutosizeSettings()
    {
        GFx::TextField* ptf = GetTextField();
        const bool autoSizeByX = (ptf->IsAutoSize() && !ptf->IsWordWrap());
        const bool autoSizeByY = (ptf->IsAutoSize());
        if (autoSizeByX)
            ptf->SetAutoSizeX();
        else
            ptf->ClearAutoSizeX();
        if (autoSizeByY)
            ptf->SetAutoSizeY();
        else
            ptf->ClearAutoSizeY();
        ptf->SetNeedUpdateGeomData();
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 TextField::tito[TextField::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 23, 24, 26, 27, 29, 30, 32, 33, 35, 36, 38, 39, 41, 42, 43, 45, 46, 47, 48, 50, 51, 53, 54, 55, 57, 58, 60, 61, 63, 64, 66, 67, 68, 69, 71, 72, 74, 75, 77, 78, 80, 81, 82, 83, 85, 86, 88, 89, 91, 92, 94, 96, 98, 101, 103, 105, 108, 110, 112, 114, 116, 118, 120, 123, 125, 129, 132, 
    // };
    const TypeInfo* TextField::tit[136] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_text::TextFormatTI, 
        NULL, &AS3::fl_text::TextFormatTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_text::StyleSheetTI, 
        NULL, &AS3::fl_text::StyleSheetTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl_text::TextLineMetricsTI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl_text::TextFormatTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::StringTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl_text::TextFormatTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
    };
    const Abc::ConstValue TextField::dva[4] = {
        {Abc::CONSTANT_Int, 4}, {Abc::CONSTANT_Int, 4}, 
        {Abc::CONSTANT_Int, 4}, {Abc::CONSTANT_Int, 4}, 
    };
    const ThunkInfo TextField::ti[TextField::ThunkInfoNum] = {
        {TFunc_Instances_TextField_alwaysShowSelectionGet::Func, &TextField::tit[0], "alwaysShowSelection", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_alwaysShowSelectionSet::Func, &TextField::tit[1], "alwaysShowSelection", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_antiAliasTypeGet::Func, &TextField::tit[3], "antiAliasType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_antiAliasTypeSet::Func, &TextField::tit[4], "antiAliasType", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_autoSizeGet::Func, &TextField::tit[6], "autoSize", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_autoSizeSet::Func, &TextField::tit[7], "autoSize", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_backgroundGet::Func, &TextField::tit[9], "background", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_backgroundSet::Func, &TextField::tit[10], "background", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_backgroundColorGet::Func, &TextField::tit[12], "backgroundColor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_backgroundColorSet::Func, &TextField::tit[13], "backgroundColor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_borderGet::Func, &TextField::tit[15], "border", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_borderSet::Func, &TextField::tit[16], "border", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_borderColorGet::Func, &TextField::tit[18], "borderColor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_borderColorSet::Func, &TextField::tit[19], "borderColor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_bottomScrollVGet::Func, &TextField::tit[21], "bottomScrollV", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_caretIndexGet::Func, &TextField::tit[22], "caretIndex", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_condenseWhiteGet::Func, &TextField::tit[23], "condenseWhite", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_condenseWhiteSet::Func, &TextField::tit[24], "condenseWhite", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_defaultTextFormatGet::Func, &TextField::tit[26], "defaultTextFormat", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_defaultTextFormatSet::Func, &TextField::tit[27], "defaultTextFormat", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_displayAsPasswordGet::Func, &TextField::tit[29], "displayAsPassword", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_displayAsPasswordSet::Func, &TextField::tit[30], "displayAsPassword", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_embedFontsGet::Func, &TextField::tit[32], "embedFonts", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_embedFontsSet::Func, &TextField::tit[33], "embedFonts", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_gridFitTypeGet::Func, &TextField::tit[35], "gridFitType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_gridFitTypeSet::Func, &TextField::tit[36], "gridFitType", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_htmlTextGet::Func, &TextField::tit[38], "htmlText", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_htmlTextSet::Func, &TextField::tit[39], "htmlText", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_lengthGet::Func, &TextField::tit[41], "length", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_maxCharsGet::Func, &TextField::tit[42], "maxChars", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_maxCharsSet::Func, &TextField::tit[43], "maxChars", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_maxScrollHGet::Func, &TextField::tit[45], "maxScrollH", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_maxScrollVGet::Func, &TextField::tit[46], "maxScrollV", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_mouseWheelEnabledGet::Func, &TextField::tit[47], "mouseWheelEnabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_mouseWheelEnabledSet::Func, &TextField::tit[48], "mouseWheelEnabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_multilineGet::Func, &TextField::tit[50], "multiline", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_multilineSet::Func, &TextField::tit[51], "multiline", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_numLinesGet::Func, &TextField::tit[53], "numLines", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_restrictGet::Func, &TextField::tit[54], "restrict", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_restrictSet::Func, &TextField::tit[55], "restrict", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_scrollHGet::Func, &TextField::tit[57], "scrollH", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_scrollHSet::Func, &TextField::tit[58], "scrollH", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_scrollVGet::Func, &TextField::tit[60], "scrollV", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_scrollVSet::Func, &TextField::tit[61], "scrollV", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_selectableGet::Func, &TextField::tit[63], "selectable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_selectableSet::Func, &TextField::tit[64], "selectable", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_selectionBeginIndexGet::Func, &TextField::tit[66], "selectionBeginIndex", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_selectionEndIndexGet::Func, &TextField::tit[67], "selectionEndIndex", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_sharpnessGet::Func, &TextField::tit[68], "sharpness", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_sharpnessSet::Func, &TextField::tit[69], "sharpness", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_styleSheetGet::Func, &TextField::tit[71], "styleSheet", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_styleSheetSet::Func, &TextField::tit[72], "styleSheet", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_textGet::Func, &TextField::tit[74], "text", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_textSet::Func, &TextField::tit[75], "text", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_textColorGet::Func, &TextField::tit[77], "textColor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_textColorSet::Func, &TextField::tit[78], "textColor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_textHeightGet::Func, &TextField::tit[80], "textHeight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_textWidthGet::Func, &TextField::tit[81], "textWidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_thicknessGet::Func, &TextField::tit[82], "thickness", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_thicknessSet::Func, &TextField::tit[83], "thickness", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_typeGet::Func, &TextField::tit[85], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_typeSet::Func, &TextField::tit[86], "type", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_useRichTextClipboardGet::Func, &TextField::tit[88], "useRichTextClipboard", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_useRichTextClipboardSet::Func, &TextField::tit[89], "useRichTextClipboard", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_wordWrapGet::Func, &TextField::tit[91], "wordWrap", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextField_wordWrapSet::Func, &TextField::tit[92], "wordWrap", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_appendText::Func, &TextField::tit[94], "appendText", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getCharBoundaries::Func, &TextField::tit[96], "getCharBoundaries", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getCharIndexAtPoint::Func, &TextField::tit[98], "getCharIndexAtPoint", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_TextField_getFirstCharInParagraph::Func, &TextField::tit[101], "getFirstCharInParagraph", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getImageReference::Func, &TextField::tit[103], "getImageReference", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineIndexAtPoint::Func, &TextField::tit[105], "getLineIndexAtPoint", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineIndexOfChar::Func, &TextField::tit[108], "getLineIndexOfChar", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineLength::Func, &TextField::tit[110], "getLineLength", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineMetrics::Func, &TextField::tit[112], "getLineMetrics", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineOffset::Func, &TextField::tit[114], "getLineOffset", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getLineText::Func, &TextField::tit[116], "getLineText", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getParagraphLength::Func, &TextField::tit[118], "getParagraphLength", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_getTextFormat::Func, &TextField::tit[120], "getTextFormat", NULL, Abc::NS_Public, CT_Method, 0, 2, 0, 2, &TextField::dva[0]},
        {TFunc_Instances_TextField_replaceSelectedText::Func, &TextField::tit[123], "replaceSelectedText", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TextField_replaceText::Func, &TextField::tit[125], "replaceText", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Instances_TextField_setSelection::Func, &TextField::tit[129], "setSelection", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_TextField_setTextFormat::Func, &TextField::tit[132], "setTextFormat", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 2, &TextField::dva[2]},
    };

    TextField::TextField(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"InstanceTraits::TextField::TextField()"
//##protect##"InstanceTraits::TextField::TextField()"

    }

    void TextField::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<TextField&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    TextField::TextField(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"ClassTraits::TextField::TextField()"
//##protect##"ClassTraits::TextField::TextField()"

    }

    Pickable<Traits> TextField::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextField(vm, AS3::fl_text::TextFieldCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextFieldCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_text
{
    const TypeInfo TextFieldTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_text::TextField::InstanceType),
        0,
        0,
        InstanceTraits::fl_text::TextField::ThunkInfoNum,
        0,
        "TextField", "flash.text", &fl_display::InteractiveObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextFieldCI = {
        &TextFieldTI,
        ClassTraits::fl_text::TextField::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_text::TextField::ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

