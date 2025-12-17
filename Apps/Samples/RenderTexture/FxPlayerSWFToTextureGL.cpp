/**************************************************************************
 
 Filename    :   FxPlayerSWFtoTextureGL.cpp
 Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
 Created     :
 Authors     :   Ryan Holtz
 
 Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license
 agreement provided at the time of installation or download, or which
 otherwise accompanies this software in either electronic or hard copy form.
 
 **************************************************************************/

#include "FxPlayerSWFToTextureGL.h"

// Include the appropriate extensions, based on the GL platform type.
#if defined(SF_USE_GLES)
#include "Render/GL/GLES11_ExtensionMacros.h"
#elif defined(SF_USE_GLES2)
#include "Render/GL/GLES_ExtensionMacros.h"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#endif

FxPlayerSWFToTextureAppGL::FxPlayerSWFToTextureAppGL() :
    FxPlayerSWFToTextureApp(),
    RenderTextureProgramId(0),
    WorldViewProjMatrixLoc(-1),
    QuadMeshVertexBufferId(0),
    QuadMeshVAOId(0),
    RenderTextureId(0),
    RenderTextureFbo(0)
{

}

// Vertex shader code for quad rendering. Does normal vertex transformation, and passes through texture coordinates.
const char* g_vertexProgram =
#if defined(GL_VERSION_3_0)
    "#version 150\n"
    "uniform mat4 WorldViewProjMatrix;\n"
    
    "in vec3 Position;\n"
    "in vec2 InUV;\n"
    
    "out vec2 texCoord;\n"
    
    "void main() {\n"
    "    gl_Position = WorldViewProjMatrix * vec4(Position.xyz, 1.0);\n"
    "    texCoord = InUV;\n"
    "}";
#elif defined(SF_USE_OPENGL)
    "uniform mat4 WorldViewProjMatrix;\n"

    "attribute vec3 Position;\n"
    "attribute vec2 InUV;\n"

    "varying vec2 texCoord;\n"

    "void main() {\n"
    "    gl_Position = WorldViewProjMatrix * vec4(Position.xyz, 1.0);\n"
    "    texCoord = InUV;\n"
    "}";
#elif defined(SF_USE_GLES2)
    "uniform mat4 WorldViewProjMatrix;\n"

    "attribute vec3 Position;\n"
    "attribute vec2 InUV;\n"

    "varying vec2 texCoord;\n"

    "void main() {\n"
    "    gl_Position = WorldViewProjMatrix * vec4(Position.xyz, 1.0);\n"
    "    texCoord = InUV;\n"
    "}";
#endif

// Fragment shader code. Simply outputs the vertex color as the fragment color.
const char* g_fragmentProgram =
#if defined(GL_VERSION_3_0)
    "#version 150\n"

    "uniform sampler2D diffuseSampler;\n"

    "in vec2 texCoord;\n"
    "out vec4 FragColor;\n"

    "void main() {\n"
    "    FragColor = texture(diffuseSampler, texCoord);\n"
    "}\n";
#elif defined(SF_USE_OPENGL)
    "uniform sampler2D diffuseSampler;\n"

    "varying vec2 texCoord;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(diffuseSampler, texCoord);\n"
    "}\n";
#elif defined(SF_USE_GLES2)
    "precision mediump float;"      // GLES 2.0 fragment shaders require a precision statement.
    "uniform sampler2D diffuseSampler;\n"

    "varying vec2 texCoord;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(diffuseSampler, texCoord);\n"
    "}\n";
#endif

// Initializes the graphics resources used by the sample (this does not include the ones used by GFx).
// This includes the rendered texture, the depth/stencil buffer used when rendering the rendered texture,
// the shaders used to render the quad, and the vertex buffers for the quad mesh.
void FxPlayerSWFToTextureAppGL::InitGraphicsResources()
{   
    // Create the rendered texture, and its associated texture.
    glGenFramebuffers(1, &RenderTextureFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, RenderTextureFbo);    
    glGenTextures(1, &RenderTextureId);
    glBindTexture(GL_TEXTURE_2D, RenderTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RTWidth, RTHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Now create the depth/stencil buffer. We query the HAL, to determine whether the EXT_packed_depth_stencil
    // extension is available or not (or, that we are at least GL3.0, which has this format built-in). We use
    // this to either create a packed depth stencil (D24S8), or create separate depth and stencil targets.
    GLuint bufIds[2];
    glGenRenderbuffers(2, bufIds);
    bool packed = GetHAL()->CheckExtension("packed_depth_stencil") || GetHAL()->CheckGLVersion(3,0);

    if (packed)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, bufIds[0]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, RTWidth, RTHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufIds[0]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufIds[0]);
    }
    else
    {
        glBindRenderbuffer(GL_RENDERBUFFER, bufIds[0]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, RTWidth, RTHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, bufIds[1]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, RTWidth, RTHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufIds[0]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufIds[1]);
    }   
    
    // Clear the buffers we have just created.
    glClearStencil(0xffffffff);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Create a render target from the FBO we have just created.
    GL::HAL* phal = reinterpret_cast<GL::HAL*>(pPlatformHAL);
    pRenderTarget = *phal->CreateRenderTarget(RenderTextureFbo);

    // Create the shader program, and attach the vertex and fragment shaders to it.
    RenderTextureProgramId = glCreateProgram();

    if (!CompileShader(RenderTextureProgramId, GL_VERTEX_SHADER, g_vertexProgram))
    {
        printf("Vertex shader compilation failed\n");
        return;
    }
    
    if (!CompileShader(RenderTextureProgramId, GL_FRAGMENT_SHADER, g_fragmentProgram))
    {
        printf("Fragment shader compilation failed\n");
        return;
    }
        
    // Bind the position attribute.
    glBindAttribLocation(RenderTextureProgramId, 0, "Position");
    
    // OpenGL 3.x+ needs to bind the output fragcolor (GLES2 and OpenGL below 3.x use the built-in).
