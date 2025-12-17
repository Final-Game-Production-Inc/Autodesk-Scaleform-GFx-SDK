/**************************************************************************

Filename    :   Game.h
Content     :   Sample 'Game' for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Game_H
#define INC_Game_H

// Includes
#include "GFx/GFx_Event.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "States/GameState_System.h"
#include "FxRenderThread.h"
#include "Render/Render_Types2D.h"

using namespace Scaleform;

// Forward declarations
namespace Scaleform { namespace Render { class  Renderer2D; } }
struct GameUIAdvanceStats;


// ********************************************************************
// Sample game settings
//
class GameSettingsValue
{
public:
    enum ValueType
    {
        TYPE_Boolean,
        TYPE_Integer,
        TYPE_Float,
    };

    explicit GameSettingsValue(unsigned settingsID, bool b) : SettingsID(settingsID), 
                                                 Type(TYPE_Boolean), 
                                                 BoolVal(b) {}
    explicit GameSettingsValue(unsigned settingsID, int n) : SettingsID(settingsID), 
                                                 Type(TYPE_Integer), 
                                                 IntVal(n) {} 
    explicit GameSettingsValue(unsigned settingsID, float f) : SettingsID(settingsID), 
                                                 Type(TYPE_Float),
                                                 FloatVal(f) {}

    ValueType   GetType() const             { return Type; }
    unsigned    GetSettingsID() const       { return SettingsID; }

    bool        GetBool() const             { SF_ASSERT(Type == TYPE_Boolean); return BoolVal; }
    int         GetInt() const              { SF_ASSERT(Type == TYPE_Integer); return IntVal; }
    float       GetFloat() const            { SF_ASSERT(Type == TYPE_Float); return FloatVal; }

protected:
    unsigned        SettingsID;
    ValueType       Type;
    union 
    {
        bool        BoolVal;
        int         IntVal;
        float       FloatVal;
    };
};

class GameSettings : public RefCountBase<GameSettings, Stat_Default_Mem>
{
public: 
    enum SettingsType
    {
        TYPE_Boolean,
        TYPE_NumericRange,
        TYPE_ValueList,
    };

    GameSettings(unsigned id, SettingsType type, const char* name, const char* desc) : 
        ID(id), Type(type), Name(name), Desc(desc) {}
    virtual ~GameSettings() {}

    virtual void                        SetValue(const GameSettingsValue& value) = 0;
    virtual const GameSettingsValue     GetValue() const                         = 0;

    unsigned                    GetID() const       { return ID; }
    const SettingsType&     GetType() const     { return Type; }
    const String&          GetName() const     { return Name; }
    const String&          GetDesc() const     { return Desc; }

protected:
    unsigned            ID;
    SettingsType    Type;
    String         Name;
    String         Desc;
};

class GameSettings_Boolean : public GameSettings
{
public:
    GameSettings_Boolean(unsigned id, const char* name, const char* desc, bool value) 
        : GameSettings(id, TYPE_Boolean, name, desc), Value(value) {}
    virtual ~GameSettings_Boolean() {}

    virtual void                        SetValue(const GameSettingsValue& value)
    { 
        SF_ASSERT(value.GetSettingsID() == ID);
        SF_ASSERT(value.GetType() == GameSettingsValue::TYPE_Boolean);
        Value = value.GetBool();
    }
    virtual const GameSettingsValue     GetValue() const
    { 
        return GameSettingsValue(ID, Value);
    }

protected:
    bool            Value;
};

class GameSettings_NumericRange : public GameSettings
{
public:
    GameSettings_NumericRange(unsigned id, const char* name, const char* desc, float value, float min = 0.f, 
                              float max = 100.f, float increment = 1.f) 
        : GameSettings(id, TYPE_NumericRange, name, desc), Value(value), MinimumValue(min), MaximumValue(max),
          Increment(increment) {}
    virtual ~GameSettings_NumericRange() {}

    virtual void                        SetValue(const GameSettingsValue& value)
    { 
        SF_ASSERT(value.GetSettingsID() == ID);
        SF_ASSERT(value.GetType() == GameSettingsValue::TYPE_Float);
        SF_ASSERT(value.GetFloat() >= MinimumValue && value.GetFloat() <= MaximumValue);
        Value = value.GetFloat();
    }
    virtual const GameSettingsValue     GetValue() const
    { 
        return GameSettingsValue(ID, Value);
    }

    float       GetRangeValue() const       { return Value; }
    float       GetRangeMin() const         { return MinimumValue; }
    float       GetRangeMax() const         { return MaximumValue; }
    float       GetRangeIncrement() const   { return Increment; }

protected:
    float       Value;
    float       MinimumValue;
    float       MaximumValue;
    float       Increment;
};

class GameSettings_ValueList : public GameSettings
{
public:
    typedef     Array<String>     ValueListType;

    GameSettings_ValueList(unsigned id, const char* name, const char* desc, const char** args, UPInt numArgs, UPInt selectedIndex) 
        : GameSettings(id, TYPE_ValueList, name, desc), SelectedIndex(selectedIndex)
    {
        for (UPInt i=0; i < numArgs; i++)
            Values.PushBack( *(args+i) );
    }
    virtual ~GameSettings_ValueList() {}

    virtual void                        SetValue(const GameSettingsValue& value)
    { 
        SF_ASSERT(value.GetSettingsID() == ID);
        SF_ASSERT(value.GetType() == GameSettingsValue::TYPE_Integer);
        SF_ASSERT(value.GetInt() >= 0 && value.GetInt() < (int)Values.GetSize());
        SelectedIndex = (UPInt)value.GetInt();
    }
    virtual const GameSettingsValue     GetValue() const
    { 
        return GameSettingsValue(ID, (int)SelectedIndex);
    }

    const ValueListType&    GetValues() const           { return Values; }
    UPInt                   GetSelectedIndex() const    { return SelectedIndex; }

protected:
    ValueListType       Values;
    UPInt               SelectedIndex;
};



// ********************************************************************
// Sample game
//
class Game : public GameResourceManager::AccessListener
{
public:
    Game();
    virtual ~Game()     {}
    
    // Initialization and cleanup routines; called by FxPlayerApp 
#ifdef GFX_ENABLE_SOUND
    bool    Initialize(FxRenderThread* prenderer, Sound::SoundRenderer* psndrenderer);
#else
    bool    Initialize(FxRenderThread* prenderer);
#endif
    void    Shutdown();

    // Called by FxPlayerApp to tick game states and UIs
    void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);

    // Returns true if the game is still active
    bool    IsRunning() const   { return bRunning; }
    void    QuitGame()          { bRunning = false; }

    // Accessors for asset managers
    GameResourceManager&    GetResourceMgr()    { return ResourceMgr; }
    GameUIManager&          GetUIMgr()          { return UIMgr; }

    void    UpdateViewSize(Render::Size<unsigned> size);

    // Called for specific systems events to be handled by game states
    void    OnMouseEvent(const GFx::MouseEvent& e);
    void    OnKeyEvent(const GFx::KeyEvent& e);
    void    OnCharEvent(const GFx::CharEvent& e);

    void    SetGFxUserEventHandler(GFx::UserEventHandler* handler)
    {
        ResourceMgr.SetUserEventHandler(handler);
    }

    // Game state management functions
    void        SetCurrentState(GameState* state)
    {
        if (HasCurrentState()) 
        {
            CurrentState->Cleanup();
            // We store the old state in transient storage to avoid releasing it at this point.
            // The state may have called this method and expects to be alive until its 
            // execution has completed. TrashedStates is cleared in Game::Update.
            TrashedStates.PushBack(CurrentState);
        }
        CurrentState = *state;
        if (!CurrentState->Initialize()) 
        {
            bRunning = false;
        } 
        else 
        {
            // Update the new state with the current mouse cursor position
            CurrentState->OnMouseEvent(GFx::MouseEvent(GFx::Event::MouseMove, 0, LastMouseX, LastMouseY));
        }
    }
    bool        HasCurrentState() const         { return CurrentState.GetPtr() != NULL; }
    void        PauseCurrentState(bool pause) 
    {
        if (HasCurrentState())
            CurrentState->Pause(pause);
    }

    // Retrieve the memory heap used for general allocation
    MemoryHeap*        GetMainMenuHeap() const
    {
        // This can be changed to suit the target environment/platform/setup
        return  Memory::GetGlobalHeap();
    }

    typedef ArrayCPP< Ptr<GameSettings> >     SettingsArray;
    const SettingsArray&     GetSettings() const     { return Settings; }

    // ** ResourceManager access listener interface
    void        OnIoLock()
    {
        SF_ASSERT(HasCurrentState());
        CurrentState->OnIoLock();
    }
    void        OnIoUnlock()
    {
        SF_ASSERT(HasCurrentState());
        CurrentState->OnIoUnlock();
    }

    bool IsVideoEnabled() const { return VideoEnabled; }
    bool AreVideosAvailable() const { return VideosAvailable; }

private:
    GameResourceManager             ResourceMgr;
    GameUIManager                   UIMgr;

    SettingsArray                   Settings;

    // The active game state; all events will be passed to this state first
    Ptr<GameState>                  CurrentState;
    // Transient storage to avoid freeing the states inside SetCurrentState
    ArrayCPP< Ptr<GameState> >      TrashedStates;

    // Special game state for the system
    Ptr<GameState_System>           SystemState;

    Render::Size<unsigned>          Viewport;
    bool                            bRunning;

    float                           LastMouseX, LastMouseY;
    bool                            VideoEnabled, VideosAvailable;

    // System event handlers
    // ** These are invoked via special key combinations for simulation purposes
    void                            OnNetworkError();
};

#endif  // INC_Game_H
