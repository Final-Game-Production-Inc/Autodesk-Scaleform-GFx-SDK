/**************************************************************************

Filename    :   AS3_VideoCharacter.cpp
Content     :   GFx video: AS3 AvmVideoCharacter class
Created     :   March, 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/AS3/AS3_VideoCharacter.h"

#ifdef GFX_ENABLE_VIDEO

namespace Scaleform { namespace GFx {
    
using namespace Video;

namespace AS3 {

//////////////////////////////////////////////////////////////////////////
//

    AvmVideoCharacter::AvmVideoCharacter(VideoCharacter* pchar) :
    AvmInteractiveObj(pchar), AvmVideoCharacterBase()
{
}

}}} // Scaleform::GFx::AS3

#endif // GFX_ENABLE_VIDEO
