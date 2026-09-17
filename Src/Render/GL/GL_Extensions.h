/**********************************************************************

Filename    :   GL_Extensions.h
Content     :   GL  extension header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_Extensions_H
#define INC_SF_Render_GL_Extensions_H

static unsigned SF_GL_VERBOSE_EXTENSION_PRINT = 0;   // Set this to non-zero to have usage of all extension functions print in the debug output.

#include "Kernel/SF_Debug.h"
#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
    PFNGLGETACTIVEUNIFORMPROC                    p_glGetActiveUniform;
    #if defined(GL_ARB_map_buffer_range)
    PFNGLMAPBUFFERRANGEPROC                      p_glMapBufferRange;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORM4FVPROC                   p_glProgramUniform4fv;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLGETPROGRAMPIPELINEIVPROC                p_glGetProgramPipelineiv;
    #endif
    PFNGLCREATESHADERPROC                        p_glCreateShader;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC                p_glCompressedTexImage2D;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLGETPROGRAMPIPELINEINFOLOGPROC           p_glGetProgramPipelineInfoLog;
    #endif
    PFNGLGENERATEMIPMAPPROC                      p_glGenerateMipmap;
    #if defined(GL_ARB_get_program_binary)
    PFNGLPROGRAMBINARYPROC                       p_glProgramBinary;
    #endif
    PFNGLBLENDEQUATIONPROC                       p_glBlendEquation;
    #if defined(GL_ARB_sync)
    PFNGLGETSYNCIVPROC                           p_glGetSynciv;
    #endif
    PFNGLBINDATTRIBLOCATIONPROC                  p_glBindAttribLocation;
    PFNGLCOMPILESHADERPROC                       p_glCompileShader;
    PFNGLATTACHSHADERPROC                        p_glAttachShader;
    PFNGLBINDRENDERBUFFERPROC                    p_glBindRenderbuffer;
    PFNGLGETATTRIBLOCATIONPROC                   p_glGetAttribLocation;
    PFNGLBLENDFUNCSEPARATEPROC                   p_glBlendFuncSeparate;
    #if defined(GL_ARB_debug_output)
    PFNGLDEBUGMESSAGECALLBACKARBPROC             p_glDebugMessageCallbackARB;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORM3FVPROC                   p_glProgramUniform3fv;
    #endif
    PFNGLGETSHADERIVPROC                         p_glGetShaderiv;
    PFNGLGETRENDERBUFFERPARAMETERIVPROC          p_glGetRenderbufferParameteriv;
    #if defined(GL_EXT_gpu_shader4)
    PFNGLBINDFRAGDATALOCATIONPROC                p_glBindFragDataLocation;
    #endif
    #if defined(GL_GREMEDY_string_marker)
    PFNGLSTRINGMARKERGREMEDYPROC                 p_glStringMarkerGREMEDY;
    #endif
    PFNGLCREATEPROGRAMPROC                       p_glCreateProgram;
    PFNGLUNIFORM1FPROC                           p_glUniform1f;
    #if defined(GL_ARB_sync)
    PFNGLFENCESYNCPROC                           p_glFenceSync;
    #endif
    #if defined(GL_EXT_framebuffer_blit)
    PFNGLBLITFRAMEBUFFEREXTPROC                  p_glBlitFramebufferEXT;
    #endif
    PFNGLGENFRAMEBUFFERSPROC                     p_glGenFramebuffers;
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORM4FVEXTPROC                p_glProgramUniform4fvEXT;
    #endif
    PFNGLGENRENDERBUFFERSPROC                    p_glGenRenderbuffers;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLBINDPROGRAMPIPELINEPROC                 p_glBindProgramPipeline;
    #endif
    PFNGLFRAMEBUFFERRENDERBUFFERPROC             p_glFramebufferRenderbuffer;
    #if defined(GL_ARB_vertex_array_object)
    PFNGLGENVERTEXARRAYSPROC                     p_glGenVertexArrays;
    #endif
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORM1IVEXTPROC                p_glProgramUniform1ivEXT;
    #endif
    PFNGLUNIFORM4FVPROC                          p_glUniform4fv;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLUSEPROGRAMSTAGESPROC                    p_glUseProgramStages;
    #endif
    PFNGLUNMAPBUFFERPROC                         p_glUnmapBuffer;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLGENPROGRAMPIPELINESPROC                 p_glGenProgramPipelines;
    #endif
    #if defined(GL_ARB_sync)
    PFNGLCLIENTWAITSYNCPROC                      p_glClientWaitSync;
    #endif
    #if defined(GL_ARB_sync)
    PFNGLDELETESYNCPROC                          p_glDeleteSync;
    #endif
    PFNGLUNIFORM2FVPROC                          p_glUniform2fv;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORM2FVPROC                   p_glProgramUniform2fv;
    #endif
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORM1FVEXTPROC                p_glProgramUniform1fvEXT;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORMMATRIX4FVPROC             p_glProgramUniformMatrix4fv;
    #endif
    PFNGLMAPBUFFERPROC                           p_glMapBuffer;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC p_glGetFramebufferAttachmentParameteriv;
    #if defined(GL_ARB_draw_instanced)
    PFNGLDRAWELEMENTSINSTANCEDPROC               p_glDrawElementsInstanced;
    #endif
    PFNGLGENBUFFERSPROC                          p_glGenBuffers;
    PFNGLBUFFERDATAPROC                          p_glBufferData;
    PFNGLLINKPROGRAMPROC                         p_glLinkProgram;
    PFNGLACTIVETEXTUREPROC                       p_glActiveTexture;
    PFNGLGETPROGRAMIVPROC                        p_glGetProgramiv;
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC          p_glProgramUniformMatrix4fvEXT;
    #endif
    PFNGLBLENDEQUATIONSEPARATEPROC               p_glBlendEquationSeparate;
    PFNGLUNIFORM1FVPROC                          p_glUniform1fv;
    PFNGLRENDERBUFFERSTORAGEPROC                 p_glRenderbufferStorage;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC            p_glDisableVertexAttribArray;
    PFNGLUNIFORM1IVPROC                          p_glUniform1iv;
    #if defined(GL_EXT_gpu_shader4)
    PFNGLGETFRAGDATALOCATIONPROC                 p_glGetFragDataLocation;
    #endif
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORM2FVEXTPROC                p_glProgramUniform2fvEXT;
    #endif
    PFNGLCHECKFRAMEBUFFERSTATUSPROC              p_glCheckFramebufferStatus;
    PFNGLISRENDERBUFFERPROC                      p_glIsRenderbuffer;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMPARAMETERIPROC                   p_glProgramParameteri;
    #endif
    #if defined(GL_ARB_get_program_binary)
    PFNGLGETPROGRAMBINARYPROC                    p_glGetProgramBinary;
    #endif
    PFNGLUNIFORM1IPROC                           p_glUniform1i;
    #if defined(GL_EXT_direct_state_access)
    PFNGLPROGRAMUNIFORM3FVEXTPROC                p_glProgramUniform3fvEXT;
    #endif
    #if defined(GL_ARB_vertex_array_object)
    PFNGLDELETEVERTEXARRAYSPROC                  p_glDeleteVertexArrays;
    #endif
    PFNGLFRAMEBUFFERTEXTURE2DPROC                p_glFramebufferTexture2D;
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORM1FVPROC                   p_glProgramUniform1fv;
    #endif
    PFNGLDELETERENDERBUFFERSPROC                 p_glDeleteRenderbuffers;
    PFNGLBUFFERSUBDATAPROC                       p_glBufferSubData;
    PFNGLDELETEPROGRAMPROC                       p_glDeleteProgram;
    PFNGLDELETEFRAMEBUFFERSPROC                  p_glDeleteFramebuffers;
    #if defined(GL_ARB_map_buffer_range)
    PFNGLFLUSHMAPPEDBUFFERRANGEPROC              p_glFlushMappedBufferRange;
    #endif
    #if defined(GL_VERSION_3_0)
    PFNGLGETSTRINGIPROC                          p_glGetStringi;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLDELETEPROGRAMPIPELINESPROC              p_glDeleteProgramPipelines;
    #endif
    PFNGLISPROGRAMPROC                           p_glIsProgram;
    PFNGLUSEPROGRAMPROC                          p_glUseProgram;
    PFNGLGETPROGRAMINFOLOGPROC                   p_glGetProgramInfoLog;
    PFNGLDELETESHADERPROC                        p_glDeleteShader;
    PFNGLDELETEBUFFERSPROC                       p_glDeleteBuffers;
    PFNGLBINDFRAMEBUFFERPROC                     p_glBindFramebuffer;
    PFNGLSHADERSOURCEPROC                        p_glShaderSource;
    PFNGLBINDBUFFERPROC                          p_glBindBuffer;
    PFNGLISFRAMEBUFFERPROC                       p_glIsFramebuffer;
    PFNGLGETSHADERINFOLOGPROC                    p_glGetShaderInfoLog;
    PFNGLENABLEVERTEXATTRIBARRAYPROC             p_glEnableVertexAttribArray;
    PFNGLUNIFORMMATRIX4FVPROC                    p_glUniformMatrix4fv;
    PFNGLVERTEXATTRIBPOINTERPROC                 p_glVertexAttribPointer;
    PFNGLUNIFORM2FPROC                           p_glUniform2f;
    PFNGLGETUNIFORMLOCATIONPROC                  p_glGetUniformLocation;
    #if defined(GL_ARB_debug_output)
    PFNGLDEBUGMESSAGECONTROLARBPROC              p_glDebugMessageControlARB;
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    PFNGLPROGRAMUNIFORM1IVPROC                   p_glProgramUniform1iv;
    #endif
    PFNGLUNIFORM3FVPROC                          p_glUniform3fv;
    #if defined(GL_ARB_vertex_array_object)
    PFNGLBINDVERTEXARRAYPROC                     p_glBindVertexArray;
    #endif
public:
    bool Init();
    
    class ScopedGLErrorCheck
    {
    public:
        ScopedGLErrorCheck(const char* func) : FuncName(func)
        {
            performErrorCheck("before");
        }
        ~ScopedGLErrorCheck()
        {
            performErrorCheck("after");
        }
    private:
        void performErrorCheck(const char* timing)
        {
            SF_UNUSED(timing);
            #ifdef SF_BUILD_DEBUG
                GLenum error = glGetError(); SF_UNUSED(error);
                static const char* errorMessages[] = {
                    "GL_INVALID_ENUM",
                    "GL_INVALID_VALUE",
                    "GL_INVALID_OPERATION",
                    "GL_STACK_OVERFLOW",
                    "GL_STACK_UNDERFLOW",
                    "GL_OUT_OF_MEMORY" };
                if (error >= GL_INVALID_ENUM && error <= GL_OUT_OF_MEMORY)
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (error code = %s)\n", timing, FuncName, errorMessages[error-GL_INVALID_ENUM] );
                }
                else
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (non-standard error code = %d)\n", timing, FuncName, error );
                }
            #endif
        }
        const char * FuncName;
    };
     void glGetActiveUniform(GLuint a0, GLuint a1, GLsizei a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE7(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetActiveUniform(%x, %x, %d, 0x%p, 0x%p, 0x%p, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3), reinterpret_cast<const void*>(a4), reinterpret_cast<const void*>(a5), reinterpret_cast<const void*>(a6) );
        return p_glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
    }

     GLvoid* glMapBufferRange(GLenum a0, GLintptr a1, GLsizeiptr a2, GLbitfield a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glMapBufferRange(%d, 0x%p, 0x%p, 0x%08x)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2), a3 );
        #if defined(GL_ARB_map_buffer_range)
        return p_glMapBufferRange(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glMapBufferRange extension function (#if defined(GL_ARB_map_buffer_range))");
        return ( GLvoid*)(0);
        #endif
    }

     void glProgramUniform4fv(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform4fv(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniform4fv(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform4fv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glGetProgramPipelineiv(GLuint a0, GLenum a1, GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramPipelineiv(%x, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glGetProgramPipelineiv(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramPipelineiv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     GLuint glCreateShader(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glCreateShader(%d)\n" , a0 );
        return p_glCreateShader(a0);
    }

     void glCompressedTexImage2D(GLenum a0, GLint a1, GLenum a2, GLsizei a3, GLsizei a4, GLint a5, GLsizei a6, const GLvoid * a7)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE8(SF_GL_VERBOSE_EXTENSION_PRINT, "glCompressedTexImage2D(%d, %d, %d, %d, %d, %d, %d, 0x%p)\n" , a0, a1, a2, a3, a4, a5, a6, reinterpret_cast<const void*>(a7) );
        return p_glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
    }

     void glGetProgramPipelineInfoLog(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramPipelineInfoLog(%x, %d, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glGetProgramPipelineInfoLog(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramPipelineInfoLog extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glGenerateMipmap(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenerateMipmap(%d)\n" , a0 );
        return p_glGenerateMipmap(a0);
    }

     void glProgramBinary(GLuint a0, GLenum a1, const GLvoid * a2, GLsizei a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramBinary(%x, %d, 0x%p, %d)\n" , a0, a1, reinterpret_cast<const void*>(a2), a3 );
        #if defined(GL_ARB_get_program_binary)
        return p_glProgramBinary(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramBinary extension function (#if defined(GL_ARB_get_program_binary))");
        return ( void)(0);
        #endif
    }

     void glBlendEquation(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glBlendEquation(%d)\n" , a0 );
        return p_glBlendEquation(a0);
    }

     void glGetSynciv(GLsync a0, GLenum a1, GLsizei a2, GLsizei * a3, GLint * a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetSynciv(0x%p, %d, %d, 0x%p, 0x%p)\n" , reinterpret_cast<const void*>(a0), a1, a2, reinterpret_cast<const void*>(a3), reinterpret_cast<const void*>(a4) );
        #if defined(GL_ARB_sync)
        return p_glGetSynciv(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetSynciv extension function (#if defined(GL_ARB_sync))");
        return ( void)(0);
        #endif
    }

     void glBindAttribLocation(GLuint a0, GLuint a1, const GLchar * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindAttribLocation(%x, %x, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glBindAttribLocation(a0, a1, a2);
    }

     void glCompileShader(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glCompileShader(%x)\n" , a0 );
        return p_glCompileShader(a0);
    }

     void glAttachShader(GLuint a0, GLuint a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glAttachShader(%x, %x)\n" , a0, a1 );
        return p_glAttachShader(a0, a1);
    }

     void glBindRenderbuffer(GLenum a0, GLuint a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindRenderbuffer(%d, %x)\n" , a0, a1 );
        return p_glBindRenderbuffer(a0, a1);
    }

     GLint glGetAttribLocation(GLuint a0, const GLchar * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetAttribLocation(%x, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glGetAttribLocation(a0, a1);
    }

     void glBlendFuncSeparate(GLenum a0, GLenum a1, GLenum a2, GLenum a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glBlendFuncSeparate(%d, %d, %d, %d)\n" , a0, a1, a2, a3 );
        return p_glBlendFuncSeparate(a0, a1, a2, a3);
    }

     void glDebugMessageCallbackARB(GLDEBUGPROCARB a0, const GLvoid * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDebugMessageCallbackARB(0x%p, 0x%p)\n" , reinterpret_cast<const void*>(a0), reinterpret_cast<const void*>(a1) );
        #if defined(GL_ARB_debug_output)
        return p_glDebugMessageCallbackARB(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDebugMessageCallbackARB extension function (#if defined(GL_ARB_debug_output))");
        return ( void)(0);
        #endif
    }

     void glProgramUniform3fv(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform3fv(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniform3fv(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform3fv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glGetShaderiv(GLuint a0, GLenum a1, GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetShaderiv(%x, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glGetShaderiv(a0, a1, a2);
    }

     void glGetRenderbufferParameteriv(GLenum a0, GLenum a1, GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetRenderbufferParameteriv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glGetRenderbufferParameteriv(a0, a1, a2);
    }

     void glBindFragDataLocation(GLuint a0, GLuint a1, const GLchar * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindFragDataLocation(%x, %x, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        #if defined(GL_EXT_gpu_shader4)
        return p_glBindFragDataLocation(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBindFragDataLocation extension function (#if defined(GL_EXT_gpu_shader4))");
        return ( void)(0);
        #endif
    }

     void glStringMarkerGREMEDY(GLsizei a0, const GLvoid * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glStringMarkerGREMEDY(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_GREMEDY_string_marker)
        return p_glStringMarkerGREMEDY(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glStringMarkerGREMEDY extension function (#if defined(GL_GREMEDY_string_marker))");
        return ( void)(0);
        #endif
    }

     GLuint glCreateProgram()
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE (SF_GL_VERBOSE_EXTENSION_PRINT, "glCreateProgram(\n");
        return p_glCreateProgram();
    }

     void glUniform1f(GLint a0, GLfloat a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform1f(%d, %.2f)\n" , a0, a1 );
        return p_glUniform1f(a0, a1);
    }

     GLsync glFenceSync(GLenum a0, GLbitfield a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glFenceSync(%d, 0x%08x)\n" , a0, a1 );
        #if defined(GL_ARB_sync)
        return p_glFenceSync(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glFenceSync extension function (#if defined(GL_ARB_sync))");
        return ( GLsync)(0);
        #endif
    }

     void glBlitFramebufferEXT(GLint a0, GLint a1, GLint a2, GLint a3, GLint a4, GLint a5, GLint a6, GLint a7, GLbitfield a8, GLenum a9)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE10(SF_GL_VERBOSE_EXTENSION_PRINT, "glBlitFramebufferEXT(%d, %d, %d, %d, %d, %d, %d, %d, 0x%08x, %d)\n" , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 );
        #if defined(GL_EXT_framebuffer_blit)
        return p_glBlitFramebufferEXT(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        #else
        SF_UNUSED10(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBlitFramebufferEXT extension function (#if defined(GL_EXT_framebuffer_blit))");
        return ( void)(0);
        #endif
    }

     void glGenFramebuffers(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenFramebuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glGenFramebuffers(a0, a1);
    }

     void glProgramUniform4fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform4fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniform4fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform4fvEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     void glGenRenderbuffers(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenRenderbuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glGenRenderbuffers(a0, a1);
    }

     void glBindProgramPipeline(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindProgramPipeline(%x)\n" , a0 );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glBindProgramPipeline(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBindProgramPipeline extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glFramebufferRenderbuffer(GLenum a0, GLenum a1, GLenum a2, GLuint a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glFramebufferRenderbuffer(%d, %d, %d, %x)\n" , a0, a1, a2, a3 );
        return p_glFramebufferRenderbuffer(a0, a1, a2, a3);
    }

     void glGenVertexArrays(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenVertexArrays(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_ARB_vertex_array_object)
        return p_glGenVertexArrays(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGenVertexArrays extension function (#if defined(GL_ARB_vertex_array_object))");
        return ( void)(0);
        #endif
    }

     void glProgramUniform1ivEXT(GLuint a0, GLint a1, GLsizei a2, const GLint * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1ivEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniform1ivEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1ivEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     void glUniform4fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform4fv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glUniform4fv(a0, a1, a2);
    }

     void glUseProgramStages(GLuint a0, GLbitfield a1, GLuint a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUseProgramStages(%x, 0x%08x, %x)\n" , a0, a1, a2 );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glUseProgramStages(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glUseProgramStages extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     GLboolean glUnmapBuffer(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glUnmapBuffer(%d)\n" , a0 );
        return p_glUnmapBuffer(a0);
    }

     void glGenProgramPipelines(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenProgramPipelines(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glGenProgramPipelines(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGenProgramPipelines extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     GLenum glClientWaitSync(GLsync a0, GLbitfield a1, GLuint64 a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glClientWaitSync(0x%p, 0x%08x, 0x%p)\n" , reinterpret_cast<const void*>(a0), a1, reinterpret_cast<const void*>(a2) );
        #if defined(GL_ARB_sync)
        return p_glClientWaitSync(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glClientWaitSync extension function (#if defined(GL_ARB_sync))");
        return ( GLenum)(0);
        #endif
    }

     void glDeleteSync(GLsync a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteSync(0x%p)\n" , reinterpret_cast<const void*>(a0) );
        #if defined(GL_ARB_sync)
        return p_glDeleteSync(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteSync extension function (#if defined(GL_ARB_sync))");
        return ( void)(0);
        #endif
    }

     void glUniform2fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform2fv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glUniform2fv(a0, a1, a2);
    }

     void glProgramUniform2fv(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform2fv(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniform2fv(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform2fv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glProgramUniform1fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniform1fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1fvEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     void glProgramUniformMatrix4fv(GLuint a0, GLint a1, GLsizei a2, GLboolean a3, const GLfloat * a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniformMatrix4fv(%x, %d, %d, %d, 0x%p)\n" , a0, a1, a2, a3, reinterpret_cast<const void*>(a4) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniformMatrix4fv(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniformMatrix4fv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     GLvoid* glMapBuffer(GLenum a0, GLenum a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glMapBuffer(%d, %d)\n" , a0, a1 );
        return p_glMapBuffer(a0, a1);
    }

     void glGetFramebufferAttachmentParameteriv(GLenum a0, GLenum a1, GLenum a2, GLint * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetFramebufferAttachmentParameteriv(%d, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        return p_glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
    }

     void glDrawElementsInstanced(GLenum a0, GLsizei a1, GLenum a2, const GLvoid * a3, GLsizei a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glDrawElementsInstanced(%d, %d, %d, 0x%p, %d)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3), a4 );
        #if defined(GL_ARB_draw_instanced)
        return p_glDrawElementsInstanced(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDrawElementsInstanced extension function (#if defined(GL_ARB_draw_instanced))");
        return ( void)(0);
        #endif
    }

     void glGenBuffers(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenBuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glGenBuffers(a0, a1);
    }

     void glBufferData(GLenum a0, GLsizeiptr a1, const GLvoid * a2, GLenum a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glBufferData(%d, 0x%p, 0x%p, %d)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2), a3 );
        return p_glBufferData(a0, a1, a2, a3);
    }

     void glLinkProgram(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glLinkProgram(%x)\n" , a0 );
        return p_glLinkProgram(a0);
    }

     void glActiveTexture(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glActiveTexture(%d)\n" , a0 );
        return p_glActiveTexture(a0);
    }

     void glGetProgramiv(GLuint a0, GLenum a1, GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramiv(%x, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glGetProgramiv(a0, a1, a2);
    }

     void glProgramUniformMatrix4fvEXT(GLuint a0, GLint a1, GLsizei a2, GLboolean a3, const GLfloat * a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniformMatrix4fvEXT(%x, %d, %d, %d, 0x%p)\n" , a0, a1, a2, a3, reinterpret_cast<const void*>(a4) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniformMatrix4fvEXT(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniformMatrix4fvEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     void glBlendEquationSeparate(GLenum a0, GLenum a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glBlendEquationSeparate(%d, %d)\n" , a0, a1 );
        return p_glBlendEquationSeparate(a0, a1);
    }

     void glUniform1fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform1fv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glUniform1fv(a0, a1, a2);
    }

     void glRenderbufferStorage(GLenum a0, GLenum a1, GLsizei a2, GLsizei a3)
    {
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glRenderbufferStorage(%d, %d, %d, %d)\n" , a0, a1, a2, a3 );
        return p_glRenderbufferStorage(a0, a1, a2, a3);
    }

     void glDisableVertexAttribArray(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glDisableVertexAttribArray(%x)\n" , a0 );
        return p_glDisableVertexAttribArray(a0);
    }

     void glUniform1iv(GLint a0, GLsizei a1, const GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform1iv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glUniform1iv(a0, a1, a2);
    }

     GLint glGetFragDataLocation(GLuint a0, const GLchar * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetFragDataLocation(%x, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_EXT_gpu_shader4)
        return p_glGetFragDataLocation(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetFragDataLocation extension function (#if defined(GL_EXT_gpu_shader4))");
        return ( GLint)(0);
        #endif
    }

     void glProgramUniform2fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform2fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniform2fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform2fvEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     GLenum glCheckFramebufferStatus(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glCheckFramebufferStatus(%d)\n" , a0 );
        return p_glCheckFramebufferStatus(a0);
    }

     GLboolean glIsRenderbuffer(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glIsRenderbuffer(%x)\n" , a0 );
        return p_glIsRenderbuffer(a0);
    }

     void glProgramParameteri(GLuint a0, GLenum a1, GLint a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramParameteri(%x, %d, %d)\n" , a0, a1, a2 );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramParameteri(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramParameteri extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glGetProgramBinary(GLuint a0, GLsizei a1, GLsizei * a2, GLenum * a3, GLvoid * a4)
    {
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramBinary(%x, %d, 0x%p, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3), reinterpret_cast<const void*>(a4) );
        #if defined(GL_ARB_get_program_binary)
        return p_glGetProgramBinary(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramBinary extension function (#if defined(GL_ARB_get_program_binary))");
        return ( void)(0);
        #endif
    }

     void glUniform1i(GLint a0, GLint a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform1i(%d, %d)\n" , a0, a1 );
        return p_glUniform1i(a0, a1);
    }

     void glProgramUniform3fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform3fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_direct_state_access)
        return p_glProgramUniform3fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform3fvEXT extension function (#if defined(GL_EXT_direct_state_access))");
        return ( void)(0);
        #endif
    }

     void glDeleteVertexArrays(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteVertexArrays(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_ARB_vertex_array_object)
        return p_glDeleteVertexArrays(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteVertexArrays extension function (#if defined(GL_ARB_vertex_array_object))");
        return ( void)(0);
        #endif
    }

     void glFramebufferTexture2D(GLenum a0, GLenum a1, GLenum a2, GLuint a3, GLint a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glFramebufferTexture2D(%d, %d, %d, %x, %d)\n" , a0, a1, a2, a3, a4 );
        return p_glFramebufferTexture2D(a0, a1, a2, a3, a4);
    }

     void glProgramUniform1fv(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1fv(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniform1fv(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1fv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glDeleteRenderbuffers(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteRenderbuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glDeleteRenderbuffers(a0, a1);
    }

     void glBufferSubData(GLenum a0, GLintptr a1, GLsizeiptr a2, const GLvoid * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glBufferSubData(%d, 0x%p, 0x%p, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        return p_glBufferSubData(a0, a1, a2, a3);
    }

     void glDeleteProgram(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteProgram(%x)\n" , a0 );
        return p_glDeleteProgram(a0);
    }

     void glDeleteFramebuffers(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteFramebuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glDeleteFramebuffers(a0, a1);
    }

     void glFlushMappedBufferRange(GLenum a0, GLintptr a1, GLsizeiptr a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glFlushMappedBufferRange(%d, 0x%p, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2) );
        #if defined(GL_ARB_map_buffer_range)
        return p_glFlushMappedBufferRange(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glFlushMappedBufferRange extension function (#if defined(GL_ARB_map_buffer_range))");
        return ( void)(0);
        #endif
    }

     const GLubyte * glGetStringi(GLenum a0, GLuint a1)
    {
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetStringi(%d, %x)\n" , a0, a1 );
        #if defined(GL_VERSION_3_0)
        return p_glGetStringi(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetStringi extension function (#if defined(GL_VERSION_3_0))");
        return ( const GLubyte *)(0);
        #endif
    }

     void glDeleteProgramPipelines(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteProgramPipelines(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glDeleteProgramPipelines(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteProgramPipelines extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     GLboolean glIsProgram(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glIsProgram(%x)\n" , a0 );
        return p_glIsProgram(a0);
    }

     void glUseProgram(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glUseProgram(%x)\n" , a0 );
        return p_glUseProgram(a0);
    }

     void glGetProgramInfoLog(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramInfoLog(%x, %d, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        return p_glGetProgramInfoLog(a0, a1, a2, a3);
    }

     void glDeleteShader(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteShader(%x)\n" , a0 );
        return p_glDeleteShader(a0);
    }

     void glDeleteBuffers(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteBuffers(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glDeleteBuffers(a0, a1);
    }

     void glBindFramebuffer(GLenum a0, GLuint a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindFramebuffer(%d, %x)\n" , a0, a1 );
        return p_glBindFramebuffer(a0, a1);
    }

     void glShaderSource(GLuint a0, GLsizei a1, const GLchar* * a2, const GLint * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glShaderSource(%x, %d, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        return p_glShaderSource(a0, a1, a2, a3);
    }

     void glBindBuffer(GLenum a0, GLuint a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindBuffer(%d, %x)\n" , a0, a1 );
        return p_glBindBuffer(a0, a1);
    }

     GLboolean glIsFramebuffer(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glIsFramebuffer(%x)\n" , a0 );
        return p_glIsFramebuffer(a0);
    }

     void glGetShaderInfoLog(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetShaderInfoLog(%x, %d, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        return p_glGetShaderInfoLog(a0, a1, a2, a3);
    }

     void glEnableVertexAttribArray(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glEnableVertexAttribArray(%x)\n" , a0 );
        return p_glEnableVertexAttribArray(a0);
    }

     void glUniformMatrix4fv(GLint a0, GLsizei a1, GLboolean a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniformMatrix4fv(%d, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        return p_glUniformMatrix4fv(a0, a1, a2, a3);
    }

     void glVertexAttribPointer(GLuint a0, GLint a1, GLenum a2, GLboolean a3, GLsizei a4, const GLvoid * a5)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE6(SF_GL_VERBOSE_EXTENSION_PRINT, "glVertexAttribPointer(%x, %d, %d, %d, %d, 0x%p)\n" , a0, a1, a2, a3, a4, reinterpret_cast<const void*>(a5) );
        return p_glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
    }

     void glUniform2f(GLint a0, GLfloat a1, GLfloat a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform2f(%d, %.2f, %.2f)\n" , a0, a1, a2 );
        return p_glUniform2f(a0, a1, a2);
    }

     GLint glGetUniformLocation(GLuint a0, const GLchar * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetUniformLocation(%x, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        return p_glGetUniformLocation(a0, a1);
    }

     void glDebugMessageControlARB(GLenum a0, GLenum a1, GLenum a2, GLsizei a3, const GLuint * a4, GLboolean a5)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE6(SF_GL_VERBOSE_EXTENSION_PRINT, "glDebugMessageControlARB(%d, %d, %d, %d, 0x%p, %d)\n" , a0, a1, a2, a3, reinterpret_cast<const void*>(a4), a5 );
        #if defined(GL_ARB_debug_output)
        return p_glDebugMessageControlARB(a0, a1, a2, a3, a4, a5);
        #else
        SF_UNUSED6(a0, a1, a2, a3, a4, a5);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDebugMessageControlARB extension function (#if defined(GL_ARB_debug_output))");
        return ( void)(0);
        #endif
    }

     void glProgramUniform1iv(GLuint a0, GLint a1, GLsizei a2, const GLint * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1iv(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_ARB_separate_shader_objects)
        return p_glProgramUniform1iv(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1iv extension function (#if defined(GL_ARB_separate_shader_objects))");
        return ( void)(0);
        #endif
    }

     void glUniform3fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUniform3fv(%d, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        return p_glUniform3fv(a0, a1, a2);
    }

     void glBindVertexArray(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindVertexArray(%x)\n" , a0 );
        #if defined(GL_ARB_vertex_array_object)
        return p_glBindVertexArray(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBindVertexArray extension function (#if defined(GL_ARB_vertex_array_object))");
        return ( void)(0);
        #endif
    }

};

}}}
#endif
