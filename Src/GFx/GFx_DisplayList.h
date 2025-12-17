/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_DisplayList.h
Content     :   Character display list for root and nested clips
Created     :   
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_DISPLAYLIST_H
#define INC_SF_GFX_DISPLAYLIST_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Alg.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_PlayerStats.h"
#include "Render/Render_TreeNode.h"

#include "GFx/GFx_CharPosInfo.h"

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class CharPosInfo;

// A list of active characters.
class DisplayList : public NewOverrideBase<StatMV_MovieClip_Mem>
{
    friend class DisplayObjectBase;
    friend class DisplayObject;
public:
    enum AddFlags
    {
        Flags_ReplaceIfDepthIsOccupied  = 0x1,
        Flags_DeadOnArrival             = 0x2,
        Flags_PlaceObject               = 0x4
    };
    enum
    {
        INVALID_DEPTH                   = DisplayObjectBase::INVALID_DEPTH,

        // number of elements until we can use exhaustive search rather
        // than build DepthToIndexMap
        THRESHOLD_CNT                   = 10
    };

    // A describing an entry in the display list. A depth-sorted array of these
    // structures is maintained.
    class DisplayEntry
    {
    private:
        enum  FrameFlags
        {           
            // An object is marked for removal. Objects that are marked for remove in
            // the last frame of the movie before wrap-around to first frame.
            // are not considered a part of display list, unless 
            Flag_MarkedForRemove    = 0x00000001
        };

        DisplayObjectBase*          pCharacter;
    public:
        // An index in the TreeContainer; if character is masked then this is the index
        // of parent node with the mask.
        unsigned                    TreeIndex;  

        // If character is masked, this is the index in the mask branch.
        unsigned                    MaskTreeIndex;
       
    public:

        DisplayEntry()          
        {
            pCharacter = NULL;
            TreeIndex = MaskTreeIndex = ~0u;
        }

        DisplayEntry(const DisplayEntry& di)            
        {
            pCharacter = NULL;
            *this= di;
        }

        ~DisplayEntry()
        {
            DisplayObjectBase* prevch = GetDisplayObject();
            if (prevch)
                prevch->Release();
        }

        void    operator=(const DisplayEntry& di)
        {
            SetDisplayObject(di.GetDisplayObject());
            TreeIndex       = di.TreeIndex;
            MaskTreeIndex   = di.MaskTreeIndex;
        }

        void    SetDisplayObject(DisplayObjectBase* pch)
        {
            DisplayObjectBase* prevch = GetDisplayObject();
            if (prevch)
                prevch->Release();
            pCharacter = pch;
            if (pch)
                pch->AddRef();
        }
        DisplayObjectBase* GetDisplayObject() const
        {
            return pCharacter;
        }

        // Returns depth of a character, no null checking for speed.
        int     GetDepth() const            { return GetDisplayObject()->GetDepth(); }

        int     GetClipDepth() const        { return GetDisplayObject()->GetClipDepth(); }

        bool    IsMarkedForRemove() const   
        { 
            SF_ASSERT(GetDisplayObject());
            return GetDisplayObject()->IsMarkedForRemove(); 
        }
        // Only advance characters if they are not just loaded and not marked for removal.
        bool    CanAdvanceChar() const      
        { 
            return !IsMarkedForRemove(); 
        }
        void    MarkForRemove(bool remove)
        {
            SF_ASSERT(GetDisplayObject());
            GetDisplayObject()->MarkForRemove(remove); 
        }
        void    ClearMarkForRemove() { MarkForRemove(false); }
    
        bool    IsInRenderTree() const { return TreeIndex != ~0u; }
        bool    IsMaskedTreeNode() const { return MaskTreeIndex != ~0u; }
    };

    struct MemberVisitor 
    {
        virtual ~MemberVisitor () { }
        virtual void    Visit(const ASString& name, InteractiveObject* pch) = 0;
    };

private:
    // Display array, sorted by depth. In AS3 this array may have "non-depth"
	// elements, inserted by ActionScript (addChildAt, swapChildren). In this
	// case, DepthToIndexMap should be used.
    ArrayLH<DisplayEntry>  DisplayObjectArray;
	
	struct DepthToIndexMapElem
	{
		int			Depth;
		unsigned	Index;

