/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Event.h
Content     :   Public interface for SWF Movie input events
Created     :   June 21, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX__Event_H
#define INC_SF_GFX__Event_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Stats.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_KeyCodes.h"
#include "Kernel/SF_String.h"

#include <stdlib.h>
#include <string.h>

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class Event;
class MouseEvent;
class KeyEvent;
class CharEvent;
class AppLifecycleEvent;
class KeyboardState;
class EventId;
class AccelerometerEvent;
class GeolocationEvent;

// Forward declarations.
class InteractiveObject;

// ***** Debug check support

// This expression will only be avaluated in debug mode.
#ifdef SF_BUILD_DEBUG
#define SF_DEBUG_EXPR(arg) arg
#else
#define SF_DEBUG_EXPR(arg) ((void)0)
#endif

// ***** Event classes


// Event is a base class for input event notification. Objects of this type
// can be passed to Movie::HandleEvent to inform the movie of various input
// actions. Events that have specific subclasses, such as MouseEvent, must
// use those subclasses and NOT the base Event class.

class Event : public NewOverrideBase<Stat_Default_Mem>
{
public:
    enum EventType
    {
        Unknown,       
        // Informative events sent to the player.
        MouseMove,
        MouseDown,
        MouseUp,
        MouseWheel,
        KeyDown,
        KeyUp,
        SceneResize,
        SetFocus,
        KillFocus,
        TouchBegin,
        TouchEnd,
        TouchMove,
        TouchTap,
        
        GestureBegin,
        Gesture,
        GestureEnd,
        GestureSimple,

        GamePadAnalog,

        OrientationChanged,
        AppLifecycle,

        // Action events, to be handled by user.
        DoShowMouse,
        DoHideMouse,
        DoSetMouseCursor,

        EnableClipping,
        DisableClipping,

		Accelerometer,
		Geolocation,

		Status,

        Char,
        IME
    };

    // What kind of event this is.
    EventType       Type;
    // State of special keys
    KeyModifiers    Modifiers;

    // Size of class, used in debug build to verify that
    // appropriate classes are used for messages.
#ifdef SF_BUILD_DEBUG
	unsigned EventClassSize;
#endif

    // must be explicit ctor but for compatibility reasons commenting 
    // 'explicit' out.
    /*explicit*/ Event(EventType eventType = Unknown)
    {
        Type = eventType;
        SF_DEBUG_EXPR(EventClassSize = sizeof(Event));
    }
    Event(EventType eventType, KeyModifiers keysState)
    {
        Type = eventType;
        Modifiers = keysState;
        SF_DEBUG_EXPR(EventClassSize = sizeof(Event));
    }
};


class MouseEvent : public Event
{
public:
    float    x, y;
    float    ScrollDelta;
    unsigned Button;
    unsigned MouseIndex;

