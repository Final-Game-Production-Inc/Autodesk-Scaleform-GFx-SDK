/**************************************************************************

Filename    :   AS3_IMEManager.cpp
Content     :   AS3 IMEManager functinality
Created     :   Dec 17, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//#include <AS3/IME/AS3_IMEManager.h>

#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS3/AS3_Object.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_VM.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequest.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Loader.h"
#include "GFx/AS3/IME/AS3_IMEManager.h"
#include "GFx/AS3/Obj/GFx/AS3_Obj_Gfx_IMEEventEx.h"
#include "GFxConfig.h"
#include "GFx/GFx_DisplayObject.h"

namespace Scaleform { namespace GFx { namespace AS3{

#ifndef SF_NO_IME_SUPPORT

IMEManager::IMEManager(): ASIMEManager()
{ 
    UnsupportedIMEWindowsFlag = GFxIME_ShowUnsupportedIMEWindows;
	pTextField	= 0;
	pMovie		= 0;
	CandidateListState = 0;
	CandListVal.SetNull();
}

bool IMEManager::IsCandidateListLoaded() const
{
	// check if we already have loaded candidate list into _level9999
	if (pMovie)
	{
		Scaleform::GFx::Value v;
	//	MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pMovie));
		if (!pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
			v.SetNumber(0);
	//	return pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL) && v.GetNumber() == 2;
		return true;
	}
	return false;
}

void IMEManager::OnOpenCandidateList()
{
    if (!pMovie)
        return;

//  On Win Vista, it's possible for the candidate list to be loaded up before STARTCOMPOSITION
//  message arrives (which sets the textfield pointer), hence if the textfield pointer is found to be
//  null, we should try to get it from the currently focussed object. This happens with Chinese Traditional
//  IME DaYi 6.0 and Array
//  SF_ASSERT(pTextField.GetPtr());

    TextField* ptextFld = pTextField;
    if (!ptextFld)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
		if (pfocusedCh && pfocusedCh->GetType() == CharacterDef::TextField)
        {
            ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
        }
        if(!ptextFld)
            return;
    }

	Sprite* psprite = GetCandidateListSprite();
    if (psprite)
	    ((TextField*)ptextFld)->SetCandidateListFont(psprite);
}

bool IMEManager::IsTextFieldFocused() const
{
    if (pMovie)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
        return IsTextFieldFocused(pfocusedCh);
    }
    return false;
}

bool IMEManager::IsTextFieldFocused(InteractiveObject* ptextfield) const
{
    if (pMovie)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
		return (pfocusedCh && pfocusedCh->GetType() == CharacterDef::TextField &&
                static_cast<TextField*>(pfocusedCh.GetPtr())->IsIMEEnabled() &&
                pfocusedCh == ptextfield);
    }
    return false;
}

void IMEManager::SetIMEMoviePath(const char* pcandidateSwfPath)
{
    CandidateSwfPath = pcandidateSwfPath;
}

bool IMEManager::GetIMEMoviePath(String& candidateSwfPath)
{
    candidateSwfPath = "";
    return !candidateSwfPath.IsEmpty();
}
/*
namespace
{
	class CandidateListLoader : public AS2::MovieClipLoader
    {
        IMEManager*      pIMEManager;
   
    public:
        CandidateListLoader(IMEManager* pimeManager,  AS2::Environment* penv) :
          MovieClipLoader(penv), pIMEManager(pimeManager) {}

        void NotifyOnLoadInit(AS2::Environment* penv, InteractiveObject* ptarget)
        {
            SF_UNUSED(penv); 
			IMEManagerBase* pbaseIMEManager = pIMEManager->GetIMEManager();
            if (pIMEManager->pMovie)
            {
				GFx::Value v;
                v.SetNumber(2); // indicates - "loaded"
                pIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
                v.SetString(pIMEManager->CandidateSwfPath.ToCStr());
                pIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_path", v);
            }

            if (ptarget)
            {
                ptarget->GetResourceMovieDef()->PinResource(); // so, do pin instead
                String path;
                ptarget->GetAbsolutePath(&path);
				// obtain the GFx part of the IMEManager.				
				if (pbaseIMEManager)
					pbaseIMEManager->OnCandidateListLoaded(path.ToCStr());
            }
            else
			{
				if (pbaseIMEManager)
					pbaseIMEManager->OnCandidateListLoaded(NULL);
			}
        }
        void NotifyOnLoadError(AS2::Environment* penv, InteractiveObject* ptarget, const char* errorCode, int status)
        {
            SF_UNUSED3(penv, ptarget, status);
            pIMEManager->CandidateSwfErrorMsg = "Error in loading candidate list from ";
            pIMEManager->CandidateSwfErrorMsg += pIMEManager->CandidateSwfPath;
            if (pIMEManager->pMovie)
            {
                String level0Path;
				AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pIMEManager->pMovie));
                pmovieRoot->GetLevel0Path(&level0Path);
                pIMEManager->CandidateSwfErrorMsg += " at ";
                pIMEManager->CandidateSwfErrorMsg += level0Path;

				GFx::Value v;
                v.SetNumber(-1); // means - "failed to load"
                pIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
            }
            pIMEManager->CandidateSwfErrorMsg += ": ";
            pIMEManager->CandidateSwfErrorMsg += errorCode;
        }
    };
}
*/
class NotifyLoadInitCandidateList : public NotifyLoadInitC 
{
	
public:
    NotifyLoadInitCandidateList(AS3::MovieRoot* pmovieRoot, Instances::fl_display::Loader* loader, IMEManager* pASIMEManager):
	  pMovieRoot(pmovieRoot), pLoader(loader), pASIMEManager(pASIMEManager){};
	void  OpenEventCallback() 
	{
		/*
		GFx::Value result;
		GFx::Value args;
		AS3::Value val;
		AS3::Value val2;
		
		AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pASIMEManager->pMovie));
		
		AS3::Multiname mn(pMovieRoot->GetAVM()->GetPublicNamespace(),
			pMovieRoot->GetStringManager()->CreateString("contentLoaderInfo"));
		if (pLoader->GetProperty(mn, val))
		{
			AS3::Multiname mn2(pMovieRoot->GetAVM()->GetPublicNamespace(),
				pMovieRoot->GetStringManager()->CreateString("content"));
			if (val.GetObject()->GetProperty(mn2, val2))
			{
				pMovieRoot->ASValue2GFxValue(val2, &(pASIMEManager->CandListVal));
			}	
			Ptr<MyFunc> customFunc = *SF_HEAP_NEW(Memory::GetGlobalHeap()) MyFunc();
			GFx::Value func;
			pASIMEManager->pMovie->CreateFunction(&func, customFunc);
			pASIMEManager->CandListVal.SetMember("SendHello", func);
		}	
		*/
	}

