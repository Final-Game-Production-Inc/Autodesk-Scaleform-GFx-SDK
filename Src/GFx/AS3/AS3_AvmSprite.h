#ifndef INC_SF_GFx_AS3_AvmSprite_H
#define INC_SF_GFx_AS3_AvmSprite_H

#include "Kernel/SF_BitSet.h"
#include "GFx/AS3/AS3_AvmDisplayObjContainer.h"
#include "GFx/GFx_SpriteDef.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS3/AS3_Value.h"

namespace Scaleform {
namespace GFx {

namespace AS3 {

class MovieRoot;

#if 0
// This class contains associations of frame numbers with corresponding
// ActionScript method to be called when appropriate frame is executed.
// Contains a sorted by frame number array of descriptors along with
// a bitset that indicates presence of action at the corresponding frame
// (for quick check, before going to do sorted array search).
class FrameScript
{
public:
    struct Descr
    {
        Value    Method;
        unsigned Frame;  // number of frame

        Descr():Frame(0) {}
        Descr(unsigned f, const Value& frMethod):Method(frMethod), Frame(f) {}
        Descr(const Descr& o): Method(o.Method), Frame(o.Frame) {}

        bool operator < (const Descr& f) const { return Frame < f.Frame; }
    };
private:
    UByte*  pData;
    UInt16  DescrCnt; // number of Descr elements
    UInt16  FrameCnt; // total number of frames (pData[DescrCnt - 1].Number + 1))
public:
    FrameScript() : pData(NULL), DescrCnt(0), FrameCnt(0) {}
    ~FrameScript();

    // initial setup
    void Set(MemoryHeap* pheap, unsigned cnt, const Descr*);

    // gets descr by frame number. Frame number is 0-based
    const Descr* Get(unsigned frameNum);

    // Release all records. Necessary to do before AVM shutdown.
    void Clear();

