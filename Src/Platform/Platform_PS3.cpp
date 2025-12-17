/**************************************************************************

Filename    :   Platform_PS3.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform.h"
#include "Platform/Platform_SystemCursorManager.h"

#include "Kernel/SF_KeyCodes.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_Debug.h"

#include <sys/process.h>
#include <sys/timer.h>
#include <sys/spu_initialize.h>
#include <sys/spu_thread_group.h>
#include <cell/spurs/control.h>
#include <cell/pad.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/sysmodule.h>
#include <cell/libkey2char.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_imejp.h>
#include <math.h>

#include <sys/process.h>
SYS_PROCESS_PARAM(1001, 0x40000)

#define MAX_KEYBD_SUPPORTED     1
#define MAX_PADS_SUPPORTED      2
#define GAMEPAD_DEADZONE        16


namespace Scaleform { namespace Platform {


//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public Device::Window, public AppImplBase
{
    friend class AppBase;

    AppImpl(AppBase* app);
	~AppImpl();

    // Window Interface
    virtual void* GetHandle() { return 0; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);    
    
private:
    bool          setupWindow(const String& title, const ViewConfig& config);
    void          processKey(unsigned vkCode, bool downFlag);

    int           initKeyboard();
    bool          processMessages();
    void          detectPads();
    bool          isPadConnected(unsigned idx);
    int32_t       processKeyboard(unsigned& keyCode, unsigned* charCode, unsigned& modifiers, unsigned& numChar);
    void          setMousePosition(const Point<unsigned> & p );

    struct Pad
    {    
        CellPadData     pad;
        int             MouseX, MouseY;
        float           MouseXadj, MouseYadj;
        bool            PadConnected;

        bool    LStickLeft, LStickRight, LStickUp, LStickDown;
        bool    nLStickLeft, nLStickRight, nLStickUp, nLStickDown;

        UInt64  LastMsgTime, LastRepeatTime, RepeatDelay;

        Pad() :
        MouseX(0), MouseY(0),
            MouseXadj(0), MouseYadj(0),
            PadConnected(false)        
        {
            memset(&pad, 0, sizeof(pad));
            pad.button[4] = pad.button[5] = 0x80;
            pad.button[6] = pad.button[7] = 0x80;

            LStickLeft = LStickRight = LStickUp = LStickDown = false;
            nLStickLeft = nLStickRight = nLStickUp = nLStickDown = false;
            LastRepeatTime = RepeatDelay = 0;
            LastMsgTime = Timer::GetTicks();
        }
    } Pads[MAX_PADS_SUPPORTED];

    bool            Created;
    bool            BlockOnSize;
    bool            bEnglishMode;

    CellKbInfo          Info;        // Connection information buffer
    CellKbData          KeybdData;   // Keyboard data buffer
    CellKey2CharHandle	Handle[MAX_KEYBD_SUPPORTED];
    uint8_t             Old_status[MAX_KEYBD_SUPPORTED];
    uint32_t            Old_info;

	CellSpurs*		GetSpurs();

    static const uint32_t SPURS_NUM_SPUS = 5;
    static const uint32_t SPURS_PPU_THREAD_PRIO = 1000;
    static const uint32_t SPURS_SPU_THRGRP_PRIO = 200;
    CellSpurs* pSpurs;
};


//------------------------------------------------------------------------

class FxPS3IMEEvent : public AppBase::IMEEvent
{
public:
    FxPS3IMEEvent(): KeyCode(0), CharCode(0), Modifiers(0), LastKeyCode(0), LastCharCode(0), LastModifiers(0){};
    FxPS3IMEEvent(unsigned keyCode, unsigned code, unsigned mods)
    {
        KeyCode 	    = keyCode;
        CharCode 	    = code;
        Modifiers 	    = mods;
        LastKeyCode     = 0;
        LastCharCode    = 0;
        LastModifiers   = 0;
    };
    unsigned KeyCode;
    unsigned CharCode;
    unsigned Modifiers;
    mutable unsigned  LastKeyCode;
    mutable unsigned  LastCharCode;
    mutable unsigned  LastModifiers;
};

static struct {
    unsigned psKey;
    KeyCode appKey;
} KeyCodeMap[] = 
{ 
    { CELL_KEYC_BS,                 Key::Backspace }, 
    { CELL_KEYC_TAB,                Key::Tab }, 
    { CELL_KEYC_ENTER,              Key::Return }, 
    { CELL_KB_MKEY_L_SHIFT,         Key::Shift }, 
    { CELL_KB_MKEY_R_SHIFT,         Key::Shift }, 
    { CELL_KB_MKEY_L_CTRL,          Key::Control }, 
    { CELL_KB_MKEY_R_CTRL,          Key::Control },
    { CELL_KB_MKEY_L_ALT,           Key::Alt }, 
    { CELL_KB_MKEY_R_ALT,           Key::Alt },
    { CELL_KEYC_PAUSE,              Key::Pause }, 
    { CELL_KEYC_CAPS_LOCK,          Key::CapsLock }, 
    { CELL_KEYC_ESCAPE,             Key::Escape }, 
    { CELL_KEYC_SPACE,              Key::Space }, 
    { CELL_KEYC_PAGE_UP,            Key::PageUp }, 
    { CELL_KEYC_PAGE_DOWN,          Key::PageDown }, 
    { CELL_KEYC_END,                Key::End }, 
    { CELL_KEYC_HOME,               Key::Home }, 
    { CELL_KEYC_LEFT_ARROW,         Key::Left }, 
    { CELL_KEYC_UP_ARROW,           Key::Up }, 
    { CELL_KEYC_RIGHT_ARROW,        Key::Right }, 
    { CELL_KEYC_DOWN_ARROW,         Key::Down }, 
    { CELL_KEYC_INSERT,             Key::Insert }, 
    { CELL_KEYC_DELETE,             Key::Delete }, 
    { CELL_KEYC_NUM_LOCK,           Key::NumLock }, 
    { CELL_KEYC_SCROLL_LOCK,        Key::ScrollLock }, 
    { CELL_KEYC_LEFT_BRACKET_101,   Key::BracketLeft},
    { CELL_KEYC_RIGHT_BRACKET_101,  Key::BracketRight}, 	    
    { CELL_KEYC_BACKSLASH_101,      Key::Backslash},	        
    { CELL_KEYC_SEMICOLON,          Key::Semicolon},                                 
    { CELL_KEYC_EQUAL_101,          Key::Equal},         
    { CELL_KEYC_COMMA,              Key::Comma},         
    { CELL_KEYC_MINUS,              Key::Minus},         
    { CELL_KEYC_PERIOD,             Key::Period},        
    { CELL_KEYC_SLASH,              Key::Slash},                   
    { CELL_KEYC_QUOTATION_101,      Key::Quote},
    { 0,                            Key::None}
};

static PadKeyCode keymap[2][8] = 
{
    { Pad_Select, Pad_LT, Pad_RT, Pad_Start,
    Pad_Up, Pad_Right, Pad_Down, Pad_Left },
    { Pad_L2, Pad_R2, Pad_L1, Pad_R1, Pad_T,
    Pad_O, Pad_X, Pad_S }
};


//------------------------------------------------------------------------

AppImpl::AppImpl(AppBase* app) :         
    AppImplBase(app),
    Created(false),
    pSpurs(0)
{
#if (CELL_SDK_VER >= 93)
    cellSysmoduleLoadModule(CELL_SYSMODULE_RESC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
    cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_GCM);
#endif

    sys_spu_initialize(6, 1);

    // Keyboard initialization
    int ret = initKeyboard();
    if (ret != CELL_OK)
    {
        // Write to log that Initializing keyboard didn't work.
    }
    pSpurs = 0;

    cellPadInit(MAX_PADS_SUPPORTED);
}

AppImpl::~AppImpl() 
{
   if (pSpurs)
   {
       cellSpursFinalize(pSpurs);
       free(pSpurs);
       pSpurs = 0;
   }

    cellPadEnd();
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;

    if (Created)
        return false;

    CellVideoOutState         cvs;
    CellVideoOutConfiguration cvc;
    CellVideoOutResolution    cvr;

    cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &cvs);
    cellVideoOutGetResolution(cvs.displayMode.resolutionId, &cvr);

    memset(&cvc, 0, sizeof(cvc));
    cvc.resolutionId = cvs.displayMode.resolutionId;
    cvc.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
    cvc.pitch = (cvr.width * 4 + 63) & ~63;

    if (CELL_OK != cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &cvc, 0, 0))
        return 0;

    Point<unsigned> center(cvr.width/2, cvr.height/2);
    setMousePosition(center);

    return true;
}

void AppImpl::ConfigureWindow(ViewConfig& newConfig)
{
    // Configures window for a new video mode.
    Config = newConfig;
}

int AppImpl::initKeyboard() 
{
    uint8_t old_status[MAX_KEYBD_SUPPORTED];
    int32_t ret = 0;
    int rtn     = 0;

    // libKey2Char variables
    CellKey2CharKeyData key2kbdata; 
    memset(&key2kbdata, '\0', sizeof(key2kbdata));

    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
    if (ret != CELL_OK)
    {
        return ret;
    }

    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_KEY2CHAR);
    if (ret != CELL_OK)
    {
        return (ret);
    }

    if ((ret = cellKbInit(MAX_KEYBD_SUPPORTED)) != CELL_KB_OK) 
    {
        return ret;
    }

    for (uint32_t i = 0; i < MAX_KEYBD_SUPPORTED; i++) {
        old_status[i] = 0;
        if ((ret = cellKbSetReadMode(i, CELL_KB_RMODE_INPUTCHAR)) != CELL_KB_OK) {
            cellKbEnd();
            return ret;
        }
        if ((ret = cellKbSetCodeType(i, CELL_KB_CODETYPE_RAW)) != CELL_KB_OK) {
            cellKbEnd();
            return ret;
        }
    }

    for (int32_t hdl = 0; hdl < MAX_KEYBD_SUPPORTED; hdl++) 
    {
        memset(&Handle[hdl], '\0', sizeof(Handle[hdl]));
        rtn = cellKey2CharOpen(Handle[hdl]);
        if (rtn != CELL_K2C_OK)
            return ret;

        // Set keyboard mode
        cellKey2CharSetMode(Handle[hdl], CELL_KEY2CHAR_MODE_NATIVE);
    }
    return ret;
}

bool AppImpl::processMessages()
{
    detectPads();

    for (unsigned k = 0; k < MAX_PADS_SUPPORTED; ++k)
    {
        if (!isPadConnected(k))
            continue;
    
        CellPadData pad;
        int mbutton = -1;
        UInt64 msgtime = Timer::GetTicks();
        float delta = (msgtime - Pads[k].LastMsgTime) / 1000000.0f;
        bool didrepeat = 0;

        if (cellPadGetData(k, &pad) || pad.len < 3)
            pad = Pads[k].pad;
        else if (pad.len < 5)
        {
            for (int i = 4; i < 8; i++)
                pad.button[i] = Pads[k].pad.button[i];
        }

        for (int i = 2; i <= 3; i++)
            for (int j = 0; j < 8; j++)
            {
                if ((Pads[k].pad.button[i] & (1 << j)) != (pad.button[i] & (1 << j))) {
                    if (keymap[i-2][j] == Pad_X && pApp->IsCursorEnabled())
                        mbutton = (pad.button[i] & (1 << j)) ? 1 : 0;
                    else
                    {
                        pApp->OnPad(k, keymap[i-2][j], (pad.button[i] & (1 << j)) ? 1 : 0);
                        Pads[k].RepeatDelay = 350000;
                        didrepeat = 1;
                    }
                }
                else if ((Pads[k].pad.button[i] & (1 << j)) && (pad.button[i] & (1 << j))
                    && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay
                    && (!pApp->IsCursorEnabled() || keymap[i-2][j] != Pad_X))
                {
                    Pads[k].RepeatDelay = 100000;
                    //pApp->OnPad(k, keymap[i-2][j], 0);
                    pApp->OnPad(k, keymap[i-2][j], 1);
                    didrepeat = 1;
                }
            }

        bool curR = ((pad.button[6] & 0xF8) >= 192);
        bool oldR = ((Pads[k].pad.button[6] & 0xF8) >= 192);
        bool curL = ((pad.button[6] & 0xF8) <= 64);
        bool oldL = ((Pads[k].pad.button[6] & 0xF8) <= 64);
        bool curD = ((pad.button[7] & 0xF8) >= 192);
        bool oldD = ((Pads[k].pad.button[7] & 0xF8) >= 192);
        bool curU = ((pad.button[7] & 0xF8) <= 64);
        bool oldU = ((Pads[k].pad.button[7] & 0xF8) <= 64);

        Pads[k].nLStickLeft = Pads[k].nLStickRight = Pads[k].nLStickUp = Pads[k].nLStickDown = false;
        if (curR)
        {
            if (oldR && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Right, 0);
                pApp->OnPad(k, Pad_Right, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldL)
                pApp->OnPad(k, Pad_Left, 0);
            if (oldL || !oldR)
            {
                pApp->OnPad(k, Pad_Right, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickRight = true;
        }
        if (curL)
        {
            if (oldL && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Left, 0);
                pApp->OnPad(k, Pad_Left, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldR)
                pApp->OnPad(k, Pad_Right, 0);
            if (oldR || !oldL)
            {
                pApp->OnPad(k, Pad_Left, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickLeft = true;
        }
        if (curU)
        {
            if (oldU && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Up, 0);
                pApp->OnPad(k, Pad_Up, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldD)
                pApp->OnPad(k, Pad_Down, 0);
            if (oldD || !oldU)
            {
                pApp->OnPad(k, Pad_Up, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickUp = true;
        }
        if (curD)
        {
            if (oldD && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Down, 0);
                pApp->OnPad(k, Pad_Down, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldU)
                pApp->OnPad(k, Pad_Up, 0);
            if (oldU || !oldD)
            {
                pApp->OnPad(k, Pad_Down, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickDown = true;
        }
        
        if (Pads[k].LStickDown && !Pads[k].nLStickDown)
			pApp->OnPad(k, Pad_Down, 0);

        if (Pads[k].LStickUp && !Pads[k].nLStickUp)
            pApp->OnPad(k, Pad_Up, 0);

        if (Pads[k].LStickLeft && !Pads[k].nLStickLeft)
            pApp->OnPad(k, Pad_Left, 0);

        if (Pads[k].LStickRight && !Pads[k].nLStickRight)
            pApp->OnPad(k, Pad_Right, 0);

        Pads[k].LStickDown = Pads[k].nLStickDown;
        Pads[k].LStickLeft = Pads[k].nLStickLeft;
        Pads[k].LStickRight = Pads[k].nLStickRight;
        Pads[k].LStickUp = Pads[k].nLStickUp;

        if ((pad.button[4] & 0xF8) > 128 + GAMEPAD_DEADZONE)
            Pads[k].MouseXadj += (float(pad.button[4] & 0xF8) - 128.0f - GAMEPAD_DEADZONE) * 4.0f * delta;
        else if ((pad.button[4] & 0xF8) < 128 - GAMEPAD_DEADZONE)
            Pads[k].MouseXadj += (float(pad.button[4] & 0xF8) - 128.0f + GAMEPAD_DEADZONE) * 4.0f * delta;

        if ((pad.button[5] & 0xF8) > 128 + GAMEPAD_DEADZONE)
            Pads[k].MouseYadj += (float(pad.button[5] & 0xF8) - 128.0f - GAMEPAD_DEADZONE) * 4.0f * delta;
        else if ((pad.button[5] & 0xF8) < 128 - GAMEPAD_DEADZONE)
            Pads[k].MouseYadj += (float(pad.button[5] & 0xF8) - 128.0f + GAMEPAD_DEADZONE) * 4.0f * delta;

        Pads[k].pad = pad;

        if (fabs(Pads[k].MouseXadj) >= 1 || fabs(Pads[k].MouseYadj) >= 1) {
            Pads[k].MouseX += int(Pads[k].MouseXadj);
            Pads[k].MouseY += int(Pads[k].MouseYadj);
            Pads[k].MouseXadj -= int(Pads[k].MouseXadj);
            Pads[k].MouseYadj -= int(Pads[k].MouseYadj);
            if (Pads[k].MouseX < 0)
                Pads[k].MouseX = 0;
            if (Pads[k].MouseX >= int(Config.ViewSize.Width))
                Pads[k].MouseX = Config.ViewSize.Width;
            if (Pads[k].MouseY < 0)
                Pads[k].MouseY = 0;
            if (Pads[k].MouseY >= int(Config.ViewSize.Height))
                Pads[k].MouseY = Config.ViewSize.Height;

            pApp->OnMouseMove(k, Point<int>(Pads[k].MouseX, Pads[k].MouseY), 0);

            // Change the mouse position in the impl.
            MousePos = Point<int>(Pads[k].MouseX, Pads[k].MouseY);
        }
        if (mbutton != -1)
            pApp->OnMouseButton(k, 0, mbutton, Point<int>(Pads[k].MouseX, Pads[k].MouseY), 0);

        Pads[k].LastMsgTime = msgtime;
        if (didrepeat)
            Pads[k].LastRepeatTime = msgtime;

        unsigned keyCode;
        unsigned charCode[10];
        unsigned modifiers;    
        unsigned lastKeyCode    = 0;
        unsigned lastCharCode   = 0;
        unsigned lastModifiers  = 0;
        unsigned numChar        = 1;
        KeyCode kc              = Key::None;

        processKeyboard(keyCode, charCode, modifiers, numChar);
        
        for (unsigned ii = 0; ii < numChar; ii++)
        {
            if (1 /*bEnglishMode*/) // Send keystrokes to IME only for English mode.
            {
             //   FxPS3IMEEvent ev2(keyCode, charCode[ii], modifiers);
                if (pApp->OnIMEEvent(0, keyCode, charCode[ii], 0, false))
                {
                    // If successful, use values set by IME
                    // lastKeyCode     = ev2.LastKeyCode;
                    // lastCharCode    = ev2.LastCharCode;
                    // lastModifiers   = ev2.LastModifiers;
                }
                else  
                {
                    // Use default values
                    lastKeyCode     = keyCode;
                    lastCharCode    = charCode[ii];
                    lastModifiers   = modifiers;
                }

            }
            else
            {
                // Use default values
                lastKeyCode     = keyCode;
                lastCharCode    = charCode[ii];
                lastModifiers   = modifiers;
            }

            if (lastKeyCode != 0)
            {
                KeyModifiers new_mods;
                if ((lastModifiers & CELL_KB_MKEY_L_SHIFT) || (lastModifiers
                    & CELL_KB_MKEY_R_SHIFT)) {
                        new_mods.SetShiftPressed(true);
                }

                if ((lastModifiers & CELL_KB_MKEY_L_CTRL) || (lastModifiers
                    & CELL_KB_MKEY_R_CTRL)) {
                        new_mods.SetCtrlPressed(true);
                }

                if ((lastModifiers & CELL_KB_MKEY_L_ALT) || (lastModifiers
                    & CELL_KB_MKEY_R_ALT)) {
                        new_mods.SetAltPressed(true);
                }

                if (lastKeyCode >= CELL_KEYC_A && lastKeyCode <= CELL_KEYC_Z) {
                    kc = (KeyCode) ((lastKeyCode - CELL_KEYC_A) + Key::A);
                } else if (lastKeyCode >= CELL_KEYC_F1 && lastKeyCode <= CELL_KEYC_F12) {
                    kc = (KeyCode) ((lastKeyCode - CELL_KEYC_F1) + Key::F1);
                } else if (lastKeyCode >= CELL_KEYC_1 && lastKeyCode <= CELL_KEYC_0) {
                    kc = (KeyCode) ((lastKeyCode - CELL_KEYC_1) + Key::Num1);
                }
                //	else if (lastKeyCode >= VK_NUMPAD0 && lastKeyCode <= VK_DIVIDE) {
                //		kc = (lastKeyCode) ((lastKeyCode - VK_NUMPAD0) + KP_0);
                //	}
                else {
                    for (int i = 0; KeyCodeMap[i].psKey != 0; i++) {
                        if (lastKeyCode == (unsigned)KeyCodeMap[i].psKey) {
                            kc = KeyCodeMap[i].appKey;
                            break;
                        }
                    }
                }

                // The checks below are used in Korean input. While inputting korean text, keystrokes
                // don't always result in new characters, but can also alter the character created
                // by the previous keystroke. When such a keystroke is input, processkeyboard returns
                // an array of keystrokes, the first entry is backspace character which causes the current
                // character to be deleted, followed by the new character which takes place of the existing
                // one. The keycode always corresponds to the PS3 keycode of the key that was pushed, the
                // charcode indicates the character code (0x08 for backspace etc) of the key. 

                if (lastCharCode == 0x08)
                    kc = Key::Backspace;

                if (lastCharCode != 0x08 && kc == Key::Backspace)
                {
                    // ? pApp->OnChar(lastCharCode, 0);
                }
                else
                {
                    //pApp->OnKey(k, kc, lastCharCode, lastCharCode, new_mods, 1);
                    //pApp->OnKey(k, kc, lastCharCode, lastCharCode, new_mods, 0);
                    pApp->OnKey(k, kc, lastCharCode, 1, new_mods);
                    pApp->OnKey(k, kc, lastCharCode, 0, new_mods);
                }
            }
        }
    }
    return 1;
}

