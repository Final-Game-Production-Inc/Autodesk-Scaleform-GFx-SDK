/**************************************************************************

PublicHeader:   Render
Filename    :   Render_ThreadCommandQueue.h
Content     :   CommandQueue and command items interfaces for the
                render thread. 
Created     :   January 19, 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_ThreadCommandQueue_H
#define INC_SF_Render_ThreadCommandQueue_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { namespace Render {


// ThreadCommand is a base class for a command that is expected to be executed
// on a render thread.
class ThreadCommand : public RefCountBase<ThreadCommand, Stat_Default_Mem>
{
public:
    virtual void Execute() = 0;
};


class TextureManager;
class HAL;
class Renderer2D;

// Render thread may need to provide these render device access interfaces
// on request. Note that interfaces here are not AddRefed; it is expected that
// they are for temporary use only.
struct Interfaces
{
    TextureManager* pTextureManager;
    HAL*            pHAL;
    Renderer2D*     pRenderer2D;
    ThreadId        RenderThreadID;

    Interfaces(TextureManager* pmanager = 0, HAL* hal = 0, Renderer2D* r2d = 0, ThreadId rtid = 0)
        : pTextureManager(pmanager), pHAL(hal), pRenderer2D(r2d), RenderThreadID(rtid)
    { }
    Interfaces(const Interfaces& other)
        : pTextureManager(other.pTextureManager), pHAL(other.pHAL),
          pRenderer2D(other.pRenderer2D), RenderThreadID(other.RenderThreadID)
    { }

    void Clear()
    {
        pTextureManager = 0;
        pHAL = 0;
        pRenderer2D = 0;
        RenderThreadID = 0;
    }
};


//--------------------------------------------------------------------
// ***** ThreadCommandQueue

// ThreadCommandQueue is a command interface that should be implemented by
// a RenderThread and passed to Render::HAL. ThreadCommand-derived objects
// will be pushed on it for execution by the render thread.
//
// Currently used for servicing TextureManager texture creation and
// render Context shutdown requests, which need to be handled quickly even
// when the render thread has no other work.

class ThreadCommandQueue
{
public:
	virtual ~ThreadCommandQueue() {}

    // Pushes a command to be executed, it is expected that command will be
    // AddRef-ed until executed on the render thread.
    virtual void PushThreadCommand(ThreadCommand* command) = 0;

    // Queries render interfaces used by the thread. This is currenlty necessary
    // for implementing the DrawabeImage. Should be overriden by the render
    // thread to provide proper values.
    virtual void GetRenderInterfaces(Render::Interfaces* p)
    {
        p->Clear();
    }
};


}} // Scaleform::Render

#endif