        DepthToIndexMapElem(int d = INVALID_DEPTH, unsigned i = ~0u):Depth(d), Index(i) {}
	};
    struct  DepthToIndexContainer : public NewOverrideBase<StatMV_MovieClip_Mem>
    {
        ArrayLH<DepthToIndexMapElem> Array;

        void SetElement(UPInt index, int depth, unsigned mappedIndex)
        {
            if (index < Array.GetSize())
                Array[index] = DepthToIndexMapElem(depth, mappedIndex);
            else
                Array.PushBack(DepthToIndexMapElem(depth,mappedIndex));
        }
        void SetSize(UPInt sz) { Array.Resize(sz); }

        DepthToIndexMapElem& operator[](UPInt i) { return Array[i]; }
        const DepthToIndexMapElem& operator[](UPInt i) const { return Array[i]; }
        UPInt GetSize() const { return Array.GetSize(); }
    };
	// This optional array contains mapping between Depth and index in 
	// DisplayObjextArray. It is sorted by depth and allocated only
	// if InvalidateDepthMappings() method is called. Note, this map
    // will be created only if number of elements in the display list
    // is greater than THRESHOLD_CNT (see above).
	mutable DepthToIndexContainer *DepthToIndexMap;

    // An instance of last character accessed by name (using GetDisplayObjectByName). Needs to be
    // reset every time DisplayList is changed or name of any character
    // in display list is changed.
    mutable DisplayObject*   pCachedChar;

    // modification id to track concurrent displaylist modifications.
    UInt16                   ModId;

    enum 
    {
        // indicates that DepthToIndexMap is invalid and must
        // be rebuilt.
        Flags_DepthToIndexInvalid       = 0x1,

        // indicates that displaylist may contain objects
        // without depth.
        Flags_MayContainForeignElems    = 0x2
    };
    // indicates that DepthToIndexMap is invalid and must
    // be rebuilt.
	mutable UInt8                 Flags;

private:
    static int DepthLess(const DisplayEntry& de, int depth)
    {
        return (de.GetDepth() < depth);
    }
    static int DepthLess1(const DepthToIndexMapElem& de, int depth)
    {
        return (de.Depth < depth);
    }
    // checks if depthToIndex map should be invalidated
    // and invalidate if it should.
    void            CheckAndInvalidateDepthMappings() 
    { 
        if ((Flags & Flags_MayContainForeignElems) != 0)
            Flags |= Flags_DepthToIndexInvalid; 
    }

    void InsertIntoRenderTree(DisplayObjectBase* owner, UPInt index);
    void RemoveFromRenderTree(DisplayObjectBase* owner, UPInt index);
    void ReplaceRenderTreeNode(DisplayObjectBase* owner, UPInt index);
    bool SwapRenderTreeNodes(DisplayObjectBase* owner, UPInt index1, UPInt index2);
public:
    DisplayList();
    ~DisplayList()
    {
        delete DepthToIndexMap;
    }

