//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_TextFieldEx.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_TextFieldEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "../Display/AS3_Obj_Display_BitmapData.h"
#include "../AS3_Obj_Array.h"
#include "GFx/AS3/AS3_AvmTextField.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    TextFieldEx::TextFieldEx(ClassTraits::Traits& t)
    : Class(t)
    , VALIGN_NONE("none")
    , VALIGN_TOP("top")
    , VALIGN_CENTER("center")
    , VALIGN_BOTTOM("bottom")
    , TEXTAUTOSZ_NONE("none")
    , TEXTAUTOSZ_SHRINK("shrink")
    , TEXTAUTOSZ_FIT("fit")
    , VAUTOSIZE_NONE("none")
    , VAUTOSIZE_TOP("top")
    , VAUTOSIZE_CENTER("center")
    , VAUTOSIZE_BOTTOM("bottom")
    {
//##protect##"class_::TextFieldEx::TextFieldEx()"
//##protect##"class_::TextFieldEx::TextFieldEx()"
    }
    void TextFieldEx::appendHtml(const Value& result, Instances::fl_text::TextField* textField, const ASString& newHtml)
    {
//##protect##"class_::TextFieldEx::appendHtml()"
        SF_UNUSED(result);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasStyleSheet()) // doesn't work if style sheet is set
                return;
            Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(ptxtDisp));
            ptxtDisp->AppendHtml(newHtml.ToCStr(), SF_MAX_UPINT, false, &imageInfoArray);
            if (imageInfoArray.GetSize() > 0)
            {
                ptxtDisp->ProcessImageTags(imageInfoArray);
            }
            ptxtDisp->SetDirtyFlag();
        }

//##protect##"class_::TextFieldEx::appendHtml()"
    }
    void TextFieldEx::setVerticalAlign(const Value& result, Instances::fl_text::TextField* textField, const ASString& valign)
    {
//##protect##"class_::TextFieldEx::setVerticalAlign()"
        SF_UNUSED(result);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (valign == "none")
                ptxtDisp->SetVAlignment(Text::DocView::VAlign_None);
            else 
            {
                if (valign == "top")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Top);
                else if (valign == "bottom")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (valign == "center")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Center);
            }
            ptxtDisp->SetDirtyFlag();
        }

//##protect##"class_::TextFieldEx::setVerticalAlign()"
    }
    void TextFieldEx::getVerticalAlign(ASString& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getVerticalAlign()"
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        GFx::TextField* ptxtDisp = textField->GetTextField();
        switch (ptxtDisp->GetVAlignment())
        {
        case Text::DocView::VAlign_Bottom:  result = "bottom"; break;
        case Text::DocView::VAlign_Center:  result = "center"; break;
        case Text::DocView::VAlign_Top:     result = "top"; break;
        default:                            result = "none"; break;
        }

//##protect##"class_::TextFieldEx::getVerticalAlign()"
    }
    void TextFieldEx::setVerticalAutoSize(const Value& result, Instances::fl_text::TextField* textField, const ASString& vautoSize)
    {
//##protect##"class_::TextFieldEx::setVerticalAutoSize()"
        SF_UNUSED3(result, textField, vautoSize);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (vautoSize == "none")
            {
                ptxtDisp->ClearAutoSizeY();
                ptxtDisp->SetVAlignment(Text::DocView::VAlign_None);
            }
            else 
            {
                ptxtDisp->SetAutoSizeY();
                if (vautoSize == "top")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Top);
                else if (vautoSize == "bottom")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (vautoSize == "center")
                    ptxtDisp->SetVAlignment(Text::DocView::VAlign_Center);
            }
            ptxtDisp->SetDirtyFlag();
        }
