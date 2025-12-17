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

#if !defined(SF_USE_GLES_ANY)

#include "Render/GL/GL_HAL.h"
#include "Render/GL/GL_ExtensionMacros.h"

namespace Scaleform { namespace Render { namespace GL {

class RenderEvent : public Render::RenderEvent
{
public:
    RenderEvent() : HasExtension(false), ExtensionChecked(false) { }
    virtual ~RenderEvent() { }

    virtual void Begin( const char* eventName )
    {
#ifdef GL_GREMEDY_string_marker
        if (!ExtensionChecked)
        {
            HasExtension = pHAL->CheckExtension("GL_GREMEDY_string_marker");
            ExtensionChecked = true;
        }
        if (HasExtension)
            glStringMarkerGREMEDY(0, eventName);
#else
        SF_UNUSED(eventName);
#endif
    }
    virtual void End()
    {
#ifdef GL_GREMEDY_string_marker
        if (HasExtension)
            glStringMarkerGREMEDY(0, "End");
#endif
    }
protected:
    friend class Render::GL::HAL;
    HAL* pHAL;
    HAL* GetHAL() const { return pHAL; }
    bool HasExtension;
    bool ExtensionChecked;
};

}}} // Scaleform::Render::GL

#endif // !GLES

#endif // INC_SF_GL_Events_H
