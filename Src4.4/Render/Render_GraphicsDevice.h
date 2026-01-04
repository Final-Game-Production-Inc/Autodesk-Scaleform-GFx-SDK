/**********************************************************************

PublicHeader:   Render
Filename    :   Render_GraphicsDevice.h
Content     :   
Created     :   2014/03/25
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#pragma once
#ifndef INC_Render_GraphicsDevice_h
#define INC_Render_GraphicsDevice_h

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Threads.h"
#include "Render/Render_Stats.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Atomic.h"

namespace Scaleform { namespace Render {

class GraphicsDevice;
class GraphicsDeviceImmediate;
class GraphicsDeviceRecorder;

class GraphicsDevice : public RefCountBase<GraphicsDevice, StatRender_Context_Mem>
{
public:
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual bool IsDeferred() const { return false; }

protected:
    ThreadId            RenderThreadID;     // ThreadId of the main render thread (may be 0, or equal to the main thread's ID in single threaded mode).
};

class GraphicsDeviceRecorder
{
public:
    // DrawingCommands are used in recordings to determine which functions to perform.
    enum DrawingCommands
    {
        FinishedDraw           = 0x8001, // NOTE: start command IDs at an 'odd' number, easier to validate.
        PlatformCommands_Start,
    };

    GraphicsDeviceRecorder();
    virtual ~GraphicsDeviceRecorder();

    // Executes the commands recorded by this interface into the given GraphicsDevice. In general, this is 
    // called within HAL::Submit automatically. Returns false if no commands were executed (because none were
    // recorded), and true otherwise.
    virtual bool ExecuteRecording(GraphicsDevice& playbackDevice);

protected:
    unsigned    CommandBufferSize;
    unsigned    ResourceBufferSize;
    UByte*      CommandBuffer;
    UByte*      CommandReadPointer;
    UByte*      CommandWritePointer;
    UByte*      ResourceBuffer;
    UByte*      ResourceWritePointer;
    static const unsigned MaxStaleResourceBuffers = 8;
    UByte*      StaleResourceBuffers[MaxStaleResourceBuffers];
    Lock        BufferLock;

    UByte* alloc(size_t size);
    void   free(const void* pointer);

    template<typename T>
    void write(const T& v)
    {
        UPInt alignedSize = Alg::Align<sizeof(UPInt)>(sizeof(v));
        SPInt writeOffset    = CommandWritePointer - CommandBuffer;
        SPInt spaceRemaining = CommandBufferSize - alignedSize - writeOffset;
        if (spaceRemaining < 0)
        {
            // Writing only locks when space has run out.
            Lock::Locker lock(&BufferLock);
            do
            {
                CommandBufferSize *= 2;
            } while (CommandBufferSize <  alignedSize);
            CommandBuffer = (UByte*)SF_REALLOC(CommandBuffer, CommandBufferSize, StatRender_Context_Mem);
            CommandReadPointer = CommandBuffer;
            CommandWritePointer = CommandBuffer + writeOffset;
        }
        *((T*)CommandWritePointer) = v;
        CommandWritePointer += alignedSize;
    }
    template<typename T>
    void read(T& v)
    {
        // Yield until next command is available.
        while (CommandReadPointer == CommandWritePointer)
            Thread::Sleep(0);

        Lock::Locker lock(&BufferLock);
        v = *((T*)CommandReadPointer);
        CommandReadPointer += Alg::Align<sizeof(UPInt)>(sizeof(v));
    }
};



}} // Scaleform::Render



#endif
