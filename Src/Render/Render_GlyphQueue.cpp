/**************************************************************************

Filename    :   Render_GlyphQueue.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

----------------------------------------------------------------------
The code of recursiveBlur was taken from the Anti-Grain 
Geometry Project and modified for the use by Scaleform/Autodesk. 
Permission to use without restrictions is hereby granted to 
Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
See http://antigrain.com for details.
**************************************************************************/
#include "Kernel/SF_Types.h"
#include "Kernel/SF_AmpInterface.h"

//#ifdef SF_RENDER_ENABLE_GLYPH_CACHE

#include "Render_GlyphQueue.h"

//#include <stdio.h> // DBG

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
GlyphQueue::GlyphQueue() :
    MinSlotSpace(10),
    FirstTexture(0),
    NumTextures(0),
    TextureWidth(0),
    TextureHeight(0),
    MaxSlotHeight(0),
    NumBandsInTexture(0),
    SlotQueueSize(0),
    NumUsedBands(0),
    pEvictNotifier(0)
{}


//------------------------------------------------------------------------
void GlyphQueue::Clear()
{
    GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        // Notify all batches that the slot is evicted. The batch must clean up
        // all notifiers, and as the result clean up this list completely.
        while(!slot->TextFields.IsEmpty())
        {
            pEvictNotifier->Evict(slot->TextFields.GetFirst()->pText);
        }
        slot = slot->pNext;
    }

    GlyphHTable.Clear();

    SlotQueue.Clear();
    ActiveSlots.Clear();

    for (unsigned i = 0; i < NumUsedBands; ++i)
        Bands[i].Slots.Clear();

    Slots.ClearAndRelease();
    Glyphs.ClearAndRelease();

    SlotQueueSize = 0;
    NumUsedBands  = 0;
//printf("C"); // DBG
}

//------------------------------------------------------------------------
void GlyphQueue::Init(GlyphEvictNotifier* notifier, 
                      unsigned firstTexture, unsigned numTextures, 
                      unsigned textureWidth, unsigned textureHeight, 
                      unsigned maxSlotHeight, bool fenceWaitOnFull)
{
    Clear();
    FirstTexture            = firstTexture;
    NumTextures             = numTextures;
    TextureWidth            = textureWidth;
    TextureHeight           = textureHeight;
    MaxSlotHeight           = maxSlotHeight;
    NumBandsInTexture       = textureHeight / maxSlotHeight;
    FenceWaitOnFullCache    = fenceWaitOnFull;

    Bands.Resize(NumBandsInTexture * NumTextures);
    pEvictNotifier = notifier;
}

void GlyphQueue::PinSlot( GlyphSlot* slot )
{
    ++slot->PinCount;
}

void GlyphQueue::UnpinSlot(GlyphSlot* slot, Fence* fence)
{ 
    // If the fence is available and pending, store it with the slot, as long
    // as it is later that the current fence for that slot.
    if ( fence && fence->IsPending(FenceType_Fragment) )
    {
        if ( !slot->SlotFence || *fence > *slot->SlotFence )
            slot->SlotFence = fence;
    }

    SF_ASSERT(slot->PinCount); 
    --slot->PinCount; 
}

