/**************************************************************************

Filename    :   GFx_IMEWin32Impl.h
Content     :   Contains declarations for GFxIMEWin32Impl, GFxIMEVista, GFxIMEXP. 
Created     :   OCt 4, 2007
Authors     :   Ankur Mohan

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.
					 
Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFXIMEWIN32IMPL_H
#define INC_GFXIMEWIN32IMPL_H

#include "GFx_CandidateListInterface.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "GFx_HklLangNameMap.h"
#include "GFx_IMEIdMap.h"
#include "Kernel/SF_Debug.h"

// Vista specific includes
//#include "GFxIMETSF.h"

// This class is the base class for the implementation of IME on XP and Vista. 
// GNewOverrideBase class overloads new and delete operators so that custom 
// memory allocation is used.

namespace Scaleform {
	namespace GFx {
		namespace IME{

struct styleInfo
{
    UInt32 begin;
    UInt32 end;
    UInt32 highLightStyle;
};

class GFxIMEWin32Impl: public NewOverrideBase<StatIME_Mem>
{
public:

    // Constructor
    // Initializes the pointer to movieView and sets window handle. Also creates
    // ListBox object to handle candidate list logic.
    GFxIMEWin32Impl(IMEManagerBase* pbase, HWND hwnd): pMovie(0), pBase(pbase), hWND(hwnd)
    {
        ListBox = new CandidateListBox(this);
        CmdManager.Register(ListBox);
        GlobalIMEState = true;

        if(hWND == NULL)
            SF_DEBUG_WARNING(1, "Window Handle passed to IME is NULL!");
        SetWndHandle(hWND);
        
        // Set default properties for Candidate List.
        // Font size
        CandListProps.CandListFontSize        = 20;
        // Text color for non-selected rows
        CandListProps.CandListFontColorReg    = 0x000000;
        // Text color for the selected row
        CandListProps.CandListFontColorSel    = 0xffffff;
    };
    
    // Destructor
    virtual ~GFxIMEWin32Impl()
    {
        ListBox->RemoveAllListItems();
        if (ListBox)
            delete ListBox;
    }

    // Functions for setting IME state

    virtual void    SetMovie(Ptr<Movie> movie)
    {
        pMovie = movie;
        ListBox->RegisterMovieContainingWidget(pMovie);
    }

    virtual void    SetWndHandle(HWND hWnd)
    {
        hWND = hWnd;
    }

	bool       Invoke(const char* ppathToMethod, Value *presult, const Value* pargs, unsigned numArgs)
	{
		return pBase->Invoke(ppathToMethod, presult, pargs, numArgs);
	}

    /* Records path to the CandidateList AS object to send invokes */
    virtual void    RecordCandidateListPath(const char* parg)
    {
        CandListPath = parg;
        ListBox->SetASWidgetPath(parg);
    }

	/* Records path to the CandidateList AS object to send invokes */
	virtual void    ClearCandidateListPath()
	{
		CandListPath = "";
		ListBox->SetASWidgetPath("");
	}

    /* Records path to the CandidateList AS object to send invokes */
    virtual void    RecordStatusWindowPath(const char* parg)
    {
        StatusWindowPath = parg;
    }

	// Checks if cursor is in a textfield. For any other element, IME messages/TSF notifications are discarded
	virtual bool IsTextFieldFocussed(Movie* pmovie)
	{
		MovieImpl* pmovieRoot = static_cast<MovieImpl*>(pmovie);
        bool hRes = false;

        if (pmovieRoot)
        {
		    Ptr<InteractiveObject> pfocusedCh = pmovieRoot->GetFocusedCharacter(0);


		    if (pfocusedCh && pfocusedCh->GetType() == CharacterDef::TextField)
		    {
			    Ptr<TextField> ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
			    if (ptextFld && ptextFld->IsIMEEnabled())
			    {
				    hRes = true;
			    }
		    }
        }
		return hRes;
	}

    virtual void    HandleStatusWindowNotifications(const char* pcommand, const char* parg){ SF_UNUSED2(pcommand, parg); }

    /* Records path to the CandidateList AS object to send invokes */
    virtual void    RecordLanguageBarPath(const char* parg)
    {
        LanguageBarPath = parg;
    }

    /* Sets properties for the candidate list. 
    param1: Font size for cand list text param2: Font color for regular text
    param3: Font color for highlighted row text.  */
    virtual void    SetCandidateListProperties(INT size, INT fontColor1, INT fontColor2)
    {
        CandListProps.CandListFontSize        = size;
        CandListProps.CandListFontColorReg    = fontColor1;
        CandListProps.CandListFontColorSel    = fontColor2;

    }

    // Functions for handling windows IME messages

    /* Handles WM_IMECOMPOSITION */ 
    virtual void    OnIME(UPInt key, UPInt info, int options){ SF_UNUSED3(key, info, options);};

    /* Handles WM_IMESTART */ 
	virtual UInt32    OnIMEStart(UPInt key, UPInt info, bool downFlag){ SF_UNUSED3(key, info, downFlag); return Movie::HE_NotHandled; };

    /* Handles WM_IMESTOP */ 
    virtual void    OnIMEStop(UPInt key, UPInt info, bool downFlag){ SF_UNUSED3(key, info, downFlag);};

    /* Handles WM_IME_NOTIFY for candidate list related notifications */ 
    virtual LRESULT OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag)
    {
        SF_UNUSED4(message, key, info, downFlag);
        return 0;
    };

    /* Handles some WM_NOTIFY messages that get sent when backspace, space, esc keys etc
    are pushed. */
    virtual void    CustomProcessing(UPInt message, WPARAM wParam, LPARAM lParam){ SF_UNUSED3(message, wParam, lParam); };
   
    /*************************************************/
    // Miscellaneous Functions 
    /************************************************/

    /* Finalizes IME composition text, closes any pop ups (candidate list, reading window) */ 
    virtual void    Finalize() {}
    virtual void    Cleanup()  
    {
        HIMC     hIMC;

        hIMC = ImmGetContext(hWND);
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
        ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_REVERT, 0);
        ImmReleaseContext(hWND, hIMC);

        // Clear Reading window contents and remove the reading window

        ReadingTextBuffer[0] = 0;
    }

    virtual void    SetIMETag(GFxIMETag tag){ IMETag = tag; }

    void            SetImeVersionId(int imeVersionId) { ImeVersionId = imeVersionId; };

    virtual void    Shutdown()
    {
        pMovie = NULL; // prevents from Invokes
        Cleanup();
    }

    /* Selects a particular row of the Candidate List (specified by index). Doesn't finalize */ 
    virtual void    SelectAndClose(int index){  SF_UNUSED(index);}; 

    virtual void    FsCallBack(Movie* pmovie, const char* pcommand, const char* parg){ SF_UNUSED3(pmovie, pcommand, parg); };
    
    /* Records the last keystroke for reading window display */
    virtual void    PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt) { SF_UNUSED(winEvt);};

    /* Takes care of interacting with AS to display reading (input) window */
    virtual void    DisplayReadingWindow(wchar_t* pReadingStr){ SF_UNUSED(pReadingStr);};
    
    /* Displays composition string, keeps track of how many characters are on the screen, cursor
       position etc */
    virtual void    DisplayCompositionString(UInt32& numCharDisplay){ SF_UNUSED(numCharDisplay);};

    /* Checks if we are in a textfield or not. If not, translate message is not called. This
       is to make TAB work correctly when IME is active and focus is on a non textfield object. */
    virtual bool    CheckIfInTextField()
    {
        Value val;
        if(!pMovie) return true;
        Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
        if(!pimeMgr) return true;
        SF_ASSERT(pimeMgr && pimeMgr->IsMovieActive(pMovie));
        return pimeMgr->IsTextFieldFocused();
    }
    
    // Override from GFxIMEManaager. Invoked when candidate list loading is completed.
    virtual void OnCandidateListStyleChanged(const IMECandidateListStyle& style)
    {
        if (!pMovie)
            return;
        Ptr<IMEManagerBase> pimeMgr = pMovie->GetIMEManager();
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
        UInt32   fontSize                 = (UInt32)(-1);
        UInt32   readingWindowFontSize    =  (UInt32)(-1);
        
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

    virtual void    SetOpenStatus(bool status)
    {
        GlobalIMEState = status;
    }

    // This is used to turn IME on/off when the user clicks to an editable field (ime = on) or
    // a non-editable field (ime = off). In addition, user can also SetEnabled AS function to
    // turn IME on/off. 
    virtual void    OnEnableIME(bool enable)
    {
        // Note that we can't use ImmSetOpenStatus here to enable/disable IME, because
        // one should never be able to enable IME on a textfield on which imeDisable flag
        // has been set or on a non-editable object. Associating the Null context is a 
        // much more powerful way of disabling IME. 
        HIMC hIMC = ImmGetContext(hWND);
   //     ImmSetOpenStatus(hIMC, (enable) ? IACE_DEFAULT : 0);

        ImmAssociateContextEx(hWND, NULL, (enable) ? IACE_DEFAULT : 0);
        // This needs to be done since associating the new context overwrites the previously
        // set IME state. 
        // SetEnabled(GlobalIMEState);
        ImmReleaseContext(hWND, hIMC);
    }

    virtual String GetInputLanguage()
    {
        HKL currLocale  = GetKeyboardLayout(0); 
        Un.hkl          = currLocale;
        
        switch (Un.val & 0x0000FFFF)
        {                    
        case 0x0411:
            return String("Japanese");
        case 0x0412:
            return String("Korean");
        case 0x0404:
        case 0x0804:
            return String("Chinese");
        case 0x0409: // English US
        case 0x0809: // English UK
        case 0x0c09: // English Aus
        case 0x1009: // English Canada
            return String("English");
        case 0x0419:
            return String("Russian");
        default:
            return String("Unknown");
        }
    }

    virtual bool    SetEnabled(bool enabled)
    {
        bool res;
        bool isIMEEnabled   = true;

        HIMC hIMC = ImmGetContext(hWND);

        if(hIMC == NULL)
        {
            isIMEEnabled = false;
            // IME might have been disabled on this object- enable it temporarily.
            ImmAssociateContextEx(hWND, NULL,  IACE_DEFAULT);
            // Get context again
            hIMC = ImmGetContext(hWND);
            // if hIMC is still NULL, return
            if (hIMC == NULL)
            {
                ImmReleaseContext(hWND, hIMC);
                return false;
            }
        }

        res = (ImmSetOpenStatus(hIMC, enabled) == 0)? false: true;
        
        if (isIMEEnabled == false)
        {
            // Since IME was disabled originally, we should disable it again.
            ImmAssociateContextEx(hWND, NULL,  0);
        }

        ImmReleaseContext(hWND, hIMC);
        // Remember this state
        GlobalIMEState = enabled;
        return res;
    }

    virtual bool    GetEnabled()
    {
        bool enabled        = false;
        bool isIMEEnabled   = true;

        HIMC hIMC = ImmGetContext(hWND);
        if(hIMC == NULL)
        {
            isIMEEnabled = false;
            // IME might have been disabled on this object- enable it temporarily.
            ImmAssociateContextEx(hWND, NULL,  IACE_DEFAULT);
            // Get context again
            hIMC = ImmGetContext(hWND);
            // if hIMC is still NULL, return
            if (hIMC == NULL)
            {
                ImmReleaseContext(hWND, hIMC);
                return false;
            }
        }
        enabled = ImmGetOpenStatus(hIMC) == 0? false: true;

        if (isIMEEnabled == false)
        {
            // Since IME was disabled originally, we should disable it again.
            ImmAssociateContextEx(hWND, NULL,  0);
        }

        ImmReleaseContext(hWND, hIMC);

        return enabled;
    }

    virtual bool    SetCompositionString(const char* pCompString)
    {
        HIMC hIMC   = ImmGetContext(hWND);
     //   UInt32 len    = (UInt32)SFstrlen(pCompString);
        UInt32 wlen   = (UInt32)UTF8Util::GetLength(pCompString);

        WStringBuffer wBuff;
        wBuff = pCompString;
        
        if (hIMC)
        {
            bool res = (ImmSetCompositionString(hIMC, SCS_SETSTR, (LPVOID)wBuff.ToWStr(), 2*wlen, 0, 0) == 0)? false: true;
            ImmReleaseContext(hWND, hIMC);
            return res;
        }

        return false;
    }

    virtual bool    SetConversionMode(const UInt32 conversionMode)
    {

        // First check what input language is currently active.

        HKL currLocale  = GetKeyboardLayout(0); 
        Un.hkl          = currLocale;

        HIMC hIMC = ImmGetContext(hWND);
        bool isIMEEnabled = true;
        if(hIMC == NULL)
        {
            isIMEEnabled = false;
            // IME might have been disabled on this object- enable it temporarily.
            ImmAssociateContextEx(hWND, NULL,  IACE_DEFAULT);
            // Get context again
            hIMC = ImmGetContext(hWND);
            // if hIMC is still NULL, return
            if (hIMC == NULL)
            {
                ImmReleaseContext(hWND, hIMC);
                return false;
            }
        }

        DWORD   fdwConversion;
        DWORD   fdwSentence;
        bool    res = true;

        ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);
        
        if((fdwConversion & 0x0000000F) == IME_CMODE_ALPHANUMERIC)
        {
            // for Direct Input mode, set the status of the IME to be open.
            // This is important because without this, we would not be able to change 
            // status from DirectInput mode (where the status of the IME is closed) to other 
            // modes.
            ImmSetOpenStatus(hIMC, true); 
        }
        
        // Zero out the last 4 bits
        fdwConversion = fdwConversion & 0xFFFFFFF0;

        // Japanese Conversion Modes
        if((Un.val&0x0000FFFF) == 0x00000411)
        {
            switch (conversionMode)
            {
            case 0x00:  // ALPHANUMERIC_FULL
                fdwConversion |= IME_CMODE_FULLSHAPE;
                break;
            case 0x01:  // ALPHANUMERIC_HALF
                fdwConversion |= IME_CMODE_ALPHANUMERIC;
                break;
            case 0x04:  // JAPANESE_HIRAGANA
                fdwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
                break;
            case 0x08:  // JAPANESE_KATAKANA_FULL
                fdwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_LANGUAGE;
                break;
            case 0x016: // JAPANESE_KATAKANA_HALF
                fdwConversion |= IME_CMODE_LANGUAGE;
                break;
            default:
                res = false; 
            }
        }
        

         // Korean Conversion Modes
        if ((Un.val&0x0000FFFF) == 0x00000412)
        {
            if (conversionMode == 0x032)  // KOREAN
            {
                fdwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
            }

            else if (conversionMode == 0x00)
            {
                fdwConversion |= IME_CMODE_FULLSHAPE;
            }

            else if (conversionMode == 0x01)
            {
                fdwConversion |= IME_CMODE_ALPHANUMERIC;
            }
            else
            {
                res = false;
            }
        }

        // CHINESE Conversion Modes
        if ((Un.val&0x0000FFFF) == 0x00000404 || (Un.val&0x0000FFFF) == 0x00000804)
        {
            if (conversionMode == 0x02)  // CHINESE
            {
                fdwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
            }

            else if (conversionMode == 0x00)
            {
                fdwConversion |= IME_CMODE_FULLSHAPE;
            }

            else if (conversionMode == 0x01)
            {
                fdwConversion |= IME_CMODE_ALPHANUMERIC;
            }

            else
            {
                res = false;
            }
        }

        // We can't rely on the return value of ImmsetConversionStatus since it always returns true no 
        // matter what the fdwconversion is. Hence, we have to disambiguate between valid/invalid
        // conversion mode values depending on the current language as done above. 
        if(res)
        {
        //    bool result = (ImmSetConversionStatus(hIMC, fdwConversion, fdwSentence) == 0)? false: true;
            ImmSetConversionStatus(hIMC, fdwConversion, fdwSentence);
            // Reset global IME state
            GlobalIMEState = true;
            ImmSetOpenStatus(hIMC, GlobalIMEState);
        }

        ImmReleaseContext(hWND, hIMC);

        if (isIMEEnabled == false)
        {
            // Since IME was disabled originally, we should disable it again.
            ImmAssociateContextEx(hWND, NULL,  0);
        } 
        return res;
    }

    // Retrieves conversion mode. 
    virtual const char* GetConversionMode()
    {
        DWORD fdwConversion;
        DWORD fdwSentence;
       
        HIMC hIMC = ImmGetContext(hWND);
        bool isIMEEnabled = true;

        if(hIMC == NULL)
        {
            isIMEEnabled = false;
            // IME might have been disabled on this object- enable it temporarily.
            ImmAssociateContextEx(hWND, NULL,  IACE_DEFAULT);
            // Get context again
            hIMC = ImmGetContext(hWND);
            // if hIMC is still NULL, return
            if (hIMC == NULL)
            {
                ImmReleaseContext(hWND, hIMC);
                return "UNKNOWN";
            }
        }
        ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);

        // fdwConversion values have to be interpreted differently depending upon the 
        // language currently in use.

        ConversionMode = "";
        // Zero out all but last 4 bits of fdwConversion

        fdwConversion = fdwConversion & 0x0000000F;
        // Japanese Conversion modes
        if (IMETag & GFxIME_Jp_Flag)
        {
            if ((fdwConversion & (~IME_CMODE_FULLSHAPE)) == IME_CMODE_NATIVE)
            {
                ConversionMode = "JAPANESE_HIRAGANA";
            }

            if (fdwConversion == (IME_CMODE_FULLSHAPE | IME_CMODE_LANGUAGE)) 
            {
                ConversionMode = "JAPANESE_KATAKANA_FULL";
            }

            if (fdwConversion == IME_CMODE_LANGUAGE) 
            {
                ConversionMode = "JAPANESE_KATAKANA_HALF";   
            }

            if (fdwConversion == IME_CMODE_FULLSHAPE) 
            {
                ConversionMode = "ALPHANUMERIC_FULL";
            }

            if (fdwConversion == IME_CMODE_ALPHANUMERIC) 
            {
                ConversionMode = "ALPHANUMERIC_HALF";
            }
        }

        // Korean Conversion Modes

        if (IMETag & GFxIME_Kr_Flag)
        {
            if ((fdwConversion & (~IME_CMODE_FULLSHAPE)) == IME_CMODE_NATIVE)
            {
                ConversionMode = "KOREAN";
            }

            if (fdwConversion == IME_CMODE_FULLSHAPE)
            {
                ConversionMode = "ALPHANUMERIC_FULL";
            }
            if (fdwConversion == IME_CMODE_ALPHANUMERIC)
            {
                ConversionMode = "ALPHANUMERIC_HALF";
            }
        }

        // Chinese Conversion Modes
        if ((IMETag & GFxIME_Ch_Simp_Flag) || (IMETag & GFxIME_Ch_Trad_Flag))
        {
            if ((fdwConversion & (~IME_CMODE_FULLSHAPE)) == IME_CMODE_NATIVE)
            {
                ConversionMode = "CHINESE";
            }

            if (fdwConversion == IME_CMODE_FULLSHAPE)
            {
                ConversionMode = "ALPHANUMERIC_FULL";
            }

            if (fdwConversion == IME_CMODE_ALPHANUMERIC)
            {
                ConversionMode = "ALPHANUMERIC_HALF";
            }
        }

        if (isIMEEnabled == false)
        {
            // Since IME was disabled originally, we should disable it again.
            ImmAssociateContextEx(hWND, NULL,  0);
        }  
        
        ImmReleaseContext(hWND, hIMC);

        if(ConversionMode.IsEmpty())
            return "UNKNOWN";
        else
            return ConversionMode.ToCStr();
    }

    // TSF class declared as a friend so it can access private members of this class without
    // breaking encapsulation
    friend class CTSF;

