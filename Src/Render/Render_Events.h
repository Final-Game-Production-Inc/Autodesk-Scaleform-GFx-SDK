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

// In some APIs, it is possible to determine whether there is a tool listening for GPU events.
// This is the list of possibilities.
enum EventListenerStatus
{
    ELS_Unknown,    // The system cannot determine the listening status.
    ELS_None,       // There is no supported tool attached which listens for GPU events.
    ELS_Listening,  // There is a tool attached which may be interpreting GPU events.
    ELS_Capturing,  // There is a tool attached which is currently interpreting GPU events.
};

class RenderEvent
{
public:
    virtual ~RenderEvent()                 { }
    virtual bool GetListenerStatus() const { return ELS_Unknown; };
    virtual void Begin( const char* eventName ) { SF_UNUSED(eventName); };
    virtual void End()                     { };
};

// Helper class for scoped GPU events.
class ScopedRenderEvent
{
public:
    ScopedRenderEvent(RenderEvent& evt, const char* eventName, bool trigger = true) : EventObj(evt)
    {
        // Trigger can be used to have an event end on a scope, even if it was being somewhere else.
        if ( trigger )
            EventObj.Begin(eventName);
    }
    ~ScopedRenderEvent()
    {
        EventObj.End();
    }

    ScopedRenderEvent& operator=(const ScopedRenderEvent&) { return *this; } // avoid warning.
private:
    RenderEvent& EventObj;
};

}} // Scaleform::GFx

#endif // INC_SF_Render_Events_H