//------------------------------------------------------------------------
GlyphNode* GlyphQueue::packGlyph(unsigned w, unsigned h, GlyphNode* glyph)
{
    // A recursive procedure that performs Binary Space Partitioning.
    // It finds an appropriate suitable rectangle and subdivides the 
    // rest of the space. 
    // The method is described in details here: 
    // http://www.blackpawn.com/texts/lightmaps/default.html
    //
    // If glyph->pFont is zero it means an empty space. The children nodes
    // are pNext and pNex2. Name pNext is required by the GPodStructAllocator 
    // and reused in the tree.
    //--------------------------
    if (glyph->Param.pFont)
    {
        GlyphNode* newGlyph = 0;
        if (glyph->pNext) 
            newGlyph = packGlyph(w, h, glyph->pNext);

        if (newGlyph)
            return newGlyph;

        if (glyph->pNex2) 
            newGlyph = packGlyph(w, h, glyph->pNex2);

        return newGlyph;
    }

    if (w <= glyph->mRect.w && h <= glyph->mRect.h)
    {
        unsigned dw = glyph->mRect.w - w;
        unsigned dh = glyph->mRect.h - h;

        // Subdivide space only if it's big enough. If it's not,
        // just use it as a whole.
        //----------------
        if (dw >= MinSlotSpace || dh >= MinSlotSpace)
        {
            if (dw > dh)
            {
                glyph->pNext = Glyphs.Alloc(*glyph);
                glyph->pNext->mRect.x = UInt16(glyph->mRect.x + w);
                glyph->pNext->mRect.w = UInt16(dw);
                if (dh >= MinSlotSpace)
                {
                    glyph->pNex2 = Glyphs.Alloc(*glyph);
                    glyph->pNex2->pNext  = 0;
                    glyph->pNex2->mRect.y = UInt16(glyph->mRect.y + h);
                    glyph->pNex2->mRect.h = UInt16(dh);
                    glyph->pNex2->mRect.w = UInt16(w);
                }
            }
            else
            {
                glyph->pNext = Glyphs.Alloc(*glyph);
                glyph->pNext->mRect.y = UInt16(glyph->mRect.y + h);
                glyph->pNext->mRect.h = UInt16(dh);
                if (dw >= MinSlotSpace)
                {
                    glyph->pNex2 = Glyphs.Alloc(*glyph);
                    glyph->pNex2->pNext  = 0;
                    glyph->pNex2->mRect.x = UInt16(glyph->mRect.x + w);
                    glyph->pNex2->mRect.w = UInt16(dw);
                    glyph->pNex2->mRect.h = UInt16(h);
                }
            }
        }
        glyph->mRect.w = UInt16(w);
        glyph->mRect.h = UInt16(h);
        return glyph;
    }
    return 0;
}

//------------------------------------------------------------------------
void GlyphQueue::splitSlot(GlyphSlot* slot, unsigned w)
{
    // Split the slot as "w" and "slot->w - w". The slot must be 
    // empty, but with the initialized BSP root. These conditions
    // are preserved by the callers.
    //-----------------------
    GlyphSlot* newSlot   = initNewSlot(slot->pBand, slot->x + w, slot->w - w);
    slot->w              = UInt16(w);
    slot->pRoot->mRect.w = UInt16(w);
    SlotQueue.PushFront(newSlot);
    SlotQueueSize++;
    slot->pBand->Slots.InsertAfter(slot, newSlot);
    ActiveSlots.PushFront(newSlot);
}

//------------------------------------------------------------------------
void GlyphQueue::splitGlyph(GlyphSlot* slot, bool left, unsigned w)
{
    GlyphNode* root = slot->pRoot;
    GlyphSlot* newSlot;
    unsigned newW = root->mRect.w - w;
    if (left)
    {
        newSlot = initNewSlot(slot->pBand, root->mRect.x, newW);
        slot->pBand->Slots.InsertBefore(slot, newSlot);
        slot->x      = UInt16(slot->x + newW);
        root->mRect.x = slot->x;
    }
    else
    {
        newSlot = initNewSlot(slot->pBand, root->mRect.x + w, newW);
        slot->pBand->Slots.InsertAfter(slot, newSlot);
    }
    root->mRect.w = UInt16(w);
    slot->w      = UInt16(slot->w - newW);
    SlotQueue.PushFront(newSlot);
    SlotQueueSize++;
    ActiveSlots.PushFront(newSlot);
}

