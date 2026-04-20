

/**************************************************************************

Filename    :   GFx_IMENamesManagerVista.h
Content     :   Overrides the functions used to obtain names of installed ime's and 
                activating ime's according to platform specific implementation.
Created     :   Oct 01, 2008
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.
					 
Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFXIMENAMESMANAGERVISTA_H
#define INC_GFXIMENAMESMANAGERVISTA_H

#include "GFx_IMEIdMap.h"
#include <msctf.h>
#include "GFx_TSFUIElementDef.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

extern InputLangProps SupportedIMEs[];

/*
WINBASEAPI
int
WINAPI
GetLocaleInfoEx(
               __in LCID     Locale,
               __in LCTYPE   LCType,
               __out_ecount_opt(cchData) LPWSTR  lpLCData,
               __in int      cchData);

*/
// 71c6e74e-0f28-11d8-a82a-00065b84435c
DEFINE_GUID(IID_ITfInputProcessorProfileActivationSink_GFx,
            0x71c6e74e, 0x0f28, 0x11d8, 0xa8, 0x2a, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c);

// {CCF05DD8-4A87-11D7-A6E2-00065B84435C}
DEFINE_GUID(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION_GFX,
            0xCCF05DD8, 0x4A87, 0x11D7, 0xa6, 0xe2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c);

class GImeNamesManagerVista:    public  GImeNamesManager,
                                public ITfCompartmentEventSink,
                                public ITfInputProcessorProfileActivationSink
                            
{      
public:
    GImeNamesManagerVista(GFxIMEManagerWin32* pimemanager);

    ~GImeNamesManagerVista() { CleanUp(); };
  
    // IUnknown
    STDMETHODIMP            QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

    // ITfInputProcessorProfileActivationSink
    STDMETHODIMP_(HRESULT)  OnActivated(DWORD dwProfileType,
                            LANGID langid,
                            REFCLSID rclsid,
                            REFGUID catid,
                            REFGUID guidProfile,
                            HKL hkl,
                            DWORD dwFlags);
    
    // ITfCompartmentEventSink
    STDMETHODIMP            OnChange(REFGUID refguid );

    virtual bool            HashIMENames(void);

#ifndef GFC_NO_LANGBAR_SUPPORT

    virtual void            ActivateIME(const char* imeName);
    virtual void            ActivateInputLanguage(const char* inputLangName);
    virtual void            SetConversionMode(UInt32 conversionParams = -1);
    virtual void            HandleStatusWindowNotifications(const char* pcommand, const char* parg);
    void                    OnLangBarLoaded();
#endif

    void                    CleanUp()
    {
        for (UInt32 i = 0; i < NumIme; i++)
        {
			ProfileInfo* profileInfo = (ProfileInfo*)(SupportedIMEs[i].Id);

			while (profileInfo)
			{
				SF_FREE ((TF_LANGUAGEPROFILE*)(profileInfo->profile));
				ProfileInfo* tmp = profileInfo->next;
				SF_FREE (profileInfo);
				profileInfo = tmp;
			}
        }
        
        for (UInt32 i = 0; i < HklLayoutTextMap.GetSize(); i++)
        {
            SF_FREE((char*)HklLayoutTextMap[i].LayoutName);
            SF_FREE((char*)HklLayoutTextMap[i].ImeFileName);
        }

        for (UInt32 i = 0; i < NumLanguages; i++)
        {
            if (SupportedInputLanguages[i].ItemNameOnSystem != NULL)
                SF_FREE((char*)SupportedInputLanguages[i].ItemNameOnSystem);
        }

        UnInitCompartmentSink();
        UnInstallProfileActivationSink();
        Release();
    }
    
    virtual GFxIMETag       GetImeTag(const char* name = NULL);
	virtual void			OnInputLangChange(DWORD langId);
    void                    UnInstallProfileActivationSink();
	virtual bool			SwitchIME(String imeName);
    
    DWORD                   dwInputProcessorProfileEventSinkCookie;
    LONG                    cRef;
    DWORD                   dwCompartmentSinkCookie;
    ITfCompartment          *pCompartment;

private:

    TfClientId              pClientId;
    ITfThreadMgr            *pThreadMgr;
    // Registry query related
    void                    CheckForSupportedIME(TF_INPUTPROCESSORPROFILE& profile, const char* imeName);
    void                    CheckForSupportedInputLang(TF_INPUTPROCESSORPROFILE& langProfile, const wchar_t* inputLangName);
    
    // TSF Related
    BOOL                    InitCompartmentSink();
    void                    UnInitCompartmentSink();
};
}}}
#endif //INC_GFXIMENAMESMANAGERVISTA_H
