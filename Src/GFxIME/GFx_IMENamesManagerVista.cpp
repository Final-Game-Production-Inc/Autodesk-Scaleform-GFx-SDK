/**********************************************************************

Filename    :   GFxIMENamesManagerVista.cpp
Content     :   Overrides the functions used to obtain names of installed ime's and 
                activating ime's according to platform specific implementation.
Created     :   Oct 01, 2008
Authors     :   Ankur Mohan

Notes       :   20/Mar/09: Must use SUCCEEDED or FAILED to test for success or failure.
History     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFxConfig.h"
#ifndef SF_NO_IME_SUPPORT

#include <InitGuid.h>

#include <olectl.h>
//EXTERN_C const IID IID_ITfInputProcessorProfileMgr_GFx;
#include "GFx_IMENamesManagerVista.h"
#include "GFx_IMEManagerWin32.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{
extern int ImeFileNameOrLayoutText[];

STDAPI GImeNamesManagerVista::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfInputProcessorProfileActivationSink_GFx))
    {
        *ppvObj = (ITfInputProcessorProfileActivationSink *)this;
    }

    else if (IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (ITfCompartmentEventSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) GImeNamesManagerVista::AddRef()
{
    return ++cRef;
}


STDAPI_(ULONG) GImeNamesManagerVista::Release()
{
    
    LONG cr = --cRef;


    if (cRef == 0)
    {
//        delete this;
    }

    return cr;
}

// Must explicitly call base class constructor..
GImeNamesManagerVista::GImeNamesManagerVista(GFxIMEManagerWin32* pimemanager):GImeNamesManager(pimemanager)
{
    SF_UNUSED(pimemanager);
    // Install the activateinputprofile sink
    BOOL fRet               = FALSE;
    ITfSource *pSource      = NULL;
    ITfCompartmentMgr* pCompartmentMgr = NULL;
    pThreadMgr              = NULL;
    pCompartment            = NULL;
    cRef                    = 1;
    InstalledInputLangs     = ""; 
	

    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_ITfThreadMgr, 
        (void**)&pThreadMgr);

    if (FAILED(hr))
        return;

    if (FAILED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
		if (pThreadMgr)
			pThreadMgr->Release();
        goto Exit;
    }

//    hr = pThreadMgr->Activate(&pClientId);

    fRet = FALSE;

    if (FAILED(pSource->AdviseSink(IID_ITfInputProcessorProfileActivationSink_GFx, (ITfInputProcessorProfileActivationSink *)this, &dwInputProcessorProfileEventSinkCookie)))
    {
        // make sure we don't try to Unadvise _dwThreadMgrEventSinkCookie later
        dwInputProcessorProfileEventSinkCookie = TF_INVALID_COOKIE;
        //    goto Exit;
    }

    // Get the compartment manager first and then obtain the conversion mode
    // compartment so we receive only conversion mode related notifications.

    pCompartmentMgr = NULL;

    hr = pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void**)&pCompartmentMgr);
    if (SUCCEEDED(hr) && pCompartmentMgr)
    {
        hr = pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION_GFX,
            &pCompartment);
        if (hr != S_OK)
        {
            //    printf("Failed to create compartment interface\n");
            //    return FALSE;
        }
        pCompartmentMgr->Release();
    }

    if (InitCompartmentSink() != TRUE)
    {
        //   printf("Failed to create compartment sink\n");
    }
    fRet = TRUE;

Exit:
    if(pSource)
        pSource->Release();
}

BOOL GImeNamesManagerVista::InitCompartmentSink()
{
    ITfSource *pSource = NULL;
    BOOL fRet = FALSE;

    if (pCompartment == NULL) goto Exit;

    if (FAILED(pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfCompartmentEventSink, (ITfCompartmentEventSink *)this, &dwCompartmentSinkCookie)))
    {
        // make sure we don't try to Unadvise Cookie later
        dwCompartmentSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    if(pSource)
        pSource->Release();
    return fRet;
}

void GImeNamesManagerVista::UnInitCompartmentSink()
{
    ITfSource *pSource = 0;

    if (dwCompartmentSinkCookie == TF_INVALID_COOKIE)
        return; // never Advised
	if (pThreadMgr)
	{
		if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
		{
			if (pSource->UnadviseSink(dwCompartmentSinkCookie) == CONNECT_E_NOCONNECTION)
				Release();
			pSource->Release();
		}
	}

    dwCompartmentSinkCookie = TF_INVALID_COOKIE;
}

void GImeNamesManagerVista::UnInstallProfileActivationSink()
{
    ITfSource *pSource;

    if (dwInputProcessorProfileEventSinkCookie == TF_INVALID_COOKIE)
        return; // never Advised
	
	if (pThreadMgr)
	{
		if (SUCCEEDED(pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
		{
			pSource->UnadviseSink(dwInputProcessorProfileEventSinkCookie);
			pSource->Release();
		}
	}
    dwInputProcessorProfileEventSinkCookie = TF_INVALID_COOKIE;
}

HRESULT GImeNamesManagerVista::OnChange(REFGUID refguid )
{
    VARIANT var;

    if (refguid == GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION_GFX)
    {
        if (SUCCEEDED(pCompartment->GetValue(&var))) 
        {
            if (var.vt == VT_I4) 
            {
                //  ret = (var.lVal & TF_CONVERSIONMODE_FULLSHAPE) != 0;
                CachedConversionMode = var.lVal;
#ifndef GFC_NO_LANGBAR_SUPPORT
                GImeNamesManager::SetConversionMode(CachedConversionMode);
#endif
            }
        }
    }

    // Return value is ignored.
    return true;
}

// Override and do nothing since all the stuff that needs to happen here takes place in OnActivated.
void GImeNamesManagerVista::OnInputLangChange(DWORD langId)
{
	SF_UNUSED(langId);
	return;
}


/*
Important Note: This function is called when the input language/IME is changed using either our language bar or the windows language bar.
the WM_LANG_CHANGE notification is also sent in response, which leads to OnInputLangChange being called which we'll override and ignore 
since we don't want to repeat the same steps.
*/
HRESULT   GImeNamesManagerVista::OnActivated(DWORD dwProfileType,
                            LANGID langid,
                            REFCLSID rclsid,
                            REFGUID catid,
                            REFGUID guidProfile,
                            HKL hkl,
                            DWORD dwFlags)
{


    SF_UNUSED3(catid, hkl, dwFlags);
    HRESULT						hr;
    ITfInputProcessorProfiles	*pProfiles;
	ITfInputProcessorProfileMgr *pprofilesMgr;
	TF_INPUTPROCESSORPROFILE	profile1;
    BSTR						currentProfileName = SysAllocStringByteLen(NULL, 0);
    wchar_t						szLangName[MAX_PATH];
    bool						imeNameSet = false; // This variable records if we were able to obtain the IME name or not.

	
	
		// Note according to the documentation:
	// hkl: Specifies the keyboard layout handle of this profile. If dwProfileType is TF_PROFILETYPE_ INPUTPROCESSOR, hkl is NULL.
	// On chinese Vista I've found that this is not always true (for Changjie, new Changjie, phonetic and new phonetic). 
	// If this member is not NULL, the profile doesn't match what we've stored in the 
	// SupportedIME's list and the IME is not activated properly. 
	// To take care of this, we set hkl to 0 if dwProfileType = TF_PROFILETYPE_ INPUTPROCESSOR
	
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, __uuidof(ITfInputProcessorProfiles), (LPVOID*)&pProfiles);
	if (FAILED(hr))
		return false;

	hr = pProfiles->QueryInterface(IID_ITfInputProcessorProfileMgr_GFx, (LPVOID*)&pprofilesMgr);
	if (FAILED(hr))
		return false;
   
	// This must be done before obtaining the profile from the profilesManager. 
	if (dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) hkl = 0;

	pprofilesMgr->GetProfile(dwProfileType, langid, rclsid, guidProfile, hkl, &profile1);
	HKL_Size_t_Union un1;
	un1.hkl = profile1.hkl;

	GUID guid_null = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

	// When this happens, it means (for some Korean IMEs) that this profile is bogus and was never registered in HashIMENames, so don't do anything.
	if ( IsEqualGUID(profile1.guidProfile, guid_null) && IsEqualGUID(profile1.catid, guid_null) && ((un1.val & 0xFFFFFFFF) == 0) ) return true;

