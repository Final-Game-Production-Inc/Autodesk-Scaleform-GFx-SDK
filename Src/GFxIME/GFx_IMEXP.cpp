/**************************************************************************

Filename    :   GFx_IMEXP.cpp
Content     :   Implementation of Input Method Support on Windows XP using IMM. 
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
#include "GFx_IMEUnicodeMap.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Kernel/SF_Array.h"
#include "GFx_IMEManagerWin32.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

GFxIMEXP::GFxIMEXP(IMEManagerBase* pbase, HWND wnd): GFxIMEWin32Impl(pbase, wnd)
{

    CurrLocale      = NULL;
    NumRowsMax      = 0;
    NumCharDisplay  = 0;
    Track           = false;
    ImeMessageRecd  = false;
    hSavedIMC       = NULL;
    
	// This initialization is important since for some IME's (Google Pinyin)
    // OpenCandidate message arrives before the StartComposition message and
    // we don't get a chance to correctly initialize CandListStartFrom1.
    CandListStartFrom1	= 1;
	DrawCandidateList	= false;
	CandListOpen = false;

    ReadingTextChTradNewPhonetic.Resize(ReadingTextBufferSize);
    ReadingTextBuffer.Resize(ReadingTextBufferSize);
    int size = sizeof(TradNewPhoneticKeyCodesA)/(3 * sizeof(int));
    for(int i = 0; i < size; i++)
	{
        for(int j = 0; j < 3; j++){
            char c = (char)TradNewPhoneticKeyCodesA[i][0];
            int idx = (int)(tolower(c)) - 0x20;
            //             if ( !(idx > -1 && idx < NumTradNewPhoneticKeyCodes) )
            //                 printf("buffer overrun\n");
            TradNewPhoneticKeyCodesB[idx][j] = TradNewPhoneticKeyCodesA[i][j];
        }
	}

}


GFxIMEXP::~GFxIMEXP()
{
}

// Route all fscommands to the CmdManager object which will redirect them to the appropriate handler object.
void GFxIMEXP::FsCallBack(Movie* pmovie, const char* pcommand, const char* parg)
{
    CmdManager.FsCallback(pmovie, pcommand, parg);
}

void GFxIMEXP::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{

    int     isIMEMessage = 0; 

    BYTE ks[256];
    wchar_t charCode[20];


    if((winEvt.Message == WM_KEYDOWN) || (winEvt.Message == WM_KEYUP))
    {
        ::GetKeyboardState(ks);
        CurrLocale      = GetKeyboardLayout(0); 
        Un.hkl          = CurrLocale;

        UInt32 uScanCode  = (UInt32)((winEvt.LParam >> 16) & 0xFF); // fetch the scancode

        LastCharacter   = (wchar_t)ImmGetVirtualKey(hWND);

		if ((IMETag == GFxIME_Ch_Simp_MSPinyin_2007) && (LastCharacter == VK_NEXT))
		{
			HIMC hIMC = ImmGetContext(hWND);
			ImmNotifyIME(hIMC, NI_OPENCANDIDATE, 0, 0);
			ImmNotifyIME(hIMC, NI_SETCANDIDATE_PAGESTART, 0, CurrentPageStart+9);
			ImmReleaseContext(hWND,hIMC);

		}

		if ((IMETag == GFxIME_Ch_Simp_MSPinyin_2007) && (LastCharacter == VK_PRIOR))
		{
			HIMC hIMC = ImmGetContext(hWND);
			ImmNotifyIME(hIMC, NI_OPENCANDIDATE, 0, 0);
			ImmNotifyIME(hIMC, NI_SETCANDIDATE_PAGESTART, 0, CurrentPageStart-9);
			ImmReleaseContext(hWND,hIMC);

		}

        // ToUnicodeEx function translates the specified virtual-key code and keyboard state 
        // to the corresponding Unicode character or characters
        ToUnicodeEx(LastCharacter, uScanCode, ks, charCode, 20, 0, CurrLocale);
     //   LastCharacter = (char)charCode[0];

    }

    // This logic checks to see if an IME message is received between WM_KEYDOWN and WM_KEYUP.
    // If not, it closes the IME. This is to replicate candidate list behaviour for Simplified Ch IMEs.
    // If the cand window is open and the user pushes arrow keys, the list should close. 
    // Note that LastCharacter will be zero if a non letter/number key (like arrow keys, page down etc) 
    // are pushed. Otherwise we don't want to close IME. This is a hack to fix problems with MS Pinyin
    // Reading window. 

	/*
	Additional Info: While investigating ABC IME, I've discovered that if IME doesn't want us to process a key, it makes the virtual key code VK_PROCESSKEY.
	for example, with ABC IME, push ga and then space. Candiddate list will open. Push page down now. The next page of the list will open. Push page down again.
	since the list has only two pages, nothing will happen. Therefore for the KEYDOWN message corresponding to the pagedown, the virtual key is VK_PROCESSKEY. If 
	ime wants the message to processed as usual, the virtual keycode will be something other that VK_PROCESSKEY.
	*/
    if ((winEvt.Message == WM_KEYDOWN) && (winEvt.WParam != VK_PROCESSKEY))
    {
        Track           = true; // start the track
        ImeMessageRecd  = false;
    }

    if (Track)
        isIMEMessage = ImmIsUIMessage(NULL, winEvt.Message, winEvt.WParam, winEvt.LParam);
    if (isIMEMessage)
    {
        ImeMessageRecd = true;

    }
    if ((winEvt.Message == WM_KEYUP) /*&& (LastCharacter == 0)*/)
    {
        if (Track) 
        {
            Track = false;
            // The NumCharDisplay == 0 condition is important for IME's such as MS Pinyin. Here, the input characters
            // appear as part of the composition string, not the reading window, so we don't want the composition
            // to be cancelled if the user pushes up/down keys for which IME messages might not be generated..(write 
            // more about this)
            if (/*(IMETag != GFxIME_Ch_Trad_WuXia) && */ !ImeMessageRecd && (NumCharDisplay == 0))
            {
                CloseIME();
            }
        }
    }

    // Here we obtain the character code corresponding to the last key pressed and look into our keyboard mapping
    // table to obtain the unicode of the character/stroke for reading window display. This needs to be done
    // since IMM doesn't provide reading window information for certain IME's, hence we need to generate this
    // information ourselves using the last input keystroke. 

    if (winEvt.Message == WM_KEYDOWN)
    {
        UInt32 uScanCode  = (UInt32)((winEvt.LParam >> 16) & 0xFF); // fetch the scancode
        LastCharacter   = (wchar_t)ImmGetVirtualKey(hWND);
        // ToUnicodeEx function translates the specified virtual-key code and keyboard state 
        // to the corresponding Unicode character or characters
        ToUnicodeEx(LastCharacter, uScanCode, ks, charCode, 20, 0, CurrLocale);

        LastCharacter = (char)charCode[0];
        // IME messages will be generated in response to these, so no special processing is needed.

        if (LastCharacter == 0x1b) // Escape 
            return;

        if (LastCharacter == 0x20) // Space
            return; //Don't translate

        if (LastCharacter == 0x0D) // Return
            return;

        if (LastCharacter == 0x08) // backspace
            return;

        if (IMETag == GFxIME_Ch_Trad_NewPhonetic) // Chinese Traditional New Phonetic
        {
            if (LastCharacter >= 0x20 && LastCharacter <= 0x7A)
            {
                int lastCharacterCopy   = (int)(tolower(LastCharacter)) - 0x020;
                LastCharacter           = (wchar_t)TradNewPhoneticKeyCodesB[(int)lastCharacterCopy ][1];
                ReadingWindowCharacterPos = TradNewPhoneticKeyCodesB[(int)lastCharacterCopy ][2];
            }

        }

        if (IMETag == GFxIME_Ch_Trad_NewChangJie) // Chinese Traditional New ChangJie
        {
            if (LastCharacter >= 0x20 && LastCharacter <= 0x7A)
            {
                LastCharacter = (wchar_t)NewCangJieKeyCodes[(int)(tolower(LastCharacter)) - 'a'][1];
            }
        }

        // The key idea here is as follows:
        /*
        Below is more detailed background information regarding how IME does the trick to generate VK_PROCESSKEY:
        When the user presses a key, the system generates a keyboard event. USER.EXE checks to see whether the 
        currently active input language handle (HKL) points to an IME. If so, USER.EXE passes the keyboard event 
        to the Input Method Manager, which passes the event to the IME. If the IME intends to respond to the keyboard 
        event (for example, when the user presses the Spacebar to activate the candidate window on Japanese Windows), 
        the IME translates the keyboard event into the virtual key, VK_PROCESSKEY, which it passes back to the IMM. 
        The IMM then sends this virtual key to the application. If the application provides a customized IME user 
        interface, it can trap VK_PROCESSKEY and call the API ImmGetVirtualKey to translate it into a more specific 
        virtual key value, and then respond accordingly. For example, if the virtual key is VK_SPACE, the application 
        would respond by calling code to paint the candidate window. If the IME doesn't use a key, it doesn't set 
        msg.wParam = VK_PROCESSKEY and leaves it intact. This allows the application to respond to key inputs such as 
        arrow keys, page down/up in the normal manner. An example is as follows. If you are using Japanese IME, when
        u'r in composition mode and there are characters in the composition string, if you use the left/right arrows keys,
        IME sends WM_COMPOSITION message with the new cursor position and sets msg.wParam = VK_PROCESSKEY since the effect
        of the left/right arrow key input has been accounter for by sending the WM_COMPOSITION message and the application
        shouldn't take any further action. However, if you are using quanpin IME, there is no composition string and left/right
        arrow key should be left untouched by the IME. 
        The problem with Boshiamy ime is that it makes msg.wParam = VK_PROCESSKEY for left/right arrow key inputs
        (and other keys for which a WM_CHAR is not generated) even when IME is not active which
        means that GFxPlayer doesn't process these keys appropriately. When the IME is active (which is determined by
        !LastCharacter) we don't mess with the message and leave it intact. This IME apparently doesn't allow the user
        to change cursor position in the composition string and the composition will just be cancelled when the user
        pushes left/right arrow keys. The code below resets the wParam to the correct
        val and sends a new message. We can't change the current message since the message params are passed to us in 
        a const struct. This logic is also responsible for the correct behaviour with enter key. The sequence in this
        case is: WM_KEYDOWN (with VK_PROCESSKEY) -> WM_KEYDOWN (generated by this logic) ->WM_CHAR-> WM_KEYUP which
        causes the ime to close due to the logic above.

        
        if (IMETag == GFxIME_Ch_Trad_WuXia && !LastCharacter)
        {
            SendMessage(hWND, winEvt.Message, ImmGetVirtualKey(hWND), winEvt.LParam );
        }
		*/
    }


	if (winEvt.Message == WM_KEYUP)
	{
		if (DrawCandidateList)
		{
			RepositionCandidateList(-ClausePosition);
			DrawCandidateList = false;
		}
	}
}

