/**************************************************************************

Filename    :   GFx_IMEManager.h
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

#ifndef INC_BASEIMEMANAGER_H
#define INC_BASEIMEMANAGER_H
#include "GFxConfig.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_ASIMEManager.h"

namespace Scaleform { namespace GFx {

#ifndef SF_NO_IME_SUPPORT

#define GFX_CANDIDATELIST_LEVEL 9999
#define GFX_CANDIDATELIST_FONTNAME "$IMECandidateListFont"

//class InteractiveObject;
class IMEEvent;
class ASIMEManager;

enum IMEConversionModes
{
	IME_FullShape       = 0x01,
	IME_HalfShape       = 0x02,
	IME_Hanja           = 0x03,
	IME_NonHanja        = 0x04,
	IME_Katakana        = 0x05,
	IME_Hiragana        = 0x06,
	IME_Native          = 0x07,
	IME_AlphaNumeric    = 0x08

};

// A structure used to transfer IME candidate list styles
// from ActionScript function to IME implementation.

class IMECandidateListStyle : public NewOverrideBase<StatMV_Other_Mem>
{
	UInt32 TextColor;
	UInt32 BackgroundColor;
	UInt32 IndexBackgroundColor;
	UInt32 SelectedTextColor;
	UInt32 SelectedBackgroundColor;
	UInt32 SelectedIndexBackgroundColor;
	UInt32 ReadingWindowTextColor;
	UInt32 ReadingWindowBackgroundColor;
	unsigned   FontSize; // in points 
	unsigned   ReadingWindowFontSize; 
	enum
	{
		Flag_TextColor                      = 0x01,
		Flag_BackgroundColor                = 0x02,
		Flag_IndexBackgroundColor           = 0x04,
		Flag_SelectedTextColor              = 0x08,
		Flag_SelectedBackgroundColor        = 0x10,
		Flag_SelectedIndexBackgroundColor   = 0x20,
		Flag_FontSize                       = 0x40,
		Flag_ReadingWindowTextColor         = 0x80,
		Flag_ReadingWindowBackgroundColor   = 0x100,
		Flag_ReadingWindowFontSize          = 0x200
	};
	UInt16  Flags;
public:
	IMECandidateListStyle():Flags(0) {}

	void    SetTextColor(UInt32 color)         { Flags |= Flag_TextColor; TextColor = color; }
	UInt32  GetTextColor() const               { return (HasTextColor()) ? TextColor : 0; }
	void    ClearTextColor()                   { Flags &= (~Flag_TextColor); }
	bool    HasTextColor() const               { return (Flags & Flag_TextColor) != 0; }

	void    SetBackgroundColor(UInt32 backgr)  { Flags |= Flag_BackgroundColor; BackgroundColor = backgr; }
	UInt32  GetBackgroundColor() const         { return (HasBackgroundColor()) ? BackgroundColor : 0; }
	void    ClearBackgroundColor()             { Flags &= (~Flag_BackgroundColor); }
	bool    HasBackgroundColor() const         { return (Flags & Flag_BackgroundColor) != 0; }

	void    SetIndexBackgroundColor(UInt32 backgr) { Flags |= Flag_IndexBackgroundColor; IndexBackgroundColor = backgr; }
	UInt32  GetIndexBackgroundColor() const        { return (HasIndexBackgroundColor()) ? IndexBackgroundColor : 0; }
	void    ClearIndexBackgroundColor()            { Flags &= (~Flag_IndexBackgroundColor); }
	bool    HasIndexBackgroundColor() const        { return (Flags & Flag_IndexBackgroundColor) != 0; }

	void    SetSelectedTextColor(UInt32 color) { Flags |= Flag_SelectedTextColor; SelectedTextColor = color; }
	UInt32  GetSelectedTextColor() const       { return (HasSelectedTextColor()) ? SelectedTextColor : 0; }
	void    ClearSelectedTextColor()           { Flags &= (~Flag_SelectedTextColor); }
	bool    HasSelectedTextColor() const       { return (Flags & Flag_SelectedTextColor) != 0; }

	void    SetSelectedBackgroundColor(UInt32 backgr) { Flags |= Flag_SelectedBackgroundColor; SelectedBackgroundColor = backgr; }
	UInt32  GetSelectedBackgroundColor() const        { return (HasSelectedBackgroundColor()) ? SelectedBackgroundColor : 0; }
	void    ClearSelectedBackgroundColor()            { Flags &= (~Flag_SelectedBackgroundColor); }
	bool    HasSelectedBackgroundColor() const        { return (Flags & Flag_SelectedBackgroundColor) != 0; }

	void    SetSelectedIndexBackgroundColor(UInt32 backgr) { Flags |= Flag_SelectedIndexBackgroundColor; SelectedIndexBackgroundColor = backgr; }
	UInt32  GetSelectedIndexBackgroundColor() const        { return (HasSelectedIndexBackgroundColor()) ? SelectedIndexBackgroundColor : 0; }
	void    ClearSelectedIndexBackgroundColor()            { Flags &= (~Flag_SelectedIndexBackgroundColor); }
	bool    HasSelectedIndexBackgroundColor() const        { return (Flags & Flag_SelectedIndexBackgroundColor) != 0; }

	void    SetFontSize(unsigned fs) { Flags |= Flag_FontSize; FontSize = fs; }
	unsigned    GetFontSize() const  { return (HasFontSize()) ? FontSize : 0; }
	void    ClearFontSize()      { Flags &= (~Flag_FontSize); }
	bool    HasFontSize() const  { return (Flags & Flag_FontSize) != 0; }

	// Reading Window styles.
	void    SetReadingWindowTextColor(unsigned fs) { Flags |= Flag_ReadingWindowTextColor; ReadingWindowTextColor = fs; }
	unsigned    GetReadingWindowTextColor() const  { return (HasReadingWindowTextColor()) ? ReadingWindowTextColor : 0; }
	void    ClearReadingWindowTextColor()      { Flags &= (~Flag_ReadingWindowTextColor); }
	bool    HasReadingWindowTextColor() const  { return (Flags & Flag_ReadingWindowTextColor) != 0; }

	void    SetReadingWindowBackgroundColor(unsigned fs) { Flags |= Flag_ReadingWindowBackgroundColor; ReadingWindowBackgroundColor = fs; }
	unsigned    GetReadingWindowBackgroundColor() const  { return (HasReadingWindowBackgroundColor()) ? ReadingWindowBackgroundColor : 0; }
	void    ClearReadingWindowBackgroundColor()      { Flags &= (~Flag_ReadingWindowBackgroundColor); }
	bool    HasReadingWindowBackgroundColor() const  { return (Flags & Flag_ReadingWindowBackgroundColor) != 0; }

	void    SetReadingWindowFontSize(unsigned fs) { Flags |= Flag_ReadingWindowFontSize; ReadingWindowFontSize = fs; }
	unsigned    GetReadingWindowFontSize() const  { return (HasReadingWindowFontSize()) ? ReadingWindowFontSize : 0; }
	void    ClearReadingWindowFontSize()      { Flags &= (~Flag_ReadingWindowFontSize); }
	bool    HasReadingWindowFontSize() const  { return (Flags & Flag_ReadingWindowFontSize) != 0; }
};

class IMEManagerBase : public State
{
	Ptr<ASIMEManager>	pASIMEManager;
	Movie*              pMovie;
    Loader*             pLoader;
	Ptr<TextField>		pTextField;
	SPInt               CursorPosition;
	String              CandidateSwfPath;
	bool                IMEDisabled;
	Hash<Movie*, Ptr<ASIMEManager> > MovieASIMEManagerHash;
	typedef Hash<Movie*, Ptr<ASIMEManager> > MovieASIMEManagerHashDef; 
protected: 
	bool				bCheckIMEExists;
public:

	// styles of text highlighting (used in HighlightText)
	enum TextHighlightStyle
	{
		THS_CompositionSegment   = 0,
		THS_ClauseSegment        = 1,
		THS_ConvertedSegment     = 2,
		THS_PhraseLengthAdj      = 3,
		THS_LowConfSegment       = 4
	};

	Log*				pLog;

	IMEManagerBase();
	~IMEManagerBase();


	virtual UInt32 Init(Log* plog, FileOpenerBase* fileOpener = NULL, const char* xmlFileName = NULL, bool bcheckIMEExists = true)
	{ 
		SF_UNUSED3(fileOpener, xmlFileName, bcheckIMEExists); 
		bCheckIMEExists = bcheckIMEExists;
		plog = 0; 
		return 1;
	};
	// sets currently focused movie view to IME manager.

    void SetLoader(Loader* ploader) { pLoader = ploader; }
    Loader* GetLoader() { return pLoader; }
	void ASRootMovieCreated(Ptr<Sprite> spr) { if (pASIMEManager) pASIMEManager->ASRootMovieCreated(spr);}
	// handle "_global.imecommand(cmd, param)"
	virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
	{ SF_UNUSED3(pmovie, pcommand, pparam); }

	virtual void ProcessIMEMessage(const FunctionHandler::Params& params)
	{
		SF_UNUSED(params);
	}

	// Handles IME events, calling callbacks and switching states.
	virtual unsigned HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent);

	void SetIMEMoviePath(const char* pcandidateSwfPath)
	{
		CandidateSwfPath = pcandidateSwfPath;
	}
	
	bool       Invoke(const char* ppathToMethod, Value *presult, const Value* pargs, unsigned numArgs)
	{
		ASIMEManager* pASIMEManagerTmp = GetASIMEManager();
		return pASIMEManagerTmp ? pASIMEManagerTmp->Invoke(ppathToMethod, presult, pargs, numArgs): false;
	}

	virtual void    ClearCandidateListPath(){};

	ASIMEManager* GetASIMEManager() {return pASIMEManager;};

    void OnMouseDown(Movie* pmovie, int buttonState, InteractiveObject* pitemUnderMousePtr);
	
    void OnMouseUp(Movie* pmovie, int buttonState, InteractiveObject* pitemUnderMousePtr);

	InteractiveObject* HandleFocus(Movie* pmovie, 
		InteractiveObject* poldFocusedItem, 
		InteractiveObject* pnewFocusingItem, 
		InteractiveObject* ptopMostItem)
	{
		ASIMEManager* pasIMEManager = GetASIMEManager();
		if (pasIMEManager)
		{
			return pasIMEManager->HandleFocus(pmovie, poldFocusedItem, pnewFocusingItem, ptopMostItem);
		}
		return pnewFocusingItem;
	};

	// Returns true, if text field is currently focused.
	bool IsTextFieldFocused() const {return pASIMEManager->IsTextFieldFocused();};
	// Returns true, if the specified text field is currently focused.
	bool IsTextFieldFocused(InteractiveObject* ptextfield) const 
	{
		return pASIMEManager->IsTextFieldFocused(ptextfield);
	}

	 //**** utility methods
    // creates the composition string, if not created yet
    void StartComposition();

    // finalizes the composition string by inserting the string into the
    // actual text. If pstr is not NULL then the content of pstr is being used;
    // otherwise, the current content of composition string will be used.
    void FinalizeComposition(const wchar_t* pstr, UPInt len = SF_MAX_UPINT);

    // clears the composition string. FinalizeComposition with pstr != NULL
    // still may be used after ClearComposition is invoked.
    void ClearComposition();

    // release the composition string, so no further composition string related
    // functions may be used.
    void ReleaseComposition();

    // changes the text in composition string
    void SetCompositionText(const wchar_t* pstr, UPInt len = SF_MAX_UPINT);

    // relocates the composition string to the current caret position
    void SetCompositionPosition();

    // sets cursor inside the composition string. "pos" is specified relative to 
    // composition string.
    void SetCursorInComposition(UPInt pos);

    // turns on/off wide cursor.
    void SetWideCursor(bool = true);

	// Sets conversion mode. Base class version does nothing.
	virtual bool SetConversionMode(const UInt32 convMode){ SF_UNUSED(convMode); return false;};

	// retrieves conversion mode. Base class version does nothing.
	virtual const char* GetConversionMode() {return (const char*)("UNKNOWN"); }; 

	// Enables/Disables IME. 
	virtual bool SetEnabled(bool enable) { SF_UNUSED(enable); return false; }; 

	// Retrieves IME state. 
	virtual bool GetEnabled() { return false; }; 

	// Retrieves current input language
	virtual const String GetInputLanguage() { return String("UNKNOWN");};

	// Support for OnIMEComposition event
	virtual void BroadcastIMEConversion(const wchar_t* pString) 
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
		    pasimemanager->BroadcastIMEConversion(pString);
        }
	}
  
	// Support for OnSwitchLanguage event
	virtual void BroadcastSwitchLanguage(const char* pString)
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager(); 
        if (pasimemanager)
        {
            pasimemanager->BroadcastSwitchLanguage(pString);
        }
	};

	// Support for OnSetSupportedLanguages event
	virtual void BroadcastSetSupportedLanguages(const char* pString)
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastSetSupportedLanguages(pString);
        }
	};

	// Support for OnSetSupportedIMEs event
	virtual void BroadcastSetSupportedIMEs(const char* pString)
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastSetSupportedIMEs(pString);
        }
	};

	// Support for OnSetCurrentInputLanguage event

	virtual void BroadcastSetCurrentInputLanguage(const char* pString)
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastSetCurrentInputLanguage(pString);
        }
	};

	// Support for OnSetIMEName event
	virtual void BroadcastSetIMEName(const char* pString) 
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastSetIMEName(pString);
        }
	};

	// Support for OnSetConversionStatus event
	virtual void BroadcastSetConversionStatus(const char* pString) 
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastSetConversionStatus(pString);
        }
	};

	// Support for OnBroadcastRemoveStatusWindow event
	virtual void BroadcastRemoveStatusWindow()
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastRemoveStatusWindow("");
        }
	};

	// Support for OnBroadcastDisplayStatusWindow event
	virtual void BroadcastDisplayStatusWindow()
	{ 
        ASIMEManager *pasimemanager = GetASIMEManager();
        if (pasimemanager)
        {
            pasimemanager->BroadcastDisplayStatusWindow("");
        }
	};

	virtual void SendLangBarMessage(GFx::DisplayObject* mc, const ASString& command, const ASString& message)
	{ 
		SF_UNUSED3(mc, command, message);
	};

	// SetCompositionString
	virtual bool SetCompositionString(const char* pCompString) { SF_UNUSED(pCompString); return false; }; 

	// GetCompositionString
	virtual const wchar_t* GetCompositionString(); 
	
	void Reset() { pTextField = NULL; pMovie = NULL; }

	virtual void OnShutdown() {}

	// returns view rectangle of currently focused text field
	// and cursor rectangle, both in stage (root) coordinate space.
	// cursorOffset may be negative, specifies the offset from the 
	// actual cursor pos.
	void GetMetrics(RectF* pviewRect, RectF* pcursorRect, int cursorOffset = 0);

	// highlights the clause in composition string.
	// Parameter "clause" should be true, if this method is called to highlight
	// the clause (for example, for Japanese IME). In this case, in addition to the
	// requested highlighting whole composition string will be underline by single
	// underline.
	void HighlightText(UPInt pos, UPInt len, TextHighlightStyle style, bool clause);

	// Finalize the composition and release the text field.
	void DoFinalize();

	// invoked when need to finalize the composition.
	virtual void OnFinalize() {}

	// Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
	bool SetCandidateListStyle(const IMECandidateListStyle& st);

	// Gets style of candidate list
	bool GetCandidateListStyle(IMECandidateListStyle* pst) const;

	bool GetCandidateListErrorMsg(String* pdest);

	// invoked when candidate list's style has been changed by ActionScript
	virtual void OnCandidateListStyleChanged(const IMECandidateListStyle& style) { SF_UNUSED(style); }

	// invoked when candidate list loading is completed.
	virtual void OnCandidateListLoaded(const char* pcandidateListPath) { SF_UNUSED(pcandidateListPath); };

	// invoked when ActionScript is going to get candidate list style properties.
	// The default implementation just retrieves styles from movie view.
	virtual void OnCandidateListStyleRequest(IMECandidateListStyle* pstyle) const;

	bool IsCandidateListLoaded()
	{
		if (pASIMEManager)
		{
			return GetASIMEManager()->IsCandidateListLoaded();
		}
		return false;
	}
	
	// enables/disables IME
	void EnableIME(bool enable);

	// handles enabling/disabling IME, invoked from EnableIME method
	virtual void OnEnableIME(bool enable) { SF_UNUSED(enable); }

	// sets currently focused movie view to IME manager.
	virtual void SetActiveMovie(Movie* pmovie);

	virtual bool IsMovieActive(Movie* pmovie) const;

	virtual Movie* GetActiveMovie() const ;
	// cleans out the movie from the IME. NO ACTIONSCRIPT should be used
	// with movie being cleaned up! As well as, do not save it in Ptr.
	virtual void ClearActiveMovie();

	virtual String			GetWindowsVersion() {return "Unknown";}

	// Used to decide if we should check whether the candidate list file exists in the directory of the
	// current swf or not. Some users have their own packaging system, so they want the FileOpener to still
	// be invoked whether or not the candidate list file exists or not so that they can take appropriate action
	// At the same time, for the purposes of our demo's, we don't want to attempt to load the file, if it doesn't 
	// exist to prevent unseemly error messages.
	bool		CheckCandListExists() { return bCheckIMEExists;}
};

inline void                 StateBag::SetIMEManager(IMEManagerBase *ptr)       
{ 
    SetState(State::State_IMEManager, ptr); 
    ptr->SetLoader((Loader*)this);
}

inline Ptr<IMEManagerBase>  StateBag::GetIMEManager() const                   
{ 
    return *(IMEManagerBase*) GetStateAddRef(State::State_IMEManager); 
}

//DOM-IGNORE-BEGIN
#else // #ifndef SF_NO_IME_SUPPORT
// inline stub for GFxIMEManager
class IMEManagerBase : public State
{
public:
	IMEManagerBase();
};

class IMECandidateListStyle : public NewOverrideBase<StatMV_Other_Mem>
{
};
#endif // #ifndef SF_NO_IME_SUPPORT
}} // namespace Scaleform { namespace GFx {
//DOM-IGNORE-END
#endif //INC_BASEIMEMANAGER_H