//------------------------------------------------------------------------
inline GlyphNode* GlyphQueue::packGlyph(unsigned w, unsigned h, GlyphSlot* slot)
{
    // Check to see if the slot is empty (no glyph allocated and 
    // no BSP tree), and its width is twice as much than requested,
    // split it. This splits considerably reduce the number of 
    // BSP tree operations.
    //-------------------------
    GlyphNode* glyph = slot->pRoot;
    if (glyph->Param.pFont == 0 && slot->w > 2*w)
    {
        if (glyph->pNext == 0 && glyph->pNex2 == 0)
        {
            splitSlot(slot, w);
        }
        else
        if (glyph->pNex2 == 0 && 
            glyph->mRect.h == slot->pBand->h &&
            glyph->mRect.w > w)
        {
            // The slot may have a big empty glyph after merging, 
            // which is also good to split.
            //---------------------
            bool left  = glyph->mRect.x == slot->x;
            bool right = glyph->mRect.x + glyph->mRect.w == slot->x + slot->w;
            if (left != right)
                splitGlyph(slot, left, w);
        }
    }

    // An attempt to pack the glyph. First, the recursive packGlyph() 
    // is called, then, if it fails, the number of failures is increased.
    // If this number exceeds a certain value (MaxSlotFailures) the slot
    // is excluded from search. In case packing succeeds the number of failures
    // is decreased to keep the slot active.
    //-----------------------
    glyph = packGlyph(w, h, slot->pRoot);
    if (glyph == 0)
        slot->Failures++;

    if (slot->Failures > MaxSlotFailures)
    {
        ActiveSlots.Remove(slot);
        slot->TextureId |= GlyphSlot::FullFlag;
    }
    else
    if (slot->Failures && glyph)
        slot->Failures--;

    return glyph;
}

//------------------------------------------------------------------------
GlyphNode* GlyphQueue::findSpaceInSlots(unsigned w, unsigned h)
{
    // Search for empty space in the active slots. 
    //-----------------------------

    //// Iterate trough the active slots and try to pack. If
    //// packing fails, but the slot still remains active, move
    //// it to the back of the active slot chain, as most probably
    //// the next glyph will fail too.
    ////---------------------------
    //GlyphSlot* slot = ActiveSlots.GetFirst();
    //GlyphSlot* last = ActiveSlots.GetLast();

    //while (!ActiveSlots.IsNull(slot) && slot != last)
    //{
    //    GlyphSlot* next  = ActiveSlots.GetNext(slot);
    //    GlyphNode*     glyph = packGlyph(w, h, slot);

    //    if (glyph)
    //        return glyph;

    //    if ((slot->TextureId & GlyphSlot::FullFlag) == 0)
    //        ActiveSlots.SendToBack(slot);

    //    slot = next;
    //}
    //return ActiveSlots.IsNull(last) ? 0 : packGlyph(w, h, last);


    // Simple variant. Just iterate trough the active slots 
    // and try to pack, keeping the active slot chain as is.
    //----------------------------
    GlyphSlot* slot = ActiveSlots.GetFirst();
    while (!ActiveSlots.IsNull(slot))
    {
        GlyphSlot* next  = ActiveSlots.GetNext(slot);
        GlyphNode* glyph = packGlyph(w, h, slot);
        if (glyph)
            return glyph;
        slot = next;
    }

    return 0;
}

//------------------------------------------------------------------------
GlyphSlot* GlyphQueue::initNewSlot(GlyphBand* band, unsigned x, unsigned w)
{
    // Allocate and init a new slot and the BSP tree root.
    //----------------------
    GlyphSlot* slot = Slots.Alloc();
    GlyphNode* glyph = Glyphs.Alloc();

    slot->pRoot       = glyph;
    slot->pBand       = band;
    slot->TextureId   = band->TextureId;
    slot->x           = UInt16(x);
    slot->w           = UInt16(w);
    slot->Failures    = 0;
    slot->PinCount    = 0;
    slot->TextFields.Clear();
    slot->SlotFence.NullWithoutRelease();

    glyph->Param.Clear();
    glyph->pSlot      = slot;
    glyph->pNext      = 0;
    glyph->pNex2      = 0;
    glyph->mRect       = GlyphRect(slot->x, band->y, slot->w, band->h);
    glyph->Origin.x   = 0;
    glyph->Origin.y   = 0;
    return slot;
}


