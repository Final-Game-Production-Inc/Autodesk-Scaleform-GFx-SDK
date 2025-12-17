
/**************************************************************************

Filename    :   GFx_IMEXBox360.cpp
Content     :   Contains declaration for GFxIMEManagerXBox360. 
Created     :   Dec 10, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifndef SF_NO_IME_SUPPORT
#include "GFx_IMEXBox360.h"
#include "GFxPlayerImpl.h"
#include "GFxXBox360KeyMapping.h"

// Internal G Evaluation License Reader
#ifdef G_BUILD_EVAL
#define G_EVAL(x) GFxIME_##x
#define G_LIB_NAME_S "GFxIME"
#include "GValidateEval.cpp"
#else
void    GFxIME_ValidateEvaluation() { }
#endif

static const DWORD NumInputKeymap = sizeof(GFxIMEManagerXBox360::InputKeymap)/sizeof(GFxIMEManagerXBox360::InputKeymap[0]);

GFxIMEManagerXBox360::GFxIMEManagerXBox360()
{
    // Validate license
    GFxIME_ValidateEvaluation();

    InitState   = 0;
    CharsInIME  = 0;
    ListBox = new CandidateListBox();
    CmdManager.Register(ListBox);
    CurrentImeMode  = MODE_OFF;
    CurrentLanguage = MODE_JP;
    IMEOn           = FALSE;
    IMEState        = true;
    ZeroMemory(&ximeJpCreate, sizeof(ximeJpCreate));
    ZeroMemory(&ximeChtCreate, sizeof(ximeChtCreate));
    // GFxMovieRoot* pmovieroot = static_cast<GFxMovieRoot*>(pMovie);
    // GFxLog* plog = pmovieroot->GetCachedLog();
    // plog->LogWarning("Failed to Initialize XBoxIMEManager: Check Dictionary paths\n");

};

GFxIMEManagerXBox360::~GFxIMEManagerXBox360()
{
    ListBox->RemoveAllListItems();
    if (ListBox)
        delete ListBox;

    XIMEREQUEST* request = RequestList.GetFirst();
    while(!RequestList.IsNull(request))
    {
        XIMEREQUEST* nextRequest = RequestList.GetNext(request);
        delete(request);
        request = nextRequest;
    }

    for(UInt i = 0; i < XimeDicJp.GetSize(); i++)
    {
#if _XDK_VER >= 9328
        if(!XimeDicJp[i])
        {
            GFREE((void*)XimeDicJp[i]);
        }
#else
		if(!XimeDicJp[i].fDicOnMemory)
        {
            GFREE((void*)XimeDicJp[i].pcszDicFile);
        }
#endif
    }

    for(UInt i = 0; i < XimeDicCht.GetSize(); i++)
    {
#if _XDK_VER >= 9328
        if(!XimeDicCht[i])
        {
            GFREE((void*)XimeDicCht[i]);
        }
#else
		if(!XimeDicCht[i].fDicOnMemory)
        {
            GFREE((void*)XimeDicCht[i].pcszDicFile);
        }
#endif
    }
};

UInt GFxIMEManagerXBox360::HandleIMEEvent(GFxMovieView* pmovie, const GFxIMEEvent& imeEvent)
{
    // cast to XBox event
    const GFxIMEXBox360Event& xboxEvt = static_cast<const GFxIMEXBox360Event&>(imeEvent);
    UInt        ret = 0;
    
    XINPUT_KEYSTROKE key;
    if(GetEnabled())
    {
        if(xboxEvt.eType == 0)
        {
            ret = PreUpdate();
            return ret;
        }

        if(xboxEvt.eType == 1)
        {
            key.VirtualKey  = xboxEvt.VirtualKey;
            key.Unicode     = xboxEvt.Unicode;
            key.Flags       = xboxEvt.Flags;
            key.UserIndex   = xboxEvt.UserIndex;
            key.HidCode     = xboxEvt.HidCode;

            PostUpdate(key);
        }

        if(xboxEvt.eType == 2)
        {
            GFxString s;
            GetString( s ); // So that Completed gets set to false correctly.
            GetLastKey(&key);
            memcpy((XINPUT_KEYSTROKE*)xboxEvt.pLastKey, &key, sizeof(XINPUT_KEYSTROKE));        
            ret = 1; 
            UInt dummy = 0;
            Render(dummy);
        }
        return GFxMovieView::HE_NotHandled;
    }

    // If IME is disabled, don't pass anything to XIME.
    else 
    {
        if(xboxEvt.eType == 0)
        {
            return GFxMovieView::HE_NotHandled;
        }

        if(xboxEvt.eType == 1)
        {
            key.VirtualKey  = xboxEvt.VirtualKey;
            key.Unicode     = xboxEvt.Unicode;
            key.Flags       = xboxEvt.Flags;
            key.UserIndex   = xboxEvt.UserIndex;
            key.HidCode     = xboxEvt.HidCode;

            Key = key;
        }

        if(xboxEvt.eType == 2)
        {
            GFxString s;
            GetString( s );
           // GetLastKey(&key);
            key = Key;
            memcpy((XINPUT_KEYSTROKE*)xboxEvt.pLastKey, &key, sizeof(XINPUT_KEYSTROKE)); 
            memset( &Key, 0, sizeof(XINPUT_KEYSTROKE) ); // Must reset Key!
            ret = 1; 
        }
        return GFxMovieView::HE_NotHandled;
    }   
}

bool GFxIMEManagerXBox360::SetEnabled(bool enable)
{
    // Reset IME State. 
    IMEState = enable;
    if(enable && IMEOn)
    {
        SetInputLanguage(CurrentLanguage);
    }
    else
    {
        XIMEClose();
        ClearComposition();
        InitState = 0; // XIME should be reinitialized since we called XIMEClose();
    }
    return true;
}

bool GFxIMEManagerXBox360::GetEnabled()
{
    return IMEState;
}

void GFxIMEManagerXBox360::OnEnableIME(bool enable)
{
    SetEnabled(enable);
}

bool GFxIMEManagerXBox360::SetJapaneseDictionaryParameters(X_DICTPTRTYPE* pximeDic, UInt numOfDictionaries, UInt numberOfCandidatesInPage)
{
    // First destroy any existing XimeDicJp

    for(UInt i = 0; i < XimeDicJp.GetSize(); i++)
    {
		
#if _XDK_VER >= 9328

		if(!XimeDicJp[i])
        {
            GFREE((void*)XimeDicJp[i]);
		}
#else
		if(!XimeDicJp[i].fDicOnMemory)
        {
            GFREE((void*)XimeDicJp[i].pcszDicFile);
        }
#endif
        
    }

    XimeDicJp.Resize(numOfDictionaries);

#if _XDK_VER < 9328

    for(UInt i = 0; i < numOfDictionaries; i++)
    {
        XimeDicJp[i]    = pximeDic[i];
        if(!pximeDic[i].fDicOnMemory)
        {
			UInt len        = G_strlen(pximeDic[i].pcszDicFile)+1;
            XimeDicJp[i].pcszDicFile = (LPCSTR)GALLOC(len, GFxStatIME_Mem);
            G_strcpy((char*)XimeDicJp[i].pcszDicFile, len, pximeDic[i].pcszDicFile);
        }
    }

#endif

    ximeJpCreate.dwNumberOfDictionaries = numOfDictionaries;
#if _XDK_VER >= 9328
    ximeJpCreate.pcszDicFile            = &XimeDicJp[0];
#else
	ximeJpCreate.pXimeDictionary           = &XimeDicJp[0];
#endif

    ximeJpCreate.dwNumberOfCandidateListInPage = numberOfCandidatesInPage;
    // The flag will be set when SetInputLanguage() is called with 
    // MODE_JP
    InitState = InitState & ~InitJapaneseSuccessful; 
    
    return true;
}

bool GFxIMEManagerXBox360::SetChineseDictionaryParameters(X_DICTPTRTYPE* pximeDic, UInt numOfDictionaries, UInt numberOfCandidatesInPage)
{
    // First destroy any existing XimeDicCht

    for(UInt i = 0; i < XimeDicCht.GetSize(); i++)
    {
#if _XDK_VER >= 9328

		if(!XimeDicCht[i])
        {
            GFREE((void*)XimeDicCht[i]);
		}
#else
		if(!XimeDicCht[i].fDicOnMemory)
        {
            GFREE((void*)XimeDicCht[i].pcszDicFile);
        }
#endif
    }

    XimeDicCht.Resize(numOfDictionaries);

#if _XDK_VER < 9328

    for(UInt i = 0; i < numOfDictionaries; i++)
    {
     //   XimeDicCht[i]    = pximeDic[i];
        if(!pximeDic[i].fDicOnMemory)
        {
            UInt len        = G_strlen(pximeDic[i].pcszDicFile)+1;
            XimeDicCht[i].pcszDicFile   = (LPCSTR)GALLOC(len, GFxStatIME_Mem);
            G_strcpy((char*)XimeDicCht[i].pcszDicFile, len, pximeDic[i].pcszDicFile);
        }
    }
#endif

    ximeChtCreate.dwNumberOfDictionaries    = numOfDictionaries;
#if _XDK_VER >= 9328
    ximeChtCreate.pcszDicFile            = &XimeDicCht[0];
#else
	ximeChtCreate.pXimeDictionary           = &XimeDicCht[0];
#endif
    ximeChtCreate.dwNumberOfCandidateListInPage = numberOfCandidatesInPage;
    InitState = InitState & ~InitChineseSuccessful; 
    return true;
}

HRESULT GFxIMEManagerXBox360::SetInputLanguage( IMEMODE eLanguage )
{
    // force itself to close XIME before initializing due to re-init.
    XIMEClose();

    // Initialize IME state. 
    NeedBlocking    = FALSE;
    Completed       = FALSE;

    ZeroMemory( &Key, sizeof( Key ) );
    ZeroMemory( &LastKey, sizeof( LastKey ) );
    CompletedString.Clear();
    IMEOn = TRUE;
    SetCurrentMode( MODE_INPUT );

    switch ( eLanguage )
    {
    case MODE_JP:
        {
            CurrentLanguage = MODE_JP;
#if _XDK_VER >= 9328
            if( XIMECreateEx(2, XIMEJPInit, &ximeJpCreate ) != ERROR_SUCCESS )
#else
			if( XIMECreate( XIMEJPInit, &ximeJpCreate ) != ERROR_SUCCESS )
#endif
            {
                InitState = InitState & ~InitJapaneseSuccessful;
                ClearComposition();
                return E_FAIL;
            }
            else
            {
                InitState = InitState | InitJapaneseSuccessful;
            }  
            InputMode = XIME_MODE_JP_ROMAJI_HIRAGANA_WITH_FULLWIDTH_ALPHANUMERIC;
            KeyboardLayout = XIME_LAYOUT_ALPHANUMERIC;
        }
        break;

    case MODE_KR:
        CurrentLanguage = MODE_KR;
#if _XDK_VER >= 9328
        if( XIMECreateEx(2, XIMEKRInit, NULL ) != ERROR_SUCCESS )
#else
		if( XIMECreate(XIMEKRInit, NULL ) != ERROR_SUCCESS )
#endif
        {
            InitState = InitState & ~InitKoreanSuccessful;
            ClearComposition();
            return E_FAIL;
        }
        InputMode = XIME_MODE_KR_HANGUL;
        break;

    case MODE_TC:
        {      
            CurrentLanguage = MODE_TC;
#if _XDK_VER >= 9328
            if( XIMECreateEx(2, XIMECHBopomofoInit, &ximeChtCreate ) != ERROR_SUCCESS )
#else
			if( XIMECreate( XIMECHBopomofoInit, &ximeChtCreate ) != ERROR_SUCCESS )
#endif
            {
                InitState = InitState & ~InitChineseSuccessful;
                ClearComposition();
                return E_FAIL;
            }
            else
            {
                InitState = InitState | InitChineseSuccessful;
            }
            InputMode = XIME_MODE_CHT_BOPOMOFO;
        }
        break;

    default:
        return E_FAIL;
    }

//    IMEOn = TRUE;

    // GotClause must be initialized here and not above the eLanguage switch since
    // it should change state only if initialization is successful. This has an effect
    // on setting Complete in IsStringReady(). Relevant situation:
    // If user is in composition mode and doesn't finalize text and then switches to
    // a different language for which initialization fails (perhaps due to dictionary file
    // not available). Now if the user toggles to english mode, he should be able to type
    // English characters. However, ToggleIME() sets complete to true which causes IsIMEOn() 
    // to return true which in turn causes GetLastKey() to not return the last typed key. 
    // GetString() is responsible for setting Complete to false again, but can only do so
    // if GotClause is true- so we should reset GotClause only if SetInputLanguage() succeeds
    // and shouldn't touch it if it fails.

    GotClause   = FALSE;
    CharsInIME  = 0;
//    SetCurrentMode( MODE_INPUT );

    XIMESetCharacterLimit(30);
    // IMEManager function- Need to tell IMEManager that composition is starting.
    //   StartComposition();
    return S_OK;
}


void GFxIMEManagerXBox360::FsCallBack(GFxMovieView* pmovie, const char* pcommand, const char* parg)
{
    CmdManager.FsCallback(pmovie, pcommand, parg);
}

void GFxIMEManagerXBox360::SetCharacterLimit(UInt dwNumChar)
{
    XIMESetCharacterLimit(dwNumChar);
}

void GFxIMEManagerXBox360::SetMovie(GPtr<GFxMovieView> movie)
{
    pMovie = movie;
    ListBox->RegisterMovieContainingWidget(pMovie);
}

UInt GFxIMEManagerXBox360::PreUpdate()
{
    //Check request queue status
    if( !RequestList.IsNull(RequestList.GetFirst()) )
    {
        XIMEREQUEST* xr = RequestList.GetFirst();
        if( XHasOverlappedIoCompleted( &xr->ov ) )
        {
            // Check error code.
            DWORD dwResult = XGetOverlappedResult( &xr->ov, NULL, false );
            if( dwResult == ERROR_SUCCESS )
            {
                if( xr->dwRequestId == NeedToDisableList )
                    ShowCandidateList = FALSE;
                else if( xr->dwRequestId == NeedToComplete )
                    Complete(this, NULL);
                // Refer to Comment LastKeyUp. For some reason, 
                // dwResult == ERROR_SUCCESS so this simulated KeyUp needs to 
                // be handled here.
                else if( xr->dwRequestId == HandleLastKeyUp )
                {
                    Completed   = TRUE;
                    GotClause   = FALSE; 
                    LastKey     = xr->key;
                }
            }
            else if(IMEOn && dwResult == ERROR_NO_DATA )
            {
                Completed = TRUE;
                GotClause = FALSE; // Setting GotClause = FALSE prevents GetString from resetting Completed.
                // This is important because: Suppose we are in Japanese mode, and the user pushes backspace. In the previous
                // invocation of Update, the backspace key would have been passed on to XIME as well as put at the back of the 
                // request list. We now retrieve the data associated with the last key press (backspace in our example) and 
                // remember it so that GetLastKey() will return the correct result and the text on the screen can be updated
                // properly. This only applies if there are no characters in the composition string. If there are, backspace
                // will be handled by XIME internally and when Render is called, it will draw the composition string with one
                // less character in it. If there are no characters in the composition string, there is nothing for XIME to do
                // and hence, we must pass the backspace key on to regular key stroke processing. 
                LastKey = xr->key;
            }
            XIMEREQUEST* tmp = RequestList.GetFirst();
            RequestList.Remove(tmp); // This is the only place the request is removed from the queue.
            delete(tmp);
        }
        if( NeedBlocking )
            return GFxMovieView::HE_Handled;
    }
    else
        NeedBlocking = FALSE;

    return GFxMovieView::HE_NotHandled;
}

HRESULT GFxIMEManagerXBox360::PostUpdate(XINPUT_KEYSTROKE& key)
{

    HRESULT hr;
    
    // We need this since while using the TID, HidCode == 0 and we can't look up the correct
    // unicode corresponding to the key press. Hence we need to check for HidCode == 0 and fill
    // in the correct unicode ourselves.

    Key = key;

    if(Key.HidCode == 0)
    {
        if(Key.Unicode >= 'a' && Key.Unicode <= 'z')
            Key.HidCode = (BYTE)(Key.Unicode - 'a') + 4;

        if(Key.Unicode >= 'A' && Key.Unicode <= 'Z')
            Key.HidCode = (BYTE)(Key.Unicode - 'A') + 4;

        if(Key.Unicode >= '1' && Key.Unicode <= '0')
            Key.HidCode = (BYTE)(Key.Unicode - '1') + 30;

        if(Key.Unicode == '0')
            Key.HidCode = 39;

        if(Key.Unicode == ',')
            Key.HidCode = 54;

        if(Key.Unicode == '.')
            Key.HidCode = 55;

        if(Key.Unicode == '/')
            Key.HidCode = 56;

        if(Key.Unicode == ';')
            Key.HidCode = 51;

        if(Key.Unicode == '\'')
            Key.HidCode = 52;

        if(Key.Unicode == '-')
            Key.HidCode = 45;

        if(Key.Unicode == '=')
            Key.HidCode = 46;

        if(Key.Unicode == '\\')
            Key.HidCode = 49;

        if(Key.Unicode == '-')
            Key.HidCode = 45;

        if(Key.Unicode == '`')
            Key.HidCode = 53;

    }
    // Just ignore released key
    if( Key.Flags & XINPUT_KEYSTROKE_KEYUP ) 
    {
        if(CallBack)
        {
            memset( &Key, 0, sizeof(XINPUT_KEYSTROKE) ); // Must reset Key!
        }
        return S_OK;
    }

    // Ignore some flags
    // Note that there is an issue with CAPSLOCK in JP mode. After toggling capslock, none of the
    // characters input are converted into Japanese characters. This can be fixed by going through s
    // a switch language cycle (JP->KR->CH->JP). Might want to remove CAPSLOCK flag to prevent
    // this behaviour.
/*
    Key.Flags &= ~( XINPUT_KEYSTROKE_VALIDUNICODE |
        XINPUT_KEYSTROKE_CAPSLOCK |
        XINPUT_KEYSTROKE_NUMLOCK |
        XINPUT_KEYSTROKE_ROMAJI |
        XINPUT_KEYSTROKE_HIRAGANA |
        XINPUT_KEYSTROKE_HANGUL |
        XINPUT_KEYSTROKE_BOPOMOFO |
        XINPUT_KEYSTROKE_CHAJEI |
        XINPUT_KEYSTROKE_REMOTE |
        XINPUT_KEYSTROKE_REPEAT );
*/

    Key.Flags &= ~( XINPUT_KEYSTROKE_VALIDUNICODE |
        XINPUT_KEYSTROKE_CAPSLOCK |
        XINPUT_KEYSTROKE_NUMLOCK |
        XINPUT_KEYSTROKE_ROMAJI |
        XINPUT_KEYSTROKE_HIRAGANA |
        XINPUT_KEYSTROKE_HANGUL |
        XINPUT_KEYSTROKE_BOPOMOFO |
        XINPUT_KEYSTROKE_CHAJEI |
        XINPUT_KEYSTROKE_REMOTE );

    DWORD   dwFlags = CurrentImeMode | CurrentLanguage;
    XIMEKEY eControlKey = XIME_KEY_UNICODE;
    CallBack = NULL;
    int i;

    // Look up Unicode from HID before looking up a keymap table
 
    INT iInputMode = InputMode;
    LayoutConversion( &Key, &iInputMode );

    XIMESetCharacterLimit(30);

    // Look up a table...
    for( i = 0; i < NumInputKeymap; i++ )
    {
        if( (dwFlags & InputKeymap[ i ].dwFlags) == dwFlags
            && Key.VirtualKey == InputKeymap[ i ].wVK
            && ((Key.Flags & ~InputKeymap[ i ].wIgnoredXinputFlags) == InputKeymap[ i ].wXinputFlags) )
        //    && ((InputKeymap[ i ].wXinputFlags & Key.Flags) == InputKeymap[ i ].wXinputFlags) )
        {
            eControlKey = InputKeymap[ i ].eControlKey;
            CallBack = InputKeymap[ i ].function;
            break;
        }
    }

    if( CallBack )
    {
        // Then, invoke callback
        hr = CallBack( this, &InputKeymap[ i ] );

        // In order to insert an Unicode character if hr is S_FALSE.
        eControlKey = XIME_KEY_UNICODE;
    }
    else if( !IMEOn )
    {
        hr = ERROR_SERVICE_DISABLED;
    }
    else
    {
        hr = S_FALSE;
    }

    if( hr == S_FALSE && (Key.Unicode || eControlKey != XIME_KEY_UNICODE) )
    {
        hr = S_OK;
        // eControlKey != XIME_KEY_UNICODE if there is a callback associated with the current keystroke and
        // flag combination, but it is NULL. For example for backspace key. In this case we'll set the unicode for
        // this key to 0 as required by the XIME API and pass it on to XIMEInsertCharacter.

        if( eControlKey == XIME_KEY_UNICODE &&
            (CurrentImeMode == MODE_CONVERT || CurrentImeMode == MODE_ALLCONVERT) )
        { 
            // We can get here if for example we are in Trad Chinese and candidate list is open, and now we push a letter key..
            // there is no callback associated with this key and it has a unicode value. Hence we would want to close the candidate list
            // after updating the composition text to the currently selected row in the candidate list and then enter the current 
            // letter in the composition string. 
            Complete( this, NULL );
            // Need to block since input mode has been changed
            SetCurrentMode( MODE_INPUT );
        }

        XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
        RequestList.PushBack(request);
        
        if( eControlKey != XIME_KEY_UNICODE )
        {
            // API does not accept Unicode value except when using XIME_KEY_UNICODE
            Key.Unicode = 0x0;
        }
        // save the keystroke for pop back
        RequestList.GetLast()->key = Key;

        XIMEInsertCharacter( Key.Unicode,
            eControlKey,
            iInputMode,
            &RequestList.GetLast()->ov );
    }
    return hr;
}

