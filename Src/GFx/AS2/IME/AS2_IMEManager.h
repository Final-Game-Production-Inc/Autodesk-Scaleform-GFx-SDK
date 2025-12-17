
/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_IMEManager.h
Content     :   IME Manager base functinality
Created     :   Dec 6, 2010
Authors     :   Ankur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS2IMEMANAGER_H
#define INC_AS2IMEMANAGER_H

#include "GFxConfig.h"
#include "GFx/GFx_Loader.h"
#include "Render/Render_Types2D.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "GFx/GFx_ASIMEManager.h"
#include "GFx/AS2/AS2_Value.h"
#ifdef SF_CC_ARM
#include "GFx/AS2/AS2_FunctionRefImpl.h"
#endif
#include "GFx/AS2/IME/GASIme.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2{

#ifndef SF_NO_IME_SUPPORT


// IME manager interface class. This class may be used as a base class for various
// IME implementations. It also provides utility functions to control composition, 
// candidate list and so on.

class IMEManager : public ASIMEManager
{
public:
    friend class MovieImpl;

    unsigned UnsupportedIMEWindowsFlag;
    // handles focus. 
    InteractiveObject* HandleFocus(Movie* pmovie, 
        InteractiveObject* poldFocusedItem, 
        InteractiveObject* pnewFocusingItem, 
        InteractiveObject* ptopMostItem);

    // callback, invoked when mouse button is down. buttonsState is a mask:
    //   bit 0 - right button is pressed,
    //   bit 1 - left
    //   bit 2 - middle
    void OnMouseDown(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);
    void OnMouseUp(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);

    enum 
    {
        GFxIME_AlwaysHideUnsupportedIMEWindows          = 0x01,
        GFxIME_HideUnsupportedIMEWindowsInFullScreen    = 0x02,
        GFxIME_ShowUnsupportedIMEWindows                = 0x03
    } ; 

    Log*		pLog;
	
	String		CandListPath;
    IMEManager();
    virtual ~IMEManager()
	{
	};

	virtual bool    Invoke(const char* ppathToMethod, GFx::Value *presult, const GFx::Value* pargs, unsigned numArgs)
	{
		if (CandListPath.GetLength()) 
		{
			String fullPath = CandListPath + "." + ppathToMethod;
			bool retVal = false;
			if (pMovie)
			{
				retVal = pMovie->Invoke(fullPath, presult, pargs, numArgs);
			}
			return retVal;
		}
		return false;
	}

	bool       Invoke(const char* ppathToMethod, const char* pargFmt, ...)
	{
		if (CandListPath)
		{
			String fullPath = CandListPath + "." + ppathToMethod;
			bool retVal = false;
			if (pMovie)
			{
				va_list args;
				va_start(args, pargFmt);
				retVal = pMovie->InvokeArgs(fullPath.ToCStr(), NULL, pargFmt, args);
				va_end(args);
			}
			return retVal;
		}
		return false;
	}
	
	void SetActiveMovie(Movie* pmovie)
	{
/*
		GFx::Value func;
		AS2::Value asFunc;
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pmovie));
		pmovie->CreateFunction(&func, CustomFuncLanguageBar);
		pmovieRoot->Value2ASValue(func, &asFunc);
		AvmSprite* avmSpr = AS2::ToAvmSprite(pmovieRoot->GetLevel0Movie());
		if (avmSpr)
		{
			avmSpr->SetMember(avmSpr->GetASEnvironment(), pmovieRoot->GetStringManager()->CreateConstString("SendLangBarMessage"), 
				asFunc);
		}
*/
		ASIMEManager::SetActiveMovie(pmovie);
	}
    
    // Returns true, if text field is currently focused.
    bool IsTextFieldFocused() const;

    // Returns true, if the specified text field is currently focused.
    bool IsTextFieldFocused(InteractiveObject* ptextfield) const;

    // Checks if candidate list is loaded
    virtual bool IsCandidateListLoaded() const;
	
	bool SetCandidateListStyle(const IMECandidateListStyle& st);

    // Sets candidate list movie path
    void SetIMEMoviePath(const char* pcandidateSwfPath);

    // Gets candidate list path for logging
    bool GetIMEMoviePath(String& candidateSwfPath);


    void ShowUnsupportedIMEWindows(unsigned unsupportedIMEWindowsFlag) {UnsupportedIMEWindowsFlag = unsupportedIMEWindowsFlag;};

    unsigned GetUnsupportedIMEWindowsFlag(){ return UnsupportedIMEWindowsFlag; };