	void  LoadEventCallback()
	{
	
	}

	void  InitEventCallback()
	{

		GFx::Value result;
		GFx::Value args;
		AS3::Value val;
		AS3::Value val2;

		AS3::Multiname mn(pMovieRoot->GetAVM()->GetPublicNamespace(),
			pMovieRoot->GetStringManager()->CreateString("contentLoaderInfo"));
		if (pLoader->GetProperty(mn, val) && pASIMEManager->CandListVal.IsNull())
		{
			AS3::Multiname mn2(pMovieRoot->GetAVM()->GetPublicNamespace(),
				pMovieRoot->GetStringManager()->CreateString("content"));
			if (val.GetObject()->GetProperty(mn2, val2))
			{
				pMovieRoot->ASValue2GFxValue(val2, &(pASIMEManager->CandListVal));

			//	pASIMEManager->pCustomFunc = *SF_HEAP_NEW(Memory::GetGlobalHeap())ASIMEManager::IMEFuncHandler(pASIMEManager->pimeManager);
				if (!pASIMEManager->CandListVal.IsNull())
				{
					GFx::Value func;
					pASIMEManager->pMovie->CreateFunction(&func, pASIMEManager->CustomFuncCandList);
					pASIMEManager->CandListVal.SetMember("SendIMEMessage", func);
					pASIMEManager->CandidateListState = 2;
					Stage* pstage = pMovieRoot->GetStage();
					ToAvmDisplayObjContainer(pstage)->AddChild(pLoader->GetDisplayObjContainer());
					pASIMEManager->OnOpenCandidateList();
					pASIMEManager->CandListVal.Invoke("Init");
				}				
			}	
			
		//	pASIMEManager->CandListVal.Invoke("Init");
		}		
	}
	void  ErrorEventCallback()
	{
//		printf("hello\n");
	}

	AS3::MovieRoot*         pMovieRoot; // shouldn't it be a strong ptr??
	SPtr<Instances::fl_display::Loader> pLoader;
	Ptr<IMEManager>         pASIMEManager;
};

