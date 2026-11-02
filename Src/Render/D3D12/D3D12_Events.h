/**************************************************************************

    PublicHeader:   Render
    Filename    :   D3D12_Events.h
    Content     :   'Events' used for debugging with various frame capture tools.
                    The D3D12 prototype does not wire up to any specific capture
                    tool, so events are implemented as no-ops. This class can
                    be extended to support WinPixEventRuntime / PIX markers.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_EVENTS_H
#define INC_SF_D3D12_EVENTS_H
#pragma once

#include "Render/Render_Events.h"

namespace Scaleform { namespace Render { namespace D3D12 {

// D3D12 RenderEvent implementation. The D3D12 API does not expose a simple
// per-command-list event marker interface in the stock headers (PIX support
// is provided through the separate WinPixEventRuntime), so the prototype
// implementation is a no-op that always reports that no capture tool is
// listening.
class RenderEvent : public Render::RenderEvent
{
public:
    RenderEvent() { }
    virtual ~RenderEvent() { }

    // Returns the listener status. No tool is attached in the prototype.
    virtual bool GetListenerStatus() const { return ELS_None; }

    // Begin/End are no-ops in the prototype implementation.
    virtual void Begin(const char* eventName) { SF_UNUSED(eventName); }
    virtual void End() { }
};

// Helper that wraps the array of RenderEvent objects used by the HAL.
class RenderEvents
{
public:
    RenderEvents() { }

    Render::RenderEvent& GetEvent(EventType eventType)
    {
        SF_ASSERT(eventType < Event_Count);
        return Events[eventType];
    }

private:
    RenderEvent Events[Event_Count];
};

}}} // Scaleform::Render::D3D12

#endif // INC_SF_D3D12_EVENTS_H