//	GUID guidclsidLocal		= rclsid;
//	GUID guidProfileLocal	= guidProfile;

	if (profile1.hklSubstitute != 0)
	{
	//	hkl = profile.hklSubstitute;
	//	profile.guidProfile = 0;
	//	GUID guid_null = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
	//	guidclsidLocal		= guid_null;
	//	guidProfileLocal	= guid_null;
	}

	if (pprofilesMgr)
		pprofilesMgr->Release();

    if (SUCCEEDED(hr) && pProfiles) 
    {
	//	if (dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) hkl = 0;

        CurrentInputLangTag = GetLangNamefromLangId(langid);
        // Note- must use the W version of GetLocaleInfo, since otherwise szLangName will contain bad characters for
		// languages containing non-ascii characters.
        GetLocaleInfoW(MAKELCID(langid, SORT_DEFAULT), 
            LOCALE_SLANGUAGE, szLangName, MAX_PATH);

		// 1) Send notifications to AS about input language change and update text in the InputLangTab of the language bar
		//	  Note that if the new input language is not supported, the InputLangTab might say "Unknown" corresponding to the 
		//    GFxIMETag_NotSupported.

        // Note that SetCurrentInputLanguage also calls BroadcastSwitchLanguage
        pImeMgr->SetCurrentInputLanguage(CurrentInputLangTag);
		
#ifndef GFC_NO_LANGBAR_SUPPORT
		// If the user uses the language bar to change the input language, we should change
        // the text in the InputLangTab of our language bar in action script. 
		String str(szLangName);
        pImeMgr->BroadcastSetCurrentInputLanguage(str.ToCStr());
#endif

		/* Following cases to consider:

			1- System IME (Supported)
			2- System IME (UnSupported)
			3- Third Party IME (Supported)
			4- Third Party IME (UnSupported)
			5- Keyboard Layout (Doesn't have any IME)
		*/
		
		//2) 
		// This obtains the name of the new input language and updates the IMENameTab. Note that at this point, we haven't 
		// checked if we support the new input language or not. We are just setting the text in the IMENameTab to whatever 
		// TSF tells us is the name of the input language. 
		 pProfiles->GetLanguageProfileDescription(rclsid, langid, guidProfile, &currentProfileName);
	
        if (SysStringLen(currentProfileName) == 0) 
        {
			// So we are either a keyboard layout, or third party IME. The dwProfileType member doesn't help distinguish betweehn
			// the two since it's equal to TF_PROFILETYPE_KEYBOARDLAYOUT  for both. The way we will distinguish is to 
			// match the hkl against the registry entries and check if there is an entry corresponding to imefile or not. 
			Un.hkl = hkl;
			UInt32 numInstalledHKls = (UInt32)HklLayoutTextMap.GetSize();
			GImeNamesManager::SetLastIMEName(String(""));

			if (Un.val != 0x04120412)
			{
				bool matchFound = false;
				for (UInt32 i = 0; i < numInstalledHKls; i++)
				{
					if (((Un.val & 0xFFFFFFFF) == HklLayoutTextMap[i].hkl))
					{
						if (SFstrlen(HklLayoutTextMap[i].ImeFileName) != 0)
						{
							// This is a third party IME. We will check if it's supported or not in the next step
							matchFound = true;
							break;
						}
					}
				}
				if (!matchFound)
				{
					// This is a keyboard layout for sure. Return
					GImeNamesManager::SetLastIMEName(String(szLangName));
					goto exit;
				}
			}
        }

    //    GFxString profileName((const wchar_t*)(currentProfileName));
	//	GImeNamesManager::SetLastIMEName(profileName);
		

		// 3) 
		// Now we'll look into our data structures and check to see if we actually support this new input language. If we 
		// do, we'll update the name to what we get from our DS and also show the status window.
        
		// Terminology: We'll use the term "Input Processor Profile" (following MSDN documentation) to refer to an system IME, 
		// keyboard layout (for example- English(US), English (UK) etc) and third party IMEs. According to my observations, the
		// following is true:
		// 1) When the new input processor profile is a system IME (such as Quan Pin, Phonetic, Korean etc- anything that comes
		// pre-installed with the operating system), dwProfileType is TF_PROFILETYPE_INPUTPROCESSOR. For this case, hkl is null.
		
		// 2) When the new input processor profile is a third-party IME or a keyboard layout, 
		// dwProfileType = TF_PROFILETYPE_KEYBOARDLAYOUT. For this case, rclsid, catid and guidProfile are null. 
		

		CurrentIMETag = (langid == 0x0804 ? GFxIME_Ch_Simp_Other : GFxIME_NotSupported);
		imeNameSet = false;
		for (UInt32 i = 0; i < NumIme; i++)
		{
			ProfileInfo* profileInfo = (ProfileInfo*)SupportedIMEs[i].Id;
			while (profileInfo)
			{
				
				TF_INPUTPROCESSORPROFILE profile2 = *((TF_INPUTPROCESSORPROFILE*)(profileInfo->profile));

				if ( (profile2.langid			== langid)			&&
					(profile2.clsid				== rclsid)			&&
					(profile2.catid				== catid)			&&
					(profile2.hkl				== hkl)				&&
					(profile2.guidProfile		== guidProfile)		&&
					(profile2.hklSubstitute		== profile1.hklSubstitute))

				{
					CurrentIMETag = SupportedIMEs[i].ItemTag;
					pImeMgr->SetIMETag(CurrentIMETag);

					if ( ImeFileNameOrLayoutText[i] == 2)
						GImeNamesManager::SetLastIMEName(SupportedIMEs[i].ItemNameCommon);
					if ( ImeFileNameOrLayoutText[i] == 1)
						GImeNamesManager::SetLastIMEName(SupportedIMEs[i].ItemNameOnSystem);

					imeNameSet = true;
#ifndef GFC_NO_LANGBAR_SUPPORT
					pImeMgr->BroadcastDisplayStatusWindow();
#endif
					break;
				}
				profileInfo = profileInfo->next;
			}
			if (imeNameSet) break;
		}

		if (!imeNameSet) // Failed to set the imename by the previous two techniques, just set it to the language name as the fallback option
		{
			GImeNamesManager::SetLastIMEName(String("Not Supported"));
		}

		if (SysStringLen(currentProfileName) != 0) SysFreeString(currentProfileName);
exit:	if (pProfiles)
			pProfiles->Release();
		// Return value is ignored.

		return true;
	}

	return true;

