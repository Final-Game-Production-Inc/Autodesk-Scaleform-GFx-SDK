/**************************************************************************

Filename    :   FxAppImpl.h
Content     :   
Created     :   April 24, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_FXAPPIMPL_H
#define INC_FXAPPIMPL_H

#include <GTypes.h>

#if defined(GFC_OS_XBOX360)
#include <Direct3DXbox360App.h>
#elif defined(GFC_OS_WIN32)
    #ifdef FXPLAYER_RENDER_DIRECT3D
    #include <Direct3DWin32App.h>
    #else
    #include <OpenGLWin32App.h>
    #endif
#elif defined(GFC_OS_PS2)
#include <PS2App.h>
#elif defined(GFC_OS_PS3)
#include <GcmPS3App.h>
//#include <OpenGLPS3App.h>
#elif defined(GFC_OS_WII)
#include <WiiApp.h>
#elif defined(GFC_OS_MAC)
#include <OpenGLMacApp.h>
#elif defined(GFC_OS_LINUX)
#include <OpenGLX11App.h>
#endif

#endif // INC_FXAPPIMPL_H

