
/**************************************************************************

Filename    :   GFx_IMEVista.cpp
Content     :   Implementation of Input Method Support on Windows Vista using TSF 
Created     :   OCt 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFxConfig.h"
#ifndef SF_NO_IME_SUPPORT

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include "GFx_IMEWin32Impl.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx_IMETSF.h"
#include "GFx_IMEManagerWin32.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

typedef struct tagLAYOUTORTIPPROFILE {
    DWORD  dwProfileType;       // InputProcessor or HKL
#define LOTP_INPUTPROCESSOR 1
#define LOTP_KEYBOARDLAYOUT 2
    LANGID langid;              // language id
    CLSID  clsid;               // CLSID of tip
    GUID   guidProfile;         // profile description
    GUID   catid;               // category of tip
    DWORD  dwSubstituteLayout;  // substitute hkl
    DWORD  dwFlags;             // Flags
    WCHAR  szId[MAX_PATH];      // KLID or TIP profile for string
} LAYOUTORTIPPROFILE;

typedef UInt32  (WINAPI *PTF_ENUMENABLEDLAYOUTORTIP)(LPCWSTR pszUserSidString, LAYOUTORTIPPROFILE *pLayoutOrTipProfile,  
  UInt32 uBufLength);

GFxIMEVista::GFxIMEVista(IMEManagerBase* pbase, HWND hwnd):GFxIMEWin32Impl(pbase, hwnd)
{
    CurrLocale              = NULL;
    NumCharDisplay          = 0;
    ReadingTextBuffer.Resize(ReadingTextBufferSize);
    ReadingTextBuffer[0]    = 0;
    ClausePosition          = 0;
    CandListStartFrom1      = 1;
    pTSF                    = new CTSF(this, hwnd);
    pTSF->Init();

	CandListOpen = false;	// zhaojun@snda
	TypingStringBuffer.Clear();	// zhaojun@snda
}

GFxIMEVista::~GFxIMEVista()
{
    
    pTSF->UnInitUIElementSink();
    pTSF->UninitThreadMgrSink();

    pTSF->ReleaseInterfaces();
    if(pTSF != NULL)
       pTSF->Release();
    
    
}

void GFxIMEVista::FsCallBack(Movie* pmovie, const char* pcommand, const char* parg)
{
    CmdManager.FsCallback(pmovie, pcommand, parg);
}



void GFxIMEVista::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{
	// Special processing for the shift key:
	/*
	Here's why this is needed. The shift key can be used to toggle between english input and ime enabled input. 
	On Vista, when the shift key is pushed, a WM_KEYDOWN with VK_SHIFT is sent, followed by a bunch of IME notify messages
	to change the input mode. The Virtual code for the corresponding WM_KEYUP message however, is not VK_SHIFT but 
	VK_PROCESSKEY. This is a problem because GFx relies on the correct virtual keycode in the WM_KEYUP to reset the 
	shift state- if it doesn't get the right message, it thinks that shift is still down. To alleviate this problem,
	we track WM_KEYDOWN with VK_SHIFT and if we don't see the correct VK for the KEY_UP message, we create the right 
	keyevent ourself.

	*/
	if ((winEvt.Message == WM_KEYDOWN))	// zhaojun@snda
	{
		UINT s = ImmGetVirtualKey(hWND);
		if ((s >= 'a' && s <='z')
			|| (s >= 'A' && s <='Z'))
		{
			TypingStringBuffer.AppendChar(s);// zhaojun@snda
		}

		if (s == VK_BACK)
		{
			TypingStringBuffer = TypingStringBuffer.Substring(0, TypingStringBuffer.GetSize() - 1);
		}
	}

	if ((winEvt.Message == WM_KEYDOWN) && (winEvt.WParam == VK_SHIFT))
	{
		Track           = true; // start the track
		ImeMessageRecd  = false;
	}

	if ((winEvt.Message == WM_KEYUP) && (Track == true) && (winEvt.WParam == VK_PROCESSKEY))
	{
		KeyEvent ev(KeyEvent::KeyUp, Key::Shift, 0, 0);
		if (pMovie)
			pMovie->HandleEvent(ev);	
	}

	// This is used to solve the following problem:
	/*
	On Vista, we use TSF sinks to obtain the candidate list and reading string data and IME messages for drawing the composition string,
	setting the cursor position etc. Now, there doesn't seem to be any safe order for these events. In particular, the following may happen:

	1) User types a key which causes the TSF event requesting a candidate list to be drawn to arrive. The StartComposition message has not 
	yet arrived. This causes the candidate list to be drawn to a bogus position since the composition string related bookkeeping inside 
	the imemanager is done in response to the startcomposition message. If the current cursor position coordinates are requested earlier
	than when then composition string has been initialized, bogus values are returned. 

	2) It's good to have the candidate list to appear below the current cursor position in the composition string - this way if the user
	changes the cursor position, the candidate list always stays below it, leading to a more natural IME user experience. The problem however 
	is that the IME message containing the cursor position arrives after the candidate list notification from TSF. This means that we can either
	draw the list when the TSF candidate notification arrives and then redraw the list when the cursor position is available. This works, but 
	causes annoying flickering as well as requires unnecessary drawing. 

	3) So why not just draw the candidate list when the IMEComposition message is received you ask? This will also take care of (1), since
	IMEComposition is always received after StartComposition. The problem is that not every candidate list notification is followed by 
	a IMEComposition mesasge. For example, if using QuanPin IME, if you type "ga" and then erase the "a", only a candidate list TSF notification
	is reveived, not IMEComposition message is received. This makes sense, since there was no change in the composition. Another example is when 
	a secondary candidate list is opened in DaYi or Array IME's. 

	This is really a very annoying problem and there seem no easy and elegant solutions besides having switches for different IME names leading to 
	code maintainance nightmare. After through research, the solution I've found is to set a flag indicating that candidatelist data is available for 
	drawing when the corresponding TSF notification is received, but save the drawing for later. The safest time to draw any pending candidate data is 
	when the KeyUp message arrives since it's guaranteed (I've never seen it happen otherwise) to arrive after all the IME messages and TSF notifications
	produced by the keystroke have been processed.

	8/27/2010: Just discovered that the above is not true for Japanese Office IME 2010. With this IME, the TSF notifications
	arrive AFTER the IME messages and Key_UP, so candidate list repositioning must be done during TSF notification handling.
	
	Similar logic applies to WM_LBUTTONUP. It's used to han

	*/

	if (winEvt.Message == WM_KEYUP || winEvt.Message == WM_LBUTTONUP)
	{
		if (DrawCandidateList)
		{
			RepositionCandidateList(-ClausePosition);
			DrawCandidateList = false;
		}
	}
	/*
	The logic below deals with removing the "secondary candidate list" in Dayi, Array and Boshiamy IMEs.
	What is a secondary candidate list? Using Array IME, type "t" (candidate list will open) and then space.
	Another candidate list will open. This is called the secondary candidate list. What's special about it?
	In wordpad, open secondary candidate list and type left/right arrow key. Candidate list will stay where it 
	is and the cursor will move. Candidate list doesn't get cancelled. In order to close composition, user can 
	1) either click on a candidate list row 2) use shift+number key corresponding to the candidate list row 3) escape
	out. The trouble is that we need to cancel the candidate list if the user uses left/right arrow key or return key etc
	to change the position of the cursor because we always want to finalize the state of the composition before any 
	changes in the cursor (not caused by IME) take place. Failing to do so will lead to "dangling candidate list" where
	the candidate list is just hanging over the UI not connected to anything. When the secondary list is open and the
	user pushes left/right arrow key, IME doesn't send us any information- it doesn't even set the VK to VK_PROCESSKEY
	therefore, the only option is to hardcode the desired behaviour as done below. 
	*/
	if (IMETag == GFxIME_Ch_Trad_WuXia || IMETag == GFxIME_Ch_Trad_DaYi || IMETag == GFxIME_Ch_Trad_Array
		|| IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Ch_Simp_WuBi98)
	{
		if ((winEvt.Message == WM_KEYDOWN) &&
			(winEvt.WParam == VK_LEFT) ||
			(winEvt.WParam == VK_RIGHT)||
			(winEvt.WParam == VK_UP)||
			(winEvt.WParam == VK_DOWN)||
			(winEvt.WParam == VK_RETURN)
			)
		{
			CloseIME();
		}
	}
