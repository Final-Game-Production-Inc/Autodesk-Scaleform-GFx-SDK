/**************************************************************************

Filename    :   AS2_AvmSprite.cpp
Content     :   Implementation of AS2 part of Sprite (MovieClip)
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "GFx/AS2/AS2_TransformObject.h"
#include "GFx/AS2/AS2_BitmapData.h"
#include "GFx/AS2/AS2_TextSnapshot.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "Kernel/SF_MsgFormat.h"

// Version numbers for $version and getVersion()
// TBD: Perhaps these should be moved into a different header?
#if defined(SF_OS_WIN32)
#define GFX_FLASH_VERSION "WIN 8,0,0,0"
#elif defined(SF_OS_MAC)
#define GFX_FLASH_VERSION "MAC 8,0,0,0"
#elif defined(SF_OS_LINUX)
#define GFX_FLASH_VERSION "LINUX 8,0,0,0"
#elif defined(SF_OS_XBOX360)
#define GFX_FLASH_VERSION "XBOX360 8,0,0,0"
#elif defined(SF_OS_PS3)
#define GFX_FLASH_VERSION "PS3 8,0,0,0"
#elif defined(SF_OS_WII)
#define GFX_FLASH_VERSION "WII 8,0,0,0"
#elif defined(SF_OS_3DS)
#define GFX_FLASH_VERSION "3DS 8,0,0,0"
#else
#define GFX_FLASH_VERSION "OTHER 8,0,0,0"
#endif

namespace Scaleform {
namespace GFx {
namespace AS2 {

// SF_INLINE MovieRoot* AvmSprite::GetAS2Root() const
// { 
//     //if (!pSprite) return NULL;
//     SF_ASSERT(pDispObj);
//     return static_cast<MovieRoot*>(pDispObj->GetASMovieRoot()); 
// }



AvmSprite::AvmSprite(Sprite* psprite)
: AvmCharacter(psprite), Level(-1)
{
    ASEnvironment.SetTargetOnConstruct(psprite);

    // By default LockRoot is false.
    GetSprite()->SetLockRoot(false);
    GetSprite()->SetMovieClipFlag();

    // Initialize the flags for init action executed.
    unsigned frameCount = GetDef()->GetFrameCount();
    SF_ASSERT(frameCount != 0);

    InitActionsExecuted.Resize(frameCount);
    memset(&InitActionsExecuted[0], 0, sizeof(InitActionsExecuted[0]) * frameCount);

    // From now on, the ASMovieClipObj is created on-demand by the GetMovieClipObject method
    //ASMovieClipObj = *SF_HEAP_NEW(pRoot->GetMovieHeap()) MovieClipObject(GetGC(), this);
    // let the base class know what's going on
    //pProto = ASMovieClipObj->Get__proto__();

    // Though, we still need pProto to set correctly.
    pProto = GetGC()->GetActualPrototype(&ASEnvironment, ASBuiltin_MovieClip);

    // NOTE: DoInitActions are executed separately in Advance.
}

AvmSprite::~AvmSprite() 
{
}

bool AvmSprite::IsLevelMovie() const
{
    return Level >= 0;
}

#ifndef SF_NO_IME_SUPPORT
bool AvmSprite::CheckCandidateListLevel(int level) const
{
	if (Level != level)
		return false;
	return true;
}
#endif

void AvmSprite::ForceShutdown()
{

}

void AvmSprite::ExecuteInitActionFrameTags(unsigned frame)
{
    SF_ASSERT(pDispObj && pDispObj->IsSprite());

    // Execute this frame's init actions, if necessary.
    if (InitActionsExecuted[frame] == false)
    {
        // Keep this (particularly Environment) alive during execution!
        Ptr<InteractiveObject> thisPtr(pDispObj);

        bool markFrameAsVisited = false;

        /*
        // execute init clip actions for each first time used import movies.
        const Array<GFxMovieDataDef*>* pimportMoviesSet = pDef->GetImportSourceMoviesForFrame(frame);
        if (pimportMoviesSet && pimportMoviesSet->GetSize() > 0)
        {
        // iterate through all import source movies for the current frame
        for(unsigned f = 0, nf = pimportMoviesSet->GetSize(); f < nf; ++f)
        {
        GFxMovieDefSub* psrcImportMovie = (*pimportMoviesSet)[f];
        SF_ASSERT(psrcImportMovie);
        if (psrcImportMovie->HasInitActions())
        {
        ExecuteImportedInitActions(psrcImportMovie);
        }
        }
        markFrameAsVisited = true;
        }
        */

        TimelineDef::Frame initActionsFrame;        

        if (GetDef()->GetInitActions(&initActionsFrame, frame) &&
            initActionsFrame.GetTagCount() > 0)
        {
            // Need to execute these actions.
            for (unsigned i= 0; i < initActionsFrame.GetTagCount(); i++)
            {
                ExecuteTag*  e = initActionsFrame.GetTag(i);
#ifdef GFX_TRACE_TIMELINE_EXECUTION
                printf("\ninit action execution, fr = %d, %s\n", frame, 
                    pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());
                e->Trace("");
#endif
                e->Execute(GetSprite());
            }
            markFrameAsVisited = true;
        }
        if (markFrameAsVisited)
        {
            // Mark this frame done, so we never execute these init actions again.
            InitActionsExecuted[frame] = true;
        }
    }
}

void AvmSprite::CloneInternalData(const InteractiveObject* src)
{
    AvmCharacter::CloneInternalData(src);
    if (src->IsSprite())
    {
        const Sprite* pSrcSprite = src->CharToSprite();
        if (pSrcSprite->ActsAsButton())
            SetHasButtonHandlers (true);
    }

}

bool AvmSprite::HasEventHandler(const EventId& id) const
{
    if (HasClipEventHandler(id))
        return true;

    // Check for member function.   
    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    //ASStringContext *psc = GetASEnvironment()->GetSC();
    ASString         methodName(EventId_GetFunctionName(GetAS2Root()->GetStringManager(), id));
    if (!methodName.IsEmpty())
    {
        // Event methods can only be in MovieClipObject or Environment.
        // It's important to not call GFx::GetMemberRaw here to save on overhead 
        // of checking display list and member constants, which will cost
        // us a lot when this is called in Advance.
        Value    method;

        // Resolving of event handlers should NOT involve __resolve and properties handles. Thus,
        // we need to use GetMemberRaw instead of GetMember. (AB)
        Object* asObj = const_cast<Object*> ((ASMovieClipObj) ? ASMovieClipObj : Get__proto__());
        if (asObj && asObj->GetMemberRaw(ASEnvironment.GetSC(), methodName, &method))    
            return true;            

    }
    return false;

}

void    AvmSprite::CallFrameActions(unsigned frameNumber)
{
    SF_ASSERT(pDispObj);
    if (frameNumber == unsigned(~0) || frameNumber >= pDispObj->GetLoadingFrame())
    {
        // No dice.
        pDispObj->LogError("CallFrame('%d') - unknown frame", frameNumber);
        return;
    }

    // These actions will be executed immediately. Such out-of-order execution
    // only happens due to a CallFrame instruction. We implement this by
    // simply assigning a different session id to actions being queued during
    // the execution.

    unsigned aqOldSession, aqCurSession = GetAS2Root()->ActionQueue.StartNewSession(&aqOldSession);

    // Execute the actions.
    const TimelineDef::Frame playlist = GetDef()->GetPlaylist(frameNumber);
    for (unsigned i = 0; i < playlist.GetTagCount(); i++)
    {
        ExecuteTag*  e = playlist.GetTag(i);
        if (e->IsActionTag())
        {
            e->Execute(GetSprite());
        }
    }
    GetAS2Root()->ActionQueue.RestoreSession(aqOldSession);

    // Execute any new actions triggered by the tag.    
    GetAS2Root()->DoActionsForSession(aqCurSession);

    // If some other actions were queued during the DoAction (like gotoAnd<>)- execute them 
    // as usual, at the next Advance. (AB)
}

void    AvmSprite::OnEventLoad()
{
    // check if the sprite is timeline animated or was created by attachMovie.
    // In the latter case we need to execute "onLoad" in frame priority to match up
    // with potentially following "onUnload" (if it gets unloaded right after attachMovie).
    if (HasClipEventHandler(EventId::Event_Load))
    {
        GetAS2Root()->InsertEmptyAction(ActionPriority::AP_Frame)->SetAction(pDispObj, EventId::Event_Load);
        GetSprite()->SetJustLoaded(true);
    }
    else
    {
        GetAS2Root()->InsertEmptyAction(ActionPriority::AP_Load)->SetAction(pDispObj, EventId::Event_Load);
        GetSprite()->SetJustLoaded(true);
    }

    AvmSprite::ExecuteInitActionFrameTags(0);
    GetSprite()->DefaultOnEventLoad();

    // Check if this sprite is a HitArea, and set HitAreaHolder  
    UPInt spriteArraySize =  GetAS2Root()->SpritesWithHitArea.GetSize(); 
    for (unsigned i = 0; i < spriteArraySize; i++)
        if (GetSprite() == GetAS2Root()->SpritesWithHitArea[i]->GetHitArea())
            GetAS2Root()->SpritesWithHitArea[i]->SetHitArea(GetSprite()); // Reset hitArea for efficient resolving
}

int AvmSprite::GetHitAreaIndex() const
{
    UPInt spriteArraySize =  GetAS2Root()->SpritesWithHitArea.GetSize(); 
    if (GetSprite()->GetHitAreaHandle()) // Should not be in array, so don't search  
    {
        for (unsigned i = 0; i < spriteArraySize; i++)
            if (GetAS2Root()->SpritesWithHitArea[i] == GetSprite())
                return i;
    }
    return -1;
}

void AvmSprite::SetHitAreaNotify(Sprite* phitArea)
{
    int haIndex = GetHitAreaIndex(); // Returns -1 if not found
    if (phitArea)
    {
        if (haIndex == -1)
            GetAS2Root()->SpritesWithHitArea.PushBack(GetSprite());
    }
    else
    {
        if (haIndex > -1)
            GetAS2Root()->SpritesWithHitArea.RemoveAt(haIndex);
    }

}

DisplayObject::TopMostResult
AvmSprite::GetTopMostEntity(const Render::PointF &localPt, TopMostDescr* pdescr,
                            const ArrayPOD<UByte>& hitTest)
{
    Sprite* const _this = GetSprite();
    Sprite* phitAreaHolder = _this->GetHitAreaHolder();
    Sprite* phitArea = _this->GetHitArea();
    TopMostDescr    savedDescr;
    TopMostResult   savedTe = DisplayObjectBase::TopMost_FoundNothing;
    bool            foundButMouseDisabled = false;

    // Go backwards, to check higher objects first.
    SPInt i, n;
    n = (SPInt)GetDisplayList().GetCount();
    for (i = n - 1; i >= 0; i--)
    {
        DisplayObjectBase* ch = GetDisplayList().GetDisplayObject(i);

        if (hitTest.GetSize() && (!hitTest[i] || ch->GetClipDepth()>0))
            continue;

        if (ch->IsTopmostLevelFlagSet()) // do not check children w/topmostLevel
            continue;

        // MA: This should consider submit masks in the display list,
        // such masks can obscure/clip out buttons for the purpose of
        // hit-testing as well.

        if (ch != NULL)
        {           
            TopMostResult te = ch->GetTopMostMouseEntity(localPt, pdescr);

            if (te == DisplayObjectBase::TopMost_Found)
            {
                //if (IsMouseChildrenDisabledFlagSet() || (pdescr->pResult && pdescr->pResult->IsMouseDisabledFlagSet()))
                if (_this->IsMouseChildrenDisabledFlagSet())
                    pdescr->pResult = _this;
                // AS3 may have a hitarea with mouseEnabled = false: still should go through regular
                // hitArea checks. AS3/test_hitArea_mouseDisabled4.swf
                if (pdescr->pResult && pdescr->pResult->IsMouseDisabledFlagSet())
                {
                    pdescr->pResult = _this;
                    foundButMouseDisabled = true;
                    //return TopMost_Continue;
                    continue;
                }
                if (pdescr->TestAll) 
                    te = DisplayObjectBase::TopMost_Found;
            }
            else if (te == DisplayObjectBase::TopMost_Continue && pdescr->pResult)
            {
                // if continue,
                savedTe     = DisplayObjectBase::TopMost_Found;
                savedDescr  = *pdescr;
            }

            // If we found anything and we are in button mode, _this refers to us.
            if (ActsAsButton() || (phitAreaHolder && (pdescr->TestAll || phitAreaHolder->ActsAsButton())))
            {   
                // It is either child or us; no matter - button mode takes precedence.
                // Note, if both - the hitArea and the holder of hitArea have button handlers
                // then the holder (parent) takes precedence; otherwise, if only the hitArea has handlers
                // it should be returned.
                if (te == DisplayObjectBase::TopMost_Found || savedTe == DisplayObjectBase::TopMost_Found)
                {
                    // In AS3, allow hitArea and hitArea's children to receive mouse
                    // events (unless they are disabled implicitly).
                    // See AS3/test_hitArea_mouseDisabledN.swfs
                    // However, the change conflicts with AS3/test_hitArea4.swf, disabling for now.
                    if (phitAreaHolder && (pdescr->TestAll || phitAreaHolder->ActsAsButton()))
                    {
                        pdescr->pResult = phitAreaHolder;               
                        pdescr->pHitArea = _this;
                        return DisplayObjectBase::TopMost_Found;
                    }
                    else 
                    {
                        // Sprites with hit area also shouldn't receive mouse events if _this hit area is not the sprite's child
                        // We need to check here if a hit area is our child  
                        if (phitArea)
                        {
                            InteractiveObject* parent = phitArea;
                            do {
                                parent = parent->GetParent();
                            } while (parent && parent != _this);
                            if (!parent)
                            {
                                // hit area is not our child so we should not receive mouse events
                                pdescr->pResult = NULL;
                                return DisplayObjectBase::TopMost_FoundNothing;
                            }
                            if (phitArea == pdescr->pHitArea)
                            {
                                pdescr->pResult = _this;
                                return DisplayObjectBase::TopMost_Found;
                            }
                            else
                            {
                                pdescr->pResult = NULL;
                                savedTe = DisplayObjectBase::TopMost_FoundNothing;
                                continue;
                            }
                        }
                        else
                        { 
                            pdescr->pResult = _this;
                            return DisplayObjectBase::TopMost_Found;
                        }
                    }
                }
                if (te == DisplayObjectBase::TopMost_Found && pdescr->TestAll) 
                    return te;
            }
            else 
            {
                if (te == DisplayObjectBase::TopMost_Found && pdescr->TestAll) 
                    return te;

                if (te == DisplayObjectBase::TopMost_Found && pdescr->pResult != _this)
                {
                    // Found one.
                    // @@ TU: GetVisible() needs to be recursive here!

                    // character could be _root, in which case it would have no parent.
                    if (pdescr->pResult->GetParent() && pdescr->pResult->GetParent()->GetVisible())
                        //if (te->GetVisible()) // TODO move _this check to the base case(s) only
                    {
                        // @@
                        // Vitaly suggests "return ch" here, but it breaks
                        // samples/test_button_functions.swf
                        //
                        // However, it fixes samples/clip_as_button.swf
                        //
                        // What gives?
                        //
                        // Answer: a button event must be passed up to parent until
                        // somebody handles it.
                        //
                        return te;
                    }
                    else
                    {
                        pdescr->pResult = NULL;
                        return DisplayObjectBase::TopMost_FoundNothing;
                    }
                }           
            }
        }
    }
    // if we are here then we are going to return 'Continue'. But first, check
    // if there is a hitArea, and if there is one check if we hit into it. Otherwise,
    // return 'NotFound'. Refer to AS3 test_hitArea1.swf,test_hitArea2.swf,test_hitArea3.swf.
    if (phitArea)
    {
        // hitArea must be ignored if hit occurred on a child that is InteractiveObject.
        // In this case the child still should get the event (test_hitarea_and_child_mc.swf).
        if (!pdescr->pResult || pdescr->pResult == _this || !pdescr->pResult->IsInteractiveObject())
        {
            if (phitArea == pdescr->pHitArea)
            {
                pdescr->pResult = _this;
                return DisplayObjectBase::TopMost_Found;
            }
            else
            {
                pdescr->pResult = NULL;
                return DisplayObjectBase::TopMost_FoundNothing;
            }
        }
    }
    if (savedTe == DisplayObjectBase::TopMost_Found)
    {
        *pdescr = savedDescr;
        return DisplayObjectBase::TopMost_Found;
    }
    pdescr->LocalPt = localPt;
    if (foundButMouseDisabled)
    {
        pdescr->pResult = _this;
        return DisplayObjectBase::TopMost_Found;
    }
    else
        pdescr->pResult = NULL;
    return DisplayObjectBase::TopMost_Continue;
}

