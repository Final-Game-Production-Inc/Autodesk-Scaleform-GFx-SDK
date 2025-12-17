//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_StyleSheet.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_StyleSheet.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "AS3_Obj_Text_TextFormat.h"
#include "Render/Text/Text_Core.h"
#include "../../../Text/Text_StyleSheet.h"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

// 
// Helper to collate style properties
//
class CSSStringBuilder
{
public:
    static void    Process(String& dest, AS3::Object* pobj)
    {
        GlobalSlotIndex slotIdx(0);
        while ( (slotIdx = pobj->GetNextDynPropIndex(slotIdx)).Get() )
        {
            Value name, value;
            pobj->GetNextPropertyName(name, slotIdx);
            pobj->GetNextPropertyValue(value, slotIdx);
            if (name.IsString())
                processSub(dest, name.AsString(), value);
        }
    }

protected:
    static void processSub(String& dest, const ASString& name, const Value& val)
    {
        StringBuffer buff, sval;

        // Translate from Flash prop to CSS1 prop
        if (name == "fontFamily")               buff += "font-family";
        else if (name == "fontSize")            buff += "font-size";
        else if (name == "fontStyle")           buff += "font-style";
        else if (name == "fontWeight")          buff += "font-weight";
        else if (name == "letterSpacing")       buff += "letter-spacing";
        else if (name == "marginLeft")          buff += "margin-left";
        else if (name == "marginRight")         buff += "margin-right";
        else if (name == "textAlign")           buff += "text-align";
        else if (name == "textDecoration")      buff += "text-decoration";
        else if (name == "textIndent")          buff += "text-indent";
        else                                    buff += name.ToCStr();
        buff += ":";
        
        val.Convert2String(sval).DoNotCheck();
        
        // PPS: Check for whitespace, if so we need to add explicit quotes around the value.
        bool quotedValue = false;
        for (UPInt i = 0; i < sval.GetSize(); i++)
        {
            if (isspace(sval[i]))
            {
                quotedValue = true;
                break;
            }
        }
        
        if (quotedValue) 
        {
            buff += '"';
            buff += sval;
            buff += '"';
        }
        else
            buff += sval.ToCStr();

        buff += ";";

        dest += buff.ToCStr();
    }
};

class CSSTextFormatLoader
{
public:
    static void     Process(Instances::fl_text::TextFormat* tf, AS3::Object* pobj)
    {
        GlobalSlotIndex slotIdx(0);
        while ( (slotIdx = pobj->GetNextDynPropIndex(slotIdx)).Get() )
        {
            Value name, value;
            pobj->GetNextPropertyName(name, slotIdx);
            pobj->GetNextPropertyValue(value, slotIdx);
            if (name.IsString())
                processSub(tf, name.AsString(), value);
        }
    }

protected:
    static void processSub(Instances::fl_text::TextFormat* tf, const ASString& name, const Value& val)
    {
        StringBuffer buff;
        val.Convert2String(buff).DoNotCheck();
        const char* pstr = buff.ToCStr();
        char* temp = NULL;
        UPInt sz = buff.GetSize();

        // Switch on valid props
        if      (name == "color")           
        {
            UInt32 colval = SFstrtol(pstr+1, &temp, 16);
            tf->mColor.SetUInt32(colval);
        }
        else if (name == "display")         
        {
            // Not supported currently
        }
        else if (name == "fontFamily")
        {
            tf->mFont = val;
        }
        else if (name == "fontSize")        
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mSize.SetNumber(num);
        }
        else if (name == "fontStyle")
        {
            if ( !SFstrncmp("normal", pstr, (sz < 4) ? sz : 4) ) 
                tf->mItalic.SetBool(false);
            else if ( !SFstrncmp("italic", pstr, (sz < 9) ? sz : 9) )
                tf->mItalic.SetBool(true);
        }
        else if (name == "fontWeight")          
        {
            if ( !SFstrncmp("normal", pstr, (sz < 6) ? sz : 6) ) 
                tf->mBold.SetBool(false);
            else if ( !SFstrncmp("bold", pstr, (sz < 4) ? sz : 4) )
                tf->mBold.SetBool(true);
        }
        else if (name == "kerning")
        {
            if ( !SFstrncmp("false", pstr, (sz < 5) ? sz : 5) )
                tf->mKerning.SetBool(false);
            else if ( !SFstrncmp("true", pstr, (sz < 4) ? sz : 4) ) 
                tf->mKerning.SetBool(true);
        }
        else if (name == "leading")         
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mLeading.SetNumber(num);
        }
        else if (name == "letterSpacing")       
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mLetterSpacing.SetNumber(num);
        }
        else if (name == "marginLeft")          
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mLeftMargin.SetNumber(num);
        }
        else if (name == "marginRight")         
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mRightMargin.SetNumber(num);
        }
        else if (name == "textAlign")           
        {
            tf->mAlign = val;
        }
        else if (name == "textDecoration")      
        {
            if ( !SFstrncmp("none", pstr, (sz < 4) ? sz : 4) ) 
                tf->mUnderline.SetBool(false);
            else if ( !SFstrncmp("underline", pstr, (sz < 9) ? sz : 9) )
                tf->mUnderline.SetBool(true);
        }
        else if (name == "textIndent")          
        {
            float num = (float)SFstrtod(pstr, &temp);
            tf->mIndent.SetNumber(num);
        }
    }
};

