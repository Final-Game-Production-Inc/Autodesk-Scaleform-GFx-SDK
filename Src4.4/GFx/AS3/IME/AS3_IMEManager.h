
/**************************************************************************

PublicHeader:   GFx
Filename    :   AS3_IMEManager.h
Content     :   IME Manager base functinality
Created     :   Dec 6, 2010
Authors     :   Ankur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3IMEMANAGER_H
#define INC_AS3IMEMANAGER_H

#include "GFxConfig.h"
#include "GFx/GFx_Loader.h"
#include "Render/Render_Types2D.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "GFx/GFx_ASIMEManager.h"
#include "GFx/AS3/AS3_Value.h"

namespace Scaleform { namespace GFx { namespace AS3{

#ifndef SF_NO_IME_SUPPORT


// IME manager interface class. This class may be used as a base class for various
// IME implementations. It also provides utility functions to control composition, 
// candidate list and so on.

class IMEManager : public ASIMEManager
{
public:
    friend class MovieImpl;

    unsigned UnsupportedIMEWindowsFlag;
    // handles focus. 
    InteractiveObject* HandleFocus(Movie* pmovie, 
        InteractiveObject* poldFocusedItem, 
        InteractiveObject* pnewFocusingItem, 
        InteractiveObject* ptopMostItem);

    // callback, invoked when mouse button is down. buttonsState is a mask:
    //   bit 0 - right button is pressed,
    //   bit 1 - left
    //   bit 2 - middle
    void OnMouseDown(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);
    void OnMouseUp(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);

    enum 
    {
        GFxIME_AlwaysHideUnsupportedIMEWindows          = 0x01,
        GFxIME_HideUnsupportedIMEWindowsInFullScreen    = 0x02,
        GFxIME_ShowUnsupportedIMEWindows                = 0x03
    } ; 

    Log*		pLog;
    SInt32		CandidateListState;
    GFx::Value	CandListVal;
    Ptr<ASIMEManager::IMEFuncHandler> pCustomFunc;
    IMEManager();
    virtual ~IMEManager()
    {
    };

    virtual void ShutDown()
    {
        CandListVal.SetUndefined(); pMovie = 0;
    };
    virtual UInt32 Init(Log* plog){ plog = 0; return 1;};

    void Reset(); 

    virtual bool Invoke(const char* pmethodName, GFx::Value *presult, const GFx::Value* pargFmt, unsigned numArgs)
    {
        if (!pMovie || CandListVal.IsUndefined() || CandListVal.IsNull()) 
            return false;

        char *result	= NULL;
        int len			= static_cast<int>(SFstrlen(pmethodName));
        char* tmpBuf	= (char*)SF_ALLOC((len+1), StatIME_Mem);
        char* pcommand	= 0;
        char* context	= 0;
        memcpy(tmpBuf, pmethodName, len);
        tmpBuf[len] = 0;
#ifdef SF_MSVC_SAFESTRING
        result			= strtok_s( tmpBuf, ".", &context);
#else
        SF_UNUSED(context);
        result			= strtok( tmpBuf, ".");
#endif
        GFx::Value mem	= CandListVal;
        GFx::Value func = CandListVal;
        while( !mem.IsNull() && (result != NULL) )
        {
            mem = func;
            pcommand = result;
            mem.GetMember(result, &func);
#ifdef SF_MSVC_SAFESTRING
            result = strtok_s(NULL, ".", &context);
#else
            result = strtok(NULL, ".");
#endif
        }
        //	CandListVal.GetMember()
        bool retVal = false;
        if (!mem.IsNull())
        {
            mem.Invoke(pcommand, presult, pargFmt, numArgs);
        }
        SF_FREE(tmpBuf);
        return retVal;
    }

    // Returns true, if text field is currently focused.
    bool IsTextFieldFocused() const;

    // Returns true, if the specified text field is currently focused.
    bool IsTextFieldFocused(InteractiveObject* ptextfield) const;

    // Checks if candidate list is loaded
    virtual bool IsCandidateListLoaded() const;

    bool SetCandidateListStyle(const IMECandidateListStyle& st);

    // Gets the sprite corresponding to the candidate list
    Sprite* GetCandidateListSprite();

    // Sets candidate list movie path
    void SetIMEMoviePath(const char* pcandidateSwfPath);

    // Gets candidate list path for logging
    bool GetIMEMoviePath(String& candidateSwfPath);

    virtual void ASRootMovieCreated(Ptr<Sprite> spr) ;

    virtual void SetActiveMovie(Movie* pmovie)
    {


        ASIMEManager::SetActiveMovie(pmovie);
        /*
        AS3::MovieRoot* pmovieRoot = AS3::ToAS3Root((MovieImpl*)(pmovie));
        pmovie->CreateFunction(&func, CustomFuncLanguageBar);
        pmovieRoot->Value2ASValue(func, &asFunc);
        AvmSprite* avmSpr = AS3::ToAvmSprite(pmovieRoot->Get);
        avmSpr->SetMember(avmSpr->GetASEnvironment(), pmovieRoot->GetStringManager()->CreateConstString("SendLangBarMessage"), 
        asFunc);
        */
        // Important Language Bar related Note: The language bar uses the SendLangBarMessage function 
        // to send notifications to IME core. We use the function injection mechanism to inject a 
        // C++ implementation for this function. However this injection can't take place at this point,
        // since the advance on the root movie has not been called yet (assuming we get here from the 
        // first SetFocus call right when the movie is loaded), and the avmDispObj has not been created.
        // The solution is for the parent swf to send a Language Bar Init request, in response to which
        // the appropriate C++ callback is injected. 

    }

    void ShowUnsupportedIMEWindows(unsigned unsupportedIMEWindowsFlag) {UnsupportedIMEWindowsFlag = unsupportedIMEWindowsFlag;};

    unsigned GetUnsupportedIMEWindowsFlag(){ return UnsupportedIMEWindowsFlag; };

    /****** Flash IME class functions **************/

    // Support for OnIMEComposition event
    virtual void BroadcastIMEConversion(const wchar_t* pString);

    // Support for OnSwitchLanguage event
    virtual void BroadcastSwitchLanguage(const char* pString);

    // Support for OnSetSupportedLanguages event
    virtual void BroadcastSetSupportedLanguages(const char* pString);

    // Support for OnSetSupportedIMEs event
    virtual void BroadcastSetSupportedIMEs(const char* pString);

    // Support for OnSetCurrentInputLanguage event
    virtual void BroadcastSetCurrentInputLanguage(const char* pString);

    // Support for OnSetIMEName event
    virtual void BroadcastSetIMEName(const char* pString);

    // Support for OnSetConversionStatus event
    virtual void BroadcastSetConversionStatus(const char* pString);

    // Support for OnBroadcastRemoveStatusWindow event
    virtual void BroadcastRemoveStatusWindow(const char* pString);

    // Support for OnBroadcastDisplayStatusWindow event
    virtual void BroadcastDisplayStatusWindow(const char* pString);

    // SetCompositionString
    virtual bool SetCompositionString(const char* pCompString) { SF_UNUSED(pCompString); return false; }; 

 
    // Loads candidate list movie, if it wasn't loaded yet. It will invoke
    // OnCandidateListLoaded(path) virtual method once movie is loaded.
    bool AcquireCandidateList();

    // Loads the Status window movie.
    bool AcquireStatusWindowMovie();

    // Loads the Language bar movie.
    bool AcquireLanguageBarMovie();

    // Fills pdest with error message, if candidate list failed to load.
    // Returns 'true' if error occurred.
    bool GetCandidateListErrorMsg(String* pdest);

	bool GetCandidateListStyle(IMECandidateListStyle* pst) const;

    // This callback function is called then the candidate list dialog
    // is ready to be shown. It is used to set the font to show the dialog 
    // from the current text field.
    void OnOpenCandidateList();

   
    // *** callbacks, invoked from core. These virtual methods
    // might be overloaded by the implementation.

    // invoked to check does the symbol belong to candidate list or not
    virtual bool IsCandidateList(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to status window or not
    virtual bool IsStatusWindow(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to lang bar window or not
    virtual bool IsLangBar(const char* ppath) { SF_UNUSED(ppath); return false; }

    // handle "_global.imecommand(cmd, param)"
    virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
    { SF_UNUSED3(pmovie, pcommand, pparam); }

    // invoked when need to cleanup and shutdown the IME. Do not invoke ActionScript from it!
    virtual void OnShutdown() {}

private:
    void	DispatchEvent(const char* message, const char* messageType, const char* type);

};

#else // #ifdef SF_NO_IME_SUPPORT
class IMECandidateListStyle : public NewOverrideBase<Stat_Default_Mem>
{
};

#endif // #ifdef SF_NO_IME_SUPPORT

}}} // namespace Scaleform { namespace GFx {

#endif //INC_IMEMANAGER_H
