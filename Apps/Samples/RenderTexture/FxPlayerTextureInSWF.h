/**********************************************************************

PublicHeader:   Render
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise, Bart Muzzin
Created     :   2013/02/01

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_FxPlayerTextureInSWF_H
#define INC_SF_FxPlayerTextureInSWF_H

// Must be included before 'HUD' base class decision, as it makes the platform #defines.
#include "Kernel/SF_Types.h"

// NOTE: On desktop system, we derive the application class from 'FxPlayerAppBaseHUD'. This adds
// the HUD, which is accessible via 'F1' and 'F2'. It displays help/statistics about the file being
// executed.
#if (defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)) || defined(SF_OS_MAC) || defined(SF_OS_LINUX)
#include "FxPlayerAppBaseHUD.h"
typedef FxPlayerAppBaseHUD FxPlayerTextureInSWFAppBase;
#else
#include "FxPlayerAppBase.h"
typedef FxPlayerAppBase FxPlayerTextureInSWFAppBase;
#endif

class FxPlayerTextureInSWFApp;

// Rendering command classes.

// This command gets sent to the render thread, to initialize the graphics resources, such
// as shaders, the rendered texture, and the vertex buffer for the cube mesh.
class InitGraphicsResourcesThreadCommand : public ThreadCommand
{
public:
    InitGraphicsResourcesThreadCommand(FxPlayerTextureInSWFApp* papp) : pApp(papp) { }
    virtual void Execute();
    FxPlayerTextureInSWFApp* pApp;
};

// This command gets sent to the render thread to re-render the cube mesh into the rendered texture.
class RenderMeshThreadCommand : public ThreadCommand
{
public:
    RenderMeshThreadCommand(FxPlayerTextureInSWFApp* papp) : pApp(papp) { }
    virtual void Execute();
    FxPlayerTextureInSWFApp* pApp;
};

// This command gets sent to the render thread to replace the texture within the loaded .swf
class ReplaceTextureThreadCommand : public ThreadCommand
{
public:
    ReplaceTextureThreadCommand(FxPlayerTextureInSWFApp* papp) : pApp(papp) { }
    virtual void Execute();
    FxPlayerTextureInSWFApp* pApp;
};

// The application class for the sample. See note above about the base class. There is a platform
// specific derived class, which will implement the actual rendering-to-texture portion of this
// sample (as it is platform specific). For example, FxPlayerTextureInSWFAppGL.
class FxPlayerTextureInSWFApp : public FxPlayerTextureInSWFAppBase
{
public:

    FxPlayerTextureInSWFApp();

    // From FxPlayerAppBase.
    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);

    // New interface
    void                    ReplaceTexture();
    virtual void            RenderMesh() = 0;
    virtual void            InitGraphicsResources() = 0;

protected:

    virtual void             ComputeMatrices();

    // Structure defining the elements of the cube mesh (contained in CubeVertices).
    struct Vertex
    {
        float x, y, z;
        unsigned char r, g, b, a;
    };

    // Graphics related members.
    Render::HAL*                        pPlatformHAL;           // The HAL used with this sample.
    Ptr<Render::Texture>                pMyHWTexture;           // The rendered texture, that contains the rendered cube.
    float                               MeshRotation;           // The current rotation of the cube.
    UInt64                              LastRotationTick;       // The last timer value, with respect to the cube rotation.
    bool                                CubeWireframe;          // Whether to render the cube in wireframe or not.
    unsigned                            RTWidth, RTHeight;      // The desired dimensions of the rendered texture.
    ImageSize                           OriginalResSize;        // The size of the original image resource we replace.
    static Vertex                       CubeVertices[24];       // The vertices that define the cube mesh.

    // Matrix members
    Matrix4F                            WorldMatrix;            // The world matrix of the cube mesh.
    Matrix4F                            ViewMatrix;             // The view matrix.
    Matrix4F                            WorldViewMatrix;        // The WxV matrix.
    Matrix4F                            ProjMatrix;             // The projection matrix.
    Matrix4F                            WorldViewProjMatrix;    // The full matrix chain (WxVxP).

    // Instances of our render thread commands. Definitions of these commands are in the .cpp file.
    Ptr<InitGraphicsResourcesThreadCommand>  pInitGraphicsResourcesCommand;
    Ptr<ReplaceTextureThreadCommand>         pReplaceTextureThreadCommand;
    Ptr<RenderMeshThreadCommand>             pRenderMeshCommand;
};

#endif // INC_SF_FxPlayerTextureInSWF_H
