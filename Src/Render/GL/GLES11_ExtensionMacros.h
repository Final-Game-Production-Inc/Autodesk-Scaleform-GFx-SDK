/**********************************************************************

Filename    :   GLES11_ExtensionMacros.h
Content     :   GL ES11 extension macros header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2011 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GLES11_ExtensionMacros_H
#define INC_SF_Render_GLES11_ExtensionMacros_H
#include "Render/GL/GLES11_HAL.h"

#ifdef SF_GL_RUNTIME_LINK

#define glBlendFuncSeparateOES GetHAL()->glBlendFuncSeparateOES
#define glUnmapBufferOES GetHAL()->glUnmapBufferOES
#define glGenerateMipmapOES GetHAL()->glGenerateMipmapOES
#define glMapBufferOES GetHAL()->glMapBufferOES
#define glBlendEquationOES GetHAL()->glBlendEquationOES

#endif
#endif
