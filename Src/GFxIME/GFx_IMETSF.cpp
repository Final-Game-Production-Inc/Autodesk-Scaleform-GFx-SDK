/**************************************************************************

Filename    :   GFx_IMETSF.cpp
Content     :   Implementation of the CTSF class - TSF (Text Services
                Framework) related functionality for the Vista IME.
Created     :   Nov 5, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// The IIDs declared with DEFINE_GUID in GFx_IMETSF.h are byte-identical
// aliases of the standard msctf IIDs; no initguid.h is required here.
#include <stdint.h>

#include "GFx_IMETSF.h"

#include "GFx_IMEWin32Impl.h"
#include "GFx_CandidateListBox.h"

namespace Scaleform {
namespace GFx {
namespace IME {

//--------------------------------------------------------------------
// CTSF
//--------------------------------------------------------------------

CTSF::CTSF(GFxIMEWin32Impl* pIME, HWND hwnd)
    : pIME((GFxIMEVista*)pIME),
      dwThreadMgrEventSinkCookie(TF_INVALID_COOKIE),
      dwUIElementSinkCookie(TF_INVALID_COOKIE),
      dwTextEditSinkCookie(TF_INVALID_COOKIE),
      pThreadMgr(NULL),
      pContextOwnerCompositionSinkContext(NULL),
      ptid(TF_CLIENTID_NULL),
      cRef(1),
      pTimEx(NULL),
      pDisplayAttrMgr(NULL),
      CandidateListId((DWORD)-1),
      ReadingWindowId((DWORD)-1),
      NumCandidates(0),
      NeedRepositioning(false),
      ReadingWindowFlag(false),
      CandidateWindowFlag(false),
      CurrentPageStart(0),
      dwCandListId((DWORD)-1),
      PrevX(0),
      PrevY(0),
      hWnd(hwnd)
{
    m_str[0]  = 0;
    m_wstr[0] = 0;
}

CTSF::~CTSF()
{
}

STDMETHODIMP CTSF::QueryInterface(REFIID riid, void** ppvObj)
{
    if (!ppvObj)
        return E_INVALIDARG;
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    if (IsEqualIID(riid, IID_ITfUIElementSink_GFx))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CTSF::AddRef(void)
{
    return InterlockedIncrement(&cRef);
}

STDMETHODIMP_(ULONG) CTSF::Release(void)
{
    LONG nRef = InterlockedDecrement(&cRef);
    if (nRef == 0)
        delete this;
    return nRef;
}

//--------------------------------------------------------------------
// ITfThreadMgrEventSink
//--------------------------------------------------------------------

STDMETHODIMP CTSF::OnInitDocumentMgr(ITfDocumentMgr* pDocMgr)
{
    SF_UNUSED(pDocMgr);
    return S_OK;
}

STDMETHODIMP CTSF::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr)
{
    SF_UNUSED(pDocMgr);
    return S_OK;
}

STDMETHODIMP CTSF::OnSetFocus(ITfDocumentMgr* pDocMgrFocus, ITfDocumentMgr* pDocMgrPrevFocus)
{
    SF_UNUSED2(pDocMgrFocus, pDocMgrPrevFocus);
    return S_OK;
}

STDMETHODIMP CTSF::OnPushContext(ITfContext* pContext)
{
    SF_UNUSED(pContext);
    return S_OK;
}

STDMETHODIMP CTSF::OnPopContext(ITfContext* pContext)
{
    SF_UNUSED(pContext);
    return S_OK;
}

//--------------------------------------------------------------------
// ITfUIElementSink
//--------------------------------------------------------------------

STDMETHODIMP CTSF::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
    if (pbShow)
        *pbShow = TRUE;

    if (!pTimEx)
        return S_OK;

    ITfUIElementMgr* pUIElementMgr = NULL;
    ITfUIElement* pUIElement = NULL;
    ITfCandidateListUIElementBehavior* pCandBehavior = NULL;

    if (SUCCEEDED(pTimEx->QueryInterface(IID_ITfUIElementMgr_GFx, (void**)&pUIElementMgr)))
    {
        if (SUCCEEDED(pUIElementMgr->GetUIElement(dwUIElementId, &pUIElement)))
        {
            if (pUIElement && SUCCEEDED(pUIElement->QueryInterface(
                    IID_ITfCandidateListUIElementBehavior_GFx, (void**)&pCandBehavior)))
            {
                pIME->DrawCandidateList = true;
                pCandBehavior->Release();
            }
            pUIElement->Release();
        }
        pUIElementMgr->Release();
    }
    return S_OK;
}

STDMETHODIMP CTSF::UpdateUIElement(DWORD dwUIElementId)
{
    if (!pTimEx)
        return E_FAIL;

    ITfUIElementMgr* pUIElementMgr = NULL;
    ITfUIElement* pUIElement = NULL;
    ITfCandidateListUIElementBehavior* pCandBehavior = NULL;
    ITfReadingInformationUIElement* pReadingInfo = NULL;
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(pTimEx->QueryInterface(IID_ITfUIElementMgr_GFx, (void**)&pUIElementMgr)))
    {
        if (SUCCEEDED(pUIElementMgr->GetUIElement(dwUIElementId, &pUIElement)) && pUIElement)
        {
            if (SUCCEEDED(pUIElement->QueryInterface(
                    IID_ITfCandidateListUIElementBehavior_GFx, (void**)&pCandBehavior)))
            {
                CandidateListId = dwUIElementId;
                CandidateWindowFlag = true;

                UINT page = 0, selection = 0, count = 0;
                pCandBehavior->GetCurrentPage(&page);
                pCandBehavior->GetSelection(&selection);
                pCandBehavior->GetCount(&count);
                UINT pages[64];
                UINT pageCnt = 0;
                pCandBehavior->GetPageIndex(pages, 64, &pageCnt);
                CurrentPageStart = (page < pageCnt) ? pages[page] : 0;

                if (pIME->ListBox)
                {
                    pIME->ListBox->RemoveAllListItems();
                    // Note: the modern msctf.h no longer exposes
                    // IEnumTfCandidates, so only the currently selected
                    // candidate can be retrieved through the UI element.
                    BSTR bstr = NULL;
                    NumCandidates = count;
                    if (SUCCEEDED(pCandBehavior->GetString(selection, &bstr)) && bstr)
                    {
                        char itemBuf[512];
                        int nLen = WideCharToMultiByte(CP_UTF8, 0, bstr, -1,
                                                       itemBuf, sizeof(itemBuf), NULL, NULL);
                        pIME->ListBox->AddListItem(new CandidateListItem(
                            nLen > 0 ? itemBuf : (char*)""));
                        SysFreeString(bstr);
                    }
                    pIME->ListBox->UIRefreshView();
                    UINT selInPage = (selection >= CurrentPageStart) ?
                                     (selection - CurrentPageStart) : 0;
                    pIME->ListBox->SetSelectedItemIndex((int)selInPage);
                }
                hr = S_OK;
            }
            else if (SUCCEEDED(pUIElement->QueryInterface(
                    IID_ITfReadingInformationUIElement, (void**)&pReadingInfo)))
            {
                ReadingWindowId = dwUIElementId;
                ReadingWindowFlag = true;

                BSTR bstr = NULL;
                if (SUCCEEDED(pReadingInfo->GetString(&bstr)) && bstr)
                {
                    pIME->DisplayReadingWindow(bstr);
                    SysFreeString(bstr);
                }
                hr = S_OK;
            }
            pUIElement->Release();
        }
        pUIElementMgr->Release();
    }

    if (pCandBehavior) pCandBehavior->Release();
    if (pReadingInfo) pReadingInfo->Release();

    return hr;
}

STDMETHODIMP CTSF::EndUIElement(DWORD dwUIElementId)
{
    if (CandidateWindowFlag)
    {
        if (dwUIElementId == CandidateListId)
        {
            CandidateWindowFlag = false;
            if (pIME->pMovie && pIME->CandListPath.GetLength() > 0)
                pIME->pMovie->Invoke((pIME->CandListPath + ".removeList").ToCStr(), "");
            if (pIME->ListBox)
                pIME->ListBox->RemoveAllListItems();
        }
        else if (dwUIElementId == ReadingWindowId)
        {
            ReadingWindowFlag = false;
            if (pIME->pMovie && pIME->CandListPath.GetLength() > 0)
                pIME->pMovie->Invoke((pIME->CandListPath + ".removeInputWindow").ToCStr(), "");
        }
    }

    if (pIME->CandWndRefCount > 0)
        pIME->CandWndRefCount--;

    if (pIME->CandWndRefCount == 0 && pIME->NeedToDisableIME)
        pIME->OnEnableIME(false);

    return S_OK;
}

//--------------------------------------------------------------------
// ITfContextOwnerCompositionSink (declared but not wired into the
// base interface list; kept for API completeness).
//--------------------------------------------------------------------

STDMETHODIMP CTSF::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
    SF_UNUSED(pComposition);
    if (pfOk)
        *pfOk = TRUE;
    return S_OK;
}

STDMETHODIMP CTSF::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
    SF_UNUSED2(pComposition, pRangeNew);
    return S_OK;
}

STDMETHODIMP CTSF::OnEndComposition(ITfCompositionView* pComposition)
{
    SF_UNUSED(pComposition);
    if (pContextOwnerCompositionSinkContext)
    {
        pContextOwnerCompositionSinkContext->Release();
        pContextOwnerCompositionSinkContext = NULL;
    }
    return S_OK;
}

//--------------------------------------------------------------------
// TSF initialization
//--------------------------------------------------------------------

BOOL CTSF::Init()
{
    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITfThreadMgr, (void**)&pThreadMgr);
    if (FAILED(hr) || !pThreadMgr)
        return FALSE;

    hr = pThreadMgr->QueryInterface(IID_ITfThreadMgrEx_GFx, (void**)&pTimEx);
    if (FAILED(hr) || !pTimEx)
        return FALSE;

    hr = pTimEx->ActivateEx(&ptid, TF_TMAE_UIELEMENTENABLEDONLY);
    if (FAILED(hr))
        return FALSE;

    if (!InitThreadMgrSink())
        return FALSE;

    if (!InitUIElementSink())
        return FALSE;

    return TRUE;
}

BOOL CTSF::InitThreadMgrSink()
{
    ITfSource* pSource = NULL;
    HRESULT hr = pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource);
    if (FAILED(hr) || !pSource)
        return FALSE;

    hr = pSource->AdviseSink(IID_ITfThreadMgrEventSink,
                             (ITfThreadMgrEventSink*)this,
                             &dwThreadMgrEventSinkCookie);
    pSource->Release();
    return SUCCEEDED(hr) ? TRUE : FALSE;
}

BOOL CTSF::InitUIElementSink()
{
    ITfSource* pSource = NULL;
    HRESULT hr = pTimEx->QueryInterface(IID_ITfSource, (void**)&pSource);
    if (FAILED(hr) || !pSource)
        return FALSE;

    hr = pSource->AdviseSink(IID_ITfUIElementSink_GFx,
                             (ITfUIElementSink*)this,
                             &dwUIElementSinkCookie);
    pSource->Release();
    return SUCCEEDED(hr) ? TRUE : FALSE;
}

BOOL CTSF::InitTextEditSink()
{
    // The text edit sink is not used by the current composition model;
    // composition sink handling is routed through ITfUIElementSink.
    return FALSE;
}

void CTSF::UninitThreadMgrSink()
{
    ITfSource* pSource = NULL;
    if (dwThreadMgrEventSinkCookie != TF_INVALID_COOKIE && pThreadMgr)
    {
        if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource)) && pSource)
        {
            pSource->UnadviseSink(dwThreadMgrEventSinkCookie);
            pSource->Release();
        }
        dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    }
}

void CTSF::UnInitUIElementSink()
{
    ITfSource* pSource = NULL;
    if (dwUIElementSinkCookie != TF_INVALID_COOKIE && pTimEx)
    {
        if (SUCCEEDED(pTimEx->QueryInterface(IID_ITfSource, (void**)&pSource)) && pSource)
        {
            pSource->UnadviseSink(dwUIElementSinkCookie);
            pSource->Release();
        }
        dwUIElementSinkCookie = TF_INVALID_COOKIE;
    }
}

void CTSF::ReleaseInterfaces()
{
    if (pContextOwnerCompositionSinkContext)
    {
        pContextOwnerCompositionSinkContext->Release();
        pContextOwnerCompositionSinkContext = NULL;
    }
    if (pTimEx)
    {
        pTimEx->Deactivate();
        pTimEx->Release();
        pTimEx = NULL;
    }
    if (pThreadMgr)
    {
        pThreadMgr->Release();
        pThreadMgr = NULL;
    }
    if (pDisplayAttrMgr)
    {
        pDisplayAttrMgr->Release();
        pDisplayAttrMgr = NULL;
    }
}

//--------------------------------------------------------------------
// Candidate list selection
//--------------------------------------------------------------------

HRESULT CTSF::SelectAndClose(UInt32 index)
{
    if (!pTimEx)
        return E_FAIL;

    ITfUIElementMgr* pUIElementMgr = NULL;
    ITfUIElement* pUIElement = NULL;
    ITfCandidateListUIElementBehavior* pCandBehavior = NULL;
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(pTimEx->QueryInterface(IID_ITfUIElementMgr_GFx, (void**)&pUIElementMgr)))
    {
        if (SUCCEEDED(pUIElementMgr->GetUIElement(CandidateListId, &pUIElement)) && pUIElement)
        {
            if (SUCCEEDED(pUIElement->QueryInterface(
                    IID_ITfCandidateListUIElementBehavior_GFx, (void**)&pCandBehavior)))
            {
                hr = pCandBehavior->SetSelection((UINT)(CurrentPageStart + index));
                if (SUCCEEDED(hr))
                    pUIElementMgr->EndUIElement(CandidateListId);
            }
            pUIElement->Release();
        }
        pUIElementMgr->Release();
    }

    if (pCandBehavior)
        pCandBehavior->Release();

    return hr;
}

bool CTSF::AbortCandidateList()
{
    if (!pTimEx)
        return false;

    ITfUIElementMgr* pUIElementMgr = NULL;
    ITfUIElement* pUIElement = NULL;
    ITfCandidateListUIElementBehavior* pCandBehavior = NULL;
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(pTimEx->QueryInterface(IID_ITfUIElementMgr_GFx, (void**)&pUIElementMgr)))
    {
        if (SUCCEEDED(pUIElementMgr->GetUIElement(CandidateListId, &pUIElement)) && pUIElement)
        {
            if (SUCCEEDED(pUIElement->QueryInterface(
                    IID_ITfCandidateListUIElementBehavior_GFx, (void**)&pCandBehavior)))
            {
                if (pCandBehavior->Finalize() == S_OK)
                    hr = S_OK;
            }
            pUIElement->Release();
        }
        pUIElementMgr->Release();
    }

    if (pCandBehavior)
        pCandBehavior->Release();

    return SUCCEEDED(hr);
}

//--------------------------------------------------------------------
// Display attribute helpers
//--------------------------------------------------------------------

HRESULT CTSF::GetDispAttrFromGUID(GUID* pguid, TF_DISPLAYATTRIBUTE* pdispAttr)
{
    if (!pDisplayAttrMgr)
    {
        HRESULT hr = CoCreateInstance(CLSID_TF_DisplayAttributeMgr, NULL, CLSCTX_INPROC_SERVER,
                                      IID_ITfDisplayAttributeMgr, (void**)&pDisplayAttrMgr);
        if (FAILED(hr) || !pDisplayAttrMgr)
            return E_FAIL;
    }

    ITfDisplayAttributeInfo* pDispAttrInfo = NULL;
    HRESULT hr = pDisplayAttrMgr->GetDisplayAttributeInfo(*pguid, &pDispAttrInfo, NULL);
    if (FAILED(hr) || !pDispAttrInfo)
        return E_FAIL;

    hr = pDispAttrInfo->GetAttributeInfo(pdispAttr);
    pDispAttrInfo->Release();
    return hr;
}

HRESULT CTSF::GetDispAttrFromRange(ITfContext* pContext, ITfRange* pRange,
                                   TfEditCookie ec, TF_DISPLAYATTRIBUTE* pDispAttr)
{
    ITfProperty* pDisplayAttribute = NULL;
    HRESULT hr = pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttribute);
    if (FAILED(hr) || !pDisplayAttribute)
        return E_FAIL;

    VARIANT var;
    VariantInit(&var);
    hr = pDisplayAttribute->GetValue(ec, pRange, &var);
    if (FAILED(hr) || var.vt != VT_UNKNOWN || !var.punkVal)
    {
        VariantClear(&var);
        pDisplayAttribute->Release();
        return E_FAIL;
    }

    ITfRange* pRangeAttr = NULL;
    hr = var.punkVal->QueryInterface(IID_ITfRange, (void**)&pRangeAttr);
    if (SUCCEEDED(hr) && pRangeAttr)
    {
        TfGuidAtom guidAtom = 0;
        hr = pRangeAttr->GetText(ec, 0, (wchar_t*)&guidAtom, 1, NULL);
        if (SUCCEEDED(hr))
        {
            ITfCategoryMgr* pCategoryMgr = NULL;
            hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITfCategoryMgr, (void**)&pCategoryMgr);
            if (SUCCEEDED(hr) && pCategoryMgr)
            {
                GUID guid;
                hr = pCategoryMgr->GetGUID(guidAtom, &guid);
                if (SUCCEEDED(hr))
                    hr = GetDispAttrFromGUID(&guid, pDispAttr);
                pCategoryMgr->Release();
            }
        }
        pRangeAttr->Release();
    }

    VariantClear(&var);
    pDisplayAttribute->Release();
    return hr;
}

}}}