void    AvmSprite::OnEventUnload()
{

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
    Set__proto__(ASEnvironment.GetSC(), NULL);
}

bool AvmSprite::OnUnloading(bool mayRemove)
{
    int haIndex = GetHitAreaIndex(); // Returns -1 if not found
    if (haIndex > -1)
        GetAS2Root()->SpritesWithHitArea.RemoveAt(haIndex);

    // check, if we can unload the sprite instantly or if we need just
    // to mark it as "unloading".
    if (mayRemove && HasEventHandler(EventId::Event_Unload))
        mayRemove = false;
    if (!mayRemove)
    {
        // we can't instantly remove the character, because either it has one of the
        // children with Unload event or itself has the Unload handler.

        // Thus, we need to mark it as "unloading" and queue up the Unload event
        // (no matter, does this character have the unload handler or not; we would
        // need to re-mark it as "unloaded" after the event is executed, see
        // ExecuteEvent).

        // Here is a special case: if movie is loaded by attachMovie and its 
        // onLoad is not called yet, we need to simulate onLoad call before onUnload.
        // Otherwise, we will see only onUnload, since onLoad will be attempted to execute 
        // AFTER onLoad. See test_onload_onunload_attachmovie.swf 
        if (GetSprite()->IsJustLoaded() && !GetSprite()->IsAcceptAnimMovesFlagSet())
        {
            MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction();
            if (pe) pe->SetAction(pDispObj, EventId::Event_Load);
        }

        MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction();
        if (pe) pe->SetAction(pDispObj, EventId::Event_Unload);
    }
   pDispObj->RemoveFromPlayList();
   return mayRemove;
}

// ThisPtr - sprite
// Arg(0) - constructor function of the class
void AvmSprite::InitializeClassInstance(const FnCall& fn)
{
    AvmSprite* pscriptCh = fn.ThisPtr->ToAvmSprite();
    SF_ASSERT(pscriptCh);

    FunctionRef ctorFunc = fn.Arg(0).ToFunction(fn.Env);
    SF_ASSERT(!ctorFunc.IsNull());

    pscriptCh->SetProtoToPrototypeOf(ctorFunc.GetObjectPtr());
}

// ThisPtr - sprite
// Arg(0) - String, name of instance
// Arg(1) - Boolean, true, if construct event should be fired (!AB: not anymore)
//static 
void AvmSprite::FindClassAndInitializeClassInstance(const FnCall& fn)
{
    SF_ASSERT(fn.Env);

    GlobalContext* gctxt = fn.Env->GetGC();
    SF_ASSERT(gctxt);

    FunctionRef ctorFunc;
    const ASString symbolName(fn.Arg(0).ToString(fn.Env));
    if (!symbolName.IsEmpty())
    {
        AvmSprite* pscriptCh = fn.ThisPtr->ToAvmSprite();
        SF_ASSERT(pscriptCh);

        if (gctxt->FindRegisteredClass(fn.Env->GetSC(), symbolName, &ctorFunc))
        {
            SF_ASSERT(!ctorFunc.IsNull());

            pscriptCh->SetProtoToPrototypeOf(ctorFunc.GetObjectPtr());

            // now, check for 'construct' event and fire it if Arg(1) is set to TRUE
            //if (fn.Arg(1).ToBool(fn.Env)) //!AB, fire onConstruct always, since it is impossible
            {                               // to check whether onConstruct method exists or not
                MovieRoot::ActionEntry e(pscriptCh->GetDispObj(), EventId::Event_Construct);
                e.Execute(pscriptCh->GetAS2Root());
            }

            // now, invoke a constructor
            MovieRoot::ActionEntry e(pscriptCh->GetDispObj(), ctorFunc);
            e.Execute(pscriptCh->GetAS2Root());
        }
        else
        {
            // a special case, when a component doesn't have a class (it is possible if Linkage->AS 2.0 Class field left empty)
            // but there is a construct event for its instance.

            // now, check for 'construct' event and fire it if Arg(1) is set to TRUE
            //if (fn.Arg(1).ToBool(fn.Env)) //!AB, fire onConstruct always, since it is impossible
            {                               // to check whether onConstruct method exists or not
                MovieRoot::ActionEntry e(pscriptCh->GetDispObj(), EventId::Event_Construct);
                e.Execute(pscriptCh->GetAS2Root());
            }

        }
    }
}

// Arg(0) - Object, initializer object
void AvmSprite::InitObjectMembers(const FnCall& fn)
{
    SF_ASSERT(fn.Env);
    SF_ASSERT(fn.NArgs == 1);

    Ptr<Sprite> pscriptCh = fn.ThisPtr->ToSprite();
    if (!pscriptCh)
        return; // init object members works only for movieclips (per Flash)

    const ObjectInterface* pinitSource = fn.Arg(0).ToObjectInterface(fn.Env);

    struct InitVisitor : public Object::MemberVisitor
    {
        Environment*     pEnv;
        InteractiveObject*  pCharacter;

        InitVisitor(Environment *penv, InteractiveObject *pchar)
            : pEnv(penv), pCharacter(pchar)
        { }

        virtual void    Visit(const ASString& name, const Value& val, UByte flags)
        {
            SF_UNUSED(flags);                
            GFx::AS2::ToAvmCharacter(pCharacter)->SetMember(pEnv, name, val);
        }
    };
    InitVisitor memberVisitor(fn.Env, pscriptCh);
    pinitSource->VisitMembers(memberVisitor.pEnv->GetSC(), &memberVisitor);
}

// Add an object to the display list.
DisplayObjectBase*   AvmSprite::AddDisplayObject(
    const CharPosInfo &pos, 
    const ASString& name,
    const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
    const void *pinitSource,
    unsigned createFrame,
    UInt32 addFlags,
    CharacterCreateInfo* pcharCreateOverride,
    InteractiveObject* origChar)
{
    //MovieDef* pdef = GetDef();
    MovieDefImpl* pdefImpl = GetDefImpl();
    //SF_ASSERT(pdef);
    SF_ASSERT(GetDispObj());

    bool placeObject = (addFlags & DisplayList::Flags_PlaceObject) != 0;
    bool replaceIfDepthIsOccupied = (addFlags & DisplayList::Flags_ReplaceIfDepthIsOccupied) != 0;

    // TBD:
    // Here we look up a character but do not return it's binding.
    // This means that while we will have the right character created,
    // its pdefImpl will correspond OUR table and not THEIR table.
    // However, imported characters MUST reference their own tables for resources!

    CharacterCreateInfo ccinfo = pcharCreateOverride ? *pcharCreateOverride :    
        pdefImpl->GetCharacterCreateInfo(pos.CharacterId);
    if (!ccinfo.pCharDef)
    {
        GetDispObj()->LogError("Sprite::AddDisplayObject() - unknown cid = %d", pos.CharacterId.GetIdIndex());
        return NULL;
    }

    // If we already have this object on this plane, then move it instead of replacing it.
    // This can happen when wrapping around from last frame to first one, where
    // the first frame fill bring back to life objects marked for removal after last frame.
    // It can also happen when seeking back.
    bool markedForRemove = false;
    DisplayObjectBase* pexistingChar = GetSprite()->GetCharacterAtDepth(pos.Depth, &markedForRemove);

    unsigned frame = ((createFrame == SF_MAX_UINT) ? GetSprite()->GetCurrentFrame() : createFrame);
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
            const ASString& originalName =
                ((pexistingChar->IsInteractiveObject()) ? pexistingChar->CharToInteractiveObject_Unsafe()->GetOriginalName() : 
                GetAS2Root()->GetBuiltinsMgr().GetBuiltin(ASBuiltin_empty_));

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

                    GetSprite()->MoveDisplayObject(newPos);             

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
    GetSprite()->SetDirtyFlag();

    Ptr<DisplayObjectBase> ch = *GetAS2Root()->GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), ccinfo, GetSprite(), pos.CharacterId);
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

        // in the case if AddDisplayObject is not initiated through timeline
        // (Flag_PlaceObject is NOT set) we need to mark the newly created
        // object by SetAcceptAnimMovies(false).
        if (!(addFlags & DisplayList::Flags_PlaceObject))
            intObj->SetAcceptAnimMoves(false);
        else
            intObj->SetTimelineObjectFlag(true);
    }

    ch->SetCreateFrame(frame);

    unsigned sessionId = 0;
    // Attach event Handlers (if any).
    if (intObj)
    {
        sessionId = OnAddingDisplayObject(ccinfo, intObj, peventHandlers, pinitSource, placeObject);
    }

    addFlags &= ~DisplayList::Flags_ReplaceIfDepthIsOccupied;
    if (replaceIfDepthIsOccupied)
        addFlags |= DisplayList::Flags_ReplaceIfDepthIsOccupied;

    if (intObj)
    {
        // AS 2.0 adds this sprite into a playlist BEFORE its children
        intObj->AddToPlayList();
        intObj->ModifyOptimizedPlayList();
    }

    GetSprite()->GetDisplayList().AddDisplayObject(GetSprite(), pos, ch.GetPtr(), addFlags);   

    // Assign sticky variables (only applies if there is an identifiable name).
    if (nameSet)
        GetAS2Root()->ResolveStickyVariables(intObj);

    SF_ASSERT(!ch || ch->GetRefCount() > 1);

    if (intObj)
    {
        OnAddedDisplayObject(intObj, sessionId, placeObject);
    }

    if (intObj && nameSet)
    {
        // check if the name is "hitArea" and assign the hitArea if it is so.
        if (name == GetAS2Root()->GetBuiltinsMgr().GetBuiltin(ASBuiltin_hitArea))
            GetSprite()->SetHitArea(static_cast<Sprite*>(intObj));
    }
    //mDisplayList.CheckConsistency(GetSprite());
    return ch.GetPtr();
}

unsigned AvmSprite::OnAddingDisplayObject(const CharacterCreateInfo& ccinfo, 
                                      InteractiveObject* pscriptCh, 
                                      const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
                                      const void* pinitSource,
                                      bool placeObject)
{
    Sprite* pspriteCh = pscriptCh->CharToSprite();

    bool hasConstructEvent = true; //!AB, fire onConstruct always, since it is impossible
    // to check whether onConstruct method exists or not
//    MovieImpl* proot = GetMovieImpl();
    MovieRoot* asroot = GetAS2Root();
    if (pscriptCh && peventHandlers)
    {
        for (UPInt i = 0, n = peventHandlers->GetSize(); i < n; i++)   
        {       
            (*peventHandlers)[i]->AttachTo(pscriptCh);
            // If we installed a button handler, sprite will work in button mode...
            if (pspriteCh && (*peventHandlers)[i]->Event.IsButtonEvent())
                GFx::AS2::ToAvmSprite(pspriteCh)->SetHasButtonHandlers (true);

            if (placeObject)
            {
                if ((*peventHandlers)[i]->Event.Id == EventId::Event_Initialize)
                {
                    ActionBufferData* pactionOpData = (*peventHandlers)[i]->pActionOpData;
                    if (pactionOpData && !pactionOpData->IsNull())
                    {
                        MovieRoot::ActionEntry* pe = asroot->InsertEmptyAction(MovieRoot::AP_Initialize);
                        if (pe) pe->SetAction(pscriptCh, EventId::Event_Initialize);
                        //{
                        //    Ptr<ActionBuffer> pbuff =
                        //        *SF_HEAP_NEW(GetMovieHeap())
                        //        ActionBuffer(pscriptCh->GetASEnvironment()->GetSC(), pactionOpData);
                        //    pe->SetAction(pscriptCh, pbuff);
                        //}
                    }
                }
                //!AB, fire onConstruct always, since it is impossible
                // to check whether onConstruct method exists or not. It is only possible
                // to check whether onClipEvent(construct) exists or not, but this is not enough.
                //if ((*peventHandlers)[i]->Event.Id == EventId::Event_Construct)
                //{
                //    hasConstructEvent = true;
                //}
            }
        }
    }

    // Check for registered classes
    unsigned sessionId = 0;
    if (pscriptCh)
    {
        GlobalContext* gctxt = this->GetGC();
        SF_ASSERT(gctxt);

        FunctionRef ctorFunc;    
        const String* psymbolName = pscriptCh->GetResourceMovieDef()->GetNameOfExportedResource(ccinfo.pCharDef->GetId());

        unsigned oldSessionId;
        sessionId = asroot->ActionQueue.StartNewSession(&oldSessionId);

        bool initSourceUsed = false;

        if (psymbolName)
        {
            ASString symbolName = GetASEnvironment()->CreateString(*psymbolName);

            if (gctxt->FindRegisteredClass(GetASEnvironment()->GetSC(), symbolName, &ctorFunc))
            {
                // first - schedule initializer of class instance. It will
                // assign proper prototype to the pscriptCh.
                MovieRoot::ActionEntry* pe = asroot->InsertEmptyAction(MovieRoot::AP_Initialize);
                ValueArray params;
                params.PushBack(ctorFunc);
                if (pe) pe->SetAction(pscriptCh, InitializeClassInstance, &params);

                // schedule "construct" event, if any
                //if (hasConstructEvent) //!AB, fire onConstruct always, since it is impossible
                {                        // to check whether onConstruct method exists or not
                    MovieRoot::ActionEntry* ape = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                    if (ape) ape->SetAction(pscriptCh, EventId::Event_Construct);
                    hasConstructEvent = false;
                }

                // Queue up copying init properties if they exist (for SWF 6+).
                if (GetVersion() >= 6 && pinitSource && pscriptCh)
                {
                    params.Resize(0);
                    Value v;
                    v.SetAsObjectInterface
                        (const_cast<ObjectInterface*>(reinterpret_cast<const ObjectInterface*>(pinitSource)));
                    params.PushBack(v);
                    MovieRoot::ActionEntry* ape = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                    if (ape) ape->SetAction(pscriptCh, InitObjectMembers, &params);
                }
                initSourceUsed = true;

                // now, schedule constructor invocation
                pe = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, ctorFunc);
            }
            else if (placeObject)
            {
                ValueArray params;
                params.PushBack(Value(symbolName)); //[0]
                //params.PushBack(Value(hasConstructEvent)); //[1]

                MovieRoot::ActionEntry* pe = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, FindClassAndInitializeClassInstance, &params);
                hasConstructEvent = false;
            }
        }
        if (placeObject)
        {
            if (hasConstructEvent)
            {
                // if no class is registered, only onClipEvent(construct)

                MovieRoot::ActionEntry* pe = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, EventId::Event_Construct);
            }
        }
        else if (!initSourceUsed)
        {
            // Not a placeobject (means - attachMovie or duplicateMovieClip) and no registered class:
            // queue up copying init properties if they exist (for SWF 6+).
            if (GetVersion() >= 6 && pinitSource && pscriptCh)
            {
                ValueArray params;
                Value v;
                v.SetAsObjectInterface
                    (const_cast<ObjectInterface*>(reinterpret_cast<const ObjectInterface*>(pinitSource)));
                params.PushBack(v);
                MovieRoot::ActionEntry* pe = asroot->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, InitObjectMembers, &params);
            }
        }
        asroot->ActionQueue.RestoreSession(oldSessionId);
    }
    return sessionId;
}