//##protect##"class_::TextFieldEx::setVerticalAutoSize()"
    }
    void TextFieldEx::getVerticalAutoSize(ASString& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getVerticalAutoSize()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        GFx::TextField* ptxtDisp = textField->GetTextField();
        if (!ptxtDisp->IsAutoSizeY())
            result = "none";
        else
        {
            switch (ptxtDisp->GetVAlignment())
            {
            case Text::DocView::VAlign_Bottom:  result = "bottom"; break;
            case Text::DocView::VAlign_Center:  result = "center"; break;
            case Text::DocView::VAlign_Top:     result = "top"; break;
            default:                            result = "none"; break;
            }
        }
//##protect##"class_::TextFieldEx::getVerticalAutoSize()"
    }
    void TextFieldEx::setTextAutoSize(const Value& result, Instances::fl_text::TextField* textField, const ASString& autoSz)
    {
//##protect##"class_::TextFieldEx::setTextAutoSize()"
        SF_UNUSED(result);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (autoSz == "none")
                ptxtDisp->SetTextAutoSize(Text::DocView::TAS_None);
            else 
            {
                if (autoSz == "shrink")
                    ptxtDisp->SetTextAutoSize(Text::DocView::TAS_Shrink);
                else if (autoSz == "fit")
                    ptxtDisp->SetTextAutoSize(Text::DocView::TAS_Fit);
            }
            ptxtDisp->SetDirtyFlag();
        }

//##protect##"class_::TextFieldEx::setTextAutoSize()"
    }
    void TextFieldEx::getTextAutoSize(ASString& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getTextAutoSize()"
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        GFx::TextField* ptxtDisp = textField->GetTextField();
        switch (ptxtDisp->GetTextAutoSize())
        {
        case Text::DocView::TAS_Shrink:     result = "shrink"; break;
        case Text::DocView::TAS_Fit:        result = "fit"; break;
        default:                            result = "none"; break;
        }

//##protect##"class_::TextFieldEx::getTextAutoSize()"
    }
    void TextFieldEx::setIMEEnabled(const Value& result, Instances::fl_text::TextField* textField, bool isEnabled)
    {
//##protect##"class_::TextFieldEx::setIMEEnabled()"
		SF_UNUSED(result);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

		GFx::TextField* pgfxTextField = textField->GetTextField();
		pgfxTextField->SetIMEDisabledFlag(!isEnabled);
//##protect##"class_::TextFieldEx::setIMEEnabled()"
    }
    void TextFieldEx::setImageSubstitutions(const Value& result, Instances::fl_text::TextField* textField, const Value& substInfo)
    {
//##protect##"class_::TextFieldEx::setImageSubstitutions()"
        SF_UNUSED2(result, substInfo);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        VM& vm = GetVM();
        GFx::TextField* pthis = textField->GetTextField();
        if (substInfo.IsNull() || substInfo.IsUndefined())
        {
            // clear all substitutions
            pthis->ClearIdImageDescAssoc();
            pthis->ClearImageSubstitutor();
            pthis->ForceCompleteReformat();
            pthis->SetDirtyFlag();
        }
        else
        {
            if (substInfo.IsObject())
            {
                Object& obj = *substInfo.GetObject();

                // if array is specified as a parameter, proceed it; otherwise
                // if an object is specified - proceed it as single element.
                if (vm.IsOfType(substInfo, vm.GetClassTraitsArray()))
                {
                    Instances::fl::Array& arr = static_cast<Instances::fl::Array&>(obj);
                    for (UPInt i = 0, n = arr.GetSize(); i < n; ++i)
                    {
                        const Value& ve = arr.At(i);
                        if (ve.IsObject())
                        {
                            ToAvmTextField(pthis)->ProceedImageSubstitution(vm, int(i), ve);
                        }
                    }
                }
                else
                {
                    const Value& ve = substInfo;
                    if (ve.IsObject())
                    {
                        ToAvmTextField(pthis)->ProceedImageSubstitution(vm, 0, ve);
                    }
                }
            }
            else
            {
                pthis->LogScriptWarning("%s.setImageSubstitutions() failed: parameter should be either 'null', object or array",
                    pthis->GetName().ToCStr());
            }
        }
//##protect##"class_::TextFieldEx::setImageSubstitutions()"
    }
    void TextFieldEx::updateImageSubstitution(const Value& result, Instances::fl_text::TextField* textField, const ASString& id, Instances::fl_display::BitmapData* image)
    {
//##protect##"class_::TextFieldEx::updateImageSubstitution()"
        SF_UNUSED3(result, id, image);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        GFx::TextField* pthis = textField->GetTextField();
        StringHashLH<Ptr<Text::ImageDesc> >* pimageDescAssoc = pthis->GetImageDescAssoc();
        if (pimageDescAssoc)
        {
            Ptr<Text::ImageDesc>* ppimgDesc = pimageDescAssoc->Get(id.ToCStr());
            if (ppimgDesc)
            {
                Text::ImageDesc* pimageDesc = ppimgDesc->GetPtr();
                SF_ASSERT(pimageDesc);
                if (!image)
                {
                    // if null or undefined - remove the substitution and reformat
                    Text::DocView::ImageSubstitutor* pimgSubst = pthis->CreateImageSubstitutor();
                    if (pimgSubst)
                    {
                        pimgSubst->RemoveImageDesc(pimageDesc);
                        pthis->ForceCompleteReformat();
                        pthis->RemoveIdImageDescAssoc(id.ToCStr());
                        pthis->SetDirtyFlag();
                    }
                }
                else
                {
                    ImageResource* pimgRes = image->GetImageResource();

                    Ptr<MovieDefImpl> md = pthis->GetResourceMovieDef();
                    Ptr<Render::Image> img;
                    if (pimgRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
                        img = static_cast<Render::Image*>(pimgRes->GetImage());
                    else
                    {
                        if (!md->GetImageCreator())
                            LogDebugMessage(Log_Warning, "ImageCreator is null in UpdateImageSubstitution");
                        else
                        {
                            ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(pthis));
                            img = *md->GetImageCreator()->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimgRes->GetImage()));
                        }
                    }
                    pimageDesc->pImage = img;
                    pthis->SetDirtyFlag();
                }
            }
        }
