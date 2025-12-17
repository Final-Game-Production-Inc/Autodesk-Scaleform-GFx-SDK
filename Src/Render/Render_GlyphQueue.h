/**************************************************************************

PublicHeader:   Render
Filename    :   Render_GlyphQueue.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GlyphQueue_H
#define INC_SF_Render_GlyphQueue_H

#include "Kernel/SF_Types.h"

//#ifdef SF_RENDER_ENABLE_GLYPH_CACHE

#include "Kernel/SF_ArrayUnsafe.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_ListAlloc.h"
#include "Render_Types2D.h"
#include "Render_GlyphParam.h"
#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render {

class FontCacheHandle;


//------------------------------------------------------------------------
struct GlyphRect
{
    UInt16 x,y,w,h;
    GlyphRect() {}
    GlyphRect(unsigned x_, unsigned y_, unsigned w_, unsigned h_):
       x(UInt16(x_)), y(UInt16(y_)), w(UInt16(w_)), h(UInt16(h_)) {}
};


struct GlyphNode;
struct GlyphBand;
class  TextMeshProvider;
struct GlyphSlot;

//------------------------------------------------------------------------
class GlyphEvictNotifier
{
public:
    virtual ~GlyphEvictNotifier() {}
    virtual void Evict(TextMeshProvider* b) = 0;
    virtual void ApplyInUseList() = 0;
    virtual bool UpdatePinList() = 0;
};

//------------------------------------------------------------------------
struct TextNotifier : public ListNode<TextNotifier>
{
    TextMeshProvider* pText;
    GlyphSlot*        pSlot;
};

//------------------------------------------------------------------------
struct GlyphSlot : ListNode<GlyphSlot>
{
    enum 
    { 
        FullFlag = 0x8000U,
        Mask     = 0x8000U
    };

    GlyphNode*          pRoot;
    GlyphSlot*          pPrevInBand;
    GlyphSlot*          pNextInBand;
    GlyphSlot*          pPrevActive;
    GlyphSlot*          pNextActive;
    GlyphBand*          pBand;
    UInt16              TextureId;   // Index of the texture in the array. May have FullFlag.
    UInt16              x,w;
    UInt16              Failures;
    unsigned            PinCount;
    List<TextNotifier>  TextFields;
    Ptr<Fence>          SlotFence;      // The latest fence that uses this slot (or 0 if none).
};


//------------------------------------------------------------------------
struct GlyphSlot_Band
{
static void SetPrev(GlyphSlot* self, GlyphSlot* what)  { self->pPrevInBand = what; }
static void SetNext(GlyphSlot* self, GlyphSlot* what)  { self->pNextInBand = what; }
static const GlyphSlot* GetPrev(const GlyphSlot* self) { return self->pPrevInBand; }
static const GlyphSlot* GetNext(const GlyphSlot* self) { return self->pNextInBand; }
static       GlyphSlot* GetPrev(GlyphSlot* self)       { return self->pPrevInBand; }
static       GlyphSlot* GetNext(GlyphSlot* self)       { return self->pNextInBand; }
};

//------------------------------------------------------------------------
struct GlyphSlot_Active
{
static void SetPrev(GlyphSlot* self, GlyphSlot* what)  { self->pPrevActive = what; }
static void SetNext(GlyphSlot* self, GlyphSlot* what)  { self->pNextActive = what; }
static const GlyphSlot* GetPrev(const GlyphSlot* self) { return self->pPrevActive; }
static const GlyphSlot* GetNext(const GlyphSlot* self) { return self->pNextActive; }
static       GlyphSlot* GetPrev(GlyphSlot* self)       { return self->pPrevActive; }
static       GlyphSlot* GetNext(GlyphSlot* self)       { return self->pNextActive; }
};

//------------------------------------------------------------------------
struct GlyphNode
{
    GlyphParam     Param;
    GlyphSlot*     pSlot;
    GlyphNode*     pNext;       // pNext plays the role of Child[0]
    GlyphNode*     pNex2;       // pNex2 plays the role of Child[1]
    GlyphRect      mRect;
    Point<SInt16>  Origin;
    float          Scale;
};

//------------------------------------------------------------------------
struct GlyphBand
{
    UInt16 TextureId;
    UInt16 y,h;
    UInt16 RightSpace;
    List2<GlyphSlot, GlyphSlot_Band> Slots;
};

//------------------------------------------------------------------------
class GlyphQueueVisitor
{
public:
    enum RectType
    {
        Rect_Slot,
        Rect_SlotPinned,
        Rect_Glyph
    };

    virtual ~GlyphQueueVisitor() {}
    virtual void Visit(const GlyphParam* param, const RectF& rect, RectType type) = 0;
};



//------------------------------------------------------------------------
struct GlyphParamHash
{
    // Make sure the member variables stay with no padding
    // Padding breaks the hash function that uses this struct as a key
    const GlyphParam* Param;

    GlyphParamHash() : Param(0) {}
    GlyphParamHash(const GlyphParam* p) : Param(p) {}

    UPInt operator()(const GlyphParamHash& key) const
    {
        return (((UPInt)key.Param->pFont) >> 6) ^ (UPInt)key.Param->pFont ^ 
                 (UPInt)key.Param->GlyphIndex ^
                 (UPInt)key.Param->FontSize ^
                 (UPInt)key.Param->Flags ^
                 (UPInt)key.Param->BlurX ^
                ((UPInt)key.Param->BlurY << 1) ^
                 (UPInt)key.Param->BlurStrength;
    }

    bool operator == (const GlyphParamHash& key) const 
    { 
        return  Param->pFont        == key.Param->pFont && 
                Param->GlyphIndex   == key.Param->GlyphIndex &&
                Param->FontSize     == key.Param->FontSize &&
                Param->Flags        == key.Param->Flags &&
                Param->BlurX        == key.Param->BlurX &&
                Param->BlurY        == key.Param->BlurY &&
                Param->BlurStrength == key.Param->BlurStrength;
    }
};


//------------------------------------------------------------------------
class GlyphQueue
{
public:
    enum { SID = StatRender_Font_Mem }; // TO DO: StatID

    enum 
    { 
        // The maximal number of failed attempts, after which 
        // the slot is removed from the active list
        MaxSlotFailures = 16
    };

    GlyphQueue();

    void Clear();
    void Init(GlyphEvictNotifier* notifier, 
              unsigned firstTexture, unsigned numTextures, 
              unsigned textureWidth, unsigned textureHeight, 
              unsigned maxSlotHeight, bool fenceWaitOnFull);

    unsigned GetMaxSlotHeight() const { return MaxSlotHeight; }

    unsigned GetNumPacked() const { return (unsigned)GlyphHTable.GetSize(); }

    static void PinSlot  (GlyphSlot* slot);
    static void UnpinSlot(GlyphSlot* slot, Fence* fence);

    static bool IsPinned(GlyphSlot* slot, bool waitOn = false)
    {
        // Has pin counts left, or its fence has not passed.
        if ( slot->PinCount > 0 )
            return true;
        if ( waitOn && slot->SlotFence )
            slot->SlotFence->WaitFence(FenceType_Fragment);
        else if (slot->SlotFence && slot->SlotFence->IsPending(FenceType_Fragment))
            return true;

        // Fence has passed on the slot, clear it.
        SF_ASSERT(!slot->SlotFence || !slot->SlotFence->IsPending(FenceType_Fragment));
        slot->SlotFence.Clear();
        return false;
    }

    TextNotifier*  CreateNotifier(GlyphNode* node, TextMeshProvider* batch);
    void           RemoveNotifier(TextNotifier* notifier);

    void UnpinAllSlots();
    void CleanUpTexture(unsigned textureId);
    void CleanUpFont(const FontCacheHandle* font);
    void MergeEmptySlots();

    void SendGlyphToBack(GlyphNode* r) { SlotQueue.SendToBack(r->pSlot); }

    GlyphNode* FindGlyph(const GlyphParam& gp);
    GlyphNode* AllocateGlyph(const GlyphParam& gp, unsigned w, unsigned h);

    unsigned ComputeUsedArea() const;
    void VisitGlyphs(GlyphQueueVisitor* visitor) const;

    void PrintMemStats();
    UPInt GetBytes() const;

private:
    typedef HashLH<GlyphParamHash, GlyphNode*, GlyphParamHash, SID> CachedGlyphsType;
    typedef List2<GlyphSlot, GlyphSlot_Active>                      ActiveSlotsType;

    GlyphNode*          packGlyph(unsigned w, unsigned h, GlyphNode* glyph);
    GlyphNode*          packGlyph(unsigned w, unsigned h, GlyphSlot* slot);
    GlyphNode*          findSpaceInSlots(unsigned w, unsigned h);
    GlyphSlot*          initNewSlot(GlyphBand* band, unsigned x, unsigned w);
    GlyphNode*          allocateNewSlot(unsigned w, unsigned h);
    void                splitSlot(GlyphSlot* slot, unsigned w);
    void                splitGlyph(GlyphSlot* slot, bool left, unsigned w);
    bool                checkDistance(GlyphSlot* slot) const;
    GlyphSlot*          mergeSlotWithNeighbor(GlyphSlot* slot);
    void                mergeSlots(GlyphSlot* from, GlyphSlot* to, unsigned w);
    GlyphNode*          evictOldSlot(unsigned w, unsigned h, unsigned pass);
    GlyphNode*          evictOldSlot(unsigned w, unsigned h);
    void                releaseGlyphTree(GlyphNode* glyph);
    void                releaseSlot(GlyphSlot* slot);
    void                computeGlyphArea(const GlyphNode* glyph, unsigned* used) const;
    static const GlyphNode* findFontInSlot(GlyphNode* glyph, const FontCacheHandle* font);
    static bool             slotIsEmpty(const GlyphSlot* slot);
    static bool             slotsAreEmpty(const GlyphSlot* slot1, const GlyphSlot* slot2);

    unsigned                                    MinSlotSpace;
    unsigned                                    FirstTexture;
    unsigned                                    NumTextures;
    unsigned                                    TextureWidth;
    unsigned                                    TextureHeight;
    unsigned                                    MaxSlotHeight;
    unsigned                                    NumBandsInTexture;
    bool                                        FenceWaitOnFullCache;

    ListAllocLH_POD<GlyphSlot, 127, SID>        Slots;
    List<GlyphSlot>                             SlotQueue;
    unsigned                                    SlotQueueSize;
    ActiveSlotsType                             ActiveSlots;
    ListAllocLH_POD<GlyphNode, 127, SID>        Glyphs;
    ArrayUnsafeLH_POD<GlyphBand, SID>           Bands;
    CachedGlyphsType                            GlyphHTable;
    unsigned                                    NumUsedBands;
    ListAllocLH_POD<TextNotifier, 127, SID>     Notifiers;
    GlyphEvictNotifier*                         pEvictNotifier;
};

}} // Scaleform::Render
//#endif // SF_RENDER_ENABLE_GLYPH_CACHE

#endif
