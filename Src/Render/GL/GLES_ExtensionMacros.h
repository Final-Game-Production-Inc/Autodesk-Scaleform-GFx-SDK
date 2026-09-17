/**********************************************************************

Filename    :   GLES_ExtensionMacros.h
Content     :   GL ES extension macros header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GLES_ExtensionMacros_H
#define INC_SF_Render_GLES_ExtensionMacros_H
#include "Render/GL/GL_HAL.h"

#ifdef SF_GL_RUNTIME_LINK

    #if defined(GL_EXT_separate_shader_objects)
    #define glDeleteProgramPipelinesEXT GetHAL()->glDeleteProgramPipelinesEXT
    #endif
    #if defined(GL_OES_vertex_array_object)
    #define glGenVertexArraysOES GetHAL()->glGenVertexArraysOES
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniform1fvEXT GetHAL()->glProgramUniform1fvEXT
    #endif
    #if defined(GL_OES_mapbuffer)
    #define glUnmapBufferOES GetHAL()->glUnmapBufferOES
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glUseProgramStagesEXT GetHAL()->glUseProgramStagesEXT
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glGenProgramPipelinesEXT GetHAL()->glGenProgramPipelinesEXT
    #endif
    #if defined(GL_EXT_map_buffer_range)
    #define glMapBufferRangeEXT GetHAL()->glMapBufferRangeEXT
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniformMatrix4fvEXT GetHAL()->glProgramUniformMatrix4fvEXT
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glGetProgramPipelineInfoLogEXT GetHAL()->glGetProgramPipelineInfoLogEXT
    #endif
    #if defined(GL_EXT_map_buffer_range)
    #define glFlushMappedBufferRangeEXT GetHAL()->glFlushMappedBufferRangeEXT
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniform4fvEXT GetHAL()->glProgramUniform4fvEXT
    #endif
    #if defined(GL_APPLE_sync)
    #define glGetSyncivAPPLE GetHAL()->glGetSyncivAPPLE
    #endif
    #if defined(GL_APPLE_sync)
    #define glFenceSyncAPPLE GetHAL()->glFenceSyncAPPLE
    #endif
    #if defined(GL_OES_vertex_array_object)
    #define glBindVertexArrayOES GetHAL()->glBindVertexArrayOES
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glGetProgramPipelineivEXT GetHAL()->glGetProgramPipelineivEXT
    #endif
    #if defined(GL_OES_get_program_binary)
    #define glProgramBinaryOES GetHAL()->glProgramBinaryOES
    #endif
    #if defined(GL_KHR_debug_output)
    #define glDebugMessageControl GetHAL()->glDebugMessageControl
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniform2fvEXT GetHAL()->glProgramUniform2fvEXT
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniform1ivEXT GetHAL()->glProgramUniform1ivEXT
    #endif
    #if defined(GL_OES_vertex_array_object)
    #define glDeleteVertexArraysOES GetHAL()->glDeleteVertexArraysOES
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glBindProgramPipelineEXT GetHAL()->glBindProgramPipelineEXT
    #endif
    #if defined(GL_KHR_debug_output)
    #define glDebugMessageCallback GetHAL()->glDebugMessageCallback
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramParameteriEXT GetHAL()->glProgramParameteriEXT
    #endif
    #if defined(GL_APPLE_sync)
    #define glClientWaitSyncAPPLE GetHAL()->glClientWaitSyncAPPLE
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    #define glProgramUniform3fvEXT GetHAL()->glProgramUniform3fvEXT
    #endif
    #if defined(GL_OES_mapbuffer)
    #define glMapBufferOES GetHAL()->glMapBufferOES
    #endif
    #if defined(GL_OES_get_program_binary)
    #define glGetProgramBinaryOES GetHAL()->glGetProgramBinaryOES
    #endif
    #if defined(GL_APPLE_sync)
    #define glDeleteSyncAPPLE GetHAL()->glDeleteSyncAPPLE
    #endif

#else

#endif
#endif