#if defined(GL_VERSION_3_0)
    glBindFragDataLocation(RenderTextureProgramId, 0, "FragColor");
#endif
    
    // Attempt to link the program. If it fails, print out the error.
    glLinkProgram(RenderTextureProgramId);
    GLint result;
    glGetProgramiv(RenderTextureProgramId, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        printf("Can't link shader. Output:\n");
        int length = 0;
        char logBuffer[32768];
        glGetProgramInfoLog(RenderTextureProgramId, 32768, &length, logBuffer);
        printf("%s\n", logBuffer);
        glDeleteProgram(RenderTextureProgramId);
        RenderTextureProgramId = 0;
        return;
    }

    // Get the uniform location of the WVP matrix and the texture sampler, once the shader has compiled.
    WorldViewProjMatrixLoc = glGetUniformLocation(RenderTextureProgramId, "WorldViewProjMatrix");
    TextureLoc = glGetUniformLocation(RenderTextureProgramId, "diffuseSampler");

    // Now initialize the vertex buffer for the quad mesh. Note that if we are using VAOs, we set them
    // up here. With or without VAOs, the quad vertices are still uploaded to a VBO.
    // (see http://www.opengl.org/wiki/Vertex_Specification for details)
    glGenBuffers(1, &QuadMeshVertexBufferId);

    if (GetHAL()->ShouldUseVAOs())
    {
        glBindVertexArray(0);
        glGenVertexArrays(1, &QuadMeshVAOId);
        glBindVertexArray(QuadMeshVAOId);
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, QuadMeshVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (GLvoid*)12);

    if (GetHAL()->ShouldUseVAOs())
        glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// This function renders a quad on the screen, with the GFx movie rendered texture applied to it.
void FxPlayerSWFToTextureAppGL::RenderMovieTextureToQuad()
{   
    // Ensure that many GL states are the way we expect (GFx may modify any of these states).
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);    
    glColorMask(true, true, true, true);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Wireframe mode not available in GLES 2.0.
#if !defined(SF_USE_GLES2)
    glPolygonMode(GL_FRONT_AND_BACK, CubeWireframe ? GL_LINE : GL_FILL);
#endif
    
    // Setup the world, view, and projection matrices
    ComputeMatrices();

    // Now set the shader program, and update the uniform (computed in ComputeMatrices)
    glUseProgram(RenderTextureProgramId);
    glUniformMatrix4fv(WorldViewProjMatrixLoc, 1, false, WorldViewProjMatrix);

    // Bind the vertex buffer with the quad vertices. If we are using VAOs, then we simply
    // need to bind the VAO for the quad, otherwise, we need to bind the buffer and enable
    // the vertex arrays.
    if (GetHAL()->ShouldUseVAOs())
    {
        glBindVertexArray(QuadMeshVAOId);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, QuadMeshVertexBufferId);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (GLvoid*)12);
    }

    // Bind the rendered texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, RenderTextureId);
    glUniform1i(TextureLoc, 0);

    // Draw the sides of the cube.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Helper function, which compiles a GLSL shader of the given type, and attaches
// it to the program. The shader is deleted, thus, once the program is destroyed,
// the shader will be as well.s
bool FxPlayerSWFToTextureAppGL::CompileShader(GLint programId, GLenum type, const char* code)
{
    GLuint shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &code, 0);
    glCompileShader(shaderId);
    GLint result = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        printf("Can't compile shader. Output:\n");
        int length = 0;
        char logBuffer[32768];
        glGetShaderInfoLog(shaderId, 32768, &length, logBuffer);
        printf("%s\n", logBuffer);
    }
    else
    {
        glAttachShader(programId, shaderId);
    }

    glDeleteShader(shaderId);
    return (result == GL_TRUE);
}

// This function is required to take advantage of runtime linking extensions already available
// within the GL::HAL. When SF_GL_RUNTIME_LINK is defined, GL extension methods become methods
// of the GL::HAL, and the macros inside "Render/GL/GL_ExtensionMacros.h" redefine them to their
// standard GL names. If not using runtime linking, the macros do nothing, and thus the GL methods
// are called directly.
GL::HAL* FxPlayerSWFToTextureAppGL::GetHAL()
{
    return reinterpret_cast<GL::HAL*>(pPlatformHAL);
}

//------------------------------------------------------------------------
// ***** Main APP implementation (This macro handles the main function, on all platforms).

SF_PLATFORM_SYSTEM_APP(SWFToTexture, Scaleform::GFx::System, FxPlayerSWFToTextureAppGL)
