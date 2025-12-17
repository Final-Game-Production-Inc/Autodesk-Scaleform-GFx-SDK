/**************************************************************************

Filename    :   GFx_Input.cpp
Content     :   Input handling
Created     :   
Authors     :   Artem Bolgar

Notes       :   This file contains class declarations used in
GFxPlayerImpl.cpp only. Declarations that need to be
visible by other player files should be placed
in GFxCharacter.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_Input.h"

namespace Scaleform { namespace GFx {

///////////////////////////
// *** MouseState
//
MouseState::MouseState()
{
    ResetState();
}

void MouseState::ResetState()
{
    mPresetCursorType      = ~0u; // no preset cursor type
    CursorType             = MouseCursorEvent::ARROW;
    CurButtonsState        = PrevButtonsState = 0;
    LastPosition.x         = LastPosition.y   = 0;
    MouseInsideEntityLast  = false;
    TopmostEntityIsNull    = PrevTopmostEntityWasNull = false;
    Activated = MouseMoved = false;
    WheelDelta             = 0;
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    TouchID                = ~0u;
    ContactSize            = SizeF(0,0);
    Pressure               = 1.0f;
    PrimaryPoint           = false;
#endif
}

void MouseState::UpdateState(const InputEventsQueue::QueueEntry& qe)
{
    Activated = true;
    PrevButtonsState = CurButtonsState;
    if (qe.GetMouseEntry().IsButtonsStateChanged())
    {
        if (qe.GetMouseEntry().IsAnyButtonReleased())
        {
            CurButtonsState &= (~(qe.GetMouseEntry().ButtonsState & MouseButton_AllMask));
        }
        else
        {
            unsigned bs = qe.GetMouseEntry().ButtonsState;
            CurButtonsState |= (bs & MouseButton_AllMask);
        }
    }

    if (qe.GetMouseEntry().IsMouseWheel())
        WheelDelta = qe.GetMouseEntry().WheelScrollDelta;
    else
        WheelDelta = 0;

    // update MouseMoved and LastPos
    if (((int)qe.GetMouseEntry().GetPosition().x != (int)LastPosition.x || 
        (int)qe.GetMouseEntry().GetPosition().y != (int)LastPosition.y))
        MouseMoved = true;
    else
        MouseMoved = false;
    LastPosition  = qe.GetMouseEntry().GetPosition();
}

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
void MouseState::UpdateState(const InputEventsQueueEntry::TouchEntry& te)
{
    Activated = true;
	TouchID   = te.TouchPointID;
    Pressure  = te.Pressure;
    ContactSize = SizeF(te.WContact, te.HContact);
    PrimaryPoint= te.PrimaryPoint;
    PrevButtonsState = CurButtonsState;
    if (te.Type == InputEventsQueueEntry::Touch_Begin)
        CurButtonsState |= 1;
    else if (te.Type == InputEventsQueueEntry::Touch_End)
        CurButtonsState &= ~1;

    // update MouseMoved and LastPos
    if (((int)te.GetPosition().x != (int)LastPosition.x || 
         (int)te.GetPosition().y != (int)LastPosition.y))
        MouseMoved = true;
    else
        MouseMoved = false;
    LastPosition  = te.GetPosition();

}
#endif

void MouseState::SetMouseButtonDownEntity(unsigned buttonIdx, InteractiveObject* pch)
{
    if (buttonIdx >= MouseButtonDownEntities.GetSize())
        MouseButtonDownEntities.Resize(buttonIdx + 1);
    MouseButtonDownEntities[buttonIdx] = pch;
}

Ptr<InteractiveObject> MouseState::GetMouseButtonDownEntity(unsigned buttonIdx) const  
{ 
    if (buttonIdx >= MouseButtonDownEntities.GetSize())
        return NULL;
    return MouseButtonDownEntities[buttonIdx];
}

//////////////////////////////////////////////////////////////////////////
//
// InputEventsQueue
InputEventsQueue::InputEventsQueue():
StartPos(0), UsedEntries(0), LastMousePosMask(0) 
{
}

const InputEventsQueue::QueueEntry* InputEventsQueue::PeekLastQueueEntry() const 
{ 
    if (UsedEntries == 0) 
        return NULL;
    SF_ASSERT(StartPos < Queue_Length);
    UPInt idx = StartPos + UsedEntries - 1;
    if (idx >= Queue_Length)
        idx -= Queue_Length;
    SF_ASSERT(idx < Queue_Length);
    return &Queue[idx];
}

InputEventsQueue::QueueEntry* InputEventsQueue::AddEmptyQueueEntry()
{
    if (UsedEntries == Queue_Length)
    {
        // queue is full!
        // just skip the first entry to make room for new entry
        ++StartPos;
        --UsedEntries;
        if (StartPos == Queue_Length)
            StartPos = 0;
    }
    SF_ASSERT(UsedEntries < Queue_Length);
    UPInt idx = StartPos + UsedEntries;
    if (idx >= Queue_Length)
        idx -= Queue_Length;
    SF_ASSERT(idx < Queue_Length);
    ++UsedEntries;
    return &Queue[idx];
}

void InputEventsQueue::AddMouseMove(unsigned mouseIndex, const PointF& pos) 
{ 
    if (mouseIndex < GFX_MAX_MICE_SUPPORTED)
    {
        SF_ASSERT(pos.x != SF_MIN_FLOAT && pos.y != SF_MIN_FLOAT);

        LastMousePosMask |= (1 << mouseIndex);
        LastMousePos[mouseIndex].x = pos.x;
        LastMousePos[mouseIndex].y = pos.y;
    }
}

void InputEventsQueue::AddMouseButtonEvent(unsigned mouseIndex, const PointF& pos, unsigned buttonsSt, unsigned flags)
{
    // reset last position, since the record being added contains more recent position.
    if (pos.x != SF_MIN_FLOAT)
        LastMousePosMask &= ~(1 << mouseIndex);

    QueueEntry* pqe           = AddEmptyQueueEntry();
    pqe->t                    = QueueEntry::QE_Mouse;
    QueueEntry::MouseEntry& mouseEntry = pqe->GetMouseEntry();
    mouseEntry.MouseIndex     = (UInt8)mouseIndex;
    mouseEntry.PosX           = pos.x;
    mouseEntry.PosY           = pos.y;
    mouseEntry.ButtonsState   = (ButtonsStateType)buttonsSt;
    mouseEntry.Flags          = (UInt8)flags;
}

void InputEventsQueue::AddMouseWheel(unsigned mouseIndex, const PointF& pos, int delta) 
{ 
    // reset last position, since the record being added contains more recent position.
    if (pos.x != SF_MIN_FLOAT)
        LastMousePosMask &= ~(1 << mouseIndex);

    QueueEntry* pqe             = AddEmptyQueueEntry();
    pqe->t                      = QueueEntry::QE_Mouse;
    QueueEntry::MouseEntry& mouseEntry = pqe->GetMouseEntry();
    mouseEntry.MouseIndex       = (UInt8)mouseIndex;
    mouseEntry.PosX             = pos.x;
    mouseEntry.PosY             = pos.y;
    mouseEntry.WheelScrollDelta = (SInt8)delta;
    mouseEntry.ButtonsState     = 0;
    mouseEntry.Flags            = QueueEntry::MouseButton_Wheel;
}

void InputEventsQueue::AddKeyEvent
    (UInt32 code, UByte ascii, UInt32 wcharCode, bool isKeyDown, KeyModifiers specialKeysState,
     UInt8 keyboardIndex)
{
    QueueEntry* pqe           = AddEmptyQueueEntry();
    pqe->t                    = QueueEntry::QE_Key;
    QueueEntry::KeyEntry& keyEntry = pqe->GetKeyEntry();
    keyEntry.Code             = code;
    keyEntry.AsciiCode        = ascii;
    keyEntry.WcharCode        = wcharCode;
    keyEntry.KeysState        = specialKeysState.States;
    keyEntry.KeyboardIndex    = keyboardIndex;
    keyEntry.KeyIsDown        = isKeyDown;
}

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
void    InputEventsQueue::AddGamePadAnalogEvent(UInt32 code, float analogx, float analogy, unsigned controllerIdx)
{
    QueueEntry* pqe         = AddEmptyQueueEntry();
    pqe->t                  = QueueEntry::QE_AnalogGamePad;
    QueueEntry::GamePadAnalogEntry& gamePadAnalogEntry = pqe->GetGamePadAnalogEntry();
    gamePadAnalogEntry.Code = code;
    gamePadAnalogEntry.ControllerIndex = (UInt8)controllerIdx;
    gamePadAnalogEntry.AnalogX = analogx;
    gamePadAnalogEntry.AnalogY = analogy;
}
#endif

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
void InputEventsQueue::AddTouchEvent(TouchType type, unsigned id, const PointF& pos, const SizeF& touchSize, float pressure, bool primary)
{
//     // reset last position, since the record being added contains more recent position.
//     if (pos.x != SF_MIN_FLOAT)
//         LastMousePosMask &= ~(1 << mouseIndex);
// 
    QueueEntry* pqe           = AddEmptyQueueEntry();
    pqe->t                    = QueueEntry::QE_Touch;
    QueueEntry::TouchEntry& touchEntry = pqe->GetTouchEntry();
    touchEntry.Type           = type;
    touchEntry.TouchPointID   = id;
    touchEntry.PosX           = pos.x;
    touchEntry.PosY           = pos.y;
    touchEntry.Pressure       = pressure;
    touchEntry.WContact       = touchSize.Width;
    touchEntry.HContact       = touchSize.Height;
    touchEntry.PrimaryPoint   = primary;
}

void InputEventsQueue::AddGestureEvent(GesturePhase phase, unsigned gestureMask, const PointF& pos, const PointF& translationVector, 
                                       float zoomX, float zoomY, float rotation)
{
    QueueEntry* pqe           = AddEmptyQueueEntry();
    pqe->t                    = QueueEntry::QE_Gesture;
    QueueEntry::GestureEntry& gestureEntry = pqe->GetGestureEntry();
    gestureEntry.Phase        = phase;
    gestureEntry.PosX         = pos.x;
    gestureEntry.PosY         = pos.y;
    gestureEntry.OffsetX      = translationVector.x;
    gestureEntry.OffsetY      = translationVector.y;
    gestureEntry.ScaleX       = zoomX;
    gestureEntry.ScaleY       = zoomY;
    gestureEntry.Rotation     = rotation;
    gestureEntry.GestureMask  = gestureMask;
}

#endif

void InputEventsQueue::AddAccelerometerEvent(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ)
{
    QueueEntry* pqe         = AddEmptyQueueEntry();
    pqe->t                  = QueueEntry::QE_Accelerometer;
    QueueEntry::AccelerometerEntry& accelerometerEntry = pqe->GetAccelerometerEntry();
    accelerometerEntry.IdAcc = idAcc;
	accelerometerEntry.Timestamp = timestamp;
    accelerometerEntry.AccelerationX = accelerationX;
    accelerometerEntry.AccelerationY = accelerationY;
    accelerometerEntry.AccelerationZ = accelerationZ;
}

void InputEventsQueue::AddGeolocationEvent(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp)
{
    QueueEntry* pqe         = AddEmptyQueueEntry();
    pqe->t                  = QueueEntry::QE_Geolocation;
    QueueEntry::GeolocationEntry& geolocationEntry = pqe->GetGeolocationEntry();
    geolocationEntry.IdGeo = idGeo;
	geolocationEntry.Latitude = latitude;
	geolocationEntry.Longitude = longitude;
	geolocationEntry.Altitude = altitude;
	geolocationEntry.HAccuracy = hAccuracy;
	geolocationEntry.VAccuracy = vAccuracy;
	geolocationEntry.Speed = speed;
	geolocationEntry.Heading = heading;
	geolocationEntry.Timestamp = timestamp;
}

void InputEventsQueue::AddStatusEvent(String* code, String* level, String* extensionId, String* contextId)
{
    QueueEntry* pqe         = AddEmptyQueueEntry();
    pqe->t                  = QueueEntry::QE_Status;
    QueueEntry::StatusEntry& statusEntry = pqe->GetStatusEntry();
	statusEntry.Code = code;
	statusEntry.Level = level;
	statusEntry.ExtensionId = extensionId;
	statusEntry.ContextId = contextId;
}

const InputEventsQueue::QueueEntry* InputEventsQueue::GetEntry() 
{ 
    if (UsedEntries == 0) 
    {
        // a special case of simmulating mouse move events after queue is over.
        for (unsigned i = 0, mask = 1; i < GFX_MAX_MICE_SUPPORTED; ++i, mask <<= 1)
        {
            if (LastMousePosMask & mask)
            {
                SF_ASSERT(LastMousePos[i].x != SF_MIN_FLOAT && LastMousePos[i].y != SF_MIN_FLOAT);
                QueueEntry* pqe           = AddEmptyQueueEntry();
                pqe->t                    = QueueEntry::QE_Mouse;
                QueueEntry::MouseEntry& mouseEntry = pqe->GetMouseEntry();
                mouseEntry.MouseIndex     = (UInt8)i;
                mouseEntry.PosX           = LastMousePos[i].x;
                mouseEntry.PosY           = LastMousePos[i].y;
                mouseEntry.ButtonsState   = 0;
                mouseEntry.Flags          = QueueEntry::MouseButton_Move;
                LastMousePosMask         &=  ~mask;
            }
        }
        if (UsedEntries == 0)
            return NULL;
    }
    SF_ASSERT(StartPos < Queue_Length);
    UPInt idx = StartPos;

    ++StartPos;
    --UsedEntries;
    if (StartPos == Queue_Length)
        StartPos = 0;
    return &Queue[idx];
}

//
// EventId
//
/*
ASBuiltinType      EventId::GetFunctionNameBuiltinType() const
{
    // Note: There are no function names for Event_KeyPress, Event_Initialize, and 
    // Event_Construct. We use "@keyPress@", "@initialize@", and "@construct@" strings.
    static ASBuiltinType  functionTypes[Event_COUNT] =
    {        
        ASBuiltin_INVALID,             // Event_Invalid "INVALID"
        ASBuiltin_onLoad,              // Event_Load
        ASBuiltin_onEnterFrame,        // Event_EnterFrame
        ASBuiltin_onUnload,            // Event_Unload
        ASBuiltin_onMouseMove,         // Event_MouseMove
        ASBuiltin_onMouseDown,         // Event_MouseDown
        ASBuiltin_onMouseUp,           // Event_MouseUp
        ASBuiltin_onKeyDown,           // Event_KeyDown
        ASBuiltin_onKeyUp,             // Event_KeyUp
        ASBuiltin_onData,              // Event_Data
        ASBuiltin_ainitializea_,       // Event_Initialize "@initialize@"
        ASBuiltin_onPress,             // Event_Press
        ASBuiltin_onRelease,           // Event_Release
        ASBuiltin_onReleaseOutside,    // Event_ReleaseOutside
        ASBuiltin_onRollOver,          // Event_RollOver
        ASBuiltin_onRollOut,           // Event_RollOut
        ASBuiltin_onDragOver,          // Event_DragOver
        ASBuiltin_onDragOut,           // Event_DragOut
        ASBuiltin_akeyPressa_,         // Event_KeyPress  "@keyPress@"
        ASBuiltin_onConstruct,        // Event_Construct
        // These are for the MoveClipLoader ActionScript only
        ASBuiltin_onLoadStart,         // Event_LoadStart
        ASBuiltin_onLoadError,         // Event_LoadError
        ASBuiltin_onLoadProgress,      // Event_LoadProgress
        ASBuiltin_onLoadInit,          // Event_LoadInit
        // These are for the XMLSocket ActionScript only
        ASBuiltin_onSockClose,         // Event_SockClose
        ASBuiltin_onSockConnect,       // Event_SockConnect
        ASBuiltin_onSockData,          // Event_SockData
        ASBuiltin_onSockXML,           // Event_SockXML
        // These are for the XML ActionScript only
        ASBuiltin_onXMLLoad,           // Event_XMLLoad
        ASBuiltin_onXMLData,           // Event_XMLData

        //!AB:? GFXSTATICSTRING("onTimer"),  // setInterval GFxTimer expired
    };  
    SF_ASSERT(GetEventsCount() == 1);
    unsigned idx;
    if (Id >= Event_Invalid && Id <= Event_LastCombined)
        idx = Alg::BitCount32(Id);
    else
        idx = (Id - Event_NextAfterCombined) + 20;
    SF_ASSERT(idx > Event_Invalid && idx < Event_COUNT);
    //DBG:SF_ASSERT(Id != Event_Press);

    if (idx > Event_Invalid && idx < Event_COUNT)
        return functionTypes[idx];
    return ASBuiltin_unknown_;
}

ASString   EventId::GetFunctionName(StringManager *psm) const
{
    return psm->GetBuiltin(GetFunctionNameBuiltinType()); 
}
*/
// converts keyCode/asciiCode from this event to the on(keyPress <>) format
int EventId::ConvertToButtonKeyCode() const
{
    int kc = 0;

    // convert keycode/ascii to button's keycode
    switch (KeyCode) 
    {
    case Key::Left:      kc = 1; break;
    case Key::Right:     kc = 2; break;
    case Key::Home:      kc = 3; break;
    case Key::End:       kc = 4; break;
    case Key::Insert:    kc = 5; break;
    case Key::Delete:    kc = 6; break;
    case Key::Backspace: kc = 8; break;
    case Key::Return:    kc = 13; break;
    case Key::Up:        kc = 14; break;
    case Key::Down:      kc = 15; break;
    case Key::PageUp:    kc = 16; break;
    case Key::PageDown:  kc = 17; break;
    case Key::Tab:       kc = 18; break;
    case Key::Escape:    kc = 19; break;
    default:
        if (AsciiCode >= 32)
            kc = AsciiCode;
    }
    return kc;
}