// This is to handle backspace key for some chinese IME's for which regular (IME_COMPOSITION) IME messages 
// are not sent. This is undocumented behaviour, so this might not work as expected on all systems.

void GFxIMEXP::CustomProcessing(UPInt message, WPARAM wParam, LPARAM lParam)
{
    Value val;


    switch (message)
    {
    case WM_IME_NOTIFY:
        switch (wParam)
        {
        case IMN_PRIVATE:

            if (lParam == 0x012) // Backspace for chinese simplified MS Pinyin and chinese traditional
            {                   // New Phonetic
                if (LastCharacter == 0x08)
                {   //  Chinese Simplified MS Pinyin and New ChangJie
                    if (IMETag == GFxIME_Ch_Trad_NewChangJie || IMETag == GFxIME_Ch_Simp_MSPinyin_3_0) 
                    {
                        // Remove previous input window..
                      //  if (CandListPath.GetLength() > 0)
						{
							Value result;
							Invoke("RemoveInputWindow", &result, 0, 0);
							 //    pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
						}
                       
                        int len = (int)wcslen(&ReadingTextBuffer[0]);
                        ReadingTextBuffer[len-1] = 0;

                        int utf8Len = (int)(int)UTF8Util::GetEncodeStringSize(&ReadingTextBuffer[0], -1);
                        char* utf8Str = (char*)SF_ALLOC(utf8Len+1, StatIME_Mem);
                        UTF8Util::EncodeString(utf8Str, &ReadingTextBuffer[0], -1);

                        Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
                        SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
                        RectF viewRect, caretRect;
                        pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
                        // Params: 1: Reading String 2,3: x-y position of the Reading Window 4: Color of Reading text. 
                       // if (CandListPath.GetLength() > 0)
						{
							Array<Value> valArray;
						//	Value* valArray = (Value*)SF_ALLOC( 4*sizeof(Value), StatIME_Mem);
							Value result;
							valArray.PushBack(Value(utf8Str));
							valArray.PushBack(Value(caretRect.TopLeft().x));
							valArray.PushBack(Value(caretRect.BottomRight().y));
							valArray.PushBack(float(0));
							Invoke("DisplayInputWindow", &result, valArray.GetDataPtr(), 4);
                        //    pMovie->Invoke((CandListPath + ".DisplayInputWindow").ToCStr(), "%s %d %d %d", utf8Str, (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, 0);
						}
                        SF_FREE(utf8Str);

                    }

                    if (IMETag == GFxIME_Ch_Trad_NewPhonetic)  //  Chinese (Traditional) - New Phonetic
                    {
                        // Remove previous input window..
                       // if (CandListPath.GetLength() > 0)
                            pMovie->Invoke("RemoveInputWindow", 0);
                        int i = 3; // Since the Reading Window for New Phonetic has max 4 chars. 
                        // Find the first non space character
                        while(i >= 0 && (ReadingTextChTradNewPhonetic[i] == 0x020)) i--;
                        // Replace it by space
                        if (i >= 0)
                            ReadingTextChTradNewPhonetic[i] = 0x020;

                        // Get stage coordinates of the input window and then display it with the new text.
                        // (duplicate Value val removed: unused)
                        int utf8Len = (int)(int)UTF8Util::GetEncodeStringSize(&ReadingTextChTradNewPhonetic[0], -1);
                        char* utf8Str = (char*)SF_ALLOC(utf8Len+1, StatIME_Mem);
                        UTF8Util::EncodeString(utf8Str, &ReadingTextChTradNewPhonetic[0], -1);

                        Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
                        SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
                        RectF viewRect, caretRect;
                        pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
                        // Params: 1: Reading String 2,3: x-y position of the Reading Window 4: Color of Reading text. 
                       // if (CandListPath.GetLength() > 0)
						{
							Array<Value> valArray;
						//	Value* valArray = (Value*)SF_ALLOC( 4*sizeof(Value), StatIME_Mem);
							Value result;
							valArray.PushBack(Value(utf8Str));
							valArray.PushBack(Value(caretRect.TopLeft().x));
							valArray.PushBack(Value(caretRect.BottomRight().y));
							valArray.PushBack(float(0));
							Invoke("DisplayInputWindow", &result, valArray.GetDataPtr(), 4);
						
							//    pMovie->Invoke((CandListPath + ".DisplayInputWindow").ToCStr(), "%s %d %d %d", utf8Str, (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, 0);
						}

                        if (utf8Str)
                            SF_FREE(utf8Str);

                    }
                }
                if(LastCharacter == 0x020)
                {
                    // Chinese Traditional New ChangJie- When user pushes enter and reading window characters can't be composed into 
                    // a character. Reading window should be removed in this case
                    if (IMETag == GFxIME_Ch_Trad_NewChangJie) 
                    {
						//if (CandListPath.GetLength() > 0)
						{
							Value result;
							Invoke("RemoveInputWindow", &result, 0, 0);
							//    pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
						}
                        ReadingTextBuffer[0] = 0;
                    }
                }
            }   
        }
    }
}



