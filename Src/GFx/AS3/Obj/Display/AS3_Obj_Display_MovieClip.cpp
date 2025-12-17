//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_MovieClip.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_MovieClip.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../Obj/AS3_Obj_Array.h"
#include "AS3_Obj_Display_Scene.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "Kernel/SF_Array.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_Resource.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_gotoAndPlay, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_gotoAndStop, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_currentFrameGet, SInt32> TFunc_Instances_MovieClip_currentFrameGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_currentFrameLabelGet, ASString> TFunc_Instances_MovieClip_currentFrameLabelGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_currentLabelGet, ASString> TFunc_Instances_MovieClip_currentLabelGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_currentLabelsGet, SPtr<Instances::fl::Array> > TFunc_Instances_MovieClip_currentLabelsGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_currentSceneGet, SPtr<Instances::fl_display::Scene> > TFunc_Instances_MovieClip_currentSceneGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_enabledGet, bool> TFunc_Instances_MovieClip_enabledGet;
typedef ThunkFunc1<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_enabledSet, const Value, bool> TFunc_Instances_MovieClip_enabledSet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_framesLoadedGet, SInt32> TFunc_Instances_MovieClip_framesLoadedGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_scenesGet, SPtr<Instances::fl::Array> > TFunc_Instances_MovieClip_scenesGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_totalFramesGet, SInt32> TFunc_Instances_MovieClip_totalFramesGet;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_trackAsMenuGet, bool> TFunc_Instances_MovieClip_trackAsMenuGet;
typedef ThunkFunc1<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_trackAsMenuSet, const Value, bool> TFunc_Instances_MovieClip_trackAsMenuSet;
typedef ThunkFunc2<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_addFrameScript, Value, unsigned, const Value*> TFunc_Instances_MovieClip_addFrameScript;
typedef ThunkFunc2<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_gotoAndPlay, const Value, const Value&, const Value&> TFunc_Instances_MovieClip_gotoAndPlay;
typedef ThunkFunc2<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_gotoAndStop, const Value, const Value&, const Value&> TFunc_Instances_MovieClip_gotoAndStop;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_nextFrame, const Value> TFunc_Instances_MovieClip_nextFrame;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_nextScene, const Value> TFunc_Instances_MovieClip_nextScene;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_play, const Value> TFunc_Instances_MovieClip_play;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_prevFrame, const Value> TFunc_Instances_MovieClip_prevFrame;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_prevScene, const Value> TFunc_Instances_MovieClip_prevScene;
typedef ThunkFunc0<Instances::fl_display::MovieClip, Instances::fl_display::MovieClip::mid_stop, const Value> TFunc_Instances_MovieClip_stop;

