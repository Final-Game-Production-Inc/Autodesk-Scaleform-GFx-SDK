/**************************************************************************

Filename    :   AS3_AvmStage.h
Content     :   Implementation of AS3-dependent part of Stage.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_AS3_AvmStage_H
#define INC_SF_GFx_AS3_AvmStage_H

#include "GFx/AS3/AS3_AvmDisplayObjContainer.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/GFx_Sprite.h"

namespace Scaleform {
namespace GFx {

namespace AS3 {

class MovieRoot;

class FrameExecSnapshot
{
    ArrayDH<Ptr<Sprite> > mArray;
public:
    FrameExecSnapshot(MemoryHeap* pheap) : mArray(pheap) {}

    UPInt GetSize() const { return mArray.GetSize(); }
    Sprite* operator[](UPInt i) const { return mArray[i]; }

    void PushBack(Sprite* d)
    {
        mArray.PushBack(d);
    }
};


// The Stage class. This is a top level container in AS3 hierarchy.
class Stage : public DisplayObjContainer
{
    Ptr<DisplayObjContainer>    pRoot;
    Ptr<InteractiveObject>      FrameCounterObj;
    ASString                    CurrentStageOrientation;
    unsigned                    MouseCursorEventCnt;
public:
    Stage(MovieDefImpl* pbindingDefImpl, 
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id);
    ~Stage();

    virtual unsigned        GetLoadingFrame() const { return 1; }
    virtual bool            IsFocusEnabled(FocusMovedType = GFx_FocusMovedByKeyboard) const { return false; }

    // initialize the stage
    virtual void            ExecuteFrame0Events();
    virtual void            ExecuteFrameTags(unsigned frame);
    virtual void            OnInsertionAsLevel(int level);

    // methods to be redirected to root movie
    virtual unsigned        GetCurrentFrame() const;
    virtual bool            GetLabeledFrame(const char* plabel, unsigned* frameNumber, bool translateNumbers = 1);
    virtual const String*   GetFrameLabel(unsigned, unsigned* = NULL);
    virtual void            GotoFrame(unsigned targetFrameNumber);

    void SetHasMouseCursorEventFlag()   { ++MouseCursorEventCnt; }
    void ClearHasMouseCursorEventFlag() { SF_ASSERT(MouseCursorEventCnt > 0); --MouseCursorEventCnt; }
    bool HasMouseCursorEvent() const    { return MouseCursorEventCnt != 0; }

    DisplayObjContainer*    GetRoot() const
    {
        return pRoot;
    }
    void                    SetRoot(DisplayObjContainer* proot)
    {
        pRoot = proot;
    }

    virtual void            ForceShutdown ();

    virtual InteractiveObject* GetTopParent(bool ignoreLockRoot = false) const 
    { 
        if (!pParentChar)
            return const_cast<Stage*>(this);
        return pParentChar->GetTopParent(ignoreLockRoot);
    }

    void                    QueueFrameActions();
#if defined(GFX_ENABLE_MOBILE_APP_SUPPORT)
    void                    OnAppLifecycleEvent(const char* event);
    // newOrientation is "default", "rotatedLeft", "rotatedRight", "unknown" or "upsideDown"
    void                    OnDeviceOrientationChange(const char* newOrientation);
#endif

    const ASString&         GetCurrentStageOrientation() const { return CurrentStageOrientation; }
    void                    SetOrientation(const ASString& newOrient);
protected:
    void                    PropagateMouseEvent(const EventId& id);

    virtual ASString        CreateNewInstanceName() const;
};

// AVM-dependable part of Stage class.
class AvmStage : 
    public AvmDisplayObjContainer
{
public:
    AvmStage(DisplayObjContainer* pobj) : AvmDisplayObjContainer(pobj) {}
    virtual ~AvmStage();

    virtual void                OnEventLoad();
    virtual void                OnEventUnload();

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    virtual bool                OnEvent(const EventId& id);
#endif

    //virtual void                OnRemoved() {}
    virtual const char*         GetDefaultASClassName() const { return "flash.display.Stage"; }

    virtual bool                IsStage() const { return true; }

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::Stage"; }
#endif
};


}}} // SF::GFx::AS3
#endif // INC_SF_GFx_AS3_AvmStage_H

