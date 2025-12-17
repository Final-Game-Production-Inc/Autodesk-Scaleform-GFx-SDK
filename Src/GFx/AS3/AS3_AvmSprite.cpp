/**************************************************************************

Filename    :   AS3_AvmSprite.cpp
Content     :   Implementation of AS3 part of Sprite (MovieClip)
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_MovieClip.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

// SF_INLINE MovieRoot* AvmSprite::GetAS2Root() const
// { 
//     //if (!pSprite) return NULL;
//     SF_ASSERT(pDispObj);
//     return static_cast<MovieRoot*>(pDispObj->GetASMovieRoot()); 
// }



AvmSprite::AvmSprite(Sprite* psprite)
: 
    AvmDisplayObjContainer(psprite),
    InitActionsExecuted(GetDef()->GetFrameCount()),
    Flags(0)
{
    // By default LockRoot is false.
    GetSprite()->SetLockRoot(false);
}

AvmSprite::~AvmSprite()
{
//    SF_FREE(pFrameDescs);
}

bool AvmSprite::IsLevelMovie() const
{
    return GetParent() == NULL;
}

void AvmSprite::ForceShutdown()
{
    AvmDisplayObjContainer::ForceShutdown();
    if (HasAS3Obj())
    {
        GetAS3Obj()->SetLoaderInfo(NULL);
    }
}

void AvmSprite::CloneInternalData(const InteractiveObject* src)
{
    AvmInteractiveObj::CloneInternalData(src);
}

bool    AvmSprite::MustBeInPlaylist() const
{
    return IsNeedExecuteFrameFlagSet() || HasEventHandler(EventId::Event_EnterFrame);
}

void    AvmSprite::CreateChildren()
{
    if (!IsChildrenCreatedFlagSet())
    {
        GFx::Sprite* spr = GetSprite();
        //printf("??? CreateChildren %s\n", spr->GetName().ToCStr()); //@DBG
        //@TODO, @TEMP: need to revise part "spr->GetParent() && spr->GetParent()->IsInPlayList()" later
        // when unattached sprites will work correctly.
        if (!spr->IsInPlayList() && spr->GetParent() && spr->GetParent()->IsInPlayList())
        {
            if (spr->GetParent())
                GetAvmParent()->InsertChildToPlayList(spr);
            else
                spr->AddToPlayList();
            spr->ModifyOptimizedPlayList();
        }

        if (!spr->IsOnEventLoadCalled())
            spr->DefaultOnEventLoad();
        spr->ExecuteInitActionFrameTags(0);

        SetChildrenCreatedFlag();
    }
}

void    AvmSprite::OnEventLoad()
{
    AvmInteractiveObj::OnEventLoad();
}

void    AvmSprite::OnEventUnload()
{
    AvmInteractiveObj::OnEventUnload();
    
    // now, wipe out the character, remove the name, release all local variables 
    // from environment...
    // This is necessary, if there are still pointers on this character somewhere.
    // For example, if the AS code was not terminated (see GASEnv::NeedTermination).
    // Actually, GFx does not release all local variables, and it doesn't wipe out
    // all members at the moment. TODO(?).

    //!AB: actually, we can't reset name here, because if another sprite with the same name
    // will appear after on then it may reuse the name handle and the name should be correct.
    //if (pNameHandle)
    //    pNameHandle->ResetName(ASEnvironment.GetSC());
}


bool AvmSprite::OnUnloading(bool mayRemove)
{
    return AvmDisplayObjContainer::OnUnloading(mayRemove);
}

// Add an object to the display list.
DisplayObjectBase*   AvmSprite::AddDisplayObject(
    const CharPosInfo &pos, 
    const ASString& name,
    const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* ,
    const void *,
    unsigned createFrame,
    UInt32 addFlags,
    CharacterCreateInfo* pcharCreateOverride,
    InteractiveObject* origChar)
{
    //MovieDef* pdef = GetDef();
    MovieDefImpl* pdefImpl = GetDefImpl();
    //SF_ASSERT(pdef);
    SF_ASSERT(GetDispObj());

    Sprite* thisSpr = GetSprite();
    bool placeObject = (addFlags & DisplayList::Flags_PlaceObject) != 0;
    bool replaceIfDepthIsOccupied = (addFlags & DisplayList::Flags_ReplaceIfDepthIsOccupied) != 0;

    // TBD:
    // Here we look up a character but do not return it's binding.
    // This means that while we will have the right character created,
    // its pdefImpl will correspond OUR table and not THEIR table.
    // However, imported characters MUST reference their own tables for resources!
    CharacterDef::CharacterDefType type = CharacterDef::Unknown;
    CharacterCreateInfo ccinfo = pcharCreateOverride ? *pcharCreateOverride :    
        pdefImpl->GetCharacterCreateInfo(pos.CharacterId);
    if (!ccinfo.pCharDef)
    {
        // check if we have a classname; if we do - try to get createInfo using it as export name.
        if (pos.HasClassName())
        {
            ResourceBindData resBindData;
            if (GetMovieImpl()->FindExportedResource(pdefImpl, &resBindData, pos.ClassName))
            {
                if (resBindData.pResource->GetResourceType() & Resource::RT_CharacterDef_Bit) 
                    ccinfo.pCharDef     = (CharacterDef*) resBindData.pResource.GetPtr();
                else
                    ccinfo.pResource = resBindData.pResource;
                ccinfo.pBindDefImpl = resBindData.pBinding->GetOwnerDefImpl();
            }
        }

        if (!ccinfo.pCharDef)
        {
            // special case - bitmap. In AS3 bitmap (ImageResource) can be placed on the stage
            // as well.
            if (ccinfo.pResource && ccinfo.pResource->GetResourceType() == Resource::RT_Image)
            {
                type = CharacterDef::Bitmap; // forces to create a "Bitmap" character
                if (!ccinfo.pBindDefImpl)
                    ccinfo.pBindDefImpl = pdefImpl;//?
            }
            else
            {
                GetDispObj()->LogError("Sprite::AddDisplayObject(): unknown cid = %d\n", pos.CharacterId.GetIdIndex());
                return NULL;
            }
        }
    }

    // If we already have this object on this plane, then move it instead of replacing it.
    // This can happen when wrapping around from last frame to first one, where
    // the first frame fill bring back to life objects marked for removal after last frame.
    // It can also happen when seeking back.
    bool markedForRemove = false;
    DisplayObjectBase* pexistingChar = thisSpr->GetCharacterAtDepth(pos.Depth, &markedForRemove);

    unsigned frame = ((createFrame == SF_MAX_UINT) ? thisSpr->GetCurrentFrame() : createFrame);
    if (placeObject)
    {
        // If the object was moved explicitly in Action script, leave it there.
        bool animatedByTimeline = true;
        if (pexistingChar)
        {
            animatedByTimeline = pexistingChar->GetAcceptAnimMoves();
            if (!animatedByTimeline && pexistingChar->GetContinueAnimationFlag())
            {
                pexistingChar->SetAcceptAnimMoves(true);// reset the flag
                animatedByTimeline = true;
            }
        }
        // If the object was moved explicitly in Action script, leave it there.
        if (!markedForRemove && pexistingChar && !pexistingChar->GetAcceptAnimMoves())
            return NULL;        

        if ( pexistingChar && !pexistingChar->IsUnloadQueuedUp() &&
            (pexistingChar->GetId() == pos.CharacterId))
        {
            // We need to use OriginalName here to determine is it the same character
            // or a different one. We can't use the regular name (GetName() method), since
            // it could be changed via ActionScript (_name property) and this shouldn't
            // affect the timeline execution (see test_name_change1.swf)
            ASString originalName(GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_));
            if (pexistingChar->IsInteractiveObject())
                originalName = pexistingChar->CharToInteractiveObject_Unsafe()->GetOriginalName();

            // The name can be set only for scriptable characters, such as Sprite, Button and TextField.
            // Though, theoretically it is possible that PlaceObject for generic character contains
            // a name. Technically, this is incorrect, but possible (see wizardy.swf).
            // In this case, we need to skip names comparison. We can use pexistingChar->IsASCharacter()
            // criteria, since Ids are equal.

            if (!pexistingChar->IsInteractiveObject() || 
                (name.IsEmpty() && originalName.IsEmpty()) ||
                (name.IsEmpty() && pexistingChar->CharToInteractiveObject_Unsafe()->HasInstanceBasedName()) ||
                (!name.IsEmpty() && originalName == name))
            {
                // Move will bring the object back by clearing its MarkForRemove flag
                // in display list. We only do so if we are talking about THE SAME object,
                // i.e. created in the same frame. Flash considers objects created in
                // different frames to be different, even if they share a name.
                if (pexistingChar->GetCreateFrame() == frame)
                {
                    CharPosInfo newPos = pos;
                    if (!pos.HasBlendMode() && pexistingChar->GetBlendMode() != Render::Blend_None)
                    {
                        newPos.SetBlendModeFlag();
                        newPos.BlendMode = Render::Blend_None;
                    }
                    if (!pos.HasCxform() && !pexistingChar->GetCxform().IsIdentity())
                    {
                        // if the object is moved by AddDisplayObject and no color transform suppose to 
                        // to be used then we need to reset the color transform for the existing
                        // character, since it may have it changed earlier. 
                        // This is actual when the character has changed Cxform matrix and
                        // then time-line goes to the first frame, where this Cxform doesn't
                        // exist. (!AB)
                        //                    CharPosInfo newPos = pos;
                        newPos.SetCxFormFlag();
                        newPos.ColorTransform.SetIdentity();
                    }

                    thisSpr->MoveDisplayObject(newPos);  

                    // we reuse previously existing sprite, but very likely the ActionScript property
                    // is already set to null or other value due to executesnapshot activity. So, we
                    // need to reset the property here. @TODO: revise logic in AvmDisplayObj::OnEventLoad
                    // and here.
                    if (pexistingChar->IsScriptableObject())
                    {
                        DisplayObject* existObj = pexistingChar->CharToScriptableObject_Unsafe();
                        AvmDisplayObj* avmEexistObj = ToAvmDisplayObj(existObj);
                        AS3::Object* as3existObjParent = avmEexistObj->GetAS3Parent();
                        if (!existObj->IsInstanceBasedNameFlagSet() && as3existObjParent)
                        {
                            const ASString& nameString = existObj->GetName();
                            // !!! SetProperty() can throw exceptions. Exception checking is disabled by the DoNotCheck() call.
                            as3existObjParent->SetProperty(Multiname(GetAS3Root()->GetAVM()->GetPublicNamespace(), nameString),
                                Value(avmEexistObj->GetAS3Obj())).DoNotCheck();
                        }
                    }

                    // if character was marked for remove the
                    return NULL;
                }
                // Otherwise replace object with a new-state fresh instance of it.
                replaceIfDepthIsOccupied = 1;
            }
        }
    }
    else
    {
        replaceIfDepthIsOccupied = 1;
    }
    thisSpr->SetDirtyFlag();

    Ptr<DisplayObjectBase> ch = *GetAS3Root()->GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), ccinfo, thisSpr, pos.CharacterId, type);
    SF_ASSERT(ch);
    InteractiveObject*  intObj   = ch->CharToInteractiveObject();
    bool                nameSet     = 0;

    ch->SetScale9GridExists(false);
    const InteractiveObject* parent = ch->GetParent();
    while (parent)
    {
        if (parent->HasScale9Grid())
        {
            ch->SetScale9GridExists(true);
            ch->PropagateScale9GridExists();
            break;
        }
        parent = parent->GetParent();
    }

    if (intObj)
    {
        if (!name.IsEmpty())
        {
            intObj->SetOriginalName(name);          
            nameSet = 1;         
        }

        if (origChar)
        {
            // if we are duplicating char - call CloneInternalData
            intObj->CloneInternalData(origChar);
        }

    }

    // in the case if AddDisplayObject is not initiated through timeline
    // (Flag_PlaceObject is NOT set) we need to mark the newly created
    // object by SetAcceptAnimMovies(false).
    if (!(addFlags & DisplayList::Flags_PlaceObject))
        ch->CharToScriptableObject_Unsafe()->SetAcceptAnimMoves(false);
    else
        ch->CharToScriptableObject_Unsafe()->SetTimelineObjectFlag(true);

    ch->SetCreateFrame(frame);

    addFlags &= ~DisplayList::Flags_ReplaceIfDepthIsOccupied;
    if (replaceIfDepthIsOccupied)
        addFlags |= DisplayList::Flags_ReplaceIfDepthIsOccupied;

    // set class name if it exists
    if (pos.HasClassName())
    {
         ToAvmDisplayObj(ch->CharToScriptableObject_Unsafe())->InitClassName(pos.ClassName);
    }

    thisSpr->GetDisplayList().AddDisplayObject(GetSprite(), pos, ch.GetPtr(), addFlags);   

    if (intObj)
    {
        // AS 3.0 adds this sprite into a playlist in display order
//?        InsertChildToPlayList(intObj);
        //?        intObj->ModifyOptimizedPlayList();
    }

    AvmDisplayObj* avmObj = ToAvmDisplayObj(ch->CharToScriptableObject_Unsafe());
    avmObj->SetAppDomain(*AppDomain);
    if (avmObj->CreateASInstanceNoCtor()) // create AS3 instance
    {
        SF_ASSERT(avmObj->GetAS3Obj());
        if (!ch->CharToScriptableObject_Unsafe()->IsInstanceBasedNameFlagSet() && avmObj->GetAS3Parent())
        {
            // init name property before calling ctor
            GetAS3Obj()->SetProperty(Multiname(GetAS3Root()->GetAVM()->GetPublicNamespace(), name), Value(avmObj->GetAS3Obj())).DoNotCheck();
            if (GetAS3Root()->GetAVM()->IsException())
            {
                GetAS3Root()->GetAVM()->OutputAndIgnoreException();
                pDispObj->SetExecutionAborted();
            }
        }
        // call the ctor
        avmObj->CallCtor(true);
        // check if children were created (they should be created from AS3::Sprite::AS3Constructor)
        if (ch->IsSprite())
        {
            ToAvmSprite(ch->CharToSprite_Unsafe())->CreateChildren();
        }
    }

    // Assign sticky variables (only applies if there is an identifiable name).
    if (nameSet)
        GetAS3Root()->ResolveStickyVariables(intObj);

    SF_ASSERT(!ch || ch->GetRefCount() > 1);

    if (intObj && nameSet)
    {
        // check if the name is "hitArea" and assign the hitArea if it is so.
        if (name == GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_hitArea))
            thisSpr->SetHitArea(static_cast<Sprite*>(intObj));
    }
    //mDisplayList.CheckConsistency(thisSpr);

    if (intObj)
    {
        // Fire ADDED_TO_STAGE event here (if InteractiveObject)
        ToAvmInteractiveObj(intObj)->OnAdded(true);
    }

    return ch.GetPtr();
}

bool    AvmSprite::HasFrameScript() const
{
    if (GetAS3Obj())
        return static_cast<Instances::fl_display::Sprite*>(GetAS3Obj())->HasFrameScript();
    return false;
}

void    AvmSprite::QueueFrameScript(unsigned frame)
{
    Value method;
    if (GetFrameScript(frame, &method))
    {
        GetAS3Root()->InsertEmptyAction(MovieRoot::AL_Frame)->SetAction(
            static_cast<InteractiveObject*>(pDispObj), 
            method);
    }
}

bool    AvmSprite::GetFrameScript(unsigned frame, Value* pmethod)
{
//     // check, if we need to execute ("queue up") AS execution for the frame
//     const FrameScript::Descr* pdescr = mFrameScript.Get(frame);
//     if (pdescr)
//     {
//         SF_ASSERT(pmethod);
//         *pmethod = pdescr->Method;
//         return true;
//     }
    if (GetAS3Obj())
        return static_cast<Instances::fl_display::Sprite*>(GetAS3Obj())->GetFrameScript(frame, pmethod);
    return false;
}

void    AvmSprite::ExecuteFrameTags(unsigned frame)
{
    SF_UNUSED(frame);
    SetNeedExecuteFrameFlag();
    GetSprite()->SetJustLoaded(false);
    //QueueFrameScript(frame);
    //SF_DEBUG_MESSAGE2(1, "FRAME %d   %s\n", frame, GetName().ToCStr());
}

void    AvmSprite::AdvanceFrame(bool nextFrame, float framePos)
{
    SF_UNUSED(framePos);
    Ptr<Sprite> spr = GetSprite();
    if (spr->IsAdvanceDisabled() || spr->IsUnloading())
        return;

    SF_ASSERT(GetDef() && GetMovieImpl() != NULL);

    // If flag "need-execute-frame" is already set then do not advance the sprite:
    // this means that current frame is already set correctly and ActionScript
    // for the frame is already queued up (by mouse/button/timer/etc events, 
    // for example).
    if (nextFrame && !IsNeedExecuteFrameFlagSet())
    {   
        // remove all unloaded
        unsigned    currentFrame = spr->GetCurrentFrame();

        // Update current and next frames.
        if (spr->GetPlayState() == State_Playing)
        {
            unsigned    currentFrame0 = currentFrame;
            spr->IncrementFrameAndCheckForLoop();
            currentFrame = spr->GetCurrentFrame();

            //@DBG
//             String s;
//             GetAbsolutePath(&s);
//             printf("!!! %s currframe %d\n", s.ToCStr(), currentFrame);

            // Execute the current frame's tags.
            if (currentFrame != currentFrame0)
            {

#if 0
                // Shape dump logic for external debugging.
                FILE* fd = fopen("shapes", "at");
                fprintf(fd, "Frame %d\n", currentFrame);
                fclose(fd);
#endif 
                // Init action tags must come before Event_EnterFrame
                ExecuteInitActionFrameTags(currentFrame);

                // Post OnEnterFrame event, so that it arrives before other
                // frame actions generated by frame tags.
                //OnEvent(EventId::Event_EnterFrame);

                spr->ExecuteFrameTags(currentFrame);
                SetNeedExecuteFrameFlag();
                //QueueFrameScript(currentFrame);
            }
        }

        // Clean up display List (remove dead objects). Such objects
        // can only exist if we looped around to frame 0.
        if (currentFrame == 0)
            GetDisplayList().UnloadMarkedObjects(GetSprite());

        GetDisplayList().CheckConsistency(GetSprite());
    }

#ifdef GFX_TRACE_DIPLAYLIST_EVERY_FRAME
    if (spr->IsLevelMovie()) 
        spr->DumpDisplayList(0, "");
#endif
}

// Dispatch event Handler(s), if any.
bool    AvmSprite::OnEvent(const EventId& id)
{
    if (IsButtonMode())
    {
        // Check if the sprite has special frames, such as _up, _down, _over.
        // Go to these frames if event is Release, Press or RollOver.
        SpriteDef* sp;
        if ((sp = GetSpriteDef()) != NULL)
        {
            if (sp->HasSpecialFrames())
            {
                switch(id.Id)
                {
                case EventId::Event_MouseUp:
                case EventId::Event_Release:
                    if (sp->HasFrame_over())
                        GetSprite()->GotoLabeledFrame("_over");
                    break;
                case EventId::Event_RollOver:
                    if (sp->HasFrame_over())
                    {
                        GFx::MouseState& ms = *GetMovieImpl()->GetMouseState(id.MouseIndex);
                        SF_ASSERT(&ms);
                        if (GetIntObj()->IsTrackAsMenuFlagSet() && 
                            (ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left))
                            GetSprite()->GotoLabeledFrame("_down");
                        // prevent button from changing state if left mouse button is pressed (like in Flash)
                        else if (!(ms.GetButtonsState() & GFx::MouseState::MouseButton_Left))
                            GetSprite()->GotoLabeledFrame("_over");
                    }
                    break;
                case EventId::Event_DragOut:
                    if (GetIntObj()->IsTrackAsMenuFlagSet())
                    {
                        if (sp->HasFrame_up())
                            GetSprite()->GotoLabeledFrame("_up");
                    }
                    else
                    {
                        if (sp->HasFrame_over())
                            GetSprite()->GotoLabeledFrame("_over");
                    }
                    break;
                case EventId::Event_MouseDown:
                case EventId::Event_DragOver:
                    if (sp->HasFrame_down())
                        GetSprite()->GotoLabeledFrame("_down");
                    break;
                case EventId::Event_RollOut:
                case EventId::Event_ReleaseOutside:
                    if (sp->HasFrame_up())
                        GetSprite()->GotoLabeledFrame("_up");
                    break;
                default:;
                }
            }
        }
        if (id.Id == EventId::Event_Press)
        {
            if (HasAS3Obj())
            {
                //simulate Click event
                EventId e(id);
                e.Id = EventId::Event_Click;
                GetAS3Obj()->Dispatch(e, pDispObj);
            }
            return true;
        }
    }

    return AvmDisplayObjContainer::OnEvent(id);
}

bool AvmSprite::ActsAsButton() const    
{ 
    if (IsButtonMode())
        return true;
    if (!HasAS3Obj())
        return false;
    return AvmInteractiveObj::ActsAsButton();
}

bool     AvmSprite::IsFocusEnabled(FocusMovedType fmt) const
{
    switch (fmt)
    {
    case GFx_FocusMovedByAS:    return true;
    case GFx_FocusMovedByMouse: return IsButtonMode() || IsTabable();
    default: ; 
    }
    return (ActsAsButton());
}

unsigned AvmSprite::GetCursorType() const
{
    if (IsButtonMode())
    {
        return MouseCursorEvent::HAND;
    }
    return AvmInteractiveObj::GetCursorType();
}

void AvmSprite::ExecuteFrame0Events()
{
}

void AvmSprite::ExecuteInitActionFrameTags(unsigned frame)
{
    SF_ASSERT(pDispObj && pDispObj->IsSprite());

    // Execute this frame's init actions, if necessary.
    if (!InitActionsExecuted[frame])
    {
        // Keep this (particularly GASEnvironment) alive during execution!
        Ptr<DisplayObject> thisPtr(pDispObj);

        bool markFrameAsVisited = false;

        TimelineDef::Frame initActionsFrame;        

        if (GetDef()->GetInitActions(&initActionsFrame, frame) &&
            initActionsFrame.GetTagCount() > 0)
        {
            // Need to execute these actions.
            for (unsigned i= 0; i < initActionsFrame.GetTagCount(); i++)
            {
                ExecuteTag*  e = initActionsFrame.GetTag(i);
#ifdef GFX_TRACE_TIMELINE_EXECUTION
                printf("\ninit action execution, fr = %d, %s\n", frame, pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());
                e->Trace("");
#endif
                e->Execute(GetSprite());
            }
            markFrameAsVisited = true;
        }
        if (markFrameAsVisited)
        {
            // Mark this frame done, so we never execute these init actions again.
            InitActionsExecuted.Set(frame);
        }
    }
}

bool AvmSprite::IsTabable() const 
{ 
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) 
        return false;
    if (GetIntObj()->IsTabEnabledFlagDefined())
    {
        if (GetIntObj()->IsTabEnabledFlagTrue())
        {
            // check if any of the parents has tabChildren = false
            InteractiveObject* c = GetIntObj()->GetParent();
            while(c)
            {
                if (c->IsTabChildrenDisabledFlagSet())
                    return false;
                c = c->GetParent();
            }
            return true;
        }
        return false;
    }
    else if (GetIntObj()->GetTabIndex() > 0)
        return true;
    return IsButtonMode();
}

bool AvmSprite::GetObjectsUnderPoint(
    ArrayDH<Ptr<DisplayObjectBase> >* destArray, const PointF& pt) const
{
    bool rv = AvmDisplayObjContainer::GetObjectsUnderPoint(destArray, pt);
    if (!rv && HasDrawingContext())
    {
        rv = GetDrawingContext()->DefPointTestLocal(pt, DisplayObjectBase::HitTest_TestShape, pDispObj);
        if (rv)
        {
            // PPS: A push front op matches Flash player behavior; requires optimization
            //destArray->PushBack(pDispObj);
            destArray->InsertAt(0, pDispObj);
        }
    }
    return rv;
}

//////////////////////////////////////////////////////////////////////////
AvmMovieClip::~AvmMovieClip()
{
}

}}} // SF::GFx::AS3