/*
        if (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR)
        {
            pProfiles->GetLanguageProfileDescription(rclsid, langid, guidProfile, &currentProfileName);
            if (SysStringLen(currentProfileName) == 0) 
            {
                pProfiles->Release();
				pProfiles = 0;
                goto Exit;
            }
            GFxString profileName((const wchar_t*)(currentProfileName));
            
            CurrentIMETag = GFxIME_NotSupported;

			for (UInt i = 0; i < NumIme; i++)
			{
				if ( SupportedIMEs[i].Id && (guidProfile.Data1 == ((TF_INPUTPROCESSORPROFILE*)(SupportedIMEs[i].Id))->guidProfile.Data1))
				{
					CurrentIMETag = SupportedIMEs[i].ItemTag;
					break;
				}
			}

			pImeMgr->SetIMETag(CurrentIMETag);

            SysFreeString(currentProfileName);
            GImeNamesManager::SetLastIMEName(profileName);
            GFxIMETag inputLangTag;
            inputLangTag = GetLangNamefromLangId(langid);

#ifndef GFC_NO_LANGBAR_SUPPORT
            // Note: Third party IME's profile type is TF_PROFILETYPE_KEYBOARDLAYOUT, so this prevents
            // the status window to be displayed for third party IME's (which doesn't work, so doesn't need to be shown)
            if (inputLangTag != GFxIME_Jp && inputLangTag != GFxIME_Kr && CurrentIMETag != GFxIME_NotSupported)
                pImeMgr->BroadcastDisplayStatusWindow();
#endif
        }

        if ( dwProfileType & TF_PROFILETYPE_KEYBOARDLAYOUT)
        {
            // This could be a keyboard layout or a third party ime. 
            // look in the hkllayoutmap to see if there is a corresponding entry.
        //  wchar_t layoutName[100];
        //  GetKeyboardLayoutName(layoutName);
            for (UInt i = 0; i < HklLayoutTextMap.GetSize(); i++)
            {
                Un.hkl = hkl;
				// The & 0xFFFFFFFF is for taking care of 64 bit computers where the first 4 bytes may not be set to zero
				// and therefore shouldn't be considered.
                if ((Un.val& 0xFFFFFFFF) == HklLayoutTextMap[i].hkl)
                {
                    // Found an entry! Now look in the list of supported IMEs to check if we support this third party ime
                    for(UInt j = 0; j < NumIme; j++)
                    {
                        if(!G_stricmp(SupportedIMEs[j].ItemNameOnSystem, HklLayoutTextMap[i].LayoutName) 
						|| !G_stricmp(SupportedIMEs[j].ItemNameOnSystem, HklLayoutTextMap[i].ImeFileName) )
                        {
                            CurrentIMETag = SupportedIMEs[j].ItemTag;
                            GImeNamesManager::SetLastIMEName(HklLayoutTextMap[i].LayoutName);
                            goto Exit;
                        }
                    }
                }
            }
            CurrentIMETag = GFxIME_NotSupported;
            // right now we are note handling multiple keyboard layouts for the same input\
            // language correctly. For example, if United States-Dvorak and US layouts are 
            // chosen for English inpute language, there is no way to get their names 
            // from the hkl. So right now just display the input language name in the IMEName Tab. 
            GImeNamesManager::SetLastIMEName(szLangName);
            
        }
		*/
   
}
/*
 The basic idea here is as follows: We have our data structure called SupportedIME's which is an array of 
 InputLangProps structs. This array contains a list of IME's that we support. The entries are as follows:

 struct InputLangProps
 {
 char* ItemNameOnSystem;    // The name of the IME on the system. Usually corresponds to the "LayoutText" entry 
                            // in the "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts" key of the registry.
 char* ItemNameCommon;      // A descriptive name assigned by us to the IME. Its use is described later.     
 GFxIMETag   ItemTag;       // An enum assigned to the IME used in comparisons.
 SPInt       Id;            // Locale Id or pointer to Language Profile struct. It's use is described later.
 };

 We want to query the system for the IME's installed and check in the SupportedIME DS to see which ones we support. 
 For every IME we support, we set the Id field to either the HKL for that IME (on XP) or pointer to a profile structure
 (on Vista, refer to MSDN doc on TSF for more details. HKL can't be used on Vista since more than one IME's can have the 
 same HKL). This field is used when we are called upon to switch to a specific IME. 

 Obtaining the list of active IME's is relatively straighforward on XP (the GetKeyboardLayoutList function comes handy 
 and provides us with exactly what we need). This functions provides us a list of HKL's that correspond to the active
 input langugages and IME's. We now need to correspond this information with the SupportedIME DS. The HKL's can't be used
 directly since the same IME can have a different HKL on different computers. To solve this problem, we look into the registry
 and make a list of the IMEFileName and LayoutText field for every installed IME (the entries in the registry are indexed
 by a key which is the same as the HKL for the IME- for example, look at the SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts
 entry on your comp and you'll know what I mean). Now it's important to understand the difference between "installed" and 
 "active" IME's. The user doesn't have to activate all the IME's installed on this computer. He can use the language bar
 to add or remove IME's from the list of active IME's for each input language. In the GFx language bar, we only want to 
 display the IME's that are active. Therefore, we look at the intersection of the HKLs returned by GetKEyboardLayoutList
 and those in the registry to get a list of active IMEs. For each active IME, we check if the LayoutName or the IMEFileName
 matches the corresponding entry in the SupportedIMEs DS. The IMEFileName is needed since the LayoutName can be localized
 (On a Chinese computer, the IME name could be in chinese etc), but the IMEFileName is the same. Different versions of the 
 same IME can have different IMEFileNames through, so all known versions must be tested. If the LayoutText matches, we use that
 to display the IME info in our language bar. If the IMEFileName matches, we use the ItemNameCommon entry. 

 On Vista, the situation is more complicated. The system IME's (the ones that come preinstalled with Windows Vista) all have
 the same HKL, so HKL can't be used to distinguish between IME's. 
 pProfiles->GetLanguageProfileDescription(langProfile.clsid, langProfile.langid, langProfile.guidProfile, &profileName);
 is used to get the profile info (on Vista, the term profile is used to describe IME's and Keyboardlayouts) for each
 input language. For the system IME's, the hkl field of the profile structure is 0, and the name of the profile is 
 returned in profileName (see code below). For third party IME's, a unique HKL IS assigned to each IME, so we can check
 against the registry information to obtain the IMEFilename and the LayoutText name. So to reiterate, for the system IME's,
 only the profile name as returned by the GetLanguageProfileDescription can be obtained- since we can't index these IME's
 against the registry, IMEFileName can't be obtained. For the third party IME's both the LayoutText as well as IMEFileName 
 can be obtained. These two pieces of info (IMEFileName = null for system IMEs) are used in the CheckForSupportedIME to 
 check if that IME is supported by us or not. This means that if the system IME names are localized on OS in different languages,
 we'll need to have the corresponding entry in the supportedIME DS. So far this doesn't seem to be the case. Also, on Vista,
 a pointer to the profile structure is stored in the id field of the inputLangProps DS, which can be used by TSF functions
 to switch to a certain IME or inputlanguage.


*/
bool GImeNamesManagerVista::HashIMENames()
{
    HRESULT hr;
    ITfInputProcessorProfiles   *pProfiles;
    ITfInputProcessorProfileMgr *pProfilesMgr;
    TF_INPUTPROCESSORPROFILE    langProfile;
    TF_INPUTPROCESSORPROFILE    activeProfile;
    CurrentIMETag               = GFxIME_NotSupported;
    CurrentInputLangTag         = GFxIME_NotSupported;
    
    GImeNamesManager::MakeKeyboardLayoutListFromRegistry(0,0);
    
    for (UInt32 i = 0; i < NumIme; i++) ImeFileNameOrLayoutText[i] = 0;

    //Create the object.
    hr = CoCreateInstance(  CLSID_TF_InputProcessorProfiles, 
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfInputProcessorProfiles, 
                            (LPVOID*)&pProfiles);

    if(FAILED(hr) || !pProfiles)
        return true;

    hr = pProfiles->QueryInterface(IID_ITfInputProcessorProfileMgr_GFx, (LPVOID*)&pProfilesMgr);
	CurrentIMETag = GFxIME_NotSupported;
    if(SUCCEEDED(hr) && pProfilesMgr)
    {
        LANGID  *rgLangIDs;
        ULONG   ulCount;
    //  IEnumTfLanguageProfiles* ppEnum;
        IEnumTfInputProcessorProfiles* ppEnum;
		pProfilesMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &activeProfile);
        hr = pProfiles->GetLanguageList(&rgLangIDs, &ulCount);
        if(SUCCEEDED(hr))
        {
            ULONG   i;

            for(i = 0; i < ulCount; i++)
            {
                wchar_t   szLangName[MAX_PATH];
                if( 0 != GetLocaleInfoW(MAKELCID(rgLangIDs[i], SORT_DEFAULT), 
                            LOCALE_SLANGUAGE, 
                            szLangName, 
                            MAX_PATH))
                {
                    pProfilesMgr->EnumProfiles(rgLangIDs[i], &ppEnum);

                    GUID guid; guid.Data1 = guid.Data2 = guid.Data3 = 0; 
                    langProfile.catid = guid; langProfile.clsid = guid; // langProfile.fActive = 0;
                    langProfile.guidProfile = guid; langProfile.langid = rgLangIDs[i];
                    // Note: 
                    // Some languages don't have IME's. For example English. Even though they might
                    // have "language profiles" for example- speech to text conversion and other text 
                    // services. In our SupportedIMEs list, we only have language names and IME names.
                    // Hence, for languages such as English, we must call CheckForSupportedIME outside
                    // of the EnumProfile loop, so that we can create an entry for it in order to switch 
                    // the input language to English. 
                    CheckForSupportedInputLang(langProfile, szLangName); 
                    BSTR profileName = SysAllocString(NULL);
                    String profileNameStr;
					String imeNameStr;

                    if (ppEnum)
                    {
						// For each input language..
                        while (ppEnum->Next(1, &langProfile, NULL) == S_OK)
                        {
                            SysFreeString(profileName);
                            profileNameStr.Clear();
                            pProfiles->GetLanguageProfileDescription(langProfile.clsid, langProfile.langid, langProfile.guidProfile, &profileName);
							// Check if there are any special characters in the profileName such as percentages
							// for example: "@"%ProgramFiles%\Windows Journal\NBMapTip.dll",-16"
							if (profileName)
							{
								UPInt len = SFwcslen(profileName);
								bool bspecialChar = false;
								for (UPInt ii = 0; ii < len; ii++)
								{
									if (profileName[ii] == '%')
									{
										bspecialChar = true;
										break;
									}
								}

								if (bspecialChar)
									continue;
							}
							
							// If the profile is neither active or enabled
                            if (!((langProfile.dwFlags & TF_IPP_FLAG_ENABLED) || (langProfile.dwFlags & TF_IPP_FLAG_ACTIVE)))
                                continue;

							// This variable checks if the current profile is a keyboard layout or not. 
							// Checking for TF_PROFILETYPE_KEYBOARDLAYOUT is not enough since third party IME's
							// can have dwProfileType = above flag.

							bool isThirdPartyIME  = false;
							bool isKoreanIME  = false;
							if ((langProfile.hkl != 0)/* || (langProfile.hklSubstitute != 0) *//*&& SysStringLen(profileName) == 0*/)
                            {
                                // Check in the layout text table indexed by the hkl.
								// At this point, we know we are dealing with either keyboard layout or third party ime.
								// keyboard layouts can be the default keyboard layout (for example US English) in which 
								// case the corresponding hkl is what's obtained from the registry (0x04090409). User can
								// also add different keyboard layouts to an input language (for example, United Kingdom (International),
								// United Kingdom (Extended) etc using the language settings in the control panel. 
								// Changing the keyboard layout generally changes the mapping between keystrokes and the virtual
								// key code generated and doesn't involve the IME system. In our language bar, we don't care about
								// different keyboard layouts installed for a certain input language. The system doesn't 
								// seem to provide a way to obtain information about these layouts anyways. 
							
								// We distinguish between the keyboard layouts and third party IME based on the value of 
								// ImeFileName in the corresponding registry entry.
                                for (UInt32 hklIdx = 0; hklIdx < HklLayoutTextMap.GetSize(); hklIdx++)
                                {
                                    HKL_Size_t_Union un1;
									HKL_Size_t_Union un2;
									un1.hkl = langProfile.hkl;
									un2.hkl = langProfile.hklSubstitute;
									// Korean keyboard layout is a special case, and we must create a profile for it for 
									// things to work properly
									if ((un1.val & 0xFFFFFFFF) == 0x04120412)
									{
										if ((un1.val & 0x0000FFFF) == HklLayoutTextMap[hklIdx].hkl)
										{
											isKoreanIME = true;
											 profileNameStr	= HklLayoutTextMap[hklIdx].LayoutName;
											 break;
										}
									}
						
                                    else if (((un1.val & 0xFFFFFFFF) == HklLayoutTextMap[hklIdx].hkl) ||
										((un2.val & 0xFFFFFFFF) == HklLayoutTextMap[hklIdx].hkl))
                                    {
                                        profileNameStr	= HklLayoutTextMap[hklIdx].LayoutName;
										imeNameStr		= HklLayoutTextMap[hklIdx].ImeFileName;
                                        
										if (!imeNameStr.IsEmpty()) // this must be keyboard layout, no need to record it.
										{										
											// This is a third party IME, don't know if we support it or not.
											isThirdPartyIME = true;
										}
										
										break;
                                    }
									else
									{
									}
                                }
								if (!isKoreanIME && !isThirdPartyIME)
								{
									// no need to store this profile, this is just a keyboard layout (either a default
									// layout like english (US) or custom layouts. Consider the next profile
									if (langProfile.dwFlags & TF_IPP_FLAG_ACTIVE)
									{
										CurrentIMETag = GFxIME_DoesntExist;
									}
									continue;
								}

                            }

							else //System IME
                            {
                                profileNameStr = (wchar_t*)profileName;
                            }
							// Now lets check if this profile (either a system IME or third party IME) is supported or not.
							bool isSupported = false;
							int j = -1;
                            if (!profileNameStr.IsEmpty())
							{

								j = GImeNamesManager::CheckForSupportedIME(profileNameStr.ToCStr(), imeNameStr.ToCStr()); // Check if we support this IME. 
								// Reset imeNameStr and profileNameStr
								// Sometimes the profileNameStr can have escape characters such as percentages in it. 
								if (pImeMgr->pLog)
									pImeMgr->pLog->LogMessage(String("Registering IME: ") + profileNameStr + String("\n"));
								imeNameStr.Clear();
								profileNameStr.Clear();
								isSupported = (j == -1)? false:true;
							}
							if (isSupported)
							{
								TF_INPUTPROCESSORPROFILE* profile = (TF_INPUTPROCESSORPROFILE*)SF_ALLOC(sizeof(TF_INPUTPROCESSORPROFILE), StatIME_Mem);
								/*
								We need to create a list of profiles for each IME since TSF can send us multiple profiles for each IME and 
								when OnActivated is called, we don't know which profile is sent last. Se we need to store all the profiles and 
								match against all the profiles stored for an IME
								To activate an IME, we just use the first profile stored. That has worked fine so far. 
								*/
								memcpy(profile, (void*)(&langProfile), sizeof(TF_INPUTPROCESSORPROFILE));

								ProfileInfo* newProfile = (ProfileInfo*)SF_ALLOC(sizeof(ProfileInfo), StatIME_Mem);
								newProfile->profile		= profile;
								newProfile->next		= (ProfileInfo*)SupportedIMEs[j].Id;
								SupportedIMEs[j].Id		= (SPInt)newProfile;
								// check if this profile is active
								if (langProfile.dwFlags & TF_IPP_FLAG_ACTIVE)
								{
									CurrentIMETag = SupportedIMEs[j].ItemTag;
								}
							}
                        }
                        ppEnum->Release();
                    }         

                    SysFreeString(profileName);
                    profileName = NULL;
                }               
            }
        
            CoTaskMemFree(rgLangIDs);
        }
        
        // Set currently active language
		for (UInt32 j = 0; j < NumLanguages; j++)
        {
            // If this Input Language is supported, add the lang id in the Input Language Name table
            if (SupportedInputLanguages[j].Id == activeProfile.langid)
            {
                CurrentInputLangTag = SupportedInputLanguages[j].ItemTag; 
            }
        }
		
        if (pProfiles)
            pProfiles->Release();
        if (pProfilesMgr)
            pProfilesMgr->Release();
        return false;
    }
    return true;
}

