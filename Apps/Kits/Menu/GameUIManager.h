/**************************************************************************

Filename    :   GameUIManager.h
Content     :   Sample UI Manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameUIManager_H
#define INC_GameUIManager_H

// Includes
#include "GFx/GFx_Player.h"
#include "Render/Renderer2D.h"
#include "Render/Render_Types2D.h"

using namespace Scaleform;

// Forward declarations
class GameResourceManager;
class GameUIListener;
class GameUICallbackProxy;


// ********************************************************************
// Stats collected by the UI Manager
//
struct GameUIAdvanceStats
{
    UInt64  AdvanceTicks;   // Ticks spent advancing SWFs
};


// ********************************************************************
// A SWF managed by the UI manager
//
class GameUISwfMovie : public RefCountBase<GameUISwfMovie, Stat_Default_Mem>
{
    friend class GameUICallbackProxy;

public:
    GameUISwfMovie() : Listener(NULL) {}
    virtual ~GameUISwfMovie();

    // Create a SWF movie instance with the provided SWF definition and install the 
    // callback listener.
    bool    Initialize(GFx::MovieDef* movieDef, GameUIListener* xi, bool bNoAdvanceOnInit = false);
    // Destroy the SWF movie instance and cleanup other resources.
    void    Destroy();

    // Set the SWF viewport dimensions.
    void    SetViewport(GFx::Size<unsigned> size);

    // Called by the application to advance the SWF
    void    Advance(GameUIAdvanceStats* pstats);

    // Pauses/unpauses the SWF
    void    Pause(bool pause);

    // Called for specific system events that should be routed to the SWF
    bool    OnMouseEvent(const GFx::MouseEvent& e);
    bool    OnKeyEvent(const GFx::KeyEvent& e);
    bool    OnCharEvent(const GFx::CharEvent& e);

    // Returns true if the SWF instance is not valid
     bool    IsValid() const;

    // Returns the SWF instance
    GFx::Movie*   GetSwf() const  { return pSwf; }

protected:
    Ptr<GFx::MovieDef>      pSwfDef;
    Ptr<GFx::Movie>         pSwf;
    GFx::MovieInfo          SwfInfo;

    GameUIListener* Listener;

    UInt64                  TimeTicks;          // Current ticks
    UInt64                  NextTicksTime;      // Ticks when next advance should be called.
    UInt64                  MovieLastTicks;    
    UInt64                  MovieTicks;
};


// ********************************************************************
// The game's UI Manager
//
class GameUIManager
{
public:
    GameUIManager();
    ~GameUIManager();
    
    void    Initialize(GameResourceManager* presourceMgr);
    void    Shutdown();

    void    SetViewport(GFx::Size<unsigned>& viewport);

    // Methods to manage the UI stack. The top most SWF will receive all events and be
    // the currently focused UI element.
    void    PushUI(GameUISwfMovie* pswf);
    void    PopUI();

private:
    GameResourceManager*                pResourceMgr;
    
    // Stack of UIs; the topmost UI is considered 'active' and will recieve all events
    ArrayCPP< Ptr<GameUISwfMovie> >   UIStack;
};


// ********************************************************************
// Custom callback delegate for SWF XI callbacks and SWF state events
//
class GameUIListener
{
public:
    virtual ~GameUIListener() {}

    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount) = 0;

    virtual void    OnPreAdvance(GameUISwfMovie* ui)      { SF_UNUSED(ui); }
    virtual void    OnPostAdvance(GameUISwfMovie* ui)     { SF_UNUSED(ui); }
};


#endif  // INC_GameUIManager_H