void IMEManager::ASRootMovieCreated(Ptr<Sprite> spr) 
{
	GFx::Value func;
	AS3::Value asFunc;
	AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pMovie));
	AS3::VM*        vm   = pmovieRoot->GetAVM();
	pMovie->CreateFunction(&func, CustomFuncLanguageBar);
	pmovieRoot->GFxValue2ASValue(func, &asFunc);
	AvmSprite* avmSpr = AS3::ToAvmSprite(spr);
	if (avmSpr)
	{
		AS3::Multiname mn1(pmovieRoot->GetAVM()->GetPublicNamespace(),
			pmovieRoot->GetStringManager()->CreateString("SendLangBarMessage"));
		// for some to be determined reasons, the as3obj can be null
		if (avmSpr->GetAS3Obj())
		{
			if (!avmSpr->GetAS3Obj()->SetProperty(mn1, asFunc))
			{
				if (vm->IsException())
					vm->OutputAndIgnoreException();
			}
		}
	}	
}

bool IMEManager::AcquireCandidateList()
{
	AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pMovie));
	Value result;
	Value result2;
	Value tmp;

	if (CandidateListState == 0)
	{
		if (GetIMEManager() && GetIMEManager()->CheckCandListExists())
		{
			// First check to see if the candidate list movie (ime.swf by default) exists in the context of the
			// current active movie- if not, it'll fail to load, so no point creating a load queue entry.

			Ptr<FileOpenerBase> pfileOpener = GetIMEManager()->GetLoader()->GetFileOpener();
			Ptr<URLBuilder> purlBuilder = GetIMEManager()->GetLoader()->GetURLBuilder();

			if (!purlBuilder)
				purlBuilder = *new URLBuilder();
			if (pfileOpener)
			{
				String parentPath;
				parentPath = pMovie->GetMovieDef()->GetFileURL();
				purlBuilder->ExtractFilePath(&parentPath);
				// Check if path is absolute. This needs to be done because: If the user
				// specifies a working directory and swf file to be loaded on start up (using
				// vis studio settings), then getfileurl will just return the name of the file to be loaded
				// for example, IMESample.swf. In this case, we need to use WinAPI to get the current
				// working directory and then append the candidateswfpath to it. 
				// if the user drags/drops a swf file, then GetFileURL will return the actual
				// path and candidateswfpath can be appended directly to that path. 
				if (purlBuilder->IsPathAbsolute(parentPath))
				{
					URLBuilder::LocationInfo loc(URLBuilder::File_Regular, CandidateSwfPath, parentPath);

					String path;
					if (purlBuilder)
						purlBuilder->BuildURL(&path, loc);
					else
						URLBuilder::DefaultBuildURL(&path, loc);

					if (pfileOpener->GetFileModifyTime(path) == -1)
						return false;
				}
				else
				{
#if   defined(SF_OS_WIN32)
					wchar_t workingDir[MAX_PATH];
					GetCurrentDirectoryW(MAX_PATH, workingDir);
					char utf8workingDir[MAX_PATH];
					UTF8Util::EncodeStringSafe(utf8workingDir, MAX_PATH, workingDir);
					URLBuilder::LocationInfo loc(URLBuilder::File_Regular, CandidateSwfPath, String(utf8workingDir));
					String path;
					if (purlBuilder)
						purlBuilder->BuildURL(&path, loc);
					if (pfileOpener->GetFileModifyTime(path) == -1)
						return false;
#else
					return false;
#endif
				}
			}
		}

		CandidateListState = 1; // means - "loading"
		ASVM* vm = pmovieRoot->GetAVM();

		if (!vm->ConstructBuiltinValue(result, "flash.display.Loader"))
            return false;

		if (!vm->ConstructBuiltinValue(result2, "flash.net.URLRequest"))
            return false;

		Instances::fl_display::Loader* loader = (Instances::fl_display::Loader*)(result.GetObject());
		loader->InitInstance(false);
        Instances::fl_net::URLRequest* request = (Instances::fl_net::URLRequest*)(result2.GetObject());
		ASString imeFile = pmovieRoot->GetStringManager()->CreateString(CandidateSwfPath);
		request->urlSet(tmp, imeFile);
		LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None;
		
		LoadQueueEntry *pentry = SF_HEAP_NEW(pmovieRoot->GetMovieHeap()) LoadQueueEntry(request, loader, method);

		Ptr<NotifyLoadInitC> pnotifyCandidateListInit = 
			*new NotifyLoadInitCandidateList(pmovieRoot, loader, this);
		pentry->SetNotifyLoadInitCInterface(pnotifyCandidateListInit);
		// Calling AddLoadQueueEntry ensures candidate list is always loaded in single threaded
		// mode eliminating complications resulting from multithreaded loading
		pmovieRoot->AddLoadQueueEntry(pentry);
		
		return true;
	}
	return true;
}