    MouseEvent() : Event()
    {
        Button = 0; x = 0; y = 0; ScrollDelta = 0.0f; MouseIndex = 0;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
    // User event constructor - x, y must always be filled in.
    MouseEvent(EventType eventType, unsigned button,
        float xpos, float ypos, float scrollVal = 0.0f, unsigned mouseIdx = 0)
        : Event(eventType)
    {
        Button = button; x = xpos; y = ypos; ScrollDelta = scrollVal;
        MouseIndex = mouseIdx;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
    MouseEvent(EventType eventType, unsigned mouseIdx)
        : Event(eventType)
    {
        Button = 0; x = y = 0; ScrollDelta = 0;
        MouseIndex = mouseIdx;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
};

class TouchEvent : public Event 
{
public:
    float    x, y;
    unsigned TouchPointID;      // ID of the touch point
    float    Pressure;          // 0..1
    float    WContact, HContact; // Width and height of contact area, in pixels
    UInt32   TimeStamp;         // A timestamp of event, milliseconds
    bool     PrimaryPoint;

    TouchEvent() : Event()
    {
        x = y = WContact = HContact = 0;
        TouchPointID = 0;
        Pressure = 1.0f;
        TimeStamp = 0;
        PrimaryPoint = false;
        SF_DEBUG_EXPR(EventClassSize = sizeof(TouchEvent));
    }
    TouchEvent(EventType evtType, unsigned id, float _x, float _y, 
               float wcontact = 0, float hcontact = 0, bool primary = true, 
               float pressure = 1.0f) : Event(evtType)
    {
        x = _x;
        y = _y;
        WContact = wcontact;
        HContact = hcontact;
        TouchPointID = id;
        Pressure = pressure;
        TimeStamp = 0;
        PrimaryPoint = primary;
        SF_DEBUG_EXPR(EventClassSize = sizeof(TouchEvent));
    }
};


class AccelerometerEvent : public Event 
{
public:
	int		idAcc;
    double	timestamp;
	double	accelerationX, accelerationY, accelerationZ;

    AccelerometerEvent() : Event()
    {
		idAcc = -1;
        timestamp = accelerationX = accelerationY = accelerationZ = 0;
        SF_DEBUG_EXPR(EventClassSize = sizeof(AccelerometerEvent));
    }

    AccelerometerEvent(EventType evtType, int _idAcc, double _timestamp, double _accelerationX, double _accelerationY, double _accelerationZ) 
		: Event(evtType)
    {
		idAcc = _idAcc;
		timestamp = _timestamp;
        accelerationX = _accelerationX;
        accelerationY = _accelerationY;
        accelerationZ = _accelerationZ;
        SF_DEBUG_EXPR(EventClassSize = sizeof(AccelerometerEvent));
    }
};


class GeolocationEvent : public Event 
{
public:
	int idGeo;
	double latitude, longitude, altitude;
	double hAccuracy, vAccuracy, speed, heading, timestamp;

    GeolocationEvent() : Event()
    {
		idGeo = -1;
        latitude = longitude = altitude = hAccuracy = vAccuracy = speed = heading = timestamp = 0;
        SF_DEBUG_EXPR(EventClassSize = sizeof(GeolocationEvent));
    }

    GeolocationEvent(EventType evtType, int _idGeo, double _latitude, double _longitude, double _altitude, double _hAccuracy, double _vAccuracy, double _speed, double _heading, double _timestamp) 
		: Event(evtType)
    {
		idGeo = _idGeo;
		latitude = _latitude;
		longitude = _longitude;
		altitude = _altitude;
		hAccuracy = _hAccuracy;
		vAccuracy = _vAccuracy;
		speed = _speed;
		heading = _heading;
		timestamp = _timestamp;
        SF_DEBUG_EXPR(EventClassSize = sizeof(GeolocationEvent));
    }
};


class StatusEvent : public Event 
{
public:
    String*	code;
	String*	level;
	String*	extensionId;
	String*	contextId;

    StatusEvent(EventType evtType, String* _code, String* _level, String* _extensionId, String* _contextId) 
		: Event(evtType)
    {
		code = _code;
		level = _level;
		extensionId = _extensionId;
		contextId = _contextId;
        SF_DEBUG_EXPR(EventClassSize = sizeof(StatusEvent));
    }
};

class GestureEvent : public Event
{
public:
    // IMPORTANT: these bits should be exactly the same as in Platform.h!
    enum GestureMaskBits
    {
        GestureBit_Unknown      = 0,
        GestureBit_Pan          = 0x01,
        GestureBit_Zoom         = 0x02,
        GestureBit_Rotate       = 0x04,
        GestureBit_Swipe        = 0x08,
        GestureBit_PressAndTap  = 0x10, // only x,y are meaningful
        GestureBit_TwoFingerTap = 0x20, // only x,y are meaningful

        GestureBit_Last         = GestureBit_TwoFingerTap,
        GestureBit_All          = 0xFF
    };
    UInt32   GestureMask;
    float    x, y;
    float    OffsetX, OffsetY;
    float    ScaleX, ScaleY;
    float    Rotation;

    GestureEvent() : Event(Gesture)
    {
        GestureMask = GestureBit_All;
        x = y = 0;
        OffsetX = OffsetY = Rotation = 0;
        ScaleX = ScaleY = 1.0f;
        SF_DEBUG_EXPR(EventClassSize = sizeof(GestureEvent));
    }
    GestureEvent(EventType evtType) : Event(evtType)
    {
        GestureMask = GestureBit_All;
        x = y = 0;
        OffsetX = OffsetY = Rotation = 0;
        ScaleX = ScaleY = 1.0f;
        SF_DEBUG_EXPR(EventClassSize = sizeof(GestureEvent));
    }
    GestureEvent(EventType evtType, UInt32 gestureMask, float _x, float _y, 
        float offX = 0, float offY = 0, float scaleX = 1.0f, float scaleY = 1.0f, float rotation = 0) 
        : Event(evtType), 
        GestureMask(gestureMask), x(_x), y(_y), OffsetX(offX), OffsetY(offY), 
		ScaleX(scaleX), ScaleY(scaleY), Rotation(rotation)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(GestureEvent));
    }
};

// Event that should be send if orientation of a mobile device is changed
class OrientationEvent : public Event
{
public:
    enum OrientationType
    {
        Default, // right side up
        RotatedLeft,
        RotatedRight,
        UpsideDown,
        Unknown
    } Orientation;

