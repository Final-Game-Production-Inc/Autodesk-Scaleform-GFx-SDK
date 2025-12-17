/**************************************************************************

Filename    :   Render_Bundle.cpp
Content     :   Implements SortKey and BundleEntry classes, maintaining
                sorted Bundle lists for tree.
Created     :   September 3, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Bundle.h"
#include "Render_TreeCacheNode.h"
#include "Kernel/SF_HeapNew.h"

extern unsigned BundlePatternFrameId;

namespace Scaleform { namespace Render {


SortKeyInterface SortKeyInterface::NullInterface(SortKey_None);

bool BundleEntry::UpdateBundleEntry(TreeCacheRoot* tr, Renderer2DImpl* r, const BundleIterator& ibundles)
{
    Key.UpdateBundleEntry(this, tr, r, ibundles);
    
    if (pBundle)
    {
        // Save pointe since it may be released during update.
        Ptr<Bundle> bundle = pBundle;
        if (bundle->NeedsUpdate())
            bundle->UpdateChain(this);
    }
    return true;
}


//--------------------------------------------------------------------

void BundleEntryRange::VerifyValidChildDepth()
{         
    BundleEntry* p = pFirst;
    while(1)
    {   
        SF_ASSERT(p->pSourceNode->Depth != 0);

        BundleEntry* chain = p;
        while(chain->pChain)
        {
            SF_ASSERT(chain->pSourceNode->Depth != 0);
            chain = chain->pChain;
        }            
        if (p == pLast)
            break;            
        p = p->pNextPattern;
    }
}
   
void BundleEntryRange::StripChainsByDepth(unsigned topDepth)
{
	if (!IsEmpty())
	{
		BundleEntry* p = pFirst;
		do
		{
			BundleEntry* chain = p;
//			SF_ASSERT(p->Removed == false);

			if (p->pBundle)
				p->pBundle->SetNeedsUpdate();

			while(chain->pChain &&
				(chain->ChainHeight <= (chain->pSourceNode->Depth - topDepth)))
			{
//				SF_ASSERT(chain->Removed == false);
				chain = chain->pChain;
			}
			// Strip chain.
			chain->pChain = 0;
			chain->ChainHeight = 0;

			if (p == pLast)
				break;
			p = p->pNextPattern;
		} while (true);
	}

   // No clearing of last!
   // pLast->pNextPattern = 0;
}

// Strips all chains unconditionally; useful for leaf nodes.
void BundleEntryRange::StripChains()
{
    if (IsEmpty())
        return;

    BundleEntry* p = pFirst;
    while(1)
    {   
        BundleEntry* chain = p;
        SF_ASSERT(p->Removed == false);

        if (p->pBundle)
            p->pBundle->SetNeedsUpdate();
        // Strip chain.
        chain->pChain = 0;
        chain->ChainHeight = 0;

        if (p == pLast)
            break;
        p = p->pNextPattern;
    }

   // pLast->pNextPattern = 0;
}


/*
// Simple Match used originally.
bool BundleEntryRange::Match(BundleEntryRange& other) const
{        
    if (other.GetLength() > GetLength())
        return false;

    BundleEntry* p0 = pFirst;
    BundleEntry* p1 = other.pFirst;

    while (p0 && p1)
    {
        if (p0->Key == p1->Key)
            p1 = p1->pNextPattern;
        p0 = p0->pNextPattern;
    }

    return (p1 == 0);
}
*/

inline BundleEntry* BundleEntryRangeMatcher::getLastEntry(UPInt index) const
{
    return (index < LastEntryCount) ? pLastEntries[index] : 0;
}
inline void BundleEntryRangeMatcher::setLastEntry(UPInt index, BundleEntry* entry) const
{
    if (index >= LastEntryCount)
    {
        if (index >= Bundle_MergeLengthLimit)
            return;
        while (LastEntryCount < index)
            pLastEntries[LastEntryCount++] = 0;
        LastEntryCount++;
    }
    pLastEntries[index] = entry;
}