template <> const TFunc_Instances_MovieClip_currentFrameGet::TMethod TFunc_Instances_MovieClip_currentFrameGet::Method = &Instances::fl_display::MovieClip::currentFrameGet;
template <> const TFunc_Instances_MovieClip_currentFrameLabelGet::TMethod TFunc_Instances_MovieClip_currentFrameLabelGet::Method = &Instances::fl_display::MovieClip::currentFrameLabelGet;
template <> const TFunc_Instances_MovieClip_currentLabelGet::TMethod TFunc_Instances_MovieClip_currentLabelGet::Method = &Instances::fl_display::MovieClip::currentLabelGet;
template <> const TFunc_Instances_MovieClip_currentLabelsGet::TMethod TFunc_Instances_MovieClip_currentLabelsGet::Method = &Instances::fl_display::MovieClip::currentLabelsGet;
template <> const TFunc_Instances_MovieClip_currentSceneGet::TMethod TFunc_Instances_MovieClip_currentSceneGet::Method = &Instances::fl_display::MovieClip::currentSceneGet;
template <> const TFunc_Instances_MovieClip_enabledGet::TMethod TFunc_Instances_MovieClip_enabledGet::Method = &Instances::fl_display::MovieClip::enabledGet;
template <> const TFunc_Instances_MovieClip_enabledSet::TMethod TFunc_Instances_MovieClip_enabledSet::Method = &Instances::fl_display::MovieClip::enabledSet;
template <> const TFunc_Instances_MovieClip_framesLoadedGet::TMethod TFunc_Instances_MovieClip_framesLoadedGet::Method = &Instances::fl_display::MovieClip::framesLoadedGet;
template <> const TFunc_Instances_MovieClip_scenesGet::TMethod TFunc_Instances_MovieClip_scenesGet::Method = &Instances::fl_display::MovieClip::scenesGet;
template <> const TFunc_Instances_MovieClip_totalFramesGet::TMethod TFunc_Instances_MovieClip_totalFramesGet::Method = &Instances::fl_display::MovieClip::totalFramesGet;
template <> const TFunc_Instances_MovieClip_trackAsMenuGet::TMethod TFunc_Instances_MovieClip_trackAsMenuGet::Method = &Instances::fl_display::MovieClip::trackAsMenuGet;
template <> const TFunc_Instances_MovieClip_trackAsMenuSet::TMethod TFunc_Instances_MovieClip_trackAsMenuSet::Method = &Instances::fl_display::MovieClip::trackAsMenuSet;
template <> const TFunc_Instances_MovieClip_addFrameScript::TMethod TFunc_Instances_MovieClip_addFrameScript::Method = &Instances::fl_display::MovieClip::addFrameScript;
template <> const TFunc_Instances_MovieClip_gotoAndPlay::TMethod TFunc_Instances_MovieClip_gotoAndPlay::Method = &Instances::fl_display::MovieClip::gotoAndPlay;
template <> const TFunc_Instances_MovieClip_gotoAndStop::TMethod TFunc_Instances_MovieClip_gotoAndStop::Method = &Instances::fl_display::MovieClip::gotoAndStop;
template <> const TFunc_Instances_MovieClip_nextFrame::TMethod TFunc_Instances_MovieClip_nextFrame::Method = &Instances::fl_display::MovieClip::nextFrame;
template <> const TFunc_Instances_MovieClip_nextScene::TMethod TFunc_Instances_MovieClip_nextScene::Method = &Instances::fl_display::MovieClip::nextScene;
template <> const TFunc_Instances_MovieClip_play::TMethod TFunc_Instances_MovieClip_play::Method = &Instances::fl_display::MovieClip::play;
template <> const TFunc_Instances_MovieClip_prevFrame::TMethod TFunc_Instances_MovieClip_prevFrame::Method = &Instances::fl_display::MovieClip::prevFrame;
template <> const TFunc_Instances_MovieClip_prevScene::TMethod TFunc_Instances_MovieClip_prevScene::Method = &Instances::fl_display::MovieClip::prevScene;
template <> const TFunc_Instances_MovieClip_stop::TMethod TFunc_Instances_MovieClip_stop::Method = &Instances::fl_display::MovieClip::stop;

namespace Instances { namespace fl_display
{
    MovieClip::MovieClip(InstanceTraits::Traits& t)
    : Instances::fl_display::Sprite(t)
//##protect##"instance::MovieClip::MovieClip()$data"
//    , pDisplaySprite(NULL)
//##protect##"instance::MovieClip::MovieClip()$data"
    {
//##protect##"instance::MovieClip::MovieClip()$code"
//##protect##"instance::MovieClip::MovieClip()$code"
    }

    void MovieClip::currentFrameGet(SInt32& result)
    {
//##protect##"instance::MovieClip::currentFrameGet()"
        if (!pDispObj)
        {
            result = 0;
            return;
        }
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        result = spr->GetCurrentFrame() + 1;
//##protect##"instance::MovieClip::currentFrameGet()"
    }
    void MovieClip::currentFrameLabelGet(ASString& result)
    {
//##protect##"instance::MovieClip::currentFrameLabelGet()"
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        unsigned fr = spr->GetCurrentFrame(), exactFr;

        const String* label = spr->GetDef()->GetFrameLabel(fr, &exactFr);
        if (label && fr == exactFr)
            result = asvm.GetStringManager().CreateString(label->ToCStr());
        else
            result.SetNull();
//##protect##"instance::MovieClip::currentFrameLabelGet()"
    }
    void MovieClip::currentLabelGet(ASString& result)
    {
//##protect##"instance::MovieClip::currentLabelGet()"
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        unsigned fr = spr->GetCurrentFrame();

        const String* label = spr->GetDef()->GetFrameLabel(fr, &fr);
        if (label)
            result = asvm.GetStringManager().CreateString(label->ToCStr());
        else
            result.SetNull();
//##protect##"instance::MovieClip::currentLabelGet()"
    }
    void MovieClip::currentLabelsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::MovieClip::currentLabelsGet()"

        SPtr<Instances::fl_display::Scene> scene;
        currentSceneGet(scene);
        if (scene)
            scene->labelsGet(result);
        else
            result = NULL;
//##protect##"instance::MovieClip::currentLabelsGet()"
    }
    void MovieClip::currentSceneGet(SPtr<Instances::fl_display::Scene>& result)
    {
//##protect##"instance::MovieClip::currentSceneGet()"
        result = NULL;

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            // main time line
            unsigned fr = spr->GetCurrentFrame();
            UPInt cnt = 0;
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);

