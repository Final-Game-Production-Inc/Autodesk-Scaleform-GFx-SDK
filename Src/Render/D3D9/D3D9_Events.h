/**********************************************************************

PublicHeader:   Render
Filename    :   D3D9_Events.h
Content     :   Implementation of GPU events for use with PIX.
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_D3D9_Events_H
#define INC_SF_D3D9_Events_H

#include "Render/Render_Events.h"
#include <d3d9.h>

namespace Scaleform { namespace Render { namespace D3D9 {

class RenderEvent : public Render::RenderEvent
{
public:
    virtual ~RenderEvent() { }

    virtual void Begin( const char* eventName )
    {
        // PIX events only take wide-character strings.
        wchar_t dest[256];
        size_t  chars;
        mbstowcs_s(&chars, dest, eventName, 256);
        D3DPERF_BeginEvent(D3DCOLOR_XRGB(0,0,0), dest);
    }
    virtual void End()
    {
        D3DPERF_EndEvent();
    }
};

}}} // Scaleform::Render::D3D9

#endif // INC_SF_D3D9_Events_H
