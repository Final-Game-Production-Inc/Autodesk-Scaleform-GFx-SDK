/**************************************************************************

Filename    :   GFx_IMENamesManagerXP.cpp
Content     :   Overrides the functions used to obtain names of installed ime's and 
                activating ime's according to platform specific implementation.
Created     :   Oct 01, 2008
Authors     :   Ankur Mohan

Notes       :   First part of this file contains the function implementations for 
                querying the registry for IME names and checking which IME's are supported
                The second part contains the functions used for implementing the language bar.
History     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFxConfig.h"
#ifndef SF_NO_IME_SUPPORT

#include "GFx_IMENamesManagerXP.h"
#include "GFx_IMEManagerWin32.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

extern InputLangProps SupportedIMEs[];
extern int ImeFileNameOrLayoutText[];


bool GImeNamesManagerXP::HashIMENames(void)
{
	HRESULT hr;
	ITfInputProcessorProfiles   *pProfiles;
	TF_LANGUAGEPROFILE          langProfile;
	CurrentIMETag               = GFxIME_NotSupported;
	CurrentInputLangTag         = GFxIME_NotSupported;
	wchar_t						szLangName[MAX_PATH];
	HKL* hkls;

	UInt32 numInstalledIMEs = GetKeyboardLayoutList(0, 0);

	if (numInstalledIMEs == 0)
		LogError("No IME's Installed!", "GFxIME Error");

	hkls = (HKL*)SF_ALLOC(sizeof(HKL)*numInstalledIMEs, StatIME_Mem);
	// Get a list of installed Keyboard layouts. This gives us a list of HKLS for ime's 
	// installed on the system. Very valuable info!
	GetKeyboardLayoutList(numInstalledIMEs, hkls);

	for (UInt32 i = 0; i < numInstalledIMEs; i++)
	{
		// This is needed since for 64 bit builds, the last 8 bytes are not filled out 
		// correctly. 
		Un.hkl  = hkls[i];
		Un.val  = Un.val & 0xFFFFFFFF;
		hkls[i] = Un.hkl;
	}
	// The above method doesn't give us the ime names though. Using GetKeyboardLayoutName doesn't help
	// The only way (according to Michael Kaplan's blog) is to look into the registry for the hkls
	bool ret = 1;
	ret = GImeNamesManager::MakeKeyboardLayoutListFromRegistry(hkls, numInstalledIMEs);

	SF_FREE(hkls);

	if (ret) return ret; // Some problem with querying the registry- abort.

	for (UInt32 i = 0; i < NumIme; i++) ImeFileNameOrLayoutText[i] = 0;
	
	//Create the object.
	hr = CoCreateInstance( CLSID_TF_InputProcessorProfiles, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_ITfInputProcessorProfiles, 
		(LPVOID*)&pProfiles);

	if (!SUCCEEDED(hr))
	{
		// Not sure if we should record an error message here..
		LogError(hr, "GFxIME Error: Failure to Initialize InputProcessorProfiles!");
	}
	// InputProcessorProfileMgr seems not available on Windows XP. So need to use 
	// InputProcessorProfiles.
	//  hr = pProfiles->QueryInterface(IID_ITfInputProcessorProfileMgr_GFx, (LPVOID*)&pProfilesMgr);

	//	AM (2 April 09): Note: I just discovered that there exist XP systems on which trying to obtain ITfInputProcessorProfiles can
	//	fail. If that happens, we need to have an alternative way of obtaining installed input languages. 

	if(SUCCEEDED(hr) && pProfiles)
	{
		LANGID  *rgLangIDs;
		ULONG   ulCount;
		//  IEnumTfLanguageProfiles* ppEnum;
		IEnumTfLanguageProfiles* ppEnum;
		hr = pProfiles->GetLanguageList(&rgLangIDs, &ulCount);

		if(SUCCEEDED(hr))
		{
			ULONG   i;

			for(i = 0; i < ulCount; i++)
			{
				
				if( 0 != GetLocaleInfoW(MAKELCID(rgLangIDs[i], SORT_DEFAULT), 
					LOCALE_SLANGUAGE, 
					szLangName, 
					MAX_PATH))
				{
					pProfiles->EnumLanguageProfiles(rgLangIDs[i], &ppEnum);

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

					// Also, note that only a few ime's register themselves as language profiles as listed
					// by EnumLanguageProfiles(). For most of the others (including all of the third party ime's) 
					// we have to go through the list of hkl's in the registry as done later.

					CheckForSupportedInputLang(langProfile, szLangName); 

					// This only works with registering system IME's unfortunately. 
					// Currently deciding to rely solely on registry lookup. 

					BSTR profileName = SysAllocString(NULL);
					String profileNameStr;
					if (ppEnum)
					{
						while (ppEnum->Next(1, &langProfile, NULL) != S_FALSE)
						{
							pProfiles->GetLanguageProfileDescription(langProfile.clsid, langProfile.langid, langProfile.guidProfile, &profileName);

							//  profileNameStr = (wchar_t*)profileName;
							//  if (!profileNameStr.IsEmpty())
							//      CheckForSupportedIME(langProfile, profileNameStr.ToCStr()); // Check if we support this IME. If we do, put it in the hash table.

							if (SysStringLen(profileName) != 0)
							{
								SysFreeString(profileName);
								profileName = NULL;
							}
						}

						ppEnum->Release();
					}
				}
			}
            CoTaskMemFree(rgLangIDs);
		}
	}
	// This is the alternative way: The idea is to look at the installed hkls in the HklLayoutTextMap and 
	// mask out the higher 16 bits (that correspond to the IME/LayoutName). We match these against the known
	// values for the languages we support. The ones that match are added to the SupportedInputLanguages array.
	// Note that we need to make sure that each inputlanguage is only added once since each input language can
	// have multiple IMEs. Also, if languages other than English, Chinese (simp), Chinese (trad), Korean, Jap
	// are installed, they will not be detected using this method (while they will be using the inputprocessorprofiles
	// based method used above). Right now this doesn't seem like a big deal.
	else 
	{
		// Use alternative method to enumerate installed input languages. 
		for (UInt32 i = 0; i < HklLayoutTextMap.GetSize(); i++)
		{
			if (HklLayoutTextMap[i].IsInstalled)
			{
				InputLangProps props;

				props.ItemTag = GFxIME_NotSupported;
				String str;

				if ((HklLayoutTextMap[i].hkl & 0x00000FFF) == 0x409)
				{
					props.ItemTag	= GFxIME_English;
					str				= "English";	
				}

				if ((HklLayoutTextMap[i].hkl & 0x00000FFF) == 0x404)
				{
					props.ItemTag = GFxIME_Ch_Trad;
					str				= "Chinese (Traditional)";
				}

				if ((HklLayoutTextMap[i].hkl & 0x00000FFF) == 0x804)
				{
					props.ItemTag = GFxIME_Ch_Simp;
					str				= "Chinese (Simplified)";
				}

				if ((HklLayoutTextMap[i].hkl & 0x00000FFF) == 0x412)
				{
					props.ItemTag	= GFxIME_Kr;
					str				= "Korean";
				}

				if ((HklLayoutTextMap[i].hkl & 0x00000FFF) == 0x411)
				{
					props.ItemTag	= GFxIME_Jp;
					str				= "Japanese";
				}

				if (props.ItemTag != GFxIME_NotSupported)
				{
					bool hasBeenAdded = false;
					// Check if this input language has already been added
					for (UInt32 j = 0; j < SupportedInputLanguages.GetSize(); j++)
					{
						if (SupportedInputLanguages[j].ItemTag == props.ItemTag)
						{
							hasBeenAdded = true;
							break;
						}
					}
					if (hasBeenAdded) continue;

					props.Id = (HklLayoutTextMap[i].hkl & 0x00000FFF);
					props.ItemNameOnSystem = (char*)SF_ALLOC(str.GetSize() + 1, StatIME_Mem);
					memcpy((void*)props.ItemNameOnSystem, str.ToCStr(), str.GetSize() + 1);
					SupportedInputLanguages.PushBack(props);
					InstalledInputLangs += str + "@@";
					NumLanguages++;
				}
			}
		}
	}

	// Below we are going to match the imeName and LayoutName info in the HklLayoutTextMap against
	// what's stored in the SupportedIMEs array. Note that on XP this is done differently than on Vista.
	// On Vista, the system provides a list of IMe's for each input language, while on XP, we obtain the list
	// of input languages (using the two methods described above) first and then make a list of supported IMEs.
	// The two are done seperately and it's not possible for the LayoutNames (as far as I can tell) to be the same
	// so the extra check for the currentinputlanguage (check implementation of CheckForSupportedIMEs in NamesManager)
	// is not needed.

	HKL currentKbLayout = GetKeyboardLayout(0);

	int j = -1;
	// Now compare what we found in the system registry with the supported IME's and fill in the HKL values,
	// so we can switch to a desired language/IME from our language bar. 
	// We first check against the layoutname and if no match is found for it, we check for the IMEFilename. 
	// Also, it's important to remember that the CurrentInputLangTag must be set correctly before 
	// CheckForSupportedIME is called. 
	for (UInt32 i = 0; i < HklLayoutTextMap.GetSize(); i++)
	{
		if (! HklLayoutTextMap[i].IsInstalled) continue;

		j = CheckForSupportedIME(HklLayoutTextMap[i].LayoutName, HklLayoutTextMap[i].ImeFileName);

		if (j != -1)
		{
			SupportedIMEs[j].Id = HklLayoutTextMap[i].hkl;

			Un.val = HklLayoutTextMap[i].hkl;

			if (Un.hkl == currentKbLayout)
			{
				CurrentIMETag   = SupportedIMEs[j].ItemTag; 
			}

		}
	}

	// Set currently active language
	LANGID activeLangId;
	if (pProfiles)
		pProfiles->GetCurrentLanguage(&activeLangId);
	//else ?

	Un.hkl = GetKeyboardLayout(0);
	Un.val = Un.val & 0x00000FFF;

	CurrentInputLangTag = GFxIME_NotSupported;

	for (UInt32 langIdx = 0; langIdx < NumLanguages; langIdx++)
	{
		if ((size_t)SupportedInputLanguages[langIdx].Id == Un.val)
		{
			CurrentInputLangTag = SupportedInputLanguages[langIdx].ItemTag; 
		}
	}

	return false;
}

int GImeNamesManagerXP::CheckForSupportedIME(const char* layoutTextName, const char* imeFileName)
{
	String name;
	bool foundMatch = false;
	UInt32 j = 0;
	GFxIMETag tag = GFxIME_NotSet;

	// First match against LayoutName field, if not match is found, try ImeFileName. 
	for(j = 0; j < NumIme; j++)
	{
		if (layoutTextName && !SFstricmp(layoutTextName, SupportedIMEs[j].ItemNameOnSystem) && (ImeFileNameOrLayoutText[j] == 0))
		{
			name = SupportedIMEs[j].ItemNameOnSystem;
			ImeFileNameOrLayoutText[j] = 1;
			foundMatch = true;
			break;
		}
	}

	if (foundMatch != true)
	{
		for(j = 0; j < NumIme; j++)
		{
			if (imeFileName && !SFstricmp(imeFileName, SupportedIMEs[j].ItemNameOnSystem))
			{
				name = SupportedIMEs[j].ItemNameCommon;
				ImeFileNameOrLayoutText[j] = 2;
				foundMatch = true;
				tag = SupportedIMEs[j].ItemTag;
				break;
			}			
		}

		for(UInt32 jj = 0; jj < NumIme; jj++)
		{
			if ((ImeFileNameOrLayoutText[jj] == 1) && (SupportedIMEs[jj].ItemTag == tag))
			{
				foundMatch = false;
				break;
			}
		}
	}

	if (foundMatch == true )
	{
		GFxIMETag inputLangTag = GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag);

		if (inputLangTag & (UInt32)GFxIME_Jp_Flag)
		{ 
			JapaneseIMEs.PushBack(name);
		}  

		if (inputLangTag & (UInt32)GFxIME_Kr_Flag)
		{ 
			KoreanIMEs.PushBack(name);
		} 

		if (inputLangTag & (UInt32)GFxIME_Ch_Simp_Flag)
		{ 
			ChineseSimpIMEs.PushBack(name);
		} 

		if (inputLangTag & (UInt32)GFxIME_Ch_Trad_Flag)
		{ 
			ChineseTradIMEs.PushBack(name);
		} 
		return j;
	}

	return -1;
}

GFxIMETag GImeNamesManagerXP::GetImeTag(const char* name)
{   
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

GImeStringKey GImeNamesManagerXP::GetKey()
{
    char layoutName[KL_NAMELENGTH];
    ::GetKeyboardLayoutNameA((layoutName));
    int             length = (int)strlen(layoutName);
    GImeStringKey   key(layoutName, IMEHashFunction((char*)layoutName, length), length);
    return key;
}

void GImeNamesManagerXP::CheckForSupportedInputLang(TF_LANGUAGEPROFILE& langProfile, const wchar_t* inputLangName)
{
   
    String str(inputLangName);
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
	memcpy((void*)props.ItemNameOnSystem, str.ToCStr(), str.GetSize() + 1);
    SupportedInputLanguages.PushBack(props);
    InstalledInputLangs += String(inputLangName) + "@@";
    NumLanguages++;

/*  for(UInt32 j = 0; j < NumLanguages; j++)
    {
        // If this Input Language is supported, set the Id field in the corresponding entry of the SupportedInputLanguages
        // data structure.
        if(!SFstricmp(SupportedInputLanguages[j].ItemNameOnSystem, utf8Buf))
        {
            SupportedInputLanguages[j].Id = langProfile.langid;
            InstalledInputLangs += String(inputLangName) + "@@";
            NumInstalledInputLang++;
            break;
        }
    
        if(SupportedInputLanguages[j].Id == (langProfile.langid & 0x00000FFF))
        {
            SupportedInputLanguages[j].ItemNameOnSystem = (char*)SF_ALLOC(str.GetSize() + 1);
            memcpy((void*)SupportedInputLanguages[j].ItemNameOnSystem, utf8Buf, str.GetSize() + 1);
            
            InstalledInputLangs += str + "@@";
            NumInstalledInputLang++;
            break;
        }
    }
*/
}