void AvmSprite::OnAddedDisplayObject(InteractiveObject* pscriptCh, 
                                     unsigned sessionId,
                                     bool placeObject)
{
    Sprite* pspriteCh = pscriptCh->CharToSprite();
    MovieRoot* asroot = GetAS2Root();

    // here is a special case for attachMovie. If we have queued up constructors' invocations before 
    // DisplayList.AddDisplayObject then we need to execute them right now.
    if (pscriptCh && !placeObject)
    {
        asroot->DoActionsForSession(sessionId);

        // check if onLoad event is allowed. OnEventLoad puts onLoad event into a queue. 
        // This happens BEFORE all children are added and BEFORE constructor is executed.
        // But real decision about executing/not executing onLoad event may be done only
        // AFTER all children are added ("DisplayList.AddDisplayObject") and ctor is
        // executed ("DoActionsForSession"), but BEFORE the first frame execution. 
        // So, we set a flag "OnLoadReqd", if new sprite really needs "onLoad" event to 
        // be executed. This flag is tested in ExecuteEvent and if it is not set for 
        // non-timeline sprite, the onLoad event will not be executed, even though 
        // it is in the queue. This is necessary for correct execution of onLoad events
        // during "attachMovie" call.
        if (pspriteCh && pspriteCh->HasEventHandler(EventId::Event_Load))
            pspriteCh->SetOnLoadReqd();    
    }
    else
    {
        // Allow onLoad events for timeline sprites
        if (pspriteCh)
            pspriteCh->SetOnLoadReqd();    
    }
}

void    AvmSprite::AdvanceFrame(bool nextFrame, float framePos)
{
    SF_UNUSED(framePos);
    Ptr<Sprite> spr = GetSprite();
    if (spr->IsAdvanceDisabled() || spr->IsUnloading())
        return;

    SF_ASSERT(GetDef() && GetMovieImpl() != NULL);

    if (nextFrame)
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
                OnEvent(EventId::Event_EnterFrame);

                spr->ExecuteFrameTags(currentFrame);
            }
            else
            {
                // Post OnEnterFrame event, so that it arrives before other
                // frame actions generated by frame tags.
                OnEvent(EventId::Event_EnterFrame);
            }
        }
        else
        {
            OnEvent(EventId::Event_EnterFrame);
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

//
// *** Sprite ActionScript support
//

// InteractiveObject override to indicate which standard members are handled for us.
UInt32  AvmSprite::GetStandardMemberBitMask() const
{
    // MovieClip lets base handle all members it supports.
    return UInt32(
        M_BitMask_PhysicalMembers |
        M_BitMask_CommonMembers |           
        (1u << M_target) |
        (1u << M_droptarget) |
        (1u << M_url) |          
        (1u << M_blendMode) |
        (1u << M_cacheAsBitmap) |
        (1u << M_filters) |
        (1u << M_focusrect) |
        (1u << M_enabled) |
        (1u << M_trackAsMenu) |
        (1u << M_lockroot) |
        (1u << M_tabEnabled) |
        (1u << M_tabIndex) |
        (1u << M_useHandCursor) |        
        (1u << M_quality) |
        (1u << M_highquality) |
        (1u << M_soundbuftime) |
        (1u << M_xmouse) |
        (1u << M_ymouse) |

        // InteractiveObject will report these as read-only for us,
        // but we still need to handle them in GetMember.
        (1u << M_currentframe) |
        (1u << M_totalframes) |
        (1u << M_framesloaded) 
        );
}

bool    AvmSprite::GetStandardMember(StandardMember member, Value* pval, bool opcodeFlag) const
{
    if (AvmCharacter::GetStandardMember (member, pval, opcodeFlag))
        return true;

    // Handle MovieClip specific "standard" members.
    switch(member)
    {
    case M_currentframe:
        pval->SetInt(GetSprite()->GetCurrentFrame() + 1);
        return true;

    case M_totalframes:             
        pval->SetInt(GetDef()->GetFrameCount());
        return true;

    case M_framesloaded:
        pval->SetInt(GetSprite()->GetLoadingFrame());
        return true;

    case M_lockroot:
        pval->SetBool(GetSprite()->IsLockRoot());            
        return true;

    case M_focusEnabled:   
        {
            if (GetSprite()->GetFocusEnabledFlag().IsDefined())
                pval->SetBool(GetSprite()->GetFocusEnabledFlag().IsTrue());
            else
                pval->SetUndefined();
            return 1;
        }

    case M_tabChildren:   
        {
            // Is a yellow rectangle visible around a focused InteractiveObject Clip (?)
            if (TabChildren.IsDefined())
                pval->SetBool(TabChildren.IsTrue());
            else
                pval->SetUndefined();
            return 1;
        }

    case M_hitArea:
        if (GetSprite()->GetHitAreaHandle())
        {
            pval->SetAsCharacterHandle(GetSprite()->GetHitAreaHandle());
            return 1;
        }
        else
            pval->SetUndefined();
        break;

    case M_scale9Grid:
        if (GetASEnvironment()->GetVersion() >= 8)
        {
            if (GetSprite()->HasScale9Grid())
            {
                RectF s9g = GetSprite()->GetScale9Grid();
                Environment* penv = const_cast<Environment*>(GetASEnvironment());

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> rectObj = *SF_HEAP_NEW(penv->GetHeap()) RectangleObject(penv);
                ASRect gr(TwipsToPixels(s9g.x1), 
                          TwipsToPixels(s9g.y1), 
                          TwipsToPixels(s9g.x2), 
                          TwipsToPixels(s9g.y2)); 
                rectObj->SetProperties(penv, gr);
#else
                Ptr<Object> rectObj = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
                ASStringContext *psc = penv->GetSC();
                rectObj->SetConstMemberRaw(psc, "x", TwipsToPixels(s9g.x1));
                rectObj->SetConstMemberRaw(psc, "y", TwipsToPixels(s9g.y1));
                rectObj->SetConstMemberRaw(psc, "width", TwipsToPixels(s9g.x2));
                rectObj->SetConstMemberRaw(psc, "height", TwipsToPixels(s9g.y2));
#endif
               pval->SetAsObject(rectObj);
            }
            else
            {
                pval->SetUndefined();
            }
            return true;
        }
        break;

    case M_scrollRect:
        if (GetASEnvironment()->GetVersion() >= 8)
        {
            if (GetSprite()->HasScrollRect())
            {
                RectD sr = *GetSprite()->GetScrollRect();
                Environment* penv = const_cast<Environment*>(GetASEnvironment());

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> rectObj = *SF_HEAP_NEW(penv->GetHeap()) RectangleObject(penv);
                ASRect gr(TwipsToPixels(sr.x1), 
                    TwipsToPixels(sr.y1), 
                    TwipsToPixels(sr.x2), 
                    TwipsToPixels(sr.y2)); 
                rectObj->SetProperties(penv, gr);
#else
                Ptr<Object> rectObj = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
                ASStringContext *psc = penv->GetSC();
                rectObj->SetConstMemberRaw(psc, "x", TwipsToPixels(sr.x1));
                rectObj->SetConstMemberRaw(psc, "y", TwipsToPixels(sr.y1));
                rectObj->SetConstMemberRaw(psc, "width", TwipsToPixels(sr.x2));
                rectObj->SetConstMemberRaw(psc, "height", TwipsToPixels(sr.y2));
#endif
                pval->SetAsObject(rectObj);
            }
            else
            {
                pval->SetUndefined();
            }
            return true;
        }
        break;

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(GetSprite()->IsHitTestDisableFlagSet());
            return 1;
        }
        break;

    default:
        break;
    }
    return false;
}

bool    AvmSprite::SetStandardMember(StandardMember member, const Value& origVal, bool opcodeFlag)
{   
    Value val(origVal);
    Environment* penv = GetASEnvironment();
    if (member > M_BuiltInProperty_End)
    {
        // Check, if there are watch points set. Invoke, if any.
        if (penv && ASMovieClipObj && ASMovieClipObj->HasWatchpoint()) // have set any watchpoints?
        {
            Value newVal;
            if (ASMovieClipObj->InvokeWatchpoint(penv, 
                penv->CreateConstString(AvmCharacter::MemberTable[member].pName), val, &newVal))
            {
                val = newVal;
            }
        }
    }
    if (AvmCharacter::SetStandardMember (member, val, opcodeFlag))
        return true;

    // Handle MovieClip specific "standard" members.
    switch(member)
    {   
    case M_currentframe:
    case M_totalframes:
    case M_framesloaded:
        // Already handled as Read-Only in base,
        // so we won't get here.
        return true;

    case M_lockroot:
        GetSprite()->SetLockRoot(val.ToBool(GetASEnvironment()));
        return true;

    case M_focusEnabled:
        if (!val.IsUndefined())
            GetSprite()->SetFocusEnabledFlag(val.ToBool(GetASEnvironment()));
        else
            GetSprite()->UndefineFocusEnabledFlag();
        return 1;

    case M_tabChildren:
        if (!val.IsUndefined())
        {
            bool v = val.ToBool(GetASEnvironment());
            TabChildren = v;
            GetSprite()->SetTabChildrenDisabledFlag(!v);
        }
        else
           TabChildren = Bool3W();
        return 1;

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            GetSprite()->SetHitTestDisableFlag(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;

    case M_hitArea:
        {
            // If hit area is not a MovieClip then it should be handled as regular member 
            InteractiveObject* pcharacter = val.ToCharacter(GetASEnvironment());
            if (pcharacter && pcharacter->IsSprite())
            {
                GetSprite()->SetHitArea((Sprite*) pcharacter);
                return 1;
            }
            GetSprite()->SetHitArea(0);
        }
        break;

    case M_scale9Grid:
        if (GetASEnvironment()->GetVersion() >= 8)
        {
            Environment* penvGrid = GetASEnvironment();
            Object* pobj = val.ToObject(penvGrid);

#ifdef GFX_AS2_ENABLE_RECTANGLE
            if (pobj && pobj->GetObjectType() == Object_Rectangle)
            {
                RectangleObject* prect = (RectangleObject*)pobj;
                ASRect gr;
                prect->GetProperties(penvGrid, gr);
                RectF sg;
                sg.x1 = PixelsToTwips(float(gr.x1));
                sg.y1 = PixelsToTwips(float(gr.y1));
                sg.SetWidth(PixelsToTwips(float(gr.Width())));
                sg.SetHeight(PixelsToTwips(float(gr.Height())));
                GetSprite()->SetScale9Grid(sg);
            }
#else
            if (pobj)
            {
                ASStringContext *psc = penv->GetSC();
                Value params[4];
                pobj->GetConstMemberRaw(psc, "x", &params[0]);
                pobj->GetConstMemberRaw(psc, "y", &params[1]);
                pobj->GetConstMemberRaw(psc, "width", &params[2]);
                pobj->GetConstMemberRaw(psc, "height", &params[3]);
                RectF sg;
                sg.x1 = PixelsToTwips(float(params[0].ToNumber(penvGrid)));
                sg.y2 = PixelsToTwips(float(params[1].ToNumber(penvGrid)));
                sg.SetWidth(PixelsToTwips(float(params[2].ToNumber(penvGrid))));
                sg.SetHeight(PixelsToTwips(float(params[3].ToNumber(penvGrid))));
                GetSprite()->SetScale9Grid(sg);
            }
#endif
            else
                GetSprite()->SetScale9Grid(RectF(0));
            return true;
        }
        break;
    case M_scrollRect:
        {
            if (GetASEnvironment()->GetVersion() >= 8)
            {
                Environment* penvRect = GetASEnvironment();
                Object* pobj = val.ToObject(penvRect);
#ifdef GFX_AS2_ENABLE_RECTANGLE
                if (pobj)
                {
                    if (pobj->GetObjectType() == Object_Rectangle)
                    {
                        RectangleObject* prect = (RectangleObject*)pobj;
                        ASRect gr;
                        prect->GetProperties(penvRect, gr);
                        RectD sg;
                        sg.x1 = PixelsToTwips(gr.x1);
                        sg.y1 = PixelsToTwips(gr.y1);
                        sg.SetWidth(PixelsToTwips(gr.Width()));
                        sg.SetHeight(PixelsToTwips(gr.Height()));
                        GetSprite()->SetScrollRect(&sg);
                    }
                }
#else
                if (pobj)
                {
                    ASStringContext *psc = penvRect->GetSC();
                    Value params[4];
                    pobj->GetConstMemberRaw(psc, "x", &params[0]);
                    pobj->GetConstMemberRaw(psc, "y", &params[1]);
                    pobj->GetConstMemberRaw(psc, "width", &params[2]);
                    pobj->GetConstMemberRaw(psc, "height", &params[3]);
                    RectD sg;
                    sg.x1 = PixelsToTwips(params[0].ToNumber(penvRect));
                    sg.y2 = PixelsToTwips(params[1].ToNumber(penvRect));
                    sg.SetWidth(PixelsToTwips(params[2].ToNumber(penvRect)));
                    sg.SetHeight(PixelsToTwips(params[3].ToNumber(penvRect)));
                    GetSprite()->SetScrollRect(&sg);
                }
#endif
                else
                    pDispObj->SetScrollRect(NULL);
            }
        }
        break;

        // No other custom properties to set for now.
    default:
        break;
    }
    return false;
}

// Set the named member to the value.  Return true if we have
// that member; false otherwise.
bool    AvmSprite::SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags)
{    
    if (IsStandardMember(name))
    {
        StandardMember member = GetStandardMemberConstant(name);
        if (SetStandardMember(member, val, 0))
            return true;
    }

    if (ASMovieClipObj || GetMovieClipObject())
    {
        // Note that MovieClipObject will also track setting of button
        // handlers, i.e. 'onPress', etc.
        return ASMovieClipObj->SetMemberRaw(psc, name, val, flags);
    }
    return false;
}

// internal method, a common implementation for both GetMember/GetMemberRaw.
// Only either penv or psc should be not NULL. If penv is not null, then this method
// works like GetMember, if psc is not NULL - it is like GetMemberRaw
bool    AvmSprite::GetMember(Environment* penv, ASStringContext *psc, const ASString& name, Value* pval)
{
    if (IsStandardMember(name))
    {
        StandardMember member = GetStandardMemberConstant(name);
        if (GetStandardMember(member, pval, 0))
            return true;

        switch(member)
        {
        case M_transform:
            {

#ifdef GFX_AS2_ENABLE_TRANSFORM
                // create a TransformObject
                Ptr<TransformObject> transfObj = 
                    *SF_HEAP_NEW(GetASEnvironment()->GetHeap()) TransformObject(GetASEnvironment(), pDispObj);
                pval->SetAsObject(transfObj);
                return true;
#else
                SF_DEBUG_WARNING(1, "Transform ActionScript class was not included in this GFx build.");
                pval->SetUndefined();
                return true;
#endif
            }
        case M__version:
            if (IsLevelMovie())
            {
                pval->SetString(GetASEnvironment()->CreateConstString(GFX_FLASH_VERSION));
                return true;
            }
            break;

        case M_xrotation:
            {
                // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
                pval->SetNumber(pDispObj->GetXRotation());
                return true;
            }

        case M_yrotation:
            {
                // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
                pval->SetNumber(pDispObj->GetYRotation());
                return true;
            }

        case M_z:
            {
                // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
                pval->SetNumber(pDispObj->GetZ());
                return true;
            }

        case M_zscale:
            {
                // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
                pval->SetNumber(pDispObj->GetZScale());
                return true;
            }
        case M_matrix3d:
            {
                Matrix3F m3D = pDispObj->GetMatrix3D();
                Matrix4F mat4(m3D);
                mat4.Transpose();

                // create a GASArrayObject
                Ptr<ArrayObject> arrayObj = *SF_HEAP_NEW(penv->GetHeap()) ArrayObject(GetASEnvironment());
                arrayObj->Resize(16);
                for(int i=0;i<16;i++)
                    arrayObj->SetElement(i, Value(mat4.Data()[i])); 

                pval->SetAsObject(arrayObj);
                return true;
            }

        case M_fov:
            {
                pval->SetNumber(pDispObj->GetFOV());
                return true;
            }

        default:
            break;
        }
    }

    // Handle the __proto__ property here, since we are going to 
    // zero out it temporarily (see comments below).
    if ((penv && name == penv->GetBuiltin(ASBuiltin___proto__)) ||
        (psc  && name == psc->GetBuiltin(ASBuiltin___proto__)))
    {
        Object* proto = Get__proto__();
        pval->SetAsObject(proto);
        return true;
    }
    // The sprite's variables have higher priority than display list, BUT
    // the display list has HIGHER priority than prototype's properties.
    // That is why we would need to do a trick: set __proto__ of the ASMovieClipObj
    // to NULL in order to exclude the prototype and then put it back.
    if (ASMovieClipObj)
    {
        // HACK: zero the __proto__ of ASMovieClipObj to let DisplayList to be
        // checked before the prototype. TODO: redesign.
        Ptr<Object> savedProto = ASMovieClipObj->Exchange__proto__(NULL);
        SF_ASSERT(savedProto == Get__proto__());
        bool rv = false;
        if (penv && ASMovieClipObj->GetMember(penv, name, pval))    
        {
            rv = true;
        }
        else if (psc && ASMovieClipObj->GetMemberRaw(psc, name, pval))    
        {
            rv = true;
        }
        // restore the __proto__
        ASMovieClipObj->Exchange__proto__(savedProto);
        if (rv)
            return true;
    }

    // Not a built-in property.  Check items on our display list.
    DisplayObject* pch = GetSprite()->GetDisplayObjectByName(name, ASEnvironment.IsCaseSensitive());
    if (pch && pch->IsInteractiveObject())
    {
        // Found object.
        pval->SetAsCharacter(pch->CharToInteractiveObject_Unsafe());
        return true;
    }
    else 
    {
        // Now we can search in the __proto__
        Object* proto = Get__proto__();
        if (proto)
        {
            // ASMovieClipObj is not created yet; use __proto__ instead
            if (penv && proto->GetMember(penv, name, pval))    
            {
                return true;
            }
            else if (psc && proto->GetMemberRaw(psc, name, pval))    
            {
                return true;
            }
        }
    }

    if (name.GetLength() > 0 && name[0] == '_')
    {
        Environment::GetVarParams params(name, pval);
        Bool3W rv = ASEnvironment.CheckGlobalAndLevels(params);
        if (rv.IsDefined())
            return rv.IsTrue();
    }
    return false;
}

