/**************************************************************************

Filename    :   FxAppImpl.cpp
Content     :   
Created     :   April 24, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxAppImpl.h"

#if defined(GFC_OS_XBOX360)
    #include <Direct3DXbox360App.cpp>
#elif defined(GFC_OS_WIN32)
    #include <FxWin32App.cpp>

    #ifdef FXPLAYER_RENDER_DIRECT3D
    #include <Direct3DWin32App.cpp>
    #else
    #include <OpenGLWin32App.cpp>
    #endif
#elif defined(GFC_OS_PS2)
    #include <PS2App.cpp>
#elif defined(GFC_OS_PS3)
    #include <PS3App.cpp>
    #include <GcmPS3App.cpp>
    //#include <OpenGLPS3App.cpp>
#elif defined(GFC_OS_WII)
    #include <WiiApp.cpp>
#elif defined(GFC_OS_MAC)
    #include <OpenGLMacApp.mm>
#elif defined(GFC_OS_LINUX)
    #include <OpenGLX11App.cpp>
#endif
