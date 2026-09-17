/**********************************************************************

Filename    :   Durango_ShaderBinary.cpp
Content     :   
Created     :   August 2013
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "GFxConfig.h"

#if defined(SF_BUILD_DEBUG)
    #include "XboxOne_Debug_ShaderBinary.cpp"
#elif defined(SF_BUILD_DEBUGOPT)
    #include "XboxOne_DebugOpt_ShaderBinary.cpp"
#elif defined(SF_BUILD_SHIPPING)
    #include "XboxOne_Shipping_ShaderBinary.cpp"
#else
    #include "XboxOne_Release_ShaderBinary.cpp"
#endif
