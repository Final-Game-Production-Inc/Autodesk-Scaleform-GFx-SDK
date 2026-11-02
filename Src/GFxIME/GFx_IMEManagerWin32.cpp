/**************************************************************************

Filename    :   GFx_IMEManagerWin32.cpp
Content     :   Implementation of the Win32 IME manager facade, the
                XP / Vista Win32 IME impl classes and the XML parser used
                to load the supported IME list.
Created     :   Oct 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <stdint.h>

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Allocator.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_File.h"

#include <windows.h>
#include <imm.h>

#include "GFx/IME/GFx_IMEManager.h"
#include "GFx_IMEManagerWin32.h"
#include "GFx_IMEWin32Impl.h"
#include "GFx_IMEIdMap.h"
#include "GFx_IMENamesManagerXP.h"
#include "GFx_IMENamesManagerVista.h"
#include "GFx_CandidateListBox.h"
#include "GFx_Command.h"
#include "GFx_IMETSF.h"
#include "GFx_IMEUnicodeMap.h"
#include "expat.h"

// g_StringToTagTable is defined in GFx_IMEIdMap.cpp

namespace Scaleform {
namespace GFx {
namespace IME {

extern StringToTagDef g_StringToTagTable[];

static size_t IMEwcslen(const wchar_t* pstr)
{
    size_t len = 0;
    if (pstr)
        while (pstr[len] != 0)
            len++;
    return len;
}

// Maps an IMM32 attribute character (ATTR_INPUT, ATTR_TARGET_CONVERTED, ...)
// to the TextHighlightStyle used by IMEManagerBase::HighlightText.
static IMEManagerBase::TextHighlightStyle GFxIMEAttrToStyle(char attr)
{
    switch (attr)
    {
    case ATTR_TARGET_CONVERTED:     return IMEManagerBase::THS_ClauseSegment;
    case ATTR_CONVERTED:            return IMEManagerBase::THS_ConvertedSegment;
    case ATTR_INPUT_ERROR:          return IMEManagerBase::THS_LowConfSegment;
    case ATTR_TARGET_NOTCONVERTED:  return IMEManagerBase::THS_ClauseSegment;
    default:                        return IMEManagerBase::THS_CompositionSegment;
    }
}

// Copies a wide candidate string into a UTF-8 char buffer. The buffer is a
// caller-provided stack array because CandidateListItem copies the string.
static char* IMEWideToUtf8(const wchar_t* pstr, char* pbuf, int bufSize)
{
    if (!pstr)
        return NULL;
    int nLen = WideCharToMultiByte(CP_UTF8, 0, pstr, -1, pbuf, bufSize, NULL, NULL);
    if (nLen <= 0)
    {
        pbuf[0] = 0;
        return NULL;
    }
    return pbuf;
}

//--------------------------------------------------------------------
// XML parser callback handler used to parse the supported IME list.
// The expected XML format is:
//
//  <IMEList>
//      <IME>
//          <imeName>...</imeName>
//          <displayName>...</displayName>
//          <Tag>GFxIME_...</Tag>
//      </IME>
//      <Language>
//          <language>...</language>
//          <Tag>GFxIME_...</Tag>
//      </Language>
//  </IMEList>
//--------------------------------------------------------------------

class ExpatCallbackHandler2
{
public:
    char*              Buffer;
    int                BufIndex;
    bool               mark_language;
    GImeNamesManager*  pIMENamesMgr;
    GFxIMEManagerWin32* pImeMgrWin32;

    ExpatCallbackHandler2(GFxIMEManagerWin32* pImeMgr, GImeNamesManager* pnamesMgr)
        : Buffer(0), BufIndex(0), mark_language(false),
          pIMENamesMgr(pnamesMgr), pImeMgrWin32(pImeMgr)
    {
        Buffer = (char*)SF_ALLOC(1024, StatIME_Mem);
        Buffer[0] = 0;
    }
    ~ExpatCallbackHandler2()
    {
        if (Buffer)
            SF_FREE(Buffer);
    }

    static void XMLCALL StartElement(void* pUserData, const char* pname, const char** patts);
    static void XMLCALL EndElement(void* pUserData, const char* pname);
    static void XMLCALL CharacterData(void* pUserData, const char* ptext, int len);

    void AddCharData(const char* ptext, int len)
    {
        if (!Buffer || len <= 0)
            return;
        if (BufIndex + len >= 1023)
            len = 1023 - BufIndex;
        if (len <= 0)
            return;
        memcpy(Buffer + BufIndex, ptext, len);
        BufIndex += len;
        Buffer[BufIndex] = 0;
    }
    void AppendName(GImeNamesManager* pnamesMgr, const char* pname)
    {
        if (!pnamesMgr || pnamesMgr->SupportedIMEs.GetSize() == 0)
            return;
        InputLangProps2& entry = pnamesMgr->SupportedIMEs[pnamesMgr->SupportedIMEs.GetSize() - 1];
        switch (pnamesMgr->label)
        {
        case GImeNamesManager::IMENAME:
            entry.ItemNameOnSystem = pname ? pname : "";
            break;
        case GImeNamesManager::DISPLAYNAME:
            entry.ItemNameCommon = pname ? pname : "";
            break;
        case GImeNamesManager::IMETAG:
            if (pname)
            {
                for (int i = 0; g_StringToTagTable[i].Tag != GFxIME_NotSet; i++)
                {
                    if (SFstrcmp(g_StringToTagTable[i].String, pname) == 0)
                    {
                        entry.ItemTag = g_StringToTagTable[i].Tag;
                        break;
                    }
                }
            }
            break;
        default:
            break;
        }
    }
};

void XMLCALL ExpatCallbackHandler2::StartElement(void* pUserData, const char* pname, const char** patts)
{
    SF_UNUSED(patts);
    if (!pUserData)
        return;
    ExpatCallbackHandler2* pHandler = (ExpatCallbackHandler2*)pUserData;
    GImeNamesManager* pnamesMgr = pHandler->pIMENamesMgr;
    if (!pnamesMgr)
        return;

    pHandler->BufIndex = 0;
    if (pHandler->Buffer)
        pHandler->Buffer[0] = 0;

    if (SFstrcmp(pname, "IME") == 0)
    {
        InputLangProps2 entry;
        pnamesMgr->SupportedIMEs.PushBack(entry);
        pnamesMgr->mark_ime = true;
    }
    else if (SFstrcmp(pname, "imeName") == 0)
    {
        pnamesMgr->label = GImeNamesManager::IMENAME;
    }
    else if (SFstrcmp(pname, "displayName") == 0)
    {
        pnamesMgr->label = GImeNamesManager::DISPLAYNAME;
    }
    else if (SFstrcmp(pname, "Tag") == 0)
    {
        pnamesMgr->label = GImeNamesManager::IMETAG;
    }
    else if (SFstrcmp(pname, "Language") == 0)
    {
        pHandler->mark_language = true;
    }
    else if (SFstrcmp(pname, "language") == 0)
    {
        pnamesMgr->label = GImeNamesManager::IMENAME;
    }
}

void XMLCALL ExpatCallbackHandler2::EndElement(void* pUserData, const char* pname)
{
    if (!pUserData)
        return;
    ExpatCallbackHandler2* pHandler = (ExpatCallbackHandler2*)pUserData;
    GImeNamesManager* pnamesMgr = pHandler->pIMENamesMgr;
    if (!pnamesMgr)
        return;

    if (SFstrcmp(pname, "imeName") == 0 || SFstrcmp(pname, "displayName") == 0 ||
        SFstrcmp(pname, "Tag") == 0)
    {
        pHandler->AppendName(pnamesMgr, pHandler->Buffer);
        pnamesMgr->label = GImeNamesManager::NONE;
    }
    else if (SFstrcmp(pname, "IME") == 0)
    {
        pnamesMgr->mark_ime = false;
    }
    else if (SFstrcmp(pname, "language") == 0)
    {
        // Record an input language entry.
        InputLangProps lang;
        lang.ItemNameOnSystem = pHandler->Buffer ? (char*)pHandler->Buffer : (char*)"";
        lang.ItemNameCommon   = lang.ItemNameOnSystem;
        lang.ItemTag          = GFxIME_NotSet;
        lang.Id               = 0;
        pnamesMgr->SupportedInputLanguages.PushBack(lang);
        pnamesMgr->NumLanguages++;
        pnamesMgr->label = GImeNamesManager::NONE;
    }
    else if (SFstrcmp(pname, "Language") == 0)
    {
        pHandler->mark_language = false;
    }
}

void XMLCALL ExpatCallbackHandler2::CharacterData(void* pUserData, const char* ptext, int len)
{
    if (!pUserData)
        return;
    ExpatCallbackHandler2* pHandler = (ExpatCallbackHandler2*)pUserData;
    pHandler->AddCharData(ptext, len);
}

//--------------------------------------------------------------------
// GFxIMEManagerWin32 - facade implementation
//--------------------------------------------------------------------

GFxIMEManagerWin32::GFxIMEManagerWin32(HWND hwnd)
    : IMEManagerBase()
{
    hWnd                   = hwnd;
    pIME                   = NULL;
    pIMENamesMgr           = NULL;
    CandListLoaded         = false;
    IMETag                 = GFxIME_NotSet;
    ConversionParams       = 0;
    SentenceParams         = 0;
    ImeOpenStatus          = true;
    NamesManagerInitStatus = false;
    Version                = Unknown;
    ImeVersionId           = 0;
    pMovie                 = NULL;
    pLog                   = NULL;
}

GFxIMEManagerWin32::~GFxIMEManagerWin32()
{
    delete pIME;
    delete pIMENamesMgr;
}

UInt32 GFxIMEManagerWin32::Init(Log* plog, FileOpener* pfileOpener, const char* xmlFileName, bool bCheck)
{
    pLog = plog;
    GetWindowsVersion(Version);
    this->bCheckIMEExists = bCheck;

    if (Version == WinXP)
        pIMENamesMgr = new GImeNamesManagerXP(this);
    else
        pIMENamesMgr = new GImeNamesManagerVista(this);
    if (!pIMENamesMgr)
        return 0;

    // Populate the compiled-in list of supported IMEs.
    pIMENamesMgr->PopulateSupportedIMEs();

    // Parse the XML file, if one was provided.
    if (pfileOpener && xmlFileName)
    {
        ExpatCallbackHandler2 handler(this, pIMENamesMgr);
        XML_Parser pParser = XML_ParserCreate(NULL);
        if (pParser)
        {
            XML_SetElementHandler(pParser, ExpatCallbackHandler2::StartElement,
                                  ExpatCallbackHandler2::EndElement);
            XML_SetCharacterDataHandler(pParser, ExpatCallbackHandler2::CharacterData);
            XML_SetUserData(pParser, &handler);

            File* pfile = pfileOpener->OpenFile(xmlFileName, FileConstants::Open_Read,
                                                FileConstants::Mode_ReadWrite);
            if (pfile)
            {
                char buf[512];
                SInt32 nread;
                while ((nread = pfile->Read((UByte*)buf, sizeof(buf))) > 0)
                {
                    if (XML_Parse(pParser, buf, (int)nread, 0) == XML_STATUS_ERROR)
                    {
                        if (pLog)
                            pLog->LogMessage(
                                "GFxIME: XML parse error reading '%s'\n", xmlFileName);
                        break;
                    }
                }
                XML_Parse(pParser, NULL, 0, 1);
                delete pfile;
            }
            else if (pLog)
            {
                pLog->LogMessage(
                    "GFxIME: Failed to open XML file '%s'\n", xmlFileName);
            }
            XML_ParserFree(pParser);
        }
    }
    else if (pLog)
    {
        pLog->LogMessage(
            "GFxIME: No file opener or XML file specified; using built-in IME list.\n");
    }

    // Build the HKL -> name hash used to resolve IME tags.
    pIMENamesMgr->HashIMENames();

    // Create the Win32 impl layer for the detected OS.
    if (Version == WinXP)
        pIME = new GFxIMEXP(this, hWnd);
    else
        pIME = new GFxIMEVista(this, hWnd);
    if (!pIME)
        return 0;

    if (pIME && pMovie)
        pIME->SetMovie(pMovie);

    return 1;
}

void GFxIMEManagerWin32::SetActiveMovie(Movie* pmovie)
{
    pMovie = pmovie;
    if (pIME)
        pIME->SetMovie(pmovie);
    CandListLoaded = false;
}

unsigned GFxIMEManagerWin32::HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent)
{
    SF_UNUSED(pmovie);
    if (!pIME)
        return Movie::HE_NotHandled;

    if (imeEvent.IMEEvtType == IMEEvent::IME_PreProcessKeyboard)
    {
        const IMEWin32Event& winEvt = static_cast<const IMEWin32Event&>(imeEvent);
        pIME->PreProcessWM_KEYDOWN(winEvt);
        return Movie::HE_NotHandled;
    }

    const IMEWin32Event& winEvt = static_cast<const IMEWin32Event&>(imeEvent);
    switch (winEvt.Message)
    {
    case WM_IME_STARTCOMPOSITION:
        pIME->OnIMEStart(winEvt.WParam, winEvt.LParam, (winEvt.Options != 0));
        return Movie::HE_Handled;
    case WM_IME_COMPOSITION:
        pIME->OnIME(winEvt.WParam, winEvt.LParam, 0);
        return Movie::HE_Handled;
    case WM_IME_ENDCOMPOSITION:
        pIME->OnIMEStop(winEvt.WParam, winEvt.LParam, (winEvt.Options != 0));
        return Movie::HE_Handled;
    case WM_IME_NOTIFY:
        pIME->OnIMECandidateProcessing(winEvt.Message, winEvt.WParam, winEvt.LParam, false);
        return Movie::HE_Handled;
    case WM_IME_SETCONTEXT:
    case WM_IME_CONTROL:
        return 0;
    default:
        return Movie::HE_NotHandled;
    }
}

void GFxIMEManagerWin32::IMECommand(Movie* pmovie, const char* pcommand, const char* parg)
{
    if (!pcommand)
        return;
    if (SFstrcmp(pcommand, "recordCandidateListPath") == 0)
    {
        RecordCandidateListPath(parg);
        return;
    }
    if (SFstrcmp(pcommand, "clearCandidateListPath") == 0)
    {
        ClearCandidateListPath();
        return;
    }
    if (SFstrcmp(pcommand, "recordStatusWindowPath") == 0)
    {
        RecordStatusWindowPath(parg);
        return;
    }
    if (SFstrcmp(pcommand, "recordLanguageBarPath") == 0)
    {
        RecordLanguageBarPath(parg);
        return;
    }
    if (SFstrcmp(pcommand, "recordIMEControllerPath") == 0)
    {
        RecordIMEControllerPath(parg);
        return;
    }
    if (SFstrcmp(pcommand, "setCandidateListProps") == 0 && parg)
    {
        int size = 0, c1 = 0, c2 = 0;
        if (sscanf_s(parg, "%d %d %d", &size, &c1, &c2) == 3)
            SetCandidateListProperties(size, c1, c2);
        return;
    }
    if (SFstrcmp(pcommand, "setSelection") == 0)
    {
        if (pIME)
            pIME->SelectAndClose(parg ? atoi(parg) : 0);
        return;
    }
    if (SFstrcmp(pcommand, "displayStatusWindow") == 0 ||
        SFstrcmp(pcommand, "removeStatusWindow") == 0)
    {
        if (pIMENamesMgr)
            pIMENamesMgr->HandleStatusWindowNotifications(pcommand, parg);
        return;
    }
    if (pIME)
        pIME->FsCallBack(pmovie, pcommand, parg);
}

void GFxIMEManagerWin32::ProcessIMEMessage(const FunctionHandler::Params& params)
{
    if (params.ArgCount < 1 || !params.pArgs)
        return;
    const Value& vCmd = params.pArgs[0];
    if (!vCmd.IsString())
        return;
    const char* pcommand = vCmd.GetString();
    const char* parg = NULL;
    if (params.ArgCount >= 2 && params.pArgs[1].IsString())
        parg = params.pArgs[1].GetString();
    IMECommand(params.pMovie ? params.pMovie : pMovie, pcommand, parg);
}

void GFxIMEManagerWin32::OnFinalize()
{
    if (pIME)
        pIME->Finalize();
}

void GFxIMEManagerWin32::OnShutdown()
{
    if (pIME)
        pIME->Shutdown();
    IMEManagerBase::OnShutdown();
}

bool GFxIMEManagerWin32::IsCandidateList(const char* path)
{
    return path ? (strstr(path, "_level9999") != NULL) : false;
}

bool GFxIMEManagerWin32::IsStatusWindow(const char* path)
{
    return path ? (strstr(path, "_level9998") != NULL) : false;
}

bool GFxIMEManagerWin32::IsLangBar(const char* path)
{
    return path ? (strstr(path, "_level9997") != NULL) : false;
}

void GFxIMEManagerWin32::OnCandidateListLoaded(const char* pcandidateListPath)
{
    RecordCandidateListPath(pcandidateListPath);
    CandListLoaded = true;
}

void GFxIMEManagerWin32::OnCandidateListStyleChanged(const IMECandidateListStyle& style)
{
    if (pIME)
        pIME->OnCandidateListStyleChanged(style);
}

void GFxIMEManagerWin32::OnEnableIME(bool enable)
{
    if (pIME)
        pIME->OnEnableIME(enable);
}

void GFxIMEManagerWin32::SendLangBarMessage(DisplayObject* mc, const ASString& command, const ASString& message)
{
    SF_UNUSED(mc);
    if (pIMENamesMgr)
        pIMENamesMgr->HandleStatusWindowNotifications(command.ToCStr(), message.ToCStr());
}

void GFxIMEManagerWin32::Init()
{
    Init(0, (FileOpener*)0, 0, true);
}

void GFxIMEManagerWin32::SetWndHandle(HWND hwnd)
{
    hWnd = hwnd;
    if (pIME)
        pIME->SetWndHandle(hwnd);
}

void GFxIMEManagerWin32::RecordCandidateListPath(const char* pcandidateListPath)
{
    if (pIME)
        pIME->RecordCandidateListPath(pcandidateListPath);
}

void GFxIMEManagerWin32::ClearCandidateListPath()
{
    if (pIME)
        pIME->ClearCandidateListPath();
}

void GFxIMEManagerWin32::RecordStatusWindowPath(const char* pstatusWindowPath)
{
    if (pIME)
        pIME->RecordStatusWindowPath(pstatusWindowPath);
}

void GFxIMEManagerWin32::RecordLanguageBarPath(const char* planguageBarPath)
{
    if (pIME)
        pIME->RecordLanguageBarPath(planguageBarPath);
}

void GFxIMEManagerWin32::RecordIMEControllerPath(const char* pimeControllerPath)
{
    SF_UNUSED(pimeControllerPath);
}

void GFxIMEManagerWin32::SetCandidateListProperties(INT size, INT fontColor1, INT fontColor2)
{
    if (pIME)
        pIME->SetCandidateListProperties(size, fontColor1, fontColor2);
}

LRESULT GFxIMEManagerWin32::OnIMECandidateProcessing(UInt32 message, UInt32 key, UInt32 info, bool downFlag)
{
    if (pIME)
        return pIME->OnIMECandidateProcessing(message, key, info, downFlag);
    return 0;
}

void GFxIMEManagerWin32::SetInputLanguage(const char* inputLanguage)
{
    if (pIMENamesMgr)
        pIMENamesMgr->ActivateInputLanguage(inputLanguage);
}

bool GFxIMEManagerWin32::SetConversionMode(UInt32 convMode)
{
    if (pIME)
        pIME->SetConversionMode(convMode);
    if (pIMENamesMgr)
        pIMENamesMgr->SetConversionMode(convMode);
    return true;
}

const char* GFxIMEManagerWin32::GetConversionMode()
{
    return pIME ? pIME->GetConversionMode() : "";
}

bool GFxIMEManagerWin32::SetEnabled(bool enable)
{
    if (pIME)
        pIME->SetEnabled(enable);
    return enable;
}

bool GFxIMEManagerWin32::GetEnabled()
{
    return pIME ? pIME->GetEnabled() : false;
}

const String GFxIMEManagerWin32::GetInputLanguage()
{
    return pIME ? pIME->GetInputLanguage() : String();
}

void GFxIMEManagerWin32::SetCurrentInputLanguage(GFxIMETag imetag)
{
    IMETag = imetag;
    if (pIME)
        pIME->SetIMETag(imetag);
}

bool GFxIMEManagerWin32::SetCompositionString(const char* pCompString)
{
    if (pIME)
    {
        pIME->SetCompositionString(pCompString);
        return true;
    }
    return false;
}

void GFxIMEManagerWin32::CustomProcessing(UInt32 message, UInt32 key, UInt32 info)
{
    if (pIME)
        pIME->CustomProcessing(message, (WPARAM)key, (LPARAM)info);
}

void GFxIMEManagerWin32::SelectAndClose(int index)
{
    if (pIME)
        pIME->SelectAndClose(index);
}

void GFxIMEManagerWin32::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{
    if (pIME)
        pIME->PreProcessWM_KEYDOWN(winEvt);
}

void GFxIMEManagerWin32::DisplayReadingWindow(wchar_t* pReadingStr)
{
    if (pIME)
        pIME->DisplayReadingWindow(pReadingStr);
}

void GFxIMEManagerWin32::DisplayCompositionString(UInt32& numCharDisplay)
{
    if (pIME)
        pIME->DisplayCompositionString(numCharDisplay);
}

bool GFxIMEManagerWin32::CheckIfInTextField()
{
    return pIME ? pIME->CheckIfInTextField() : false;
}

String GFxIMEManagerWin32::GetSystemLanguageInfo()
{
    char buf[64];
    SFsprintf(buf, sizeof(buf), "%d", pIMENamesMgr ? (int)pIMENamesMgr->GetNumSupportedIMEs() : 0);
    return String(buf);
}

String GFxIMEManagerWin32::GetWindowsVersion()
{
    switch (Version)
    {
    case WinXP:    return "WinXP";
    case WinVista: return "WinVista";
    case Win2K:    return "Win2K";
    default:       return "Unknown";
    }
}

bool GFxIMEManagerWin32::SwitchIME(String imeName)
{
    if (pIMENamesMgr)
        return pIMENamesMgr->SwitchIME(imeName);
    return false;
}

void GFxIMEManagerWin32::GetWindowsVersion(OSVersion& os)
{
    // RtlGetVersion reports the real OS version regardless of the
    // application manifest; GetVersionEx is deprecated.
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
            os = WinVista;
        else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)
            os = WinXP;
        else
            os = Win2K;
    }
    else
        os = Unknown;
}

void GFxIMEManagerWin32::SetIMETag(GFxIMETag imeTag)
{
    IMETag = imeTag;
    if (pIME)
        pIME->SetIMETag(imeTag);
}

void GFxIMEManagerWin32::SetImeVersionId(int versionId)
{
    ImeVersionId = versionId;
    if (pIME)
        pIME->SetImeVersionId(versionId);
}

void GFxIMEManagerWin32::ResetText()
{
    CompositionString = "";
    if (pIME)
        pIME->Cleanup();
}

//--------------------------------------------------------------------
// GFxIMEVista - TSF/IMM based implementation for Windows Vista and up
//--------------------------------------------------------------------

GFxIMEVista::GFxIMEVista(IMEManagerBase* pbase, HWND hwnd)
    : GFxIMEWin32Impl(pbase, hwnd)
{
    CurrLocale              = NULL;
    NumCharDisplay          = 0;
    ReplaceChar             = false;
    ClausePosition          = 0;
    CandListStartFrom1      = 1;
    StartCompositionReceived = false;
    DrawCandidateList       = false;
    CandWndRefCount         = 0;
    NeedToDisableIME        = false;
    NumCandidates           = 0;
    PrevX                   = 0;
    PrevY                   = 0;

    ReadingTextBuffer.Resize(ReadingTextBufferSize);
    ReadingTextBuffer[0] = 0;

    pTSF = new CTSF(this, hwnd);
    if (pTSF && !pTSF->Init())
    {
        if (pBase && pBase->pLog)
            pBase->pLog->LogMessage("GFxIME: TSF Init() failure!\n");
    }
}

GFxIMEVista::~GFxIMEVista()
{
    if (pTSF)
    {
        pTSF->UnInitUIElementSink();
        pTSF->UninitThreadMgrSink();
        pTSF->ReleaseInterfaces();
        pTSF->Release();
        pTSF = NULL;
    }
}

void GFxIMEVista::OnIME(UPInt key, UPInt info, int options)
{
    SF_UNUSED(options);

    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    char* lpastr = NULL;

    if (info == 0)
    {
        NumCharDisplay = 0;
        return;
    }

    if (info & GCS_RESULTSTR)
    {
        hIMC = ImmGetContext(hWND);
        dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
        lpstr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        LONG res = ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
        if (res != IMM_ERROR_NODATA && res != IMM_ERROR_GENERAL)
            lpstr[dwSize / 2] = 0;
        NumCharDisplay = 0;
        ReplaceChar = false;
        ReadingTextBuffer[0] = 0;
        ImmReleaseContext(hWND, hIMC);
        pBase->SetWideCursor(false);
        pBase->FinalizeComposition(lpstr);
        pBase->BroadcastIMEConversion(lpstr);
        SF_FREE(lpstr);
        return;
    }

    if ((info & CS_INSERTCHAR) && (info & CS_NOMOVECARET))
    {
        wchar_t ch[2];
        ch[0] = (wchar_t)key;
        ch[1] = 0;
        ReplaceChar = true;
        NumCharDisplay = 1;
        pBase->SetWideCursor(true);
        pBase->SetCompositionPosition();
        pBase->SetCompositionText(ch);
        pBase->SetCursorInComposition(0);
    }
    else if ((info & GCS_COMPSTR) && (info & GCS_CURSORPOS))
    {
        DisplayCompositionString(NumCharDisplay);
    }

    if (info & GCS_COMPATTR)
    {
        hIMC = ImmGetContext(hWND);
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPATTR, NULL, 0);
        lpastr = (char*)SF_ALLOC(dwSize + 1, StatIME_Mem);
        memset(lpastr, 0, dwSize + 1);
        ImmGetCompositionString(hIMC, GCS_COMPATTR, lpastr, dwSize);
        int numChar = (dwSize < (DWORD)NumCharDisplay) ? (int)dwSize : (int)NumCharDisplay;
        ClausePosition = numChar;
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPREADSTR, NULL, 0);
        wchar_t* lpReadStr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        memset(lpReadStr, 0, 2 * (dwSize + 1));
        ImmGetCompositionString(hIMC, GCS_COMPREADSTR, lpReadStr, dwSize);
        lpReadStr[dwSize / 2] = 0;
        int pos = (int)ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        ImmReleaseContext(hWND, hIMC);
        pBase->SetCursorInComposition((UPInt)pos);

        if (!((UInt32)IMETag & GFxIME_Kr_Flag))
        {
            char currentStyle = lpastr[0];
            int  charPos = 0;
            for (int i = 0; i < numChar; i++)
            {
                if (lpastr[i] != currentStyle)
                {
                    pBase->HighlightText((UPInt)charPos, (UPInt)(i - charPos),
                                         GFxIMEAttrToStyle(currentStyle), false);
                    currentStyle = lpastr[i];
                    charPos = i;
                }
            }
            pBase->HighlightText((UPInt)charPos, (UPInt)(numChar - charPos),
                                 GFxIMEAttrToStyle(currentStyle), false);
        }
        DisplayReadingWindow(lpReadStr);
        SF_FREE(lpReadStr);
        SF_FREE(lpastr);
    }
}

UInt32 GFxIMEVista::OnIMEStart(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    StartCompositionReceived = true;
    pBase->StartComposition();
    CurrLocale = GetKeyboardLayout(0);
    int prop = ImmGetProperty(CurrLocale, IGP_PROPERTY);
    CandListStartFrom1 = (prop & IME_PROP_CANDLIST_START_FROM_1) == 0 ? 0 : 1;
    if (IMETag == GFxIME_Ch_Trad_WuXia || IMETag == GFxIME_Ch_Trad_DaYi)
        CandListStartFrom1 = 0;
    if (IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Ch_Simp_WuBi98)
        CandListStartFrom1 = 1;

    CANDIDATEFORM candForm;
    COMPOSITIONFORM compForm;
    POINT pt;
    pt.x = -1000;
    pt.y = -1000;
    candForm.dwIndex = 0;
    candForm.dwStyle = CFS_CANDIDATEPOS;
    candForm.ptCurrentPos = pt;
    compForm.dwStyle = CFS_FORCE_POSITION;
    compForm.ptCurrentPos = pt;
    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmSetCompositionWindow(hIMC, &compForm);
        ImmSetCandidateWindow(hIMC, &candForm);
        ImmReleaseContext(hWND, hIMC);
    }

    if (IMETag == GFxIME_NotSupported)
        return Movie::HE_NotHandled;
    return Movie::HE_Handled;
}

void GFxIMEVista::OnIMEStop(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    pBase->SetWideCursor(false);
    pBase->ClearComposition();
    StartCompositionReceived = false;
}

void GFxIMEVista::OnChangeCandidate(int candListIndex)
{
    SF_UNUSED(candListIndex);
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    int numChar = 0;

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCandidateListW(hIMC, 0, NULL, 0);
    if (dwSize)
    {
        dwSize += 4;
        lpstr = (wchar_t*)SF_ALLOC(dwSize, StatIME_Mem);
        memset(lpstr, 0, dwSize);
        ImmGetCandidateListW(hIMC, 0, (LPCANDIDATELIST)lpstr, dwSize);
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        numChar = (int)pCandList->dwCount;
    }
    ImmReleaseContext(hWND, hIMC);
    if (!lpstr)
        return;

    if (ListBox)
    {
        ListBox->RemoveAllListItems();
        UInt32 numRows = 10;
        int nStartFrom1 = CandListStartFrom1 ? 1 : 0;
        int nSelectedIndex = -1;
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        DWORD selected = pCandList->dwSelection;
        if (selected >= numRows)
            nStartFrom1 = 0;
        else if (CandListStartFrom1)
            nStartFrom1 = 1;
        nSelectedIndex = (int)selected;
        NumCandidates = numChar;

        for (int i = 0; i < numChar; i++)
        {
            wchar_t* pStr = (wchar_t*)((char*)pCandList + pCandList->dwOffset[i]);
            char itemBuf[512];
            ListBox->AddListItem(new CandidateListItem(
                IMEWideToUtf8(pStr, itemBuf, sizeof(itemBuf)) ? itemBuf : (char*)""));
        }
        ListBox->UIRefreshView();
        ListBox->SetSelectedItemIndex(nSelectedIndex);
    }
    SF_FREE(lpstr);
}

void GFxIMEVista::OnOpenCandidate(int candListIndex)
{
    SF_UNUSED(candListIndex);
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    int numChar = 0;

    if (pMovie && CandListPath.GetLength() > 0)
    {
        pMovie->Invoke((CandListPath + ".removeList").ToCStr(), "");
        pMovie->Invoke((CandListPath + ".createList").ToCStr(), "%d %d", 10,
                       CandListStartFrom1 ? 1 : 0);
    }

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCandidateListW(hIMC, 0, NULL, 0);
    if (dwSize)
    {
        dwSize += 4;
        lpstr = (wchar_t*)SF_ALLOC(dwSize, StatIME_Mem);
        memset(lpstr, 0, dwSize);
        ImmGetCandidateListW(hIMC, 0, (LPCANDIDATELIST)lpstr, dwSize);
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        numChar = (int)pCandList->dwCount;
    }
    ImmReleaseContext(hWND, hIMC);
    if (!lpstr)
        return;

    if (ListBox)
    {
        ListBox->RemoveAllListItems();
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        NumCandidates = numChar;
        for (int i = 0; i < numChar; i++)
        {
            wchar_t* pStr = (wchar_t*)((char*)pCandList + pCandList->dwOffset[i]);
            char itemBuf[512];
            ListBox->AddListItem(new CandidateListItem(
                IMEWideToUtf8(pStr, itemBuf, sizeof(itemBuf)) ? itemBuf : (char*)""));
        }
        ListBox->UIRefreshView();
    }
    SF_FREE(lpstr);
}

LRESULT GFxIMEVista::OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED(message);
    int nRecd = (int)key;
    switch (nRecd)
    {
    case IMN_CHANGECANDIDATE:
        OnChangeCandidate((int)info);
        break;
    case IMN_OPENCANDIDATE:
        OnOpenCandidate((int)info);
        break;
    case IMN_CLOSECANDIDATE:
        if (pMovie && CandListPath.GetLength() > 0)
            pMovie->Invoke((CandListPath + ".removeList").ToCStr(), "");
        if (ListBox)
            ListBox->RemoveAllListItems();
        break;
    case IMN_PRIVATE:
        CustomProcessing(message, key, info);
        break;
    default:
        break;
    }
    SF_UNUSED(downFlag);
    return S_OK;
}

void GFxIMEVista::Finalize()
{
    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
        ImmReleaseContext(hWND, hIMC);
    }
    if (pTSF)
        pTSF->AbortCandidateList();
}

void GFxIMEVista::SelectAndClose(int index)
{
    // For 3rd party IMEs we cannot force candidate selection through
    // ImmNotifyIME; the IME is handled by the TSF layer.
    if (IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_SogouPinyin ||
        IMETag == GFxIME_Ch_Simp_JJ || IMETag == GFxIME_Ch_Simp_ABC)
    {
        return;
    }

    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmNotifyIME(hIMC, NI_SELECTCANDIDATESTR, 0, index);
        ImmReleaseContext(hWND, hIMC);
    }
    if (pTSF)
        pTSF->SelectAndClose(index);
}

void GFxIMEVista::FsCallBack(Movie* pmovie, const char* pcommand, const char* parg)
{
    if (pcommand && SFstrcmp(pcommand, "Selection.setSelection") == 0)
    {
        SelectAndClose(parg ? atoi(parg) : 0);
        return;
    }
    CmdManager.FsCallback(pmovie, pcommand, parg);
}

void GFxIMEVista::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{
    if (winEvt.Message == WM_KEYDOWN && winEvt.WParam == VK_SHIFT)
    {
        Track = true;
        ImeMessageRecd = false;
    }
    if (Track)
    {
        if (ImmIsUIMessage(NULL, (UINT)winEvt.Message, winEvt.WParam, winEvt.LParam))
            ImeMessageRecd = true;
    }
    if (winEvt.Message == WM_KEYUP)
    {
        if (Track)
        {
            Track = false;
            if (!ImeMessageRecd && NumCharDisplay == 0)
                CloseIME();
        }
        if (DrawCandidateList)
        {
            RepositionCandidateList((UInt32)-ClausePosition);
            DrawCandidateList = false;
        }
    }
}

void GFxIMEVista::DisplayCompositionString(UInt32& numCharDisplay)
{
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
    if (dwSize)
    {
        lpstr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        memset(lpstr, 0, 2 * (dwSize + 1));
        LONG res = ImmGetCompositionString(hIMC, GCS_COMPSTR, lpstr, dwSize);
        if (res != IMM_ERROR_NODATA && res != IMM_ERROR_GENERAL)
            lpstr[dwSize / 2] = 0;
    }
    ImmReleaseContext(hWND, hIMC);

    if (lpstr == NULL)
    {
        pBase->SetWideCursor(false);
        return;
    }

    numCharDisplay = dwSize / 2;

    switch (IMETag)
    {
    case GFxIME_Ch_Trad_NewChangJie:
    case GFxIME_Ch_Trad_DaYi:
        pBase->SetWideCursor(true);
        pBase->SetCompositionPosition();
        pBase->SetCompositionText(lpstr);
        pBase->SetCursorInComposition(numCharDisplay);
        break;

    default:
        if (dwSize == 0)
        {
            pBase->SetWideCursor(false);
        }
        else if (!ReplaceChar)
        {
            pBase->SetWideCursor(true);
            pBase->SetCompositionPosition();
            pBase->SetCompositionText(lpstr);
            int nCursorPos = (int)ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
            pBase->SetCursorInComposition((UPInt)nCursorPos);
        }
        break;
    }
    SF_FREE(lpstr);
}

void GFxIMEVista::DisplayReadingWindow(wchar_t* pReadingStr)
{
    if (!pMovie || CandListPath.GetLength() == 0)
        return;
    if (!pReadingStr || pReadingStr[0] == 0)
        return;

    ReadinStringBuffer = pReadingStr;

    RectF viewRect, caretRect;
    pBase->GetMetrics(&viewRect, &caretRect, -ClausePosition);
    String utf8str(pReadingStr);
    pMovie->Invoke((CandListPath + ".displayInputWindow").ToCStr(), "%d %d %s",
                   (int)caretRect.x1, (int)caretRect.y1, utf8str.ToCStr());
}

void GFxIMEVista::RepositionCandidateList(UInt32 offset)
{
    if (!pMovie || CandListPath.GetLength() == 0)
        return;
    RectF viewRect, caretRect;
    pBase->GetMetrics(&viewRect, &caretRect, (int)offset);
    pMovie->Invoke((CandListPath + ".repositionCandidateList").ToCStr(), "%d %d",
                   (int)caretRect.x1, (int)caretRect.y1);
}

void GFxIMEVista::CloseIME()
{
    Cleanup();
}

//--------------------------------------------------------------------
// GFxIMEXP - IMM based implementation for Windows XP
//--------------------------------------------------------------------

GFxIMEXP::GFxIMEXP(IMEManagerBase* pbase, HWND hwnd)
    : GFxIMEWin32Impl(pbase, hwnd)
{
    CurrLocale              = 0;
    NumRowsMax              = 0;
    NumCharDisplay          = 0;
    Track                   = false;
    ImeMessageRecd          = false;
    CandListStartFrom1      = 1;
    DrawCandidateList       = false;
    LastCharacter           = 0;
    ReadingWindowCharacterPos = 0;
    CurrChar                = 1;
    hSavedIMC               = 0;
    CandidateListIndex      = 0;

    memset(TradNewPhoneticKeyCodesB, 0, sizeof(TradNewPhoneticKeyCodesB));
    int size = (int)(sizeof(TradNewPhoneticKeyCodesA) / (3 * sizeof(int)));
    for (int i = 0; i < size; i++)
    {
        char c = (char)TradNewPhoneticKeyCodesA[i][0];
        int idx = (int)((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c) - 0x20;
        if (idx >= 0 && idx < NumTradNewPhoneticKeyCodes)
        {
            TradNewPhoneticKeyCodesB[idx][0] = TradNewPhoneticKeyCodesA[i][0];
            TradNewPhoneticKeyCodesB[idx][1] = TradNewPhoneticKeyCodesA[i][1];
            TradNewPhoneticKeyCodesB[idx][2] = TradNewPhoneticKeyCodesA[i][2];
        }
    }
    ReadingTextChTradNewPhonetic.Resize(9);
    ReadingTextChTradNewPhonetic[0] = 0;
}

GFxIMEXP::~GFxIMEXP()
{
}

void GFxIMEXP::OnIME(UPInt key, UPInt info, int options)
{
    SF_UNUSED(options);

    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    char* lpastr = NULL;

    if (info == 0)
    {
        NumCharDisplay = 0;
        return;
    }

    if (info & GCS_RESULTSTR)
    {
        hIMC = ImmGetContext(hWND);
        dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
        lpstr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        LONG res = ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
        if (res != IMM_ERROR_NODATA && res != IMM_ERROR_GENERAL)
            lpstr[dwSize / 2] = 0;
        NumCharDisplay = 0;
        ReplaceChar = false;
        ReadingTextBuffer[0] = 0;
        ImmReleaseContext(hWND, hIMC);
        pBase->SetWideCursor(false);
        pBase->FinalizeComposition(lpstr);
        pBase->BroadcastIMEConversion(lpstr);
        SF_FREE(lpstr);
        return;
    }

    if ((info & CS_INSERTCHAR) && (info & CS_NOMOVECARET))
    {
        wchar_t ch[2];
        ch[0] = (wchar_t)key;
        ch[1] = 0;
        ReplaceChar = true;
        NumCharDisplay = 1;
        pBase->SetWideCursor(true);
        pBase->SetCompositionPosition();
        pBase->SetCompositionText(ch);
        pBase->SetCursorInComposition(0);
    }
    else if ((info & GCS_COMPSTR) && (info & GCS_CURSORPOS))
    {
        DisplayCompositionString(NumCharDisplay);
    }

    if (info & GCS_COMPATTR)
    {
        hIMC = ImmGetContext(hWND);
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPATTR, NULL, 0);
        lpastr = (char*)SF_ALLOC(dwSize + 1, StatIME_Mem);
        memset(lpastr, 0, dwSize + 1);
        ImmGetCompositionString(hIMC, GCS_COMPATTR, lpastr, dwSize);
        int numChar = (dwSize < (DWORD)NumCharDisplay) ? (int)dwSize : (int)NumCharDisplay;
        ClausePosition = numChar;
        dwSize = ImmGetCompositionString(hIMC, GCS_COMPREADSTR, NULL, 0);
        wchar_t* lpReadStr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        memset(lpReadStr, 0, 2 * (dwSize + 1));
        ImmGetCompositionString(hIMC, GCS_COMPREADSTR, lpReadStr, dwSize);
        lpReadStr[dwSize / 2] = 0;
        int pos = (int)ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
        ImmReleaseContext(hWND, hIMC);
        pBase->SetCursorInComposition((UPInt)pos);

        if (!((UInt32)IMETag & GFxIME_Kr_Flag))
        {
            char currentStyle = lpastr[0];
            int  charPos = 0;
            for (int i = 0; i < numChar; i++)
            {
                if (lpastr[i] != currentStyle)
                {
                    pBase->HighlightText((UPInt)charPos, (UPInt)(i - charPos),
                                         GFxIMEAttrToStyle(currentStyle), false);
                    currentStyle = lpastr[i];
                    charPos = i;
                }
            }
            pBase->HighlightText((UPInt)charPos, (UPInt)(numChar - charPos),
                                 GFxIMEAttrToStyle(currentStyle), false);
        }
        DisplayReadingWindow(0, numChar, lpastr, lpReadStr);
        SF_FREE(lpReadStr);
        SF_FREE(lpastr);
    }
}

UInt32 GFxIMEXP::OnIMEStart(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    pBase->StartComposition();
    CurrLocale = GetKeyboardLayout(0);
    int prop = ImmGetProperty(CurrLocale, IGP_PROPERTY);
    CandListStartFrom1 = (prop & IME_PROP_CANDLIST_START_FROM_1) == 0 ? 0 : 1;
    if (IMETag == GFxIME_Ch_Trad_WuXia || IMETag == GFxIME_Ch_Trad_DaYi)
        CandListStartFrom1 = 0;
    if (IMETag == GFxIME_Ch_Simp_WuBi86 || IMETag == GFxIME_Ch_Simp_WuBi98)
        CandListStartFrom1 = 1;

    CANDIDATEFORM candForm;
    COMPOSITIONFORM compForm;
    POINT pt;
    pt.x = -1000;
    pt.y = -1000;
    candForm.dwIndex = 0;
    candForm.dwStyle = CFS_CANDIDATEPOS;
    candForm.ptCurrentPos = pt;
    compForm.dwStyle = CFS_FORCE_POSITION;
    compForm.ptCurrentPos = pt;
    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmSetCompositionWindow(hIMC, &compForm);
        ImmSetCandidateWindow(hIMC, &candForm);
        ImmReleaseContext(hWND, hIMC);
    }

    if (IMETag == GFxIME_NotSupported)
        return Movie::HE_NotHandled;
    return Movie::HE_Handled;
}

void GFxIMEXP::OnIMEStop(UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED3(key, info, downFlag);
    pBase->SetWideCursor(false);
    pBase->ClearComposition();
}

void GFxIMEXP::OnChangeCandidate(int candListIndex)
{
    SF_UNUSED(candListIndex);
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    int numChar = 0;

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCandidateListW(hIMC, 0, NULL, 0);
    if (dwSize)
    {
        dwSize += 4;
        lpstr = (wchar_t*)SF_ALLOC(dwSize, StatIME_Mem);
        memset(lpstr, 0, dwSize);
        ImmGetCandidateListW(hIMC, 0, (LPCANDIDATELIST)lpstr, dwSize);
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        numChar = (int)pCandList->dwCount;
    }
    ImmReleaseContext(hWND, hIMC);
    if (!lpstr)
        return;

    if (ListBox)
    {
        ListBox->RemoveAllListItems();
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        int nStartFrom1 = CandListStartFrom1 ? 1 : 0;
        DWORD selected = pCandList->dwSelection;
        if (selected >= 10)
            nStartFrom1 = 0;
        else if (CandListStartFrom1)
            nStartFrom1 = 1;
        NumCandidates = numChar;

        for (int i = 0; i < numChar; i++)
        {
            wchar_t* pStr = (wchar_t*)((char*)pCandList + pCandList->dwOffset[i]);
            char itemBuf[512];
            ListBox->AddListItem(new CandidateListItem(
                IMEWideToUtf8(pStr, itemBuf, sizeof(itemBuf)) ? itemBuf : (char*)""));
        }
        ListBox->UIRefreshView();
        ListBox->SetSelectedItemIndex((int)selected);
    }
    SF_FREE(lpstr);
}

void GFxIMEXP::OnOpenCandidate(int candListIndex)
{
    SF_UNUSED(candListIndex);
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;
    int numChar = 0;

    if (pMovie && CandListPath.GetLength() > 0)
    {
        pMovie->Invoke((CandListPath + ".removeList").ToCStr(), "");
        pMovie->Invoke((CandListPath + ".createList").ToCStr(), "%d %d", 10,
                       CandListStartFrom1 ? 1 : 0);
    }

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCandidateListW(hIMC, 0, NULL, 0);
    if (dwSize)
    {
        dwSize += 4;
        lpstr = (wchar_t*)SF_ALLOC(dwSize, StatIME_Mem);
        memset(lpstr, 0, dwSize);
        ImmGetCandidateListW(hIMC, 0, (LPCANDIDATELIST)lpstr, dwSize);
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        numChar = (int)pCandList->dwCount;
    }
    ImmReleaseContext(hWND, hIMC);
    if (!lpstr)
        return;

    if (ListBox)
    {
        ListBox->RemoveAllListItems();
        CANDIDATELIST* pCandList = (CANDIDATELIST*)lpstr;
        NumCandidates = numChar;
        for (int i = 0; i < numChar; i++)
        {
            wchar_t* pStr = (wchar_t*)((char*)pCandList + pCandList->dwOffset[i]);
            char itemBuf[512];
            ListBox->AddListItem(new CandidateListItem(
                IMEWideToUtf8(pStr, itemBuf, sizeof(itemBuf)) ? itemBuf : (char*)""));
        }
        ListBox->UIRefreshView();
    }
    SF_FREE(lpstr);
}

LRESULT GFxIMEXP::OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag)
{
    SF_UNUSED(message);
    int nRecd = (int)key;
    switch (nRecd)
    {
    case IMN_CHANGECANDIDATE:
        OnChangeCandidate((int)info);
        break;
    case IMN_OPENCANDIDATE:
        OnOpenCandidate((int)info);
        break;
    case IMN_CLOSECANDIDATE:
        if (pMovie && CandListPath.GetLength() > 0)
            pMovie->Invoke((CandListPath + ".removeList").ToCStr(), "");
        if (ListBox)
            ListBox->RemoveAllListItems();
        break;
    case IMN_PRIVATE:
        CustomProcessing(message, key, info);
        break;
    default:
        break;
    }
    SF_UNUSED(downFlag);
    return S_OK;
}

void GFxIMEXP::Finalize()
{
    if (IMETag == GFxIME_Ch_Simp_JJ)
    {
        // Escape the JJ IME composition mode.
        INPUT input;
        memset(&input, 0, sizeof(input));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_ESCAPE;
        SendInput(1, &input, sizeof(INPUT));
    }
    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
        ImmReleaseContext(hWND, hIMC);
    }
}

void GFxIMEXP::Cleanup()
{
    GFxIMEWin32Impl::Cleanup();
    LastCharacter = 0;
}

void GFxIMEXP::SelectAndClose(int index)
{
    if (IMETag == GFxIME_GooglePinyin || IMETag == GFxIME_SogouPinyin ||
        IMETag == GFxIME_Ch_Simp_JJ || IMETag == GFxIME_Ch_Simp_ABC)
    {
        INPUT input;
        memset(&input, 0, sizeof(input));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (WORD)((index > 0) ? (0x30 + index) : VK_ESCAPE);
        SendInput(1, &input, sizeof(INPUT));
        return;
    }

    HIMC hIMC = ImmGetContext(hWND);
    if (hIMC)
    {
        ImmNotifyIME(hIMC, NI_SELECTCANDIDATESTR, 0, index);
        ImmReleaseContext(hWND, hIMC);
    }
}

void GFxIMEXP::FsCallBack(Movie* pmovie, const char* pcommand, const char* parg)
{
    if (pcommand && SFstrcmp(pcommand, "Selection.setSelection") == 0)
    {
        SelectAndClose(parg ? atoi(parg) : 0);
        return;
    }
    CmdManager.FsCallback(pmovie, pcommand, parg);
}

void GFxIMEXP::PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt)
{
    if (winEvt.Message == WM_KEYDOWN)
    {
        if (winEvt.WParam == VK_SHIFT)
        {
            Track = true;
            ImeMessageRecd = false;
        }
        if (winEvt.WParam == VK_PROCESSKEY)
        {
            // No default key handling for the composition itself.
        }
        if (Track)
        {
            if (ImmIsUIMessage(NULL, (UINT)winEvt.Message, winEvt.WParam, winEvt.LParam))
                ImeMessageRecd = true;
        }
    }
    if (winEvt.Message == WM_KEYUP)
    {
        if (Track)
        {
            Track = false;
            if (!ImeMessageRecd && NumCharDisplay == 0)
                CloseIME();
        }
        if (DrawCandidateList)
        {
            RepositionCandidateList((UInt32)-ClausePosition);
            DrawCandidateList = false;
        }
    }
}

void GFxIMEXP::DisplayReadingWindow(UInt32 keyCode, int numChar, char* lpastr, wchar_t* lpReadStr)
{
    SF_UNUSED2(lpastr, numChar);
    if (!pMovie || CandListPath.GetLength() == 0)
        return;

    wchar_t* pReadingStr = NULL;
    switch (IMETag)
    {
    case GFxIME_Ch_Trad_NewPhonetic:
        if (keyCode >= 0x20 && keyCode <= 0x7A)
        {
            int idx = (int)(keyCode - 0x20);
            if (idx < NumTradNewPhoneticKeyCodes)
            {
                int stroke = TradNewPhoneticKeyCodesB[idx][1];
                int keyRep = TradNewPhoneticKeyCodesB[idx][2];
                if (stroke == 0)
                    pReadingStr = NULL;
                else
                {
                    ReadingTextChTradNewPhonetic[ReadingWindowCharacterPos++] = (wchar_t)keyRep;
                    pReadingStr = &ReadingTextChTradNewPhonetic[0];
                }
            }
        }
        break;

    default:
        pReadingStr = lpReadStr;
        break;
    }

    if (pReadingStr && pReadingStr[0] != 0)
    {
        // Copy to the reading buffer and display it through the input window.
        size_t len = IMEwcslen(pReadingStr);
        if (len >= ReadingTextBufferSize)
            len = ReadingTextBufferSize - 1;
        for (size_t i = 0; i <= len; i++)
            ReadingTextBuffer[i] = pReadingStr[i];

        RectF viewRect, caretRect;
        pBase->GetMetrics(&viewRect, &caretRect, -ClausePosition);
        String utf8str(pReadingStr);
        pMovie->Invoke((CandListPath + ".displayInputWindow").ToCStr(), "%d %d %s",
                       (int)caretRect.x1, (int)caretRect.y1, utf8str.ToCStr());
    }
}

void GFxIMEXP::DisplayCompositionString(UInt32& numCharDisplay)
{
    HIMC hIMC;
    DWORD dwSize;
    wchar_t* lpstr = NULL;

    hIMC = ImmGetContext(hWND);
    if (!hIMC)
        return;
    dwSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
    if (dwSize)
    {
        lpstr = (wchar_t*)SF_ALLOC(2 * (dwSize + 1), StatIME_Mem);
        memset(lpstr, 0, 2 * (dwSize + 1));
        LONG res = ImmGetCompositionString(hIMC, GCS_COMPSTR, lpstr, dwSize);
        if (res != IMM_ERROR_NODATA && res != IMM_ERROR_GENERAL)
            lpstr[dwSize / 2] = 0;
    }
    ImmReleaseContext(hWND, hIMC);

    if (lpstr == NULL)
    {
        pBase->SetWideCursor(false);
        return;
    }

    numCharDisplay = dwSize / 2;

    switch (IMETag)
    {
    case GFxIME_Ch_Trad_NewChangJie:
    case GFxIME_Ch_Trad_DaYi:
        pBase->SetWideCursor(true);
        pBase->SetCompositionPosition();
        pBase->SetCompositionText(lpstr);
        pBase->SetCursorInComposition(numCharDisplay);
        break;

    default:
        if (dwSize == 0)
        {
            pBase->SetWideCursor(false);
        }
        else if (!ReplaceChar)
        {
            pBase->SetWideCursor(true);
            pBase->SetCompositionPosition();
            pBase->SetCompositionText(lpstr);
            int nCursorPos = (int)ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);
            pBase->SetCursorInComposition((UPInt)nCursorPos);
        }
        break;
    }
    SF_FREE(lpstr);
}

void GFxIMEXP::CustomProcessing(UPInt message, WPARAM wParam, LPARAM lParam)
{
    SF_UNUSED(message);
    if (wParam == 0x51) // backspace during composition
    {
        switch (IMETag)
        {
        case GFxIME_Ch_Simp_MSPinyin_3_0:
        case GFxIME_Ch_Trad_NewChangJie:
        case GFxIME_Ch_Trad_NewPhonetic:
            if (pMovie && CandListPath.GetLength() > 0)
                pMovie->Invoke((CandListPath + ".removeInputWindow").ToCStr(), "");
            break;
        default:
            break;
        }
    }
    SF_UNUSED(lParam);
}

void GFxIMEXP::CloseIME()
{
    Cleanup();
}

void GFxIMEXP::RepositionCandidateList(UInt32 offset)
{
    if (!pMovie || CandListPath.GetLength() == 0)
        return;
    RectF viewRect, caretRect;
    pBase->GetMetrics(&viewRect, &caretRect, (int)offset);
    pMovie->Invoke((CandListPath + ".repositionCandidateList").ToCStr(), "%d %d",
                   (int)caretRect.x1, (int)caretRect.y1);
}

//--------------------------------------------------------------------
// Evaluation validation hook
//--------------------------------------------------------------------

void GFxIME_ValidateEvaluation()
{
    // Compatibility export of the original libgfx_ime. The hook exists for
    // evaluation builds; the IME layer performs no runtime validation here.
}

}}}