//##protect##"class_::TextFieldEx::updateImageSubstitution()"
    }
    void TextFieldEx::setNoTranslate(const Value& result, Instances::fl_text::TextField* textField, bool noTranslate)
    {
//##protect##"class_::TextFieldEx::setNoTranslate()"
        SF_UNUSED(result);

        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        GFx::TextField* pthis = textField->GetTextField();
        pthis->SetNoTranslate(noTranslate);

//##protect##"class_::TextFieldEx::setNoTranslate()"
    }
    void TextFieldEx::getNoTranslate(bool& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getNoTranslate()"
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }        

        GFx::TextField* pthis = textField->GetTextField();
        result = pthis->IsNoTranslate();

//##protect##"class_::TextFieldEx::getNoTranslate()"
    }
    void TextFieldEx::setBidirectionalTextEnabled(const Value& result, Instances::fl_text::TextField* textField, bool en)
    {
//##protect##"class_::TextFieldEx::setBidirectionalTextEnabled()"
        SF_UNUSED3(result, textField, en);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }        

        GFx::TextField* pthis = textField->GetTextField();
        pthis->EnableBidirectionalText(en);
//##protect##"class_::TextFieldEx::setBidirectionalTextEnabled()"
    }
    void TextFieldEx::getBidirectionalTextEnabled(bool& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getBidirectionalTextEnabled()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }        

        GFx::TextField* pthis = textField->GetTextField();
        result = pthis->IsBidirectionalTextEnabled();
//##protect##"class_::TextFieldEx::getBidirectionalTextEnabled()"
    }
    void TextFieldEx::setSelectionTextColor(const Value& result, Instances::fl_text::TextField* textField, UInt32 selColor)
    {
//##protect##"class_::TextFieldEx::setSelectionTextColor()"
        SF_UNUSED3(result, textField, selColor);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                ptxtDisp->GetEditorKit()->SetActiveSelectionTextColor(selColor);
        }
//##protect##"class_::TextFieldEx::setSelectionTextColor()"
    }
    void TextFieldEx::getSelectionTextColor(UInt32& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getSelectionTextColor()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                result = ptxtDisp->GetEditorKit()->GetActiveSelectionTextColor();
        }
