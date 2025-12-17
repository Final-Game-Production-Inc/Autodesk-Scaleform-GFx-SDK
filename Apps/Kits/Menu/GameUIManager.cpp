/**************************************************************************

Filename    :   GameUIManager.cpp
Content     :   Sample UI Manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "GameUIManager.h"
#include "GameResourceManager.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

#define WHEEL_DELTA                     120

//////////////////////////////////////////////////////////////////////////

class GameUICallbackProxy : public GFx::ExternalInterface
{
public:
    GameUICallbackProxy(GameUISwfMovie* pui) : pUI(pui) {}

    virtual void    Callback(GFx::Movie* movie, const char* methodName, const GFx::Value *args, unsigned argCount)
    {
        SF_UNUSED(movie);
        if (pUI->Listener)
            pUI->Listener->OnUICallback(pUI, methodName, args, argCount);
    }

protected:
    GameUISwfMovie*     pUI;
};


//////////////////////////////////////////////////////////////////////////

GameUISwfMovie::~GameUISwfMovie()
{
    if (pSwf) Destroy();
}

bool    GameUISwfMovie::Initialize(GFx::MovieDef* movieDef, GameUIListener* xi, bool bNoAdvanceOnInit)
{
    Listener = xi;

    pSwfDef = movieDef;
    if (!pSwfDef) return false;

    Ptr<GFx::ActionControl> pactionControl = *new GFx::ActionControl();
    pSwf = *pSwfDef->CreateInstance(GFx::MemoryParams(), false, pactionControl);
    SF_ASSERT(pSwf);

    Ptr<GameUICallbackProxy> xicallback = *SF_HEAP_AUTO_NEW(this) GameUICallbackProxy(this);
    pSwf->SetExternalInterface(xicallback);

    pSwf->SetBackgroundAlpha(0.0f);
    if (!bNoAdvanceOnInit)
    {
        pSwf->Advance(0.0f, 0);
    }

    UInt64 timeStartTicks = Timer::GetTicks()/1000;
    NextTicksTime = timeStartTicks;
    MovieLastTicks = timeStartTicks;

    return true;
}

void    GameUISwfMovie::Destroy()
{
    GFx::Value p;
    if (pSwf->GetVariable(&p, "onUnload"))
    {
        pSwf->Invoke("onUnload", NULL);
    }
    p.SetUndefined();

    if (pSwf)
        pSwf = NULL;
}

bool    GameUISwfMovie::IsValid() const
{
    return pSwf.GetPtr() != NULL;
}

void    GameUISwfMovie::Pause(bool pause)
{
    if (!pSwf) return;

    GFx::Value p;
    if (pSwf->GetVariable(&p, "onPause"))
    {
        GFx::Value arg = pause;
        pSwf->Invoke("onPause", NULL, &arg, 1);
    }
    p.SetUndefined();

    pSwf->SetPause(pause);
}

void    GameUISwfMovie::Advance(GameUIAdvanceStats* pstats)
{
    if (!pSwf) return;

    if (Listener)
        Listener->OnPostAdvance(this);

    UInt64  currTicks = Timer::GetTicks();

    TimeTicks = currTicks / 1000;

    MovieTicks = TimeTicks;
    int deltaTicks = (int)(MovieTicks - MovieLastTicks);
    float deltaT = deltaTicks / 1000.f;
    MovieLastTicks  = MovieTicks;

    float timeTillNextTicks = pSwf->Advance(deltaT, 0);

    UInt64 advTicks = Timer::GetTicks()- currTicks;
    pstats->AdvanceTicks += advTicks;

    NextTicksTime = TimeTicks + (UInt32)(timeTillNextTicks * 1000.0f);
    if (NextTicksTime < TimeTicks) // wrap-around check.
        NextTicksTime = TimeTicks;

    if (Listener)
        Listener->OnPostAdvance(this);
}

void    GameUISwfMovie::SetViewport(Render::Size<unsigned> size)
{
    if (!pSwf) return;
    
    // Scale to fit the window always
    int viewWidth = size.Width;
    int viewHeight = size.Height;

    pSwf->SetViewport(size.Width, size.Height, (size.Width-viewWidth)/2, 
        (size.Height-viewHeight)/2, viewWidth, viewHeight);
}

bool    GameUISwfMovie::OnMouseEvent(const GFx::MouseEvent& e)
{
    if (!pSwf) return false;
    pSwf->HandleEvent(e);
    return true;
}

bool    GameUISwfMovie::OnKeyEvent(const GFx::KeyEvent& e)
{
    if (!pSwf) return false;
    pSwf->HandleEvent(e);
    return true;
}

bool    GameUISwfMovie::OnCharEvent(const GFx::CharEvent& e)
{
    if (!pSwf) return false;
    pSwf->HandleEvent(e);
    return true;
}

//////////////////////////////////////////////////////////////////////////

GameUIManager::GameUIManager() : pResourceMgr(NULL)
{
}

GameUIManager::~GameUIManager()
{
}

void    GameUIManager::PushUI(GameUISwfMovie* pmov)
{
    UIStack.PushBack(pmov);
    pResourceMgr->GetRenderThread()->AddDisplayHandle(pmov->GetSwf()->GetDisplayHandle(), Platform::RenderThread::DHCAT_Normal, false);
    SF_DEBUG_MESSAGE1(1, "UI:Push (new sz: %d)\n", UIStack.GetSize());
}

void    GameUIManager::PopUI()
{
    UIStack.PopBack();
    pResourceMgr->GetRenderThread()->RemoveDisplayHandle();
    SF_DEBUG_MESSAGE1(1, "UI:Pop (new sz: %d)\n", UIStack.GetSize());
}

void    GameUIManager::Initialize(GameResourceManager* presourceMgr)
{
    pResourceMgr = presourceMgr;
}

void    GameUIManager::Shutdown()
{
    pResourceMgr->GetRenderThread()->ClearDisplayHandles();
    for (unsigned i = 0; i < UIStack.GetSize(); i++)
    {
        GameUISwfMovie* ui = UIStack[i];
        //
        ui->Destroy();
        //
    }
}

void    GameUIManager::SetViewport(Render::Size<unsigned>& viewport)
{
    for (unsigned i = 0; i < UIStack.GetSize(); i++)
    {
        GameUISwfMovie* ui = UIStack[i];
        //
        ui->SetViewport(viewport);
        //
    }
}
