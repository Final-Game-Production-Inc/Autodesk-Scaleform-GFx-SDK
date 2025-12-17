/**************************************************************************

Filename    :   AS3_AvmInteractiveObj.cpp
Content     :   Implementation of AS3-dependent part of InteractiveObject.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_InteractiveObject.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_FocusEvent.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {


// Constructor.
AvmInteractiveObj::AvmInteractiveObj(InteractiveObject* pdispObj) 
:
    AvmDisplayObj(pdispObj),
    MouseOverCnt(0),
    Flags(0)
{
	pdispObj->SetTabIndex(-1);
}

AvmInteractiveObj::~AvmInteractiveObj()
{
}


void AvmInteractiveObj::OnEventLoad()
{
    AvmDisplayObj::OnEventLoad();
}

void AvmInteractiveObj::OnEventUnload()
{
    AvmDisplayObj::OnEventUnload();
}

bool AvmInteractiveObj::OnEvent(const EventId& id)
{
    switch (id.Id)
    {
    case EventId::Event_Click:
    case EventId::Event_DoubleClick:
    case EventId::Event_MouseOver:
    case EventId::Event_MouseOut:
    case EventId::Event_RollOver:
    case EventId::Event_RollOut:
    case EventId::Event_MouseMove:
    case EventId::Event_MouseDown:
    case EventId::Event_MouseWheel:
    case EventId::Event_MouseUp:
        if (HasAS3Obj())
        {
            // notify listeners
            GetAS3Obj()->Dispatch(id, GetIntObj());
        }
        break;
    case EventId::Event_Resize:
        GetAS3Root()->InsertEmptyAction(MovieRoot::AL_ControllerEvents)->SetAction(pDispObj, id);
        break;

    default:
        {
            //printf("%X\n",id.Id);
            return AvmDisplayObj::OnEvent(id);
        }
    }
    return true;
}

void AvmInteractiveObj::CloneInternalData(const InteractiveObject*)
{
}

void    AvmInteractiveObj::CopyPhysicalProperties(const InteractiveObject *)
{
}

void AvmInteractiveObj::OnFocus(InteractiveObject::FocusEventType evt, 
                                InteractiveObject* oldOrNewFocusCh, 
                                unsigned controllerIdx,
                                FocusMovedType)
{
    if (HasAS3Obj())
    {
        SF_UNUSED(controllerIdx);
        const char* focusName = (evt == InteractiveObject::KillFocus) ? "focusOut" : "focusIn";

        //!AB: can the interactive obj have null AS3 obj?
        // SF_ASSERT(!oldOrNewFocusCh || ToAvmInteractiveObj(oldOrNewFocusCh)->GetAS3ObjSafe());
        Instances::fl_display::DisplayObject* relatedObj = (oldOrNewFocusCh) ? 
            ToAvmInteractiveObj(oldOrNewFocusCh)->GetAS3ObjSafe() : NULL;

        Instances::fl_display::DisplayObject* as3Obj = GetAS3ObjSafe();
        SPtr<Instances::fl_events::FocusEvent> focusEvt = 
            as3Obj->CreateFocusEventObject(GetMovieImpl()->GetStringManager()->CreateConstString(focusName), 
            relatedObj, controllerIdx);
        as3Obj->Dispatch(focusEvt.GetPtr(), pDispObj);
    }
}

bool AvmInteractiveObj::OnFocusChange
    (InteractiveObject* toBeFocused, unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* pfocusKeyInfo)
{
    SF_ASSERT(fmt != GFx_FocusMovedByKeyboard || pfocusKeyInfo);
    const char* focusName = (fmt == GFx_FocusMovedByKeyboard) ? "keyFocusChange" : "mouseFocusChange";
    
    if (HasAS3Obj())
    {
        SF_UNUSED(controllerIdx);
        Instances::fl_display::DisplayObject* relatedObj = (toBeFocused) ? 
            ToAvmInteractiveObj(toBeFocused)->GetAS3ObjSafe() : NULL;

        Instances::fl_display::DisplayObject* as3Obj = GetAS3ObjSafe();
        SPtr<Instances::fl_events::FocusEvent> focusEvt = 
            as3Obj->CreateFocusEventObject(GetMovieImpl()->GetStringManager()->CreateConstString(focusName), 
            relatedObj, controllerIdx);

        focusEvt->Cancelable = true;
        if (fmt == GFx_FocusMovedByKeyboard)
        {
            focusEvt->KeyCode = pfocusKeyInfo->KeyCode;
            focusEvt->ShiftKey = KeyModifiers(pfocusKeyInfo->KeysState).IsShiftPressed();
        }
        as3Obj->Dispatch(focusEvt.GetPtr(), pDispObj);
        return !focusEvt->IsDefaultPrevented();
    }
    return true;
}

InteractiveObject* AvmInteractiveObj::GetTopParent(bool ignoreLockRoot) const
{
    InteractiveObject* pparent = GetParent();
    if (!pparent || !pparent->IsAVM2())
        return static_cast<InteractiveObject*>(pDispObj); // already top parent
    return pparent->GetTopParent(ignoreLockRoot);
}

AvmInteractiveObj* AvmInteractiveObj::GetAvmTopParent(bool ignoreLockRoot) const
{
    if (!GetParent()->IsAVM2())
        return NULL;

    return static_cast<AvmInteractiveObj*>(
        GetParent()->GetTopParent(ignoreLockRoot)->GetAvmIntObj());
}

bool    AvmInteractiveObj::IsTabable() const
{
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) 
        return false;
    if (GetIntObj()->IsTabEnabledFlagDefined())
        return GetIntObj()->IsTabEnabledFlagTrue();
    else if (GetIntObj()->GetTabIndex() > 0)
        return true;
    return false;
}

bool AvmInteractiveObj::ActsAsButton() const    
{ 
    if (HasAS3Obj())
    {
        return GetAS3Obj()->HasButtonHandlers();
    }
    return true;//false;
}

InteractiveObject* AvmInteractiveObj::FindInsertToPlayList(InteractiveObject* ch)
{
    // In the case when this object is already unloaded or is unloading the obj is already excluded
    // from the playlist.
    if (GetIntObj()->IsUnloaded() || GetIntObj()->IsUnloading())
        return NULL;

    InteractiveObject* parent = ch->GetParent();
    if (parent && parent->pPlayPrev)
    {
        // insert before the parent
        //ch->InsertToPlayListAfter(parent->pPlayPrev);
        return parent->pPlayPrev;
    }
    else
        //ch->AddToPlayList();
        return NULL;
}

void AvmInteractiveObj::MoveBranchInPlayList()
{
    InteractiveObject* p = GetIntObj();
    //SF_ASSERT(p->IsInPlayList());
    InteractiveObject* pend = p;

    // save the previous in the playlist
    InteractiveObject* pbefore = p->pPlayNext;
    InteractiveObject* pafter = NULL;

    // now look for the last item in the branch
    InteractiveObject* pcur = p->pPlayPrev;
    InteractiveObject* parpar = p;
    for (; pcur ; pcur = pcur->pPlayPrev)
    {
        // check the parent chain.
        InteractiveObject* c = pcur;
        for (; c; c = c->GetParent())
        {
            if (c == parpar)
            {
                // still have the parent...
                break;
            }
        }
        if (!c)
        {
            // there is no parent in the parent chain!
            pafter = pcur;
            break;
        }
    }
    // connect pbefore->pPlayPrev and pafter->pPlayNext, thus excluding the branch
    // from the playlist....
    if (pafter)
    {
        pend   = pafter->pPlayNext;
        pafter->pPlayNext = pbefore;
    }
    else
    {
        if (pbefore)
        {
            pend = GetMovieImpl()->pPlayListHead;
            GetMovieImpl()->pPlayListHead = pbefore;
        }
        else
            pend = p;
    }
    if (pbefore)
        pbefore->pPlayPrev  = pafter;

    SF_ASSERT(p && pend);
    p->pPlayNext    = NULL;
    pend->pPlayPrev = NULL;

    // now need to insert the branch [pend->p] into the playlist at new location
    InteractiveObject* parent = GetParent();
    if (parent)
    {
        InteractiveObject* pafterObj = ToAvmInteractiveObj(parent)->FindInsertToPlayList(p);
        if (pafterObj)
        {
            p->pPlayNext      = pafterObj->pPlayNext;
            if (p->pPlayNext)
                p->pPlayNext->pPlayPrev = p;

            pafterObj->pPlayNext = pend;
            pend->pPlayPrev   = pafterObj;
        }
        else
            parent = NULL;
    }
    if (!parent)
    {
        // no parent, insert at the head of playlist
        InteractiveObject* phead = GetMovieImpl()->pPlayListHead;
        if (phead)
        {
            p->pPlayNext = phead;
            phead->pPlayPrev = p;
        }
        GetMovieImpl()->pPlayListHead = pend;
    }
    GetMovieImpl()->CheckPlaylistConsistency();
    GetMovieImpl()->InvalidateOptAdvanceList();
}

void AvmInteractiveObj::InsertChildToPlayList(InteractiveObject* ch)
{
    SF_ASSERT(!ch->IsDisplayObjContainer() || ch->CharToDisplayObjContainer_Unsafe()->GetNumChildren() == 0);
    InteractiveObject* pafter = FindInsertToPlayList(ch);
    if (pafter)
        ch->InsertToPlayListAfter(pafter);
    else
        ch->AddToPlayList();
}

}}} // SF::GFx::AS3