            for (UPInt i = 0; i < cnt; ++i)
            {
                if (fr >= scenes[i].Offset && fr < scenes[i].Offset + scenes[i].NumFrames)
                {
                    // create a Scene object
                    Class* sceneClassVal = GetVM().GetClass("flash.display.Scene", GetVM().GetCurrentAppDomain());

                    asvm.ConstructInstance(result, sceneClassVal, 0);

                    // populate scene
                    result->Set(&scenes[i]);
                    break;
                }
            }
        }
        if (result == NULL)
            result = CreateFakeScene();
//##protect##"instance::MovieClip::currentSceneGet()"
    }
    void MovieClip::enabledGet(bool& result)
    {
//##protect##"instance::MovieClip::enabledGet()"
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        result = pDispObj->CharToInteractiveObject_Unsafe()->IsEnabledFlagSet();
//##protect##"instance::MovieClip::enabledGet()"
    }
    void MovieClip::enabledSet(const Value& result, bool value)
    {
//##protect##"instance::MovieClip::enabledSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        
        pDispObj->CharToInteractiveObject_Unsafe()->SetEnabledFlag(value);

//##protect##"instance::MovieClip::enabledSet()"
    }
    void MovieClip::framesLoadedGet(SInt32& result)
    {
//##protect##"instance::MovieClip::framesLoadedGet()"
        if (!pDispObj)
        {
            result = 1;
            return;
        }
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        result = spr->GetLoadingFrame();
//##protect##"instance::MovieClip::framesLoadedGet()"
    }
    void MovieClip::scenesGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::MovieClip::scenesGet()"
        SF_UNUSED1(result);
        // create an array of scenes
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        // Create an array
        result = GetVM().MakeArray();

        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            UPInt cnt = 0;
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);

            Class* sceneClassVal = GetVM().GetClass("flash.display.Scene", GetVM().GetCurrentAppDomain());

            for (UPInt i = 0; i < cnt; ++i)
            {
                // create a Scene object

                SPtr<Instances::fl_display::Scene> scene;
                asvm.ConstructInstance(scene, sceneClassVal, 0);

                // populate scene
                scene->Set(&scenes[i]);

                result->PushBack(Value(scene));
            }
        }
        else
        {
            SPtr<Instances::fl_display::Scene> scene = CreateFakeScene();
            result->PushBack(Value(scene));
        }
