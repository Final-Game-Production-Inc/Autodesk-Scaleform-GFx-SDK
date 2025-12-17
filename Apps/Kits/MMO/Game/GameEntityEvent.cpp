/**************************************************************************

Filename    :   GameEntityEvent.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameEntityEvent.h"

//////////////////////////////////////////////////////////////////////////

void    GameEntityListenerGroup::AddListener(GameEntityListener* l)
{
    Listeners.PushBack(l);
}

void    GameEntityListenerGroup::RemoveListener(GameEntityListener* l)
{
    int idx = -1;
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
    {
        if (Listeners[i] == l)
        {
            idx = i;
            break;
        }
    }
    if (idx > -1)
    {
        Listeners.RemoveAt(idx);
    }
}

void    GameEntityListenerGroup::NotifyEvent(const GameEntityEvent& e)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnEntityEvent(e);
}



//////////////////////////////////////////////////////////////////////////

void    GameEntitySlotListenerGroup::AddListener(GameEntitySlotListener* l)
{
    SF_ASSERT(l);
    Listeners.PushBack(l);
}

void    GameEntitySlotListenerGroup::RemoveListener(GameEntitySlotListener* l)
{
    SF_ASSERT(l);
    int idx = -1;
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
    {
        if (Listeners[i] == l)
        {
            idx = i;
            break;
        }
    }
    if (idx > -1)
    {
        Listeners.RemoveAt(idx);
    }
}

void    GameEntitySlotListenerGroup::NotifyEvent(const GameEntitySlotEvent& e)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnSlotEvent(e);
}