GFxIMETag GImeNamesManagerVista::GetImeTag(const char* name)
{   
    // If name == NULL, obtain the hash key using system specific method- on XP use 
    // GetKeyboardLayoutName to get the string corresponding to the current input language
    // and the make a Key out of it. If the user has passed a non null name, then call the 
    // base class method and use the name to return the corresponding tag. The base class
    // implementation just looks into the hash table for the tag- no system specific details
    // needed. 
    // that to make a key. 
    if ( name != NULL) return GImeNamesManager::GetImeTag(name);

    if (CurrentInputLangTag == GFxIME_NotSet)
    {
        // The CurrentInputLangTag and CurrentIMETag should be set while IMENames are being hashed.
        // If the current input language/ime is not supported, they should be set to GFxIMETagNotSupported.

        return GFxIME_NotSupported;             
    }
    else
        return CurrentIMETag;

}

extern StringToTagDef StringToTag[];

bool GImeNamesManagerVista::SwitchIME(String imeName)
{
	GFxIMETag tag;
    UInt32 i = 0;

    while (StringToTag[i].Tag != GFxIME_NotSet)
    {
        if (!SFstrcmp(imeName, StringToTag[i].String))
        {
            // Found the tag- now search in SupportedIMEs
            tag = StringToTag[i].Tag;
            for (UInt32 j = 0; j < NumIme; j++ )
            {   
                // important to run through the entire list of supported ime's since the first occurance of the 
                // tag may not be the one that the ime is registered against.
                if (SupportedIMEs[j].ItemTag == tag)
                {
					// On Vista, the input language must be activated before the IME.
					// Make sure that this IME is registered with us- if not, don't activate the corresponding input language
					if (ImeFileNameOrLayoutText[j] == 1 || ImeFileNameOrLayoutText[j] == 2)
					{
						CurrentInputLangTag = GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag);
						// Which input language is this?
						for (UInt32 k = 0; k < SupportedInputLanguages.GetSize(); k++)
						{
							if (SupportedInputLanguages[k].ItemTag == CurrentInputLangTag)
							{
								// Switch to this input language
								ActivateInputLanguage(SupportedInputLanguages[k].ItemNameOnSystem);
							}
						}
					}
                    if (ImeFileNameOrLayoutText[j] == 1)
                    {
						
                        ActivateIME(SupportedIMEs[j].ItemNameOnSystem);
                        return true;
                    }
                    else if (ImeFileNameOrLayoutText[j] == 2)
                    {
					//	CurrentInputLangTag = GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag);
                        ActivateIME(SupportedIMEs[j].ItemNameCommon);
                        return true;
                    }                    
                }               
            }
        }
        i++;
    }
    return false;
}

