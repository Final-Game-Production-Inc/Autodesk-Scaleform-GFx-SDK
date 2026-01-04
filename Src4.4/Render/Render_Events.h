/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Events.h
Content     :   'Events' used for debugging with various frame capture tools.
                This interface must be overridden to support each tool's API.
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_Events_H
#define INC_SF_Render_Events_H

#include "Kernel/SF_Uncopyable.h"

namespace Scaleform { namespace Render { 

// EventType is the list of rendering events that the event system tracks.
enum EventType
{
    // Top-level Commands
    Event_InitHAL,          // Brackets InitHAL
    Event_Frame,            // Brackets BeginFrame/EndFrame
    Event_Scene,            // Brackets BeginScene/EndScene
    Event_BeginScene,       // Brackets BeginScene/EndScene
    Event_Display,          // Brackets beginDisplay/endDisplay (lowercase!)

    // HAL Commands
    Event_Clear,            // Brackets all within clearSolidRectangle
    Event_Mask,             // Brackets PushMask_BeginSubmit/EndMask
    Event_PopMask,          // Brackets all within PopMask
    Event_MaskClear,        // Brackets all within a drawMaskClearRectangles
    Event_DrawPrimitive,    // Brackets all calls within DrawProcessedPrimitive
    Event_DrawComplex,      // Brackets all calls within DrawProcessedComplexMeshes
    Event_RenderTarget,     // Brackets all within PushRenderTarget/PopRenderTarget
    Event_Filter,           // Brackets all within PushFilters/PopFilters
    Event_ApplyBlend,       // Brackets all within applyBlendMode
    Event_ApplyDepthStencil,// Brackets all within applyDepthStencilMode
    Event_BlendMode,        // Brackets all within PushBlendMode/PopBlendMode

    // DrawableImage Commands
    Event_DrawableImage,    // Brackets all operations within a single DrawableImage
    Event_DICxform,         // Brackets all within ColorTransform command
    Event_DICompare,        // Brackets all within Compare command
    Event_DICopyPixels,     // Brackets all within CopyPixels command
    Event_DIMerge,          // Brackets all within Merge command (also CopyChannel)
    Event_DIPaletteMap,     // Brackets all within PaletteMap command
    Event_DIThreshold,      // Brackets all within Threshold command
    Event_DICopyback,       // Brackets all within a copy back to a DrawableImage.

    Event_Count
};

class RenderEvents
{
public:
    RenderEvents() :
        Disabled(false)
    {
        memset(OpenCount, 0, sizeof(OpenCount));
    }

    virtual ~RenderEvents()                 { }

    void SetDisabled(bool disable=true) { Disabled = disable; }
    
    virtual void Begin( EventType type, const char* eventName ) 
    {
#if defined(SF_RENDERER_PROFILE)
        if (Disabled)
            return;
            
        SF_DEBUG_ASSERT2(type < Event_Count, "Invalid RenderEvent issued (%d, name=%s)", type, eventName);
        OpenCount[type]++;
        beginImpl(eventName);
#else
        SF_UNUSED2(type, eventName);
#endif
    }
    virtual void End(EventType type)
    {
#if defined(SF_RENDERER_PROFILE)
        if (Disabled)
            return;

        SF_DEBUG_WARNONCE(OpenCount == 0, "Ending event that has not Begun.");
        OpenCount[type]--;
        endImpl();
#else
        SF_UNUSED(type);
#endif
    }
    unsigned GetOpenCount(EventType type) const { return OpenCount[type]; }

    virtual void EndFrame()
    {
        if (Disabled)
            return;

#if defined(SF_RENDERER_PROFILE)
        for (unsigned evt = 0; evt < Event_Count; ++evt)
        {
            SF_DEBUG_WARNING2(GetOpenCount(EventType(evt)) != 0, "RenderEvent(%d) has %d outstanding Begin calls at EndFrame.", evt, GetOpenCount(EventType(evt)));
        }
#endif
    }
protected:
    // These have default implementations instead of being pure virtual, for platforms which do not support RenderEvents at all.
    virtual void beginImpl(const char * eventName) { SF_UNUSED(eventName); }
    virtual void endImpl()                         { }

private:
    int    OpenCount[Event_Count];
    bool   Disabled;
};

// Helper class for scoped GPU events.
class ScopedRenderEvent : public Unassignable
{
public:
    ScopedRenderEvent(RenderEvents& events, EventType type, const char *eventName, bool trigger = true) : Type(type), EventObj(events)
    {
        // Trigger can be used to have an event end on a scope, even if it was being somewhere else.
        if ( trigger )
            EventObj.Begin(Type, eventName);
    }
    ~ScopedRenderEvent()
    {
        EventObj.End(Type);
    }

private:
    EventType    Type;
    RenderEvents& EventObj;
};

}} // Scaleform::GFx

#endif // INC_SF_Render_Events_H
