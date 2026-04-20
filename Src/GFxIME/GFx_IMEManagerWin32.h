/**************************************************************************

Filename    :   GFx_IMEManagerWin32.h
Content     :   Implementation of Input Method Support on Win32 platforms.
Created     :   OCt 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_IMEWIN32_H
#define INC_IMEWIN32_H

#define GFX_TEST_IME

// GFx includes
#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
//#include "GFx/GFx_RenderConfig.h"
#include <GFx/IME/GFx_IMEManager.h>
#include "GFx_IMEIdMap.h"
#pragma comment(lib, "libgfx_ime")

namespace Scaleform {
	namespace GFx {
		namespace IME{

class GFxIMEWin32Impl;

// Public interface for Input Method Editor.

class GFxIMEManagerWin32 : public IMEManagerBase
{
public:

    /* Constructor */
    GFxIMEManagerWin32(HWND hwnd);

    /* Destructor */
    ~GFxIMEManagerWin32();



    /*************************************************/
    // Implementations of GFxIMEManager's methods
    /************************************************/

    virtual UInt32  Init(Log* plog = 0, FileOpener* pfileOpener = NULL, const char* xmlFileName = NULL, bool bCheckIMEExists = true);

    virtual void	SetActiveMovie(Movie*);

    // handles IME events
    virtual unsigned HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent);

    // handle "imecommand(cmd, param)"
    virtual void    IMECommand(Movie* pmovie, const char* pcommand, const char* parg);

	// handle IMEMessage which uses function object interface and will make "imecommand" redundant
	virtual void ProcessIMEMessage(const FunctionHandler::Params& params);

    /* Finalizes IME composition text, closes any pop ups (candidate list, reading window) */
    virtual void    OnFinalize();

    virtual void    OnShutdown();

    /* Override from GFxIMEManaager. Parses path of the argument to detect if it's a candidatelist
    object or not. */
    virtual bool    IsCandidateList(const char* path);

    /* Override from GFxIMEManaager. Parses path of the argument to detect if it's a status window
    object or not. */
    virtual bool    IsStatusWindow(const char* path);

    virtual bool    IsLangBar(const char* path);

    // Override from GFxIMEManaager. Invoked when candidate list loading is completed.
    virtual void    OnCandidateListLoaded(const char* pcandidateListPath);

    /* Override from GFxIMEManaager. Invoked when SetCandidateListStyle is called
    from Action Script */
    virtual void    OnCandidateListStyleChanged(const IMECandidateListStyle& style);

    virtual void    OnEnableIME(bool enable);

	virtual void SendLangBarMessage(DisplayObject* mc, const ASString& command, const ASString& message);
	
    /*************************************************/
    // Functions for setting IME state
    /************************************************/

    void    Init();

    void    SetWndHandle(HWND hWnd);

    /* Records path to the CandidateList AS object to send invokes */
    void    RecordCandidateListPath(const char* parg);

	/* Clears candidate list path */
	void	ClearCandidateListPath();

    /* Records path to the Status Window AS object to send invokes */
    void    RecordStatusWindowPath(const char* path);


    /* Records path to the Language Bar AS object to send invokes */
    void    RecordLanguageBarPath(const char* path);

    /* Records path to the AS object that handles composition string displays, coloring the text,
    loading up the Candidate List swf file etc. */
    void    RecordIMEControllerPath(const char* parg);

    /* Sets properties for the candidate list.
       param1: Font size for cand list text param2: Font color for regular text
       param3: Font color for highlighted row text.  */
    void    SetCandidateListProperties(INT size, INT fontColor1, INT fontColor2);


    // Handles WM_IME_NOTIFY for candidate list related notifications
    LRESULT OnIMECandidateProcessing(UInt32 message, UInt32 key, UInt32 info, bool downFlag);

    // Sets Input Language
    void    SetInputLanguage(const char* inputLanguage);

    // Sets Conversion Mode
    bool    SetConversionMode(const UInt32 convMode);

    // Retrieves Conversion Mode
    const char*     GetConversionMode();

    // Enables/Disables IME. 
    virtual bool    SetEnabled(bool enable); 

    // Retrieves IME state. 
    virtual bool    GetEnabled(); 

    const	String GetInputLanguage();

    void            SetCurrentInputLanguage(GFxIMETag IMETag);

    virtual bool    SetCompositionString(const char* pCompString);
    /* Handles some WM_NOTIFY messages that get sent when backspace, space, esc keys etc
    are pushed. */
    void            CustomProcessing(UInt32 message, UInt32 key, UInt32 info);

    /*************************************************/
    // Miscellaneous Functions
    /************************************************/
    /* Selects a particular row of the Candidate List (specified by index). Doesn't finalize */
    void            SelectAndClose(int index); // Selects the text in row # and closes Candidate List

     /* Records the last keystroke for reading window display */
    void            PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt);

     /* Takes care of interacting with AS to display reading (input) window */
    void            DisplayReadingWindow(wchar_t* pReadingStr);

    /* Displays composition string, keeps track of how many characters are on the screen, cursor
    position etc */
    void            DisplayCompositionString(UInt32& numCharDisplay);

    /* Checks if we are in a textfield or not. If not, translate message is not called. This
       is to make TAB work correctly when IME is active and focus is on a non textfield object. */
    bool            CheckIfInTextField();

	/*************************************************/
    // Used by the testing framework
	/************************************************/
	String			GetSystemLanguageInfo();

	String			GetWindowsVersion();

    /* Used to switch to a desired IME while testing */
#ifdef GFX_TEST_IME
    bool            SwitchIME(String imeName);

#endif
    // Variables
public:

    HWND            hWnd;
   
    // This class implements Input Method Editor functionality.
    class GFxIMEWin32Impl* pIME;

    class GImeNamesManager* pIMENamesMgr;

    // This variables keeps track of if the candidate list is loaded or not. If not, 
    // it's used to print a message to GFx log. The message is printed only the first
    // time the list is called into use. The variable is reset every time SetActiveMovie 
    // is called.
    bool                    CandListLoaded;

    GFxIMETag               IMETag;
    String					StatusWindowPath;
    String					LanguageBarPath;
    // Used to record the composition string so it can be written to the log if requested 
    // during testing.
    String					CompositionString;
    Movie*					pMovie;
    DWORD					ConversionParams;
    DWORD                   SentenceParams;
    String					CurrentLanguage;
    bool                    ImeOpenStatus;
    bool                    NamesManagerInitStatus;
    friend void             GImeNamesManager::SetLastIMEName(String);
    enum                    OSVersion {WinXP, WinVista, Win2K, Unknown};
    OSVersion               Version;
    int                     ImeVersionId;
    // Detects Windows Version
    virtual void			GetWindowsVersion(OSVersion&);
    
	void					SetIMETag(GFxIMETag imeTag); 

    void                    SetImeVersionId(int versionId); 

    // Used by the test framework
    void                    ResetText();
   

};
}}}
#endif //INC_IMEWIN32_H