//    HWND    hwndIME      = ImmGetDefaultIMEWnd(hWND);
//    ShowOwnedPopups(hwndIME, false);
}

UInt32 GFxIMEVista::OnIMEStart(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED(downFlag);
    SF_UNUSED(info);
    SF_UNUSED(key);

	Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));

	StartCompositionReceived = true;

    // Important Note: If Dayi 6.0 IME is used, StartComposition is called after the candidate
    // list is opened and there has to be special handling for this case in IMEManager. 

    pimeMgr->StartComposition();
	
	StartCompositionReceived = true;
    // Record current system locale- This is used to determine which IME is active
    CurrLocale = GetKeyboardLayout(0);

    HIMC hIMC = ImmGetContext(hWND);

    // Determine if the candidate list should start from 1 or 0
    int prop = ImmGetProperty(CurrLocale, IGP_PROPERTY);
    CandListStartFrom1 = (prop & IME_PROP_CANDLIST_START_FROM_1) == 0? 0: 1;

	if (IMETag == GFxIME_Ch_Trad_WuXia || IMETag == GFxIME_Ch_Trad_DaYi)
    {
        // For this IME, IMM reports the starting index of the candidate list incorrectly, so manually set it here.
        CandListStartFrom1 = 0;
    }

	if (IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Ch_Simp_WuBi98)
	{
		// For this IME, IMM reports the starting index of the candidate list incorrectly, so manually set it here.
		CandListStartFrom1 = 1;
	}
    ///////////////////////////////
   
	CANDIDATEFORM candForm;
	COMPOSITIONFORM compForm;	// This comes in handy to hide the Japanese IME composition window.

	POINT pt; pt.x = -1000; pt.y = -1000;
	RECT rect; rect.top = 0; rect.left = 0; rect.bottom = 10; rect.right = 10;

	candForm.dwIndex		= 0;
	candForm.dwStyle		= CFS_CANDIDATEPOS;
	candForm.ptCurrentPos	= pt;

	compForm.dwStyle        = CFS_FORCE_POSITION;
	compForm.ptCurrentPos   = pt;

	if (IMETag == GFxIME_NotSupported)
	{
		
		if (pMovie)
		{
			Ptr<IMEManagerBase> pimeMgrCaret = pMovie->GetIMEManager();
			SF_ASSERT(pimeMgrCaret && pimeMgrCaret->IsMovieActive(pMovie));
			RectF viewRect, caretRect;
			RectF caretRectTransformed;

			if (pimeMgrCaret)
			{
				pimeMgrCaret->GetMetrics(&viewRect, &caretRect, 0);
				caretRectTransformed = ((MovieImpl*)(pMovie))->TranslateToScreen(caretRect);
				
				pt.x = (LONG)caretRectTransformed.TopLeft().x;
				pt.y = (LONG)caretRectTransformed.BottomRight().y;
			}

			candForm.ptCurrentPos	= pt;
			compForm.ptCurrentPos	= pt;
		}
	}

	ImmSetCompositionWindow(hIMC, &compForm);   
	ImmSetCandidateWindow(hIMC, &candForm);
	ImmReleaseContext(hWND, hIMC);

	if (IMETag == GFxIME_NotSupported) return  Movie::HE_NotHandled;

	return Movie::HE_Handled;

   
}

