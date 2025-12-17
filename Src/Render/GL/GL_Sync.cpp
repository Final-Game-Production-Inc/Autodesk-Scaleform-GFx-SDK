/**********************************************************************

PublicHeader:   Render
Filename    :   GL_Sync.cpp
Content     :   
Created     :   2012/12/05
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/GL/GL_Sync.h"
#include "Render/GL/GL_Common.h"
#include "Render/GL/GL_HAL.h"

#if defined(SF_USE_GLES)
#include "Render/GL/GLES11_ExtensionMacros.h"
#elif defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ExtensionMacros.h"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#endif

namespace Scaleform { namespace Render { namespace GL {

void RenderSync::KickOffFences( FenceType waitType )
{
    SF_UNUSED(waitType);
    glFlush();
}

Scaleform::UInt64 RenderSync::SetFence()
{
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    return reinterpret_cast<UInt64>(sync);
}

bool RenderSync::IsPending( FenceType waitType, UInt64 handle, const FenceFrame& parent )
{
    SF_UNUSED2(waitType, parent);
    GLsync  sync = reinterpret_cast<GLsync>(handle);
    GLsizei length;
    GLint   status;
    glGetSynciv(sync, GL_SYNC_STATUS, 4, &length, &status);
    SF_UNUSED3(sync, length, status); // fix warning if glGetSynciv is a no-op.
    return (status == GL_UNSIGNALED);
}

void RenderSync::WaitFence( FenceType waitType, UInt64 handle, const FenceFrame& parent )
{
    SF_UNUSED2(waitType, parent);
    GLsync  sync = reinterpret_cast<GLsync>(handle);
    // Make sure KickOffFences has been called.
    KickOffFences(waitType);
    GLenum ret = glClientWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    // fix warning if glClientWaitSync is a no-op.
    SF_UNUSED2(ret, sync);
}

void RenderSync::ReleaseFence( UInt64 handle )
{
    GLsync  sync = reinterpret_cast<GLsync>(handle);
    glDeleteSync(sync);
    // fix warning if glDeleteSync is a no-op.
    SF_UNUSED(sync);
}


}}}; // Scaleform::Render::GL