// Find the InteractiveObject which is one degree removed from us,
// given the relative pathname.
//
// If the pathname is "..", then return our parent.
// If the pathname is ".", then return ourself.  If
// the pathname is "_level0" or "_root", then return
// the root pMovie.
//
// Otherwise, the name should refer to one our our
// named characters, so we return it.
//
// NOTE: In ActionScript 2.0, top level Names (like
// "_root" and "_level0") are CASE SENSITIVE.
// DisplayObject names in a display list are CASE
// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
//
// In ActionScript 1.0, everything seems to be CASE
// INSENSITIVE.
InteractiveObject* AvmSprite::GetRelativeTarget(const ASString& name, bool first_call)
{
    bool           caseSensitive = IsCaseSensitive();
    Environment &e = ASEnvironment;

    if (name.IsBuiltin())
    {    
        // Special branches for case-sensitivity for efficiency.
        if (caseSensitive)
        {
            if (e.GetBuiltin(ASBuiltin_dot_) == name || // "."
                e.GetBuiltin(ASBuiltin_this) == name)
            {
                return pDispObj;
            }
            else if (e.GetBuiltin(ASBuiltin_dotdot_) == name || // ".."
                e.GetBuiltin(ASBuiltin__parent) == name)
            {
                return GetParent();
            }
            else if (e.GetBuiltin(ASBuiltin__root) == name)
            {
                return GetTopParent();
            }
        }
        else
        {   // Case-insensitive version.
            name.ResolveLowercase();
            if (e.GetBuiltin(ASBuiltin_dot_) == name ||
                e.GetBuiltin(ASBuiltin_this).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {
                return pDispObj;
            }
            else if (e.GetBuiltin(ASBuiltin_dotdot_) == name || // ".."
                e.GetBuiltin(ASBuiltin__parent).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {
                return GetParent();
            }
            else if (e.GetBuiltin(ASBuiltin__root).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {
                return GetTopParent();
            }
        }
    }

    if (name[0] == '_' && first_call)
    {
        // Parse level index.
        const char* ptail = 0;
        int         levelIndex = GetAS2Root()->ParseLevelName(name.ToCStr(), &ptail,
            caseSensitive);
        if ((levelIndex != -1) && !*ptail)
            return GetAS2Root()->GetLevelMovie(levelIndex);
    }

    // See if we have a match on the display list.
    DisplayObject* r = GetSprite()->GetDisplayObjectByName(name, e.IsCaseSensitive());
    if (r && r->IsInteractiveObject())
        return r->CharToInteractiveObject_Unsafe();
    return NULL;
}

void    AvmSprite::SetLevel(int level)
{
    // need to find the index to insert the render node at
    UPInt i = 0, n = GetMovieImpl()->MovieLevels.GetSize();
    for (; i < n; ++i)
    {
        const MovieImpl::LevelInfo& li = GetMovieImpl()->MovieLevels[i];
        if (li.Level > level)
            break;
    }
    GetAS2Root()->TopNode->Insert(i, GetSprite()->GetRenderNode());

    Level = level;

    // Assign the default name to level.
    // Note that for levels, Flash allows changing these names later,
    // but such new names can not be used for path lookup.
    char nameBuff[64] = "";
    Format(StringDataPtr(nameBuff, sizeof(nameBuff)), "_level{0}", level);
    GetSprite()->SetName(ASEnvironment.CreateString(nameBuff));
}


// Dispatch event Handler(s), if any.
bool    AvmSprite::OnEvent(const EventId& id)
{
    /*  
    if(GetName().ToCStr() && GetName().ToCStr()[0])
    {
    // IF unload executed, clear pparent
    if (id.Id == EventId::Event_Unload)                
    SF_DEBUG_MESSAGE1(1, "Event_Unload event - %s", GetName().ToCStr());
    if (id.Id == EventId::Event_Load)              
    SF_DEBUG_MESSAGE1(1, "Event_Load event - %s", GetName().ToCStr());
    } */

    // Check if the sprite has special frames, such as _up, _down, _over.
    // Go to these frames if event is Release, Press or RollOver.
    SpriteDef* sp;
    if ((sp = GetSpriteDef()) != NULL)
    {
        if (sp->HasSpecialFrames())
        {
            switch(id.Id)
            {
            case EventId::Event_DragOut:
            case EventId::Event_Release:
            case EventId::Event_RollOver:
                if (sp->HasFrame_over())
                    GetSprite()->GotoLabeledFrame("_over");
                break;
            case EventId::Event_Press:
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

    do {

        // First, check for built-in event onClipEvent() handler.   
        if (HasClipEventHandler(id))
        {
            // Dispatch.
            continue;
        }

        // Check for member function.   
        // In ActionScript 2.0, event method names are CASE SENSITIVE.
        // In ActionScript 1.0, event method names are CASE INSENSITIVE.
        ASStringContext *psc = GetASEnvironment()->GetSC();
        ASString         methodName(EventId_GetFunctionName(psc->GetStringManager(), id));
        if (!methodName.IsEmpty())
        {
            // Event methods can only be in MovieClipObject or Environment.
            // It's important to not call GFx::GetMemberRaw here to save on overhead 
            // of checking display list and member constants, which will cost
            // us a lot when this is called in Advance.
            bool hasMethod = false;
            Value    method;

            // Resolving of event handlers should NOT involve __resolve and properties handles. Thus,
            // we need to use GetMemberRaw instead of GetMember. (AB)
            Object* asObj = (ASMovieClipObj) ? ASMovieClipObj : Get__proto__();
            if (asObj && asObj->GetMemberRaw(ASEnvironment.GetSC(), methodName, &method))    
                hasMethod = true;
            //!AB, now we put named functions as a member of target, thus they should be found by the
            // statement above.

            if (id.Id == EventId::Event_KeyDown || id.Id == EventId::Event_KeyUp) 
            {
                // onKeyDown/onKeyUp are available only in Flash 6 and later
                // (don't mess with onClipEvent (keyDown/keyUp)!)
                if (ASEnvironment.GetVersion() >= 6)
                {
                    // also, onKeyDown/onKeyUp should be invoked only if focus
                    // is enabled and set to this movieclip
                    if (!GetMovieImpl()->IsKeyboardFocused(GetSprite(), id.KeyboardIndex))
                        hasMethod = false;
                }
                else
                {
                    // Flash 5 and below doesn't have onKeyDown/Up function handlers.
                    hasMethod = false;
                }
            }

            if (hasMethod)
                continue;
        }   

        return false;
    } while (0);

    // do actual dispatch
    MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction();
    if (pe) pe->SetAction(GetSprite(), id);
    return true;
}

bool    AvmSprite::ExecuteBuffer(ActionBuffer* pactionbuffer)
{
    if (!IsUnloaded()) 
    {
        pactionbuffer->Execute(GetASEnvironment());
        return true;
    }
    else
    {
        // This happens due to untimely Event_Load/Event_Unload during seek
        //SF_DEBUG_WARNING1(1, "Action execute after unload: %s", pSprite->GetName().ToCStr());
    }
    return false;
}

// Execute this even immediately (called for processing queued event actions).
bool    AvmSprite::ExecuteEvent(const EventId& id)
{
    if (IsUnloaded())
        return false; // do not execute events for unloaded-ready-to-die characters

    // Keep Environment alive during any method calls!
    Ptr<Sprite> thisPtr(GetSprite());

    if (id == EventId::Event_Load)
    {
        GetSprite()->SetJustLoaded(false);
        if (!GetSprite()->IsAcceptAnimMovesFlagSet() && !GetSprite()->IsOnLoadReqd())
            return false;
    }

    bool rv = AvmCharacter::ExecuteEvent (id);
    // Handler done, mark us as unloaded.
    if (id == EventId::Event_Unload)
    {
        // The character continues its existence after onUnload is invoked;
        // It is removed by the next Advance.
        GetSprite()->SetUnloaded(true);

        // Add to unloaded list
        SF_ASSERT(!GetSprite()->IsOptAdvListFlagSet());
        GetSprite()->SetNextUnloaded(GetMovieImpl()->pUnloadListHead);
        GetMovieImpl()->pUnloadListHead  = GetSprite();
        GetSprite()->AddRef(); // need to addref it since it might be not held by anyone else
    }
    return rv;
}


bool AvmSprite::Invoke(const char* methodName, Value* presult, unsigned numArgs)
{
    // Keep Environment alive during any method calls!
    Ptr<Sprite> thisPtr(GetSprite());

    return GAS_Invoke(methodName, presult, this, &ASEnvironment, numArgs, ASEnvironment.GetTopIndex());
}

bool AvmSprite::InvokeArgs(const char* methodName, Value* presult, const char* methodArgFmt, va_list args)
{
    // Keep Environment alive during any method calls!
    Ptr<Sprite> thisPtr(GetSprite());

    return GAS_InvokeParsed(methodName, presult, this, &ASEnvironment, methodArgFmt, args);
}

/*const char* AvmSprite::InvokeArgs(const char* methodName, const char* methodArgFmt, 
const void* const* methodArgs, int numArgs)
{
// Keep Environment alive during any method calls!
Ptr<Sprite> thisPtr(this);

return GAS_InvokeParsed(&ASEnvironment, this, methodName, methodArgFmt, methodArgs, numArgs);
}*/


bool AvmSprite::ActsAsButton() const    
{ 
    if (IsLevelMovie() || !IsEnabledFlagSet())
        return false; // _levelN should NOT act as a button no matter what.
    const MovieClipObject* asObj = (ASMovieClipObj) ? ASMovieClipObj.GetPtr() :
        static_cast<const MovieClipObject*>(Get__proto__());
    if (asObj) 
        return asObj->ActsAsButton(); 
    return false;
}

void AvmSprite::SetHasButtonHandlers(bool has)
{
    MovieClipObject* asObj = GetMovieClipObject();
    if (asObj) 
        asObj->SetHasButtonHandlers(has);  
}

void    AvmSprite::VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface*) const
{
    if (visitFlags & VisitMember_ChildClips)
    {
        struct Visitor : public DisplayList::MemberVisitor 
        {
            ObjectInterface::MemberVisitor   *pVisitor;
            unsigned                            VisitFlags;

            Visitor(ObjectInterface::MemberVisitor *pvisitor, unsigned visitFlags)
                : pVisitor(pvisitor), VisitFlags(visitFlags) {}
            virtual void    Visit(const ASString& name, InteractiveObject* pch)
            {
                SF_ASSERT(pVisitor);
                pVisitor->Visit(name, Value(pch), (UByte)VisitFlags);
            }
        } dispListVisitor(pvisitor, visitFlags);
        GetSprite()->VisitChildren(&dispListVisitor); 
    }
    AvmCharacter::VisitMembers(psc, pvisitor, visitFlags, this);
}


// Delete a member field, if not read-only. Return true if deleted.
bool    AvmSprite::DeleteMember(ASStringContext *psc, const ASString& name)
{
    if (AvmCharacter::DeleteMember(psc, name))
        return true;
    return false; 
}

MovieClipObject* AvmSprite::GetMovieClipObject()          
{ 
    if (!ASMovieClipObj)
        ASMovieClipObj = *SF_HEAP_NEW(GetAS2Root()->GetMovieHeap()) MovieClipObject(GetGC(), GetSprite());
    return ASMovieClipObj;
}

Object* AvmSprite::GetASObject ()          
{ 
    return GetMovieClipObject(); 
}

Object* AvmSprite::GetASObject () const
{ 
    return const_cast<AvmSprite*>(this)->GetMovieClipObject(); 
}

bool     AvmSprite::IsFocusEnabled(FocusMovedType fmt) const
{
    if (fmt == GFx_FocusMovedByMouse)
        return AvmCharacter::IsFocusEnabled(fmt);

    Object* pproto = Get__proto__();
    if (pproto)
    {
        // check prototype for focusEnabled
        Value val;
        if (pproto->GetMemberRaw(ASEnvironment.GetSC(), ASEnvironment.CreateConstString("focusEnabled"), &val))
        {
            if (!val.IsUndefined())
                return val.ToBool(&ASEnvironment);
        }
    }
    return (ActsAsButton());
}

void AvmSprite::ExecuteFrame0Events()
{
    GetAS2Root()->InsertEmptyAction(MovieRoot::AP_Load)->SetAction(pDispObj, EventId::Event_Load);
}

unsigned AvmSprite::GetCursorType() const
{
    if (ActsAsButton())
    {
        const Environment* penv = GetASEnvironment();
        Value val;

        if (const_cast<AvmSprite*>(this)->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_useHandCursor), &val))
        {
            if (val.IsUndefined() || !val.ToBool(penv))
                return AvmCharacter::GetCursorType();
        }
        return MouseCursorEvent::HAND;
    }
    return AvmCharacter::GetCursorType();
}





// AvmSprite::SpriteGetTarget can return NULL in the case when
// sprite-related method (such as 'attachMovie', etc) is being
// called for MovieClip instance without attached sprite, for 
// example:
// var m = new MovieClip;
// m.attachMovie(...);
// Here AvmSprite::SpriteGetTarget wiil return NULL.
Sprite* AvmSprite::SpriteGetTarget(const FnCall& fn)
{
    Sprite* psprite = NULL;
    if (fn.ThisPtr == NULL)
    {
        SF_ASSERT(fn.Env->GetTarget()->IsSprite());
        psprite = static_cast<Sprite*>(fn.Env->GetTarget());
    }
    else
    {
        psprite = fn.ThisPtr->ToSprite();
    }
    //SF_ASSERT(psprite);
    return psprite;
}

InteractiveObject* AvmSprite::CharacterGetTarget(const FnCall& fn)
{
    InteractiveObject* psprite = NULL;
    if (fn.ThisPtr == NULL)
    {
        psprite = fn.Env->GetTarget();
    }
    else
    {
        psprite = fn.ThisPtr->ToCharacter();
    }
    //SF_ASSERT(psprite);
    return psprite;
}

bool    AvmSprite::RemoveCharacter(DisplayObjectBase* ch)
{
    Sprite* spr = GetSprite();
    DisplayList& dlist  = spr->GetDisplayList();
    int     depth       = ch->GetDepth();
    UPInt   size        = dlist.GetCount();    
    UPInt   index       = dlist.FindDisplayIndex(depth);

    // 1st validity checks.
    if (index >= size)
    {
        return false;
    }
    // we need to find a character to remove. The following loop is to iterate through
    // potential set of different characters at the same depth.
    DisplayObjectBase* pcurCh = dlist.GetDisplayObject(index);
    while (pcurCh && pcurCh != ch && depth == pcurCh->GetDepth() && index + 1 < size)
    {
        pcurCh = dlist.GetDisplayObject(++index);
    }
    if (index < size && pcurCh == ch)
    {
        // we've found a character to remove
        dlist.RemoveEntryAtIndex(spr, index);
        return true;
    }
    return false;
}

//
// *** Sprite Built-In ActionScript methods
//

void AvmSprite::SpritePlay(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;
    psprite->SetPlayState(State_Playing);
}

void AvmSprite::SpriteStop(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;
    psprite->SetPlayState(State_Stopped);
}

void AvmSprite::SpriteGotoAndPlay(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs < 1)
    {
        psprite->LogScriptError("AvmSprite::SpriteGotoAndPlay needs one arg");
        return;
    }

    const Value& arg = fn.Arg(0);
    unsigned        targetFrame = SF_MAX_UINT;
    if (arg.GetType() == Value::STRING)
    {   // Frame label or string frame number
        ASString sa0(arg.ToString(fn.Env));
        if (!psprite->GetLabeledFrame(sa0.ToCStr(), &targetFrame))
            return;
    }
    else
    {   // Convert to 0-based
        targetFrame = arg.ToUInt32(fn.Env) - 1;
    }

    psprite->GotoFrame(targetFrame);
    psprite->SetPlayState(State_Playing);
}

void AvmSprite::SpriteGotoAndStop(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs < 1)
    {
        psprite->LogScriptError("AvmSprite::SpriteGotoAndStop needs one arg");
        return;
    }

    const Value& arg = fn.Arg(0);
    unsigned        targetFrame = SF_MAX_UINT;
    if (arg.GetType() == Value::STRING)
    {   // Frame label or string frame number
        ASString sa0(arg.ToString(fn.Env));
        if (!psprite->GetLabeledFrame(sa0.ToCStr(), &targetFrame))
            return;
    }
    else
    {   // Convert to 0-based
        targetFrame = arg.ToUInt32(fn.Env) - 1;
    }

    psprite->GotoFrame(targetFrame);
    psprite->SetPlayState(State_Stopped);
}

void AvmSprite::SpriteNextFrame(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    int availableFrameCount = psprite->GetLoadingFrame();
    int currentFrame = psprite->GetCurrentFrame();
    if (currentFrame < availableFrameCount)
        psprite->GotoFrame(currentFrame + 1);      
    psprite->SetPlayState(State_Stopped);
}

void AvmSprite::SpritePrevFrame(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    int currentFrame = psprite->GetCurrentFrame();
    if (currentFrame > 0)
        psprite->GotoFrame(currentFrame - 1);
    psprite->SetPlayState(State_Stopped);
}


void AvmSprite::SpriteGetBytesLoaded(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Value of getBytesLoaded must always match the frames loaded,
    // because many Flash files rely on it. In particular, if this value
    // is equal to getTotalBytes, they assume that all frames must be
    // available. If that does not hold, many pre-loader algorithms
    // will end up in infinite AS loops.

    fn.Result->SetInt(psprite->GetBytesLoaded());
}

void AvmSprite::SpriteGetBytesTotal(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;
    fn.Result->SetInt(psprite->GetResourceMovieDef()->GetFileBytes());
}


// Duplicate / remove clip.
void AvmSprite::SpriteDuplicateMovieClip(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 2) return;

    Ptr<InteractiveObject> newCh = GFx::AS2::ToAvmCharacter(psprite)->CloneDisplayObject(
        fn.Arg(0).ToString(fn.Env), ((int)fn.Arg(1).ToNumber(fn.Env)) + 16384,
        (fn.NArgs == 3) ? fn.Arg(2).ToObjectInterface(fn.Env) : 0);

    //!AB: duplicateMovieClip in Flash 6 and above should return newly created clip
    if (psprite->GetVersion() >= 6)
    {
        fn.Result->SetAsCharacter(newCh.GetPtr());
    }
}