    /****** Flash IME class functions **************/

    // Support for OnIMEComposition event
    virtual void BroadcastIMEConversion(const wchar_t* pString);

    // Support for OnSwitchLanguage event
    virtual void BroadcastSwitchLanguage(const char* pString);

    // Support for OnSetSupportedLanguages event
    virtual void BroadcastSetSupportedLanguages(const char* pString);

    // Support for OnSetSupportedIMEs event
    virtual void BroadcastSetSupportedIMEs(const char* pString);

    // Support for OnSetCurrentInputLanguage event
    virtual void BroadcastSetCurrentInputLanguage(const char* pString);

    // Support for OnSetIMEName event
    virtual void BroadcastSetIMEName(const char* pString);

    // Support for OnSetConversionStatus event
    virtual void BroadcastSetConversionStatus(const char* pString);

    // Support for OnBroadcastRemoveStatusWindow event
    virtual void BroadcastRemoveStatusWindow(const char* pString);

    // Support for OnBroadcastDisplayStatusWindow event
    virtual void BroadcastDisplayStatusWindow(const char* pString);

    // SetCompositionString
    virtual bool SetCompositionString(const char* pCompString) { SF_UNUSED(pCompString); return false; }; 

    /****** End Flash IME class functions **************/

	static void OnBroadcastSetConversionMode(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSetConversionMode(fn.Env, fn.Arg(0).ToString(fn.Env));
	}

	static void OnBroadcastIMEConversion(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnIMEComposition(fn.Env, fn.Arg(0).ToString(fn.Env));
	}

	static void OnBroadcastSwitchLanguage(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSwitchLanguage(fn.Env, fn.Arg(0).ToString(fn.Env));

	}

	static void OnBroadcastSetSupportedLanguages(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSetSupportedLanguages(fn.Env, fn.Arg(0).ToString(fn.Env));

	}

	static void OnBroadcastSetCurrentInputLanguage(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSetCurrentInputLang(fn.Env, fn.Arg(0).ToString(fn.Env));

	}

	static void OnBroadcastSetSupportedIMEs(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSetSupportedIMEs(fn.Env, fn.Arg(0).ToString(fn.Env));
	}

	static void OnBroadcastSetIMEName(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnSetIMEName(fn.Env, fn.Arg(0).ToString(fn.Env));
	}

	static void OnBroadcastRemoveStatusWindow(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnRemoveStatusWindow(fn.Env);
	}

	static void OnBroadcastDisplayStatusWindow(const FnCall& fn)
	{
		if (!fn.Env)
			return;

		GASIme::BroadcastOnDisplayStatusWindow(fn.Env);
	}
    // Loads candidate list movie, if it wasn't loaded yet. It will invoke
    // OnCandidateListLoaded(path) virtual method once movie is loaded.
    bool AcquireCandidateList();

    // Loads the Status window movie.
    bool AcquireStatusWindowMovie();

    // Loads the Language bar movie.
    bool AcquireLanguageBarMovie();

    // Fills pdest with error message, if candidate list failed to load.
    // Returns 'true' if error occurred.
    bool GetCandidateListErrorMsg(String* pdest);

	bool GetCandidateListStyle(IMECandidateListStyle* pst) const;

    // This callback function is called then the candidate list dialog
    // is ready to be shown. It is used to set the font to show the dialog 
    // from the current text field.
    void OnOpenCandidateList();

    virtual UInt32 Init(Log* plog){ plog = 0; return 1;};
   
	void			Reset(); 

    // *** callbacks, invoked from core. These virtual methods
    // might be overloaded by the implementation.

    // invoked to check does the symbol belong to candidate list or not
    virtual bool IsCandidateList(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to status window or not
    virtual bool IsStatusWindow(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to lang bar window or not
    virtual bool IsLangBar(const char* ppath) { SF_UNUSED(ppath); return false; }

    // handle "_global.imecommand(cmd, param)"
    virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
    { SF_UNUSED3(pmovie, pcommand, pparam); }

    // invoked when need to cleanup and shutdown the IME. Do not invoke ActionScript from it!
    virtual void OnShutdown() {}

};

#else // #ifdef SF_NO_IME_SUPPORT
class IMECandidateListStyle : public NewOverrideBase<Stat_Default_Mem>
{
};

#endif // #ifdef SF_NO_IME_SUPPORT

}}} // namespace Scaleform { namespace GFx {

#endif //INC_IMEMANAGER_H
