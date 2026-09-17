/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Events.h
Content     :   
Created     :   2012/09/30
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_Events_H
#define INC_SF_PS4_Events_H

#include "Render/Render_Events.h"
#include "Render/PS4/PS4_HAL.h"

namespace Scaleform { namespace Render { namespace PS4 {

class RenderEvent : public Render::RenderEvent
{
public:
    virtual ~RenderEvent() { }

    virtual void Begin( const char* eventName )
    {
        if (pHAL && pHAL->GnmxCtx)
            pHAL->GnmxCtx->pushMarker(eventName);
    }
    virtual void End()
    {
        if (pHAL && pHAL->GnmxCtx)
            pHAL->GnmxCtx->popMarker();
    }
protected:
    friend class Render::PS4::HAL;
    PS4::HAL* pHAL;
    PS4::HAL* GetHAL() const { return pHAL; }

};

}}}

#endif // INC_SF_Orbis_Events_H
