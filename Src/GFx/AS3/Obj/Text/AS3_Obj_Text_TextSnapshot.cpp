//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextSnapshot.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextSnapshot.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
class TextSnapshotGlyphVisitor : public StaticTextSnapshotData::GlyphVisitor
{
public:
    TextSnapshotGlyphVisitor(VM* vm, Instances::fl::Array* parr) : pVM(vm), pArray(parr) {}

    void OnVisit()
    {
        // Create new object
        SPtr<Instances::fl::Object> pobj = pVM->MakeObject();

        StringManager& sm = pVM->GetStringManager();
        Value val;

        // Fill object
        val.SetUInt32((unsigned)GetRunIndex());
        pobj->AddDynamicSlotValuePair(sm.CreateString("indexInRun"), val);

        val.SetBool(IsSelected());
        pobj->AddDynamicSlotValuePair(sm.CreateConstString("selected"), val);

        pobj->AddDynamicSlotValuePair(sm.CreateString("font"), sm.CreateString(GetFontName()));

        val.SetNumber((float)GetColor().ToColor32());
        pobj->AddDynamicSlotValuePair(sm.CreateConstString("color"), val);

        val.SetNumber(GetHeight());
        pobj->AddDynamicSlotValuePair(sm.CreateConstString("height"), val);

        const Render::Matrix2F& mat = GetMatrix();
        val.SetNumber(TwipsToPixels(mat.Sx()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_a"), val);
        val.SetNumber(TwipsToPixels(mat.Shy()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_b"), val);
        val.SetNumber(TwipsToPixels(mat.Shx()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_c"), val);
        val.SetNumber(TwipsToPixels(mat.Sy()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_d"), val);
        val.SetNumber(TwipsToPixels(mat.Tx()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_tx"), val);
        val.SetNumber(TwipsToPixels(mat.Ty()));
        pobj->AddDynamicSlotValuePair(sm.CreateString("matrix_ty"), val);

        const Render::PointF& bl = GetCorners().BottomLeft();
        const Render::PointF& br = GetCorners().BottomRight();
        const Render::PointF& tl = GetCorners().TopLeft();
        const Render::PointF& tr = GetCorners().TopRight();
        val.SetNumber(TwipsToPixels(bl.x));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner0x"), val);
        val.SetNumber(TwipsToPixels(bl.y));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner0y"), val);
        val.SetNumber(TwipsToPixels(br.x));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner1x"), val);
        val.SetNumber(TwipsToPixels(br.y));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner1y"), val);
        val.SetNumber(TwipsToPixels(tr.x));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner2x"), val);
        val.SetNumber(TwipsToPixels(tr.y));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner2y"), val);
        val.SetNumber(TwipsToPixels(tl.x));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner3x"), val);
        val.SetNumber(TwipsToPixels(tl.y));
        pobj->AddDynamicSlotValuePair(sm.CreateString("corner3y"), val);

        // Add to array
        pArray->PushBack(Value(pobj));
    }

    private:
        VM* pVM;
        Instances::fl::Array* pArray;
	};
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_hitTestTextNearPos, 2, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    UInt32 GetMethodDefArg<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_setSelectColor, 0, UInt32>(AS3::StringManager&)
    {
        return 0xFFFF00;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_charCountGet, SInt32> TFunc_Instances_TextSnapshot_charCountGet;
typedef ThunkFunc3<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_findText, SInt32, SInt32, const ASString&, bool> TFunc_Instances_TextSnapshot_findText;
typedef ThunkFunc2<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_getSelected, bool, SInt32, SInt32> TFunc_Instances_TextSnapshot_getSelected;
typedef ThunkFunc1<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_getSelectedText, ASString, bool> TFunc_Instances_TextSnapshot_getSelectedText;
typedef ThunkFunc3<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_getText, ASString, SInt32, SInt32, bool> TFunc_Instances_TextSnapshot_getText;
typedef ThunkFunc2<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_getTextRunInfo, SPtr<Instances::fl::Array>, SInt32, SInt32> TFunc_Instances_TextSnapshot_getTextRunInfo;
typedef ThunkFunc3<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_hitTestTextNearPos, Value::Number, Value::Number, Value::Number, Value::Number> TFunc_Instances_TextSnapshot_hitTestTextNearPos;
typedef ThunkFunc1<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_setSelectColor, const Value, UInt32> TFunc_Instances_TextSnapshot_setSelectColor;
typedef ThunkFunc3<Instances::fl_text::TextSnapshot, Instances::fl_text::TextSnapshot::mid_setSelected, const Value, SInt32, SInt32, bool> TFunc_Instances_TextSnapshot_setSelected;

template <> const TFunc_Instances_TextSnapshot_charCountGet::TMethod TFunc_Instances_TextSnapshot_charCountGet::Method = &Instances::fl_text::TextSnapshot::charCountGet;
template <> const TFunc_Instances_TextSnapshot_findText::TMethod TFunc_Instances_TextSnapshot_findText::Method = &Instances::fl_text::TextSnapshot::findText;
template <> const TFunc_Instances_TextSnapshot_getSelected::TMethod TFunc_Instances_TextSnapshot_getSelected::Method = &Instances::fl_text::TextSnapshot::getSelected;
template <> const TFunc_Instances_TextSnapshot_getSelectedText::TMethod TFunc_Instances_TextSnapshot_getSelectedText::Method = &Instances::fl_text::TextSnapshot::getSelectedText;
template <> const TFunc_Instances_TextSnapshot_getText::TMethod TFunc_Instances_TextSnapshot_getText::Method = &Instances::fl_text::TextSnapshot::getText;
template <> const TFunc_Instances_TextSnapshot_getTextRunInfo::TMethod TFunc_Instances_TextSnapshot_getTextRunInfo::Method = &Instances::fl_text::TextSnapshot::getTextRunInfo;
template <> const TFunc_Instances_TextSnapshot_hitTestTextNearPos::TMethod TFunc_Instances_TextSnapshot_hitTestTextNearPos::Method = &Instances::fl_text::TextSnapshot::hitTestTextNearPos;
template <> const TFunc_Instances_TextSnapshot_setSelectColor::TMethod TFunc_Instances_TextSnapshot_setSelectColor::Method = &Instances::fl_text::TextSnapshot::setSelectColor;
template <> const TFunc_Instances_TextSnapshot_setSelected::TMethod TFunc_Instances_TextSnapshot_setSelected::Method = &Instances::fl_text::TextSnapshot::setSelected;

namespace Instances { namespace fl_text
{
    TextSnapshot::TextSnapshot(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::TextSnapshot::TextSnapshot()$data"
//##protect##"instance::TextSnapshot::TextSnapshot()$data"
    {
//##protect##"instance::TextSnapshot::TextSnapshot()$code"
//##protect##"instance::TextSnapshot::TextSnapshot()$code"
    }

    void TextSnapshot::charCountGet(SInt32& result)
    {
//##protect##"instance::TextSnapshot::charCountGet()"
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        result = static_cast<SInt32>(GetSnapshotData().GetCharCount());
#else
        result = 0;
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::charCountGet()"
    }
    void TextSnapshot::findText(SInt32& result, SInt32 beginIndex, const ASString& textToFind, bool caseSensitive)
    {
//##protect##"instance::TextSnapshot::findText()"
        SF_UNUSED3(beginIndex, textToFind, caseSensitive);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        String query(textToFind.ToCStr(), textToFind.GetSize());
        result = GetSnapshotData().FindText(beginIndex, query, caseSensitive);
#else
        result = 0;
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::findText()"
    }
    void TextSnapshot::getSelected(bool& result, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::TextSnapshot::getSelected()"
        SF_UNUSED2(beginIndex,endIndex);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        if (endIndex <= beginIndex)
            endIndex = beginIndex + 1;
        result = GetSnapshotData().IsSelected(beginIndex, endIndex);
#else
        result = false;
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::getSelected()"
    }
    void TextSnapshot::getSelectedText(ASString& result, bool includeLineEndings)
    {
//##protect##"instance::TextSnapshot::getSelectedText()"
        SF_UNUSED2(result, includeLineEndings);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        String selectedText = GetSnapshotData().GetSelectedText(includeLineEndings);
        result = GetVM().GetStringManager().CreateString(selectedText);
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::getSelectedText()"
    }
    void TextSnapshot::getText(ASString& result, SInt32 beginIndex, SInt32 endIndex, bool includeLineEndings)
    {
//##protect##"instance::TextSnapshot::getText()"
        SF_UNUSED4(result,beginIndex,endIndex,includeLineEndings);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        if (endIndex <= beginIndex)
            endIndex = beginIndex + 1;
        String subStr = GetSnapshotData().GetSubString(beginIndex, endIndex, includeLineEndings);
        result = GetVM().GetStringManager().CreateString(subStr);
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::getText()"
    }
    void TextSnapshot::getTextRunInfo(SPtr<Instances::fl::Array>& result, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::TextSnapshot::getTextRunInfo()"
        SF_UNUSED3(result, beginIndex,endIndex);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        VM& vm = GetVM();
        SPtr<Instances::fl::Array> parr = vm.MakeArray();
        TextSnapshotGlyphVisitor visitor(&vm, parr);
        GetSnapshotData().Visit(&visitor, beginIndex, endIndex);
        result = parr;
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::getTextRunInfo()"
    }
    void TextSnapshot::hitTestTextNearPos(Value::Number& result, Value::Number x, Value::Number y, Value::Number maxDistance)
    {
//##protect##"instance::TextSnapshot::hitTestTextNearPos()"
        SF_UNUSED4(result, x, y, maxDistance);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        int idx = GetSnapshotData().HitTestTextNearPos(
            float(PixelsToTwips(x)), float(PixelsToTwips(y)), float(PixelsToTwips(maxDistance)));
        result = Value::Number(idx);
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::hitTestTextNearPos()"
    }
    void TextSnapshot::setSelectColor(const Value& result, UInt32 hexColor)
    {
//##protect##"instance::TextSnapshot::setSelectColor()"
        SF_UNUSED2(result, hexColor);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        Color color(hexColor);
        color.SetAlpha(255);
        GetSnapshotData().SetSelectColor(color);
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::setSelectColor()"
    }
    void TextSnapshot::setSelected(const Value& result, SInt32 beginIndex, SInt32 endIndex, bool select)
    {
//##protect##"instance::TextSnapshot::setSelected()"
        SF_UNUSED4(result, beginIndex, endIndex, select);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        if (endIndex <= beginIndex)
            endIndex = beginIndex + 1;
        GetSnapshotData().SetSelected(beginIndex, endIndex, select);
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance::TextSnapshot::setSelected()"
    }

    SPtr<Instances::fl::Array> TextSnapshot::getTextRunInfo(SInt32 beginIndex, SInt32 endIndex)
    {
        SPtr<Instances::fl::Array> result;
        getTextRunInfo(result, beginIndex, endIndex);
        return result;
    }
//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 TextSnapshot::tito[TextSnapshot::ThunkInfoNum] = {
    //    0, 1, 5, 8, 10, 14, 17, 21, 23, 
    // };
    const TypeInfo* TextSnapshot::tit[27] = {
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::StringTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
        &AS3::fl::ArrayTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue TextSnapshot::dva[2] = {
        {Abc::CONSTANT_Double, 0}, 
        {Abc::CONSTANT_UInt, 6}, 
    };
    const ThunkInfo TextSnapshot::ti[TextSnapshot::ThunkInfoNum] = {
        {TFunc_Instances_TextSnapshot_charCountGet::Func, &TextSnapshot::tit[0], "charCount", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_findText::Func, &TextSnapshot::tit[1], "findText", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_getSelected::Func, &TextSnapshot::tit[5], "getSelected", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_getSelectedText::Func, &TextSnapshot::tit[8], "getSelectedText", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_getText::Func, &TextSnapshot::tit[10], "getText", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_getTextRunInfo::Func, &TextSnapshot::tit[14], "getTextRunInfo", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_TextSnapshot_hitTestTextNearPos::Func, &TextSnapshot::tit[17], "hitTestTextNearPos", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 1, &TextSnapshot::dva[0]},
        {TFunc_Instances_TextSnapshot_setSelectColor::Func, &TextSnapshot::tit[21], "setSelectColor", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &TextSnapshot::dva[1]},
        {TFunc_Instances_TextSnapshot_setSelected::Func, &TextSnapshot::tit[23], "setSelected", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
    };

    TextSnapshot::TextSnapshot(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::TextSnapshot::TextSnapshot()"
//##protect##"InstanceTraits::TextSnapshot::TextSnapshot()"

    }

    void TextSnapshot::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<TextSnapshot&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    TextSnapshot::TextSnapshot(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextSnapshot::TextSnapshot()"
//##protect##"ClassTraits::TextSnapshot::TextSnapshot()"

    }

    Pickable<Traits> TextSnapshot::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextSnapshot(vm, AS3::fl_text::TextSnapshotCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextSnapshotCI));
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
    const TypeInfo TextSnapshotTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_text::TextSnapshot::InstanceType),
        0,
        0,
        InstanceTraits::fl_text::TextSnapshot::ThunkInfoNum,
        0,
        "TextSnapshot", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextSnapshotCI = {
        &TextSnapshotTI,
        ClassTraits::fl_text::TextSnapshot::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_text::TextSnapshot::ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

