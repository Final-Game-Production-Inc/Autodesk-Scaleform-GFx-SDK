/**************************************************************************

Filename    :   GFx_IMEManagerWin32.cpp
Content     :   Derived from GFxIMEManager- implements Win32 related IMEManager functionality 
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
#define XML_STATIC
#include <expat.h>
#include "GFx_IMEManagerWin32.h"
#include "String.h"
#include "GFx_IMEWin32Impl.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "GFx_IMENamesManagerVista.h"
#include "GFx_IMENamesManagerXP.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{
// Internal GFC Evaluation License Reader
#ifdef GFC_BUILD_EVAL
#define GFC_EVAL(x) GFxIME_##x
#define GFC_LIB_NAME_S "GFxIME"
#include "GFCValidateEval.cpp"
#else
void    GFxIME_ValidateEvaluation() { }
#endif

extern StringToTagDef StringToTag[];
extern int StringToTagSize;

GFxIMEManagerWin32::GFxIMEManagerWin32(HWND hwnd)
{
    // Validate license
    GFxIME_ValidateEvaluation();
    pLog            = 0;
    pIMENamesMgr    = 0;
    hWnd            = hwnd;
    pLog            = 0;
    pIME            = NULL;
}

#ifdef GFX_ENABLE_XML
class ExpatCallbackHandler2
{
public:
	//
	// Expat callback handlers
	//

	static void     StartElementExpatCallback(void *userData, const XML_Char *name, const XML_Char **atts)
	{
		SF_UNUSED(atts);
		if (userData)
		{
			if (!SFstrcmp(name, "language"))
			{
				((GImeNamesManager*)userData)->mark = true;
			}
			((GImeNamesManager*)userData)->label = GImeNamesManager::NONE;
			if (!SFstrcmp(name, "imeName"))
			{
				((GImeNamesManager*)userData)->label = GImeNamesManager::IMENAME;
				((GImeNamesManager*)userData)->mark_ime = true;
				InputLangProps2 inputLanguage;
				((GImeNamesManager*)(userData))->SupportedIMEs.PushBack(inputLanguage);
				((GImeNamesManager*)(userData))->NumIme++;

			}

			if (!SFstrcmp(name, "displayName"))
			{
				((GImeNamesManager*)userData)->label = GImeNamesManager::DISPLAYNAME;
				// mark boolean is used to make sure we are between opening and closing tags
				((GImeNamesManager*)userData)->mark_displayname = true;
			}

			if (!SFstrcmp(name, "Tag"))
			{
				((GImeNamesManager*)userData)->label = GImeNamesManager::IMETAG;
				((GImeNamesManager*)userData)->mark_tag = true;
			}
		}
	}

	static void     EndElementExpatCallback(void *userData, const XML_Char *name)
	{
		if (!SFstrcmp(name, "Language"))
		{
			((GImeNamesManager*)userData)->mark = false;
		}
		if (!SFstrcmp(name, "imeName"))
		{
			((GImeNamesManager*)userData)->mark_ime = false;
			((GImeNamesManager*)userData)->label	= GImeNamesManager::NONE;
		}
		if (!SFstrcmp(name, "displayName"))
		{
			((GImeNamesManager*)userData)->mark_displayname = false;
			((GImeNamesManager*)userData)->label	= GImeNamesManager::NONE;
		}
		if (!SFstrcmp(name, "Tag"))
		{
			((GImeNamesManager*)userData)->mark_tag = false;
			((GImeNamesManager*)userData)->label	= GImeNamesManager::NONE;
		}
	}
	static void     CharacterDataExpatCallback(void *userData, const XML_Char *s, int len)
	{
		if (((GImeNamesManager*)userData)->mark)
		{
			InputLangProps2* prop = 0;
			if (userData)
			{
				int sz = static_cast<int>(((GImeNamesManager*)(userData))->SupportedIMEs.GetSize());
				if (sz > 0)
					prop = &((GImeNamesManager*)(userData))->SupportedIMEs[sz-1];
			}
			else
			{
				return;
			}
			switch (((GImeNamesManager*)userData)->label)
			{
			case GImeNamesManager::IMENAME:
				prop->ItemNameOnSystem = String(s, len);
				break;
			case GImeNamesManager::DISPLAYNAME:
				prop->ItemNameCommon = String(s, len);
				break;
			case GImeNamesManager::IMETAG:
				{
					int len2 = StringToTagSize;
					String str = String(s, len);
					for (int i = 0; i < len2; i++)
					{
						if (str == StringToTag[i].String)
						{
							prop->ItemTag = StringToTag[i].Tag;
							break;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	//
	// Helper function to fill the locator object
	//
	//static void     FillLocator(ExpatHandlerArg* pha);

	String currentLanguage;
};

#endif //GFC_NO_XML_SUPPORT


UInt32 GFxIMEManagerWin32::Init(Log* plog, FileOpenerBase* pfileOpener, const char* xmlFileName, bool bcheckIMEExists)
{
    pLog = plog;
    // Detect windows version
    
    GetWindowsVersion(Version); 
    pIMENamesMgr    = 0;
    pIME            = 0;
	bCheckIMEExists = bcheckIMEExists;
    // Initialize COM
    /*

    Important Note: CoInitialize must be called without COINIT_MULTITHREADED for OnActivated to be called everytime an IME change    is made (there could be other side effects of COINIT_MULTITHREADED as well that I don't know of). According to the               documentation, Typically, the COM library is initialized on a thread only once. Subsequent calls to CoInitialize or              CoInitializeEx on the same thread will succeed, as long as they do not attempt to change the concurrency model, 
    but will return S_FALSE.Now it's possible that some other module in the application calls CoInitialize in the multithreaded      mode, which will cause this CoInitialize
    invocation to fail and Vista IME to not work. We can't fully remedy this problem, but we can at least check for the error        value retured by CoInitialize and log an error. 

    Also: 

    In addition, calls made to objects initialized using the COINIT_APARTMENTTHREADED setting arrive only at message-queue           boundaries. Because of this serialization, it is not typically necessary to write concurrency control into the 
    code for the object, other than to avoid calls to PeekMessage and SendMessage during processing that must not be interrupted     by other method invocations or calls to other objects in the same apartment/thread.

    An important implication of this is that we must make sure that PeekMessage is called often enough, otherwise IME processing     will be very slow if a long time is spent in Advance/Display.

    */

    HRESULT hr = CoInitialize(0); // CoInitializeEx(NULL, COINIT_MULTITHREADED);

