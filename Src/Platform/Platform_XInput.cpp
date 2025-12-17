/**************************************************************************

Filename    :   Platform_XInput.cpp
Content     :   Xbox 360 controller suport implementation
Created     :   June 2, 2009
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Platform_XInput.h"

namespace Scaleform { namespace Platform {
// Button bit to key maping
static struct XBoxButtonToKey
{
    DWORD          Button;
    PadKeyCode  Key;
} XBoxButtonToKey_Map[] =
{
    { XINPUT_GAMEPAD_BACK,          Pad_Back },
    { XINPUT_GAMEPAD_START,         Pad_Start},
    { XINPUT_GAMEPAD_A,             Pad_A},
    { XINPUT_GAMEPAD_B,             Pad_B},
    { XINPUT_GAMEPAD_X,             Pad_X},
    { XINPUT_GAMEPAD_Y,             Pad_Y},
    { XINPUT_GAMEPAD_RIGHT_SHOULDER,Pad_R1},
    { XINPUT_GAMEPAD_LEFT_SHOULDER, Pad_L1},
    { XINPUT_GAMEPAD_RIGHT_THUMB,	Pad_RT},
    { XINPUT_GAMEPAD_LEFT_THUMB,	Pad_LT},
    { XINPUT_GAMEPAD_DPAD_UP,       Pad_Up},
    { XINPUT_GAMEPAD_DPAD_DOWN,     Pad_Down},
    { XINPUT_GAMEPAD_DPAD_LEFT,     Pad_Left},
    { XINPUT_GAMEPAD_DPAD_RIGHT,    Pad_Right},
    { 0,                            Pad_None }
};

GamePad::GamePad()
{
    DetectPads();
}

void GamePad::Update( XInputEventAdapter& app )
{
    const float moveScaleFactor = 0.03f;

	for (unsigned k = 0; k < XUSER_MAX_COUNT; ++k)
	{
		XINPUT_STATE newState;
		int mbutton = -1;
		int rthumbbutton = -1;
		UInt64 msgtime = Timer::GetTicks();
		float  delta = (msgtime-Pads[k].LastMsgTime)/1000000.0f;
		bool didrepeat = 0;

        // Only update at 60hz, otherwise, XInputGetState takes a lot of CPU time.
        if ( delta < 1/60.0f)
            continue;

		if (XInputGetState(k, &newState) == ERROR_SUCCESS)
		{
			Pads[k].PadConnected = true;
			// Send out fake keystrokes for buttons
			for (int i=0; XBoxButtonToKey_Map[i].Button != 0; i++)
			{
				bool isPressedOld = (Pads[k].InputState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button) != 0;
				bool isPressedNew = (newState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button) != 0;

				if (!isPressedOld && isPressedNew)
				{
					if ((XBoxButtonToKey_Map[i].Key == Pad_A) && (!(app.GetCursor() == Cursor_Hidden) && app.IsConsole()))
					{
						mbutton = 1;
					}
					else
					{
						app.OnPad(k, XBoxButtonToKey_Map[i].Key, 1);
						Pads[k].RepeatDelay = 350000;
						didrepeat = 1;
					}
				}
				else if (isPressedOld && isPressedNew && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay
					&& ((XBoxButtonToKey_Map[i].Key != Pad_A) || ((app.GetCursor() == Cursor_Hidden) && app.IsConsole()) ) )
				{
					Pads[k].RepeatDelay = 100000;
					//pApp->OnPad(k, XBoxButtonToKey_Map[i].Key, 0);
					app.OnPad(k, XBoxButtonToKey_Map[i].Key, 1);
					didrepeat = 1;

				}
				else if (isPressedOld && !isPressedNew)
				{
					if ((XBoxButtonToKey_Map[i].Key == Pad_A) && (!(app.GetCursor() == Cursor_Hidden) && app.IsConsole()))
						mbutton = 0;
					else
						app.OnPad(k, XBoxButtonToKey_Map[i].Key, 0);
				}

			}

			if (newState.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
				Pads[k].InputState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				app.OnPad(k, Pad_L2, 1);
			else if (newState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
				Pads[k].InputState.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				app.OnPad(k, Pad_L2, 0);

			if (newState.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
				Pads[k].InputState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				app.OnPad(k, Pad_R2, 1);
			else if (newState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
				Pads[k].InputState.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				app.OnPad(k, Pad_R2, 0);

			//if ((newState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) !=
			//    (InputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB))
			//    mbutton = (newState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0;

			if ((newState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) !=
				(Pads[k].InputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB))
				rthumbbutton = (newState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0;

			if (newState.Gamepad.sThumbRX > 0)
			{
				newState.Gamepad.sThumbRX -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbRX < 0)
					newState.Gamepad.sThumbRX = 0;
			}
			else if (newState.Gamepad.sThumbRX < 0)
			{
				newState.Gamepad.sThumbRX += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbRX > 0)
					newState.Gamepad.sThumbRX = 0;
			}
			if (newState.Gamepad.sThumbRY > 0)
			{
				newState.Gamepad.sThumbRY -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbRY < 0)
					newState.Gamepad.sThumbRY = 0;
			}
			else if (newState.Gamepad.sThumbRY < 0)
			{
				newState.Gamepad.sThumbRY += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbRY > 0)
					newState.Gamepad.sThumbRY = 0;
			}
			if (newState.Gamepad.sThumbLX > 0)
			{
				newState.Gamepad.sThumbLX -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbLX < 0)
					newState.Gamepad.sThumbLX = 0;
			}
			else if (newState.Gamepad.sThumbLX < 0)
			{
				newState.Gamepad.sThumbLX += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbLX > 0)
					newState.Gamepad.sThumbLX = 0;
			}
			if (newState.Gamepad.sThumbLY > 0)
			{
				newState.Gamepad.sThumbLY -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbLY < 0)
					newState.Gamepad.sThumbLY = 0;
			}
			else if (newState.Gamepad.sThumbLY < 0)
			{
				newState.Gamepad.sThumbLY += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
				if (newState.Gamepad.sThumbLY > 0)
					newState.Gamepad.sThumbLY = 0;
			}

			//DPad emulation for the Left Stick

			int stickThreshold = 2000; // Experimental value that "feels good"

			bool curR = (newState.Gamepad.sThumbLX >  stickThreshold);
			bool oldR = (Pads[k].InputState.Gamepad.sThumbLX >  stickThreshold);
			bool curL = (newState.Gamepad.sThumbLX <  -stickThreshold);
			bool oldL = (Pads[k].InputState.Gamepad.sThumbLX <  -stickThreshold);
			bool curD = (newState.Gamepad.sThumbLY <  -stickThreshold);
			bool oldD = (Pads[k].InputState.Gamepad.sThumbLY <  -stickThreshold);
			bool curU = (newState.Gamepad.sThumbLY >  stickThreshold);
			bool oldU = (Pads[k].InputState.Gamepad.sThumbLY >  stickThreshold);
			bool curRR = (newState.Gamepad.sThumbRX >  stickThreshold);
			bool curLR = (newState.Gamepad.sThumbRX < -stickThreshold);
			bool curDR = (newState.Gamepad.sThumbRY < -stickThreshold);
			bool curUR = (newState.Gamepad.sThumbRY >  stickThreshold);

            float curXL = (float)newState.Gamepad.sThumbLX / 32768.0f;
            float curYL = (float)newState.Gamepad.sThumbLY / 32768.0f;
            float curXR = (float)newState.Gamepad.sThumbRX / 32768.0f;
            float curYR = (float)newState.Gamepad.sThumbRY / 32768.0f;

            if (curR || curL || curD || curU)
            {
                app.OnPadStick(k, Pad_LT, curXL, curYL);
            }

            if (curRR || curLR || curDR || curUR)
            {
                app.OnPadStick(k, Pad_RT, curXR, curYR);
            }

            Pads[k].nLStickLeft = Pads[k].nLStickRight = Pads[k].nLStickUp = Pads[k].nLStickDown = false;
			if (curR)
			{
				if (oldR && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
				{
					//pApp->OnPad(k, Pad_Right, 0);
					app.OnPad(k, Pad_Right, 1);
					Pads[k].RepeatDelay = 100000;
					didrepeat = 1;
				}
				else if (oldL)
					app.OnPad(k, Pad_Left, 0);
				if (oldL || !oldR)
				{
					app.OnPad(k, Pad_Right, 1);
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
					app.OnPad(k, Pad_Left, 1);
					Pads[k].RepeatDelay = 100000;
					didrepeat = 1;
				}
				else if (oldR)
					app.OnPad(k, Pad_Right, 0);
				if (oldR || !oldL)
				{
					app.OnPad(k, Pad_Left, 1);
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
					app.OnPad(k, Pad_Up, 1);
					Pads[k].RepeatDelay = 100000;
					didrepeat = 1;
				}
				else if (oldD)
					app.OnPad(k, Pad_Down, 0);
				if (oldD || !oldU)
				{
					app.OnPad(k, Pad_Up, 1);
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
					app.OnPad(k, Pad_Down, 1);
					Pads[k].RepeatDelay = 100000;
					didrepeat = 1;
				}
				else if (oldU)
					app.OnPad(k, Pad_Up, 0);
				if (oldU || !oldD)
				{
					app.OnPad(k, Pad_Down, 1);
					Pads[k].RepeatDelay = 350000;
					didrepeat = 1;
				}
				Pads[k].nLStickDown = true;
			}

			if (Pads[k].LStickDown && !Pads[k].nLStickDown)
				app.OnPad(k, Pad_Down, 0);

			if (Pads[k].LStickUp && !Pads[k].nLStickUp)
				app.OnPad(k, Pad_Up, 0);

			if (Pads[k].LStickLeft && !Pads[k].nLStickLeft)
				app.OnPad(k, Pad_Left, 0);

			if (Pads[k].LStickRight && !Pads[k].nLStickRight)
				app.OnPad(k, Pad_Right, 0);

			Pads[k].LStickDown = Pads[k].nLStickDown;
			Pads[k].LStickLeft = Pads[k].nLStickLeft;
			Pads[k].LStickRight = Pads[k].nLStickRight;
			Pads[k].LStickUp = Pads[k].nLStickUp;

            // Temporarily(?) enabling controller-as-mouse on PC. It won't interfere with the mouse functionality
            // on Windows as long as the customer isn't trying to use the mouse and a pad stick at the same time,
            // which is unlikely, and being able to use the pad stick as a mouse is useful for prototyping console
            // content on Windows. (holtzr 3/4/2013)
#if 0
			if (app.IsConsole())
#endif
            {
				Pads[k].MouseXadj = float(newState.Gamepad.sThumbRX) * moveScaleFactor * delta;
				Pads[k].MouseYadj = -float(newState.Gamepad.sThumbRY) * moveScaleFactor * delta;

				if (Pads[k].MouseXadj || Pads[k].MouseYadj)
				{
					Pads[k].MouseX += Pads[k].MouseXadj;
					Pads[k].MouseY += Pads[k].MouseYadj;
					if (Pads[k].MouseX < 0) Pads[k].MouseX = 0;
					if (Pads[k].MouseX >= app.GetViewSize().Width) Pads[k].MouseX = (float)app.GetViewSize().Width;
					if (Pads[k].MouseY < 0) Pads[k].MouseY = 0;
					if (Pads[k].MouseY >= app.GetViewSize().Height) Pads[k].MouseY = (float)app.GetViewSize().Height;

					app.OnMouseMove (k, Point<int>((int)Pads[k].MouseX, (int)Pads[k].MouseY), 0);
				}
				if (mbutton != -1)
					app.OnMouseButton(k, 0, mbutton ? 1 : 0, Point<int>((int)Pads[k].MouseX, (int)Pads[k].MouseY), 0);
				if (rthumbbutton != -1)
					app.OnMouseButton(k, 1, rthumbbutton ? 1 : 0, Point<int>((int)Pads[k].MouseX, (int)Pads[k].MouseY), 0);
			}

			memcpy(&Pads[k].InputState, &newState, sizeof(XINPUT_STATE));
		}
        else
        {
            Pads[k].PadConnected = false;
        }

		Pads[k].LastMsgTime = msgtime;
		if (didrepeat)
			Pads[k].LastRepeatTime = msgtime;
	}
}

void GamePad::DetectPads()
{
    XINPUT_STATE state;
    for (unsigned i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        Pads[i].PadConnected = (XInputGetState(i, &state) == ERROR_SUCCESS);
    }
}

void GamePad::SetMousePosition(const Point<unsigned> & p)
{
    for ( int i = 0; i < XUSER_MAX_COUNT; i++ )
    {
        Pads[i].MouseX = (float)p.x;
        Pads[i].MouseY = (float)p.y;
    }
}

}} // Scaleform::Platform