void GImeNamesManagerVista::CheckForSupportedInputLang(TF_INPUTPROCESSORPROFILE& langProfile, const wchar_t* inputLangName)
{
    
    char utf8Buf[MAX_KEY_LENGTH];
    String str(inputLangName);
    SFstrcpy(utf8Buf, MAX_KEY_LENGTH, str.ToCStr());

    InputLangProps props;

    props.ItemTag = GFxIME_NotSupported;

    if ((langProfile.langid & 0x00000FFF) == 0x409)
        props.ItemTag = GFxIME_English;

    if ((langProfile.langid & 0x00000FFF) == 0x404)
        props.ItemTag = GFxIME_Ch_Trad;

    if ((langProfile.langid & 0x00000FFF) == 0x804)
        props.ItemTag = GFxIME_Ch_Simp;

    if ((langProfile.langid & 0x00000FFF) == 0x412)
        props.ItemTag = GFxIME_Kr;

    if ((langProfile.langid & 0x00000FFF) == 0x411)
        props.ItemTag = GFxIME_Jp;

    props.Id = langProfile.langid;
	CurrentInputLangTag = props.ItemTag;
    props.ItemNameOnSystem = (char*)SF_ALLOC(str.GetSize() + 1, StatIME_Mem);
    memcpy((void*)props.ItemNameOnSystem, utf8Buf, str.GetSize() + 1);
    SupportedInputLanguages.PushBack(props);
    InstalledInputLangs += String(inputLangName) + "@@";
    NumLanguages++;

/*
    char utf8Buf[MAX_KEY_LENGTH];
    SFstrcpy(utf8Buf, MAX_KEY_LENGTH, String(inputLangName).ToCStr());

    for(UInt32 j = 0; j < NumLanguages; j++)
    {
        // If this Input Language is supported, add the lang id in the Input Language Name table
        if(!SFstricmp(SupportedInputLanguages[j].ItemNameOnSystem, utf8Buf))
        {
            // We use the LocaleId member of the SupportedIMEs data structure to store
            // the language profile info. This info will be used later in ActivateIME to 
            // switch to a desired IME. 
            SupportedInputLanguages[j].Id = langProfile.langid;
            InstalledInputLangs += String(inputLangName) + "@@"; // a delimiter very unlikely to be in the name of a language
        }
    }
*/
}