void AvmSprite::SpriteAttachMovie(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 3) return;

    // Get resource id, looked up locally based on  
    ASString           sa0(fn.Arg(0).ToString(fn.Env));

    ResourceBindData resBindData;
    if (!psprite->GetMovieImpl()->FindExportedResource(psprite->GetResourceMovieDef(), &resBindData, sa0.ToCStr()))
    {
        psprite->LogScriptWarning("%s.attachMovie() failed - export name \"%s\" is not found.",
            psprite->GetName().ToCStr(), sa0.ToCStr());
        return;
    }

    SF_ASSERT(resBindData.pResource.GetPtr() != 0); // MA TBD: Could this be true?     
    if (!(resBindData.pResource->GetResourceType() & Resource::RT_CharacterDef_Bit))
    {
        psprite->LogScriptWarning("%s.attachMovie() failed - \"%s\" is not a movieclip.",
            psprite->GetName().ToCStr(), sa0.ToCStr());
        return;
    }

    CharacterCreateInfo ccinfo;
    ccinfo.pCharDef     = (CharacterDef*) resBindData.pResource.GetPtr();
    ccinfo.pBindDefImpl = resBindData.pBinding->GetOwnerDefImpl();

    // Create a new object and add it.
    CharPosInfo pos( ccinfo.pCharDef->GetId(), ((int)fn.Arg(2).ToNumber(fn.Env)) + 16384,
        1, Cxform::Identity, 1, Matrix2F::Identity);
    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
    {
        psprite->LogScriptWarning("%s.attachMovie(\"%s\") failed - depth (%d) must be >= 0",
            psprite->GetName().ToCStr(), sa0.ToCStr(), pos.Depth);
        return;
    }

    // We pass pchDef to make sure that symbols from nested imports use
    // the right definition scope, which might be different from that of psprite.
    Ptr<DisplayObjectBase> newCh = psprite->AddDisplayObject(
        pos, fn.Arg(1).ToString(fn.Env), 0,
        (fn.NArgs == 4) ? fn.Arg(3).ToObjectInterface(fn.Env) : 0, SF_MAX_UINT, 
        DisplayList::Flags_ReplaceIfDepthIsOccupied, &ccinfo);
    if (newCh)
    {
        newCh->SetAcceptAnimMoves(false);

        //!AB: attachMovie in Flash 6 and above should return newly created clip
        if (psprite->GetVersion() >= 6)
        {
            InteractiveObject* pspriteCh = newCh->CharToInteractiveObject(); 
            SF_ASSERT (pspriteCh != 0);
            fn.Result->SetAsCharacter(pspriteCh);
        }
    }
}

#if defined(GFX_ENABLE_SOUND) && defined(GFX_ENABLE_VIDEO)

void AvmSprite::SpriteAttachAudio(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (!psprite)
        return;

    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("%s.attachAudio() needs one Argument", psprite->GetName().ToCStr());
        return;
    }
    Object* obj = fn.Arg(0).ToObject(fn.Env);
    if (obj)
    {
        if (obj->GetObjectType() == Object::Object_NetStream)
        {
            Video::VideoBase* pvideoState = fn.Env->GetMovieImpl()->GetVideo();
            if (pvideoState && pvideoState->GetAS2Support())
                pvideoState->GetAS2Support()->AttachAudio(obj, psprite);
        }
    }
}
#endif // SF_NO_VIDEO && GFX_NO_SOUND 

void AvmSprite::SpriteAttachBitmap(const FnCall& fn)
{
    fn.Result->SetUndefined();
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 2) 
        return;
    if (psprite->GetVersion() < 8) // supported only in Flash 8
        return;

    // Arg(0) - BitmapData
    // Arg(1) - depth
    // Arg(2) - String, pixelSnapping, optional (auto, always, never)
    // Arg(3) - Boolean, smoothing, optional

    Ptr<Object> pobj = fn.Arg(0).ToObject(fn.Env);
    if (!pobj || pobj->GetObjectType() != Object::Object_BitmapData)
    {
        psprite->LogScriptWarning("%s.attachBitmap() failed - the argument is not a BitmapData.",
            psprite->GetName().ToCStr());
        return;
    }

    BitmapData*    pbmpData  = static_cast<BitmapData*>(pobj.GetPtr());
    ImageResource* pimageRes = pbmpData->GetImage();
    if (!pimageRes)
    {
        psprite->LogScriptWarning("%s.attachBitmap() failed - no image set in BitmapData.",
            psprite->GetName().ToCStr());
        return;
    }

    // Create position info
    CharPosInfo pos = CharPosInfo(ResourceId(CharacterDef::CharId_ImageMovieDef_ShapeDef),
        ((int)fn.Arg(1).ToNumber(fn.Env)) + 16384,
        1, Cxform::Identity, 1, Matrix2F::Identity);
    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
    {
        psprite->LogScriptWarning("%s.attachBitmap() failed - depth (%d) must be >= 0",
            psprite->GetName().ToCStr(), pos.Depth);
        return;
    }


    bool                smoothing = (fn.NArgs >= 4) ? fn.Arg(3).ToBool(fn.Env) : false;
    MovieImpl*          pmroot = fn.Env->GetMovieImpl();
    MovieRoot*          as2root = fn.Env->GetAS2Root();
    Ptr<MovieDefImpl>   pimageMovieDef = *pmroot->CreateImageMovieDef(pimageRes, smoothing, "");

    if (pimageMovieDef)
    {           
        // need to save a strong pointer to ImageResource's heap to make sure the resource is not released
        // while pimageMovieDef exists.
        pimageMovieDef->GetDataDef()->pData->SetExtMovieDef(pbmpData->GetMovieDef());

        // Empty sprite based on new Def. New Def gives correct Version, URL, and symbol lookup behavior.
        Ptr<Sprite> pchildSprite = *as2root->CreateSprite
            (pimageMovieDef->GetDataDef(), pimageMovieDef, psprite, 
             ResourceId(CharacterDef::CharId_EmptyMovieClip), true);

        if (pchildSprite)
        {    

            // Verified: Flash assigns depth -16383 to image shapes (depth value = 1 in our list).
            int                         depth = 1;
            CharPosInfo              locpos = CharPosInfo(ResourceId(CharacterDef::CharId_ImageMovieDef_ShapeDef),
                depth, 0, Cxform(), 1, Matrix2F());

            ASString                   emptyName(fn.Env->GetBuiltin(ASBuiltin_empty_));        

            pchildSprite->AddToPlayList();
            pchildSprite->ModifyOptimizedPlayList();

            pchildSprite->AddDisplayObject(locpos, emptyName, NULL, NULL, 1);
            psprite->ReplaceDisplayObject(pos, pchildSprite, emptyName);

            psprite->SetAcceptAnimMoves(false);
        }
    }
#else
    SF_DEBUG_WARNING(1, "Error: attachBitmap is failed since SF_NO_FXPLAYER_AS_BITMAPDATA is defined.");
#endif
}

void AvmSprite::SpriteCreateEmptyMovieClip(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 2) return;

    // Create a new object and add it.
    CharPosInfo pos = CharPosInfo( ResourceId(CharacterDef::CharId_EmptyMovieClip),
        ((int)fn.Arg(1).ToNumber(fn.Env)) + 16384,
        1, Cxform::Identity, 1, Matrix2F::Identity);
    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
        return;

    Ptr<DisplayObjectBase> newCh = psprite->AddDisplayObject(
        pos, fn.Arg(0).ToString(fn.Env), NULL, NULL, SF_MAX_UINT, 
        DisplayList::Flags_ReplaceIfDepthIsOccupied);  
    if (newCh)
    {
        newCh->SetAcceptAnimMoves(false);

        // Return newly created clip.
        InteractiveObject* pspriteCh = newCh->CharToInteractiveObject(); 
        SF_ASSERT (pspriteCh != 0);
        fn.Result->SetAsCharacter(pspriteCh);
    }
}

void AvmSprite::SpriteCreateTextField(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 6) return; //?AB, could it work with less than 6 params specified?

    // Create a new object and add it.    
    CharPosInfo pos = CharPosInfo( ResourceId(CharacterDef::CharId_EmptyTextField),
        ((int)fn.Arg(1).ToNumber(fn.Env)) + 16384,
        1, Cxform::Identity, 1, Matrix2F::Identity);
    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
        return;

    Ptr<DisplayObjectBase> newCh = psprite->AddDisplayObject(
        pos, fn.Arg(0).ToString(fn.Env), NULL, NULL, SF_MAX_UINT, 
        DisplayList::Flags_ReplaceIfDepthIsOccupied);  
    if (newCh)
    {
        newCh->SetAcceptAnimMoves(false);

        // Return newly created clip.
        InteractiveObject* ptextCh = newCh->CharToInteractiveObject(); 
        SF_ASSERT (ptextCh != 0);
        AvmCharacter* pavmCh = GFx::AS2::ToAvmCharacter(ptextCh);
        pavmCh->SetStandardMember(AvmSprite::M_x, fn.Arg(2), false);
        pavmCh->SetStandardMember(AvmSprite::M_y, fn.Arg(3), false);
        pavmCh->SetStandardMember(AvmSprite::M_width,  fn.Arg(4), false);
        pavmCh->SetStandardMember(AvmSprite::M_height, fn.Arg(5), false);
        fn.Result->SetAsCharacter(ptextCh);
    }
}


void AvmSprite::SpriteRemoveMovieClip(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (psprite->GetDepth() < 16384)
    {
        psprite->LogScriptWarning("%s.removeMovieClip() failed - depth must be >= 0",
            psprite->GetName().ToCStr());
        return;
    }
    psprite->RemoveDisplayObject();
}

// Implementation of MovieClip.setMask method. 
void AvmSprite::SpriteSetMask(const FnCall& fn)
{
    fn.Result->SetUndefined();

    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs >= 1)
    {
        if (fn.Arg(0).IsNull())
        {
            // remove mask
            psprite->SetMask(NULL);
        }
        else
        {
            InteractiveObject* pobj = fn.Arg(0).ToCharacter(fn.Env);
            if (pobj)
            {
                Sprite* pmaskSpr = pobj->CharToSprite();
                psprite->SetMask(pmaskSpr);
            }
            else
                psprite->SetMask(NULL);
        }
    }
}

void AvmSprite::SpriteGetBounds(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Get target sprite.
    InteractiveObject*  pobj    = (fn.NArgs > 0) ? fn.Arg(0).ToCharacter(fn.Env) : psprite;
    Sprite*             ptarget = pobj ? pobj->CharToSprite() : 0;

    RectF              b(0);
    Matrix2F   matrix;

    if (ptarget)
    {
        // Transform first by sprite's matrix, then by inverse of target.
        if (ptarget != psprite) // Optimize identity case.
        {
            matrix.SetInverse(ptarget->GetWorldMatrix());
            matrix *= psprite->GetWorldMatrix();
        }

        // A "perfect" implementation would be { b = psprite->GetBounds(matrix); },
        // however, Flash always gets the local bounding box before a transform,
        // as follows.
        matrix.EncloseTransform(&b, psprite->GetBounds(Matrix2F()));
    }

    // Store into result object.    
    Ptr<Object>     presult = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    ASStringContext*   psc = fn.Env->GetSC();
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_xMin), TwipsToPixels(Double(b.x1)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_xMax), TwipsToPixels(Double(b.x2)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_yMin), TwipsToPixels(Double(b.y1)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_yMax), TwipsToPixels(Double(b.y2)));

    fn.Result->SetAsObject(presult.GetPtr());   
}


//
// MovieClip.getRect()
//
// Returns properties that are the minimum and maximum x and y 
// coordinate values of the movie clip, based on the bounds parameter,
// excluding any strokes on shapes. The values that getRect() returns 
// are the same or smaller than those returned by MovieClip.getBounds(). 
//
void AvmSprite::SpriteGetRect(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Get target sprite.
    InteractiveObject*  pobj    = (fn.NArgs > 0) ? fn.Arg(0).ToCharacter(fn.Env) : psprite;
    Sprite*             ptarget = pobj ? pobj->CharToSprite() : 0;

    RectF               b(0);
    Matrix2F            matrix;



    if (ptarget)
    {
        // Transform first by sprite's matrix, then by inverse of target.
        if (ptarget != psprite) // Optimize identity case.
        {
            matrix.SetInverse(ptarget->GetWorldMatrix());
            matrix *= psprite->GetWorldMatrix();
        }

        // A "perfect" implementation would be { b = psprite->GetBounds(matrix); },
        // however, Flash always gets the local bounding box before a transform,
        // as follows.
        matrix.EncloseTransform(&b, psprite->GetRectBounds(Matrix2F()));
    }

    // Store into result object.    
    Ptr<Object>     presult = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    ASStringContext*   psc = fn.Env->GetSC();
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_xMin), TwipsToPixels(Double(b.x1)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_xMax), TwipsToPixels(Double(b.x2)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_yMin), TwipsToPixels(Double(b.y1)));
    presult->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_yMax), TwipsToPixels(Double(b.y2)));

    fn.Result->SetAsObject(presult.GetPtr());     
}