//------------------------------------------------------------------------
GlyphNode* GlyphQueue::allocateNewSlot(unsigned w, unsigned h)
{
    // Moved down here so we don't count early returns
    SF_AMP_SCOPE_RENDER_TIMER_ID("GlyphQueue::allocateNewSlot", Amp_Native_Function_Id_GlyphCache_RasterizeGlyph);

    // Allocate a new slot if available. First, check for the necessity
    // to initialize a new band. 
    //----------------------
    if (NumUsedBands == 0 || Bands[NumUsedBands-1].RightSpace < w)
    {
        if (NumUsedBands < Bands.GetSize())
        {
            unsigned bandInTexture = NumUsedBands % NumBandsInTexture;
            GlyphBand& band = Bands[NumUsedBands];

            band.TextureId  =  UInt16(FirstTexture + NumUsedBands / NumBandsInTexture);
            band.y          =  UInt16(bandInTexture * MaxSlotHeight);
            band.h          =  UInt16((bandInTexture+1 == NumBandsInTexture) ? 
                                       TextureHeight - band.y : MaxSlotHeight);
            band.RightSpace =  UInt16(TextureWidth);
            band.Slots.Clear();
            ++NumUsedBands;
        }
    }

    // Check for the empty space in the right of the last band. 
    // If there is any and it fits the glyph, initialize the new slot. 
    // Also, check for the rest of the space. If it's less than 2*w
    // merge the rest with the slot.
    // -----------------------
    GlyphBand& band = Bands[NumUsedBands-1];
    if (w <= band.RightSpace)
    {
        unsigned dw = band.RightSpace - w;
        unsigned ws = (dw < w) ? band.RightSpace : w;

        GlyphSlot* slot = initNewSlot(&band, TextureWidth - band.RightSpace, ws);

        band.RightSpace = UInt16(band.RightSpace - slot->w);
        SlotQueue.PushBack(slot); 
        SlotQueueSize++;
        band.Slots.PushBack(slot);
        ActiveSlots.PushFront(slot);
        return packGlyph(w, h, slot);
    }
    return 0;
}


//------------------------------------------------------------------------
void GlyphQueue::releaseGlyphTree(GlyphNode* glyph)
{
    // Recursively release the glyph BSP tree and remove the 
    // glyphs from the hash table.
    //----------------------
    if (glyph)
    {
        releaseGlyphTree(glyph->pNext);
        releaseGlyphTree(glyph->pNex2);
        if (glyph->Param.pFont)
        {
            GlyphParamHash h(&glyph->Param);
            GlyphHTable.Remove(h);
        }
        glyph->Param.pFont = 0;
        Glyphs.Free(glyph);
    }
}

//------------------------------------------------------------------------
void GlyphQueue::releaseSlot(GlyphSlot* slot)
{
    // Release the slot. The whole BSP tree is being released, except
    // for the root. The root glyph is also released (if any), but 
    // the space is initialized as empty to be reused later.
    //-----------------------
    bool isPinned = IsPinned(slot, true);
    SF_ASSERT(!isPinned); SF_UNUSED(isPinned);

    releaseGlyphTree(slot->pRoot->pNext);
    releaseGlyphTree(slot->pRoot->pNex2);

    // Notify all batches that the slot is evicted. The batch must clean up
    // all notifiers, and as the result clean up this list completely.
    while(!slot->TextFields.IsEmpty())
    {
        pEvictNotifier->Evict(slot->TextFields.GetFirst()->pText);
    }

    if (slot->pRoot->Param.pFont)
    {
        GlyphParamHash h(&slot->pRoot->Param);
        GlyphHTable.Remove(h);
    }

    GlyphBand* band = slot->pBand;
    if (band->RightSpace && band->Slots.IsLast(slot))
    {
        // Merge the right space with the slot.
        slot->w = UInt16(slot->w + band->RightSpace);
        band->RightSpace = 0;
    }

    slot->pRoot->Param.pFont = 0;
    slot->pRoot->mRect  = GlyphRect(slot->x, slot->pBand->y, slot->w, slot->pBand->h);
    slot->pRoot->pNext = 0;
    slot->pRoot->pNex2 = 0;
    slot->Failures     = 0;
    slot->SlotFence.Clear();

    if (slot->TextureId & GlyphSlot::FullFlag)
    {
        slot->TextureId &= ~GlyphSlot::FullFlag;
        ActiveSlots.PushFront(slot);
    }
}