void GImeNamesManagerVista::CheckForSupportedIME(TF_INPUTPROCESSORPROFILE& langProfile, const char* imeName)
{
    // Check to see if this IME is supported

    //  char utf8BufLt[MAX_KEY_LENGTH];
    //  char key[MAX_KEY_LENGTH];

    //  SFstrcpy(key, MAX_KEY_LENGTH, String(imeName).ToCStr());
    //  SFstrcpy(utf8BufLt, MAX_KEY_LENGTH, String(imeName).ToCStr());

    // Which input language does this ime belong to?
    GFxIMETag inputLangTag;
    inputLangTag = GetLangNamefromLangId(langProfile.langid);

    for(UInt32 j = 0; j < NumIme; j++)
    {
        // If this IME is supported, put the IME key in the hash table

        // Note: different input languages can have the same IME name associated with them.
        // For example, both Japanese and Korean input languages have "Microsoft IME" as the
        // associated IME. This means that IMEName can't used as a key. It must be combined
        // with LangId to form a unique Id. 

        // Since IMEName is not unique, we check for the language tag match as well. 

        if((!SFstricmp(SupportedIMEs[j].ItemNameOnSystem, imeName)) && 
            (inputLangTag == GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)))
        {

            TF_INPUTPROCESSORPROFILE* profile = (TF_INPUTPROCESSORPROFILE*)SF_ALLOC(sizeof(TF_INPUTPROCESSORPROFILE), StatIME_Mem);
            // We use the LocaleId member of the SupportedIMEs data structure to store
            // the language profile info. This info will be used later in ActivateIME to 
            // switch to a desired IME. 
            memcpy(profile, (void*)(&langProfile), sizeof(TF_INPUTPROCESSORPROFILE));
            SupportedIMEs[j].Id = (SPInt)(profile);
            
            ImeFileNameOrLayoutText[j] = 1;
            //  if (profile->guidProfile.Data1 == 0)
            // This means that this profile doesn't correspond to an IME, but an
            // input language. Therefore, don't put it in the list of IME's for any language.
            //      continue;

            if (profile->dwFlags & TF_IPP_FLAG_ACTIVE)
            {
                CurrentIMETag       = SupportedIMEs[j].ItemTag;
                CurrentInputLangTag = inputLangTag;
            }

            if (inputLangTag & (UInt32)GFxIME_Jp_Flag)
            { 
                JapaneseIMEs.PushBack(String(SupportedIMEs[j].ItemNameOnSystem));
            }  

            if (inputLangTag & (UInt32)GFxIME_Kr_Flag)
            { 
                KoreanIMEs.PushBack(String(SupportedIMEs[j].ItemNameOnSystem));
            } 

            if (inputLangTag & (UInt32)GFxIME_Ch_Simp_Flag)
            { 
                ChineseSimpIMEs.PushBack(String(SupportedIMEs[j].ItemNameOnSystem));
            } 

            if (inputLangTag & (UInt32)GFxIME_Ch_Trad_Flag)
            { 
                ChineseTradIMEs.PushBack(String(SupportedIMEs[j].ItemNameOnSystem));
            } 
        }
    }
} 

