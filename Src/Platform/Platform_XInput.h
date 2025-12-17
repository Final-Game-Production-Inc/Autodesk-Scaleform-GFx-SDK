/**************************************************************************

Filename    :   Platform_XInput.h
Content     :   Xbox 360 controller suport
Created     :   June 2, 2009
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_Platform_XInput_H
#define INC_Platform_XInput_H

#include "Platform.h"
#include "GFx_Kernel.h"

#if defined(SF_OS_WIN32)
#include <windows.h>
#include <XInput.h>
#elif defined(SF_OS_XBOX360)
#include <xtl.h>
#endif

// XInput.h may not contain the XUSER_MAX_COUNT constant (if the header file
// comes from Windows SDK and not from DirectX SDK)
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

namespace Scaleform { namespace Platform {

class XInputEventAdapter;

class GamePad
{
public:
    GamePad();
    virtual ~GamePad() {}

	virtual void Update(XInputEventAdapter& app);

    virtual bool IsConnected(unsigned idx) { return Pads[idx].PadConnected; }
    virtual void DetectPads();
    virtual void SetMousePosition(const Point<unsigned> &p);

protected:
    struct Pad
    {
        UInt64  LastMsgTime, LastRepeatTime, RepeatDelay;

        // mouse position
        float                   MouseX, MouseY;
        float                   MouseXadj, MouseYadj;
        XINPUT_STATE            InputState;
        bool                    PadConnected;

        bool					LStickRight, LStickLeft, LStickUp, LStickDown;
        bool					nLStickLeft, nLStickRight, nLStickUp, nLStickDown;

        Pad()
            :LastMsgTime(0), LastRepeatTime(0), RepeatDelay(0),
            MouseX(0), MouseY(0),
            MouseXadj(0), MouseYadj(0) 
        {
            ZeroMemory(&InputState, sizeof(XINPUT_STATE));
            LStickLeft = LStickRight = LStickUp = LStickDown = false;
        }
    } Pads[XUSER_MAX_COUNT];  
    UInt64  LastMsgTime, LastRepeatTime, RepeatDelay;
    
    // mouse position
    float                   MouseX, MouseY;
    float                   MouseXadj, MouseYadj;
    XINPUT_STATE            InputState;
    bool                    PadConnected;
};


// This class is overridden in the platform specific source, to determine the action
// take when XInput generates events.
class XInputEventAdapter
{
public:
    XInputEventAdapter(AppBase* papp) : pApp(papp) { }    
    CursorType GetCursor() const { return pApp->GetCursor(); }
    bool IsConsole() const { return pApp->IsConsole(); }
    Size<unsigned> GetViewSize() const { return pApp->GetViewSize(); }

    virtual void OnPadStick(unsigned controllerIndex, PadKeyCode padCode,
        float xpos, float ypos)
    {
        pApp->OnPadStick(controllerIndex, padCode, xpos, ypos);
    }

    virtual void OnKey(unsigned controllerIndex, KeyCode keyCode,
        unsigned wcharCode, bool downFlag, KeyModifiers mods)
    {
        pApp->OnKey(controllerIndex, keyCode, wcharCode, downFlag, mods);
    }
    
    virtual void OnPad(unsigned controllerIndex, PadKeyCode padCode, bool downFlag)
    {
        pApp->OnPad( controllerIndex, padCode, downFlag );
    }
    
    virtual void OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
        const Point<int>& mousePos, KeyModifiers mods)
    {
        pApp->OnMouseButton(mouseIndex, button, downFlag, mousePos, mods);
    }
    
    virtual void OnMouseMove(unsigned mouseIndex,
        const Point<int>& mousePos, KeyModifiers mods)
    {
        pApp->OnMouseMove(mouseIndex, mousePos, mods);
    }

protected:
    AppBase* pApp;
};

static struct 
{
    WPARAM  winKey;
    KeyCode appKey;
} KeyCodeMap[] = 
{
    {VK_BACK,    Key::Backspace},
    {VK_TAB,     Key::Tab},
    {VK_CLEAR,   Key::Clear},
    {VK_RETURN,  Key::Return},
    {VK_SHIFT,   Key::Shift},
    {VK_CONTROL, Key::Control},
    {VK_MENU,    Key::Alt},
    {VK_PAUSE,   Key::Pause},
    {VK_CAPITAL, Key::CapsLock},
    {VK_ESCAPE,  Key::Escape},
    {VK_SPACE,   Key::Space},
    {VK_PRIOR,   Key::PageUp},
    {VK_NEXT,    Key::PageDown},
    {VK_END,     Key::End},
    {VK_HOME,    Key::Home},
    {VK_LEFT,    Key::Left},
    {VK_UP,      Key::Up},
    {VK_RIGHT,   Key::Right},
    {VK_DOWN,    Key::Down},
    {VK_INSERT,  Key::Insert},
    {VK_DELETE,  Key::Delete},
    {VK_HELP,    Key::Help},
    {VK_NUMLOCK, Key::NumLock},
    {VK_SCROLL,  Key::ScrollLock},

    {VK_OEM_1,     Key::Semicolon},
    {VK_OEM_PLUS,  Key::Equal},
    {VK_OEM_COMMA, Key::Comma},
    {VK_OEM_MINUS, Key::Minus},
    {VK_OEM_PERIOD,Key::Period},
    {VK_OEM_2,     Key::Slash},
    {VK_OEM_3,     Key::Bar},
    {VK_OEM_4,     Key::BracketLeft},
    {VK_OEM_5,     Key::Backslash},
    {VK_OEM_6,     Key::BracketRight},
    {VK_OEM_7,     Key::Quote},
    {VK_OEM_AX,	   Key::OEM_AX},   //  'AX' key on Japanese AX kbd
    {VK_OEM_102,   Key::OEM_102},  //  "<>" or "\|" on RT 102-key kbd.
    {VK_ICO_HELP,  Key::ICO_HELP}, //  Help key on ICO
    {VK_ICO_00,	   Key::ICO_00},	//  00 key on ICO

    {VK_SNAPSHOT,  Key::PrintScreen},
    // Terminator
    {0,            Key::None }
};

}} // Scaleform::Platform
#endif