void GFxIMEVista::OnIMEStop(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    pimeMgr->SetWideCursor(false);
    pimeMgr->ClearComposition();
	StartCompositionReceived = false;

	TypingStringBuffer.Clear();	// zhaojun@snda

}

// Processes candidate list notifications.

LRESULT GFxIMEVista::OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED(downFlag);
    if (!pMovie)
        return 0;
    Value val;


    if (!pMovie) return 0;

    switch (key)
    {
    case IMN_CHANGECANDIDATE:
        {
			if ( IMETag == GFxIME_Kr_2003 || IMETag == GFxIME_Ch_Simp_ABC || IMETag == GFxIME_Ch_Simp_WuBi98 || 
				IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Jp_ATOK2008 || IMETag == GFxIME_Jp_ATOK2009
				|| IMETag == GFxIME_Jp_GOOG2010
			/*	|| IMETag == GFxIME_Jp_2010 */)
            {
				if (!CandListOpen)
				{
					// zhaojun@snda
					OnOpenCandidate(0);
				}
                OnChangeCandidate(0);
                break;
            }    
			if ( IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_GooglePinyin|| IMETag == GFxIME_Ch_Simp_QQPinyin || IMETag == GFxIME_Ch_Simp_Other)
			{
				// We return NoDefaultAction, since if we pass this message to defwndproc, the def candidate list will pop up
				// (tested on Windows 7, Sogou Pinyin 5.0). So far I haven't seen any negative consequences of doing this on
				// the functioning of the candidate list. In case you see some unexpected problems with the candidate list
				// on Win Vista/7, try uncommenting this.
				if (!CandListOpen)
				{
					// zhaojun@snda
					OnOpenCandidate(0);
				}
				OnChangeCandidate(0);
				return Movie::HE_NoDefaultAction;
			}
			break;
        }

    case IMN_OPENCANDIDATE:
        {
		//	if (DrawCandidateList)
		//		RepositionCandidateList(-ClausePosition);

        
            if (IMETag == GFxIME_Kr_2003 || IMETag == GFxIME_Ch_Simp_ABC || IMETag == GFxIME_Ch_Simp_WuBi98 || 
				 IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Jp_ATOK2008 || IMETag == GFxIME_Jp_ATOK2009 ||
				 IMETag == GFxIME_Jp_GOOG2010 || IMETag == GFxIME_Ch_Simp_QQPinyin
			/*	 || IMETag == GFxIME_Jp_2010 */)
            {
                OnOpenCandidate(0);   
				
				// See comment above.
			//	return MovieView::HE_NoDefaultAction;
				break;
            }
			if ( IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_Ch_Simp_QQPinyin || IMETag == GFxIME_Ch_Simp_Other)
			{
				OnOpenCandidate(0);
				return Movie::HE_NoDefaultAction;
			}
			break;
        }

    case IMN_CLOSECANDIDATE:
        if ( IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_Kr_2003 || IMETag == GFxIME_Ch_Simp_ABC || IMETag == GFxIME_Ch_Simp_WuBi98 || 
			 IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Jp_ATOK2008 || IMETag == GFxIME_Jp_ATOK2009
		  || IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_Jp_GOOG2010/*|| IMETag == GFxIME_Jp_2010*/
		  || IMETag == GFxIME_Ch_Simp_QQPinyin || IMETag == GFxIME_Ch_Simp_Other)
        {
            ListBox->RemoveAllListItems();
        //    if (CandListPath.GetLength() > 0)
			{
				Value result;
				Invoke("RemoveList", &result, 0, 0);
				CandListOpen = false;	// zhaojun@snda
			}
        }
        return DefWindowProc(hWND, message, key, info);
    }

    CustomProcessing(message, key, info);
    return 0;

}