// To Process WM_IMESTART message. Initializes IME state variables, stores a path to the 
// currently selected Textfield object. This is important because if the user clicks
// somewhere else and the focus gets transferred, the current composition should be finalized
// where it got started.
UInt32 GFxIMEXP::OnIMEStart(UPInt key, UPInt info, bool downFlag)
{
    if (!pMovie)
		return Movie::HE_NotHandled;

    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    pimeMgr->StartComposition();
    ///////////////////////////////

    SF_UNUSED(downFlag);
    SF_UNUSED(info);
    SF_UNUSED(key);
    NumCandidates       = 0;
    ClausePosition      = 0; 
    NumRowsMax          = 0;
    CurrChar            = 1;
    Value val;

    memset(&ReadingTextBuffer[0], 0, ReadingTextBufferSize);

    // Reset the Reading Window buffer for Ch Trad New Phonetic
    for (int i = 0; i < 4; i++)
        ReadingTextChTradNewPhonetic[i] = 0x020;
    ReadingTextChTradNewPhonetic[4] = 0;

    // Record current system locale- This is used to determine which IME is active
    CurrLocale = GetKeyboardLayout(0);

    //   HIMC hIMC = ImmGetContext(hWND);

    // Determine if the candidate list should start from 1 or 0
    int prop = ImmGetProperty(CurrLocale, IGP_PROPERTY);
    CandListStartFrom1 = (prop & IME_PROP_CANDLIST_START_FROM_1) == 0? 0: 1;

    if (IMETag == GFxIME_Ch_Trad_WuXia)
    {
        // For this IME, IMM reports the starting index of the candidate list incorrectly, so manually set it here.
        CandListStartFrom1 = 0;
    }
	
	// Since Wubi reports the wrong starting row
	if (IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Ch_Simp_WuBi98 || IMETag == GFxIME_Ch_Trad_Array )
    {
        CandListStartFrom1 = 1;
    }

    // Set the position of the composition window to an off-screen position so it's not 
    // visible. Setting the flags while processing WM_IME_SETCONTEXT doesn't always work

	CANDIDATEFORM candForm;
	COMPOSITIONFORM compForm;	// This comes in handy to hide the Japanese IME composition window.

	HIMC hIMC = ImmGetContext(hWND);
	POINT pt; pt.x = -1000; pt.y = -1000;
	RECT rect; rect.top = 0; rect.left = 0; rect.bottom = 10; rect.right = 10;
	
	candForm.dwIndex		= 0;
	candForm.dwStyle		= CFS_CANDIDATEPOS;
	candForm.ptCurrentPos	= pt;

	compForm.dwStyle        = CFS_FORCE_POSITION;
	compForm.ptCurrentPos   = pt;

	// If the IME is not supported, use IMM API to resposition default IME windows near the cursor position. 
	// If the IME is supported, push the default windows as far out of the way as possible. 
	if (IMETag == GFxIME_NotSupported)
	{
		WINDOWINFO wi;
		
		if (pMovie)
		{
			Ptr<IMEManagerBase> pimeMgrUnsupported = pMovie->GetIMEManager();
			SF_ASSERT(pimeMgrUnsupported && pimeMgrUnsupported->IsMovieActive(pMovie));
			RectF viewRect, caretRect;
			RectF caretRectTransformed;

			if (pimeMgrUnsupported)
			{
				pimeMgrUnsupported->GetMetrics(&viewRect, &caretRect, 0);
				caretRectTransformed = ((MovieImpl*)(pMovie))->TranslateToScreen(caretRect);
				
				pt.x = (LONG)caretRectTransformed.TopLeft().x;
				pt.y = (LONG)caretRectTransformed.BottomRight().y;
			}

			GetWindowInfo(hWND, &wi);
			
	//		pt.x = wi.rcWindow.left + pt.x;
	//		pt.y = wi.rcWindow.top + pt.y;

			candForm.ptCurrentPos	= pt;
			compForm.ptCurrentPos	= pt;
		}
	}

	ImmSetCompositionWindow(hIMC, &compForm);   
	ImmSetCandidateWindow(hIMC,	&candForm);
	ImmReleaseContext(hWND, hIMC);

	if (IMETag == GFxIME_NotSupported) return  Movie::HE_NotHandled;

	return Movie::HE_Handled;

}

void GFxIMEXP::OnIMEStop(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    if (!pMovie)
        return;

    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    pimeMgr->SetWideCursor(false);
//	pimeMgr->FinalizeComposition(L"");
    pimeMgr->ClearComposition();
}

// Processes candidate list notifications.

LRESULT GFxIMEXP::OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED(downFlag);
    if (!pMovie)
        return 0;
    Value val;

    if (!pMovie) return 0;

    CandidateListIndex = (int)info;

    switch (key)
    {
    case IMN_CHANGECANDIDATE:
        {
			if (!CandListOpen)
			{
				// zhaojun@snda
				OnOpenCandidate(0);
			}
            OnChangeCandidate(0);
			// Dayi and Array candidate list messages must be sent to DefWndProc for secondary candidate lists to work properly
			// Pinyin2007 candidate list messages must NOT be sent to DefWndProc, otherwise pageup/down, up/down keys don't work
			// properly.

            if (IMETag == GFxIME_Ch_Trad_DaYi || IMETag == GFxIME_Ch_Trad_Array /*|| IMETag == GFxIME_Ch_Simp_MSPinyin_2007*/)
                return 0;
            return 1;

        }

    case IMN_OPENCANDIDATE:
        {
            Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
            OnOpenCandidate(0);

            if (IMETag == GFxIME_Ch_Trad_DaYi || IMETag == GFxIME_Ch_Trad_Array /*|| IMETag == GFxIME_Ch_Simp_MSPinyin_2007*/)
                return 0;
            return 1;
        }

    case IMN_CLOSECANDIDATE:
        {
            ListBox->RemoveAllListItems();
            DrawCandidateList = false;
			//if (CandListPath.GetLength() > 0)
			{
				Value result;
				Invoke("RemoveList", &result, 0, 0);
				CandListOpen = false;
				//    pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
			}
                
            return 0;
        }
        
    case IMN_PRIVATE:
        {
            // Refer to comment in GFxIMEIdMap.h
            if (IMETag == GFxIME_Ch_Trad_NewChangJie )
            {
                if (ImeVersionId == IMEID_CHT_VER44)
                {
                    if ((info == 1) || (info == 2))
                    {
                        return 1;
                    }   
                }
            }
            
            if (IMETag == GFxIME_Ch_Trad_NewPhonetic)
            {     
                if ((info == 16) || (info == 17) || (info == 26) || (info == 27) || (info == 28))
                {
                    return 1;
                }
                break;
            }
        }
    }

    CustomProcessing(message, key, info);
    return 0;
}

