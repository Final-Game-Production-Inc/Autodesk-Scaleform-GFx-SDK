//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_IMEEx.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_IMEEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "AS3_Obj_Gfx_IMECandidateListStyle.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    IMEEx::IMEEx(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::IMEEx::IMEEx()"
//##protect##"class_::IMEEx::IMEEx()"
    }
    void IMEEx::getIMECandidateListStyle(SPtr<Instances::fl_gfx::IMECandidateListStyle>& result)
    {
//##protect##"class_::IMEEx::getIMECandidateListStyle()"
#ifndef SF_NO_IME_SUPPORT		
		GFx::IMECandidateListStyle st; 

		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl();   
	
		if (pmovieImpl)
		{
			pmovieImpl->GetIMECandidateListStyle(&st);
		
            Class* candListStyleVal = GetVM().GetClass("scaleform.gfx.IMECandidateListStyle", GetVM().GetCurrentAppDomain());
			if ( asvm.ConstructInstance(result, candListStyleVal, 0))
			{
				if (st.HasTextColor())
				{
					UInt32 c = (st.GetTextColor() & 0xFFFFFFu);
					result->set_textColor(c);
				}
				if (st.HasBackgroundColor())
				{
					UInt32 c = (st.GetBackgroundColor() & 0xFFFFFFu);
					result->set_backgroundColor(c);
				}
				if (st.HasIndexBackgroundColor())
				{
					UInt32 c = (st.GetIndexBackgroundColor() & 0xFFFFFFu);
					result->set_indexBackgroundColor(c);
				}
				if (st.HasSelectedTextColor())
				{
					UInt32 c = (st.GetSelectedTextColor() & 0xFFFFFFu);
					result->set_selectedTextColor(c);
				}
				if (st.HasSelectedBackgroundColor())
				{
					UInt32 c = (st.GetSelectedBackgroundColor() & 0xFFFFFFu);
					result->set_selectedBackgroundColor(c);
				}
				if (st.HasSelectedIndexBackgroundColor())
				{
					UInt32 c = (st.GetSelectedIndexBackgroundColor() & 0xFFFFFFu);
					result->set_selectedIndexBackgroundColor(c);
				}
				if (st.HasFontSize())
				{
					UInt32 c = st.GetFontSize();
					result->set_fontSize(c); 

				}
				// Reading Window Styles

				if (st.HasReadingWindowTextColor())
				{
					UInt32 c = st.GetReadingWindowTextColor();
					result->set_readingWindowTextColor(c);
				}

				if (st.HasReadingWindowBackgroundColor())
				{
					UInt32 c = st.GetReadingWindowBackgroundColor();
					result->set_readinwWindowBackgroundColor(c); 
				}

				if (st.HasReadingWindowFontSize())
				{
					UInt32 c = st.GetReadingWindowFontSize();
					result->set_readingWindowFontSize(c);
				}
			}
		}
#else
		SF_UNUSED(result);
#endif
//##protect##"class_::IMEEx::getIMECandidateListStyle()"
    }
    void IMEEx::setIMECandidateListStyle(const Value& result, Instances::fl_gfx::IMECandidateListStyle* style)
    {
//##protect##"class_::IMEEx::setIMECandidateListStyle()"
#ifndef SF_NO_IME_SUPPORT		
		GFx::IMECandidateListStyle st; 

		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl();   

		if (pmovieImpl)
		{
			int c;
			if ((c = style->textColor) != -1)
			{
				st.SetTextColor(c);
			}
			if ((c = style->selectedTextColor) != -1)
			{
				st.SetSelectedTextColor(c);
			}
			if ((c = style->selectedTextColor) != -1)
			{
				st.SetSelectedTextColor(c);
			}
			if ((c = style->fontSize) != -1)
			{
				st.SetFontSize(c);
			}
			if ((c = style->backgroundColor) != -1)
			{
				st.SetBackgroundColor(c);
			}
			if ((c = style->selectedBackgroundColor) != -1)
			{
				st.SetSelectedBackgroundColor(c);
			}
			if ((c = style->indexBackgroundColor) != -1)
			{
				st.SetIndexBackgroundColor(c);
			}
			if ((c = style->selectedIndexBackgroundColor) != -1)
			{
				st.SetSelectedIndexBackgroundColor(c);
			}
			if ((c = style->readingWindowTextColor) != -1)
			{
				st.SetReadingWindowTextColor(c);
			}
			if ((c = style->readingWindowBackgroundColor) != -1)
			{
				st.SetReadingWindowBackgroundColor(c);
			}
			if ((c = style->readingWindowFontSize) != -1)
			{
				st.SetReadingWindowFontSize(c);
			}
			pmovieImpl->SetIMECandidateListStyle(st);
		}
		SF_UNUSED1(result);
#else
		SF_UNUSED2(result, style);
#endif
//##protect##"class_::IMEEx::setIMECandidateListStyle()"
    }
    void IMEEx::SendLangBarMessage(const Value& result, Instances::fl_display::Sprite* mc, const ASString& command, const ASString& message)
    {
//##protect##"class_::IMEEx::SendLangBarMessage()"
#ifndef SF_NO_IME_SUPPORT
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl();   
		if (pmovieImpl)
		{
			GFx::IMEManagerBase* pimeManager = pmovieImpl->GetIMEManager();
			if (pimeManager)
			{

				GFx::DisplayObject *pdispObj = mc->pDispObj;
				pimeManager->SendLangBarMessage(pdispObj, command, message);
			}
		}
        SF_UNUSED1(result);
#else
		SF_UNUSED4(result, mc, command, message);
#endif
//##protect##"class_::IMEEx::SendLangBarMessage()"
    }
    void IMEEx::GetOSVersion(ASString& result)
    {
//##protect##"class_::IMEEx::GetOSVersion()"
#ifndef SF_NO_IME_SUPPORT
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl();   
		if (pmovieImpl)
		{
			GFx::IMEManagerBase* pimeManager = pmovieImpl->GetIMEManager();
			if (pimeManager)
			{

				Scaleform::String version = pimeManager->GetWindowsVersion();
				result = version;
			}
		}
#else
		SF_UNUSED(result);
#endif
//##protect##"class_::IMEEx::GetOSVersion()"
    }
    void IMEEx::GetCompositionString(ASString& result)
    {
//##protect##"class_::IMEEx::GetCompositionString()"
#ifndef SF_NO_IME_SUPPORT
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		MovieImpl* pmovieImpl = asvm.GetMovieImpl();   
		if (pmovieImpl)
		{
			GFx::IMEManagerBase* pimeManager = pmovieImpl->GetIMEManager();
			if (pimeManager)
			{
				const wchar_t* pcompString = pimeManager->GetCompositionString();
				Scaleform::String compString(pcompString);
				result = compString;
			}
		}
#else
		SF_UNUSED(result);
#endif
//##protect##"class_::IMEEx::GetCompositionString()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_gfx::IMEEx, Classes::fl_gfx::IMEEx::mid_getIMECandidateListStyle, SPtr<Instances::fl_gfx::IMECandidateListStyle> > TFunc_Classes_IMEEx_getIMECandidateListStyle;
typedef ThunkFunc1<Classes::fl_gfx::IMEEx, Classes::fl_gfx::IMEEx::mid_setIMECandidateListStyle, const Value, Instances::fl_gfx::IMECandidateListStyle*> TFunc_Classes_IMEEx_setIMECandidateListStyle;
typedef ThunkFunc3<Classes::fl_gfx::IMEEx, Classes::fl_gfx::IMEEx::mid_SendLangBarMessage, const Value, Instances::fl_display::Sprite*, const ASString&, const ASString&> TFunc_Classes_IMEEx_SendLangBarMessage;
typedef ThunkFunc0<Classes::fl_gfx::IMEEx, Classes::fl_gfx::IMEEx::mid_GetOSVersion, ASString> TFunc_Classes_IMEEx_GetOSVersion;
typedef ThunkFunc0<Classes::fl_gfx::IMEEx, Classes::fl_gfx::IMEEx::mid_GetCompositionString, ASString> TFunc_Classes_IMEEx_GetCompositionString;

template <> const TFunc_Classes_IMEEx_getIMECandidateListStyle::TMethod TFunc_Classes_IMEEx_getIMECandidateListStyle::Method = &Classes::fl_gfx::IMEEx::getIMECandidateListStyle;
template <> const TFunc_Classes_IMEEx_setIMECandidateListStyle::TMethod TFunc_Classes_IMEEx_setIMECandidateListStyle::Method = &Classes::fl_gfx::IMEEx::setIMECandidateListStyle;
template <> const TFunc_Classes_IMEEx_SendLangBarMessage::TMethod TFunc_Classes_IMEEx_SendLangBarMessage::Method = &Classes::fl_gfx::IMEEx::SendLangBarMessage;
template <> const TFunc_Classes_IMEEx_GetOSVersion::TMethod TFunc_Classes_IMEEx_GetOSVersion::Method = &Classes::fl_gfx::IMEEx::GetOSVersion;
template <> const TFunc_Classes_IMEEx_GetCompositionString::TMethod TFunc_Classes_IMEEx_GetCompositionString::Method = &Classes::fl_gfx::IMEEx::GetCompositionString;

namespace ClassTraits { namespace fl_gfx
{
    // const UInt16 IMEEx::tito[IMEEx::ThunkInfoNum] = {
    //    0, 1, 3, 7, 8, 
    // };
    const TypeInfo* IMEEx::tit[9] = {
        &AS3::fl_gfx::IMECandidateListStyleTI, 
        NULL, &AS3::fl_gfx::IMECandidateListStyleTI, 
        NULL, &AS3::fl_display::SpriteTI, &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo IMEEx::ti[IMEEx::ThunkInfoNum] = {
        {TFunc_Classes_IMEEx_getIMECandidateListStyle::Func, &IMEEx::tit[0], "getIMECandidateListStyle", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_IMEEx_setIMECandidateListStyle::Func, &IMEEx::tit[1], "setIMECandidateListStyle", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_IMEEx_SendLangBarMessage::Func, &IMEEx::tit[3], "SendLangBarMessage", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Classes_IMEEx_GetOSVersion::Func, &IMEEx::tit[7], "GetOSVersion", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_IMEEx_GetCompositionString::Func, &IMEEx::tit[8], "GetCompositionString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    IMEEx::IMEEx(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::IMEEx::IMEEx()"
//##protect##"ClassTraits::IMEEx::IMEEx()"

    }

    Pickable<Traits> IMEEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IMEEx(vm, AS3::fl_gfx::IMEExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::IMEExCI));
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
    const TypeInfo IMEExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::IMEEx::InstanceType),
        ClassTraits::fl_gfx::IMEEx::ThunkInfoNum,
        0,
        0,
        0,
        "IMEEx", "scaleform.gfx", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo IMEExCI = {
        &IMEExTI,
        ClassTraits::fl_gfx::IMEEx::MakeClassTraits,
        ClassTraits::fl_gfx::IMEEx::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