/************************************ Language Bar Functions ******************************************/

#ifndef GFC_NO_LANGBAR_SUPPORT

// Setting the language name is done in the derived class since we use SetLocaleInfo for XP and SetLocaleInfoEx for Vista
// as recommended in the documentation. For setting the IMEname, the base class function is called.
void    GImeNamesManagerXP::OnLangBarLoaded()
{
    wchar_t szLangName[MAX_PATH];

    Un.hkl = GetKeyboardLayout(0);
    CurrentInputLangTag = (GFxIMETag)GetLangNamefromLangId((UInt32)(0x0000FFFF&Un.val));
    pImeMgr->SetCurrentInputLanguage(CurrentInputLangTag);
    if ( GetLocaleInfoW(MAKELCID(Un.val, SORT_DEFAULT), LOCALE_SLANGUAGE, szLangName, MAX_PATH) == 0 )
    {
        DWORD err = GetLastError();
        LogError(err, "GFxIME Error: Error Initializing Input Language Name ");
        pImeMgr->BroadcastSetCurrentInputLanguage("Unsupported");
    }
    else
    {
        pImeMgr->BroadcastSetCurrentInputLanguage(String(szLangName).ToCStr());

        // Here we try to distinguish between a keyboard layout and an IME
        // on XP, the HKL for IME's seem to start with an 'E'..This is not always
        // true so need to be careful here. 
        // So, if the langid is not an IME, we just set the IME name to be the same
        // as the input language name.
        if ((Un.val & 0xE0000000) == 0)
        {
            CurrentIMETag = GFxIME_NotSupported;
            UInt32 numInstalledHKls = (UInt32)HklLayoutTextMap.GetSize();
            for (UInt32 i = 0; i < numInstalledHKls; i++)
            {
                if (((Un.val&0xFFFF0000) >> 16) == HklLayoutTextMap[i].hkl)
                {
                    GImeNamesManager::SetLastIMEName(String(HklLayoutTextMap[i].LayoutName));
                    return; 
                }
            }
            GImeNamesManager::SetLastIMEName(String(szLangName));
        }
        else
        {
            // Set IME Name in the base class.
            GImeNamesManager::OnLangBarLoaded();
        }
    }
}

