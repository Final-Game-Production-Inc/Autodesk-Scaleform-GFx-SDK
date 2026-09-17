/**********************************************************************

Filename    :   GLES_Extensions.h
Content     :   GL ES extension header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GLES_Extensions_H
#define INC_SF_Render_GLES_Extensions_H

static unsigned SF_GL_VERBOSE_EXTENSION_PRINT = 0;   // Set this to non-zero to have usage of all extension functions print in the debug output.

#include "Kernel/SF_Debug.h"
#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLDELETEPROGRAMPIPELINESEXTPROC    p_glDeleteProgramPipelinesEXT;
    #endif
    #if defined(GL_OES_vertex_array_object)
    PFNGLGENVERTEXARRAYSOESPROC           p_glGenVertexArraysOES;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORM1FVEXTPROC         p_glProgramUniform1fvEXT;
    #endif
    #if defined(GL_OES_mapbuffer)
    PFNGLUNMAPBUFFEROESPROC               p_glUnmapBufferOES;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLUSEPROGRAMSTAGESEXTPROC          p_glUseProgramStagesEXT;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLGENPROGRAMPIPELINESEXTPROC       p_glGenProgramPipelinesEXT;
    #endif
    #if defined(GL_EXT_map_buffer_range)
    PFNGLMAPBUFFERRANGEEXTPROC            p_glMapBufferRangeEXT;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC   p_glProgramUniformMatrix4fvEXT;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC p_glGetProgramPipelineInfoLogEXT;
    #endif
    #if defined(GL_EXT_map_buffer_range)
    PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC    p_glFlushMappedBufferRangeEXT;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORM4FVEXTPROC         p_glProgramUniform4fvEXT;
    #endif
    #if defined(GL_APPLE_sync)
    PFNGLGETSYNCIVAPPLEPROC               p_glGetSyncivAPPLE;
    #endif
    #if defined(GL_APPLE_sync)
    PFNGLFENCESYNCAPPLEPROC               p_glFenceSyncAPPLE;
    #endif
    #if defined(GL_OES_vertex_array_object)
    PFNGLBINDVERTEXARRAYOESPROC           p_glBindVertexArrayOES;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLGETPROGRAMPIPELINEIVEXTPROC      p_glGetProgramPipelineivEXT;
    #endif
    #if defined(GL_OES_get_program_binary)
    PFNGLPROGRAMBINARYOESPROC             p_glProgramBinaryOES;
    #endif
    #if defined(GL_KHR_debug_output)
    PFNGLDEBUGMESSAGECONTROLPROC          p_glDebugMessageControl;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORM2FVEXTPROC         p_glProgramUniform2fvEXT;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORM1IVEXTPROC         p_glProgramUniform1ivEXT;
    #endif
    #if defined(GL_OES_vertex_array_object)
    PFNGLDELETEVERTEXARRAYSOESPROC        p_glDeleteVertexArraysOES;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLBINDPROGRAMPIPELINEEXTPROC       p_glBindProgramPipelineEXT;
    #endif
    #if defined(GL_KHR_debug_output)
    PFNGLDEBUGMESSAGECALLBACKPROC         p_glDebugMessageCallback;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMPARAMETERIEXTPROC         p_glProgramParameteriEXT;
    #endif
    #if defined(GL_APPLE_sync)
    PFNGLCLIENTWAITSYNCAPPLEPROC          p_glClientWaitSyncAPPLE;
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    PFNGLPROGRAMUNIFORM3FVEXTPROC         p_glProgramUniform3fvEXT;
    #endif
    #if defined(GL_OES_mapbuffer)
    PFNGLMAPBUFFEROESPROC                 p_glMapBufferOES;
    #endif
    #if defined(GL_OES_get_program_binary)
    PFNGLGETPROGRAMBINARYOESPROC          p_glGetProgramBinaryOES;
    #endif
    #if defined(GL_APPLE_sync)
    PFNGLDELETESYNCAPPLEPROC              p_glDeleteSyncAPPLE;
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
    void glDeleteProgramPipelinesEXT(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteProgramPipelinesEXT(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glDeleteProgramPipelinesEXT(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteProgramPipelinesEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glGenVertexArraysOES(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenVertexArraysOES(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_OES_vertex_array_object)
        p_glGenVertexArraysOES(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGenVertexArraysOES extension function (#if defined(GL_OES_vertex_array_object))");
        #endif
    }

    void glProgramUniform1fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniform1fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1fvEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    GLboolean glUnmapBufferOES(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glUnmapBufferOES(%d)\n" , a0 );
        #if defined(GL_OES_mapbuffer)
        return p_glUnmapBufferOES(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glUnmapBufferOES extension function (#if defined(GL_OES_mapbuffer))");
        return (GLboolean)(0);
        #endif
    }

    void glUseProgramStagesEXT(GLuint a0, GLbitfield a1, GLuint a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glUseProgramStagesEXT(%x, 0x%08x, %x)\n" , a0, a1, a2 );
        #if defined(GL_EXT_separate_shader_objects)
        p_glUseProgramStagesEXT(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glUseProgramStagesEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glGenProgramPipelinesEXT(GLsizei a0, GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glGenProgramPipelinesEXT(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glGenProgramPipelinesEXT(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGenProgramPipelinesEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void* glMapBufferRangeEXT(GLenum a0, GLintptr a1, GLsizeiptr a2, GLbitfield a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glMapBufferRangeEXT(%d, 0x%p, 0x%p, 0x%08x)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2), a3 );
        #if defined(GL_EXT_map_buffer_range)
        return p_glMapBufferRangeEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glMapBufferRangeEXT extension function (#if defined(GL_EXT_map_buffer_range))");
        return (void*)(0);
        #endif
    }

    void glProgramUniformMatrix4fvEXT(GLuint a0, GLint a1, GLsizei a2, GLboolean a3, const GLfloat * a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniformMatrix4fvEXT(%x, %d, %d, %d, 0x%p)\n" , a0, a1, a2, a3, reinterpret_cast<const void*>(a4) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniformMatrix4fvEXT(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniformMatrix4fvEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glGetProgramPipelineInfoLogEXT(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramPipelineInfoLogEXT(%x, %d, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glGetProgramPipelineInfoLogEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramPipelineInfoLogEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glFlushMappedBufferRangeEXT(GLenum a0, GLintptr a1, GLsizeiptr a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glFlushMappedBufferRangeEXT(%d, 0x%p, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1), reinterpret_cast<const void*>(a2) );
        #if defined(GL_EXT_map_buffer_range)
        p_glFlushMappedBufferRangeEXT(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glFlushMappedBufferRangeEXT extension function (#if defined(GL_EXT_map_buffer_range))");
        #endif
    }

    void glProgramUniform4fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform4fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniform4fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform4fvEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glGetSyncivAPPLE(GLsync a0, GLenum a1, GLsizei a2, GLsizei * a3, GLint * a4)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetSyncivAPPLE(0x%p, %d, %d, 0x%p, 0x%p)\n" , reinterpret_cast<const void*>(a0), a1, a2, reinterpret_cast<const void*>(a3), reinterpret_cast<const void*>(a4) );
        #if defined(GL_APPLE_sync)
        p_glGetSyncivAPPLE(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetSyncivAPPLE extension function (#if defined(GL_APPLE_sync))");
        #endif
    }

    GLsync glFenceSyncAPPLE(GLenum a0, GLbitfield a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glFenceSyncAPPLE(%d, 0x%08x)\n" , a0, a1 );
        #if defined(GL_APPLE_sync)
        return p_glFenceSyncAPPLE(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glFenceSyncAPPLE extension function (#if defined(GL_APPLE_sync))");
        return (GLsync)(0);
        #endif
    }

    void glBindVertexArrayOES(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindVertexArrayOES(%x)\n" , a0 );
        #if defined(GL_OES_vertex_array_object)
        p_glBindVertexArrayOES(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBindVertexArrayOES extension function (#if defined(GL_OES_vertex_array_object))");
        #endif
    }

    void glGetProgramPipelineivEXT(GLuint a0, GLenum a1, GLint * a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramPipelineivEXT(%x, %d, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glGetProgramPipelineivEXT(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramPipelineivEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glProgramBinaryOES(GLuint a0, GLenum a1, const GLvoid * a2, GLint a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramBinaryOES(%x, %d, 0x%p, %d)\n" , a0, a1, reinterpret_cast<const void*>(a2), a3 );
        #if defined(GL_OES_get_program_binary)
        p_glProgramBinaryOES(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramBinaryOES extension function (#if defined(GL_OES_get_program_binary))");
        #endif
    }

    void glDebugMessageControl(GLenum a0, GLenum a1, GLenum a2, GLsizei a3, const GLuint * a4, GLboolean a5)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE6(SF_GL_VERBOSE_EXTENSION_PRINT, "glDebugMessageControl(%d, %d, %d, %d, 0x%p, %d)\n" , a0, a1, a2, a3, reinterpret_cast<const void*>(a4), a5 );
        #if defined(GL_KHR_debug_output)
        p_glDebugMessageControl(a0, a1, a2, a3, a4, a5);
        #else
        SF_UNUSED6(a0, a1, a2, a3, a4, a5);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDebugMessageControl extension function (#if defined(GL_KHR_debug_output))");
        #endif
    }

    void glProgramUniform2fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform2fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniform2fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform2fvEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glProgramUniform1ivEXT(GLuint a0, GLint a1, GLsizei a2, const GLint * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform1ivEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniform1ivEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform1ivEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glDeleteVertexArraysOES(GLsizei a0, const GLuint * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteVertexArraysOES(%d, 0x%p)\n" , a0, reinterpret_cast<const void*>(a1) );
        #if defined(GL_OES_vertex_array_object)
        p_glDeleteVertexArraysOES(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteVertexArraysOES extension function (#if defined(GL_OES_vertex_array_object))");
        #endif
    }

    void glBindProgramPipelineEXT(GLuint a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glBindProgramPipelineEXT(%x)\n" , a0 );
        #if defined(GL_EXT_separate_shader_objects)
        p_glBindProgramPipelineEXT(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glBindProgramPipelineEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void glDebugMessageCallback(GLDEBUGPROC a0, const void * a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glDebugMessageCallback(0x%p, 0x%p)\n" , reinterpret_cast<const void*>(a0), reinterpret_cast<const void*>(a1) );
        #if defined(GL_KHR_debug_output)
        p_glDebugMessageCallback(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDebugMessageCallback extension function (#if defined(GL_KHR_debug_output))");
        #endif
    }

    void glProgramParameteriEXT(GLuint a0, GLenum a1, GLint a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramParameteriEXT(%x, %d, %d)\n" , a0, a1, a2 );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramParameteriEXT(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramParameteriEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    GLenum glClientWaitSyncAPPLE(GLsync a0, GLbitfield a1, GLuint64 a2)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE3(SF_GL_VERBOSE_EXTENSION_PRINT, "glClientWaitSyncAPPLE(0x%p, 0x%08x, 0x%p)\n" , reinterpret_cast<const void*>(a0), a1, reinterpret_cast<const void*>(a2) );
        #if defined(GL_APPLE_sync)
        return p_glClientWaitSyncAPPLE(a0, a1, a2);
        #else
        SF_UNUSED3(a0, a1, a2);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glClientWaitSyncAPPLE extension function (#if defined(GL_APPLE_sync))");
        return (GLenum)(0);
        #endif
    }

    void glProgramUniform3fvEXT(GLuint a0, GLint a1, GLsizei a2, const GLfloat * a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE4(SF_GL_VERBOSE_EXTENSION_PRINT, "glProgramUniform3fvEXT(%x, %d, %d, 0x%p)\n" , a0, a1, a2, reinterpret_cast<const void*>(a3) );
        #if defined(GL_EXT_separate_shader_objects)
        p_glProgramUniform3fvEXT(a0, a1, a2, a3);
        #else
        SF_UNUSED4(a0, a1, a2, a3);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glProgramUniform3fvEXT extension function (#if defined(GL_EXT_separate_shader_objects))");
        #endif
    }

    void* glMapBufferOES(GLenum a0, GLenum a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE2(SF_GL_VERBOSE_EXTENSION_PRINT, "glMapBufferOES(%d, %d)\n" , a0, a1 );
        #if defined(GL_OES_mapbuffer)
        return p_glMapBufferOES(a0, a1);
        #else
        SF_UNUSED2(a0, a1);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glMapBufferOES extension function (#if defined(GL_OES_mapbuffer))");
        return (void*)(0);
        #endif
    }

    void glGetProgramBinaryOES(GLuint a0, GLsizei a1, GLsizei * a2, GLenum * a3, GLvoid * a4)
    {
        SF_DEBUG_MESSAGE5(SF_GL_VERBOSE_EXTENSION_PRINT, "glGetProgramBinaryOES(%x, %d, 0x%p, 0x%p, 0x%p)\n" , a0, a1, reinterpret_cast<const void*>(a2), reinterpret_cast<const void*>(a3), reinterpret_cast<const void*>(a4) );
        #if defined(GL_OES_get_program_binary)
        p_glGetProgramBinaryOES(a0, a1, a2, a3, a4);
        #else
        SF_UNUSED5(a0, a1, a2, a3, a4);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glGetProgramBinaryOES extension function (#if defined(GL_OES_get_program_binary))");
        #endif
    }

    void glDeleteSyncAPPLE(GLsync a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        SF_DEBUG_MESSAGE1(SF_GL_VERBOSE_EXTENSION_PRINT, "glDeleteSyncAPPLE(0x%p)\n" , reinterpret_cast<const void*>(a0) );
        #if defined(GL_APPLE_sync)
        p_glDeleteSyncAPPLE(a0);
        #else
        SF_UNUSED1(a0);
        SF_DEBUG_ASSERT(1, "glext.h did not contain required preprocessor defines to "
                           "use the glDeleteSyncAPPLE extension function (#if defined(GL_APPLE_sync))");
        #endif
    }

};

}}}
#endif
