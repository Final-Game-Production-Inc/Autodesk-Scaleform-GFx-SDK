/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_SpriteDef.h
Content     :   Sprite Definition header file.
Created     :   
Authors     :   Artem Bolgar

Notes       :   This file contains class declarations used in
GFxSprite.cpp only. Declarations that need to be
visible by other player files should be placed
in DisplayObjectBase.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_SPRITEDEF_H
#define INC_SF_GFX_SPRITEDEF_H

#include "GFx/GFx_MovieDef.h"
//#include "GFx/GFx_Scale9Grid.h"
#include "Kernel/SF_ListAlloc.h"
#include "Kernel/SF_Alg.h"

#ifdef GFX_ENABLE_SOUND
#include "GFx/Audio/GFx_Sound.h"
#endif

namespace Scaleform { namespace GFx {

class TimelineDef;
class TimelineIODef;
class InteractiveObject;
class DisplayObjectBase;

struct Scale9Grid;

//
// ***** SpriteDef
//

// A sprite is a mini movie-within-a-movie.  It doesn't define
// its own characters; it uses the characters from the parent
// MovieDefImpl, but it has its own frame counter, display list, etc.
//
// The sprite implementation is divided into a
// SpriteDef and a GFxSprite.  The Definition
// holds the immutable data for a sprite, while the Instance
// contains the state for a specific instance being updated
// and displayed in the parent InteractiveObject's display list.

class SpriteDef : public TimelineIODef
{
    // MovieDef to which this sprite belongs. Note that this pointer can be 
    // invalid temporarily during loading if all references to MovieDataDef
    // have been released but load thread hasn't yet died. This shouldn't
    // be a problem since constructor / Read methods don't use this pointer.
    MovieDataDef*       pMovieDef;

    StringHashLH<unsigned> NamedFrames;    // stores 0-based frame #'s
    int                 FrameCount;
    int                 LoadingFrame;

    // GFxSprite control events for each frame.
    ArrayLH<Frame, StatMD_Other_Mem> Playlist;

    Scale9Grid*         pScale9Grid;
#ifdef GFX_ENABLE_SOUND
    Ptr<SoundStreamDef> pSoundStream;
#endif

    enum
    {
        Flags_Has_Frame_up      = 0x1,
        Flags_Has_Frame_down    = 0x2,
        Flags_Has_Frame_over    = 0x4,
        Flags_HasSpecialFrames = (Flags_Has_Frame_up|Flags_Has_Frame_down|Flags_Has_Frame_over)
    };
    UByte               Flags;

public:

    SpriteDef(MovieDataDef* pmd);
    ~SpriteDef();

    virtual CharacterDefType GetType() const { return Sprite; }

    // Initialize an empty clip.
    void                    InitEmptyClipDef();
    // Load the sprite from disk (usually called from background thread).
    void                    Read(LoadProcess* p, ResourceId charId);

    // overloads from GFxMovieDef
    virtual float           GetWidth() const            { return 1; }
    virtual float           GetHeight() const           { return 1; }
    virtual bool            DefPointTestLocal
        (const Render::PointF &pt, bool testShape = 0, const DisplayObjectBase *pinst = 0) const;
    virtual unsigned        GetFrameCount() const       { return FrameCount; }
    virtual float           GetFrameRate() const        { return pMovieDef->GetFrameRate(); }
    virtual RectF           GetFrameRect() const        { return RectF(0,0,1,1); }
    virtual unsigned        GetLoadingFrame() const     { return LoadingFrame; }
    virtual unsigned        GetVersion() const          { return pMovieDef->GetVersion(); }
    virtual unsigned        GetSWFFlags() const         { return pMovieDef->GetSWFFlags(); }
    virtual unsigned        GetTagCount() const         { return 0; }
    virtual UInt32          GetFileBytes() const        { return pMovieDef->GetFileBytes(); }
    virtual const char*     GetFileURL() const          { return pMovieDef->GetFileURL(); }