// Activates a requested IME if supported
void GImeNamesManagerXP::ActivateIME(const char* imeName)
{
    bool foundMatch = false;
    
    Un.val = 0; // Initialize to avoid warning

    for(UInt32 j = 0; j < NumIme; j++)
    {
        // check if need to match against ime file name or layout name
        if (ImeFileNameOrLayoutText[j] == 1)
        {
            if((!strcmp(imeName, SupportedIMEs[j].ItemNameOnSystem)) && (SupportedIMEs[j].Id != 0)) // The second condition precludes non-supported IMEs
            {
                foundMatch = true;
                Un.val = SupportedIMEs[j].Id;
                break;
            }
        }
        else if (ImeFileNameOrLayoutText[j] == 2)
        {
            if ((!strcmp(imeName, SupportedIMEs[j].ItemNameCommon)) && (SupportedIMEs[j].Id != 0)) 
            {
                foundMatch = true;
                Un.val = SupportedIMEs[j].Id;
                break;
            }
        }
    }
            
    if (foundMatch)
        if (ActivateKeyboardLayout(Un.hkl, 0) == 0)
        {
			LogError((String("Failure to activate input language ") + imeName).ToCStr(), "GFxIME Error");
        }
}

// Activates a requested input language if supported
void    GImeNamesManagerXP::ActivateInputLanguage(const char* inputLangName)
{
    // First obtain the entry in SupportedIME's corresponding to this ime
    int imeIndex = -1;

    for (UInt32 j = 0; j < NumLanguages; j++)
    {
        if( (!strcmp(SupportedInputLanguages[j].ItemNameOnSystem, inputLangName)) && 
            (SupportedInputLanguages[j].Id != 0)) // The second condition precludes non-supported IMEs
        {
            imeIndex = j;
            break;
        }
    }


    if (imeIndex == -1) return ;

    Un.val = SupportedInputLanguages[imeIndex].Id;
    ActivateKeyboardLayout(Un.hkl, 0);
}

