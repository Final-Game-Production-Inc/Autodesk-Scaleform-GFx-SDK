/**************************************************************************

Filename    :   GFx_IMETSF.cpp
Content     :   Text Services Framework related implementation.
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

// The correct way to define GUIDs. You put DEFINE_GUID
//macro in some header file. All sources that need the definition can
//include the header. In exactly one .cpp file, you include initguid.h,
//then include the header with GUID definitions after that. 

#include <initguid.h>
//#include "GFxTSFUIElementDef.h"
#include "GFx_IMETSF.h"
#include "GFx_IMEWin32Impl.h"


namespace Scaleform {
	namespace GFx {
		namespace IME{


#define TF_CLUIE_COUNT            0x00000002 
#define TF_CLUIE_SELECTION        0x00000004 
#define TF_CLUIE_STRING           0x00000008 
#define TF_CLUIE_PAGEINDEX        0x00000010 
#define TF_CLUIE_CURRENTPAGE      0x00000020


#define TF_PROFILETYPE_INPUTPROCESSOR         0x0001
#define TF_PROFILETYPE_KEYBOARDLAYOUT         0x0002

#define TF_CONVERSIONMODE_ALPHANUMERIC        0x0000
#define TF_CONVERSIONMODE_NATIVE              0x0001
#define TF_CONVERSIONMODE_KATAKANA            0x0002
#define TF_CONVERSIONMODE_FULLSHAPE           0x0008
#define TF_CONVERSIONMODE_ROMAN               0x0010
#define TF_CONVERSIONMODE_CHARCODE            0x0020
#define TF_CONVERSIONMODE_SOFTKEYBOARD        0x0080
#define TF_CONVERSIONMODE_NOCONVERSION        0x0100
#define TF_CONVERSIONMODE_EUDC                0x0200
#define TF_CONVERSIONMODE_SYMBOL              0x0400
#define TF_CONVERSIONMODE_FIXED               0x0800
#define TF_IPPMF_FORPROCESS                     0x10000000

EXTERN_C const GUID GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION_GFX;

// Constructor

CTSF::CTSF(GFxIMEWin32Impl* _pIME, HWND _hwnd)
{
    pIME            = (GFxIMEVista*)_pIME;
    hWnd            = _hwnd;
    pThreadMgr      = NULL;
    pTimEx          = NULL;
    cRef            = 1;
    dwThreadMgrEventSinkCookie  = TF_INVALID_COOKIE;
    dwUIElementSinkCookie       = TF_INVALID_COOKIE;
	dwTextEditSinkCookie		= TF_INVALID_COOKIE;
    
}

// Destructor

CTSF::~CTSF()
{
}


//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CTSF::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }

    else if (IsEqualIID(riid, IID_ITfUIElementSink_GFx))
    {
        *ppvObj = (ITfUIElementSink *)this;
    }

	else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
    {
        *ppvObj = (ITfContextOwnerCompositionSink *)this;
    }

	else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CTSF::AddRef()
{
    return ++cRef;
}


//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CTSF::Release()
{
    
    LONG cr = --cRef;


    if (cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// Init
//
//----------------------------------------------------------------------------

BOOL CTSF::Init()
{
   

    ITfThreadMgr* ptim = 0;
    HRESULT hr;
   
    hr = CoCreateInstance(CLSID_TF_ThreadMgr, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_ITfThreadMgr, 
                          (void**)&ptim);

    if (FAILED(hr))
    {
		// According to doc, if CoCreateInstance fails, ptim will be null. Check anyways..
		if (ptim)
		{
			ptim->Release();
			ptim = 0;
		}
        return FALSE;
    }

    pThreadMgr = ptim;

	
    hr = ptim->QueryInterface(IID_ITfThreadMgrEx_GFx, (void **)&pTimEx);
    if (FAILED(hr))
    {
		if (ptim)
		{
			ptim->Release();
		}
     //   pTimEx->Release();
     //   printf("Failed to load the TSF thread manager\n");
        return FALSE;
    }
    hr = pTimEx->ActivateEx(&ptid, TF_TMAE_UIELEMENTENABLEDONLY);
    if (FAILED(hr))
    {
    //    printf("Failed to activate UILess mode\n");
    //    return FALSE;
    }

    if (InitThreadMgrSink() != TRUE)
    {
    //    printf("Failed to initialize thread manager sink\n");
        return FALSE;
    }
    if (InitUIElementSink() != TRUE)
    {
     //   printf("Failed to initialize UI Element sink\n");
        return FALSE;
    }

	if (InitTextEditSink() != TRUE)
    {
     //   printf("Failed to initialize UI Element sink\n");
        return FALSE;
    }

	pIME->CandWndRefCount = 0;
  
    // printf("TSF initialized successfully!\n");
    return TRUE;
}

void CTSF::ReleaseInterfaces()
{
	if (pThreadMgr)
		pThreadMgr->Release();
    if (pTimEx)
	{
		pTimEx->Deactivate(); // Must deactivate before releasing
		pTimEx->Release();
	}
}


BOOL CTSF::InitUIElementSink()
{
    ITfSource *pSource = NULL;
    BOOL fRet = FALSE;

    if (FAILED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    fRet = FALSE;

    if (FAILED(pSource->AdviseSink(IID_ITfUIElementSink_GFx, (ITfUIElementSink *)this, &dwUIElementSinkCookie)))
    {
        // make sure we don't try to Unadvise _dwThreadMgrEventSinkCookie later
        dwUIElementSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    if(pSource)
        pSource->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// InitThreadMgrSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CTSF::InitThreadMgrSink()
{
    ITfSource *pSource;
    BOOL fRet = FALSE;

    if (FAILED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &dwThreadMgrEventSinkCookie)))
    {
        // make sure we don't try to Unadvise _dwThreadMgrEventSinkCookie later
        dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    if(pSource != NULL)
        pSource->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// UninitThreadMgrSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CTSF::UnInitUIElementSink()
{
    ITfSource *pSource;

    if (dwUIElementSinkCookie == TF_INVALID_COOKIE)
        return; // never Advised

    if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        if (pSource->UnadviseSink(dwUIElementSinkCookie) == CONNECT_E_NOCONNECTION)
            Release();
        pSource->Release();
    }

    dwUIElementSinkCookie = TF_INVALID_COOKIE;
}

//+---------------------------------------------------------------------------
//
// UninitThreadMgrSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CTSF::UninitThreadMgrSink()
{
    ITfSource *pSource;

    if (dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
        return; // never Advised

    if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        // Sometimes the UnadviseSink function returns CONNECT_E_NOCONNECTION.
        // when this happens, release will not be called leading to memory leaks.
        // don't know why it returns CONNECT_E_NOCONNECTION. Currently this only happens
        // with the CompartmentSink. So if UnadviseSink fails, call release manually.
        if( pSource->UnadviseSink(dwThreadMgrEventSinkCookie) == CONNECT_E_NOCONNECTION)
            Release();
        pSource->Release();
    }

    dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}

STDAPI CTSF::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
    SF_UNUSED(dwUIElementId);
	
 	pIME->NeedToDisableIME		= false;
	memset(m_str, 0, 50);
    *pbShow = FALSE;
	if (pIME->IMETag == GFxIME_NotSupported || pIME->IMETag == GFxIME_NotSet)
		return S_OK;

	bool hRes = pIME->IsTextFieldFocussed(pIME->pMovie);
	if (!hRes)
	{
		return S_OK;
	}
	pIME->CandWndRefCount++;

    if (pIME->CandListPath.GetLength() > 0)
    {
		Value result;
		Array<Value> valArray;
		valArray.PushBack(Value(10.0));
		valArray.PushBack(Value(1.0));
		pIME->Invoke("RemoveList", &result, 0,0);
		pIME->Invoke("CreateList", &result, valArray.GetDataPtr(), 2);
    //    pIME->pMovie->Invoke((pIME->CandListPath + "RemoveList").ToCStr(), "");
    //    pIME->pMovie->Invoke((pIME->CandListPath + "CreateList").ToCStr(), "%d %d", 10, 1);
    }
    
    Ptr<IMEManagerBase> pimeMgr = pIME->pMovie->GetIMEManager();
    SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pIME->pMovie));

    CandidateWindowFlag = false;
    ReadingWindowFlag = false;
      
    // Get Language Bar Info
    ITfLangBarItemMgr* pLangBarMgr;
    HRESULT hr;

    hr = pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarMgr);
    if (SUCCEEDED(hr) && pLangBarMgr)
    {
        IEnumTfLangBarItems* pEnumTfLangBarItem;
        hr = pLangBarMgr->EnumItems(&pEnumTfLangBarItem);
        if (hr == S_OK)
        {
            ITfLangBarItem* pItem;
             while (pEnumTfLangBarItem->Next(1, &pItem, NULL) == S_OK)
             {
				TF_LANGBARITEMINFO info;
                pItem->GetInfo(&info);
                pItem->Release();
             }
             pEnumTfLangBarItem->Release();
        }
        pLangBarMgr->Release();
    }
          
    return S_OK;
}

HRESULT CTSF::SelectAndClose(UInt32 index)
{
    ITfUIElementMgr*    pUIElementMgr;
    ITfUIElement*       pElement;

    ITfCandidateListUIElementBehavior* pCandListBehaviour;
     if (FAILED(pThreadMgr->QueryInterface(IID_ITfUIElementMgr_GFx, (void **)&pUIElementMgr)))
         return FALSE;

     if ( FAILED(pUIElementMgr->GetUIElement(CandidateListId, &pElement)))
     {
		 if (pUIElementMgr)
			pUIElementMgr->Release();
         return FALSE;
     }
     
     if ( FAILED(pElement->QueryInterface(IID_ITfCandidateListUIElementBehavior_GFx, (void **)&pCandListBehaviour)))
     {
		 if (pElement)
			pElement->Release();
		 if (pUIElementMgr)
			pUIElementMgr->Release();
         return FALSE;
     }
     pCandListBehaviour->SetSelection(CurrentPageStart + index);
  //   pCandListBehaviour->Finalize();
	 if (pElement)
		pElement->Release();
	 if (pUIElementMgr)
		pUIElementMgr->Release();
	 if (pCandListBehaviour)
		pCandListBehaviour->Release();
     return S_OK;
}

STDAPI CTSF::UpdateUIElement(DWORD dwUIElementId)
{
    ITfUIElementMgr*    pUIElementMgr;
    ITfUIElement*       pElement;
    wchar_t*            pReadingStr;
    UInt32                numCandidates;
    UInt32                globalNumCandidates = 0;
	GUID				pguid;
    BSTR				pDesc;

	ITfReadingInformationUIElement  *preading = NULL;
    ITfCandidateListUIElement		*pcandidate = NULL;

	if ( pIME->IMETag == GFxIME_NotSupported || pIME->IMETag == GFxIME_NotSet)
		return FALSE;

	bool hRes = pIME->IsTextFieldFocussed(pIME->pMovie);
	if (!hRes)
	{
		return S_OK;
	}

    if ( FAILED(pThreadMgr->QueryInterface(IID_ITfUIElementMgr_GFx, (void **)&pUIElementMgr)))
        return FALSE;
   
    pUIElementMgr->GetUIElement(dwUIElementId, &pElement);
    
	if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
											(void **)&pcandidate)))
    {
        pcandidate->Release();
    }

    pElement->GetGUID(&pguid);
    pElement->GetDescription(&pDesc);
    
       
//	if (!wcscmp(pDesc, L"Reading") || !wcscmp(pDesc, L"CCIME_Read"))
	if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfReadingInformationUIElement),
                                      (void **)&preading)))
    
    {
        preading->GetString(&pReadingStr);
        ReadingWindowId = dwUIElementId;
		// This is needed since for some IME's UpdateUIElement is called before the StartComposition message is received.
		// The composition string in the text field is initialized during processing of startcomposition. So if StartComposition
		// has not yet been received, we store the reading string and don't try to display it immediately, otherwise it may not
		// be positioned properly.

        if (pReadingStr != NULL){
			if(pIME->StartCompositionReceived == false)
				pIME->ReadinStringBuffer = pReadingStr;
			else
			{
				pIME->ReadinStringBuffer = pReadingStr;
				pIME->DisplayReadingWindow(pReadingStr);
			}
        }
        // This needs to be done since for chinese simplified IMEs if user types a combination that results 
        // in no candidates, candidate list logic below will never be executed and we'll be stuck with the previous 
        // candidate list. Hence, we need to remove the candidate list here itself. 
    //   pIME->pMovie->Invoke((pIME->CandListPath + "RemoveList").ToCStr(), "");
        
       if (pIME->pMovie)
       {
		   Value result;
		   pIME->Invoke("MakeInvisible", &result, 0, 0);
       }

       // If two readingwindow messages are received in a row without an intervening candidatewindow message,
       // remove the candidate list.
       if (ReadingWindowFlag == true)
        {
            if (pIME->pMovie) 
            {
				Value result;
				pIME->Invoke("RemoveList", &result, 0, 0);
            }            
            CandidateWindowFlag = false;
        }
        ReadingWindowFlag = true;
		preading->Release();
    }

//    if (!wcscmp(pDesc, L"Cand") || !wcscmp(pDesc, L"CandidateList") || !wcscmp(pDesc, L"ExternalCandidateUI"))
	if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
											(void **)&pcandidate)))
    {
	//	printf("Update Called: %d\n", dwUIElementId); 
        // Reset ReadingWindowFlag
		dwCandListId = dwUIElementId;
        ReadingWindowFlag = false;
        if (CandidateWindowFlag == false)
        {
			// Decide if the candidate list starts from 0 or 1. Default is 1. 
			int candListStartFrom1 = 1;
			if (pIME->IMETag == GFxIME_Ch_Trad_WuXia || pIME->IMETag == GFxIME_Ch_Trad_DaYi)
			{
				// For this IME, IMM reports the starting index of the candidate list incorrectly, so manually set it here.
				candListStartFrom1 = 0;
			}
 
			{ 
				Value result;
				Array<Value> valArray;
				valArray.PushBack(Value(10.0));
				valArray.PushBack(Value(double(candListStartFrom1)));
				pIME->Invoke("CreateList", &result, valArray.GetDataPtr(), 2);
                Ptr<IMEManagerBase> pimeMgr = pIME->pMovie->GetIMEManager();
                SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pIME->pMovie));
            //    pimeMgr->OnOpenCandidateList();
                CandidateWindowFlag = true;
            }
        }

        pcandidate->GetCount((UINT*)&numCandidates);
        if (numCandidates == 0) goto exit;
		
        UInt32 currentPageIndex   = 0;
        UInt32 currentPage        = 0;
        UInt32 currentSelection   = 0;
        UInt32 numPages           = 0;
        UInt32* pIndex            = 0;
        wchar_t* pstr           = 0;
        UInt32 numCandidatesOnPage = 0;

        CandidateListId = dwUIElementId;
       
  //     pIME->pMovie->Invoke((pIME->CandListPath + "RemoveList").ToCStr(), "");
       
       // For some strange reason, for Japanese, GetPageIndex leads to whacky results. The number of pages returned is usually
        // a lot more than expected.

       // Here we try to determine if the candidate list needs to be repositioned or not. If we are going through different rows of 
       // same candidate list, there is no need to reposition the list. If it's a different candidate list, then we should reposition.
       // We check different flags obtained from GetUpdatedFlags function to make this determination.
       ((ITfCandidateListUIElement*)pElement)->GetPageIndex(NULL, 0, (UINT*)&numPages);
        DWORD pFlags;
        ((ITfCandidateListUIElement*)pElement)->GetUpdatedFlags(&pFlags);
        
        bool flagCount      = (pFlags & TF_CLUIE_COUNT)         == 0? false: true;
        bool flagSelection  = (pFlags & TF_CLUIE_SELECTION)     == 0? false: true;
        bool flaString     = (pFlags & TF_CLUIE_STRING)        == 0? false: true;
        bool flagPageIndex  = (pFlags & TF_CLUIE_PAGEINDEX)     == 0? false: true;
        bool flagCurPage    = (pFlags & TF_CLUIE_CURRENTPAGE)   == 0? false: true;
        
        SF_UNUSED2(flagPageIndex, flagCount);
        if ((flagSelection || flagCurPage) && (!flaString))
        {
            NeedRepositioning = false;
        }
        else
        {
            NeedRepositioning = true;
        }


        if (numPages != 0)
        {
            pIndex = (UInt32*) SF_ALLOC(sizeof(UInt32)*(numPages+1), StatIME_Mem);
            // Get the starting index of every page in pIndex.
            pcandidate->GetPageIndex((UINT*)pIndex, numPages, (UINT*)&numPages);
        }
        if (numPages > 1)
        {
            numCandidatesOnPage = pIndex[1] - pIndex[0];
        }
        else
        {
            numCandidatesOnPage = numCandidates; //Only one page
        }
      
        // Get the page we are on currently
        pcandidate->GetCurrentPage((UINT*)&currentPageIndex);

        // What is the currently selected row?
        pcandidate->GetSelection((UINT*)&currentSelection);
        
        currentPage     = (int)currentPageIndex;
        UInt32 pageStart  = pIndex[currentPage];
        CurrentPageStart= pageStart;

        if ((pIME->IMETag & (UInt32)GFxIME_Jp_Flag) || (pIME->IMETag & (UInt32)GFxIME_Ch_Trad_Flag))
        {
            // Obtain the number of candidates on the current page. This has to be done
            // since numCandidatesOnPage doesn't give the correct number of candidates
            // on the last page. 

            if (numCandidates < numCandidatesOnPage)
                globalNumCandidates = numCandidates;
            else
            {
                // If we are on the last page. Calculate the number of rows on the last page. This information can't reliably
                // be obtained from pIndex.
                if ((currentPage+1)*numCandidatesOnPage > numCandidates)
                    globalNumCandidates = numCandidates%numCandidatesOnPage; // Last Page
                // If we are not on the last page
                else
                    globalNumCandidates = numCandidatesOnPage;
            }
        }

        else
        {
            if (currentPageIndex == numPages - 1) // we are on last page
                globalNumCandidates = numCandidates - pageStart;
            else
                globalNumCandidates = pIndex[currentPageIndex+1] - pIndex[currentPageIndex];
        }

		// For google pinyin, the page indicies are not reported correctly in pIndex. For Sogou pinyin IME, candidate list TSF notifications
		// are never sent. 

		if (pIME->IMETag & (UInt32)GFxIME_Ch_ThirdParty_Flag)
		{
			globalNumCandidates = numCandidates;
			pageStart			= 0;
		}

         if (pIndex)
            SF_FREE(pIndex);

		 if ((int)currentSelection >= 0) 
            pIME->ListBox->SetSelectedItemIndex(currentSelection - pageStart, true); // offset from beginning of current page

         int numNullStrings = 0;
/*
		int len = 0;
		wchar_t str_tmp1[50];
		 for (unsigned int i = 0; i < globalNumCandidates; i++)
         {
             // Just obtain the strings for the current page- not the entire list
			((ITfCandidateListUIElement*)pElement)->GetString(i+pageStart, &pstr);
			if (pstr != NULL)
            {
                
               len += wcslen(pstr);
			   if (len < 50)
			   {
				   wcscpy(str_tmp1 + len- wcslen(pstr), pstr);
			   }
			   else
				   break;
			}
		}
				
		if (!wcscmp(m_wstr, str_tmp1))
		{
			goto exit;
		}
		else
		{
			wcscpy(m_wstr, str_tmp1);
		}
			
*/
        pIME->ListBox->RemoveAllListItems();
        for (unsigned int i = 0; i < globalNumCandidates; i++)
         {
             // Just obtain the strings for the current page- not the entire list
            ((ITfCandidateListUIElement*)pElement)->GetString(i+pageStart, &pstr);
            if (pstr != NULL)
            {
                // Convert to UTF8
                int utf8Len = (int)UTF8Util::GetEncodeStringSize(pstr, -1);
                char* buf = (char*)SF_ALLOC(utf8Len+1, StatIME_Mem);
                UTF8Util::EncodeString(buf, pstr, -1);
                Ptr<CandidateListItem> pItem  = *new CandidateListItem(buf);
                pIME->ListBox->AddListItem(pItem, false); // Only copy as much data as necessary.
                SF_FREE(buf);
            }
            else
                numNullStrings++;
        }
       
         // This needs to be done since for chinese simplified pinyin IME, globalNumCandidates might not be correct since 
         // GetPageIndex returns incorrect results. Hence, we have to subtract the number of bad strings from the total 
         // number of strings on the page which might be incorrect.

        globalNumCandidates = globalNumCandidates - numNullStrings;
        NumCandidates = globalNumCandidates;
         
        pIME->ListBox->UIRefreshView();
		pIME->DrawCandidateList = true;
		// For this IME, all TSF notifications arrive after windows messages, so candidate list repositioning must be
		// done here. Note that candidate list visibility is set during repositioning (look at actionscript code), so 
		// repositioning is vital, otherwise list will not show up
		if (pIME->IMETag == GFxIME_Jp_2010)
		{
			pIME->RepositionCandidateList(-pIME->ClausePosition);
			// no need since list is already drawn
			pIME->DrawCandidateList = false;
		}
		
		pcandidate->Release();
    }