namespace Classes
{
    class TextFormat;
}
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_styleNamesGet, SPtr<Instances::fl::Array> > TFunc_Instances_StyleSheet_styleNamesGet;
typedef ThunkFunc0<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_clear, const Value> TFunc_Instances_StyleSheet_clear;
typedef ThunkFunc1<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_getStyle, SPtr<Instances::fl::Object>, const ASString&> TFunc_Instances_StyleSheet_getStyle;
typedef ThunkFunc1<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_parseCSS, const Value, const ASString&> TFunc_Instances_StyleSheet_parseCSS;
typedef ThunkFunc2<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_setStyle, const Value, const ASString&, const Value&> TFunc_Instances_StyleSheet_setStyle;
typedef ThunkFunc1<Instances::fl_text::StyleSheet, Instances::fl_text::StyleSheet::mid_transform, SPtr<Instances::fl_text::TextFormat>, const Value&> TFunc_Instances_StyleSheet_transform;

template <> const TFunc_Instances_StyleSheet_styleNamesGet::TMethod TFunc_Instances_StyleSheet_styleNamesGet::Method = &Instances::fl_text::StyleSheet::styleNamesGet;
template <> const TFunc_Instances_StyleSheet_clear::TMethod TFunc_Instances_StyleSheet_clear::Method = &Instances::fl_text::StyleSheet::clear;
template <> const TFunc_Instances_StyleSheet_getStyle::TMethod TFunc_Instances_StyleSheet_getStyle::Method = &Instances::fl_text::StyleSheet::getStyle;
template <> const TFunc_Instances_StyleSheet_parseCSS::TMethod TFunc_Instances_StyleSheet_parseCSS::Method = &Instances::fl_text::StyleSheet::parseCSS;
template <> const TFunc_Instances_StyleSheet_setStyle::TMethod TFunc_Instances_StyleSheet_setStyle::Method = &Instances::fl_text::StyleSheet::setStyle;
template <> const TFunc_Instances_StyleSheet_transform::TMethod TFunc_Instances_StyleSheet_transform::Method = &Instances::fl_text::StyleSheet::transform;

