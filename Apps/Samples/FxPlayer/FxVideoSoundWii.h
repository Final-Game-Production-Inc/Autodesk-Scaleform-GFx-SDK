/**************************************************************************

Filename    :   FxVideoSoundWii.h
Content     :   Sample system based allocator class that can be
                installed in FxPlayer to show overriding allocators.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxVideoPlayerCreatorWii_H
#define INC_FxVideoPlayerCreatorWii_H


#include "GFxVideoPlayerCreator.h"

class FxVideoSoundCreatorWiiImpl;
class FxVideoSoundCreatorWii : public GFxVideoSoundCreator
{
public:
    FxVideoSoundCreatorWii();
    virtual ~FxVideoSoundCreatorWii();

    virtual GFxVideoSoundInterface* Create(GMemoryHeap* );

protected:
    FxVideoSoundCreatorWiiImpl* pImpl;
};

#endif //INC_FxVideoPlayerCreatorWii_H
