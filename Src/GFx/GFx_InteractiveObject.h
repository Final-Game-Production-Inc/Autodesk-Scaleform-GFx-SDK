/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_InteractiveObject.h
Content     :   InteractiveObject
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_InteractiveObject_H
#define INC_SF_GFX_InteractiveObject_H

#include "GFx/GFx_Types.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_ASSoundIntf.h"

namespace Scaleform { namespace GFx {

class MovieImpl;
class CharPosInfo;
class SwfEvent;
struct CharacterCreateInfo;

class Sprite;
class InteractiveObject;
class AvmInteractiveObjBase;
class SoundResource;

// InteractiveObject is a regular character that adds ActionScript control capabilities
// and extra fields, such as _name, which are only available in those objects.

class InteractiveObject : public DisplayObject
{
    friend class DisplayObjContainer;
//    friend class GFxButtonCharacter;
private:
    //SF_INLINE AvmInteractiveObjBase* GetAvmIntObj() const;
public:
    // pPlayNext and pPrevPlay
    InteractiveObject*  pPlayNext;
    InteractiveObject*  pPlayPrev;
    union
    {
        InteractiveObject* pPlayNextOpt;  // next optimized char to play (OptAdvancedList flag is set)
        InteractiveObject* pNextUnloaded; // next unloaded char (Unloaded flag is set)
    };
    InteractiveObject*  pPlayPrevOpt;
protected:
    // Data binding root for character; store it since there is no binding 
    // information in the corresponding SpriteDef, ButtonDef, etc.
    // Resources for locally loaded handles come from here.
    // Technically, some other DisplayObjectBase's might need this too, but in most
    // cases they are ok just relying on binding info within GFxDisplayContext.
    Ptr<MovieDefImpl>               pDefImpl;

    enum FlagMasks
    {
        //Mask_Visible            = 0x1,
        //Mask_Alpha0             = 0x2,   // set, if alpha is zero
        Mask_NoAdvanceLocal     = 0x000004,   // disable advance locally
        Mask_NoAdvanceGlobal    = 0x000008,   // disable advance globally
        Mask_Enabled            = 0x000010,
        Mask_TabEnabled         = 0x000060,  // 0 - undef, 1,2 - false, 3 - true
        Mask_FocusRect          = 0x000180, // 0 - undef, 1,2 - false, 3 - true
        Mask_UseHandCursor      = 0x000600, // 0 - undef, 1,2 - false, 3 - true
        Mask_HitTestDisable     = 0x000800,
        Mask_MouseDisabled      = 0x001000,
        Mask_MouseChildrenDisabled= 0x2000,
        Mask_TrackAsMenu        = 0x004000,
        Mask_TabChildrenDisabled= 0x008000, 
        Mask_ChangedFlags       = 0x0F0000, // State changed flags (see StateChangedFlags)
        Mask_ReqPartialAdvance  = 0x100000, // should get partial advance calls
        Mask_OptAdvList         = 0x200000, // is in optimized advance list
        Mask_MarkedToRemoveFromOptAdvList = 0x400000,
        Mask_OptAdvListMarker   = 0x800000, // must be equal to pMovieImpl->Flag2_OptAdvListMarker
                                            // to be considered as 'valid' member of opt list.

        Shift_ChangedFlags      = 16
    };
    UInt32      Flags;
    SInt16      TabIndex;
    UInt16      FocusGroupMask; // 0 means - take from parent, 0xFFFF - all ctrlrs may focus on
    UInt8       RollOverCnt; // count of rollovers (for multiple mouse cursors)

    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
public:
    void SetNoAdvanceLocalFlag(bool v = true)  { (v) ? Flags |= Mask_NoAdvanceLocal : Flags &= (~Mask_NoAdvanceLocal); }
    void ClearNoAdvanceLocalFlag()             { SetNoAdvanceLocalFlag(false); }
    bool IsNoAdvanceLocalFlagSet() const       { return (Flags & Mask_NoAdvanceLocal) != 0; }