exit:
	if (pUIElementMgr)
		pUIElementMgr->Release();
    if (pElement)
		pElement->Release();
    return S_OK;
}


STDAPI CTSF::EndUIElement(DWORD dwUIElementId)
{

//	printf("EndUIElement Called: %d\n", dwUIElementId); 
	
	if (pIME->IMETag == GFxIME_NotSupported || pIME->IMETag == GFxIME_NotSet)
		return S_OK;

	bool hRes = pIME->IsTextFieldFocussed(pIME->pMovie);
	if (!hRes)
	{
		return S_OK;
	}

	pIME->CandWndRefCount--;

    if (dwUIElementId == CandidateListId)
    {
        if (pIME->pMovie)
		{
			Value result;
			pIME->Invoke("RemoveList", &result, 0, 0);
        //    pIME->pMovie->Invoke((pIME->CandListPath + "RemoveList").ToCStr(), 0);
		}
        pIME->ListBox->RemoveAllListItems();
        CandidateWindowFlag = false;
     }

     if (dwUIElementId == ReadingWindowId)
    {
        if (pIME->pMovie)
		{
			Value result;
			pIME->Invoke("RemoveInputWindow", &result, 0, 0);
		}
    
		pIME->ReadinStringBuffer = "";
    }

	 if (pIME->CandWndRefCount == 0 && pIME->NeedToDisableIME)
		 pIME->OnEnableIME(false);

 //    pIME->ClausePosition = 0;
    return S_OK;
}