    OrientationEvent(OrientationType t = Default) : Event(OrientationChanged), Orientation(t)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(OrientationEvent));
    }
};

class AppLifecycleEvent : public Event
{
public:
    enum LifecycleEventType
    {
        OnPause,
        OnResume
    } Status;

    AppLifecycleEvent(LifecycleEventType t) : Event(AppLifecycle), Status(t)
	{
		SF_DEBUG_EXPR(EventClassSize = sizeof(AppLifecycleEvent));
	}
};

class ClippingEvent : public Event
{
public:
    ClippingEvent() : Event(Event::EnableClipping) {}
    ClippingEvent(EventType eventType) : Event(eventType) {}
};

class MouseCursorEvent : public Event
{
public:
    enum CursorShapeType
    {
        ARROW = 0,
        HAND  = 1,
        IBEAM = 2,
        BUTTON = 3
    };
    unsigned mCursorShape;
    unsigned MouseIndex;

    MouseCursorEvent() : Event(Event::DoSetMouseCursor), mCursorShape(HAND), 
        MouseIndex(0)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }
    MouseCursorEvent(CursorShapeType cursorShape, unsigned mouseIndex)
        : Event(Event::DoSetMouseCursor), mCursorShape(cursorShape), 
        MouseIndex(mouseIndex)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }
    MouseCursorEvent(EventType eventType, unsigned mouseIndex)
        : Event(eventType), mCursorShape(ARROW), 
        MouseIndex(mouseIndex)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }

    CursorShapeType GetCursorShape() const { return (CursorShapeType)mCursorShape; }
};

class GamePadAnalogEvent : public Event
{
public:
    UInt32      Code;
    UInt8       ControllerIndex;
    float       AnalogX, AnalogY;

    GamePadAnalogEvent(UInt32 code, float analogX, float analogY = 0, UInt8 controllerIdx = 0)
        : Event(Event::GamePadAnalog), Code(code), ControllerIndex(controllerIdx), AnalogX(analogX), AnalogY(analogY)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(GamePadAnalogEvent));
    }
};

class KeyEvent : public Event
{
public:
    // Key, and whether it was pressed up or down
    Key::Code     KeyCode;
    // ASCII code. 0, if unavailable.
    UByte       AsciiCode; // left for compatibility reasons only
    unsigned    WcharCode; // left for compatibility reasons only

    // The index of the physical keyboard controller.
    UInt8       KeyboardIndex;