void IMEManager::DispatchEvent(const char* message, const char* messageType, const char* type)
{

	AS3::ValueArray params;
	AS3::MovieRoot* proot = AS3::ToAS3Root((MovieImpl*)(pMovie));
	//AS3::Instances::fl_display::DisplayObject*    as3obj = ToAvmDisplayObj(pdispContainer)->GetAS3Obj();
	bool isdispObj = true;
/*
	if (!SFstrcmp(type, "LangBar"))
	{
		isdispObj = pLangContext.IsDisplayObject();
	}
	else if (!SFstrcmp(type, "StatusWindow"))
	{
		isdispObj = pStatusContext.IsDisplayObject();
	}
*/	
	if (isdispObj)
	{
		
		AvmDisplayObjBase* avmDispObj = 0;
		AS3::Value as3val; 
		if (pLangContext2 && !SFstrcmp(type, "LangBar"))
		{
			avmDispObj = pLangContext2->GetAvmObjImpl();
		//	proot->GFxValue2ASValue(pLangContext, &as3val);
		}
		else if (pStatusContext2 && !SFstrcmp(type, "StatusWindow"))
		{
			avmDispObj = pStatusContext2->GetAvmObjImpl();
		//	proot->GFxValue2ASValue(pStatusContext, &as3val);
		}
		
		//AS3::Instances::fl_display::DisplayObject*    as3obj = (AS3::Instances::fl_display::DisplayObject*)as3val.GetObject()
		
		if (proot && avmDispObj )
		{
			AS3::Instances::fl_display::DisplayObject* as3obj = ((AvmDisplayObj*)avmDispObj)->GetAS3Obj();
			const ASString& evtName = proot->GetStringManager()->CreateString(messageType);
			if (as3obj != NULL && as3obj->HasEventHandler(evtName, false))
			{
                SPtr<Instances::fl_gfx::IMEEventEx> evt;
				Value paramsValue[] = { Value(evtName), Value(true), Value(true) };
				ASVM* asvm = proot->GetAVM();
				asvm->ConstructInstance(evt, asvm->GetClass("scaleform.gfx.IMEEventEx", asvm->GetCurrentAppDomain()), 3, paramsValue);
				SF_ASSERT(evt);
				evt->Target = as3obj;
				evt->SetMessage(message);
				as3obj->Dispatch(evt, as3obj->pDispObj);
			}
		}
	}
}

void IMEManager::BroadcastSetSupportedLanguages(const char* pString)
{
	DispatchEvent(pString, "SetSupportedLanguages", "LangBar");
}

void IMEManager::BroadcastSetSupportedIMEs(const char* pString)
{
	DispatchEvent(pString, "SetSupportedIMENames", "LangBar");
}

void IMEManager::BroadcastSetCurrentInputLanguage(const char* pString)
{
	DispatchEvent(pString, "SetCurrentInputLanguage", "LangBar");
}

void IMEManager::BroadcastSetIMEName(const char* pString)
{
	DispatchEvent(pString, "SetIMEName", "LangBar");
}

void IMEManager::BroadcastSwitchLanguage(const char* pString)
{
	DispatchEvent(pString, "SetCurrentLanguage", "StatusWindow");
}

void IMEManager::BroadcastSetConversionStatus(const char* pString)
{
	DispatchEvent(pString, "SetConversionStatus", "StatusWindow");
}

void IMEManager::BroadcastRemoveStatusWindow(const char* pString)
{
	DispatchEvent(pString, "RemoveStatusWindow", "StatusWindow");
}

void IMEManager::BroadcastDisplayStatusWindow(const char* pString)
{
	DispatchEvent(pString, "DisplayStatusWindow", "StatusWindow");
}

void IMEManager::Reset() { pTextField = NULL; pMovie = NULL; }

void IMEManager::BroadcastIMEConversion(const wchar_t* pString)
{
	SF_UNUSED(pString);
/*
	AS2::ValueArray params;
    // Convert to UTF8
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString imeConv = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(imeConv);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),  
			OnBroadcastIMEConversion, &params);
	}
*/
}

