/**************************************************************************

Filename    :   GameState_System.h
Content     :   System game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_System_H
#define INC_GameState_System_H

// Includes
#include "GFx/GFx_Loader.h"
#include "GameState.h"
#include "../GameUIManager.h"
#include "../Views/SystemUIDialog.h"
#include "../GameResourceManager.h"

// ********************************************************************
// This game state is active throughout the game's lifetime and monitors
// for system events such as network errors, etc.
//
class GameState_System : public GameStateWidgetSupport, public GameUIListener
{
public:
   GameState_System(Game* env) 
       : GameStateWidgetSupport(env, "system"), pSystemUIDialog(NULL), bDialogInitialized(false), bClosingDialogView(false) {}
   virtual ~GameState_System()
   {
        if (pSystemUIDialog != NULL)
        {
            delete(pSystemUIDialog);
            pSystemUIDialog = NULL;
        }
   }

    virtual bool    Initialize();
    virtual void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);
    virtual void    Cleanup();

    virtual bool    OnKeyEvent(const GFx::KeyEvent& e);
    virtual bool    OnMouseEvent(const GFx::MouseEvent& e);
    virtual bool    OnCharEvent(const GFx::CharEvent& e);

    virtual bool    OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);
    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value *args, unsigned argCount);
    virtual void    OnPostAdvance(GameUISwfMovie* ui);

    // Show an error dialog
    void            ShowError();
    void            OnSystemDialogFullyLoaded();
    void            CloseDialogImpl();
    void            OnCloseSystemDialog();

protected:
    void            PostWidgetInit();

    Ptr<GFx::MovieDef>      SystemUIDef;
    Ptr<GameUISwfMovie>     SystemUI;
    SystemUIDialog*         pSystemUIDialog;
#ifdef GFX_ENABLE_SOUND
    Ptr<GameSoundManager>   pSoundMgr;
#endif
    GFx::Loader             Loader;
    bool                    bDialogInitialized;
    bool                    bClosingDialogView;
};

#endif  // INC_GameState_System_H