void AvmSprite::SpriteLocalToGlobal(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 1) return;

    // Get target object.
    ASStringContext*   psc = fn.Env->GetSC();
    ObjectInterface* pobj = fn.Arg(0).ToObjectInterface(fn.Env);
    if (!pobj) return;

    // Object must have x & y members, which are numbers; otherwise 
    // the function does nothing. This is the expected behavior.
    Value            xval, yval;  
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &xval);
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &yval);
    if (!xval.IsNumber() || !yval.IsNumber())
        return;

    // Compute transformation and set members.
    Render::PointF pt((float)SF_PIXELS_TO_TWIPS(xval.ToNumber(fn.Env)), (float)SF_PIXELS_TO_TWIPS(yval.ToNumber(fn.Env)));
	pt = psprite->LocalToGlobal(pt);
    pobj->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), TwipsToPixels(Double(pt.x)));
    pobj->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), TwipsToPixels(Double(pt.y)));
}

void AvmSprite::SpriteGlobalToLocal(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL || fn.NArgs < 1) return;

    // Get target object.
    ASStringContext*   psc = fn.Env->GetSC();
    ObjectInterface* pobj = fn.Arg(0).ToObjectInterface(fn.Env);
    if (!pobj) return;

    // Object must have x & y members, which are numbers; otherwise 
    // the function does nothing. This is the expected behavior.
    Value xval, yval;
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), &xval);
    pobj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), &yval);
    if (!xval.IsNumber() || !yval.IsNumber())
        return;

    // Compute transformation and set members.
    Render::PointF pt((float)SF_PIXELS_TO_TWIPS(xval.ToNumber(fn.Env)), (float)SF_PIXELS_TO_TWIPS(yval.ToNumber(fn.Env)));
	pt = psprite->GlobalToLocal(pt);
    pobj->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_x), TwipsToPixels(Double(pt.x)));
    pobj->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_y), TwipsToPixels(Double(pt.y)));
}


// Hit-test implementation.
void AvmSprite::SpriteHitTest(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    fn.Result->SetBool(0);

    RectF spriteLocalBounds = psprite->GetBounds(Matrix2F());
    if (spriteLocalBounds.IsNull())
        return;

    // Hit-test has either 1, 2 or 3 arguments.
    if (fn.NArgs >= 2)
    {
        UInt8 hitTestMask = 0;
        // x, y, shapeFlag version of hitTest.
        Render::PointF     pt( (float)SF_PIXELS_TO_TWIPS(fn.Arg(0).ToNumber(fn.Env)),
            (float)SF_PIXELS_TO_TWIPS(fn.Arg(1).ToNumber(fn.Env)) );
        if (fn.NArgs >= 3) // optional parameter shapeFlag
            hitTestMask |= (fn.Arg(2).ToBool(fn.Env)) ? DisplayObject::HitTest_TestShape : 0;
        if (fn.NArgs >= 4) // optional parameter shapeFlag
            hitTestMask |= (fn.Arg(3).ToBool(fn.Env)) ? DisplayObject::HitTest_IgnoreInvisible : 0;

 /*
       if (pt.x == 0 && pt.y == 0)
            return;
*/
        MovieImpl * proot = psprite->GetMovieImpl();
        if ( proot && psprite->Is3D(true) )
        {
            proot->SetNormalizedScreenCoords(pt);

            Render::ScreenToWorld & stw = proot->ScreenToWorld;
            Matrix3F view;
            Matrix4F proj; 

            if (psprite->GetProjectionMatrix3D(&proj))
                stw.SetProjection(proj);

            if (psprite->GetViewMatrix3D(&view))
                stw.SetView(view);

            stw.SetWorld(psprite->GetWorldMatrix3D());
            Render::PointF pp;
            stw.GetWorldPoint(&pp);

            fn.Result->SetBool(psprite->PointTestLocal (pp, hitTestMask));
            return;
        }

        Render::PointF ptSpr = psprite->GetLevelMatrix().TransformByInverse(pt);
        // pt is already in root's coordinates!

        if (psprite->DoesScale9GridExist())
        {
            fn.Result->SetBool(psprite->PointTestLocal (ptSpr, hitTestMask));
        }
        else
        {
            if (spriteLocalBounds.Contains(ptSpr))
            {
                if (!(hitTestMask & DisplayObject::HitTest_TestShape))
                    fn.Result->SetBool(true);
                else
                    fn.Result->SetBool(psprite->PointTestLocal (ptSpr, hitTestMask));
            }
            else
                fn.Result->SetBool(false);
        }
    }
    else if (fn.NArgs == 1)
    {
        // Target sprite version of hit-test.
        const Value&     arg = fn.Arg(0);
        DisplayObject* ptarget = NULL;
        if (arg.IsCharacter())
        {
            ptarget = arg.ToCharacter(fn.Env);
        }
        else
        {
            // if argument is not a character, then convert it to string
            // and try to resolve as variable (AB)
            ASString varName = arg.ToString(fn.Env);
            Value result;
            if (fn.Env->GetVariable(varName, &result))
            {
                ptarget = result.ToCharacter(fn.Env);
            }
        }
        if (!ptarget)
            return;

        RectF targetLocalRect  = ptarget->GetBounds(Matrix2F());
        if (targetLocalRect.IsNull())
            return;
        // Rect rectangles in same coordinate space & check intersection.
        RectF spriteWorldRect  = psprite->GetWorldMatrix().EncloseTransform(spriteLocalBounds);
        RectF targetWorldRect  = ptarget->GetWorldMatrix().EncloseTransform(targetLocalRect);
        fn.Result->SetBool(spriteWorldRect.Intersects(targetWorldRect));
    }
}

// Implemented as member function so that it can access Sprite's private methods.
void    AvmSprite::SpriteSwapDepths(const FnCall& fn)
{
    InteractiveObject* pchar = AvmSprite::CharacterGetTarget(fn);
    if (pchar == NULL || fn.NArgs < 1) return;

    Sprite* pparent = (Sprite*)pchar->GetParent();
    const Value& arg     = fn.Arg(0);
    int             depth2  = 0;
    InteractiveObject* ptarget = 0;
    Sprite* psprite = (pchar->IsSprite()) ? (Sprite*)pchar : 0;

    // Depth can be a number at which the character is inserted.
    if (arg.IsNumber())
    {
        depth2 = ((int)arg.ToNumber(fn.Env)) + 16384;
        // Verified: Flash will discard smaller values.
        if (depth2 < 0)
            return;
        // Documented depth range is -16384 to 1048575,
        // but Flash does not seem to enforce that upper constraint. Instead
        // manually determined upper constraint is 2130690045.
        if (depth2 > (2130690045 + 16384))
            return;
    }
    else
    {
        // Need to search for target in our scope (environment scope might be different).
        if (psprite)
        {
            InteractiveObject* poldTarget = fn.Env->GetTarget();
            fn.Env->SetTarget(psprite);
            ptarget = fn.Env->FindTargetByValue(arg);
            fn.Env->SetTarget(poldTarget);
        }
        else
            ptarget = fn.Env->FindTargetByValue(arg); //???

        if (!ptarget || ptarget == pchar)
            return;         
        // Must have same parent.
        if (pparent != ptarget->GetParent())
            return;

        // Can we swap with text fields, etc? Yes.
        depth2 = ptarget->GetDepth();
    }

    // Flash doesn't allow to swap depths for "semi-dead" objects (i.e 
    // depth_in_flash < -16384, depth_in_gfx < -1).
    if (pchar->GetDepth() < 0)
        return;

    pchar->SetAcceptAnimMoves(0);
    // Do swap or depth change.
    if (pparent && pparent->SwapDepths(pchar->GetDepth(), depth2, pparent->GetCurrentFrame()))
    {
        pparent->SetDirtyFlag();
        if (ptarget)
            ptarget->SetAcceptAnimMoves(0);
    }
}

void AvmSprite::SpriteGetNextHighestDepth(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Depths is always at least 0. No upper bound constraint is done
    // (i.e. 2130690046 will be returned in Flash, although invalid).
    int depth = Alg::Max<int>(0, psprite->GetLargestDepthInUse() - 16384 + 1);
    fn.Result->SetInt(depth);
}

void AvmSprite::SpriteGetInstanceAtDepth(const FnCall& fn)
{
    fn.Result->SetUndefined();
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Note: this can also report buttons, etc.
    if (fn.NArgs >= 1)
    {
        DisplayObjectBase* pchar = psprite->GetCharacterAtDepth(((int)fn.Arg(0).ToNumber(fn.Env)) + 16384);
        if (pchar)
            fn.Result->SetAsCharacter(pchar->CharToInteractiveObject());
    }
}


//
// Returns a TextSnapshot object that contains the text in all the 
// static text fields in the specified movie clip; text in child movie 
// clips is not included. This method always returns a TextSnapshot 
// object.
//
void AvmSprite::SpriteGetTextSnapshot(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
    Ptr<TextSnapshotObject> ptextSnapshot = *SF_HEAP_NEW(fn.Env->GetHeap()) TextSnapshotObject(fn.Env);
    // Collect all static text chars
    ptextSnapshot->Process(psprite);
    // Return a TextSnapshot AS Object
    fn.Result->SetAsObject(ptextSnapshot);
#endif
}


void AvmSprite::SpriteGetSWFVersion(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    fn.Result->SetInt(psprite->GetVersion());
}

void AvmSprite::SpriteStartDrag(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    MovieImpl::DragState st;
    bool                    lockCenter = false;

    // If there are arguments, init appropriate values.
    if (fn.NArgs > 0)
    {
        lockCenter = fn.Arg(0).ToBool(fn.Env);
        if (fn.NArgs > 4)
        {
            st.Bound = true;
            RectF bounds;
            bounds.x1   = SF_PIXELS_TO_TWIPS((float) fn.Arg(1).ToNumber(fn.Env));
            bounds.y1   = SF_PIXELS_TO_TWIPS((float) fn.Arg(2).ToNumber(fn.Env));
            bounds.x2   = SF_PIXELS_TO_TWIPS((float) fn.Arg(3).ToNumber(fn.Env));
            bounds.y2   = SF_PIXELS_TO_TWIPS((float) fn.Arg(4).ToNumber(fn.Env));
            bounds.Normalize();
            st.BoundLT = bounds.TopLeft();
            st.BoundRB = bounds.BottomRight();
        }
    }

    st.pCharacter = psprite;
    // Init mouse offsets based on LockCenter flag.
    st.InitCenterDelta(lockCenter);

    // Begin dragging.
    MovieImpl* proot = psprite->GetMovieImpl();
    proot->SetDragState(st);  
    psprite->ModifyOptimizedPlayListLocal<Sprite>();
}


void AvmSprite::SpriteStopDrag(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Stop dragging.
    MovieImpl* proot = psprite->GetMovieImpl();
    proot->StopDrag(0);    
    psprite->ModifyOptimizedPlayListLocal<Sprite>();
}

void AvmSprite::SpriteLoadMovie(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs > 0)
    {
        LoadQueueEntry::LoadMethod lm = LoadQueueEntry::LM_None;

        // Decode load method argument.
        if (fn.NArgs > 1)
        {
            ASString str(fn.Arg(1).ToString(fn.Env).ToLower());
            if (str == "get")
                lm = LoadQueueEntry::LM_Get;
            else if (str == "post")
                lm = LoadQueueEntry::LM_Post;
        }

        // Post loadMovie into queue.
        ASString urlStr(fn.Arg(0).ToString(fn.Env));
        ToAS2Root(psprite)->AddLoadQueueEntry(psprite, urlStr.ToCStr(), lm);
    }
}

void AvmSprite::SpriteUnloadMovie(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    // Post unloadMovie into queue (empty url == unload).
    ToAS2Root(psprite)->AddLoadQueueEntry(psprite, "");
}

void AvmSprite::SpriteLoadVariables(const FnCall& fn)
{
    Sprite* psprite = AvmSprite::SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs > 0)
    {
        LoadQueueEntry::LoadMethod lm = LoadQueueEntry::LM_None;

        // Decode load method argument.
        if (fn.NArgs > 1)
        {
            ASString str(fn.Arg(1).ToString(fn.Env).ToLower());
            if (str == "get")
                lm = LoadQueueEntry::LM_Get;
            else if (str == "post")
                lm = LoadQueueEntry::LM_Post;
        }

        // Post loadMovie into queue.
        ASString urlStr(fn.Arg(0).ToString(fn.Env));
        ToAS2Root(psprite)->AddVarLoadQueueEntry(psprite, urlStr.ToCStr(), lm);
    }
}

// Drawing API
//=============================================================================
void AvmSprite::SetNoLine()
{
    DrawingContext* pdrawingAPI = GetDrawingContext();

    if (!pdrawingAPI->NoLine())
    {
        AcquirePath(false);
        pdrawingAPI->SetNoLine();
    }
}

void AvmSprite::SetNoFill()
{
    DrawingContext* pdrawingAPI = GetDrawingContext();
    AcquirePath(true);
    pdrawingAPI->SetNoFill();
}

void AvmSprite::SetLineStyle(float lineWidth, 
                          unsigned  rgba, 
                          bool  hinting, 
                          unsigned scaling, 
                          unsigned caps,
                          unsigned joins,
                          float miterLimit)
{
    DrawingContext* pdrawingAPI = GetDrawingContext();
    pdrawingAPI->ChangeLineStyle(PixelsToTwips(lineWidth), rgba, hinting, scaling, caps, joins, miterLimit);
}

// The function begins new fill with an "empty" style. 
// It returns the pointer to just created fill style, so the caller
// can set any values. The function is used in Action Script, beginGradientFill
Render::ComplexFill* AvmSprite::BeginFill()
{
    DrawingContext* pdrawingAPI = AcquirePath(true);
    Render::ComplexFill* f = pdrawingAPI->CreateNewComplexFill();
    pdrawingAPI->BeginFill();
    return f;
}

Render::ComplexFill* AvmSprite::CreateLineComplexFill()
{
    DrawingContext* pdrawingAPI = AcquirePath(true);
    return pdrawingAPI->CreateLineComplexFill();
}

void AvmSprite::BeginFill(unsigned rgba)
{
    DrawingContext* pdrawingAPI = AcquirePath(true);
    pdrawingAPI->BeginSolidFill(rgba);
}


void AvmSprite::BeginBitmapFill(FillType fillType,
                                ImageResource* pimageRes,
                                const Matrix& mtx)
{
    DrawingContext* pdrawingAPI = AcquirePath(true);
    pdrawingAPI->BeginBitmapFill(fillType, pimageRes, mtx);
}

void AvmSprite::EndFill()
{
    DrawingContext* pdrawingAPI = AcquirePath(true);
    pdrawingAPI->EndFill();
}

void AvmSprite::MoveTo(float x, float y)
{
    DrawingContext* pdrawingAPI = AcquirePath(false);
    pdrawingAPI->MoveTo(PixelsToTwips(x), 
        PixelsToTwips(y));
    GetSprite()->InvalidateHitResult();
    //FILE* fd = fopen("coord", "at");
    //fprintf(fd, "mc.moveTo(%.3f, %.3f);\n", x, y);
    //fclose(fd);
}

void AvmSprite::LineTo(float x, float y)
{
    DrawingContext* pdrawingAPI = GetDrawingContext();
    pdrawingAPI->LineTo(PixelsToTwips(x), 
        PixelsToTwips(y));
    GetSprite()->InvalidateHitResult();
    //FILE* fd = fopen("coord", "at");
    //fprintf(fd, "mc.lineTo(%.3f, %.3f);\n", x, y);
    //fclose(fd);
}

