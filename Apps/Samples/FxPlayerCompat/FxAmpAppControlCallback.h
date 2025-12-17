/**************************************************************************

Filename    :   FxAmpAppControlCallback.h
Content     :   Interface that allows AMP to control FxPlayer
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INCLUDE_FX_AMP_APP_CONTROL_CALLBACK_H
#define INCLUDE_FX_AMP_APP_CONTROL_CALLBACK_H

#include "GFx_AMP.h"

class FxApp;

class FxAmpAppControlCallback : public GRefCountBase<FxAmpAppControlCallback, GStat_Default_Mem>, public GFxAmpAppControlInterface
{
public:
    FxAmpAppControlCallback(FxApp* app);
    virtual bool HandleAmpRequest(const GFxAmpMessageAppControl* message);
private:
    FxApp*              App;
};

#endif