void    GFxIMEXP::OnChangeCandidate(int candListIndex)
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

    /*
    The fields below have different interpretations for different IME's
    Japanese IME: 
    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    always 9 regardless of how many candidates are 
    actually on the page. For example, if the list has 
    16 candidate strings, the first page will have 9 and 
    last page will have 7 strings, but dwPageSize = 9 for 
    both the pages.
    dwPageStart:    Always zero.

    Chinese Simplified PinYin 3.0:
    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    Reflects the actual number of candidates on the page.
    dwPageStart:    The actual start of the current page

    Chinese Simplified QuanPin, ShuangPin:

    dwCount:        Number of Candidate Strings only on the current page.
    dwSelection:    Always 0. 
    dwPageSize :    Always 10, regardless of number of strings actually on the page.
    dwPageStart:    Always 0.

    Chinese Traditional IMEs:
    Phonetic:
    *********
    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    Always the beginning of the page, one can't go up/down rows using 
    arrow keys.
    dwPageSize :    Always 9, regardless of number of strings actually on the page.
    dwPageStart:    Always the beginning of the page.
    Note that for this IME, using arrow keys or home/end keys when list is open leads 
    to finalization.

    New Phonetic:
    *************

    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    Always 9.
    dwPageStart:    The actual start of the current page

    New ChangJie:
    *************

    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    Always 9.
    dwPageStart:    The actual start of the current page


    Da Yi:
    *******

    dwCount:        Total Number of candidate strings in the entire list
    not just the current page.
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    Always 9.
    dwPageStart:    The actual start of the current page


    Boshiamy Traditional:
    *******

    dwCount:        Total Number of candidate strings in the entire list
    not just the current page (not sure of this- have only been able to crete a list with 2 candidates)
    dwSelection:    currently selected row wrt beginning of the list. 
    dwPageSize :    Always 1 (from what I've seen so far).
    dwPageStart:    The actual start of the current page

	 ABC IME:
    *******

    dwCount:        Total Number of candidate strings in the entire list
    dwSelection:    This IME doesn't support row selection- dwSelection instead tells us the row on which the current page start.
    dwPageSize :    Always 9 (from what I've seen so far).
    dwPageStart:    always 0

    */  

	//@tangbinjie
	if ((int)lpCandList->dwPageSize != NumCandidatesOnListPrev)
	{
		OnOpenCandidate(0);
		return;
	}	
	//@tangbinjie

    NumCandidates       = lpCandList->dwCount ;    // Number of candidates
    int currSelection   = lpCandList->dwSelection; // Currently selected row
    int pageSize        = lpCandList->dwPageSize;  // Size of a page
    NumRowsMax          = max(NumRowsMax, pageSize);
    CurrentPageStart    = lpCandList->dwPageStart;

    // Since for all the other IME's except for Jap, CurrentPageStart contains the correct
    // page start.
    if (IMETag & (UInt32)GFxIME_Jp_Flag)
    {
        CurrentPageStart = currSelection - currSelection % NumRowsMax;
    }
    // Calculate the row where the current page starts. The reason why we take the max above 
    // is the following:
    // Suppose there are 23 elements in the list. 9 elements in the first page, 9 on the second
    // and 5 on the last page. Suppose currently selected row is row 20. When we are calculating 
    // page start for the current (last- since row number 20 is on last page) page, if we didn't
    // take the max above, we'll get 20 - 20%5 = 20: which is incorrect. 
    // Correct result: 20- 20%9 = 18 which is the row at which the last page starts.

    ImmReleaseContext(hWND, hIMC);

    if (NumCandidates ==  0)
    {
		// This is needed since for Sogou IME, if a candidate list is open and the user uses the arrow key
		// to move the cursor and then pushes the number key to select a candidate list row, the closecandidate
		// message is not sent, and this is our only chance to close the list. For example, type "ni" and then right
		// arrow key twice. Now push a number key. Without the RemoveList below, the list will hang around. This might
		// be an issue with other ime's also. Needs to be tested. Hopefully there are no sideeffects of this. 
		//if (CandListPath.GetLength() > 0)
		{
			Value result;
			Invoke("RemoveList", &result, 0, 0);
			CandListOpen = false;
			//    pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
		}
        SF_FREE(lpCandList);
        return;
    }

    // For simplified Chinese Pinyin 3.0, the CANDIDATELIST data structure provides correct
    // values for all parameters, so special computation is not needed. Refer to the comments above.

    int numCandidatesOnList = pageSize;

    if(IMETag == GFxIME_Ch_Simp_Pinyin03)
    {
        // For this IME, number of candidates on a page is always reported as 1 unless
        // the number of candidates is less < 5. How bizzare!
        if (NumCandidates > 5)
            numCandidatesOnList = min(5, NumCandidates - currSelection); // To take care of the last page
        // where actual number of candidates on the page is less than 5.
        else
            numCandidatesOnList = NumCandidates;

        pageSize = numCandidatesOnList; 
        CurrentPageStart = currSelection; // Since CurrentPageStart is always reported as 0
    }

    else if(IMETag == GFxIME_Ch_Simp_MSPinyin_3_0)
    {
        // This needs to be done because for Chinese Pinyin 3.0, sometimes for the last 
        // page of the candidate list, OnChangeCandidate gets called twice, the first time,
        // pageSize is not correct (it can be larger than number of candidates left on the list)
        // hence this min is needed to prevent crashes. This is bizzare behaviour!
        numCandidatesOnList = min(pageSize, NumCandidates - CurrentPageStart);
    }

	else if(IMETag == GFxIME_Ch_Simp_ABC)
    {
        numCandidatesOnList = min(pageSize, NumCandidates - currSelection);
		CurrentPageStart = currSelection;
    }

    else if(IMETag == GFxIME_Ch_Trad_WuXia)
    {
        // Number of candidates on the list is the same as dwCount. 
        numCandidatesOnList = NumCandidates;
    }

    else // For all other IME's
    {
        // Calculate the number of candidates on the current page.
        numCandidatesOnList = 0;
        if (NumCandidates < pageSize)
            numCandidatesOnList = NumCandidates; // Only one page
        else if (NumCandidates - CurrentPageStart <= pageSize) // Last Page
            numCandidatesOnList = NumCandidates - CurrentPageStart;
        else
            numCandidatesOnList = pageSize;

    }
    // We read in data for only the current page. This saves time since we shouldn't read data for the 
    // entire list most of which won't be displayed. 

    ListBox->RemoveAllListItems();

    for (int i = 0; i < numCandidatesOnList; i++)
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

    if(lpCandList)
        SF_FREE(lpCandList);

    ImmReleaseContext(hWND, hIMC);

    // The row selection only needs to work for certain IME's- the others don't 
    // support selecting rows by pushing down/up arrow key 

    if (IMETag ==   GFxIME_Ch_Trad_NewPhonetic ||
        IMETag ==   GFxIME_Ch_Trad_NewChangJie ||
		IMETag ==   GFxIME_Ch_Trad_NewChangJie2010 ||
        IMETag & (UInt32)GFxIME_Jp_Flag ||
        IMETag ==   GFxIME_Ch_Simp_MSPinyin_3_0 ||
        IMETag ==   GFxIME_Ch_Simp_MSPinyin_2007 ||
        IMETag ==   GFxIME_GooglePinyin ||
        IMETag ==   GFxIME_SogouPinyin ||
        IMETag ==   GFxIME_Ch_Simp_Pinyin03)
    {
        // Get the currently selected row index wrt the current page. currSelection
        // is the row index wrt the entire list.

        ListBox->SetSelectedItemIndex(max(0, (currSelection - CurrentPageStart)));
    }
    else
    {
        ListBox->SetSelectedItemIndex(-1);
    }

    ListBox->UIRefreshView();

	DrawCandidateList = true;

    //  if (NumCandidatesOnListPrev != numCandidatesOnList)
    {
        // The repositioning below needs to be done for simplified chinese (quan pin, shuang pin etc) to work properly.
        // As the user types in text in the input window, the contents of the candidate list change. Everytime the list
        // changes, CHANGE_CANDIDATE notification is sent and the list needs to be repositioned in response. For example,
        // if the number of elements in the list changes, the list might need to be moved above or below the input window
        // depending on the position of the caret and space available above and below the current line. 

/*        
		Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
        SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
        RectF viewRect, caretRect;
        pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
            MovieImpl* pmovieRoot = static_cast<MovieImpl*>(pMovie);
        //    pmovieRoot->TranslateToScreen(caretRect);
        if (CandListPath.GetLength() > 0)
            pMovie->Invoke((CandListPath + ".RepositionCandWindow").ToCStr(), "%d %d %d", (int)caretRect.Left, (int)caretRect.Bottom, (int)caretRect.Height());
        NumCandidatesOnListPrev = numCandidatesOnList;
*/
    }
}

