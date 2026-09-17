/**********************************************************************

Filename    :   GL_ExtensionMacros.h
Content     :   GL  extension macros header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_ExtensionMacros_H
#define INC_SF_Render_GL_ExtensionMacros_H
#include "Render/GL/GL_HAL.h"

#ifdef SF_GL_RUNTIME_LINK

    #define glGetActiveUniform GetHAL()->glGetActiveUniform
    #if defined(GL_ARB_map_buffer_range)
    #define glMapBufferRange GetHAL()->glMapBufferRange
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniform4fv GetHAL()->glProgramUniform4fv
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glGetProgramPipelineiv GetHAL()->glGetProgramPipelineiv
    #endif
    #define glCreateShader GetHAL()->glCreateShader
    #define glCompressedTexImage2D GetHAL()->glCompressedTexImage2D
    #if defined(GL_ARB_separate_shader_objects)
    #define glGetProgramPipelineInfoLog GetHAL()->glGetProgramPipelineInfoLog
    #endif
    #define glGenerateMipmap GetHAL()->glGenerateMipmap
    #if defined(GL_ARB_get_program_binary)
    #define glProgramBinary GetHAL()->glProgramBinary
    #endif
    #define glBlendEquation GetHAL()->glBlendEquation
    #if defined(GL_ARB_sync)
    #define glGetSynciv GetHAL()->glGetSynciv
    #endif
    #define glBindAttribLocation GetHAL()->glBindAttribLocation
    #define glCompileShader GetHAL()->glCompileShader
    #define glAttachShader GetHAL()->glAttachShader
    #define glBindRenderbuffer GetHAL()->glBindRenderbuffer
    #define glGetAttribLocation GetHAL()->glGetAttribLocation
    #define glBlendFuncSeparate GetHAL()->glBlendFuncSeparate
    #if defined(GL_ARB_debug_output)
    #define glDebugMessageCallbackARB GetHAL()->glDebugMessageCallbackARB
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniform3fv GetHAL()->glProgramUniform3fv
    #endif
    #define glGetShaderiv GetHAL()->glGetShaderiv
    #define glGetRenderbufferParameteriv GetHAL()->glGetRenderbufferParameteriv
    #if defined(GL_EXT_gpu_shader4)
    #define glBindFragDataLocation GetHAL()->glBindFragDataLocation
    #endif
    #if defined(GL_GREMEDY_string_marker)
    #define glStringMarkerGREMEDY GetHAL()->glStringMarkerGREMEDY
    #endif
    #define glCreateProgram GetHAL()->glCreateProgram
    #define glUniform1f GetHAL()->glUniform1f
    #if defined(GL_ARB_sync)
    #define glFenceSync GetHAL()->glFenceSync
    #endif
    #if defined(GL_EXT_framebuffer_blit)
    #define glBlitFramebufferEXT GetHAL()->glBlitFramebufferEXT
    #endif
    #define glGenFramebuffers GetHAL()->glGenFramebuffers
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniform4fvEXT GetHAL()->glProgramUniform4fvEXT
    #endif
    #define glGenRenderbuffers GetHAL()->glGenRenderbuffers
    #if defined(GL_ARB_separate_shader_objects)
    #define glBindProgramPipeline GetHAL()->glBindProgramPipeline
    #endif
    #define glFramebufferRenderbuffer GetHAL()->glFramebufferRenderbuffer
    #if defined(GL_ARB_vertex_array_object)
    #define glGenVertexArrays GetHAL()->glGenVertexArrays
    #endif
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniform1ivEXT GetHAL()->glProgramUniform1ivEXT
    #endif
    #define glUniform4fv GetHAL()->glUniform4fv
    #if defined(GL_ARB_separate_shader_objects)
    #define glUseProgramStages GetHAL()->glUseProgramStages
    #endif
    #define glUnmapBuffer GetHAL()->glUnmapBuffer
    #if defined(GL_ARB_separate_shader_objects)
    #define glGenProgramPipelines GetHAL()->glGenProgramPipelines
    #endif
    #if defined(GL_ARB_sync)
    #define glClientWaitSync GetHAL()->glClientWaitSync
    #endif
    #if defined(GL_ARB_sync)
    #define glDeleteSync GetHAL()->glDeleteSync
    #endif
    #define glUniform2fv GetHAL()->glUniform2fv
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniform2fv GetHAL()->glProgramUniform2fv
    #endif
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniform1fvEXT GetHAL()->glProgramUniform1fvEXT
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniformMatrix4fv GetHAL()->glProgramUniformMatrix4fv
    #endif
    #define glMapBuffer GetHAL()->glMapBuffer
    #define glGetFramebufferAttachmentParameteriv GetHAL()->glGetFramebufferAttachmentParameteriv
    #if defined(GL_ARB_draw_instanced)
    #define glDrawElementsInstanced GetHAL()->glDrawElementsInstanced
    #endif
    #define glGenBuffers GetHAL()->glGenBuffers
    #define glBufferData GetHAL()->glBufferData
    #define glLinkProgram GetHAL()->glLinkProgram
    #define glActiveTexture GetHAL()->glActiveTexture
    #define glGetProgramiv GetHAL()->glGetProgramiv
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniformMatrix4fvEXT GetHAL()->glProgramUniformMatrix4fvEXT
    #endif
    #define glBlendEquationSeparate GetHAL()->glBlendEquationSeparate
    #define glUniform1fv GetHAL()->glUniform1fv
    #define glRenderbufferStorage GetHAL()->glRenderbufferStorage
    #define glDisableVertexAttribArray GetHAL()->glDisableVertexAttribArray
    #define glUniform1iv GetHAL()->glUniform1iv
    #if defined(GL_EXT_gpu_shader4)
    #define glGetFragDataLocation GetHAL()->glGetFragDataLocation
    #endif
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniform2fvEXT GetHAL()->glProgramUniform2fvEXT
    #endif
    #define glCheckFramebufferStatus GetHAL()->glCheckFramebufferStatus
    #define glIsRenderbuffer GetHAL()->glIsRenderbuffer
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramParameteri GetHAL()->glProgramParameteri
    #endif
    #if defined(GL_ARB_get_program_binary)
    #define glGetProgramBinary GetHAL()->glGetProgramBinary
    #endif
    #define glUniform1i GetHAL()->glUniform1i
    #if defined(GL_EXT_direct_state_access)
    #define glProgramUniform3fvEXT GetHAL()->glProgramUniform3fvEXT
    #endif
    #if defined(GL_ARB_vertex_array_object)
    #define glDeleteVertexArrays GetHAL()->glDeleteVertexArrays
    #endif
    #define glFramebufferTexture2D GetHAL()->glFramebufferTexture2D
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniform1fv GetHAL()->glProgramUniform1fv
    #endif
    #define glDeleteRenderbuffers GetHAL()->glDeleteRenderbuffers
    #define glBufferSubData GetHAL()->glBufferSubData
    #define glDeleteProgram GetHAL()->glDeleteProgram
    #define glDeleteFramebuffers GetHAL()->glDeleteFramebuffers
    #if defined(GL_ARB_map_buffer_range)
    #define glFlushMappedBufferRange GetHAL()->glFlushMappedBufferRange
    #endif
    #if defined(GL_VERSION_3_0)
    #define glGetStringi GetHAL()->glGetStringi
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glDeleteProgramPipelines GetHAL()->glDeleteProgramPipelines
    #endif
    #define glIsProgram GetHAL()->glIsProgram
    #define glUseProgram GetHAL()->glUseProgram
    #define glGetProgramInfoLog GetHAL()->glGetProgramInfoLog
    #define glDeleteShader GetHAL()->glDeleteShader
    #define glDeleteBuffers GetHAL()->glDeleteBuffers
    #define glBindFramebuffer GetHAL()->glBindFramebuffer
    #define glShaderSource GetHAL()->glShaderSource
    #define glBindBuffer GetHAL()->glBindBuffer
    #define glIsFramebuffer GetHAL()->glIsFramebuffer
    #define glGetShaderInfoLog GetHAL()->glGetShaderInfoLog
    #define glEnableVertexAttribArray GetHAL()->glEnableVertexAttribArray
    #define glUniformMatrix4fv GetHAL()->glUniformMatrix4fv
    #define glVertexAttribPointer GetHAL()->glVertexAttribPointer
    #define glUniform2f GetHAL()->glUniform2f
    #define glGetUniformLocation GetHAL()->glGetUniformLocation
    #if defined(GL_ARB_debug_output)
    #define glDebugMessageControlARB GetHAL()->glDebugMessageControlARB
    #endif
    #if defined(GL_ARB_separate_shader_objects)
    #define glProgramUniform1iv GetHAL()->glProgramUniform1iv
    #endif
    #define glUniform3fv GetHAL()->glUniform3fv
    #if defined(GL_ARB_vertex_array_object)
    #define glBindVertexArray GetHAL()->glBindVertexArray
    #endif

#else

#endif
#endif
