/**********************************************************************

PublicHeader:   Render
Filename    :   GL_GraphicsDevice.h
Content     :   
Created     :   2014/04/21
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#pragma once
#ifndef INC_GL_GraphicsDevice_h
#define INC_GL_GraphicsDevice_h

#include "Kernel/SF_Uncopyable.h"
#include "Render/Render_GraphicsDevice.h"
#include "Render/GL/GL_Extensions.h"

#if defined(SF_RENDER_GLES)
#include "Render/GL/GLES_ShaderDescs.h"
#else
#include "Render/GL/GL_ShaderDescs.h"
#endif

namespace Scaleform { namespace Render { namespace GL {

class ShaderManager;
class GraphicsDevice;
class GraphicsDeviceImmediate;
class GraphicsDeviceRecorder;
class MeshCache;
class MeshBuffer;
class Texture;
class ShaderObject;

class HALGLBuffer : public RefCountBase<HALGLBuffer, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
    friend class GL::MeshCache;
    friend class GL::MeshBuffer;
public:
    operator bool() const { return Name != 0; }
    HALGLBuffer(GLuint name = 0) : Name(name), Size(0), Usage(0) { }
protected:

    GLuint      Name;
    GLsizeiptr  Size;
    GLenum      Usage;
};

class HALGLTexture : public RefCountBase<HALGLTexture, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
    friend class GL::Texture;
public:
    operator bool() const { return Name != 0; }
    HALGLTexture(GLuint name = 0) : Name(name) { }
protected:

    GLuint      Name;
};

class HALGLRenderbuffer : public RefCountBase<HALGLRenderbuffer, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
public:
    operator bool() const { return Name != 0; }
    HALGLRenderbuffer() : Name(0) { }
    HALGLRenderbuffer(GLuint name, GLenum target, GraphicsDevice& device);
protected:
    Hash<GLenum,GLint>  Parameters;
    GLuint              Name;
};

class HALGLFramebuffer : public RefCountBase<HALGLFramebuffer, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
public:
    operator bool() const { return Name != 0; }
    HALGLFramebuffer() : Name(0) { }
    HALGLFramebuffer(GLuint name, GLenum binding, GraphicsDevice& device);

    struct FramebufferAttachment
    {
        Hash<GLenum,GLint>      Parameters;
        Ptr<HALGLRenderbuffer>  RenderBuffer;
        Ptr<HALGLTexture>       Texture;
    };
    const FramebufferAttachment* GetAttachment(GLenum a0) const;
protected:
    HashLH<GLenum, FramebufferAttachment>  BindingIndexerAttachments;
    GLuint                                 Name;
};

class HALGLProgram : public RefCountBase<HALGLProgram, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
public:
    operator bool() const { return Name != 0; }
    HALGLProgram(GLuint name = 0) : Name(name) { }
protected:

    GLuint      Name;
};

class HALGLProgramPipeline : public RefCountBase<HALGLProgramPipeline, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
    friend class GL::ShaderObject;
public:
    operator bool() const { return Name != 0; }
    HALGLProgramPipeline(GLuint name = 0) : Name(name) { }
protected:

    GLuint      Name;
};

class HALGLShader : public RefCountBase<HALGLShader, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
public:
    operator bool() const { return Name != 0; }
    HALGLShader(GLuint name = 0) : Name(name) { }
protected:

    GLuint      Name;
};

class HALGLUniformLocation : public RefCountBase<HALGLUniformLocation, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
public:
    operator bool() const { return Location >= 0; }
    HALGLUniformLocation(GLint location = -1) : Location(location) { };
protected:
    GLint Location;

};

class HALGLVertexArray : public RefCountBase<HALGLVertexArray, StatRender_RenderPipeline_Mem>
{
    friend class GraphicsDeviceImmediate;
    friend class GraphicsDeviceRecorder;
    friend class GL::MeshCache;
public:
    operator bool() const { return Name != 0; }
    HALGLVertexArray(GLuint name = 0) : Name(name) { }
protected:

    GLuint                              Name;
    HashLH<GLenum, Ptr<HALGLBuffer> >   BindingIndexerBuffers;
};



class GraphicsDevice : public Render::GraphicsDevice, public Unassignable
{
    friend class GL::ShaderManager;             // Modifies Caps.
    friend class ScopedImmediateDeviceUsage;    // calls clearCachedBindings.
public:
    GraphicsDevice();
    GraphicsDevice(const GraphicsDevice& other);
    virtual ~GraphicsDevice() { }

    unsigned    GetCaps();

    virtual bool CheckExtension(GLExtensionType type);
    virtual bool CheckGLVersion(unsigned reqMajor, unsigned reqMinor);

    virtual void glActiveTexture(GLenum  a0) = 0;
    virtual void glAttachShader(HALGLProgram* a0, HALGLShader* a1) = 0;
    virtual void glBindAttribLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2) = 0;
    virtual void glBindBuffer(GLenum  a0, HALGLBuffer* a1) = 0;
    virtual void glBindFragDataLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2) = 0;
    virtual void glBindFramebuffer(GLenum  a0, HALGLFramebuffer* a1) = 0;
    virtual void glBindProgramPipeline(HALGLProgramPipeline* a0) = 0;
    virtual void glBindRenderbuffer(GLenum  a0, HALGLRenderbuffer* a1) = 0;
    virtual void glBindTexture(GLenum  a0, HALGLTexture* a1) = 0;
    virtual void glBindVertexArray(HALGLVertexArray* a0) = 0;
    virtual void glBlendEquation(GLenum  a0) = 0;
    virtual void glBlendEquationSeparate(GLenum  a0, GLenum  a1) = 0;
    virtual void glBlendFunc(GLenum  a0, GLenum  a1) = 0;
    virtual void glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3) = 0;
    virtual void glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3) = 0;
    virtual void glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3) = 0;
    virtual GLenum glCheckFramebufferStatus(GLenum  a0) = 0;
    virtual void glClear(GLbitfield  a0) = 0;
    virtual void glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3) = 0;
    virtual GLenum glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2) = 0;
    virtual void glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3) = 0;
    virtual void glCompileShader(HALGLShader* a0) = 0;
    virtual void glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7) = 0;
    virtual void glCreateProgram(HALGLProgram* a0) = 0;
    virtual void glCreateShader(GLenum  a0, HALGLShader* a1) = 0;
    virtual void glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1)= 0;
    virtual void glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5)= 0;
    virtual void glDeleteBuffers(GLsizei  a0, HALGLBuffer** a1) = 0;
    virtual void glDeleteFramebuffers(GLsizei  a0, HALGLFramebuffer** a1) = 0;
    virtual void glDeleteProgram(HALGLProgram* a0) = 0;
    virtual void glDeleteProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1) = 0;
    virtual void glDeleteRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1) = 0;
    virtual void glDeleteShader(HALGLShader* a0) = 0;
    virtual void glDeleteSync(GLsync  a0) = 0;
    virtual void glDeleteTextures(GLsizei  a0, HALGLTexture** a1) = 0;
    virtual void glDeleteVertexArrays(GLsizei  a0, HALGLVertexArray** a1) = 0;
    virtual void glDepthFunc(GLenum  a0) = 0;
    virtual void glDepthMask(GLboolean  a0) = 0;
    virtual void glDisable(GLenum  a0) = 0;
    virtual void glDisableVertexAttribArray(GLuint  a0) = 0;
    virtual void glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2) = 0;
    virtual void glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3) = 0;
    virtual void glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4) = 0;
    virtual void glEnable(GLenum  a0) = 0;
    virtual void glEnableVertexAttribArray(GLuint  a0) = 0;
    virtual GLsync glFenceSync(GLenum  a0, GLbitfield  a1) = 0;
    virtual void glFlush() = 0;
    virtual void glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2) = 0;
    virtual void glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, HALGLRenderbuffer* a3) = 0;
    virtual void glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, HALGLTexture* a3, GLint  a4) = 0;
    virtual void glGenBuffers(GLsizei  a0, HALGLBuffer** a1) = 0;
    virtual void glGenFramebuffers(GLsizei  a0, HALGLFramebuffer** a1) = 0;
    virtual void glGenProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1) = 0;
    virtual void glGenRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1) = 0;
    virtual void glGenTextures(GLsizei  a0, HALGLTexture** a1) = 0;
    virtual void glGenVertexArrays(GLsizei  a0, HALGLVertexArray** a1) = 0;
    virtual void glGenerateMipmap(GLenum  a0) = 0;
    virtual void glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6) = 0;
    virtual GLenum glGetError() = 0;
    virtual void glGetFloatv(GLenum  a0, GLfloat * a1) = 0;
    virtual GLint glGetFragDataLocation(HALGLProgram* a0, const GLchar * a1) = 0;
    virtual void glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3) = 0;
    virtual void glGetIntegerv(GLenum  a0, GLint * a1) = 0;
    virtual void glGetProgramBinary(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4) = 0;
    virtual void glGetProgramInfoLog(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLchar * a3) = 0;
    virtual void glGetProgramiv(HALGLProgram* a0, GLenum  a1, GLint * a2) = 0;
    virtual void glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2) = 0;
    virtual void glGetShaderInfoLog(HALGLShader* a0, GLsizei  a1, GLsizei * a2, GLchar * a3) = 0;
    virtual void glGetShaderiv(HALGLShader* a0, GLenum  a1, GLint * a2) = 0;
    virtual const GLubyte * glGetString(GLenum  a0) = 0;
    virtual const GLubyte * glGetStringi(GLenum  a0, GLuint  a1) = 0;
    virtual void glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4) = 0;
    virtual void glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4) = 0;
    virtual void glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3) = 0;
    virtual void glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2) = 0;
    virtual void glGetUniformLocation(HALGLProgram* a0, const GLchar * a1, HALGLUniformLocation* a2) = 0;
    virtual GLboolean glIsFramebuffer(HALGLFramebuffer*  a0) = 0;
    virtual GLboolean glIsProgram(HALGLProgram* a0) = 0;
    virtual GLboolean glIsRenderbuffer(HALGLRenderbuffer* a0) = 0;
    virtual void glLinkProgram(HALGLProgram* a0) = 0;
    virtual void * glMapBuffer(GLenum  a0, GLenum  a1) = 0;
    virtual void * glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3) = 0;
    virtual void glPixelStorei(GLenum  a0, GLint  a1) = 0;
    virtual void glPolygonMode(GLenum  a0, GLenum  a1) = 0;
    virtual void glPopGroupMarker() = 0;
    virtual void glProgramBinary(HALGLProgram* a0, GLenum  a1, const void * a2, GLint  a3) = 0;
    virtual void glProgramParameteri(HALGLProgram* a0, GLenum  a1, GLint  a2) = 0;
    virtual void glProgramUniform1fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3) = 0;
    virtual void glProgramUniform1iv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLint * a3) = 0;
    virtual void glProgramUniform2fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3) = 0;
    virtual void glProgramUniform3fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3) = 0;
    virtual void glProgramUniform4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3) = 0;
    virtual void glProgramUniformMatrix4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4) = 0;
    virtual void glPushGroupMarker(GLsizei  a0, const GLchar * a1) = 0;
    virtual void glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6) = 0;
    virtual void glReleaseShaderCompiler() = 0;
    virtual void glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3) = 0;
    virtual void glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3) = 0;
    virtual void glShaderSource(HALGLShader* a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3) = 0;
    virtual void glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2) = 0;
    virtual void glStencilMask(GLuint  a0) = 0;
    virtual void glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2) = 0;
    virtual void glStringMarker(GLsizei  a0, const void * a1) = 0;
    virtual void glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8) = 0;
    virtual void glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2) = 0;
    virtual void glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8) = 0;
    virtual void glUniform1f(HALGLUniformLocation* a0, GLfloat  a1) = 0;
    virtual void glUniform1fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2) = 0;
    virtual void glUniform1i(HALGLUniformLocation* a0, GLint  a1) = 0;
    virtual void glUniform1iv(HALGLUniformLocation* a0, GLsizei  a1, const GLint * a2) = 0;
    virtual void glUniform2f(HALGLUniformLocation* a0, GLfloat  a1, GLfloat  a2) = 0;
    virtual void glUniform2fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2) = 0;
    virtual void glUniform3fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2) = 0;
    virtual void glUniform4fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2) = 0;
    virtual void glUniformMatrix4fv(HALGLUniformLocation* a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3) = 0;
    virtual GLboolean glUnmapBuffer(GLenum  a0) = 0;
    virtual void glUseProgram(HALGLProgram* a0) = 0;
    virtual void glUseProgramStages(HALGLProgramPipeline* a0, GLbitfield  a1, HALGLProgram* a2) = 0;
    virtual void glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5) = 0;
    virtual void glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3) = 0;

    // Obtain current bindings.
    HALGLBuffer*            GetBoundBuffer(GLenum a0);
    HALGLTexture*           GetBoundTexture(GLenum a0);
    HALGLFramebuffer*       GetBoundFramebuffer(GLenum a0);
    HALGLRenderbuffer*      GetBoundRenderbuffer(GLenum a0);
    HALGLProgramPipeline*   GetBoundProgramPipeline();
    HALGLVertexArray*       GetBoundVertexArray();

    static const int                            MaximumGLResourcesPerCall = 1024;     // Maximum number of resources expected to be used within a single GL call.

protected:

    void         clearCachedBindings();

    unsigned                                    MajorVersion, MinorVersion;                                     // Stored GL versions.
    bool                                        ExtensionSupported[SF_GL_Extension_Count];                      // Recording of whether extensions are supported.
    unsigned                                    Caps;                                                           // Capability flags. See CapFlags for values.

    HashLH<GLenum, Ptr<HALGLBuffer> >           BindingIndexerBuffers;                                          // Current GL buffer bindings.
    GLenum                                      BindingActiveTexture;                                           // GL_ACTIVE_TEXTURE value.
    HashLH<GLenum, Ptr<HALGLTexture> >          BindingIndexerTextures[FragShaderDesc::MaxTextureSamplers+1];   // Current GL texture bindings (one per possible sampler).
    HashLH<GLenum, Ptr<HALGLFramebuffer> >      BindingIndexerFramebuffers;                                     // Current GL framebuffer bindings.
    HashLH<GLenum, Ptr<HALGLRenderbuffer> >     BindingIndexerRenderbuffers;                                    // Current GL renderbuffer bindings.
    Ptr<HALGLProgramPipeline>                   BindingProgramPipeline;                                         // Current GL program pipeline binding.
    Ptr<HALGLVertexArray>                       BindingVertexArray;                                             // Current GL vertex array object (VAO) binding.
};

class GraphicsDeviceImmediate : public GraphicsDevice
{
    friend class GraphicsDevice;
public:

    GraphicsDeviceImmediate();

    virtual void Initialize(unsigned configFlags);
    virtual void Begin();
    virtual void End() { }

    void glActiveTexture(GLenum  a0);
    void glAttachShader(HALGLProgram* a0, HALGLShader* a1);
    void glBindAttribLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2);
    void glBindBuffer(GLenum  a0, HALGLBuffer* a1);
    void glBindFragDataLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2);
    void glBindFramebuffer(GLenum  a0, HALGLFramebuffer* a1);
    void glBindProgramPipeline(HALGLProgramPipeline* a0);
    void glBindRenderbuffer(GLenum  a0, HALGLRenderbuffer* a1);
    void glBindTexture(GLenum  a0, HALGLTexture* a1);
    void glBindVertexArray(HALGLVertexArray* a0);
    void glBlendEquation(GLenum  a0);
    void glBlendEquationSeparate(GLenum  a0, GLenum  a1);
    void glBlendFunc(GLenum  a0, GLenum  a1);
    void glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3);
    void glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3);
    void glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3);
    GLenum glCheckFramebufferStatus(GLenum  a0);
    void glClear(GLbitfield  a0);
    void glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3);
    GLenum glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2);
    void glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3);
    void glCompileShader(HALGLShader* a0);
    void glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7);
    void glCreateProgram(HALGLProgram* a0);
    void glCreateShader(GLenum  a0, HALGLShader* a1);
    void glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1);
    void glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5);
    void glDeleteBuffers(GLsizei  a0, HALGLBuffer** a1);
    void glDeleteFramebuffers(GLsizei  a0, HALGLFramebuffer** a1);
    void glDeleteProgram(HALGLProgram* a0);
    void glDeleteProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1);
    void glDeleteRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1);
    void glDeleteShader(HALGLShader* a0);
    void glDeleteSync(GLsync  a0);
    void glDeleteTextures(GLsizei  a0, HALGLTexture** a1);
    void glDeleteVertexArrays(GLsizei  a0, HALGLVertexArray** a1);
    void glDepthFunc(GLenum  a0);
    void glDepthMask(GLboolean  a0);
    void glDisable(GLenum  a0);
    void glDisableVertexAttribArray(GLuint  a0);
    void glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2);
    void glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3);
    void glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4);
    void glEnable(GLenum  a0);
    void glEnableVertexAttribArray(GLuint  a0);
    GLsync glFenceSync(GLenum  a0, GLbitfield  a1);
    void glFlush();
    void glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2);
    void glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, HALGLRenderbuffer* a3);
    void glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, HALGLTexture* a3, GLint  a4);
    void glGenBuffers(GLsizei  a0, HALGLBuffer** a1);
    void glGenFramebuffers(GLsizei  a0, HALGLFramebuffer** a1);
    void glGenProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1);
    void glGenRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1);
    void glGenTextures(GLsizei  a0, HALGLTexture** a1);
    void glGenVertexArrays(GLsizei  a0, HALGLVertexArray** a1);
    void glGenerateMipmap(GLenum  a0);
    void glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6);
    GLenum glGetError();
    void glGetFloatv(GLenum  a0, GLfloat * a1);
    GLint glGetFragDataLocation(HALGLProgram* a0, const GLchar * a1);
    void glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3);
    void glGetIntegerv(GLenum  a0, GLint * a1);
    void glGetProgramBinary(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4);
    void glGetProgramInfoLog(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetProgramiv(HALGLProgram* a0, GLenum  a1, GLint * a2);
    void glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    void glGetShaderInfoLog(HALGLShader* a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetShaderiv(HALGLShader* a0, GLenum  a1, GLint * a2);
    const GLubyte * glGetString(GLenum  a0);
    const GLubyte * glGetStringi(GLenum  a0, GLuint  a1);
    void glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4);
    void glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4);
    void glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3);
    void glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    void glGetUniformLocation(HALGLProgram* a0, const GLchar * a1, HALGLUniformLocation* a2);
    GLboolean glIsFramebuffer(HALGLFramebuffer* a0);
    GLboolean glIsProgram(HALGLProgram* a0);
    GLboolean glIsRenderbuffer(HALGLRenderbuffer* a0);
    void glLinkProgram(HALGLProgram* a0);
    void * glMapBuffer(GLenum  a0, GLenum  a1);
    void * glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3);
    void glPixelStorei(GLenum  a0, GLint  a1);
    void glPolygonMode(GLenum  a0, GLenum  a1);
    void glPopGroupMarker();
    void glProgramBinary(HALGLProgram* a0, GLenum  a1, const void * a2, GLint  a3);
    void glProgramParameteri(HALGLProgram* a0, GLenum  a1, GLint  a2);
    void glProgramUniform1fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform1iv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLint * a3);
    void glProgramUniform2fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform3fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniformMatrix4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4);
    void glPushGroupMarker(GLsizei  a0, const GLchar * a1);
    void glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6);
    void glReleaseShaderCompiler();
    void glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3);
    void glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);
    void glShaderSource(HALGLShader* a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3);
    void glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2);
    void glStencilMask(GLuint  a0);
    void glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2);
    void glStringMarker(GLsizei  a0, const void * a1);
    void glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2);
    void glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glUniform1f(HALGLUniformLocation* a0, GLfloat  a1);
    void glUniform1fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform1i(HALGLUniformLocation* a0, GLint  a1);
    void glUniform1iv(HALGLUniformLocation* a0, GLsizei  a1, const GLint * a2);
    void glUniform2f(HALGLUniformLocation* a0, GLfloat  a1, GLfloat  a2);
    void glUniform2fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform3fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform4fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniformMatrix4fv(HALGLUniformLocation* a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3);
    GLboolean glUnmapBuffer(GLenum  a0);
    void glUseProgram(HALGLProgram* a0);
    void glUseProgramStages(HALGLProgramPipeline* a0, GLbitfield  a1, HALGLProgram* a2);
    void glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5);
    void glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);

protected:
    GLImmediate Device;
};

class GraphicsDeviceRecorder : public GraphicsDevice, public Render::GraphicsDeviceRecorder
{
public:
    enum GLFunctions
    {
        HAL_glActiveTexture = PlatformCommands_Start,
        HAL_glAttachShader,
        HAL_glBindAttribLocation,
        HAL_glBindBuffer,
        HAL_glBindFragDataLocation,
        HAL_glBindFramebuffer,
        HAL_glBindProgramPipeline,
        HAL_glBindRenderbuffer,
        HAL_glBindTexture,
        HAL_glBindVertexArray,
        HAL_glBlendEquation,
        HAL_glBlendEquationSeparate,
        HAL_glBlendFunc,
        HAL_glBlendFuncSeparate,
        HAL_glBufferData,
        HAL_glBufferSubData,
        HAL_glCheckFramebufferStatus,
        HAL_glClear,
        HAL_glClearColor,
        HAL_glClientWaitSync,
        HAL_glColorMask,
        HAL_glCompileShader,
        HAL_glCompressedTexImage2D,
        HAL_glCreateProgram,
        HAL_glCreateShader,
        HAL_glDebugMessageCallback,
        HAL_glDebugMessageControl,
        HAL_glDeleteBuffers,
        HAL_glDeleteFramebuffers,
        HAL_glDeleteProgram,
        HAL_glDeleteProgramPipelines,
        HAL_glDeleteRenderbuffers,
        HAL_glDeleteShader,
        HAL_glDeleteSync,
        HAL_glDeleteTextures,
        HAL_glDeleteVertexArrays,
        HAL_glDepthFunc,
        HAL_glDepthMask,
        HAL_glDisable,
        HAL_glDisableVertexAttribArray,
        HAL_glDrawArrays,
        HAL_glDrawElements,
        HAL_glDrawElementsInstanced,
        HAL_glEnable,
        HAL_glEnableVertexAttribArray,
        HAL_glFenceSync,
        HAL_glFlush,
        HAL_glFlushMappedBufferRange,
        HAL_glFramebufferRenderbuffer,
        HAL_glFramebufferTexture2D,
        HAL_glGenBuffers,
        HAL_glGenFramebuffers,
        HAL_glGenProgramPipelines,
        HAL_glGenRenderbuffers,
        HAL_glGenTextures,
        HAL_glGenVertexArrays,
        HAL_glGenerateMipmap,
        HAL_glGetActiveUniform,
        HAL_glGetError,
        HAL_glGetFloatv,
        HAL_glGetFragDataLocation,
        HAL_glGetFramebufferAttachmentParameteriv,
        HAL_glGetIntegerv,
        HAL_glGetProgramBinary,
        HAL_glGetProgramInfoLog,
        HAL_glGetProgramiv,
        HAL_glGetRenderbufferParameteriv,
        HAL_glGetShaderInfoLog,
        HAL_glGetShaderiv,
        HAL_glGetString,
        HAL_glGetStringi,
        HAL_glGetSynciv,
        HAL_glGetTexImage,
        HAL_glGetTexLevelParameteriv,
        HAL_glGetTexParameteriv,
        HAL_glGetUniformLocation,
        HAL_glIsFramebuffer,
        HAL_glIsProgram,
        HAL_glIsRenderbuffer,
        HAL_glLinkProgram,
        HAL_glMapBuffer,
        HAL_glMapBufferRange,
        HAL_glPixelStorei,
        HAL_glPolygonMode,
        HAL_glPopGroupMarker,
        HAL_glProgramBinary,
        HAL_glProgramParameteri,
        HAL_glProgramUniform1fv,
        HAL_glProgramUniform1iv,
        HAL_glProgramUniform2fv,
        HAL_glProgramUniform3fv,
        HAL_glProgramUniform4fv,
        HAL_glProgramUniformMatrix4fv,
        HAL_glPushGroupMarker,
        HAL_glReadPixels,
        HAL_glReleaseShaderCompiler,
        HAL_glRenderbufferStorage,
        HAL_glScissor,
        HAL_glShaderSource,
        HAL_glStencilFunc,
        HAL_glStencilMask,
        HAL_glStencilOp,
        HAL_glStringMarker,
        HAL_glTexImage2D,
        HAL_glTexParameteri,
        HAL_glTexSubImage2D,
        HAL_glUniform1f,
        HAL_glUniform1fv,
        HAL_glUniform1i,
        HAL_glUniform1iv,
        HAL_glUniform2f,
        HAL_glUniform2fv,
        HAL_glUniform3fv,
        HAL_glUniform4fv,
        HAL_glUniformMatrix4fv,
        HAL_glUnmapBuffer,
        HAL_glUseProgram,
        HAL_glUseProgramStages,
        HAL_glVertexAttribPointer,
        HAL_glViewport,
    };

    GraphicsDeviceRecorder(GraphicsDeviceImmediate& immediateDevice);

    virtual void    Begin() {};
    virtual void    End();
    virtual bool    IsDeferred() const { return true; }

    virtual bool    ExecuteRecording(Render::GraphicsDevice& playbackDevice);

    void glActiveTexture(GLenum  a0);
    void glAttachShader(HALGLProgram* a0, HALGLShader* a1);
    void glBindAttribLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2);
    void glBindBuffer(GLenum  a0, HALGLBuffer* a1);
    void glBindFragDataLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2);
    void glBindFramebuffer(GLenum  a0, HALGLFramebuffer* a1);
    void glBindProgramPipeline(HALGLProgramPipeline* a0);
    void glBindRenderbuffer(GLenum  a0, HALGLRenderbuffer* a1);
    void glBindTexture(GLenum  a0, HALGLTexture* a1);
    void glBindVertexArray(HALGLVertexArray* a0);
    void glBlendEquation(GLenum  a0);
    void glBlendEquationSeparate(GLenum  a0, GLenum  a1);
    void glBlendFunc(GLenum  a0, GLenum  a1);
    void glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3);
    void glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3);
    void glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3);
    GLenum glCheckFramebufferStatus(GLenum  a0);
    void glClear(GLbitfield  a0);
    void glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3);
    GLenum glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2);
    void glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3);
    void glCompileShader(HALGLShader* a0);
    void glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7);
    void glCreateProgram(HALGLProgram* a0);
    void glCreateShader(GLenum  a0, HALGLShader* a1);
    void glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1);
    void glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5);
    void glDeleteBuffers(GLsizei  a0, HALGLBuffer** a1);
    void glDeleteFramebuffers(GLsizei  a0, HALGLFramebuffer** a1);
    void glDeleteProgram(HALGLProgram* a0);
    void glDeleteProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1);
    void glDeleteRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1);
    void glDeleteShader(HALGLShader* a0);
    void glDeleteSync(GLsync  a0);
    void glDeleteTextures(GLsizei  a0, HALGLTexture** a1);
    void glDeleteVertexArrays(GLsizei  a0, HALGLVertexArray** a1);
    void glDepthFunc(GLenum  a0);
    void glDepthMask(GLboolean  a0);
    void glDisable(GLenum  a0);
    void glDisableVertexAttribArray(GLuint  a0);
    void glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2);
    void glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3);
    void glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4);
    void glEnable(GLenum  a0);
    void glEnableVertexAttribArray(GLuint  a0);
    GLsync glFenceSync(GLenum  a0, GLbitfield  a1);
    void glFlush();
    void glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2);
    void glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, HALGLRenderbuffer* a3);
    void glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, HALGLTexture* a3, GLint  a4);
    void glGenBuffers(GLsizei  a0, HALGLBuffer ** a1);
    void glGenFramebuffers(GLsizei  a0, HALGLFramebuffer** a1);
    void glGenProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1);
    void glGenRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1);
    void glGenTextures(GLsizei  a0, HALGLTexture** a1);
    void glGenVertexArrays(GLsizei  a0, HALGLVertexArray** a1);
    void glGenerateMipmap(GLenum  a0);
    void glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6);
    GLenum glGetError();
    void glGetFloatv(GLenum  a0, GLfloat * a1);
    GLint glGetFragDataLocation(HALGLProgram* a0, const GLchar * a1);
    void glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3);
    void glGetIntegerv(GLenum  a0, GLint * a1);
    void glGetProgramBinary(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4);
    void glGetProgramInfoLog(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetProgramiv(HALGLProgram* a0, GLenum  a1, GLint * a2);
    void glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    void glGetShaderInfoLog(HALGLShader* a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetShaderiv(HALGLShader* a0, GLenum  a1, GLint * a2);
    const GLubyte * glGetString(GLenum  a0);
    const GLubyte * glGetStringi(GLenum  a0, GLuint  a1);
    void glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4);
    void glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4);
    void glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3);
    void glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    void glGetUniformLocation(HALGLProgram* a0, const GLchar * a1, HALGLUniformLocation* a2);
    GLboolean glIsFramebuffer(HALGLFramebuffer* a0);
    GLboolean glIsProgram(HALGLProgram* a0);
    GLboolean glIsRenderbuffer(HALGLRenderbuffer* a0);
    void glLinkProgram(HALGLProgram* a0);
    void * glMapBuffer(GLenum  a0, GLenum  a1);
    void * glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3);
    void glPixelStorei(GLenum  a0, GLint  a1);
    void glPolygonMode(GLenum  a0, GLenum  a1);
    void glPopGroupMarker();
    void glProgramBinary(HALGLProgram* a0, GLenum  a1, const void * a2, GLint  a3);
    void glProgramParameteri(HALGLProgram* a0, GLenum  a1, GLint  a2);
    void glProgramUniform1fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform1iv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLint * a3);
    void glProgramUniform2fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform3fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniformMatrix4fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4);
    void glPushGroupMarker(GLsizei  a0, const GLchar * a1);
    void glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6);
    void glReleaseShaderCompiler();
    void glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3);
    void glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);
    void glShaderSource(HALGLShader* a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3);
    void glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2);
    void glStencilMask(GLuint  a0);
    void glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2);
    void glStringMarker(GLsizei  a0, const void * a1);
    void glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2);
    void glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glUniform1f(HALGLUniformLocation* a0, GLfloat  a1);
    void glUniform1fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform1i(HALGLUniformLocation* a0, GLint  a1);
    void glUniform1iv(HALGLUniformLocation* a0, GLsizei  a1, const GLint * a2);
    void glUniform2f(HALGLUniformLocation* a0, GLfloat  a1, GLfloat  a2);
    void glUniform2fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform3fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniform4fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2);
    void glUniformMatrix4fv(HALGLUniformLocation* a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3);
    GLboolean glUnmapBuffer(GLenum  a0);
    void glUseProgram(HALGLProgram* a0);
    void glUseProgramStages(HALGLProgramPipeline* a0, GLbitfield  a1, HALGLProgram* a2);
    void glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5);
    void glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);

protected:
    void executeCommand( UInt32 commandName, GL::GraphicsDevice &device );
};

}}} // Scaleform::Render::GL

#endif