void AppImpl::detectPads()
{
    CellPadInfo2 info;
    if(cellPadGetInfo2(&info) != CELL_PAD_OK)
        return;
    for (unsigned i = 0; i < MAX_PADS_SUPPORTED; ++i)
    {
        Pads[i].PadConnected =
            (info.port_status[i] & CELL_PAD_STATUS_CONNECTED == CELL_PAD_STATUS_CONNECTED);
    }    
}

bool AppImpl::isPadConnected(unsigned idx) 
{ 
    return Pads[idx].PadConnected; 
}    

int32_t AppImpl::processKeyboard(unsigned& keyCode, unsigned* charCode, unsigned& modifiers, unsigned& numChar)
{
    int32_t ret     = 0;
    int     rtn     = 0;
    bool processed  = false; 
    CellKey2CharKeyData key2kbdata;
    memset(&key2kbdata, '\0', sizeof(key2kbdata));
    uint32_t charNum = 0;
    uint16_t *pCharCode = '\0';

    charCode[0] = 0;
    keyCode     = 0;
    modifiers   = 0;

    if ((ret = cellKbGetInfo(&Info)) != CELL_KB_OK) 
    {
        return ret;
    }
    
    if ((Info.info & CELL_KB_INFO_INTERCEPTED) && (!(Old_info
        & CELL_KB_INFO_INTERCEPTED))) 
    {
            Old_info = Info.info;
    } 
    else if ((!(Info.info & CELL_KB_INFO_INTERCEPTED)) && (Old_info
        & CELL_KB_INFO_INTERCEPTED))
    {
            Old_info = Info.info;
    }

    for (uint32_t i = 0; i < MAX_KEYBD_SUPPORTED; i++)
    {
        ret = cellKbRead(i, &KeybdData);
        // cellKbClearBuf(i); // If this is uncommented, kdata will no longer contain key data
        // when a key is pressed continuously. 

        if (CELL_KB_OK != ret) 
        {
            return ret;
        }
       
        if (KeybdData.len == 0) { // This check is very important. Do not Remove!! This prevents duplicate keystrokes.
            return 0;
        }

        key2kbdata.mkey = KeybdData.mkey;
        key2kbdata.led  = KeybdData.led;
        key2kbdata.keycode = KeybdData.keycode[0]; // Device KeyCode
        processed   = false;
        charNum     = 0;

        if (KeybdData.len > 0) 
        {
            if ((key2kbdata.led &(CELL_KB_LED_SCROLL_LOCK | CELL_KB_LED_KANA))) 
            {
                cellKey2CharSetMode(Handle[i], CELL_KEY2CHAR_MODE_NATIVE);
                bEnglishMode = false;
            } 
            else 
            {
                cellKey2CharSetMode(Handle[i], CELL_KEY2CHAR_MODE_ENGLISH);
                bEnglishMode = true;
            }

            rtn = cellKey2CharGetChar(Handle[i], &key2kbdata, &pCharCode,
                &charNum, &processed);
        }
  
        if (rtn == CELL_K2C_OK) 
        {
            // number of characters in the conversion-result character string will return in *charNum.
            // In usual cases, 'true' will return in *processed. However, when the input device key code
            // represents a special key such as the 'F1' key and therefore cannot be converted into a character
            // code, 'false' will return. At this time, the input device key code will directly return in
            // *charcode as the conversion result and 0 will return in *charNum.

            if(charNum != 0)
                numChar = charNum;
            
            charCode[0] = 0;
            keyCode     = key2kbdata.keycode;
            for(unsigned i = 0; i < charNum; i++)
            {
                charCode[i] = pCharCode[i];
                modifiers   = key2kbdata.mkey;
                keyCode     = key2kbdata.keycode;
            }
            
        }
    }

    for (uint32_t i = 0; i < MAX_KEYBD_SUPPORTED; i++) {
        Old_status[i] = Info.status[i];
    }

    return rtn;
}