extern StringToTagDef StringToTag[];

bool GImeNamesManagerXP::SwitchIME(String imeName)
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
                    if (ImeFileNameOrLayoutText[j] == 1)
                    {
						// Need to set the current input language since the IME name could be the same for different IMEs
						// For example- Microsoft IME for Korean and Japanese
					//	CurrentInputLangTag = GetInputLangTagfromImeTag(SupportedIMEs[j].ItemTag);
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

void GImeNamesManagerXP::HandleStatusWindowNotifications(const char* pcommand, const char* parg)
{

    DWORD   fdwConversion, fdwSentence;
    HIMC    hIMC;

    hIMC = ImmGetContext(pImeMgr->hWnd);

    ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);

    if (hIMC == NULL) return;

    if(SFstrcmp(pcommand, "StatusWindow_OnShape") == 0)
    {
        if (!strcmp(parg, "false"))
        {
            fdwConversion = fdwConversion & (~IME_CMODE_FULLSHAPE); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | IME_CMODE_FULLSHAPE; 
        }

    }

    else if(SFstrcmp(pcommand, "StatusWindow_OnInputMode") == 0)
    {
        // Pertains to Chinese IME only
        if (!strcmp(parg, "false"))
        {
            fdwConversion = fdwConversion & (~IME_CMODE_NATIVE); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | IME_CMODE_NATIVE;
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
            fdwConversion = fdwConversion & (~IME_CMODE_KATAKANA); 
            fdwConversion = fdwConversion | IME_CMODE_NATIVE;
        }

        if (!strcmp(parg, "Full-Width Katakana"))
        {         
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion | IME_CMODE_KATAKANA | IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE; 
        }

        if (!strcmp(parg, "Full-Width Alphanumeric"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion & (~IME_CMODE_NATIVE); 
            fdwConversion = fdwConversion | IME_CMODE_FULLSHAPE; 
        }

        if (!strcmp(parg, "Half-Width Katakana"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion | IME_CMODE_KATAKANA | IME_CMODE_NATIVE;
            fdwConversion = fdwConversion & (~IME_CMODE_FULLSHAPE);
        }

        if (!strcmp(parg, "Half-Width Alphanumeric"))
        {
            if (ImmGetOpenStatus(hIMC) == 0)
            {
                ImmSetOpenStatus(hIMC, true);
            }
            fdwConversion = fdwConversion & (~IME_CMODE_NATIVE);
            fdwConversion = fdwConversion & (~IME_CMODE_FULLSHAPE);
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
            fdwConversion = fdwConversion & (~IME_CMODE_SYMBOL); 
        }

        if (!strcmp(parg, "true"))
        {
            fdwConversion = fdwConversion | IME_CMODE_SYMBOL; 
        }
    }

    ImmSetConversionStatus(hIMC, fdwConversion, fdwSentence);
    ImmReleaseContext(pImeMgr->hWnd, hIMC);

}

#endif
}}}
#endif
