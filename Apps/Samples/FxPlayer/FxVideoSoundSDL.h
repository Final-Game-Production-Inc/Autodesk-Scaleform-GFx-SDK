/**************************************************************************

Filename    :   FxVideoSoundSDL.h
Content     :   Sample system based allocator class that can be
                installed in FxPlayer to show overriding allocators.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxVideoPlayerCreator.h"

class GFxVideoSoundCreatorSDLImpl;
class GFxVideoSoundCreatorSDL : public GFxVideoSoundCreator
{
public:
    GFxVideoSoundCreatorSDL();
    virtual ~GFxVideoSoundCreatorSDL();

    virtual GFxVideoSoundInterface* Create();

protected:
    GFxVideoSoundCreatorSDLImpl* pImpl;
};