//	HRESULT hr =  CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == S_FALSE)
        SF_DEBUG_WARNING(1, "GFxIME Error: The COM library is already initialized on this thread");

    if (hr == RPC_E_CHANGED_MODE)
        SF_DEBUG_WARNING(1, "GFxIME Error: A previous call to CoInitializeEx specified the concurrency model for this thread as     multithread apartment (MTA)"); 


    if(Version == WinVista)
    {
        pIME            = new GFxIMEVista(this, hWnd);
        pIMENamesMgr    = new GImeNamesManagerVista(this);
    }
    else if(Version == WinXP)
    {
        pIME            = new GFxIMEXP(this, hWnd);
        pIMENamesMgr    = new GImeNamesManagerXP(this);
    }

#ifndef GFX_ENABLE_XML
	xmlFileName = NULL;
#endif

	if (pfileOpener != NULL && xmlFileName != NULL)
	{
#ifdef GFX_ENABLE_XML

		XML_Parser parser = XML_ParserCreate(NULL);
		Ptr<File> pfile = 0;
		if (URLBuilder::IsPathAbsolute(xmlFileName))
		{
			pfile = *pfileOpener->OpenFile(xmlFileName);
		}
		else
		{
#ifdef SF_OS_WIN32
			// look in $GFXSDK\bin\ime
			char gfxsdk[_MAX_PATH];
            size_t size;
            getenv_s(&size, gfxsdk, _MAX_PATH, "GFXSDK");
			// Append "Bin\IME"
			String fullpath = String(gfxsdk) + "\\Bin\\Data\\" + xmlFileName;
			pfile = *pfileOpener->OpenFile(fullpath.ToCStr());

#endif
		}

		if (pfile && pfile->IsValid())
		{
			SInt32 flen = 0;
			if ((flen = pfile->GetLength()) != 0)
			{
				// Load the file into memory
				UByte* td = (UByte*) SF_ALLOC(flen, StatIME_Mem);
				pfile->Read(td, flen);
				XML_SetStartElementHandler(parser, ExpatCallbackHandler2::StartElementExpatCallback);
				XML_SetEndElementHandler(parser, ExpatCallbackHandler2::EndElementExpatCallback);
				XML_SetCharacterDataHandler(parser, ExpatCallbackHandler2::CharacterDataExpatCallback);
				XML_SetUserData(parser, pIMENamesMgr);
                if (pIMENamesMgr)
				    pIMENamesMgr->mark = false;
				if (XML_Parse(parser, (const char*)td, flen, true) != XML_STATUS_OK)
				{
					XML_Error ecode = XML_GetErrorCode(parser);
					unsigned long long column = (unsigned long long)XML_GetCurrentColumnNumber64(parser) + 1;
					unsigned long long line = (unsigned long long)XML_GetCurrentLineNumber64(parser);
					if (pLog)
						pLog->LogWarning("IME Error: Error reading xml file: %s \
										 @ line:%llu col:%llu - %.64s\n",
										 xmlFileName, (unsigned long long)line, (unsigned long long)column, XML_ErrorString(ecode));

				}	
				// Cleanup
				SF_FREE(td);
			}
            pfile->Close();
		}
		else
		{
			if (pLog)
				pLog->LogWarning("IME Warning:error in opening xml file, check path. Defaulting to internal list\n");
            if (pIMENamesMgr)
			    pIMENamesMgr->PopulateSupportedIMEs();
		}
        XML_ParserFree(parser);
#endif
	}
	else
	{
		if (pLog)
			pLog->LogWarning("IME Warning: No fileopener and/or xml file specified. Defaulting to internal list\n");
        if (pIMENamesMgr)
		    pIMENamesMgr->PopulateSupportedIMEs();
	}

    if (pIMENamesMgr)
    	pIMENamesMgr->AllocateImeNameOrLayoutTextArray();

    // HashIMENames returns 0 for success and 1 for failure.
    NamesManagerInitStatus = false;
    if (pIMENamesMgr)
    {
        NamesManagerInitStatus = pIMENamesMgr->HashIMENames() == 0;
    }
    
    return NamesManagerInitStatus;
}

