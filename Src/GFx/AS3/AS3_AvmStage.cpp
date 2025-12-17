/**************************************************************************

Filename    :   AS3_AvmStage.cpp
Content     :   Implementation of AS3-dependent part of Stage.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS3/AS3_AvmStage.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "Kernel/SF_MsgFormat.h"
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
#include "Obj/Events/AS3_Obj_Events_StageOrientationEvent.h"
#include "Obj/Events/AS3_Obj_Events_AppLifecycleEvent.h"
#endif
#include "Obj/Display/AS3_Obj_Display_LoaderInfo.h"

namespace Scaleform { namespace GFx { namespace AS3 {

class FrameCounter : public InteractiveObject
{
    Ptr<DisplayObjContainer>    pRoot;
public:
    FrameCounter(MovieDefImpl* pbindingDefImpl,
                 ASMovieRootBase* pasRoot,
                 InteractiveObject* pparent, 
                 ResourceId id)
        : InteractiveObject(pbindingDefImpl, pasRoot, pparent, id)
    { 
    }
    ~FrameCounter() {}

    virtual int             CheckAdvanceStatus(bool playingNow) 
    { 
        SF_UNUSED(playingNow); 
        return 1; 
    }
    virtual void            AdvanceFrame(bool nextFrame, float framePos = 0.0f); 
    virtual unsigned        GetLoadingFrame() const { return 1; }
    virtual CharacterDef*   GetCharacterDef() const { return NULL; }

    DisplayObjContainer*    GetRoot()
    {
        return pRoot;
    }
    void                    SetRoot(DisplayObjContainer* proot)
    {
        pRoot = proot;
    }

    virtual void            ForceShutdown () {}
    void                    AddScriptableMovieClip(Sprite* pspr);

    void                    QueueFrameActions();
protected:
    void    AddChildsFrameExecutionSnapshot(
        DisplayObjContainer* pcont);
};

void FrameCounter::AdvanceFrame(bool nextFrame, float framePos) 
{
    SF_AMP_SCOPE_TIMER(GetMovieImpl()->AdvanceStats, "FrameCounter::AdvanceFrame", Amp_Profile_Level_Medium);

    SF_UNUSED(framePos);
    MovieRoot* asroot = ToAS3Root(GetMovieImpl());
    if (nextFrame)
    {
        // need to make sure this is the last character in the playlist
        // since it will basically execute all AS for the frame.
        SF_ASSERT(!pPlayNext);
#ifdef GFX_TRACE_DIPLAYLIST_EVERY_FRAME
        GetMovieImpl()->GetMainContainer()->CharToDisplayObjContainer_Unsafe()->DumpDisplayList(0, "");
#endif
        // need to execute constructors before queueing up the frames:
        // ctors might call addFrameScript.
        asroot->ExecuteCtors();

        asroot->ExecuteActionQueue(MovieRoot::AL_Highest);
        asroot->ExecuteActionQueue(MovieRoot::AL_High);

        // traverse through playlist backward, starting from the stage 
        // ("this") and queue up frames
        QueueFrameActions();

        // reinsert entries from AL_Manual to AL_Frame. This is necessary
        // to execute COMPLETE events for Loader.
        asroot->RequeueActionQueue(MovieRoot::AL_Manual, MovieRoot::AL_Frame);
    }
}

void FrameCounter::QueueFrameActions()
{
    MovieRoot* asroot = ToAS3Root(GetMovieImpl());
    // queue up FRAME_CONSTRUCTED
    asroot->QueueEventChain(EventId::Event_FrameConstructed);

    // traverse through playlist backward, starting from the stage 
    // ("this") and queue up frames
    if (GetMovieImpl()->IsOptAdvanceListInvalid())
    {
        // we can't use optimized playlist since it is invalid.
        for (InteractiveObject* cur = pPlayPrev; cur; cur = cur->pPlayPrev)
        {
            ToAvmDisplayObj(cur)->SetFirstFrameCalledFlag();
            if (cur->IsSprite())
            {
                AvmSprite* aspr = ToAvmSprite(cur->CharToSprite_Unsafe());
                if (aspr->IsNeedExecuteFrameFlagSet())
                {
                    aspr->QueueFrameScript(aspr->GetCurrentSprite());
                    aspr->SetNeedExecuteFrameFlag(false);
                }
            }
        }
    }
    else
    {
        // we can use optimized playlist, should be much faster
        for (InteractiveObject* cur = pPlayPrevOpt; cur; cur = cur->pPlayPrevOpt)
        {
            SF_ASSERT(cur->IsOptAdvListFlagSet());
            ToAvmDisplayObj(cur)->SetFirstFrameCalledFlag();
            if (cur->IsSprite())
            {
                AvmSprite* aspr = ToAvmSprite(cur->CharToSprite_Unsafe());
                if (aspr->IsNeedExecuteFrameFlagSet())
                {
                    aspr->QueueFrameScript(aspr->GetCurrentSprite());
                    aspr->SetNeedExecuteFrameFlag(false);
                }
            }
        }
    }

    // queue up FRAME_EXIT
    asroot->QueueEventChain(EventId::Event_ExitFrame);
}

Stage::Stage (MovieDefImpl* pbindingDefImpl, 
              ASMovieRootBase* pasRoot,
              InteractiveObject* pparent, 
              ResourceId id) : 
DisplayObjContainer(pbindingDefImpl, pasRoot, pparent, id),
CurrentStageOrientation(GetStringManager()->CreateEmptyString()),
MouseCursorEventCnt(0)
{
    SetLoaded(); // mark as loaded to call OnEventUnload
    SetTimelineObjectFlag(true);
}

Stage::~Stage()
{
}

void Stage::OnInsertionAsLevel(int level)
{
    FrameCounterObj = *SF_HEAP_AUTO_NEW(this) FrameCounter(
        pDefImpl, GetASMovieRoot(), NULL, ResourceId(0));
    FrameCounterObj->AddToPlayList();
    DisplayObjContainer::OnInsertionAsLevel(level);
}

void Stage::ForceShutdown ()
{
    FrameCounterObj->RemoveFromPlayList();
    ClearDisplayList();
    if (pRoot) pRoot->ForceShutdown();
    pRoot = NULL;
    DisplayObjContainer::ForceShutdown();
}

// initialize the stage
void Stage::ExecuteFrame0Events()
{
    SF_AMP_SCOPE_TIMER(GetMovieImpl()->AdvanceStats, "Stage::ExecuteFrame0Events", Amp_Profile_Level_Medium);
    AS3::MovieRoot* pas3Root = ToAS3Root(GetMovieImpl());

    SF_ASSERT(GetNumChildren() == 0);

    // Create a root movieclip
    Ptr<Sprite> prootMovie = *static_cast<MovieRoot*>(GetASMovieRoot())->
        CreateMovieClip(pDefImpl->GetDataDef(), pDefImpl, this, ResourceId(0), true);

    if (!prootMovie)
    {
        return ;
    }
    prootMovie->SetLoaded(); // mark as loaded to call OnEventUnload

    // Flash behaves like mouseEnabled for root is set to false;
    // however, it traces this property set to "true". 
    // Need some more investigations here.
    prootMovie->SetMouseDisabledFlag(true);

    /*// run its init actions (DoAbc tags)
    TimelineDef::Frame initActionsFrame;        
    if (pDefImpl->GetDataDef()->GetInitActions(&initActionsFrame, 0) &&
        initActionsFrame.GetTagCount() > 0)
    {
        // Need to execute these actions.
        for (unsigned i= 0; i < initActionsFrame.GetTagCount(); i++)
        {
            ExecuteTag*  e = initActionsFrame.GetTag(i);
            e->Execute(prootMovie);
        }
    }*/

    prootMovie->ExecuteInitActionFrameTags(0);

    // add the root movie as a child of the stage. OnEventLoad will
    // call CreateASInstance for the root movie.
    //mDisplayList.AddEntryAtIndex(0, prootMovie);
    prootMovie->SetName(GetMovieImpl()->GetStringManager()->CreateConstString("root1"));
    prootMovie->SetTimelineObjectFlag(true);
    SetRoot(prootMovie);
   
    // Allocate a default PerspectiveDataType object and set fieldOfView on the root and stage.
    SetFOV(DEFAULT_FLASH_FOV);
    prootMovie->SetFOV(DEFAULT_FLASH_FOV);

    //AddChild(prootMovie);
    Ptr<Render::TreeContainer> node = GetRenderContainer();
    mDisplayList.AddEntryAtIndex(this, unsigned(mDisplayList.GetCount()), prootMovie);
    GetMovieImpl()->GetRenderRoot()->Insert(0, node);

    prootMovie->SetParent(this);
    prootMovie->AddToPlayList();

    // create AS3 instance of Stage
    pas3Root->IncASFramesCnt(ToAvmDisplayObj(this)->CreateASInstance(false));

    // Create LoaderInfo obj for stage
    SPtr<AS3::Object> _class = pas3Root->GetAVM()->GetClass("flash.display.LoaderInfo", pas3Root->GetAVM()->GetCurrentAppDomain());
    if (_class)  
    {
        AS3::Instances::fl_display::DisplayObject* as3obj = ToAvmDisplayObj(this)->GetAS3Obj();
        SPtr<Instances::fl_display::LoaderInfo> ploaderInfo;
        if (pas3Root->GetAVM()->ConstructInstance(ploaderInfo, _class) && as3obj)
        {
            as3obj->SetLoaderInfo(ploaderInfo);
            ploaderInfo->SetContent(as3obj);
        }
        // @TODO: error handling
    }

    // create AS3 instance of root
    ToAvmDisplayObj(prootMovie)->CreateASInstanceNoCtor();
    // Create LoaderInfo obj for root
    if (_class)  
    {
        AS3::Instances::fl_display::DisplayObject* as3obj = ToAvmDisplayObj(prootMovie)->GetAS3Obj();
        SPtr<Instances::fl_display::LoaderInfo> ploaderInfo;
        if (pas3Root->GetAVM()->ConstructInstance(ploaderInfo, _class) && as3obj)
        {
            as3obj->SetLoaderInfo(ploaderInfo);
            ploaderInfo->SetContent(as3obj);
        }
        // @TODO: error handling
    }
    //pas3Root->IncASFramesCnt(ToAvmDisplayObj(prootMovie)->CallCtor(false));
    ToAvmDisplayObj(prootMovie)->CallCtor(true);