void    GFxIMEXP::OnOpenCandidate(int candListIndex)
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
    int pageSize        = lpCandList->dwPageSize == 0 ? 5 : lpCandList->dwPageSize;
    NumRowsMax          = pageSize;

    ImmReleaseContext(hWND, hIMC);

	//@tangbinjie
	if (NumCandidates ==  0)
	{
		{
			Value result;
			Invoke("RemoveList", &result, 0, 0);
			CandListOpen = false;
			//    pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
		}
		SF_FREE(lpCandList);
		return;
	}
	//@tangbinjie
    // Note: OnOpenCandidate can also get called when user is on (say) the second page and clicks on a row.
    // The composition string will be updated according to the data on the selected row and the list will close
    // When the user pushes space, the list will open again an OnOpenCandidate will get called- but for the 
    // second page (following this example)- not for the first page as is normally the case. Since OnOpenCandidate
    // doesn't only get called for the first page on the list, all the computation and special cases needed for 
    // OnChangeCandidate also have to be performed here. 

    // Since for all the other IME's except for Japanese, CurrentPageStart contains the correct
    // page start (Therefore, for Jap, we have to compute correct PageStart ourselves)

    if(IMETag & (UInt32)GFxIME_Jp_Flag)
    {
        CurrentPageStart = currSelection - currSelection % NumRowsMax;
    }

    // For simplified Chinese Pinyin 3.0, the CANDIDATELIST data structure provides correct
    // values for all parameters, so special computation is not needed. Refer to the comments above (for OnChangeCandidate)

    int numCandidatesOnList = pageSize;
    if(IMETag == GFxIME_Ch_Simp_Pinyin03)
    {
        // For this IME, number of candidates on a page is always reported as 1 unless
        // the number of candidates is less < 5. How bizzare!
        // For this IME, number of candidates on a page is always reported as 1 unless
        // the number of candidates is less < 5. How bizzare!
        if (NumCandidates > 5)
            numCandidatesOnList = 5;
        else
            numCandidatesOnList = NumCandidates;

        pageSize = 5; 
    }

    else if(IMETag == GFxIME_Ch_Simp_MSPinyin_3_0)
    {
        // This needs to be done because for Chinese Pinyin 3.0, sometimes for the last 
        // page of the candidate list, OnChangeCandidate gets called twice, the first time,
        // pageSize is not correct (it can be larger than number of candidates left on the list)
        // hence this min is needed to prevent crashes. This is bizzare behaviour!
        numCandidatesOnList = min(pageSize, NumCandidates - CurrentPageStart);
    }

	else if(IMETag == GFxIME_Ch_Simp_ABC)
    {
        numCandidatesOnList = min(pageSize, NumCandidates - currSelection);
		CurrentPageStart = currSelection;
    }

    else if(IMETag == GFxIME_Ch_Trad_WuXia)
    {
        // Number of candidates on the list is the same as dwCount. 
        numCandidatesOnList = NumCandidates;
    }

    else // For all other IME's
    {
        // Calculate the number of candidates on the current page.
        numCandidatesOnList = 0;
        if (NumCandidates < pageSize)
            numCandidatesOnList = NumCandidates; // Only one page
        else if (NumCandidates - CurrentPageStart <= pageSize) // Last Page
            numCandidatesOnList = NumCandidates - CurrentPageStart;
        else
            numCandidatesOnList = pageSize;

    }
    // We read in data for only the current page. This saves time since we shouldn't read data for the 
    // entire list most of which won't be displayed. 

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

    NumCandidatesOnListPrev = numCandidatesOnList;

   // if (CandListPath.GetLength() > 0)
    {
		Value result;
		// Value* valArray = (Value*)SF_ALLOC(2*sizeof(Value), StatIME_Mem);
		Array<Value> valArray;
		valArray.PushBack(Value((float)pageSize));
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
		Invoke("RemoveList", &result, 0, 0);
		Invoke("CreateList", &result, valArray.GetDataPtr(), 2);
		CandListOpen = true;
        // pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
        // pMovie->Invoke((CandListPath + ".CreateList").ToCStr(), "%d %d", pageSize, CandListStartFrom1);
    }
    ListBox->RemoveAllListItems();

    for (int i = 0; i < numCandidatesOnList; i++)
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

    if (IMETag ==   GFxIME_Ch_Trad_NewPhonetic ||
        IMETag ==   GFxIME_Ch_Trad_NewChangJie ||
		IMETag ==   GFxIME_Ch_Trad_NewChangJie2010 ||
        IMETag & (UInt32)GFxIME_Jp_Flag ||
        IMETag ==   GFxIME_Ch_Simp_MSPinyin_3_0 ||
         IMETag ==   GFxIME_Ch_Simp_MSPinyin_2007 ||
        IMETag ==   GFxIME_GooglePinyin ||
        IMETag ==   GFxIME_SogouPinyin ||
        IMETag ==   GFxIME_Ch_Simp_Pinyin03)
    {
        ListBox->SetSelectedItemIndex((currSelection) % NumRowsMax);
    }
    else
    {
        ListBox->SetSelectedItemIndex(-1);
    }

    ListBox->UIRefreshView();

    // Need to reposition candidate list so that it appears below currently highlighted clause. Since clause information 
    // is available as a result of processing WM_COMPOSITION message, it needs to be stored in a class member variable.

	DrawCandidateList = true;

	/*
	Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    RectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
    if (CandListPath.GetLength() > 0)
        pMovie->Invoke((CandListPath + ".RepositionCandWindow").ToCStr(), "%d %d %d", (int)caretRect.Left, (int)caretRect.Bottom, (int)caretRect.Height());
	*/
}

void GFxIMEXP::RepositionCandidateList(UInt32 offset)
{
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    RectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, offset);
    //if (CandListPath.GetLength() > 0)
	{
		Array<Value> valArray;
		// Value* valArray = (Value*)SF_ALLOC( 2*sizeof(Value), StatIME_Mem);
		Value result;
		valArray.PushBack(Value(caretRect.TopLeft().x));
		valArray.PushBack(Value(caretRect.BottomRight().y));
		valArray.PushBack(Value(caretRect.Height()));
		Invoke("RepositionCandWindow", &result, valArray.GetDataPtr(), 3);
	//	pMovie->Invoke((CandListPath + ".RepositionCandWindow").ToCStr(), "%d %d %d", (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, (int)caretRect.Height()); 

	}
        
}

void GFxIMEXP::Finalize()
{

    ///////////////////////////////

    Value val;
    Value isInTextFieldVal;
    HIMC     hIMC;

    // This IME is the only one I've discovered so far that doesn't obey the NotifyIME instructions
    // to finalize current composition ( for example when the user clicks somewhere not on the candidate list)
    // All the other IME's even those that don't respond to clicking on the candidate list respond to the 
    // notify instructions to finalize current composition. Hence for this IME, we insert a esc in the input queue
    // which causes the current composition to cancel. Note that we can't finalize the current composition in this case
    // since by the time the finalize composition message would arrive, the focus would already have changed leading to
    // an inconsistent state of the composition string (orphaned characters with underline left in the textfield)

    if (IMETag == GFxIME_Ch_Simp_JJ)
    {
        Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
        SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));

        INPUT input;
        input.type      = INPUT_KEYBOARD;
        input.ki.wVk    = 0x1B;
        input.ki.wScan  = (WORD)MapVirtualKey(0x1B, 0);
        SendInput(1, &input, sizeof(input));
        pimeMgr->ClearComposition();
        ListBox->RemoveAllListItems();
       // if (CandListPath.GetLength() > 0)
		{
			Value result;
			Invoke("RemoveList", &result, 0, 0);
			CandListOpen = false;
			// pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
		}
            
        return;
    }

    hIMC = ImmGetContext(hWND);

    // Note that calling the functions below will cause windows to directly call MemberWndProc which
    // will in turn call the appropriate candidate list closing functions. This will cause the ime
    // state to update correctly before focus is transferred.
    // Finalize
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
    ImmNotifyIME(hIMC, NI_CLOSECANDIDATE, 0,0);
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);

    ImmReleaseContext(hWND, hIMC);

    // Clear Reading window contents and remove the reading window

    ReadingTextBuffer[0] = 0;
    LastCharacter = 0;
   // if (CandListPath.GetLength() > 0)
	{
		Value result;
		Invoke("RemoveInputWindow", &result, 0, 0);
		// pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
	}
        
}

void GFxIMEXP::Cleanup()
{
    GFxIMEWin32Impl::Cleanup();
    LastCharacter = 0;
}

