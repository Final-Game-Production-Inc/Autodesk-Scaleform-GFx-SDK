/**************************************************************************

PublicHeader:   None
Filename    :   GFxConfigIME.h
Content     :   GFx configuration file - contains #ifdefs for
                the optional components of the library managed by
                installers
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software, PLEASE OBEY GPL-3.0 LICENSE.

**************************************************************************/

#ifdef SF_OS_WIN32
	#define SF_ENABLE_IME
	#define SF_ENABLE_IME_WIN32
#endif

#if defined(SF_BUILD_DEBUG) || defined(SF_BUILD_DEBUGOPT)
    #define SF_FPE_ENABLE
#endif
