/**************************************************************************

Filename    :   Render_Queue.cpp
Content     :   RenderQueue and its processing
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Queue.h"
#include "Render_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// ***** Rendering Queue

RenderQueue::RenderQueue()
 :  QueueSize(0), pQueue(0),
    QueueHead(0), QueueTail(0),
    HeadReserved(false)
{
}

RenderQueue::~RenderQueue()
{
    Shutdown();
}

bool RenderQueue::Initialize( UPInt itemCount )
{
    // Can only initialize the queue once.
    SF_ASSERT(pQueue == 0);

    // If the queue only has one item, rendering will fail, need at least two.
    if ( itemCount < 2 )
        return false;

    pQueue    = (RenderQueueItem*)SF_HEAP_AUTO_ALLOC(this, sizeof(RenderQueueItem) * itemCount);
    QueueSize = (unsigned)itemCount;
    SF_ASSERT(pQueue);
    if ( !pQueue )
        return false;

    // Initialize queue elements to be empty.
    for (unsigned i=0; i< QueueSize; i++)
        pQueue[i].Clear();
    return true;
}


void RenderQueue::Shutdown()
{
    if ( pQueue )
    {
        // Queue should be empty once in destructor.
        SF_ASSERT(QueueHead == QueueTail);
        SF_FREE(pQueue);    
        pQueue = 0;
    }
}

RenderQueueItem* RenderQueue::ReserveHead()
{
    // Ensure that there is one spot in the queue.
    if (QueueHead <= QueueTail)
    {
        // We must always leave at least one item slot to distinguish
        // between empty and full queue. Size of 1 means Head is approaching
        // head so fail.
        if ((QueueTail - QueueHead) == 1)
            return 0;
    }
    else if ((QueueSize + QueueTail) == (QueueHead+1))
    {
        return 0;
    }

    RenderQueueItem* pitem = pQueue + QueueHead;
    HeadReserved = true;
    return pitem;
}

void RenderQueue::AdvanceTail()
{    
     RenderQueueItem* pitem = GetTailItem();
     pitem->Clear();
     if (++QueueTail == QueueSize)
         QueueTail = 0;
}



// ***** Primitive processor

RenderQueueProcessor::RenderQueueProcessor(RenderQueue& queue, HAL* hal)
:   pHAL(hal),    
    Queue(queue), QueueMode(MeshCache::QM_WaitForFences), 
	QueuePrepareFilter(QPF_All), QueueEmitFilter(QPF_All)
{
    resetBufferItems();
}

RenderQueueProcessor::~RenderQueueProcessor()
{
}

void    RenderQueueProcessor::BeginFrame()
{   
    Render::MeshCache& meshCache = pHAL->GetMeshCache();
    QueueMode = meshCache.GetQueueMode();
    meshCache.SetRQCacheInterface(&Caches);    

    CurrentItem = Queue.GetTail();
}

void    RenderQueueProcessor::Flush()
{
    // Flush rendering.
    ProcessQueue(RenderQueueProcessor::QPM_All);
    resetBufferItems();
}


void RenderQueueProcessor::drawProcessedPrimitives()
{   
    RenderQueue &queue = Queue;
    if (Caches.AreCachesLocked())
        Caches.UnlockCaches();

    // Force-process items in the tail of the queue:
    //  a) Unlock the buffers, so that their data is available
    //  b) Proceed forward drawing items until we hit un-cached or
    //     virtual batch.
    //      - if it points to next, then this pointer will need to be updated
    //        one virtual batch is decoded. Or draw location can have a
    //        special flag as 'virtual next'.

    while (queue.GetTail() != CurrentItem)
    {
        RenderQueueItem* pitem = queue.GetTailItem();
        pitem->EmitToHAL(*this);
        queue.AdvanceTail();
    }

    // CarrentItem == Head means the queue is empty, so don't process that.
    if (CurrentItem != queue.GetHead())
    {
        // Draw the front part of the batch being currently processed
        queue.GetTailItem()->EmitToHAL(*this);
    }

    // After we draw, reset the prepare and emit buffers, as the same RQ items could
    // be used again and the system could erroneously think that they have already started
    // prepare/emit steps.
    resetBufferItems();
}


// Called during Draw to process as many items as possible
void    RenderQueueProcessor::ProcessQueue(QueueProcessMode mode)
{
    bool waitForCache = false;
    
    if (mode != QPM_Any)
    {
        if (QueueMode == MeshCache::QM_WaitForFences)
            waitForCache = true;
        else
        {
            SF_ASSERT(QueueMode == MeshCache::QM_ExtendLocks);
            if (Caches.AreCachesLocked())
                Caches.UnlockCaches();
        }
    }

    // TBD: This may need cleanup, especially on consoles.
    // Although processCurrentPrimitive will return PR_Done for any already processed
    // primitive if it is already in cache, enabling queuing, that is not the case
    // for currently tessellated shape with no cache. Such shape would return PR_NeedCache
    // and will thus keep spinning even for QM_Any/QM_One, while it could've returned
    // to try on a later pass...
    // All the checks can also be optimized...

    while(CurrentItem != Queue.GetHead())
    {
        RenderQueueItem* pitem = CurrentItem.GetItem();
        RenderQueueItem::QIPrepareResult result;
        
        // Execute Prepare/Draw loop, if necessary.
        while ((result = pitem->Prepare(*this, waitForCache)) != RenderQueueItem::QIP_Done)
        {
            // Some systems require an actual wait call if there are items available to be drawn, because rendering
            // the current items to completion is expensive.
            if (result == RenderQueueItem::QIP_NeedCache)
                waitForCache = true;

            drawProcessedPrimitives();
        }
   
        if (mode == QPM_One)
            waitForCache = false;

        // Move pointer to next
        ++CurrentItem;
    }

    if ((QueueMode != MeshCache::QM_ExtendLocks) || !Caches.AreCachesLocked())
    {
        drawProcessedPrimitives();
    }
}


}} // Scaleform::Render