#ifndef SF_NO_IME_SUPPORT
	// we need because the ASEnvironment for the root movie is created during the first 
	// advance in AS3. This means that the function injection that is performed during
	// the SetFocus HandleEvent  call for IME handling must wait until this point. 
	//IMEManagerBase* pimeManager = GetMovieImpl()->GetIMEManager();
    //if (pimeManager) {
	//    pimeManager->ASRootMovieCreated(prootMovie);
    //}
#endif
    // Children might be already created by CreateASInstance(false).
    if (!ToAvmSprite(prootMovie)->IsChildrenCreatedFlagSet())
        prootMovie->ExecuteFrame0Events();

    DisplayObjContainer::ExecuteFrame0Events();

    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_Highest);
    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_High);
    ToAvmSprite(prootMovie)->SetChildrenCreatedFlag();

    while (pas3Root->ASFramesToExecute > 0)
    {
        unsigned cnt = pas3Root->ASFramesToExecute;
        pas3Root->ASFramesToExecute = 0;
        pas3Root->pAVM->ExecuteCode(cnt);  
        if (pas3Root->pAVM->IsException())
        {
            pas3Root->pAVM->OutputAndIgnoreException();

            // not sure what to mark as 'execution aborted' here. 
            // At the moment just marking the FrameCounter (thus, execution will
            // be completely stopped) (!AB).
            FrameCounterObj->SetExecutionAborted();
        }
    }

    // initiate added_to_stage for root
    if (prootMovie->GetParent())
        ToAvmSprite(prootMovie)->OnAdded(true);

    // need to force frame script execution here, since
    // AvmSprite::ExecuteFrameTags didn't do this for MainTimeline
    // sprite due to delayed initialization of its class.
    //ToAvmSprite(prootMovie)->QueueFrameScript(0);
    GetMovieImpl()->InvalidateOptAdvanceList();

    FrameCounterObj->AdvanceFrame(true, 0); 
    //AdvanceFrame(true, 0); 
    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_Frame);
}

