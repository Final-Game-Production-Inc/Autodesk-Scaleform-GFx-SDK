/**************************************************************************

Filename    :   AS2_IMEManager.cpp
Content     :   IME Manager base functinality
Created     :   Dec 17, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//#include <AS2/IME/AS2_IMEManager.h>

#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_MovieClipLoader.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS2/AS2_AvmSprite.h"

#include "GFx/AS2/IME/AS2_IMEManager.h"
#include "GFxConfig.h"

namespace Scaleform { namespace GFx { namespace AS2{

#ifndef SF_NO_IME_SUPPORT

IMEManager::IMEManager(): ASIMEManager()
{ 
    UnsupportedIMEWindowsFlag = GFxIME_ShowUnsupportedIMEWindows;
	pTextField	= 0;
	pMovie		= 0;
}

bool IMEManager::IsCandidateListLoaded() const
{
	// check if we already have loaded candidate list into _level9999
	if (pMovie)
	{
		Scaleform::GFx::Value v;
		MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		if (!pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
			v.SetNumber(0);
		return pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL) && v.GetNumber() == 2;
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

    FontResource* pfont = ptextFld->GetFontResource();
    if (!pfont)
        return;
	GFx::Value v;
    if (!pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
        v.SetNumber(0);
    
	AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
	Sprite* imeMovie = pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL);
    if (!imeMovie || v.GetNumber() != 2)
        return;
    imeMovie->SetIMECandidateListFont(pfont);

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
    candidateSwfPath = CandidateSwfPath;
    return !candidateSwfPath.IsEmpty();
}

namespace
{
	class CandidateListLoader : public AS2::MovieClipLoader
    {
        Ptr<IMEManager>      pASIMEManager;
   
    protected:
#ifdef GFX_AS_ENABLE_GC
        virtual void Finalize_GC()
        {
            pASIMEManager = NULL;
            AS2::MovieClipLoader::Finalize_GC();
        }
#endif // SF_NO_GC

    public:
        CandidateListLoader(IMEManager* pimeManager,  AS2::Environment* penv) 
            : MovieClipLoader(penv), pASIMEManager(pimeManager) {}

        void NotifyOnLoadComplete(AS2::Environment* penv, InteractiveObject* ptarget, int status)
        {
            SF_UNUSED(status);
            if (ptarget)
            {
                AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pASIMEManager->pMovie));
                AvmSprite* avmSpr = AS2::ToAvmSprite(ptarget);
                GFx::Value func;
                AS2::Value asFunc;
                pASIMEManager->pMovie->CreateFunction(&func, pASIMEManager->CustomFuncCandList);
                pmovieRoot->Value2ASValue(func, &asFunc);
                avmSpr->SetMember(penv, pmovieRoot->GetStringManager()->CreateConstString("SendIMEMessage"), 
                    asFunc);
            }
        }

		void NotifyOnLoadInit(AS2::Environment* penv, InteractiveObject* ptarget)
        {
            SF_UNUSED(penv); 
			IMEManagerBase* pbaseIMEManager = pASIMEManager->GetIMEManager();
            if (pASIMEManager->pMovie)
            {
				GFx::Value v;
                v.SetNumber(2); // indicates - "loaded"
                pASIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
                v.SetString(pASIMEManager->CandidateSwfPath.ToCStr());
                pASIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_path", v);
            }

            if (ptarget)
            {
                ptarget->GetResourceMovieDef()->PinResource(); // so, do pin instead
                String path;
                ptarget->GetAbsolutePath(&path);
				pASIMEManager->CandListPath = path;
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
            pASIMEManager->CandidateSwfErrorMsg = "Error in loading candidate list from ";
            pASIMEManager->CandidateSwfErrorMsg += pASIMEManager->CandidateSwfPath;
            if (pASIMEManager->pMovie)
            {
                String level0Path;
				AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pASIMEManager->pMovie));
                pmovieRoot->GetLevel0Path(&level0Path);
                pASIMEManager->CandidateSwfErrorMsg += " at ";
                pASIMEManager->CandidateSwfErrorMsg += level0Path;

				GFx::Value v;
                v.SetNumber(-1); // means - "failed to load"
                pASIMEManager->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
            }
            pASIMEManager->CandidateSwfErrorMsg += ": ";
            pASIMEManager->CandidateSwfErrorMsg += errorCode;
        }
    };
}

bool IMEManager::AcquireCandidateList()
{
	AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));

    if (pMovie && pmovieRoot->GetLevelMovie(0))
    {
        // check if we already have loaded candidate list into _level9999
        // or if we are loading it at the time. We can't store bool inside the 
        // IME Manager since candidate lists are different for different movies,
        // but IME Manager could be the same.
		GFx::Value v;
        if (!pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
            v.SetNumber(0);
        if (v.GetNumber() < 0)
            return false;

		//We need reload the candidate list	for the case user use both AS2 and AS3 movie
        if (/*!pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL) &&*/ v.GetNumber() != 1)
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

			GFx::Value va;
            va.SetNumber(1); // means - "loading"
            pMovie->SetVariable("_global.gfx_ime_candidate_list_state", va);
            
			AS2::GFxAS2LoadQueueEntry* pentry = new AS2::GFxAS2LoadQueueEntry
				(GFX_CANDIDATELIST_LEVEL, CandidateSwfPath, AS2::GFxAS2LoadQueueEntry::LM_None, false, true);
			Ptr<AS2::MovieClipLoader> clipLoader = *new CandidateListLoader
                (this, pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment());
            pentry->MovieClipLoaderHolder.SetAsObject(clipLoader);
            pmovieRoot->AddMovieLoadQueueEntry(pentry);
            return false;
        }
        return true;
    }
    return false;
}