//##protect##"class_::TextFieldEx::getSelectionTextColor()"
    }
    void TextFieldEx::setSelectionBkgColor(const Value& result, Instances::fl_text::TextField* textField, UInt32 selColor)
    {
//##protect##"class_::TextFieldEx::setSelectionBkgColor()"
        SF_UNUSED3(result, textField, selColor);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                ptxtDisp->GetEditorKit()->SetActiveSelectionBackgroundColor(selColor);
        }
//##protect##"class_::TextFieldEx::setSelectionBkgColor()"
    }
    void TextFieldEx::getSelectionBkgColor(UInt32& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getSelectionBkgColor()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                result = ptxtDisp->GetEditorKit()->GetActiveSelectionBackgroundColor();
        }
//##protect##"class_::TextFieldEx::getSelectionBkgColor()"
    }
    void TextFieldEx::setInactiveSelectionTextColor(const Value& result, Instances::fl_text::TextField* textField, UInt32 selColor)
    {
//##protect##"class_::TextFieldEx::setInactiveSelectionTextColor()"
        SF_UNUSED3(result, textField, selColor);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                ptxtDisp->GetEditorKit()->SetInactiveSelectionTextColor(selColor);
        }
//##protect##"class_::TextFieldEx::setInactiveSelectionTextColor()"
    }
    void TextFieldEx::getInactiveSelectionTextColor(UInt32& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getInactiveSelectionTextColor()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                result = ptxtDisp->GetEditorKit()->GetInactiveSelectionTextColor();
        }
//##protect##"class_::TextFieldEx::getInactiveSelectionTextColor()"
    }
    void TextFieldEx::setInactiveSelectionBkgColor(const Value& result, Instances::fl_text::TextField* textField, UInt32 selColor)
    {
//##protect##"class_::TextFieldEx::setInactiveSelectionBkgColor()"
        SF_UNUSED3(result, textField, selColor);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                ptxtDisp->GetEditorKit()->SetInactiveSelectionBackgroundColor(selColor);
        }
//##protect##"class_::TextFieldEx::setInactiveSelectionBkgColor()"
    }
    void TextFieldEx::getInactiveSelectionBkgColor(UInt32& result, Instances::fl_text::TextField* textField)
    {
//##protect##"class_::TextFieldEx::getInactiveSelectionBkgColor()"
        SF_UNUSED2(result, textField);
        if (!textField) 
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("textField")));
            return;
        }
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            GFx::TextField* ptxtDisp = textField->GetTextField();
            if (ptxtDisp->HasEditorKit())
                result = ptxtDisp->GetEditorKit()->GetInactiveSelectionBackgroundColor();
        }
