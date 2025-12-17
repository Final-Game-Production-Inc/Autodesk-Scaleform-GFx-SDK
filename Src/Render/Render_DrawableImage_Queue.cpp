/**************************************************************************

PublicHeader:   Render
Filename    :   Render_DrawableImage_Queue.cpp
Content     :   Commands and command queue implementation for DrawableImage
Created     :   January 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

//For DICommand
#include "Render/Render_HAL.h"

#include "Render_DrawableImage_Queue.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

// If this is not defined, DI commands will only execute on the GPU (unless they have no GPU implementations)
#define SF_DRAWABLEIMAGE_CPU_ENABLE
// If this is not defined, DI commands will only execute on the CPU (unless they have no CPU implementations)
#define SF_DRAWABLEIMAGE_GPU_ENABLE
#if !defined(SF_DRAWABLEIMAGE_CPU_ENABLE) && !defined(SF_DRAWABLEIMAGE_GPU_ENABLE)
    #error "Can't disable both CPU and GPU DrawableImage commands"
#endif


namespace Scaleform { namespace Render {


unsigned DICommand::GetRenderCaps() const
{
    // CPU capability flags are built into the command themselves.
    unsigned cpuCaps = GetCPUCaps();

    // If the GPU enable is off, and there is a CPU version, use that explicitly.
#ifndef SF_DRAWABLEIMAGE_GPU_ENABLE
    if (cpuCaps)
        return cpuCaps;
#endif

    // GPU capabilities need to be queried from the HAL.
    if (!pImage || !pImage->GetContext() || !pImage->GetContext()->GetQueue())
        return cpuCaps;

    DICommandContext cmdCtx(pImage->GetContext()->GetQueue());
    if (!cmdCtx.pHAL)
        return cpuCaps;
    
    // Mix the CPU flags with the GPU flags reported from the HAL.
    unsigned gpuCaps = cmdCtx.pHAL->DrawableCommandGetFlags(this);

    // If CPU enable is off, and there is a GPU version, use that explicitly.
#ifndef SF_DRAWABLEIMAGE_CPU_ENABLE
    if (gpuCaps)
        return gpuCaps;
#endif

    return gpuCaps | cpuCaps;
}

bool DICommand::ExecuteSWOnAddCommand(DrawableImage* i) const
{
    unsigned       caps = GetRenderCaps();

    // If the command does not have CPU capability, then it cannot be executed here.
    if ((caps & DICommand::RC_CPU) == 0)
        return false;

    // To execute on the main thread, this image, and all its sources must currently be mapped, with no
    // pending commands to be executed. If they are not mapped, it will be executed on the render thread 
    // (while the advance thread waits). If no GPU commands are executed before another 'return' command 
    // is executed, the image should remain mapped, and it will be executed on the main thread (next time).
    DISourceImages sources;
    unsigned       imageCount = GetSourceImages(&sources);

    // Note: Lock must occur here, if there is an operation executing on the render thread, it may try to unmap
    // the image, which it shouldn't be able to do, if we are going to execute a command on it assuming it is mapped.
    Lock::Locker lock(&i->pQueue->QueueLock);

    // Optimization: because DICommand store smart pointers to their DrawableImages, we can use its refcount
    // to determine whether it's referenced by a command already in the queue. DrawableImages should have two
    // references, one from the BitmapData that owns them, and a second from this DICommand.
    if (!i->isMapped() || i->GetRefCount() > 2)
        return false;

    for (unsigned src = 0; src < DISourceImages::MaximumSources; ++src)
    {
        if (sources[src] != 0)
        {
            if (sources[src]->GetImageType() == Image::Type_DrawableImage)
            {
                DrawableImage* disource = reinterpret_cast<DrawableImage*>(sources[src]);
                if (!disource->isMapped() || disource->GetRefCount() > 2)
                    return false;
            }
        }
    }


    // We can now safely execute the command, there should be no outstanding operations on the destination
    // image, because they can only be added by the Advance thread, which is where execution is currently
    // taking place.
    DICommandContext context(i->GetContext()->GetQueue());
    bool executed = executeSWHelper(context, i, sources, imageCount);
    SF_UNUSED(executed); // release warning.
    SF_DEBUG_ASSERT(executed, "Expected DrawableImage with return value to be executed immediately.");

    // If this command modifies the target BitmapData, add it to the modified list.
    if ((caps & DICommand::RC_CPU_NoModify) == 0)
        i->addToCPUModifiedList();

    // Return true, as this command cannot be queued anyhow, because it should return a value.
    return true;
}

// RenderThread execution function; does proper map/unmap and dispatches
// to either ExecuteHW or ExecuteSW. 
void DICommand::ExecuteRT(DICommandContext& context) const
{
    // 
    if ((GetRenderCaps() & (RC_CPU|RC_GPUPreference)) == RC_CPU)
    {
        // Update any GPU modified images, we will need the results for the CPU execution.
        pImage->pQueue->updateGPUModifiedImagesRT();

        // Map image & source data and call ExecuteSW.
        DISourceImages images;
        unsigned       imageCount = GetSourceImages(&images);

        
        if (!pImage->isMapped() && !pImage->mapTextureRT((GetRenderCaps() & RC_CPU_NoModify) != 0))
        {
            SF_DEBUG_WARNING(1, "Render::DICommand::ExecuteRT - map destination failed");
            return;
        }

        executeSWHelper(context, pImage, images, imageCount);
    }
    else
    {
        SF_DEBUG_ASSERT(GetRenderCaps()&RC_GPU, "DrawableImage: neither CPU or GPU flags are set for command.");

        // Update any CPU modified images, we will need the results for the GPU execution.
        pImage->pQueue->updateCPUModifiedImagesRT();
        executeHWHelper(context, pImage);
    }
}


bool DICommand::executeSWHelper(DICommandContext& context,
                                DrawableImage* di,
                                DISourceImages& images,
                                unsigned imageCount) const
{
    ImageData& destData = di->getMappedData();

    if (imageCount)
    {
        ImageData idata[2];
        ImageData* ipdata[2] = { 0, 0 };

        if (images[0] && !DrawableImage::MapImageSource(&idata[0], images[0]))
            return false;
        else
            ipdata[0] = &idata[0];

        if (images[1] && !DrawableImage::MapImageSource(&idata[1], images[1]))
            return false;
        else
            ipdata[1] = &idata[1];

        ExecuteSW(context, destData, &ipdata[0]);
    }
    else
    {
        ExecuteSW(context, destData, 0);
    }

    // Only add it to the CPU modified list, if the command actually has the possibility of modifying it.
    if ((GetRenderCaps() & RC_CPU_NoModify) == 0)
        di->addToCPUModifiedList();

    return true;
}

bool DICommand::executeHWHelper(DICommandContext& context, DrawableImage* i) const
{
    // Must always unmap the image, if we are going to perform any GPU command.
    i->unmapTextureRT();

    ExecuteHW(context);

    if ( (GetRenderCaps() & (RC_GPU|RC_GPU_NoRT)) == RC_GPU )
    {
        i->addToGPUModifiedListRT();
    }

    return true;
}

//--------------------------------------------------------------------

DICommandQueue::DICommandQueue(DrawableImageContext* dicontext)
: pRTCommands(0),
  pCPUModifiedImageList(0),
  pGPUModifiedImageList(0),
  pRTCommandQueue(dicontext->GetQueue()),
  ExecuteCmd(*SF_NEW ExecuteCommand(getThis())),
  CaptureFrameId(0), FreePageCount(0), AllocPageCount(0)
{
}

DICommandQueue::~DICommandQueue()
{   
    // Unmap any currently mapped textures in OnShutdown.
    OnShutdown();

    Lock::Locker lock(&QueueLock);
    SF_ASSERT(Queues[DIQueue_Active].IsEmpty());
    SF_ASSERT(Queues[DIQueue_Captured].IsEmpty());
    SF_ASSERT(Queues[DIQueue_Displaying].IsEmpty());

    // Free any pages in free list.
    while(!Queues[DIQueue_Free].IsEmpty())
    {
        DIQueuePage* page = Queues[DIQueue_Free].GetFirst();
        page->RemoveNode();
        delete page;
    }
}

void DICommandQueue::popCommandSet(DICommandSet* cmdSet, DICommandSetType type)
{
    Mutex::Locker mlock(&CommandSetMutex);

    // Displaying list should be processed first.
    cmdSet->QueueList.PushListToBack(Queues[DIQueue_Displaying]);
    if (type == DICommand_All)
    {
        cmdSet->QueueList.PushListToBack(Queues[DIQueue_Captured]);
        cmdSet->QueueList.PushListToBack(Queues[DIQueue_Active]);
    }
}

void* DICommandQueue::allocCommandFromPage(unsigned size, Lock* locked)
{
    // Allocate memory from the current page. If page doesn't exist,
    // create it.
    DIQueuePage* page = Queues[DIQueue_Active].GetLast();
    if (Queues[DIQueue_Active].IsNull(page) || !page->HasFreeSize(size))
    {
        if (!FreePageCount && (AllocPageCount >= AllocPageLimit))
        {
            locked->Unlock();
            ExecuteCommandsAndWait();
            locked->DoLock();
        }

        page = allocPage();
    }

    if (!page)
        return 0;
    return page->AllocSize(size);
}

DIQueuePage* DICommandQueue::allocPage()
{
    DIQueuePage* page;
    if (!Queues[DIQueue_Free].IsEmpty())
    {
        page = Queues[DIQueue_Free].GetFirst();
        page->RemoveNode();
        FreePageCount--;
    }
    else
    {
        page = SF_HEAP_AUTO_NEW(this) DIQueuePage;
    }
    if (page)
        Queues[DIQueue_Active].PushBack(page);
    return page;
}

void DICommandQueue::freePage(DIQueuePage* page)
{
    if (FreePageCount < FreePageLimit)
    {
        page->Offset = 0; // Tell the page it has nothing allocated in it.
        Queues[DIQueue_Free].PushBack(page);
        FreePageCount++;
    }
    else
    {
        delete page;
    }
}


void DICommandQueue::lockCommandSetAndWait( DICommandSet** pcommands )
{
    CommandSetMutex.DoLock();
    while(*pcommands)
        CommandSetWC.Wait(&CommandSetMutex);
}

void DICommandQueue::unlockCommandSet()
{
    CommandSetMutex.Unlock();
}

void DICommandQueue::notifyCommandSetFinished( DICommandSet** pcommands )
{
    Mutex::Locker mlock(&CommandSetMutex);
    *pcommands = 0;
    CommandSetWC.NotifyAll();
}

void DICommandQueue::updateCPUModifiedImagesRT()
{
    // Thread safety may be accomplished in a lighter method.
    Lock::Locker lock(&QueueLock);

    Ptr<DrawableImage> p;
    Ptr<DrawableImage> pmodified = pCPUModifiedImageList;
    pCPUModifiedImageList = 0;
    while(pmodified)
    {
        p = pmodified;
        SF_DEBUG_ASSERT(p->DrawableImageState & DrawableImage::DIState_CPUDirty, "Internal error - expected CPUDirty flag to be set.");

        // Copy the data from the CPU to the GPU.
        p->updateRenderTargetRT();

        pmodified = pmodified->pCPUModifiedNext;
        p->pCPUModifiedNext  = 0;
        p->DrawableImageState &= ~DrawableImage::DIState_CPUDirty;
    }
}

void DICommandQueue::updateGPUModifiedImagesRT()
{
    // Thread safety may be accomplished in a lighter method.
    Lock::Locker lock(&QueueLock);

    Ptr<DrawableImage> p;
    Ptr<DrawableImage> pmodified = pGPUModifiedImageList;
    pGPUModifiedImageList = 0;
    while(pmodified)
    {
        p = pmodified;
        SF_DEBUG_ASSERT(p->DrawableImageState & DrawableImage::DIState_GPUDirty, "Internal error - expected GPUDirty flag to be set.");

        // Copy the data from the GPU to the CPU.
        p->updateStagingTargetRT();

        pmodified = pmodified->pGPUModifiedNext;
        p->pGPUModifiedNext  = 0;
        p->DrawableImageState &= ~DrawableImage::DIState_GPUDirty;
    }
}


void DICommandQueue::OnCapture()
{
    Mutex::Locker mlock(&CommandSetMutex);
    while(pRTCommands)
        CommandSetWC.Wait(&CommandSetMutex);

    // List lock is always the second (or only) lock.
    Lock::Locker lock(&QueueLock);
    Queues[DIQueue_Captured].PushListToBack(Queues[DIQueue_Active]);
}


void DICommandQueue::OnNextCapture(ContextImpl::RenderNotify* pnotify)
{
    // Note: After textures are updated (updateC/GPUModifiedImages is called), this queue may not
    // have any references, if the MT has disposed of the image already. Thus, protect the queue
    // from deletion in this case.
    AddRef();

    // If the CPU has modified any images for this queue, we need to update them, in case
    // we need to use the results in DrawableImage operations, or simply in rendering.
    updateCPUModifiedImagesRT();
    ExecuteNextCapture(pnotify);
    Release();
}

void DICommandQueue::OnShutdown()
{
    // Ensure that there are no modified images on shutdown.
    SF_DEBUG_ASSERT(pCPUModifiedImageList == 0 && pGPUModifiedImageList == 0, "Expected no modified images after shutdown.");
}

void DICommandQueue::ExecuteNextCapture(ContextImpl::RenderNotify* pnotify)
{
    // If there is a NULL command queue, use the notify's queue.
    if (!pRTCommandQueue && pnotify)
        pRTCommandQueue = pnotify->pRTCommandQueue;

    SF_ASSERT(pRTCommandQueue);
    DICommandContext context(pRTCommandQueue);
    DICommandSet     cmdSet(this);

    // pHAL/pR2D must either be initialized externally or
    // ThreadCommandQueue::GetRenderInterfaces implemented by the render thread.
    SF_ASSERT(context.pHAL);

    // TBD: Wait for any outstanding MapExecute completion on the advance thread.
    {
        Mutex::Locker mlock(&CommandSetMutex);

        // List lock is always the second (or only) lock.
        Lock::Locker lock(&QueueLock);
        Queues[DIQueue_Displaying].PushListToBack(Queues[DIQueue_Captured]);
        popCommandSet(&cmdSet, DICommand_Displaying);
        pRTCommands = &cmdSet;
    }

    cmdSet.ExecuteCommandsRT(context);    

    // Signal NextCapture completion for Advance thread in case of wait
    {
        Mutex::Locker mlock(&CommandSetMutex);
        pRTCommands = 0;
        CommandSetWC.NotifyAll();
    }
}

void DICommandQueue::ExecuteCommandsAndWait()
{
    // ExecuteCmd/Queue may not be reference counted by the ThreadCommandQueue, so protect them
    // against its deletion until after it executes here by adding extra refs.
    AddRef();
    ExecuteCmd->AddRef();

    pRTCommandQueue->PushThreadCommand(ExecuteCmd);
    ExecuteCmd->WaitDoneAndReset();
}

// This function discards any commands currently queue (eg. without executing them). This is used when
// a DrawableImageContext is being shutdown, and is unable to wait. 
void DICommandQueue::DiscardCommands()
{
    DICommandContext context;
    DICommandSet cmdSet(this);

    // Push the commands into the RTCommand list.
    popCommandSet(&cmdSet, DICommand_All);
    while(!cmdSet.QueueList.IsEmpty())
    {
        DIQueuePage* page = cmdSet.QueueList.GetFirst();
        page->RemoveNode();
        DICommand* cmd = page->getFirst();
        while (cmd)
        {
            cmd->ExecuteDiscard();
            cmd = page->destroyAndGetNext(cmd);
        }
        delete page;
    }
}

void DICommandQueue::ExecuteCommand::Execute()
{
    DICommandContext context(pQueue->pRTCommandQueue);
    DICommandSet cmdSet(pQueue);

    // Push the commands into the RTCommand list.
    pQueue->popCommandSet(&cmdSet, DICommand_All);
    pQueue->pRTCommands = &cmdSet;

    // Execute GPU/CPU commands on the RT
    cmdSet.ExecuteCommandsRT(context);

    // Signal NextCapture completion for Advance thread in case of wait
    {
        Mutex::Locker mlock(&pQueue->CommandSetMutex);
        pQueue->pRTCommands = 0;
        pQueue->CommandSetWC.NotifyAll();
    }

    // Remove ref added in ExecuteCommandsAndWait()
    pQueue->Release();

    // Notify Execute done. Must happen after pQueue->Release, but before this->Release()
    ExecuteDone.SetEvent();

    Release();
}

class ExecuteCommandsRTSceneManagement
{
public:

    ExecuteCommandsRTSceneManagement(HAL& h) : hal(h), SceneHandled(false), WasInFrame(true), WasInScene(false) { }

    void OpenScene()
    {
        if (!SceneHandled)
        {
            if ( (hal.GetHALState() & HAL::HS_InScene) )
            {
                WasInScene = true;
                hal.EndScene();
            }
            if ( (hal.GetHALState() & HAL::HS_InFrame) == 0)
            {
                hal.BeginFrame(); 
                WasInFrame = false;
            }
            SceneHandled = true;
        }
    }

    void CloseScene()
    {
        if (SceneHandled)
        {
            if ( !WasInFrame )
                hal.EndFrame(); // TODOBM: what if not in frame?
            if ( WasInScene )
                hal.BeginScene();
        }
    }

private:
    HAL& hal;
    bool SceneHandled;
    bool WasInFrame;
    bool WasInScene;

    // Fix assignment error warning
    ExecuteCommandsRTSceneManagement& operator=(const ExecuteCommandsRTSceneManagement& other) { SF_UNUSED(other); return *this; }
};

void DICommandSet::ExecuteCommandsRT(DICommandContext& context)
{
    Render::HAL& hal = *context.pHAL;
    bool gpuEvent = false;
    ExecuteCommandsRTSceneManagement sceneManager(hal);

    while(!QueueList.IsEmpty())
    {
        DIQueuePage* page = QueueList.GetFirst();
        page->RemoveNode();

        // Trigger the GPU event.
        if (!gpuEvent)
        {
            hal.GetEvent(Event_DrawableImage).Begin("Scaleform::Render::DrawableImage");
            gpuEvent = true;
        }

        // Execute & destroy all commands in the page.
        DICommand* cmd = page->getFirst();           
        Ptr<DrawableImage> drawableTarget = 0;
        bool targetOnStack = false;

        while(cmd)
        {
            // We have not rendered a DrawableImage yet. Do Scene/Display setup via HAL.
            Ptr<DrawableImage> nextDrawableTarget = cmd->pImage.GetPtr();
            unsigned caps = cmd->GetRenderCaps();

            // If we're changing targets, choose based on the GPU preference flag. Otherwise,
            // give preference to whatever mode we're currently in (if possible).
            bool needsRenderTarget;
            if (drawableTarget != nextDrawableTarget || !drawableTarget || !targetOnStack)
                needsRenderTarget = ((caps & DICommand::RC_GPUPreference) || ((caps & DICommand::RC_CPU) == 0)) && ((caps & DICommand::RC_GPU_NoRT) == 0);
            else
                needsRenderTarget = targetOnStack ? (caps&DICommand::RC_GPU) != 0 : (caps&DICommand::RC_CPU) == 0;

            // Ensure that we can render to the target first.
            if (needsRenderTarget && !nextDrawableTarget->isRenderableRT())
            {
                sceneManager.OpenScene();
                if (!nextDrawableTarget->ensureRenderableRT())
                {
                    SF_DEBUG_WARNING(1, "Unable to render to DrawableImage target.");
                    cmd = page->destroyAndGetNext(cmd);
                    continue;
                }
            }

            if ((!drawableTarget || !targetOnStack) && needsRenderTarget)
            {
                sceneManager.OpenScene();
                drawableTarget = nextDrawableTarget;
                
                RectF imageRect(drawableTarget->GetSize());
                hal.PushRenderTarget(imageRect, drawableTarget->GetRenderTarget(), HAL::PRT_NoClear|HAL::PRT_Resolve);
                targetOnStack = true;
                hal.BeginScene();

                // Alpha0 - we do not want to actually clear the target.
                Viewport vp(drawableTarget->GetSize(), imageRect, Viewport::View_IsRenderTexture);
                hal.BeginDisplay(Color::Alpha0, vp);
            }
            else if ( drawableTarget != nextDrawableTarget )
            {
                if (targetOnStack)
                {
                    // We are rendering to a new drawableTarget, so pop the old one, and push the new one.
                    SF_DEBUG_ASSERT(hal.RenderTargetStack.Back().pRenderTarget == drawableTarget->GetRenderTarget(),
                        "DrawableImage's RenderTarget should be the last one on the HAL stack");
                    drawableTarget = nextDrawableTarget;
                    SF_DEBUG_ASSERT((hal.GetHALState()&HAL::HS_InDisplay), "Internal error, to be in Display bracket.");

                    hal.EndDisplay();
                    hal.EndScene();

                    pQueue->updateCPUModifiedImagesRT();
                    hal.PopRenderTarget(HAL::PRT_Resolve);

                    // If the system has a concept of fencing, insert a fence for this DrawableImage (after the PopRenderTarget,
                    // because on certain systems this may do a texture resolve, which we need to wait for).
                    if (context.pHAL->GetRenderSync())
                        drawableTarget->pFence = context.pHAL->GetRenderSync()->InsertFence();

                    targetOnStack = false;
                }

                hal.GetEvent(Event_DrawableImage).End();
                hal.GetEvent(Event_DrawableImage).Begin("Scaleform::Render::DrawableImage");

                if (needsRenderTarget)
                {
                    sceneManager.OpenScene();
                    RectF imageRect(drawableTarget->GetSize());
                    hal.PushRenderTarget(imageRect, drawableTarget->GetRenderTarget(), HAL::PRT_NoClear|HAL::PRT_Resolve);
                    targetOnStack = true;
                    hal.BeginScene();

                    // Alpha0 - we do not want to actually clear the target.
                    Viewport vp(drawableTarget->GetSize(), imageRect);
                    hal.BeginDisplay(Color::Alpha0, vp);
                }
            }

            // Execute the command.
            cmd->ExecuteRT(context);
            cmd = page->destroyAndGetNext(cmd);
        }

        // If we rendered to a target, then we need to EndDisplay/Scene.
        if (drawableTarget && targetOnStack)
        {
            hal.EndDisplay();
            hal.EndScene();

            // We are finished rendering, cleanup and pop the RT (if we had one). Must come after EndScene, as
            // the render will not be flushed until then, and the stack may be incorrect.
            SF_DEBUG_ASSERT(hal.RenderTargetStack.Back().pRenderTarget == drawableTarget->GetRenderTarget(),
                "DrawableImage's RenderTarget should be the last one on the HAL stack");

            pQueue->updateCPUModifiedImagesRT();
            hal.PopRenderTarget(HAL::PRT_Resolve);
            // If the system has a concept of fencing, insert a fence for this DrawableImage (after the PopRenderTarget,
            // because on certain systems this may do a texture resolve, which we need to wait for).
            if (context.pHAL->GetRenderSync())
                drawableTarget->pFence = context.pHAL->GetRenderSync()->InsertFence();

            targetOnStack = false;
        }

        if (gpuEvent)
            hal.GetEvent(Event_DrawableImage).End();

        // Add back to queue free list
        pQueue->freePage(page);
    }

    QueueList.Clear();

    // Once finished with commands, copy modified results between staging and render targets.
    pQueue->updateCPUModifiedImagesRT();
    pQueue->updateGPUModifiedImagesRT();

    sceneManager.CloseScene();
}

}}; // namespace Scaleform::Render

