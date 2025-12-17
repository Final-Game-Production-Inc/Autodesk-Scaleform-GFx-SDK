/**********************************************************************

Filename    :   D3D9_ShaderBinary.cpp
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
    #include "D3D9_Debug_ShaderBinary.cpp"
#elif defined(SF_BUILD_DEBUGOPT)
    #include "D3D9_DebugOpt_ShaderBinary.cpp"
#elif defined(SF_BUILD_SHIPPING)
    #include "D3D9_Shipping_ShaderBinary.cpp"
#else
    #include "D3D9_Release_ShaderBinary.cpp"
#endif
