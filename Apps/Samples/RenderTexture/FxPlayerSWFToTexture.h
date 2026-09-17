/**********************************************************************

PublicHeader:   Render
Filename    :   FxPlayerSWFToTexture.h
Content     :   
Created     :   2013/02/06
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_FxPlayerSWFToTexture_H
#define INC_SF_FxPlayerSWFToTexture_H

// Must be included before 'HUD' base class decision, as it makes the platform #defines.
#include "Kernel/SF_Types.h"

// NOTE: On desktop system, we derive the application class from 'FxPlayerAppBaseHUD'. This adds
// the HUD, which is accessible via 'F1' and 'F2'. It displays help/statistics about the file being
// executed.
#if (defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)) || defined(SF_OS_MAC) || defined(SF_OS_LINUX)
#include "FxPlayerAppBaseHUD.h"
typedef FxPlayerAppBaseHUD FxPlayerSWFToTextureAppBase;
#else
#include "FxPlayerAppBase.h"
typedef FxPlayerAppBase FxPlayerSWFToTextureAppBase;
#endif

class FxPlayerSWFToTextureApp;

// This command gets sent to the render thread, to initialize the graphics resources, such
// as shaders, the rendered texture, and the vertex buffer for the cube mesh.
class InitGraphicsResourcesThreadCommand : public ThreadCommand
{
public:
    InitGraphicsResourcesThreadCommand(FxPlayerSWFToTextureApp* papp) : pApp(papp) { }
    virtual void Execute();
    FxPlayerSWFToTextureApp* pApp;
};

// This command gets sent to the render thread to render the GFx movie to a texture, and to
// draw a quad with that texture applied to it.
class DrawFrameThreadCommand : public ThreadCommand
{
public:
    DrawFrameThreadCommand(FxPlayerSWFToTextureApp* papp) : pApp(papp) { }
    virtual void Execute();
    FxPlayerSWFToTextureApp * pApp;
};

class FxPlayerSWFToTextureApp : public FxPlayerSWFToTextureAppBase
{
public:
    FxPlayerSWFToTextureApp();

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnShutdown();
    virtual void            OnUpdateFrame(bool needRepaint);

    virtual void            OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
        const Point<int>& mousePos, KeyModifiers mods);    
    virtual void            OnMouseMove(unsigned mouseIndex,
        const Point<int>& mousePos, KeyModifiers mods);

    virtual bool            AdvanceMovie(float deltaT);
    void                    DrawFrame();

    virtual void            InitGraphicsResources() = 0;

protected:

    virtual void            RenderMovieToTexture();
    virtual void            RenderMovieTextureToQuad() = 0;
    virtual void            ComputeMatrices();

    // Used to describe the current mode of the mouse movements.
    enum TrackingState
    {
        None,
        Zooming,
        Moving,
        Tilting,
        Centering,
    };

    // Structure defining the elements of the quad mesh (contained in QuadVertices).
    struct Vertex
    {
        float x, y, z;
        float tu, tv;
    };

    // Graphics related members.
    Render::HAL*                        pPlatformHAL;          // The HAL used with this sample.
    Ptr<Render::RenderTarget>           pRenderTarget;         // The render target, in which the GFx movie will be rendered into.
    float                               MeshRotationX;         // The current rotation of the quad (x)
    float                               MeshRotationZ;         // The current rotation of the quad (z)
    UInt64                              LastRotationTick;      // The last timer value, with respect to the cube rotation.
    TrackingState                       MouseTracking;         // Handles the tracking state of the mouse.
    TrackingState                       TextureTilt;           // Handles the tracking state of the mouse (for tilting the texture)
    bool                                CubeWireframe;         // Whether to render the quad in wireframe or not.
    unsigned                            RTWidth, RTHeight;     // The desired dimensions of the rendered texture.
    static Vertex                       QuadVertices[4];       // The vertices that define the quad mesh.

    // Matrix members
    Matrix4F                            WorldMatrix;            // The world matrix of the quad mesh.
    Matrix4F                            ViewMatrix;             // The view matrix.
    Matrix4F                            WorldViewMatrix;        // The WxV matrix.
    Matrix4F                            ProjMatrix;             // The projection matrix.
    Matrix4F                            WorldViewProjMatrix;    // The full matrix chain (WxVxP).

    // GFx movie defs/instances
    Ptr<MovieDef>                       pBGDef;                // GFx movie that is rendered in the background.
    Ptr<Movie>                          pBG;                   // Instance of the GFx background movie (pBGDef).

    // Instances of our render thread commands. Definitions of these commands are in the .cpp file.
    Ptr<InitGraphicsResourcesThreadCommand> pInitGraphicsResourcesCommand;
    Ptr<DrawFrameThreadCommand>             pDrawFrameThreadCommand;
};

#endif // INC_SF_FxPlayerSWFToTexture_H