// Match with "key stack".
// Stack is used to enforce proper nesting and matching of masks,
// blend modes and other multi-item effects.
bool BundleEntryRangeMatcher::Match(BundleEntryRange& other, unsigned *mergeDepth) const
{        
    if (other.GetLength() > GetLength())
        return false;

    // We move through the original list (p0) and try to combine
    // items with it from (p1) as they match.
    BundleEntry* p0 = pFirst;
    BundleEntry* p1 = other.pFirst;

    // Empty list always matched.
    if (!p0 || !p1)
        return (p1 == 0);

    enum { KeyStackLimit = 6 };
    unsigned    keyStackTop = 0;
    unsigned    matchTop = 0;
    SortKey*    keyStack[KeyStackLimit]; 
    UPInt       chainIndex = 0;

    // If we have a range-based key:
    //  - If it doesn't match, skip pattern items until range ends.
    //  - If key matches, we can combine nested items.
    //    - a matching item is processed, and we advance.
    //    - on non-matching item we skip forward until nest non-skip
    //      key item.
    //          - if there is a match we move forward in "other" pattern.
    //          - if we hit non-skip transition, fail match.
    
    do {
        // Merge an item if possible (this happens for all key types).
        bool mergedItem = false;
        if ((matchTop == keyStackTop) && (p0->Key.MatchNoOverlap(p1->Key)))
        {
            // Connect p1 to the p0 chain end if we are in the "Merge" step.
            if (mergeDepth)
            {
                // Optimization: Instead of traversing the chain from p0,
                // use last known location.
                BundleEntry* chain = getLastEntry(chainIndex);
                if (!chain)
                    chain = p0;
                while(chain->pChain)
                    chain = chain->pChain;
                
                chain->pChain = p1;
                SF_ASSERT(*mergeDepth < chain->pSourceNode->Depth);
                chain->ChainHeight = (UInt16)(chain->pSourceNode->Depth - *mergeDepth);

                setLastEntry(chainIndex, p1);
                // Done.
            }

            // Once we reach last, set p1 to 0 indicating that matching is finished.
            p1 = (p1 != other.GetLast()) ? p1->pNextPattern : 0;
            mergedItem = true;
        }

        if (p0->Key.IsRangeStartKey())
        {
            if (keyStackTop == KeyStackLimit)
                return false;
            keyStack[keyStackTop++] = &p0->Key;
            // We'll advance and try to match next items.
            if (mergedItem)
                matchTop++;
        }

        else if (keyStackTop != 0)
        {
            SortKeyTransition t =
                keyStack[keyStackTop-1]->GetRangeTransition(p0->Key);

            if (t != SKT_Skip)
            {
                // Closing of the Range keys must match identically.
                //  - technically there can be several sub-ranges that
                //    match, although we can choose to just match the
                //    first one (or fail otherwise).
                if ((matchTop == keyStackTop) && !mergedItem)
                    return false;

                if (t == SKT_End)
                {
                    if (matchTop == keyStackTop)
                        matchTop--;
                    keyStackTop--;
                }
                else
                {
                    SF_ASSERT(t == SKT_Replace);
                    keyStack[keyStackTop-1] = &p0->Key;
                }
            }
        }

        if (p0 == GetLast())
            break;
        p0 = p0->pNextPattern;
        chainIndex++;

    } while(p1);

    // All range-based scopes must end within local subrange.
    SF_ASSERT(keyStackTop == 0);
    return (p1 == 0);
}


//--------------------------------------------------------------------
// ***** Bundle

void Bundle::UpdateChain(BundleEntry *newTop)
{
    NeedUpdate = false;
    // If we meet a bundle, just delete it for now
    UPInt        index;
    BundleEntry* p;        

//    SF_ASSERT(FrameId == BundlePatternFrameId);
    //printf("Updating Bundle %p\n", this);    
    
    // Verify bundle belongs to us in debug builds.
#ifdef SF_BUILD_DEBUG
    for (UPInt i = 0; i< Entries.GetSize(); i++)
    {
        BundleEntry* e = Entries[i];
        SF_ASSERT((e->pBundle == (Bundle*)0) || (e->pBundle == this));
    }
#endif

    for(index = 0, p = newTop; p != 0; index++, p = p->pChain)
    {
        if ((index < Entries.GetSize()) && (p == Entries[index]))
            continue;
        
        // If entry was already in this bundle, 
        // remove items until it is encountered... up to tail.
        if (p->pBundle == this)
        {
            UPInt i = index;
            while ((i < Entries.GetSize()) && (Entries[i] != p))
                i++;
            if (i > index)
            {
                RemoveEntries(index, i - index);
            }
            if ((index < Entries.GetSize()) && (p == Entries[index]))
                continue;
            // TBD: This can be extended to concatenating bundles, etc...
        }

        // Set updates indexHint and removes from earlier bundle, if any.
        p->SetBundle(this, index);
        InsertEntry(index, p);

     //   printf("Bundle %p - Inserted %p at index %d\n", this, p, index);
    }

    // Remove remainder
    if (index < Entries.GetSize())
        RemoveEntries(index, Entries.GetSize()-index);
    pTop = newTop;
}

void Bundle::InsertEntry(UPInt index, BundleEntry* shape)
{        
//    printf("Bundle InsertEntry %p at index %d\n", shape, index);
    Entries.InsertAt(index, shape);
}

void Bundle::RemoveEntries(UPInt index, UPInt count)
{    
    for (unsigned i = 0; i< count; i++)
        Entries[index + i]->clearBundlePtr();
    Entries.RemoveMultipleAt(index, count);
}

void Bundle::UpdateMesh(BundleEntry*)
{
    // Nothing to do.. must override.
}


void Bundle::RemoveEntry(BundleEntry* entry)
{
    UPInt index;
    if (findEntryIndex(&index, entry))
        RemoveEntries(index, 1);
}

bool Bundle::findEntryIndex(UPInt *pindex, BundleEntry* entry)
{        
    if ((entry->IndexHint < Entries.GetSize()) && (Entries[entry->IndexHint] == entry))
    {
        *pindex = entry->IndexHint;
        return true;
    }
    else
    {
        for (UPInt i = 0; i < Entries.GetSize(); i++)
        {
            if (Entries[i] == entry)
            {
                entry->IndexHint = (UInt16)i;
                *pindex = i;
                return true;
            }
        }
    }
    return false;
}



}} // Scaleform::Render