Sprite* IMEManager::GetCandidateListSprite()
{
    if (CandListVal.IsNull())
        return NULL;

	AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pMovie));
	AS3::Value pdestVal;
	pmovieRoot->GFxValue2ASValue(CandListVal, &pdestVal);
    if (pdestVal.IsObject())
    {
	    AS3::Object*    parentObj  = (AS3::Object*)(pdestVal.GetObject());
	    if (!AreDisplayObjectTraits(parentObj)) return 0;
	    Sprite* parentDispObj = 
		    static_cast<Sprite*>(
            static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(parentObj)->pDispObj.GetPtr());
	    return parentDispObj;
    }
    return NULL;
}

InteractiveObject* IMEManager::HandleFocus(Movie* pmovie, 
                                           InteractiveObject* poldFocusedItem, 
                                           InteractiveObject* pnewFocusingItem, 
                                           InteractiveObject* ptopMostItem)
{
	
    IMEManagerBase* pimeManagerBase = GetIMEManager();
	
	if (!pimeManagerBase)
		return pnewFocusingItem;

    if (pimeManagerBase->IsMovieActive(pmovie))
    {
	//	AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pMovie));
        // This case pertains to when the focus is transfered to a textfield for the first time before 
        // any ime messages are sent. This provides a safe place to set the candidate list font for the 
        // candidate list sprites.
		
		if (ptopMostItem)
        {
            String path;
            ptopMostItem->GetAbsolutePath(&path);
            if (IsCandidateList(path))
            {
                // prevent currently focused text field from losing focus.
                return poldFocusedItem;
            }

            InteractiveObject* currItem = ptopMostItem;
			AS3::Value val;
            while (currItem != NULL)
            {
				Sprite* sprite		 = currItem->CharToSprite();
				if (sprite)
				{
					AS3::AvmSprite* avmSprite = AS3::ToAvmSprite(sprite);
					AS3::Instances::fl_display::DisplayObject* pobj = avmSprite->GetAS3Obj();
					AS3::Value result;

                    if (pobj)
                    {
                        if (pobj->GetProperty(Multiname(avmSprite->GetAVM()->GetPublicNamespace(), 
                            avmSprite->GetAS3Root()->GetStringManager()->CreateString("IsCandidateList")), result)
                            || pobj->GetProperty(Multiname(avmSprite->GetAVM()->GetPublicNamespace(), 
                            avmSprite->GetAS3Root()->GetStringManager()->CreateString("IsStatusWindow")), result)
                            || pobj->GetProperty(Multiname(avmSprite->GetAVM()->GetPublicNamespace(), 
                            avmSprite->GetAS3Root()->GetStringManager()->CreateString("isLanguageBar")), result))

                        {
                            return poldFocusedItem;
                        }
                    }
				}
				
				/*
                if (avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
                    avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isLanguageBar"), &val)
                    ||
					avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
					avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isStatusWindow"), &val))
                {
                    return poweldFocusedItem;
                }
				*/
                currItem = currItem->GetParent();
            };
        }
		if (pnewFocusingItem && pnewFocusingItem->GetType() == CharacterDef::TextField)
        {
			Sprite* psprite = GetCandidateListSprite();
            if (psprite)
			    ((TextField*)pnewFocusingItem)->SetCandidateListFont(psprite);
        }

        if (poldFocusedItem && 
            poldFocusedItem->GetType() == CharacterDef::TextField)
        {
            if (!pnewFocusingItem)
            {
               

                // finalize, something else was clicked
                pimeManagerBase->DoFinalize();
            }
            else 
            {
                if (pnewFocusingItem->GetType() == CharacterDef::TextField)
                {
                    // clicked on another text field
                }
                // finalize?
                pimeManagerBase->DoFinalize();
            }
        }

     //   DoFinalize();

        bool isIMEEnabled   = false; 
        bool isTextField    = false;    

		if (pnewFocusingItem)
		{
            isTextField  = pnewFocusingItem->GetType() == CharacterDef::TextField;
			isIMEEnabled = isTextField ? static_cast<TextField*>(pnewFocusingItem)->IsIMEEnabled() : false;
		}
        pimeManagerBase->EnableIME(pnewFocusingItem && isIMEEnabled && isTextField);

    }
    return pnewFocusingItem;
	
}


#endif //#ifdef SF_NO_IME_SUPPORT
}}} //namespace Scaleform { namespace GFx {
