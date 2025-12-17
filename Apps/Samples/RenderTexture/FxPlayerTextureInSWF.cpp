/**************************************************************************

Filename    :   FxPlayerTextureInSWF.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise, Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerTextureInSWF.h"
#include "GFx/AS3/AS3_Global.h"

// Render-thread commands implementation. They simply call a function on the App.
void InitGraphicsResourcesThreadCommand::Execute()
{
    pApp->InitGraphicsResources();
}

void RenderMeshThreadCommand::Execute()
{
    pApp->RenderMesh();
}

void ReplaceTextureThreadCommand::Execute()
{
    pApp->ReplaceTexture();
}

// Constructor - initialize all members to default values, and create our render thread commands.
FxPlayerTextureInSWFApp::FxPlayerTextureInSWFApp() :
    pPlatformHAL(0),
    pMyHWTexture(0),
    MeshRotation(0.0f),
    LastRotationTick(0),
    CubeWireframe(false),
    RTWidth(256),
    RTHeight(256),
    OriginalResSize(0,0),
    pInitGraphicsResourcesCommand(0),
    pRenderMeshCommand(0)
{    
    // Create our render-thread commands.
    pInitGraphicsResourcesCommand = *SF_HEAP_AUTO_NEW(this) InitGraphicsResourcesThreadCommand(this);
    pReplaceTextureThreadCommand  = *SF_HEAP_AUTO_NEW(this) ReplaceTextureThreadCommand(this);
    pRenderMeshCommand            = *SF_HEAP_AUTO_NEW(this) RenderMeshThreadCommand(this);
}

// This function peforms the replacement of our rendered cube-texture on a resource within our loaded .swf.
void FxPlayerTextureInSWFApp::ReplaceTexture()
{
    // If the movie has not loaded, we cannot replace a texture within it. 
    if (!pMovie)
    {
        SF_DEBUG_WARNING(1, "Movie not loaded. Cannot call ReplaceTexture.");
        return;
    }

    // Make sure the HW texture has been created (by InitGraphicsResources).
    if (!pMyHWTexture)
    {
        SF_DEBUG_WARNING(1, "Rendered texture not created. Cannot call ReplaceTexture.");
        return;
    }

    // Locate the resource on which we want to place the rendered texture. The identifier
    // 'texture1' is hardcoded - it is known that the .swf that we load contains this resource.
    Resource*      pres = pMovie->GetMovieDef()->GetResource("texture1");
    ImageResource* pimageRes = 0;

    if (pres && pres->GetResourceType() == Resource::RT_Image)
    {
        pimageRes = (ImageResource*)pres;
    }

    // If we could not find the image resource, give a warning and then quit.
    if (!pimageRes)
    {
        SF_DEBUG_WARNING(1,"Could not find image resource 'texture1' within loaded .swf. ReplaceTexture unsuccessful.");
        return;
    }

    // Get the image on the resource, again, if it doesn't exist, give a warning and quit.
    ImageBase* pimageOrig = pimageRes->GetImage();
    if (!pimageOrig)
    {
        SF_DEBUG_WARNING(1,"Resource 'texture1' does not have an image to replace. ReplaceTexture unsuccessful.");
        return;
    }

    // Create an Render::Image (TextureImage) from our rendered texture.
    Ptr<TextureImage> pti = * SF_NEW TextureImage(Image_R8G8B8, pMyHWTexture->GetSize(), 0, pMyHWTexture);

    // If the texture is different size than the image it replaces, set the image matrix to their relative scales.
    // Note, this must be done before replacing the image, otherwise the dimensions will be for the new image. Also,
    // we must store the original image dimensions, if we are going to replace the image multiple times.
    if (OriginalResSize.IsEmpty())
        OriginalResSize = pimageOrig->GetSize();
    ImageSize newSize      = pMyHWTexture->GetSize();
    Size<float> scaleParameters(((float)OriginalResSize.Width) / newSize.Width, ((float)OriginalResSize.Height) / newSize.Height);

    // Note: In GL, render targets are 'upside-down' relative to their counterparts on other platforms. Thus, we
    // invert the scaling matrix, so the texture appears as it does on other platforms (passed in Size is unused).
    Size<int> sizeParam(1,1);
    Matrix2F& viewportMatrix = pPlatformHAL->GetMatrices()->GetFullViewportMatrix(sizeParam);
    bool invertTexture = (viewportMatrix.M[1][1] > 0.0f);
    Matrix2F textureMatrix = Matrix2F::Scaling(scaleParameters.Width, scaleParameters.Height * (invertTexture ? -1.0f : 1.0f));
    pti->SetMatrix(textureMatrix);

    // Now set the image on the resource.
    pimageRes->SetImage(pti);

    // This causes the images to update. If the movie has not been displayed yet, this is not strictly necessary,
    // but in the case of D3D9 device loss, and re-applying an image, it is required for the new image to be displayed.
    pMovie->ForceUpdateImages();
}

//------------------------------------------------------------------------

// Called to initialize the application. We use this to load the .swf in which we will replace the texture.
// We also use it to issue the resource initialization code to the render thread, and actually replace the texture.
bool FxPlayerTextureInSWFApp::OnInit(Platform::ViewConfig& config)
{
    // Call the base class OnInit function. 
    if (!FxPlayerTextureInSWFAppBase::OnInit(config))
    {
        SF_DEBUG_WARNING(1, "Application base OnInit failed.");
        return false;
    }

    // Once the base is initialized, the platform should make the HAL being used available.
    Platform::Device* pDeviceInner = GetDevice();
    if (!pDeviceInner || !pDeviceInner->GetHAL())
    {
        SF_DEBUG_WARNING(1, "Platform::Device, or Device::HAL not initialized properly.");
        return false;
    }
    pPlatformHAL = pDeviceInner->GetHAL();

    // Base class initialization was successful, now issue command to the render thread
    // to create the rendered texture, and vertex buffer.
    pRenderThread->PushThreadCommand(pInitGraphicsResourcesCommand);

    // Load the movie. This uses the GFxPlayer's LoadMovie function, which handles many cases.
    // For simpler example of movie loading, see TinyPlayer's Run function, which shows a very
    // simple example for loading movies.
    String filename = GetContentDirectory() + "Window_texture.swf";
    if (!LoadMovie(filename.ToCStr()))
    {
        SF_DEBUG_MESSAGE(1,"Failed to load: Window_texture.swf");
        return false;
    }

    // Now replace the texture within the movie (on the render thread). Note that this doesn't necessarily
    // need to happen on the render thread, but it depends on the InitGraphicsResourceCommand execution,
    // so if wasn't queued to happen on the render thread, this thread would need to wait for that to complete.
    pRenderThread->PushThreadCommand(pReplaceTextureThreadCommand);

    return true;
}

// Called once per frame. It is overridden here, so that the cube mesh rendering can be queued to the render thread.
void FxPlayerTextureInSWFApp::OnUpdateFrame( bool needRepaint )
{
    // Do the regular OnUpdateFrame, which will Advance the pMovie, update the HUD (if applicable), etc.
    FxPlayerAppBase::OnUpdateFrame(needRepaint);

    // Issue the command to render the cube mesh.
    pRenderThread->PushThreadCommand(pRenderMeshCommand);
}

// This function computes the various matrices used to render the cube. They are applied as 
// uniforms/shader constants in different platforms. 
void FxPlayerTextureInSWFApp::ComputeMatrices()
{
    // 1/10 revolution per second
    UInt64 ticks     = Timer::GetTicks() / 1000;
    float  time      = (float)((fmod((double)ticks, 20000.0) / 20000.0) * 2.0 * SF_MATH_PI);
    float  lastTime  = (float)((fmod((double)LastRotationTick, 20000.0) / 20000.0) * 2.0 * SF_MATH_PI);
       
    // Update the rotation, and handle wrap-around.
    LastRotationTick    = ticks;
    MeshRotation        += (time - lastTime);

    if (MeshRotation > (2.0f * SF_MATH_PI))
        MeshRotation -= (float)(2.0f * SF_MATH_PI);
    if (MeshRotation < 0.0f)
        MeshRotation += (float)(2.0f * SF_MATH_PI);


    // Update the world matrix with the rotation of the mesh.
    WorldMatrix = Matrix4F::RotationY((float)(MeshRotation));

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up 4 units, look at the
    // origin, and define "up" to be in the y-direction.
    Point3F vEyePt( 0.0f, 4.0f, -5.5f );
    Point3F vLookatPt( -1.0f, 0.0f, 0.0f );
    Point3F vUpVec( -0.1f, 1.0f, 0.0f );

    // View and projection matrix update.
    ViewMatrix.ViewLH(vEyePt, vLookatPt, vUpVec);
    ViewMatrix.Transpose();
    ProjMatrix.PerspectiveLH((float)(45.0f * SF_MATH_DEGTORAD), 1.0f, 0.1f, 100.0f);
    ProjMatrix.Transpose();

    // Create the WorldView and full matrix chain (World, View, Projection).
    WorldViewMatrix.SetToAppend(ViewMatrix, WorldMatrix);
    WorldViewProjMatrix.SetToAppend(ProjMatrix, WorldViewMatrix);
}

// This member defines the vertices that define the cube mesh.
FxPlayerTextureInSWFApp::Vertex FxPlayerTextureInSWFApp::CubeVertices[24] =
{
    {-1.0f, 1.0f,-1.0f,  180, 0, 0, 255 },
    { 1.0f, 1.0f,-1.0f,  180, 0, 0, 255 },
    {-1.0f,-1.0f,-1.0f,  180, 0, 0, 255 },
    { 1.0f,-1.0f,-1.0f,  180, 0, 0, 255 },

    {-1.0f, 1.0f, 1.0f,  180, 180, 0, 255 },
    {-1.0f,-1.0f, 1.0f,  180, 180, 0, 255 },
    { 1.0f, 1.0f, 1.0f,  180, 180, 0, 255 },
    { 1.0f,-1.0f, 1.0f,  180, 180, 0, 255 },

    {-1.0f, 1.0f, 1.0f,  0, 180, 180, 255 },
    { 1.0f, 1.0f, 1.0f,  0, 180, 180, 255 },
    {-1.0f, 1.0f,-1.0f,  0, 180, 180, 255 },
    { 1.0f, 1.0f,-1.0f,  0, 180, 180, 255 },

    {-1.0f,-1.0f, 1.0f,  0, 180, 0, 255 },
    {-1.0f,-1.0f,-1.0f,  0, 180, 0, 255 },
    { 1.0f,-1.0f, 1.0f,  0, 180, 0, 255 },
    { 1.0f,-1.0f,-1.0f,  0, 180, 0, 255 },

    { 1.0f, 1.0f,-1.0f,  0, 0, 180, 255 },
    { 1.0f, 1.0f, 1.0f,  0, 0, 180, 255 },
    { 1.0f,-1.0f,-1.0f,  0, 0, 180, 255 },
    { 1.0f,-1.0f, 1.0f,  0, 0, 180, 255 },

    {-1.0f, 1.0f,-1.0f,  180, 0, 180, 255 },
    {-1.0f,-1.0f,-1.0f,  180, 0, 180, 255 },
    {-1.0f, 1.0f, 1.0f,  180, 0, 180, 255 },
    {-1.0f,-1.0f, 1.0f,  180, 0, 180, 255 }
};