HRESULT GFxIMEManagerXBox360::Convert( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    
    //Input some letters
   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    // save the keystroke for pop back
    pXIME->RequestList.GetLast()->key = pXIME->Key;

    if( !pXIME->CharsInIME )
    {
        // Just input white space
        XIMEInsertCharacter( L' ',
            XIME_KEY_UNICODE,
            pXIME->InputMode,
            &pXIME->RequestList.GetLast()->ov );

    }
    else
    {
        pXIME->SetCurrentMode( MODE_CONVERT );

        // do convert
        XIMEInsertCharacter( 0,
            XIME_KEY_CONVERT,
            pXIME->InputMode,
            &pXIME->RequestList.GetLast()->ov );

        // Build candidate list
        BuildCandidateList( pXIME, pKey );
        if (pXIME->CandListPath.GetLength() > 0)
        {
            pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
            pXIME->pMovie->Invoke((pXIME->CandListPath + ".CreateList").ToCStr(), "%d %d", 5, 1);
        }
    }

    return S_OK;
}


HRESULT GFxIMEManagerXBox360::NonConvert( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    // Start the convert 
    Convert( pXIME, pKey );
    pXIME->SetCurrentMode( MODE_CONVERT );

    // And set the XIMEKEY
    SetCandidate( pXIME, pKey );

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::ChangeFocus( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_CONVERT );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    BOOL bMoveToRight = ( pKey->wVK == VK_RIGHT ? TRUE : FALSE );
    XIMEMoveClauseFocus( bMoveToRight, &pXIME->RequestList.GetLast()->ov );

    BuildCandidateList( pXIME, pKey );

    return S_OK;
}