void Stage::ExecuteFrameTags(unsigned )
{
}

// For AS3 we don't need to recursively propagate mouse event,
// Just call OnEvent for topmost entity.
void    Stage::PropagateMouseEvent(const EventId& id)
{
    Ptr<DisplayObjContainer> thisHolder(this); // PropagateMouseEvent may release "this"; preventing.

    if (id.Id == EventId::Event_MouseMove)
    {
        // Adjust x,y of this character if it is being dragged.
        if (GetMovieImpl()->IsMouseSupportEnabled())
            DisplayObjContainer::DoMouseDrag(id.MouseIndex);
    }
/*
    // Handling mouse events for stage is tricky. Stage gets mouse events
    // only if nobody else gets the event.
    MouseState* ms = GetMovieImpl()->GetMouseState(id.MouseIndex);
    SF_ASSERT(ms);
    //?if (!ms->GetTopmostEntity())
    {
        // We need to check if there is a MouseMove event handler installed on
        // stage to avoid extra work every mouse move.
        GetAvmIntObj()->OnEvent(id);
    }*/
}

ASString Stage::CreateNewInstanceName() const
{
    MovieImpl* proot = GetMovieImpl();
    return ASString(proot->GetStringManager()->CreateEmptyString());
}

void  Stage::QueueFrameActions()
{
    static_cast<FrameCounter*>(FrameCounterObj.GetPtr())->QueueFrameActions();
}

