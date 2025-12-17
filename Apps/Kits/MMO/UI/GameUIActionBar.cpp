/**************************************************************************

Filename    :   GameUIActionBar.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameUIActionBar.h"


//////////////////////////////////////////////////////////////////////////


GameUIActionBar::GameUIActionBar(unsigned numSlots)
{
    Links.Resize(numSlots);
}

GameUIActionBarManager::GameUIActionBarManager()
{
    // Add 3 bars
    Ptr<GameUIActionBar> bar0 = *SF_NEW GameUIActionBar(12);
    AddBar(bar0);
    Ptr<GameUIActionBar> bar1 = *SF_NEW GameUIActionBar(12);
    AddBar(bar1);
    Ptr<GameUIActionBar> bar2 = *SF_NEW GameUIActionBar(8);
    AddBar(bar2);
}