protected:

    // public because this will be used within the CTSF class.
    // Ptr<Movie>  pMovie;
    Movie*       pMovie; //!AB, can't be Ptr here!

    // Stores path to the AS object that implements Candidate List and Reading Window handling
    String           CandListPath;   

    // Stores path to the AS object that implements Status Window handling
    String           StatusWindowPath;  

    // Stores path to the AS object that implements Language Bar handling
    String           LanguageBarPath;

    HWND                hWND;
    // For Candidate List listbox
    CandidateListBox*   ListBox;
    FxCommandManager    CmdManager;
    enum                {ReadingTextBufferSize = 256};
    ArrayLH<wchar_t, StatIME_Mem>   ReadingTextBuffer; // Buffer for containing reading window characters  
    HKL                 CurrLocale; 

    // Indicates the number of characters displayed last time 
    UInt32                NumCharDisplay;

    // This tells us the position of the first character in the currently highlighted clause
    // Its used to adjust the position of the candidate list window.
    int                 ClausePosition; 

    // Used to record if we should write over the current character or write a new one 
    // in Korean IME
    int                 ReplaceChar; 

    // Records the number of candidates on the candidate list on the previous 
    // display. 
    int                 NumCandidatesOnListPrev; 

    // Tells us the index from which the current page starts in the list of candidate strings
    int                 CurrentPageStart;
    
    // Stores the current conversion mode
    String           ConversionMode;

    // Global IME State
    bool                GlobalIMEState;
    struct CandListProps
    {
        INT CandListFontSize;        
        INT CandListFontColorReg;    
        INT CandListFontColorSel;   
    } CandListProps;

    union 
    {
        HKL     hkl;
        size_t  val;
    } Un;

    GFxIMETag           IMETag;

    int                 ImeVersionId;
	// Used to keep a track of IME messages- used in PreProcessWM_KEYDOWN.
	bool                Track;
	bool                ImeMessageRecd;
	IMEManagerBase		*pBase;
};