bool CTSF::AbortCandidateList()
{
	ITfUIElementMgr*    pUIElementMgr	= 0;
	ITfUIElement*       pElement		= 0;
	ITfCandidateListUIElementBehavior* pcandListUIElementBehavior = 0;
	
	bool ret = FALSE;
	HRESULT hr = S_OK;
	
	if (!pThreadMgr) return ret;

	if (FAILED(pThreadMgr->QueryInterface(IID_ITfUIElementMgr_GFx, (void **)&pUIElementMgr)))
		goto exit;

	hr = pUIElementMgr->GetUIElement(dwCandListId, &pElement);
	if (SUCCEEDED(hr) && pElement)
	{
		if (FAILED(pElement->QueryInterface(IID_ITfCandidateListUIElementBehavior_GFx, (void **)&pcandListUIElementBehavior)))
			goto exit;

		if (pcandListUIElementBehavior)
		{
			pcandListUIElementBehavior->Abort();
			ret = true;
		}
	}
	
exit:
	if (pUIElementMgr)
		pUIElementMgr->Release();
	if (pElement)
		pElement->Release();
	if (pcandListUIElementBehavior)
		pcandListUIElementBehavior->Release();

	return ret;
}

BOOL CTSF::InitTextEditSink()
{
	return true;
}