    KeyEvent(EventType eventType = Unknown, Key::Code code = Key::None, 
             UByte asciiCode = 0, UInt32 wcharCode = 0,
             UInt8 keyboardIndex = 0)
        : Event(eventType), KeyCode (code), AsciiCode (asciiCode), WcharCode(wcharCode),
        KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(KeyEvent));
    }

    KeyEvent(EventType eventType, Key::Code code, UByte asciiCode, UInt32 wcharCode,
             KeyModifiers mods, UInt8 keyboardIndex = 0)
        : Event(eventType, mods), KeyCode (code), AsciiCode (asciiCode), WcharCode(wcharCode),
          KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(KeyEvent));
    }
};

class CharEvent : public Event
{
public:
    UInt32              WcharCode;
    // The index of the physical keyboard controller.
    UInt8               KeyboardIndex;

    CharEvent(UInt32 wcharCode, UInt8 keyboardIndex = 0)
        : Event(Char), WcharCode(wcharCode), KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(CharEvent));
    }
};

#ifdef GFX_ENABLE_KEYBOARD
// Keyboard state and queue class. 
class KeyboardState : public RefCountBase<KeyboardState, Stat_Default_Mem>
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void OnKeyDown(InteractiveObject *pmovie, const EventId& evt, int keyMask) = 0;
        virtual void OnKeyUp(InteractiveObject *pmovie, const EventId& evt, int keyMask) = 0;
        virtual void Update(const EventId& evt) = 0;
    };

protected:
    IListener*          pListener;

    class KeyQueue 
    {
    private:
        enum { KeyQueueSize = 100 };
        struct KeyRecord 
        {
            UInt32              wcharCode;
            short               code;
            Event::EventType    event;
            UByte               ascii;
            KeyModifiers        keyMods;
        }               Buffer[KeyQueueSize];
        unsigned        PutIdx;
        unsigned        GetIdx;
        unsigned        Count;
    public:
        KeyQueue ();

        void Put(short code, UByte ascii, UInt32 wcharCode, Event::EventType event, KeyModifiers mods = 0);
        bool Get(short* code, UByte* ascii, UInt32* wcharCode, Event::EventType* event, KeyModifiers* mods = 0);

        bool IsEmpty () const { return Count == 0; }

        void ResetState();

    } KeyQueue;

    UInt8               KeyboardIndex;

    UByte               Keymap[Key::KeyCount / 8 + 1];   // bit-Array
    bool                Toggled[3];
public:

    KeyboardState();

    void    SetKeyboardIndex(UInt8 ki) { SF_ASSERT(ki < GFX_MAX_KEYBOARD_SUPPORTED); KeyboardIndex = ki; }
    UInt8   GetKeyboardIndex() const   { return KeyboardIndex; }

    bool    IsKeyDown(int code) const;
    bool    IsKeyToggled(int code) const;
    void    SetKeyToggled(int code, bool toggle);
    void    SetKeyDown(int code, UByte ascii, KeyModifiers mods = 0, bool putInQueue = true);
    void    SetKeyUp(int code, UByte ascii, KeyModifiers mods = 0, bool putInQueue = true);
    void    SetChar(UInt32 wcharCode);

    bool    IsQueueEmpty() const { return KeyQueue.IsEmpty(); }
    bool    GetQueueEntry(short* code, 
                          UByte* ascii, 
                          UInt32* wcharCode, 
                          Event::EventType* event, 
                          KeyModifiers* mods = 0);

    void    CleanupListener();
    void    SetListener(IListener* listener);

    void    NotifyListeners(InteractiveObject *pmovie, 
                            const EventId& evt,
                            int keyMask) const;
    void    UpdateListeners(const EventId& evt);

    void    ResetState();

    KeyModifiers GetKeyModifiers() const;
};
#else
class KeyboardState : public RefCountBase<KeyboardState, Stat_Default_Mem>
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void OnKeyDown(InteractiveObject *, const EventId&, int) {}
        virtual void OnKeyUp(InteractiveObject *, const EventId&, int) {}
        virtual void Update(const EventId&) {}
    };
};
#endif //GFX_ENABLE_KEYBOARD