//##protect##"class_::TextFieldEx::getInactiveSelectionBkgColor()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_appendHtml, const Value, Instances::fl_text::TextField*, const ASString&> TFunc_Classes_TextFieldEx_appendHtml;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setVerticalAlign, const Value, Instances::fl_text::TextField*, const ASString&> TFunc_Classes_TextFieldEx_setVerticalAlign;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getVerticalAlign, ASString, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getVerticalAlign;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setVerticalAutoSize, const Value, Instances::fl_text::TextField*, const ASString&> TFunc_Classes_TextFieldEx_setVerticalAutoSize;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getVerticalAutoSize, ASString, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getVerticalAutoSize;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setTextAutoSize, const Value, Instances::fl_text::TextField*, const ASString&> TFunc_Classes_TextFieldEx_setTextAutoSize;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getTextAutoSize, ASString, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getTextAutoSize;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setIMEEnabled, const Value, Instances::fl_text::TextField*, bool> TFunc_Classes_TextFieldEx_setIMEEnabled;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setImageSubstitutions, const Value, Instances::fl_text::TextField*, const Value&> TFunc_Classes_TextFieldEx_setImageSubstitutions;
typedef ThunkFunc3<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_updateImageSubstitution, const Value, Instances::fl_text::TextField*, const ASString&, Instances::fl_display::BitmapData*> TFunc_Classes_TextFieldEx_updateImageSubstitution;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setNoTranslate, const Value, Instances::fl_text::TextField*, bool> TFunc_Classes_TextFieldEx_setNoTranslate;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getNoTranslate, bool, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getNoTranslate;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setBidirectionalTextEnabled, const Value, Instances::fl_text::TextField*, bool> TFunc_Classes_TextFieldEx_setBidirectionalTextEnabled;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getBidirectionalTextEnabled, bool, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getBidirectionalTextEnabled;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setSelectionTextColor, const Value, Instances::fl_text::TextField*, UInt32> TFunc_Classes_TextFieldEx_setSelectionTextColor;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getSelectionTextColor, UInt32, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getSelectionTextColor;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setSelectionBkgColor, const Value, Instances::fl_text::TextField*, UInt32> TFunc_Classes_TextFieldEx_setSelectionBkgColor;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getSelectionBkgColor, UInt32, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getSelectionBkgColor;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setInactiveSelectionTextColor, const Value, Instances::fl_text::TextField*, UInt32> TFunc_Classes_TextFieldEx_setInactiveSelectionTextColor;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getInactiveSelectionTextColor, UInt32, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getInactiveSelectionTextColor;
typedef ThunkFunc2<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_setInactiveSelectionBkgColor, const Value, Instances::fl_text::TextField*, UInt32> TFunc_Classes_TextFieldEx_setInactiveSelectionBkgColor;
typedef ThunkFunc1<Classes::fl_gfx::TextFieldEx, Classes::fl_gfx::TextFieldEx::mid_getInactiveSelectionBkgColor, UInt32, Instances::fl_text::TextField*> TFunc_Classes_TextFieldEx_getInactiveSelectionBkgColor;