namespace Instances { namespace fl_text
{
    StyleSheet::StyleSheet(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::StyleSheet::StyleSheet()$data"
//##protect##"instance::StyleSheet::StyleSheet()$data"
    {
//##protect##"instance::StyleSheet::StyleSheet()$code"
//##protect##"instance::StyleSheet::StyleSheet()$code"
    }

    void StyleSheet::styleNamesGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::StyleSheet::styleNamesGet()"

#ifdef GFX_ENABLE_CSS
        // An array that contains the names (as strings) of all of the 
        // styles registered in this style sheet. 

        Pickable<Instances::fl::Array> array = GetVM().MakeArray();
        StringManager& smgr = GetStringManager();
        String temp;
        for (Text::StyleHash::ConstIterator iter = CSS.GetStyles().Begin(); 
            iter != CSS.GetStyles().End(); ++iter)
        {
            temp.Clear();
            if (iter->First.Type == Text::StyleManager::CSS_Class)
                temp.AppendChar('.');
            temp += iter->First.Value;
            array->PushBack( smgr.CreateString(temp) );
        }
        result = array;

#else

        SF_UNUSED1(result);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::styleNamesGet()");

#endif

//##protect##"instance::StyleSheet::styleNamesGet()"
    }
    void StyleSheet::clear(const Value& result)
    {
//##protect##"instance::StyleSheet::clear()"

#ifdef GFX_ENABLE_CSS

        // Removes all styles from the style sheet object. 

        SF_UNUSED1(result);
        CSS.ClearStyles();

#else

        SF_UNUSED1(result);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::clear()");

#endif

//##protect##"instance::StyleSheet::clear()"
    }
    void StyleSheet::getStyle(SPtr<Instances::fl::Object>& result, const ASString& styleName)
    {
//##protect##"instance::StyleSheet::getStyle()"

#ifdef GFX_ENABLE_CSS

        // Returns a copy of the style object associated with the style named 
        // styleName. If there is no style object associated with styleName, 
        // null is returned. 

        const Text::Style* pstyle = NULL;
        if (styleName.GetSize() > 0 && styleName[0] == '.')
            pstyle = CSS.GetStyle(Text::StyleManager::CSS_Class, styleName.ToCStr()+1, styleName.GetSize()-1);
        else
            pstyle = CSS.GetStyle(Text::StyleManager::CSS_Tag, styleName.ToCStr(), styleName.GetSize());
        if (!pstyle)
        {
            result = NULL;
            return;
        }

        Pickable<Instances::fl::Object> pobj = GetVM().MakeObject();

        // Fill in all set properties
        const Render::Text::TextFormat& tf = pstyle->mTextFormat;
        const Render::Text::ParagraphFormat& pf = pstyle->mParagraphFormat;

        StringManager& sm = GetVM().GetStringManager();
        Instances::fl::Namespace& publicNS = GetVM().GetPublicNamespace();

        // color
        if (tf.IsColorSet())
        {
            String colorStr;
            // create html color string
            colorStr.AppendChar('#');
            Color color = tf.GetColor();
            const char* refstr = "0123456789ABCDEF";
            UByte comp = color.GetRed();
            colorStr.AppendChar(refstr[comp >> 4]);
            colorStr.AppendChar(refstr[comp & 0x0F]);
            comp = color.GetGreen();
            colorStr.AppendChar(refstr[comp >> 4]);
            colorStr.AppendChar(refstr[comp & 0x0F]);
            comp = color.GetBlue();
            colorStr.AppendChar(refstr[comp >> 4]);
            colorStr.AppendChar(refstr[comp & 0x0F]);

            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("color")), 
                Value(sm.CreateString(colorStr))).DoNotCheck();
        }

        // display
        // TODO

