/**************************************************************************

Filename    :   GFx_DisplayList.cpp
Content     :   Implementation of Diplay List
Created     :   
Authors     :   Artem Bolgar, Michael Antonov, TU

Notes       :   Significantly modified for GFx 3.0

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Log.h"

#include "GFx/GFx_DisplayList.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_FilterDesc.h"
#include "GFx/AMP/Amp_ViewStats.h"

namespace Scaleform { namespace GFx {

DisplayList::DisplayList() 
: DepthToIndexMap(NULL), pCachedChar(NULL), ModId(0), Flags(0) 
{
}

// ***** DisplayList - Character entry access.
SPInt DisplayList::FindDisplayIndex(const DisplayObjectBase* ch) const
{
    for (UPInt i = 0; i < DisplayObjectArray.GetSize(); i++)
    {
        if (DisplayObjectArray[i].GetDisplayObject() == ch)
            return (SPInt)i;
    }
    return -1;
}

// Like the above, but looks for an exact match, and returns -1 if failed.
UPInt DisplayList::GetDisplayIndex(int depth) const
{
    UPInt index = FindDisplayIndex(depth);
    if (index >= DisplayObjectArray.GetSize()
        || GetDisplayEntry(index).GetDepth() != depth)
    {
        // No object at that depth.
        return SF_MAX_UPINT;
    }
    return index;
}


DisplayObjectBase*   DisplayList::GetDisplayObjectAtDepth(int depth, bool* pisMarkedForRemove) const
{
    UPInt index = GetDisplayIndex(depth);
    if (index != SF_MAX_UPINT)
    {
        DisplayObjectBase*   ch = DisplayObjectArray[index].GetDisplayObject();
        SF_ASSERT(ch);
        if (ch->GetDepth() == depth)
        {
            if (pisMarkedForRemove)
                *pisMarkedForRemove = DisplayObjectArray[index].IsMarkedForRemove();
            return ch;
        }
    }

    return NULL;
}

DisplayObjectBase* DisplayList::GetCharacterAtDepth(
    int depth, ResourceId id, UPInt* pindex) const
{
    UPInt index = GetDisplayIndex(depth);
    if (index != SF_MAX_UPINT)
    {
        for (UPInt n = DisplayObjectArray.GetSize(); index < n; ++index)
        {
            Ptr<DisplayObjectBase>  ch = DisplayObjectArray[index].GetDisplayObject();
            SF_ASSERT(ch);
            if (ch->GetDepth() == depth && ch->GetId() == id)
            {
                if (pindex)
                    *pindex = index;
                return ch;
            }
        }
    }

    return NULL;
}

DisplayObjectBase*   DisplayList::GetDisplayObjectAtDepthAndUnmark(DisplayObjectBase* owner, int depth)
{
    UPInt index = GetDisplayIndex(depth);
    if (index != SF_MAX_UPINT)
    {
        DisplayObjectBase*   ch = DisplayObjectArray[index].GetDisplayObject();
        SF_ASSERT(ch);
        if (ch->GetDepth() == depth)
        {
            if (DisplayObjectArray[index].IsMarkedForRemove())
            {
                // re-insert node into the renderer tree
                InsertIntoRenderTree(owner, index);

                DisplayObjectArray[index].ClearMarkForRemove();
            }
            return ch;
        }
    }

    return NULL;
}


// Swaps two objects at depths.
bool    DisplayList::SwapDepths(DisplayObjectBase* owner, int depth1, int depth2, unsigned frame)
{
    if (depth1 == depth2)
        return true;

    UPInt index1 = GetDisplayIndex(depth1);
    if (index1 == SF_MAX_UPINT)
        return false;
    UPInt index2 = FindDisplayIndex(depth2);

    //@TODO: do not apply swap if any of the nodes has indirect transform:
    if ((index1 < DisplayObjectArray.GetSize() && GetDisplayObject(index1)->HasIndirectTransform()) || 
        (index2 < DisplayObjectArray.GetSize() && GetDisplayObject(index2)->HasIndirectTransform()))
        return false;

    pCachedChar = NULL;
    if (index2 >= DisplayObjectArray.GetSize() ||
        GetDisplayEntry(index2).GetDepth() != depth2)
    {
        // Index 2 must be used for insertion.
        // Move the object and set depth.
        DisplayEntry de(DisplayObjectArray[index1]);
        RemoveFromRenderTree(owner, index1);
        DisplayObjectArray.RemoveAt(index1);
        if (index1 < index2)
            index2--;
        DisplayObjectArray.InsertAt(index2, de);
        InsertIntoRenderTree(owner, index2);
    }
    else
    {
        // Otherwise, we are swapping depths with both existing entries.
        Alg::Swap(DisplayObjectArray[index1], DisplayObjectArray[index2]);
        SwapRenderTreeNodes(owner, index1, index2);
        DisplayObjectBase* ch1 = DisplayObjectArray[index1].GetDisplayObject();
        if (ch1)
        {
            ch1->SetDepth(depth1);
            // if depth is changed and char is a timeline object then this
            // char will be removed at the timeline's loop
            ch1->SetCreateFrame(frame + 1);
        }
    }

    DisplayObjectBase* ch2 = DisplayObjectArray[index2].GetDisplayObject();
    if (ch2)
    {
        ch2->SetDepth(depth2);
        // if depth is changed and char is a timeline object then this
        // char will be removed at the timeline's loop
        ch2->SetCreateFrame(frame + 1);
    }

    ++ModId;
    CheckAndInvalidateDepthMappings();
    return true;
}

bool    DisplayList::SwapEntriesAtIndexes(DisplayObjectBase* owner, UPInt origIndex1, UPInt origIndex2)
{
    if (origIndex1 == origIndex2)
        return true;
    SF_ASSERT(origIndex1 < GetCount() && origIndex2 < GetCount());
    DisplayEntry e = DisplayObjectArray[origIndex1];

    //@TODO: do not apply swap if any of the nodes has indirect transform:
    if (e.GetDisplayObject()->HasIndirectTransform() || 
        (origIndex2 < DisplayObjectArray.GetSize() && GetDisplayObject(origIndex2)->HasIndirectTransform()))
        return false;

    ++ModId;
    DisplayObjectArray[origIndex1] = DisplayObjectArray[origIndex2];
    DisplayObjectArray[origIndex2] = e;
    return SwapRenderTreeNodes(owner, origIndex1, origIndex2);
}

// Returns maximum used depth.
int     DisplayList::GetLargestDepthInUse() const
{
    // Display array is sorted by size, so get last value
    UPInt size = DisplayObjectArray.GetSize();
    return size ? DisplayObjectArray[size-1].GetDepth() : -1;
}


DisplayObject* DisplayList::GetDisplayObjectByName(const ASString& name, bool caseSensitive) const
{
    if (name.IsEmpty())
        return NULL;
    DisplayObjectBase*  pch = NULL;

    // See if we have a match on the display list.
    if (caseSensitive)
    {
        if (pCachedChar && pCachedChar->GetName() == name)
            return pCachedChar;
        
        UPInt          i, n = GetCount();
        for (i = 0; i < n; i++)
        {
            DisplayObjectBase* p = GetDisplayObject(i);
            SF_ASSERT(p);
            if (p && p->IsScriptableObject() && p->CharToScriptableObject_Unsafe()->GetName() == name)
            {
                pch = p;
                break;
            }
        }
    }
    else
    {   // Case-insensitive version.
        name.ResolveLowercase();
        if (pCachedChar && name.Compare_CaseInsensitive_Resolved(pCachedChar->GetName()))
            return pCachedChar;

        UPInt          i, n = GetCount();
        for (i = 0; i < n; i++)
        {
            DisplayObjectBase* p = GetDisplayObject(i);
            SF_ASSERT(p);
            if (p && p->IsScriptableObject() && name.Compare_CaseInsensitive_Resolved(p->CharToScriptableObject_Unsafe()->GetName()))
            {
                pch = p;
                break;
            }
        }
    }    
    // Only ScriptableObject's have names.
    SF_ASSERT((pch == 0) || pch->IsScriptableObject());
    if (pch)
        pCachedChar = pch->CharToScriptableObject_Unsafe();
    else
        pCachedChar = NULL;
    return pCachedChar;
}



// ***** DisplayList - Display list management


void    DisplayList::AddDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos, DisplayObjectBase* ch, UInt32 addFlags)    
{
    SF_ASSERT(ch);
    
    int depth   = pos.Depth;
    UPInt size  = DisplayObjectArray.GetSize();
    UPInt index = FindDisplayIndex(depth);
    pCachedChar = NULL;

    if (addFlags & Flags_ReplaceIfDepthIsOccupied)
    {
        // Eliminate an existing object if it's in the way.

        if (index < size)
        {
            DisplayEntry & dobj = DisplayObjectArray[index];

            if (dobj.GetDepth() == depth)
            {               
                // Unload the object at the occupied depth.
                UnloadDisplayObjectAtIndex(owner, index);
                // correct size and index since they might be invalid
                // after UnloadDisplayObjectAtIndex.
                size    = DisplayObjectArray.GetSize();
                index   = FindDisplayIndex(depth);
            }
        }
    }
    else
    {
        // Basically, the display list should contain the only one character at one particular depth. 
        // But there are several exceptions from the depth's uniqueness rule and for the short period 
        // of time the display list may contain characters at the same depth:
        //  1.  There could be several characters at the same depth < -32768. The "semi-dead" characters 
        //      (being removed but having the unload handler) are being moved to the depth = (-32769 - curDepth).
        //      If several characters with onUnload handler were created and removed at the same depth in the same 
        //      frame then we may get several "semi-dead" characters at the same negative depth. These characters 
        //      will be cleaned up once their onUnload handlers are invoked.
        //  2.  It is possible to get characters at the same depth during gotoFrame, when the current character 
        //      is marked for remove and a new character is being added at the same depth. These characters will be 
        //      cleaned up once UnloadMarkedForRemove() function is called right after the gotoFrame is finished.
        //  3.  Another possibility of existence of several characters at the same depth is the gotoFrame backward. 
        //      For example if the character was created at frame 5 and we at the frame 10. Frame 4 contains another 
        //      character at the same depth. Now, we got gotoFrame(5). Since the CreateFrame of the existing character 
        //      is not greater than 5 this object will not be marked for remove. When gotoFrame(5) is executed we 
        //      create character for frame 4 at the same depth as the currently existing frame 5 instance has. 
        //      This extra object will be eliminated once ExecuteFrameTags(5) is executed (right after ExecuteSnapshot
        //      is finished), because the tags for frame 5 will contain RemoveObject for the frame 4 character and 
        //      then AddObject for frame 5. New character will not be re-created here because the existing one will 
        //      be reused. Thus, these two characters will have different CreateFrame. It is also important that 
        //      AddObject adds record for the same depth character IN FRONT OF existing one (so, lower bound algorithm 
        //      should be used). Otherwise, the RemoveObject tag might remove incorrect character (it should also use 
        //      the lower bound algorithm when searching for the character to remove).
        SF_ASSERT(!(size > 0 && (index < size) && 
            DisplayObjectArray[index].GetDepth() == depth && !DisplayObjectArray[index].IsMarkedForRemove() &&
            (DisplayObjectArray[index].GetDisplayObject() && DisplayObjectArray[index].GetDisplayObject()->GetCreateFrame() == ch->GetCreateFrame())));
    }

    ch->SetDepth(depth);

    ch->SetCxform(pos.ColorTransform);
    ch->SetMatrix(pos.Matrix_1);
    ch->SetRatio(pos.Ratio);
    ch->SetClipDepth(pos.ClipDepth);
    ch->SetBlendMode((DisplayObjectBase::BlendType)pos.BlendMode);
    ch->SetFilters(pos.pFilters.GetPtr());
    if (pos.HasVisibility())
        ch->SetVisible(pos.Visible ? true : false);

    // Insert into the display list...
    SF_ASSERT(index == FindDisplayIndex(depth));
    
    AddDisplayObject(owner, index, ch);
}

void    DisplayList::AddDisplayObject(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch)
{
    ++ModId;
    AddEntryAtIndex(owner, index, ch);
    ch->SetUnloaded(false);

    // Do the frame1 Actions (if applicable) and the "OnClipEvent (load)" event.
    ch->OnEventLoad();
    //CheckConsistency(owner);

//    ch->UpdateViewAndPerspective(); 
}

void DisplayList::AddEntryAtIndex(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch)
{
    SF_ASSERT(owner);
    DisplayEntry di;    
    di.SetDisplayObject(ch);

    ++ModId;
    SF_ASSERT(index <= DisplayObjectArray.GetSize());
    DisplayObjectArray.InsertAt(index, di);

    InsertIntoRenderTree(owner, index);

    CheckAndInvalidateDepthMappings();
}

// Updates the transform properties of the object at the specified depth.
void    DisplayList::MoveDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos)
{
    int     depth   = pos.Depth;
    UPInt   size    = DisplayObjectArray.GetSize();
    UPInt   index   = FindDisplayIndex(depth);

    // Bounds check.
    if (index >= size)
    {       
        if (size == 0)
            SF_DEBUG_WARNING(1, "MoveDisplayObject() - no objects on display list");
        else
            SF_DEBUG_WARNING1(1, "MoveDisplayObject() - can't find object at depth %d", depth);      
        return;
    }
    
    DisplayEntry&   di = DisplayObjectArray[index];
    DisplayObjectBase*   ch = di.GetDisplayObject();
    SF_ASSERT(ch);
    if (ch->GetDepth() != depth)
    {       
        SF_DEBUG_WARNING1(1, "MoveDisplayObject() - no object at depth %d", depth);  
        return;
    }

    ++ModId;
    // Object re-added, unmark for remove.
    di.ClearMarkForRemove();
    if (!di.IsInRenderTree())
    {
        // re-insert node into the renderer tree
        InsertIntoRenderTree(owner, index);
    }

    if (!ch->GetAcceptAnimMoves())
    {
        // This DisplayObjectBase is rejecting anim moves.  This happens after it
        // has been manipulated by ActionScript.
        // This GFxCharacter is rejecting anim moves.  This happens after it
        // has been manipulated by ActionScript.
        if (ch->GetContinueAnimationFlag())
            ch->SetAcceptAnimMoves(true); // reset the flag
        else
            return;
    }

    if (pos.HasCxform())  
        ch->SetCxform(pos.ColorTransform);
    if (pos.HasMatrix())
        ch->SetMatrix(pos.Matrix_1);    
    if (pos.HasBlendMode())
        ch->SetBlendMode((DisplayObjectBase::BlendType)pos.BlendMode);
    if (pos.HasVisibility())
        ch->SetVisible(pos.Visible ? true : false);
    ch->SetFilters(pos.pFilters);
    ch->SetRatio(pos.Ratio);

    // MoveDisplayObject apparently does not change clip depth!  Thanks to Alexeev Vitaly.
    // ch->SetClipDepth(ClipDepth);
}


// Puts a new DisplayObjectBase at the specified depth, replacing any
// existing DisplayObjectBase.  If HasCxform or HasMatrix are false,
// then keep those respective properties from the existing character.
void    DisplayList::ReplaceDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos, DisplayObjectBase* ch)   
{   
    UPInt   size    = DisplayObjectArray.GetSize();
    int     depth   = pos.Depth;
    UPInt   index   = FindDisplayIndex(depth);
    
    // Bounds check.
    if (index >= size)
    {
        // Error, no existing object found at depth.
        // Fallback -- add the object.
        AddDisplayObject(owner, pos, ch, Flags_ReplaceIfDepthIsOccupied);
        return;
    }
    
    DisplayEntry&   di = DisplayObjectArray[index];
    if (di.GetDepth() != depth)
    {   
        // This may occur if we are seeking *back* in a movie and find a ReplaceObject tag.
        // In that case, a previous object will not exist, since it would have been removed,
        // if it did.
        AddDisplayObject(owner, pos, ch, Flags_ReplaceIfDepthIsOccupied);
        return;
    }
    
    ++ModId;
    Ptr<DisplayObjectBase>  poldCh = di.GetDisplayObject();
    SF_ASSERT(poldCh);

    // Put the new DisplayObjectBase in its place.
    SF_ASSERT(ch);
    SF_ASSERT(ch != poldCh);
    ch->SetDepth(depth);
    ch->Restart();
    
    // Set the display properties.
    di.ClearMarkForRemove();
    if (!di.IsInRenderTree())
    {
        // re-insert node into the renderer tree
        InsertIntoRenderTree(owner, index);
    }
    di.SetDisplayObject(ch);

    // Use old characters matrices if new ones not specified.
    ch->SetCxform(pos.HasCxform() ? pos.ColorTransform : poldCh->GetCxform());
    ch->SetMatrix(pos.HasMatrix() ? pos.Matrix_1 : poldCh->GetMatrix());
    ch->SetBlendMode(pos.HasBlendMode() ?
        (DisplayObjectBase::BlendType)(pos.BlendMode) : poldCh->GetBlendMode());
    ch->SetRatio(pos.Ratio);
    ch->SetClipDepth(pos.ClipDepth);    
    ch->SetFilters(pos.pFilters);
    if (pos.HasVisibility())
        ch->SetVisible(pos.Visible ? true : false);
    else
        ch->SetVisible(poldCh->GetVisible());

    ReplaceRenderTreeNode(owner, index);

    CheckAndInvalidateDepthMappings();

    pCachedChar = NULL;
    poldCh->OnUnload();
    ch->OnEventLoad();
}

void    DisplayList::ReplaceDisplayObjectAtIndex(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch)
{
    // 1st validity checks.
    if (index >= DisplayObjectArray.GetSize())
    {
        // Error -- no DisplayObjectBase at the given depth.
        SF_DEBUG_WARNING1(1, "ReplaceDisplayObjectAtIndex - invalid index %d", (unsigned)index);
        return;
    }

    ++ModId;
    pCachedChar = NULL;
    DisplayList::DisplayEntry &e = GetDisplayEntry(index);
    e.SetDisplayObject(ch);

    if (e.IsInRenderTree())
        ReplaceRenderTreeNode(owner, index);
    else
        InsertIntoRenderTree(owner, index);

    CheckAndInvalidateDepthMappings();
}

// Removes the object at the specified depth.
void    DisplayList::RemoveDisplayObject(DisplayObjectBase* owner, int depth, ResourceId id)
{
    UPInt   size    = DisplayObjectArray.GetSize();    
    UPInt   index   = FindDisplayIndex(depth);
    
    // 1st validity checks.
    if (index >= size)
    {
        // Error -- no DisplayObjectBase at the given depth.
        SF_DEBUG_WARNING1(1, "RemoveDisplayObject - no DisplayObjectBase at depth %d", depth);
        return;
    }

    Ptr<DisplayObjectBase> ch = GetDisplayObject(index);
    SF_ASSERT(ch);

    // 2nd validity checks.
    if (ch->GetDepth() != depth)
    {
        // Error -- no DisplayObjectBase at the given depth.
        SF_DEBUG_WARNING1(1, "RemoveDisplayObject - no DisplayObjectBase at depth %d", depth);
        return;
    }

    SF_ASSERT(ch->GetDepth() == depth);
    pCachedChar = NULL;
    ++ModId;
    
    if (id != ResourceId::InvalidId)
    {
        // Caller specifies a specific id; scan forward til we find it.
        for (;;)
        {
            if (GetDisplayObject(index)->GetId() == id)
            {
                break;
            }
            if (index + 1 >= size || GetDisplayObject(index + 1)->GetDepth() != depth)
            {
                // Didn't find a match!
                SF_DEBUG_WARNING2(1, "RemoveDisplayObject - no DisplayObjectBase at depth %d with id %d", depth, id.GetIdValue());
                return;
            }
            index++;
        }
        SF_ASSERT(index < size);
        SF_ASSERT(GetDisplayObject(index)->GetDepth() == depth);
        SF_ASSERT(GetDisplayObject(index)->GetId() == id);
    }

    UnloadDisplayObjectAtIndex(owner, index);
}

// bool    DisplayList::RemoveCharacter(DisplayObjectBase* ch)
// {
//     SF_ASSERT(ch);
//     int     depth   = ch->GetDepth();
//     UPInt   size    = DisplayObjectArray.GetSize();    
//     UPInt   index   = FindDisplayIndex(depth);
// 
//     // 1st validity checks.
//     if (index >= size)
//     {
//         return false;
//     }
//     // we need to find a character to remove. The following loop is to iterate through
//     // potential set of different characters at the same depth.
//     DisplayObjectBase* pcurCh = GetDisplayObject(index);
//     while (pcurCh && pcurCh != ch && depth == pcurCh->GetDepth() && index + 1 < size)
//     {
//         pcurCh = GetDisplayObject(++index);
//     }
//     if (index < size && pcurCh == ch)
//     {
//         // we've found a character to remove
//         DisplayObjectArray.RemoveAt(index);
//         pCachedChar = NULL;
//         return true;
//     }
//     return false;
// }

bool DisplayList::RemoveDisplayObject(DisplayObjectBase* owner, const DisplayObjectBase* ch)
{
    SPInt index = FindDisplayIndex(ch);
    if (index >= 0)
    {
        ++ModId;
        UnloadDisplayObjectAtIndex(owner, UPInt(index));
        pCachedChar = NULL;
    }
    return false;
}

void DisplayList::RemoveEntryAtIndex(DisplayObjectBase* owner, UPInt index)
{
    SF_ASSERT(index < DisplayObjectArray.GetSize());
    RemoveFromRenderTree(owner, index);
    DisplayObjectArray.RemoveAt(index);
    pCachedChar = NULL;
    ++ModId;
    CheckAndInvalidateDepthMappings();
}

// void    DisplayList::MarkForRemoval(int depth, ResourceId id)
// {
//     UPInt   size    = DisplayObjectArray.GetSize();    
//     UPInt   index   = FindDisplayIndex(depth);
// 
//     // Validity checks.
//     if (index >= size || GetDisplayObject(index)->GetDepth() != depth)
//     {
//         // Error -- no DisplayObjectBase at the given depth.
//         SF_DEBUG_WARNING1(1, "RemoveDisplayObject - no DisplayObjectBase at depth %d", depth);
//         return;
//     }
// 
//     SF_ASSERT(GetDisplayObject(index) && GetDisplayObject(index)->GetDepth() == depth);
// 
//     if (id != ResourceId::InvalidId)
//     {
//         // Caller specifies a specific id; scan forward til we find it.
//         for (;;)
//         {
//             if (GetDisplayObject(index)->GetId() == id)
//             {
//                 break;
//             }
//             if (index + 1 >= size || GetDisplayObject(index + 1)->GetDepth() != depth)
//             {
//                 // Didn't find a match!
//                 SF_DEBUG_WARNING2(1, "RemoveDisplayObject - no DisplayObjectBase at depth %d with id %d", depth, id.GetIdValue());
//                 return;
//             }
//             index++;
//         }
//         SF_ASSERT(index < size);
//         SF_ASSERT(GetDisplayObject(index)->GetDepth() == depth);
//         SF_ASSERT(GetDisplayObject(index)->GetId() == id);
//     }
// 
//     // Removing the DisplayObjectBase at GetDisplayObject(index).
//     DisplayEntry&   di = DisplayObjectArray[index];
//     di.MarkForRemove(true);
// }

// Clear the display list.
void    DisplayList::Clear(DisplayObjectBase* owner) 
{
    owner->GetMovieImpl()->AddMovieDefToKillList(owner->GetResourceMovieDef());
    while (DisplayObjectArray.GetSize() != 0)
    {
        pCachedChar = NULL;
        DisplayEntry&   di = DisplayObjectArray[0];     
        SF_ASSERT(di.GetDisplayObject());
        ++ModId;
        di.GetDisplayObject()->OnUnload(); // this can change the DisplayObjectArray, so we need to remove element-by-element
        RemoveFromRenderTree(owner, 0);
        di.GetDisplayObject()->SetParent(NULL); //!AB: clear the parent for children in the display list 
        // being cleared, since their parent used to be removed, 
        // but children might stay alive for sometime.
        DisplayObjectArray.RemoveAt(0);
    }   
    pCachedChar = NULL;
    DisplayObjectArray.Clear();
}

bool    DisplayList::UnloadAll(DisplayObjectBase* owner) 
{
    pCachedChar = NULL;
    bool mayRemove = true;
    // don't replace DisplayObjectArray.GetSize() by pre-saved variable here! 
    for (UPInt i = 0; i < DisplayObjectArray.GetSize(); )
    {
        ++ModId;
        // we need to correct index only if UnloadDisplayObjectAtIndex returns true;
        // this means, that the element at the index 'i' was removed and
        // we don't need to increment the index to get the next element.
        // If it returns false, then the element at index 'i' was moved
        // to the beginning of the array (since depth will be negative) and thus
        // we still need to increment the index.
        bool mayRemoveTheChild = UnloadDisplayObjectAtIndex(owner, i);
        if (!mayRemoveTheChild)
        {
            ++i;
            mayRemove = false;
        }
    }   
    SF_ASSERT(!pCachedChar);
    return mayRemove;
}


// Mark all entries for removal.
void    DisplayList::MarkAllEntriesForRemoval(DisplayObjectBase* owner, unsigned targetFrame)  
{   
    UPInt i, n = DisplayObjectArray.GetSize();
    for (i = 0; i < n; i++) 
    {
        DisplayEntry& de = DisplayObjectArray[i];
        Ptr<DisplayObjectBase> ch = de.GetDisplayObject();

        SF_ASSERT(ch);

        // do not mark characters created by ActionScript. But we still need to mark
        // characters, which were touched by AS, if they were created on frame other
        // than 0.
        int depth = ch->GetDepth();
        if (!ch || (depth >= 0 && depth < 16384 && ch->GetCreateFrame() > targetFrame))
        {
            //if (ch && ch->IsASCharacter())
            //    printf(" dlist.markForRemoval: %s, depth = %d, createFrame = %d\n",
            //        ch->ToASCharacter()->GetDisplayObjectHandle()->GetNamePath().ToCStr(),
            //        ch->GetDepth(), ch->GetCreateFrame());
            //else if (ch)
            //    printf(" dlist.markForRemoval: depth = %d, createFrame = %d\n", 
            //        ch->GetDepth(), ch->GetCreateFrame());
            //else 
            //    printf(" dlist.markForRemoval: <empty>\n");
            de.MarkForRemove(true);
            RemoveFromRenderTree(owner, i);
        }
    }
}

// Removes all objects that are marked for removal.
void    DisplayList::UnloadMarkedObjects(DisplayObjectBase* owner)   
{   
    pCachedChar = NULL;
    // Must be in first-to-last order.
    // don't replace DisplayObjectArray.GetSize() by pre-saved variable here! 
    for (UPInt i = 0; i < DisplayObjectArray.GetSize(); i++)
    {
        if (DisplayObjectArray[i].IsMarkedForRemove())
        {
            // remove "marked-for-removal" flag
            DisplayObjectArray[i].ClearMarkForRemove();
            
            // we need to correct index only if UnloadDisplayObjectAtIndex returns true;
            // this means, that the element at the index 'i' was removed and
            // we don't need to increment the index to get the next element.
            // If it returns false, then the element at index 'i' was moved
            // to the beginning of the array (since depth will be negative) and thus
            // we still need to increment the index.
            if (UnloadDisplayObjectAtIndex(owner, i))
                i--;
        }
    }
    pCachedChar = NULL;
    ++ModId;
}

// Replaces treenode at the specified index. New display entry should be already
// in place in displaylist and DisplayEntry should have indices of the previous treenode
void    DisplayList::ReplaceRenderTreeNode(DisplayObjectBase* owner, UPInt index)
{
    DisplayEntry& dobj = DisplayObjectArray[index];

    if (dobj.GetDisplayObject()->HasIndirectTransform())
    {
        // inserting indirect transform node into new parent;
        // need to update its TransformParent.
        owner->GetMovieImpl()->UpdateTransformParent(dobj.GetDisplayObject(), owner);
        return;
    }

    Render::TreeContainer* pcontainerNode = owner->GetRenderContainer();
    SF_ASSERT(pcontainerNode); // owner MUST HAVE a container!
    SF_ASSERT(dobj.TreeIndex != ~0u);

    SF_ASSERT(owner);
    // need to add a moviedefimpl of old char to kill list to make sure memory heap
    // is not destroyed instantly after obj is died, since RenderContext still may have references to nodes
    // from the heap.
    if (dobj.GetDisplayObject()->GetResourceMovieDef() != owner->GetResourceMovieDef())
        dobj.GetDisplayObject()->GetMovieImpl()->AddMovieDefToKillList(dobj.GetDisplayObject()->GetResourceMovieDef());

    Render::TreeNode* node = pcontainerNode->GetAt(dobj.TreeIndex);
    if (dobj.MaskTreeIndex == ~0u)
    {
        // not masked
        if (node->HasMask())   
        {
            // if replacing mask - just replace a mask 
            node->SetMaskNode(dobj.GetDisplayObject()->GetRenderNode());
        }
        else
        {
            // just remove - insert
            pcontainerNode->Remove(dobj.TreeIndex, 1);
            pcontainerNode->Insert(dobj.TreeIndex, dobj.GetDisplayObject()->GetRenderNode());
        }
    }
    else
    {
        // masked node
        Render::TreeContainer* maskNode = static_cast<Render::TreeContainer*>(node);
        
        maskNode->Remove(dobj.MaskTreeIndex, 1);
        maskNode->Insert(dobj.MaskTreeIndex, dobj.GetDisplayObject()->GetRenderNode());
    }
}

void    DisplayList::RemoveFromRenderTree(DisplayObjectBase* owner, UPInt index)
{
    DisplayEntry& dobj = DisplayObjectArray[index];
    if (dobj.TreeIndex == ~0u)
    {
        if (dobj.GetDisplayObject()->HasIndirectTransform())
        {
            // removing the node that has indirect transform;
            // therefore we need to nullify TransformParent
            owner->GetMovieImpl()->UpdateTransformParent(dobj.GetDisplayObject(), NULL);
        }
        return;
    }
    SF_ASSERT(!dobj.GetDisplayObject()->HasIndirectTransform());

    SF_ASSERT(owner);
    // need to add a moviedefimpl of old char to kill list to make sure memory heap
    // is not destroyed instantly after obj is died, since RenderContext still may have references to nodes
    // from the heap.
    if (dobj.GetDisplayObject()->GetResourceMovieDef() != owner->GetResourceMovieDef())
        dobj.GetDisplayObject()->GetMovieImpl()->AddMovieDefToKillList(dobj.GetDisplayObject()->GetResourceMovieDef());

    Render::TreeContainer* pcontainerNode = owner->GetRenderContainer();
    SF_ASSERT(pcontainerNode); // owner MUST HAVE a container!
    if (dobj.MaskTreeIndex == ~0u)
    {
        // not masked - remove the node itself
        // however, check if the node has mask first...
        SF_ASSERT(dobj.TreeIndex < pcontainerNode->GetSize());
        Render::TreeNode* node = pcontainerNode->GetAt(dobj.TreeIndex);
        SF_ASSERT(node);
        int treeIndexDelta = -1;
        if (node->HasMask())
        {
            // ok, we are removing the mask. Re-insert all children nodes into
            // the regular branch starting at "index"
            Render::TreeContainer* maskNode = static_cast<Render::TreeContainer*>(node);

            for (UPInt i = index + 1; 
                 i < DisplayObjectArray.GetSize() && 
                 ((DisplayObjectArray[i].TreeIndex == dobj.TreeIndex && DisplayObjectArray[i].MaskTreeIndex != ~0u) ||
                 !DisplayObjectArray[i].IsInRenderTree()); ++i)
            {
                DisplayEntry& cdobj = DisplayObjectArray[i];

                ++index;

                if (!cdobj.IsInRenderTree())
                    continue; // skip not inserted nodes
                SF_ASSERT(cdobj.MaskTreeIndex != ~0u);
                Ptr<Render::TreeNode> maskedNode = maskNode->GetAt(0);
                maskNode->Remove(0, 1);
                cdobj.TreeIndex = dobj.TreeIndex + cdobj.MaskTreeIndex;
                cdobj.MaskTreeIndex = ~0u;
                pcontainerNode->Insert(cdobj.TreeIndex, maskedNode);
                ++treeIndexDelta;
            }

            // make sure we will remove the maskNode
            SF_ASSERT(pcontainerNode->GetAt(dobj.TreeIndex + treeIndexDelta + 1) == maskNode);
        }
        // remove the node
        pcontainerNode->Remove(dobj.TreeIndex + treeIndexDelta + 1, 1);

        // need to update tree indices for all entries after the one being inserted...
        for (UPInt i = index + 1; i < DisplayObjectArray.GetSize(); ++i)
        {
            DisplayEntry& cdobj = DisplayObjectArray[i];
            if (cdobj.TreeIndex != ~0u)
            {
                SF_ASSERT(int(cdobj.TreeIndex + treeIndexDelta) >= 0);
                cdobj.TreeIndex += treeIndexDelta;
            }
        }
    }
    else
    {
        // masked - remove the node from mask node
        // get mask node first
        Render::TreeContainer* maskNode = static_cast<Render::TreeContainer*>(pcontainerNode->GetAt(dobj.TreeIndex));
        maskNode->Remove(dobj.MaskTreeIndex, 1);

        // need to update tree indices for all entries after the one being inserted...
        for (UPInt i = index + 1; i < DisplayObjectArray.GetSize(); ++i)
        {
            if (DisplayObjectArray[i].TreeIndex == ~0u)
                continue; // skip alreay detached entries
            if (DisplayObjectArray[i].TreeIndex != dobj.TreeIndex)
                break;
            SF_ASSERT(DisplayObjectArray[i].MaskTreeIndex != ~0u);
            --DisplayObjectArray[i].MaskTreeIndex;
        }
    }
    dobj.TreeIndex = dobj.MaskTreeIndex = ~0u;
}

void    DisplayList::InsertIntoRenderTree(DisplayObjectBase* owner, UPInt index)
{
    DisplayEntry& di = DisplayObjectArray[index];
    if (di.GetDisplayObject()->HasIndirectTransform())
    {
        // inserting indirect transform node into new parent;
        // need to update its TransformParent.
        owner->GetMovieImpl()->UpdateTransformParent(di.GetDisplayObject(), owner);
        return;
    }

    // need to figure out the index in pContainerNode
    Render::TreeContainer* pcontainerNode = owner->GetRenderContainer();
    SF_ASSERT(pcontainerNode); // owner MUST HAVE a container!
    DisplayObjectBase* ch = di.GetDisplayObject();
    Ptr<Render::TreeNode> renNode = ch->GetRenderNode();

    bool masked = false;
    if (index == 0)
        di.TreeIndex = 0; // the first node
    else
    {
        SF_ASSERT(DisplayObjectArray.GetSize() > 0);
        // get the current entry at the index to determine its tree index
        SPInt prevIdx = (SPInt)Alg::PMin(index - 1, DisplayObjectArray.GetSize()-1);
        // find previous entry by skipping all, which are not in the tree...
        while (prevIdx >= 0 && (!DisplayObjectArray[prevIdx].IsInRenderTree()))
            --prevIdx;
        if (prevIdx < 0)
            di.TreeIndex = 0; // the first node
        else
        {
            const DisplayEntry& prevDE = DisplayObjectArray[prevIdx];
            if (prevDE.GetClipDepth() != 0 || prevDE.MaskTreeIndex != ~0u)
            {
                // masked? mask itself cannot be masked.
                // find the entry with clipdepth != 0 first to determine the clip depth
                UPInt maskEntryIdx = prevIdx;
                UPInt alreadyMaskedEntries = 0;
                while (maskEntryIdx > 0 && (DisplayObjectArray[maskEntryIdx].IsMaskedTreeNode() || !DisplayObjectArray[maskEntryIdx].IsInRenderTree()))
                {
                    if (DisplayObjectArray[maskEntryIdx].IsInRenderTree())
                        ++alreadyMaskedEntries;
                    --maskEntryIdx;
                }

                const DisplayEntry& maskEntry = DisplayObjectArray[maskEntryIdx];

                if (ch->GetDepth() <= maskEntry.GetClipDepth() && 
                    (ch->GetDepth() > maskEntry.GetDepth() || ch->GetDepth() == INVALID_DEPTH))
                {
                    // masked!
                    masked = true;

                    // treeIndex is the index of parent mask node in pContainerTree
                    di.TreeIndex     = maskEntry.TreeIndex;
                    // MaskTreeNode is the index inside the mask branch
                    SF_ASSERT((int)(prevIdx - maskEntryIdx) >= 0);
                    di.MaskTreeIndex = (unsigned)alreadyMaskedEntries;

                    // Need to insert into the maskNode
                    Render::TreeContainer* maskNode = static_cast<Render::TreeContainer*>(pcontainerNode->GetAt(maskEntry.TreeIndex));
                    if (ch->GetClipDepth() == 0)
                    {
                        maskNode->Insert(di.MaskTreeIndex, renNode);
                    }
                    else
                    {
                        // a weird special case when mask is inside the mask is possible when swapDepths is used.
                        // Just insert an empty mask node in this case...
                        // Create a tree container
                        Ptr<Render::TreeContainer> node = *owner->GetRenderContext().CreateEntry<Render::TreeContainer>();
                        node->SetMaskNode(renNode);
                        maskNode->Insert(di.MaskTreeIndex, node);
                    }

                    // need to update tree indices for all entries after the one being inserted...
                    for (UPInt i = index + 1; i < DisplayObjectArray.GetSize() && 
                        DisplayObjectArray[i].GetDepth() <= maskEntry.GetClipDepth() && 
                        DisplayObjectArray[i].GetDepth() > maskEntry.GetDepth(); ++i)
                    {
                        if (DisplayObjectArray[i].IsInRenderTree())
                        {
                            SF_ASSERT(DisplayObjectArray[i].MaskTreeIndex != ~0u);
                            ++DisplayObjectArray[i].MaskTreeIndex;
                        }
                    }
                }
            }

            if (!masked)
            {
                // not masked
                di.TreeIndex = prevDE.TreeIndex + 1;
            }
        }
    }
    int treeIndexDelta = 1;
    if (!masked)
    {
        UPInt updateFromIndex = index + 1;
        if (ch->GetClipDepth() != 0)
        {
            // it is a mask!
            // Create a tree container
            Ptr<Render::TreeContainer> maskNode = *owner->GetRenderContext().CreateEntry<Render::TreeContainer>();
            maskNode->SetMaskNode(renNode);

            // need to re-insert masked nodes into the newly created maskNode
            if (index + 1 < DisplayObjectArray.GetSize())
            {
                // skip nodes currently not in render tree
                unsigned tidx = DisplayObjectArray[index + 1].TreeIndex;
                while (!DisplayObjectArray[index + 1].IsInRenderTree())
                {
                    ++updateFromIndex;
                    ++index;
                    if (index + 1 < DisplayObjectArray.GetSize())
                        tidx = DisplayObjectArray[index + 1].TreeIndex;
                    else
                        break;
                }
                if (tidx != ~0u)
                {
                    for (UPInt i = index + 1, j = 0; 
                        i < DisplayObjectArray.GetSize() && 
                        DisplayObjectArray[i].GetDepth() <= ch->GetClipDepth() &&
                        DisplayObjectArray[i].GetDepth() > ch->GetDepth(); ++i)
                    {
                        DisplayEntry& de = DisplayObjectArray[i];
                        ++updateFromIndex;
                        if (!de.IsInRenderTree())
                        {
                            // if the entry is not in render tree - skip it
                            continue;
                        }
                        // another mask inside the maks??? Only possible if crazy swapDepths took place. What to do?
                        if (de.GetClipDepth() != 0)
                            break;
                        Ptr<Render::TreeNode> tnode = pcontainerNode->GetAt(tidx);
                        pcontainerNode->Remove(tidx, 1);
                        de.TreeIndex     = di.TreeIndex;
                        de.MaskTreeIndex = (unsigned)j;
                        maskNode->Insert(de.MaskTreeIndex, tnode);
                        --treeIndexDelta;
                        ++j;
                    }
                }
            }
            pcontainerNode->Insert(di.TreeIndex, maskNode);
        }
        else
        {
            SF_ASSERT (ch->GetClipDepth() == 0);
            pcontainerNode->Insert(di.TreeIndex, renNode);
        }
        // need to update tree indices for all entries after the one being inserted...
        for (UPInt i = updateFromIndex; i < DisplayObjectArray.GetSize(); ++i)
        {
            DisplayEntry& de = DisplayObjectArray[i];
            if (de.TreeIndex != ~0u)
            {
                SF_ASSERT(int(de.TreeIndex + treeIndexDelta) >= 0);
                de.TreeIndex += treeIndexDelta;
            }
        }
    }
}

bool DisplayList::SwapRenderTreeNodes(DisplayObjectBase* owner, UPInt index1, UPInt index2)
{
    DisplayEntry& de1 = DisplayObjectArray[index1];
    DisplayEntry& de2 = DisplayObjectArray[index2];

    //@TODO: do not apply swap if any of the nodes has indirect transform:
    if (de1.GetDisplayObject()->HasIndirectTransform() || de2.GetDisplayObject()->HasIndirectTransform())
        return false;

    Render::TreeNode* n1 = de1.GetDisplayObject()->GetRenderNode();
    UPInt treeIndex1 = (de1.MaskTreeIndex == ~0u) ? de1.TreeIndex : de1.MaskTreeIndex;
    Render::TreeContainer* par1 = static_cast<Render::TreeContainer*>(n1->GetParent());
    SF_ASSERT(par1);

    Render::TreeNode* n2 = de2.GetDisplayObject()->GetRenderNode();
    UPInt treeIndex2 = (de2.MaskTreeIndex == ~0u) ? de2.TreeIndex : de2.MaskTreeIndex;
    Render::TreeContainer* par2 = static_cast<Render::TreeContainer*>(n2->GetParent());
    SF_ASSERT(par2);

    if (de1.GetClipDepth() == 0 && de2.GetClipDepth() == 0)
    {
        // no mask involved
        if (par1 == par2)
        {
            SF_ASSERT(treeIndex1 != treeIndex2);
            // parents are the same, so need to be careful not to screw indices up...
            if (treeIndex1 < treeIndex2)
            {
                par1->Remove(treeIndex1, 1);
                par2->Remove(treeIndex2 - 1, 1);
                par1->Insert(treeIndex1, n2);
                par2->Insert(treeIndex2, n1);
            }
            else
            {
                par2->Remove(treeIndex2, 1);
                par1->Remove(treeIndex1 - 1, 1);
                par2->Insert(treeIndex2, n1);
                par1->Insert(treeIndex1, n2);
            }
        }
        else
        {
            // different parents, easy
            par1->Remove(treeIndex1, 1);
            par2->Remove(treeIndex2, 1);
            par1->Insert(treeIndex1, n2);
            par2->Insert(treeIndex2, n1);
        }
        Alg::Swap(de1.TreeIndex, de2.TreeIndex);
        Alg::Swap(de1.MaskTreeIndex, de2.MaskTreeIndex);
    }
    else
    {
        // one or both are masks
        Alg::Swap(de1.TreeIndex, de2.TreeIndex);
        Alg::Swap(de1.MaskTreeIndex, de2.MaskTreeIndex);
        RemoveFromRenderTree(owner, index1);
        RemoveFromRenderTree(owner, index2);
        InsertIntoRenderTree(owner, index1);
        InsertIntoRenderTree(owner, index2);
    }
    return true;
}

bool    DisplayList::UnloadDisplayObjectAtIndex(DisplayObjectBase* owner, UPInt index)
{
    const DisplayEntry & dobj = DisplayObjectArray[index];
    RemoveFromRenderTree(owner, index);
    
    // If entry is already marked for remove, do nothing at the point,
    // let the UnloadMarkedObjects do its job later.
    if (dobj.IsMarkedForRemove())
        return false;

    DisplayObjectBase* pch = dobj.GetDisplayObject();
    SF_ASSERT(pch);

    if (pch)    
    {
        // It is possible that UnloadEntryAtIndex is called multiple times for the 
        // same character. Do nothing if the pch is already being unloaded, return
        // 'false' since it can't be removed immediately.
        if (pch->IsUnloading())
            return false;

        // OnUnloading handler determines the possibility of instant unloading.
        // It also queues up the onUnload AS event, if it is defined.
        bool mayRemove = pch->OnUnloading();
        pch->SetUnloading(true);
        if (mayRemove)
        {
            // We can remove the object instantly, so, do it now.
            pch->OnUnload();
            if (DisplayObjectArray[index].GetDisplayObject())
                DisplayObjectArray[index].GetDisplayObject()->SetParent(NULL);
            DisplayObjectArray.RemoveAt(index);
        }
        else
        {
            //SF_ASSERT(dobj.GetDepth() >= 0);
            // if the depth is already negative this means the clip is already being removed.
            // Such situation is possible when a child clip has onUnload handler and when
            // removeMovieClip is called for the child clip first, and then removeMovieClip for
            // the parent.
            if (dobj.GetDepth() >= 0)
            {
                // ok, we can't remove the object right now, because of unload handler.
                // In this case we need to move the object to the depth = -1 - curDepth.
                // Note, GFx shifts all depths by +16384, so the real new depth of the
                // character will be -32769 - curDepth.
                int newDepth = -1 - dobj.GetDepth();

                // Index 2 must be used for insertion.
                // Move the object and set depth.
                DisplayEntry de(dobj);
                DisplayObjectArray.RemoveAt(index);
                de.GetDisplayObject()->SetDepth(newDepth);
                UPInt index2 = FindDisplayIndex(newDepth);
                DisplayObjectArray.InsertAt(index2, de);
            }
        }
        pCachedChar = NULL; 
        CheckAndInvalidateDepthMappings();
        return mayRemove;
    }
    else
    {
        if (DisplayObjectArray[index].GetDisplayObject())
            DisplayObjectArray[index].GetDisplayObject()->SetParent(NULL);
        DisplayObjectArray.RemoveAt(index);
        pCachedChar = NULL; 
        CheckAndInvalidateDepthMappings();
    }
    SF_ASSERT(!pCachedChar);
    return true;
}

// Calls all onClipEvent handlers due to a mouse event. 
void    DisplayList::PropagateMouseEvent(const EventId& id)
{       
    // Objects further on display list get mouse messages first.
    
    for (SPInt i = (SPInt)(DisplayObjectArray.GetSize() - 1); i >= 0; i--)
    {
        DisplayEntry&   dobj = DisplayObjectArray[i];
        DisplayObjectBase*  ch   = dobj.GetDisplayObject();
        SF_ASSERT(ch);

        if (!ch->GetVisible())
            continue;

        if (ch->IsInteractiveObject())
            ch->CharToInteractiveObject()->PropagateMouseEvent(id);

        // Can display list change during an event ? check for bounds.
        // This probably isn't the right behavior, since some of characters might miss
        // the mouse event.
        // Copy pointers first, and process them later ? @TODO
        if (i >= (SPInt)DisplayObjectArray.GetSize())
            i = (SPInt)DisplayObjectArray.GetSize(); //? reset to the end?
    }
}

// Calls all onClipEvent handlers due to a key event.   
void    DisplayList::PropagateKeyEvent(const EventId& id, int* pkeyMask)
{       
    // Objects at the beginning of display list get key messages first.
    
    for (UPInt i = 0; i < DisplayObjectArray.GetSize(); ++i)
    {
        DisplayEntry&   dobj = DisplayObjectArray[i];
        // need to hold a strong ptr since obj might be released!
        Ptr<DisplayObjectBase>   ch   = dobj.GetDisplayObject();
        SF_ASSERT(ch);

        if (!ch->GetVisible())  
            continue;

        if (ch->IsInteractiveObject())
            ch->CharToInteractiveObject()->PropagateKeyEvent(id, pkeyMask);

        // Can display list change during an event ? (answer: yes!) check for recent bounds.
        // This probably isn't the right behavior, since some of characters might miss
        // the key event.
        // Copy pointers first, and process them later ? @TODO.
        //if (i + 1 >= DisplayObjectArray.GetSize())
        //    break;
    }
}

// Visit all display list members that have a name.
void    DisplayList::VisitMembers(MemberVisitor *pvisitor) const
{
    // Enumeration Op lists children in-order.
    for (UPInt i = 0, n = DisplayObjectArray.GetSize(); i < n; ++i)
    {
        const DisplayEntry& dobj = DisplayObjectArray[i];
        DisplayObjectBase*       ch   = dobj.GetDisplayObject();
        SF_ASSERT(ch);

        // This is used in enumeration op, which returns values by name only.
        if (ch->IsInteractiveObject())
        {
            const ASString& name = ch->CharToInteractiveObject_Unsafe()->GetName();
            if (!name.IsEmpty())
                pvisitor->Visit(name, ch->CharToInteractiveObject_Unsafe());
        }
    }
}

// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  DisplayList::GetBounds(const Matrix2F &transform) const
{
    DisplayObjectBase*   ch;
    UPInt           i, n = GetCount();
    RectF          r(0);   
    RectF          tempRect;   
    Matrix2F m;

    for (i = 0; i < n; i++)
    {
        ch = GetDisplayObject(i);
        if (ch != NULL)
        {
            // Build transform to target.       
            m = transform;
            m *= ch->GetMatrix();
            // get bounds transformed by matrix 
            tempRect = ch->GetBounds(m);                        

            if (!tempRect.IsEmpty ())
            {
                if (!r.IsEmpty ())
                    r.Union(tempRect);
                else
                    r = tempRect;
            }
        }
    }
    return r;
}


// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
// Return a single character "pure rectangle" bounds (not considering the stroke)
RectF  DisplayList::GetRectBounds(const Matrix2F &transform) const
{
    DisplayObjectBase*   ch;
    UPInt           i, n = GetCount();
    RectF          r(0);   
    RectF          tempRect;   
    Matrix2F m;

    for (i = 0; i < n; i++)
    {
        ch = GetDisplayObject(i);
        if (ch != NULL)
        {
            // Build transform to target.       
            m = transform;
            m *= ch->GetMatrix();
            // get bounds transformed by matrix 
            tempRect = ch->GetRectBounds(m);                        

            if (!tempRect.IsEmpty ())
            {
                if (!r.IsEmpty ())
                    r.Union(tempRect);
                else
                    r = tempRect;
            }
        }
    }
    return r;
}

UPInt DisplayList::FindDisplayIndex(int depth) const
{
    if ((Flags & Flags_DepthToIndexInvalid) != 0)
    {
        SPInt rv = -1;
        if (DisplayObjectArray.GetSize() > THRESHOLD_CNT)
        {
            UPInt reqdCapacity = 0;
            if (!DepthToIndexMap)
                DepthToIndexMap = SF_HEAP_AUTO_NEW(this) DepthToIndexContainer();
            // need to rebuild DepthToIndexMap
            for (UPInt i = 0, n = DisplayObjectArray.GetSize(); i < n; ++i)
            {
                const DisplayEntry& de = DisplayObjectArray[i];
                if (rv == -1 && de.GetDepth() >= depth)
                {
                    // do search, since anyway we are going through the whole array.
                    rv = i;
                }
                if (de.GetDepth() != INVALID_DEPTH)
                {
                    DepthToIndexMap->SetElement(reqdCapacity, de.GetDepth(), unsigned(i));
                    ++reqdCapacity;
                }
            }
            DepthToIndexMap->SetSize(reqdCapacity);
        }
        else
        {
            if (DepthToIndexMap)
                DepthToIndexMap->SetSize(0);
        }
        Flags &= ~Flags_DepthToIndexInvalid;
        if (rv != -1)
            return (UPInt)rv;
    }

    UPInt rv;
    if (!(Flags & Flags_MayContainForeignElems))
    {
        rv = Alg::LowerBound(DisplayObjectArray, depth, DepthLess);
    }
    else
    {
        if (DepthToIndexMap && DisplayObjectArray.GetSize() > THRESHOLD_CNT)
        {
            // search using DepthToIndexMap
            UPInt i = Alg::LowerBound(*DepthToIndexMap, depth, DepthLess1);
            if (i == DepthToIndexMap->GetSize())
                return DisplayObjectArray.GetSize();
            SF_ASSERT((*DepthToIndexMap)[i].Depth == DisplayObjectArray[(*DepthToIndexMap)[i].Index].GetDepth());
            rv = (*DepthToIndexMap)[i].Index;
        }
        else
        {
            UPInt n;
            n = rv = DisplayObjectArray.GetSize();
            // do just exhaustive search.
            SF_ASSERT(GetCount() <= THRESHOLD_CNT);
            for (UPInt i = 0; i < n; ++i)
            {
                const DisplayEntry& de = DisplayObjectArray[i];
                if (de.GetDepth() >= depth)
                {
                    rv = i;
                    break;
                }
            }
        }
    }
    return rv;
}


#ifdef SF_BUILD_DEBUG
// checks for consistency of the display list
void DisplayList::CheckConsistency(DisplayObjectBase* owner) const
{
    if (DisplayObjectArray.GetSize() > 0)
    {
        // check for multiple characters at the same depth. Note, sometimes
        // multiple characters at the same depth are allowed (see AddDisplayObject).

        DisplayObjectBase* curch = DisplayObjectArray[0].GetDisplayObject();
        SF_ASSERT(curch);
        int curdepth = DisplayObjectArray[0].GetDepth();
        for (unsigned i = 1; i < DisplayObjectArray.GetSize(); i++)
        {
            const DisplayEntry& de = DisplayObjectArray[i];
            DisplayObjectBase* ich = de.GetDisplayObject();
            SF_ASSERT(ich);
            if (de.GetDepth() >= 0)
                SF_ASSERT(de.GetDepth() != curdepth);
            curdepth = de.GetDepth();
        }

        // check the consistency of render tree
        Render::TreeContainer* pcontainerNode = owner->GetRenderContainer();
        for (unsigned i = 0, trIdx = 0, maskIdx = ~0u; i < DisplayObjectArray.GetSize(); ++i)
        {
            const DisplayEntry& de = DisplayObjectArray[i];
            
            // INVALID_DEPTH is used by AS3 for attached clips.
            if ((de.GetDepth() >= 0 || de.GetDepth() == INVALID_DEPTH) && de.TreeIndex != ~0u)
            {
                if (maskIdx != ~0u && (de.MaskTreeIndex == ~0u || de.TreeIndex != trIdx))
                    ++trIdx;

                // check the tree index
                SF_ASSERT(de.TreeIndex == trIdx);
                if (de.MaskTreeIndex != ~0u)
                {
                    // check masked nodes

                    // check the mask tree index
                    SF_ASSERT(de.MaskTreeIndex == maskIdx);
                    ++maskIdx;

                    SF_ASSERT(pcontainerNode->GetAt(de.TreeIndex)->HasMask());

                    Render::TreeContainer* maskNode = static_cast<Render::TreeContainer*>(pcontainerNode->GetAt(de.TreeIndex));

                    if (de.GetClipDepth() == 0)
                        SF_ASSERT(maskNode->GetAt(de.MaskTreeIndex) == de.GetDisplayObject()->GetRenderNode());
                    else
                    {
                        Render::TreeContainer* cont = static_cast<Render::TreeContainer*>(maskNode->GetAt(de.MaskTreeIndex));
                        SF_ASSERT(cont->GetSize() == 0);
                        SF_ASSERT(cont->GetMaskNode() == de.GetDisplayObject()->GetRenderNode());
                    }
                }
                else
                {
                    // make sure the tree node is right
                    DisplayObject* sobj = de.GetDisplayObject()->CharToScriptableObject();
                    if (pcontainerNode->GetAt(de.TreeIndex)->HasMask() && (!sobj || !sobj->GetMask()))
                    {
                        if (sobj && sobj->GetScrollRectInfo())
                            SF_ASSERT(pcontainerNode->GetAt(de.TreeIndex)->GetMaskNode() == sobj->GetScrollRectInfo()->Mask->GetTreeNode());
                        else
                            SF_ASSERT(pcontainerNode->GetAt(de.TreeIndex)->GetMaskNode() == de.GetDisplayObject()->GetRenderNode());
                        maskIdx = 0;
                    }
                    else 
                    {
                        if (sobj && sobj->GetMask())
                        {
                            SF_ASSERT(sobj->GetMask()->GetMaskOwner() == sobj);
                            SF_ASSERT(sobj->GetMask()->GetRenderNode()->GetParent() == pcontainerNode->GetAt(de.TreeIndex));
                        }

                        SF_ASSERT(pcontainerNode->GetAt(de.TreeIndex) == de.GetDisplayObject()->GetRenderNode());
                        maskIdx = ~0u;
                        ++trIdx;
                    }
                }
            }
            else
            {
                // char in limbo space - no tree node
                SF_ASSERT(de.TreeIndex == ~0u && de.MaskTreeIndex == ~0u);
            }
        }
    }
}
#endif // SF_BUILD_DEBUG

}} // namespace Scaleform::GFx