    void SetOptAdvListFlag(bool v = true)      { (v) ? Flags |= Mask_OptAdvList : Flags &= (~Mask_OptAdvList); }
    void ClearOptAdvListFlag()                 { SetOptAdvListFlag(false); }
    bool IsOptAdvListFlagSet() const           { return (Flags & Mask_OptAdvList) != 0; }

    void SetReqPartialAdvanceFlag(bool v = true) { (v) ? Flags |= Mask_ReqPartialAdvance : Flags &= (~Mask_ReqPartialAdvance); }
    void ClearReqPartialAdvanceFlag()            { SetReqPartialAdvanceFlag(false); }
    bool IsReqPartialAdvanceFlagSet() const      { return (Flags & Mask_ReqPartialAdvance) != 0; }

    UInt16 GetFocusGroupMask() const;
    UInt16 GetFocusGroupMask();
    virtual bool   IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const;
    virtual bool   IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx);
public:

    bool IsAdvanceDisabled() const { return (Flags & (Mask_NoAdvanceLocal | Mask_NoAdvanceGlobal)) != 0; }

    void SetNoAdvanceGlobalFlag(bool v = true)  { (v) ? Flags |= Mask_NoAdvanceGlobal : Flags &= (~Mask_NoAdvanceGlobal); }
    void ClearNoAdvanceGlobalFlag()             { SetNoAdvanceGlobalFlag(false); }
    bool IsNoAdvanceGlobalFlagSet() const       { return (Flags & Mask_NoAdvanceGlobal) != 0; }

    // Technically TrackAsMenu and Enabled are only available in Sprite and Button,
    // but its convenient to put them here.
    void SetTrackAsMenuFlag(bool v = true) { (v) ? Flags |= Mask_TrackAsMenu : Flags &= (~Mask_TrackAsMenu); }
    void ClearTrackAsMenuFlag()            { SetTrackAsMenuFlag(false); }
    bool IsTrackAsMenuFlagSet() const      { return (Flags & Mask_TrackAsMenu) != 0; }

    void SetEnabledFlag(bool v = true) { (v) ? Flags |= Mask_Enabled : Flags &= (~Mask_Enabled); }
    void ClearEnabledFlag()            { SetEnabledFlag(false); }
    bool IsEnabledFlagSet() const      { return (Flags & Mask_Enabled) != 0; }

    void SetMouseDisabledFlag(bool v = true) { (v) ? Flags |= Mask_MouseDisabled : Flags &= (~Mask_MouseDisabled); }
    void ClearMouseDisabledFlag()            { SetMouseDisabledFlag(false); }
    bool IsMouseDisabledFlagSet() const      { return (Flags & Mask_MouseDisabled) != 0; }

    void SetMouseChildrenDisabledFlag(bool v = true) { (v) ? Flags |= Mask_MouseChildrenDisabled : Flags &= (~Mask_MouseChildrenDisabled); }
    void ClearMouseChildrenDisabledFlag()            { SetMouseChildrenDisabledFlag(false); }
    bool IsMouseChildrenDisabledFlagSet() const      { return (Flags & Mask_MouseChildrenDisabled) != 0; }

    void SetTabChildrenDisabledFlag(bool v = true) { (v) ? Flags |= Mask_TabChildrenDisabled : Flags &= (~Mask_TabChildrenDisabled); }
    void ClearTabChildrenDisabledFlag()            { SetTabChildrenDisabledFlag(false); }
    bool IsTabChildrenDisabledFlagSet() const      { return (Flags & Mask_TabChildrenDisabled) != 0; }

    void SetTabEnabledFlag(bool v = true) 
    { 
        (v) ? (Flags |= Mask_TabEnabled) : Flags = ((Flags & (~Mask_TabEnabled)) | ((Mask_TabEnabled - 1) & Mask_TabEnabled)); 
    }
    void ClearTabEnabledFlag()            { SetTabEnabledFlag(false); }
    void UndefineTabEnabledFlag()         { Flags &= (~Mask_TabEnabled); }
    bool IsTabEnabledFlagDefined() const  { return (Flags & Mask_TabEnabled) != 0; }
    bool IsTabEnabledFlagTrue() const     { return (Flags & Mask_TabEnabled) == Mask_TabEnabled; }
    bool IsTabEnabledFlagFalse() const    { return IsTabEnabledFlagDefined() && !IsTabEnabledFlagTrue(); }