/*
 This class implements IME on Windows Vista. It uses the TSF (Text Services Framework) architecture
 for handling CandidateList and Reading Window logic. 
 We decided to use TSF for implementing IME on Windows Vista for the following reasons:

 1) TSF always provides us with the correct Reading window text information. 
    Unlike IMM where reading window text information is not available for some IME's
    for example- New Phonetic, New Chang Jie, Quan Pin. The lack of this information 
    complicates IME implementation for XP where we have to provide keyboard mapping 
    tables for different IMEs. 

 2) IMM doesn't provide information on when to close the Candidate List on Windows Vista.
 
 In addition to the above mentioned reasons, there are a number of other inconsistencies between
 IMM documentation as provided on MSDN and actual IMM behaviour. 

 We use TSF only for handling Candidate List and Reading Window logic. The composition string,
 cursor positioning and attribute logic is still handled using IMM.
 */

class GFxIMEVista: public GFxIMEWin32Impl
{
public:
    
    // Constructor
    GFxIMEVista(IMEManagerBase* pbase, HWND hwnd);

    // Destructor
    ~GFxIMEVista();

	String		ReadinStringBuffer;
	bool			StartCompositionReceived;
	bool			DrawCandidateList;

	virtual void    DisplayReadingWindow(wchar_t* pReadingStr);
	virtual void    RepositionCandidateList(UInt32 offset);
	/*
	We override OnEnableIME for Vista for the following reason: 
	When the user clicks on a non-ime enabled flash object such as the background, before the focus transfer takes place to the new object,
	the following two events happen (in this order):

	1) Finalize is called, which causes the IME state to be finalized (candidate list closes, composition is cancelled or finialized)

	2) IME is disabled by associating a null context. This is done since we don't want IME popups when the focus is on a non-editable or ime disabled
	object

	On XP, we use the IMM functions to finalize- these functions cause the defwndproc to be called directly and ime state is finalized appropriately
	before IME is disabled in OnEnableIME(false). On Vista, for the Dayi and Array IME's the secondary candidate lists don't follow the IMM api. Therefore,
	in order to cause the state of IME to be finalized during "Finalize", I use SendInput to insert an Escape character in the input queue. However,
	the effect of this Escape is asynchronous, so if IME is disabled immediately afterwards, by the time the escape character is interpreted by the IME
	, it's state has already been disabled and no action is taken (no EndUIElement is called) leading to dangling candidate lists. Therefore, we maintain
	a referencecount for IME UI elements and check to see if any UI elements need to be closed. If there are any, we defer disabling IME until EndUIElement
	has been called on all the open UI elements and IME can safely be disabled.
	*/
	virtual void    OnEnableIME(bool enable)
	{
		 
		if (enable == false && CandWndRefCount > 0)
			NeedToDisableIME = true;
#define TF_CONVERSIONMODE_ALPHANUMERIC      0x0000
#define TF_CONVERSIONMODE_NATIVE            0x0001
		if (CandWndRefCount == 0)
		{
			HIMC hIMC = ImmGetContext(hWND);
			DWORD fdwConversion, sentence;
			
			if (pBase->GetInputLanguage() == "Korean")
			{
				ImmAssociateContextEx(hWND, NULL, (enable) ? IACE_DEFAULT : 0);
			}
			
			if (pBase->GetInputLanguage() != "Korean")
			{
				ImmGetConversionStatus(hIMC, &fdwConversion, &sentence);
				// Pertains to Chinese IME only
				if (!enable)
				{
					fdwConversion = fdwConversion & (~TF_CONVERSIONMODE_NATIVE); 
				}

				if (enable)
				{
					fdwConversion = fdwConversion | TF_CONVERSIONMODE_NATIVE; 
				}
				ImmSetConversionStatus(hIMC, fdwConversion, 0);
			}

			ImmReleaseContext(hWND, hIMC);
			NeedToDisableIME = false;
		}
	}
	
