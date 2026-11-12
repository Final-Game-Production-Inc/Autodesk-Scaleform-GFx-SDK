/**************************************************************************

Filename    :   GFx_IMECommands.cpp
Content     :   Contains implementation for IME commands.
Created     :   OCt 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
// Important Note for Status Window implementation on Vista:
// The IMM mechanism to handle status window features is broken for many IME's
// but not all. From what I've investigated so far, IMM works for Chinese Trad Phonetic,
// New Phonetic, ChangJie and New Changjie. It doesn't work for DaYi (6.0). For the 
// Chinese Simplified IME's, it works for MS Pinyin, but not for Quan Pin (6.0), 
// Shuang Pin (6.0) and Zheng Ma (6.0). So it seems it's broken for all the 6.0 IMEs.

// Just discovered that using TSF doesn't help with the above problem, and a google search
// reveals that this is a problem acknowledged by microsoft. A hot fix is available at:
// http://support.microsoft.com/kb/944515
// haven't tested this yet.

#include "GFxConfig.h"
#ifndef SF_NO_IME_SUPPORT

#include "GFx_IMEManagerWin32.h"
#include "GFx_IMEWin32Impl.h"
#include "GFx_IMEIdMap.h"


namespace Scaleform {
	namespace GFx {
		namespace IME{

extern InputLangProps SupportedIMEs[];

void GFxIMEManagerWin32::SendLangBarMessage(DisplayObject* mc, const ASString& _command, const ASString& msg)
{ 
	const char* pmessage = msg.GetBuffer();
	const char* pcommand = _command.GetBuffer();

	if(SFstrcmp(pcommand, "LangBar_OnInit") == 0)
	{
		GetASIMEManager()->pLangContext2 = mc;
		if (pIMENamesMgr)			
			pIMENamesMgr->OnLangBarLoaded();

	}

	else if(SFstrcmp(pcommand, "StatusWindow_OnInit") == 0)
	{
		GetASIMEManager()->pStatusContext2 = mc;
		// Status window needs to know the currently active language 
		BroadcastSwitchLanguage(CurrentLanguage);
		// if (CurrentLanguage != "Japanese" && CurrentLanguage != "Korean")
		BroadcastDisplayStatusWindow();
		SetConversionMode(0xFFFFFFFF);

	}

	else if(SFstrcmp(pcommand, "LangBar_OnSelectInputLang") == 0)
	{
		// Close status window at this point since otherwise the newly created IME list might overlap the 
		// status window leading to dirty looking display. The status window for the new chosen IME
		// will be created when the openstatuswindow message is sent.

		//  if (StatusWindowPath.GetLength() != 0)
		//  pMovie->Invoke((StatusWindowPath + ".RemoveStatusWindow").ToCStr(), "%d", 0); 
		BroadcastRemoveStatusWindow();

		// Get the last ime/default ime for this input language and call SetLastIME() in ActionScript.
		// if the user is not satisfied with the default IME, he can click on the IME tab and will be presented
		// a list of the installed ime's for that input language.
		if (pIMENamesMgr)
			pIMENamesMgr->ActivateInputLanguage(pmessage);
	}

	else if(SFstrcmp(pcommand, "LangBar_GetIMENames") == 0)
	{
		//        OnFinalize(); 
		GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;

		switch (imeTag)
		{
		case GFxIME_Kr:
			{
				String imeNames;

				int numIme = (int)pIMENamesMgr->KoreanIMEs.GetSize();
				if (numIme == 0) // no ime's associated, activate input language directly
				{
					//  pIMENamesMgr->ActivateIME(parg);
				}
				else 
				{
					for (int i = 0; i < numIme; i++)
					{
						imeNames += pIMENamesMgr->KoreanIMEs[i] + "@@"; 
					}
					//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
					BroadcastSetSupportedIMEs(imeNames.ToCStr());
				}
			}
			break;

		case GFxIME_Ch_Simp:
			{
				String imeNames;

				int numIme = (int)pIMENamesMgr->ChineseSimpIMEs.GetSize();
				if (numIme == 0) // no ime's associated, activate input language directly
				{
					//  pIMENamesMgr->ActivateIME(parg);
				}
				else 
				{
					for (int i = 0; i < numIme; i++)
					{
						imeNames += pIMENamesMgr->ChineseSimpIMEs[i] + "@@"; 
					}
					//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme);
					BroadcastSetSupportedIMEs(imeNames.ToCStr());
				}
			}
			break;

		case GFxIME_Ch_Trad:
			{
				String imeNames;

				int numIme = (int)pIMENamesMgr->ChineseTradIMEs.GetSize();
				if (numIme == 0) // no ime's associated, activate input language directly
				{
					//  pIMENamesMgr->ActivateIME(parg);
				}
				else 
				{
					for (int i = 0; i < numIme; i++)
					{
						imeNames += pIMENamesMgr->ChineseTradIMEs[i] + "@@"; 
					}
					//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
					BroadcastSetSupportedIMEs(imeNames.ToCStr());
				}
			}
			break;

		case GFxIME_Jp:
			{
				String imeNames;

				int numIme = (int)pIMENamesMgr->JapaneseIMEs.GetSize();
				for (int i = 0; i < numIme; i++)
				{
					imeNames += pIMENamesMgr->JapaneseIMEs[i] + "@@"; 
				}
				//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
				BroadcastSetSupportedIMEs(imeNames.ToCStr());
			}

			break;

		case GFxIME_English:
			{
				BroadcastRemoveStatusWindow();
			}
			break;
		default:
			break;
		}
	}

	else if(SFstrcmp(pcommand, "LangBar_OnSelectIME") == 0)
	{
		// ActivateIME will activate the IME corresponding to the IMEName and return it's tag.
		if (pIMENamesMgr)
			pIMENamesMgr->ActivateIME(pmessage);
		GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;
		// This is needed, since for Chinese Traditional IMEs, languagechange notification is not sent
		// therefore, this notification is our only chance of adjusting the position of the status window
		if (imeTag == GFxIME_Ch_Trad)
			BroadcastDisplayStatusWindow();
		// For japanese and korean, the status window should be displayed only when the user has selected the IME
		// from the ime from the drop down list.
		if (CurrentLanguage == "Japanese" || CurrentLanguage == "Korean")
		{
			BroadcastDisplayStatusWindow();
			pIMENamesMgr->SetConversionMode();
		}
	}

	else if(SFstrcmp(pcommand, "LangBar_GetInstalledLanguages") == 0)
	{
		BroadcastSetSupportedLanguages(pIMENamesMgr->InstalledInputLangs.ToCStr());
		//  pMovie->Invoke((LanguageBarPath + ".SetSupportedLanguages").ToCStr(), "%s, %d", pIMENamesMgr->InstalledInputLangs.ToCStr(), pIMENamesMgr->NumInstalledInputLang); 
	}

	// Pass all the status window and language bar related commands to the
	// platform specific implementation
	else 
	{
		OnFinalize();
		if (pIMENamesMgr)
		{
		/*	
			if (args[1].IsBool())
			{
				bool bval = args[1].GetBool();
				if (bval)
					parg = "true";
				else
					parg = "false";

			}
		*/
			pIMENamesMgr->HandleStatusWindowNotifications(pcommand, pmessage);
		}
	}
};