// Handles IME_WMCOMPOSITION message
void    GFxIMEXP::OnIME(UPInt keyCode, UPInt info, int options) //keyCode: wParam, info: lParam
{
    SF_UNUSED(options);

    if (!pMovie)
        return;
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    ///////////////////////////////

    HIMC        hIMC;
    DWORD       dwSize;
    wchar_t*    lpstr   = NULL;
    char*       lpastr  = NULL; // Stores attribute string
    Value    val;

    // Deals with ESC
    if (info == 0)
    {
        //pMovie->Invoke(commandReplaceText, "%d %d %s", NumCharPermanant, NumCharPermanant + NumCharDisplay, "");
        NumCharDisplay = 0;
        return;
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
        LastCharacter           = 0;
       // if (CandListPath.GetLength() > 0)
		{
			Value result;
			Invoke("RemoveInputWindow", &result, 0, 0);
        //    pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
		}
        ImmReleaseContext(hWND, hIMC);
        pimeMgrResult->FinalizeComposition(lpstr);
        // Store the composition string so can be written to the log if we are conducting an IME test.
        ((GFxIMEManagerWin32*)pimeMgrResult.GetPtr())->CompositionString = lpstr;

        pimeMgrResult->BroadcastIMEConversion(lpstr);

        if(lpstr != NULL)
        {
            SF_FREE(lpstr); 
            lpstr = 0;
        }
    }

    if ((info & CS_INSERTCHAR) && (info & CS_NOMOVECARET) )
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
        pimeMgr->SetCursorInComposition(0);
    }
    //else
    //pimeMgr->SetOverwriteMode(false);


    if ((info & GCS_COMPSTR) && (info & GCS_CURSORPOS))
    {
        DisplayCompositionString(NumCharDisplay);
    }

    if(info & GCS_COMPREADSTR || info & GCS_COMPREADATTR || info & GCS_COMPREADCLAUSE || info & GCS_DELTASTART 
        || info & GCS_RESULTREADSTR || info & GCS_RESULTREADCLAUSE || info & GCS_RESULTCLAUSE || info & GCS_COMPCLAUSE)
    {
        //  char tm[30];
        //  hIMC    = ImmGetContext(hWND);
        //  dwSize  = ImmGetCompositionString(hIMC, GCS_COMPREADCLAUSE, tm, 30);
        //  ImmReleaseContext(hWND, hIMC);
    }

    if ((info & GCS_COMPATTR))
    {
        hIMC    = ImmGetContext(hWND);
        dwSize  = ImmGetCompositionString(hIMC, GCS_COMPATTR, NULL, 0);
        lpastr  = (char*)SF_ALLOC((dwSize+1), StatIME_Mem);

        ImmGetCompositionString(hIMC, GCS_COMPATTR, (LPVOID)lpastr, dwSize);

        // NumCharDisplay records the number of characters in the composition string (not as reported by the IME
        // but the number of chars as written in the composition string in the text field). This min allows us to 
        // skip doing any editing on the composition string such as highlighting clause, setting cursor position etc
        // if it is not drawn- for example for Quan Pin IME for which the composition characters are instead drawn
        // in the reading window, as opposed to being part of the composition string.
        int numChar = min(dwSize, NumCharDisplay);

        // Obtain reading string information.
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPREADSTR, NULL, 0);
        wchar_t* lpReadStr = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);
        memset(lpReadStr, 0, 2*dwSize);
        ImmGetCompositionString(hIMC, GCS_COMPREADSTR, (LPVOID)lpReadStr, dwSize);
        lpReadStr[dwSize/2] = 0; // Since the IME doesn't always put in the terminating NULL

        // Get cursor position information
        int pos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        ImmReleaseContext(hWND, hIMC);

        /*
        Highlighting scheme:
        Different IME's employ different means to highlight composition text, clause text and 
        composition text selection.
        The process of entering text through IME can be divided into three modes. 
        INPUT MODE: text is being entered using keyboard.
        CONVERSION MODE: The input text is converted into clauses. This is usually done 
        by pushing space key. 
        CANDIDATE MODE: The candidate list is open. Also usually done by pushing space key.

        Japanese IME:
        ------------- 
        In Input Mode, composition text is displayed with a dotted underline. 
        Upon entering conversion mode, the current clause is displayed with a bold underline 
        while the rest of the composition string is displayed with a regular underline. With 
        Japanese, the user can change the length of the current clause using shift + arrow keys. 
        When this is done, the clause is displayed selected with the ATTR_TARGET_NOTCONVERTED 
        bit set in the attribute string. Using shift+arrow key can't be used to select 
        composition text in Japanese mode. 

        Other IME's (Chinese Traditional and Simplified)
        ------------------------------------------------
        There doesn't seem to be any CONVERSION MODE. The user inputs text in INPUT Mode which 
        is displayed with a dotted underline. The cursor can be moved around using arrow keys 
        and upon entering CANDIDATE MODE (usually by pusing space or down arrow key), the candidate
        list appears under the character behind the caret. Since there is no concept of a clause,
        there is no INPUT MODE -> CONVERT MODE -> CANDIDATE MODE transition. The user can revert
        back to the INPUT MODE from CANDIDATE MODE using escape key. Shift + arrow keys can be used
        highlight composition text. The span of the highlighted text is indicated using the 
        ATTR_TARGET_CONVERTED bit which in case of japanese denotes current clause. 

        NOTE: It's important to distinguish between Japanese and other IME's when shift + arrow
        keys are used to select text. In Japanese IME, the non clause text needs to be displayed
        with a solid underline while the clause is displayed highlighted. With other IME's, 
        the non selected text appears with a dotted underline.

        NOTE ON MOUSE CLICK: If the user clicks on the composition string (in MSWORD)
        the cursor is repositioned to the position of the mouse click. If the click
        happens over non-composition string region, current composition is finalized.
        currently we don't support this behaviour- we finalize every time a mouse click
        takes place.
        */

        // Set cursor position in the composition string
        pimeMgr->SetCursorInComposition((UPInt)pos);

        // Take care of highlighting clauses etc.
        // Note: indexes for arrays/strings should be UPInt to eliminate warnings for 64 bit comps.
        // UPInt start = 0; 
        // bool clauseFlag = false;

        struct styleInfo style;

        Array<styleInfo> styleList;

        char currentStyle   = lpastr[0];
        ClausePosition      = numChar;
        style.begin         = 0; 
        style.highLightStyle = lpastr[0];

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

        if(!(IMETag & (UInt32)GFxIME_Kr_Flag)) // No highlighting necessary for Korean.
        {
            for(UPInt i = 0; i < styleList.GetSize(); i++)
            {
                UInt32 highlightStyle  = styleList[i].highLightStyle;
                UInt32 start         = styleList[i].begin;
                UInt32 len           = styleList[i].end - styleList[i].begin + 1;

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

        if (IMETag == GFxIME_Ch_Simp_MSPinyin_3_0)
        {
            // For Microsoft Simpified Chinese Pinyin 3.0, when the candidate list is open and the selected row 
            // contains more than two characters and the user moves to the next clause using arrow keys, the 
            // candidate list processing message is sent earlier than the clause update message leading to the 
            // candidate list opening above the previous clause. Therefore, for this IME, we update the position
            // of the list again here. 
            RectF viewRect, caretRect;
            pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
           // if (CandListPath.GetLength() > 0)
			{
				// Value* valArray = (Value*)SF_ALLOC( 3*sizeof(Value), StatIME_Mem);
				Array<Value> valArray;
				Value result;
				valArray.PushBack(Value(caretRect.TopLeft().x));
				valArray.PushBack(Value(caretRect.BottomRight().y));
				valArray.PushBack(Value(caretRect.Height()));
				Invoke("RepositionCandWindow", &result, valArray.GetDataPtr(), 3);
			//	pMovie->Invoke((CandListPath + ".RepositionCandWindow").ToCStr(), "%d %d %d", (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, (int)caretRect.Height());

			}
        }

        DisplayReadingWindow((UInt32)keyCode, numChar, lpastr, lpReadStr);
        if (lpReadStr)
            SF_FREE(lpReadStr);

        if (lpastr)
            SF_FREE(lpastr);
    }

    if (info == 0x0180) // This is a weird special case.. If this happens, turn the text in the 
        // reading window to red. This is applicable to Chinese Taiwan (DaYi, ChangJie)
    {
        if(CandListPath.GetLength() != 0)
		{
			Value result;
			Invoke("RemoveInputWindow", &result, 0, 0);
            // pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
		}
        int utf8Len     = (int)UTF8Util::GetEncodeStringSize(&ReadingTextBuffer[0], -1);
        char* utf8Str   = (char*)SF_ALLOC(utf8Len+1, StatIME_Mem);
        UTF8Util::EncodeString(utf8Str, &ReadingTextBuffer[0], -1);
        // Get IMEManager
        Ptr<IMEManagerBase> pimeMgrReading = pMovie->GetIMEManager();
        SF_ASSERT(pimeMgrReading && pimeMgrReading->IsMovieActive(pMovie));
        RectF viewRect, caretRect;
        // Obtain position of the Reading Window
        pimeMgrReading->GetMetrics(&viewRect, &caretRect, -ClausePosition);
        // Params: 1: Reading String 2,3: x-y position of the Reading Window 4: Color of Reading text. 
       // if (CandListPath.GetLength() > 0)
		{
			Array<Value> valArray;
			// Value* valArray = (Value*)SF_ALLOC( 4*sizeof(Value), StatIME_Mem);
			Value result;
			valArray.PushBack(Value(utf8Str));
			valArray.PushBack(Value(caretRect.TopLeft().x));
			valArray.PushBack(Value(caretRect.BottomRight().y));
			valArray.PushBack(Value((float)(0)));
			Invoke("DisplayInputWindow", &result, valArray.GetDataPtr(), 4);
        //pMovie->Invoke((CandListPath + ".DisplayInputWindow").ToCStr(), "%s %d %d %d", utf8Str, (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, 0);
		}
        ReadingTextBuffer[0] = 0;
        SF_FREE (utf8Str);
    }

}