void    GFxIMEVista::OnChangeCandidate(int candListIndex)
{
    HIMC        hIMC = 0;
    DWORD       dwSize;
    Value    val;
    LPCANDIDATELIST lpCandList = NULL;

    hIMC = ImmGetContext(hWND);

    dwSize = ImmGetCandidateList(hIMC, candListIndex, lpCandList, 0);
    if (dwSize == 0) // GetCandidateList failed
        return ;

    lpCandList = (CANDIDATELIST*)SF_ALLOC( dwSize*sizeof(CANDIDATELIST), StatIME_Mem);
    dwSize = ImmGetCandidateList(hIMC, candListIndex, lpCandList, dwSize);

	//@tangbinjie
	if ((int)lpCandList->dwPageSize != NumCandidatesOnListPrev)
	{
		OnOpenCandidate(0);
		return;
	}	
	//@tangbinjie

    NumCandidates       = lpCandList->dwCount ;    // Number of candidates
    int currSelection   = lpCandList->dwSelection; // Currently selected row
 //   int pageSize        = lpCandList->dwPageSize;  // Size of a page
    CurrentPageStart    = lpCandList->dwPageStart;

    ImmReleaseContext(hWND, hIMC);

    if (NumCandidates ==  0)
    {
		//@tangbinjie
		Value result;
		Invoke("RemoveList", &result, 0, 0);
		CandListOpen = false;
		//@tangbinjie

        SF_FREE(lpCandList);
        return;
    }

    ListBox->RemoveAllListItems();

    for (int i = 0; i < NumCandidates; i++)
    {
		// oscar@snda
		if(i + CurrentPageStart >= NumCandidates)
		{
			continue;
		}
		// oscar@snda

        wchar_t* rowData = (wchar_t*)(((char*)(lpCandList))+lpCandList->dwOffset[i+CurrentPageStart]);
        int utf8Len = (int)UTF8Util::GetEncodeStringSize(rowData, -1);
        char* rowDataUTF8 = (char*)SF_ALLOC(utf8Len + 1, StatIME_Mem);
        UTF8Util::EncodeString(rowDataUTF8, rowData, -1);
        rowDataUTF8[utf8Len] = 0;
        Ptr<CandidateListItem> pItem  = *new CandidateListItem(rowDataUTF8);
        ListBox->AddListItem(pItem, false); // Only copy as much data as necessary.
        SF_FREE(rowDataUTF8);    
    }

    if(lpCandList)
        SF_FREE(lpCandList);

    ImmReleaseContext(hWND, hIMC);

    // The row selection only needs to work for certain IME's- the others don't 
    // support selecting rows by pushing down/up arrow key 
    if ( IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_Jp_ATOK2008 
		|| IMETag == GFxIME_Jp_ATOK2009 || IMETag == GFxIME_Jp_GOOG2010 || IMETag == GFxIME_Ch_Simp_Other)    //  Sogou Pinyin IME
    {
        ListBox->SetSelectedItemIndex(max(0, (currSelection - CurrentPageStart)));
    }
    else
    {
        ListBox->SetSelectedItemIndex(-1);
    }

    ListBox->UIRefreshView();
	DrawCandidateList = true;
 //   RepositionCandidateList(-ClausePosition);     
  
}

void    GFxIMEVista::OnOpenCandidate(int candListIndex)
{
    HIMC    hIMC = 0;
    DWORD   dwSize;
    LPCANDIDATELIST lpCandList = NULL;
    Value val;

    hIMC = ImmGetContext(hWND);

    // first obtain size of the list
    dwSize = ImmGetCandidateList(hIMC, candListIndex, lpCandList, 0);
    if (dwSize == 0) // GetCandidateList failed
        return ;
    lpCandList = (CANDIDATELIST*)SF_ALLOC( dwSize*sizeof(CANDIDATELIST), StatIME_Mem);
    // Now obtain list data
    dwSize       = ImmGetCandidateList(hIMC, candListIndex, lpCandList, dwSize);

    NumCandidates       = lpCandList->dwCount ;
    int currSelection   = lpCandList->dwSelection;
    CurrentPageStart    = lpCandList->dwPageStart;
    int pageSize        = lpCandList->dwPageSize;
  
    ImmReleaseContext(hWND, hIMC);

    if (NumCandidates ==  0)
    {
		//@tangbinjie
		Value result;
		Invoke("RemoveList", &result, 0, 0);
		CandListOpen = false;
		//@tangbinjie

        SF_FREE(lpCandList);
        return;
    }
  
    int numCandidatesOnList = pageSize;

    NumCandidatesOnListPrev = numCandidatesOnList;

	//@tangbinjie
	Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
	SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));

	const wchar_t* lpCompostiontstr = pimeMgr->GetCompositionString();

	int isCharacterID = 0;
	if ((IMETag == GFxIME_Ch_Simp_QQPinyin || IMETag == GFxIME_SogouPinyin) &&lpCompostiontstr)
	{
		UPInt len = SFwcslen(lpCompostiontstr);
		if (len > 0 && lpCompostiontstr[len-1] >= '0' && lpCompostiontstr[len-1] <= '9' )
		{
			isCharacterID = 1;
		}
	}

	//@tangbinjie

