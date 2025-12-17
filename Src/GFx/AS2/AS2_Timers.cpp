/**************************************************************************

Filename    :   AS2_Timers.cpp
Content     :   
Created     :   August, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_Timers.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2 {

IntervalTimer::IntervalTimer(const FunctionRef& function, ASStringContext* psc) :
    Function(function), MethodName(psc->GetBuiltin(ASBuiltin_empty_)), Interval(0), InvokeTime(0), Id(0), 
    Active(true), Timeout(false)
{
    // Note: ignore 
}

IntervalTimer::IntervalTimer(Object* object, const ASString& methodName) :
    pObject(object), MethodName(methodName), Interval(0), InvokeTime(0), Id(0), Active(true), Timeout(false)
{
}

IntervalTimer::IntervalTimer(InteractiveObject* character, const ASString& methodName) :
    Character(character), MethodName(methodName), Interval(0), InvokeTime(0), Id(0), Active(true), Timeout(false)
{
}

void IntervalTimer::Start(MovieImpl* proot)
{
    SF_ASSERT(proot);

    UInt64 startTime = proot->GetTimeElapsedMks();
    InvokeTime = startTime + Interval;
}

bool IntervalTimer::Invoke(MovieImpl* proot, float frameTime)
{
    SF_AMP_SCOPE_TIMER(proot->AdvanceStats, "IntervalTimer::Invoke", Amp_Profile_Level_Medium);

    SF_ASSERT(proot);
    if (!Active) return false;
    GFx::AS2::MovieRoot* asroot = static_cast<GFx::AS2::MovieRoot*>(proot->pASMovieRoot.GetPtr());

    UInt64 currentTime = proot->GetTimeElapsedMks();
    bool retval = false;
    if (currentTime >= InvokeTime)
    {
        FunctionRef          function;
        ObjectInterface*     thisPtr = 0;
        Ptr<Object>         thisHolder;
        Ptr<InteractiveObject> targetHolder;
        Environment*         penv = 0;
        
        // invoke timer handle
        if (!Function.IsNull())
        {
            // just function
            function = Function;
        }
        else
        {
            thisHolder = Ptr<Object>(pObject);
            if (thisHolder)
            {
                // object
                thisPtr = thisHolder;

            }
            else if ((targetHolder = Ptr<InteractiveObject>(Character)))
            {
                // character
                thisPtr = ToAvmCharacter(targetHolder);
                penv    = ToAvmCharacter(targetHolder)->GetASEnvironment();
            }
            if (thisPtr)
            {
                Value methodVal;
                ASStringContext *psc = ToAvmCharacter(asroot->GetLevelMovie(0))->GetASEnvironment()->GetSC();
                if (thisPtr->GetMemberRaw(psc, MethodName, &methodVal))
                {
                    function = methodVal.ToFunction(penv);
                }
            }
            else // mark timer as inactive since thisPtr == NULL
                Active = false;
        }

        if (!function.IsNull())
        {
            Value result;
            if (!penv)
            {
                // if env is not set (it might be set only if character is used) then
                // get parent's _levelN's one.
                if (LevelHandle)
                {
                    Ptr<DisplayObject> levelCh = LevelHandle->ResolveCharacter(proot);
                    // levelCh can be NULL if level is unloaded already
                    if (levelCh)
                        penv = ToAvmCharacter(levelCh->CharToInteractiveObject())->GetASEnvironment();
                }
                if (!penv)
                {
                    // still no penv? Well, let get _level0 ones, but not
                    // sure it is correct... (AB)
                    penv = ToAvmCharacter(asroot->GetLevelMovie(0))->GetASEnvironment();
                }
            }
            SF_ASSERT(penv);

            int nArgs = (int)Params.GetSize();
            if (nArgs > 0)
            {
                for (int i = nArgs - 1; i >= 0; --i)
                    penv->Push(Params[i]);
            }
            function.Invoke(FnCall(&result, thisPtr, penv, nArgs, penv->GetTopIndex()));
            if (nArgs > 0)
            {
                penv->Drop(nArgs);
            }
        }

        if (Timeout)
        {
            // clear the timeout timer
            Active = false;
        }
        else
        {
            // set new invoke time
            UInt64 interval = GetNextInterval(currentTime, UInt64(frameTime * 1000000.f));
            if (interval > 0)
                InvokeTime += interval;
            else
                InvokeTime = currentTime; //?
        }
        retval = true;
    }
    return retval;
}

UInt64 IntervalTimer::GetNextInterval(UInt64 currentTime, UInt64 frameTime) const
{
    UInt64 interval; 
    if (Interval < frameTime/10) // make sure to have not more than 10 calls a frame
        interval = unsigned(frameTime/10);
    else
        interval = Interval;
    if (interval==0)
        return 0;
    return (((currentTime - InvokeTime) + interval)/interval)*interval;
}

void IntervalTimer::Clear()
{
    Active = false;
}

void IntervalTimer::SetInterval(const FnCall& fn)
{
    Set(fn, false);
}

void IntervalTimer::ClearInterval(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    MovieImpl* proot = fn.Env->GetMovieImpl();
    SF_ASSERT(proot);

    Number x = fn.Arg(0).ToNumber(fn.Env);
    if (!NumberUtil::IsNaN(x))
        proot->ClearIntervalTimer(int(x));
}

void IntervalTimer::SetTimeout(const FnCall& fn)
{
    Set(fn, true);
}

void IntervalTimer::ClearTimeout(const FnCall& fn)
{
    ClearInterval(fn);
}

void IntervalTimer::Set(const FnCall& fn, bool timeout)
{
    fn.Result->SetUndefined();
    if (fn.NArgs < 2)
        return;
    Ptr<IntervalTimer> timer;
    int nextArg = 1;
    MemoryHeap* pheap = fn.Env->GetHeap();

    if (fn.Arg(0).IsFunction())
    {
        // function variant
        timer = *SF_HEAP_NEW(pheap) IntervalTimer(fn.Arg(0).ToFunction(fn.Env), fn.Env->GetSC());
    }
    else if (fn.Arg(0).IsObject())
    {
        // object variant
        timer = *SF_HEAP_NEW(pheap) IntervalTimer(fn.Arg(0).ToObject(fn.Env), fn.Arg(1).ToString(fn.Env));
        ++nextArg;
    }
    else if (fn.Arg(0).IsCharacter())
    {
        // character variant
        timer = *SF_HEAP_NEW(pheap) IntervalTimer(fn.Arg(0).ToCharacter(fn.Env), fn.Arg(1).ToString(fn.Env));
        ++nextArg;
    }
    else
        return;
    
    if (fn.NArgs <= nextArg) // there is no timeout set
        return;
    
    if (fn.Env->GetTarget())
    {
        timer->LevelHandle = fn.Env->GetTarget()->GetTopParent()->GetCharacterHandle();
    }
    timer->Interval = UInt64(fn.Arg(nextArg++).ToNumber(fn.Env)) * 1000; // conv to mks
    timer->Timeout  = timeout;
    for (int i = nextArg; i < fn.NArgs; ++i)
    {
        timer->Params.PushBack(fn.Arg(i));
    }
    
    MovieImpl* proot = fn.Env->GetMovieImpl();
    SF_ASSERT(proot);

    int id = proot->AddIntervalTimer(timer);
    fn.Result->SetNumber((Number)id);

    timer->Start(proot);
}
}}} //SF::GFx::AS2