HRESULT GFxIMEManagerXBox360::ChangeClauseLength( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_CONVERT );

    XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    BOOL bExpand = ( pKey->wVK == VK_RIGHT ? TRUE : FALSE );
    XIMEChangeClauseLength( bExpand, &pXIME->RequestList.GetLast()->ov );

    BuildCandidateList( pXIME, pKey );

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::Revert( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_CONVERT );

    pXIME->SetCurrentMode( MODE_INPUT );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);
    // Reverts a converted string to alphanumeric string. This method works for Japanese only 
    // according to XIME documentation
    XIMERevertString( &pXIME->RequestList.GetLast()->ov );

    // close any candidate list that might be open
    if (pXIME->CandListPath.GetLength() > 0)
    {
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
    }
    return S_OK;
}

HRESULT GFxIMEManagerXBox360::SetCandidate( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_CONVERT );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    XIMESetCandidateIndex( (XIMEINDEX)pKey->eControlKey,
        0,
        pXIME->CurrentLanguage == MODE_JP,
        &pXIME->RequestList.GetLast()->ov );

    pXIME->ShowCandidateList = TRUE;

    // Candidate list needs to be created here also, since when user changes between clauses, SetCandidate gets 
    // called. (Not Convert)
    if (pXIME->CandListPath.GetLength() > 0)
    {
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".CreateList").ToCStr(), "%d %d", 5, 1);
    }
    return S_OK;
}

