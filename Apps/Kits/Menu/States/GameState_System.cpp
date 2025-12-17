/**************************************************************************

Filename    :   GameState_System.cpp
Content     :   System game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "GameState_System.h"
#include "../Game.h"
#include "GameState_Start.h"

#ifdef SF_OS_WIN32
#include <tchar.h>
#endif

//////////////////////////////////////////////////////////////////////////

bool    GameState_System::Initialize()
{
    String uiFilename = Env->GetResourceMgr().BuildPath("SystemUI.swf");
    Ptr<SysFile> pf = *new SysFile(uiFilename);
    int len = pf->GetLength();
    if (len <= 0)
    {
        printf("Failed loading file info for '%s'\nPlease check your working directory and try again.\n", uiFilename.ToCStr());
#ifdef SF_OS_WIN32
        char temp[128];
        sprintf_s(temp, ("Failed loading file info for '%s'\nPlease check your working directory and try again.\n"), uiFilename.ToCStr());
        MessageBoxA(NULL, temp, _T("MenuKit Error"), MB_OK);
#endif
        return false;
    }

    Ptr<GameMemoryFile> pmem = *Env->GetResourceMgr().GetMemoryFileCache()->CreateFile(uiFilename, len);
    pf->Read(pmem->GetData(), len);

    Env->GetResourceMgr().InitLoader(Loader);
    
    // Load the systemui.swf
    SystemUI = *SF_HEAP_AUTO_NEW(this) GameUISwfMovie();
    SystemUIDef = *Env->GetResourceMgr().LoadMovieDef(&Loader, Env->GetResourceMgr().BuildPath("SystemUI.swf"));

#ifdef GFX_ENABLE_SOUND
    pSoundMgr = *SF_NEW GameSoundManager(&Env->GetResourceMgr(), Env->GetResourceMgr().BuildPath(""));
    pSoundMgr->LoadSound("defaulterror", "sounds/menuError.wav");
#endif

    return true;
}

void    GameState_System::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);
    if (bClosingDialogView)
    {
        CloseDialogImpl();
        bClosingDialogView = false;
    }
    else
    {
        if (!SystemUI) return;

        SystemUI->Advance(pstats);
    }
}

void    GameState_System::Cleanup()
{
    // Clean up UI assets
    if (!SystemUI || !SystemUI->IsValid()) return;

    SF_DEBUG_MESSAGE(1, "Popping System UI\n");
    Env->GetUIMgr().PopUI();    // PPS: This is not ideal.. Need a mechanism to remove specific swfs instead of a stack?
}

void    GameState_System::OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount)
{
    SF_UNUSED4(ui, methodName, args, argCount);
    // TODO
}


bool    GameState_System::OnKeyEvent(const GFx::KeyEvent& e)
{
     if (!SystemUI->IsValid()) return false;

    if (e.Type == GFx::Event::KeyUp && e.KeyCode == Key::Escape)
    {
        bClosingDialogView = true;
        return true;
    }

    return SystemUI->OnKeyEvent(e);
}

bool    GameState_System::OnCharEvent(const GFx::CharEvent& e)
{
    return SystemUI->OnCharEvent(e);
}

bool    GameState_System::OnMouseEvent(const GFx::MouseEvent& e)
{
    return SystemUI->OnMouseEvent(e);
}

void    GameState_System::OnCloseSystemDialog()
{
    bClosingDialogView = true;
}

void    GameState_System::CloseDialogImpl()
{
    SF_DEBUG_MESSAGE(1, "Popping System UI\n");
    Env->GetUIMgr().PopUI();
    
    if (pSystemUIDialog != NULL)
    {
        delete(pSystemUIDialog);
        pSystemUIDialog = NULL;
        bDialogInitialized = false;
    }
    SystemUI->Destroy();

    // Clear resources used by internal caches
    // PPS: 4.0 doesn't have this functionality..
    //Loader.GetMeshCacheManager()->ClearCache();

    // PPS: There is still memory used by the font cache manager. Unfortunately there
    // is no public Clear() method. Alternative is to recreate the font cache manager
    // state in Loader/Def for the next view creation pass. It may work..

    // PPS: The statement above was relevant to 3.x. Complications from 4.0 have yet
    // to be determined..

    Env->PauseCurrentState(false);
}

void    GameState_System::ShowError()
{
    Env->PauseCurrentState(true);

    if (!SystemUI->IsValid())
    {
        // Initialize the MovieView but don't advance it until we install the widget callbacks.
        if (!SystemUI->Initialize(SystemUIDef, this, true)) 
        {
            fprintf(stderr, "Error: MenuResourceManager failed to create movie instance\n");
        }
        InstallWidgetCallbacks(SystemUI);
        
        if (SystemUI->GetSwf()->GetAVMVersion() == 2)
        {
            GFx::Value nullVal;
            pSystemUIDialog = new SystemUIDialog(nullVal, this);
            pSystemUIDialog->SetMovieView(SystemUI->GetSwf());
            String tempWidgetPath("root1");
            WidgetPathBindings.Set(tempWidgetPath, pSystemUIDialog);
        }

#ifdef GFX_ENABLE_SOUND
        Env->GetResourceMgr().RegisterSoundEventBank(SystemUI, pSoundMgr);
#endif // GFX_ENABLE_SOUND

        SF_DEBUG_MESSAGE(1, "Pushing System UI\n");
        Env->GetUIMgr().PushUI(SystemUI);
    }
}

void    GameState_System::OnSystemDialogFullyLoaded()
{
    pSystemUIDialog->SetTitle("Network Error");
    pSystemUIDialog->SetInfo("A network error has occured. Please press OK to continue.");
    pSystemUIDialog->SetBackButtonLabel("OK");

    pSystemUIDialog->OnViewActivated();
    pSystemUIDialog->OnTopMostView( true );

    bDialogInitialized = true;
}

bool    GameState_System::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget)
{
    bool bHandled = false;
    //printf("\nGameState_System: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);

    if (SystemUI->GetSwf()->GetAVMVersion() == 1)
    {
        if (!SFstrcmp(widgetName, "ErrorDialog"))
        {
            if (pSystemUIDialog == NULL)
            {
                pSystemUIDialog = new SystemUIDialog(widget, this);
                pSystemUIDialog->SetMovieView(SystemUI->GetSwf());
                String tempWidgetPath(widgetPath);
                WidgetPathBindings.Set(tempWidgetPath, pSystemUIDialog);
                bHandled = true;
            }
        }
    }
    else
    {
        if (!SFstrcmp(widgetName, "ErrorDialog"))
        {
            pSystemUIDialog->SetWidgetRef(widget);
        }
    }
    return bHandled;
}

void    GameState_System::OnPostAdvance(GameUISwfMovie* ui)
{
    SF_UNUSED(ui);
    if (bWidgetsInitializedThisFrame)
    {
        PostWidgetInit();
        bWidgetsInitializedThisFrame = false;
    }
}

void    GameState_System::PostWidgetInit()
{
    if (!bDialogInitialized)
    {
        OnSystemDialogFullyLoaded();
    }
}