//    if (CandListPath.GetLength() > 0)
    {
		Value result;
		Invoke("RemoveList", &result, 0, 0);
		Array<Value> valArray;
		valArray.PushBack(double(pageSize));

		//@tangbinjie
		if (isCharacterID == 0)
		{
			valArray.PushBack(double(CandListStartFrom1));
		}
		else
		{
			valArray.PushBack(double(-1));
		}
		//@tangbinjie
		Invoke("CreateList", &result, valArray.GetDataPtr(), 2);
		CandListOpen = true;
	
    //    pMovie->Invoke(("RemoveList").ToCStr(), "");
    //    pMovie->Invoke(("CreateList").ToCStr(), "%d %d", pageSize, CandListStartFrom1);
    }
    ListBox->RemoveAllListItems();

    for (int i = 0; i < NumCandidates; i++)
    {
        wchar_t* rowData = (wchar_t*)(((char*)(lpCandList))+lpCandList->dwOffset[i+CurrentPageStart]);
        int utf8Len = (int)UTF8Util::GetEncodeStringSize(rowData, -1);
        char* rowDataUTF8 = (char*)SF_ALLOC(utf8Len + 1, StatIME_Mem);
        UTF8Util::EncodeString(rowDataUTF8, rowData, -1);
        rowDataUTF8[utf8Len] = 0;
        Ptr<CandidateListItem> pItem  = *new CandidateListItem(rowDataUTF8);
        ListBox->AddListItem(pItem, false); // Only copy as much data as necessary.
        SF_FREE(rowDataUTF8);    
    }

    if (lpCandList)
        SF_FREE(lpCandList);
    ImmReleaseContext(hWND, hIMC);

    //  pMovie->Invoke("_root.RepositionCandWindow", "%d", ClausePosition );

    // The row selection only needs to work for certain IME's- the other's don't 
    // support selecting rows by pushing down/up arrow key

    if ( IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_Jp_ATOK2008 
		|| IMETag == GFxIME_Jp_ATOK2009 || IMETag == GFxIME_Jp_GOOG2010)    //  Sogou Pinyin IME)     
    {
        ListBox->SetSelectedItemIndex((currSelection) % pageSize);
    }
    else
    {
        ListBox->SetSelectedItemIndex(-1);
    }

    ListBox->UIRefreshView();
	DrawCandidateList = true;
    // Need to reposition candidate list so that it appears below currently highlighted clause. Since clause information 
    // is available as a result of processing WM_COMPOSITION message, it needs to be stored in a class member variable.

 //   RepositionCandidateList(-ClausePosition);     

}

void GFxIMEVista::Finalize()
{
    Value val;
    Value isInTextFieldVal;
    HIMC     hIMC;

    hIMC = ImmGetContext(hWND);

    // In addition to finalizing IME, reset cursor position to the position of the mouse click. This is to deal
    // with the situation when composition has not been finalized and the user clicks elsewhere in the textfield. 
    // The cursor should return to the position of the click once the IME composition has been finalized. 

    // Finalize- This will cause WM_IMECOMPOSITION with GCS_RESULTSTR to be sent. 
    // In response, we'll obtain the result string and call FinalizeComposition on the 
    // IMEManager.

    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);

    ImmReleaseContext(hWND, hIMC);

	pTSF->AbortCandidateList();

	if ( IMETag == GFxIME_Ch_Trad_Array ||
		IMETag == GFxIME_Ch_Trad_DaYi )
	{
		INPUT input;
		input.type      = INPUT_KEYBOARD;
		input.ki.wVk    = VK_ESCAPE;
		input.ki.wScan  = (WORD)MapVirtualKey(VK_ESCAPE, 0);
		SendInput(1, &input, sizeof(input));
	}

    // Clear Reading window contents and remove the reading window
    ReadingTextBuffer[0] = 0;
//    if (CandListPath.GetLength() > 0)
	{
		Value result;
		Invoke("RemoveList", &result, 0, 0);
		CandListOpen = false;
	}
}

