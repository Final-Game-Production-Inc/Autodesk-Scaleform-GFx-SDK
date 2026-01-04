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
#include "Render/Render_Shader.h"

#if defined(SF_OS_IPHONE)
    #define SF_RENDER_GLES
    #include <Availability.h>

    #if defined(__IPHONE_7_0)
        #include <OpenGLES/ES3/gl.h>
        #include <OpenGLES/ES3/glext.h>
        #include <OpenGLES/ES2/glext.h> 
   #else
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#elif defined(SF_OS_ANDROID)
    #define SF_RENDER_GLES
    #include <android/api-level.h>
    #if !defined(__ANDROID_API__)
        #error Expected __ANDROID_API__ to be defined within android/api-level.h.
    #endif

    // Check for GLES3 support (optional).
    #if __ANDROID_API__ >= 18
        // NOTE: gl3.h is included, but libGLESv3.so is not linked with, all functions are obtained through EGL at runtime.
        // It doesn't appear that all devices include the library, even if they claim support for GLES3, causing an
        // UnsatisfiedLinkError at runtime when launching.
        #include <GLES3/gl3.h>      
        #include <GLES3/gl3ext.h>
        #include <GLES2/gl2ext.h>   // NOTE: GLES 3.0 is backwards compatible with gl2ext.h, so both gl3ext.h and gl2ext.h can both be included
    #elif __ANDROID_API__ >= 8
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
    #else
        #error Minimum supported GLES version is 2.0, which requires Android API level 8.
    #endif


    #if defined(SF_USE_EGL)
      #include <EGL/egl.h>
      #define SF_GL_RUNTIME_LINK(x) eglGetProcAddress(x)
    #else
      #define GL_GLEXT_PROTOTYPES
    #endif
#elif defined(SF_OS_WIN32)
    #define SF_RENDER_OPENGL
    #include <windows.h>
	// JWB - Clean up after windows.h
    #undef GetObject
    #undef LoadImage
    #undef CopyFile
    #undef CreateFont
    #undef GetClassInfo
	#undef GetFileAttributes
    #include <gl/gl.h>
    #include "glext.h"
    #define SF_GL_RUNTIME_LINK(x) wglGetProcAddress(x)
#elif defined(SF_OS_MAC)
    #define SF_RENDER_OPENGL
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

    #define SF_RENDER_OPENGL
    // Use GLX to link gl extensions at runtime; comment out if not using GLX
    #define SF_GLX_RUNTIME_LINK

    #ifdef SF_GLX_RUNTIME_LINK
        #include <GL/glx.h>
        #include <GL/glxext.h>
        #define SF_GL_RUNTIME_LINK(x) glXGetProcAddressARB((const GLubyte*) (x))

        // NOTE: Xlib.h #defines Status, and does not undef it. This causes problems
        // when compiling Scaleform, so undefine it immediately after its include (via glx.h)
        #ifdef Status
            #undef Status
        #endif
    #else
        #define GL_GLEXT_PROTOTYPES
    #endif
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

// Make sure GL_APIENTRY is defined (even if it is defined to be nothing)
#if !defined(GL_APIENTRY)
    #if defined(APIENTRY)
        #define GL_APIENTRY APIENTRY
    #else
        #define GL_APIENTRY 
    #endif
#endif

// Make sure GL_APIENTRYP is defined. This is what is used by default as the linkage in
// GL_Extensions.h, however, the exact name varies from platform to platform.
#if !defined(GL_APIENTRYP) 
    #if defined(APIENTRYP)
        // Add the GL_ prefix.
        #define GL_APIENTRYP APIENTRYP
    #elif defined(GL_APIENTRY)
        // Function pointer type is not defined.
        #define GL_APIENTRYP GL_APIENTRY*
    #elif defined(APIENTRY)
        // Function pointer type is not defined, and needs prefix.
        #define GL_APIENTRYP APIENTRY*
    #else
        // Just guess that there is no required linkage.
        #define GL_APIENTRYP *
    #endif
#endif

// Make sure that exactly one of GL_RENDER_GLES or GL_RENDER_OPENGL is defined
#if !defined(SF_RENDER_GLES) && !defined(SF_RENDER_OPENGL)
    #error Neither SF_RENDER_GLES or SF_RENDER_OPENGL is defined. Exactly one must be defined.
