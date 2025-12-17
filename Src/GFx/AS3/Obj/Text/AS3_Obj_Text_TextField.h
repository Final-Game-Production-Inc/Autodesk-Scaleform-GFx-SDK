//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextField.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextField_H
#define INC_AS3_Obj_Text_TextField_H

#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"
#include "GFx/GFx_TextField.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextFieldTI;
    extern const ClassInfo TextFieldCI;
    extern const TypeInfo TextFormatTI;
    extern const ClassInfo TextFormatCI;
    extern const TypeInfo StyleSheetTI;
    extern const ClassInfo StyleSheetCI;
    extern const TypeInfo TextLineMetricsTI;
    extern const ClassInfo TextLineMetricsCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_text
{
    class TextField;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextField;
}}

namespace Classes { namespace fl_text
{
    class TextField;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_text
    {
        class TextFormat;
        class StyleSheet;
        class TextLineMetrics;
    }
    namespace fl_display
    {
        class NativeMenu;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class TextField : public Instances::fl_display::InteractiveObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::TextField;
        
    public:
        typedef TextField SelfType;
        typedef Classes::fl_text::TextField ClassType;
        typedef InstanceTraits::fl_text::TextField TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextFieldTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::TextField"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextField(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void                InitInstance(bool extCall = false);
        virtual GFx::DisplayObject* CreateStageObject();
        void                        UpdateAutosizeSettings();
	public:
        GFx::TextField* GetTextField() const 
        { 
            SF_ASSERT(pDispObj && pDispObj->GetType() == CharacterDef::TextField);
            return static_cast<GFx::TextField*>(pDispObj.GetPtr()); 
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alwaysShowSelectionGet, 
            mid_alwaysShowSelectionSet, 
            mid_antiAliasTypeGet, 
            mid_antiAliasTypeSet, 
            mid_autoSizeGet, 
            mid_autoSizeSet, 
            mid_backgroundGet, 
            mid_backgroundSet, 
            mid_backgroundColorGet, 
            mid_backgroundColorSet, 
            mid_borderGet, 
            mid_borderSet, 
            mid_borderColorGet, 
            mid_borderColorSet, 
            mid_bottomScrollVGet, 
            mid_caretIndexGet, 
            mid_condenseWhiteGet, 
            mid_condenseWhiteSet, 
            mid_contextMenuGet, 
            mid_defaultTextFormatGet, 
            mid_defaultTextFormatSet, 
            mid_displayAsPasswordGet, 
            mid_displayAsPasswordSet, 
            mid_embedFontsGet, 
            mid_embedFontsSet, 
            mid_gridFitTypeGet, 
            mid_gridFitTypeSet, 
            mid_htmlTextGet, 
            mid_htmlTextSet, 
            mid_lengthGet, 
            mid_maxCharsGet, 
            mid_maxCharsSet, 
            mid_maxScrollHGet, 
            mid_maxScrollVGet, 
            mid_mouseWheelEnabledGet, 
            mid_mouseWheelEnabledSet, 
            mid_multilineGet, 
            mid_multilineSet, 
            mid_numLinesGet, 
            mid_restrictGet, 
            mid_restrictSet, 
            mid_scrollHGet, 
            mid_scrollHSet, 
            mid_scrollVGet, 
            mid_scrollVSet, 
            mid_selectableGet, 
            mid_selectableSet, 
            mid_selectionBeginIndexGet, 
            mid_selectionEndIndexGet, 
            mid_sharpnessGet, 
            mid_sharpnessSet, 
            mid_styleSheetGet, 
            mid_styleSheetSet, 
            mid_textGet, 
            mid_textSet, 
            mid_textColorGet, 
            mid_textColorSet, 
            mid_textHeightGet, 
            mid_textWidthGet, 
            mid_thicknessGet, 
            mid_thicknessSet, 
            mid_typeGet, 
            mid_typeSet, 
            mid_useRichTextClipboardGet, 
            mid_useRichTextClipboardSet, 
            mid_wordWrapGet, 
            mid_wordWrapSet, 
            mid_appendText, 
            mid_getCharBoundaries, 
            mid_getCharIndexAtPoint, 
            mid_getFirstCharInParagraph, 
            mid_getImageReference, 
            mid_getLineIndexAtPoint, 
            mid_getLineIndexOfChar, 
            mid_getLineLength, 
            mid_getLineMetrics, 
            mid_getLineOffset, 
            mid_getLineText, 
            mid_getParagraphLength, 
            mid_getTextFormat, 
            mid_replaceSelectedText, 
            mid_replaceText, 
            mid_setSelection, 
            mid_setTextFormat, 
        };
        void alwaysShowSelectionGet(bool& result);
        void alwaysShowSelectionSet(const Value& result, bool value);
        void antiAliasTypeGet(ASString& result);
        void antiAliasTypeSet(const Value& result, const ASString& value);
        void autoSizeGet(ASString& result);
        void autoSizeSet(const Value& result, const ASString& value);
        void backgroundGet(bool& result);
        void backgroundSet(const Value& result, bool value);
        void backgroundColorGet(UInt32& result);
        void backgroundColorSet(const Value& result, UInt32 value);
        void borderGet(bool& result);
        void borderSet(const Value& result, bool value);
        void borderColorGet(UInt32& result);
        void borderColorSet(const Value& result, UInt32 value);
        void bottomScrollVGet(SInt32& result);
        void caretIndexGet(SInt32& result);
        void condenseWhiteGet(bool& result);
        void condenseWhiteSet(const Value& result, bool value);
        void contextMenuGet(SPtr<Instances::fl_display::NativeMenu>& result);
        void defaultTextFormatGet(SPtr<Instances::fl_text::TextFormat>& result);
        void defaultTextFormatSet(const Value& result, Instances::fl_text::TextFormat* value);
        void displayAsPasswordGet(bool& result);
        void displayAsPasswordSet(const Value& result, bool value);
        void embedFontsGet(bool& result);
        void embedFontsSet(const Value& result, bool value);
        void gridFitTypeGet(ASString& result);
        void gridFitTypeSet(const Value& result, const ASString& value);
        void htmlTextGet(ASString& result);
        void htmlTextSet(const Value& result, const ASString& value);
        void lengthGet(SInt32& result);
        void maxCharsGet(SInt32& result);
        void maxCharsSet(const Value& result, SInt32 value);
        void maxScrollHGet(SInt32& result);
        void maxScrollVGet(SInt32& result);
        void mouseWheelEnabledGet(bool& result);
        void mouseWheelEnabledSet(const Value& result, bool value);
        void multilineGet(bool& result);
        void multilineSet(const Value& result, bool value);
        void numLinesGet(SInt32& result);
        void restrictGet(ASString& result);
        void restrictSet(const Value& result, const ASString& value);
        void scrollHGet(SInt32& result);
        void scrollHSet(const Value& result, SInt32 value);
        void scrollVGet(SInt32& result);
        void scrollVSet(const Value& result, SInt32 value);
        void selectableGet(bool& result);
        void selectableSet(const Value& result, bool value);
        void selectionBeginIndexGet(SInt32& result);
        void selectionEndIndexGet(SInt32& result);
        void sharpnessGet(Value::Number& result);
        void sharpnessSet(const Value& result, Value::Number value);
        void styleSheetGet(SPtr<Instances::fl_text::StyleSheet>& result);
        void styleSheetSet(const Value& result, Instances::fl_text::StyleSheet* value);
        void textGet(ASString& result);
        void textSet(const Value& result, const ASString& value);
        void textColorGet(UInt32& result);
        void textColorSet(const Value& result, UInt32 value);
        void textHeightGet(Value::Number& result);
        void textWidthGet(Value::Number& result);
        void thicknessGet(Value::Number& result);
        void thicknessSet(const Value& result, Value::Number value);
        void typeGet(ASString& result);
        void typeSet(const Value& result, const ASString& value);
        void useRichTextClipboardGet(bool& result);
        void useRichTextClipboardSet(const Value& result, bool value);
        void wordWrapGet(bool& result);
        void wordWrapSet(const Value& result, bool value);
        void appendText(const Value& result, const ASString& newText);
        void getCharBoundaries(SPtr<Instances::fl_geom::Rectangle>& result, SInt32 charIndex);
        void getCharIndexAtPoint(SInt32& result, Value::Number x, Value::Number y);
        void getFirstCharInParagraph(SInt32& result, SInt32 charIndex);
        void getImageReference(SPtr<Instances::fl_display::DisplayObject>& result, const ASString& id);
        void getLineIndexAtPoint(SInt32& result, Value::Number x, Value::Number y);
        void getLineIndexOfChar(SInt32& result, SInt32 charIndex);
        void getLineLength(SInt32& result, SInt32 lineIndex);
        void getLineMetrics(SPtr<Instances::fl_text::TextLineMetrics>& result, SInt32 lineIndex);
        void getLineOffset(SInt32& result, SInt32 lineIndex);
        void getLineText(ASString& result, SInt32 lineIndex);
        void getParagraphLength(SInt32& result, SInt32 charIndex);
        void getTextFormat(SPtr<Instances::fl_text::TextFormat>& result, SInt32 beginIndex = -1, SInt32 endIndex = -1);
        void replaceSelectedText(const Value& result, const ASString& value);
        void replaceText(const Value& result, SInt32 beginIndex, SInt32 endIndex, const ASString& newText);
        void setSelection(const Value& result, SInt32 beginIndex, SInt32 endIndex);
        void setTextFormat(const Value& result, Instances::fl_text::TextFormat* format, SInt32 beginIndex = -1, SInt32 endIndex = -1);