void    GFxIMEVista::OnIME(UPInt keyCode, UPInt info, int options) //keyCode: wParam, info: lParam
{

    SF_UNUSED(options);
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    ///////////////////////////////

    HIMC        hIMC;
    DWORD       dwSize;
    wchar_t*    lpstr = NULL;
    char*       lpastr = NULL; // Stores attribute string
    Value    val;

    if (info == 0)
    {

        DisplayCompositionString(NumCharDisplay);
    }

     if (info & GCS_RESULTSTR) 
    {
        Ptr<IMEManagerBase> pimeMgrResult = pMovie->GetIMEManager();
        SF_ASSERT(pimeMgrResult && pimeMgrResult->IsMovieActive(pMovie));

        hIMC = ImmGetContext(hWND);
        // Get the size of the result string.
        dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
        lpstr = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);

        // Get the result string that is generated by IME into lpstr.
        LONG res = ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
        if (res != IMM_ERROR_NODATA && res != IMM_ERROR_GENERAL)
        {
            lpstr[dwSize/2] = 0;
            pimeMgrResult->SetWideCursor(false);
        }

        NumCharDisplay          = 0;
        ReplaceChar             = 0;
        ReadingTextBuffer[0]    = 0;
   //     if (CandListPath.GetLength() > 0)
   //         pMovie->Invoke(("RemoveInputWindow").ToCStr(), 0);
        ImmReleaseContext(hWND, hIMC);
        pimeMgrResult->FinalizeComposition(lpstr);
		// Store the composition string so can be written to the log if we are conducting an IME test.
        ((GFxIMEManagerWin32*)pimeMgrResult.GetPtr())->CompositionString = lpstr;

        if(lpstr != NULL)
        {
            SF_FREE(lpstr); 
            lpstr = 0;
        }
    }

    if((info & CS_INSERTCHAR) && (info & CS_NOMOVECARET) )
    {
        // This is for Korean- a character can change shape depending upon what is typed next, 
        // so don't advance caret position. Move Caret when the character is finalized.
        // The "replace" flag is used to indicate whether we should replace the current character 
        // or not-In case of korean, when the user begins to type, the character should be 
        // inserted at the current location, but subsequent modifications to the same character 
        // should replace the existing character.
        wchar_t koreanChar[2];
        koreanChar[0] = (wchar_t)keyCode; koreanChar[1] = 0;
        ReplaceChar     = 1;
        NumCharDisplay  = 1;

        pimeMgr->SetWideCursor(true);
        //pimeMgr->TypeChar(lpstr[0]);
        pimeMgr->SetCompositionPosition();
        pimeMgr->SetCompositionText(koreanChar, 1);
    }

    if((info & GCS_COMPSTR) && (info & GCS_CURSORPOS))
    {

		if (IMETag != GFxIME_Ch_Trad_WuXia)
			DisplayCompositionString(NumCharDisplay);

    }

    if((info & GCS_COMPATTR))
    {
        hIMC    = ImmGetContext(hWND);
        dwSize  = ImmGetCompositionString(hIMC, GCS_COMPATTR, NULL, 0);
        lpastr  = (char*)SF_ALLOC((dwSize+1), StatIME_Mem);
        ImmGetCompositionString(hIMC, GCS_COMPATTR, (LPVOID)lpastr, dwSize);

		// The min needs to be taken since for chinese IME (ZhengMa), dwSize doesn't get updated correctly
		// when user pushes backspace.
		int numChar = min(dwSize, NumCharDisplay);

		// Obtain reading string information.
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPREADSTR, NULL, 0);
        wchar_t* lpReadStr = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);
        memset(lpReadStr, 0, 2*dwSize);
        ImmGetCompositionString(hIMC, GCS_COMPREADSTR, (LPVOID)lpReadStr, dwSize);
        lpReadStr[dwSize/2] = 0; // Since the IME doesn't always put in the terminating NULL

		SF_FREE(lpReadStr);
        // Get cursor position information
        int pos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        ImmReleaseContext(hWND, hIMC);

        pimeMgr->SetCursorInComposition((UPInt)pos);

        // Note: indexes for arrays/strings should be UPInt to eliminate warnings for 64 bit comps.
        // UPInt start = 0; 
        // bool clauseFlag = false;
        
        struct styleInfo style;

        Array<styleInfo> styleList;

        char currentStyle   = lpastr[0];
        ClausePosition      = numChar - pos;
        style.begin         = 0; 
        style.highLightStyle = lpastr[0];
		
        // This is important because for Japanese IME, the candidate list doesn't always have to be next to the clause
        // for example, when one converts the composition string to convert mode by pushing space, the current clause is
        // usually in the beginning of the string while the cursor is at the end. For other IME's like Chinese Simplified
        // Pinyin, the candidate list is always at the position of the cursor and no clause information is provided. Hence
        // the ClausePosition must be initialized to the cursorPosition unless IME explicitly provides clause information 
        // in the attribute string.
        if(lpastr[0] == ATTR_TARGET_CONVERTED || lpastr[0] == ATTR_TARGET_NOTCONVERTED)
            ClausePosition = numChar;

		DisplayReadingWindow(lpReadStr);

        for(int i = 0; i < numChar; i++)
        {
            if(lpastr[i] != currentStyle)
            {
                styleList.PushBack(style);
                if(lpastr[i] == ATTR_TARGET_CONVERTED || lpastr[i] == ATTR_TARGET_NOTCONVERTED)
                    ClausePosition = numChar - i;
                style.begin         = i;
                currentStyle        = lpastr[i];
                style.highLightStyle = currentStyle;
            }
            style.end = i;
        }

        style.end = numChar - 1;
        styleList.PushBack(style);

        pimeMgr->SetCursorInComposition((UPInt)pos);
        if(!(IMETag & (UInt32)GFxIME_Kr_Flag)) // No highlighting necessary for Korean.
        {
            for(UPInt i = 0; i < styleList.GetSize(); i++)
            {
                UInt32 highlightStyle  = styleList[i].highLightStyle;
                UInt32 start         = styleList[i].begin;
                UInt32 len           = styleList[i].end - styleList[i].begin + 1;
				if (len == 0) continue; 
                switch(highlightStyle)
                {
                case ATTR_INPUT:
                    pimeMgr->HighlightText(start, len, IMEManagerBase::THS_CompositionSegment, 0);
                    break;   
                case ATTR_TARGET_CONVERTED:
                case ATTR_TARGET_NOTCONVERTED:
                    pimeMgr->HighlightText(start, len, IMEManagerBase::THS_ClauseSegment, 0);
                    break;
                case ATTR_CONVERTED:
                    pimeMgr->HighlightText(start, len, IMEManagerBase::THS_ConvertedSegment, 0);
                    break;
                case ATTR_INPUT_ERROR:
                    pimeMgr->HighlightText(start, len, IMEManagerBase::THS_LowConfSegment, 0);
                    break;
                }
            }
        }
       
        if(pTSF->GetCandidateWindowFlag())
		{
        //     RepositionCandidateList(-ClausePosition);
		}

        if(lpastr)
            SF_FREE(lpastr);
    }
}

