
/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ASIMEManager.h
Content     :   
Created     :   Mar 27, 2008
Authors     :   A. Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
// IME manager interface class. This class may be used as a base class for various
// IME implementations. It also provides utility functions to control composition, 
// candidate list and so on.

#ifndef INC_ASIMEMANAGER_H
#define INC_ASIMEMANAGER_H
#include "GFx/GFx_Player.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_Sprite.h"

namespace Scaleform { namespace GFx {

#ifndef SF_NO_IME_SUPPORT
class ASIMEManager : public RefCountBase<ASIMEManager, StatIME_Mem>
{
public:

	String      CandidateSwfPath;
	String      CandidateSwfErrorMsg;
	Movie*		pMovie;
	// Stores the movie context used in function calls.
	GFx::Value			pLangContext;
	GFx::DisplayObject*	pLangContext2;
	GFx::DisplayObject*	pStatusContext2;
	GFx::Value			pStatusContext;
	Movie*				pMovieContext;
	TextField			*pTextField;

	ASIMEManager() 
	{
		pMovie			= 0;
		pTextField		= 0;
		pLangContext2	= 0;
		pStatusContext2	= 0;
	};

	virtual ~ASIMEManager()
	{
	};
	
	virtual void ShutDown() { pMovie = 0; };
	class IMEFuncHandler : public GFx::FunctionHandler
	{
	public:
		IMEFuncHandler(IMEManagerBase* _pimeMgr): pimeManager(_pimeMgr){}
		// Can't create a body for Call here since we don't have class defn of IMEManagerBase.
		// Function defined in GFx_IMEManager.cpp
		virtual void Call(const FunctionHandler::Params& params);
		IMEManagerBase* pimeManager;
	};
	
	Ptr<ASIMEManager::IMEFuncHandler> CustomFuncCandList;
	Ptr<ASIMEManager::IMEFuncHandler> CustomFuncLanguageBar;

	virtual void ASRootMovieCreated(Ptr<Sprite> spr) {SF_UNUSED(spr);}
	// handle "_global.imecommand(cmd, param)"
	virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
	{ SF_UNUSED3(pmovie, pcommand, pparam); }

	// The functions below will be overridden in AS2/AS3 derived classes to implement
	// the broadcast mechanism accoding to the version of actionscript being used.
	// Support for OnIMEComposition event
	virtual void BroadcastIMEConversion(const wchar_t* pString) { SF_UNUSED(pString);}

	// Support for OnSwitchLanguage event
	virtual void BroadcastSwitchLanguage(const char* pString){ SF_UNUSED(pString); }

	// Support for OnSetSupportedLanguages event
	virtual void BroadcastSetSupportedLanguages(const char* pString){ SF_UNUSED(pString); }

	// Support for OnSetSupportedIMEs event
	virtual void BroadcastSetSupportedIMEs(const char* pString){ SF_UNUSED(pString); }

	// Support for OnSetCurrentInputLanguage event
	virtual void BroadcastSetCurrentInputLanguage(const char* pString){ SF_UNUSED(pString); }

	// Support for OnSetIMEName event
	virtual void BroadcastSetIMEName(const char* pString){ SF_UNUSED(pString); }

	// Support for OnSetConversionStatus event
	virtual void BroadcastSetConversionStatus(const char* pString){ SF_UNUSED(pString); }

	// Support for OnBroadcastRemoveStatusWindow event
	virtual void BroadcastRemoveStatusWindow(const char* pString) { SF_UNUSED(pString);};

	// Support for OnBroadcastDisplayStatusWindow event
	virtual void BroadcastDisplayStatusWindow(const char* pString) { SF_UNUSED(pString); };

	virtual bool IsCandidateListLoaded() const {return false;}

	// Returns true, if text field is currently focused.
	virtual bool IsTextFieldFocused() const {return false;};

	// Returns true, if the specified text field is currently focused.
	bool IsTextFieldFocused(InteractiveObject* ptextfield) const 
	{
		SF_UNUSED(ptextfield); 
		return false;
	};

	virtual bool    Invoke(const char* ppathToMethod, GFx::Value *presult, const GFx::Value* pargs, unsigned numArgs)
	{
		SF_UNUSED4(ppathToMethod, presult, pargs, numArgs);
		return false;
	}

	virtual InteractiveObject* HandleFocus(Movie* pmovie, 
		InteractiveObject* poldFocusedItem, 
		InteractiveObject* pnewFocusingItem, 
		InteractiveObject* ptopMostItem)
	{
		SF_UNUSED4(pmovie, poldFocusedItem, pnewFocusingItem, ptopMostItem);
		return 0;
	}

	virtual void OnOpenCandidateList() {};
	virtual bool AcquireCandidateList() { return false; }
	virtual void SetIMEMoviePath(const char* pcandidateSwfPath) 
	{ 
		SF_UNUSED(pcandidateSwfPath);
	};
	virtual void SetActiveMovie(Movie* pmovie) {pMovie = pmovie;};
	void SetTextFieldPtr(TextField* ptextfield) { pTextField = ptextfield;};
	virtual void SetIMEManager(IMEManagerBase* pimeMgr) {pimeManager = pimeMgr;};

	IMEManagerBase* GetIMEManager() {return pimeManager;};

	IMEManagerBase* pimeManager;

};
#else
class ASIMEManager : public RefCountBase<ASIMEManager, StatIME_Mem>
{
};
#endif // #ifndef SF_NO_IME_SUPPORT
}} // namespace Scaleform { namespace GFx {

#endif //INC_ASIMEMANAGER_H
