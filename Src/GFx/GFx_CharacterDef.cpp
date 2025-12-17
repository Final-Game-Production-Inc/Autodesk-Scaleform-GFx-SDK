/**************************************************************************

Filename    :   GFx_CharacterDef.cpp
Content     :   Implementation of basic CharacterDef functionality.
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_CharacterDef.h"

// For GASExecutTag().
#include "GFx/GFx_MovieDef.h"

namespace Scaleform { namespace GFx {

// ***** CharacterDef

// Create a most basic instance through GFxGenericCharacter
// GFxCharacter*   CharacterDef::CreateCharacterInstance(InteractiveObject* parent, ResourceId rid,
//                                                          MovieDefImpl *pbindingImpl)
// {
//     SF_UNUSED(pbindingImpl);
//     return SF_HEAP_AUTO_NEW(parent) GFxGenericCharacter(this, parent, rid);
// }

void    CharacterDef::Display(DisplayContext &, DisplayObjectBase* pinstanceInfo)  
{ 
    SF_UNUSED(pinstanceInfo); 
}


// *****  TimelineDef

// Calls destructors on all of the tag objects.
void    TimelineDef::Frame::DestroyTags()
{
    for (unsigned i=0; i<TagCount; i++)
        pTagPtrList[i]->~ExecuteTag();
    // Clear count and pointers.
    pTagPtrList = 0;
    TagCount    = 0;
}

}} // namespace Scaleform::GFx