template <> const TFunc_Classes_TextFieldEx_appendHtml::TMethod TFunc_Classes_TextFieldEx_appendHtml::Method = &Classes::fl_gfx::TextFieldEx::appendHtml;
template <> const TFunc_Classes_TextFieldEx_setVerticalAlign::TMethod TFunc_Classes_TextFieldEx_setVerticalAlign::Method = &Classes::fl_gfx::TextFieldEx::setVerticalAlign;
template <> const TFunc_Classes_TextFieldEx_getVerticalAlign::TMethod TFunc_Classes_TextFieldEx_getVerticalAlign::Method = &Classes::fl_gfx::TextFieldEx::getVerticalAlign;
template <> const TFunc_Classes_TextFieldEx_setVerticalAutoSize::TMethod TFunc_Classes_TextFieldEx_setVerticalAutoSize::Method = &Classes::fl_gfx::TextFieldEx::setVerticalAutoSize;
template <> const TFunc_Classes_TextFieldEx_getVerticalAutoSize::TMethod TFunc_Classes_TextFieldEx_getVerticalAutoSize::Method = &Classes::fl_gfx::TextFieldEx::getVerticalAutoSize;
template <> const TFunc_Classes_TextFieldEx_setTextAutoSize::TMethod TFunc_Classes_TextFieldEx_setTextAutoSize::Method = &Classes::fl_gfx::TextFieldEx::setTextAutoSize;
template <> const TFunc_Classes_TextFieldEx_getTextAutoSize::TMethod TFunc_Classes_TextFieldEx_getTextAutoSize::Method = &Classes::fl_gfx::TextFieldEx::getTextAutoSize;
template <> const TFunc_Classes_TextFieldEx_setIMEEnabled::TMethod TFunc_Classes_TextFieldEx_setIMEEnabled::Method = &Classes::fl_gfx::TextFieldEx::setIMEEnabled;
template <> const TFunc_Classes_TextFieldEx_setImageSubstitutions::TMethod TFunc_Classes_TextFieldEx_setImageSubstitutions::Method = &Classes::fl_gfx::TextFieldEx::setImageSubstitutions;
template <> const TFunc_Classes_TextFieldEx_updateImageSubstitution::TMethod TFunc_Classes_TextFieldEx_updateImageSubstitution::Method = &Classes::fl_gfx::TextFieldEx::updateImageSubstitution;
template <> const TFunc_Classes_TextFieldEx_setNoTranslate::TMethod TFunc_Classes_TextFieldEx_setNoTranslate::Method = &Classes::fl_gfx::TextFieldEx::setNoTranslate;
template <> const TFunc_Classes_TextFieldEx_getNoTranslate::TMethod TFunc_Classes_TextFieldEx_getNoTranslate::Method = &Classes::fl_gfx::TextFieldEx::getNoTranslate;
template <> const TFunc_Classes_TextFieldEx_setBidirectionalTextEnabled::TMethod TFunc_Classes_TextFieldEx_setBidirectionalTextEnabled::Method = &Classes::fl_gfx::TextFieldEx::setBidirectionalTextEnabled;
template <> const TFunc_Classes_TextFieldEx_getBidirectionalTextEnabled::TMethod TFunc_Classes_TextFieldEx_getBidirectionalTextEnabled::Method = &Classes::fl_gfx::TextFieldEx::getBidirectionalTextEnabled;
template <> const TFunc_Classes_TextFieldEx_setSelectionTextColor::TMethod TFunc_Classes_TextFieldEx_setSelectionTextColor::Method = &Classes::fl_gfx::TextFieldEx::setSelectionTextColor;
template <> const TFunc_Classes_TextFieldEx_getSelectionTextColor::TMethod TFunc_Classes_TextFieldEx_getSelectionTextColor::Method = &Classes::fl_gfx::TextFieldEx::getSelectionTextColor;
template <> const TFunc_Classes_TextFieldEx_setSelectionBkgColor::TMethod TFunc_Classes_TextFieldEx_setSelectionBkgColor::Method = &Classes::fl_gfx::TextFieldEx::setSelectionBkgColor;
template <> const TFunc_Classes_TextFieldEx_getSelectionBkgColor::TMethod TFunc_Classes_TextFieldEx_getSelectionBkgColor::Method = &Classes::fl_gfx::TextFieldEx::getSelectionBkgColor;
template <> const TFunc_Classes_TextFieldEx_setInactiveSelectionTextColor::TMethod TFunc_Classes_TextFieldEx_setInactiveSelectionTextColor::Method = &Classes::fl_gfx::TextFieldEx::setInactiveSelectionTextColor;
template <> const TFunc_Classes_TextFieldEx_getInactiveSelectionTextColor::TMethod TFunc_Classes_TextFieldEx_getInactiveSelectionTextColor::Method = &Classes::fl_gfx::TextFieldEx::getInactiveSelectionTextColor;
template <> const TFunc_Classes_TextFieldEx_setInactiveSelectionBkgColor::TMethod TFunc_Classes_TextFieldEx_setInactiveSelectionBkgColor::Method = &Classes::fl_gfx::TextFieldEx::setInactiveSelectionBkgColor;
template <> const TFunc_Classes_TextFieldEx_getInactiveSelectionBkgColor::TMethod TFunc_Classes_TextFieldEx_getInactiveSelectionBkgColor::Method = &Classes::fl_gfx::TextFieldEx::getInactiveSelectionBkgColor;

namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo TextFieldEx::mi[TextFieldEx::MemberInfoNum] = {
        {"VALIGN_NONE", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VALIGN_NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VALIGN_TOP", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VALIGN_TOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VALIGN_CENTER", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VALIGN_CENTER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VALIGN_BOTTOM", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VALIGN_BOTTOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TEXTAUTOSZ_NONE", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, TEXTAUTOSZ_NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TEXTAUTOSZ_SHRINK", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, TEXTAUTOSZ_SHRINK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TEXTAUTOSZ_FIT", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, TEXTAUTOSZ_FIT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VAUTOSIZE_NONE", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VAUTOSIZE_NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VAUTOSIZE_TOP", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VAUTOSIZE_TOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VAUTOSIZE_CENTER", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VAUTOSIZE_CENTER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VAUTOSIZE_BOTTOM", NULL, OFFSETOF(Classes::fl_gfx::TextFieldEx, VAUTOSIZE_BOTTOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };

    // const UInt16 TextFieldEx::tito[TextFieldEx::ThunkInfoNum] = {
    //    0, 3, 6, 8, 11, 13, 16, 18, 21, 24, 28, 31, 33, 36, 38, 41, 43, 46, 48, 51, 53, 56, 
    // };
    const TypeInfo* TextFieldEx::tit[58] = {
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::ObjectTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::StringTI, &AS3::fl_display::BitmapDataTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_text::TextFieldTI, 
        NULL, &AS3::fl_text::TextFieldTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_text::TextFieldTI, 
    };
    const ThunkInfo TextFieldEx::ti[TextFieldEx::ThunkInfoNum] = {
        {TFunc_Classes_TextFieldEx_appendHtml::Func, &TextFieldEx::tit[0], "appendHtml", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setVerticalAlign::Func, &TextFieldEx::tit[3], "setVerticalAlign", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getVerticalAlign::Func, &TextFieldEx::tit[6], "getVerticalAlign", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setVerticalAutoSize::Func, &TextFieldEx::tit[8], "setVerticalAutoSize", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getVerticalAutoSize::Func, &TextFieldEx::tit[11], "getVerticalAutoSize", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setTextAutoSize::Func, &TextFieldEx::tit[13], "setTextAutoSize", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getTextAutoSize::Func, &TextFieldEx::tit[16], "getTextAutoSize", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setIMEEnabled::Func, &TextFieldEx::tit[18], "setIMEEnabled", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setImageSubstitutions::Func, &TextFieldEx::tit[21], "setImageSubstitutions", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_updateImageSubstitution::Func, &TextFieldEx::tit[24], "updateImageSubstitution", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setNoTranslate::Func, &TextFieldEx::tit[28], "setNoTranslate", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getNoTranslate::Func, &TextFieldEx::tit[31], "getNoTranslate", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setBidirectionalTextEnabled::Func, &TextFieldEx::tit[33], "setBidirectionalTextEnabled", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getBidirectionalTextEnabled::Func, &TextFieldEx::tit[36], "getBidirectionalTextEnabled", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setSelectionTextColor::Func, &TextFieldEx::tit[38], "setSelectionTextColor", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getSelectionTextColor::Func, &TextFieldEx::tit[41], "getSelectionTextColor", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setSelectionBkgColor::Func, &TextFieldEx::tit[43], "setSelectionBkgColor", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getSelectionBkgColor::Func, &TextFieldEx::tit[46], "getSelectionBkgColor", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setInactiveSelectionTextColor::Func, &TextFieldEx::tit[48], "setInactiveSelectionTextColor", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getInactiveSelectionTextColor::Func, &TextFieldEx::tit[51], "getInactiveSelectionTextColor", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_setInactiveSelectionBkgColor::Func, &TextFieldEx::tit[53], "setInactiveSelectionBkgColor", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_TextFieldEx_getInactiveSelectionBkgColor::Func, &TextFieldEx::tit[56], "getInactiveSelectionBkgColor", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    TextFieldEx::TextFieldEx(VM& vm, const ClassInfo& ci)
    : fl_gfx::InteractiveObjectEx(vm, ci)
    {
//##protect##"ClassTraits::TextFieldEx::TextFieldEx()"
//##protect##"ClassTraits::TextFieldEx::TextFieldEx()"

    }

    Pickable<Traits> TextFieldEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextFieldEx(vm, AS3::fl_gfx::TextFieldExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::TextFieldExCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo TextFieldExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::TextFieldEx::InstanceType),
        ClassTraits::fl_gfx::TextFieldEx::ThunkInfoNum,
        ClassTraits::fl_gfx::TextFieldEx::MemberInfoNum,
        0,
        0,
        "TextFieldEx", "scaleform.gfx", &fl_gfx::InteractiveObjectExTI,
        TypeInfo::None
    };

    const ClassInfo TextFieldExCI = {
        &TextFieldExTI,
        ClassTraits::fl_gfx::TextFieldEx::MakeClassTraits,
        ClassTraits::fl_gfx::TextFieldEx::ti,
        ClassTraits::fl_gfx::TextFieldEx::mi,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

