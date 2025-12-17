/**************************************************************************

Filename    :   AS2_MovieClipLoader.cpp
Content     :   Implementation of MovieClipLoader class
Created     :   March, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_MovieClipLoader.h"
#ifdef GFX_AS2_ENABLE_MOVIECLIPLOADER
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2 {

MovieClipLoader::MovieClipLoader(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

void MovieClipLoader::commonInit (Environment* penv)
{    
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_MovieClipLoader));
    AsBroadcaster::Initialize(penv->GetSC(), this);
}

int MovieClipLoader::GetLoadedBytes(InteractiveObject* pch) const
{
    String path;
    pch->GetAbsolutePath(&path);
    const ProgressDesc* pprogress = ProgressInfo.Get(path);
    if (pprogress)
        return pprogress->LoadedBytes;
    return 0;
}

int MovieClipLoader::GetTotalBytes(InteractiveObject* pch)  const
{
    String path;
    pch->GetAbsolutePath(&path);
    const ProgressDesc* pprogress = ProgressInfo.Get(path);
    if (pprogress)
        return pprogress->TotalBytes;
    return 0;
}

void MovieClipLoader::NotifyOnLoadStart(Environment* penv, InteractiveObject* ptarget)
{
    penv->Push(ptarget);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoadStart"), 1, penv->GetTopIndex());
    penv->Drop1();
}

void MovieClipLoader::NotifyOnLoadComplete(Environment* penv, InteractiveObject* ptarget, int status)
{
    penv->Push(status);
    penv->Push(ptarget);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoadComplete"), 2, penv->GetTopIndex());
    penv->Drop2();
}

void MovieClipLoader::NotifyOnLoadInit(Environment* penv, InteractiveObject* ptarget)
{
    penv->Push(ptarget);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoadInit"), 1, penv->GetTopIndex());
    penv->Drop1();
}

void MovieClipLoader::NotifyOnLoadError(Environment* penv, InteractiveObject* ptarget, const char* errorCode, int status)
{
    penv->Push(status);
    penv->Push(penv->CreateConstString(errorCode));
    penv->Push(ptarget);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoadError"), 3, penv->GetTopIndex());
    penv->Drop3();
}

void MovieClipLoader::NotifyOnLoadProgress(Environment* penv, InteractiveObject* ptarget, int loadedBytes, int totalBytes)
{
    if (ptarget)
    {
        String path;
        ptarget->GetAbsolutePath(&path);
        ProgressDesc* pprogress = ProgressInfo.Get(path);
        if (pprogress)
        {
            pprogress->LoadedBytes = loadedBytes;
            pprogress->TotalBytes  = totalBytes;
        }
        else
        {
            ProgressInfo.Add(path, ProgressDesc(loadedBytes, totalBytes));
        }
    }

    penv->Push(totalBytes);
    penv->Push(loadedBytes);
    penv->Push(ptarget);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoadProgress"), 3, penv->GetTopIndex());
    penv->Drop3();
}

//////////////////////////////////////////
//
void MovieClipLoaderProto::GetProgress(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);
    fn.Result->SetUndefined();

    MovieClipLoader* pmovieClipLoader = NULL;
    if (fn.ThisPtr->GetObjectType() == Object::Object_MovieClipLoader)
    {
        pmovieClipLoader = static_cast<MovieClipLoader*>(fn.ThisPtr);

        Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

        ASString path = fn.Arg(0).ToString(fn.Env);
        ProgressDesc* pprogress = pmovieClipLoader->ProgressInfo.Get(path.ToCStr());
        if (pprogress)
        {
            pobj->SetConstMemberRaw(fn.Env->GetSC(), "bytesLoaded", Value(pprogress->LoadedBytes));
            pobj->SetConstMemberRaw(fn.Env->GetSC(), "bytesTotal",  Value(pprogress->TotalBytes));
        }
        fn.Result->SetAsObject(pobj);
    }
}

void MovieClipLoaderProto::LoadClip(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.NArgs < 2)
        return;

    SF_ASSERT(fn.ThisPtr);

    MovieClipLoader* pmovieClipLoader = NULL;
    if (fn.ThisPtr->GetObjectType() == Object::Object_MovieClipLoader)
        pmovieClipLoader = static_cast<MovieClipLoader*>(fn.ThisPtr);

    // Post loadMovie into queue.
    ASString urlStr(fn.Arg(0).ToString(fn.Env));
    Ptr<InteractiveObject> psprite;
    if (fn.Arg(1).IsCharacter())
        psprite = fn.Arg(1).ToCharacter(fn.Env);
    else
        psprite = fn.Env->FindTarget(fn.Arg(1).ToString(fn.Env));
    if (psprite)
        fn.Env->GetAS2Root()->AddLoadQueueEntry(psprite, urlStr.ToCStr(), LoadQueueEntry::LM_None, pmovieClipLoader);
    else
    {
        const char* ptail = "";
        int         level = MovieRoot::ParseLevelName(fn.Arg(1).ToString(fn.Env).ToCStr(), &ptail,
                                                         fn.Env->GetTarget()->IsCaseSensitive());
        if (level == -1)
            return;
        fn.Env->GetAS2Root()->AddLoadQueueEntry(fn.Arg(1).ToString(fn.Env).ToCStr(), urlStr.ToCStr(), fn.Env,
                                                  LoadQueueEntry::LM_None, pmovieClipLoader);
    }
    fn.Result->SetBool(true);
}

void MovieClipLoaderProto::UnloadClip(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);
    
    Ptr<InteractiveObject> psprite;
    if (fn.Arg(0).IsCharacter())
        psprite = fn.Arg(0).ToCharacter(fn.Env);
    else if (fn.Arg(0).IsNumber())
        psprite = fn.Env->GetAS2Root()->GetLevelMovie(fn.Arg(0).ToInt32(fn.Env));
    else
        psprite = fn.Env->FindTarget(fn.Arg(0).ToString(fn.Env));

    if (!psprite)
        return;

    // Post unloadMovie into queue (empty url == unload).
    fn.Env->GetAS2Root()->AddLoadQueueEntry(psprite, "");
    fn.Result->SetBool(true);
}

static const NameFunction GAS_MovieClipLoaderFuncTable[] = 
{
    { "getProgress",        &MovieClipLoaderProto::GetProgress       },
    { "loadClip",           &MovieClipLoaderProto::LoadClip          },
    { "unloadClip",         &MovieClipLoaderProto::UnloadClip        },
    { 0, 0 }
};

MovieClipLoaderProto::MovieClipLoaderProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<MovieClipLoader>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_MovieClipLoaderFuncTable);
}

//////////////////
MovieClipLoaderCtorFunction::MovieClipLoaderCtorFunction(ASStringContext *psc)
:   CFunctionObject(psc, GlobalCtor)
{
}

void MovieClipLoaderCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_MovieClipLoader && 
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        MovieClipLoader* ab = static_cast<MovieClipLoader*>(fn.ThisPtr);
        AsBroadcaster::AddListener(fn.Env, ab, ab);
        fn.Result->SetAsObject(ab);
    }
    else
        fn.Result->SetNull();
}

Object* MovieClipLoaderCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) MovieClipLoader(penv);
}

FunctionRef MovieClipLoaderCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) MovieClipLoaderCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) MovieClipLoaderProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_MovieClipLoader, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_MovieClipLoader), Value(ctor));
    return ctor;
}
}}} // SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_MOVIECLIPLOADER