void	GFxIMEManagerWin32::ProcessIMEMessage(const FunctionHandler::Params& params)
{
	Movie* pmovie	= params.pMovie;
	Value* args		= params.pArgs;

	if (params.ArgCount > 0)
	{
		const char* pcommand= args[0].GetString();
		const char* parg	= 0;
		if (params.ArgCount > 1)
		{
			if (args[1].IsString())
			{
				parg = args[1].GetString();
			}
		}
		
			

		if(SFstrcmp(pcommand, "CandidateList_OnSetItemDataMap") == 0)
		{
			if(pIME != NULL)
				pIME->FsCallBack(pmovie, pcommand, parg);
		}

		else if(SFstrcmp(pcommand, "CandidateList_OnItemSelect") == 0)
		{
			int rowNum;
			if (args[1].IsNumber())
			{
				rowNum = (int)args[1].GetNumber();
				SelectAndClose(rowNum);
			}			
		}

		else if(SFstrcmp(pcommand, "LangBar_OnInit") == 0)
		{
			if (params.ArgCount >=2 && !args[1].IsUndefined())
			{
				GetASIMEManager()->pLangContext = args[1];
				if (pIMENamesMgr)			
					pIMENamesMgr->OnLangBarLoaded();
			}
			
		}

		else if(SFstrcmp(pcommand, "StatusWindow_OnInit") == 0)
		{
			if (params.ArgCount >=1 && !args[1].IsUndefined())
			{
				GetASIMEManager()->pStatusContext = args[1];
				// Status window needs to know the currently active language 
				BroadcastSwitchLanguage(CurrentLanguage);
				// if (CurrentLanguage != "Japanese" && CurrentLanguage != "Korean")
				BroadcastDisplayStatusWindow();
				SetConversionMode(0xFFFFFFFF);
			}
			
		}

		else if(SFstrcmp(pcommand, "LangBar_OnSelectInputLang") == 0)
		{
			// Close status window at this point since otherwise the newly created IME list might overlap the 
			// status window leading to dirty looking display. The status window for the new chosen IME
			// will be created when the openstatuswindow message is sent.

			//  if (StatusWindowPath.GetLength() != 0)
			//  pMovie->Invoke((StatusWindowPath + ".RemoveStatusWindow").ToCStr(), "%d", 0); 
			BroadcastRemoveStatusWindow();

			// Get the last ime/default ime for this input language and call SetLastIME() in ActionScript.
			// if the user is not satisfied with the default IME, he can click on the IME tab and will be presented
			// a list of the installed ime's for that input language.
			if (pIMENamesMgr)
				pIMENamesMgr->ActivateInputLanguage(parg);
		}

		else if(SFstrcmp(pcommand, "LangBar_GetIMENames") == 0)
		{
			//        OnFinalize(); 
			GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;

			switch (imeTag)
			{
			case GFxIME_Kr:
				{
					String imeNames;

					int numIme = (int)pIMENamesMgr->KoreanIMEs.GetSize();
					if (numIme == 0) // no ime's associated, activate input language directly
					{
						//  pIMENamesMgr->ActivateIME(parg);
					}
					else 
					{
						for (int i = 0; i < numIme; i++)
						{
							imeNames += pIMENamesMgr->KoreanIMEs[i] + "@@"; 
						}
						//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
						BroadcastSetSupportedIMEs(imeNames.ToCStr());
					}
				}
				break;

			case GFxIME_Ch_Simp:
				{
					String imeNames;

					int numIme = (int)pIMENamesMgr->ChineseSimpIMEs.GetSize();
					if (numIme == 0) // no ime's associated, activate input language directly
					{
						//  pIMENamesMgr->ActivateIME(parg);
					}
					else 
					{
						for (int i = 0; i < numIme; i++)
						{
							imeNames += pIMENamesMgr->ChineseSimpIMEs[i] + "@@"; 
						}
						//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme);
						BroadcastSetSupportedIMEs(imeNames.ToCStr());
					}
				}
				break;

			case GFxIME_Ch_Trad:
				{
					String imeNames;

					int numIme = (int)pIMENamesMgr->ChineseTradIMEs.GetSize();
					if (numIme == 0) // no ime's associated, activate input language directly
					{
						//  pIMENamesMgr->ActivateIME(parg);
					}
					else 
					{
						for (int i = 0; i < numIme; i++)
						{
							imeNames += pIMENamesMgr->ChineseTradIMEs[i] + "@@"; 
						}
						//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
						BroadcastSetSupportedIMEs(imeNames.ToCStr());
					}
				}
				break;

			case GFxIME_Jp:
				{
					String imeNames;

					int numIme = (int)pIMENamesMgr->JapaneseIMEs.GetSize();
					for (int i = 0; i < numIme; i++)
					{
						imeNames += pIMENamesMgr->JapaneseIMEs[i] + "@@"; 
					}
					//  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
					BroadcastSetSupportedIMEs(imeNames.ToCStr());
				}

				break;

			case GFxIME_English:
				{
					BroadcastRemoveStatusWindow();
				}
				break;
			default:
				break;
			}
		}

		else if(SFstrcmp(pcommand, "LangBar_OnSelectIME") == 0)
		{
			// ActivateIME will activate the IME corresponding to the IMEName and return it's tag.
			if (pIMENamesMgr)
				pIMENamesMgr->ActivateIME(parg);
			GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;
			// This is needed, since for Chinese Traditional IMEs, languagechange notification is not sent
			// therefore, this notification is our only chance of adjusting the position of the status window
			if (imeTag == GFxIME_Ch_Trad)
				BroadcastDisplayStatusWindow();
			// For japanese and korean, the status window should be displayed only when the user has selected the IME
			// from the ime from the drop down list.
			if (CurrentLanguage == "Japanese" || CurrentLanguage == "Korean")
			{
				BroadcastDisplayStatusWindow();
				pIMENamesMgr->SetConversionMode();
			}
		}

		else if(SFstrcmp(pcommand, "LangBar_GetInstalledLanguages") == 0)
		{
			BroadcastSetSupportedLanguages(pIMENamesMgr->InstalledInputLangs.ToCStr());
			//  pMovie->Invoke((LanguageBarPath + ".SetSupportedLanguages").ToCStr(), "%s, %d", pIMENamesMgr->InstalledInputLangs.ToCStr(), pIMENamesMgr->NumInstalledInputLang); 
		}

		// Pass all the status window and language bar related commands to the
		// platform specific implementation
		else 
		{
			OnFinalize();
			if (pIMENamesMgr)
			{
				if (args[1].IsBool())
				{
					bool bval = args[1].GetBool();
					if (bval)
						parg = "true";
					else
						parg = "false";

				}
				pIMENamesMgr->HandleStatusWindowNotifications(pcommand, parg);
			}
		}
	}
	
}