    // Returns 0-based frame #  
    virtual bool            GetLabeledFrame(const char* label, unsigned* frameNumber, bool translateNumbers = 1) const
    {
        return MovieDataDef::TranslateFrameString(NamedFrames, label, frameNumber, translateNumbers);        
    }
    virtual const String*   GetFrameLabel(unsigned frameNumber, unsigned* exactFrameNumberForLabel = NULL) const
    {
        return MovieDataDef::TranslateNumberToFrameString(NamedFrames, frameNumber, exactFrameNumberForLabel);
    }
    // fills array of labels for the passed frame. One frame may have multiple labels.
    virtual Array<String>*  GetFrameLabels(unsigned frameNumber, Array<String>* destArr) const;

    // Labels the frame currently being loaded with the
    // given name.  A copy of the name string is made and
    // kept in this object.
    virtual void            AddFrameName(const String& name, LogState *plog);  
    virtual void            SetLoadingPlaylistFrame(const Frame& frame, LogState *plog)
    {
        if (int(Playlist.GetSize()) <= LoadingFrame)
        {
            if (plog && plog->GetLog())
                plog->GetLog()->LogError("Invalid SWF file: failed to load sprite's frame #%d since total frames counter is %d",
                int(LoadingFrame + 1), int(Playlist.GetSize()));
            return;
        }
        SF_ASSERT(Playlist[LoadingFrame].TagCount == 0);
        Playlist[LoadingFrame] = frame;
    }

    // *** TimelineDef implementation

    // FrameNumber is 0-based
    virtual const Frame     GetPlaylist(int frameNumber) const    
    {
        return Playlist[frameNumber];
    }    
    virtual bool            GetInitActions(Frame* pframe, int frameNumber) const
    {
        SF_UNUSED2(frameNumber, pframe);
        // Sprites do not have init actions in their playlists!
        // Only the root sprite (MovieDataDef) does.
        return false;
    }    

#ifdef GFX_ENABLE_SOUND
    virtual SoundStreamDef* GetSoundStream() const;
    virtual void            SetSoundStream(SoundStreamDef* psoundStream);
#endif    

    // *** Resource implementation

    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_SpriteDef); }

    void                    SetScale9Grid(const RectF& r) 
    {
        if (pScale9Grid == 0) 
            pScale9Grid = SF_HEAP_AUTO_NEW(this) Scale9Grid;
        *pScale9Grid = r;
    }

    RectF                   GetScale9Grid() const { return (pScale9Grid) ? pScale9Grid->Rect : RectF(0); }
    bool                    HasScale9Grid() const { return pScale9Grid != NULL; }

    bool                    HasSpecialFrames() const { return (Flags & Flags_HasSpecialFrames) != 0; }
    bool                    HasFrame_up() const   { return (Flags & Flags_Has_Frame_up) != 0; }
    bool                    HasFrame_down() const { return (Flags & Flags_Has_Frame_down) != 0; }
    bool                    HasFrame_over() const { return (Flags & Flags_Has_Frame_over) != 0; }
};


// This class is designed to assemble and execute so called "timeline snapshot". 
// It is used for gotoFrame backward and forward. It actually is being assembled
// from ExecuteTags - PlaceObject and RemoveObject.
class TimelineSnapshot
{
public:
    enum PlaceType
    {
        Place_Add       = PlaceObjectTag::Place_Add,
        Place_Move      = PlaceObjectTag::Place_Move,
        Place_Replace   = PlaceObjectTag::Place_Replace,
        Place_Remove,
        Place_RemoveAndAdd,

