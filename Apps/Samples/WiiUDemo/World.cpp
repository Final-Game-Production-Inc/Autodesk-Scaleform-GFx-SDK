/**************************************************************************

Filename    :   World.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "World.h"

WorldLocation*  World::GetLocation(const String& name) const
{
    return Locations.Get(name);
}

void            World::AddLocation(const WorldLocation& loc)
{
    Locations.Add(loc.Name, loc);
}