void AvmSprite::CurveTo(float cx, float cy, float ax, float ay)
{
    DrawingContext* pdrawingAPI = GetDrawingContext();
    pdrawingAPI->CurveTo(PixelsToTwips(cx), 
        PixelsToTwips(cy),
        PixelsToTwips(ax), 
        PixelsToTwips(ay));
    GetSprite()->InvalidateHitResult();
}

DrawingContext* AvmSprite::AcquirePath(bool newShapeFlag)
{
    DrawingContext* pdrawingAPI = GetDrawingContext();
    GetSprite()->SetDirtyFlag();
    GetSprite()->InvalidateHitResult();
    pdrawingAPI->AcquirePath(newShapeFlag);
    return pdrawingAPI;
}

void AvmSprite::SpriteClear(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (psprite->GetDrawingContext())
    {
        psprite->GetDrawingContext()->Clear();
    }
    psprite->InvalidateHitResult();
    psprite->SetDirtyFlag();
}

void AvmSprite::SpriteBeginFill(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if(fn.NArgs > 0) // rgb:Number
    {
        unsigned rgba = fn.Arg(0).ToUInt32(fn.Env) | 0xFF000000;
        if(fn.NArgs > 1) // alpha:Number
        {
            // Alpha is clamped to 0 to 100 range.
            float alpha = ((float)fn.Arg(1).ToNumber(fn.Env)) * 255.0f / 100.0f; 
            rgba &= 0xFFFFFF;            
            rgba |= unsigned(Alg::Clamp(alpha, 0.0f, 255.0f)) << 24;
        }
        GFx::AS2::ToAvmSprite(psprite)->BeginFill(rgba);
    }
    else
    {
        GFx::AS2::ToAvmSprite(psprite)->SetNoFill();
    }
}

void AvmSprite::SpriteCreateGradient(const FnCall& fn, Render::ComplexFill* complexFill)
{
    // beginGradientFill(fillType:String, 
    //                   colors:Array, 
    //                   alphas:Array, 
    //                   ratios:Array, 
    //                   matrix:Object, 
    //                   [spreadMethod:String], 
    //                   [interpolationMethod:String], 
    //                   [focalPointRatio:Number]) : Void
    Object* arg = 0;
    if (fn.NArgs > 0)
    {
        ASString fillTypeStr(fn.Arg(0).ToString(fn.Env));

        if (fn.NArgs > 1 && (arg = fn.Arg(1).ToObject(fn.Env)) != NULL &&
            arg->GetObjectType() == ObjectInterface::Object_Array)
        {
            const ArrayObject* colors = (const ArrayObject*)arg;
            if (fn.NArgs > 2 && (arg = fn.Arg(2).ToObject(fn.Env)) != NULL &&
                arg->GetObjectType() == ObjectInterface::Object_Array)
            {
                const ArrayObject* alphas = (const ArrayObject*)arg;
                if (fn.NArgs > 3 && (arg = fn.Arg(3).ToObject(fn.Env)) != NULL &&
                    arg->GetObjectType() == ObjectInterface::Object_Array)
                {
                    const ArrayObject* ratios = (const ArrayObject*)arg;
                    if (fn.NArgs > 4 && 
                        colors->GetSize() > 0 && 
                        colors->GetSize() == alphas->GetSize() &&
                        colors->GetSize() == ratios->GetSize())
                    {
                        Render::Matrix2F matrix;
                        Value v;
                        ASStringContext *psc = fn.Env->GetSC();

                        arg = fn.Arg(4).ToObject(fn.Env);

#ifdef GFX_AS2_ENABLE_MATRIX
                        if(arg->GetObjectType() == ObjectInterface::Object_Matrix)
                        {
                            matrix = ((MatrixObject*)arg)->GetMatrix(fn.Env);
                        }
                        else
#endif
                        {
                            if (arg->GetConstMemberRaw(psc, "matrixType",  &v) && 
                                v.ToString(fn.Env) == "box")
                            {
                                float x = 0;
                                float y = 0;
                                float w = 100;
                                float h = 100;
                                float r = 0;
                                if(arg->GetConstMemberRaw(psc, "x",  &v)) x = (float)v.ToNumber(fn.Env);
                                if(arg->GetConstMemberRaw(psc, "y",  &v)) y = (float)v.ToNumber(fn.Env);
                                if(arg->GetConstMemberRaw(psc, "w",  &v)) w = (float)v.ToNumber(fn.Env);
                                if(arg->GetConstMemberRaw(psc, "h",  &v)) h = (float)v.ToNumber(fn.Env);
                                if(arg->GetConstMemberRaw(psc, "r",  &v)) r = (float)v.ToNumber(fn.Env);

                                x += w / 2;
                                y += h / 2;
                                w *= GASGradientBoxMagicNumber; 
                                h *= GASGradientBoxMagicNumber;

                                matrix.AppendRotation(r);
                                matrix.AppendScaling(w, h);
                                matrix.AppendTranslation(x, y);
                            }
                            else
                            {
                                // The matrix is transposed:
                                // a(0,0)  d(0,1)  g(0,2)
                                // b(1,0)  e(1,1)  h(1,2)
                                // c(2,0)  f(2,1)  i(2,2)
                                // Elements c,f,i are not used
                                if(arg->GetConstMemberRaw(psc, "a",  &v)) matrix.Sx() = (float)v.ToNumber(fn.Env) * GASGradientBoxMagicNumber;
                                if(arg->GetConstMemberRaw(psc, "d",  &v)) matrix.Shx() = (float)v.ToNumber(fn.Env) * GASGradientBoxMagicNumber;
                                if(arg->GetConstMemberRaw(psc, "g",  &v)) matrix.Tx() = (float)v.ToNumber(fn.Env);
                                if(arg->GetConstMemberRaw(psc, "b",  &v)) matrix.Shy() = (float)v.ToNumber(fn.Env) * GASGradientBoxMagicNumber;
                                if(arg->GetConstMemberRaw(psc, "e",  &v)) matrix.Sy() = (float)v.ToNumber(fn.Env) * GASGradientBoxMagicNumber;
                                if(arg->GetConstMemberRaw(psc, "h",  &v)) matrix.Ty() = (float)v.ToNumber(fn.Env);
                            }
                        }

                        // int   spreadMethod    = 0; // pad
                        bool  linearRGB       = false;
                        float focalPointRatio = 0;
                        if (fn.NArgs > 5)
                        {
                            ASString str(fn.Arg(5).ToString(fn.Env));

                            // if(str == "reflect") spreadMethod = 1;
                            // else if(str == "repeat")  spreadMethod = 2;

                            if (fn.NArgs > 6)
                            {
                                linearRGB = fn.Arg(6).ToString(fn.Env) == "linearRGB";
                                if (fn.NArgs > 7)
                                {
                                    if (NumberUtil::IsNaN(focalPointRatio))
                                        focalPointRatio = 0;
                                    else if (focalPointRatio < -1) focalPointRatio = -1;
                                    else if (focalPointRatio >  1) focalPointRatio =  1;
                                }
                            }
                        }

                        // Create Gradient itself
                        Render::GradientType gradType = Render::GradientLinear;
                        if (fillTypeStr == "radial")
                        {
                            gradType = Render::GradientRadial;
                            if (focalPointRatio != 0) 
                                gradType = Render::GradientFocalPoint;
                        }
                        // Gradient data must live in a global heap since it is used as a key in ResourceLib.
                        complexFill->pGradient = *SF_NEW Render::GradientData(gradType, (UInt16)colors->GetSize(), linearRGB);

                        if (complexFill->pGradient)
                        {
                            complexFill->pGradient->SetFocalRatio(focalPointRatio);

                            for (int i = 0; i < colors->GetSize(); i++)
                            {
                                unsigned rgba = colors->GetElementPtr(i)->ToUInt32(fn.Env) | 0xFF000000;

                                // Alpha is clamped to 0 to 100 range.
                                float alpha = ((float)alphas->GetElementPtr(i)->ToNumber(fn.Env)) * 255.0f / 100.0f; 
                                rgba &= 0xFFFFFF;            
                                rgba |= unsigned(Alg::Clamp(alpha, 0.0f, 255.0f)) << 24;

                                float ratio = (float)ratios->GetElementPtr(i)->ToNumber(fn.Env); 
                                ratio = Alg::Clamp(ratio, 0.0f, 255.0f);

                                GradientRecord& record = complexFill->pGradient->At(i);
                                record.Ratio = (UByte)ratio;
                                record.ColorV = rgba;
                            }

                            matrix.Tx() = PixelsToTwips(matrix.Tx());
                            matrix.Ty() = PixelsToTwips(matrix.Ty());
                            complexFill->ImageMatrix.SetIdentity();

                            if (gradType == Render::GradientLinear)
                            {
                                complexFill->ImageMatrix.AppendScaling(1.0f / 32768.F);
                                complexFill->ImageMatrix.AppendTranslation(0.5F, 0.F);
                            }
                            else
                            {
                                complexFill->ImageMatrix.AppendScaling(1.0f / 32768.F);
                                complexFill->ImageMatrix.AppendTranslation(0.5F, 0.5F);
                                // gradient's square size is 32768x32768 (-16,384 - 16,384)
                            }
                            complexFill->ImageMatrix.Prepend(matrix.GetInverse());
                        }
                    }
                }
            }
        }
    }
}

void AvmSprite::SpriteBeginGradientFill(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    Render::ComplexFill* fillStyle = AS2::ToAvmSprite(psprite)->BeginFill();
    if (fillStyle)
        SpriteCreateGradient(fn, fillStyle);
}

void AvmSprite::SpriteLineGradientStyle(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    Render::ComplexFill* fillStyle = AS2::ToAvmSprite(psprite)->CreateLineComplexFill();
    if (fillStyle)
        SpriteCreateGradient(fn, fillStyle);
}

void AvmSprite::SpriteBeginBitmapFill(const FnCall& fn)
{
    SF_UNUSED(fn);
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    //beginBitmapFill(bmp:BitmapData, 
    //               [matrix:Matrix], 
    //               [repeat:Boolean], 
    //               [smoothing:Boolean]) : Void

    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs > 0) // bmp:BitmapData,
    {
        Ptr<Object> pobj = fn.Arg(0).ToObject(fn.Env);
        if (!pobj || pobj->GetObjectType() != Object::Object_BitmapData)
            return;

        BitmapData*    pbmpData = static_cast<BitmapData*>(pobj.GetPtr());
        ImageResource* pimageRes = pbmpData->GetImage();
        if (!pimageRes)
            return;

        Render::Matrix2F matrix;
        bool repeat = true;
        bool smoothing = false;

        if (fn.NArgs > 1) // [matrix:Matrix]
        {
#ifdef GFX_AS2_ENABLE_MATRIX
            Object* arg = fn.Arg(1).ToObject(fn.Env);
            if (arg && arg->GetObjectType() == ObjectInterface::Object_Matrix)
            {
                matrix = ((MatrixObject*)arg)->GetMatrix(fn.Env);
            }
#endif

            if (fn.NArgs > 2) // [repeat:Boolean]
            {
                repeat = fn.Arg(2).ToBool(fn.Env);
                if (fn.NArgs > 3) // [smoothing:Boolean]
                {
                    smoothing = fn.Arg(3).ToBool(fn.Env);
                }
            }
        }

        FillType fillType;

        if (smoothing)
        {
            if (repeat) fillType = Fill_TiledSmoothImage;
            else        fillType = Fill_ClippedSmoothImage;
        }
        else
        {
            if (repeat) fillType = Fill_TiledImage;
            else        fillType = Fill_ClippedImage;
        }        

        AS2::ToAvmSprite(psprite)->BeginBitmapFill(fillType, pimageRes, matrix);
    }
#else
    SF_DEBUG_WARNING(1, "Error: beginBitmapFill is failed since SF_NO_FXPLAYER_AS_BITMAPDATA is defined.");
#endif
}

void AvmSprite::SpriteEndFill(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    AS2::ToAvmSprite(psprite)->EndFill();
}

void AvmSprite::SpriteLineStyle(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs > 0) // thickness:Number
    {
        float           lineWidth = (float)(fn.Arg(0).ToNumber(fn.Env));
        unsigned        rgba    = 0xFF000000;
        bool            hinting = false;
        unsigned        scaling = Render::StrokeScaling_Normal;
        unsigned        caps    = Render::StrokeCap_Round | Render::StrokeEndCap_Round;
        unsigned        joins   = Render::StrokeJoin_Round;
        float           miterLimit = 3.0f;

        if (fn.NArgs > 1) // rgb:Number
        {
            rgba = fn.Arg(1).ToUInt32(fn.Env) | 0xFF000000;

            if (fn.NArgs > 2) // alpha:Number
            {
                float alpha = ((float)fn.Arg(2).ToNumber(fn.Env)) * 255.0f / 100.0f; 

                rgba &= 0xFFFFFF;                
                rgba |= unsigned(Alg::Clamp(alpha, 0.0f, 255.0f)) << 24;

                if (fn.NArgs > 3) // pixelHinting:Boolean
                {
                    hinting = fn.Arg(3).ToBool(fn.Env);

                    if (fn.NArgs > 4) // noScale:String
                    {
                        ASString str(fn.Arg(4).ToString(fn.Env));
                        if (str == "none")            scaling = Render::StrokeScaling_None;
                        else if (str == "vertical")   scaling = Render::StrokeScaling_Vertical;
                        else if (str == "horizontal") scaling = Render::StrokeScaling_Horizontal;

                        if (fn.NArgs > 5) // capsStyle:String
                        {
                            str = fn.Arg(5).ToString(fn.Env);
                            if (str == "none")        caps = Render::StrokeCap_None | Render::StrokeEndCap_None;
                            else if (str == "square") caps = Render::StrokeCap_Square | Render::StrokeEndCap_Square;

                            if (fn.NArgs > 6) // jointStyle:String
                            {
                                str = fn.Arg(6).ToString(fn.Env);
                                if (str == "miter")      joins = Render::StrokeJoin_Miter;
                                else if (str == "bevel") joins = Render::StrokeJoin_Bevel;

                                if (fn.NArgs > 7) // miterLimit:Number
                                {
                                    miterLimit = (float)(fn.Arg(7).ToNumber(fn.Env));
                                    if(miterLimit < 1.0f)   miterLimit = 1.0f;
                                    if(miterLimit > 255.0f) miterLimit = 255.0f;
                                }
                            }
                        }
                    }
                }
            }
        }
        AS2::ToAvmSprite(psprite)->SetLineStyle(lineWidth, rgba, hinting, scaling, caps, joins, miterLimit);
    }
    else
    {
        AS2::ToAvmSprite(psprite)->SetNoLine();
    }
}

void AvmSprite::SpriteMoveTo(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs >= 2) 
    {
        float x = (float)(fn.Arg(0).ToNumber(fn.Env));
        float y = (float)(fn.Arg(1).ToNumber(fn.Env));
        AS2::ToAvmSprite(psprite)->MoveTo(x, y);
    }
}

void AvmSprite::SpriteLineTo(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs >= 2) 
    {
        float x = (float)(fn.Arg(0).ToNumber(fn.Env));
        float y = (float)(fn.Arg(1).ToNumber(fn.Env));
        AS2::ToAvmSprite(psprite)->LineTo(x, y);
    }
}

void AvmSprite::SpriteCurveTo(const FnCall& fn)
{
    Sprite* psprite = SpriteGetTarget(fn);
    if (psprite == NULL) return;

    if (fn.NArgs >= 4) 
    {
        float cx = (float)(fn.Arg(0).ToNumber(fn.Env));
        float cy = (float)(fn.Arg(1).ToNumber(fn.Env));
        float ax = (float)(fn.Arg(2).ToNumber(fn.Env));
        float ay = (float)(fn.Arg(3).ToNumber(fn.Env));
        AS2::ToAvmSprite(psprite)->CurveTo(cx, cy, ax, ay);
    }
}

void AvmSprite::AddActionBuffer(ActionBuffer* a, ActionPriority::Priority prio) 
{
    MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction(prio);
    if (pe) pe->SetAction(GetSprite(), a);
}

