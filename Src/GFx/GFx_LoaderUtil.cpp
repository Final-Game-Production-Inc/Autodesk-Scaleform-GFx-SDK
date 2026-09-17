/**************************************************************************

Filename    :   GFx_LoaderUtil.cpp
Content     :   Eval License Reading code 
Created     :   March 16, 2012
Authors     :

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

// Validate Check - Eval License Reader code was moved here so that 
// other engines could more easily replace it with their own validation checking code.

**************************************************************************/

#include "GFxConfig.h"

void    GFx_ValidateEvaluation();

// Internal GFC Evaluation License Reader
#if  defined (SF_BUILD_EVAL)
#define SF_EVAL(x) GFx_##x
#define SF_LIB_NAME_S "GFx"
#define SF_PRODUCT_ID SCALEFORM
#define SF_LICENSE_FILE "sf_license.txt"
#include "GFCValidateEval.cpp"
#elif defined (SF_BUILD_CONSUMER)
#define SF_EVAL(x) GFx_##x
#define SF_LIB_NAME_S "GFx"
#define SF_PRODUCT_ID SCALEFORM
#if defined(SF_OS_ANDROID) && !defined(SF_BUILD_UNITY)
#define SF_LICENSE_FILE "sf_consumer_license_android_mobile.txt"
#elif defined(SF_OS_ANDROID) && defined (SF_BUILD_UNITY)
#define SF_LICENSE_FILE "sf_consumer_license_android_unity.txt"
#elif defined(SF_OS_IPHONE) && !defined(SF_BUILD_UNITY)
#define SF_LICENSE_FILE "sf_consumer_license_ios_mobile.txt"
#elif defined(SF_OS_IPHONE) && defined(SF_BUILD_UNITY)
#define SF_LICENSE_FILE "sf_consumer_license_ios_unity.txt"
#elif defined(SF_OS_WIN32)
#define SF_LICENSE_FILE "sf_consumer_license_pc_unity.txt"
#elif defined(SF_OS_MAC)
#define SF_LICENSE_FILE "sf_consumer_license_mac_unity.txt"
#else
#define SF_LICENSE_FILE "sf_consumer_license.txt"
#endif
#include "GFCValidateEval.cpp"
#else
void    GFx_ValidateEvaluation() { }
void    GFx_SetEvalKey(const char* key) {SF_UNUSED(key);}
#endif