//------------------------------------------------------------------------
void GlyphQueue::mergeSlots(GlyphSlot* from, GlyphSlot* to, unsigned w)
{
    // Merge slots "from...to" in the band. All slots are being released 
    // and the ones on the right of "from" are removed.
    //---------------------
    GlyphSlot* slot = from;
    GlyphBand* band = slot->pBand;
    SF_UNUSED(band); // Suppress silly MSVC warning
    for (;;)
    {
        GlyphSlot* next = band->Slots.GetNext(slot);
        releaseSlot(slot);
        if (slot != from)
        {
            Glyphs.Free(slot->pRoot);
            SlotQueue.Remove(slot);
            SlotQueueSize--;
            if ((slot->TextureId & GlyphSlot::FullFlag) == 0)
            {
                ActiveSlots.Remove(slot);
            }
            band->Slots.Remove(slot);
            Slots.Free(slot);
        }
        if (slot == to)
            break;
        slot = next;
    }
    from->w              = UInt16(w);
    from->pRoot->mRect.w = UInt16(w);
    SlotQueue.BringToFront(from);
}

//------------------------------------------------------------------------
GlyphSlot* GlyphQueue::mergeSlotWithNeighbor(GlyphSlot* slot)
{
    // Merge the slot with its right or left neighbor in the band. 
    // This procedure prevents the old slots from "hanging" in the queue
    // for a long time.
    //-----------------------
    GlyphBand* band   = slot->pBand;
    GlyphSlot* target = band->Slots.GetNext(slot);
    bool toRight = true;
    if (band->Slots.IsNull(target))
    {
        target  = band->Slots.GetPrev(slot);
        toRight = false;
    }

    // Restrict merging. Merge only if the target on the right is no wider
    // than the given slot. It prevents from having too wide slots.
    //-----------------------
    if (!band->Slots.IsNull(target) && target->w <= slot->w)
    {
        GlyphNode* slotGlyph   = slot->pRoot;
        GlyphNode* targetGlyph = target->pRoot;
        releaseSlot(slot);
        unsigned x = slot->x;
        unsigned w = slot->w;
        SlotQueue.Remove(slot);
        SlotQueueSize--;
        if ((slot->TextureId & GlyphSlot::FullFlag) == 0)
        {
            ActiveSlots.Remove(slot);
        }
        band->Slots.Remove(slot);
        Slots.Free(slot);

        bool mergeRect = false;
        if (targetGlyph->Param.pFont == 0 && 
            targetGlyph->pNex2 == 0 &&
            targetGlyph->mRect.h == band->h)
        {
            mergeRect = toRight ?
                (x + w == unsigned(targetGlyph->mRect.x)) :
                (unsigned(targetGlyph->mRect.x + targetGlyph->mRect.w) == x);

        }
        if (mergeRect)
        {
            Glyphs.Free(slotGlyph);
            if (toRight)
                targetGlyph->mRect.x = UInt16(x);
            targetGlyph->mRect.w     = UInt16(targetGlyph->mRect.w + w);
        }
        else
        {
            slotGlyph->pNext  = targetGlyph;
            slotGlyph->pNex2  = 0;
            slotGlyph->pSlot  = target;
            slotGlyph->mRect  = GlyphRect(x, band->y, w, band->h);
            target->pRoot     = slotGlyph;
        }

        if (toRight)
            target->x = UInt16(x);
        target->w     = UInt16(target->w + w);

        if (target->TextureId & GlyphSlot::FullFlag)
        {
            target->Failures = 0;
            target->TextureId &= ~GlyphSlot::FullFlag;
            ActiveSlots.PushFront(target);
        }
        return target;
    }
    return 0;
}

//------------------------------------------------------------------------
bool GlyphQueue::checkDistance(GlyphSlot* slot) const
{
    // Check the distance to the beginning. The function returns 
    // true if the slot is closer to the beginning of the queue. 
    //------------------
    GlyphSlot* slot1 = slot;
    GlyphSlot* slot2 = slot;
    for(;;)
    {
        if (SlotQueue.IsNull(slot1))
            return true;

        if (SlotQueue.IsNull(slot2))
            break;

        slot1 = SlotQueue.GetPrev(slot1);
        slot2 = SlotQueue.GetNext(slot2);
    }
    return false;
}