    void SetFocusRectFlag(bool v = true) 
    { 
        (v) ? (Flags |= Mask_FocusRect) : Flags = ((Flags & (~Mask_FocusRect)) | ((Mask_FocusRect - 1) & Mask_FocusRect)); 
    }
    void ClearFocusRectFlag()            { SetFocusRectFlag(false); }
    void UndefineFocusRectFlag()         { Flags &= (~Mask_FocusRect); }
    bool IsFocusRectFlagDefined() const  { return (Flags & Mask_FocusRect) != 0; }
    bool IsFocusRectFlagTrue() const     { return (Flags & Mask_FocusRect) == Mask_FocusRect; }
    bool IsFocusRectFlagFalse() const    { return IsFocusRectFlagDefined() && !IsFocusRectFlagTrue(); }

    void SetUseHandCursorFlag(bool v = true) 
    { 
        (v) ? (Flags |= Mask_UseHandCursor) : Flags = ((Flags & (~Mask_UseHandCursor)) | ((Mask_UseHandCursor - 1) & Mask_UseHandCursor)); 
    }
    void ClearUseHandCursorFlag()            { SetUseHandCursorFlag(false); }
    void UndefineUseHandCursorFlag()         { Flags &= (~Mask_UseHandCursor); }
    bool IsUseHandCursorFlagDefined() const  { return (Flags & Mask_UseHandCursor) != 0; }
    bool IsUseHandCursorFlagTrue() const     { return (Flags & Mask_UseHandCursor) == Mask_UseHandCursor; }
    bool IsUseHandCursorFlagFalse() const    { return IsUseHandCursorFlagDefined() && !IsUseHandCursorFlagTrue(); }

    void SetHitTestDisableFlag(bool v = true) { (v) ? Flags |= Mask_HitTestDisable : Flags &= (~Mask_HitTestDisable); }
    void ClearHitTestDisableFlag()            { SetHitTestDisableFlag(false); }
    bool IsHitTestDisableFlagSet() const      { return (Flags & Mask_HitTestDisable) != 0; }

    void MarkToRemoveFromOptimizedPlayList()           { Flags |= Mask_MarkedToRemoveFromOptAdvList;  }
    void ClearMarkToRemoveFromOptimizedPlayListFlag()  { Flags &= (~Mask_MarkedToRemoveFromOptAdvList); }
    bool IsMarkedToRemoveFromOptimizedPlayList() const { return (Flags & Mask_MarkedToRemoveFromOptAdvList) != 0; }

    bool GetOptAdvListMarker() const { return (Flags & Mask_OptAdvListMarker) != 0; }
    void SetOptAdvListMarker(bool v) { (v) ? Flags |= Mask_OptAdvListMarker : Flags &= (~Mask_OptAdvListMarker); }
    bool IsValidOptAdvListMember(MovieImpl* proot) const;

    // Display callbacks
    void                    (SF_CDECL *pDisplayCallback)(void*);
    void*                   DisplayCallbackUserPtr;

public:

    // Constructor.
    InteractiveObject(
        MovieDefImpl* pbindingDefImpl, 
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id);
    ~InteractiveObject();

    virtual MovieDefImpl*   GetResourceMovieDef() const;    
    virtual void            SetBlendMode(BlendType blend)       
    { 
        DisplayObjectBase::SetBlendMode(blend); 
        SetDirtyFlag(); 
    }
    virtual Render::TreeContainer* GetRenderContainer();

    // *** native methods

    // Button & Sprite shared vars access.
    bool                    GetTrackAsMenu() const              { return IsTrackAsMenuFlagSet(); } 
    bool                    GetEnabled() const                  { return IsEnabledFlagSet();   }

