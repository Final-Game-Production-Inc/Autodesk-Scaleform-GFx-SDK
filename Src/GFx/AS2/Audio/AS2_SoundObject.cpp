/**************************************************************************

Filename    :   AS2_SoundObject.cpp
Content     :   GFx audio: AS2 Sound class
Created     :   October 2006
Authors     :   Artyom Bolgar, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_Sound.h"

#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/Audio/AS2_SoundObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//////////////////////////////////////////////////////////////////////////
//

void SoundObject::AttachToTarget(Sprite *pcharacter)
{
    SF_ASSERT(pcharacter);
    pTargetHandle = pcharacter->GetCharacterHandle();
    pcharacter->AttachSoundObject(this);
}

Sprite* SoundObject::GetSprite()
{
    if (!pTargetHandle)
        return NULL;
    if (!pMovieRoot)
        return NULL;

    DisplayObject* ptarget = pTargetHandle->ResolveCharacter(pMovieRoot);
    if (!ptarget) return NULL;

    SF_ASSERT(ptarget->IsSprite());
    return ptarget->CharToSprite();
}

void SoundObject::DetachFromTarget()
{
    if(!pTargetHandle) return;
    Sprite* psprite = GetSprite();
    if (psprite)
        psprite->DetachSoundObject(this);
}

void SoundObject::ReleaseTarget()
{
    pTargetHandle = NULL;
}

void SoundObject::ExecuteOnSoundComplete()
{
    Sprite* psprite = GetSprite();
    if (!psprite) return;
    Environment* penv = GFx::AS2::ToAvmCharacter(psprite)->GetASEnvironment();
    Value callback;
    if (GetMemberRaw(penv->GetSC(), penv->CreateConstString("onSoundComplete"), &callback))
    {
        FunctionRef method = callback.ToFunction(penv);
        Value result;
        method.Invoke(FnCall(&result, this, penv, 0, penv->GetTopIndex()));
    }
}

bool SoundObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if(name == "position")
    {
        Sprite* psprite = GetSprite();
        if (psprite)
            *val = psprite->GetActiveSoundPosition(this)*1000;
        else
            *val = Value(0.0f);
        return true;
    }
    else if(name == "duration")
    {
        if (pSample)
            *val = pSample->GetDuration() * 1000;
        else
            *val = Value(0.0f);
        return true;
    }
    return Object::GetMember(penv, name, val);
}

//////////////////////////////////////////////////////////////////////////
//

void SoundProto::Start(const FnCall& fn)
{
    MovieImpl* proot = fn.Env->GetMovieImpl();
    if (!proot) return;
    Sound::SoundRenderer* pplayer = proot->GetSoundRenderer();
    if (!pplayer) return;

    if (!fn.ThisPtr || fn.ThisPtr->GetObjectType() != Object_Sound)
        return;

    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    if (!pobj) return;

    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;

    SInt32 secondOffset = 0;
    SInt32 loops = 0;

    if (fn.NArgs > 0)
        secondOffset = fn.Arg(0).ToInt32(fn.Env);

    if (fn.NArgs > 1)
        loops = fn.Arg(1).ToInt32(fn.Env);

    if (loops == 0)
        loops = 1;
    
    Ptr<Sound::SoundChannel> pchan;
    if (!pobj->pResource)
    {
        // This means that this sound was created by Sound.loadSound method.
        // We need to check if it is already playing and if it is the don't start it again.
        if (psprite->IsSoundPlaying(pobj))
            return;
    }
    pchan = *pplayer->PlaySample(pobj->pSample,true);
    if (pchan)
    {
        if (secondOffset > 0 || loops > 0)
            pchan->Loop(loops, secondOffset/1.f);
        pchan->SetVolume(psprite->GetRealSoundVolume());
        pchan->SetPan(psprite->GetRealSoundPan());
        pchan->Pause(false);
        psprite->AddActiveSound(pchan, pobj, pobj->pResource);
    }
}

void SoundProto::Stop(const FnCall& fn)
{
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    if (!pobj) return;

    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;

    if (fn.NArgs > 0)
    {
        ASString id(fn.Arg(0).ToString(fn.Env));
        SoundObject* spobj = static_cast<SoundObject*>(fn.ThisPtr);
        SF_ASSERT(spobj);
        Ptr<MovieDefImpl> md = fn.Env->GetTarget()->GetResourceMovieDef();
        if (spobj && md)
        {
            // Get exported resource for linkageId and verify that it is a sound.
            ResourceBindData resBindData;
            if (!fn.Env->GetMovieImpl()->FindExportedResource(md, &resBindData, id.ToCStr()))
                return;
            SF_ASSERT(resBindData.pResource.GetPtr() != 0);
            // Must check resource type, since users can theoretically pass other resource ids.
            if (resBindData.pResource->GetResourceType() != Resource::RT_SoundSample)
                return;

            SoundResource* psound = (SoundResource*)resBindData.pResource.GetPtr();
            if (psound)
            {
                psprite->StopActiveSounds(psound);
            }
        }
    }
    else
        psprite->StopActiveSounds();
}

void SoundProto::Attach(const FnCall& fn)
{
    MovieImpl* proot = fn.Env->GetMovieImpl();
    if (!proot) return;
    Sound::SoundRenderer* pplayer = proot->GetSoundRenderer();
    if (!pplayer) return;

    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Sound.AttachSound requires one argument (file name)");
        return;
    }
    ASString id(fn.Arg(0).ToString(fn.Env));
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);

    Ptr<MovieDefImpl> md = fn.Env->GetTarget()->GetResourceMovieDef();
    if (pobj && md)
    {
        // Get exported resource for linkageId and verify that it is a sound.
        ResourceBindData resBindData;
        if (!fn.Env->GetMovieImpl()->FindExportedResource(md, &resBindData, id.ToCStr()))
            return;
        SF_ASSERT(resBindData.pResource.GetPtr() != 0);
        // Must check resource type, since users can theoretically pass other resource ids.
        if (resBindData.pResource->GetResourceType() != Resource::RT_SoundSample)
            return;

        SoundResource* psound = (SoundResource*)resBindData.pResource.GetPtr();
        if (psound)
        {
            pobj->pSample = psound->GetSoundInfo()->GetSoundSample(pplayer);
            pobj->pResource = psound;
        }
    }
}

void SoundProto::GetBytesLoaded(const FnCall& fn) 
{ 
    fn.Result->SetUndefined();
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    if (pobj && !pobj->pResource && pobj->pSample)
    {
        // BytesTotal is only defined of sounds loaded with Sound.loadSound method
        fn.Result->SetInt(pobj->pSample->GetBytesLoaded());
    }
}

void SoundProto::GetBytesTotal(const FnCall& fn)  
{ 
    fn.Result->SetUndefined();
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    if (pobj && !pobj->pResource && pobj->pSample)
    {
        // BytesTotal is only defined of sounds loaded with Sound.loadSound method
        fn.Result->SetInt(pobj->pSample->GetBytesTotal());
    }
}

void SoundProto::GetPan(const FnCall& fn)         
{ 
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    if (!pobj) return;
    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;

    fn.Result->SetInt(psprite->GetSoundPan()); 
}

void SoundProto::GetTransform(const FnCall& fn)   
{ 
    fn.Result->SetUndefined(); 
}

void SoundProto::GetVolume(const FnCall& fn)      
{ 
    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    fn.Result->SetUndefined();
    SF_ASSERT(pobj);
    if (!pobj) return;
    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;

    fn.Result->SetInt(psprite->GetSoundVolume()); 
}

void SoundProto::LoadSound(const FnCall& fn)
{ 
    MovieImpl* proot = fn.Env->GetMovieImpl();
    if (!proot) return;
    Sound::SoundRenderer* pplayer = proot->GetSoundRenderer();
    if (!pplayer) return;

    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;

    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Sound.AttachSound requires one argument (file name)");
        return;
    }

    ASString url(fn.Arg(0).ToString(fn.Env));

    bool streamflag = false;
    if (fn.NArgs > 1)
        streamflag = fn.Arg(1).ToBool(fn.Env);

    Ptr<LoadStates> pls = *SF_NEW LoadStates(proot->pMainMovieDef->pLoaderImpl, proot->pStateBag);

    // Obtain level0 path before it has a chance to be unloaded below.
    String level0Path;
    proot->GetMainMoviePath(&level0Path);

    URLBuilder::LocationInfo loc(URLBuilder::File_Regular, String(url.ToCStr()), level0Path);
    String fileName;
    pls->BuildURL(&fileName, loc);

    Ptr<Sound::SoundSample> psample = *pplayer->CreateSampleFromFile(fileName.ToCStr(), streamflag);
    if (!psample)
    {
        Value callback;
        if (pobj->GetMemberRaw(fn.Env->GetSC(),fn.Env->CreateConstString("onLoad"), &callback))
        {
            FunctionRef method = callback.ToFunction(fn.Env);
            Value result;
            fn.Env->Push(false);
            method.Invoke(FnCall(&result, pobj, fn.Env, 1, fn.Env->GetTopIndex()));
            fn.Env->Drop(1);
        }
        return;
    }
    pobj->pResource = NULL;
    pobj->pSample = psample;
    if (streamflag)
    {
        Ptr<Sound::SoundChannel> pchan;
        pchan = *pplayer->PlaySample(pobj->pSample,true);
        if (pchan)
        {
            pchan->SetVolume(psprite->GetRealSoundVolume());
            pchan->SetPan(psprite->GetRealSoundPan());
            pchan->Pause(false);
            psprite->AddActiveSound(pchan, pobj, pobj->pResource);
        }
    }
    Value callback;
    if (pobj->GetMemberRaw(fn.Env->GetSC(),fn.Env->CreateConstString("onLoad"), &callback))
    {
        FunctionRef method = callback.ToFunction(fn.Env);
        Value result;
        fn.Env->Push(true);
        method.Invoke(FnCall(&result, pobj, fn.Env, 1, fn.Env->GetTopIndex()));
        fn.Env->Drop(1);
    }
    fn.Env->Drop(1);
}

void  SoundProto::SetPan(const FnCall& fn)         
{ 
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Sound.SetPan requires one argument (pan)");
        return;
    }

    int pan = fn.Arg(0).ToInt32(fn.Env);

    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    if (!pobj) return;
    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;
    psprite->SetSoundPan(pan);
}

void SoundProto::SetTransform(const FnCall& fn)   
{ 
    SF_UNUSED(fn); 
}

void SoundProto::SetVolume(const FnCall& fn)      
{ 
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Sound.SetVolume requires at least one argument (volume)");
        return;
    }

    int volume = fn.Arg(0).ToInt32(fn.Env);
    int subvol = 100;
    if (fn.NArgs > 1)
        subvol = fn.Arg(1).ToInt32(fn.Env);

    SoundObject* pobj = static_cast<SoundObject*>(fn.ThisPtr);
    SF_ASSERT(pobj);
    if (!pobj) return;
    Sprite* psprite = pobj->GetSprite();
    if (!psprite) return;
    SF_UNUSED(subvol);
    psprite->SetSoundVolume(volume, subvol);
}

static const NameFunction AS2_SoundFunctionTable[] = 
{
    { "attachSound",    &SoundProto::Attach  },
    { "getBytesLoaded", &SoundProto::GetBytesLoaded },
    { "getBytesTotal",  &SoundProto::GetBytesTotal },
    { "getPan",         &SoundProto::GetPan },
    { "getTransform",   &SoundProto::GetTransform },
    { "getVolume",      &SoundProto::GetVolume },
    { "loadSound",      &SoundProto::LoadSound },
    { "setPan",         &SoundProto::SetPan },
    { "setTransform",   &SoundProto::SetTransform },
    { "setVolume",      &SoundProto::SetVolume },
    { "start",          &SoundProto::Start },
    { "stop",           &SoundProto::Stop },
    { 0, 0 }
};

SoundProto::SoundProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) :
    Prototype<SoundObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, AS2_SoundFunctionTable);
}

//////////////////////////////////////////////////////////////////////////
//

// Constructor for ActionScript class Sound.
void SoundCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<SoundObject> psoundObj;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_Sound && !fn.ThisPtr->IsBuiltinPrototype())
        psoundObj = static_cast<SoundObject*>(fn.ThisPtr);
    else
        psoundObj = *SF_HEAP_NEW(fn.Env->GetHeap()) SoundObject(fn.Env);

    InteractiveObject* mc = NULL;
    if (fn.NArgs > 0)
    {
        mc = fn.Arg(0).ToCharacter(fn.Env);
        if (mc && !mc->IsSprite())
            mc = NULL;
    }
    // If we were not provided with the correct Sprite we link this object to MovieRoot
    if (!mc)
        mc = fn.Env->GetAS2Root()->GetLevelMovie(0);
    SF_ASSERT(mc);
    SF_ASSERT(mc->IsSprite());
    psoundObj->AttachToTarget(mc->CharToSprite());
    fn.Result->SetAsObject(psoundObj.GetPtr());
}

Object* SoundCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) SoundObject(penv);
}

FunctionRef SoundCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) SoundCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) SoundProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Sound, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Sound), Value(ctor));
    return ctor;
}

}}} // namespace Scaleform::GFx::AS2

#endif // GFX_ENABLE_SOUND
