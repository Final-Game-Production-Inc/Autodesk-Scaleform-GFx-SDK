/**************************************************************************

Filename    :   FxPlayerSWFtoTextureGL.h
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Ryan Holtz

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerSWFtoTextureGL_H
#define FX_PlayerSWFtoTextureGL_H

#include "FxPlayerSWFToTexture.h"
#include "Render/GL/GL_Common.h"
#include "Render/GL/GL_HAL.h"

class FxPlayerSWFToTextureAppGL : public FxPlayerSWFToTextureApp
{
public:
    FxPlayerSWFToTextureAppGL();

    virtual void    InitGraphicsResources();

protected:
    virtual void    RenderMovieTextureToQuad();

    // Helper functions.
    bool            CompileShader(GLint programId, GLenum type, const char* code);
    GL::HAL*        GetHAL();

    // Shader program variables.
    GLint               RenderTextureProgramId;     // The ID of the program which will render the cube mesh.
    GLint               WorldViewProjMatrixLoc;     // The location of the WVP matrix in the compiled vertex shader.
    GLint               TextureLoc;                 // The location of the texture sampler in the compiled fragment shader.
    GLuint              QuadMeshVertexBufferId;     // The ID of the vertex buffer, which contains the cube mesh data.
    GLuint              QuadMeshVAOId;              // The ID of the cube mesh VAO (if VAOs are in use - more details in InitGraphicsResources).
    GLuint              RenderTextureId;            // The ID of the rendered texture.
    GLuint              RenderTextureFbo;           // The ID of the F(rame)B(uffer)O(bject) for the rendered texture.
};

#endif
