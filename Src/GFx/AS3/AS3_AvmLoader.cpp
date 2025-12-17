/**************************************************************************

Filename    :   AS3_AvmLoader.cpp
Content     :   Implementation of timeline part of Loader.
Created     :   Jan, 2011
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmLoader.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_LoaderInfo.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Loader.h"

namespace Scaleform { namespace GFx { namespace AS3 {

AvmLoader::AvmLoader(ASMovieRootBase* pasRoot,
                     MovieDefImpl* pdefImpl,
                     InteractiveObject* pparent, 
                     ResourceId id)
:   DisplayObjContainer(pdefImpl, pasRoot, pparent, id),
    AvmDisplayObjContainer(GetThis())
{
}

AvmLoader::~AvmLoader()
{
    // We must nullify AvmObjOffset to avoid a second call of ~AvmLoader from 
    // ~DisplayObjectBase @ 'GetAvmObjImpl()->~AvmDisplayObjBase()'. This can happen,
    // since the AvmLoader is inherited from both DisplayObjContainer and AvmDisplayObjContainer and
    // ~AvmDisplayObjContainer is called automatically.
    AvmObjOffset = 0;
}


}}} // SF::GFx::AS3