GFxIMEManagerWin32::~GFxIMEManagerWin32()
{
    if(pIME)
        delete pIME;

    if(pIMENamesMgr)
        delete pIMENamesMgr;
}

String GFxIMEManagerWin32::GetWindowsVersion()
{
	switch (Version)
	{
	case WinVista:
		return "Vista";
		break;
	case WinXP:
		return "XP";
		break;
	default:
		return "Unknown";

	}
}

void GFxIMEManagerWin32::GetWindowsVersion(OSVersion& version)
{
    // RtlGetVersion reports the real OS version regardless of the
    // application manifest; GetVersionEx is deprecated and lies on Win8.1+.
    typedef LONG (WINAPI *RtlGetVersionFn)(PRTL_OSVERSIONINFOW);
    RTL_OSVERSIONINFOW osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    RtlGetVersionFn pRtlGetVersion = hNtdll ?
        (RtlGetVersionFn)GetProcAddress(hNtdll, "RtlGetVersion") : NULL;

    if (pRtlGetVersion && pRtlGetVersion(&osvi) == 0)
    {
        if (osvi.dwMajorVersion >= 6)
            version = WinVista;
        else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)
            version = WinXP;
        else
            version = Win2K;
    }
    else
        version = Unknown;
}

String GFxIMEManagerWin32::GetSystemLanguageInfo()
{
	char buf[4];

    if (GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, LOCALE_SABBREVLANGNAME, buf, 4) != 0)
	{
		return String(buf);
	}
	else
	{
		DWORD err = GetLastError();
		// Do error processing
		if (err == ERROR_INSUFFICIENT_BUFFER)
		{
			
		}
		if (err == ERROR_INVALID_FLAGS)
		{
			
		}
		if (err == ERROR_INVALID_PARAMETER)
		{
			
		}
	}
	return 0;
}