void AppImpl::setMousePosition(const Point<unsigned> & p ) 
{
    for ( int i = 0; i < MAX_PADS_SUPPORTED; i++ )
    {
        Pads[i].MouseX = p.x;
        Pads[i].MouseY = p.y;
    }
}

CellSpurs* AppImpl::GetSpurs()
{
    if (pSpurs)
        return pSpurs;

    pSpurs = (CellSpurs*)memalign(CELL_SPURS_ALIGN, CELL_SPURS_SIZE);
    SF_ASSERT(pSpurs);

    CellSpursAttribute attr;
    int ret = cellSpursAttributeInitialize(&attr, SPURS_NUM_SPUS, SPURS_SPU_THRGRP_PRIO, SPURS_PPU_THREAD_PRIO, 0);
    if (ret != CELL_OK)
	{
		SF_DEBUG_WARNING1(true, "GetSpurs() failed: cellSpursAttributeInitialize: (%d)", ret);
		return 0;
		
    }
    ret = cellSpursInitializeWithAttribute(pSpurs, &attr);
    if (ret != CELL_OK)
	{
		SF_DEBUG_WARNING1(true, "GetSpurs() failed: cellSpursInitializeWithAttribute: (%d)", ret);
        return 0;
    }

    return pSpurs;
}


//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase() :
    Cursor(false),
    ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}