HRESULT GFxIMEManagerXBox360::BuildCandidateList( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    XIMEBuildCandidateList( &pXIME->RequestList.GetLast()->ov );

 //   pXIME->pMovie->Invoke((pXIME->CandListPath + ".MakeInvisible").ToCStr(), "");
    if (pXIME->CandListPath.GetLength() > 0)
    {
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
    }
    // Since for chinese GFxIMEManagerXBox360, when one enters convert mode (by pushing space for example), candidate list automatically opens
    // This is not true for japanese where one has to open the list for every clause separately by pushing space. 
    pXIME->ShowCandidateList = (pXIME->CurrentLanguage == MODE_TC);

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::Complete( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);
    // This is 
    if( pXIME->CurrentLanguage == MODE_TC &&
        pXIME->CurrentImeMode == MODE_CONVERT &&
        pKey != NULL )
    {
        // We get here if we are in chinese traditional mode and the candidate list is open.
        // if the user pushes enter key, we must first tell IME about the currently selected row
        // and set NeedBlocking to true. We also put a new Request at the back of the request
        // queue with requestId = NeedToComplete. Later as we retrieve this request and process it
        // in update(), we call complete() again with pKey = NULL which causes IME to finalize. 
        // Important note: If in TC the list is open and the user pushes a letter key, the composition
        // will be finalized with the current contents of the composition string, not the currently
        // selected row in the candidate list. This is incorrect behaviour, but reflects that of XIME
        // application, so we replicate it for now until MS corrects it. 
        
        pXIME->RequestList.GetLast()->dwRequestId = NeedToComplete;
        pXIME->NeedBlocking = TRUE;

        XIMESetCandidateIndex( XIME_INDEX_CURRENT, 0, TRUE, &pXIME->RequestList.GetLast()->ov );
    }
    else
    {
        // We can get here for example if in Chinese traditional, the candidate list is open and the user inputs a key. We need 
        // to complete IME, change back to input mode, Note that pKey == NULL and so we just put an empty keystroke on the queue.
        // the actual keystroke is put on the queue later.

        // We can also get here if ime is in input mode and the user pushes enter. In this case, Key = Enter key 
        // which is put on the queue and retrieved later on in update. This mechanism allows the enter key to work
        // even in input mode. 
        
        pXIME->RequestList.GetLast()->key = (pKey != NULL)? pXIME->Key : XINPUT_KEYSTROKE();
        XIMEInsertCharacter( 0,
            XIME_KEY_COMPLETE,
            pXIME->InputMode,
            &pXIME->RequestList.GetLast()->ov );

        // We always change the mode to MODE_INPUT, and
        // we will get the completed string after XIME_KEY_COMPLETE is done.
        pXIME->SetCurrentMode( MODE_INPUT );
        pXIME->Completed = TRUE;
    }

    // Remove candidate window if any
    if (pXIME->CandListPath.GetLength() > 0)
    {
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
    }
    pXIME->ListBox->RemoveAllListItems();

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::HandleNumpadConversion( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );

    if( !pXIME->ShowCandidateList )
        return S_FALSE;

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    DWORD dwIndex = (pKey->wVK >= VK_NUMPAD0)? pKey->wVK-VK_NUMPAD1 : pKey->wVK-VK_1;
    if( dwIndex > 9)
        dwIndex = 10;

    DWORD dwRet = XIMESetCandidateIndex( XIME_INDEX_CURRENT_PAGE_OFFSET,
        dwIndex, true,
        &pXIME->RequestList.GetLast()->ov );

    // we can't process the next command until XIMESetCandidateIndex completes successfully
    if( dwRet == ERROR_SUCCESS || dwRet == ERROR_IO_INCOMPLETE )
    {
        if( pXIME->CurrentLanguage == MODE_JP )
        {
            pXIME->RequestList.GetLast()->dwRequestId = NeedToDisableList;
        }
        else if( pXIME->CurrentLanguage == MODE_TC )
        {
            // CHT Bopomofo needs to complete after the candidate index is updated.
            pXIME->RequestList.GetLast()->dwRequestId = NeedToComplete;
            pXIME->NeedBlocking = TRUE;
        }
    }
    return S_OK;
}

HRESULT GFxIMEManagerXBox360::AllConvert( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );

    pXIME->SetCurrentMode( MODE_ALLCONVERT );
    //Input some letters
   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    // save the keystroke for pop back 
    pXIME->RequestList.GetLast()->key = pXIME->Key;
    XIMEInsertCharacter( 0,
        pKey->eControlKey,
        pXIME->InputMode,
        &pXIME->RequestList.GetLast()->ov );

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::AllRevert( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_ALLCONVERT );

    pXIME->SetCurrentMode( MODE_INPUT );

   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    pXIME->RequestList.PushBack(request);

    // save the keystroke for pop back when this call is not taken the key.
    pXIME->RequestList.GetLast()->key = pXIME->Key;

    XIMEInsertCharacter(  0,
        XIME_KEY_REVERT,
        pXIME->InputMode,
        &pXIME->RequestList.GetLast()->ov );

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::ChangeInputMode( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );
    GASSERT( pXIME->CurrentImeMode == MODE_INPUT );

    pXIME->InputMode = pKey->eControlKey;

    return S_OK;
}

void    GFxIMEManagerXBox360::SetCurrentMode( IMEMODE mode )
{
    GASSERT( mode == MODE_OFF || mode == MODE_CONVERT || mode == MODE_INPUT || mode == MODE_ALLCONVERT );

    if( CurrentImeMode != mode )
    {
        NeedBlocking = TRUE;
    }
    CurrentImeMode = mode;
}

