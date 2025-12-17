/**************************************************************************

Filename    :   GL_Common.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GL_Common_H
#define INC_SF_Render_GL_Common_H

#include "Kernel/SF_Types.h"

#if defined(SF_OS_IPHONE)
    #ifdef SF_USE_GLES
        #include <OpenGLES/ES1/gl.h>
        #include <OpenGLES/ES1/glext.h>
    #else
        #ifndef SF_USE_GLES2
            #define SF_USE_GLES2
        #endif
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#elif defined(SF_USE_GLES)
    #include <GLES/gl.h>
    #if defined(SF_USE_EGL)
      #include <EGL/egl.h>
      #define SF_GL_RUNTIME_LINK(x) eglGetProcAddress(x)
    #else
      #define GL_GLEXT_PROTOTYPES
    #endif
    #include <GLES/glext.h>
#elif defined(SF_USE_GLES2)
    #include <GLES2/gl2.h>
    #if defined(SF_USE_EGL)
      #include <EGL/egl.h>
      #define SF_GL_RUNTIME_LINK(x) eglGetProcAddress(x)
    #else
      #define GL_GLEXT_PROTOTYPES
    #endif
    #include <GLES2/gl2ext.h>
#elif defined(SF_OS_WIN32)
    #include <windows.h>
    #include <gl/gl.h>
    #include "glext.h"
    #define SF_GL_RUNTIME_LINK(x) wglGetProcAddress(x)
#elif defined(SF_OS_MAC)
    #include <OpenGL/OpenGL.h>
    #define GL_GLEXT_PROTOTYPES
    #if defined(MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7)
        #include <OpenGL/gl3.h>
        #include <OpenGL/gl3ext.h>
    #else
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
    #endif
#else

    // Use GLX to link gl extensions at runtime; comment out if not using GLX
    #define SF_GLX_RUNTIME_LINK

    #ifdef SF_GLX_RUNTIME_LINK
        #include <GL/glx.h>
        #include <GL/glxext.h>
        #define SF_GL_RUNTIME_LINK(x) glXGetProcAddressARB((const GLubyte*) (x))
    #else
        #define GL_GLEXT_PROTOTYPES
    #endif
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

// Setup the calling convention for callback functions (mostly applies to Win32).
#if !defined(GL_APIENTRY)
    #ifdef APIENTRY
        #define GL_APIENTRY APIENTRY
    #else
        #define GL_APIENTRY
    #endif
#endif

// Helper macro for any GLES (vs. OpenGL).
#if defined(SF_USE_GLES) || defined(SF_USE_GLES2)
    #define SF_USE_GLES_ANY
#else
    #define SF_USE_OPENGL
#endif

// Some data types need to be defined (if they are not already), before the extension functions are included,
// because the prototypes contain these types, and thus will have compile errors. For GLES 2.0:
#if defined(SF_USE_GLES2)
    // GL_APPLE_sync
    #if !defined(GL_APPLE_sync)
        #define GLsync                                  void*
        #define GLuint64                                uint64_t
    #endif
    // GL_KHR_debug
    #if !defined(GL_KHR_debug)
        #define GLDEBUGPROC                             void*
    #endif
#endif

// For OpenGL:
#if !defined(SF_USE_GLES_ANY)
    // GL_ARB_debug_output
    #if !defined(GL_ARB_debug_output)
        #define GLDEBUGPROCARB                          void*
    #endif
#endif

// Now include the extension definitions
#ifdef SF_GL_RUNTIME_LINK
    #if defined(SF_USE_GLES)
        #include "Render/GL/GLES11_Extensions.h"
    #elif defined(SF_USE_GLES2)
        #include "Render/GL/GLES_Extensions.h"
    #else
        #include "Render/GL/GL_Extensions.h"
    #endif
    #define GLEXT GetHAL()->
#else
  #define GLEXT
#endif

#if defined(SF_USE_GLES2) || defined(SF_USE_OPENGL)

    // ES 2.0 specific
    #if defined(SF_USE_GLES2)
        // Unsupported features
        #define GL_MIN                                  GL_FUNC_ADD
        #define GL_MAX                                  GL_FUNC_ADD

        // Built-in in GL2.x, extensions in ES2.0
        #define GL_WRITE_ONLY                           GL_WRITE_ONLY_OES

        #define glMapBuffer                             glMapBufferOES
        #define glUnmapBuffer                           glUnmapBufferOES

        // Removed in iOS7 SDK.
        #if !defined(GL_STENCIL_INDEX)
            #define GL_STENCIL_INDEX                    GL_STENCIL_INDEX8
        #endif

        // GL_OES_packed_depth_stencil
        #if !defined(GL_OES_packed_depth_stencil)
            #define GL_DEPTH_STENCIL                        0
            #define GL_DEPTH24_STENCIL8                     0
            #define GL_UNSIGNED_INT_24_8                    0
        #else
            #define GL_DEPTH_STENCIL                        GL_DEPTH_STENCIL_OES
            #define GL_DEPTH24_STENCIL8                     GL_DEPTH24_STENCIL8_OES
            #define GL_UNSIGNED_INT_24_8                    GL_UNSIGNED_INT_24_8_OES
        #endif
    #endif // GL_ES_2_0


    // GL_OES_get_program_binary
    #if (defined(SF_USE_GLES2) && !defined(GL_OES_get_program_binary)) || (defined(SF_USE_OPENGL) && !defined(GL_ARB_get_program_binary))
        #define GL_PROGRAM_BINARY_LENGTH                0
        #define glProgramBinary                         { SF_DEBUG_ASSERT(0, "glProgramBinary did not exist in glext.h"); }
        #define glGetProgramBinary                      { SF_DEBUG_ASSERT(0, "glGetProgramBinary did not exist in glext.h"); }
    #elif (defined(SF_USE_GLES2) && defined(GL_OES_get_program_binary))
        #define GL_PROGRAM_BINARY_LENGTH                GL_PROGRAM_BINARY_LENGTH_OES
        #define glProgramBinary                         glProgramBinaryOES
        #define glGetProgramBinary                      glGetProgramBinaryOES
    #endif

    // GL_ARB_sync/GL_APPLE_sync
    #if (defined(SF_USE_GLES2) && !defined(GL_APPLE_sync)) || (defined(SF_USE_OPENGL) && !defined(GL_ARB_sync))
        #define GL_SYNC_STATUS                          0
        #define GL_UNSIGNALED                           0
        #define GL_SYNC_GPU_COMMANDS_COMPLETE           0
        #define GL_TIMEOUT_IGNORED                      0
        #define glFenceSync(...)                        0; SF_DEBUG_ASSERT(0, "glFenceSync did not exist in glext.h");
        #define glGetSynciv(...)                        { SF_DEBUG_ASSERT(0, "glGetSynciv did not exist in glext.h"); }
        #define glClientWaitSync(...)                   0; SF_DEBUG_ASSERT(0, "glClientWaitSync did not exist in glext.h");
        #define glDeleteSync(...)                       { SF_DEBUG_ASSERT(0, "glDeleteSync did not exist in glext.h"); }
    #elif (defined(SF_USE_GLES2) && defined(GL_APPLE_sync))
        #define GL_SYNC_STATUS                          GL_SYNC_STATUS_APPLE
        #define GL_UNSIGNALED                           GL_UNSIGNALED_APPLE
        #define GL_SYNC_GPU_COMMANDS_COMPLETE           GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE
        #define GL_TIMEOUT_IGNORED                      GL_TIMEOUT_IGNORED_APPLE
        #define glFenceSync                             glFenceSyncAPPLE
        #define glGetSynciv                             glGetSyncivAPPLE
        #define glClientWaitSync                        glClientWaitSyncAPPLE
        #define glDeleteSync                            glDeleteSyncAPPLE
    #endif // GL_APPLE_sync

    // GL_EXT_map_buffer_range
    #if (defined(SF_USE_GLES2)  && !defined(GL_EXT_map_buffer_range)) || (defined(SF_USE_OPENGL) && !defined(GL_ARB_map_buffer_range))
        #define GL_MAP_WRITE_BIT                        0
        #define GL_MAP_FLUSH_EXPLICIT_BIT               0
        #define GL_MAP_UNSYNCHRONIZED_BIT               0
        #define glFlushMappedBufferRange(...)           { SF_DEBUG_ASSERT(0, "glFlushMappedBufferRange did not exist in glext.h"); }
        #define glMapBufferRange(...)                   0; SF_DEBUG_ASSERT(0, "glMapBufferRange did not exist in glext.h");
    #elif defined(SF_USE_GLES2)  && defined(GL_EXT_map_buffer_range)
        #define GL_MAP_WRITE_BIT                        GL_MAP_WRITE_BIT_EXT
        #define GL_MAP_FLUSH_EXPLICIT_BIT               GL_MAP_FLUSH_EXPLICIT_BIT_EXT
        #define GL_MAP_UNSYNCHRONIZED_BIT               GL_MAP_UNSYNCHRONIZED_BIT_EXT
        #define glFlushMappedBufferRange                glFlushMappedBufferRangeEXT
        #define glMapBufferRange                        glMapBufferRangeEXT
    #endif

    // GL_OES_vertex_array_object. Note: Android defines this in gl2ext.h, but its library does not actually contain the functions.
    #if (defined(SF_USE_GLES2) && !defined(GL_OES_vertex_array_object) || (defined(SF_OS_ANDROID) && !defined(SF_GL_RUNTIME_LINK))) || (defined(SF_USE_OPENGL) && !defined(GL_ARB_vertex_array_object))
        #define glBindVertexArray(...)                  { SF_DEBUG_ASSERT(0, "glBindVertexArray did not exist in glext.h"); }
        #define glDeleteVertexArrays(...)               { SF_DEBUG_ASSERT(0, "glDeleteVertexArrays did not exist in glext.h"); }
        #define glGenVertexArrays(...)                  { SF_DEBUG_ASSERT(0, "glGenVertexArrays did not exist in glext.h"); }
    #elif defined(SF_USE_GLES2) && defined(GL_OES_vertex_array_object)
        #define glBindVertexArray                       glBindVertexArrayOES
        #define glDeleteVertexArrays                    glDeleteVertexArraysOES
        #define glGenVertexArrays                       glGenVertexArraysOES
    #endif // GL_OES_vertex_array_object/GL_ARB_vertex_array_object

    // GL_ARB_draw_instanced. Not supported on GLES, became standard in GL 3.1, and can be used with GL_ARB_draw_instanced.
    #if defined(SF_USE_GLES_ANY) || (!defined(GL_VERSION_3_1) && !defined(GL_ARB_draw_instanced))
        #define glDrawElementsInstanced(...)            { SF_DEBUG_ASSERT(0, "glDrawElementsInstanced did not exist in glext.h"); }
    #elif defined(SF_USE_OPENGL) && !defined(GL_VERSION_3_1) && defined(GL_ARB_draw_instanced)
        #define glDrawElementsInstanced                 glDrawElementsInstancedARB
    #endif // GL_ARB_draw_instanced

    // GL_EXT_separate_shader_objects. Note that there is a GL_EXT_separate_shader_objects for OpenGL, it is a similar extension,
    // however, it does not provide all the functionality that GL_ARB_separate_shader_objects does, and is not supported.
    #if (defined(SF_USE_GLES2) && !defined(GL_EXT_separate_shader_objects)) || !defined(GL_ARB_separate_shader_objects)
        #define GL_VERTEX_SHADER_BIT                    0
        #define GL_FRAGMENT_SHADER_BIT                  0
        #define GL_GEOMETRY_SHADER_BIT                  0
        #define GL_TESS_CONTROL_SHADER_BIT              0
        #define GL_TESS_EVALUATION_SHADER_BIT           0
        #define GL_ALL_SHADER_BITS                      0
        #define GL_PROGRAM_SEPARABLE                    0
        #define GL_ACTIVE_PROGRAM                       0
        #define GL_PROGRAM_PIPELINE_BINDING             0
        #define glGenProgramPipelines(...)              { SF_DEBUG_ASSERT(0, "glGenProgramPipelines did not exist in glext.h"); }
        #define glDeleteProgramPipelines(...)           { SF_DEBUG_ASSERT(0, "glDeleteProgramPipelines did not exist in glext.h"); }
        #define glUseProgramStages(...)                 { SF_DEBUG_ASSERT(0, "glUseProgramStages did not exist in glext.h"); }
        #define glGetProgramPipelineInfoLog(...)        { SF_DEBUG_ASSERT(0, "glGetProgramPipelineInfoLog did not exist in glext.h"); }
        #define glGetProgramPipelineiv(...)             { SF_DEBUG_ASSERT(0, "glGetProgramPipelineiv did not exist in glext.h"); }
        #define glProgramParameteri(...)                { SF_DEBUG_ASSERT(0, "glProgramParameteri did not exist in glext.h"); }
        #define glBindProgramPipeline(...)              { SF_DEBUG_ASSERT(0, "glBindProgramPipeline did not exist in glext.h"); }
    #elif (defined(GL_EXT_separate_shader_objects) && defined(SF_USE_GLES2))
        #define GL_VERTEX_SHADER_BIT                    GL_VERTEX_SHADER_BIT_EXT
        #define GL_FRAGMENT_SHADER_BIT                  GL_FRAGMENT_SHADER_BIT_EXT
        #define GL_GEOMETRY_SHADER_BIT                  GL_GEOMETRY_SHADER_BIT_EXT
        #define GL_TESS_CONTROL_SHADER_BIT              GL_TESS_CONTROL_SHADER_BIT_EXT
        #define GL_TESS_EVALUATION_SHADER_BIT           GL_TESS_EVALUATION_SHADER_BIT_EXT
        #define GL_ALL_SHADER_BITS                      GL_ALL_SHADER_BITS_EXT
        #define GL_PROGRAM_SEPARABLE                    GL_PROGRAM_SEPARABLE_EXT
        #define GL_ACTIVE_PROGRAM                       GL_ACTIVE_PROGRAM_EXT
        #define GL_PROGRAM_PIPELINE_BINDING             GL_PROGRAM_PIPELINE_BINDING_EXT
        #define glGenProgramPipelines                   glGenProgramPipelinesEXT
        #define glDeleteProgramPipelines                glDeleteProgramPipelinesEXT
        #define glUseProgramStages                      glUseProgramStagesEXT
        #define glGetProgramPipelineInfoLog             glGetProgramPipelineInfoLogEXT
        #define glGetProgramPipelineiv                  glGetProgramPipelineivEXT
        #define glProgramParameteri                     glProgramParameteriEXT
        #define glBindProgramPipeline                   glBindProgramPipelineEXT
    #endif // GL_EXT_separate_shader_objects

    // glProgramUniform* are defined in EXT_separate_shader_objects (GLES2) and EXT_direct_state_access (OpenGL), with EXT suffix.
    // They are also defined within GL_ARB_seprate_shader_objects on OpenGL, but they have no EXT suffix.
    #if (defined(SF_USE_GLES2) && !defined(GL_EXT_separate_shader_objects)) || (!defined(GL_ARB_separate_shader_objects) && !defined(GL_EXT_direct_state_access))
        #define glProgramUniformMatrix4fv(...)          { SF_DEBUG_ASSERT(0, "glProgramUniformMatrix4fv did not exist in glext.h"); }
        #define glProgramUniform4fv(...)                { SF_DEBUG_ASSERT(0, "glProgramUniform4fv did not exist in glext.h"); }
        #define glProgramUniform3fv(...)                { SF_DEBUG_ASSERT(0, "glProgramUniform3fv did not exist in glext.h"); }
        #define glProgramUniform2fv(...)                { SF_DEBUG_ASSERT(0, "glProgramUniform2fv did not exist in glext.h"); }
        #define glProgramUniform1fv(...)                { SF_DEBUG_ASSERT(0, "glProgramUniform1fv did not exist in glext.h"); }
        #define glProgramUniform1iv(...)                { SF_DEBUG_ASSERT(0, "glProgramUniform1iv did not exist in glext.h"); }
    #elif (defined(GL_EXT_separate_shader_objects) && defined(SF_USE_GLES2)) || (defined(GL_EXT_direct_state_access) && !defined(GL_ARB_separate_shader_objects))
        #define glProgramUniformMatrix4fv               glProgramUniformMatrix4fvEXT
        #define glProgramUniform4fv                     glProgramUniform4fvEXT
        #define glProgramUniform3fv                     glProgramUniform3fvEXT
        #define glProgramUniform2fv                     glProgramUniform2fvEXT
        #define glProgramUniform1fv                     glProgramUniform1fvEXT
        #define glProgramUniform1iv                     glProgramUniform1ivEXT
    #endif

    // GL 3.0+ specific

    // These calls exist on other GL implementations, but are currently only hooked up for OpenGL.
    #if defined(GL_ARB_debug_output)
        #define GLDEBUGPROC                             GLDEBUGPROCARB
        #define glDebugMessageControl                   glDebugMessageControlARB
        #define glDebugMessageCallback                  glDebugMessageCallbackARB
        #define GL_DEBUG_OUTPUT_SYNCHRONOUS             GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB
        #define GL_MAX_DEBUG_MESSAGE_LENGTH             GL_MAX_DEBUG_MESSAGE_LENGTH_ARB
        #define GL_MAX_DEBUG_LOGGED_MESSAGES            GL_MAX_DEBUG_LOGGED_MESSAGES_ARB
        #define GL_DEBUG_LOGGED_MESSAGES                GL_DEBUG_LOGGED_MESSAGES_ARB
        #define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH     GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB
        #define GL_DEBUG_CALLBACK_FUNCTION              GL_DEBUG_CALLBACK_FUNCTION_ARB
        #define GL_DEBUG_CALLBACK_USER_PARAM            GL_DEBUG_CALLBACK_USER_PARAM_ARB
        #define GL_DEBUG_SOURCE_API                     GL_DEBUG_SOURCE_API_ARB
        #define GL_DEBUG_SOURCE_WINDOW_SYSTEM           GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB
        #define GL_DEBUG_SOURCE_SHADER_COMPILER         GL_DEBUG_SOURCE_SHADER_COMPILER_ARB
        #define GL_DEBUG_SOURCE_THIRD_PARTY             GL_DEBUG_SOURCE_THIRD_PARTY_ARB
        #define GL_DEBUG_SOURCE_APPLICATION             GL_DEBUG_SOURCE_APPLICATION_ARB
        #define GL_DEBUG_SOURCE_OTHER                   GL_DEBUG_SOURCE_OTHER_ARB
        #define GL_DEBUG_TYPE_ERROR                     GL_DEBUG_TYPE_ERROR_ARB
        #define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR       GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB
        #define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR        GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB
        #define GL_DEBUG_TYPE_PORTABILITY               GL_DEBUG_TYPE_PORTABILITY_ARB
        #define GL_DEBUG_TYPE_PERFORMANCE               GL_DEBUG_TYPE_PERFORMANCE_ARB
        #define GL_DEBUG_TYPE_OTHER                     GL_DEBUG_TYPE_OTHER_ARB
        #define GL_DEBUG_SEVERITY_HIGH                  GL_DEBUG_SEVERITY_HIGH_ARB
        #define GL_DEBUG_SEVERITY_MEDIUM                GL_DEBUG_SEVERITY_MEDIUM_ARB
        #define GL_DEBUG_SEVERITY_LOW                   GL_DEBUG_SEVERITY_LOW_ARB
    #elif !defined(GL_KHR_debug)
        #define glDebugMessageControl(...)              { SF_DEBUG_ASSERT(0, "glDebugMessageControl did not exist in glext.h"); }
        #define glDebugMessageCallback(...)             { SF_DEBUG_ASSERT(0, "glDebugMessageCallback did not exist in glext.h"); }
        #define GL_DEBUG_OUTPUT_SYNCHRONOUS             0
        #define GL_MAX_DEBUG_MESSAGE_LENGTH             0
        #define GL_MAX_DEBUG_LOGGED_MESSAGES            0
        #define GL_DEBUG_LOGGED_MESSAGES                0
        #define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH     0
        #define GL_DEBUG_CALLBACK_FUNCTION              0
        #define GL_DEBUG_CALLBACK_USER_PARAM            0
        #define GL_DEBUG_SOURCE_API                     0
        #define GL_DEBUG_SOURCE_WINDOW_SYSTEM           0
        #define GL_DEBUG_SOURCE_SHADER_COMPILER         0
        #define GL_DEBUG_SOURCE_THIRD_PARTY             0
        #define GL_DEBUG_SOURCE_APPLICATION             0
        #define GL_DEBUG_SOURCE_OTHER                   0
        #define GL_DEBUG_TYPE_ERROR                     0
        #define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR       0
        #define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR        0
        #define GL_DEBUG_TYPE_PORTABILITY               0
        #define GL_DEBUG_TYPE_PERFORMANCE               0
        #define GL_DEBUG_TYPE_OTHER                     0
        #define GL_DEBUG_SEVERITY_HIGH                  0
        #define GL_DEBUG_SEVERITY_MEDIUM                0
        #define GL_DEBUG_SEVERITY_LOW                   0
    #endif

    #if defined(GL_EXT_framebuffer_blit)
        #define glBlitFramebuffer                       glBlitFramebufferEXT
    #else
        #define glBlitFramebuffer(...)                  { SF_DEBUG_ASSERT(0, "glDebugMessageControlARB did not exist in glext.h"); }
    #endif

    #if !defined(GL_TEXTURE_COMPONENTS)
        #define GL_TEXTURE_COMPONENTS                   GL_TEXTURE_INTERNAL_FORMAT
    #endif

    // Convert standard enums to extension ones
    #if !defined(GL_ARB_framebuffer_object) && defined(GL_EXT_framebuffer_object)
        #define GL_FRAMEBUFFER                           GL_FRAMEBUFFER_EXT
        #define GL_FRAMEBUFFER_BINDING                   GL_FRAMEBUFFER_BINDING_EXT
        #define GL_FRAMEBUFFER_COMPLETE                  GL_FRAMEBUFFER_COMPLETE_EXT
        #define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT
        #define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT
        #define GL_FRAMEBUFFER_COMPLETE                  GL_FRAMEBUFFER_COMPLETE_EXT
        #define GL_RENDERBUFFER                          GL_RENDERBUFFER_EXT
        #define GL_RENDERBUFFER_BINDING                  GL_RENDERBUFFER_BINDING_EXT
        #define GL_RENDERBUFFER_WIDTH                    GL_RENDERBUFFER_WIDTH_EXT
        #define GL_RENDERBUFFER_HEIGHT                   GL_RENDERBUFFER_HEIGHT_EXT
        #define GL_COLOR_ATTACHMENT0                     GL_COLOR_ATTACHMENT0_EXT
        #define GL_DEPTH_ATTACHMENT                      GL_DEPTH_ATTACHMENT_EXT
        #define GL_STENCIL_ATTACHMENT                    GL_STENCIL_ATTACHMENT_EXT

        #define glGenRenderbuffers                       glGenRenderbuffersEXT
        #define glBindRenderbuffer                       glBindRenderbufferEXT
        #define glRenderbufferStorage                    glRenderbufferStorageEXT
        #define glGenFramebuffers                        glGenFramebuffersEXT
        #define glBindFramebuffer                        glBindFramebufferEXT
        #define glFramebufferTexture2D                   glFramebufferTexture2DEXT
        #define glIsFramebuffer                          glIsFramebufferEXT
        #define glGetFramebufferAttachmentParameteriv    glGetFramebufferAttachmentParameterivEXT
        #define glDeleteRenderbuffers                    glDeleteRenderbuffersEXT
        #define glIsRenderbuffer                         glIsRenderbufferEXT
        #define glGetRenderbufferParameteriv             glGetRenderbufferParameterivEXT
        #define glFramebufferRenderbuffer                glFramebufferRenderbufferEXT
        #define glDeleteFramebuffers                     glDeleteFramebuffersEXT
        #define glCheckFramebufferStatus                 glCheckFramebufferStatusEXT

        // in GLES2.0, glGenerateMipmap is not in the framebuffer extension, it is part of the standard.
        #if !defined(SF_USE_GLES2)
            #define glGenerateMipmap                         glGenerateMipmapEXT
        #endif
    #endif // GL_EXT_framebuffer_object

    #if !defined(GL_BGRA) && defined(GL_BGRA_EXT)
        #define GL_BGRA GL_BGRA_EXT
    #endif
    #if !defined(GL_BGRA_EXT) && defined(GL_BGRA)
        #define GL_BGRA_EXT GL_BGRA
    #endif

    typedef char GLchar;

    // Some GLES 2.0 devices do support S3TC texture compression, however, the extensions aren't included
    // in the glext.h on these platforms. Specifically, Tegra2 devices on Android. So, define the values 
    // here, if they are undefined, and check the extension at runtime to determine compatability.
    #ifndef GL_EXT_texture_compression_s3tc
        #define GL_EXT_texture_compression_s3tc                1
        #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
        #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
    #endif
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
    #endif
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
    #endif

#elif defined(SF_USE_GLES)

    // Unsupported features
    #define GL_MIN                   0
    #define GL_MAX                   0
    #define GL_FUNC_ADD              GL_ADD
    #define GL_FUNC_REVERSE_SUBTRACT 0

    // Convert extension enums to standard ones
    #define GL_WRITE_ONLY GL_WRITE_ONLY_OES
    #define glMapBuffer glMapBufferOES
    #define glUnmapBuffer glUnmapBufferOES
    #define glGenerateMipmap glGenerateMipmapOES

#else // SF_USE_GLES

    // Statically linking, with possible extensions.
    #if !defined SF_GL_RUNTIME_LINK
        #if defined(GL_APPLE_vertex_array_object) && GL_APPLE_vertex_array_object
            #define glBindVertexArray                           glBindVertexArrayAPPLE
            #define glGenVertexArrays                           glGenVertexArraysAPPLE
            #define glDeleteVertexArrays                        glDeleteVertexArraysAPPLE
        #endif

        #if defined(GL_ARB_instanced_arrays) && GL_ARB_instanced_arrays
            #define glDrawElementsInstanced                     glDrawElementsInstancedARB
        #endif
    #endif

#endif

#endif