        Place_Unknown = 255
    };
    enum FlagsType
    {
        Flags_NoReplaceAllowed = 0x1, // TagType=4, no replaces allowed
        Flags_DeadOnArrival    = 0x2  // Character is released, but has unload handler
    };
    struct SourceTags
    {
        GFxPlaceObjectBase*         pMainTag;
        GFxPlaceObjectBase*         pMatrixTag;
        GFxPlaceObjectBase*         pCxFormTag;
        GFxPlaceObjectBase*         pFiltersTag;
        GFxPlaceObjectBase*         pBlendModeTag;
        GFxPlaceObjectBase*         pDepthTag;
        GFxPlaceObjectBase*         pClipDepthTag;
        GFxPlaceObjectBase*         pRatioTag;
        GFxPlaceObjectBase*         pCharIdTag;
        GFxPlaceObjectBase*         pClassNameTag;
        SourceTags() 
            : pMainTag(0), pMatrixTag(0), pCxFormTag(0), pFiltersTag(0), 
            pBlendModeTag(0), pDepthTag(0), pClipDepthTag(0), 
            pRatioTag(0), pCharIdTag(0), pClassNameTag(0) {}
        SF_INLINE bool HasMainTag() { return (pMainTag != 0); }
        SF_INLINE void Assign(GFxPlaceObjectBase* ptag)
        {
            pMainTag = pMatrixTag = pCxFormTag = pFiltersTag = 
                pBlendModeTag = pDepthTag = pClipDepthTag = pRatioTag = 
                pCharIdTag = pClassNameTag = ptag;
        }
        SF_INLINE void Union(GFxPlaceObjectBase* ptag)
        {
            CharPosInfoFlags flags = ptag->GetFlags();
            if (flags.HasMatrix())      pMatrixTag = ptag;
            if (flags.HasCxform())      pCxFormTag = ptag;
            if (flags.HasFilters())     pFiltersTag = ptag;
            if (flags.HasBlendMode())   pBlendModeTag = ptag;
            if (flags.HasDepth())       pDepthTag = ptag;
            if (flags.HasClipDepth())   pClipDepthTag = ptag;
            if (flags.HasRatio())       pRatioTag = ptag;
            if (flags.HasCharacterId()) pCharIdTag = ptag;
            if (flags.HasClassName())   pClassNameTag = ptag;
        }
        void Unpack(GFxPlaceObjectBase::UnpackedData& data) const;
    };
    struct SnapshotElement : public ListNode<SnapshotElement>
    {
        unsigned                    CreateFrame;
        int                         Depth;

        // Matrices, depth and character index.
        SourceTags                  Tags;

        UInt8                       PlaceType;
        UInt8                       Flags;

        SnapshotElement()
            :CreateFrame(~0u), PlaceType(Place_Unknown), Flags(0) {}
        SnapshotElement(int depth)
            :CreateFrame(~0u), PlaceType(Place_Unknown), Flags(0) { Depth = depth; }
        ~SnapshotElement() {}
    };
    struct SnapshotElementComparator
    {
        int Compare(const SnapshotElement& p1, int depth) const
        {
            return (int)p1.Depth - (int)depth;
        }
    };
    // a heap for elements
    ListAllocDH<SnapshotElement, 50>    SnapshotHeap;
    // a sorted by depth array of pointers to elements, ascending order
    ArrayDH_POD<SnapshotElement*>       SnapshotSortedArray;
    // a doubly-linked list of elements, in order of addition
    List<SnapshotElement>               SnapshotList;
    // the owner sprite
    InteractiveObject*                  pOwnerSprite;

    // finds the last element at the specified depth (upper-bound)
    SnapshotElement* FindDepth(int depth, UPInt* pidx = NULL)
    {
        UPInt i = Alg::UpperBound(SnapshotSortedArray, depth, DepthLess);
        if (i != 0)
        {
            --i;
            if (SnapshotSortedArray[UPInt(i)]->Depth == depth)
            {
                if (pidx)
                    *pidx = i;
                return SnapshotSortedArray[UPInt(i)];
            }
        }
        return NULL;
    }
private:
    static int DepthLess(int depth, const SnapshotElement* pse)
    {
        return (depth < pse->Depth);
    }
public:
    enum DirectionType
    {
        Direction_Forward,
        Direction_Backward
    } Direction;
    TimelineSnapshot(DirectionType dir, MemoryHeap* pheap, InteractiveObject* powner) 
        : SnapshotHeap(pheap), SnapshotSortedArray(pheap), pOwnerSprite(powner), Direction(dir) {}

    ~TimelineSnapshot()
    {
        FreeListElements(SnapshotList, SnapshotHeap);
    }

    // Add an element at the specified depth
    SnapshotElement* Add(int depth);

    // Remove the last (upper-bound) element at the specified depth
    void Remove(int depth);

    // Remove element at the specified index
    void RemoveAtIndex(UPInt idx)
    {
        SnapshotElement* pe = SnapshotSortedArray[idx];
        SnapshotList.Remove(pe);
        SnapshotSortedArray.RemoveAt(idx);
        SnapshotHeap.Free(pe);
    }

    void MakeSnapshot(TimelineDef* pdef, unsigned startFrame, unsigned endFrame);
    void ExecuteSnapshot(DisplayObjContainer* pdispObj);
};

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_SPRITEDEF_H