void AvmSprite::FillTabableArray(InteractiveObject::FillTabableParams* params)
{
    UPInt n = GetDisplayList().GetCount();
    if (n == 0)
        return;

    if (!TabChildren.IsDefined())
    {
        if (!params->TabChildrenInProto.IsDefined())
        {
            Object* pproto = Get__proto__();
            if (pproto)
            {
                // check prototype for tabChildren
                Value val;
                if (pproto->GetMemberRaw(ASEnvironment.GetSC(), ASEnvironment.CreateConstString("tabChildren"), &val))
                {
                    if (!val.IsUndefined())
                        params->TabChildrenInProto = val.ToBool(&ASEnvironment);
                }
            }
        }
    }
    if (TabChildren.IsFalse() || params->TabChildrenInProto.IsFalse()) return;

    DisplayObjectBase*   ch;

    for (UPInt i = 0; i < n; i++)
    {
        ch = GetDisplayList().GetDisplayObject(i);
        if (ch != NULL && ch->IsInteractiveObject() && ch->GetVisible())
        {
            InteractiveObject* asch = ch->CharToInteractiveObject_Unsafe();
            if (asch->IsTabIndexed() && !params->TabIndexed)
            {
                // the first char with tabIndex: release the current array and
                // start to fill it again according to tabIndex.
                params->Array->Clear();
                params->TabIndexed = true;
            }
            // Append for now; if tabIndexed - later it will be sorted by tabIndex
            // Note, we might add focusEnabled characters as well; we are doing this only for
            // 'moveFocus' extension: for regular focus handling these characters will
            // be ignored, if IsTabable returns false.
            if ((asch->IsTabable() || (params->InclFocusEnabled && 
                asch->IsFocusEnabled(GFx_FocusMovedByKeyboard))) && 
                (!params->TabIndexed || asch->IsTabIndexed()))
                params->Array->PushBack(asch);
            if (asch->IsDisplayObjContainer())
            {
                asch->CharToDisplayObjContainer_Unsafe()->FillTabableArray(params);
            }
        }
    }
}

// Movie Loading support
bool AvmSprite::ReplaceChildCharacter(InteractiveObject *poldChar, InteractiveObject *pnewChar)
{
    SF_ASSERT(poldChar != 0);
    SF_ASSERT(pnewChar != 0);
    SF_ASSERT(poldChar->GetParent() == GetSprite());

    poldChar->SetMask(NULL);
    if (poldChar->HasIndirectTransform())
        GetMovieImpl()->RemoveTopmostLevelCharacter(poldChar);
    
    // Get the display entry.
    UPInt index = GetDisplayList().GetDisplayIndex(poldChar->GetDepth());
    if (index == SF_MAX_UPINT)
        return 0;

    // Copy physical properties & re-link all ActionScript references.
    pnewChar->CopyPhysicalProperties(poldChar);
    // For Sprites, we also copy _lockroot.
    if (pnewChar->IsSprite() && poldChar->IsSprite())
    {
        pnewChar->CharToSprite_Unsafe()->SetLockRoot(
            poldChar->CharToSprite_Unsafe()->IsLockRoot());
    }

    // Inform old character of unloading.
    // Use OnUnloading instead of OnEventUnload to fire onUnload events.
    poldChar->OnUnloading(); 
    GetAS2Root()->DoActions();

    pnewChar->MoveNameHandle(poldChar);

    // Alter the display list.
    // Need to get the index again since it might be changed because of DoActions.
    index = GetDisplayList().GetDisplayIndex(poldChar->GetDepth());
    if (index == SF_MAX_UPINT)
        return 0;
    GetDisplayList().ReplaceDisplayObjectAtIndex(GetSprite(), index, pnewChar);
    return true;
}

bool AvmSprite::ReplaceChildCharacterOnLoad(InteractiveObject *poldChar, InteractiveObject *pnewChar)
{
    if (!ReplaceChildCharacter(poldChar, pnewChar))
        return false;

    // And call load.
    pnewChar->OnEventLoad();
    GetAS2Root()->DoActions();
    return true;
}

bool AvmSprite::MustBeInPlaylist() const
{
    return HasEventHandler(EventId::Event_EnterFrame);
}

//////////////////////////////////////////
void MovieClipObject::commonInit ()
{
    ButtonEventMask         = 0;
    HasButtonHandlers       = 0;
}

UInt16   MovieClipObject::GetButtonEventNameMask(ASStringContext* psc, const ASString& name)
{
    if (psc->GetBuiltin(ASBuiltin_onPress) == name)
        return Mask_onPress;
    else if (psc->GetBuiltin(ASBuiltin_onRelease) == name)
        return Mask_onRelease;
    else if (psc->GetBuiltin(ASBuiltin_onReleaseOutside) == name)
        return Mask_onReleaseOutside;
    else if (psc->GetBuiltin(ASBuiltin_onRollOver) == name)
        return Mask_onRollOver;
    else if (psc->GetBuiltin(ASBuiltin_onRollOut) == name)
        return Mask_onRollOut;
    else if (psc->GetBuiltin(ASBuiltin_onDragOver) == name)
        return Mask_onDragOver;
    else if (psc->GetBuiltin(ASBuiltin_onDragOut) == name)
        return Mask_onDragOut;

    else if (psc->GetBuiltin(ASBuiltin_onPressAux) == name)
        return Mask_onPressAux;
    else if (psc->GetBuiltin(ASBuiltin_onReleaseAux) == name)
        return Mask_onReleaseAux;
    else if (psc->GetBuiltin(ASBuiltin_onReleaseOutsideAux) == name)
        return Mask_onReleaseOutsideAux;
    else if (psc->GetBuiltin(ASBuiltin_onDragOverAux) == name)
        return Mask_onDragOverAux;
    else if (psc->GetBuiltin(ASBuiltin_onDragOutAux) == name)
        return Mask_onDragOutAux;

    return 0;
}

// Updates DynButtonHandlerCount if name is begin added through SetMember or
// deleted through DeleteMember.
void    MovieClipObject::TrackMemberButtonHandler(ASStringContext* psc, const ASString& name, bool deleteFlag)
{
    // If we are one of button handlers, do the check.
    if (name.GetSize() > 2 && name[0] == 'o' && name[1] == 'n')
    {
        Value val;
        //!if (ASEnvironment.GetMember(name, &val))
        if (GetMemberRaw(psc, name, &val))
        {
            // Set and member already exists? No tracking increment.
            if (!deleteFlag)
                return;
        }
        else
        {
            // Delete and does not exist? No decrement.
            if (deleteFlag)
                return;
        }
        // Any button handler enables button behavior.
        UInt16 bemask;
        if ((bemask = GetButtonEventNameMask(psc, name)) != 0)
        {
            if (deleteFlag)
                ButtonEventMask &= ~bemask;
            else
                ButtonEventMask |= bemask;
        }
    }
}

void MovieClipObject::Set__proto__(ASStringContext *psc, Object* protoObj) 
{   
    Object::Set__proto__(psc, protoObj);
    // proto can be NULL, when we reset the proto in OnEventUnload
    //SF_ASSERT(protoObj);
    if (protoObj && protoObj->GetObjectType() != Object_MovieClipObject)
    {
        //AB: if proto is being set to the sprite, and this proto is not 
        // an instance of MovieClipObject then we need to rescan it for button's
        // handlers to react on events correctly. This usually happen if 
        // component is extending MovieClip as a class and defines
        // buttons' handler as methods.
        struct MemberVisitor : ObjectInterface::MemberVisitor
        {
            Ptr<MovieClipObject> obj;
            ASStringContext*        pStringContext;

            MemberVisitor(ASStringContext* psc, MovieClipObject* _obj)
                : obj(_obj), pStringContext(psc) { }

            virtual void Visit(const ASString& name, const Value& val, UByte flags)
            {
                SF_UNUSED2(val, flags);
                if (IsButtonEventName(pStringContext, name))
                {
                    obj->SetHasButtonHandlers(true);
                }
            }
        } visitor (psc, this);
        pProto->VisitMembers(psc, &visitor, 
            ObjectInterface::VisitMember_Prototype|
            ObjectInterface::VisitMember_DontEnum | 
            ObjectInterface::VisitMember_NamesOnly);
    }
}

bool MovieClipObject::ActsAsButton() const   
{ 
    if ((ButtonEventMask != 0) || HasButtonHandlers)
        return true;
    for(Object* pproto = pProto; pproto != 0; pproto = pproto->Get__proto__())
    {
        if (pProto->GetObjectType() == Object_MovieClipObject) 
        {
            // MA: No need to AddRef to 'pproto'.
            MovieClipProto* proto = static_cast<MovieClipProto*>(pproto);
            return proto->ActsAsButton();
        }
    }
    return false;
}

void    MovieClipObject::SetMemberCommon(ASStringContext *psc, 
                                         const ASString& name, 
                                         const Value& val)
{
    // _levelN can't function as a button
    // MovieClip.prototype has no sprite (GetSprite() returns 0) but
    // it should track button handlers.
    Ptr<Sprite> spr = GetSprite();
    if (!spr || spr->GetTopParent() != spr)
        TrackMemberButtonHandler(psc, name);

    // Intercept the user data properties and cache them in pSprite
    if (spr && name.IsBuiltin())
    {
        AvmSprite* avmSpr = GFx::AS2::ToAvmSprite(spr);
        if (name == psc->GetBuiltin(ASBuiltin_rendererString))
        {
            spr->SetRendererString(val.ToString(avmSpr->GetASEnvironment()).ToCStr());
        }
        else if (name == psc->GetBuiltin(ASBuiltin_rendererFloat))
        {
            spr->SetRendererFloat((float)val.ToNumber(avmSpr->GetASEnvironment()));
        }
        else if (name == psc->GetBuiltin(ASBuiltin_disableBatching))
        {
            spr->DisableBatching(val.ToBool(avmSpr->GetASEnvironment()));
        }
//         else if (name == psc->GetBuiltin(ASBuiltin_rendererMatrix))
//         {
//             Ptr<Object> pObj = val.ToObject(avmSpr->GetASEnvironment());
//             if (pObj && pObj->GetObjectType() == ObjectInterface::Object_Array)
//             {
//                 ArrayObject* pArray = (ArrayObject*)pObj.GetPtr();
//                 float    f[16];
//                 unsigned size = Alg::Min(16, pArray->GetSize());
//                 for (unsigned i = 0; i < size; i++)
//                     f[i] = (float)pArray->GetElementPtr(i)->ToNumber(avmSpr->GetASEnvironment());
//                 spr->SetRendererMatrix(f, size);
//             }
//         }
    }
}

// Set the named member to the value.  Return true if we have
// that member; false otherwise.
// Note, we need to define both - SetMember and SetMemberRaw, since both of them
// might be called independently. So, moved common part to SetMemberCommon.
bool    MovieClipObject::SetMemberRaw(ASStringContext *psc, 
                                         const ASString& name, 
                                         const Value& val, 
                                         const PropFlags& flags)
{
    SetMemberCommon(psc, name, val);
    return Object::SetMemberRaw(psc, name, val, flags);
}

bool    MovieClipObject::SetMember(Environment *penv, 
                                      const ASString& name, 
                                      const Value& val, 
                                      const PropFlags& flags)
{
    SetMemberCommon(penv->GetSC(), name, val);
    return Object::SetMember(penv, name, val, flags);
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
/*bool    MovieClipObject::GetMember(Environment* penv, const GASString& name, Value* val)
{
return Object::GetMember(penv, name, val);
}*/

// Delete a member field, if not read-only. Return true if deleted.
bool    MovieClipObject::DeleteMember(ASStringContext *psc, const ASString& name)
{
    TrackMemberButtonHandler(psc, name, 1);
    return Object::DeleteMember(psc, name);
}

void    MovieClipObject::ForceShutdown ()
{
    // traverse through all members, look for FUNCTION and release local frames.
    // Needed to prevent memory leak until we don't have garbage collection.
#ifndef GFX_AS_ENABLE_GC
    for (MemberHash::Iterator iter = Members.Begin(); iter != Members.End(); ++iter)
    {        
        Value& value = iter->Second.Value;
        if (value.IsFunction ())
        {
            value.V.FunctionValue.SetLocalFrame (0);
        }
    }
#endif
}

//using GFx::AS2::AvmSprite; //@TEMP!
//using GFx::AS2::AvmCharacter;

static const NameFunction MovieClipFunctionTable[] = 
{
    { "play",                   &AvmSprite::SpritePlay },
    { "stop",                   &AvmSprite::SpriteStop },
    { "gotoAndStop",            &AvmSprite::SpriteGotoAndStop },
    { "gotoAndPlay",            &AvmSprite::SpriteGotoAndPlay },
    { "nextFrame",              &AvmSprite::SpriteNextFrame },
    { "prevFrame",              &AvmSprite::SpritePrevFrame },
    { "getBytesLoaded",         &AvmSprite::SpriteGetBytesLoaded },
    { "getBytesTotal",          &AvmSprite::SpriteGetBytesTotal },

    { "getBounds",              &AvmSprite::SpriteGetBounds },
    { "getRect",                &AvmSprite::SpriteGetRect },
    { "localToGlobal",          &AvmSprite::SpriteLocalToGlobal },
    { "globalToLocal",          &AvmSprite::SpriteGlobalToLocal },
    { "hitTest",                &AvmSprite::SpriteHitTest },

    { "attachBitmap",           &AvmSprite::SpriteAttachBitmap },
    { "attachMovie",            &AvmSprite::SpriteAttachMovie },
#if defined(GFX_ENABLE_SOUND) && defined(GFX_ENABLE_VIDEO)
    { "attachAudio",            &AvmSprite::SpriteAttachAudio },
#endif
    { "duplicateMovieClip",     &AvmSprite::SpriteDuplicateMovieClip },
    { "removeMovieClip",        &AvmSprite::SpriteRemoveMovieClip },
    { "createEmptyMovieClip",   &AvmSprite::SpriteCreateEmptyMovieClip },
    { "createTextField",        &AvmSprite::SpriteCreateTextField },

    { "getDepth",               &AvmCharacter::CharacterGetDepth },
    { "swapDepths",             &AvmSprite::SpriteSwapDepths },
    { "getNextHighestDepth",    &AvmSprite::SpriteGetNextHighestDepth },
    { "getInstanceAtDepth",     &AvmSprite::SpriteGetInstanceAtDepth },

    { "getTextSnapshot",        &AvmSprite::SpriteGetTextSnapshot },

    { "getSWFVersion",          &AvmSprite::SpriteGetSWFVersion },

    { "startDrag",              &AvmSprite::SpriteStartDrag },
    { "stopDrag",               &AvmSprite::SpriteStopDrag },

    { "setMask",                &AvmSprite::SpriteSetMask },

    { "loadMovie",              &AvmSprite::SpriteLoadMovie },
    { "unloadMovie",            &AvmSprite::SpriteUnloadMovie },

    { "loadVariables",          &AvmSprite::SpriteLoadVariables },

    // Drawing API

    { "clear",                  &AvmSprite::SpriteClear     },
    { "beginFill",              &AvmSprite::SpriteBeginFill },
    { "beginGradientFill",      &AvmSprite::SpriteBeginGradientFill },
    { "beginBitmapFill",        &AvmSprite::SpriteBeginBitmapFill },
    { "lineGradientStyle",      &AvmSprite::SpriteLineGradientStyle },
    { "endFill",                &AvmSprite::SpriteEndFill   },
    { "lineStyle",              &AvmSprite::SpriteLineStyle },
    { "moveTo",                 &AvmSprite::SpriteMoveTo    },
    { "lineTo",                 &AvmSprite::SpriteLineTo    },
    { "curveTo",                &AvmSprite::SpriteCurveTo   },

    { 0, 0 }
};

MovieClipProto::MovieClipProto(ASStringContext* psc, Object* prototype, const FunctionRef& constructor) :
Prototype<MovieClipObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, MovieClipFunctionTable);
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_useHandCursor), Value(true), 
        PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

Object* MovieClipCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) MovieClipObject(penv);
}

void MovieClipCtorFunction::GlobalCtor(const FnCall& fn) 
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_MovieClipObject)
    {
        MovieClipObject* pobj = static_cast<MovieClipObject*>(fn.ThisPtr);
        fn.Result->SetAsObject(pobj);
    }
    else
        fn.Result->SetUndefined();
}

} // SF
} // AS2
} // GFx


