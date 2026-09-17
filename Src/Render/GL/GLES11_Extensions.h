/**********************************************************************

Filename    :   GLES11_Extensions.h
Content     :   GL ES11 extension header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GLES11_Extensions_H
#define INC_SF_Render_GLES11_Extensions_H

#include "Kernel/SF_Debug.h"
#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
    PFNGLBLENDFUNCSEPARATEOESPROC p_glBlendFuncSeparateOES;
    PFNGLUNMAPBUFFEROESPROC       p_glUnmapBufferOES;
    PFNGLGENERATEMIPMAPOESPROC    p_glGenerateMipmapOES;
    PFNGLMAPBUFFEROESPROC         p_glMapBufferOES;
    PFNGLBLENDEQUATIONOESPROC     p_glBlendEquationOES;
public:
    bool Init();
    
    class ScopedGLErrorCheck
    {
    public:
        ScopedGLErrorCheck(const char* func) : FuncName(func)
        {
        #ifdef SF_BUILD_DEBUG
            GLenum error = glGetError(); SF_UNUSED(error);
            SF_DEBUG_ASSERT2(!error, "GL error before function %s (error code = %d)\n", FuncName, error );
        #endif
        }
        ~ScopedGLErrorCheck()
        {
        #ifdef SF_BUILD_DEBUG
            GLenum error = glGetError(); SF_UNUSED(error);
            SF_DEBUG_ASSERT2(!error, "GL error after function %s (error code = %d)\n", FuncName, error );
        #endif
        }
    private:
        const char * FuncName;
    };
    void glBlendFuncSeparateOES(GLenum a0, GLenum a1, GLenum a2, GLenum a3)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        p_glBlendFuncSeparateOES(a0, a1, a2, a3);
    }

    GLboolean glUnmapBufferOES(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        return p_glUnmapBufferOES(a0);
    }

    void glGenerateMipmapOES(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        p_glGenerateMipmapOES(a0);
    }

    void* glMapBufferOES(GLenum a0, GLenum a1)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        return p_glMapBufferOES(a0, a1);
    }

    void glBlendEquationOES(GLenum a0)
    {
        ScopedGLErrorCheck check(__FUNCTION__);
        p_glBlendEquationOES(a0);
    }

};

}}}
#endif