    bool IsEmpty() const { return DescrCnt == 0; }
};
#endif

// AS3-specific part of the Sprite. Allocated in AS3Support class.
class AvmSprite : 
    public AvmDisplayObjContainer,
    public AvmSpriteBase
{
    friend class Stage;
public:
    FixedBitSetLH<StatMV_ActionScript_Mem>  InitActionsExecuted;
    enum
    {
        Flag_ButtonMode         = 0x01,
        Flag_NeedExecuteFrame   = 0x02,
        Flag_ChildrenCreated    = 0x04
    };
    UInt8   Flags;

protected:
    void SetChildrenCreatedFlag(bool v = true)
    {
        (v) ? Flags |= Flag_ChildrenCreated : Flags &= ~Flag_ChildrenCreated;
    }

public:
    AvmSprite(Sprite* psprite);
    ~AvmSprite();

    //***** AvmDisplayObjBase implementation
    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmInteractiveObj*>(this); 
    }
    virtual class AvmSpriteBase*            ToAvmSpriteBase() { return this; }
    virtual class AvmButtonBase*            ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase() { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() 
    { 
        return AvmDisplayObjContainer::ToAvmDispContainerBase(); 
    }
    virtual bool                            OnEvent(const EventId& );
    virtual void                            OnEventLoad();
    virtual void                            OnEventUnload();
    //virtual void                            OnRemoved() { AvmInteractiveObj::OnRemoved(); }
    virtual bool                            OnUnloading(bool mayRemove);
    virtual bool                            HasEventHandler(const EventId& id) const
    {
        return AvmDisplayObj::HasEventHandler(id);
    }

    //***** AvmInteractiveObjectBase implementation
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmInteractiveObj::CopyPhysicalProperties(poldChar);
    }
    // Determines the absolute path of the character.
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmInteractiveObj::GetAbsolutePath(ppath);
    }
    virtual bool            IsTabable() const;
    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }
    virtual void            OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnGettingKeyboardFocus(controllerIdx, fmt);
    }
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask)
    {
        return AvmInteractiveObj::OnKeyEvent(id, pkeyMask);
    }
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard)
    {
        return AvmInteractiveObj::OnLosingKeyboardFocus(newFocusCh, controllerIdx, fmt);
    }


    //AvmDisplayObjBase*      ToAvmDispObj() { return static_cast<AvmInteractiveObj*>(this); }
    SF_INLINE Sprite*       GetSprite() const { return static_cast<Sprite*>(pDispObj); }


    SpriteDef*              GetSpriteDef() const 
    { 
        SF_ASSERT(pDispObj); 
        return static_cast<Sprite*>(pDispObj)->GetSpriteDef();
    }
    TimelineDef*            GetDef() const
    {
        SF_ASSERT(pDispObj); 
        return GetSprite()->GetDef();
    }
    MovieDefImpl*           GetDefImpl() const    
    { 
        SF_ASSERT(pDispObj); 
        return GetSprite()->GetDefImpl();
    }

    bool            IsUnloaded() const { return GetSprite()->IsUnloaded(); }
    bool            IsEnabledFlagSet() const { return GetSprite()->IsEnabledFlagSet(); }

    virtual const char* GetDefaultASClassName() const { return "flash.display.Sprite"; }

    //////////////////////////////////////////////////////////////////////////
    virtual void                AdvanceFrame(bool nextFrame, float framePos);
    virtual DisplayObjectBase*  AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame,
        UInt32 addFlags,
        CharacterCreateInfo* pcharCreateOverride,
        InteractiveObject* origChar);

    virtual bool    ActsAsButton() const;
    virtual bool    IsLevelMovie() const;
    virtual void    ForceShutdown();
    virtual unsigned    GetCursorType() const;

    virtual void    ExecuteInitActionFrameTags(unsigned frame);
    // Used for root-level sprites.
    virtual void    ExecuteFrame0Events();
    virtual void    ExecuteFrameTags(unsigned frame);
    virtual void    CloneInternalData(const InteractiveObject* src);

    virtual TopMostResult GetTopMostEntity(const Render::PointF &localPt, TopMostDescr* pdescr,
                                           const ArrayPOD<UByte>& hitTest)
    {
        return AvmDisplayObjContainer::GetTopMostEntity(localPt, pdescr, hitTest);
    }
    virtual void    FillTabableArray(InteractiveObject::FillTabableParams* params)
    {
        AvmDisplayObjContainer::FillTabableArray(params);
    }

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool    IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const;

    virtual bool    MustBeInPlaylist() const;

    // Associate number of frames with corresponding method
    void            QueueFrameScript(unsigned frame);
    bool            GetFrameScript(unsigned frame, Value* pmethod);
    bool            HasFrameScript() const;

    unsigned        GetCurrentSprite() const { return GetSprite()->GetCurrentFrame(); }

    void SetButtonMode(bool v = true)
    {
        (v) ? Flags |= Flag_ButtonMode : Flags &= ~Flag_ButtonMode;
    }
    bool IsButtonMode() const
    {
        return (Flags & Flag_ButtonMode) != 0;
    }
    void SetNeedExecuteFrameFlag(bool v = true)
    {
        if (v)
        {
            Flags |= Flag_NeedExecuteFrame;
            // execution of a frame requires the display obj to be in optimized playlist;
            // unless the optimized playlist is marked 'invalid' (this condition is checked
            // inside the AddToOptimizedPlayList). If it is not in playlist yet, then
            // CheckAdvanceStatus will take care of it.
            if (GetSprite()->IsInPlayList())
                GetSprite()->AddToOptimizedPlayList();
        }
        else
            Flags &= ~Flag_NeedExecuteFrame;
    }
    bool IsNeedExecuteFrameFlagSet() const
    {
        return (Flags & Flag_NeedExecuteFrame) != 0;
    }

    // Creates children
    void  CreateChildren();
    bool  IsChildrenCreatedFlagSet() const
    {
        return (Flags & Flag_ChildrenCreated) != 0;
    }

    // fill an array with objects under the point. The point is in container's coord space.
    bool                GetObjectsUnderPoint(ArrayDH<Ptr<DisplayObjectBase> >* destArray, const PointF& pt) const;

    DrawingContext*     GetDrawingContext() const { return GetSprite()->GetDrawingContext(); }
    bool                HasDrawingContext() const { return GetSprite()->HasDrawingContext(); }

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::Sprite"; }
#endif
};

SF_INLINE AvmSprite* ToAvmSprite(InteractiveObject* po)
{
    SF_ASSERT(po);
    SF_ASSERT(po->IsAVM2() && po->IsSprite());
    return static_cast<AvmSprite*>(static_cast<AvmDisplayObj*>(po->GetAvmObjImpl()));
}

// Representation of AVM-dependent MovieClip
class AvmMovieClip : public AvmSprite
{
public:
    AvmMovieClip(Sprite* psprite) : AvmSprite(psprite) {}
    virtual ~AvmMovieClip();

    virtual const char* GetDefaultASClassName() const { return "flash.display.MovieClip"; }
};


}}} // SF::GFx::AS3
#endif // INC_SF_GFx_AS3_AvmSprite_H