class IMEEvent : public Event
{
public:
    enum IMEEventType
    {
        IME_Default,
        IME_PreProcessKeyboard
    } IMEEvtType;

    IMEEvent(IMEEventType t = IME_Default) : Event(IME), IMEEvtType(t)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEEvent));
    }
};

class IMEWin32Event : public IMEEvent
{
public:
    unsigned Message;  // Win32 message id (WM_<>)
    UPInt    WParam;
    UPInt    LParam;
    UPInt    hWND;
    int      Options;

    IMEWin32Event(IMEEventType t = IME_Default) : IMEEvent(t), 
        Message(0), WParam(0), LParam(0), hWND(0), Options(0)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEWin32Event));
    }
    IMEWin32Event(IMEEventType t, UPInt hwnd, unsigned win32msg, UPInt wp, UPInt lp, int options = 0) : IMEEvent(t),
        Message(win32msg), WParam(wp), LParam(lp), hWND(hwnd), Options(options)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEWin32Event));
    }
};

class IMEXBox360Event : public IMEEvent
{
public:
    // Unpacked XINPUT_KEYSTROKE structure
    UInt16   VirtualKey;
    wchar_t  Unicode;
    UInt16   Flags;
    UInt8    UserIndex;
    UInt8    HidCode;
    void*    pLastKey;
    unsigned eType; 

    IMEXBox360Event(UInt16 virtualKey, wchar_t unicode, UInt16 flags, UInt8 userIndex, UInt8 hidCode, void* plastKey,
        unsigned etype) : IMEEvent(IME_Default), VirtualKey(virtualKey), Unicode(unicode), Flags(flags),
        UserIndex(userIndex), HidCode(hidCode), pLastKey(plastKey), eType(etype)

    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEXBox360Event));
    }
};

class IMEPS3Event : public IMEEvent
{
public:

    IMEPS3Event(): KeyCode(0), CharCode(0), Modifiers(0), LastKeyCode(0), LastCharCode(0), LastModifiers(0){};
    IMEPS3Event(unsigned keyCode, unsigned code, unsigned mods, unsigned lastKeyCode, unsigned lastCharCode, unsigned lastMods)
    {
        KeyCode         = keyCode;
        CharCode        = code;
        Modifiers       = mods;
        LastKeyCode     = lastKeyCode;
        LastCharCode    = lastCharCode;
        LastModifiers   = lastMods;
    };
    unsigned KeyCode;
    unsigned CharCode;
    unsigned Modifiers;
    mutable unsigned LastKeyCode;
    mutable unsigned LastCharCode;
    mutable unsigned LastModifiers; 
};

class IMEMacEvent : public IMEEvent
{
public:

    IMEMacEvent(): compositionString(NULL), attributes(NULL), eType(0){};
    IMEMacEvent(char* compString, char* attrib, unsigned etype)
    {
        compositionString   = compString;
        attributes          = attrib;
        eType               = etype;
    };

    char*     compositionString;
    char*     attributes;
    unsigned  eType;
};

// Event to use when movie's window is getting focus. Special key state
// should be provided in order to maintain correct state of keys such as
// CapsLock, NumLock, ScrollLock.
class SetFocusEvent : public Event
{
public:
    // State of special keys
    KeyModifiers KeysStates[GFX_MAX_KEYBOARD_SUPPORTED];

    SetFocusEvent() : Event(SetFocus) {}

    SetFocusEvent(KeyModifiers specialKeysState) : 
      Event(SetFocus, specialKeysState) { KeysStates[0] = specialKeysState; }

    SetFocusEvent(unsigned numKeyboards, KeyModifiers* specialKeysStates) 
      : Event(SetFocus) 
    {
        if (numKeyboards > 0)
            Modifiers = specialKeysStates[0];
        for (unsigned i = 0, n = Alg::Min(unsigned(GFX_MAX_KEYBOARD_SUPPORTED), numKeyboards); i < n; ++i)
            KeysStates[i] = specialKeysStates[i];
    }
};

}} // Scaleform::GFx

#endif