#ifdef GFX_TEST_IME

bool GFxIMEManagerWin32::SwitchIME(String imeName)
{
    return pIMENamesMgr->SwitchIME(imeName);
}

void GFxIMEManagerWin32::ResetText()
{
    pMovie->SetVariable("_root.Edit_MLine1.textBox.text", "Hello World");
    pMovie->Invoke("Selection.setSelection", "%d %d", 5,5);
}
#endif

// Delegating function calls

unsigned GFxIMEManagerWin32::HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent)
{
    // If some error in initialization of the names manager, abort. Should also log error
    if (!NamesManagerInitStatus)
    {
        // Print error message to the log.
        if (pmovie)
        {
            MovieImpl* pmovieroot = static_cast<MovieImpl*>(pmovie);
            Log* plog = pmovieroot->GetCachedLog();
            plog->LogWarning("Failure in obtaining list of installed IMEs! IME is disabled");
        }   
        return Movie::HE_NotHandled; 
    }

    if (pIME != NULL)
    {
        const IMEWin32Event& winEvt = static_cast<const IMEWin32Event&>(imeEvent);
        if (!IsMovieActive(pmovie))
            return Movie::HE_NotHandled;

        // First check if this IME is supported
        if (pIMENamesMgr)
            SetIMETag(pIMENamesMgr->GetImeTag()); // Also sets the IMETag member of GFxIMEWin32Impl.

//      HWND hwndIME        = ImmGetDefaultIMEWnd(hWnd);
/*
        if (IMETag == GFxIME_NotSupported)
        {
            // Decide whether or not to hide IME windows
            if (GetUnsupportedIMEWindowsFlag() == GFxIMEManager::GFxIME_AlwaysHideUnsupportedIMEWindows)
            {
                //   ShowOwnedPopups(hwndIME, false); // In order to hide IME Popups
            }
        }
        else
        {
            // For supported IME's, always hide default IME windows
            ShowOwnedPopups(hwndIME, false); // In order to hide IME Popups
        }
*/  
        
        // Reposition the default IME windows appropriately. See comment in OnIMEStart.
        if (winEvt.Message == WM_IME_STARTCOMPOSITION)
        {
            return pIME->OnIMEStart(winEvt.WParam, winEvt.LParam, winEvt.Options != 0);
        }

        if (winEvt.Message == WM_IME_SETCONTEXT)
            return Movie::HE_NoDefaultAction;

        // Language bar and Status window notifications should be handled even if we are not in a textfield
        // to keep the state of the language bar consistent with that of the system.
        if (winEvt.Message == WM_INPUTLANGCHANGE)
        {
            // Note that we can't use GetKeyboardLayout to get the name of the input locale
            // since it returns the hex value of the locale as a string. For example: "0x0000409"
            // GetKeyboardLayoutNameA(langName);
            pIMENamesMgr->OnInputLangChange((UInt32)(winEvt.LParam));

            // This is important because for JJ Pinyin IME, when the input language changes,
            // finalize messages are not sent and therefore the previous ime UI elements can
            // still be present when a new input language is activated. 
            if (pIME)
                pIME->Finalize();
        }

        if(winEvt.Message == WM_IME_NOTIFY && winEvt.WParam == IMN_SETCONVERSIONMODE)
        {
#ifndef GFC_NO_LANGBAR_SUPPORT
            pIMENamesMgr->SetConversionMode();
#endif
        }
		
		bool hRes = pIME->IsTextFieldFocussed(pmovie);
		if (!hRes)
		{
			return Movie::HE_NotHandled;
		}

        if (winEvt.IMEEvtType == IMEEvent::IME_PreProcessKeyboard)
        {
            if (IsTextFieldFocused()) 
            {
                
                if(IMETag != GFxIME_NotSupported)
                {
                    UInt32    msg    = winEvt.Message;
                    UPInt   wParam = winEvt.WParam;
                    UPInt   lParam = winEvt.LParam;

              //      pIME->SetIMETag(IMETag);
                    bool imeUIMsg = (ImmIsUIMessage(NULL, msg, wParam, lParam) == 0)? FALSE: TRUE;

                    if((msg == WM_KEYDOWN) || (msg == WM_KEYUP) || imeUIMsg || msg == WM_LBUTTONUP) 
                        PreProcessWM_KEYDOWN(winEvt);

                    return Movie::HE_NoDefaultAction;
                }
            }
            return Movie::HE_NotHandled;
        }
        else
        {
            if(IMETag == GFxIME_NotSupported)
                return Movie::HE_NotHandled;

            //     pIME->SetIMETag(IMETag);
            //     SetCurrentInputLanguage(IMETag);

            switch(winEvt.Message)
            {
        //  case WM_IME_SETCONTEXT:
        //      return Movie::HE_NoDefaultAction;

            case WM_IME_ENDCOMPOSITION:
                pIME->OnIMEStop(winEvt.WParam, winEvt.LParam, winEvt.Options != 0);
                break;

            case WM_IME_COMPOSITION:
                pIME->OnIME(winEvt.WParam, winEvt.LParam, winEvt.Options);
                return Movie::HE_NoDefaultAction; 

            case WM_IME_NOTIFY:

                if (winEvt.WParam == IMN_CHANGECANDIDATE || winEvt.WParam == IMN_OPENCANDIDATE
                    || winEvt.WParam == IMN_CLOSECANDIDATE || winEvt.WParam == IMN_PRIVATE)
                {
                    if(CandListLoaded == false)
                    {
                        // Check to see if the candidate list was successfully loaded or not. Only 
                        // do this the first time the list is used. If the list is not loaded, print
                        // a message on the log.
                        String dest;
                        if (GetCandidateListErrorMsg(&dest))
                        {
                            // Print error message to the log.
                            MovieImpl* pmovieroot = static_cast<MovieImpl*>(pmovie);
                            Log* plog = pmovieroot->GetCachedLog();
                            plog->LogWarning(dest.ToCStr());
                        }
                        CandListLoaded = true;

                    }
                    if (pIME->OnIMECandidateProcessing(WM_IME_NOTIFY, winEvt.WParam, winEvt.LParam, 1))
                        return Movie::HE_NoDefaultAction;
                    else
                        return Movie::HE_NotHandled;
                }

                else if(winEvt.WParam == IMN_SETCONVERSIONMODE)
                {
#ifndef GFC_NO_LANGBAR_SUPPORT
                    pIMENamesMgr->SetConversionMode();
#endif
                }

                else if(winEvt.WParam == IMN_SETOPENSTATUS)
                {

                    HIMC hIMC = ImmGetContext(HWND(winEvt.hWND));
                    if (ImmGetOpenStatus(hIMC) == 0)
                    {
                        ImeOpenStatus = false;
                        pIME->SetOpenStatus(false);
                    }
                    else
                    {
                        ImeOpenStatus = true;
                        pIME->SetOpenStatus(true);
                    }
                    
                    // If Japanese IME is being used and the user switches back and forth from direct input to 
                        // hiragana, katakana etc, SetConversionMode message is not sent, since switching to Direct 
                        // Input is considered turning the IME on/off. Handle that case here.
#ifndef GFC_NO_LANGBAR_SUPPORT                  
                    if (CurrentLanguage == "Japanese")
                        pIMENamesMgr->SetConversionMode();
#endif
                    ImmReleaseContext(HWND(winEvt.hWND), hIMC);
                }

                else if(winEvt.WParam == IMN_OPENSTATUSWINDOW)
                {
    
                }

                else if(winEvt.WParam == IMN_CLOSESTATUSWINDOW)
                {
        
                }

                else if(winEvt.WParam == IMN_SETSTATUSWINDOWPOS)
                {
    
                }

                pIME->CustomProcessing(WM_IME_NOTIFY, winEvt.WParam, winEvt.LParam);
                break;

            case WM_IME_CHAR:
                return Movie::HE_NoDefaultAction;

            default:
                return Movie::HE_NotHandled;
            }
        }
    }
    return Movie::HE_Handled;
}