void GFxIMEXP::DisplayCompositionString(UInt32& numCharDisplay)
{
    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));

    Value    val; 
    int         pos;

    HIMC hIMC = ImmGetContext(hWND);
    int dwSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
    wchar_t* lpstr = (wchar_t*)SF_ALLOC(2*(dwSize+1), StatIME_Mem);
    ImmGetCompositionString(hIMC, GCS_COMPSTR, (LPVOID)lpstr, dwSize);
    lpstr[dwSize/2] = 0;
    // Scan for space and replace with non breaking space.
    for(int i = 0; i < dwSize/2; i++)
    {
        if(lpstr[i] == 0x20 || lpstr[i] == 0x3000)
            lpstr[i] = 160; // Insert non breaking space

    }
  
    switch (IMETag)
    {

        // Don't display composition characters in the text field since those will be displayed in the reading
        // Window. Since we didn't display any characters, don't change NumCharDisplay.
    case GFxIME_Ch_Simp_QuanPin:    
    case GFxIME_Ch_Simp_ShuangPin:  
    case GFxIME_Ch_Simp_ZhengMa:    
    case GFxIME_Ch_Trad_ChangJie:   
    case GFxIME_Ch_Trad_Phonetic:   
    case GFxIME_Ch_Trad_DaYi:   
	case GFxIME_Ch_Trad_Quick:
    case GFxIME_Ch_Trad_WuXia:
	case GFxIME_Ch_Simp_ABC:

        pimeMgr->SetCompositionText(L"");
        numCharDisplay = 0;
        break;

    case GFxIME_Ch_Trad_Array:
	
    case GFxIME_Ch_Trad_NewPhonetic:   
    case GFxIME_Ch_Trad_NewChangJie: 
	case GFxIME_Ch_Trad_NewChangJie2010: 
    case GFxIME_Jp:
    case GFxIME_Jp_2002:      
    case GFxIME_Jp_2003:
    case GFxIME_Jp_2007:
	case GFxIME_Jp_2010:
    case GFxIME_Jp_ATOK2008:
	case GFxIME_Jp_ATOK2009:
    case GFxIME_Ch_Simp_MSPinyin_3_0: 
    case GFxIME_Ch_Simp_MSPinyin_2007: 
	case GFxIME_Ch_Simp_MSPinyin1_2010:
	case GFxIME_Ch_Simp_MSPinyin2_2010:
	case GFxIME_Ch_Trad_NewChewing:

        pimeMgr->SetCompositionText(lpstr);
        numCharDisplay = dwSize/2;
        break;

    case GFxIME_GooglePinyin:  
        // Note that for this IME, no attribute information is sent, so cursor position and 
        // composition string attribute must be set here.
        pimeMgr->SetCompositionText(lpstr);
        numCharDisplay = dwSize/2;
        // Get cursor position information
        pos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        // Set cursor position in the composition string
        pimeMgr->SetCursorInComposition((UPInt)pos);
        ClausePosition      = dwSize/2 - pos;
        pimeMgr->HighlightText(0, dwSize/2, IMEManagerBase::THS_CompositionSegment, 0);
        break;

    case GFxIME_SogouPinyin:          
    case GFxIME_Ch_Simp_Pinyin03:
    case GFxIME_Ch_Simp_QQPinyin:
    case GFxIME_Ch_Simp_NianQing:
    case GFxIME_Ch_Simp_JJ:

        // Note that for this IME, no attribute information is sent, so cursor position and 
        // composition string attribute must be set here.
        pimeMgr->SetCompositionText(lpstr);
        numCharDisplay = dwSize/2;
        // Get cursor position information
        pos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        // Set cursor position in the composition string
        pimeMgr->SetCursorInComposition((UPInt)pos);
        ClausePosition      = dwSize/2 - pos;
        pimeMgr->HighlightText(0, dwSize/2, IMEManagerBase::THS_CompositionSegment, 0);
        break;

    default:
        break;
    }

    ImmReleaseContext(hWND, hIMC);
    if (lpstr)
        SF_FREE(lpstr);

}