#ifndef GFC_NO_LANGBAR_SUPPORT

void GImeNamesManagerVista::SetConversionMode(UInt32 fdwConversion)
{
    SF_UNUSED(fdwConversion);
    // do nothing
    GImeNamesManager::SetConversionMode();
}

void GImeNamesManagerVista::ActivateInputLanguage(const char* inputLangName)
{
    HRESULT hr;
    ITfInputProcessorProfiles *pProfiles;

    // First obtain the entry in SupportedIME's corresponding to this ime
    int imeIndex = -1;
    for(UInt32 j = 0; j < NumLanguages; j++)
    {
		// It's ok to use strcmp here since we are matching against the inputLangName is exactly what we have stored in
		// our lang bar data structure. Therefore, case insensitive comparison is not needed. 
        if( (!strcmp(SupportedInputLanguages[j].ItemNameOnSystem, inputLangName)) && 
            (SupportedInputLanguages[j].Id != 0)) // The second condition precludes non-supported IMEs
        {
            imeIndex = j;
            break;
        }
    }


	if (imeIndex == -1)
	{
		pImeMgr->pLog->LogMessage("\nCould not find requested IME in supported IME list!");

		pImeMgr->pLog->LogMessage(String("\n--> You requested: ") + inputLangName);
		return;
	}
   
    hr = CoCreateInstance(  CLSID_TF_InputProcessorProfiles, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_ITfInputProcessorProfiles, 
        (LPVOID*)&pProfiles);

	if(hr==0x800401f0) //CoCreateInstance never called (this happens when using different threads!)
	{
		pImeMgr->pLog->LogMessage(String("\n Failure in COM, perhaps you called CoCreateInstance in another thread... "));
	}

    if(SUCCEEDED(hr) && pProfiles)
    {
        hr = pProfiles->ChangeCurrentLanguage(LANGID(SupportedInputLanguages[imeIndex].Id));
        pProfiles->Release();
    }
}

// The idea here is that when the user clicks on an input language tab, we check in our list 
// to see if there are ime's associated with that input language. If there are, we present a list 
// of input languages to the user. The user then selects the desired input language which causes
// the fscommand "LangBar_OnIME" to be fired which calls ActivateIME function. 
// Now if there are no ime's associated with a selected input language, that input language 
// should be activated directly. So this function can be used to switch to a desired IME
// as well as an input language
void GImeNamesManagerVista::ActivateIME(const char* imeName)
{
    HRESULT hr;
    ITfInputProcessorProfiles *pProfiles;

    // First obtain the entry in SupportedIME's corresponding to this ime
    int imeIndex = -1;
	for(UInt32 j = 0; j < NumIme; j++)
	{

		// check if need to match against ime file name or layout name
		// Matching the InputLangTag is needed because IME's of different input languages can have 
		// the same name. For example, "Microsoft IME" for Japanese and Korean IMEs.
		if (ImeFileNameOrLayoutText[j] == 1)
		{
			if((!strcmp(imeName, SupportedIMEs[j].ItemNameOnSystem)) && 
				(CurrentInputLangTag == GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)) && 
				(SupportedIMEs[j].Id != 0)) // The second condition precludes non-supported IMEs
			{
				imeIndex = j;
				break;
			}
		}
		else if (ImeFileNameOrLayoutText[j] == 2)
		{
			if ((!strcmp(imeName, SupportedIMEs[j].ItemNameCommon)) && 
				(CurrentInputLangTag == GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)) && 
				(SupportedIMEs[j].Id != 0)) 
			{
				imeIndex = j;
				break;
			}
		}
	}
	/*
	if( (!strcmp(SupportedIMEs[j].ItemNameOnSystem, imeName)) &&
	(CurrentInputLangTag == GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag)) && 
	(SupportedIMEs[j].Id != 0)) // The second condition precludes non-supported IMEs
	{
	imeIndex = j;
	break;
	}
	*/


    if (imeIndex == -1) return ;

    hr = CoCreateInstance(  CLSID_TF_InputProcessorProfiles, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_ITfInputProcessorProfiles, 
        (LPVOID*)&pProfiles);

    if(SUCCEEDED(hr) && pProfiles)
    {
        ITfInputProcessorProfileMgr *pprofilesMgr;
        hr = pProfiles->QueryInterface(__uuidof(ITfInputProcessorProfileMgr), (LPVOID*)&pprofilesMgr);
        if (SUCCEEDED(hr) && pprofilesMgr) 
        {
			ProfileInfo*				profileInfo	= (ProfileInfo*)(SupportedIMEs[imeIndex].Id);
			TF_INPUTPROCESSORPROFILE*	profile		= (TF_INPUTPROCESSORPROFILE*)(profileInfo->profile);
			
			if (profile)
				pprofilesMgr->ActivateProfile(profile->dwProfileType, profile->langid,
					profile->clsid, profile->guidProfile, profile->hkl, TF_IPPMF_FORPROCESS);
            
            pprofilesMgr->Release();
        }
        pProfiles->Release();
    }
}

