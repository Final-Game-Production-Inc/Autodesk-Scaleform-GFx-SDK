/**************************************************************************

Filename    :   FxPlayerTextureInSWFGL.h
Content     :   Sample program to show geometry in a SWF
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerTextureInSWFGL_H
#define FX_PlayerTextureInSWFGL_H

#include "FxPlayerTextureInSWF.h"
#include "Render/GL/GL_Common.h"
#include "Render/GL/GL_HAL.h"

// The GL-platform specific application class. Note that this is used on all platforms
// which use any flavor of GL, including GLES. Fixed function platforms (GLES 1.1) are
// not supported by this sample. Note that we derive from HALNotify, because
// we want to be notified of the context reset events in GLES 2.0.
class FxPlayerTextureInSWFAppGL : public FxPlayerTextureInSWFApp, public HALNotify
{
public:
    FxPlayerTextureInSWFAppGL();

    // Overridden from FxPlayerTextureInSWFApp.
    virtual void    InitGraphicsResources();
    virtual void    RenderMesh();

    // Overridden from HALNotify
    virtual void    OnHALEvent(HALNotifyType type);

protected:

    // Helper functions.
    bool            CompileShader(GLint programId, GLenum type, const char* code);
    GL::HAL*        GetHAL();

    // Shader program variables.
    GLint               RenderTextureProgramId;     // The ID of the program which will render the cube mesh.
    GLint               WorldViewProjMatrixLoc;     // The location of the WVP matrix in the compiled vertex shader.
    GLuint              CubeMeshVertexBufferId;     // The ID of the vertex buffer, which contains the cube mesh data.
    GLuint              CubeMeshVAOId;              // The ID of the cube mesh VAO (if VAOs are in use - more details in InitGraphicsResources).
    GLuint              RenderTextureId;            // The ID of the rendered texture.
    GLuint              RenderTextureFbo;           // The ID of the F(rame)B(uffer)O(bject) for the rendered texture.

};

#endif
