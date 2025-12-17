/**************************************************************************

Filename    :   GameStateWidgetSupport.h
Content     :   Game state with CLIK widget handling support for 
                 Menu Kit
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameStateWidgetSupport_H
#define INC_GameStateWidgetSupport_H

// Includes
#include "GameState.h"
#include "../GameUIManager.h"

using namespace Scaleform;

// Forward declarations
class Game;
class GFxCLIKObjectOnLoadEventCallback;


// ********************************************************************
// UI widget callback handler interface
//
class WidgetHandler
{
public:
    virtual ~WidgetHandler() {}

    // ** Invoked when a widget is created/destroyed
    virtual bool OnWidgetInitialized(const char* widgetName, 
                                     const char* widgetPath, 
                                     GFx::Value widget) = 0;

    virtual bool OnWidgetUninitialized(const char* widgetName, 
                                       const char* widgetPath, 
                                       GFx::Value widget) 
    { 
        (void)widget;
        SF_UNUSED(widgetName); 
        SF_UNUSED(widgetPath); 
        return false; 
    }
};


// ********************************************************************
// Game state with support for UI widgets
//
class GameStateWidgetSupport : public GameState, public WidgetHandler
{
    friend class GFxCLIKObjectOnLoadEventCallback;

public:
    GameStateWidgetSupport(Game* env, const char* name) 
        : GameState(env, name), bWidgetsInitializedThisFrame(false) {}
    virtual ~GameStateWidgetSupport() {}

    // Retrieve the widget handler for a specific path
    WidgetHandler*  GetHandlerForPath(const String& path)
    {
        WidgetHandler** pHandler = WidgetPathBindings.Get(path);
        if (pHandler)
        {
            return *pHandler;
        }
        return NULL;
    }

    // Have any widgets initialized since the last update call?
    bool            HaveWidgetsInitializedThisFrame() const
    {
        return bWidgetsInitializedThisFrame; 
    }

protected:
    Hash<String,WidgetHandler*, String::HashFunctor>   WidgetPathBindings;
    bool                            bWidgetsInitializedThisFrame;

    void            InstallWidgetCallbacks(GameUISwfMovie* pswfMovie);
};


#endif  // INC_GameStateWidgetSupport_H
