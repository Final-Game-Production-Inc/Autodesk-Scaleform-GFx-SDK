/**************************************************************************

Filename    :   GFxPlayerGL.h
Content     :   Integration sample for OpenGL.
Created     :   Nov 1, 2012
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx.h"

namespace Scaleform 
{

//High level class for GFx Integration
class GFxPlayer
{
public:
    GFxPlayer();    
    ~GFxPlayer();

    // Initialize the player and run playback loop.
    int     Init();

    // Advance playback and render a frame.
    void    AdvanceAndDisplay();

    // Input handler functions. Process mouse and send appropriate GFx::Event notifications to Movie::HandleEvent.
    void    OnKey(unsigned keyCode, bool downFlag);
    void    OnMouseEvent(GFx::Event::EventType eventType, unsigned button, int x, int y);

    // Handle resize events.
    void    OnResize(int w, int h);

    // Performs hit-testing on the GFx content.
    bool    IsHit(int x, int y);

private:
    // Scaleform GFx object pointers.
    GFx::Loader                 mLoader;        // The Loader is responsible for loading GFx::MovieDef objects from .swf (or .gfx) files.
    Ptr<GFx::MovieDef>          pMovieDef;      // The MovieDef is the static data contained within a .swf (or .gfx).
    Ptr<GFx::Movie>			    pMovie;         // The Movie contains the runtime data associated with a .swf (or .gfx).

    Ptr<Render::Renderer2D>     pRenderer;      // The Renderer2D is the interface used for rendering GFx::Movie
    Ptr<Render::HAL>            pRenderHAL;     // The HAL is the object responsible for actually performing the rendering of the GFx::Movie.
    Render::ThreadCommandQueue* pCommandQueue;  // The queue used to issue commands to the renderer.
    GFx::MovieDisplayHandle     hMovieDisplay;  // The display handle, used to render the GFx::Movie.

    UInt32                      MovieLastTime;  // Holds the last time the GFx::Movie was Advanced (used for computing next delta time).
};

}; // namespace Scaleform