AppBase::~AppBase()
{
    delete pImpl;
}

void AppBase::SetOrientationMode(bool)
{
}

void AppBase::Shutdown()
{
    ShuttingDown = true;
}

void AppBase::BringWindowToFront()
{
}

Device::Window* AppBase::GetDeviceWindow() const
{
    return pImpl;
}

bool AppBase::IsConsole()
{
    return true;
}

Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.05f);
}

const char* AppBase::GetDefaultFilePath()
{
    return "/app_home/";
}

String AppBase::GetContentDirectory()
{
    return GetDefaultFilePath();
}

int AppBase::GetMouseCount()
{
	unsigned i, cnt = 0;
    for (i = 0; i < MAX_PADS_SUPPORTED; i++)
    {
        if (pImpl->isPadConnected(i))
			cnt++;
	}
    return cnt;
}

int AppBase::GetKeyboardCount()
{
    return 1;
}

UInt32 AppBase::GetCaps() const
{
    return 0;
}

bool AppBase::IsPadConnected(unsigned pad)
{
	return pImpl->isPadConnected(pad);
}

// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{    
    // Scaleform::System initSFSystem;

    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return 0;

    // Initialize application and its window.
    ViewConfig config(Device::GetDefaultViewSize());
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return 0;
    }

    // Application / Player message loop.
    pImpl->NextUpdateTicks = Timer::GetTicks() + pImpl->FrameTimeTicks;

    while(!IsShuttingDown())
    {
        // Wait for Update time if necessary; consider timing wrap-around.
        pImpl->processMessages();

        if (pImpl->FrameTime != 0.0f)
        {
            UInt64  ticks = Timer::GetTicks();
            if (pImpl->NextUpdateTicks > ticks)
            {
                usecond_t waitUs = pImpl->NextUpdateTicks - ticks;
                sys_timer_usleep(waitUs);
                continue;
            }
            pImpl->NextUpdateTicks = ticks + pImpl->FrameTimeTicks;
        }
        OnUpdateFrame(true);
    }

    OnShutdown();

    return 0;
}