//##protect##"instance::MovieClip::scenesGet()"
    }
    void MovieClip::totalFramesGet(SInt32& result)
    {
//##protect##"instance::MovieClip::totalFramesGet()"
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        result = spr->GetDef()->GetFrameCount();
//##protect##"instance::MovieClip::totalFramesGet()"
    }
    void MovieClip::trackAsMenuGet(bool& result)
    {
//##protect##"instance::MovieClip::trackAsMenuGet()"
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        result = spr->GetTrackAsMenu();
//##protect##"instance::MovieClip::trackAsMenuGet()"
    }
    void MovieClip::trackAsMenuSet(const Value& result, bool value)
    {
//##protect##"instance::MovieClip::trackAsMenuSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        spr->SetTrackAsMenuFlag(value);
//##protect##"instance::MovieClip::trackAsMenuSet()"
    }
    void MovieClip::addFrameScript(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::MovieClip::addFrameScript()"
        SF_UNUSED3(result, argc, argv);

        if (!pDispObj) CreateStageObject();
        SF_ASSERT(pDispObj);

        GFx::Sprite* pspr = pDispObj->CharToSprite();
        if (pspr)
        {
            AvmSprite* avmSpr = ToAvmSprite(pspr);
            int n = argc/2;
            Scaleform::ArrayCPP<FrameScript::Descr> frames;
            //frames.Reserve(n);
            for (int i = 0, k = 0; i < n; ++i)
            {
                SInt32 frameNum;
                if (!argv[k++].Convert2Int32(frameNum))
                    continue; // wrong number, skip? or break?
                // frames for addFrameScript are zero-based;
                // however, our frames are 1-based (0 frame is used
                // for initialization logic, rigistering classes, etc).
                FrameScript::Descr fr(frameNum, argv[k++]);
                frames.PushBack(fr);
            }
            mFrameScript.Set(avmSpr->GetHeap(), pspr->GetFrameCount(), n, &frames[0]);

            // each call to addFrameScript adds current frame to queue, regardless to
            // parameters
            const FrameScript::Descr* pd = mFrameScript.Get(pspr->GetCurrentFrame());
            if (pd)
            {
                avmSpr->SetNeedExecuteFrameFlag();
            }
        }
//##protect##"instance::MovieClip::addFrameScript()"
    }
    void MovieClip::gotoAndPlay(const Value& result, const Value& frame, const Value& scene)
    {
//##protect##"instance::MovieClip::gotoAndPlay()"
        SF_UNUSED3(result, frame, scene);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        unsigned targetFrame;
        if (frame.IsString())
        {
            if (!GetLabeledFrame(spr, frame, scene, &targetFrame))
            {
                const MovieDataDef::SceneInfo* curSceneInfo = GetSceneInfo(spr->GetCurrentFrame());
                const char* sceneName = "unknown";
                if (curSceneInfo)
                {
                    sceneName = curSceneInfo->Name.ToCStr();
                }
                GetVM().ThrowArgumentError(VM::Error(VM::eFrameLabelNotFoundInScene, GetVM()
                    SF_DEBUG_ARG(frame) 
                    SF_DEBUG_ARG(sceneName)
                    ));
                return;
            }
        }
        else
        {
            UInt32 r;
            if (!frame.Convert2UInt32(r))
                return;
            targetFrame = r - 1;
        }
        ASVM& vm = static_cast<ASVM&>(GetVM());
        if (targetFrame != spr->GetCurrentFrame())
        {
            // This seems to be necessary to avoid extra execution of already queued up
            // frames: gotoAnd... in AS3 overrides it (unlike in AS2).
            vm.GetMovieRoot()->RemoveActionQueueEntriesFor(MovieRoot::AL_Frame, pDispObj);

            spr->GotoFrame(targetFrame);
        }
        spr->SetPlayState(State_Playing);

        // execute all events right here
        vm.GetMovieRoot()->QueueFrameActions();

        // If we are already in the process of executing actions after gotoAnd... then don't
        // execute again to avoid stack overflow. Actions will be executed by the outer gotoAnd...
        // so all what we need to do is to put the action into the queue (see above). Flash does the same
        // (see test_nested_gotos.swf).
        if (!vm.GetMovieRoot()->IsInsideGotoFrameFlagSet())
        {
            vm.GetMovieRoot()->SetInsideGotoFrameFlag();
            // execute highest/high/frame actions.
            // Do not execute enterframe and lower levels (render events and such)
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_Highest);
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_High);
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_Frame);
            vm.GetMovieRoot()->ClearInsideGotoFrameFlag();
        }

//##protect##"instance::MovieClip::gotoAndPlay()"
    }
    void MovieClip::gotoAndStop(const Value& result, const Value& frame, const Value& scene)
    {
//##protect##"instance::MovieClip::gotoAndStop()"
        SF_UNUSED3(result, frame, scene);

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        unsigned targetFrame;
        if (frame.IsString())
        {
            if (!GetLabeledFrame(spr, frame, scene, &targetFrame))
            {
                const MovieDataDef::SceneInfo* curSceneInfo = GetSceneInfo(spr->GetCurrentFrame());
                const char* sceneName = "unknown";
                if (curSceneInfo)
                {
                    sceneName = curSceneInfo->Name.ToCStr();
                }
                GetVM().ThrowArgumentError(VM::Error(VM::eFrameLabelNotFoundInScene, GetVM()
                    SF_DEBUG_ARG(frame) 
                    SF_DEBUG_ARG(sceneName)
                    ));
                return;
            }
        }
        else
        {
            UInt32 r;
            if (!frame.Convert2UInt32(r))
                return;
            targetFrame = r - 1;
        }
        ASVM& vm = static_cast<ASVM&>(GetVM());
        if (targetFrame != spr->GetCurrentFrame())
        {
            // This seems to be necessary to avoid extra execution of already queued up
            // frames: gotoAnd... in AS3 overrides it (unlike in AS2).
            vm.GetMovieRoot()->RemoveActionQueueEntriesFor(MovieRoot::AL_Frame, pDispObj);

            spr->GotoFrame(targetFrame);
        }
        spr->SetPlayState(State_Stopped);

        // execute all events right here
        vm.GetMovieRoot()->QueueFrameActions();

        // If we are already in the process of executing actions after gotoAnd... then don't
        // execute again to avoid stack overflow. Actions will be executed by the outer gotoAnd...
        // so all what we need to do is to put the action into the queue (see above). Flash does the same
        // (see test_nested_gotos.swf).
        if (!vm.GetMovieRoot()->IsInsideGotoFrameFlagSet())
        {
            vm.GetMovieRoot()->SetInsideGotoFrameFlag();
            // execute highest/high/frame actions.
            // Do not execute enterframe and lower levels (render events and such)
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_Highest);
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_High);
            vm.GetMovieRoot()->ExecuteActionQueue(MovieRoot::AL_Frame);
            vm.GetMovieRoot()->ClearInsideGotoFrameFlag();
        }