        // fontFamily
        if (tf.IsFontListSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("fontFamily")), 
                Value(sm.CreateString(tf.GetFontList()))).DoNotCheck();
        }

        // fontSize
        if (tf.IsFontSizeSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("fontSize")), 
                Value(tf.GetFontSize())).DoNotCheck();
        }

        // fontStyle
        if (tf.IsItalicSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("fontStyle")), 
                Value(sm.CreateString(tf.IsItalic() ? "italic" : "normal"))).DoNotCheck();
        }

        // fontWeight
        if (tf.IsBoldSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("fontWeight")), 
                Value(sm.CreateString(tf.IsBold() ? "bold" : "normal"))).DoNotCheck();
        }

        // kerning
        if (tf.IsKerningSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("kerning")), 
                Value(sm.CreateString(tf.IsKerning() ? "true" : "false"))).DoNotCheck();
        }

        // leading
        if (pf.IsLeadingSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("leading")), 
                Value((SInt32)pf.GetLeading())).DoNotCheck();
        }

        // letterSpacing
        if (tf.IsLetterSpacingSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("letterSpacing")), 
                Value(tf.GetLetterSpacing())).DoNotCheck();
        }

        // marginLeft
        if (pf.IsLeftMarginSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("marginLeft")), 
                Value((UInt32)pf.GetLeftMargin())).DoNotCheck();
        }

        // marginRight
        if (pf.IsRightMarginSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("marginRight")), 
                Value((UInt32)pf.GetRightMargin())).DoNotCheck();
        }

        // textAlign
        if (pf.IsAlignmentSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("textAlign")), 
                Value(sm.CreateString( 
                    pf.IsLeftAlignment() ? "left" : 
                        pf.IsCenterAlignment() ? "center" : 
                            pf.IsRightAlignment() ? "right" : 
                                "justify"
                ))).DoNotCheck();
        }

        // textDecoration
        if (tf.IsUnderlineSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("textDecoration")), 
                Value(sm.CreateString(tf.IsUnderline() ? "underline" : "none"))).DoNotCheck();
        }

        // textIndent
        if (pf.IsIndentSet())
        {
            pobj->SetProperty(Multiname(
                publicNS, sm.CreateString("textIndent")), 
                Value((SInt32)pf.GetIndent())).DoNotCheck();
        }

        result = pobj;

#else

        SF_UNUSED2(result, styleName);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::getStyle()");

#endif

//##protect##"instance::StyleSheet::getStyle()"
    }
    void StyleSheet::parseCSS(const Value& result, const ASString& CSSText)
    {
//##protect##"instance::StyleSheet::parseCSS()"

#ifdef GFX_ENABLE_CSS

        // Parses the CSS in CSSText and loads the style sheet with it. If a 
        // style in CSSText is already in styleSheet, the properties in styleSheet 
        // are retained, and only the ones in CSSText are added or changed in 
        // styleSheet. 

        SF_UNUSED1(result);
        if (CSS.ParseCSS( CSSText.ToCStr(), CSSText.GetSize()))
        {
            CSS.SetState(Text::StyleManager::LoadingFinished);

            // we need to notify all textfields about new stylesheet is ready.
            // Need to enforce TextField::AdvanceFrame to be called. Since we
            // don't know which textfields use this particular stylesheet we need
            // to force whole playlist rebuild.
            static_cast<ASVM&>(GetVM()).GetMovieImpl()->InvalidateOptAdvanceList();
        }
        else
            CSS.SetState(Text::StyleManager::Error);

#else
        SF_UNUSED2(result, CSSText);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::parseCSS()");
#endif

//##protect##"instance::StyleSheet::parseCSS()"
    }
    void StyleSheet::setStyle(const Value& result, const ASString& styleName, const Value& styleObject)
    {
//##protect##"instance::StyleSheet::setStyle()"

#ifdef GFX_ENABLE_CSS

        // Adds a new style with the specified name to the style sheet object. If 
        // the named style does not already exist in the style sheet, it is added. 
        // If the named style already exists in the style sheet, it is replaced. 
        // If the styleObject parameter is null, the named style is removed. 

        SF_UNUSED1(result);

        if (styleName.IsEmpty()) return;
        if (styleObject.IsNull())
        {
            // remove style
            if (styleName.GetSize() > 0)
            {
                if (styleName[0] == '.')
                    CSS.ClearStyle(Text::StyleManager::CSS_Class, styleName.ToCStr());
                else
                    CSS.ClearStyle(Text::StyleManager::CSS_Tag, styleName.ToCStr());
            }
            return;
        }
        else
        {
            // replace style
            if (!styleObject.IsObject())
            {
                SF_DEBUG_WARNING(1, "Style is not an Object\n");
                return;
            }
            AS3::Object* pobj = styleObject.GetObject();
            // build a css string with given info
            String cssstr;
            cssstr.AppendString(styleName.ToCStr());
            cssstr.AppendChar('{');
            CSSStringBuilder::Process(cssstr, pobj);
            cssstr.AppendChar('}');
            // parse it using the textstylemanager
            CSS.ParseCSS(cssstr.ToCStr(), cssstr.GetSize());
        }

#else

        SF_UNUSED3(result, styleName, styleObject);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::setStyle()");

#endif

//##protect##"instance::StyleSheet::setStyle()"
    }
    void StyleSheet::transform(SPtr<Instances::fl_text::TextFormat>& result, const Value& formatObject)
    {
//##protect##"instance::StyleSheet::transform()"

#ifdef GFX_ENABLE_CSS

        // Extends the CSS parsing capability. Advanced developers can override this 
        // method by extending the StyleSheet class. 

        if (!formatObject.IsObject() || !formatObject.GetObject())
        {
            SF_DEBUG_WARNING(1, "Format is not an Object\n");
            return;
        }
        AS3::Object* pobj = formatObject.GetObject();        

        SPtr<TextFormat> textFormat;
        bool ok = GetVM().ConstructBuiltinObject(textFormat, "flash.text.TextFormat");
        SF_ASSERT(ok);  // PPS: Why was the class not found?
        SF_UNUSED1(ok);

        CSSTextFormatLoader::Process(textFormat, pobj);

        result = textFormat;

#else

        SF_UNUSED2(result, formatObject);
        WARN_NOT_SUPPORTED_MISSING_DEPENDENCY("StyleSheet::transform()");

#endif

//##protect##"instance::StyleSheet::transform()"
    }

    SPtr<Instances::fl::Array> StyleSheet::styleNamesGet()
    {
        SPtr<Instances::fl::Array> result;
        styleNamesGet(result);
        return result;
    }
    SPtr<Instances::fl_text::TextFormat> StyleSheet::transform(const Value& formatObject)
    {
        SPtr<Instances::fl_text::TextFormat> result;
        transform(result, formatObject);
        return result;
    }