//------------------------------------------------------------------------
GlyphNode* GlyphQueue::evictOldSlot(unsigned w, unsigned h, unsigned pass)
{
    // Try to find an appropriate slot. First, iterate through the 
    // slot queue and find the slot of the required width. To keep
    // the LRU strategy in action, we iterate through only the first
    // half of the queue. Then, if the attempt fails, try to 
    // merge slots.
    //-------------------
    GlyphSlot* slot = SlotQueue.GetFirst();
    unsigned i;
    unsigned queueLimit = pass ? SlotQueueSize : (SlotQueueSize / 2);
    for (i = 0; i <= queueLimit && !SlotQueue.IsNull(slot); ++i)
    {
        if (!IsPinned(slot, FenceWaitOnFullCache))
        {
            if (unsigned(slot->w) >= w)
            {
                releaseSlot(slot); // Slot is found, release it and pack.
                return packGlyph(w, h, slot);
            }
            else
            {
                // In case the slot is narrower than requested, 
                // merge it with its neighbor. It prevents small glyphs 
                // from "hanging" in the beginning of the queue for a 
                // long time. 
                //--------------------
                GlyphSlot* mergedWith = mergeSlotWithNeighbor(slot);
                if (mergedWith)
                {
                    if (mergedWith->pRoot->mRect.w >= w)
                    {
                        return packGlyph(w, h, mergedWith);
                    }
                    break;
                }
            }
        }
//else printf("%d", slot->PinCount); // DBG
        slot = SlotQueue.GetNext(slot);
    }


    // An attempt to find the existing slot failed. Try to merge.
    // To do this we iterate the slots along the band until the 
    // required width is reached.
    //--------------------
    slot = SlotQueue.GetFirst();
    while (!SlotQueue.IsNull(slot))
    {
        if (!IsPinned(slot, FenceWaitOnFullCache))
        {
            unsigned w2 = 0;
            GlyphSlot* to = slot;
            GlyphBand* band = slot->pBand;
            while (!band->Slots.IsNull(to) && !IsPinned(to, FenceWaitOnFullCache))
            {
                bool canContinue = (pass == 0) ? checkDistance(to) : true;
                if (to != slot && !canContinue)
                    break;

                // Check to see if there's enough space in the slot.
                // If there is, reuse it without merging.
                // It prevents from obtaining too wide slots.
                //-----------------------
                if (to->w >= w)
                {
                    releaseSlot(to);
                    return packGlyph(w, h, to);
                }

                w2 += to->w;

                // Try to incorporate space on the right of the band
                // if there is any.
                //-----------------------
                if (band->RightSpace && 
                    band->Slots.IsLast(to) &&
                    w2 + band->RightSpace >= w)
                {
                    w2 += band->RightSpace;
                    band->RightSpace = 0;
                }

                // Merge slots slot...to in the band.
                if (w2 >= w)
                {
                    mergeSlots(slot, to, w2);
                    return packGlyph(w, h, slot);
                }
                to = band->Slots.GetNext(to);
            }
        }
        slot = SlotQueue.GetNext(slot);
    }

    return 0;
}


//------------------------------------------------------------------------
GlyphNode* GlyphQueue::evictOldSlot(unsigned w, unsigned h)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("GlyphQueue::evictOldSlot", Amp_Native_Function_Id_GlyphCache_EvictText);
    pEvictNotifier->ApplyInUseList();
    GlyphNode* node = evictOldSlot(w, h, 0);
    if (node == 0)
    {
        pEvictNotifier->UpdatePinList();
        node = evictOldSlot(w, h, 1);
    }
    return node;
}


//------------------------------------------------------------------------
TextNotifier* GlyphQueue::CreateNotifier(GlyphNode* node, TextMeshProvider* tm)
{
    // Each glyph slot has a linked list of batch notifiers. When the slot is 
    // evicted from the cache it calls OnEvict() for all batches. It invalidates
    // the batches and makes the renderer to re-build them eventually. It's 
    // reasonable to avoid duplicating the notifiers, so, before creating we
    // check if the batch notifier already exists. It's OK to check only the last
    // element and it works because the batches are always created one by one.
    //----------------------
    GlyphSlot* slot = node->pSlot;
    TextNotifier* notifier = slot->TextFields.GetLast();
    if (slot->TextFields.IsNull(notifier) || tm != notifier->pText)
    {
        notifier = Notifiers.Alloc();
        notifier->pText = tm;
        notifier->pSlot = slot;
        slot->TextFields.PushBack(notifier);
        return notifier;
    }
    return 0;
}