//##protect##"instance::MovieClip::gotoAndStop()"
    }
    void MovieClip::nextFrame(const Value& result)
    {
//##protect##"instance::MovieClip::nextFrame()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        int availableFrameCount = spr->GetLoadingFrame();
        int currentFrame = spr->GetCurrentFrame();
        if (currentFrame < availableFrameCount)
            spr->GotoFrame(currentFrame + 1);      
        spr->SetPlayState(State_Stopped);
//##protect##"instance::MovieClip::nextFrame()"
    }
    void MovieClip::nextScene(const Value& result)
    {
//##protect##"instance::MovieClip::nextScene()"
        SF_UNUSED1(result);
        
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        // Determine the current scene first
        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            UPInt cnt = 0;
            //ASVM& asvm = static_cast<ASVM&>(GetVM());
            //MovieRoot* proot = asvm.GetMovieRoot();
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);
            unsigned frameNumber = spr->GetCurrentFrame();

            for (UPInt i = 0; i < cnt; ++i)
            {
                if (frameNumber >= scenes[i].Offset && frameNumber < scenes[i].Offset + scenes[i].NumFrames)
                {
                    // get the next one, if exists
                    if (i + 1 < cnt)
                    {
                        // goto frame
                        unsigned availableFrameCount = spr->GetLoadingFrame();
                        if (scenes[i + 1].Offset < availableFrameCount)
                        {
                            spr->GotoFrame(scenes[i + 1].Offset);      
                            spr->SetPlayState(State_Playing);
                        }
                    }
                }
            }
        }
//##protect##"instance::MovieClip::nextScene()"
    }
    void MovieClip::play(const Value& result)
    {
//##protect##"instance::MovieClip::play()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        spr->SetPlayState(State_Playing);
//##protect##"instance::MovieClip::play()"
    }
    void MovieClip::prevFrame(const Value& result)
    {
//##protect##"instance::MovieClip::prevFrame()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        int currentFrame = spr->GetCurrentFrame();
        if (currentFrame > 0)
            spr->GotoFrame(currentFrame - 1);
        spr->SetPlayState(State_Stopped);
//##protect##"instance::MovieClip::prevFrame()"
    }
    void MovieClip::prevScene(const Value& result)
    {
//##protect##"instance::MovieClip::prevScene()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        // Determine the current scene first
        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            UPInt cnt = 0;
            //ASVM& asvm = static_cast<ASVM&>(GetVM());
            //MovieRoot* proot = asvm.GetMovieRoot();
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);
            unsigned frameNumber = spr->GetCurrentFrame();

            for (UPInt i = 0; i < cnt; ++i)
            {
                if (frameNumber >= scenes[i].Offset && frameNumber < scenes[i].Offset + scenes[i].NumFrames)
                {
                    // get the prev one, if exists
                    if (i > 0)
                    {
                        // goto frame
                        spr->GotoFrame(scenes[i - 1].Offset);      
                        spr->SetPlayState(State_Playing);
                    }
                }
            }
        }
