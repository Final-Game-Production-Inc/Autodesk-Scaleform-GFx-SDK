/**********************************************************************

Filename    :   GL_Extensions.cpp
Content     :   GL  extension implementation.
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
    p_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) SF_GL_RUNTIME_LINK("glGetActiveUniform");
        if (!p_glGetActiveUniform)
        {
             p_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) SF_GL_RUNTIME_LINK("glGetActiveUniformARB");
             if (!p_glGetActiveUniform)
                  p_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) SF_GL_RUNTIME_LINK("glGetActiveUniformEXT");
        }
    if (!p_glGetActiveUniform && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetActiveUniform\n");
        result = 0;
    }

    #if defined(GL_ARB_map_buffer_range)
    p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRange");
        if (!p_glMapBufferRange)
        {
             p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRangeARB");
             if (!p_glMapBufferRange)
                  p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRangeEXT");
        }
    if (!p_glMapBufferRange && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glMapBufferRange\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fv");
        if (!p_glProgramUniform4fv)
        {
             p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvARB");
             if (!p_glProgramUniform4fv)
                  p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvEXT");
        }
    if (!p_glProgramUniform4fv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform4fv\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineiv");
        if (!p_glGetProgramPipelineiv)
        {
             p_glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineivARB");
             if (!p_glGetProgramPipelineiv)
                  p_glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineivEXT");
        }
    if (!p_glGetProgramPipelineiv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramPipelineiv\n");
        result = 0;
    }
    #endif

    p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShader");
        if (!p_glCreateShader)
        {
             p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShaderARB");
             if (!p_glCreateShader)
                  p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShaderEXT");
        }
    if (!p_glCreateShader && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glCreateShader\n");
        result = 0;
    }

    p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2D");
        if (!p_glCompressedTexImage2D)
        {
             p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2DARB");
             if (!p_glCompressedTexImage2D)
                  p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2DEXT");
        }
    if (!p_glCompressedTexImage2D && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glCompressedTexImage2D\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineInfoLog");
        if (!p_glGetProgramPipelineInfoLog)
        {
             p_glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineInfoLogARB");
             if (!p_glGetProgramPipelineInfoLog)
                  p_glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramPipelineInfoLogEXT");
        }
    if (!p_glGetProgramPipelineInfoLog && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramPipelineInfoLog\n");
        result = 0;
    }
    #endif

    p_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) SF_GL_RUNTIME_LINK("glGenerateMipmap");
        if (!p_glGenerateMipmap)
        {
             p_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) SF_GL_RUNTIME_LINK("glGenerateMipmapARB");
             if (!p_glGenerateMipmap)
                  p_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) SF_GL_RUNTIME_LINK("glGenerateMipmapEXT");
        }
    if (!p_glGenerateMipmap && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenerateMipmap\n");
        result = 0;
    }

    #if defined(GL_ARB_get_program_binary)
    p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinary");
        if (!p_glProgramBinary)
        {
             p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinaryARB");
             if (!p_glProgramBinary)
                  p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinaryEXT");
        }
    if (!p_glProgramBinary && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramBinary\n");
        result = 0;
    }
    #endif

    p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquation");
        if (!p_glBlendEquation)
        {
             p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquationARB");
             if (!p_glBlendEquation)
                  p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquationEXT");
        }
    if (!p_glBlendEquation && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBlendEquation\n");
        result = 0;
    }

    #if defined(GL_ARB_sync)
    p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSynciv");
        if (!p_glGetSynciv)
        {
             p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSyncivARB");
             if (!p_glGetSynciv)
                  p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSyncivEXT");
        }
    if (!p_glGetSynciv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetSynciv\n");
        result = 0;
    }
    #endif

    p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocation");
        if (!p_glBindAttribLocation)
        {
             p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocationARB");
             if (!p_glBindAttribLocation)
                  p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocationEXT");
        }
    if (!p_glBindAttribLocation && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindAttribLocation\n");
        result = 0;
    }

    p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShader");
        if (!p_glCompileShader)
        {
             p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShaderARB");
             if (!p_glCompileShader)
                  p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShaderEXT");
        }
    if (!p_glCompileShader && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glCompileShader\n");
        result = 0;
    }

    p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShader");
        if (!p_glAttachShader)
        {
             p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShaderARB");
             if (!p_glAttachShader)
                  p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShaderEXT");
        }
    if (!p_glAttachShader && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glAttachShader\n");
        result = 0;
    }

    p_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glBindRenderbuffer");
        if (!p_glBindRenderbuffer)
        {
             p_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glBindRenderbufferARB");
             if (!p_glBindRenderbuffer)
                  p_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glBindRenderbufferEXT");
        }
    if (!p_glBindRenderbuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindRenderbuffer\n");
        result = 0;
    }

    p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocation");
        if (!p_glGetAttribLocation)
        {
             p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocationARB");
             if (!p_glGetAttribLocation)
                  p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocationEXT");
        }
    if (!p_glGetAttribLocation && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetAttribLocation\n");
        result = 0;
    }

    p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparate");
        if (!p_glBlendFuncSeparate)
        {
             p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateARB");
             if (!p_glBlendFuncSeparate)
                  p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateEXT");
        }
    if (!p_glBlendFuncSeparate && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBlendFuncSeparate\n");
        result = 0;
    }

    #if defined(GL_ARB_debug_output)
    p_glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackARB");
    if (!p_glDebugMessageCallbackARB && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDebugMessageCallbackARB\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fv");
        if (!p_glProgramUniform3fv)
        {
             p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvARB");
             if (!p_glProgramUniform3fv)
                  p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvEXT");
        }
    if (!p_glProgramUniform3fv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform3fv\n");
        result = 0;
    }
    #endif

    p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderiv");
        if (!p_glGetShaderiv)
        {
             p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderivARB");
             if (!p_glGetShaderiv)
                  p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderivEXT");
        }
    if (!p_glGetShaderiv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetShaderiv\n");
        result = 0;
    }

    p_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetRenderbufferParameteriv");
        if (!p_glGetRenderbufferParameteriv)
        {
             p_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetRenderbufferParameterivARB");
             if (!p_glGetRenderbufferParameteriv)
                  p_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetRenderbufferParameterivEXT");
        }
    if (!p_glGetRenderbufferParameteriv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetRenderbufferParameteriv\n");
        result = 0;
    }

    #if defined(GL_EXT_gpu_shader4)
    p_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glBindFragDataLocation");
        if (!p_glBindFragDataLocation)
        {
             p_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glBindFragDataLocationARB");
             if (!p_glBindFragDataLocation)
                  p_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glBindFragDataLocationEXT");
        }
    if (!p_glBindFragDataLocation && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindFragDataLocation\n");
        result = 0;
    }
    #endif

    #if defined(GL_GREMEDY_string_marker)
    p_glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC) SF_GL_RUNTIME_LINK("glStringMarkerGREMEDY");
        if (!p_glStringMarkerGREMEDY)
        {
             p_glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC) SF_GL_RUNTIME_LINK("glStringMarkerGREMEDYARB");
             if (!p_glStringMarkerGREMEDY)
                  p_glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC) SF_GL_RUNTIME_LINK("glStringMarkerGREMEDYEXT");
        }
    if (!p_glStringMarkerGREMEDY && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glStringMarkerGREMEDY\n");
        result = 0;
    }
    #endif

    p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgram");
        if (!p_glCreateProgram)
        {
             p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgramARB");
             if (!p_glCreateProgram)
                  p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgramEXT");
        }
    if (!p_glCreateProgram && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glCreateProgram\n");
        result = 0;
    }

    p_glUniform1f = (PFNGLUNIFORM1FPROC) SF_GL_RUNTIME_LINK("glUniform1f");
        if (!p_glUniform1f)
        {
             p_glUniform1f = (PFNGLUNIFORM1FPROC) SF_GL_RUNTIME_LINK("glUniform1fARB");
             if (!p_glUniform1f)
                  p_glUniform1f = (PFNGLUNIFORM1FPROC) SF_GL_RUNTIME_LINK("glUniform1fEXT");
        }
    if (!p_glUniform1f && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform1f\n");
        result = 0;
    }

    #if defined(GL_ARB_sync)
    p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSync");
        if (!p_glFenceSync)
        {
             p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSyncARB");
             if (!p_glFenceSync)
                  p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSyncEXT");
        }
    if (!p_glFenceSync && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFenceSync\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_framebuffer_blit)
    p_glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC) SF_GL_RUNTIME_LINK("glBlitFramebufferEXT");
    if (!p_glBlitFramebufferEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBlitFramebufferEXT\n");
        result = 0;
    }
    #endif

    p_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenFramebuffers");
        if (!p_glGenFramebuffers)
        {
             p_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenFramebuffersARB");
             if (!p_glGenFramebuffers)
                  p_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenFramebuffersEXT");
        }
    if (!p_glGenFramebuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenFramebuffers\n");
        result = 0;
    }

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvEXT");
    if (!p_glProgramUniform4fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform4fvEXT\n");
        result = 0;
    }
    #endif

    p_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffers");
        if (!p_glGenRenderbuffers)
        {
             p_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffersARB");
             if (!p_glGenRenderbuffers)
                  p_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffersEXT");
        }
    if (!p_glGenRenderbuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenRenderbuffers\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipeline");
        if (!p_glBindProgramPipeline)
        {
             p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipelineARB");
             if (!p_glBindProgramPipeline)
                  p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipelineEXT");
        }
    if (!p_glBindProgramPipeline && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindProgramPipeline\n");
        result = 0;
    }
    #endif

    p_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbuffer");
        if (!p_glFramebufferRenderbuffer)
        {
             p_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbufferARB");
             if (!p_glFramebufferRenderbuffer)
                  p_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbufferEXT");
        }
    if (!p_glFramebufferRenderbuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFramebufferRenderbuffer\n");
        result = 0;
    }

    #if defined(GL_ARB_vertex_array_object)
    p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArrays");
        if (!p_glGenVertexArrays)
        {
             p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArraysARB");
             if (!p_glGenVertexArrays)
                  p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArraysEXT");
        }
    if (!p_glGenVertexArrays && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenVertexArrays\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivEXT");
    if (!p_glProgramUniform1ivEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1ivEXT\n");
        result = 0;
    }
    #endif

    p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fv");
        if (!p_glUniform4fv)
        {
             p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fvARB");
             if (!p_glUniform4fv)
                  p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fvEXT");
        }
    if (!p_glUniform4fv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform4fv\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStages");
        if (!p_glUseProgramStages)
        {
             p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStagesARB");
             if (!p_glUseProgramStages)
                  p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStagesEXT");
        }
    if (!p_glUseProgramStages && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUseProgramStages\n");
        result = 0;
    }
    #endif

    p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBuffer");
        if (!p_glUnmapBuffer)
        {
             p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferARB");
             if (!p_glUnmapBuffer)
                  p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferEXT");
        }
    if (!p_glUnmapBuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUnmapBuffer\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelines");
        if (!p_glGenProgramPipelines)
        {
             p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelinesARB");
             if (!p_glGenProgramPipelines)
                  p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelinesEXT");
        }
    if (!p_glGenProgramPipelines && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenProgramPipelines\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_sync)
    p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSync");
        if (!p_glClientWaitSync)
        {
             p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncARB");
             if (!p_glClientWaitSync)
                  p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncEXT");
        }
    if (!p_glClientWaitSync && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glClientWaitSync\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_sync)
    p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSync");
        if (!p_glDeleteSync)
        {
             p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSyncARB");
             if (!p_glDeleteSync)
                  p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSyncEXT");
        }
    if (!p_glDeleteSync && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteSync\n");
        result = 0;
    }
    #endif

    p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fv");
        if (!p_glUniform2fv)
        {
             p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fvARB");
             if (!p_glUniform2fv)
                  p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fvEXT");
        }
    if (!p_glUniform2fv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform2fv\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fv");
        if (!p_glProgramUniform2fv)
        {
             p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvARB");
             if (!p_glProgramUniform2fv)
                  p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvEXT");
        }
    if (!p_glProgramUniform2fv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform2fv\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvEXT");
    if (!p_glProgramUniform1fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fv");
        if (!p_glProgramUniformMatrix4fv)
        {
             p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvARB");
             if (!p_glProgramUniformMatrix4fv)
                  p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvEXT");
        }
    if (!p_glProgramUniformMatrix4fv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniformMatrix4fv\n");
        result = 0;
    }
    #endif

    p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBuffer");
        if (!p_glMapBuffer)
        {
             p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferARB");
             if (!p_glMapBuffer)
                  p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferEXT");
        }
    if (!p_glMapBuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glMapBuffer\n");
        result = 0;
    }

    p_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetFramebufferAttachmentParameteriv");
        if (!p_glGetFramebufferAttachmentParameteriv)
        {
             p_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetFramebufferAttachmentParameterivARB");
             if (!p_glGetFramebufferAttachmentParameteriv)
                  p_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetFramebufferAttachmentParameterivEXT");
        }
    if (!p_glGetFramebufferAttachmentParameteriv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetFramebufferAttachmentParameteriv\n");
        result = 0;
    }

    #if defined(GL_ARB_draw_instanced)
    p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstanced");
        if (!p_glDrawElementsInstanced)
        {
             p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedARB");
             if (!p_glDrawElementsInstanced)
                  p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedEXT");
        }
    if (!p_glDrawElementsInstanced && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDrawElementsInstanced\n");
        result = 0;
    }
    #endif

    p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffers");
        if (!p_glGenBuffers)
        {
             p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffersARB");
             if (!p_glGenBuffers)
                  p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffersEXT");
        }
    if (!p_glGenBuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGenBuffers\n");
        result = 0;
    }

    p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferData");
        if (!p_glBufferData)
        {
             p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferDataARB");
             if (!p_glBufferData)
                  p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferDataEXT");
        }
    if (!p_glBufferData && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBufferData\n");
        result = 0;
    }

    p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgram");
        if (!p_glLinkProgram)
        {
             p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgramARB");
             if (!p_glLinkProgram)
                  p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgramEXT");
        }
    if (!p_glLinkProgram && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glLinkProgram\n");
        result = 0;
    }

    p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTexture");
        if (!p_glActiveTexture)
        {
             p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTextureARB");
             if (!p_glActiveTexture)
                  p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTextureEXT");
        }
    if (!p_glActiveTexture && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glActiveTexture\n");
        result = 0;
    }

    p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramiv");
        if (!p_glGetProgramiv)
        {
             p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivARB");
             if (!p_glGetProgramiv)
                  p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivEXT");
        }
    if (!p_glGetProgramiv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramiv\n");
        result = 0;
    }

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvEXT");
    if (!p_glProgramUniformMatrix4fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniformMatrix4fvEXT\n");
        result = 0;
    }
    #endif

    p_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendEquationSeparate");
        if (!p_glBlendEquationSeparate)
        {
             p_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendEquationSeparateARB");
             if (!p_glBlendEquationSeparate)
                  p_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendEquationSeparateEXT");
        }
    if (!p_glBlendEquationSeparate && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBlendEquationSeparate\n");
        result = 0;
    }

    p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fv");
        if (!p_glUniform1fv)
        {
             p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fvARB");
             if (!p_glUniform1fv)
                  p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fvEXT");
        }
    if (!p_glUniform1fv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform1fv\n");
        result = 0;
    }

    p_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorage");
        if (!p_glRenderbufferStorage)
        {
             p_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorageARB");
             if (!p_glRenderbufferStorage)
                  p_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorageEXT");
        }
    if (!p_glRenderbufferStorage && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glRenderbufferStorage\n");
        result = 0;
    }

    p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArray");
        if (!p_glDisableVertexAttribArray)
        {
             p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArrayARB");
             if (!p_glDisableVertexAttribArray)
                  p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArrayEXT");
        }
    if (!p_glDisableVertexAttribArray && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDisableVertexAttribArray\n");
        result = 0;
    }

    p_glUniform1iv = (PFNGLUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glUniform1iv");
        if (!p_glUniform1iv)
        {
             p_glUniform1iv = (PFNGLUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glUniform1ivARB");
             if (!p_glUniform1iv)
                  p_glUniform1iv = (PFNGLUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glUniform1ivEXT");
        }
    if (!p_glUniform1iv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform1iv\n");
        result = 0;
    }

    #if defined(GL_EXT_gpu_shader4)
    p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocation");
        if (!p_glGetFragDataLocation)
        {
             p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocationARB");
             if (!p_glGetFragDataLocation)
                  p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocationEXT");
        }
    if (!p_glGetFragDataLocation && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetFragDataLocation\n");
        result = 0;
    }
    #endif

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvEXT");
    if (!p_glProgramUniform2fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform2fvEXT\n");
        result = 0;
    }
    #endif

    p_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatus");
        if (!p_glCheckFramebufferStatus)
        {
             p_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatusARB");
             if (!p_glCheckFramebufferStatus)
                  p_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatusEXT");
        }
    if (!p_glCheckFramebufferStatus && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glCheckFramebufferStatus\n");
        result = 0;
    }

    p_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glIsRenderbuffer");
        if (!p_glIsRenderbuffer)
        {
             p_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glIsRenderbufferARB");
             if (!p_glIsRenderbuffer)
                  p_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glIsRenderbufferEXT");
        }
    if (!p_glIsRenderbuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glIsRenderbuffer\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteri");
        if (!p_glProgramParameteri)
        {
             p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteriARB");
             if (!p_glProgramParameteri)
                  p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteriEXT");
        }
    if (!p_glProgramParameteri && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramParameteri\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_get_program_binary)
    p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinary");
        if (!p_glGetProgramBinary)
        {
             p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinaryARB");
             if (!p_glGetProgramBinary)
                  p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinaryEXT");
        }
    if (!p_glGetProgramBinary && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramBinary\n");
        result = 0;
    }
    #endif

    p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1i");
        if (!p_glUniform1i)
        {
             p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1iARB");
             if (!p_glUniform1i)
                  p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1iEXT");
        }
    if (!p_glUniform1i && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform1i\n");
        result = 0;
    }

    #if defined(GL_EXT_direct_state_access)
    p_glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvEXT");
    if (!p_glProgramUniform3fvEXT && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform3fvEXT\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_vertex_array_object)
    p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArrays");
        if (!p_glDeleteVertexArrays)
        {
             p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArraysARB");
             if (!p_glDeleteVertexArrays)
                  p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArraysEXT");
        }
    if (!p_glDeleteVertexArrays && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteVertexArrays\n");
        result = 0;
    }
    #endif

    p_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2D");
        if (!p_glFramebufferTexture2D)
        {
             p_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2DARB");
             if (!p_glFramebufferTexture2D)
                  p_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2DEXT");
        }
    if (!p_glFramebufferTexture2D && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFramebufferTexture2D\n");
        result = 0;
    }

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fv");
        if (!p_glProgramUniform1fv)
        {
             p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvARB");
             if (!p_glProgramUniform1fv)
                  p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvEXT");
        }
    if (!p_glProgramUniform1fv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1fv\n");
        result = 0;
    }
    #endif

    p_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffers");
        if (!p_glDeleteRenderbuffers)
        {
             p_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffersARB");
             if (!p_glDeleteRenderbuffers)
                  p_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffersEXT");
        }
    if (!p_glDeleteRenderbuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteRenderbuffers\n");
        result = 0;
    }

    p_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SF_GL_RUNTIME_LINK("glBufferSubData");
        if (!p_glBufferSubData)
        {
             p_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SF_GL_RUNTIME_LINK("glBufferSubDataARB");
             if (!p_glBufferSubData)
                  p_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SF_GL_RUNTIME_LINK("glBufferSubDataEXT");
        }
    if (!p_glBufferSubData && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBufferSubData\n");
        result = 0;
    }

    p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgram");
        if (!p_glDeleteProgram)
        {
             p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgramARB");
             if (!p_glDeleteProgram)
                  p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgramEXT");
        }
    if (!p_glDeleteProgram && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteProgram\n");
        result = 0;
    }

    p_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffers");
        if (!p_glDeleteFramebuffers)
        {
             p_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffersARB");
             if (!p_glDeleteFramebuffers)
                  p_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffersEXT");
        }
    if (!p_glDeleteFramebuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteFramebuffers\n");
        result = 0;
    }

    #if defined(GL_ARB_map_buffer_range)
    p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRange");
        if (!p_glFlushMappedBufferRange)
        {
             p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRangeARB");
             if (!p_glFlushMappedBufferRange)
                  p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRangeEXT");
        }
    if (!p_glFlushMappedBufferRange && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glFlushMappedBufferRange\n");
        result = 0;
    }
    #endif

    #if defined(GL_VERSION_3_0)
    p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringi");
        if (!p_glGetStringi)
        {
             p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringiARB");
             if (!p_glGetStringi)
                  p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringiEXT");
        }
    if (!p_glGetStringi && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetStringi\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelines");
        if (!p_glDeleteProgramPipelines)
        {
             p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelinesARB");
             if (!p_glDeleteProgramPipelines)
                  p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelinesEXT");
        }
    if (!p_glDeleteProgramPipelines && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteProgramPipelines\n");
        result = 0;
    }
    #endif

    p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgram");
        if (!p_glIsProgram)
        {
             p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgramARB");
             if (!p_glIsProgram)
                  p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgramEXT");
        }
    if (!p_glIsProgram && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glIsProgram\n");
        result = 0;
    }

    p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgram");
        if (!p_glUseProgram)
        {
             p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgramARB");
             if (!p_glUseProgram)
                  p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgramEXT");
        }
    if (!p_glUseProgram && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUseProgram\n");
        result = 0;
    }

    p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLog");
        if (!p_glGetProgramInfoLog)
        {
             p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLogARB");
             if (!p_glGetProgramInfoLog)
                  p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLogEXT");
        }
    if (!p_glGetProgramInfoLog && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetProgramInfoLog\n");
        result = 0;
    }

    p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShader");
        if (!p_glDeleteShader)
        {
             p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShaderARB");
             if (!p_glDeleteShader)
                  p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShaderEXT");
        }
    if (!p_glDeleteShader && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteShader\n");
        result = 0;
    }

    p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffers");
        if (!p_glDeleteBuffers)
        {
             p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffersARB");
             if (!p_glDeleteBuffers)
                  p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffersEXT");
        }
    if (!p_glDeleteBuffers && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDeleteBuffers\n");
        result = 0;
    }

    p_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glBindFramebuffer");
        if (!p_glBindFramebuffer)
        {
             p_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glBindFramebufferARB");
             if (!p_glBindFramebuffer)
                  p_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glBindFramebufferEXT");
        }
    if (!p_glBindFramebuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindFramebuffer\n");
        result = 0;
    }

    p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSource");
        if (!p_glShaderSource)
        {
             p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSourceARB");
             if (!p_glShaderSource)
                  p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSourceEXT");
        }
    if (!p_glShaderSource && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glShaderSource\n");
        result = 0;
    }

    p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBuffer");
        if (!p_glBindBuffer)
        {
             p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBufferARB");
             if (!p_glBindBuffer)
                  p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBufferEXT");
        }
    if (!p_glBindBuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindBuffer\n");
        result = 0;
    }

    p_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glIsFramebuffer");
        if (!p_glIsFramebuffer)
        {
             p_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glIsFramebufferARB");
             if (!p_glIsFramebuffer)
                  p_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glIsFramebufferEXT");
        }
    if (!p_glIsFramebuffer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glIsFramebuffer\n");
        result = 0;
    }

    p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLog");
        if (!p_glGetShaderInfoLog)
        {
             p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLogARB");
             if (!p_glGetShaderInfoLog)
                  p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLogEXT");
        }
    if (!p_glGetShaderInfoLog && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetShaderInfoLog\n");
        result = 0;
    }

    p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fv");
        if (!p_glUniformMatrix4fv)
        {
             p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fvARB");
             if (!p_glUniformMatrix4fv)
                  p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fvEXT");
        }
    if (!p_glUniformMatrix4fv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniformMatrix4fv\n");
        result = 0;
    }

    p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArray");
        if (!p_glEnableVertexAttribArray)
        {
             p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArrayARB");
             if (!p_glEnableVertexAttribArray)
                  p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArrayEXT");
        }
    if (!p_glEnableVertexAttribArray && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glEnableVertexAttribArray\n");
        result = 0;
    }

    p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointer");
        if (!p_glVertexAttribPointer)
        {
             p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerARB");
             if (!p_glVertexAttribPointer)
                  p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerEXT");
        }
    if (!p_glVertexAttribPointer && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glVertexAttribPointer\n");
        result = 0;
    }

    p_glUniform2f = (PFNGLUNIFORM2FPROC) SF_GL_RUNTIME_LINK("glUniform2f");
        if (!p_glUniform2f)
        {
             p_glUniform2f = (PFNGLUNIFORM2FPROC) SF_GL_RUNTIME_LINK("glUniform2fARB");
             if (!p_glUniform2f)
                  p_glUniform2f = (PFNGLUNIFORM2FPROC) SF_GL_RUNTIME_LINK("glUniform2fEXT");
        }
    if (!p_glUniform2f && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform2f\n");
        result = 0;
    }

    p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocation");
        if (!p_glGetUniformLocation)
        {
             p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocationARB");
             if (!p_glGetUniformLocation)
                  p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocationEXT");
        }
    if (!p_glGetUniformLocation && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glGetUniformLocation\n");
        result = 0;
    }

    #if defined(GL_ARB_debug_output)
    p_glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC) SF_GL_RUNTIME_LINK("glDebugMessageControlARB");
    if (!p_glDebugMessageControlARB && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glDebugMessageControlARB\n");
        result = 0;
    }
    #endif

    #if defined(GL_ARB_separate_shader_objects)
    p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1iv");
        if (!p_glProgramUniform1iv)
        {
             p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivARB");
             if (!p_glProgramUniform1iv)
                  p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivEXT");
        }
    if (!p_glProgramUniform1iv && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glProgramUniform1iv\n");
        result = 0;
    }
    #endif

    p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fv");
        if (!p_glUniform3fv)
        {
             p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fvARB");
             if (!p_glUniform3fv)
                  p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fvEXT");
        }
    if (!p_glUniform3fv && true)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glUniform3fv\n");
        result = 0;
    }

    #if defined(GL_ARB_vertex_array_object)
    p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArray");
        if (!p_glBindVertexArray)
        {
             p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArrayARB");
             if (!p_glBindVertexArray)
                  p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArrayEXT");
        }
    if (!p_glBindVertexArray && false)
    {
        SF_DEBUG_WARNING(1, "Required runtime link function was not found: glBindVertexArray\n");
        result = 0;
    }
    #endif

    return result;
}

}}}

#endif