void GFxIMEManagerWin32::SetCurrentInputLanguage(GFxIMETag inputLangTag)
{
    // IMETags are used as an intermediary between the input language strings obtained from 
    // the system and generic strings used in this class. For example, Chinese (Traditional) 
    // is called Chinese (Traditional) on XP and Chinese (Taiwan) on Vista. The GFxIMENamesManagerVista
    // and GFxIMENamesManagerXP classes handle the system specific details and produce the same IMETag 
    // (GFxIME_Ch_Trad in this case). 
    // This function just maps the IMETags to generic language ename strings that are system independent.

    CurrentLanguage = "Unknown";

    if (inputLangTag & (UInt32)GFxIME_En_Flag)
    { 
        CurrentLanguage = "English";
    } 

    if (inputLangTag & (UInt32)GFxIME_Jp_Flag)
    { 
        CurrentLanguage = "Japanese";
    }  

    if (inputLangTag & (UInt32)GFxIME_Kr_Flag)
    { 
        CurrentLanguage = "Korean";
    } 

    // Currently third party ime's are in the same category as simplified ime's
    if (inputLangTag & (UInt32)GFxIME_Ch_Simp_Flag || inputLangTag & (UInt32)GFxIME_Ch_ThirdParty_Flag )
    { 
        CurrentLanguage = "Chinese (Simplified)";
    } 

    // Currently third party ime's are in the same category as simplified ime's
    if (inputLangTag & (UInt32)GFxIME_Ch_Trad_Flag)
    { 
        CurrentLanguage = "Chinese (Traditional)";
    } 

    BroadcastSwitchLanguage(CurrentLanguage.ToCStr());
}

