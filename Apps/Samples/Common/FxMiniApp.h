/**************************************************************************

Filename    :   FxMiniApp.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxMiniApp_H
#define INC_FxMiniApp_H

//#include "GTypes.h"
//#include "GRefCount.h"

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

class FxMiniApp
{
public:
    enum DeviceOrientations
    {
        Unknown,
        Portrait,
        Landscape
    };

    int                Width, Height;  // size of viewport
    DeviceOrientations Orientation;
    bool               OrientationFlip;

    virtual ~FxMiniApp() { };

    // Functions for application to implement
    virtual bool InitPreGraphics() { return 1; }
    virtual bool InitPostGraphics() { return 1; }
    virtual void Shutdown() { }

    virtual void OnFocus(bool /*set*/, unsigned int /*mods*/) { }
    virtual void OnDeviceRotate(int /*angle*/) { }
    virtual void OnSize(int /*w*/, int /*h*/) { }
    virtual void OnMouseButton(unsigned int /*mouseIndex*/, unsigned int /*button*/, bool /*downFlag*/, 
        int /*x*/, int /*y*/, unsigned int /*mods*/) {  }
    virtual void OnMouseMove(unsigned int /*mouseIndex*/, int /*x*/, int /*y*/, int unsigned /*mods*/) {}
    virtual void OnMouseWheel(unsigned int /*mouseIndex*/, int /*zdelta*/, int /*x*/, int /*y*/, 
        unsigned int /*mods*/) { }
    virtual void OnKeyboard(unsigned /*kbIndex*/, unsigned /*key*/, bool /*down*/, unsigned /*modifiers*/) { }
    virtual void OnTouch(GFx::Event::EventType /*type*/, unsigned /*id*/, int /*x*/, int /*y*/,
                         int /*wcontact*/, int /*hcontact*/, bool /*primary*/) { }

    virtual void OnGesture(GFx::Event::EventType type, unsigned /*gestureMask*/, int /*x*/, int /*y*/,
                           float /*translateX*/, float /*translateY*/, float /*scaleX*/, float /*scaleY*/,
                           float /*rotate*/) { }

    virtual bool OnOrientation(int /*orientation*/, int /*flip*/, bool /*force*/=0) { return false; }

    virtual void OnBeginZooming(int x, int y) { }
    virtual void OnUpdateMove(int x, int y) { }
    virtual void OnUpdateZoom(float z) { }
    virtual void OnEndZooming() { }

    virtual bool OnDropFile(const char* /*filename*/) { return 0; }

    virtual void AdvanceAndDisplay() = 0;

    // Functions provided by platform implementation
    virtual const char* GetContentDirectory() = 0;
    virtual const char* GetDeviceType() { return "unknown"; }
    virtual void RequestExit() = 0;
    virtual bool NeedsOrientationMatrix() { return 0; }
    virtual unsigned HasKeyboard() const { return 0; }
    virtual void SetWindowTitle(const char *ptitle)  = 0;
    virtual void SetFrameTime(int t) = 0;
    virtual void SetOrientationMode(bool change) { }
    virtual bool GetNextFilename(char *pfilename, bool prev) = 0;

    virtual void Present() = 0;
    virtual void Clear() = 0;

    // Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
    // This is a floating point value such as 0.1 for 10% safe frame border.
    virtual float   GetSafeArea() { return 0; }

    virtual HAL* GetRenderer() = 0;
};

#endif //INC_FxMiniApp_H
