/**************************************************************************

Filename    :   FxAmpAppControlCallback.cpp
Content     :   Interface that allows AMP to control FxPlayer
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxCompat.h"
#include "FxAmpAppControlCallback.h"
#include "FxApp.h"

FxAmpAppControlCallback::FxAmpAppControlCallback(FxApp* app) : App(app)
{
}


bool FxAmpAppControlCallback::HandleAmpRequest(const GFxAmpMessageAppControl* message)
{
    if (App != NULL)
    {
        App->OnAmpAppControlToggle(message);
    }

    return true;
}