void GFxIMEManagerWin32::SetIMETag(GFxIMETag imeTag) {IMETag = imeTag; pIME->SetIMETag(IMETag);}

void GFxIMEManagerWin32::SetImeVersionId(int versionId) 
{
    ImeVersionId = versionId; 
    pIME->SetImeVersionId(ImeVersionId);
}

const String GFxIMEManagerWin32::GetInputLanguage()
{
    if(pIME != NULL)
        return(pIME->GetInputLanguage());

    return String("UNKNOWN");
}

// Redefinition of function derived from GFxIMEManager
// detects if the string "candidateList_mc" is in the path string.
bool GFxIMEManagerWin32::IsCandidateList(const char* path)
{
    return strstr(path, "_level9999") != 0;   
    /*   if (path == 0) return FALSE;
    int i = 0;
    int len = (int)SFstrlen(path);
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
    */
}

// Redefinition of function derived from GFxIMEManager
// detects if the string "statuswindow" is in the path string.
bool GFxIMEManagerWin32::IsStatusWindow(const char* path)
{
    return strstr(path, "_level9997") != 0;
}

bool GFxIMEManagerWin32::IsLangBar(const char* path)
{
    return strstr(path, "_level9998") != 0;
}


// Redefinition of function derived from GFxIMEManager
// Records path of the candidateList movie for future invokes.
void GFxIMEManagerWin32::OnCandidateListLoaded(const char* path)
{
    GetASIMEManager()->OnOpenCandidateList();
    RecordCandidateListPath(path);
}

LRESULT  GFxIMEManagerWin32::OnIMECandidateProcessing(UInt32 message, UInt32 key, UInt32 info, bool downFlag)
{
    LRESULT res = FALSE;
    if(pIME != NULL)
        res = pIME->OnIMECandidateProcessing(message, key, info, downFlag);
    return res;
}

void    GFxIMEManagerWin32::CustomProcessing(UInt32 message, UInt32 key, UInt32 info)
{
    pIME->CustomProcessing(message, key, info);
}

void    GFxIMEManagerWin32::OnFinalize()
{
    if(pIME != NULL)
        pIME->Finalize();
}

void    GFxIMEManagerWin32::OnShutdown()
{
    if(pIME != NULL)
        pIME->Shutdown();
}

void    GFxIMEManagerWin32::SelectAndClose(int index)
{
    if(pIME != NULL)
        pIME->SelectAndClose(index);
}

void    GFxIMEManagerWin32::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{
    if(pIME != NULL)
        pIME->PreProcessWM_KEYDOWN(winEvt);
}

void    GFxIMEManagerWin32::DisplayReadingWindow(wchar_t* pReadingStr)
{
    if(pIME != NULL)
        pIME->DisplayReadingWindow(pReadingStr);
}