void IMEManager::BroadcastSwitchLanguage(const char* pString)
{
	// We have to set up this queing since advance on the actionscript queue is called from a different
	// thread. 

	AS2::ValueArray params;
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString compString = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(compString);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
			OnBroadcastSwitchLanguage, &params);
	}
}

void IMEManager::BroadcastSetSupportedLanguages(const char* pString)
{
	AS2::ValueArray params;

	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString supportedLanguages = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(supportedLanguages);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
				OnBroadcastSetSupportedLanguages, &params);
	}
}

void IMEManager::BroadcastSetSupportedIMEs(const char* pString)
{
	AS2::ValueArray params;

	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString supportedIMEs = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(supportedIMEs);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),  
				OnBroadcastSetSupportedIMEs, &params);
	}
}

void IMEManager::BroadcastSetCurrentInputLanguage(const char* pString)
{
	AS2::ValueArray params;
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString currentInputLanguage = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(currentInputLanguage);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
			OnBroadcastSetCurrentInputLanguage, &params);
	}   
}

void IMEManager::BroadcastSetIMEName(const char* pString)
{
	AS2::ValueArray params;
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString imeName = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(imeName);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),
			OnBroadcastSetIMEName, &params);
	}   
}

void IMEManager::BroadcastSetConversionStatus(const char* pString)
{
	AS2::ValueArray params;
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		ASString convStatus = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
		GFx::AS2::Value val; val.SetString(convStatus);
		params.PushBack(val);
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),
			OnBroadcastSetConversionMode, &params);
	}   
}

void IMEManager::BroadcastRemoveStatusWindow(const char* pString)
{
	SF_UNUSED(pString);
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
			OnBroadcastRemoveStatusWindow, 0);
	}
}

void IMEManager::BroadcastDisplayStatusWindow(const char* pString)
{
	SF_UNUSED(pString);
	if (pMovie)
	{
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
		pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),  
			OnBroadcastDisplayStatusWindow, 0);
	}
}

void IMEManager::Reset() { pTextField = NULL; pMovie = NULL; }

void IMEManager::BroadcastIMEConversion(const wchar_t* pString)
{
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
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
        // This case pertains to when the focus is transfered to a textfield for the first time before 
        // any ime messages are sent. This provides a safe place to set the candidate list font for the 
        // candidate list sprites.
        if (pnewFocusingItem && pnewFocusingItem->GetType() == CharacterDef::TextField)
        {
            FontResource* pfont = ((TextField*)pnewFocusingItem)->GetFontResource();
			GFx::Value v;
            if (pfont)
            {
                if (!pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
                    v.SetNumber(0);
                Sprite* imeMovie = pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL);
                if (imeMovie && v.GetNumber() == 2)
                    imeMovie->SetIMECandidateListFont(pfont);
            }
        }

		if (poldFocusedItem && 
			poldFocusedItem->GetType() == CharacterDef::TextField)
		{

			// empty, check for candidate list. Need to use ptopMostItem, since
			// pnewFocusingItem is NULL
			// check, if the newly focused item candidate list or not
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
				AS2::Value val;
				while (currItem != NULL)
				{
					Sprite* sprite		 = currItem->CharToSprite();
					// currItem could be a textfield, in which case sprite pointer will be null
					if (sprite == NULL) break;
					AS2::AvmSprite* avmSprite = AS2::ToAvmSprite(sprite);
					if (avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
						avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isLanguageBar"), &val)
						||
						avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
						avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isStatusWindow"), &val))
					{
						return poldFocusedItem;
					}
					currItem = currItem->GetParent();
				};

				if (IsStatusWindow(path))
				{
					// prevent currently focused text field from losing focus.
					return poldFocusedItem;
				}

				if (IsLangBar(path))
				{
					// prevent currently focused text field from losing focus.
					return poldFocusedItem;
				}

				// finalize, something else was clicked
				pimeManagerBase->DoFinalize();
			}
            else 
            {
				// clicked from one textfield to the other or clicked on the stage etc.
                // finalize?
				if (!pnewFocusingItem)
				{
					return pnewFocusingItem;
				}
             //   pimeManagerBase->DoFinalize();
            }
        }

     //   DoFinalize();

        pimeManagerBase->EnableIME(pnewFocusingItem && 
            pnewFocusingItem->GetType() == CharacterDef::TextField &&
            static_cast<TextField*>(pnewFocusingItem)->IsIMEEnabled());

    }
    return pnewFocusingItem;
}


#else //#ifdef SF_NO_IME_SUPPORT

// just to avoid warning "LNK4221: no public symbols found; archive member will be inaccessible" 
// IMEManager::IMEManager() : State(State_IMEManager) 
// { 
// }

#endif //#ifdef SF_NO_IME_SUPPORT
}}} //namespace Scaleform { namespace GFx {
