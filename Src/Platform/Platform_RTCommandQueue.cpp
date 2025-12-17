/**************************************************************************

Filename    :   Platform_RTCommandQueue.cpp
Content     :   Render Thread command queue refrence implementation
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform_RTCommandQueue.h"

namespace Scaleform {

//------------------------------------------------------------------------
// ***** CircularDataQueue

CircularDataQueue::CircularDataQueue(UPInt size, MemoryHeap* heap)
    : Tail(0), Head(0), End(0), Size(size)
{
    if (!heap)
        heap = Memory::GetHeapByAddress(this);
    pData = (UByte*)SF_HEAP_MEMALIGN(heap, roundUpSize(size),
                                     DataAlignSize, Stat_Default_Mem);
}
CircularDataQueue::~CircularDataQueue()
{
    // Check that empty.
    SF_ASSERT(PopDataBegin() == 0);
    SF_FREE(pData);
}

// Adds data block of size, returns pointer.
//  - It is expected that command will be constructed there
// Return 0 if no space available.
UByte* CircularDataQueue::PushData(UPInt size)
{
    size = roundUpSize(size);
    SF_ASSERT(size < Size/2);
    UByte* result = 0;

    if (Head >= Tail)
    {
        SF_ASSERT(End == 0);
        if (size <= (Size - Head))
        {
            result = pData + Head;
            Head += size;
        }
        else if (size < Tail)
        {
            result = pData;
            End  = Head;
            Head = size;
            SF_ASSERT(Head != Tail);
        }
    }
    else
    {
        SF_ASSERT(End != 0);
        if ((Tail-Head) > size)
        {
            result = pData + Head;
            Head += size;
            SF_ASSERT(Head != Tail);
        }
    }

    return result;
}

// Finishes PopDataBegin; originally pushed size must be provided.
void CircularDataQueue::PopDataEnd(UPInt size)
{
    size = roundUpSize(size);
    SF_ASSERT(Tail != Head);
    
    Tail += size;
    if (Tail == Head)
    {
        SF_ASSERT(End == 0);
        Tail = Head = 0;
    }
    else if (Tail == End)
    {
        Tail = End = 0;
    }
}


//------------------------------------------------------------------------
// ***** RTCommandQueue

RTCommandQueue::RTCommandQueue(ThreadingType type)
: TType(type), Queue(64*1024),
  ProcessingStopped(false), ConsumerSleeping(false)
{
    if (type == AutoDetectThreading)
    {
        if (Thread::GetCPUCount() > 1)
            TType = MultiThreaded;
        else
            TType = SingleThreaded;
    }
}

RTCommandQueue::~RTCommandQueue()
{
    // TBD: Determine what to do with commands pushed from other threads after the
    // ExitThread / StoppProcessing call but after before exit.
    //  - perhaps apply an exiting state earlier, directly on exit call?

    Lock::Locker lock(&QueueLock);
    freeNotifiers();
}


RTNotifier* RTCommandQueue::allocNotifier_NTS()
{
    RTNotifier* notifier = FreeNotifiers.GetFirst();
    if (!FreeNotifiers.IsNull(notifier))
    {
        notifier->RemoveNode();
        return notifier;
    }
    return SF_NEW RTNotifier;
}

void  RTCommandQueue::freeNotifiers()
{
    SF_ASSERT(BlockedProducers.IsEmpty());
    while(!FreeNotifiers.IsEmpty())
    {
        RTNotifier* notifier = FreeNotifiers.GetFirst();
        notifier->RemoveNode();
        delete notifier;
    }
}


bool RTCommandQueue::pushCommand(CommandConstructor* cmd, RTNotifier** pnotifier)
{
    bool success = true;
    bool wakeConsumer;     
    RTNotifier* notifier = 0;

    if (IsProcessingStopped())
        return false;

    while(1)
    {        
        { // Lock scope
            Lock::Locker lock(&QueueLock);

            if (notifier)
            {
                freeNotifier_NTS(notifier);
                notifier = 0;
            }
            
            wakeConsumer     = ConsumerSleeping;
            ConsumerSleeping = false;

            UByte* data = Queue.PushData(cmd->Size);
            if  (data)
            {
                RTCommand* result = cmd->Construct(data);
                if (result->NeedsWait())
                {
                    SF_ASSERT(pnotifier);
                    *pnotifier = result->pNotifier = allocNotifier_NTS();
                }
            }
            else
            {
                notifier = allocNotifier_NTS();
                BlockedProducers.PushBack(notifier);
            }
        }

        if (wakeConsumer)
            ConsumerEvent.PulseEvent();
        if (notifier)
        {
            notifier->Wait();
            // Notifier will be freed on next iteration within lock scope.
            continue;
        }

        break;
    }

    return success;
}

// debugging
//static RTCommandQueue* RT_queue = 0;

bool RTCommandQueue::PopCommand(RTCommandBuffer* buffer, unsigned delay)
{
    //RT_queue = this;
    SF_ASSERT(TType == MultiThreaded);

    bool success = true;
    bool repeat  = true;
    RTNotifier* notifier = 0;
    
    do {
        { // Lock scope
            Lock::Locker lock(&QueueLock);
            UByte* data = Queue.PopDataBegin();
            if (data)
            {
                // Copy into buffer.
                buffer->SetCommand(data);
                Queue.PopDataEnd(buffer->GetSize());
                notifier = buffer->GetNotifier();
                repeat = false;
                // ConsumerSleeping could be 'true' here if timeout PopCommand was used,
                // which exited while another pushCommand set ConsumerEvent.
                // This even would pass-through next time, leaving ConsumerSleeping = true.
                // If so, clean it up.
                ConsumerSleeping = false;
            }
            else
            {
                ConsumerSleeping = true;
                notifier = 0;
            }

            // Wake any producer blocked due to full queue.
            if (!BlockedProducers.IsEmpty())
            {
                RTNotifier* p = BlockedProducers.GetFirst();
                p->RemoveNode();
                p->Notify();
            }
        }

        if (repeat)
        {
            if (!ConsumerEvent.Wait(delay))
            {
                ConsumerSleeping = false;
                return false;
            }
        }
    } while (repeat);
    return success;
}



} // Scaleform