        // C++ friendly wrappers for AS3 methods.
        bool alwaysShowSelectionGet()
        {
            bool result;
            alwaysShowSelectionGet(result);
            return result;
        }
        void alwaysShowSelectionSet(bool value)
        {
            alwaysShowSelectionSet(Value::GetUndefined(), value);
        }
        ASString antiAliasTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            antiAliasTypeGet(result);
            return result;
        }
        void antiAliasTypeSet(const ASString& value)
        {
            antiAliasTypeSet(Value::GetUndefined(), value);
        }
        ASString autoSizeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            autoSizeGet(result);
            return result;
        }
        void autoSizeSet(const ASString& value)
        {
            autoSizeSet(Value::GetUndefined(), value);
        }
        bool backgroundGet()
        {
            bool result;
            backgroundGet(result);
            return result;
        }
        void backgroundSet(bool value)
        {
            backgroundSet(Value::GetUndefined(), value);
        }
        UInt32 backgroundColorGet()
        {
            UInt32 result;
            backgroundColorGet(result);
            return result;
        }
        void backgroundColorSet(UInt32 value)
        {
            backgroundColorSet(Value::GetUndefined(), value);
        }
        bool borderGet()
        {
            bool result;
            borderGet(result);
            return result;
        }
        void borderSet(bool value)
        {
            borderSet(Value::GetUndefined(), value);
        }
        UInt32 borderColorGet()
        {
            UInt32 result;
            borderColorGet(result);
            return result;
        }
        void borderColorSet(UInt32 value)
        {
            borderColorSet(Value::GetUndefined(), value);
        }
        SInt32 bottomScrollVGet()
        {
            SInt32 result;
            bottomScrollVGet(result);
            return result;
        }
        SInt32 caretIndexGet()
        {
            SInt32 result;
            caretIndexGet(result);
            return result;
        }
        bool condenseWhiteGet()
        {
            bool result;
            condenseWhiteGet(result);
            return result;
        }
        void condenseWhiteSet(bool value)
        {
            condenseWhiteSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_text::TextFormat> defaultTextFormatGet();
        void defaultTextFormatSet(Instances::fl_text::TextFormat* value)
        {
            defaultTextFormatSet(Value::GetUndefined(), value);
        }
        bool displayAsPasswordGet()
        {
            bool result;
            displayAsPasswordGet(result);
            return result;
        }
        void displayAsPasswordSet(bool value)
        {
            displayAsPasswordSet(Value::GetUndefined(), value);
        }
        bool embedFontsGet()
        {
            bool result;
            embedFontsGet(result);
            return result;
        }
        void embedFontsSet(bool value)
        {
            embedFontsSet(Value::GetUndefined(), value);
        }
        ASString gridFitTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            gridFitTypeGet(result);
            return result;
        }
        void gridFitTypeSet(const ASString& value)
        {
            gridFitTypeSet(Value::GetUndefined(), value);
        }
        ASString htmlTextGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            htmlTextGet(result);
            return result;
        }
        void htmlTextSet(const ASString& value)
        {
            htmlTextSet(Value::GetUndefined(), value);
        }
        SInt32 lengthGet()
        {
            SInt32 result;
            lengthGet(result);
            return result;
        }
        SInt32 maxCharsGet()
        {
            SInt32 result;
            maxCharsGet(result);
            return result;
        }
        void maxCharsSet(SInt32 value)
        {
            maxCharsSet(Value::GetUndefined(), value);
        }
        SInt32 maxScrollHGet()
        {
            SInt32 result;
            maxScrollHGet(result);
            return result;
        }
        SInt32 maxScrollVGet()
        {
            SInt32 result;
            maxScrollVGet(result);
            return result;
        }
        bool mouseWheelEnabledGet()
        {
            bool result;
            mouseWheelEnabledGet(result);
            return result;
        }
        void mouseWheelEnabledSet(bool value)
        {
            mouseWheelEnabledSet(Value::GetUndefined(), value);
        }
        bool multilineGet()
        {
            bool result;
            multilineGet(result);
            return result;
        }
        void multilineSet(bool value)
        {
            multilineSet(Value::GetUndefined(), value);
        }
        SInt32 numLinesGet()
        {
            SInt32 result;
            numLinesGet(result);
            return result;
        }
        ASString restrictGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            restrictGet(result);
            return result;
        }
        void restrictSet(const ASString& value)
        {
            restrictSet(Value::GetUndefined(), value);
        }
        SInt32 scrollHGet()
        {
            SInt32 result;
            scrollHGet(result);
            return result;
        }
        void scrollHSet(SInt32 value)
        {
            scrollHSet(Value::GetUndefined(), value);
        }
        SInt32 scrollVGet()
        {
            SInt32 result;
            scrollVGet(result);
            return result;
        }
        void scrollVSet(SInt32 value)
        {
            scrollVSet(Value::GetUndefined(), value);
        }
        bool selectableGet()
        {
            bool result;
            selectableGet(result);
            return result;
        }
        void selectableSet(bool value)
        {
            selectableSet(Value::GetUndefined(), value);
        }
        SInt32 selectionBeginIndexGet()
        {
            SInt32 result;
            selectionBeginIndexGet(result);
            return result;
        }
        SInt32 selectionEndIndexGet()
        {
            SInt32 result;
            selectionEndIndexGet(result);
            return result;
        }
        Value::Number sharpnessGet()
        {
            Value::Number result;
            sharpnessGet(result);
            return result;
        }
        void sharpnessSet(Value::Number value)
        {
            sharpnessSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_text::StyleSheet> styleSheetGet();
        void styleSheetSet(Instances::fl_text::StyleSheet* value)
        {
            styleSheetSet(Value::GetUndefined(), value);
        }
        ASString textGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            textGet(result);
            return result;
        }
        void textSet(const ASString& value)
        {
            textSet(Value::GetUndefined(), value);
        }
        UInt32 textColorGet()
        {
            UInt32 result;
            textColorGet(result);
            return result;
        }
        void textColorSet(UInt32 value)
        {
            textColorSet(Value::GetUndefined(), value);
        }
        Value::Number textHeightGet()
        {
            Value::Number result;
            textHeightGet(result);
            return result;
        }
        Value::Number textWidthGet()
        {
            Value::Number result;
            textWidthGet(result);
            return result;
        }
        Value::Number thicknessGet()
        {
            Value::Number result;
            thicknessGet(result);
            return result;
        }
        void thicknessSet(Value::Number value)
        {
            thicknessSet(Value::GetUndefined(), value);
        }
        ASString typeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            typeGet(result);
            return result;
        }
        void typeSet(const ASString& value)
        {
            typeSet(Value::GetUndefined(), value);
        }
        bool useRichTextClipboardGet()
        {
            bool result;
            useRichTextClipboardGet(result);
            return result;
        }
        void useRichTextClipboardSet(bool value)
        {
            useRichTextClipboardSet(Value::GetUndefined(), value);
        }
        bool wordWrapGet()
        {
            bool result;
            wordWrapGet(result);
            return result;
        }
        void wordWrapSet(bool value)
        {
            wordWrapSet(Value::GetUndefined(), value);
        }
        void appendText(const ASString& newText)
        {
            appendText(Value::GetUndefined(), newText);
        }
        SPtr<Instances::fl_geom::Rectangle> getCharBoundaries(SInt32 charIndex);
        SInt32 getCharIndexAtPoint(Value::Number x, Value::Number y)
        {
            SInt32 result;
            getCharIndexAtPoint(result, x, y);
            return result;
        }
        SInt32 getFirstCharInParagraph(SInt32 charIndex)
        {
            SInt32 result;
            getFirstCharInParagraph(result, charIndex);
            return result;
        }
        SPtr<Instances::fl_display::DisplayObject> getImageReference(const ASString& id);
        SInt32 getLineIndexAtPoint(Value::Number x, Value::Number y)
        {
            SInt32 result;
            getLineIndexAtPoint(result, x, y);
            return result;
        }
        SInt32 getLineIndexOfChar(SInt32 charIndex)
        {
            SInt32 result;
            getLineIndexOfChar(result, charIndex);
            return result;
        }
        SInt32 getLineLength(SInt32 lineIndex)
        {
            SInt32 result;
            getLineLength(result, lineIndex);
            return result;
        }
        SPtr<Instances::fl_text::TextLineMetrics> getLineMetrics(SInt32 lineIndex);
        SInt32 getLineOffset(SInt32 lineIndex)
        {
            SInt32 result;
            getLineOffset(result, lineIndex);
            return result;
        }
        ASString getLineText(SInt32 lineIndex)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getLineText(result, lineIndex);
            return result;
        }
        SInt32 getParagraphLength(SInt32 charIndex)
        {
            SInt32 result;
            getParagraphLength(result, charIndex);
            return result;
        }
        SPtr<Instances::fl_text::TextFormat> getTextFormat(SInt32 beginIndex = -1, SInt32 endIndex = -1);
        void replaceSelectedText(const ASString& value)
        {
            replaceSelectedText(Value::GetUndefined(), value);
        }
        void replaceText(SInt32 beginIndex, SInt32 endIndex, const ASString& newText)
        {
            replaceText(Value::GetUndefined(), beginIndex, endIndex, newText);
        }
        void setSelection(SInt32 beginIndex, SInt32 endIndex)
        {
            setSelection(Value::GetUndefined(), beginIndex, endIndex);
        }
        void setTextFormat(Instances::fl_text::TextFormat* format, SInt32 beginIndex = -1, SInt32 endIndex = -1)
        {
            setTextFormat(Value::GetUndefined(), format, beginIndex, endIndex);
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class TextField : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextField"; }
#endif
    public:
        typedef Instances::fl_text::TextField InstanceType;

    public:
        TextField(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::TextField> MakeInstance(TextField& t)
        {
            return Pickable<Instances::fl_text::TextField>(new(t.Alloc()) Instances::fl_text::TextField(t));
        }
        SPtr<Instances::fl_text::TextField> MakeInstanceS(TextField& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 83 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[136];
        static const Abc::ConstValue dva[4];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class TextField : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextField"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::TextField InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextField(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