#elif defined(SF_RENDER_GLES) && defined(SF_RENDER_OPENGL)
    #error Both SF_RENDER_GLES and SF_RENDER_OPENGL are defined. Exactly one must be defined.
#endif

// Helper macro to identify GLES30 support
#if defined(SF_RENDER_GLES) && defined(GL_ES_VERSION_3_0) && GL_ES_VERSION_3_0 != 0
    #define SF_RENDER_GLES30_SUPPORT
#else
#endif

// Some data types need to be defined (if they are not already), before the extension functions are included,
// because the prototypes contain these types, and thus will have compile errors.
#if defined(SF_RENDER_GLES)
    #if !defined(GL_APPLE_sync) && !defined(SF_RENDER_GLES30_SUPPORT)
        typedef void*       GLsync;
        typedef uint64_t    GLuint64;
    #endif

    // Android's gl2.h doesn't have this.
#if defined(SF_OS_ANDROID)
    typedef khronos_int64_t GLint64;
#endif

    // GL_KHR_debug
    #define GLDEBUGPROCKHR  GLDEBUGPROC
    #if !defined(GL_KHR_debug)
        typedef void*   GLDEBUGPROC;
    #endif

    // glShaderSource 3rd parameter. iOS defines it the Khronos way, Android does not.
    #if defined(SF_OS_IPHONE)
        #define GLSHADERSOURCEA2TYPE const GLchar*const*
    #else
        #define GLSHADERSOURCEA2TYPE const GLchar**
    #endif
#endif

// For OpenGL:
#if defined(SF_RENDER_OPENGL)
    // GL_ARB_debug_output
    #define GLDEBUGPROCKHR GLDEBUGPROCARB
    #define GLDEBUGPROC    GLDEBUGPROCARB
    #if !defined(GL_ARB_debug_output)
            typedef void* GLDEBUGPROCARB;
    #endif
    typedef void *GLeglImageOES;

    // glShaderSource 3rd parameter. In GLEXT_VERSION 81, the prototype changed.
    // On Mac, GL_GLEXT_VERSION is incorrect for the version of glext.h in Xcode.
    #if (defined(GL_GLEXT_VERSION) && GL_GLEXT_VERSION >= 81) || defined(SF_OS_MAC)
        #define GLSHADERSOURCEA2TYPE const GLchar*const*
    #else
        #define GLSHADERSOURCEA2TYPE const GLchar**
    #endif
#endif

// Now include the extension definitions
#include "Render/GL/GL_Extensions.h"

namespace Scaleform { namespace Render { namespace GL {

// GL capability flags, as determined after InitHAL has been called.
enum CapFlags
{
    Cap_Align           = MVF_Align,

    Cap_NoBatching      = 0x0010,   // Not capable of doing batching.

    // Caps for buffers in mesh cache. Client buffers will be used if none of these caps are set (eg. Caps & Cap_UseMeshBuffer == 0).
    Cap_MapBuffer       = 0x0020,    // glMapBuffer is available.
    Cap_MapBufferRange  = 0x0040,    // glMapBufferRange is available.
    Cap_BufferUpdate    = 0x0080,	 // glBufferData/glBufferSubData is available.
    Cap_UseMeshBuffers  = Cap_MapBuffer | Cap_MapBufferRange | Cap_BufferUpdate,

    // Caps for shaders. 
    Cap_NoDynamicLoops  = 0x0100,    // Profile does not support dynamic looping.
    Cap_BinaryShaders   = 0x0200,    // Profile supports loading binary shaders
    Cap_NoDerivatives   = 0x0400,    // Profile does not support use of derivatives in fragment shaders (dFdx/dFdy).

    // Caps for drawing.
    Cap_Instancing      = 0x0800,    // Profile supports instancing (DrawArraysInstanced, and GLSL1.4+).

    Cap_NoVAO           = 0x1000,    // Not capable of using VAOs

    Cap_Sync            = 0x2000,    // Profile supports fence objects (GL_ARB_sync or GL_APPLE_sync).
    Cap_FBORedundancy   = 0x4000,    // Perform glBindFramebuffer calls, even if they are redundant. Some drivers do not deal well if this doesn't happen.

    // GL_MAX_VERTEX_UNIFORM_VECTORS, or a different value on certain devices
    Cap_MaxUniforms       = 0xffff0000,
    Cap_MaxUniforms_Shift = 16,
};

}}} // Scaleform::Render::GL

#endif