//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 StyleSheet::tito[StyleSheet::ThunkInfoNum] = {
    //    0, 1, 2, 4, 6, 9, 
    // };
    const TypeInfo* StyleSheet::tit[11] = {
        &AS3::fl::ArrayTI, 
        NULL, 
        &AS3::fl::ObjectTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::ObjectTI, 
        &AS3::fl_text::TextFormatTI, &AS3::fl::ObjectTI, 
    };
    const ThunkInfo StyleSheet::ti[StyleSheet::ThunkInfoNum] = {
        {TFunc_Instances_StyleSheet_styleNamesGet::Func, &StyleSheet::tit[0], "styleNames", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StyleSheet_clear::Func, &StyleSheet::tit[1], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StyleSheet_getStyle::Func, &StyleSheet::tit[2], "getStyle", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_StyleSheet_parseCSS::Func, &StyleSheet::tit[4], "parseCSS", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_StyleSheet_setStyle::Func, &StyleSheet::tit[6], "setStyle", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_StyleSheet_transform::Func, &StyleSheet::tit[9], "transform", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    StyleSheet::StyleSheet(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::StyleSheet::StyleSheet()"
//##protect##"InstanceTraits::StyleSheet::StyleSheet()"

    }

    void StyleSheet::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<StyleSheet&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    StyleSheet::StyleSheet(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::StyleSheet::StyleSheet()"
//##protect##"ClassTraits::StyleSheet::StyleSheet()"

    }

    Pickable<Traits> StyleSheet::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StyleSheet(vm, AS3::fl_text::StyleSheetCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::StyleSheetCI));
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
    const TypeInfo StyleSheetTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl_text::StyleSheet::InstanceType),
        0,
        0,
        InstanceTraits::fl_text::StyleSheet::ThunkInfoNum,
        0,
        "StyleSheet", "flash.text", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo StyleSheetCI = {
        &StyleSheetTI,
        ClassTraits::fl_text::StyleSheet::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_text::StyleSheet::ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

