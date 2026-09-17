/**********************************************************************

Filename    :   GLES_Extensions.cpp
Content     :   GL ES extension implementation.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render/GL/GL_Common.h"

#ifdef SF_GL_RUNTIME_LINK

namespace Scaleform { namespace Render { namespace GL {

bool Extensions::Init()
{
    bool result = 1;
    #if defined(GL_EXT_separate_shader_objects)
    p_glDeleteProgramPipelinesEXT = (PFNGLDELETEPROGRAMPIPELINESEXTPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelinesEXT");
    if (!p_glDeleteProgramPipelinesEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteProgramPipelinesEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_vertex_array_object)
    p_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC) SF_GL_RUNTIME_LINK("glGenVertexArraysOES");
    if (!p_glGenVertexArraysOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenVertexArraysOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvEXT");
    if (!p_glProgramUniform1fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_mapbuffer)
    p_glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glUnmapBufferOES");
    if (!p_glUnmapBufferOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUnmapBufferOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glUseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC) SF_GL_RUNTIME_LINK("glUseProgramStagesEXT");
    if (!p_glUseProgramStagesEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUseProgramStagesEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glGenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelinesEXT");
    if (!p_glGenProgramPipelinesEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenProgramPipelinesEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_map_buffer_range)
    p_glMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC) SF_GL_RUNTIME_LINK("glMapBufferRangeEXT");
    if (!p_glMapBufferRangeEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glMapBufferRangeEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvEXT");
    if (!p_glProgramUniformMatrix4fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniformMatrix4fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glGetProgramPipelineInfoLogEXT = (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineInfoLogEXT");
    if (!p_glGetProgramPipelineInfoLogEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramPipelineInfoLogEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_map_buffer_range)
    p_glFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRangeEXT");
    if (!p_glFlushMappedBufferRangeEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFlushMappedBufferRangeEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvEXT");
    if (!p_glProgramUniform4fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform4fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_APPLE_sync)
    p_glGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC) SF_GL_RUNTIME_LINK("glGetSyncivAPPLE");
        if (!p_glGetSyncivAPPLE)
        {
             p_glGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC) SF_GL_RUNTIME_LINK("glGetSyncivAPPLEARB");
             if (!p_glGetSyncivAPPLE)
                  p_glGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC) SF_GL_RUNTIME_LINK("glGetSyncivAPPLEEXT");
        }
    if (!p_glGetSyncivAPPLE && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetSyncivAPPLE\n");
        result = 0;
    }
    #endif

    #if defined(GL_APPLE_sync)
    p_glFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glFenceSyncAPPLE");
        if (!p_glFenceSyncAPPLE)
        {
             p_glFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glFenceSyncAPPLEARB");
             if (!p_glFenceSyncAPPLE)
                  p_glFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glFenceSyncAPPLEEXT");
        }
    if (!p_glFenceSyncAPPLE && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFenceSyncAPPLE\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_vertex_array_object)
    p_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) SF_GL_RUNTIME_LINK("glBindVertexArrayOES");
    if (!p_glBindVertexArrayOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindVertexArrayOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glGetProgramPipelineivEXT = (PFNGLGETPROGRAMPIPELINEIVEXTPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineivEXT");
    if (!p_glGetProgramPipelineivEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramPipelineivEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_get_program_binary)
    p_glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) SF_GL_RUNTIME_LINK("glProgramBinaryOES");
    if (!p_glProgramBinaryOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramBinaryOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_KHR_debug_output)
    p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControl");
        if (!p_glDebugMessageControl)
        {
             p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControlARB");
             if (!p_glDebugMessageControl)
                  p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControlEXT");
        }
    if (!p_glDebugMessageControl && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDebugMessageControl\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvEXT");
    if (!p_glProgramUniform2fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform2fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivEXT");
    if (!p_glProgramUniform1ivEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1ivEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_vertex_array_object)
    p_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArraysOES");
    if (!p_glDeleteVertexArraysOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteVertexArraysOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glBindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC) SF_GL_RUNTIME_LINK("glBindProgramPipelineEXT");
    if (!p_glBindProgramPipelineEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindProgramPipelineEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_KHR_debug_output)
    p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallback");
        if (!p_glDebugMessageCallback)
        {
             p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackARB");
             if (!p_glDebugMessageCallback)
                  p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackEXT");
        }
    if (!p_glDebugMessageCallback && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDebugMessageCallback\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC) SF_GL_RUNTIME_LINK("glProgramParameteriEXT");
    if (!p_glProgramParameteriEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramParameteriEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_APPLE_sync)
    p_glClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncAPPLE");
        if (!p_glClientWaitSyncAPPLE)
        {
             p_glClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncAPPLEARB");
             if (!p_glClientWaitSyncAPPLE)
                  p_glClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncAPPLEEXT");
        }
    if (!p_glClientWaitSyncAPPLE && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glClientWaitSyncAPPLE\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_separate_shader_objects)
    p_glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvEXT");
    if (!p_glProgramUniform3fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform3fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_mapbuffer)
    p_glMapBufferOES = (PFNGLMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glMapBufferOES");
    if (!p_glMapBufferOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glMapBufferOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_OES_get_program_binary)
    p_glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) SF_GL_RUNTIME_LINK("glGetProgramBinaryOES");
    if (!p_glGetProgramBinaryOES && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramBinaryOES\n");
        result = 0;
    }
    #endif

    #if defined(GL_APPLE_sync)
    p_glDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glDeleteSyncAPPLE");
        if (!p_glDeleteSyncAPPLE)
        {
             p_glDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glDeleteSyncAPPLEARB");
             if (!p_glDeleteSyncAPPLE)
                  p_glDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC) SF_GL_RUNTIME_LINK("glDeleteSyncAPPLEEXT");
        }
    if (!p_glDeleteSyncAPPLE && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteSyncAPPLE\n");
        result = 0;
    }
    #endif

    return result;
}

}}}

#endif
