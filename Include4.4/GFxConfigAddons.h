/**************************************************************************

PublicHeader:   None
Filename    :   GFxConfigAddons.h
Content     :   GFx configuration file - contains #ifdefs for
                the optional components of the library managed by
                installers
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define GFX_USE_VIDEO_WIN32
#define GFX_USE_VIDEO_XBOX360
#define GFX_USE_VIDEO_PS3
#define GFX_USE_VIDEO_XBOXONE
#define GFX_USE_VIDEO_PS4
#define GFX_USE_VIDEO_WIIU

#ifdef SF_OS_WIN32
	#define SF_ENABLE_IME
	#define SF_ENABLE_IME_WIN32
#endif
//#define SF_ENABLE_IME_XBOX360

// #if defined(SF_BUILD_DEBUG) || defined(SF_BUILD_DEBUGOPT)
//     #define SF_FPE_ENABLE
// #endif
