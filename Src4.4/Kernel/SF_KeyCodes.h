/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_KeyCodes.h
Content     :   Keyboard and gamepad constants
Created     :   June 21, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_KeyCodes_H
#define INC_SF_KeyCodes_H

#include "Kernel/SF_Types.h"

namespace Scaleform {


// ***** Key
// Key is a keyboard code value wrapper class.
// This class contains enumeration for keyboard constants.

class Key
{
public:
    // These key codes match GFx/Flash-defined values exactly.
    // If these values are reassigned, a mapping table would need to
    // be introduced in GFx.
    enum Code
    {
        // Key::None indicates that no key was specified.
        None            = 0, 

        // A through Z and numbers 0 through 9.
        A               = 65,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0            = 48,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        // Numeric keypad.
        KP_0            = 96,
        KP_1,
        KP_2,
        KP_3,
        KP_4,
        KP_5,
        KP_6,
        KP_7,
        KP_8,
        KP_9,
        KP_Multiply,
        KP_Add,
        KP_Enter,
        KP_Subtract,
        KP_Decimal,
        KP_Divide,

        // Function keys.
        F1              = 112,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,

        // Other keys.
        Backspace       = 8,
        Tab,
        Clear           = 12,
        Return,
        Shift           = 16,
        Control,
        Alt,
        Pause,
        CapsLock        = 20, // Toggle
        Escape          = 27,
        Space           = 32,
        PageUp,
        PageDown,
        End             = 35,
        Home,
        Left,
        Up,
        Right,
        Down,
        PrintScreen     = 44,
        Insert          = 45,
        Delete,
        Help,
        NumLock         = 144, // Toggle
        ScrollLock      = 145, // Toggle

        LeftShift       = 160,
        RightShift,
        LeftControl,
        RightControl,
        LeftAlt,
        RightAlt,

        Semicolon       = 186,
        Equal           = 187,
        Comma           = 188, // Platform specific?
        Minus           = 189,
        Period          = 190, // Platform specific?
        Slash           = 191,
        Bar             = 192,
        BracketLeft     = 219,
        Backslash       = 220,
        BracketRight    = 221,
        Quote           = 222,

        OEM_AX          = 0xE1,  //  'AX' key on Japanese AX kbd
        OEM_102         = 0xE2,  //  "<>" or "\|" on RT 102-key kbd.
        ICO_HELP        = 0xE3,  //  Help key on ICO
        ICO_00          = 0xE4,  //  00 key on ICO

        // Total number of keys.
        KeyCount
    };
};

typedef Key::Code KeyCode;


class KeyModifiers 
{
public:
    enum
    {
        Key_ShiftPressed    = 0x01,
        Key_CtrlPressed     = 0x02,
        Key_AltPressed      = 0x04,
        Key_CapsToggled     = 0x08,
        Key_NumToggled      = 0x10,
        Key_ScrollToggled   = 0x20,
        Key_ExtendedKey     = 0x40, // set when right shift, alt or ctrl is pressed

        Initialized_Bit     = 0x80,
        Initialized_Mask    = 0xFF
    };
    UInt8   States;

    KeyModifiers() : States(0) { }
    KeyModifiers(UInt8 st) : States(UInt8(st | Initialized_Bit)) { }

    void Reset() { States = 0; }

    bool IsShiftPressed() const { return (States & Key_ShiftPressed) != 0; }
    bool IsCtrlPressed() const  { return (States & Key_CtrlPressed) != 0; }
    bool IsAltPressed() const   { return (States & Key_AltPressed) != 0; }
    bool IsCapsToggled() const  { return (States & Key_CapsToggled) != 0; }
    bool IsNumToggled() const   { return (States & Key_NumToggled) != 0; }
    bool IsScrollToggled() const{ return (States & Key_ScrollToggled) != 0; }
    bool IsExtendedKey() const  { return (States & Key_ExtendedKey) != 0; }
    bool IsSpecialKeyPressed() const { return (States & (Key_ShiftPressed|Key_CtrlPressed|Key_AltPressed)) != 0; }

    void SetShiftPressed(bool v = true)  { (v) ? States |= Key_ShiftPressed : States &= ~Key_ShiftPressed; }
    void SetCtrlPressed(bool v = true)   { (v) ? States |= Key_CtrlPressed  : States &= ~Key_CtrlPressed; }
    void SetAltPressed(bool v = true)    { (v) ? States |= Key_AltPressed   : States &= ~Key_AltPressed; }
    void SetCapsToggled(bool v = true)   { (v) ? States |= Key_CapsToggled  : States &= ~Key_CapsToggled; }
    void SetNumToggled(bool v = true)    { (v) ? States |= Key_NumToggled   : States &= ~Key_NumToggled; }
    void SetScrollToggled(bool v = true) { (v) ? States |= Key_ScrollToggled: States &= ~Key_ScrollToggled; }
    void SetExtendedKey(bool v = true)   { (v) ? States |= Key_ExtendedKey  : States &= ~Key_ExtendedKey; }

    bool IsInitialized() const { return (States & Initialized_Mask) != 0; }
};



enum PadKeyCode
{
    Pad_None, // Indicates absence of key code.
    Pad_Back        = 0xFF00,
    Pad_Start,
    Pad_A,
    Pad_B,
    Pad_X,
    Pad_Y,
    Pad_R1,  // RightShoulder;
    Pad_L1,  // LeftShoulder;
    Pad_R2,  // RightTrigger;
    Pad_L2,  // LeftTrigger;
    Pad_Up,
    Pad_Down,
    Pad_Right,
    Pad_Left,
    Pad_Plus,
    Pad_Minus,
    Pad_1,
    Pad_2,
    Pad_H,
    Pad_C,
    Pad_Z,
    Pad_O,
    Pad_T,
    Pad_S,
    Pad_Select,
    Pad_Home,
    Pad_RT,  // RightThumb;
    Pad_LT   // LeftThumb;
};

} // Scaleform

#endif
