/**************************************************************************

Filename    :   AS2_TextFormat.cpp
Content     :   TextFormat object functinality
Created     :   April 17, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_TextFormat.h"

#ifdef GFX_AS2_ENABLE_TEXTFORMAT
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "Render/Text/Text_DocView.h"
#include "GFx/GFx_PlayerImpl.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { 

namespace Text { using namespace Render::Text; }

namespace AS2 {

TextFormatObject::TextFormatObject(Environment* penv)
    : Object(penv), mTextFormat(penv->GetHeap())
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_TextFormat));
    SetConstMemberRaw(penv->GetSC(), "align", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "blockIndent", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "bold", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "bullet", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "color", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "font", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "indent", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "italic", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "leading", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "leftMargin", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "rightMargin", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "size", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "tabStops", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "target", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "underline", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(penv->GetSC(), "url", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    if (penv->GetVersion() >= 8)
    {
        SetConstMemberRaw(penv->GetSC(), "kerning", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
        SetConstMemberRaw(penv->GetSC(), "letterSpacing", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    }
    if (penv->CheckExtensions())
    {
        SetConstMemberRaw(penv->GetSC(), "alpha", Value(Value::NULLTYPE), PropFlags::PropFlag_DontDelete);
    }
}

bool TextFormatObject::SetMember(Environment *penv, const ASString& name, 
                                    const Value& val, const PropFlags& flags)
{
    Value valset(val);
    if (name == "align")
    {
        ASString strval = val.ToString(penv);
        if (strval == "left")
        {
            mParagraphFormat.SetAlignment(Text::ParagraphFormat::Align_Left);
        }
        else if (strval == "right")
        {
            mParagraphFormat.SetAlignment(Text::ParagraphFormat::Align_Right);
        }
        else if (strval == "center")
        {
            mParagraphFormat.SetAlignment(Text::ParagraphFormat::Align_Center);
        }
        else if (strval == "justify")
        {
            mParagraphFormat.SetAlignment(Text::ParagraphFormat::Align_Justify);
        }
        else
        {
            mParagraphFormat.ClearAlignment();
            valset.SetNull();
        }
    }
    else if (name == "blockIndent")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v < 0) v = 0;
            else if (v > 720) v = 720;
            mParagraphFormat.SetBlockIndent((unsigned)v);
        }
        else
        {
            mParagraphFormat.ClearBlockIndent();
            valset.SetNull();
        }
    }
    else if (name == "bold")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            bool v = val.ToBool(penv);
            valset.SetBool(v);
            mTextFormat.SetBold(v);
        }
        else
        {
            mTextFormat.ClearBold();
            valset.SetNull();
        }
    }
    else if (name == "bullet")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            bool v = val.ToBool(penv);
            valset.SetBool(v);
            mParagraphFormat.SetBullet(v);
        }
        else
        {
            mParagraphFormat.ClearBullet();
            valset.SetNull();
        }
    }
    else if (name == "color")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            unsigned v = (unsigned)val.ToInt32(penv);
            valset.SetNumber((Number)v);
            mTextFormat.SetColor32(v);
        }
        else
        {
            mTextFormat.ClearColor();
            valset.SetNull();
        }
    }
    else if (name == "font")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            ASString v = val.ToString(penv);
            valset.SetString(v);
            mTextFormat.SetFontList(v.ToCStr());
        }
        else
        {
            mTextFormat.ClearFontList();
            valset.SetNull();
        }
    }
    else if (name == "indent")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v < -720) v = -720;
            else if (v > 720) v = 720;
            mParagraphFormat.SetIndent((int)v);
        }
        else
        {
            mParagraphFormat.ClearIndent();
            valset.SetNull();
        }
    }
    else if (name == "italic")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            bool v = val.ToBool(penv);
            valset.SetBool(v);
            mTextFormat.SetItalic(v);
        }
        else
        {
            mTextFormat.ClearItalic();
            valset.SetNull();
        }
    }
    else if (name == "leading")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v < -720) v = -720;
            else if (v > 720) v = 720;
            mParagraphFormat.SetLeading((int)v);
        }
        else
        {
            mParagraphFormat.ClearLeading();
            valset.SetNull();
        }
    }
    else if (name == "leftMargin")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v < 0) v = 0;
            else if (v > 720) v = 720;
            mParagraphFormat.SetLeftMargin((unsigned)v);
        }
        else
        {
            mParagraphFormat.ClearLeftMargin();
            valset.SetNull();
        }
    }
    else if (name == "rightMargin")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v < 0) v = 0;
            else if (v > 720) v = 720;
            mParagraphFormat.SetRightMargin((unsigned)v);
        }
        else
        {
            mParagraphFormat.ClearRightMargin();
            valset.SetNull();
        }
    }
    else if (name == "size")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            SInt32 v = val.ToInt32(penv);
            valset.SetNumber((Number)v);
            if (v >= 0)
            {
                if (v < 128)
                    mTextFormat.SetFontSize((float)(unsigned)v);
                else
                    mTextFormat.SetFontSize(127);
            }
        }
        else
        {
            mTextFormat.ClearFontSize();
            valset.SetNull();
        }
    }
    else if (name == "tabStops")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            if (val.IsObject() && val.ToObject(penv)->GetObjectType() == Object::Object_Array)
            {
                ArrayObject* parr = static_cast<ArrayObject*>(val.ToObject(penv));
                mParagraphFormat.SetTabStopsNum((unsigned)parr->GetSize());
                for(int i = 0, n = parr->GetSize(); i < n; ++i)
                {
                    Number v = parr->GetElementPtr(i)->ToNumber(penv);
                    parr->SetElement(i, Number((unsigned)v));
                    mParagraphFormat.SetTabStopsElement((unsigned)i, (unsigned)v);
                }
                return Object::SetMember(penv, name, Value(parr), flags);
            }
        }
        else
        {
            mParagraphFormat.ClearTabStops();
            valset.SetNull();
        }
    }
    else if (name == "underline")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            bool v = val.ToBool(penv);
            valset.SetBool(v);
            mTextFormat.SetUnderline(v);
        }
        else
        {
            mTextFormat.ClearUnderline();
            valset.SetNull();
        }
    }
    else if (name == "url")
    {
        if (!val.IsNull() && !val.IsUndefined())
        {
            ASString v = val.ToString(penv);
            valset.SetString(v);
            mTextFormat.SetUrl(v.ToCStr());
        }
        else
        {
            mTextFormat.ClearUrl();
            valset.SetNull();
        }
    }
//     else if (name == "target")
//     {
//     }
    else
    {
        if (penv->GetVersion() >= 8)
        {
            if (name == "letterSpacing")
            {
                if (!val.IsNull() && !val.IsUndefined())
                {
                    SInt32 v = val.ToInt32(penv);
                    valset.SetNumber((Number)v);
                    if (v < -720) v = -720;
                    else if (v > 720) v = 720;
                    mTextFormat.SetLetterSpacing(float(v));
                }
                else
                {
                    mTextFormat.ClearLetterSpacing();
                    valset.SetNull();
                }
            }
            else if (name == "kerning")
            {
                if (!val.IsNull() && !val.IsUndefined())
                {
                    bool v = val.ToBool(penv);
                    valset.SetBool(v);
                    mTextFormat.SetKerning(v);
                }
                else
                {
                    mTextFormat.ClearKerning();
                    valset.SetNull();
                }
            }
        }
        if (penv->CheckExtensions())
        {
            if (name == "alpha")
            {
                if (!val.IsNull() && !val.IsUndefined())
                {
                    SInt32 v = val.ToInt32(penv);
                    valset.SetNumber((Number)v);
                    if (v < 0) v = 0;
                    else if (v > 100) v = 100;
                    mTextFormat.SetAlpha(UInt8(v * 255. / 100.));
                }
                else
                {
                    mTextFormat.ClearAlpha();
                    valset.SetNull();
                }
            }
        }
    }
    return Object::SetMember(penv, name, valset, flags);
}

void TextFormatObject::SetTextFormat(ASStringContext* psc, const Text::TextFormat& textFmt) 
{ 
    mTextFormat = textFmt; 
    Value nullVal(Value::NULLTYPE);
    SetConstMemberRaw(psc, "bold", (textFmt.IsBoldSet())?Value(textFmt.IsBold()):nullVal);
    SetConstMemberRaw(psc, "italic", (textFmt.IsItalicSet())?Value(textFmt.IsItalic()):nullVal);
    SetConstMemberRaw(psc, "underline", (textFmt.IsUnderlineSet())?Value(textFmt.IsUnderline()):nullVal);
    SetConstMemberRaw(psc, "size", (textFmt.IsFontSizeSet())?Value(Number(textFmt.GetFontSize())):nullVal);
    SetConstMemberRaw(psc, "font", (textFmt.IsFontListSet())?Value(psc->CreateString(textFmt.GetFontList())):nullVal);
    SetConstMemberRaw(psc, "color", (textFmt.IsColorSet())?Value(Number(textFmt.GetColor32() & 0xFFFFFFu)):nullVal);
    SetConstMemberRaw(psc, "letterSpacing", (textFmt.IsLetterSpacingSet())?Value(Number(textFmt.GetLetterSpacing())):nullVal);
    SetConstMemberRaw(psc, "kerning", (textFmt.IsKerningSet())?Value(textFmt.IsKerning()):nullVal);
    SetConstMemberRaw(psc, "url", (textFmt.IsUrlSet())?Value(psc->CreateString(textFmt.GetUrl())):nullVal);
    if (psc->pContext->CheckExtensions())
    {
        SetConstMemberRaw(psc, "alpha", (textFmt.IsColorSet())?Value(Number(textFmt.GetAlpha() * 100. / 255.)):nullVal);
    }
}

void TextFormatObject::SetParagraphFormat(ASStringContext* psc, const Text::ParagraphFormat& paraFmt) 
{ 
    mParagraphFormat = paraFmt; 
    Value nullVal(Value::NULLTYPE);
    if (paraFmt.IsAlignmentSet())
    {
        const char* newAlign = "left";
        switch(paraFmt.GetAlignment())
        {
        case Text::ParagraphFormat::Align_Center:  newAlign = "center"; break;
        case Text::ParagraphFormat::Align_Right:   newAlign = "right"; break;
        case Text::ParagraphFormat::Align_Justify: newAlign = "justify"; break;
        default: break;
        }
        SetConstMemberRaw(psc, "align", Value(psc->CreateString(newAlign)));
    }
    else
        SetConstMemberRaw(psc, "align", nullVal);

    SetConstMemberRaw(psc, "bullet", (paraFmt.IsBulletSet())?Value(paraFmt.IsBullet()):nullVal);
    SetConstMemberRaw(psc, "blockIndent", (paraFmt.IsBlockIndentSet())?Value(Number(paraFmt.GetBlockIndent())):nullVal);
    SetConstMemberRaw(psc, "indent", (paraFmt.IsIndentSet())?Value(Number(paraFmt.GetIndent())):nullVal);
    SetConstMemberRaw(psc, "leading", (paraFmt.IsLeadingSet())?Value(Number(paraFmt.GetLeading())):nullVal);
    SetConstMemberRaw(psc, "leftMargin", (paraFmt.IsLeftMarginSet())?Value(Number(paraFmt.GetLeftMargin())):nullVal);
    SetConstMemberRaw(psc, "rightMargin", (paraFmt.IsRightMarginSet())?Value(Number(paraFmt.GetRightMargin())):nullVal);
    if (paraFmt.IsTabStopsSet())
    {
        unsigned num = 0;
        const unsigned* ptabStops = paraFmt.GetTabStops(&num);
        SF_ASSERT(num > 0 && ptabStops);
        Ptr<ArrayObject> parr = *SF_HEAP_NEW(psc->GetHeap()) ArrayObject(psc);
        parr->Resize((int)num);
        for (unsigned i = 0; i < num; i++)
            parr->SetElement(i, Value((Number)ptabStops[i]));
        SetConstMemberRaw(psc, "tabStops", Value(parr));
    }
    else 
        SetConstMemberRaw(psc, "tabStops", nullVal);
}

////////////////////////////////////////////////
//
static const NameFunction GAS_TextFormatFunctionTable[] = 
{
    { "getTextExtent", TextFormatProto::GetTextExtent },
    { 0, 0 }
};

TextFormatProto::TextFormatProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<TextFormatObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_TextFormatFunctionTable);
}

void TextFormatProto::GetTextExtent(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs == 0)
        return; // expecting at least one param - string
    CHECK_THIS_PTR(fn, TextFormat);
    TextFormatObject* pfmtObj = static_cast<TextFormatObject*>(fn.ThisPtr);
    Ptr<InteractiveObject> ptarget = fn.Env->GetTarget();
    if (ptarget)
    {
        Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
        ASString str = fn.Arg(0).ToString(fn.Env);

        Ptr<Text::DocView> pdocument = 
            *SF_HEAP_NEW(fn.Env->GetHeap()) Text::DocView(fn.Env->GetMovieImpl()->GetTextAllocator(), 
                                                         ptarget->GetFontManager(), NULL);
        pdocument->GetStyledText()->SetNewLine0D(); // Flash uses '\x0D' ('\r', #13) as a new-line char
        pdocument->SetAutoSizeX();
        pdocument->SetAutoSizeY();
        if (fn.Env->GetVersion() >= 7)
        {
            if (fn.NArgs >= 2)
            {
                Number width = fn.Arg(1).ToNumber(fn.Env);
                pdocument->ClearAutoSizeX();
                pdocument->SetWordWrap();
                RectF vr(0, 0, SizeF(PixelsToTwips((float)width), 0));
                pdocument->SetViewRect(vr);
            }
        }
        pdocument->SetMultiline();

        Text::TextFormat eTextFmt(fn.Env->GetHeap());
        Text::ParagraphFormat eParagraphFmt;
        eTextFmt.InitByDefaultValues();
        eParagraphFmt.InitByDefaultValues();
        eTextFmt      = eTextFmt.Merge(pfmtObj->mTextFormat);
        eParagraphFmt = eParagraphFmt.Merge(pfmtObj->mParagraphFormat);

        pdocument->SetDefaultTextFormat(eTextFmt);
        pdocument->SetDefaultParagraphFormat(eParagraphFmt);
        pdocument->SetText(str.ToCStr());
        pdocument->Format();

        pobj->SetConstMemberRaw(fn.Env->GetSC(), "textFieldWidth", 
            Value((Number)TwipsToPixels((Double)pdocument->GetTextWidth() + GFX_TEXT_GUTTER*2)));
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "textFieldHeight", 
            Value((Number)TwipsToPixels((Double)pdocument->GetTextHeight() + GFX_TEXT_GUTTER*2)));
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "width", 
            Value((Number)TwipsToPixels((Double)pdocument->GetTextWidth())));
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "height", 
            Value((Number)TwipsToPixels((Double)pdocument->GetTextHeight())));

        // now determine the ascent and descent of used font
        Ptr<Render::Text::FontHandle> pfontHandle = *pdocument->GetFontManager()->CreateFontHandle(
            eTextFmt.GetFontList(), 
            eTextFmt.IsBold(), 
            eTextFmt.IsItalic(),
            true);
        Double ascent = 0, descent = 0;
        if (pfontHandle)
        {
            ascent  = pfontHandle->GetFont()->GetAscent();
            descent = pfontHandle->GetFont()->GetDescent();
        }
        if (ascent == 0)
            ascent = 960.f;
        if (descent == 0)
            descent = 1024.f - ascent;
        Double scale = PixelsToTwips((Double)eTextFmt.GetFontSize()) / 1024.0f; // the EM square is 1024 x 1024  
        unsigned uascent  = (unsigned)TwipsToPixels(ascent * scale);
        unsigned udescent = (unsigned)TwipsToPixels(descent * scale);
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "ascent", Value(Number(uascent)));
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "descent", Value(Number(udescent)));
        fn.Result->SetAsObject(pobj);
    }
}

////////////////////////////////////////////////
//
TextFormatCtorFunction::TextFormatCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

void TextFormatCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<TextFormatObject> ab;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextFormat && !fn.ThisPtr->IsBuiltinPrototype())
        ab = static_cast<TextFormatObject*>(fn.ThisPtr);
    else
        ab = *SF_HEAP_NEW(fn.Env->GetHeap()) TextFormatObject(fn.Env);
    
    if (fn.NArgs >= 1)
    {
        ASStringContext* psc = fn.Env->GetSC();
        ab->SetMember(fn.Env, psc->CreateConstString("font"), fn.Arg(0));
        if (fn.NArgs >= 2)
        {
            ab->SetMember(fn.Env, psc->CreateConstString("size"), fn.Arg(1));
            if (fn.NArgs >= 3)
            {
                ab->SetMember(fn.Env, psc->CreateConstString("color"), fn.Arg(2));
                if (fn.NArgs >= 4)
                {
                    ab->SetMember(fn.Env, psc->CreateConstString("bold"), fn.Arg(3));
                    if (fn.NArgs >= 5)
                    {
                        ab->SetMember(fn.Env, psc->CreateConstString("italic"), fn.Arg(4));
                        if (fn.NArgs >= 6)
                        {
                            ab->SetMember(fn.Env, psc->CreateConstString("underline"), fn.Arg(5));
                            if (fn.NArgs >= 7)
                            {
                                ab->SetMember(fn.Env, psc->CreateConstString("url"), fn.Arg(6));
                                if (fn.NArgs >= 8)
                                {
                                    ab->SetMember(fn.Env, psc->CreateConstString("target"), fn.Arg(7));
                                    if (fn.NArgs >= 9)
                                    {
                                        ab->SetMember(fn.Env, psc->CreateConstString("align"), fn.Arg(8));
                                        if (fn.NArgs >= 10)
                                        {
                                            ab->SetMember(fn.Env, psc->CreateConstString("leftMargin"), fn.Arg(9));
                                            if (fn.NArgs >= 11)
                                            {
                                                ab->SetMember(fn.Env, psc->CreateConstString("rightMargin"), fn.Arg(10));
                                                if (fn.NArgs >= 12)
                                                {
                                                    ab->SetMember(fn.Env, psc->CreateConstString("indent"), fn.Arg(11));
                                                    if (fn.NArgs >= 13)
                                                    {
                                                        ab->SetMember(fn.Env, psc->CreateConstString("leading"), fn.Arg(12));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    fn.Result->SetAsObject(ab.GetPtr());
}

Object*  TextFormatCtorFunction::CreateNewObject(Environment* penv) const
{
    return SF_HEAP_NEW(penv->GetHeap()) TextFormatObject(penv);
}

FunctionRef TextFormatCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) TextFormatCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) TextFormatProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_TextFormat, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_TextFormat), Value(ctor));
    return ctor;
}
}}} //SF::GFx::AS2

#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