//##protect##"instance::MovieClip::prevScene()"
    }
    void MovieClip::stop(const Value& result)
    {
//##protect##"instance::MovieClip::stop()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        
        spr->SetPlayState(State_Stopped);
//##protect##"instance::MovieClip::stop()"
    }

    SPtr<Instances::fl::Array> MovieClip::currentLabelsGet()
    {
        SPtr<Instances::fl::Array> result;
        currentLabelsGet(result);
        return result;
    }
    SPtr<Instances::fl_display::Scene> MovieClip::currentSceneGet()
    {
        SPtr<Instances::fl_display::Scene> result;
        currentSceneGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> MovieClip::scenesGet()
    {
        SPtr<Instances::fl::Array> result;
        scenesGet(result);
        return result;
    }
//##protect##"instance$methods"
    SPtr<Instances::fl_display::Scene> MovieClip::CreateFakeScene()
    {
        // There is only one Scene in the sprite
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Class* sceneClassVal = GetVM().GetClass("flash.display.Scene", GetVM().GetCurrentAppDomain());

        SPtr<Instances::fl_display::Scene> scene;
        asvm.ConstructInstance(scene, sceneClassVal, 0);

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);
        scene->Set(spr);
        return scene;
    }

    const MovieDataDef::SceneInfo* MovieClip::GetSceneInfo(unsigned frameNumber) const
    {
        //ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            UPInt cnt = 0;
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);

            for (UPInt i = 0; i < cnt; ++i)
            {
                if (frameNumber >= scenes[i].Offset && frameNumber < scenes[i].Offset + scenes[i].NumFrames)
                {
                    return &scenes[i];
                }
            }
        }
        return NULL;
    }

    const MovieDataDef::SceneInfo* MovieClip::GetSceneInfoByName(const ASString& name) const
    {
        //ASVM& asvm = static_cast<ASVM&>(GetVM());
        //MovieRoot* proot = asvm.GetMovieRoot();

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* spr = pDispObj->CharToSprite_Unsafe();
        SF_ASSERT(spr);

        if (spr->GetDef()->GetType() == CharacterDef::MovieData)
        {
            UPInt cnt = 0;
            const MovieDataDef::SceneInfo* scenes = spr->GetResourceMovieDef()->GetDataDef()->GetScenes(&cnt);

            for (UPInt i = 0; i < cnt; ++i)
            {
                if (name == scenes[i].Name)
                {
                    return &scenes[i];
                }
            }
        }
        return NULL;
    }

    bool MovieClip::GetLabeledFrame(GFx::Sprite* spr, const Value& frame, const Value& scene, unsigned* targetFrame)
    {
        SF_ASSERT(targetFrame);
        spr->GetLabeledFrame(frame.AsString().ToCStr(), targetFrame);
        if (!scene.IsNull() && !scene.IsUndefined())
        {
            ASString sceneName = GetVM().GetStringManager().CreateEmptyString();
            scene.Convert2String(sceneName).DoNotCheck();

            // scene is specified.
            // check if the targetFrame is within the specified scene.
            const MovieDataDef::SceneInfo* sceneInfo = GetSceneInfoByName(sceneName);
            if (!sceneInfo)
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eSceneNotFound, GetVM() SF_DEBUG_ARG(sceneName.ToCStr())));
                return false;
            }
            const MovieDataDef::SceneInfo* targetSceneInfo = GetSceneInfo(*targetFrame);
            if (sceneInfo != targetSceneInfo)
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eFrameLabelNotFoundInScene, GetVM()
                    SF_DEBUG_ARG(frame) 
                    SF_DEBUG_ARG(sceneName.ToCStr())
                    ));
                return false;
            }
        }
        else
        {
            // scene is not specified.
            // just check if the targetFrame is within the curSceneInfo
            const MovieDataDef::SceneInfo* curSceneInfo = GetSceneInfo(spr->GetCurrentFrame());
            if (curSceneInfo)
            {
                const MovieDataDef::SceneInfo* targetSceneInfo = GetSceneInfo(*targetFrame);
                if (curSceneInfo != targetSceneInfo)
                {
                    GetVM().ThrowArgumentError(VM::Error(VM::eFrameLabelNotFoundInScene, GetVM()
                        SF_DEBUG_ARG(frame) 
                        SF_DEBUG_ARG(curSceneInfo->Name.ToCStr())
                        ));
                    return false;
                }
            }
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    MovieClip::FrameScript::~FrameScript() 
    { 
        Clear();
    }

    void MovieClip::FrameScript::Set(
        MemoryHeap* pheap, unsigned totalFrames, unsigned cnt, const Descr* pdescriptors)
    {
        unsigned actualDescrCnt = 0;
        if (pData)
        {
            // if pData already exists we need to update records for existing frames
            // and add new ones. No duplicates are allowed.
            SF_ASSERT(totalFrames == FrameCnt);

            actualDescrCnt = DescrCnt;
            unsigned delta = 0;
            // check if we need to expand storage...
            for (int i = (int)cnt - 1; i >= 0; --i)
            {
                unsigned fr = pdescriptors[i].Frame;
                if (fr < FrameCnt)
                {
                    if (!(pData[fr >> 3] & (1 << (fr & 7))))
                        ++delta;
                    else
                    {
                        // update the existing entry

                        Descr* pdescr = GetDescs();
                        // bit indicates the descriptor exists: do search in sorted array
                        UPInt k = Alg::LowerBoundSized(pdescr, DescrCnt, Descr(fr, Value::GetUndefined()));
                        SF_ASSERT(pdescr[k].Frame == fr);
                        pdescr[k] = pdescriptors[i];
                    }
                }
            }
            unsigned newDescrCnt = DescrCnt + delta;
            if (delta > 0)
            {
                // expand pData for delta elements
                pData = reinterpret_cast<UByte*>(
                    SF_REALLOC(pData, GetDescSize(newDescrCnt), StatMV_ActionScript_Mem));

                Descr* pdest = GetDescs();
                for (int i = (int)newDescrCnt-1, j = (int)cnt-1; i >= DescrCnt && j >= 0; --j)
                {
                    unsigned fr = pdescriptors[j].Frame;
                    if (fr < FrameCnt)
                    {
                        if (!(pData[fr >> 3] & (1 << (fr & 7))))
                        {
                            new (&pdest[i--]) Descr(pdescriptors[j]);
                            // set corresponding bits to 1
                            pData[fr >> 3] |= (1 << (fr & 7));
                            ++actualDescrCnt;
                        }
                    }
                }

                if (newDescrCnt != actualDescrCnt)
                {
                    SF_ASSERT(actualDescrCnt < newDescrCnt);
                    memcpy(pdest + DescrCnt, pdest + actualDescrCnt, 
                        (newDescrCnt - actualDescrCnt) * sizeof(Descr));
                }
                DescrCnt = (UInt16)newDescrCnt;
            }
        }
        else if (cnt > 0)
        {
            // pData is NULL, create from scratch

            DescrCnt = (UInt16)Alg::Min(cnt, totalFrames);
            FrameCnt = (UInt16)totalFrames;
            // allocate data for each frame + space for a bitset that indicates presence of
            // action in the corresponding frame.
            pData = reinterpret_cast<UByte*>(
                SF_HEAP_ALLOC(pheap, GetDescSize(DescrCnt), StatMV_ActionScript_Mem));
            if (!pData)
                return; //?

            // The bitset is at the beginning of the data; an offset to Descrs is
            // (FrameCnt + 7)/8
            memset(pData, 0, (FrameCnt + 7)/8); // set all bits to 0

            // copy descriptors
            Descr* pdest = GetDescs();
            for (int i = (int)DescrCnt - 1, j = i; i >= 0; --i)
            {
                unsigned f = pdescriptors[i].Frame;
                if (f < FrameCnt)
                {
                    if (!(pData[f >> 3] & (1 << (f & 7))))
                    {
                        new (&pdest[j--]) Descr(pdescriptors[i]);
                        // set corresponding bits to 1
                        pData[f >> 3] |= (1 << (f & 7));
                        ++actualDescrCnt;
                    }
                }
            }
            if (DescrCnt != actualDescrCnt)
            {
                SF_ASSERT(actualDescrCnt < DescrCnt);
                memcpy(pdest, pdest + (DescrCnt - actualDescrCnt), actualDescrCnt * sizeof(Descr));
            }
        }
        // sort
        Descr* pdest = GetDescs();

        if (DescrCnt != actualDescrCnt)
        {
            DescrCnt = (UInt16)actualDescrCnt;
            // compact memory if we can; usually happens when the same frame is specified twice 
            // (very rarely).
            pData = reinterpret_cast<UByte*>(
                SF_REALLOC(pData, GetDescSize(DescrCnt), StatMV_ActionScript_Mem));
        }

        Alg::QuickSortSliced(pdest, 0, DescrCnt, Alg::OperatorLess<Descr>::Compare);
    }

    const MovieClip::FrameScript::Descr* MovieClip::FrameScript::Get(unsigned frameNum)
    {
        // check if frameNum exceeds max frame number
        // checks if there is a descriptor for this frame
        if (frameNum < FrameCnt && pData[frameNum >> 3] & (1 << (frameNum & 7)))
        {
            const Descr* pdescr = GetDescs();
            // bit indicates the descriptor exists: do search in sorted array
            UPInt i = Alg::LowerBoundSized(pdescr, DescrCnt, Descr(frameNum, Value::GetUndefined()));
            if (pdescr[i].Frame == frameNum)
                return &pdescr[i];
        }
        return NULL;
    }

    void MovieClip::FrameScript::Clear()
    {
        if (pData)
        {
            const Descr* pdescr = GetDescs();
            for (unsigned i = 0; i < DescrCnt; ++i)
            {
                pdescr[i].~Descr();
            }

            SF_FREE(pData); 
            pData = NULL;
        }
    }

    void MovieClip::FrameScript::ForEachChild_GC(Collector* prcc, GcOp op
                                                SF_DEBUG_ARG(const MovieClip& owner)) const
    {
        if (pData)
        {
            const Descr* pdescr = GetDescs();
            for (unsigned i = 0; i < DescrCnt; ++i)
            {
                AS3::ForEachChild_GC(prcc, pdescr[i].Method, op SF_DEBUG_ARG(owner));
            }
        }
    }

    void MovieClip::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_display::Sprite::ForEachChild_GC(prcc, op);

        mFrameScript.ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));
    }

    bool    MovieClip::HasFrameScript() const
    {
        return !mFrameScript.IsEmpty();
    }

    bool    MovieClip::GetFrameScript(unsigned frame, Value* pmethod)
    {
        // check, if we need to execute ("queue up") AS execution for the frame
        const FrameScript::Descr* pdescr = mFrameScript.Get(frame);
        if (pdescr)
        {
            SF_ASSERT(pmethod);
            if (pdescr->Method.IsCallable() && !pdescr->Method.IsNull())
            {
                *pmethod = pdescr->Method;
                return true;
            }
        }
        return false;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 MovieClip::tito[MovieClip::ThunkInfoNum] = {
    //    0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 18, 21, 22, 23, 24, 25, 26, 
    // };
    const TypeInfo* MovieClip::tit[27] = {
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl_display::SceneTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, NULL, &AS3::fl::ObjectTI, 
        NULL, NULL, &AS3::fl::ObjectTI, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
    };
    const ThunkInfo MovieClip::ti[MovieClip::ThunkInfoNum] = {
        {TFunc_Instances_MovieClip_currentFrameGet::Func, &MovieClip::tit[0], "currentFrame", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_currentFrameLabelGet::Func, &MovieClip::tit[1], "currentFrameLabel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_currentLabelGet::Func, &MovieClip::tit[2], "currentLabel", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_currentLabelsGet::Func, &MovieClip::tit[3], "currentLabels", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_currentSceneGet::Func, &MovieClip::tit[4], "currentScene", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_enabledGet::Func, &MovieClip::tit[5], "enabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_enabledSet::Func, &MovieClip::tit[6], "enabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MovieClip_framesLoadedGet::Func, &MovieClip::tit[8], "framesLoaded", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_scenesGet::Func, &MovieClip::tit[9], "scenes", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_totalFramesGet::Func, &MovieClip::tit[10], "totalFrames", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_trackAsMenuGet::Func, &MovieClip::tit[11], "trackAsMenu", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_trackAsMenuSet::Func, &MovieClip::tit[12], "trackAsMenu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MovieClip_addFrameScript::Func, &MovieClip::tit[14], "addFrameScript", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_MovieClip_gotoAndPlay::Func, &MovieClip::tit[15], "gotoAndPlay", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_MovieClip_gotoAndStop::Func, &MovieClip::tit[18], "gotoAndStop", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_MovieClip_nextFrame::Func, &MovieClip::tit[21], "nextFrame", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_nextScene::Func, &MovieClip::tit[22], "nextScene", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_play::Func, &MovieClip::tit[23], "play", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_prevFrame::Func, &MovieClip::tit[24], "prevFrame", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_prevScene::Func, &MovieClip::tit[25], "prevScene", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MovieClip_stop::Func, &MovieClip::tit[26], "stop", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    MovieClip::MovieClip(VM& vm, const ClassInfo& ci)
    : fl_display::Sprite(vm, ci)
    {
//##protect##"InstanceTraits::MovieClip::MovieClip()"
        SetTraitsType(Traits_MovieClip);
//##protect##"InstanceTraits::MovieClip::MovieClip()"

    }

    void MovieClip::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<MovieClip&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    MovieClip::MovieClip(VM& vm, const ClassInfo& ci)
    : fl_display::Sprite(vm, ci)
    {
//##protect##"ClassTraits::MovieClip::MovieClip()"
        SetTraitsType(Traits_MovieClip);
//##protect##"ClassTraits::MovieClip::MovieClip()"

    }

    Pickable<Traits> MovieClip::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MovieClip(vm, AS3::fl_display::MovieClipCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::MovieClipCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo MovieClipTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl_display::MovieClip::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::MovieClip::ThunkInfoNum,
        0,
        "MovieClip", "flash.display", &fl_display::SpriteTI,
        TypeInfo::None
    };

    const ClassInfo MovieClipCI = {
        &MovieClipTI,
        ClassTraits::fl_display::MovieClip::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::MovieClip::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

