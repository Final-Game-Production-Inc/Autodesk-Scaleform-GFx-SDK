/**************************************************************************

Filename    :   GFx_IMETSF.h
Content     :   Declaration of CTSF class- implementation of TSF related 
                functionality for IME on windows vista 
Created     :   Nov 5, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_TSF_H
#define INC_TSF_H

#include <windows.h>
#include <stdio.h>
#include <olectl.h>
#include <msctf.h>

// GFx includes
#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
//#include "GFx_RenderConfig.h"


namespace Scaleform {
	namespace GFx {
		namespace IME{

#ifndef __ITfUIElementSink_FWD_DEFINED__
#define TF_TMAE_UIELEMENTENABLEDONLY 0x00000004
#include "GFx_TSFUIElementDef.h"
#endif

DEFINE_GUID(IID_ITfUIElementSink_GFx, 
            0xea1ea136, 0x19df, 0x11d7, 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c);

// "3e90ade3-7594-4cb0-bb58-69628f5f458c" 
DEFINE_GUID(IID_ITfThreadMgrEx_GFx,
            0x3e90ade3, 0x7594, 0x4cb0, 0xbb, 0x58, 0x69, 0x62, 0x8f, 0x5f, 0x45, 0x8c);

// 85fad185-58ce-497a-9460-355366b64b9a
DEFINE_GUID(IID_ITfCandidateListUIElementBehavior_GFx, 
            0x85fad185, 0x58ce, 0x497a, 0x94, 0x60, 0x35, 0x53, 0x66, 0xb6, 0x4b, 0x9a);

// ea1ea135-19df-11d7-a6d2-00065b84435c 
DEFINE_GUID(IID_ITfUIElementMgr_GFx,
            0xea1ea135, 0x19df, 0x11d7, 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c);

DEFINE_GUID(IID_ITfInputProcessorProfileMgr_GFx,
            0x71c6e74c, 0x0f28, 0x11d8, 0xa8, 0x2a, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c);


// Forward Declaration
class GFxIMEVista;
class GFxIMEWin32Impl;

class CTSF : public ITfThreadMgrEventSink, 
             public ITfUIElementSink, 
		//	 public ITfContextOwnerCompositionSink,
             public NewOverrideBase<StatIME_Mem>
             
             
{
public:
    CTSF(GFxIMEWin32Impl* pIME, HWND hwnd);
    ~CTSF();

    // IUnknown
    STDMETHODIMP            QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

    // ITfThreadMgrEventSink
    STDMETHODIMP            OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP            OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP            OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP            OnPushContext(ITfContext *pContext);
    STDMETHODIMP            OnPopContext(ITfContext *pContext);

    // ITfUIElementSink
    STDMETHODIMP            BeginUIElement(DWORD dwUIElementId, BOOL* pbShow);
    STDMETHODIMP            UpdateUIElement(DWORD dwUIElementId);
    STDMETHODIMP            EndUIElement(DWORD dwUIElementId);
    
	// ITfContextOwnerCompositionSink
    STDMETHODIMP			OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk);
	STDMETHODIMP			OnUpdateComposition(ITfCompositionView *pComposition,ITfRange *pRangeNew);
	STDMETHODIMP			OnEndComposition(ITfCompositionView *pComposition);
    
	// ITfTextEditSink
//	STDMETHODIMP			OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);


    BOOL                    Init();
    BOOL                    InitThreadMgrSink();
    BOOL                    InitUIElementSink();
	BOOL					InitTextEditSink();
    void                    UninitThreadMgrSink();
    void                    UnInitUIElementSink();
    void                    ReleaseInterfaces();
    HRESULT                 SelectAndClose(UInt32 index);
    UInt32                    GetNumberOfCandidates() { return NumCandidates; }; 
    bool                    GetCandidateWindowFlag(){ return CandidateWindowFlag; };
	bool					AbortCandidateList();
private:

	HRESULT					GetDispAttrFromRange( ITfContext *pContext, ITfRange *pRange, TfEditCookie ec, TF_DISPLAYATTRIBUTE *pDispAttr);
	HRESULT					GetDispAttrFromGUID(GUID* pguid, TF_DISPLAYATTRIBUTE *pdispAttr);
	
    // Microsoft TSF related variables
    GFxIMEVista*			pIME;
    DWORD                   dwThreadMgrEventSinkCookie;
    DWORD                   dwUIElementSinkCookie;
	DWORD					dwTextEditSinkCookie;
    ITfThreadMgr            *pThreadMgr;
    ITfContext              *pContextOwnerCompositionSinkContext;
    TfClientId              ptid;
    LONG                    cRef;
    ITfThreadMgrEx          *pTimEx;
    ITfDisplayAttributeMgr  *pDisplayAttrMgr;

    // Candidate List and Reading Window state variables

    DWORD                   CandidateListId;
    DWORD                   ReadingWindowId;
    UInt32                    NumCandidates;
    bool                    NeedRepositioning;
    bool                    ReadingWindowFlag;
    bool                    CandidateWindowFlag;
    UInt32                    CurrentPageStart;
	UInt32					dwCandListId;

public:
    // To store the coordinates of the pop ups (candidate window and reading window)
    int                     PrevX;
    int                     PrevY;
    HWND                    hWnd;
	char					m_str[50];
	wchar_t					m_wstr[50];
    
};

}}}

#endif // INC_TSF_H