void    GFxIMEManagerWin32::IMECommand(Movie* pmovie, const char* pcommand, const char* parg)
{
    Ptr<Log> plog = pmovie->GetLog();

    if (plog)
    {
        plog->LogMessage("imecommand: '");
        plog->LogMessage(pcommand);
        plog->LogMessage("' '");
        plog->LogMessage("%s", parg);
        plog->LogMessage("'\n");
    }

    //FxPlayerApp* pApp = (FxPlayerApp*)pmovie->GetUserData();
    //FxPlayerSettings& Settings = pApp->Settings;
/*
    if(SFstrcmp(pcommand, "CandidateList_OnItemSelect") == 0)
    {
        SelectAndClose(atoi(parg));
        return;
    }

    else if(SFstrcmp(pcommand, "CandidateList_OnSetItemDataMap") == 0)
    {
    //    FsCallBack(pmovie, pcommand, parg); //?
        return;
    }

#ifndef GFC_NO_LANGBAR_SUPPORT
    // Language Bar related

    else if(SFstrcmp(pcommand, "LangBar_OnInit") == 0)
    {
        if (pIMENamesMgr)
            pIMENamesMgr->OnLangBarLoaded();
    }

    else if(SFstrcmp(pcommand, "StatusWindow_OnInit") == 0)
    {
        // Status window needs to know the currently active language 
        BroadcastSwitchLanguage(CurrentLanguage);
        // if (CurrentLanguage != "Japanese" && CurrentLanguage != "Korean")
            BroadcastDisplayStatusWindow();
        SetConversionMode(0xFFFFFFFF);
    }

    else if(SFstrcmp(pcommand, "LangBar_OnSelectInputLang") == 0)
    {
        // Close status window at this point since otherwise the newly created IME list might overlap the 
        // status window leading to dirty looking display. The status window for the new chosen IME
        // will be created when the openstatuswindow message is sent.

    //  if (StatusWindowPath.GetLength() != 0)
        //  pMovie->Invoke((StatusWindowPath + ".RemoveStatusWindow").ToCStr(), "%d", 0); 
            BroadcastRemoveStatusWindow();

        // Get the last ime/default ime for this input language and call SetLastIME() in ActionScript.
        // if the user is not satisfied with the default IME, he can click on the IME tab and will be presented
        // a list of the installed ime's for that input language.
        if (pIMENamesMgr)
            pIMENamesMgr->ActivateInputLanguage(parg);
    }

    else if(SFstrcmp(pcommand, "LangBar_GetIMENames") == 0)
    {
        //        OnFinalize(); 
        GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;

        switch (imeTag)
        {
        case GFxIME_Kr:
            {
                String imeNames;

                int numIme = (int)pIMENamesMgr->KoreanIMEs.GetSize();
                if (numIme == 0) // no ime's associated, activate input language directly
                {
                    //  pIMENamesMgr->ActivateIME(parg);
                }
                else 
                {
                    for (int i = 0; i < numIme; i++)
                    {
                        imeNames += pIMENamesMgr->KoreanIMEs[i] + "@@"; 
                    }
                    //  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
                    BroadcastSetSupportedIMEs(imeNames.ToCStr());
                }
            }
            break;

        case GFxIME_Ch_Simp:
            {
                String imeNames;

                int numIme = (int)pIMENamesMgr->ChineseSimpIMEs.GetSize();
                if (numIme == 0) // no ime's associated, activate input language directly
                {
                    //  pIMENamesMgr->ActivateIME(parg);
                }
                else 
                {
                    for (int i = 0; i < numIme; i++)
                    {
                        imeNames += pIMENamesMgr->ChineseSimpIMEs[i] + "@@"; 
                    }
                    //  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme);
                    BroadcastSetSupportedIMEs(imeNames.ToCStr());
                }
            }
            break;

        case GFxIME_Ch_Trad:
            {
                String imeNames;

                int numIme = (int)pIMENamesMgr->ChineseTradIMEs.GetSize();
                if (numIme == 0) // no ime's associated, activate input language directly
                {
                    //  pIMENamesMgr->ActivateIME(parg);
                }
                else 
                {
                    for (int i = 0; i < numIme; i++)
                    {
                        imeNames += pIMENamesMgr->ChineseTradIMEs[i] + "@@"; 
                    }
                    //  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
                    BroadcastSetSupportedIMEs(imeNames.ToCStr());
                }
            }
            break;

        case GFxIME_Jp:
            {
                String imeNames;

                int numIme = (int)pIMENamesMgr->JapaneseIMEs.GetSize();
                for (int i = 0; i < numIme; i++)
                {
                    imeNames += pIMENamesMgr->JapaneseIMEs[i] + "@@"; 
                }
                //  pMovie->Invoke((LanguageBarPath + ".SetSupportedIMEs").ToCStr(), "%s, %d", imeNames.ToCStr(), numIme); 
                BroadcastSetSupportedIMEs(imeNames.ToCStr());
            }

            break;

        case GFxIME_English:
            {
                BroadcastRemoveStatusWindow();
            }
            break;
        default:
            break;
        }
    }

    else if(SFstrcmp(pcommand, "LangBar_OnSelectIME") == 0)
    {
        // ActivateIME will activate the IME corresponding to the IMEName and return it's tag.
        if (pIMENamesMgr)
            pIMENamesMgr->ActivateIME(parg);
        GFxIMETag imeTag = pIMENamesMgr->CurrentInputLangTag;
        // This is needed, since for Chinese Traditional IMEs, languagechange notification is not sent
        // therefore, this notification is our only chance of adjusting the position of the status window
        if (imeTag == GFxIME_Ch_Trad)
            BroadcastDisplayStatusWindow();
        // For japanese and korean, the status window should be displayed only when the user has selected the IME
        // from the ime from the drop down list.
        if (CurrentLanguage == "Japanese" || CurrentLanguage == "Korean")
        {
            BroadcastDisplayStatusWindow();
            pIMENamesMgr->SetConversionMode();
        }
    }

    else if(SFstrcmp(pcommand, "LangBar_GetInstalledLanguages") == 0)
    {
        BroadcastSetSupportedLanguages(pIMENamesMgr->InstalledInputLangs.ToCStr());
        //  pMovie->Invoke((LanguageBarPath + ".SetSupportedLanguages").ToCStr(), "%s, %d", pIMENamesMgr->InstalledInputLangs.ToCStr(), pIMENamesMgr->NumInstalledInputLang); 
    }

    // Pass all the status window and language bar related commands to the
    // platform specific implementation
    else 
    {
        OnFinalize();
        if (pIMENamesMgr)
            pIMENamesMgr->HandleStatusWindowNotifications(pcommand, parg);
    }

#endif
	*/
}
}}}
#endif  // SF_NO_IME_SUPPORT
