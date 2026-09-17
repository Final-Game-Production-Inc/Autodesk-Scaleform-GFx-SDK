/**********************************************************************

PublicHeader:   Render
Filename    :   GL_Events.h
Content     :   Implementation of GPU events for use with gDEBugger.
Created     :   Mar 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_GL_Events_H
#define INC_SF_GL_Events_H

#include "Render/Render_Events.h"

namespace Scaleform { namespace Render { namespace GL {

class HAL;

class RenderEvents : public Render::RenderEvents
{
public:
    RenderEvents(GL::HAL* hal) : pHAL(hal) { }
    virtual ~RenderEvents() { }

protected:
    virtual void beginImpl( const char* eventName );
    virtual void endImpl();

    friend class Render::GL::HAL;
    HAL* pHAL;
    HAL* GetHAL() const { return pHAL; }
};

}}} // Scaleform::Render::GL

#endif // INC_SF_GL_Events_H