    // Focus related stuff
    virtual bool            IsTabable() const;
    inline  bool            IsTabIndexed() const { return TabIndex > 0; }
    inline  int             GetTabIndex() const { return TabIndex; }
    inline  void            SetTabIndex(int ti) { TabIndex = (SInt16)ti; }
    // returns true, if yellow focus rect is enabled for the character
    virtual bool            IsFocusRectEnabled() const;
    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool            IsFocusEnabled(FocusMovedType = GFx_FocusMovedByKeyboard) const;

    // focus
    struct FillTabableParams
    {
        ArrayDH<Ptr<InteractiveObject>, StatMV_Other_Mem>* Array;
        bool    TabIndexed; 
        bool    InclFocusEnabled;
        Bool3W  TabChildrenInProto;
        UInt16  FocusGroupMask;         // current focus group mask
        unsigned ControllerIdx;

        FillTabableParams():TabIndexed(false), InclFocusEnabled(false) {}
    };

    // Special event handler; ensures that unload is called on child items in timely manner.
    virtual void            OnEventLoad();
    virtual void            OnEventUnload();
    virtual bool            OnUnloading();
    enum FocusEventType
    {
        KillFocus,
        SetFocus
    };
    // invoked when lose/gain focus
    virtual void            OnFocus(FocusEventType event, 
        InteractiveObject* oldOrNewFocusCh, unsigned controllerIdx, FocusMovedType fmt);

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType) { SF_UNUSED(controllerIdx); }
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard) 
    { 
        SF_UNUSED3(newFocusCh, fmt, controllerIdx); 
        return true; 
    }
    // returns rectangle for focusrect, in local coords
    virtual RectF           GetFocusRect() const 
    {
        return GetBounds(Matrix2F());
    }

    virtual void            CloneInternalData(const InteractiveObject* src);
    virtual void            SetPause(bool pause)    { SF_UNUSED(pause); }


    // Special event handler; key down and up
    // See also PropagateKeyEvent, KeyMask
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask)     
    { SF_UNUSED(pkeyMask); return OnEvent(id); }
    // Special event handler; char
    virtual bool            OnCharEvent(UInt32 wcharCode, unsigned controllerIdx)     
    { SF_UNUSED2(wcharCode, controllerIdx); return false; }
    // Special event handler; mouse wheel support
    virtual bool            OnMouseWheelEvent(int mwDelta)
    { SF_UNUSED(mwDelta); return false; }

    // Propagates mouse event to all of the eligible clips.
    // Called to notify handlers of onClipEvent(mouseMove, mouseDown, mouseUp)
    virtual void            PropagateMouseEvent(const EventId& id)   
    { OnEvent(id); }
    virtual bool            OnMouseEvent(const EventId& id)     { return OnEvent(id); }

    // Propagates key event to all of the eligible clips.
    // Called to notify handlers of onClipEvent(keyDown, keyUp), onKeyDown/onKeyUp, on(keyPress)
    enum KeyMask {
        KeyMask_KeyPress            = 0x010000,
        KeyMask_onKeyDown           = 0x020000,
        KeyMask_onKeyUp             = 0x040000,
        KeyMask_onClipEvent_keyDown = 0x080000,
        KeyMask_onClipEvent_keyUp   = 0x100000,
        KeyMask_FocusedItemHandledMask = 0xFFFF // should be set, if focused item already handled the keyevent (one bit per controller)
    };
    virtual void            PropagateKeyEvent(const EventId& id, int* pkeyMask)
    { 
        OnKeyEvent(id, pkeyMask); 
    }

    virtual void            SetDisplayCallback(void (SF_CDECL * callback)(void*), void* userPtr)
    {
        pDisplayCallback = callback;
        DisplayCallbackUserPtr = userPtr;
    }
    virtual void            DoDisplayCallback()
    {
        if (pDisplayCallback)
            (*pDisplayCallback)(DisplayCallbackUserPtr);
    }

    // Utility.
    void                    DoMouseDrag(unsigned mouseIndex);

    // Returns 0 if nothing to do
    // 1 - if need to add to optimized play list
    // -1 - if need to remove from optimized play list
    virtual int             CheckAdvanceStatus(bool playingNow) { SF_UNUSED(playingNow); return 0; }
    // Modify the optimized playlist by adding/removing this char from it.
    void                    ModifyOptimizedPlayList()
    {
        //if (proot->IsOptAdvanceListInvalid()) return;
        switch (CheckAdvanceStatus(IsOptAdvListFlagSet() && !IsMarkedToRemoveFromOptimizedPlayList()))
        {
        case 1:  AddToOptimizedPlayList();            break;
        // delayed removal from optimized list; required for AS3 to execute frames even
        // for objects, which are already not 'adavancing'. Actual removal will occur
        // in MovieImpl::AdvanceFrame.
        case -1: MarkToRemoveFromOptimizedPlayList(); break;
        }
    }
    template <class T>
    void                    ModifyOptimizedPlayListLocal()
    {
        //if (proot->IsOptAdvanceListInvalid()) return;
        switch (static_cast<T*>(this)->T::CheckAdvanceStatus(IsOptAdvListFlagSet() && !IsMarkedToRemoveFromOptimizedPlayList()))
        {
        case 1:  AddToOptimizedPlayList();            break;
        // delayed removal from optimized list; required for AS3 to execute frames even
        // for objects, which are already not 'adavancing'. Actual removal will occur
        // in MovieImpl::AdvanceFrame.
        case -1: MarkToRemoveFromOptimizedPlayList(); break;
        }
    }
    virtual void            PropagateNoAdvanceGlobalFlag() {}
    virtual void            PropagateNoAdvanceLocalFlag() {}
    virtual void            PropagateFocusGroupMask(unsigned mask) 
    {
        FocusGroupMask = (UInt16)mask;
    }


    // *** Parent list manipulation functions (work on all types of characters)

    // Adds a child character to playlist.
    void                    AddToPlayList();
    void                    InsertToPlayListAfter(InteractiveObject* pafterCh);
    bool                    IsInPlayList() const;
    void                    AddToOptimizedPlayList();

    // Removes the character from the playlist
    void                    RemoveFromPlayList();
    void                    RemoveFromOptimizedPlayList();

    // *** Movie Loading support

    // Helper for ReplaceChildCharacterOnLoad.
    // Copies physical properties and reassigns character handle.
    void                    CopyPhysicalProperties(const InteractiveObject *poldChar);
    // Reassigns character name handle.
    void                    MoveNameHandle(InteractiveObject *poldChar);


    // *** GFxSprite's virtual methods.

    // These timeline related methods are implemented only in GFxSprite; for other
    // ActionScript characters they do nothing. It is convenient to put them here
    // to avoid many type casts in ActionScript opcode implementation.
    virtual unsigned        GetCurrentFrame() const             
    { return 0; }   
    virtual bool            GetLabeledFrame(const char* plabel, unsigned* frameNumber, bool translateNumbers = 1) 
    { SF_UNUSED3(plabel, frameNumber, translateNumbers); return 0; }
    virtual const String*   GetFrameLabel(unsigned, unsigned* = NULL) { return NULL; }
    virtual void            GotoFrame(unsigned targetFrameNumber)   
    { SF_UNUSED(targetFrameNumber); }