void GFxIMEVista::RepositionCandidateList(UInt32 offset)
{
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    RectF viewRect, caretRect;
    // Scaleform 4.5 IMEManagerBase::GetMetrics only exposes view/caret rects;
    // the Snda-specific parent rect is not part of the public interface.
    pimeMgr->GetMetrics(&viewRect, &caretRect, offset);
//    if (CandListPath.GetLength() > 0)
	{
	//	Value* valArray = (Value*)SF_ALLOC( 3*sizeof(Value), StatIME_Mem);
		Array<Value> valArray;
		Value result;
		valArray.PushBack(Value(caretRect.TopLeft().x));
		valArray.PushBack(Value(caretRect.BottomRight().y));
		valArray.PushBack(Value(caretRect.Height()));

		//valArray.PushBack(Value(caretRect.TopLeft().x - parentRect.TopLeft().x));
		//valArray.PushBack(Value(caretRect.TopLeft().y - parentRect.TopLeft().y));
		//valArray.PushBack(Value(caretRect.Height()));

		Invoke("RepositionCandWindow", &result, valArray.GetDataPtr(), 3);

		//pimeMgr->GetASIMEManager()->RepositionCandidate(caretRect.TopLeft().x,caretRect.BottomRight().y,caretRect.Height());
	}
}

void GFxIMEVista::DisplayCompositionString(UInt32& numCharDisplay)
{
    int         cursorPos;
    Value    val; 

    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));

    // Obtain composition string information from IMM
    HIMC hIMC       = ImmGetContext(hWND);
	int dwSize      = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
	wchar_t* lpstr  = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);

	ImmGetCompositionString(hIMC, GCS_COMPSTR, (LPVOID)lpstr, dwSize);

	//if (lpstr[0] == 0x20)
	//{
	//	// hack by zhaojun@snda

	//	if(lpstr)
	//		SF_FREE(lpstr);

	//	dwSize = 2 * TypingStringBuffer.GetSize();
	//	lpstr  = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);

	//	UPInt i = 0;

	//	while(i < TypingStringBuffer.GetSize())
	//	{
	//		UINT32 c = TypingStringBuffer.GetCharAt(i);
	//		*(lpstr+i) = (wchar_t)c;
	//		++i;
	//	}
	//}

    cursorPos                   = 0;
    lpstr[dwSize/2]             = 0;

    if(lpstr[0] == 0x20 || lpstr[0] == 0x3000)
        lpstr[0] = 160; // Insert non breaking space

	// Since for ABC IME, composition text must be displayed in the reading window.
	if (IMETag == GFxIME_Ch_Simp_ABC)
	{
		ReadinStringBuffer = lpstr;
		// Setting the ReadinStringBuffer is enough since DisplayReadingWindow will be called when the WM_IMECOMPOSITION with
		// GCS_COMPATTR set is received.
		if (lpstr)
			SF_FREE(lpstr);
		return;
	}
    // Write new composition text
    pimeMgr->SetCompositionText(lpstr);
    numCharDisplay = dwSize/2;

	

    if(IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_Ch_Simp_Other) //  Chinese (Simplified) - sogou_pinyin on Win Vista
    {
        // Note that for this IME, no attribute information is sent, so cursor position and 
        // composition string attribute must be set here.
        // Get cursor position information
        UInt32 pos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);	// some ime always return 0...
        // Set cursor position in the composition string
        pimeMgr->SetCursorInComposition((UPInt)pos);
        ClausePosition      = numCharDisplay - pos;
        pimeMgr->HighlightText(0, numCharDisplay, IMEManagerBase::THS_CompositionSegment, 0);
    }
    // Now reset the cursor position in the textfield.
    // This check is important because when we don't want the cursor position to change
    // if the composition string is empty. 
    if (dwSize != 0) 
    {   
        // Adjust cursor Position
        int pos1 = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        pimeMgr->SetCursorInComposition((UPInt)pos1);
    }

    RectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, 0);
    // Now pass it on to the DisplayIn
    // putWindow function so it knows where to put the reading window. This way
    // DisplayInputWindow is independent of the flash document that contains the text field. 
    // Params: 1: Reading String 2,3: x-y position of the Reading Window 4: Color of Reading text.

    

    // Note that for Dayi 6.0 and Array 6.0 IMEs, for a fresh composition, the Reading window information is received 
    // earlier than startcomposition message. This causes GetMetrics to return bad positioning information since the 
    // textfield pointer in the IMEManager is set when StartComposition is called. To take care of this, we have added 
    // logic in IMEManager's OnOpenCandidateList to set the textfield pointer from the currently focussed item's textfield.
    // We don't call RepositionInputWindow for chinese traditional IME's since it causes the previously drawn window to
    // move leading to flickering. 

    if (/*(CandListPath.GetLength() > 0) && */!(IMETag & (UInt32)GFxIME_Ch_Trad_Flag))
	{
	//	Value* valArray = (Value*)SF_ALLOC( 2*sizeof(Value), StatIME_Mem);
		Array<Value> valArray;
		Value result;
		valArray.PushBack(Value(caretRect.TopLeft().x));
		valArray.PushBack(Value(caretRect.BottomRight().y));
		Invoke("RepositionInputWindow", &result, valArray.GetDataPtr(), 2);
	//	SF_FREE(valArray);
	}
   //    GetASIMEManager()->Invoke((("RepositionInputWindow").ToCStr(), "%d %d", (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y); 

	ImmReleaseContext(hWND, hIMC);
    if(lpstr)
        SF_FREE(lpstr);
}