//------------------------------------------------------------------------
void GlyphQueue::RemoveNotifier(TextNotifier* notifier)
{
    notifier->RemoveNode();
    Notifiers.Free(notifier);
}


//------------------------------------------------------------------------
GlyphNode* GlyphQueue::FindGlyph(const GlyphParam& gp)
{
    GlyphParamHash h(&gp);
    GlyphNode** ff = GlyphHTable.Get(h);
    if (ff)
    {
        GlyphNode* glyph = *ff;
        SlotQueue.SendToBack(glyph->pSlot);
        return glyph;
    }
    return 0;
}

//------------------------------------------------------------------------
GlyphNode* GlyphQueue::AllocateGlyph(const GlyphParam& gp, unsigned w, unsigned h)
{
    SF_ASSERT(h <= MaxSlotHeight && w > 0 && h > 0);

    if (h < MinSlotSpace) MinSlotSpace = h;
    if (w < MinSlotSpace) MinSlotSpace = w;

    GlyphNode* glyph = findSpaceInSlots(w, h);
    if (glyph == 0)
    {
        glyph = allocateNewSlot(w, h);
        if (glyph == 0)
            glyph = evictOldSlot(w, h);
// DBG
//if(glyph) printf("X");
    }
    if (glyph)
    {
        glyph->Param      = gp;
        glyph->Origin.x   = 0;
        glyph->Origin.y   = 0;
        SlotQueue.SendToBack(glyph->pSlot);
        GlyphParamHash hash(&glyph->Param);
        if(!GlyphHTable.Get(hash))
        {
            GlyphHTable.Add(hash, glyph);
        }
        return glyph;
    }
    return 0;
}


//------------------------------------------------------------------------
void GlyphQueue::UnpinAllSlots()
{
    GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        slot->PinCount = 0;
        // This causes a fence wait, if the slot is still in use.
        IsPinned(slot,true);
        slot = SlotQueue.GetNext(slot);
    }
}


//------------------------------------------------------------------------
void GlyphQueue::computeGlyphArea(const GlyphNode* glyph, unsigned* used) const
{
    if (glyph)
    {
        if (glyph->Param.pFont)
            *used += glyph->mRect.w * glyph->mRect.h;

        computeGlyphArea(glyph->pNext, used);
        computeGlyphArea(glyph->pNex2, used);
    }
}

//------------------------------------------------------------------------
unsigned GlyphQueue::ComputeUsedArea() const
{
    unsigned totalUsed = 0;
    const GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        unsigned used = 0;
        computeGlyphArea(slot->pRoot, &used);
        totalUsed += used;
        slot = SlotQueue.GetNext(slot);
    }
    return totalUsed;
}


//------------------------------------------------------------------------
inline bool GlyphQueue::slotIsEmpty(const GlyphSlot* slot)
{ 
    return (UPInt(slot->pRoot->pNext) | UPInt(slot->pRoot->pNex2)) == 0;
}

//------------------------------------------------------------------------
inline bool GlyphQueue::slotsAreEmpty(const GlyphSlot* slot1, const GlyphSlot* slot2)
{ 
    return (UPInt(slot1->pRoot->pNext) | UPInt(slot1->pRoot->pNex2)|
            UPInt(slot2->pRoot->pNext) | UPInt(slot2->pRoot->pNex2)) == 0;
}