CellSpurs* AppBase::GetSpurs()
{
	return pImpl->GetSpurs();
}


//------------------------------------------------------------------------
// ***** Directory

class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern)
        : DirectoryImplBase(path, pattern) { }

    virtual void ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Path = path;
    Pattern = pattern;

	int fd;
	if (cellFsOpendir(path.ToCStr(), &fd) == CELL_FS_SUCCEEDED)
	{
		CellFsDirent dir;
		UInt64 size = sizeof(dir);
		while (cellFsReaddir(fd, &dir, &size) == CELL_FS_SUCCEEDED && size == sizeof(dir))
		{
			String name = dir.d_name;
			if (name.GetExtension() == pattern.GetExtension())
				Filenames.PushBack(name);
		}
	}

	CurFile = 0;
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}


//------------------------------------------------------------------------
// ***** SystemCursorManager

SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) :
    SystemCursorState(systemCursorAvailable, false),
    pImpl(0)
{
}
SystemCursorManager::~SystemCursorManager()
{
}

void SystemCursorManager::update(bool forceSet)
{
    // System cursors update. Since there are none, do nothing.
}

}} // Scaleform::Platform


//------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int  result = app->AppMain(argc, argv);
    Scaleform::Platform::AppBase::DestroyInstance(app);
    return result;
}