//    virtual void            SetPlayState(GFxMovie::PlayState s) 
//    { SF_UNUSED(s); }
    virtual unsigned        GetLoadingFrame() const { return 0; }
    virtual void            ExecuteFrame0Events() {}
    virtual bool            HasLooped() const { return false; }
    virtual void            SetPlayState(PlayState) {}
    virtual PlayState       GetPlayState() const    { return State_Stopped; }

    virtual unsigned        GetCursorType() const;

    virtual void SetStateChangeFlags(UInt8 flags) 
    { 
        Flags = (Flags & ~Mask_ChangedFlags) | ((flags << Shift_ChangedFlags) & Mask_ChangedFlags); 
    }
    virtual UInt8 GetStateChangeFlags() const { return UInt8((Flags & Mask_ChangedFlags) >> Shift_ChangedFlags); }

    unsigned IncrementRollOverCnt() { return RollOverCnt++; }
    unsigned DecrementRollOverCnt() { return (RollOverCnt != 0) ? --RollOverCnt : ~0u; }

    ASStringManager*        GetStringManager() const;
    //StringManager* GetBuiltinStringManager() const;
    
    // Remove the object from parent's displaylist
    void                    RemoveDisplayObject();
    virtual void            RemoveDisplayObject(DisplayObjectBase*) {}

    MovieDefImpl*           GetDefImpl() const    { return pDefImpl; }
    MovieImpl*              GetMovieImpl() const  { return pASRoot->GetMovieImpl(); }
    int                     GetAVMVersion() const { return pASRoot->GetAVMVersion(); }
    bool                    IsAVM1() const { return GetAVMVersion() == 1; }
    bool                    IsAVM2() const { return GetAVMVersion() == 2; }

    // Returns 1 if sprite acts as a button due to handlers.
    SF_INLINE bool ActsAsButton() const;

    void                    SetNextUnloaded(InteractiveObject*);