	// Maintains reference count for the UI elements ( incremented in BeginUIElement, decremented in EndUIElement)
	int				CandWndRefCount;

	// Used to record if there is a pending DisableIME request that couldn't be satisfied since there are open UI elements. Therefore, OnEnableIME
	// should be called again when the UI elements have been closed. 
	bool			NeedToDisableIME;

private:

    // IME message handling functions

    virtual void    OnIME(UPInt key, UPInt info, int options);
    
  //  virtual void  OnIMEChar() { NumCharPermanant++; }
    
    virtual UInt32  OnIMEStart(UPInt key, UPInt info, bool downFlag);
    
    virtual void    OnIMEStop(UPInt key, UPInt info, bool downFlag);
    
    virtual void    OnChangeCandidate(int candListIndex);

    virtual void    OnOpenCandidate(int candListIndex);

    virtual LRESULT OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag);

    // Miscellaneous functions

    virtual void    Finalize();

    virtual void    SelectAndClose(int index); // Selects the text in row # and closes Candidate List 

    virtual void    FsCallBack(Movie* pmovie, const char* pcommand, const char* parg);

    virtual void    PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt);

    virtual void    DisplayCompositionString(UInt32& numCharDisplay);

    void            CloseIME(); 

    // Variables
    CTSF*           pTSF; 

    int             PrevX; // Records the previous location of the candidate list
    int             PrevY; 
    int             NumCandidates;
 
     // This variable tells us if the candidate list starts from 1 or 0.
    int             CandListStartFrom1;
    // Union to cast HKL to size_t
    union HklUn{
        HKL     hkl;
        size_t  langId;
    } Un;

};

