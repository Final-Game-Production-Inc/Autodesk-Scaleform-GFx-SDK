/**********************************************************************

PublicHeader:   Render
Filename    :   PS3_Events.h
Content     :   
Created     :   May 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS3_Events_H
#define INC_SF_PS3_Events_H

#include "Render/Render_Events.h"
#include <cell/gcm.h>

namespace Scaleform { namespace Render { namespace PS3 {

class RenderEvent : public Render::RenderEvent
{
public:
    virtual ~RenderEvent() { }

    virtual void Begin( String eventName )
    {
        cellGcmSetPerfMonPushMarker(pHAL->GcmCtx, eventName.ToCStr());
    }
    virtual void End()
    {
        cellGcmSetPerfMonPopMarker(pHAL->GcmCtx);
    }
protected:
    friend class Render::PS3::HAL;
    HAL* pHAL;
    HAL* GetHAL() const { return pHAL; }

};

}}}

#endif // INC_SF_PS3_Events_H