void GFxIMEXP::DisplayReadingWindow(UInt32 keyCode, int numChar, char* lpastr, wchar_t* lpReadStr)
{
    // The English letters only need to be displayed when the keyCode == 
    //  0x3000 (Chinese space) and don't put return and space in the English character list
    // LastCharacter is the last keyboard input char (ascii)

    Value    val; 
    int         utf8Len;
    char*       utf8Str;
    int         readStrLen;

    // Remove previous input window..
    if(CandListPath.GetLength() != 0)
	{
		Value result;
		Invoke("RemoveInputWindow", &result, 0, 0);
    //    pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
	}

    if (LastCharacter == 0x1b)
    {     // Escape key
        ReadingTextBuffer[0] = 0; 
        return;
    }

    int len = (int)wcslen(&ReadingTextBuffer[0]);

    switch (IMETag)
    {
    case GFxIME_Ch_Trad_WuXia:
    case GFxIME_Ch_Simp_QuanPin:    
    case GFxIME_Ch_Simp_ShuangPin:  
    case GFxIME_Ch_Simp_ZhengMa:  
    case GFxIME_Ch_Simp_WuBi86:
    case GFxIME_Ch_Simp_WuBi98:
	case GFxIME_Ch_Simp_ABC:
	case GFxIME_Ch_Trad_NewChewing:

        readStrLen = (int)wcslen(lpReadStr) ;
        memcpy(&ReadingTextBuffer[0], lpReadStr, 2*readStrLen);
        ReadingTextBuffer[readStrLen] = 0;
        break;

    case GFxIME_GooglePinyin:

        if (LastCharacter == 0x08) //backspace
        {
            if(len > 0)
            {
                ReadingTextBuffer[len-1] = 0;
            }
        }
        else
        {
            // Ignore everything that's not lowercase english since stupid IME sends us composition message for 
            // most keypresses- even those that don't modify the reading string.
            if ((LastCharacter >= 'a') && (LastCharacter <= 'z')) 
            {
                ReadingTextBuffer[len] = LastCharacter;
                ReadingTextBuffer[len+1] = 0;
            }           
        }
        break;

    case GFxIME_Ch_Simp_MSPinyin_3_0:  
    case GFxIME_Ch_Simp_MSPinyin_2007:
	case GFxIME_Ch_Simp_MSPinyin1_2010:
	case GFxIME_Ch_Simp_MSPinyin2_2010:
    case GFxIME_Ch_Simp_MSPinyin:  
        {   
            if ((keyCode == 0x3000))
            {
                // Find first character with attrib = ATTR_TARGET_CONVERTED
                if (LastCharacter == 0x08) //backspace
                {
                    if(len > 0)
                    {
                        ReadingTextBuffer[len-1] = 0;
                    }
                }
                else if (LastCharacter == 0x0d || LastCharacter == 0x20) //return or space
                {
                    //Ignore
                }
                else
                {
                    int i = 0;
                    while(i < numChar && lpastr[i] != ATTR_TARGET_CONVERTED) i++;
                    if (i == numChar)return; // No reading string needs to be displayed
                    if (CurrChar == i)
                    {
                        ReadingTextBuffer[len] = LastCharacter;
                        ReadingTextBuffer[len+1] = 0;
                    }
                    else
                    {
                        CurrChar = i;
                        ReadingTextBuffer[0] = LastCharacter;
                        ReadingTextBuffer[1] = 0;
                    }
                }
            }

            else
            {
                ReadingTextBuffer[0] = 0;
            }
            break;

        }

    case GFxIME_Ch_Trad_NewChangJie:  //  Chinese (Traditional) - New ChangJie
	case GFxIME_Ch_Trad_NewChangJie2010:
        {
            if (keyCode != 0x3000)
            {
                ReadingTextBuffer[0] = 0;
                return; // the last character has been converted, so no need for displaying reading window
            }
            if (LastCharacter != 0x0d && LastCharacter != 0x20 && LastCharacter != 0x08){

                int bufLen = (int)wcslen(&ReadingTextBuffer[0]);
                if (bufLen <= 4) // To limit the number of characters in the input window to 4
                {
                    ReadingTextBuffer[bufLen] = LastCharacter;
                    ReadingTextBuffer[bufLen+1] = 0;
                }
            }
            if (LastCharacter == 0x08) //Backspace
            {
                int backspaceLen = (int)wcslen(&ReadingTextBuffer[0]);
                if(backspaceLen > 0)
                    ReadingTextBuffer[backspaceLen-1] = 0;
            }

            break;
        }
    case GFxIME_Ch_Trad_NewPhonetic:

        if (ReadingWindowCharacterPos > 4)
        {
            return;
        }
        if ((keyCode == 0x3000))
        {
            ReadingTextChTradNewPhonetic[ReadingWindowCharacterPos] = LastCharacter;
        }

        else
        {
            for(int i = 0; i < 4; i++)
                ReadingTextChTradNewPhonetic[i] = 0x020;
            ReadingTextChTradNewPhonetic[4] = 0;
            return;

        }   
        SFwcscpy(&ReadingTextBuffer[0], wcslen(&ReadingTextChTradNewPhonetic[0])+1
            , &ReadingTextChTradNewPhonetic[0]);
        break;

    case GFxIME_Ch_Trad_ChangJie:    //  Chinese (Traditional) - ChangJie
    case GFxIME_Ch_Trad_DaYi:    
    case GFxIME_Ch_Trad_Phonetic:    //  Chinese (Traditional) - Phonetic
	case GFxIME_Ch_Trad_Quick:
        // Copy the reading string into text
        SFwcscpy(&ReadingTextBuffer[0], wcslen(lpReadStr)+1, lpReadStr);
        break;

    default:
        break;
    }
    // Now display new one..
    utf8Len = (int)UTF8Util::GetEncodeStringSize(&ReadingTextBuffer[0], -1);
    if(utf8Len == 0) return; // Nothing to do
    utf8Str = (char*)SF_ALLOC(utf8Len+1, StatIME_Mem);
    UTF8Util::EncodeString(utf8Str, &ReadingTextBuffer[0], -1);

    Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    RectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, -ClausePosition);
    // Now pass it on to the DisplayInputWindow function so it knows where to put the reading window. This way
    // DisplayInputWindow is independent of the flash document that contains the text field. 

   // if (CandListPath.GetLength() > 0)
    {
		//Value* valArray = (Value*)SF_ALLOC( 4*sizeof(Value), StatIME_Mem);
		Array<Value> valArray;
		Value result;
		valArray.PushBack(Value(utf8Str));
		valArray.PushBack(Value(caretRect.TopLeft().x));
		valArray.PushBack(Value(caretRect.BottomRight().y));
		valArray.PushBack(Value(caretRect.Height()));
		Invoke("DisplayInputWindow", &result, valArray.GetDataPtr(), 4);
	//	pMovie->Invoke((CandListPath + ".DisplayInputWindow").ToCStr(), "%s %d %d %d", utf8Str, (int)caretRect.TopLeft().x, (int)caretRect.BottomRight().y, (int)caretRect.Height());
    }

    if (utf8Str)
        SF_FREE(utf8Str);

}

void GFxIMEXP::SelectAndClose(int index)
{
    // This function is called when the user clicks on a candidate list row. This action should update the 
    // composition string with the contents of the clicked row as well as close the candidate list.
    // IMM api provides ImmNotifyIME functions to notify IME about the click- but some third party IMEs
    // such as google and sogou don't adhere to the API functions. So this can't be a general purpose solution
    // The other way to do this is to simulate a key press that corresponds to the candidate list row that was
    // clicked. We use the SendInput functions to accomplish this and this works for the IME's I've tested. 
    /*
    Revision: 10Mar09, AMohan: The above is not entirely true for the following reasons:
    1) For some IME's, clicking on a candidate list row is not the same thing as pushing the corresponding number key.
    DaYi IME is a good example. Using DaYi IME, type 'f'. Now push space. You should see a list with 5 elements numbered
    0 to 4. If you push 0, the corresponding character will be output and the candidate list will close. On the other
    hand, if you click on the first row (corresponding to index 0), a secondary candidate list will open

    2) To select elements from the secondary candidate list, the user has to use "shift + number key". Hence, simulating
    mouse clicking by sending just the corresponding number key doesn't quite work for secondary candidate lists.

    IMPORTANT NOTE:

    If we always block ALL the WM_IMESETCONTEXT messages (this can be done by always returning 0 while processing WM_IMESETCONTEXT
    in Win32App.cpp- returning HE_NODEFAULT while processing this message from ImeManager is ok since the first WM_IMESETCONTEXT
    is able to leak through since when the first WM_IMESETCONTEXT is sent, the movie and imemanager have not been created yet)
    the secondary candidate lists do NOT appear. This leaves us with the original problem of how to reliably hide the 
    ime pop ups. At least we now know the trade-offs involved!
    */

    if (IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_SogouPinyin || IMETag == GFxIME_Ch_Simp_JJ || IMETag == GFxIME_Ch_Simp_ABC)
    {
        INPUT input;
        input.type      = INPUT_KEYBOARD;
        input.ki.wVk    = (WORD)(0x30 + (index+1));
        input.ki.wScan  = (WORD)MapVirtualKey(0x30 + (index+1), 0);
        SendInput(1, &input, sizeof(input));
    }
    else
    {
        HIMC hIMC = ImmGetContext(hWND);
        ImmNotifyIME(hIMC, NI_SELECTCANDIDATESTR, 0, CurrentPageStart + index);
        ImmNotifyIME(hIMC, NI_CLOSECANDIDATE, 0, 0);
        ImmNotifyIME(hIMC, NI_SETCANDIDATE_PAGESTART, 0, CurrentPageStart);
        ImmReleaseContext(hWND, hIMC);
    }       
}

void GFxIMEXP::CloseIME()
{
    Cleanup();
   // if (CandListPath.GetLength() > 0 && pMovie)
	{
		Value result;
		Invoke("RemoveInputWindow", &result, 0, 0);
	//	pMovie->Invoke((CandListPath + ".RemoveInputWindow").ToCStr(), 0);
	}
}
}}}
//void    GFxIMEXP::OnEnableIME(bool enable)
//{
//    ImmAssociateContextEx(hWND, NULL, (enable) ? IACE_DEFAULT : 0);
//}

#endif  // SF_NO_IME_SUPPORT