//------------------------------------------------------------------------
void GlyphQueue::MergeEmptySlots()
{
    for (UPInt i = 0; i < NumUsedBands; ++i)
    {
        GlyphBand& band = Bands[i];
        GlyphSlot* slot = band.Slots.GetFirst();
        while(!band.Slots.IsNull(slot))
        {
            GlyphSlot* next = band.Slots.GetNext(slot);
            if (band.Slots.IsNull(next))
                break;

            if (slotsAreEmpty(slot, next))
            {
                releaseSlot(slot);
                releaseSlot(next);
                //SF_ASSERT(slot->PinCount == 0 && next->PinCount == 0);

                unsigned w = slot->w + next->w;
                Glyphs.Free(next->pRoot);
                SlotQueue.Remove(next);
                SlotQueueSize--;

                if ((next->TextureId & GlyphSlot::FullFlag) == 0)
                {
                    ActiveSlots.Remove(next);
                }

                band.Slots.Remove(next);
                Slots.Free(next);
                slot->w = UInt16(w);
                slot->pRoot->mRect.x = slot->x;
                slot->pRoot->mRect.y = slot->pBand->y;
                slot->pRoot->mRect.w = UInt16(w);
                slot->pRoot->mRect.h = slot->pBand->h;
                SlotQueue.BringToFront(slot);
            }
            else
                slot = next;
        }
    }
}

//------------------------------------------------------------------------
void GlyphQueue::CleanUpTexture(unsigned textureId)
{
    GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        GlyphSlot* next = SlotQueue.GetNext(slot);
        if (unsigned(slot->TextureId & ~GlyphSlot::Mask) == textureId)
        {
            releaseSlot(slot);
            SlotQueue.BringToFront(slot);
        }
        slot = next;
    }
    MergeEmptySlots();
}

//------------------------------------------------------------------------
const GlyphNode* GlyphQueue::findFontInSlot(GlyphNode* glyph, const FontCacheHandle* font)
{
    if (glyph)
    {
        if (glyph->Param.pFont == font)
            return glyph;

        const GlyphNode* g2 = findFontInSlot(glyph->pNext, font);
        if (g2)
            return g2;

        return findFontInSlot(glyph->pNex2, font);
    }
    return 0;
}

//------------------------------------------------------------------------
void GlyphQueue::CleanUpFont(const FontCacheHandle* font)
{
    GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        GlyphSlot* next = SlotQueue.GetNext(slot);
        if (findFontInSlot(slot->pRoot, font))
        {
            // This causes a fence wait, if the slot is still in use.
            IsPinned(slot,true);
            releaseSlot(slot);
            SlotQueue.BringToFront(slot);
        }
        slot = next;
    }
}

//------------------------------------------------------------------------
void GlyphQueue::VisitGlyphs(GlyphQueueVisitor* visitor) const
{
    RectF uvRect;
    const GlyphNode* node;

    const GlyphSlot* slot = SlotQueue.GetFirst();
    while(!SlotQueue.IsNull(slot))
    {
        uvRect.x1 = float(slot->x);
        uvRect.y1 = float(slot->pBand->y);
        uvRect.x2 = float(slot->x + slot->w);
        uvRect.y2 = float(slot->pBand->y + slot->pBand->h);
        visitor->Visit(0, uvRect, (slot->PinCount || slot->SlotFence) ? GlyphQueueVisitor::Rect_SlotPinned : GlyphQueueVisitor::Rect_Slot);
        slot = slot->pNext;
    }

    CachedGlyphsType::ConstIterator it = GlyphHTable.Begin();

    for (; it != GlyphHTable.End(); ++it)
    {
        node = it->Second;
        uvRect.x1 = float(node->mRect.x);
        uvRect.y1 = float(node->mRect.y);
        uvRect.x2 = float(node->mRect.x + node->mRect.w);
        uvRect.y2 = float(node->mRect.y + node->mRect.h);
        visitor->Visit(it->First.Param, uvRect, GlyphQueueVisitor::Rect_Glyph);
    }
}

//------------------------------------------------------------------------
void GlyphQueue::PrintMemStats()
{
    //UPInt bytes = Slots.GetNumBytes() + Glyphs.GetNumBytes() + Notifiers.GetNumBytes() + Bands.GetNumBytes();
    //printf("%u ", bytes);
}

UPInt GlyphQueue::GetBytes() const
{
    return (Slots.GetNumBytes() + Glyphs.GetNumBytes() + Notifiers.GetNumBytes() + Bands.GetNumBytes());
}

}} // Scaleform::Render

//#endif //SF_RENDER_ENABLE_GLYPH_CACHE