#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
void Stage::OnAppLifecycleEvent(const char* event)
{
    ASString eventStr = GetStringManager()->CreateString(event);
    Instances::fl_display::DisplayObject* as3obj = ToAvmDisplayObj(this)->GetAS3Obj();
    SF_ASSERT(as3obj != NULL);
    if (as3obj)
    {
        SPtr<Instances::fl_events::AppLifecycleEvent> evt = 
            as3obj->CreateAppLifecycleEventObject(eventStr, true, false, eventStr);

        as3obj->Dispatch(evt.GetPtr(), this);
    }
}

// newOrientation is "default", "rotatedLeft", "rotatedRight", "unknown" or "upsideDown"
void Stage::OnDeviceOrientationChange(const char* neworientation)
{
    ASString newOr = GetStringManager()->CreateString(neworientation);
    ASString oldOr = CurrentStageOrientation;
    Instances::fl_display::DisplayObject* as3obj = ToAvmDisplayObj(this)->GetAS3Obj();
    SF_ASSERT(as3obj);
    if (as3obj)
    {
        SPtr<Instances::fl_events::StageOrientationEvent> evt = 
            as3obj->CreateStageOrientationEventObject(GetStringManager()->CreateConstString("orientationChanging"), 
                                                       true, true, CurrentStageOrientation, newOr);

        as3obj->Dispatch(evt.GetPtr(), this);
        if (!evt->IsDefaultPrevented())
        {
            // update orientation
            SetOrientation(newOr);

            SPtr<Instances::fl_events::StageOrientationEvent> evt = 
                as3obj->CreateStageOrientationEventObject(GetStringManager()->CreateConstString("orientationChanged"), 
                                                           true, false, oldOr, newOr);

            as3obj->Dispatch(evt.GetPtr(), this);
        }
    }
}
#endif

void Stage::SetOrientation(const ASString& newOrient)
{
    CurrentStageOrientation = newOrient;
}

// methods to be redirected to root movie
unsigned        Stage::GetCurrentFrame() const
{
    if (pRoot)
        return pRoot->GetCurrentFrame();
    return 0;
}

bool            Stage::GetLabeledFrame(const char* plabel, unsigned* frameNumber, bool translateNumbers)
{
    if (pRoot)
        return pRoot->GetLabeledFrame(plabel, frameNumber, translateNumbers);
    return false;
}

const String*   Stage::GetFrameLabel(unsigned fr, unsigned* pdestfr)
{
    if (pRoot)
        return pRoot->GetFrameLabel(fr, pdestfr);
    return NULL;
}

void            Stage::GotoFrame(unsigned targetFrameNumber)
{
    if (pRoot)
        return pRoot->GotoFrame(targetFrameNumber);
}

//////////////////////////////////////////////////////////////////////////
AvmStage::~AvmStage() 
{

}

void AvmStage::OnEventLoad()
{
//    GetAS3Root()->IncASFramesCnt(CreateASInstance(false));
    //AvmDisplayObjContainer::OnEventLoad();
    //static_cast<Stage*>(pDispObj)->AdvanceFrame(true, 0);
}

void AvmStage::OnEventUnload()
{
    AvmDisplayObjContainer::OnEventUnload();
}

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
bool AvmStage::OnEvent(const EventId& id)
{
    switch (id.Id)
    {
    case EventId::Event_GamePadAnalogChange:
        if (HasAS3Obj())
        {
            // notify listeners
            GetAS3Obj()->Dispatch(id, GetIntObj());
        }
        break;
    default:
        {
            //printf("%X\n",id.Id);
            return AvmInteractiveObj::OnEvent(id);
        }
    }
    return true;
}
#endif

}}} // SF::GFx::AS3


