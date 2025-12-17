/**************************************************************************

Filename    :   FxPlayerSWFtoTexture.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS3/AS3_Global.h"
#include "FxPlayerSWFToTexture.h"

// Render-thread commands implementation. They simply call a function on the App.
void InitGraphicsResourcesThreadCommand::Execute()
{
    pApp->InitGraphicsResources();
}

void DrawFrameThreadCommand::Execute()
{
    pApp->DrawFrame();
}

// Constructor - initialize all members to default values, and create our render thread commands.
FxPlayerSWFToTextureApp::FxPlayerSWFToTextureApp() :
    FxPlayerSWFToTextureAppBase(),
    pPlatformHAL(0),
    pRenderTarget(0),
    MeshRotationX(-30.0f),
    MeshRotationZ(-30.0f),
    LastRotationTick(0),
    MouseTracking(None),
    TextureTilt(None),
    CubeWireframe(false),
    RTWidth(1024),
    RTHeight(1024),
    pBGDef(0),
    pBG(0),
    pInitGraphicsResourcesCommand(0),
    pDrawFrameThreadCommand(0)
{   
    pInitGraphicsResourcesCommand = *SF_HEAP_AUTO_NEW(this) InitGraphicsResourcesThreadCommand(this);
    pDrawFrameThreadCommand       = *SF_HEAP_AUTO_NEW(this) DrawFrameThreadCommand(this);
}


// Called to initialize the application. We use this to load the .swf in which we will render to texture, and the also
// the .swf that is shown in the background. We also use it to issue the resource initialization code to the render thread.
bool FxPlayerSWFToTextureApp::OnInit(Platform::ViewConfig& config)
{
    // Call the base class OnInit function. 
    if (!FxPlayerSWFToTextureAppBase::OnInit(config))
        return false;

    // Once the base is initialized, the platform should make the HAL being used available.
    Platform::Device* ptDevice = GetDevice();
    if (!ptDevice || !ptDevice->GetHAL())
    {
        SF_DEBUG_WARNING(1, "Platform::Device, or Device::HAL not initialized properly.");
        return false;
    }
    pPlatformHAL = ptDevice->GetHAL();

    // Load the movie. This uses the GFxPlayer's LoadMovie function, which handles many cases.
    // For simpler example of movie loading, see TinyPlayer's Run function, which shows a very
    // simple example for loading movies.
    String filename = GetContentDirectory() + "3DWindow.swf";
    if (!LoadMovie(filename.ToCStr()))
    {
        SF_DEBUG_WARNING1(1,"Could not load: %s", filename.ToCStr());
        return false;
    }

    // Now load the background movie.
    unsigned loadConstants = Loader::LoadAll;
    filename = GetContentDirectory() + "3DWindowBackground.swf";
    pBGDef = *mLoader.CreateMovie(filename.ToCStr(), loadConstants);
    if (!pBGDef)
    {
        SF_DEBUG_WARNING1(1,"Could not load: %s", filename.ToCStr());
        return false;
    }

    pBG = *pBGDef->CreateInstance(false);
    if (!pBG)
        return false;
    pBG->Advance(0.0f, 0);

    // Set the "background alphas" of the rendered texture movie (this is the opacity of the stage).
    // We set it to almost completely transparent, so you can see the background movie through it.
    pMovie->SetBackgroundAlpha(0.1f);

    // Base class initialization was successful, now issue command to the render thread
    // to create the rendered texture, and vertex buffer.
    pRenderThread->PushThreadCommand(pInitGraphicsResourcesCommand);

    return true;
}

// Called when the application is shutting down. Destroys the background movie (the movie rendered
// to texture is handled by the base class, because it is loaded via LoadMovie).
void FxPlayerSWFToTextureApp::OnShutdown()
{
    pBG = 0;
    pBGDef = 0;

    // Call the base class shutdown.
    FxPlayerSWFToTextureAppBase::OnShutdown();
}

// Called to render a frame.
void FxPlayerSWFToTextureApp::DrawFrame()
{
    Render::Renderer2D* prenderer = pRenderThread->GetRenderer2D();
    MovieDisplayHandle        displayHandle = pBG->GetDisplayHandle();

    Platform::DeviceStatus status = AppBase::pDevice->GetStatus();
    if (status != Platform::Device_Ready)
        return;

    AppBase::pDevice->BeginFrame();
    prenderer->BeginFrame();

    // Render the movie to texture. This is performed in platform specific code. Note this will
    // set the movie's render target, then put it back to the original render target.
    RenderMovieToTexture();

    // Clear the render target (which is now the final framebuffer).
    AppBase::pDevice->Clear(Color(233,236,226,255).ToColor32());
    
    // Render the background movie.
    if (pBG)
    {
        if (displayHandle.NextCapture(prenderer->GetContextNotify()))
        {
            prenderer->Display(displayHandle);
        }
    }

    // Now, render a quad which has the movie texture applied to it (rotated, etc. based on mouse movements).
    pPlatformHAL->applyBlendMode(Blend_Normal);
    RenderMovieTextureToQuad();

    prenderer->EndFrame();
    AppBase::pDevice->PresentFrame();
}

// Called once per frame. It is overridden here, because the base class handles only handles rendering to a single target.
// Since we are rendering to multiple targets (one render target, and then to the backbuffer), we need to override this.
// Much of this code is replicated in FxPlayerAppBase::OnUpdateFrame.
void FxPlayerSWFToTextureApp::OnUpdateFrame( bool needRepaint )
{
    SF_UNUSED(needRepaint);

#if !defined (SF_BUILD_SHIPPING)
    HandleAmpAppMessages();
#endif   
    // Update timing.
    UInt64 timer = Timer::GetTicks();
    UInt64 delta = timer - PrevTicks;
    PrevTicks = timer;
    AccumClockTicks += delta;

    if (Paused && Rendering)
        delta = 0;
    else if (FastForward || !Rendering)
        delta = (UInt64)(1000000.0f / mMovieInfo.FPS);

    // Check to see if exit timeout was hit.
    if ((ExitTimeout > 0.0f) && (AccumClockTicks >= (ExitTimeout * 1000.0f)))
    {
        Shutdown();
        return;
    }

    // Update (software) cursors.
    Cursor.UpdateCursor(GetMousePos(), pRenderThread );

#ifdef GFX_VIDEO_USE_WWISE
    pWwise->Update();
#endif

    if (!AdvanceMovie((float)delta / 1000000.f))
    {
        // If movie signaled exit, we are done.
        Shutdown();
        return;
    }

    // Nothing more to do if we can't render.
    pRenderThread->UpdateDeviceStatus();

    // If the device is in an invalid state, continue to call NextCapture. Otherwise, Advance changes
    // may accumulate, using lots of memory.
    if ((!ValidateDeviceStatus() || !Rendering ) && pMovie && pBG)
    {
        // If the device is in an invalid state we still must ensure NextCapture gets 
        // called on display handles. Otherwise capture change data will build up indefinitely.
        MovieDisplayHandle handle = pMovie->GetDisplayHandle();
        handle.NextCapture(pRenderThread ? pRenderThread->GetRenderer2D()->GetContextNotify() : 0 );
        handle = pBG->GetDisplayHandle();
        handle.NextCapture(pRenderThread ? pRenderThread->GetRenderer2D()->GetContextNotify() : 0 );

        // Need to 'trick' the context into thinking that a frame has ended, so that NextCapture will
        // process the snapshots correctly. Otherwise, snapshot heaps will build up while the device is lost.
        if ( pRenderThread && pRenderThread->GetRenderer2D()->GetContextNotify())
            pRenderThread->GetRenderer2D()->GetContextNotify()->EndFrameContextNotify();

        // Make sure to tick AMP, otherwise the messages will build up.
        SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame();)
        return;
    }

    // Request that the frame be rendered.
    if (Rendering)
    {   
        pRenderThread->PushThreadCommand(pDrawFrameThreadCommand);
    }
    

    // Update timing
    FrameCounter++;    
    if ((LastFPSUpdatedTicks + 1000000) <= AccumClockTicks)
    {
        LastFPS = 1000000.f * FrameCounter / (float)(AccumClockTicks - LastFPSUpdatedTicks);
        AccumFPS += LastFPS;
        LastFPSUpdatedTicks = AccumClockTicks;
        AccumFPSSecondCount++;
        FrameCounter = 0;
        UpdateFpsDisplay();
    }
    
    // Update stats more frequently; each 1/5 of a second.
    if ((LastStatUpdatedTicks + 1000000/5) <= AccumClockTicks)
    {
        LastStatUpdatedTicks = AccumClockTicks;
        UpdateStatReports();
    }
}

// Overridden from base, so that both the background and render-to-texture movie can have Advance called on them.
bool FxPlayerSWFToTextureApp::AdvanceMovie( float deltaT )
{
    if (pMovie && pBG) 
    {
        UInt64 startAdvanceTicks = Timer::GetProfileTicks();

        Size<unsigned> vsz = GetViewSize();


        // Set the bakground movie to take the entire size of the rendered texture. 
        // Note: Set the View_IsRenderTexture flag. On certain platforms (GL), this will cause
        // the framebuffer to be inverted, so that texture coordinates are consistent between
        // platforms when using rendered textures.
        pMovie->SetViewport(RTWidth, RTHeight, 0, 0, RTWidth, RTHeight, 
            Render::Viewport::View_IsRenderTexture);

        // It's important to case size difference to int, as otherwise it will yield
        // large numbers instead of negative ones (which are allowed). 
        pBG->SetViewport((int)vsz.Width, (int)vsz.Height,
            ((int)vsz.Width-(int)MViewSize.Width)/2, 
            ((int)vsz.Height-(int)MViewSize.Height)/2,
            (int)MViewSize.Width, (int)MViewSize.Height);

        float timeTillNextFrame = pMovie->Advance(deltaT, 0);
        SetFrameTime(Alg::Min( 1.0f / mMovieInfo.FPS, timeTillNextFrame));
        pBG->Advance(deltaT, 0);

        LastAdvanceTicks = Timer::GetProfileTicks() - startAdvanceTicks;
    }
    else
    {
        LastAdvanceTicks = 0;
    }
    return true;
}

// Handles when the user pushes a mouse button.
void FxPlayerSWFToTextureApp::OnMouseButton( unsigned mouseIndex, unsigned button, bool downFlag, const Point<int>& mousePos, KeyModifiers mods )
{
    bool ControlKeyDown = mods.IsCtrlPressed();
    if (!pMovie)
        return;

    MousePrevPos = mousePos;
    Render::PointF p = AdjustToViewPort(mousePos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    // If they have pushed (instead of released) the button, change the mouse tracking state.
    if (downFlag) 
    {
        MouseDownPos = mousePos;

        if (button == 0 && ControlKeyDown)
        {
            MouseTracking = Zooming;
        }
        else if (button == 1)
        {
            MouseTracking = Tilting;
            TextureTilt = Tilting;
        }

        if (MouseTracking != None) 
        {
            ZoomStart = Zoom;
            MoveStart = Move;
            return;
        }

        // They are not in a special mouse tracking mode, so pass the event to the movie.
        MouseEvent event(GFx::Event::MouseDown, button, p.x, p.y, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    } 
    else 
    {
        // They were pushing a button, but have released it, change the mouse tracking back to 'None'
        if (MouseTracking != None) 
        {
            MouseTracking = None;
            return;
        }

        // Pass the mouse up to the movie.
        MouseEvent event(GFx::Event::MouseUp, button, p.x, p.y, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    } 
}

// Handle the mouse movements. 
void FxPlayerSWFToTextureApp::OnMouseMove( unsigned mouseIndex, const Point<int>& mousePos, KeyModifiers mods )
{
    SF_UNUSED(mods);

    Point<int> delta = MousePrevPos - mousePos;
    MousePrevPos = mousePos;

    if (!pMovie)
        return;

    // Handle zooming (initiatied by holding Ctrl).
    if (MouseTracking == Zooming) 
    {
        float dZoom = Zoom; 
        Zoom += 0.01f * delta.y * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;
        Render::PointF p = AdjustToViewPort(MouseDownPos);
		Render::Matrix2F m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(MoveStart.x, MoveStart.y);

        p = m.TransformByInverse(p);

        Move.x += dZoom * p.x;
        Move.y += dZoom * p.y;

        UpdateUserMatrix();
        return;
    }

    // Handle tilting of the quad.
    if (MouseTracking == Tilting)
    {
        MeshRotationZ += 0.25f * delta.x;
        MeshRotationX -= 0.25f * delta.y;
        return;
    }

    // We were not modifying the view of the rendered movie - send the event to the movie itself for processing.
    // Note that since we have may have modified the Zoom parameter, we must transform the input coordinate to
    // movie space.
	Render::PointF p = AdjustToViewPort(mousePos);
    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    MouseEvent event(GFx::Event::MouseMove, 0, p.x, p.y, 0.0f, mouseIndex);
    pMovie->HandleEvent(event);
}

// Renders the GFx movie to a rendered texture.
void FxPlayerSWFToTextureApp::RenderMovieToTexture()
{
    if (!pRenderTarget)
    {
        SF_DEBUG_WARNING(1, "Rendered target not initialized. Cannot render mesh.");
        return;
    }

    // Push the movie render target onto the render target stack. This will cause the movie
    // to be rendered into this target, instead of the backbuffer.
    pPlatformHAL->PushRenderTarget(Rect<int>(0,0,RTWidth,RTHeight), pRenderTarget);
    
    // Clear the render target. Note that the BeginScene is important, since calling 
    // clearSolidRectangle depends on the HAL to be in a valid rendering state (on some platforms).
    pPlatformHAL->BeginScene();
    pPlatformHAL->clearSolidRectangle(Rect<int>(0,0,RTWidth,RTHeight), Color(128, 128, 128, 200), false);

    // Now, render the GFx movie to the texture.
    if (pMovie)
    {
        Render::Renderer2D* prenderer = pRenderThread->GetRenderer2D();
        MovieDisplayHandle displayHandle = pMovie->GetDisplayHandle();;
        if (displayHandle.NextCapture(prenderer->GetContextNotify()))
        {
            prenderer->Display(displayHandle);
        }
    }

    // Restore the original render target, by removing our render target from the stack.
    pPlatformHAL->EndScene();
    pPlatformHAL->PopRenderTarget();
}

// This function computes the various matrices used to render the quad. They are applied as 
// uniforms/shader constants in different platforms. 
void FxPlayerSWFToTextureApp::ComputeMatrices()
{
    UInt64    ticks = Timer::GetTicks() / 1000;

    if (TextureTilt == None && (ticks - LastRotationTick) >= 20)
    {
        float tiltMax   = 10.5f;
        float tiltDelta = 0.2f;

        if (MeshRotationZ > -tiltMax && MeshRotationX == 0 || MeshRotationX < -tiltMax)
            MeshRotationZ -= tiltDelta;

        if (MeshRotationX < tiltMax && MeshRotationZ < -tiltMax)
            MeshRotationX += tiltDelta;

        if (MeshRotationZ < tiltMax && MeshRotationX > tiltMax)
            MeshRotationZ += tiltDelta;

        if (MeshRotationX > -tiltMax && MeshRotationZ > tiltMax)
            MeshRotationX -= tiltDelta;

        LastRotationTick = ticks;
    }
    else if (TextureTilt != None)
        LastRotationTick = ticks;

    MeshRotationX = (float)fmod(MeshRotationX, 360.0f);
    MeshRotationZ = (float)fmod(MeshRotationZ, 360.0f);

    MeshRotationX = Alg::Clamp(MeshRotationX, -45.0f, 45.0f);
    MeshRotationZ = Alg::Clamp(MeshRotationZ, -45.0f, 45.0f);

    // Compute the view matrix of the quad, based on the zoom, and two-rotational values.
    WorldMatrix = Matrix4F::RotationX((float)SF_DEGTORAD(MeshRotationX)) *
                  Matrix4F::RotationZ((float)SF_DEGTORAD(MeshRotationZ));

    Point3F vEyePt( 0.0f, 0.0f, -2.5f * Zoom );
    Point3F vLookatPt( 0.0f, 0.0f, 0.0f );
    Point3F vUpVec( 0.0f, 1.0f, 0.0f );
    ViewMatrix.ViewLH(vEyePt, vLookatPt, vUpVec);
    ViewMatrix.Transpose();

    // Setup the projection matrix.
    ProjMatrix.PerspectiveLH((float)(45.0f * SF_MATH_DEGTORAD), float(RTWidth)/float(RTHeight), 0.1f, 100.0f);
    ProjMatrix.Transpose();

    // Create the WxVxP matrix.
    WorldViewMatrix.SetToAppend(ViewMatrix, WorldMatrix);
    WorldViewProjMatrix.SetToAppend(ProjMatrix, WorldViewMatrix);
}

// This member defines the vertices that define the quad mesh.
FxPlayerSWFToTextureApp::Vertex FxPlayerSWFToTextureApp::QuadVertices[4] =
{
    {-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
    { 1.0f, 1.0f, 0.0f,  1.0f,0.0f },
    {-1.0f,-1.0f, 0.0f,  0.0f,1.0f },
    { 1.0f,-1.0f, 0.0f,  1.0f,1.0f },
};