    // *** Display list management.

    
    // Adds a new character to the display list.
    // addFlags - is a bit mask of DisplayList::AddFlags bits.
    void            AddDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos, DisplayObjectBase* ch, UInt32 addFlags);
    // Adds a new character at the appropriate index
    void            AddDisplayObject(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch);
    // Adds a display list entry at the index. This method doesn't cause
    // execution of onEventLoad.
    void            AddEntryAtIndex(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch);
    // Moves an existing character in display list at given depth.
    void            MoveDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos);
    // Replaces a character at depth with a new one.
    void            ReplaceDisplayObject(DisplayObjectBase* owner, const CharPosInfo &pos, DisplayObjectBase* ch);
    // Replaces a character at index with a new one.
    void            ReplaceDisplayObjectAtIndex(DisplayObjectBase* owner, UPInt index, DisplayObjectBase* ch);
    // Remove a character at depth.
    void            RemoveDisplayObject(DisplayObjectBase* owner, int depth, ResourceId id);
    // Remove a character. Returns false if not found.
    bool            RemoveDisplayObject(DisplayObjectBase* owner, const DisplayObjectBase* ch);
    // Removes a display list entry at the index. Does not cause execution of onEventUnload.
    void            RemoveEntryAtIndex(DisplayObjectBase* owner, UPInt index);

    //void            MarkForRemoval(int depth, ResourceId id);

    // Clear the display list, unloading all objects.
    void            Clear(DisplayObjectBase* owner);
    // Unload is similar to Clear but it executes onUnload for all unloading chars
    bool            UnloadAll(DisplayObjectBase* owner);

    // Mark all entries for removal, usually done when wrapping from
    // last frame to the next one. Objects that get re-created will
    // survive, other ones will die.
    void            MarkAllEntriesForRemoval(DisplayObjectBase* owner, unsigned targetFrame = 0);
    // Removes all objects that are marked for removal. Removed objects
    // receiveUnload event.
    void            UnloadMarkedObjects(DisplayObjectBase* owner);

    bool            UnloadDisplayObjectAtIndex(DisplayObjectBase* owner, UPInt index);

    // *** Frame processing.

    // Calls all onClipEvent handlers due to a mouse event. 
    void            PropagateMouseEvent(const EventId& id);

    // Calls all onClipEvent handlers due to a key event.   
    void            PropagateKeyEvent(const EventId& id, int* pkeyMask);

    
    // *** Character entry access.

    // Finds display array index at depth, or next depth.
    UPInt           FindDisplayIndex(int depth) const;

    // Finds display array index of specified display object.
    // Returns -1 if not found.
    SPInt           FindDisplayIndex(const DisplayObjectBase* ch) const;
    // Like the above, but looks for an exact match, and returns SF_MAX_UPINT if failed.
    UPInt           GetDisplayIndex(int depth) const;

    // Swaps two objects at depths.
    // Depth1 must always have a valid object. Depth2 will be inserted, if not existent.
    bool            SwapDepths(DisplayObjectBase* owner, int depth1, int depth2, unsigned frame);
    // Swaps entries with specified indexes.
    bool            SwapEntriesAtIndexes(DisplayObjectBase* owner, UPInt origIndex1, UPInt origIndex2);
    // Returns maximum used depth, -1 if display list is empty.
    int             GetLargestDepthInUse() const;

    UPInt           GetCount() const        { return DisplayObjectArray.GetSize(); }
    DisplayObjectBase*  GetDisplayObject(UPInt index) const  { return DisplayObjectArray[index].GetDisplayObject(); }
    
    DisplayObjectBase* GetCharacterAtDepth(int depth, ResourceId id, UPInt* pindex = NULL) const;

    // Get the display object at the given position.
    const DisplayEntry&   GetDisplayEntry(UPInt idx) const { return DisplayObjectArray[idx]; }
    DisplayEntry&   GetDisplayEntry(UPInt idx) { return DisplayObjectArray[idx]; }

    // May return NULL.
    DisplayObjectBase*  GetDisplayObjectAtDepth(int depth, bool* pisMarkedForRemove = NULL) const;
    DisplayObjectBase*  GetDisplayObjectAtDepthAndUnmark(DisplayObjectBase* owner, int depth);

    // Note that only InteractiveObjects have names, so it's ok to return InteractiveObject.
    // Case sensitivity depends on string context SWF version. May return NULL.
    // If there are multiples, returns the *first* match only!
    DisplayObject*  GetDisplayObjectByName(const ASString& name, bool caseSensitive = true) const;    

    // Visit all display list members that have a name
    void            VisitMembers(MemberVisitor *pvisitor) const;

    // Removes character from display list. Note, this function just find and remove the object;
    // no events (unload) will be executed.
    // Returns true, if object is found and removed.
    //bool          RemoveCharacter(DisplayObjectBase*);

    // Resets cached last character accessed by name.
    void            ResetCachedCharacter() { pCachedChar = NULL; }

    // Bounds computation.
    // "transform" matrix describes the transform applied to parent and us,
    // including the object's matrix itself. This means that if transform is
    // identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
    RectF           GetBounds(const Matrix2F &transform) const;       

    // Return a single character "pure rectangle" bounds (not considering the stroke)
    RectF           GetRectBounds(const Matrix2F &transform) const;       

    // Invalidate depth to index mapping
    void            InvalidateDepthMappings() 
    { 
        Flags |= (Flags_DepthToIndexInvalid | Flags_MayContainForeignElems); 
    }

    unsigned GetCurModId() const { return ModId; }
#ifdef SF_BUILD_DEBUG
    void            CheckConsistency(DisplayObjectBase* owner) const;
#else
    inline void     CheckConsistency(DisplayObjectBase*) const {}
#endif // SF_BUILD_DEBUG
};

}} // namespace Scaleform::GFx
#endif // INC_SF_GFX_DISPLAYLIST_H