HRESULT GFxIMEManagerXBox360::SwitchLanguage(GFxIMEManagerXBox360 * pXIME)
{
    if( !pXIME->IMEOn ) return S_OK;
    
    // Remove any pop-ups if they exist. 
    if (pXIME->CandListPath.GetLength() > 0)
    {
        pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
    }
    
    pXIME->ListBox->RemoveAllListItems();

    switch( pXIME->CurrentLanguage )
    {
    case MODE_JP:
        pXIME->SetInputLanguage( MODE_KR );
        pXIME->BroadcastSwitchLanguage("Korean");
        break;
    case MODE_KR:
        pXIME->SetInputLanguage( MODE_TC );
        pXIME->BroadcastSwitchLanguage("Chinese");
        break;
    case MODE_TC:
        pXIME->SetInputLanguage( MODE_JP );
        pXIME->BroadcastSwitchLanguage("Japanese");
        break;
    default:
        return E_FAIL;
    }

    return S_OK;
}

const GFxString GFxIMEManagerXBox360::GetInputLanguage()
{
    if(CurrentImeMode == MODE_OFF) 
        return GFxString("English");
    switch(CurrentLanguage )
    {
    case MODE_JP:
        return GFxString("Japanese");
        break;
    case MODE_KR:
        return GFxString("Korean");
        break;
    case MODE_TC:
        return GFxString("Chinese");
        break;
    default:
        return GFxString("Unknown");
    }

}

