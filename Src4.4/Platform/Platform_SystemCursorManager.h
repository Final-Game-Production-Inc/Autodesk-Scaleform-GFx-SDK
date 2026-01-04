/**************************************************************************

Filename    :   Platform_SystemCursorManager.h
Content     :   
Created     :   Feb 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_PLATFORM_SYSTEMCURSORMANAGER_H
#define INC_SF_PLATFORM_SYSTEMCURSORMANAGER_H
#pragma once

#include "Platform_CommandLine.h"
#include "Render/Render_Primitive.h"
#include "Render/Render_Types2D.h"

namespace Scaleform { namespace Platform {

class RenderHALThread;

// Cursor type expected for the client window, modified by
// calling SetCursor.
enum CursorType
{
    Cursor_Hidden,
    Cursor_Arrow,
    Cursor_Hand,
    Cursor_IBeam,
    Cursor_Type_Count
};

// SystemCursorState/Manager are helper classes that tracks different cursor states
// and applies and/or hides system cursor based on them. This class considers:
//  - NonClient area. If cursor is in non-client area, show arrow.
//  - Software state, causes HW cursor to be hidden.
//  - Enabled state, required to display cursor in client area.
class SystemCursorState
{
public:

    SystemCursorState(bool systemCursorAvailable, bool enabled = false) :
        Type(Cursor_Arrow), 
        Enabled(enabled), 
        Software(!systemCursorAvailable), 
        InClientArea(true),
        SystemAvailable(systemCursorAvailable)
    {
    }

    virtual ~SystemCursorState() { }

    bool    IsSystemAvailable() const       { return SystemAvailable; }
    void    SetSoftware(bool softwareFlag)  { Software = softwareFlag; update(); }
    bool    IsSoftware() const              { return Software; }
    void    EnableCursor(bool enabled)      { Enabled = enabled; update(); }
    bool    IsCursorEnabled() const         { return Enabled; }
    void    SetCursorType(CursorType type)  { Type = type; update(); }
    CursorType GetCursorType() const        { return IsCursorEnabled() ? Type : Cursor_Hidden; }
    void    OnSetCursorMessage(bool clientArea)
    {
        bool oldInClientArea = InClientArea;
        InClientArea = clientArea;
        // Force SetCursor if we went from non-client area to Client.
        update(!oldInClientArea && clientArea);
    }

protected:

    virtual void update(bool forceSet = false) { SF_UNUSED(forceSet); }

    CursorType                      Type;                           // Current cursor type
    bool                            Enabled;                        // Whether the cursor is enabled.
    bool                            Software;                       // Cursor is rendered in software.
    bool                            InClientArea;                   // Cursor is within the client window.
    bool                            SystemAvailable;                // Whether a system cursor is available.
};

class SystemCursorManagerImpl;
class SystemCursorManager : public SystemCursorState
{
public:
    SystemCursorManager(bool systemCursorAvailable);
    virtual ~SystemCursorManager();

    void    UpdateCursor(const Render::Point<int>& mousePos, RenderHALThread* pRenderThread);

protected:

    virtual void update(bool forceSet = false);

    SystemCursorManagerImpl *       pImpl;                          // Platform specific cursor manager implementation.
};

}} // Scaleform::Platform

#endif // INC_SF_PLATFORM_SYSTEMCURSORMANAGER_H
