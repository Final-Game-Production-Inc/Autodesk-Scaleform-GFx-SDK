/**********************************************************************

Filename    :   GLES11_Extensions.cpp
Content     :   GL ES11 extension implementation.
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
    p_glBlendFuncSeparateOES = (PFNGLBLENDFUNCSEPARATEOESPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateOES");

    p_glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glUnmapBufferOES");

    p_glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOESPROC) SF_GL_RUNTIME_LINK("glGenerateMipmapOES");

    p_glMapBufferOES = (PFNGLMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glMapBufferOES");

    p_glBlendEquationOES = (PFNGLBLENDEQUATIONOESPROC) SF_GL_RUNTIME_LINK("glBlendEquationOES");

    return result;
}

}}}

#endif