// Setting the language name is done in the derived class since we use SetLocaleInfo for XP and SetLocaleInfoEx for Vista
// as recommended in the documentation. For setting the IMEname, the base class function is called.
void    GImeNamesManagerVista::OnLangBarLoaded()
{
    wchar_t szLangName[MAX_PATH];

    Un.hkl = GetKeyboardLayout(0);
    CurrentInputLangTag = (GFxIMETag)GetLangNamefromLangId((UInt32)(0x0000FFFF&Un.val));
    pImeMgr->SetCurrentInputLanguage(CurrentInputLangTag);
//  if ( GetLocaleInfoEx(MAKELCID(un.val, SORT_DEFAULT), LOCALE_SLANGUAGE, szLangName, MAX_PATH) == 0 )
    // Should be using the Ex version, but it's not declared on WinXP. Can add the declaration, but will have
    // to test there is no incompatibility with Vista etc. Using the old version for now. Look at the header
    // file for a sample declaration.
    if ( GetLocaleInfoW(MAKELCID(Un.val, SORT_DEFAULT), LOCALE_SLANGUAGE, szLangName, MAX_PATH) == 0 )
    {
        DWORD err = GetLastError();
        LogError(err, "GFxIME Error: Error Initializing Input Language Name ");
        pImeMgr->BroadcastSetCurrentInputLanguage("Unsupported");
    }
    else
    {
        pImeMgr->BroadcastSetCurrentInputLanguage(String(szLangName).ToCStr());

        // Note that the CurrentIMETag should always tell us what the current IME is. If the application is 
        // just starting off, it's set in HashIMENames, otherwise it's set whenever the language is changed 
        // and OnActivated is called.
        if (CurrentIMETag == GFxIME_NotSupported)
        {
        //    GImeNamesManager::SetLastIMEName(String(szLangName));
			GImeNamesManager::SetLastIMEName(String("Not Supported"));
        }
		if (CurrentIMETag == GFxIME_DoesntExist)
		{
			GImeNamesManager::SetLastIMEName(String(szLangName));
			// GImeNamesManager::SetLastIMEName(String(""));
		}
        else
        {
            // Set IME Name in the base class.
            GImeNamesManager::OnLangBarLoaded();
        }
    }
}

void GImeNamesManagerVista::HandleStatusWindowNotifications(const char* pcommand, const char* parg)
{

    DWORD   fdwConversion;
    VARIANT var;
    HIMC    hIMC;

    /*
    if(pCompartment == NULL) return;

    if (SUCCEEDED(pCompartment->GetValue(&var))) 
    {
        if (var.vt == VT_I4) 
        {
            fdwConversion = var.lVal;
        }
        else
            return;
    }
    else
        return;
    */
    hIMC = ImmGetContext(pImeMgr->hWnd);
    
    DWORD conv, sentence;
    ImmGetConversionStatus(hIMC, &fdwConversion, &sentence);

    if (hIMC == NULL) return;

    if(SFstrcmp(pcommand, "StatusWindow_OnShape") == 0)
    {
        if (!strcmp(parg, "false"))
        {
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_FULLSHAPE); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_FULLSHAPE; 
        }

    }

    else if(SFstrcmp(pcommand, "StatusWindow_OnInputMode") == 0)
    {
        // Pertains to Chinese IME only
        if (!strcmp(parg, "false"))
        {
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_NATIVE); 
            conv = conv & (~IME_CMODE_NATIVE); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_NATIVE;
            conv = conv | IME_CMODE_NATIVE; 
        }

        // Pertains to Japanese IME.

        if (!strcmp(parg, "Hiragana"))
        {
            // If in direct input mode, first turn IME on. Too bad that we have 
            // to use IMM function here. Doesn't seem to be a way to turn IME on/off
            // using TSF.
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_KATAKANA); 
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_NATIVE;
        }

        if (!strcmp(parg, "Full-Width Katakana"))
        {         
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_KATAKANA | TF_CONVERSIONMODE_FULLSHAPE | TF_CONVERSIONMODE_NATIVE; 
        }

        if (!strcmp(parg, "Full-Width Alphanumeric"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_NATIVE); 
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_FULLSHAPE; 
        }

        if (!strcmp(parg, "Half-Width Katakana"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_KATAKANA | TF_CONVERSIONMODE_NATIVE;
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_FULLSHAPE);
        }

        if (!strcmp(parg, "Half-Width Alphanumeric"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_NATIVE);
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_FULLSHAPE);
        }

        if (!strcmp(parg, "DirectInput"))
        {

            if (ImmGetOpenStatus(hIMC) != 0)
            {
                ImmSetOpenStatus(hIMC, false);
            }

        }
    }

    else if(SFstrcmp(pcommand, "StatusWindow_OnSymbol") == 0)
    {
        if (!strcmp(parg, "false"))
        {
            fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_SYMBOL); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | TF_CONVERSIONMODE_SYMBOL; 
        }
    }

    var.lVal = fdwConversion;
    
    ImmSetConversionStatus(hIMC, fdwConversion, sentence);
//  pCompartment->SetValue(pClientId, &var);
    ImmReleaseContext(pImeMgr->hWnd, hIMC);

}

#endif
}}}
#endif
