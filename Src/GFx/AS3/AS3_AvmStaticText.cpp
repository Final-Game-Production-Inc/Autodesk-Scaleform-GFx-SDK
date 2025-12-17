/**************************************************************************

Filename    :   AS3_AvmStaticText.cpp
Content     :   Implementation of timeline part of StaticText.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmStaticText.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "GFx/AS3/Obj/Text/AS3_Obj_Text_StaticText.h"

namespace Scaleform { namespace GFx { namespace AS3 {

AvmStaticText::AvmStaticText(StaticTextCharacter* pch)
:   AvmDisplayObj(pch)
{
}

AvmStaticText::~AvmStaticText()
{
}

}}} // SF::GFx::AS3
