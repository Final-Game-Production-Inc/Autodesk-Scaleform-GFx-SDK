/**********************************************************************

PublicHeader:   Render
Filename    :   X360_Events.h
Content     :   Implementation of GPU events for use with PIX.
Created     :   Jun 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_X360_Events_H
#define INC_SF_X360_Events_H

#include <xtl.h>

namespace Scaleform { namespace Render { namespace X360 { 

// Used to create hierarchies in PIX captures.
class RenderEvent : public Render::RenderEvent
{
public:
    void Begin(String eventName)
    {
        // Strip off the namespace and re-add a smaller one - it is getting truncated in the PIX UI
        const char* pfuncOnly;
        if ( (pfuncOnly = SFstrrchr(eventName, ':')) == 0 )
            pfuncOnly = eventName;
        else
            pfuncOnly++;

        char event[512];
        SFstrcpy(event, 512, "Scaleform::HAL::" );
        SFstrcat(event, 512, pfuncOnly );
        PIXBeginNamedEvent(D3DCOLOR_XRGB(0,0,0), event);
    }
    void End()
    {
        PIXEndNamedEvent();
    }
};

}}};

#endif // INC_SF_X360_Events_H