void   GFxIMEManagerWin32::DisplayCompositionString(UInt32& numCharDisplay)
{
    if(pIME != NULL)
        pIME->DisplayCompositionString(numCharDisplay);
}

void  GFxIMEManagerWin32::RecordCandidateListPath(const char* parg)
{
    if(pIME != NULL)
        pIME->RecordCandidateListPath(parg);
}

void GFxIMEManagerWin32::ClearCandidateListPath() 
{
    if(pIME != NULL)
    	pIME->ClearCandidateListPath();
}

void  GFxIMEManagerWin32::RecordStatusWindowPath(const char* parg)
{
    if(pIME != NULL)
        pIME->RecordStatusWindowPath(parg);
}

void  GFxIMEManagerWin32::RecordLanguageBarPath(const char* parg)
{
    if(pIME != NULL)
        pIME->RecordLanguageBarPath(parg);
}

void  GFxIMEManagerWin32::SetCandidateListProperties(INT size, INT fontColor1, INT fontColor2)
{
    if(pIME != NULL)
        pIME->SetCandidateListProperties(size, fontColor1, fontColor2);
}

bool GFxIMEManagerWin32::SetConversionMode(const UInt32 convMode)
{
    if(pIME != NULL)
        return(pIME->SetConversionMode(convMode));

    return false;
}

const char* GFxIMEManagerWin32::GetConversionMode()
{
    if(pIME != NULL)
        return pIME->GetConversionMode();

    return "UNKNOWN";
}

bool GFxIMEManagerWin32::SetEnabled(bool enable)
{
    if(pIME != NULL)
        return pIME->SetEnabled(enable);

    return false;
}

// Retrieves IME state. 
bool GFxIMEManagerWin32::GetEnabled() 
{ 
    if(pIME != NULL)
        return pIME->GetEnabled();

    return false;
} 

bool GFxIMEManagerWin32::SetCompositionString(const char* pCompString)
{
    if(pIME != NULL)
        return pIME->SetCompositionString(pCompString);

    return false;
}
/*void  GFxIMEManagerWin32::SetMovie(Ptr<Movie> pMovie)
{
if(pIME != NULL)
pIME->SetMovie(pMovie);
}*/
void    GFxIMEManagerWin32::SetActiveMovie(Movie* pmovie)
{
    IMEManagerBase::SetActiveMovie(pmovie);
    // Acquire Candidate list. Only loads candidate list movie once- subsequent invocations 
    // don't cause any extra unloading-loading to take place.
    GetASIMEManager()->AcquireCandidateList();

    CandListLoaded = false;

    if(pIME != NULL)
    {
        pIME->SetMovie(pmovie);
    }

    pMovie = pmovie;

    // set the variable that allows actionscript to figure out if the os is XP or Vista
    Value v;
    if (Version == WinVista)
        v.SetString("Vista");
    else if (Version == WinXP)
        v.SetString("XP");
    else
        v.SetString("Undetermined");
    if (pmovie && (pmovie->GetAVMVersion() == 1))
        pMovie->SetVariable("_global.gfx_ime_os_version", v);
}

void  GFxIMEManagerWin32::SetWndHandle(HWND hwnd)
{
    if(pIME != NULL)
        pIME->SetWndHandle(hwnd);

    hWnd = hwnd; 
}

bool  GFxIMEManagerWin32::CheckIfInTextField()
{
    if(pIME != NULL)
        return pIME->CheckIfInTextField();
    return true;
}

void GFxIMEManagerWin32::OnCandidateListStyleChanged(const IMECandidateListStyle& style) 
{ 
    if(pIME != NULL)
        return pIME->OnCandidateListStyleChanged(style);
}

void GFxIMEManagerWin32::OnEnableIME(bool enable)
{
    if(pIME != NULL /*&& pIMENamesMgr->JapaneseIMEState*/)
    {
        pIME->OnEnableIME(enable);
    }
    // Since ChangeConversion message is not sent in this case.
#ifndef GFC_NO_LANGBAR_SUPPORT
    if (CurrentLanguage == "Japanese")

        pIMENamesMgr->SetConversionMode();
#endif
}
}}}

#endif  // SF_NO_IME_SUPPORT