UByte EventId::ConvertKeyCodeToAscii() const
{
    UByte asciiCode = 0;

    // Ascii code is not set, convert keycode to ascii
    static const UByte ascii1[] = 
    {
        ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 32 - 47
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0,   0,   0,   0,   0,   0,  // 48 - 63
        0,   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', // 64 - 79
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',  0,   0,   0,   0,   0,  // 80 - 95
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  '*', '+', 0,   '-', '.', '/' // 96 - 111
    };
    static const UByte asciiShift1[] = 
    {
        ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 32 - 47
        ')', '!', '@', '#', '$', '%', '^', '&', '*', '(',  0,   0,   0,   0,   0,   0,  // 48 - 63
        0,   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', // 64 - 79
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  0,   0,   0,   0,   0,   // 80 - 95
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '+', 0,   '-', '.', '/'  // 96 - 111
    };
    static const UByte ascii2[] = 
    {
        ';', '=', ',', '-', '.', '/', '`',  0,   0,   0,   0,   0,   0,   0,   0,   0,  // 186 - 201
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 202 - 217
        0,  '[', '\\',']', '\''                                                        // 218 - 222
    };
    static const UByte asciiShift2[] = 
    {
        ':', '+', '<', '_', '>', '?', '~', 0,   0,   0,   0,   0,   0,   0,   0,   0,  // 186 - 201
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 202 - 217
        0,  '{', '|','}', '\"'                                                         // 218 - 222
    };
    bool shift = KeysState.IsShiftPressed();
    if (KeysState.IsCapsToggled())
        shift = !shift;

    if (KeyCode >= 32 && KeyCode <= sizeof(ascii1)/sizeof(ascii1[0]) + 32)
    {
        if (!shift)
            asciiCode = ascii1[KeyCode - 32];
        else
            asciiCode = asciiShift1[KeyCode - 32];
    }
    else if (KeyCode >= 186 && KeyCode <= sizeof(ascii1)/sizeof(ascii1[0]) + 186)
    {
        if (!shift)
            asciiCode = ascii2[KeyCode - 186];
        else
            asciiCode = asciiShift2[KeyCode - 186];
    }
    return asciiCode;
}

}} // namespace Scaleform::GFx