// Below is an attempt to obtain the composition string and attribute information using TSF as opposed to using windows messages.
// it turns out however, that while the composition string data can be obtained fairly easily, getting the attributes is very hard
// Also, there is no separate notification for startcomposition and endcomposition, which makes it hard to infer when the composition 
// begins and ends. I also have not found a way yet to obtain the position of the cursor within the composition string. Therefore, I'm 
// commenting out this code and just relying on ime messages for the composition string info for now.

/*
STDAPI CTSF::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{

	SF_UNUSED3(pContext, ecReadOnly, pEditRecord);
	BOOL			pfChanged;
	GUID			prgProperties[2];
	ULONG			cProperties = 0;
	HRESULT			hr;
	ULONG			uFetched;
	prgProperties[0] = GUID_PROP_COMPOSING;

	IEnumTfRanges*	pEnum;
	wprintf(L"%s\n", L"****");

	ITfCategoryMgr  *pCategoryMgr;
    hr = CoCreateInstance(  CLSID_TF_CategoryMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfCategoryMgr, 
                            (LPVOID*)&pCategoryMgr);

	 //Create the display attribute manager. 
    ITfDisplayAttributeMgr  *pDispMgr;
    hr = CoCreateInstance(  CLSID_TF_DisplayAttributeMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfDisplayAttributeMgr, 
                            (LPVOID*)&pDispMgr);

	// Lets enumerate the properties contained in this context.

	ITfProperty				*pProp;
	ITfContextComposition	*pContextComposition;
	IEnumITfCompositionView	*pEnumComposition;
	ITfCompositionView		*pCompositionView;


    hr = pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);

	// get pointer to ITfContextComposition interface
	pContext->QueryInterface(IID_ITfContextComposition, (void**)&pContextComposition);

	// creates enumerator object for all compositions in context
	pContextComposition->EnumCompositions(&pEnumComposition);

	// obtain pointer to the composition view from the current 
	// position to the end
	while(pEnumComposition->Next(1, &pCompositionView, NULL) == S_OK)
	{
		ITfRange *pvRange;
		ULONG writeSize;
		WCHAR wStr[WCHAR_MAX];

		// Get range object and receive the text into buffer (wStr)
		pCompositionView->GetRange(&pvRange);
		pvRange->GetText(ecReadOnly, TF_TF_IGNOREEND, wStr, WCHAR_MAX, &writeSize);
		wStr[writeSize] = NULL;

		pvRange->Release();
		pCompositionView->Release();
	}
	pEnumComposition->Release();
	pContextComposition->Release();

	
	if(SUCCEEDED(hr))
	{

		TF_DISPLAYATTRIBUTE dispAttr;

		// Enumerate all the ranges that contain the property. 
		IEnumTfRanges   *pEnumRanges;
		hr = pProp->EnumRanges(ecReadOnly, &pEnumRanges, NULL);
		if(SUCCEEDED(hr))
		{
			ITfRange	*pRange = 0;
			while(pEnumRanges->Next(1, &pRange, &uFetched) == S_OK )
			{
				VARIANT var; 
				hr = pProp->GetValue(ecReadOnly, pRange, &var);
				if(S_OK == hr)  //Returns S_FALSE if the range is not completely covered by the property.  
				{
					if(VT_I4 == var.vt)
					{
						//The property is a guidatom. 
						GUID    guid;
						//Convert the guidatom into a GUID. 
						hr = pCategoryMgr->GetGUID((TfGuidAtom)var.lVal, &guid);
						if(SUCCEEDED(hr))
						{
							ITfDisplayAttributeInfo *pDispInfo;

							//Get the display attribute info object for this attribute. 
							hr = pDispMgr->GetDisplayAttributeInfo(guid, &pDispInfo, NULL);
							if(SUCCEEDED(hr))
							{
								//Get the display attribute info. 
								hr = pDispInfo->GetAttributeInfo(&dispAttr);

								switch (dispAttr.bAttr)
								{
								case TF_ATTR_INPUT:
									printf("input");
									break;

								case TF_ATTR_CONVERTED:
									printf("converted");
									break;								
								}

								pDispInfo->Release();
							}
						} // if (SUCCEEDED(hr)) for pCategoryMgr->GetGUID
					} //  if(VT_I4 == var.vt)
				} //  if(S_OK == hr)
			} //  while (pEnumRanges->Next)
			pEnumRanges->Release();
		} //  if (SUCCEEDED(hr))  for pEnum->EnumRanges
		pProp->Release();
	} // if (SUCCEEDED(hr))  hr = pContext->GetProperty

	hr = pContext->GetProperty(GUID_PROP_COMPOSING, &pProp);

	if (SUCCEEDED(hr))
	{
		// Enumerate all the ranges that contain the property. 
		IEnumTfRanges   *pEnumRanges;
		hr = pProp->EnumRanges(ecReadOnly, &pEnumRanges, NULL);
		if(SUCCEEDED(hr))
		{
			ITfRange	*pRange = 0;
			while(pEnumRanges->Next(1, &pRange, &uFetched) == S_OK )
			{
				VARIANT var; 
				hr = pProp->GetValue(ecReadOnly, pRange, &var);
				if(S_OK == hr)  //Returns S_FALSE if the range is not completely covered by the property.  
				{
		
				} //  if(S_OK == hr)
			} //  while (pEnumRanges->Next)
			pEnumRanges->Release();
		} //  if (SUCCEEDED(hr))  for pEnum->EnumRanges
		pProp->Release();
	} // if (SUCCEEDED(hr))  hr = pContext->GetProperty


	if (pEditRecord)
	{
		pEditRecord->GetSelectionStatus(&pfChanged);
		if (SUCCEEDED(hr = pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, (CONST GUID**)&prgProperties, cProperties, &pEnum)))
		{
			ITfRange	*pRange = 0;
			ULONG		pcFetched;
			while (pEnum->Next(1, &pRange, &pcFetched) == S_OK)
			{
				WCHAR *pwstr;
				WCHAR wstr[256 + 1];
				ULONG ulcch = ARRAYSIZE(wstr) - 1;
				ULONG ulcchTotal;
				if (pRange)
				{
					pRange->GetText(ecReadOnly, TF_TF_MOVESTART, wstr, ulcch, &ulcch);
			
				}
			}
		}
	}

	return S_OK;
}
*/
//+---------------------------------------------------------------------------
//
// OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CTSF::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    SF_UNUSED(pDocMgr);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CTSF::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    SF_UNUSED(pDocMgr);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CTSF::OnSetFocus(ITfDocumentMgr *pDocMgr, ITfDocumentMgr *pDocMgrPrevFocus)
{
    SF_UNUSED2(pDocMgr, pDocMgrPrevFocus);

/*
	ITfContext*		pContext = 0;
	HRESULT			hr;
	ITfSource*		pSource = 0;
	bool			fRet = FALSE;
	
	if (dwTextEditSinkCookie != TF_INVALID_COOKIE)
		return TRUE; // Already initialized.

	dwTextEditSinkCookie = TF_INVALID_COOKIE;

	hr = pDocMgr->GetBase(&pContext);

	if (FAILED(hr) || !pContext)
		goto Exit;

    if (FAILED(pContext->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &dwTextEditSinkCookie)))
    {
        // make sure we don't try to Unadvise _dwThreadMgrEventSinkCookie later
        dwTextEditSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    if(pSource != NULL)
        pSource->Release();

	if(pContext!= NULL)
        pContext->Release();
    return fRet;

//	hr = pDocMgr->CreateContext(ptid, 0, (ITfContextOwnerCompositionSink*)this, &pContext, &editCookie);
	
	*/

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPushContext
//
// Sink called by the framework when a context is pushed.

//----------------------------------------------------------------------------

STDAPI CTSF::OnPushContext(ITfContext *pContext)
{
    SF_UNUSED(pContext);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CTSF::OnPopContext(ITfContext *pContext)
{
    SF_UNUSED(pContext);
    return S_OK;
}

/*
STDAPI CTSF::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
{
	SF_UNUSED2(pComposition, pfOk);
	return S_OK;
}


STDAPI	CTSF::OnUpdateComposition(ITfCompositionView *pComposition,ITfRange *pRangeNew)
{
	SF_UNUSED2(pComposition, pRangeNew);
	return S_OK;
}

STDAPI	CTSF::OnEndComposition(ITfCompositionView *pComposition)
{
	SF_UNUSED(pComposition);
	return S_OK;
}

*/

HRESULT CTSF::GetDispAttrFromGUID(GUID* pguid, TF_DISPLAYATTRIBUTE *pdispAttr)
{

	HRESULT     hr;

	//Create the display attribute manager. 
	ITfDisplayAttributeMgr  *pdispMgr;
	ITfDisplayAttributeInfo *pdispInfo;

	hr = CoCreateInstance(  CLSID_TF_DisplayAttributeMgr,
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_ITfDisplayAttributeMgr, 
		(LPVOID*)&pdispMgr);
	
	if (FAILED(hr))
	{
		return hr;
	}

	// Get the display attribute info object for this attribute.

	hr = pdispMgr->GetDisplayAttributeInfo(*pguid, &pdispInfo, NULL);

	if(SUCCEEDED(hr))
	{
		//Get the display attribute info. 
		hr = pdispInfo->GetAttributeInfo(pdispAttr);
		pdispInfo->Release();
	}
	else
	{
		hr = E_FAIL;
	}

	pdispMgr->Release();

	return hr;

}


HRESULT CTSF::GetDispAttrFromRange(   ITfContext *pContext, ITfRange *pRange, TfEditCookie ec, TF_DISPLAYATTRIBUTE *pDispAttr)
{
    HRESULT     hr;
    
    //Create the category manager. 
    ITfCategoryMgr  *pCategoryMgr;
    hr = CoCreateInstance(  CLSID_TF_CategoryMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfCategoryMgr, 
                            (LPVOID*)&pCategoryMgr);
    if(FAILED(hr))
    {
        return hr;
    }

    //Create the display attribute manager. 
    ITfDisplayAttributeMgr  *pDispMgr;
    hr = CoCreateInstance(  CLSID_TF_DisplayAttributeMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfDisplayAttributeMgr, 
                            (LPVOID*)&pDispMgr);
    if(FAILED(hr))
    {
        pCategoryMgr->Release();
        return hr;
    }
    
    //Get the display attribute property. 
    ITfProperty *pProp;
    hr = pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);
    if(SUCCEEDED(hr))
    {
        VARIANT var;

        VariantInit(&var);
        hr = pProp->GetValue(ec, pRange, &var);
        if(S_OK == hr)  //Returns S_FALSE if the range is not completely covered by the property.  
        {
            if(VT_I4 == var.vt)
            {
                //The property is a guidatom. 
                GUID    guid;

                //Convert the guidatom into a GUID. 
                hr = pCategoryMgr->GetGUID((TfGuidAtom)var.lVal, &guid);
                if(SUCCEEDED(hr))
                {
                    ITfDisplayAttributeInfo *pDispInfo;

                    //Get the display attribute info object for this attribute. 
                    hr = pDispMgr->GetDisplayAttributeInfo(guid, &pDispInfo, NULL);
                    if(SUCCEEDED(hr))
                    {
                        //Get the display attribute info. 
                        hr = pDispInfo->GetAttributeInfo(pDispAttr);

                        pDispInfo->Release();
                    }
                }
            }
            else
            {
                //An error occurred; GUID_PROP_ATTRIBUTE must always be VT_I4. 
                hr = E_FAIL;
            }
            VariantClear(&var);
        }
    pProp->Release();
    }

    pCategoryMgr->Release();
    pDispMgr->Release();

    return hr;
}
}}}
#endif