#if defined(GFX_ENABLE_SOUND) 
    virtual void            StopActiveSounds() {}
    virtual void            StopActiveSounds(SoundResource*) {}
    virtual void            StopActiveSounds(ASSoundIntf*) {}
#endif // GFX_ENABLE_SOUND

    AvmInteractiveObjBase*          GetAvmIntObj();
    const AvmInteractiveObjBase*    GetAvmIntObj() const;
};


class AvmInteractiveObjBase : public AvmDisplayObjBase
{
public:
    // Returns 1 if sprite acts as a button due to handlers.
    virtual bool            ActsAsButton() const =0;
    virtual unsigned        GetCursorType() const =0;
    virtual void            CloneInternalData(const InteractiveObject* src) =0;
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar) =0;

    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
         unsigned controllerIdx, FocusMovedType fmt) =0;

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt) =0;
    // Special event handler; key down and up
    // See also PropagateKeyEvent, KeyMask
    virtual bool            OnKeyEvent(const EventId& id, int* pkeyMask) =0;
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard) =0;

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool            IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const =0;
    virtual bool            IsTabable() const = 0;
};

SF_INLINE AvmInteractiveObjBase* InteractiveObject::GetAvmIntObj()
{
    return GetAvmObjImpl()->ToAvmInteractiveObjBase();
}
SF_INLINE const AvmInteractiveObjBase* InteractiveObject::GetAvmIntObj() const
{
    return const_cast<const AvmInteractiveObjBase*>(
        const_cast<AvmDisplayObjBase*>(GetAvmObjImpl())->ToAvmInteractiveObjBase());
}

SF_INLINE
bool InteractiveObject::ActsAsButton() const
{
    if (HasAvmObject())
        return GetAvmIntObj()->ActsAsButton();
    return false;
}

// conversion methods
SF_INLINE
InteractiveObject* DisplayObjectBase::CharToInteractiveObject()
{
    return IsInteractiveObject() ? static_cast<InteractiveObject*>(this) : 0;
}

SF_INLINE
InteractiveObject* DisplayObjectBase::CharToInteractiveObject_Unsafe()
{
    SF_ASSERT(IsInteractiveObject());
    return static_cast<InteractiveObject*>(this);
}

SF_INLINE
const InteractiveObject* DisplayObjectBase::CharToInteractiveObject() const 
{
    return IsInteractiveObject() ? static_cast<const InteractiveObject*>(this) : 0;
}

SF_INLINE
const InteractiveObject* DisplayObjectBase::CharToInteractiveObject_Unsafe() const 
{
    SF_ASSERT(IsInteractiveObject());
    return static_cast<const InteractiveObject*>(this);
}


}} // Scaleform::GFx

#endif //INC_SF_GFX_InteractiveObject_H