/*
This class contains IME implementation for Windows XP. We use the input method editor (IMM) for 
implementing IME support. When a user interacts with an IME to input complex characters, the 
IMM sends messages to the application to notify it of important events, such as starting a 
composition or showing the candidate window. This class provides functions to handle these
messages as well as fscallbacks from the candidate list.
*/
class GFxIMEXP: public GFxIMEWin32Impl
{   
public:

    // Constructor
    GFxIMEXP(IMEManagerBase* pbase, HWND wnd);

    // Destructor 
    ~GFxIMEXP();

private:
    
     // IME message handling functions
//    virtual void  OnIMEChar() { NumCharPermanant++; }

    virtual void        OnIME(UPInt key, UPInt info, int options);

    virtual UInt32        OnIMEStart(UPInt key, UPInt info, bool downFlag);

    virtual void        OnIMEStop(UPInt key, UPInt info, bool downFlag);

    virtual void        OnChangeCandidate(int candListIndex);

    virtual void        OnOpenCandidate(int candListIndex);

    // Miscellaneous functions

    virtual void        Finalize();
    virtual void        Cleanup();

    virtual void        SelectAndClose(int index); // Selects the text in row # and closes Candidate List 

    virtual void        FsCallBack(Movie* pmovie, const char* pcommand, const char* parg);

