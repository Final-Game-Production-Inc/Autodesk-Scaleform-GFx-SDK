/**************************************************************************

Filename    :   GFx_InteractiveObject.cpp
Content     :   InteractiveObject
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

// SF_INLINE AvmInteractiveObjBase* InteractiveObject::GetAvmIntObj() const
// {
//     return static_cast<AvmInteractiveObjBase*>(pAvmObjImpl.GetPtr());
// }

// Constructor.
InteractiveObject::InteractiveObject(
    MovieDefImpl* pbindingDefImpl,
    ASMovieRootBase* pasRoot,
    InteractiveObject* pparent, 
    ResourceId id)
                               :
    DisplayObject(pasRoot, pparent, id),
    pDefImpl(pbindingDefImpl),
    Flags(0),
    TabIndex (0),
    RollOverCnt(0),
    pDisplayCallback(NULL),
    DisplayCallbackUserPtr(NULL)
{
    FocusGroupMask = 0;
    SetInteractiveObjectFlag();
    pPlayNext = pPlayPrev = NULL;
    pPlayNextOpt = pPlayPrevOpt = NULL;

    SetEnabledFlag();
}

InteractiveObject::~InteractiveObject()
{
    RemoveFromPlayList();
}

Ptr<Render::TreeNode> InteractiveObject::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeContainer> node = *context.CreateEntry<Render::TreeContainer>();
    return node.GetPtr();
}

Render::TreeContainer* InteractiveObject::GetRenderContainer()
{
    // it is guaranteed that pRenNode is a TreeContainer!
    return static_cast<Render::TreeContainer*>(GetRenderNode());
}

MovieDefImpl* InteractiveObject::GetResourceMovieDef() const
{
    // Return the definition where binding takes place.
    return pDefImpl.GetPtr();
}

void InteractiveObject::SetNextUnloaded(InteractiveObject* punlObj)
{
    SF_ASSERT(!IsOptAdvListFlagSet());
    pNextUnloaded = punlObj;
    SetUnloaded();
}

void InteractiveObject::OnEventLoad()
{
    DisplayObject::OnEventLoad();
}

void InteractiveObject::OnEventUnload()
{
    SetUnloading();
    MovieImpl* proot = GetMovieImpl();
    RemoveFromPlayList();

    proot->StopDragCharacter(this);

    // Need to check, is this character currently focused or not. If yes, we need
    // to reset a pointer to currently focused character. OnEventUnload might be
    // called from the sprite's destructor (so refcnt == 0 but weak ref is not set to NULL yet)
    if (proot)
        proot->ResetFocusForChar(this);

//     // need to release the character to avoid accidental reusing unloaded character.
//     if (pNameHandle)
//         pNameHandle->ReleaseCharacter();

    DisplayObject::OnEventUnload();
}

bool InteractiveObject::OnUnloading()
{
    bool rv = DisplayObject::OnUnloading();
    return rv;
}

bool InteractiveObject::IsInPlayList() const 
{ 
    return pPlayNext || pPlayPrev ||  GetMovieImpl()->pPlayListHead == this; 
}

void InteractiveObject::AddToPlayList()
{
    if (IsUnloaded() || IsUnloading()) 
        return;
    MovieImpl* proot = GetMovieImpl();
    SF_ASSERT(proot->pPlayListHead != this);
    SF_ASSERT(!pPlayNext && !pPlayPrev);

    //@DBG
//     String s;
//     GetAbsolutePath(&s);
//     printf("=------------- %s\n", s.ToCStr()); //DBG

    // insert at the head of playlist
    if (proot->pPlayListHead)
    {
        proot->pPlayListHead->pPlayPrev = this;
        pPlayNext                       = proot->pPlayListHead;
    }
    proot->pPlayListHead                = this;

    proot->CheckPlaylistConsistency();
}

void InteractiveObject::InsertToPlayListAfter(InteractiveObject* pafterCh)
{
    if (IsUnloaded() || IsUnloading()) 
        return;
    MovieImpl* proot = GetMovieImpl();
    SF_UNUSED(proot);
    SF_ASSERT(proot->pPlayListHead != this);
    SF_ASSERT(!pPlayNext && !pPlayPrev);
    
    SF_ASSERT(pafterCh); // use AddToPlayList if pafterCh == NULL

    SF_ASSERT(pafterCh->pPlayPrev || proot->pPlayListHead == pafterCh);
    
    this->pPlayPrev     = pafterCh;
    this->pPlayNext     = pafterCh->pPlayNext;
    pafterCh->pPlayNext = this;
    if (this->pPlayNext)
        this->pPlayNext->pPlayPrev = this;

    proot->CheckPlaylistConsistency();
}

bool InteractiveObject::IsValidOptAdvListMember(MovieImpl* proot) const 
{ 
    return IsOptAdvListFlagSet() && proot->GetOptAdvListMarker() == GetOptAdvListMarker();
}

void InteractiveObject::AddToOptimizedPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    if (IsOptAdvListFlagSet() || proot->IsOptAdvanceListInvalid())
    {
        ClearMarkToRemoveFromOptimizedPlayListFlag();
        return;
    }
    // It is possible that the object is not marked as Unloaded or Unloading but still not in
    // the playlist. This happens from REMOVED_FROM_STAGE event handler, for example 
    // ('unloading' and 'unloaded' flags are cleared for execution of this event handler).
    // Therefore, need to check for IsInPlayList here.
    if (IsUnloaded() || IsUnloading() || !IsInPlayList())
        return;
    //SF_ASSERT(IsInPlayList());
    SF_ASSERT(proot->pPlayListHead == this || pPlayPrev);
    SF_ASSERT(proot->pPlayListOptHead != this && !pPlayNextOpt);

    // find the place in the optimized playlist
    InteractiveObject* pcur = pPlayPrev;
    for (; pcur && !pcur->IsValidOptAdvListMember(proot); pcur = pcur->pPlayPrev)
        ;
    proot->CheckOptPlaylistConsistency(pcur, this);

    if (pcur)
    {
        pPlayNextOpt        = pcur->pPlayNextOpt;
        pPlayPrevOpt        = pcur;
        if (pPlayNextOpt)
            pPlayNextOpt->pPlayPrevOpt = this;
        pcur->pPlayNextOpt  = this;
    }
    else
    {
        pPlayNextOpt            = proot->pPlayListOptHead;
        pPlayPrevOpt            = NULL;
        if (pPlayNextOpt)
            pPlayNextOpt->pPlayPrevOpt = this;
        proot->pPlayListOptHead = this;
    }
    SetOptAdvListFlag();
    SetOptAdvListMarker(proot->GetOptAdvListMarker());
    ClearMarkToRemoveFromOptimizedPlayListFlag();
    proot->CheckOptPlaylistConsistency(this);
}

void InteractiveObject::RemoveFromPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    SF_ASSERT(proot);

    RemoveFromOptimizedPlayList();

    if (pPlayNext)
        pPlayNext->pPlayPrev = pPlayPrev;

    if (pPlayPrev)
        pPlayPrev->pPlayNext = pPlayNext;
    else if (proot->pPlayListHead == this)
        proot->pPlayListHead = pPlayNext;
    
    pPlayNext = pPlayPrev = NULL;
}

void InteractiveObject::RemoveFromOptimizedPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    if (IsOptAdvListFlagSet())
    {
        if (!proot->IsOptAdvanceListInvalid())
        {
            SF_ASSERT(proot);
            if (IsValidOptAdvListMember(proot))
            {
                proot->CheckOptPlaylistConsistency(this);

                if (pPlayPrevOpt)
                    pPlayPrevOpt->pPlayNextOpt = pPlayNextOpt;
                else
                {
                    SF_ASSERT(proot->pPlayListOptHead == this);
                    proot->pPlayListOptHead = pPlayNextOpt;
                }
                if (pPlayNextOpt)
                    pPlayNextOpt->pPlayPrevOpt = pPlayPrevOpt;
            }
        }
        pPlayNextOpt = pPlayPrevOpt = NULL;
        ClearOptAdvListFlag();
        ClearMarkToRemoveFromOptimizedPlayListFlag();
        proot->CheckOptPlaylistConsistency(0, (!proot->IsOptAdvanceListInvalid())?this:0);
    }
}

void InteractiveObject::CloneInternalData(const InteractiveObject* src)
{
    SF_ASSERT(src);
    if (src->pGeomData)
        SetGeomData(*src->pGeomData);
    if (HasAvmObject())
        GetAvmIntObj()->CloneInternalData(src);
}

// Implement mouse-dragging for this pMovie.
void    InteractiveObject::DoMouseDrag(unsigned mouseIndex)
{
    MovieImpl::DragState     st;
    MovieImpl*   proot = GetMovieImpl();
    proot->GetDragState(mouseIndex, &st);

    if (this == st.pCharacter)
    {
        // We're being dragged!
        Render::PointF worldMouse = proot->GetMouseState(mouseIndex)->GetLastPosition();
        Render::PointF parentMouse;
        Matrix  parentWorldMat;
        if (pParent)
            parentWorldMat = pParent->GetWorldMatrix();

        parentWorldMat.TransformByInverse(&parentMouse, worldMouse);
        // if (!st.LockCenter) is not necessary, because then st.CenterDelta == 0.
        parentMouse += st.CenterDelta;

        if (st.Bound)
        {           
            // Clamp mouse coords within a defined rectangle
            parentMouse.x = Alg::Clamp(parentMouse.x, st.BoundLT.x, st.BoundRB.x);
            parentMouse.y = Alg::Clamp(parentMouse.y, st.BoundLT.y, st.BoundRB.y);
        }

        // Once touched, object is no longer animated by the timeline
        SetAcceptAnimMoves(0);

        // Place our origin so that it coincides with the mouse coords
        // in our parent frame.
        SetX(TwipsToPixels(Double(parentMouse.x)));
        SetY(TwipsToPixels(Double(parentMouse.y)));

        //Matrix    local = GetMatrix();
        //local.Tx() = parentMouse.x;
        //local.Ty() = parentMouse.y;
        //SetMatrix(local);

    }
}

void    InteractiveObject::CopyPhysicalProperties(const InteractiveObject *poldChar)
{
    // Copy physical properties, used by loadMovie().
    SetDepth(poldChar->GetDepth());
    SetCxform(poldChar->GetCxform());
    if (Is3D())
        SetMatrix3D(poldChar->GetMatrix3D());
    else
        SetMatrix(poldChar->GetMatrix());
    if (poldChar->pGeomData)
        SetGeomData(*poldChar->pGeomData);

    const FilterSet* filters = poldChar->GetFilters();
    if (filters)
        SetFilters(filters);

    if (HasAvmObject())
        GetAvmIntObj()->CopyPhysicalProperties(poldChar);
}

void    InteractiveObject::MoveNameHandle(InteractiveObject *poldChar)
{
    // Re-link all ActionScript references.
    pNameHandle = poldChar->pNameHandle;
    poldChar->pNameHandle = 0;
    if (pNameHandle)
        pNameHandle->pCharacter = this;
}

bool InteractiveObject::IsFocusRectEnabled() const
{
    if (IsFocusRectFlagDefined())
        return IsFocusRectFlagTrue();
    //!AB: _focusrect seems to ignore lockroot. that is why we specify "true" as a parameter
    // of GetASRootMovie.
    InteractiveObject* prootMovie = GetTopParent(true);
    if (prootMovie && prootMovie != this)
        return prootMovie->IsFocusRectEnabled();
    return true;
}

void InteractiveObject::OnFocus(FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
                                unsigned controllerIdx, FocusMovedType fmt)
{
    if (HasAvmObject())
        GetAvmIntObj()->OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
}

bool InteractiveObject::IsTabable() const
{
    if (HasAvmObject())
        return GetAvmIntObj()->IsTabable();
    return false;
}

ASStringManager* InteractiveObject::GetStringManager() const
{
    return GetMovieImpl()->GetStringManager();
}

unsigned InteractiveObject::GetCursorType() const
{
    if (HasAvmObject())
        return GetAvmIntObj()->GetCursorType();
    return MouseCursorEvent::ARROW;
}


// int InteractiveObject::GetAVMVersion() const
// {
//     return (pDefImpl->GetFileAttributes() & GFxMovieDef::FileAttr_UseActionScript3) ? 2 : 1;
// }

// Remove the object with the specified name.
void  InteractiveObject::RemoveDisplayObject()
{
    if (!GetParent())
        return;
    DisplayObjContainer* pparent = GetParent()->CharToDisplayObjContainer();
    if (!pparent)
        return; 
    pparent->RemoveDisplayObject(GetDepth(), GetId());
}


UInt16 InteractiveObject::GetFocusGroupMask() const
{
    if (FocusGroupMask == 0)
        return GetParent()->GetFocusGroupMask();
    return FocusGroupMask;
}

UInt16 InteractiveObject::GetFocusGroupMask()
{
    if (FocusGroupMask == 0 && GetParent())
        FocusGroupMask = GetParent()->GetFocusGroupMask();
    return FocusGroupMask;
}

bool   InteractiveObject::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const
{
    unsigned focusIdx = proot->GetFocusGroupIndex(controllerIdx);
    return (GetFocusGroupMask() & (1 << focusIdx)) != 0;
}

bool   InteractiveObject::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx)
{
    unsigned focusIdx = proot->GetFocusGroupIndex(controllerIdx);
    return (GetFocusGroupMask() & (1 << focusIdx)) != 0;
}

bool   InteractiveObject::IsFocusEnabled(FocusMovedType fmt) const 
{ 
    if (HasAvmObject())
        return GetAvmIntObj()->IsFocusEnabled(fmt);
    return true; 
}

}} // Scaleform::GFx