HRESULT GFxIMEManagerXBox360::ToggleIME( GFxIMEManagerXBox360 * pXIME )
{
    GASSERT( pXIME != NULL );
/*
    if ( ( pXIME->CurrentLanguage == MODE_JP ) && pXIME->Key.VirtualKey )   // if it's not from Gamepad
    {
        if ( XGetLanguage() == XC_LANGUAGE_JAPANESE )
        {
            if ( pXIME->Key.Unicode )                                         // no change if it's VK_OEM_3 when dash is Japanese
                return pXIME->IMEOn ? S_FALSE : S_OK;
        }
        else
        {
            if ( pXIME->Key.Unicode != L'`' )                                 // only toggle GFxIMEManagerXBox360 if it's [~/`] key when dash is non-Japanese
                return pXIME->IMEOn ? S_FALSE : S_OK;
        }
    }
*/
    if( pXIME->IMEOn )
    {
        // We Flush as GFxIMEManagerXBox360 toggle behavior.
       XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
        pXIME->RequestList.PushBack(request);

        XIMEInsertCharacter( 0,
            XIME_KEY_FLUSH,
            pXIME->InputMode,
            &pXIME->RequestList.GetLast()->ov );

        pXIME->Completed = TRUE;
        // Need to block since input mode has been changed
        pXIME->SetCurrentMode( MODE_OFF );
        pXIME->ClearComposition();
        pXIME->IMEOn = FALSE;

        // drop this keystroke.
        ZeroMemory( &pXIME->Key, sizeof( XINPUT_KEYSTROKE ) );
        if (pXIME->CandListPath.GetLength() > 0)
        {
            pXIME->pMovie->Invoke((pXIME->CandListPath + ".RemoveList").ToCStr(), "");
        }
        pXIME->ListBox->RemoveAllListItems();
        pXIME->BroadcastSwitchLanguage("English");
    }
    else
    {
        pXIME->SetCurrentMode( MODE_INPUT );
        pXIME->IMEOn = TRUE;
        switch (pXIME->CurrentLanguage)
        {
        case MODE_JP:
            if(!(pXIME->InitState & InitJapaneseSuccessful))
                pXIME->SetInputLanguage(MODE_JP);
            pXIME->BroadcastSwitchLanguage("Japanese");
            break;
        case MODE_KR:
            if(!(pXIME->InitState & InitKoreanSuccessful))
                pXIME->SetInputLanguage(MODE_KR);
            pXIME->BroadcastSwitchLanguage("Korean");
            break;
        case MODE_TC:
            if(!(pXIME->InitState & InitChineseSuccessful))
                pXIME->SetInputLanguage(MODE_TC);
            pXIME->BroadcastSwitchLanguage("Chinese");
            break;
        }
    }

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::ToggleKeyboardLayout( GFxIMEManagerXBox360 * pXIME, XIME_KEYMAPPING * pKey )
{
    GASSERT( pXIME != NULL );
    GASSERT( pKey != NULL );

    if( pXIME->CurrentLanguage == MODE_JP )
    {
        if( pXIME->KeyboardLayout == XIME_LAYOUT_ALPHANUMERIC )
        {
            pXIME->KeyboardLayout = XIME_LAYOUT_KANA;
        }
        else
        {
            pXIME->KeyboardLayout = XIME_LAYOUT_ALPHANUMERIC;
        }
    }

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::LayoutConversion( XINPUT_KEYSTROKE* pKey, INT* pInputMode )
{

    if( CurrentLanguage == MODE_JP )
    {
        if( KeyboardLayout == XIME_LAYOUT_KANA )
        {
            HidUnicodeLookUp( pKey,
                IMEOn,
                InputMode == XIME_MODE_JP_ROMAJI_KATAKANA_WITH_FULLWIDTH_ALPHANUMERIC,
                JapaneseHiraganaNonAlphabetic,
                JapaneseHiraganaAlphabetic );
            *pInputMode = XIME_LANGUAGE_JAPANESE | XIME_LAYOUT_KANA |
                ( pKey->Flags & XINPUT_KEYSTROKE_SHIFT ? XIME_COMBINE_KANA_SHIFT : 0 );
        }
        else
        {
            // for ten-key input
            HidUnicodeLookUp( pKey, FALSE, FALSE, NULL, NULL );
        }
    }
    else if( CurrentLanguage == MODE_KR )
    {
        if( XGetLanguage() != XC_LANGUAGE_KOREAN && pKey->HidCode == 0xe6 ) // Right-ALT key
        {
            pKey->VirtualKey = VK_HANGUL;
            pKey->Flags &= ~( XINPUT_KEYSTROKE_ALT | XINPUT_KEYSTROKE_ALTGR );
        }
        else
        {
            HidUnicodeLookUp( pKey,
                IMEOn,
                FALSE,
                KoreanNonAlphabetic,
                KoreanAlphabetic );
        }
        *pInputMode = XIME_LANGUAGE_HANGUL;// | XIME_LAYOUT_HANGUL;
    }
    else if( CurrentLanguage == MODE_TC )
    {
        HidUnicodeLookUp( pKey,
            IMEOn,
            FALSE,
            ChineseTraditionalBopomofoNonAlphabetic,
            ChineseTraditionalBopomofoAlphabetic );
        *pInputMode = XIME_LANGUAGE_BOPOMOFO; // | XIME_LAYOUT_BOPOMOFO;
    }

    return S_OK;
}

VOID GFxIMEManagerXBox360::HidUnicodeLookUp( XINPUT_KEYSTROKE* pKey, BOOL fImeOn, BOOL fKana,
                           HID_LOOKUP_TABLE * pTable, HID_LOOKUP_TABLE * pTable2 )
{
    HID_LOOKUP_TABLE *pLookupTable = NULL;
    DWORD  dwHidCode = pKey->HidCode;

    // always look up even if GFxIMEManagerXBox360 is off
    if( ( dwHidCode >= 0x54 ) && ( dwHidCode <= 0x63 ) )
    {
        // convert ten-key
        pKey->Unicode = TenKeyTable[ dwHidCode - 0x54 ];
    }

    else if( !fImeOn )
    {
        // GFxIMEManagerXBox360 is off
    }

    // look up when GFxIMEManagerXBox360 is on
    else if( pTable && dwHidCode >= pTable[ 0 ].wHiragana && dwHidCode <= pTable[ 0 ].wHiraganaShifted )
    {
        pLookupTable = pTable;
    }
    else if( pTable2 &&  dwHidCode >= pTable2[ 0 ].wHiragana && dwHidCode <= pTable2[ 0 ].wHiraganaShifted )
    {
        pLookupTable = pTable2;
    }
    else if( dwHidCode == 0x87 )
    {
        //
        // Handle the special Japanese international keycode for the prolonged sound mark
        //
        pKey->Unicode = fKana ? 0x30ED : 0x308D; // ro character
    }
    else if( dwHidCode == 0x89 )
    {
        pKey->Unicode = 0x30FC; // Prolonged sound mark
    }

    if( pLookupTable )
    {
        dwHidCode -= pLookupTable[ 0 ].wHiragana-1;
        if( fKana )
        {
            if(pKey->Flags & XINPUT_KEYSTROKE_SHIFT)
                pKey->Unicode = pLookupTable[ dwHidCode ].wKatakanaShifted;
            else
                pKey->Unicode = pLookupTable[ dwHidCode ].wKatakana;
        }
        else
        {
            if(pKey->Flags & XINPUT_KEYSTROKE_SHIFT)
                pKey->Unicode = pLookupTable[ dwHidCode ].wHiraganaShifted;
            else
                pKey->Unicode = pLookupTable[ dwHidCode ].wHiragana;
        }
    }

    return;
}

HRESULT GFxIMEManagerXBox360::Render(UInt& numClauses)
{

    DWORD   dwRet;
    DWORD   dwFocus;
    DWORD   dwCursor;
    DWORD   dwClauses;
    DWORD   dwDeterminedClause;

    DWORD   dwNumCandidate = 0;
    DWORD   dwCandidateIndex = 0;
    DWORD   dwHighLightIndexInlist = 0;
    int     compStringLength = 0;

    bool    bListAvailable = false;

    // we need to get the clause info until XIME will complete
    // even if the ime state is off.

    if( !IsIMEOn())
    {
        return ERROR_SERVICE_DISABLED;
    }

    // clear for XIMEGetClauseInfo behavior
    dwNumberOfCandidateStringsInList = 0;

    // Retrieve current clause information of GFxIMEManagerXBox360

    // dwDeterminedClause = 0 when the composition is finalized. At all other times, it's XIME_NO_DETERMINED_CLAUSE.
    // so this parameter can be used to determine when to finalize GFxIMEManagerXBox360. 

    dwRet = XIMEGetClauseInfo( &dwFocus, // Clause with the current focus
        &dwClauses,                      // Total number of clauses
        &dwCursor,
        &dwDeterminedClause,
        StringBuffer,                 // Pointer to buffer that will receive clause strings.
        sizeof( StringBuffer ) / sizeof( WCHAR ), // Size of clause string buffer 
        NULL,
        &dwNumberOfCandidateStringsInList, // The number of candidates in the list attached to this clause
        NULL,
        0 );

    if( dwRet != ERROR_SUCCESS ) 
    { 
        CharsInIME = 0; 
        FinalizeComposition(L"", 0); // In case a composition string exists.
        return E_FAIL; 
    };
    
    numClauses = dwClauses; // Only do this if XIMEGetClauseInfo succeeds otherwise numClauses will be filled with junk value.
    // Chinese XIME has the auto-conversion behavior.
    // so, the current mode has to be controlled by Chinese XIME
    if( CurrentLanguage == MODE_TC && CurrentImeMode != MODE_OFF)
    {
        if( dwNumberOfCandidateStringsInList > 0 )
        {
            SetCurrentMode( MODE_CONVERT );
            ShowCandidateList = TRUE;
        }
        else
        {
            SetCurrentMode( MODE_INPUT );
        }
    }

    // Retrieve candidate list if GFxIMEManagerXBox360 is in conversion mode
    if( CurrentImeMode == MODE_CONVERT )
    {
        dwRet = XIMEGetCandidateListInfo( CandidateListBuffer,
            sizeof( CandidateListBuffer ) / sizeof( WCHAR ),
            NULL,
            &dwCandidateIndex, // Index of the currently highlighted string in the displayed candidate page. 
            &dwNumCandidate,   // Number of candidates displayed on a page
            &dwHighLightIndexInlist, // index of currently highlighted string within the full candidate list.
            &dwNumberOfCandidateStringsInList ); // Number of candidates in the full candidate list. 
        if( dwRet == ERROR_SUCCESS ) bListAvailable = true;
    }

    dwNumberOfCandidateStringsInPage = dwNumCandidate; // Number of strings in current page.

    WCHAR * pString = StringBuffer; // Buffer containing the candidate strings.

    // we get the determined string exactly
    // instead of getting it after completed signal of XIME_KEY_COMPLETE
    GotClause = TRUE;

    // dwDeterminedLength is equal to the length of the finalized composition when the composition is finalized, 
    // otherwise it's zero. 
    DWORD  dwDeterminedLength;
    if( dwDeterminedClause == 0 )
    {
    //    CompletedString     += pString;
        dwDeterminedLength  = wcslen( pString );
        Completed        = TRUE;
    }
    else
    {
        dwDeterminedLength = 0;
    }

    CurrentString = L"";

    // Render clause, candidate list
    int numCharIndex        = 0;
    int clauseLength        = 0;
    wchar_t* pStringCopy    = pString; // Remember starting address of pString since it will change in the loop below
    int start, end;
    start = 0; end = 0;

    for( DWORD i = 0; i < dwClauses; i++ )
    {
        if( i == dwFocus )
        {

            int UTF8Len = GUTF8Util::GetEncodeStringSize(pString, -1);
            char* utf8Str = (char*)GALLOC(UTF8Len+1, GFxStatIME_Mem);
            GUTF8Util::EncodeString(utf8Str, pString, -1);
            if(CurrentImeMode == MODE_CONVERT)
            {
                start   =  numCharIndex;
                end     =  numCharIndex + wcslen(pString);
            }
            ClausePosition = start;
            clauseLength = wcslen(pString);          
            numCharIndex += wcslen(pString);
            GFREE(utf8Str);
        }
        else
        {
            int UTF8Len = GUTF8Util::GetEncodeStringSize(pString, -1);
            char* utf8Str = (char*)GALLOC(UTF8Len+1, GFxStatIME_Mem);
            GUTF8Util::EncodeString(utf8Str, pString, -1);
            if(i != dwDeterminedClause)
                numCharIndex += wcslen(pString);
          
            GFREE(utf8Str);

        }
        
        INT iStrLen = wcslen( pString );
        compStringLength += iStrLen;
        pString += iStrLen + 1;

        // When we are in convert mode (candidate list is open) and user pushes a key,
        // the current composition must be finalized and the input key should become the first letter in the new 
        // composition string. In case of Chinese Traditional, XIME sends us both the finalized character and the new
        // character at once and dwClauses = 2. Therefore, to avoid duplication of text, we should only send the first
        // clause to FinalizeComposition and ignore the second clause. The second clause will be sent again later on
        // as regular composition text. 
        if(i == dwDeterminedClause)
            break;
    }
    

    wchar_t* wcsCompString  = (wchar_t*)GALLOC(2*(compStringLength+1), GFxStatIME_Mem);
    int counter             = 0;
    int skip                = 0;
    
    // Logic below reads the clause strings from pStringCopy and concatenates them 
    // into wcsCompString. The clause strings are separated by the null character
    // in buffer pointed to by pStringCopy and hence they need to be concatenated. 

    for (int i = 0; i < compStringLength + skip; i++)
    {
        if(pStringCopy[i] != 0)
            wcsCompString[counter++] = *(pStringCopy + i);
        else
            skip++;

    }
    int prevCharsInIME = CharsInIME;
    CharsInIME = compStringLength - dwDeterminedLength;
    if((CharsInIME > 0) && (prevCharsInIME == 0))
    {
        StartComposition();
    }
    wcsCompString[compStringLength] = 0; //Add terminating NULL
    
    if(dwDeterminedClause == 0)
        FinalizeComposition(wcsCompString, compStringLength);
    else
    {
        // if(compStringLength != 0) // Can't have this condition otherwise when length of comp string is 0 (due to
        // backspace action for example, the last character will not be erased)
            if(CurrentLanguage != MODE_KR)
            {
                SetWideCursor(false);
                SetCompositionText(wcsCompString, compStringLength);
                HighlightText(start, end-start, GFxIMEManager::THS_ClauseSegment, 0); 
                HighlightText(0, start, GFxIMEManager::THS_CompositionSegment, 0); 
                HighlightText(end, compStringLength - end, GFxIMEManager::THS_CompositionSegment, 0); 

                if((int)(dwCursor) >= 0) // When comp string is converted to clauses, dwCursor = 0xffffff. Hence this condition.
                    SetCursorInComposition(dwCursor);
                else
                    SetCursorInComposition(compStringLength);
            }
            
            if(CurrentLanguage == MODE_KR)
            {
                if(compStringLength != 0)
                    SetWideCursor(true); // Since we don't want wide cursor unless there are korean characters to type
                // more ever, there is no "finalize composition" for korean, so we don't really have a fixed place to
                // setWideCursor(false). 
                SetCompositionPosition();
                SetCompositionText(wcsCompString, compStringLength);
                SetCursorInComposition(0);
                
            }

    }
    
    if( bListAvailable && ShowCandidateList )
    {
        // Draw candidate list
        // ClausePosition must be wrt cursor position for the candidate list to
        // appear in the correct location.
        ClausePosition = ClausePosition - compStringLength; 
        DisplayCandidateList(dwNumCandidate, dwCandidateIndex, dwHighLightIndexInlist, dwNumberOfCandidateStringsInList );      
    }

    if(!bListAvailable || !ShowCandidateList){ 
        // Remove candidate list explicitly - since not showing it doesn't remove it from 
        // the display.
        if(CandListPath.GetLength() != 0)
        {
            pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
        }
    }
    ListBox->RemoveAllListItems();
    GFREE(wcsCompString);

    return S_OK;
}

HRESULT GFxIMEManagerXBox360::GetString( GFxString stringOut )
{
    if( !IsStringReady() ) 
        return ERROR_NOT_READY;
    stringOut = CompletedString;
    Completed = FALSE;
    CompletedString.Clear();

    return S_OK;
}

BOOL GFxIMEManagerXBox360::IsStringReady( void )
{
    if( !RequestList.IsNull(RequestList.GetFirst()) ) return FALSE;
    // if CompletedString is empty, return false, otherwise return true.
    if( CompletedString.IsEmpty() )
    {
        if( GotClause )
            Completed = FALSE;
        return FALSE;
    }
    return TRUE;
}

HRESULT GFxIMEManagerXBox360::GetLastKey( XINPUT_KEYSTROKE * pKey ) 
{
    XINPUT_KEYSTROKE srcKey;
    if( IsIMEOn() )
    {
        if( Completed )
        {
        //    memset( pKey, 0, sizeof(XINPUT_KEYSTROKE) );
        //    return S_OK;
        }
        memcpy(&srcKey, &LastKey, sizeof(XINPUT_KEYSTROKE));
        
        // Comment LastKeyUp: This is important because we should pass both KeyDown as well as Keyup to 
        // HandleEvent for keystrokes that are not handled by IME. Otherwise the internal keystroke state will
        // be inconsistent. For example in a chat application, if the user types a string and then pushes
        // enter. This will cause the composition to finalize. The user furthur pushes enter to cause the 
        // string to become part of the chat dialog. This second enter will be stored in LastKey, but since 
        // we only store the KeyDown events, we'll never pass the KeyUp event to HAndleEvent leading to 
        // inconsistent Enter state causing all kinds of subtle issues. To avoid this, we create a simulated
        // KeyUp event and put it on the queue.
        if(LastKey.Flags & XINPUT_KEYSTROKE_KEYDOWN)
        {
            XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
            RequestList.PushBack(request);
            XINPUT_KEYSTROKE tKey       = LastKey;
            tKey.Flags                  = tKey.Flags & ~XINPUT_KEYSTROKE_KEYDOWN;
            tKey.Flags                  = tKey.Flags | XINPUT_KEYSTROKE_KEYUP;
            RequestList.GetLast()->key  = tKey;
            RequestList.GetLast()->dwRequestId = HandleLastKeyUp;

            XIMEInsertCharacter( tKey.Unicode,
                XIME_KEY_UNICODE,
                InputMode,
                &RequestList.GetLast()->ov );
        }
       
        SetWideCursor(false);
    }
    else
    {
        memcpy(&srcKey, &Key, sizeof(XINPUT_KEYSTROKE));
    }
    memcpy( pKey, &srcKey, sizeof(XINPUT_KEYSTROKE) );
    memset( &LastKey, 0, sizeof(XINPUT_KEYSTROKE) );
    memset( &Key, 0, sizeof(XINPUT_KEYSTROKE) );
    return S_OK;
}

void GFxIMEManagerXBox360::DisplayCandidateList(int numCandidates, int dwCandidateIndex, int dwHighLightIndexInlist, int dwNumberOfCandidateStringsInList )
{
    // figure out if anything has changed in the list or not. If nothing has changed, don't re-render.

    // compute new checksum. 
    /*
    candListRowPtr = CandidateListBuffer;
    int checkSum = 0;
    for (int i = 0; i < dwNumberOfCandidateStringsInList; i++)
    {
        checkSum += *candListRowPtr;
        candListRowPtr = candListRowPtr + wcslen(candListRowPtr) + 1;
    }

    if(numCandidatesPrev != numCandidates){}
        
    else if(dwHighLightIndexInListPrev != dwHighLightIndexInlist){}
        
    else if(dwNumberOfCandidateStringsInListPrev != dwNumberOfCandidateStringsInList){}
        
    else if(checkSumPrev != checkSum){}
        
    else
    {
        return;
    }
    numCandidatesPrev = numCandidates; dwNumberOfCandidateStringsInListPrev = dwNumberOfCandidateStringsInList;
    dwHighLightIndexInListPrev = dwHighLightIndexInlist; checkSumPrev = checkSum;
    */
    ListBox->RemoveAllListItems();
    if(!IsTextFieldFocused()) return;
    OnOpenCandidateList();
    
    wchar_t* candListRowPtr = 0;
    candListRowPtr = CandidateListBuffer;
    
    for (UInt i = 0; i < GFxIMEManagerXBox360::dwNumberOfCandidateStringsInPage; i++)
    {
        wchar_t* rowData = candListRowPtr;
        int utf8Len = GUTF8Util::GetEncodeStringSize(rowData, -1);
        char* rowDataUTF8 = (char*)GALLOC(utf8Len + 1, GFxStatIME_Mem);
        GUTF8Util::EncodeString(rowDataUTF8, rowData, -1);
        rowDataUTF8[utf8Len] = 0;
        GPtr<CandidateListItem> pItem  = *new CandidateListItem(rowDataUTF8);
        ListBox->AddListItem(pItem, false); // Only copy as much data as necessary.
        GFREE(rowDataUTF8);    
        candListRowPtr = candListRowPtr + wcslen(candListRowPtr) + 1;
    }
    
    ListBox->SetSelectedItemIndex(dwCandidateIndex);
    GPtr<GFxIMEManager> pimeMgr = pMovie->GetIMEManager();
    GASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
    GRectF viewRect, caretRect;
    pimeMgr->GetMetrics(&viewRect, &caretRect, ClausePosition);
    if (CandListPath.GetLength() > 0)
    {
        pMovie->Invoke((CandListPath + ".RepositionCandWindow").ToCStr(), "%d %d %d", (int)caretRect.Left, (int)caretRect.Bottom, (int)caretRect.Height());
    }
    ListBox->UIRefreshView();

}


void GFxIMEManagerXBox360::CompleteAndFinalize()
{
   XIMEREQUEST* request = (XIMEREQUEST*)new(XIMEREQUEST);
    RequestList.PushBack(request);

    XIMEInsertCharacter( 0,
        XIME_KEY_COMPLETE,
        InputMode,
        &RequestList.GetLast()->ov );

    UInt dwClauses = 0; // Note can't be non zero since if IME is off, this will never be reset by Render.

    // This is important since we must finalize XIME before changing input focus. The Request queue should be empty
    // as well as there should be no characters in the composition string.
    do 
    {
        PreUpdate();
        Render(dwClauses);
    } while(dwClauses != 0 || !RequestList.IsNull(RequestList.GetFirst()) );

    // Remove any pop-ups that might exist
    if (CandListPath.GetLength() > 0)
    {
        pMovie->Invoke((CandListPath + ".RemoveList").ToCStr(), "");
    }
    ListBox->RemoveAllListItems();
    
    // Set IME mode to mode_input 
    SetCurrentMode( MODE_INPUT );
   
//    Completed = TRUE;
}



void    GFxIMEManagerXBox360::IMECommand(GFxMovieView* pmovie, const char* pcommand, const char* parg)
{
    GPtr<GFxLog> plog = pmovie->GetLog();
    if (plog)
    {
        plog->LogMessage("imecommand: '");
        plog->LogMessage(pcommand);
        plog->LogMessage("' '");
        plog->LogMessage(parg);
        plog->LogMessage("'\n");
    }

    //FxPlayerApp* pApp = (FxPlayerApp*)pmovie->GetUserData();
    //FxPlayerSettings& Settings = pApp->Settings;

    if(G_strcmp(pcommand, "CandidateList_OnSetItemDataMap") == 0)
    {

        FsCallBack(pmovie, pcommand, parg); //?
        return;
    }

    if(G_strcmp(pcommand, "CandidateList_OnItemSelect") == 0)
    {
        XIME_KEYMAPPING key;
        key.wVK = *parg - 0x30 + VK_NUMPAD0 + 1;
        this->HandleNumpadConversion( this, &key );
        return;
    }
   
}

void    GFxIMEManagerXBox360::SetActiveMovie(GFxMovieView* pmovie)
{
    GFxIMEManager::SetActiveMovie(pmovie);
    // Acquire Candidate list. Only loads candidate list movie once- subsequent invocations 
    // don't cause any extra unloading-loading to take place.
    GFxIMEManager::AcquireCandidateList();
    SetMovie(pmovie);
}

// Redefinition of function derived from GFxIMEManager
// detects if the string "candidateList_mc" is in the path string.
bool IsCandidateList(const char* path)
{
    if (path == 0) return FALSE;
    int i = 0;
    int len = (int)G_strlen(path);
    while(path[i] != 0)
    {
        int j = i;
        while((path[i] != '.') && (path[i] != 0))
        {
            i++;
        }
        if(!_strnicmp(path+j, "_level9999", i-j))
            return TRUE;
        else
        {
            if( i == len)
                return FALSE;
            else
                i++; // skip the '.'
        }
    }
    return FALSE;
}

// Redefinition of function derived from GFxIMEManager
// Records path of the candidateList movie for future invokes.
void GFxIMEManagerXBox360::OnCandidateListLoaded(const char* path)
{
    RecordCandidateListPath(path);
}

void GFxIMEManagerXBox360::RecordCandidateListPath(const char* parg)
{
    CandListPath = parg;
    ListBox->SetASWidgetPath(parg);
}

void GFxIMEManagerXBox360::OnFinalize()
{
    // finalize 
    CompleteAndFinalize();
}

// Override from GFxIMEManaager. Invoked when candidate list loading is completed.
void GFxIMEManagerXBox360::OnCandidateListStyleChanged(const GFxIMECandidateListStyle& style)
{
    if (!pMovie)
        return;
    GPtr<GFxIMEManager> pimeMgr = pMovie->GetIMEManager();
    if(!pimeMgr) return;
    // ((1 << (8*size-1)))|((1 << (8*size-1))-1); // -1 in 2's compliment form
    UInt32 textColor                = (UInt32)(-1);
    UInt32 backgroundColor          = (UInt32)(-1);
    UInt32 selectedTextColor        = (UInt32)(-1);
    UInt32 selectedBackgroundColor  = (UInt32)(-1);
    UInt32 readingWindowTextColor   = (UInt32)(-1);
    UInt32 readingWindowBckgndColor = (UInt32)(-1);
    UInt32 selectedIndexBckgndColor = (UInt32)(-1);
    UInt32 indexBckgndColor         = (UInt32)(-1);
    UInt   fontSize                 = (UInt)(-1);
    UInt   readingWindowFontSize    =  (UInt)(-1);

    if(style.HasTextColor())
        textColor = style.GetTextColor();
    if(style.HasBackgroundColor())
        backgroundColor = style.GetBackgroundColor();
    if(style.HasSelectedTextColor())
        selectedTextColor = style.GetSelectedTextColor();
    if(style.HasSelectedBackgroundColor())
        selectedBackgroundColor = style.GetSelectedBackgroundColor();
    if(style.HasSelectedIndexBackgroundColor())
        selectedIndexBckgndColor = style.GetSelectedIndexBackgroundColor();
    if(style.HasIndexBackgroundColor())
        indexBckgndColor = style.GetIndexBackgroundColor();
    if(style.HasSelectedBackgroundColor())
        selectedBackgroundColor = style.GetSelectedBackgroundColor();
    if(style.HasFontSize())
        fontSize = style.GetFontSize();
    if(style.HasReadingWindowTextColor())
        readingWindowTextColor = style.GetReadingWindowTextColor();
    if(style.HasReadingWindowBackgroundColor())
        readingWindowBckgndColor = style.GetReadingWindowBackgroundColor();
    if(style.HasReadingWindowFontSize())
        readingWindowFontSize = style.GetReadingWindowFontSize();


    if(CandListPath.GetLength() != 0)
    {
        pMovie->Invoke((CandListPath + ".SetCandidateListProps").ToCStr(), "%d %d %d %d %d %d %d",
            textColor, selectedTextColor, backgroundColor,  selectedBackgroundColor, fontSize, indexBckgndColor,
            selectedIndexBckgndColor);
        pMovie->Invoke((CandListPath + ".SetReadingWindowProps").ToCStr(), "%d %d %d",
            readingWindowTextColor, readingWindowBckgndColor, readingWindowFontSize);
    }

}

HRESULT GFxIMEManagerXBox360::HandleGamepadIME(XINPUT_KEYSTROKE& Key)
{

    DWORD   dwFlags = CurrentImeMode | CurrentLanguage;
    XIMEKEY eControlKey = XIME_KEY_UNICODE;
    IMECALLBACK  CallBack = NULL;
    int i = 0;
    // Look up a table...
    for( i = 0; i < NumInputKeymap; i++ )
    {
        if( (dwFlags & InputKeymap[ i ].dwFlags) == dwFlags
            && Key.VirtualKey == InputKeymap[ i ].wVK)
        {
            eControlKey = InputKeymap[ i ].eControlKey;
            CallBack = InputKeymap[ i ].function;
            break;
        }
    }
    
    HRESULT hr = S_FALSE;;
    if( CallBack )
    {
        // Then, invoke callback
        hr = CallBack( this, &InputKeymap[ i ] );
    }
    return hr;
}

// Redefinition of function derived from GFxIMEManager
// detects if the string "candidateList_mc" is in the path string.
bool GFxIMEManagerXBox360::IsCandidateList(const char* path)
{
    if (path == 0) return FALSE;
    int i = 0;
    int len = (int)G_strlen(path);
    while(path[i] != 0)
    {
        int j = i;
        while((path[i] != '.') && (path[i] != 0))
        {
            i++;
        }
        if(!_strnicmp(path+j, "_level9999", i-j))
            return TRUE;
        else
        {
            if( i == len)
                return FALSE;
            else
                i++; // skip the '.'
        }
    }
    return FALSE;
}
#endif