    virtual void        PreProcessWM_KEYDOWN(const IMEWin32Event& winEvt);

	void                DisplayReadingWindow(wchar_t* lpReadStr) { SF_UNUSED(lpReadStr); };

    void                DisplayReadingWindow(UInt32 keyCode, int numChar, char* lpastr, wchar_t* lpReadStr);

    virtual void        DisplayCompositionString(UInt32& numCharDisplay);

    void                SetLastCharacter(char lastChar){LastCharacter = lastChar; }

    void                CustomProcessing(UPInt message, WPARAM wParam, LPARAM lParam);

    virtual LRESULT     OnIMECandidateProcessing(UInt32 message, UPInt key, UPInt info, bool downFlag);

    void                CloseIME(); 

	void				RepositionCandidateList(UInt32 offset);

    // Variables

    // Buffer for Chinese Traditional (New Phonetic)
    ArrayLH<wchar_t> ReadingTextChTradNewPhonetic;

    // Indicates which position in the reading window to Place current character
    // Used by Chinese Trad New Phonetic, where Reading Window chars (strokes) appear at
    // predetermined positions in the Reading Window. 
    int                 ReadingWindowCharacterPos; 
    int                 NumCandidates;
    wchar_t             LastCharacter;
    int                 NumRowsMax;
    
    enum                {NumTradNewPhoneticKeyCodes = 102};
    int                 TradNewPhoneticKeyCodesB[NumTradNewPhoneticKeyCodes][3];
    // This variable tells us if the candidate list starts from 1 or 0.
    int                 CandListStartFrom1; 

    // Used with Chinese Simplified (MS Pinyin) reading window display. 
    int                 CurrChar;   

    // Union to cast HKL to size_t
    union HklUn{
        HKL     hkl;
        size_t  langId;
    } Un;

    HIMC                hSavedIMC;
    int                 CandidateListIndex;
	bool				DrawCandidateList;
}; 
}}}
#endif //INC_GFXIMEWIN32IMPL_H

