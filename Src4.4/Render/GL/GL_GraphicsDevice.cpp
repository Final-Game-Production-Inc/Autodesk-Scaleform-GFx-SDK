/**********************************************************************

PublicHeader:   Render
Filename    :   GL_GraphicsDevice.cpp
Content     :   
Created     :   2014/04/21
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/GL/GL_GraphicsDevice.h"
#include "Render/GL/GL_HAL.h"

// Explicit template instantitation for Ptr classes and GL resource objects.
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLBuffer>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLTexture>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLRenderbuffer>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLFramebuffer>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLProgram>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLProgramPipeline>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLShader>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLUniformLocation>;
template class Scaleform::Ptr<Scaleform::Render::GL::HALGLVertexArray>;

namespace Scaleform { namespace Render { namespace GL {

template< typename T >
void ReleaseNullCheck(T* p)
{
    if (p)
        p->Release();
}

template< typename T >
void AddRefNullCheck(T* p)
{
    if (p)
        p->AddRef();
}


// Simple linear searching. This is only for debugging purposes, so performance is not an issue, and creating a hash seems like overkill.
struct GLEnumString
{
    GLenum      key;
    const char* str;
};

// Returns the string associated with the key, in the given list (list must terminate with key == 0).
const char* findEntry(GLenum key, GLEnumString* list)
{
    while (list && list->key)
    {
        if (list->key == key)
            return list->str;
        list++;
    }
    return "Unknown GLenum";
}

void GL_APIENTRY DebugMessageCallback(  GLenum source,
                                      GLenum type,
                                      GLuint id,
                                      GLenum severity,
                                      GLsizei,
                                      const char* message,
                                      void*)
{
    static GLEnumString sourceList[] = 
    {
        {GL_DEBUG_SOURCE_API,               "GL_DEBUG_SOURCE_API"},            
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM,     "GL_DEBUG_SOURCE_WINDOW_SYSTEM"},  
        {GL_DEBUG_SOURCE_SHADER_COMPILER,   "GL_DEBUG_SOURCE_SHADER_COMPILER"},
        {GL_DEBUG_SOURCE_THIRD_PARTY,       "GL_DEBUG_SOURCE_THIRD_PARTY"},    
        {GL_DEBUG_SOURCE_APPLICATION,       "GL_DEBUG_SOURCE_APPLICATION"},    
        {GL_DEBUG_SOURCE_OTHER,             "GL_DEBUG_SOURCE_OTHER"},          
        {0,                                 ""}
    };

    static GLEnumString typeList[] =
    {
        {GL_DEBUG_TYPE_ERROR,               "GL_DEBUG_TYPE_ERROR"},               
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"}, 
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"},  
        {GL_DEBUG_TYPE_PORTABILITY,         "GL_DEBUG_TYPE_PORTABILITY"},         
        {GL_DEBUG_TYPE_PERFORMANCE,         "GL_DEBUG_TYPE_PERFORMANCE"},         
        {GL_DEBUG_TYPE_OTHER,               "GL_DEBUG_TYPE_OTHER"},               
        {0,                                 ""}
    };

    static GLEnumString severityList[] =
    {
        {GL_DEBUG_SEVERITY_HIGH,    "GL_DEBUG_SEVERITY_HIGH"},  
        {GL_DEBUG_SEVERITY_MEDIUM,  "GL_DEBUG_SEVERITY_MEDIUM"},
        {GL_DEBUG_SEVERITY_LOW,     "GL_DEBUG_SEVERITY_LOW"},   
        {0,                         ""}
    };

    const char* sourceText   = findEntry(source, sourceList);
    const char* typeText     = findEntry(type, typeList);
    const char* severityText = findEntry(severity, severityList);

    LogDebugMessage(Log_Warning,
                    "GL Debug Message: %s\n"
                    "Source          : %s\n"
                    "Type            : %s\n"
                    "Severity        : %s\n"
                    "Id              : %d\n", 
                    message, sourceText, typeText, severityText, id);
}


HALGLFramebuffer::HALGLFramebuffer( GLuint name, GLenum binding, GraphicsDevice& device) : Name(name)
{
    // Create the internal texture/renderbuffer bindings.
    if (Name == 0)
        return;

    // Create internal bindings for the attachment points.
    GLenum attachmentTokens[] =
    {
        GL_STENCIL_ATTACHMENT,
        GL_DEPTH_ATTACHMENT,
        GL_COLOR_ATTACHMENT0,
        0
    };

    unsigned attachment = 0;
    while (attachmentTokens[attachment])
    {
        GLint type;
        FramebufferAttachment attachmentEntry;
        device.glGetFramebufferAttachmentParameteriv(binding, attachmentTokens[attachment], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
        attachmentEntry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, type);

        switch (type)
        {
            case GL_RENDERBUFFER:
            {
                GLint attachmentName;
                device.glGetFramebufferAttachmentParameteriv(binding, attachmentTokens[attachment], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachmentName);
                attachmentEntry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, attachmentName);
                attachmentEntry.RenderBuffer = *SF_NEW HALGLRenderbuffer(attachmentName, GL_RENDERBUFFER, device);
                break;
            }
            case GL_TEXTURE:
            {
                GLint attachmentName, textureLevel;
                device.glGetFramebufferAttachmentParameteriv(binding, attachmentTokens[attachment], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachmentName);
                device.glGetFramebufferAttachmentParameteriv(binding, attachmentTokens[attachment], GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &textureLevel);
                attachmentEntry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, attachmentName);
                attachmentEntry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, textureLevel);
                attachmentEntry.Texture = *SF_NEW HALGLTexture(attachmentName);            
                break;
            }
        }

        if (type != GL_NONE)
        {
            BindingIndexerAttachments.Set(attachmentTokens[attachment], attachmentEntry);
        }
        attachment++;
    }
}

const HALGLFramebuffer::FramebufferAttachment* HALGLFramebuffer::GetAttachment(GLenum a0) const
{
    return BindingIndexerAttachments.Get(a0);
}

HALGLRenderbuffer::HALGLRenderbuffer(GLuint name, GLenum target, GraphicsDevice& device) : Name(name)
{
    if (Name == 0)
        return;

    // If this is a pre-existing RenderBuffer, query the parameters that may be queried up front.
    GLenum params[] =
    {
        GL_RENDERBUFFER_WIDTH,
        GL_RENDERBUFFER_HEIGHT, 
        GL_RENDERBUFFER_INTERNAL_FORMAT, 
        GL_RENDERBUFFER_RED_SIZE, 
        GL_RENDERBUFFER_GREEN_SIZE, 
        GL_RENDERBUFFER_BLUE_SIZE, 
        GL_RENDERBUFFER_ALPHA_SIZE, 
        GL_RENDERBUFFER_DEPTH_SIZE, 
        GL_RENDERBUFFER_DEPTH_SIZE, 
        GL_RENDERBUFFER_STENCIL_SIZE, 
        GL_RENDERBUFFER_SAMPLES,
        0
    };

    device.glBindRenderbuffer(target, this);
    unsigned param = 0;
    while (params[param])
    {
        GLint value;
        device.glGetRenderbufferParameteriv(target, params[param], &value);
        Parameters.Set(params[param], value);
        param++;
    }    
}

GraphicsDevice::GraphicsDevice() : 
    Render::GraphicsDevice(),
    MajorVersion(0),
    MinorVersion(0),
    Caps(0)
{
    memset(ExtensionSupported, 0, sizeof(ExtensionSupported));
    clearCachedBindings();
}

GraphicsDevice::GraphicsDevice(const GraphicsDevice& other) :
    MajorVersion(other.MajorVersion),
    MinorVersion(other.MinorVersion),
    Caps(other.Caps)
{
    memcpy(ExtensionSupported, other.ExtensionSupported, sizeof(ExtensionSupported));
    clearCachedBindings();
}

bool GraphicsDevice::CheckExtension(GLExtensionType type)
{
    return ExtensionSupported[type];
}

bool GraphicsDevice::CheckGLVersion(unsigned reqMajor, unsigned reqMinor)
{
    return (MajorVersion > reqMajor || (MajorVersion == reqMajor && MinorVersion >= reqMinor));
}

unsigned GraphicsDevice::GetCaps()
{
    return Caps;
}

void GraphicsDevice::clearCachedBindings()
{
    BindingActiveTexture = GL_TEXTURE0;
    BindingIndexerBuffers.Clear();
    for (unsigned texture = 0; texture < FragShaderDesc::MaxTextureSamplers; ++texture)
        BindingIndexerTextures[texture].Clear();
    BindingIndexerFramebuffers.Clear();
    BindingIndexerRenderbuffers.Clear();
    BindingProgramPipeline.Clear();
    BindingVertexArray.Clear();
}

HALGLBuffer* GraphicsDevice::GetBoundBuffer(GLenum a0)
{
    Ptr<HALGLBuffer> * resource = BindingIndexerBuffers.Get(a0);
    return resource ? resource->GetPtr() : 0;
}

HALGLTexture* GraphicsDevice::GetBoundTexture(GLenum a0)
{
    Ptr<HALGLTexture> * resource = BindingIndexerTextures[BindingActiveTexture-GL_TEXTURE0].Get(a0);
    return resource ? resource->GetPtr() : 0;
}

HALGLFramebuffer* GraphicsDevice::GetBoundFramebuffer(GLenum a0)
{
    Ptr<HALGLFramebuffer> * resource = BindingIndexerFramebuffers.Get(a0);
    return resource ? resource->GetPtr() : 0;
}

HALGLRenderbuffer* GraphicsDevice::GetBoundRenderbuffer(GLenum a0)
{
    Ptr<HALGLRenderbuffer> * resource = BindingIndexerRenderbuffers.Get(a0);
    return resource ? resource->GetPtr() : 0;
}

HALGLProgramPipeline* GraphicsDevice::GetBoundProgramPipeline()
{
    return BindingProgramPipeline;
}

HALGLVertexArray* GraphicsDevice::GetBoundVertexArray()
{
    return BindingVertexArray;
}

GraphicsDeviceImmediate::GraphicsDeviceImmediate() : 
    GraphicsDevice()
{

}


void GraphicsDeviceImmediate::Initialize( unsigned configFlags )
{
    // Clear the error stack.
    GLenum error = ::glGetError();
    SF_DEBUG_ASSERT1(error == 0, "GL error before GraphicsDeviceImmediate::Initialize (0x%x)", error);
    SF_UNUSED(error);

    Device.Init((configFlags & HALConfig_TraceGLExecution) ? GLImmediate::Verbosity_DumpCalls : 0);

    // Copy the extensions and version information from the immediate device, now that it is initialized.
    SF_COMPILER_ASSERT(sizeof(Device.ExtensionSupported) == sizeof(ExtensionSupported));
    memcpy(ExtensionSupported, Device.ExtensionSupported, sizeof(ExtensionSupported));
    MajorVersion = Device.MajorVersion;
    MinorVersion = Device.MinorVersion;

    // Check for GL capabilities.
    Caps = 0;

#if defined(SF_RENDER_GLES)
    // Check for sync.
    if (CheckExtension(SF_GL_APPLE_sync) || CheckGLVersion(3,0))
        Caps |= Cap_Sync;

    // Check for map buffer range (must have sync as well)
    if ((Caps & Cap_Sync) && (CheckExtension(SF_GL_EXT_map_buffer_range) || CheckGLVersion(3,0)))
        Caps |= Cap_MapBufferRange;

    // Check for map buffer
    if (CheckExtension(SF_GL_OES_mapbuffer))
        Caps |= Cap_MapBuffer;

    // GLES2 has glBufferSubData built in.
    Caps |= Cap_BufferUpdate;

    // Check for binary shaders, but do not use them on PowerVR or Vivante drivers.
    if (CheckGLVersion(3,0) || CheckExtension(SF_GL_OES_get_program_binary))
        Caps |= Cap_BinaryShaders;

    // Check if derivatives (dFdx/dFdy) are not supported.
    if (!CheckGLVersion(3,0) && !CheckExtension(SF_GL_OES_standard_derivatives))
        Caps |= Cap_NoDerivatives;

    // GLES 3.0 has instancing, it is also provided by EXT_draw_instance.
    if (CheckGLVersion(3,0) || CheckExtension(SF_GL_EXT_draw_instanced))
        Caps |= Cap_Instancing;

    // Force vertex alignment on GLES devices. Vertex alignment is preferred for performance reasons.
    Caps |= MVF_Align;

#elif defined(SF_RENDER_OPENGL)

    // OpenGL (desktop)

    // Check for binary shaders
    if (CheckExtension(SF_GL_ARB_get_program_binary))
        Caps |= Cap_BinaryShaders;

    // Check for sync.
    if (CheckExtension(SF_GL_ARB_sync) && CheckGLVersion(3,1))
        Caps |= Cap_Sync;

    // Check for map buffer range (must have sync as well)
    // TODO: enable MapBufferRange update method on OpenGL, it currently does function as expected.
    // Check that the things we need for MapBufferRange are available.
    //if ((Caps & Cap_Sync) && CheckExtension("GL_ARB_map_buffer_range"))
    //    Caps |= Cap_MapBufferRange;

    Caps |= Cap_BufferUpdate; // Part of GL 2.1 spec.
    Caps |= Cap_MapBuffer;    // Part of GL 2.1 spec.

    Caps |= MVF_Align;        // Align for performance

    // Check for instancing (glDrawElementsInstanced is available).
    if ( (CheckGLVersion(3,0) || CheckExtension(SF_GL_ARB_draw_instanced)) && !configFlags)
        Caps |= Cap_Instancing;

#endif

    // If using a software deferred context, remove the ability to use MapBuffer, so that glBufferSubData is preferred.
    // glMapBuffer is not easy to support in a deferred context without using a large amount of additional memory, because
    // the contents of the buffer are preserved when using it, and there is no extra information about what sections of the
    // buffer are overwritten.
    if (configFlags&HALConfig_SoftwareDeferredContext)
        Caps &= ~Cap_MapBuffer;

    if (configFlags&HALConfig_NoVAO)
        Caps |= Cap_NoVAO;

    // If requested, turn on GL driver debugging messages (all of them).
    if (configFlags&HALConfig_DebugMessages)
    {
        if (CheckExtension(SF_GL_ARB_debug_output) || CheckExtension(SF_GL_KHR_debug))
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, true);
            glDebugMessageCallback((GLDEBUGPROC)DebugMessageCallback, 0);
        }
        else
        {
            SF_DEBUG_WARNING(1, "HALConfig_DebugMessages was specified in InitHALParams, but neither "
                "GL_ARB_debug_output or GL_KHR_debug extensions are not available\n");
        }
    }

    GLint maxUniforms = 128;
#if defined(SF_RENDER_GLES)
#if defined(GL_MAX_VERTEX_UNIFORM_VECTORS)
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxUniforms);
#endif
#else
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxUniforms);
#endif

#if defined(SF_OS_ANDROID)
    const char *ren = (const char*) glGetString(GL_RENDERER);
    const char *ven = (const char*) glGetString(GL_VENDOR);

    // Handicapping of particular android devices, because of driver issues.

    // Check for Vivante GPU and if found disable batching.
    // TODO: Find out why batching doesn't work
    if (!strncmp(ven, "Vivante", 7)) 
        Caps |= Cap_NoBatching;

    // Batching uniforms: 128 vertex uniforms should be guaranteed, but some drivers crash when more 
    // than 64 are used. The SGX 544 experiences extreme corruption when using more than 24. 
    if (!strncmp(ren, "PowerVR SGX 5", 12))
        maxUniforms = 64;
    if (!strncmp(ren, "PowerVR SGX 544", 14)) 
        maxUniforms = 24;

    // VAOs don't work properly on SGX 5xx series, and cause corruption and crashes.
    if (!strncmp(ren, "PowerVR SGX 5", 12))
        Caps |= Cap_NoVAO;

    // Disable sync cap for Mali (and related MapBufferRange), as it performs very badly.
    if (!strncmp(ren, "Mali", 4))
    {
        Caps &= ~Cap_Sync;
        Caps &= ~Cap_MapBufferRange;
    }

    // Do not use binary shaders on PowerVR or Vivante drivers.
    if (!strncmp(ren, "PowerVR", 7) || !strncmp(ven, "Vivante", 7))
        Caps &= ~Cap_BinaryShaders;

    // Vivante and PVR 540 drivers will crash when doing many glTexSubImage calls on FBOs, if they aren't set every time.
    if (!strncmp(ven, "Vivante", 7) || !strncmp(ren, "PowerVR SGX 540", 14))
        Caps |= Cap_FBORedundancy;
#endif

    Caps |= maxUniforms << Cap_MaxUniforms_Shift;
}

void GraphicsDeviceImmediate::Begin()
{
    // Clear the error state (and warn if it was tripped).
    GLenum error = glGetError();
    SF_UNUSED(error);
    SF_DEBUG_WARNONCE1(error != 0, "GL error %x before GL::HAL::BeginScene.", error);

    clearCachedBindings();
}

void GraphicsDeviceImmediate::glActiveTexture(GLenum  a0)
{
    //if (BindingActiveTexture != a0)
    {
        SF_DEBUG_ASSERT2(a0-GL_TEXTURE0 < FragShaderDesc::MaxTextureSamplers+1, 
            "Unexpected input to glActiveTexture (%d). Expected less than (%d)", a0-GL_TEXTURE0, FragShaderDesc::MaxTextureSamplers+1);
        Device.glActiveTexture(a0);
        BindingActiveTexture = a0;
    }
}

void GraphicsDeviceImmediate::glAttachShader(HALGLProgram* a0, HALGLShader* a1)
{
    Device.glAttachShader(a0 ? a0->Name : 0, a1 ? a1->Name : 0);
}

void GraphicsDeviceImmediate::glBindAttribLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2)
{
    Device.glBindAttribLocation(a0 ? a0->Name : 0, a1, a2);
}

void GraphicsDeviceImmediate::glBindBuffer(GLenum  a0, HALGLBuffer* a1)
{
    Ptr<HALGLBuffer> currentResource;

    // NOTE: Must differentiate between binding to a VAO, and a non-zero VAO, when using GL_ARRAY_BUFFER_ and GL_ELEMENT_ARRAY_BUFFER.
    if ((a0 != GL_ARRAY_BUFFER && a0 != GL_ELEMENT_ARRAY_BUFFER) || BindingVertexArray == 0)
    {
        if (BindingIndexerBuffers.Get(a0, &currentResource) && (currentResource == a1))
            return;
        BindingIndexerBuffers.Set(a0, a1);
    }
    else if (BindingVertexArray != 0)
    {
        // Recorder must make the binding on the object as well, so redundancy checking is difficult.
        BindingVertexArray->BindingIndexerBuffers.Set(a0, a1);
    }
    Device.glBindBuffer(a0, a1 ? a1->Name : 0);
}

void GraphicsDeviceImmediate::glBindFragDataLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2)
{
    Device.glBindFragDataLocation(a0 ? a0->Name : 0, a1, a2);
}

void GraphicsDeviceImmediate::glBindFramebuffer(GLenum  a0, HALGLFramebuffer* a1)
{
    Ptr<HALGLFramebuffer> currentResource;
    if (BindingIndexerFramebuffers.Get(a0, &currentResource) && (currentResource == a1))
    {
        // NOTE: if the Cap_FBORedundancy is set, it means that glBindFramebuffer should be called,
        // even if it is detected (here) that the call is redundant.
        if ((Caps & Cap_FBORedundancy) == 0)
            return;
    }
    BindingIndexerFramebuffers.Set(a0, a1);
    Device.glBindFramebuffer(a0, a1 ? a1->Name : 0);
}

void GraphicsDeviceImmediate::glBindProgramPipeline(HALGLProgramPipeline* a0)
{
    if (BindingProgramPipeline == a0)
        return;
    BindingProgramPipeline = a0;
    Device.glBindProgramPipeline(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glBindRenderbuffer(GLenum  a0, HALGLRenderbuffer* a1)
{
    Ptr<HALGLRenderbuffer> currentResource;
    if (BindingIndexerRenderbuffers.Get(a0, &currentResource) && (currentResource == a1))
        return;
    BindingIndexerRenderbuffers.Set(a0, a1);
    Device.glBindRenderbuffer(a0, a1 ? a1->Name : 0);
}

void GraphicsDeviceImmediate::glBindTexture(GLenum  a0, HALGLTexture* a1)
{
    Ptr<HALGLTexture> currentResource;
    if (BindingIndexerTextures[BindingActiveTexture-GL_TEXTURE0].Get(a0, &currentResource) && (currentResource == a1))
        return;
    BindingIndexerTextures[BindingActiveTexture-GL_TEXTURE0].Set(a0, a1);
    Device.glBindTexture(a0, a1 ? a1->Name : 0);
}

void GraphicsDeviceImmediate::glBindVertexArray(HALGLVertexArray* a0)
{
    if (BindingVertexArray == a0)
        return;
    BindingVertexArray = a0;
    BindingIndexerBuffers.Clear();
    Device.glBindVertexArray(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glBlendEquation(GLenum  a0)
{
    Device.glBlendEquation(a0);
}

void GraphicsDeviceImmediate::glBlendEquationSeparate(GLenum  a0, GLenum  a1)
{
    Device.glBlendEquationSeparate(a0, a1);
}

void GraphicsDeviceImmediate::glBlendFunc(GLenum  a0, GLenum  a1)
{
    Device.glBlendFunc(a0, a1);
}

void GraphicsDeviceImmediate::glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3)
{
    Device.glBlendFuncSeparate(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3)
{
    Ptr<HALGLBuffer> currentBuffer;
    if (BindingVertexArray == 0 &&
        (!BindingIndexerBuffers.Get(a0, &currentBuffer) || currentBuffer.GetPtr() == 0))
    {
        return;
    }
    else if (BindingVertexArray != 0)
    {
        if (!BindingVertexArray->BindingIndexerBuffers.Get(a0, &currentBuffer))
            return;
    }
    currentBuffer->Size = a1;
    currentBuffer->Usage = a3;
    Device.glBufferData(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3)
{
    Device.glBufferSubData(a0, a1, a2, a3);
}

GLenum GraphicsDeviceImmediate::glCheckFramebufferStatus(GLenum  a0)
{
    return Device.glCheckFramebufferStatus(a0);
}

void GraphicsDeviceImmediate::glClear(GLbitfield  a0)
{
    Device.glClear(a0);
}

void GraphicsDeviceImmediate::glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3)
{
    Device.glClearColor(a0, a1, a2, a3);
}

GLenum GraphicsDeviceImmediate::glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2)
{
    return Device.glClientWaitSync(a0, a1, a2);
}

void GraphicsDeviceImmediate::glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3)
{
    Device.glColorMask(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glCompileShader(HALGLShader* a0)
{
    Device.glCompileShader(a0 ? a0->Name : 0);

    // NOTE: If you don't immediately ask whether a shader compiled successfully, the Tegra driver will crash on backgrounding.
    GLint status;
    Device.glGetShaderiv(a0 ? a0->Name : 0, GL_COMPILE_STATUS, &status);
}

void GraphicsDeviceImmediate::glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7)
{
    Device.glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
}

void GraphicsDeviceImmediate::glCreateProgram(HALGLProgram* a0)
{
    SF_DEBUG_ASSERT(a0, "Invalid NULL resource.");
    if (a0)
        a0->Name = Device.glCreateProgram();
}

void GraphicsDeviceImmediate::glCreateShader(GLenum  a0, HALGLShader* a1)
{
    SF_DEBUG_ASSERT(a1, "Invalid NULL resource.");
    if (a1)
        a1->Name =Device.glCreateShader(a0);
}

void GraphicsDeviceImmediate::glDebugMessageCallback(GLDEBUGPROCKHR a0, const void * a1)
{
    Device.glDebugMessageCallback(a0, a1);
}

void GraphicsDeviceImmediate::glDebugMessageControl(GLenum a0, GLenum a1, GLenum a2, GLsizei a3, const GLuint * a4, GLboolean a5)
{
    Device.glDebugMessageControl(a0, a1, a2, a3, a4, a5);
}

void GraphicsDeviceImmediate::glDeleteBuffers(GLsizei  a0, HALGLBuffer** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteBuffers(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDeleteFramebuffers(GLsizei  a0, HALGLFramebuffer** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteFramebuffers(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDeleteProgram(HALGLProgram* a0)
{
    Device.glDeleteProgram(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glDeleteProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteProgramPipelines(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDeleteRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteRenderbuffers(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDeleteShader(HALGLShader* a0)
{
    Device.glDeleteShader(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glDeleteSync(GLsync  a0)
{
    Device.glDeleteSync(a0);
}

void GraphicsDeviceImmediate::glDeleteTextures(GLsizei  a0, HALGLTexture** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteTextures(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDeleteVertexArrays(GLsizei  a0, HALGLVertexArray** a1)
{
    GLuint deleteResources[MaximumGLResourcesPerCall];
    for (int resource = 0; resource < a0; ++resource)
    {
        deleteResources[resource] = a1[resource]->Name;
        a1[resource]->Name = 0;
    }
    Device.glDeleteVertexArrays(a0, deleteResources);
}

void GraphicsDeviceImmediate::glDepthFunc(GLenum  a0)
{
    Device.glDepthFunc(a0);
}

void GraphicsDeviceImmediate::glDepthMask(GLboolean  a0)
{
    Device.glDepthMask(a0);
}

void GraphicsDeviceImmediate::glDisable(GLenum  a0)
{
    Device.glDisable(a0);
}

void GraphicsDeviceImmediate::glDisableVertexAttribArray(GLuint  a0)
{
    Device.glDisableVertexAttribArray(a0);
}

void GraphicsDeviceImmediate::glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2)
{
    Device.glDrawArrays(a0, a1, a2);
}

void GraphicsDeviceImmediate::glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3)
{
    Device.glDrawElements(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4)
{
    Device.glDrawElementsInstanced(a0, a1, a2, a3, a4);
}

void GraphicsDeviceImmediate::glEnable(GLenum  a0)
{
    Device.glEnable(a0);
}

void GraphicsDeviceImmediate::glEnableVertexAttribArray(GLuint  a0)
{
    Device.glEnableVertexAttribArray(a0);
}

GLsync GraphicsDeviceImmediate::glFenceSync(GLenum  a0, GLbitfield  a1)
{
    return Device.glFenceSync(a0, a1);
}

void GraphicsDeviceImmediate::glFlush()
{
    Device.glFlush();
}

void GraphicsDeviceImmediate::glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2)
{
    Device.glFlushMappedBufferRange(a0, a1, a2);
}

void GraphicsDeviceImmediate::glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, HALGLRenderbuffer* a3)
{
    Ptr<HALGLFramebuffer> currentResource;
    if (!BindingIndexerFramebuffers.Get(a0, &currentResource))
        return;

    HALGLFramebuffer::FramebufferAttachment entry;
    if (a3)
    {
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, GL_RENDERBUFFER);
        entry.RenderBuffer  = a3;
        currentResource->BindingIndexerAttachments.Set(a1, entry);
    }
    else
    {
        currentResource->BindingIndexerAttachments.Remove(a1);
    }
    Device.glFramebufferRenderbuffer(a0, a1, a2, a3 ? a3->Name : 0);
}

void GraphicsDeviceImmediate::glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, HALGLTexture* a3, GLint  a4)
{
    Ptr<HALGLFramebuffer> currentResource;
    if (!BindingIndexerFramebuffers.Get(a0, &currentResource))
        return;

    HALGLFramebuffer::FramebufferAttachment entry;
    if (a3)
    {
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,GL_TEXTURE);
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,a4);
        entry.Texture       = a3;
        currentResource->BindingIndexerAttachments.Set(a1, entry);
    }
    else
    {
        currentResource->BindingIndexerAttachments.Remove(a1);        
    }
    Device.glFramebufferTexture2D(a0, a1, a2, a3 ? a3->Name : 0, a4);
}

void GraphicsDeviceImmediate::glGenBuffers(GLsizei  a0, HALGLBuffer** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenBuffers(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenFramebuffers(GLsizei  a0, HALGLFramebuffer** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenFramebuffers(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenProgramPipelines(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenRenderbuffers(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenTextures(GLsizei  a0, HALGLTexture** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenTextures(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenVertexArrays(GLsizei  a0, HALGLVertexArray** a1)
{
    GLuint resourceNames[MaximumGLResourcesPerCall];
    SF_DEBUG_ASSERT2(a0 < MaximumGLResourcesPerCall, "Exceeded maximum number of expected resources in a single call (%d, max = %d)", a0, MaximumGLResourcesPerCall);
    Device.glGenVertexArrays(a0, resourceNames);
    for (int resource = 0; resource < a0; ++resource)
    {
        SF_DEBUG_ASSERT(a1[resource], "Invalid NULL resource.");
        a1[resource]->Name = resourceNames[resource];
    }
}

void GraphicsDeviceImmediate::glGenerateMipmap(GLenum  a0)
{
    Device.glGenerateMipmap(a0);
}

void GraphicsDeviceImmediate::glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6)
{
    Device.glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
}

GLenum GraphicsDeviceImmediate::glGetError()
{
    return Device.glGetError();
}

void GraphicsDeviceImmediate::glGetFloatv(GLenum  a0, GLfloat * a1)
{
    Device.glGetFloatv(a0, a1);
}

GLint GraphicsDeviceImmediate::glGetFragDataLocation(HALGLProgram* a0, const GLchar * a1)
{
    return Device.glGetFragDataLocation(a0 ? a0->Name : 0, a1);
}

void GraphicsDeviceImmediate::glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3)
{
    Device.glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glGetIntegerv(GLenum  a0, GLint * a1)
{
    Device.glGetIntegerv(a0, a1);
}

void GraphicsDeviceImmediate::glGetProgramBinary(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4)
{
    Device.glGetProgramBinary(a0 ? a0->Name : 0, a1, a2, a3, a4);
}

void GraphicsDeviceImmediate::glGetProgramInfoLog(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    Device.glGetProgramInfoLog(a0 ? a0->Name : 0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glGetProgramiv(HALGLProgram* a0, GLenum  a1, GLint * a2)
{
    Device.glGetProgramiv(a0 ? a0->Name : 0, a1, a2);
}

void GraphicsDeviceImmediate::glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    Device.glGetRenderbufferParameteriv(a0, a1, a2);
}

void GraphicsDeviceImmediate::glGetShaderInfoLog(HALGLShader* a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    Device.glGetShaderInfoLog(a0 ? a0->Name : 0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glGetShaderiv(HALGLShader* a0, GLenum  a1, GLint * a2)
{
    Device.glGetShaderiv(a0 ? a0->Name : 0, a1, a2);
}

const GLubyte * GraphicsDeviceImmediate::glGetString(GLenum  a0)
{
    return Device.glGetString(a0);
}

const GLubyte * GraphicsDeviceImmediate::glGetStringi(GLenum  a0, GLuint  a1)
{
    return Device.glGetStringi(a0, a1);
}
void GraphicsDeviceImmediate::glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4)
{
    Device.glGetSynciv(a0, a1, a2, a3, a4);
}

void GraphicsDeviceImmediate::glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4)
{
    Device.glGetTexImage(a0, a1, a2, a3, a4);
}

void GraphicsDeviceImmediate::glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3)
{
    Device.glGetTexLevelParameteriv(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    Device.glGetTexParameteriv(a0, a1, a2);
}

void GraphicsDeviceImmediate::glGetUniformLocation(HALGLProgram* a0, const GLchar * a1, HALGLUniformLocation* a2)
{
    SF_DEBUG_ASSERT(a2 != 0, "Invalid NULL resource.");
    if (a2 == 0)
        return;

    a2->Location = Device.glGetUniformLocation(a0 ? a0->Name : 0, a1);

    // NOTE: Shaders could store uniform names with '[0]' appended on them. So, if we fail to find the uniform with the original name, search for that.
    if (a2->Location < 0)
    {
        char arrayname[128];
        SFstrcpy(arrayname, 128, a1);
        SFstrcat(arrayname, 128, "[0]");
        a2->Location = Device.glGetUniformLocation(a0 ? a0->Name : 0, arrayname);
    }
    SF_DEBUG_WARNING2(a2->Location < 0, "Could not find uniform location for %s. Shader program = %d\n", a1, a0 ? a0->Name : 0);
}

GLboolean GraphicsDeviceImmediate::glIsFramebuffer(HALGLFramebuffer* a0)
{
    return Device.glIsFramebuffer(a0 ? a0->Name : 0);
}

GLboolean GraphicsDeviceImmediate::glIsProgram(HALGLProgram* a0)
{
    return Device.glIsProgram(a0 ? a0->Name : 0);
}

GLboolean GraphicsDeviceImmediate::glIsRenderbuffer(HALGLRenderbuffer* a0)
{
    return Device.glIsRenderbuffer(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glLinkProgram(HALGLProgram* a0)
{
    Device.glLinkProgram(a0 ? a0->Name : 0);
}

void * GraphicsDeviceImmediate::glMapBuffer(GLenum  a0, GLenum  a1)
{
    return Device.glMapBuffer(a0, a1);
}

void * GraphicsDeviceImmediate::glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3)
{
    return Device.glMapBufferRange(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glPixelStorei(GLenum  a0, GLint  a1)
{
    Device.glPixelStorei(a0, a1);
}

void GraphicsDeviceImmediate::glPolygonMode(GLenum  a0, GLenum  a1)
{
    Device.glPolygonMode(a0, a1);
}

void GraphicsDeviceImmediate::glPopGroupMarker()
{
    Device.glPopGroupMarker();
}

void GraphicsDeviceImmediate::glProgramBinary(HALGLProgram* a0, GLenum  a1, const void * a2, GLint  a3)
{
    Device.glProgramBinary(a0 ? a0->Name : 0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramParameteri(HALGLProgram* a0, GLenum  a1, GLint  a2)
{
    Device.glProgramParameteri(a0 ? a0->Name : 0, a1, a2);
}

void GraphicsDeviceImmediate::glProgramUniform1fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    Device.glProgramUniform1fv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramUniform1iv(HALGLProgram*   a0, HALGLUniformLocation* a1, GLsizei  a2, const GLint * a3)
{
    Device.glProgramUniform1iv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramUniform2fv(HALGLProgram*   a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    Device.glProgramUniform2fv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramUniform3fv(HALGLProgram*   a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    Device.glProgramUniform3fv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramUniform4fv(HALGLProgram*   a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    Device.glProgramUniform4fv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3);
}

void GraphicsDeviceImmediate::glProgramUniformMatrix4fv(HALGLProgram*   a0, HALGLUniformLocation* a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4)
{
    Device.glProgramUniformMatrix4fv(a0 ? a0->Name : 0, a1 ? a1->Location : -1, a2, a3, a4);
}

void GraphicsDeviceImmediate::glPushGroupMarker(GLsizei  a0, const GLchar * a1)
{
    Device.glPushGroupMarker(a0, a1);
}

void GraphicsDeviceImmediate::glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6)
{
    Device.glReadPixels(a0, a1, a2, a3, a4, a5, a6);
}

void GraphicsDeviceImmediate::glReleaseShaderCompiler()
{
    Device.glReleaseShaderCompiler();
}

void GraphicsDeviceImmediate::glRenderbufferStorage(GLenum a0, GLenum  a1, GLsizei  a2, GLsizei  a3)
{
    Device.glRenderbufferStorage(a0, a1, a2, a3);

    // Record the parameters. Note, this is not a full set of possible parameters, only the ones that Scaleform
    // will query internally.
    Ptr<HALGLRenderbuffer> currentResource;
    if (BindingIndexerRenderbuffers.Get(a0, &currentResource) && currentResource)
    {
        currentResource->Parameters.Set(GL_RENDERBUFFER_WIDTH,  a2);
        currentResource->Parameters.Set(GL_RENDERBUFFER_HEIGHT, a3);
        switch(a1)
        {
        case GL_DEPTH24_STENCIL8:
            currentResource->Parameters.Set(GL_RENDERBUFFER_DEPTH_SIZE, 24);
            currentResource->Parameters.Set(GL_RENDERBUFFER_STENCIL_SIZE, 8);
            break;

        case GL_STENCIL_INDEX8:
#if !defined(SF_OS_IPHONE) // iOS SDK 6.0 removed GL_STENCIL_INDEX.
        case GL_STENCIL_INDEX:
#endif
            currentResource->Parameters.Set(GL_RENDERBUFFER_STENCIL_SIZE, 8);
            break;

        default:
            SF_DEBUG_ASSERT1(0, "Unhandled Renderbuffer format (0x%x)", a1);
            break;
        }
    }    
}

void GraphicsDeviceImmediate::glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    Device.glScissor(a0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glShaderSource(HALGLShader* a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3)
{
    Device.glShaderSource(a0 ? a0->Name : 0, a1, a2, a3);
}

void GraphicsDeviceImmediate::glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2)
{
    Device.glStencilFunc(a0, a1, a2);
}

void GraphicsDeviceImmediate::glStencilMask(GLuint  a0)
{
    Device.glStencilMask(a0);
}

void GraphicsDeviceImmediate::glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2)
{
    Device.glStencilOp(a0, a1, a2);
}

void GraphicsDeviceImmediate::glStringMarker(GLsizei  a0, const void * a1)
{
    Device.glStringMarker(a0, a1);
}

void GraphicsDeviceImmediate::glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    Device.glTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

void GraphicsDeviceImmediate::glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2)
{
    Device.glTexParameteri(a0, a1, a2);
}

void GraphicsDeviceImmediate::glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    Device.glTexSubImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

void GraphicsDeviceImmediate::glUniform1f(HALGLUniformLocation* a0, GLfloat  a1)
{
    Device.glUniform1f(a0 ? a0->Location : 0, a1);
}

void GraphicsDeviceImmediate::glUniform1fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    Device.glUniform1fv(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniform1i(HALGLUniformLocation* a0, GLint  a1)
{
    Device.glUniform1i(a0 ? a0->Location : 0, a1);
}

void GraphicsDeviceImmediate::glUniform1iv(HALGLUniformLocation* a0, GLsizei  a1, const GLint * a2)
{
    Device.glUniform1iv(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniform2f(HALGLUniformLocation* a0, GLfloat  a1, GLfloat  a2)
{
    Device.glUniform2f(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniform2fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    Device.glUniform2fv(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniform3fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    Device.glUniform3fv(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniform4fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    Device.glUniform4fv(a0 ? a0->Location : 0, a1, a2);
}

void GraphicsDeviceImmediate::glUniformMatrix4fv(HALGLUniformLocation* a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3)
{
    Device.glUniformMatrix4fv(a0 ? a0->Location : 0, a1, a2, a3);
}

GLboolean GraphicsDeviceImmediate::glUnmapBuffer(GLenum  a0)
{
    return Device.glUnmapBuffer(a0);
}

void GraphicsDeviceImmediate::glUseProgram(HALGLProgram* a0)
{
    Device.glUseProgram(a0 ? a0->Name : 0);
}

void GraphicsDeviceImmediate::glUseProgramStages(HALGLProgramPipeline* a0, GLbitfield  a1, HALGLProgram* a2)
{
    Device.glUseProgramStages(a0 ? a0->Name : 0, a1, a2 ? a2->Name : 0);
}

void GraphicsDeviceImmediate::glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5)
{
    Device.glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
}

void GraphicsDeviceImmediate::glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    Device.glViewport(a0, a1, a2, a3);
}

GraphicsDeviceRecorder::GraphicsDeviceRecorder(GraphicsDeviceImmediate& immediateDevice) :
    GraphicsDevice(immediateDevice),
    Render::GraphicsDeviceRecorder()
{

}

bool GraphicsDeviceRecorder::ExecuteRecording(Render::GraphicsDevice& playbackDevice)
{
    // Reset the cached states on the playback device.
    playbackDevice.Begin();

    GL::GraphicsDevice& device = reinterpret_cast<GL::GraphicsDevice&>(playbackDevice);
    UInt32 commandName = *(UInt32*)CommandReadPointer; // 'peek' the first command, it might be FinishedDraw on double-Submit.
    if (commandName == FinishedDraw)
        return false;

    while (commandName != FinishedDraw)
    {
        read<UInt32>(commandName);
        executeCommand(commandName, device);
    }

    // Call the base class, to reset everything.
    bool baseReturn = Render::GraphicsDeviceRecorder::ExecuteRecording(playbackDevice);

    clearCachedBindings();
    return baseReturn;
}

void GraphicsDeviceRecorder::End()
{
    write<UInt32>(FinishedDraw);
}

void GraphicsDeviceRecorder::glActiveTexture(GLenum  a0)
{
    write<UInt32>(HAL_glActiveTexture);
    write(a0);
    BindingActiveTexture = a0;
    SF_DEBUG_ASSERT2(a0-GL_TEXTURE0 < FragShaderDesc::MaxTextureSamplers+1, 
        "Unexpected input to glActiveTexture (%d). Expected less than (%d)", a0-GL_TEXTURE0, FragShaderDesc::MaxTextureSamplers+1);
}
void GraphicsDeviceRecorder::glAttachShader(HALGLProgram* a0, HALGLShader* a1)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);

    write<UInt32>(HAL_glAttachShader);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBindAttribLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2)
{
    AddRefNullCheck(a0);

    write<UInt32>(HAL_glBindAttribLocation);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glBindBuffer(GLenum  a0, HALGLBuffer* a1)
{
    AddRefNullCheck(a1);
    if ((a0 != GL_ARRAY_BUFFER && a0 != GL_ELEMENT_ARRAY_BUFFER) || BindingVertexArray == 0)
        BindingIndexerBuffers.Set(a0, a1);
    else if (BindingVertexArray != 0)
        BindingVertexArray->BindingIndexerBuffers.Set(a0, a1);
    write<UInt32>(HAL_glBindBuffer);
    write(a0);
    write(a1);

}
 void GraphicsDeviceRecorder::glBindFragDataLocation(HALGLProgram* a0, GLuint  a1, const GLchar * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glBindFragDataLocation);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glBindFramebuffer(GLenum  a0, HALGLFramebuffer* a1)
{
    AddRefNullCheck(a1);
    BindingIndexerFramebuffers.Set(a0, a1);

    write<UInt32>(HAL_glBindFramebuffer);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBindProgramPipeline(HALGLProgramPipeline* a0)
{
    AddRefNullCheck(a0);
    BindingProgramPipeline = a0;

    write<UInt32>(HAL_glBindProgramPipeline);
    write(a0);

}
void GraphicsDeviceRecorder::glBindRenderbuffer(GLenum  a0, HALGLRenderbuffer* a1)
{
    AddRefNullCheck(a1);
    BindingIndexerRenderbuffers.Set(a0, a1);

    write<UInt32>(HAL_glBindRenderbuffer);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBindTexture(GLenum  a0, HALGLTexture* a1)
{
    AddRefNullCheck(a1);
    BindingIndexerTextures[BindingActiveTexture-GL_TEXTURE0].Set(a0, a1);

    write<UInt32>(HAL_glBindTexture);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBindVertexArray(HALGLVertexArray* a0)
{
    AddRefNullCheck(a0);
    BindingVertexArray = a0;

    write<UInt32>(HAL_glBindVertexArray);
    write(a0);

}
void GraphicsDeviceRecorder::glBlendEquation(GLenum  a0)
{
    write<UInt32>(HAL_glBlendEquation);
    write(a0);

}
void GraphicsDeviceRecorder::glBlendEquationSeparate(GLenum  a0, GLenum  a1)
{
    write<UInt32>(HAL_glBlendEquationSeparate);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBlendFunc(GLenum  a0, GLenum  a1)
{
    write<UInt32>(HAL_glBlendFunc);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3)
{
    write<UInt32>(HAL_glBlendFuncSeparate);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3)
{
    write<UInt32>(HAL_glBufferData);
    write(a0);
    write(a1);
    if (a2 != 0)
    {
        UByte* data = alloc(a1);
        memcpy(data, a2, a1);
        write(data);
    }
    else
    {
        write(0);
    }
    write(a3);

    Ptr<HALGLBuffer> currentBuffer;
    if (BindingIndexerBuffers.Get(a0, &currentBuffer) && currentBuffer)
    {
        currentBuffer->Size = a1;
        currentBuffer->Usage = a3;
    }

}
void GraphicsDeviceRecorder::glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3)
{
    write<UInt32>(HAL_glBufferSubData);
    write(a0);
    write(a1);
    write(a2);
    UByte* data = alloc(a2);
    memcpy(data, a3, a2);
    write(data);

}
GLenum GraphicsDeviceRecorder::glCheckFramebufferStatus(GLenum  a0)
{
    write<UInt32>(HAL_glCheckFramebufferStatus);
    write(a0);

    // NOTE: an assumption is being made that the framebuffer will be complete. The HAL should create a
    // temporary render target, to find an appropriate color/DS format that succeeds in the current environment.
    // When this command is played back, it will verify this is the case.
    return GL_FRAMEBUFFER_COMPLETE;
}
void GraphicsDeviceRecorder::glClear(GLbitfield  a0)
{
    write<UInt32>(HAL_glClear);
    write(a0);

}
void GraphicsDeviceRecorder::glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3)
{
    write<UInt32>(HAL_glClearColor);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
GLenum GraphicsDeviceRecorder::glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2)
{
    write<UInt32>(HAL_glClientWaitSync);
    write(a0);
    write(a1);
    write(a2);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void GraphicsDeviceRecorder::glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3)
{
    write<UInt32>(HAL_glColorMask);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glCompileShader(HALGLShader* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glCompileShader);
    write(a0);

}
void GraphicsDeviceRecorder::glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7)
{
    write<UInt32>(HAL_glCompressedTexImage2D);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
    write(a6);
    UByte* data = alloc(a6);
    memcpy(data, a7, a6);
    write(data);
}
void GraphicsDeviceRecorder::glCreateProgram(HALGLProgram* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glCreateProgram);
    write(a0);
}
void GraphicsDeviceRecorder::glCreateShader(GLenum  a0, HALGLShader* a1)
{
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glCreateShader);
    write(a0);
    write(a1);
}
void GraphicsDeviceRecorder::glDebugMessageCallback(GLDEBUGPROCKHR a0, const void * a1)
{
    write<UInt32>(HAL_glDebugMessageCallback);
    write(a0);
    write(a1);
}
void GraphicsDeviceRecorder::glDebugMessageControl(GLenum a0, GLenum a1, GLenum a2, GLsizei a3, const GLuint * a4, GLboolean a5)
{
    write<UInt32>(HAL_glDebugMessageControl);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
}
void GraphicsDeviceRecorder::glDeleteBuffers(GLsizei  a0, HALGLBuffer** a1)
{
    write<UInt32>(HAL_glDeleteBuffers);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDeleteFramebuffers(GLsizei  a0, HALGLFramebuffer** a1)
{
    write<UInt32>(HAL_glDeleteFramebuffers);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDeleteProgram(HALGLProgram* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glDeleteProgram);
    write(a0);
}
void GraphicsDeviceRecorder::glDeleteProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1)
{
    write<UInt32>(HAL_glDeleteProgramPipelines);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDeleteRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1)
{
    write<UInt32>(HAL_glDeleteRenderbuffers);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDeleteShader(HALGLShader* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glDeleteShader);
    write(a0);

}
void GraphicsDeviceRecorder::glDeleteSync(GLsync  a0)
{
    write<UInt32>(HAL_glDeleteSync);
    write(a0);

}
void GraphicsDeviceRecorder::glDeleteTextures(GLsizei  a0, HALGLTexture** a1)
{
    write<UInt32>(HAL_glDeleteTextures);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDeleteVertexArrays(GLsizei  a0, HALGLVertexArray** a1)
{
    write<UInt32>(HAL_glDeleteVertexArrays);
    write(a0);
    for (int resource = 0; resource < a0; ++resource)
    {
        AddRefNullCheck(a1[resource]);
        write(a1[resource]);
    }
}
void GraphicsDeviceRecorder::glDepthFunc(GLenum  a0)
{
    write<UInt32>(HAL_glDepthFunc);
    write(a0);

}
void GraphicsDeviceRecorder::glDepthMask(GLboolean  a0)
{
    write<UInt32>(HAL_glDepthMask);
    write(a0);

}
void GraphicsDeviceRecorder::glDisable(GLenum  a0)
{
    write<UInt32>(HAL_glDisable);
    write(a0);

}
void GraphicsDeviceRecorder::glDisableVertexAttribArray(GLuint  a0)
{
    write<UInt32>(HAL_glDisableVertexAttribArray);
    write(a0);

}
void GraphicsDeviceRecorder::glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2)
{
    write<UInt32>(HAL_glDrawArrays);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3)
{
    write<UInt32>(HAL_glDrawElements);
    write(a0);
    write(a1);
    write(a2);
    write(a3); // TODOBM: need to record MeshCache's buffer method. If using client buffers, we would need to copy the data here.
}
void GraphicsDeviceRecorder::glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4)
{
    write<UInt32>(HAL_glDrawElementsInstanced);
    write(a0);
    write(a1);
    write(a2);
    write(a3); // TODOBM: need to record MeshCache's buffer method. If using client buffers, we would need to copy the data here.
    write(a4);

}
void GraphicsDeviceRecorder::glEnable(GLenum  a0)
{
    write<UInt32>(HAL_glEnable);
    write(a0);

}
void GraphicsDeviceRecorder::glEnableVertexAttribArray(GLuint  a0)
{
    write<UInt32>(HAL_glEnableVertexAttribArray);
    write(a0);

}
GLsync GraphicsDeviceRecorder::glFenceSync(GLenum  a0, GLbitfield  a1)
{
    write<UInt32>(HAL_glFenceSync);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void GraphicsDeviceRecorder::glFlush()
{
    write<UInt32>(HAL_glFlush);
}
void GraphicsDeviceRecorder::glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2)
{
    write<UInt32>(HAL_glFlushMappedBufferRange);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, HALGLRenderbuffer* a3)
{
    AddRefNullCheck(a3);
    write<UInt32>(HAL_glFramebufferRenderbuffer);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

    Ptr<HALGLFramebuffer> currentResource;
    if (!BindingIndexerFramebuffers.Get(a0, &currentResource))
        return;

    HALGLFramebuffer::FramebufferAttachment entry;
    if (a3)
    {
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, GL_RENDERBUFFER);
        entry.RenderBuffer  = a3;
        currentResource->BindingIndexerAttachments.Set(a1, entry);
    }
    else
    {
        currentResource->BindingIndexerAttachments.Remove(a1);
    }
}
void GraphicsDeviceRecorder::glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, HALGLTexture* a3, GLint  a4)
{
    AddRefNullCheck(a3);
    write<UInt32>(HAL_glFramebufferTexture2D);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);

    Ptr<HALGLFramebuffer> currentResource;
    if (!BindingIndexerFramebuffers.Get(a0, &currentResource))
        return;

    HALGLFramebuffer::FramebufferAttachment entry;
    if (a3)
    {
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,GL_TEXTURE);
        entry.Parameters.Set(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,a4);
        entry.Texture       = a3;
        currentResource->BindingIndexerAttachments.Set(a1, entry);
    }
    else
    {
        currentResource->BindingIndexerAttachments.Remove(a1);        
    }
}
void GraphicsDeviceRecorder::glGenBuffers(GLsizei  a0, HALGLBuffer** a1)
{
    write<UInt32>(HAL_glGenBuffers);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        a1[i]->AddRef();
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenFramebuffers(GLsizei  a0, HALGLFramebuffer** a1)
{
    write<UInt32>(HAL_glGenFramebuffers);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenProgramPipelines(GLsizei  a0, HALGLProgramPipeline** a1)
{
    write<UInt32>(HAL_glGenProgramPipelines);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenRenderbuffers(GLsizei  a0, HALGLRenderbuffer** a1)
{
    write<UInt32>(HAL_glGenRenderbuffers);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenTextures(GLsizei  a0, HALGLTexture** a1)
{
    write<UInt32>(HAL_glGenTextures);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenVertexArrays(GLsizei  a0, HALGLVertexArray** a1)
{
    write<UInt32>(HAL_glGenVertexArrays);
    write(a0);
    for (int i = 0; i < a0; i++)
    {
        AddRefNullCheck(a1[i]);
        write(a1[i]);
    }
}
void GraphicsDeviceRecorder::glGenerateMipmap(GLenum  a0)
{
    write<UInt32>(HAL_glGenerateMipmap);
    write(a0);

}
void GraphicsDeviceRecorder::glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6)
{
    write<UInt32>(HAL_glGetActiveUniform);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
    write(a6);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
GLenum GraphicsDeviceRecorder::glGetError()
{
    write<UInt32>(HAL_glGetError);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void GraphicsDeviceRecorder::glGetFloatv(GLenum  a0, GLfloat * a1)
{
    write<UInt32>(HAL_glGetFloatv);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
GLint GraphicsDeviceRecorder::glGetFragDataLocation(HALGLProgram* a0, const GLchar * a1)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetFragDataLocation);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void GraphicsDeviceRecorder::glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3)
{
    Ptr<HALGLFramebuffer> currentResource;
    if (BindingIndexerFramebuffers.Get(a0, &currentResource) && currentResource)
    {
        HALGLFramebuffer::FramebufferAttachment attachment;
        if (currentResource->BindingIndexerAttachments.Get(a1, &attachment))
        {
            if (!attachment.Parameters.Get(a2, a3))
            {
                SF_DEBUG_ASSERT1(0, "Could not get Framebuffer parameter 0x%x", a2);
            }
        }
    }

    // Do not need to actually execute this.
    //write<UInt32>(HAL_glGetFramebufferAttachmentParameteriv);
    //write(a0);
    //write(a1);
    //write(a2);
    //write(a3);
}
void GraphicsDeviceRecorder::glGetIntegerv(GLenum  a0, GLint * a1)
{
    write<UInt32>(HAL_glGetIntegerv);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetProgramBinary(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetProgramBinary);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetProgramInfoLog(HALGLProgram* a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetProgramInfoLog);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetProgramiv(HALGLProgram* a0, GLenum  a1, GLint * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetProgramiv);
    write(a0);
    write(a1);
    write(a2);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    Ptr<HALGLRenderbuffer> currentResource;
    if (BindingIndexerRenderbuffers.Get(a0, &currentResource) && currentResource)
    {
        if (!currentResource->Parameters.Get(a1, a2))
        {
            SF_DEBUG_ASSERT1(0, "Could not get Renderbuffer parameter 0x%x", a1);
        }
    }

    // Do not need to actually execute this.
    //write<UInt32>(HAL_glGetRenderbufferParameteriv);
    //write(a0);
    //write(a1);
    //write(a2);
}
void GraphicsDeviceRecorder::glGetShaderInfoLog(HALGLShader* a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetShaderInfoLog);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetShaderiv(HALGLShader* a0, GLenum  a1, GLint * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glGetShaderiv);
    write(a0);
    write(a1);
    write(a2);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
const GLubyte * GraphicsDeviceRecorder::glGetString(GLenum  a0)
{
    write<UInt32>(HAL_glGetString);
    write(a0);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
const GLubyte * GraphicsDeviceRecorder::glGetStringi(GLenum  a0, GLuint  a1)
{
    write<UInt32>(HAL_glGetStringi);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void GraphicsDeviceRecorder::glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4)
{
    write<UInt32>(HAL_glGetSynciv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4)
{
    write<UInt32>(HAL_glGetTexImage);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3)
{
    write<UInt32>(HAL_glGetTexLevelParameteriv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    write<UInt32>(HAL_glGetTexParameteriv);
    write(a0);
    write(a1);
    write(a2);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
}
void GraphicsDeviceRecorder::glGetUniformLocation(HALGLProgram* a0, const GLchar * a1, HALGLUniformLocation* a2)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a2);
    write<UInt32>(HAL_glGetUniformLocation);
    write(a0);
    write(a1);
    write(a2);
}
GLboolean GraphicsDeviceRecorder::glIsFramebuffer(HALGLFramebuffer* a0)
{
    write<UInt32>(HAL_glIsFramebuffer);
    write(a0);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return false;
}
GLboolean GraphicsDeviceRecorder::glIsProgram(HALGLProgram* a0)
{
    write<UInt32>(HAL_glIsProgram);
    write(a0);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return false;
}
GLboolean GraphicsDeviceRecorder::glIsRenderbuffer(HALGLRenderbuffer* a0)
{
    write<UInt32>(HAL_glIsRenderbuffer);
    write(a0);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return false;
}
void GraphicsDeviceRecorder::glLinkProgram(HALGLProgram* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glLinkProgram);
    write(a0);

}
void * GraphicsDeviceRecorder::glMapBuffer(GLenum  a0, GLenum  a1)
{
    write<UInt32>(HAL_glMapBuffer);
    write(a0);
    write(a1);
    SF_DEBUG_ASSERT(0, "This function should not be recorded.");
    return 0;
}
void * GraphicsDeviceRecorder::glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3)
{
    write<UInt32>(HAL_glMapBufferRange);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    void* data = alloc(a2);
    write(data);
    return data;
}
void GraphicsDeviceRecorder::glPixelStorei(GLenum  a0, GLint  a1)
{
    write<UInt32>(HAL_glPixelStorei);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glPolygonMode(GLenum  a0, GLenum  a1)
{
    write<UInt32>(HAL_glPolygonMode);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glPopGroupMarker()
{
    write<UInt32>(HAL_glPopGroupMarker);

}
void GraphicsDeviceRecorder::glProgramBinary(HALGLProgram* a0, GLenum  a1, const void * a2, GLint  a3)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glProgramBinary);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramParameteri(HALGLProgram* a0, GLenum  a1, GLint  a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glProgramParameteri);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glProgramUniform1fv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniform1fv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramUniform1iv(HALGLProgram* a0, HALGLUniformLocation* a1, GLsizei  a2, const GLint * a3)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniform1iv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramUniform2fv(HALGLProgram*  a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniform2fv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramUniform3fv(HALGLProgram*  a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniform3fv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramUniform4fv(HALGLProgram*  a0, HALGLUniformLocation* a1, GLsizei  a2, const GLfloat * a3)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniform4fv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glProgramUniformMatrix4fv(HALGLProgram*  a0, HALGLUniformLocation* a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a1);
    write<UInt32>(HAL_glProgramUniformMatrix4fv);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);

}
void GraphicsDeviceRecorder::glPushGroupMarker(GLsizei  a0, const GLchar * a1)
{
    write<UInt32>(HAL_glPushGroupMarker);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6)
{
    write<UInt32>(HAL_glReadPixels);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
    write(a6);
}
void GraphicsDeviceRecorder::glReleaseShaderCompiler()
{
    write<UInt32>(HAL_glReleaseShaderCompiler);

}
void GraphicsDeviceRecorder::glRenderbufferStorage(GLenum a0, GLenum  a1, GLsizei  a2, GLsizei  a3)
{
    write<UInt32>(HAL_glRenderbufferStorage);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

    // Record the parameters. Note, this is not a full set of possible parameters, only the ones that Scaleform
    // will query internally.
    Ptr<HALGLRenderbuffer> currentResource;
    if (BindingIndexerRenderbuffers.Get(a0, &currentResource) && currentResource)
    {
        currentResource->Parameters.Set(GL_RENDERBUFFER_WIDTH,  a2);
        currentResource->Parameters.Set(GL_RENDERBUFFER_HEIGHT, a3);
        switch(a1)
        {
        case GL_DEPTH24_STENCIL8:
            currentResource->Parameters.Set(GL_RENDERBUFFER_DEPTH_SIZE, 24);
            currentResource->Parameters.Set(GL_RENDERBUFFER_STENCIL_SIZE, 8);
            break;

        case GL_STENCIL_INDEX8:
#if !defined(SF_OS_IPHONE) // iOS SDK 6.0 removed GL_STENCIL_INDEX.
        case GL_STENCIL_INDEX:
#endif
            currentResource->Parameters.Set(GL_RENDERBUFFER_STENCIL_SIZE, 8);
            break;

        default:
            SF_DEBUG_ASSERT1(0, "Unhandled Renderbuffer format (0%x)", a1);
            break;
        }
    }    
}
void GraphicsDeviceRecorder::glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    write<UInt32>(HAL_glScissor);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::glShaderSource(HALGLShader* a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glShaderSource);
    write(a0);
    write(a1);
    GLchar** string = (GLchar**)alloc(sizeof(GLchar*) * a1);
    for (int i = 0; i < a1; ++i)
    {
        if (a3 != 0)
        {
            string[i] = (GLchar*)alloc((a3[i]+1) * sizeof(GLchar));
            SFstrncpy(string[i], (a3[i]+1), a2[i], a3[i]);
            string[i][a3[i]] = 0;
        }
        else
        {
            UPInt size = (SFstrlen(a2[i])+1);
            string[i] = (GLchar*)alloc(size*sizeof(GLchar));
            SFstrcpy(string[i], size, a2[i]);
        }
    }
    write(string);
    write(0); // length is always ignored, as the strings are null terminated.

}
void GraphicsDeviceRecorder::glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2)
{
    write<UInt32>(HAL_glStencilFunc);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glStencilMask(GLuint  a0)
{
    write<UInt32>(HAL_glStencilMask);
    write(a0);

}
void GraphicsDeviceRecorder::glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2)
{
    write<UInt32>(HAL_glStencilOp);
    write(a0);
    write(a1);
    write(a2);

}
 void GraphicsDeviceRecorder::glStringMarker(GLsizei  a0, const void * a1)
{
    write<UInt32>(HAL_glStringMarker);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    write<UInt32>(HAL_glTexImage2D);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
    write(a6);
    write(a7);

    Ptr<HALGLTexture> currentTexture;
    if (BindingIndexerTextures[BindingActiveTexture-GL_TEXTURE0].Get(a0, &currentTexture) && currentTexture && a8 != 0)
    {
        unsigned componentCount = 1;
        switch(a6)
        {
        case GL_RED: 
        case GL_BLUE:
        case GL_GREEN:
        case GL_ALPHA:
#if defined(GL_LUMINANCE)   // Deprecated in GL 3.0.
        case GL_LUMINANCE:
#endif
        case GL_DEPTH_COMPONENT:
            componentCount = 1; 
            break;
        case GL_RG:
#if defined(GL_LUMINANCE_ALPHA)   // Deprecated in GL 3.0.
        case GL_LUMINANCE_ALPHA:
#endif
            componentCount = 2; 
            break;
        case GL_RGB:
        case GL_BGR:
            componentCount = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
            componentCount = 4;
            break;
        default:
            SF_DEBUG_ASSERT1(0, "Unexpected 'format' value in glTexImage2D (0%x)", a6);
            break;
        }

        SF_DEBUG_ASSERT1(a7 == GL_UNSIGNED_BYTE, "Unexpected 'type' value in glTexImage2D (0x%x). Only GL_UNSIGNED_BYTE is supported.", a7);

        UPInt totalSize = a3*a4*componentCount;
        UByte* data = alloc(totalSize);
        memcpy(data, a8, totalSize);
        write(data);
    }
    else
    {
        write(0);
    }

}
void GraphicsDeviceRecorder::glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2)
{
    write<UInt32>(HAL_glTexParameteri);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    write<UInt32>(HAL_glTexSubImage2D);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);
    write(a6);
    write(a7);
    write(a8);

}
void GraphicsDeviceRecorder::glUniform1f(HALGLUniformLocation* a0, GLfloat  a1)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform1f);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glUniform1fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform1fv);
    write(a0);
    write(a1);
    UByte* data = alloc(a1 * sizeof(GLfloat));
    memcpy(data, a2, a1 * sizeof(GLfloat));
    write(data);
}
void GraphicsDeviceRecorder::glUniform1i(HALGLUniformLocation* a0, GLint  a1)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform1i);
    write(a0);
    write(a1);

}
void GraphicsDeviceRecorder::glUniform1iv(HALGLUniformLocation* a0, GLsizei  a1, const GLint * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform1iv);
    write(a0);
    write(a1);
    UByte* data = alloc(a1 * sizeof(GLint));
    memcpy(data, a2, a1 * sizeof(GLint));
    write(data);
}
void GraphicsDeviceRecorder::glUniform2f(HALGLUniformLocation* a0, GLfloat  a1, GLfloat  a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform2f);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glUniform2fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform2fv);
    write(a0);
    write(a1);
    UByte* data = alloc(a1 * 2 * sizeof(GLfloat));
    memcpy(data, a2, a1 * 2 * sizeof(GLfloat));
    write(data);
}
void GraphicsDeviceRecorder::glUniform3fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform3fv);
    write(a0);
    write(a1);
    UByte* data = alloc(a1 * 3 * sizeof(GLfloat));
    memcpy(data, a2, a1 * 3 * sizeof(GLfloat));
    write(data);
}
void GraphicsDeviceRecorder::glUniform4fv(HALGLUniformLocation* a0, GLsizei  a1, const GLfloat * a2)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniform4fv);
    write(a0);
    write(a1);
    UByte* data = alloc(a1 * 4 * sizeof(GLfloat));
    memcpy(data, a2, a1 * 4 * sizeof(GLfloat));
    write(data);
}
void GraphicsDeviceRecorder::glUniformMatrix4fv(HALGLUniformLocation* a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUniformMatrix4fv);
    write(a0);
    write(a1);
    write(a2);
    UByte* data = alloc(a1 * 4*4 * sizeof(GLfloat));
    memcpy(data, a3, a1 * 4*4 * sizeof(GLfloat));
    write(data);
}
GLboolean GraphicsDeviceRecorder::glUnmapBuffer(GLenum  a0)
{
    write<UInt32>(HAL_glUnmapBuffer);
    write(a0);
    return true;
}
void GraphicsDeviceRecorder::glUseProgram(HALGLProgram* a0)
{
    AddRefNullCheck(a0);
    write<UInt32>(HAL_glUseProgram);
    write(a0);

}
void GraphicsDeviceRecorder::glUseProgramStages(HALGLProgramPipeline* a0, GLbitfield  a1, HALGLProgram* a2)
{
    AddRefNullCheck(a0);
    AddRefNullCheck(a2);
    BindingProgramPipeline = a0;

    write<UInt32>(HAL_glUseProgramStages);
    write(a0);
    write(a1);
    write(a2);

}
void GraphicsDeviceRecorder::glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5)
{
    write<UInt32>(HAL_glVertexAttribPointer);
    write(a0);
    write(a1);
    write(a2);
    write(a3);
    write(a4);
    write(a5);

}
void GraphicsDeviceRecorder::glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    write<UInt32>(HAL_glViewport);
    write(a0);
    write(a1);
    write(a2);
    write(a3);

}
void GraphicsDeviceRecorder::executeCommand(UInt32 commandName, GL::GraphicsDevice& device)
{
    switch(commandName)
    {
        default:
        {
            SF_DEBUG_ASSERT1(0, "Invalid command in stream: 0x%x", commandName);
            break;
        }
        case FinishedDraw:
        {
            break;
        }
        case HAL_glActiveTexture:
        {
            GLenum  a0;
            read(a0);
            device.glActiveTexture(a0);
            break;
        }
        case HAL_glAttachShader:
        {
            HALGLProgram* a0;HALGLShader* a1;
            read(a0);
            read(a1);
            device.glAttachShader(a0, a1);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glBindAttribLocation:
        {
            HALGLProgram* a0;GLuint  a1;const GLchar * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glBindAttribLocation(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glBindBuffer:
        {
            GLenum  a0;HALGLBuffer* a1;
            read(a0);
            read(a1);
            device.glBindBuffer(a0, a1);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glBindFragDataLocation:
        {
            HALGLProgram* a0;GLuint  a1;const GLchar * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glBindFragDataLocation(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glBindFramebuffer:
        {
            GLenum a0;HALGLFramebuffer* a1;
            read(a0);
            read(a1);
            device.glBindFramebuffer(a0, a1);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glBindProgramPipeline:
        {
            HALGLProgramPipeline* a0;
            read(a0);
            device.glBindProgramPipeline(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glBindRenderbuffer:
        {
            GLenum  a0;HALGLRenderbuffer* a1;
            read(a0);
            read(a1);
            device.glBindRenderbuffer(a0, a1);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glBindTexture:
        {
            GLenum  a0;HALGLTexture* a1;
            read(a0);
            read(a1);
            device.glBindTexture(a0, a1);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glBindVertexArray:
        {
            HALGLVertexArray* a0;
            read(a0);
            device.glBindVertexArray(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glBlendEquation:
        {
            GLenum  a0;
            read(a0);
            device.glBlendEquation(a0);
            break;
        }
        case HAL_glBlendEquationSeparate:
        {
            GLenum  a0;GLenum  a1;
            read(a0);
            read(a1);
            device.glBlendEquationSeparate(a0, a1);
            break;
        }
        case HAL_glBlendFunc:
        {
            GLenum  a0;GLenum  a1;
            read(a0);
            read(a1);
            device.glBlendFunc(a0, a1);
            break;
        }
        case HAL_glBlendFuncSeparate:
        {
            GLenum  a0;GLenum  a1;GLenum  a2;GLenum  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glBlendFuncSeparate(a0, a1, a2, a3);
            break;
        }
        case HAL_glBufferData:
        {
            GLenum  a0;GLsizeiptr  a1;const void * a2;GLenum  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glBufferData(a0, a1, a2, a3);
            free(a2);
            break;
        }
        case HAL_glBufferSubData:
        {
            GLenum  a0;GLintptr  a1;GLsizeiptr  a2;const void * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glBufferSubData(a0, a1, a2, a3);
            free(a3);
            break;
        }
        case HAL_glCheckFramebufferStatus:
        {
            GLenum  a0;
            read(a0);
            GLenum returnValue = device.glCheckFramebufferStatus(a0);
            SF_DEBUG_ASSERT1(returnValue == GL_FRAMEBUFFER_COMPLETE, "Expected glCheckFramebufferStatus to return GL_FRAMEBUFFER_COMPLETE. Instead, it returned 0x%x", returnValue);
            SF_UNUSED(returnValue);
            break;
        }
        case HAL_glClear:
        {
            GLbitfield  a0;
            read(a0);
            device.glClear(a0);
            break;
        }
        case HAL_glClearColor:
        {
            GLclampf  a0;GLclampf  a1;GLclampf  a2;GLclampf  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glClearColor(a0, a1, a2, a3);
            break;
        }
        case HAL_glClientWaitSync:
        {
            GLsync  a0;GLbitfield  a1;GLuint64  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glClientWaitSync(a0, a1, a2);
            break;
        }
        case HAL_glColorMask:
        {
            GLboolean  a0;GLboolean  a1;GLboolean  a2;GLboolean  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glColorMask(a0, a1, a2, a3);
            break;
        }
        case HAL_glCompileShader:
        {
            HALGLShader* a0;
            read(a0);
            device.glCompileShader(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glCompressedTexImage2D:
        {
            GLenum  a0;GLint  a1;GLenum  a2;GLsizei  a3;GLsizei  a4;GLint  a5;GLsizei  a6;const void * a7;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            read(a6);
            read(a7);
            device.glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
            break;
        }
        case HAL_glCreateProgram:
        {
            HALGLProgram* a0;
            read(a0);
            device.glCreateProgram(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glCreateShader:
        {
            GLenum  a0;HALGLShader* a1;
            read(a0);
            read(a1);
            device.glCreateShader(a0, a1);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glDebugMessageCallback:
        {
            GLDEBUGPROCKHR a0;const void * a1;
            read(a0);
            read(a1);
            device.glDebugMessageCallback(a0, a1);
            break;
        }
        case HAL_glDebugMessageControl:
        {
            GLenum a0;GLenum a1;GLenum a2;GLsizei a3;const GLuint * a4;GLboolean a5;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            device.glDebugMessageControl(a0, a1, a2, a3, a4, a5);
            break;
        }
        case HAL_glDeleteBuffers:
        {
            GLsizei  a0;HALGLBuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteBuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glDeleteFramebuffers:
        {
            GLsizei  a0;HALGLFramebuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteFramebuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glDeleteProgram:
        {
            HALGLProgram* a0;
            read(a0);
            device.glDeleteProgram(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glDeleteProgramPipelines:
        {
            GLsizei  a0;HALGLProgramPipeline* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteProgramPipelines(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glDeleteRenderbuffers:
        {
            GLsizei  a0;HALGLRenderbuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteRenderbuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glDeleteShader:
        {
            HALGLShader* a0;
            read(a0);
            device.glDeleteShader(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glDeleteSync:
        {
            GLsync  a0;
            read(a0);
            device.glDeleteSync(a0);
            break;
        }
        case HAL_glDeleteTextures:
        {
            GLsizei  a0;HALGLTexture* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteTextures(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;

        }
        case HAL_glDeleteVertexArrays:
        {
            GLsizei  a0;HALGLVertexArray* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glDeleteVertexArrays(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glDepthFunc:
        {
            GLenum  a0;
            read(a0);
            device.glDepthFunc(a0);
            break;
        }
        case HAL_glDepthMask:
        {
            GLboolean  a0;
            read(a0);
            device.glDepthMask(a0);
            break;
        }
        case HAL_glDisable:
        {
            GLenum  a0;
            read(a0);
            device.glDisable(a0);
            break;
        }
        case HAL_glDisableVertexAttribArray:
        {
            GLuint  a0;
            read(a0);
            device.glDisableVertexAttribArray(a0);
            break;
        }
        case HAL_glDrawArrays:
        {
            GLenum  a0;GLint  a1;GLsizei  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glDrawArrays(a0, a1, a2);
            break;
        }
        case HAL_glDrawElements:
        {
            GLenum  a0;GLsizei  a1;GLenum  a2;const GLvoid * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glDrawElements(a0, a1, a2, a3);
            break;
        }
        case HAL_glDrawElementsInstanced:
        {
            GLenum  a0;GLsizei  a1;GLenum  a2;const void * a3;GLsizei  a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glDrawElementsInstanced(a0, a1, a2, a3, a4);
            break;
        }
        case HAL_glEnable:
        {
            GLenum  a0;
            read(a0);
            device.glEnable(a0);
            break;
        }
        case HAL_glEnableVertexAttribArray:
        {
            GLuint  a0;
            read(a0);
            device.glEnableVertexAttribArray(a0);
            break;
        }
        case HAL_glFenceSync:
        {
            GLenum  a0;GLbitfield  a1;
            read(a0);
            read(a1);
            device.glFenceSync(a0, a1);
            break;
        }
        case HAL_glFlush:
        {
            ;
            device.glFlush();
            break;
        }
        case HAL_glFlushMappedBufferRange:
        {
            GLenum  a0;GLintptr  a1;GLsizeiptr  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glFlushMappedBufferRange(a0, a1, a2);
            break;
        }
        case HAL_glFramebufferRenderbuffer:
        {
            GLenum  a0;GLenum  a1;GLenum  a2;HALGLRenderbuffer* a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glFramebufferRenderbuffer(a0, a1, a2, a3);
            ReleaseNullCheck(a3);
            break;
        }
        case HAL_glFramebufferTexture2D:
        {
            GLenum  a0;GLenum  a1;GLenum  a2;HALGLTexture* a3;GLint  a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glFramebufferTexture2D(a0, a1, a2, a3, a4);
            ReleaseNullCheck(a3);
            break;
        }
        case HAL_glGenBuffers:
        {
            GLsizei  a0;HALGLBuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenBuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenFramebuffers:
        {
            GLsizei  a0;HALGLFramebuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenFramebuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenProgramPipelines:
        {
            GLsizei  a0;HALGLProgramPipeline* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenProgramPipelines(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenRenderbuffers:
        {
            GLsizei  a0;HALGLRenderbuffer* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenRenderbuffers(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenTextures:
        {
            GLsizei  a0;HALGLTexture* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenTextures(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenVertexArrays:
        {
            GLsizei  a0;HALGLVertexArray* a1[MaximumGLResourcesPerCall];
            read(a0);
            for (int resource = 0; resource < a0; ++resource)
                read(a1[resource]);
            device.glGenVertexArrays(a0, a1);
            for (int resource = 0; resource < a0; ++resource)
                ReleaseNullCheck(a1[resource]);
            break;
        }
        case HAL_glGenerateMipmap:
        {
            GLenum  a0;
            read(a0);
            device.glGenerateMipmap(a0);
            break;
        }
        case HAL_glGetActiveUniform:
        {
            GLuint  a0;GLuint  a1;GLsizei  a2;GLsizei * a3;GLint * a4;GLenum * a5;GLchar * a6;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            read(a6);
            device.glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
            break;
        }
        case HAL_glGetError:
        {
            ;
            device.glGetError();
            break;
        }
        case HAL_glGetFloatv:
        {
            GLenum  a0;GLfloat * a1;
            read(a0);
            read(a1);
            device.glGetFloatv(a0, a1);
            break;
        }
        case HAL_glGetFragDataLocation:
        {
            HALGLProgram* a0;const GLchar * a1;
            read(a0);
            read(a1);
            device.glGetFragDataLocation(a0, a1);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetFramebufferAttachmentParameteriv:
        {
            GLenum  a0;GLenum  a1;GLenum  a2;GLint * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
            break;
        }
        case HAL_glGetIntegerv:
        {
            GLenum  a0;GLint * a1;
            read(a0);
            read(a1);
            device.glGetIntegerv(a0, a1);
            break;
        }
        case HAL_glGetProgramBinary:
        {
            HALGLProgram* a0;GLsizei  a1;GLsizei * a2;GLenum * a3;void * a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glGetProgramBinary(a0, a1, a2, a3, a4);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetProgramInfoLog:
        {
            HALGLProgram* a0;GLsizei  a1;GLsizei * a2;GLchar * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glGetProgramInfoLog(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetProgramiv:
        {
            HALGLProgram* a0;GLenum  a1;GLint * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glGetProgramiv(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetRenderbufferParameteriv:
        {
            GLenum  a0;GLenum  a1;GLint * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glGetRenderbufferParameteriv(a0, a1, a2);
            break;
        }
        case HAL_glGetShaderInfoLog:
        {
            HALGLShader* a0;GLsizei  a1;GLsizei * a2;GLchar * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glGetShaderInfoLog(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetShaderiv:
        {
            HALGLShader* a0;GLenum  a1;GLint * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glGetShaderiv(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glGetString:
        {
            GLenum  a0;
            read(a0);
            device.glGetString(a0);
            break;
        }
        case HAL_glGetStringi:
        {
            GLenum  a0;GLuint  a1;
            read(a0);
            read(a1);
            device.glGetStringi(a0, a1);
            break;
        }
        case HAL_glGetSynciv:
        {
            GLsync  a0;GLenum  a1;GLsizei  a2;GLsizei * a3;GLint * a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glGetSynciv(a0, a1, a2, a3, a4);
            break;
        }
        case HAL_glGetTexImage:
        {
            GLenum  a0;GLint  a1;GLenum  a2;GLenum  a3;GLvoid * a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glGetTexImage(a0, a1, a2, a3, a4);
            break;
        }
        case HAL_glGetTexLevelParameteriv:
        {
            GLenum  a0;GLint  a1;GLenum  a2;GLint * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glGetTexLevelParameteriv(a0, a1, a2, a3);
            break;
        }
        case HAL_glGetTexParameteriv:
        {
            GLenum  a0;GLenum  a1;GLint * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glGetTexParameteriv(a0, a1, a2);
            break;
        }
        case HAL_glGetUniformLocation:
        {
            HALGLProgram* a0;const GLchar * a1;HALGLUniformLocation* a2;
            read(a0);
            read(a1);
            read(a2);
            device.glGetUniformLocation(a0, a1, a2);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a2);
            break;
        }
        case HAL_glIsFramebuffer:
        {
            HALGLFramebuffer* a0;
            read(a0);
            device.glIsFramebuffer(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glIsProgram:
        {
            HALGLProgram* a0;
            read(a0);
            device.glIsProgram(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glIsRenderbuffer:
        {
            HALGLRenderbuffer* a0;
            read(a0);
            device.glIsRenderbuffer(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glLinkProgram:
        {
            HALGLProgram* a0;
            read(a0);
            device.glLinkProgram(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glMapBuffer:
        {
            GLenum  a0;GLenum  a1;
            read(a0);
            read(a1);
            break;
        }
        case HAL_glMapBufferRange:
        {
            GLenum  a0;GLintptr  a1;GLsizeiptr  a2;GLbitfield  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glMapBufferRange(a0, a1, a2, a3);
            break;
        }
        case HAL_glPixelStorei:
        {
            GLenum  a0;GLint  a1;
            read(a0);
            read(a1);
            device.glPixelStorei(a0, a1);
            break;
        }
        case HAL_glPolygonMode:
        {
            GLenum  a0;GLenum  a1;
            read(a0);
            read(a1);
            device.glPolygonMode(a0, a1);
            break;
        }
        case HAL_glPopGroupMarker:
        {
            ;
            device.glPopGroupMarker();
            break;
        }
        case HAL_glProgramBinary:
        {
            HALGLProgram* a0;GLenum  a1;const void * a2;GLint  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramBinary(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glProgramParameteri:
        {
            HALGLProgram* a0;GLenum  a1;GLint  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glProgramParameteri(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glProgramUniform1fv:
        {
            HALGLProgram* a0;HALGLUniformLocation* a1;GLsizei  a2;const GLfloat * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramUniform1fv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glProgramUniform1iv:
        {
            HALGLProgram*  a0;HALGLUniformLocation* a1;GLsizei  a2;const GLint * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramUniform1iv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glProgramUniform2fv:
        {
            HALGLProgram*  a0;HALGLUniformLocation* a1;GLsizei  a2;const GLfloat * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramUniform2fv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glProgramUniform3fv:
        {
            HALGLProgram*  a0;HALGLUniformLocation* a1;GLsizei  a2;const GLfloat * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramUniform3fv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glProgramUniform4fv:
        {
            HALGLProgram*  a0;HALGLUniformLocation* a1;GLsizei  a2;const GLfloat * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glProgramUniform4fv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glProgramUniformMatrix4fv:
        {
            HALGLProgram*  a0;HALGLUniformLocation* a1;GLsizei  a2;GLboolean  a3;const GLfloat * a4;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            device.glProgramUniformMatrix4fv(a0, a1, a2, a3, a4);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a1);
            break;
        }
        case HAL_glPushGroupMarker:
        {
            GLsizei  a0;const GLchar * a1;
            read(a0);
            read(a1);
            device.glPushGroupMarker(a0, a1);
            break;
        }
        case HAL_glReadPixels:
        {
            GLint  a0;GLint  a1;GLsizei  a2;GLsizei  a3;GLenum  a4;GLenum  a5;GLvoid * a6;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            read(a6);
            device.glReadPixels(a0, a1, a2, a3, a4, a5, a6);
            break;
        }
        case HAL_glReleaseShaderCompiler:
        {
            ;
            device.glReleaseShaderCompiler();
            break;
        }
        case HAL_glRenderbufferStorage:
        {
            GLenum a0;GLenum  a1;GLsizei  a2;GLsizei  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glRenderbufferStorage(a0, a1, a2, a3);
            break;
        }
        case HAL_glScissor:
        {
            GLint  a0;GLint  a1;GLsizei  a2;GLsizei  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glScissor(a0, a1, a2, a3);
            break;
        }
        case HAL_glShaderSource:
        {
            HALGLShader* a0;GLsizei  a1;GLSHADERSOURCEA2TYPE a2;const GLint * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glShaderSource(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            for (int i = 0; i < a1; i++)
                free(a2[i]);
            free(a2);
            free(a3);
            break;
        }
        case HAL_glStencilFunc:
        {
            GLenum  a0;GLint  a1;GLuint  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glStencilFunc(a0, a1, a2);
            break;
        }
        case HAL_glStencilMask:
        {
            GLuint  a0;
            read(a0);
            device.glStencilMask(a0);
            break;
        }
        case HAL_glStencilOp:
        {
            GLenum  a0;GLenum  a1;GLenum  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glStencilOp(a0, a1, a2);
            break;
        }
        case HAL_glStringMarker:
        {
            GLsizei  a0;const void * a1;
            read(a0);
            read(a1);
            device.glStringMarker(a0, a1);
            break;
        }
        case HAL_glTexImage2D:
        {
            GLenum  a0;GLint  a1;GLint  a2;GLsizei  a3;GLsizei  a4;GLint  a5;GLenum  a6;GLenum  a7;const GLvoid * a8;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            read(a6);
            read(a7);
            read(a8);
            device.glTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
            free(a8);
            break;
        }
        case HAL_glTexParameteri:
        {
            GLenum  a0;GLenum  a1;GLint  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glTexParameteri(a0, a1, a2);
            break;
        }
        case HAL_glTexSubImage2D:
        {
            GLenum  a0;GLint  a1;GLint  a2;GLint  a3;GLsizei  a4;GLsizei  a5;GLenum  a6;GLenum  a7;const GLvoid * a8;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            read(a6);
            read(a7);
            read(a8);
            device.glTexSubImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
            break;
        }
        case HAL_glUniform1f:
        {
            HALGLUniformLocation* a0;GLfloat  a1;
            read(a0);
            read(a1);
            device.glUniform1f(a0, a1);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glUniform1fv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;const GLfloat * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform1fv(a0, a1, a2);
            ReleaseNullCheck(a0);
            free(a2);
            break;
        }
        case HAL_glUniform1i:
        {
            HALGLUniformLocation* a0;GLint  a1;
            read(a0);
            read(a1);
            device.glUniform1i(a0, a1);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glUniform1iv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;const GLint * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform1iv(a0, a1, a2);
            ReleaseNullCheck(a0);
            free(a2);
            break;
        }
        case HAL_glUniform2f:
        {
            HALGLUniformLocation* a0;GLfloat  a1;GLfloat  a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform2f(a0, a1, a2);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glUniform2fv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;const GLfloat * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform2fv(a0, a1, a2);
            ReleaseNullCheck(a0);
            free(a2);
            break;
        }
        case HAL_glUniform3fv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;const GLfloat * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform3fv(a0, a1, a2);
            ReleaseNullCheck(a0);
            free(a2);
            break;
        }
        case HAL_glUniform4fv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;const GLfloat * a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUniform4fv(a0, a1, a2);
            ReleaseNullCheck(a0);
            free(a2);
            break;
        }
        case HAL_glUniformMatrix4fv:
        {
            HALGLUniformLocation* a0;GLsizei  a1;GLboolean  a2;const GLfloat * a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glUniformMatrix4fv(a0, a1, a2, a3);
            ReleaseNullCheck(a0);
            free(a3);
            break;
        }
        case HAL_glUnmapBuffer:
        {
            GLenum  a0;
            read(a0);
            device.glUnmapBuffer(a0);
            break;
        }
        case HAL_glUseProgram:
        {
            HALGLProgram* a0;
            read(a0);
            device.glUseProgram(a0);
            ReleaseNullCheck(a0);
            break;
        }
        case HAL_glUseProgramStages:
        {
            HALGLProgramPipeline* a0;GLbitfield  a1;HALGLProgram* a2;
            read(a0);
            read(a1);
            read(a2);
            device.glUseProgramStages(a0, a1, a2);
            ReleaseNullCheck(a0);
            ReleaseNullCheck(a2);
            break;
        }
        case HAL_glVertexAttribPointer:
        {
            GLuint  a0;GLint  a1;GLenum  a2;GLboolean  a3;GLsizei  a4;const void * a5;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            read(a4);
            read(a5);
            device.glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
            break;
        }
        case HAL_glViewport:
        {
            GLint  a0;GLint  a1;GLsizei  a2;GLsizei  a3;
            read(a0);
            read(a1);
            read(a2);
            read(a3);
            device.glViewport(a0, a1, a2, a3);
            break;
        }
    }
}

}}} // Scaleform::Render::GL