void GFxIMEVista::DisplayReadingWindow(wchar_t* pReadingStr)
{

	SF_UNUSED(pReadingStr);

    Value    val; 
    char*       utf8Str = 0;
	
    // Remove previous input window..
    //if (CandListPath.GetLength() > 0)
	{
		Value result;
		Invoke("RemoveInputWindow", &result, 0, 0);
	}
    // int len = (int)wcslen(pReadingStr);

	
	int len	= (int)ReadinStringBuffer.GetSize(); 
	if (len == 0) return;
	utf8Str		= (char*)SF_ALLOC(len+1, StatIME_Mem);
	memset(utf8Str, 0, len+1);
	memcpy(utf8Str, ReadinStringBuffer.ToCStr(), len+1);

    if(!utf8Str)
    {     // Escape key
        ReadingTextBuffer[0] = 0; 
        return;
    }
	
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    RectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
    // Now pass it on to the DisplayInputWindow function so it knows where to put the reading window. This way
    // DisplayInputWindow is independent of the flash document that contains the text field. 
    // Params: 1: Reading String 2,3: x-y position of the Reading Window 4: Color of Reading text. 
    //if (CandListPath.GetLength() > 0)
	{
		Array<Value> valArray;
		Value result;
		valArray.PushBack(Value(utf8Str));
		valArray.PushBack(Value(caretRect.TopLeft().x));
		valArray.PushBack(Value(caretRect.BottomRight().y));
		valArray.PushBack(Value(float(0)));
		Invoke("DisplayInputWindow", &result, valArray.GetDataPtr(), 4);
	}

	//RepositionCandidateList(0);
        
	if(utf8Str)
		SF_FREE(utf8Str);
	
    return;

}

void GFxIMEVista::SelectAndClose(int index)
{
    // This function is called when the user clicks on a candidate list row. This action should update the 
    // composition string with the contents of the clicked row as well as close the candidate list.
    // TSF provides a way to notify IME about the click- but some third party IMEs
    // such as google and sogou don't follow it. So this can't be a general purpose solution
    // The other way to do this is to simulate a key press that corresponds to the candidate list row that was
    // clicked. We use this approach for the third party IMEs and the regular approach for system IMEs
	// Note: (Mar 18 2009) Shuang Pin, Quan Pin and ZhengMa also don't follow the method recommeded in the doc
	// which involves obtaining the IID_ITfCandidateListUIElementBehavior_GFx pointer and calling finalize on it
	// For these renegade IME's, NO_INTERFACE is returned while querying for the above interface.

    if ((IMETag & (UInt32)GFxIME_Ch_ThirdParty_Flag) || 
		IMETag == GFxIME_Ch_Simp_ZhengMa ||
		IMETag == GFxIME_Ch_Simp_ShuangPin ||
		IMETag == GFxIME_Ch_Simp_QuanPin)
    {
        INPUT input;
        input.type      = INPUT_KEYBOARD;
        input.ki.wVk    = (WORD)(0x30 + (index+1));
        input.ki.wScan  = (WORD)MapVirtualKey(0x30 + (index+1), 0);
        SendInput(1, &input, sizeof(input));
    }
    else
    {
        pTSF->SelectAndClose(index);
    }
}

// Closes IME
void GFxIMEVista::CloseIME()
{
	/*
	closing IME and removing the candidate list is another can of worms. There are atleast three ways to do it, 
	none of which works for all the IME's.

	1) Using the IMM functions to cancel composition and close the candidate list. These are well documented in the 
	MSDN documentation.

	2) Using the Abort method of the ITfCandidateListUIElementBehavior interface. This obviously only works if a pointer
	to that interface can be obtained

	3) Putting an Esc in the input queue. This is the most hacky but works even for the most inconsistent IMEs.

	*/

    HIMC hIMC = ImmGetContext(hWND);
    if (!hIMC){
        //  MyError(ERROR_NULLCONTEXT);
    }
	// Method 1:
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
	ImmNotifyIME(hIMC, NI_CLOSECANDIDATE, 0, 0);
	
	// Method 2:
	pTSF->AbortCandidateList();

	// Method 3: 
	if ( IMETag == GFxIME_Ch_Trad_Array ||
		IMETag == GFxIME_Ch_Trad_DaYi )
	{
		INPUT input;
		input.type      = INPUT_KEYBOARD;
		input.ki.wVk    = VK_ESCAPE;
		input.ki.wScan  = (WORD)MapVirtualKey(VK_ESCAPE, 0);
		SendInput(1, &input, sizeof(input));
	}
		
    ReadingTextBuffer[0] = 0;
    //if (CandListPath.GetLength() > 0)
	{
		Value result;
		Invoke("RemoveInputWindow", &result, 0, 0);
	}
    ImmReleaseContext(hWND, hIMC);
}
}}}
#endif
