/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Queue.h
Content     :   RenderQueue and its processing
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_RenderQueue_H
#define INC_SF_Render_RenderQueue_H

#include "Kernel/SF_Hash.h"
#include "Render_Primitive.h"
#include "Render_MeshCache.h"

namespace Scaleform { namespace Render {


// RenderQueue holds RenderQueueItems which are being prepared for
// rendering.
// It exists to prevent stalling due to cache size limitations (Consoles)
// and to "extend locks" in D3D for performance.

class RenderQueue
{
    // Queue items.
    unsigned          QueueSize;
    RenderQueueItem*  pQueue;

    // QueueHead indexes the next location where and item will be added.
    // QueueTail is the Index of the next item to be processed.
    // QueueHead must always be < QueueTail, since (QueueHead == QueueTail)
    // means that the queue is empty.
    unsigned          QueueHead, QueueTail;

    // Set to true after ReserveHead call, but before AdvanceHead().
    bool              HeadReserved;

public:

    enum
    {
        // The default number of items in the queue.
        DefaultQueueSize = 256
    };

    RenderQueue();
    ~RenderQueue();

    // Initialize the queue, with the given number of items.
    bool Initialize( UPInt itemCount = DefaultQueueSize );
    void Shutdown();
    
    // Reserved space for one queue item; returns 0 if that
    // is impossible due to buffers being full.
    RenderQueueItem* ReserveHead();
    
    // Advanced the head. Should be called after the reserved head
    // queue item has been initialized.
    void             AdvanceHead()
    {
        SF_ASSERT(HeadReserved);
        HeadReserved = false;        
        if (++QueueHead == QueueSize)
            QueueHead = 0;
    }

    RenderQueueItem* GetTailItem()
    {
        SF_ASSERT(QueueHead != QueueTail);
        return pQueue + QueueTail;
    }
    
    void             AdvanceTail();

    class Iterator
    {
        RenderQueue *pQueue;
        unsigned    QueuePos;
    public:

        Iterator(RenderQueue *pq = 0, unsigned pos = 0) : pQueue(pq), QueuePos(pos) { }
        Iterator(const Iterator& src) : pQueue(src.pQueue), QueuePos(src.QueuePos) { }

        void operator = (const Iterator& src)
        {
            pQueue   = src.pQueue;
            QueuePos = src.QueuePos;
        }

        bool operator == (const Iterator& i) const
        {
            SF_ASSERT(pQueue == i.pQueue);
            return (QueuePos == i.QueuePos);
        }
        bool operator != (const Iterator& i) const
        {
            SF_ASSERT(pQueue == i.pQueue);
            return (QueuePos != i.QueuePos);
        }

        void operator ++ ()
        {
            SF_ASSERT(QueuePos != pQueue->QueueHead);
            QueuePos++;
            if (QueuePos == pQueue->QueueSize)
                QueuePos = 0;
        }

        RenderQueueItem* GetItem() const { return pQueue->pQueue + QueuePos; }

        bool    IsAtTail() const { return QueuePos == pQueue->QueueTail; }
        bool    IsAtHead() const { return QueuePos == pQueue->QueueHead; }
    };

    Iterator    GetHead()   { return Iterator(this, QueueHead); }
    Iterator    GetTail()   { return Iterator(this, QueueTail); }
};



// RQPrepareBuffer and RQEmitBuffer provider temporary storage for the current
// RenderQueueItem being prepared and emitted, respectively.

struct RQPrepareBuffer
{
    enum { BufferSize = sizeof(void*) * 24 };
    
    bool   IsProcessing(void* p) const { return p == pItem; }
    void   StartProcessing(void* item) { pItem = item; }
    void   ClearItem() { pItem = 0; }
    void*  GetItem() const { return pItem; }
private:
    void* pItem;
};

struct RQEmitBuffer
{
    enum { BufferSize = sizeof(void*) * 8 };    
    bool   IsEmitting(void* p) const { return p == pItem; }
    void   StartEmitting(void* item) { pItem = item; }
    void   ClearItem() { pItem = 0; }
    void*  GetItem() const { return pItem; }
private:
    void* pItem;
};



// RenderQueueProcessor is responsible for processing and rendering items
// placed in the RenderQueue. In general, rendering items are processed in
// several steps:
//   1. Batch Conversion - virtual batches are converted to fixed-size/type
//      batches that can fit the hardware.
//   2. Preparation - this brings the batches into the mesh cache.
//   3. Rendering - queued up batches are drawn.
//

class RenderQueueProcessor
{
public:

    RenderQueueProcessor(RenderQueue& queue, HAL* phal);
    ~RenderQueueProcessor();

    // QueueProcessMode controls how many items in the queue need to be processed.
    // Processing may be stalled due to mesh cache buffer limitation, which forces rendered
    // meshes to be evicted. Such eviction stalls if HW GPU fences for the evicted items
    // are not yet passed (so mesh data is still in use).
    enum QueueProcessMode
    {
        QPM_Any,    // Process zero or more items.
        QPM_One,    // At least one item needs to be processed, even if this forces CPU spins.
        QPM_All     // All of the queued up items have to be processed, spin till that is the case.
    };

    // QueueProcessFilter controls filtering of rendering commands in the queue. 
    // The RenderQueueProcessor filtering is queried in each RenderQueueItem::Interface,
    // which must respect these filters on a case-by-case basis.
    enum QueueProcessFilter
    {
        QPF_All,              // All rendering commands are processed.
        QPF_CacheableOnly,    // Only cacheables (ie. FilterPrimitive/BlendPrimitive) commands are processed.
    };

    HAL*               GetHAL() const       { return pHAL; }
    MeshCache*         GetMeshCache() const { return (MeshCache*)Caches.GetCache(Cache_Mesh); }

    // Called to initialize queue precessing in the beginning of the frame.
    void               BeginFrame();

    // Called to flush rendering (called by HAL::Flush).
    void               Flush();

    // Called during Draw to process as many items as possible
    void               ProcessQueue(QueueProcessMode mode);

	// Called to modify/query queue filtering, of both prepare processing and emit processing independently.
	QueueProcessFilter GetQueuePrepareFilter() const				{ return QueuePrepareFilter; }
	void               SetQueuePrepareFilter(QueueProcessFilter qf) { QueuePrepareFilter = qf; }
	QueueProcessFilter GetQueueEmitFilter() const					{ return QueueEmitFilter; }
	void               SetQueueEmitFilter(QueueProcessFilter qf)	{ QueueEmitFilter = qf; }
  
    RQPrepareBuffer*   GetPrepareItemBuffer() { return &PrepareItemBuffer; }
    RQEmitBuffer*      GetEmitItemBuffer()    { return &EmitItemBuffer; }

    RQCacheInterface&   GetQueueCachesRef()  { return Caches; }

private:
    // Draws as much of the queue as possible, freeing up space.
    // It moved the tail of the queue up to and potentially "into" the current item.
    void            drawProcessedPrimitives();

    void            resetBufferItems()
    {
        PrepareItemBuffer.ClearItem();
        EmitItemBuffer.ClearItem();
    }

    // Avoid assignment operator warning (C4512).
    void operator = (const RenderQueueProcessor&) { }

    //  Permanent Data
    HAL*                    pHAL;
    RQCacheInterface        Caches;
    RenderQueue&            Queue;
    MeshCache::QueueMode    QueueMode;
	QueueProcessFilter      QueuePrepareFilter;		// Controls filtering of items being prepared
	QueueProcessFilter      QueueEmitFilter;		// Controls filtering of items being emitted

    // *** State within Render Queue

    // Iterator referring to the current item in the queue worked on
    // by the queue processor. Should be:
    //   NextDrawItem <= CurrentItem <= Queue.Head
    // If (CurrentItem == Queue.Head), then the last item was fully processed.
    RenderQueue::Iterator   CurrentItem;

    union {
        RQPrepareBuffer PrepareItemBuffer;
        UByte           PrepareItemBufferBytes[RQPrepareBuffer::BufferSize];
    };
    union {
        RQEmitBuffer  EmitItemBuffer;
        UByte         EmitItemBufferBytes[RQEmitBuffer::BufferSize];
    };
};


}} // Scaleform::Render

#endif
