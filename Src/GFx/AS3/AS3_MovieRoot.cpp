/**************************************************************************

Filename    :   AS3_MovieRoot.cpp
Content     :   Implementation of AS3 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_AbcDataBuffer.h"
#include "GFx/GFx_ASString.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/AS3/Obj/AS3_Obj_Function.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObjectContainer.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Loader.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_LoaderInfo.h"
#include "GFx/AS3/AS3_AsString.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS3/Obj/AS3_Obj_Array.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequest.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequestHeader.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLLoader.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLVariables.h"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_ByteArray.h"
#include "GFx/AS3/AS3_AvmBitmap.h"
#include "GFx/AS3/Obj/GFx/AS3_Obj_Gfx_MouseCursorEvent.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_Event.h"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_ByteArray.h"
#include "GFx/AS3/Obj/System/AS3_Obj_System_ApplicationDomain.h"
#include "GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_Accelerometer.h"
#include "GFx/AS3/Obj/Sensors/AS3_Obj_Sensors_Geolocation.h"
#include "GFx/AS3/Obj/External/AS3_Obj_External_ExtensionContext.h"

#include "Kernel/SF_AmpInterface.h"
#include "GFx/AS3/AS3_SocketThreadMgr.h"

#define ENABLE_FILE_UNLOADING

namespace Scaleform { namespace GFx { namespace AS3 {
///

ASVM::ASVM(MovieRoot* pmr, FlashUI& _ui, AS3::StringManager& sm, ASRefCountCollector& gc)
: VM(_ui, sm, gc SF_AOTC_ARG(NULL) SF_AOTC2_ARG(NULL))
, pMovieRoot(pmr)
, ExtensionsEnabled(false)
{
    GraphicsClass = GetClass("flash.display.Graphics", GetCurrentAppDomain());
    TransformClass = GetClass("flash.geom.Transform", GetCurrentAppDomain());
    MatrixClass = GetClass("flash.geom.Matrix", GetCurrentAppDomain());
    Matrix3DClass = GetClass("flash.geom.Matrix3D", GetCurrentAppDomain());
    PerspectiveProjectionClass = GetClass("flash.geom.PerspectiveProjection", GetCurrentAppDomain());
    ColorTransformClass = GetClass("flash.geom.ColorTransform", GetCurrentAppDomain());
    EventClass = GetClass("flash.events.Event", GetCurrentAppDomain());
    MouseEventClass = GetClass("flash.events.MouseEvent", GetCurrentAppDomain());
    KeyboardEventClass = GetClass("flash.events.KeyboardEvent", GetCurrentAppDomain());
    FocusEventClass = GetClass("flash.events.FocusEvent", GetCurrentAppDomain());
    TextEventClass = GetClass("flash.events.TextEvent", GetCurrentAppDomain());
    
#if defined(GFX_ENABLE_ANALOG_GAMEPAD)
    GamePadAnalogEventClass = GetClass("scaleform.gfx.GamePadAnalogEvent", GetCurrentAppDomain());
#endif

    MouseEventExClass = GetClass("scaleform.gfx.MouseEventEx", GetCurrentAppDomain());
    KeyboardEventExClass = GetClass("scaleform.gfx.KeyboardEventEx", GetCurrentAppDomain());
    FocusEventExClass = GetClass("scaleform.gfx.FocusEventEx", GetCurrentAppDomain());
    TextEventExClass = GetClass("scaleform.gfx.TextEventEx", GetCurrentAppDomain());

    TimerEventClass = GetClass("flash.events.TimerEvent", GetCurrentAppDomain());
    ProgressEventClass = GetClass("flash.events.ProgressEvent", GetCurrentAppDomain());
#ifdef SF_AS3_VERSION_AIR
    OutputProgressEventClass = GetClass("flash.events.OutputProgressEvent", GetCurrentAppDomain());
#endif

#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    TouchEventClass = GetClass("flash.events.TouchEvent", GetCurrentAppDomain());
    GestureEventClass = GetClass("flash.events.GestureEvent", GetCurrentAppDomain());
    TransformGestureEventClass = GetClass("flash.events.TransformGestureEvent", GetCurrentAppDomain());
    PressAndTapGestureEventClass = GetClass("flash.events.PressAndTapGestureEvent", GetCurrentAppDomain());
#endif

#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    AppLifecycleEventClass = GetClass("flash.events.AppLifecycleEvent", GetCurrentAppDomain());
    StageOrientationEventClass = GetClass("flash.events.StageOrientationEvent", GetCurrentAppDomain());
#endif
	StatusEventClass = GetClass("flash.events.StatusEvent", GetCurrentAppDomain());

    AccelerometerEventClass = GetClass("flash.events.AccelerometerEvent", GetCurrentAppDomain());
    GeolocationEventClass = GetClass("flash.events.GeolocationEvent", GetCurrentAppDomain());

    PointClass = GetClass("flash.geom.Point", GetCurrentAppDomain());
    RectangleClass = GetClass("flash.geom.Rectangle", GetCurrentAppDomain());
    TextFormatClass = GetClass("flash.text.TextFormat", GetCurrentAppDomain());
    EventDispatcherClass = GetClass("flash.events.EventDispatcher", GetCurrentAppDomain());
    Vector3DClass = GetClass("flash.geom.Vector3D", GetCurrentAppDomain());


    SF_ASSERT(GraphicsClass);
    SF_ASSERT(TransformClass);
    SF_ASSERT(MatrixClass);
    SF_ASSERT(Matrix3DClass);
    SF_ASSERT(PerspectiveProjectionClass);
    SF_ASSERT(ColorTransformClass);
    SF_ASSERT(EventClass);
    SF_ASSERT(MouseEventClass);
    SF_ASSERT(KeyboardEventClass);
    SF_ASSERT(FocusEventClass);
    SF_ASSERT(TextEventClass);

#if defined(GFX_ENABLE_ANALOG_GAMEPAD)
    SF_ASSERT(GamePadAnalogEventClass);
#endif

    SF_ASSERT(MouseEventExClass);
    SF_ASSERT(KeyboardEventExClass);
    SF_ASSERT(FocusEventExClass);
    SF_ASSERT(TextEventExClass);

    SF_ASSERT(TimerEventClass);
    SF_ASSERT(ProgressEventClass);
#ifdef SF_AS3_VERSION_AIR
    SF_ASSERT(OutputProgressEventClass);
#endif

#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    SF_ASSERT(TouchEventClass);
    SF_ASSERT(GestureEventClass);
    SF_ASSERT(TransformGestureEventClass);
    SF_ASSERT(PressAndTapGestureEventClass);
#endif
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    //SF_ASSERT(AppLifecycleEventClass);
    SF_ASSERT(StageOrientationEventClass);
#endif

	SF_ASSERT(StatusEventClass);
    SF_ASSERT(AccelerometerEventClass);
    SF_ASSERT(GeolocationEventClass);
    SF_ASSERT(PointClass);
    SF_ASSERT(RectangleClass);
    SF_ASSERT(TextFormatClass);
    SF_ASSERT(EventDispatcherClass);
    SF_ASSERT(Vector3DClass);
}

#ifdef SF_AMP_SERVER
AMP::ViewStats* ASVM::GetAdvanceStats() const
{
    return &pMovieRoot->GetMovieImpl()->GetAdvanceStats();
}
#endif


bool ASVM::_constructInstance(SPtr<Object> &pobj, Object* classObj,
                              unsigned argc, const Value* argv)
{
    Value v;
    classObj->Construct(v, argc, argv, true);

    if (!IsException() && !v.IsNullOrUndefined())
    {
        pobj = v.GetObject();
        return true;
    }        
    pobj = NULL;
    if (IsException())
        IgnoreException();
    return false;
}

// returns a moviedef for the last abc in call stack
MovieDefImpl* ASVM::GetResourceMovieDef(Instances::fl::Object* instance) const
{
    VMAbcFile* vmabc = instance->GetTraits().GetClass().GetTraits().GetFilePtr();
    if (vmabc)
    {
        const AbcFileWithMovieDef& abc = 
            static_cast<const AbcFileWithMovieDef&>(vmabc->GetAbcFile());
        // null is allowed!
        return abc.pDefImpl;
    }
    else
    {
        // call stack is empty - get the main moviedef
        return pMovieRoot->GetMovieDefImpl();
    }
}

MovieDefRootNode::~MovieDefRootNode() 
{ 
    for (SPInt i = (SPInt)AbcFiles.GetSize() - 1; i >= 0; --i)
    {
        SF_DEBUG_MESSAGE2(GFX_UNLOAD_TRACE_ENABLED, 
            "Releasing VMAbcFile %s (%p)\n", AbcFiles[i]->GetAbcFile().GetName().ToCStr(),
            AbcFiles[i].GetPtr());
        AbcFiles[i] = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
MovieRoot::MovieRoot(MemoryContextImpl* memContext, MovieImpl* pmovie, ASSupport* pas) 
: 
    ASMovieRootBase(pas),
    MemContext(memContext),
    NumAdvancesSinceCollection(0),
    LastCollectionFrame(0),
    ActionQueue(memContext->Heap),
    BuiltinsMgr(MemContext->StringMgr),
    StageInvalidated(false),
    Flags(0)
{
//!`    CheckClipsCnt = 0;    
    ASFramesToExecute = 0;
    pMovieImpl = pmovie;
    pMovieImpl->SetASMovieRoot(this);
    CreateObjectInterface(pmovie);
#ifdef GFX_ENABLE_KEYBOARD
    pmovie->SetKeyboardListener(this);
#endif
    pInvokeAliases     = NULL;

    pmovie->SetDisableFocusAutoRelease(true);
    pmovie->SetDisableFocusRolloverEvent(true);

    MainLoaderInfoEventsState = 0;
#ifdef SF_ENABLE_SOCKETS
    SocketFactory = Socket::GetAs3DefaultFactory();
#endif //SF_ENABLE_SOCKETS
}

MovieRoot::~MovieRoot()
{
}

void MovieRoot::Shutdown()
{
    GetMovieImpl()->ClearPlayList();

    ActionQueue.Clear();
    pStage = NULL;
    ForceCollect(Movie::GCF_Full);

    // Reset invoke aliases set by ExternalInterface.addCallback
    delete pInvokeAliases; pInvokeAliases = NULL;
    //AbcFiles.Clear();

    // before releasing AVM we need to release all Values.
    ExternalIntfRetVal.SetUndefined();

    mEventChains.Shutdown();

    // cleanup mousestate
    for (UPInt i = 0; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
    {
        mMouseState[i].Clear();
    }

    HashIdentityLH<MovieDefImpl*, LoadedMovieDefInfo>::Iterator it = LoadedMovieDefs.Begin();
    for (; it != LoadedMovieDefs.End(); ++it)
    {
        if (it->First)
            it->First->RemoveReleaseNotifier(this);
    }

    // traverse all RootNodes and cleanup VMAbcFile ptrs there.
    GFx::MovieDefRootNode *pdefNode = GetMovieImpl()->RootMovieDefNodes.GetFirst();
    while(!GetMovieImpl()->RootMovieDefNodes.IsNull(pdefNode))
    {   
        static_cast<MovieDefRootNode*>(pdefNode)->AbcFiles.Clear();

        pdefNode = pdefNode->pNext;
    }

    ForceCollect(Movie::GCF_Full);
    pAVM->UnregisterAllAbcFiles();
    ForceCollect(Movie::GCF_Full);
    pAVM = NULL;
}

bool MovieRoot::Init(MovieDefImpl* pmovieDef) 
{ 
    // do we need to create VM always? SWF files with no AS are possible.
    // But there are some problems with handling input or focus w/o VM:
    // we either need to check for presence of AS3 objects/VM always or
    // to create them on fly. @TODO
    CheckAvm();

    // Create stage first
    CreateStage(pmovieDef);

    pMovieImpl->SetLevelMovie(0, pStage);

    // Register aux AS classes defined in external libraries
#ifdef GFX_ENABLE_VIDEO
    Ptr<Video::VideoBase> pvideo = pMovieImpl->pStateBag->GetVideo();
    if (pvideo && pvideo->GetAS3Support())
    {
        MovieRoot* proot = ToAS3Root(pMovieImpl);
        pvideo->GetAS3Support()->RegisterASClasses(proot->GetAVM());
    }
#endif

    // In case permanent variables were assigned.. check them.
    ResolveStickyVariables(pStage);
    return true;
}

bool MovieRoot::CheckAvm()
{
    if (!pAVM)
    {
        // turn on VM tracing
        ActionControl* pac = pMovieImpl->GetActionControl();
        if (pac != NULL)
        {
            SetNeedToCheckOpCode((pac->GetActionFlags() & ActionControl::Action_Verbose) != 0);
        }
        else
        {
            SetNeedToCheckOpCode(false);
        }
        SetState(FlashUI::sStep);

        pAVM = SF_HEAP_NEW(GetMovieHeap()) ASVM(this, *this, *GetStringManager(), *(MemContext->ASGC));
        pAVM->ExecuteCode();
    }
    return (pAVM.GetPtr() != NULL);
}

bool MovieRoot::ExecuteAbc(AbcDataBuffer* pabc, DisplayObjContainer* m)
{
    bool result = false;
    MovieDefImpl* pdefImpl = m->GetResourceMovieDef();

    String fileName(pabc->FileName);
    if (!pabc->Name.IsEmpty())
    {   
        fileName += "[";
        fileName += pabc->Name;
        fileName += "]";
    }

    // find already existing AbcFile first
    VMAppDomain& appDomain = ToAvmDisplayObjContainer(m)->GetAppDomain();
    SPtr<VMAbcFile> vmAbc = pAVM->FindVMAbcFileWeak(fileName, appDomain);

    if (!vmAbc)
    {
        AutoPtr<Abc::Reader> pReader = SF_HEAP_NEW(GetMovieHeap()) Abc::Reader(pabc->Data, pabc->DataSize);
        Ptr<Abc::File>  pAbcFile = *SF_HEAP_NEW(GetMovieHeap()) ASVM::AbcFileWithMovieDef(pdefImpl, pabc);

        //pAbcFile->SetSource(pdefImpl->GetFileURL());
        pAbcFile->SetSource(fileName);
        pAbcFile->SetDataSize(pabc->DataSize);
#ifdef SF_AMP_SERVER
        //    SF_ASSERT(pabc->SwfHandle != 0);
        pAbcFile->SetFileHandle(pabc->SwfHandle);
        pAbcFile->SetSwfFileOffset(pabc->SwfOffset);
#endif

        result = pReader->Read(*pAbcFile);
        if (result)
        {
            MovieRoot::CheckAvm();

#if 0 // SS development
            if (pAVM->LoadFile(pAbcFile) && !pAVM->IsException())
            {
                AbcFiles.PushBack(pAbcFile);
            }

            if (pAVM->IsException())
            {
                pAVM->OutputAndIgnoreException();
                result = false;
            }
#else
            //AbcFiles.PushBack(pAbcFile); //@TODO: more intellectual way of storing, 
            vmAbc = pAVM->LoadFile(pAbcFile, appDomain);
            if (pAVM->IsException())
            {
                pAVM->OutputAndIgnoreException();
                result = false;
            }
#endif
            // what if swf was unloaded and then loaded again? duplicated abc file?
        }
    }
    else
        result = true;
    // it might return null?
    if (vmAbc)
    {
        MovieDefRootNode* rootNode = static_cast<MovieDefRootNode*>(m->FindRootNode());
        SF_ASSERT(rootNode);
        rootNode->AbcFiles.PushBack(vmAbc);
    }
    return result;
}

// FlashUI methods
void MovieRoot::Output(FlashUI::OutputMessageType type, const char* msg)
{
    Log* log = pMovieImpl->GetCachedLog();
    if (!log)
        return;

    LogMessageId logId;

    switch(type)
    {
    case FlashUI::Output_Action:    logId = Log_Action; break;
    case FlashUI::Output_Error:     logId = Log_ScriptError; break;
    case FlashUI::Output_Warning:   logId = Log_ScriptWarning; break;
    case FlashUI::Output_Message:
    default:
        logId = Log_Message;
    }
    log->LogMessageById(logId, "%s", msg);
}


bool MovieRoot::OnOpCode(Abc::TCodeOffset offset)
{   
    SF_UNUSED1(offset);
#ifdef GFX_AS3_TRACE
    // Trace VM state and opcode.
    AS3::TraceVM(*pAVM, "\t");
    AS3::TraceOpCode(*this, *pAVM, offset);
#endif
    return false;
}

// UPInt MovieRoot::GetSize() const
// {
//     return AbcFiles.GetSize();
// }
// 
// const SPtr<Abc::File>& MovieRoot::GetFile(UPInt n)
// {
//     return AbcFiles[n];
// }
// 
// const AS3::Abc::File* MovieRoot::GetFile(const ASString&)
// {
//     return NULL;
// }

MovieRoot::MouseState::DoubleClickInfo& MovieRoot::MouseState::GetDoubleClickInfo(unsigned buttonMask)
{
    unsigned idx; 
    if (buttonMask > (1<<2))
        idx = (unsigned)logf((float)buttonMask);
    else
    {
        static const unsigned ll[] = { 0, 1, 2, 0, 3 };
        idx = ll[buttonMask];
    }
    SF_ASSERT(idx < sizeof(DblClick)/sizeof(DblClick[0]));
    return DblClick[idx];
}

void    MovieRoot::NotifyOnResize()
{
	QueueEventChain(EventId::Event_Resize);
}

void    MovieRoot::GenerateMouseEvents(unsigned mouseIndex)
{
    GFx::MouseState& ms = *pMovieImpl->GetMouseState(mouseIndex);
    SF_ASSERT(&ms);
    Ptr<InteractiveObject>  ActiveEntity  = ms.GetActiveEntity();
    Ptr<InteractiveObject>  TopmostEntity = ms.GetTopmostEntity();

    MovieImpl::DragState    st;
    pMovieImpl->GetDragState(mouseIndex, &st);
    if (st.MouseIndex == mouseIndex)
    {
        // Check if we are currently dragging a character. If yes, substitute the actual TopmostEntity
        // by the one being dragged to avoid unwanted over/out mouse events.
        // See test_mouseOver_out_while_dragging.swf
        if (st.pCharacter)
        {
            // See test_mouseOver_out_while_dragging_mouseDisabled.swf
            if (!st.pCharacter->IsMouseDisabledFlagSet())
            {
                //@DBG
                //printf("!! %s  tm= %s  ac= %s\n", 
                // st.pCharacter->GetName().ToCStr(), (TopmostEntity)?TopmostEntity->GetName().ToCStr():"...", 
                // (ActiveEntity)?ActiveEntity->GetName().ToCStr():"...");
                if (!st.pTopmostEntity || !ToAvmInteractiveObj(st.pTopmostEntity)->IsStageAccessible())
                    pMovieImpl->SetDragStateTopmostEntity(mouseIndex, TopmostEntity);
                else
                    TopmostEntity = st.pTopmostEntity;
            }
        }
    }

    if (ActiveEntity && ActiveEntity->IsUnloaded())
        ActiveEntity = NULL;
    if (TopmostEntity && TopmostEntity->IsUnloaded())
        TopmostEntity = NULL;

    Ptr<InteractiveObject>  PrevActiveEntity = ActiveEntity;

    KeyModifiers keyMods = 0;
#ifdef GFX_ENABLE_KEYBOARD
    KeyboardState* keybSt = GetMovieImpl()->GetKeyboardState(mouseIndex);
    keyMods = keybSt->GetKeyModifiers();
#endif

    UInt8 checkCount = (GetAVM()->ExtensionsEnabled) ? 16 : 1;
    unsigned changeMask = ms.GetPrevButtonsState() ^ ms.GetButtonsState();

    // If topmostEntity is NULL then mouse cursor is above the stage. Flash generates
    // mouse events for stage as a target is this case. 
    InteractiveObject*      
        topmostEntitySafe = (TopmostEntity) ? TopmostEntity.GetPtr() : (InteractiveObject*)pStage.GetPtr();

    SF_ASSERT(topmostEntitySafe);
    if (ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left)
    {
        // Mouse button was down.
        // Handle onDragOut, onDragOver
        if (!ms.IsMouseInsideEntityLast())
        {
            if (TopmostEntity == ActiveEntity)
            {
                // onDragOver
                if (ActiveEntity)  
                {
                    ButtonEventId evt
                        (EventId::Event_DragOver, mouseIndex, ActiveEntity->IncrementRollOverCnt());
                    evt.KeysState = keyMods;
                    ActiveEntity->OnMouseEvent(evt);
                }

                ms.SetMouseInsideEntityLast(true);
            }
        }
        else
        {
            // MouseInsideEntityLast == true
            if (TopmostEntity != ActiveEntity)
            {
                // onDragOut
                if (ActiveEntity)
                {
                    ButtonEventId evt
                        (EventId::Event_DragOut, mouseIndex, ActiveEntity->DecrementRollOverCnt());
                    evt.KeysState = keyMods;
                    ActiveEntity->OnMouseEvent(evt);
                }
                ms.SetMouseInsideEntityLast(false);
            }
        }
    }

    // RollOver/MouseOver events
    if (TopmostEntity != ActiveEntity)
    {
        // rollOut
        UPInt rosSz = mMouseState[mouseIndex].RolloverStack.GetSize();
        SPInt i;
        for (i = (SPInt)rosSz - 1; i >= 0; --i)
        {
            // check the stack of already "rollovered" characters.
            // If the new character is not the child of the one in the stack - 
            // fire rollOut and remove from stack. Stop, when the element char
            // actually is the parent of ActiveEntity.
            if (!TopmostEntity || (mMouseState[mouseIndex].RolloverStack[i] != TopmostEntity &&
                !mMouseState[mouseIndex].RolloverStack[i]->IsAncestor(TopmostEntity)))
            {
                ButtonEventId evt
                    (EventId::Event_RollOut, mouseIndex, mMouseState[mouseIndex].RolloverStack[i]->DecrementRollOverCnt());
                evt.KeysState = keyMods;
                mMouseState[mouseIndex].RolloverStack[i]->OnMouseEvent(evt);              
            }
            else
                break;
        }
        mMouseState[mouseIndex].RolloverStack.PopBack(rosSz - i - 1);

        if (ActiveEntity)
        {
            // mouseOut
            // check if the new entity is the child
            mMouseState[mouseIndex].LastMouseOverObj = ActiveEntity;
            ButtonEventId evt
                (EventId::Event_MouseOut, mouseIndex, ToAvmInteractiveObj(ActiveEntity)->DecrementMouseOverCnt());
            evt.KeysState = keyMods;
            ActiveEntity->OnMouseEvent(evt);   

            // clean up
            mMouseState[mouseIndex].LastMouseOverObj = NULL;
        }
    }

    // mouse button release
    for (UInt8 buttonIdx = 0; buttonIdx < checkCount; ++buttonIdx)   // 16 is max number of mouse buttons supported
    {
        unsigned buttonMask = 1 << buttonIdx;
        // Handle button state change
        if (((changeMask >> buttonIdx) & 0x1) != 0)
        {
            // Button was down, now up
            if (ms.GetPrevButtonsState() & buttonMask)
            {
                MouseState::DoubleClickInfo& dci = 
                    mMouseState[mouseIndex].GetDoubleClickInfo(buttonIdx);
                if (ms.GetMouseButtonDownEntity(buttonIdx))
                {
                    ButtonEventId evt(EventId::Event_ReleaseOutside, mouseIndex, 0, buttonIdx);
                    evt.KeysState = keyMods;
                    ms.GetMouseButtonDownEntity(buttonIdx)->OnMouseEvent(evt);
                }
                ButtonEventId mouseUpEvt(EventId::Event_MouseUp, mouseIndex, 0, buttonIdx);
                mouseUpEvt.KeysState = keyMods;
                topmostEntitySafe->OnMouseEvent(mouseUpEvt);
                if (ms.IsMouseInsideEntityLast())
                {
                    bool dblClick = false;
                    UInt32 tm = (UInt32)(Timer::GetTicks() / 1000);
                    if (dci.PrevClickTime != 0)
                    {
                        PointF pt = ms.GetLastPosition();
                        int ax = (int)TwipsToPixels(pt.x);
                        int ay = (int)TwipsToPixels(pt.y);
                        if (tm <= dci.PrevClickTime + GFX_DOUBLE_CLICK_TIME_MS && 
                            (int)TwipsToPixels(dci.PrevPosition.x) == ax && 
                            (int)TwipsToPixels(dci.PrevPosition.y) == ay)
                        {
                            dblClick = true;
                            //printf("DOUBLE CLICK!\n");
                        }
                    }
                    if (dblClick && ToAvmInteractiveObj(topmostEntitySafe)->IsDoubleClickEnabled())
                    {
                        ButtonEventId evt(EventId::Event_DoubleClick, mouseIndex, 0, buttonIdx);
                        evt.KeysState = keyMods;
                        topmostEntitySafe->OnMouseEvent(evt);
                    }
                    // 10/19/2010, NFM: Only fire Click if current ActiveEntity == ActiveEntity when the last MouseDown occurred.
                    // 3/23/2011, AB: changed ActiveEntity to topmostEntitySafe since otherwise Stage doesn't receive clicks
                    // (if clicked on bare stage).
                    else if (topmostEntitySafe == ms.GetMouseButtonDownEntity(buttonIdx)) 
                    {
                        ButtonEventId evt(EventId::Event_Click, mouseIndex, 0, buttonIdx);
                        evt.KeysState = keyMods;
                        topmostEntitySafe->OnMouseEvent(evt);                    
                    }
                    if (dblClick) 
                        dci.PrevClickTime = 0; 
                    else 
                        dci.PrevClickTime = tm;
                    dci.PrevPosition  = ms.GetLastPosition();
                }
                ms.SetMouseButtonDownEntity(buttonIdx, NULL);
            }
        }
    }

    // bool trackAsMenu = (ActiveEntity) ? ActiveEntity->GetTrackAsMenu() : false;
    if (ms.GetWheelDelta() != 0)
    {
        ButtonEventId evt(EventId::Event_MouseWheel, mouseIndex);
        evt.KeysState       = keyMods;
        evt.MouseWheelDelta = (SInt8)ms.GetWheelDelta();
        topmostEntitySafe->OnMouseEvent(evt);
    }

    if (ms.IsMouseMoved())
    {
        //printf("tp %s, \n", topmostEntitySafe->GetName().ToCStr());
        ButtonEventId evt(EventId::Event_MouseMove, mouseIndex);
        evt.KeysState       = keyMods;
        evt.MouseWheelDelta = (SInt8)ms.GetWheelDelta();
        topmostEntitySafe->OnMouseEvent(evt);

        // a special case to generate PropagateMouseEvent in the case if mouse
        // 'captured' (left button pressed and mouse moved). This is necessary for
        // textfields to provide scrolling or selection by mouse when mouse is
        // outside of the textfield.
        if (ms.GetMouseButtonDownEntity(0) && ms.GetMouseButtonDownEntity(0) != topmostEntitySafe)
        {
            if (ms.GetMouseButtonDownEntity(0)->GetType() == CharacterDef::TextField)
            {
                ButtonEventId bevt(EventId::Event_MouseMove, mouseIndex, 0, 0);
                bevt.KeysState = keyMods;
                ms.GetMouseButtonDownEntity(0)->PropagateMouseEvent(bevt);
            }
        }
    }

    /*
    if (topmostEntitySafe->GetTrackAsMenu())
        trackAsMenu = true;
    */

    //printf("%d %x %x %p %p\n", mouseIndex, ms.GetButtonsState(), ms.GetPrevButtonsState(), TopmostEntity.GetPtr(), ActiveEntity.GetPtr());

    // 10/19/2010, NFM: Commented out LeftMouseButton || TrackAsMenu check fix a bug where 
    //                  ROLL_OVER/MOUSE_OVER/MOUSE_OUT/ROLL_OUT were not fired when the left Mouse button was pressed.
    // if ((ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left) == 0 || 
    //     trackAsMenu)
    //{
        // Mouse button was up.
        //printf("%p %p\n", TopmostEntity.GetPtr(), ActiveEntity.GetPtr());

    // New active entity is whatever is below the mouse right now.
    if (TopmostEntity != ActiveEntity)
    {
        ActiveEntity = TopmostEntity;

        if (ActiveEntity)
        {
            // rollOver
            bool fireRollover = true;
            UPInt rosSz = mMouseState[mouseIndex].RolloverStack.GetSize();
            if (rosSz)
            {
                for (SPInt i = (SPInt)rosSz - 1; i >= 0; --i)
                {
                    if (mMouseState[mouseIndex].RolloverStack[i] == ActiveEntity)
                    {
                        fireRollover = false;
                        break;
                    }
                }
            }
            if (fireRollover)
            {
                // Seems to me rollover/rollout are fired for each parent up to the "root" (not for stage)
                // This happens anyway on capture and bubble stage, but Flash fires rollover/out
                // for each parent separately (having each parent as a "target").
                Ptr<InteractiveObject> cur = ActiveEntity;
                InteractiveObject* stage = GetStage();
                UPInt idx = mMouseState[mouseIndex].RolloverStack.GetSize();
                for (; cur && cur != stage; cur = cur->GetParent())
                {
                    if (idx != 0)
                    {
                        // check if we already have the the char in the stack; 
                        // if so, stop here.
                        if (mMouseState[mouseIndex].RolloverStack[idx - 1] == cur)
                            break;
                    }
                    ButtonEventId evt
                        (EventId::Event_RollOver, mouseIndex, cur->IncrementRollOverCnt());
                    evt.KeysState = keyMods;
                    cur->OnMouseEvent(evt);
                    mMouseState[mouseIndex].RolloverStack.InsertAt(idx, cur);
                }
            }
            
            // mouseOver
            ButtonEventId evt
                (EventId::Event_MouseOver, mouseIndex, ToAvmInteractiveObj(ActiveEntity)->IncrementMouseOverCnt());
            evt.KeysState = keyMods;
            ActiveEntity->OnMouseEvent(evt);
            // cleanup
            mMouseState[mouseIndex].LastMouseOverObj = NULL; //ActiveEntity;
        }

        ms.SetMouseInsideEntityLast(true);
    }
    else if (!TopmostEntity)
    {
        mMouseState[mouseIndex].RolloverStack.Clear();
    }

    // mouse button press
    for (UInt8 buttonIdx = 0; buttonIdx < checkCount; ++buttonIdx)   // 16 is max number of mouse buttons supported
    {
        unsigned buttonMask = 1 << buttonIdx;
        // Handle button state change
        if (((changeMask >> buttonIdx) & 0x1) != 0)
        {
            // Button was up, now down
            if ((ms.GetPrevButtonsState() & buttonMask) == 0)
            {
                // onPress
                ButtonEventId evt(EventId::Event_MouseDown, mouseIndex);
                evt.KeysState = keyMods;
                InteractiveObject* activeEntitySafe = 
                    (ActiveEntity) ? ActiveEntity.GetPtr() : (InteractiveObject*)pStage.GetPtr();
                evt.ButtonId = buttonIdx;
                activeEntitySafe->OnMouseEvent(evt);
                // 10/19/2010, NFM: Store this entity so we can check against it for a Click event.
                ms.SetMouseButtonDownEntity(buttonIdx, activeEntitySafe);

                ms.SetMouseInsideEntityLast(true);
            }
        }
    }

    // } NFM: From old if (!MouseLeftButton || TrackAsMenu) statement.

    // Write The (possibly modified) Ptr copies back
    // into the state struct.
    ms.SetActiveEntity(ActiveEntity);
    ms.ResetMouseMovedState();
}

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
void MovieRoot::GenerateTouchEvents(unsigned mouseStateIndex)
{
    GFx::MouseState& ms = *pMovieImpl->GetMouseState(mouseStateIndex);
    SF_ASSERT(&ms);
    Ptr<InteractiveObject>  ActiveEntity  = ms.GetActiveEntity();
    Ptr<InteractiveObject>  TopmostEntity = ms.GetTopmostEntity();

    MovieImpl::DragState    st;
    pMovieImpl->GetDragState(mouseStateIndex, &st);
    if (st.MouseIndex == mouseStateIndex)
    {
        // Check if we are currently dragging a character. If yes, substitute the actual TopmostEntity
        // by the one being dragged to avoid unwanted over/out mouse events.
        // See test_mouseOver_out_while_dragging.swf
        if (st.pCharacter)
        {
            // See test_mouseOver_out_while_dragging_mouseDisabled.swf
            if (!st.pCharacter->IsMouseDisabledFlagSet())
            {
                if (!st.pTopmostEntity)
                    pMovieImpl->SetDragStateTopmostEntity(mouseStateIndex, TopmostEntity);
                else
                    TopmostEntity = st.pTopmostEntity;
            }
        }
    }

    if (ActiveEntity && ActiveEntity->IsUnloaded())
        ActiveEntity = NULL;
    if (TopmostEntity && TopmostEntity->IsUnloaded())
        TopmostEntity = NULL;

    Ptr<InteractiveObject>  PrevActiveEntity = ActiveEntity;

    // If topmostEntity is NULL then mouse cursor is above the stage. Flash generates
    // mouse events for stage as a target is this case. 
    InteractiveObject*      
        topmostEntitySafe = (TopmostEntity) ? TopmostEntity.GetPtr() : (InteractiveObject*)pStage.GetPtr();

    if (topmostEntitySafe)
    {
        AvmInteractiveObj* avmTopMost = ToAvmInteractiveObj(topmostEntitySafe);
        if (avmTopMost->GetAS3Obj())
        {
            // notify listeners
            TouchEventId eid(EventId::Event_Invalid, mouseStateIndex, ms.GetTouchID(), 
                ms.GetPressure(), ms.GetContactSize(), ms.IsPrimaryPoint());
            if (ms.GetButtonsState() && !ms.GetPrevButtonsState())
            {
                eid.Id = EventId::Event_TouchBegin;
                if (eid.PrimaryPoint) // save info about primary point
                {
                    PrimaryTouchPointPos.Position = ms.GetLastPosition();
                    PrimaryTouchPointPos.OverObj  = topmostEntitySafe;
                    PrimaryTouchPointPos.ClickTime= (UInt32)(Timer::GetTicks() / 1000);
                }
            }
            else if (!ms.GetButtonsState() && ms.GetPrevButtonsState())
            {
                eid.Id = EventId::Event_TouchEnd;
                if (eid.PrimaryPoint) // cleanup primary point info
                {
                    PrimaryTouchPointPos.OverObj  = NULL;
                    PrimaryTouchPointPos.ClickTime= 0;
                }
            }
            else if (ms.IsMouseMoved())
            {
                eid.Id = EventId::Event_TouchMove;
                if (eid.PrimaryPoint) // update primary point info
                {
                    PrimaryTouchPointPos.Position = ms.GetLastPosition();
                    PrimaryTouchPointPos.OverObj  = topmostEntitySafe;
                }
            }

            // Generate touch events, if in touch mode
            MovieImpl::MultitouchInputMode mtMode = GetMovieImpl()->GetMultitouchInputMode();
            if (mtMode & MovieImpl::MTI_TouchPoint)
            {
                // generate begin
                if (eid.Id == EventId::Event_TouchBegin)
                {
                    if (avmTopMost->GetAS3Obj() != NULL)
                        avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);
                    ActiveEntity = NULL; // force generate touchOver/RollOver at the beginTouch
                }
                else if (eid.Id == EventId::Event_TouchEnd)
                {
                    TopmostEntity = NULL; // force generate touchOut/RollOut at the endTouch
                }

                // Touch_Out/Touch_Roll_Out events
                if (TopmostEntity != ActiveEntity)
                {
                    // rollOut
                    UPInt rosSz = mMouseState[mouseStateIndex].RolloverStack.GetSize();
                    SPInt i;
                    for (i = (SPInt)rosSz - 1; i >= 0; --i)
                    {
                        // check the stack of already "rollovered" characters.
                        // If the new character is not the child of the one in the stack - 
                        // fire rollOut and remove from stack. Stop, when the element char
                        // actually is the parent of ActiveEntity.
                        if (!TopmostEntity || (mMouseState[mouseStateIndex].RolloverStack[i] != TopmostEntity &&
                            !mMouseState[mouseStateIndex].RolloverStack[i]->IsAncestor(TopmostEntity)))
                        {
                            mMouseState[mouseStateIndex].RolloverStack[i]->DecrementRollOverCnt();
                            TouchEventId evt(EventId::Event_TouchRollOut, mouseStateIndex, ms.GetTouchID(), 
                                ms.GetPressure(), ms.GetContactSize(), ms.IsPrimaryPoint());
                            //evt.KeysState = keyMods;
                            //mMouseState[mouseStateIndex].RolloverStack[i]->OnMouseEvent(evt);              
                            InteractiveObject* target = mMouseState[mouseStateIndex].RolloverStack[i];
                            if (ToAvmInteractiveObj(target)->GetAS3IntObj())
                                ToAvmInteractiveObj(target)->GetAS3IntObj()->Dispatch(evt, target);
                        }
                        else
                            break;
                    }
                    mMouseState[mouseStateIndex].RolloverStack.PopBack(rosSz - i - 1);

                    if (ActiveEntity)
                    {
                        // mouseOut
                        // check if the new entity is the child
                        mMouseState[mouseStateIndex].LastMouseOverObj = ActiveEntity;
                        ToAvmInteractiveObj(ActiveEntity)->DecrementMouseOverCnt();

                        TouchEventId evt(EventId::Event_TouchOut, mouseStateIndex, ms.GetTouchID(), 
                            ms.GetPressure(), ms.GetContactSize(), ms.IsPrimaryPoint());
                        //evt.KeysState = keyMods;
                        if (ToAvmInteractiveObj(ActiveEntity)->GetAS3IntObj())
                            ToAvmInteractiveObj(ActiveEntity)->GetAS3IntObj()->Dispatch(evt, ActiveEntity);

                        // clean up
                        mMouseState[mouseStateIndex].LastMouseOverObj = NULL;
                    }

                    ActiveEntity = TopmostEntity;

                    if (ActiveEntity)
                    {
                        // rollOver
                        bool fireRollover = true;
                        UPInt rosSz = mMouseState[mouseStateIndex].RolloverStack.GetSize();
                        if (rosSz)
                        {
                            for (SPInt i = (SPInt)rosSz - 1; i >= 0; --i)
                            {
                                if (mMouseState[mouseStateIndex].RolloverStack[i] == ActiveEntity)
                                {
                                    fireRollover = false;
                                    break;
                                }
                            }
                        }
                        if (fireRollover)
                        {
                            // Seems to me rollover/rollout are fired for each parent up to the "root" (not for stage)
                            // This happens anyway on capture and bubble stage, but Flash fires rollover/out
                            // for each parent separately (having each parent as a "target").
                            Ptr<InteractiveObject> cur = ActiveEntity;
                            InteractiveObject* stage = GetStage();
                            UPInt idx = mMouseState[mouseStateIndex].RolloverStack.GetSize();
                            for (; cur && cur != stage; cur = cur->GetParent())
                            {
                                if (idx != 0)
                                {
                                    // check if we already have the the char in the stack; 
                                    // if so, stop here.
                                    if (mMouseState[mouseStateIndex].RolloverStack[idx - 1] == cur)
                                        break;
                                }
                                cur->IncrementRollOverCnt();
                                TouchEventId evt(EventId::Event_TouchRollOver, mouseStateIndex, ms.GetTouchID(), 
                                    ms.GetPressure(), ms.GetContactSize(), ms.IsPrimaryPoint());
                                //evt.KeysState = keyMods;
                                if (ToAvmInteractiveObj(cur)->GetAS3IntObj())
                                    ToAvmInteractiveObj(cur)->GetAS3IntObj()->Dispatch(evt, cur);
                                mMouseState[mouseStateIndex].RolloverStack.InsertAt(idx, cur);
                            }
                        }

                        // mouseOver
                        ToAvmInteractiveObj(ActiveEntity)->IncrementMouseOverCnt();
                        TouchEventId evt(EventId::Event_TouchOver, mouseStateIndex, ms.GetTouchID(), 
                            ms.GetPressure(), ms.GetContactSize(), ms.IsPrimaryPoint());
                        //evt.KeysState = keyMods;

                        if (ToAvmInteractiveObj(ActiveEntity)->GetAS3IntObj())
                            ToAvmInteractiveObj(ActiveEntity)->GetAS3IntObj()->Dispatch(evt, ActiveEntity);
                        // cleanup
                        mMouseState[mouseStateIndex].LastMouseOverObj = NULL; //ActiveEntity;
                    }
                }

                if (eid.Id == EventId::Event_TouchMove)
                {
                    topmostEntitySafe->DoMouseDrag(mouseStateIndex);
                    if (avmTopMost->GetAS3Obj() != NULL)
                        avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);
                }
                // generate touchTap as well, if touchEnd has been received
                else if (eid.Id == EventId::Event_TouchEnd)
                {
                    if (avmTopMost->GetAS3Obj() != NULL)
                        avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);

                    eid.Id = EventId::Event_TouchTap;
                    if (avmTopMost->GetAS3Obj() != NULL)
                    {
                        avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);
                    }
                }
            }

			/*
            // check if we can generate pressAndTap and twoFingerTap gesture events
            if (mtMode & MovieImpl::MTI_Gesture)
            {
                //printf("pp? %d  ppobj: %p\n", eid.PrimaryPoint, PrimaryTouchPointPos.OverObj.GetPtr());
                if (!eid.PrimaryPoint && PrimaryTouchPointPos.OverObj)
                {
                    //printf("ppObj: %p  topmost: %p\n", PrimaryTouchPointPos.OverObj.GetPtr(), topmostEntitySafe.GetPtr());

                    UInt32 tm = (UInt32)(Timer::GetTicks() / 1000);
                    const PointF& pos = PrimaryTouchPointPos.Position;
                    if (PrimaryTouchPointPos.OverObj == topmostEntitySafe && 
                        tm <= PrimaryTouchPointPos.ClickTime + GFX_DOUBLE_CLICK_TIME_MS) 
                    {
                        // twoFingerTap
                        GestureEventId eid(EventId::Event_GestureAll, GestureEventId::Gesture_TwoFingerTap, pos.x, pos.y);
                        if (avmTopMost->GetAS3Obj() != NULL)
                        {
                            avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);
                        }
                    }
                    else
                    {
                        // pressAndTap
                        PointF tapPos;
                        GestureEventId eid(EventId::Event_GestureAll, GestureEventId::Gesture_PressAndTap, pos.x, pos.y);
                        if (PrimaryTouchPointPos.OverObj == topmostEntitySafe)
                        {
                            tapPos = ms.GetLastPosition();
                            eid.TapX = tapPos.x;
                            eid.TapY = tapPos.y;
                            eid.TapXYUse = true;
                        }
                        if (avmTopMost->GetAS3Obj() != NULL)
                        {
                            avmTopMost->GetAS3Obj()->Dispatch(eid, topmostEntitySafe);
                        }
                    }
                }
            }
			*/
			
        }
    }
    ms.SetActiveEntity(ActiveEntity);
    ms.ResetMouseMovedState();
}

void MovieRoot::GenerateGestureEvents(InteractiveObject* topmost, 
                                      unsigned phase,
                                      const PointF& pos, 
                                      const PointF& offset,
                                      const PointF& scale,
                                      float rotation,
                                      unsigned gestureMask)
{
    if (topmost)
    {
        AvmInteractiveObj* avmTopMost = ToAvmInteractiveObj(topmost);
        if (avmTopMost->GetAS3Obj())
        {
            const static GestureEventId::GestureType evts[] = { 
                GestureEventId::Gesture_Pan,            GestureEventId::Gesture_Zoom, 
                GestureEventId::Gesture_Rotate,         GestureEventId::Gesture_Swipe, 
                GestureEventId::Gesture_PressAndTap,    GestureEventId::Gesture_TwoFingerTap
            };
            UInt32 id = EventId::Event_Invalid; 
            if (phase == InputEventsQueueEntry::Phase_Begin)
                id = EventId::Event_GestureBegin;
            else if (phase == InputEventsQueueEntry::Phase_End)
                id = EventId::Event_GestureEnd;
            else if (phase == InputEventsQueueEntry::Phase_Update)
                id = EventId::Event_GestureUpdate;
            else if (phase == InputEventsQueueEntry::Phase_All)
                id = EventId::Event_GestureAll;

            if (id != EventId::Event_Invalid)
            {
                for (unsigned m = 1, i = 0; m <= InputEventsQueueEntry::GestureBit_Last; m <<= 1, ++i)
                {
                    if ((gestureMask & m) != 0)
                    {
                        SF_ASSERT(i < sizeof(evts)/sizeof(evts[0]));
                        GestureEventId eid(id, evts[i], pos.x, pos.y, 
                            offset.x, offset.y, scale.x, scale.y, rotation);
                        avmTopMost->GetAS3Obj()->Dispatch(eid, topmost);
                    }
                }
            }
        }
    }
}
#endif

SPtr<Instances::fl_events::Event> MovieRoot::CreateEventObject(const ASString& type, bool bubbles, bool cancelable)
{
    CheckAvm();
    Classes::fl_events::EventDispatcher& evtDispClass = 
        static_cast<Classes::fl_events::EventDispatcher&>(*GetAVM()->EventDispatcherClass.GetPtr());
    return evtDispClass.CreateEventObject(type, bubbles, cancelable);
}

void    MovieRoot::DoActions()
{
    SF_AMP_SCOPE_TIMER_ID(pMovieImpl->AdvanceStats, "MovieRoot::DoActions", Amp_Native_Function_Id_DoActions);
    ExecuteCtors();
    ExecuteActionQueues();
#ifdef SF_ENABLE_SOCKETS
    CheckSocketMessages(); // Asynchronous net sockets require an update from the advance thread. 
#endif
    if (pAVM->IsException())
        pAVM->IgnoreException();
}

void    MovieRoot::ExecuteCtors()
{
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::ExecuteCtors", Amp_Profile_Level_Medium);
    // execute any scheduled AS stack frames (most likely AS ctors)
    if (ASFramesToExecute > 0)
    {
        pAVM->ExecuteCode(ASFramesToExecute);  
        if (pAVM->IsException())
            pAVM->IgnoreException();

        ASFramesToExecute = 0;
    }
}

void    MovieRoot::ExecuteActionQueue(ActionLevel lvl)
{
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::ExecuteActionQueue", Amp_Profile_Level_Medium);
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueIterator iter(lvl, &ActionQueue);
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void    MovieRoot::RequeueActionQueue(ActionLevel srclvl, ActionLevel dstlvl)
{
    if (ActionQueue.HasEntries(srclvl))
    {
        MovieRoot::ActionQueueIterator iter(srclvl, &ActionQueue);
        const ActionEntry* paction;
        while((paction = iter.getNext()) != NULL)
        {
            *InsertEmptyAction(dstlvl) = *paction;
        }
    }
}

void    MovieRoot::RemoveActionQueueEntriesFor(ActionLevel lvl, DisplayObject* dobj)
{
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    ActionEntry* cur = ActionQueue.Entries[lvl].pActionRoot;
    while (cur)
    {
        if (cur->pCharacter == dobj)
            cur->pCharacter = NULL;
        cur = cur->pNextEntry;
    }
}

#ifdef SF_ENABLE_SOCKETS
void MovieRoot::CheckSocketMessages()
{
    Array<Ptr<SocketThreadMgr> > socketsInUse;
    for (UPInt i = 0; i < Sockets.GetSize(); ++i)
    {
        // If the only reference is here, socket is not used
        if (Sockets[i]->GetRefCount() > 1)
        {
            socketsInUse.PushBack(Sockets[i]);
        }
    }
    // Clear sockets that are no longer in use
    Sockets.Clear();
    for (UPInt i = 0; i < socketsInUse.GetSize(); ++i)
    {
        socketsInUse[i]->CheckEvents();
        Sockets.PushBack(socketsInUse[i]);
    }
}

SocketThreadMgr* MovieRoot::AddSocket(bool initSockLib, Instances::fl_net::Socket* sock)
{
    Ptr<SocketThreadMgr> newSocket = *SF_HEAP_AUTO_NEW(this) SocketThreadMgr(initSockLib, SocketFactory, sock);
    Sockets.PushBack(newSocket);
    return newSocket;
}

void MovieRoot::SetSocketImplFactory(SocketImplFactory* factory)
{
    SocketFactory = factory;
}

#endif  // SF_ENABLE_SOCKETS


void    MovieRoot::ExecuteActionQueue(ActionLevel lvl, void* rootEntry)
{
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::ExecuteActionQueue", Amp_Profile_Level_Medium);
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueIterator iter(lvl, &ActionQueue, 
        static_cast<ActionEntry*>(rootEntry));
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void    MovieRoot::ExecuteActionQueues()
{
    //ForceCollect();
    for (unsigned i = 0; i < AL_Count_; ++i)
    {   
        ExecuteActionQueue(ActionLevel(i));
    }
    //ForceCollect();
}

void    MovieRoot::QueueFrameActions()
{
    pStage->QueueFrameActions();
}

void MovieRoot::ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType) 
{
    SF_ASSERT(pMovieImpl);

    if (GetStage() && GetAVM() && GetAVM()->ExtensionsEnabled)
    {
        if (GetStage()->HasMouseCursorEvent())
        {
            Classes::fl_events::EventDispatcher& evtDispClass = 
                static_cast<Classes::fl_events::EventDispatcher&>(*GetAVM()->EventDispatcherClass.GetPtr());
            
            ASString cursorStr(GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_));
            GetMouseCursorTypeString(cursorStr, newCursorType);

            SPtr<Instances::fl_gfx::MouseCursorEvent> mce = evtDispClass.CreateMouseCursorEventObject
                (cursorStr, mouseIdx);
            Instances::fl_display::DisplayObject* as3obj = ToAvmInteractiveObj(GetStage())->GetAS3Obj();
            if (as3obj && !as3obj->Dispatch(mce, GetStage()))
            {
                // prevented - return
                return;
            }
        }
    }

    if (pMovieImpl->pUserEventHandler)
    {
        pMovieImpl->pUserEventHandler->HandleEvent(pMovieImpl, 
            MouseCursorEvent((MouseCursorEvent::CursorShapeType)newCursorType, mouseIdx));
    }
}

bool MovieRoot::NotifyOnFocusChange(InteractiveObject* curFocused, 
                                    InteractiveObject* toBeFocused, 
                                    unsigned controllerIdx, 
                                    FocusMovedType fmt,
                                    ProcessFocusKeyInfo* pfocusKeyInfo)
{
    // call OnFocusChange for stage if there is no currently focused object
    AvmInteractiveObj* obj = (curFocused) ? ToAvmInteractiveObj(curFocused) : ToAvmInteractiveObj(pStage);
    return obj->OnFocusChange(toBeFocused, controllerIdx, fmt, pfocusKeyInfo);
}

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
void MovieRoot::NotifyGamePadAnalogEvent(const EventId* id)
{
    GetStage()->OnEvent(*id);
}
#endif

void MovieRoot::NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt) 
{
    // do we need to queue it like in AS2? (!AB)
    GetMovieImpl()->TransferFocus(ch, controllerIdx, fmt);
}

void MovieRoot::NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, unsigned controllerIdx) 
{ 
    SF_UNUSED3(curFocused, pNewFocus, controllerIdx); 
}

Sprite* MovieRoot::CreateMovieClip(TimelineDef* pdef, 
                                   MovieDefImpl* pdefImpl,
                                   InteractiveObject* parent,
                                   ResourceId id, 
                                   bool loadedSeparately)
{
    CharacterCreateInfo ccinfo(pdef, pdefImpl);
    DisplayObjectBase* pdo = GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), ccinfo, parent, id, CharacterDef::Sprite);
    SF_ASSERT(pdo->IsInteractiveObject() && pdo->IsSprite());
    Sprite* pspr = pdo->CharToSprite_Unsafe();
    pspr->SetLoadedSeparately(loadedSeparately);
    pspr->SetMovieClipFlag();
    return pspr;
}

Sprite* MovieRoot::CreateSprite(TimelineDef* pdef, 
                                MovieDefImpl* pdefImpl,
                                InteractiveObject* parent,
                                ResourceId id, 
                                bool loadedSeparately)
{
    CharacterCreateInfo ccinfo(pdef, pdefImpl);
    DisplayObjectBase* pdo = GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), ccinfo, parent, id, CharacterDef::EmptySprite);
    SF_ASSERT(pdo->IsInteractiveObject() && pdo->IsSprite());
    Sprite* pspr = pdo->CharToSprite_Unsafe();
    pspr->SetLoadedSeparately(loadedSeparately);
    return pspr;
}

AvmStage* MovieRoot::CreateStage(MovieDefImpl* pdefImpl)
{
    SF_ASSERT(!pStage);
    UByte* pm = (UByte*) SF_HEAP_ALLOC(GetMovieHeap(), 
        ((sizeof(Stage) + 3) & ~3) + sizeof(AS3::AvmStage),
        StatMV_ActionScript_Mem);
    pStage = *new (pm) Stage(pdefImpl, this, NULL, ResourceId(ResourceId::InvalidId));
    AvmStage* avmStage = new (pm + ((sizeof(Stage) + 3) & ~3)) AvmStage(pStage);
    avmStage->SetAppDomain(GetAVM()->GetFrameAppDomain());
    InsertEmptyAction(AL_Highest)->SetAction(pStage, EventId::Event_Load);
    return avmStage;
}

void MovieRoot::AdvanceFrame(bool nextFrame)
{
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::AdvanceFrame", Amp_Profile_Level_Low);

    if (!(MainLoaderInfoEventsState & MLISE_Completed))
    {
        MovieDefImpl* md = pMovieImpl->GetMainContainer()->GetResourceMovieDef();
        DisplayObjContainer* rt =  GetRootMovie(NULL);
        SPtr<Instances::fl_display::DisplayObject> as3obj = ToAvmDisplayObjContainer(rt)->GetAS3Obj();

        if (as3obj && as3obj->HasLoaderInfo())
        {
            if (!(MainLoaderInfoEventsState & MLISE_InitExecuted))
            {
                // init
                as3obj->pLoaderInfo->ExecuteInitEvent(rt);
                MainLoaderInfoEventsState |= MLISE_InitExecuted;
            }
            bool loadingFinished = (md->GetLoadingFrame() >= md->GetFrameCount());
            
            // progress
            // execute progress only each 'full frame' to avoid performance hit
            if (nextFrame || loadingFinished)
            {
                as3obj->pLoaderInfo->ExecuteProgressEvent(md->GetBytesLoaded(), 
                                                          md->GetFileBytes());
            }

            if (loadingFinished)
            {
                // finished, shoot COMPLETE event
                as3obj->pLoaderInfo->ExecuteCompleteEvent();
                MainLoaderInfoEventsState |= MLISE_Completed;
            }
        }
        else
            MainLoaderInfoEventsState |= MLISE_Completed;
    }

    // Looks like RENDER event should be fired even for fractional advance, if stage
    // is invalidated. (AB)
    if (/*nextFrame &&*/ IsStageInvalidated())
    {
        // fire RENDER events
        QueueEventChain(EventId::Event_Render);
        DoActions(); // execute queued up events right here
        ValidateStage();
    }

    if (nextFrame)
    {
        // if collection was scheduled by System.gc - collect
        if (MemContext->ASGC)
        {
            MemContext->ASGC->CollectIfScheduled(pMovieImpl->AdvanceStats);
#ifdef GFX_AS_ENABLE_GC
            MemContext->ASGC->AdvanceFrame
                (&NumAdvancesSinceCollection, &LastCollectionFrame, pMovieImpl->AdvanceStats);
#endif // SF_NO_GC
        }
    }
}

void MovieRoot::ClearDisplayList()
{
    // It is important to destroy the sprite before the global context,
    // so that is is not used from OnEvent(unload) in sprite destructor
    // NOTE: We clear the list here first because users can store pointers in _global,
    // which would cause pMovie assignment to not release it early (avoid "aeon.swf" crash).

    GetMainContainer()->ClearDisplayList();
    GetMainContainer()->OnEventUnload();
    GetMainContainer()->ForceShutdown();

    /*for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
        static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr())->ClearDisplayList();
    for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
    {
        Sprite* pspr = static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr());
        pspr->OnEventUnload();
        pspr->ForceShutdown();
    }*/
    // Release all refs.
    pMovieImpl->MovieLevels.Clear();
}

MovieDefRootNode* MovieRoot::CreateMovieDefRootNode(MemoryHeap* pheap, MovieDefImpl *pdefImpl, bool importFlag)
{
    return SF_HEAP_NEW(pheap) MovieDefRootNode(pdefImpl, importFlag);
}

// forces garbage collection (if GC is enabled)
void MovieRoot::ForceCollect(unsigned gcFlags)
{
    unsigned flags;
    switch (gcFlags)
    {
    case Movie::GCF_Quick:  flags = ASRefCountCollector::Collect_Quick; break;
    case Movie::GCF_Medium: flags = ASRefCountCollector::Collect_Medium; break;
    case Movie::GCF_Full:   flags = ASRefCountCollector::Collect_Full; break;
    default: flags = 0;
    }
    if (GetMovieImpl()->IsShutdowning())
        flags |= ASRefCountCollector::Collect_Shutdown;
    MemContext->ASGC->ForceCollect(pMovieImpl->AdvanceStats, flags);
}

// forces emergency garbage collection (if GC is enabled). This method is called
// when heap is overflown. 
void MovieRoot::ForceEmergencyCollect()
{
    MemContext->ASGC->ForceEmergencyCollect(pMovieImpl->AdvanceStats);
}

void MovieRoot::SuspendGC(bool suspend)
{
    MemContext->ASGC->SuspendGC(suspend);
}

void MovieRoot::ScheduleGC(unsigned gcFlags)
{
    unsigned flags;
    switch (gcFlags)
    {
    case Movie::GCF_Quick:  flags = ASRefCountCollector::Collect_Quick; break;
    case Movie::GCF_Medium: flags = ASRefCountCollector::Collect_Medium; break;
    case Movie::GCF_Full:   flags = ASRefCountCollector::Collect_Full; break;
    default: flags = 0;
    }
    MemContext->ASGC->ScheduleCollect(flags);
}

void MovieRoot::OnKeyDown(InteractiveObject *pmovie, const EventId& evt, int keyMask) 
{
    SF_UNUSED1(pmovie);

    Ptr<InteractiveObject> focused = pMovieImpl->GetFocusedCharacter(evt.KeyboardIndex);
    unsigned focusGroup     = GetMovieImpl()->GetFocusGroupIndex(evt.KeyboardIndex);
    if (((1 << focusGroup) & keyMask) == 0)
    {
        // TextField's keyDown will be processed in AvmTextField::OnKeyEvent
        // In AS3 key event listener is called BEFORE the real action takes place (before
        // caret position is changed, for example). AvmTextField::OnKeyEvent is called BEFORE
        // the action and this function is called AFTER, so just ignore the call for textfields.
        Instances::fl_display::DisplayObject* as3dobj = (focused) ? 
            ToAvmInteractiveObj(focused)->GetAS3Obj() : ToAvmDisplayObjContainer(pStage)->GetAS3Obj();
        if (as3dobj)
            as3dobj->Dispatch(evt, focused);
    }
}       
void MovieRoot::OnKeyUp(InteractiveObject *pmovie, const EventId& evt, int keyMask) 
{
    SF_UNUSED1(pmovie);

    Ptr<InteractiveObject> focused = pMovieImpl->GetFocusedCharacter(evt.KeyboardIndex);
    unsigned focusGroup     = GetMovieImpl()->GetFocusGroupIndex(evt.KeyboardIndex);
    if (((1 << focusGroup) & keyMask) == 0)
    {
        // TextField's keyUp will be processed in AvmTextField::OnKeyEvent
        // In AS3 key event listener is called BEFORE the real action takes place (before
        // caret position is changed, for example). AvmTextField::OnKeyEvent is called BEFORE
        // the action and this function is called AFTER, so just ignore the call for textfields.
        Instances::fl_display::DisplayObject* as3dobj = (focused) ? 
            ToAvmInteractiveObj(focused)->GetAS3Obj() : ToAvmDisplayObjContainer(pStage)->GetAS3Obj();
        if (as3dobj)
            as3dobj->Dispatch(evt, focused);
    }
}
void MovieRoot::Update(const EventId& evt) 
{
    SF_UNUSED1(evt);
}

const ASString& GetEventString(const StringManager& mgr, const EventId& evt)
{
    switch(evt.Id)
    {
    case EventId::Event_EnterFrame:
        return mgr.GetBuiltin(AS3Builtin_enterFrame);
    case EventId::Event_Press:
        return mgr.GetBuiltin(AS3Builtin_mouseDown);
    case EventId::Event_Release:
        return mgr.GetBuiltin(AS3Builtin_mouseUp);
//     case EventId::Event_MouseDown:
//         return mgr.GetBuiltin(AS3Builtin_mouseDown);
//     case EventId::Event_MouseUp:
//         return mgr.GetBuiltin(AS3Builtin_mouseUp);
    }
    return mgr.GetBuiltin(AS3Builtin_empty_);
}

void MovieRoot::OnMovieFocus(bool set)
{
    if (set)
        mEventChains.Dispatch(EventId::Event_Activate);
    else
        mEventChains.Dispatch(EventId::Event_Deactivate);
    //ExecuteActivateDeactivateQueue((set) ? EventId::Event_Activate : EventId::Event_Deactivate);
}

// called BEFORE processing complete (non-partial) AdvanceFrame.
void MovieRoot::OnNextFrame()
{
    // Run enterframes
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    mEventChains.QueueEvents(EventId::Event_EnterFrame);
}


void MovieRoot::SetExternalInterfaceRetVal(const GFx::Value& retVal)
{
	GFxValue2ASValue(retVal, &ExternalIntfRetVal);
}

// Extracts the last token from a path and returns both parts (delimiter is '.')
// Returns false if fullPath is the name
bool MovieRoot::ExtractPathAndName(const char* fullPath, ASString* ppath, ASString* pname)
{
    // Search for colon.
    int         colonIndex = -1;
    const char* cstr = fullPath;
    const char* p;

    if ((p = SFstrrchr(cstr, '.')) != 0)
        colonIndex = int(p - cstr);
    else
        return false;

    // Make the subparts.
    *pname = GetStringManager()->CreateString(fullPath + colonIndex + 1);
    *ppath = GetStringManager()->CreateString(fullPath, colonIndex);

    return true;
}


void MovieRoot::AddStickyVariable(const ASString& path, const ASString& name, 
                                  const Value &val, Movie::SetVarType setType)
{
    if (name.IsEmpty())
        return;

    StickyVarNode* pnode;
    if ((pnode = SF_HEAP_NEW(GetMovieHeap()) StickyVarNode(name, val, (setType == Movie::SV_Permanent))) != 0)
        pMovieImpl->AddStickyVariableNode(path, pnode);
}


void    MovieRoot::ResolveStickyVariables(InteractiveObject *pch)
{
    SF_ASSERT(pch);

    const ASString &path  = pch->GetCharacterHandle()->GetNamePath();
    MovieImpl::StickyVarNode* _pnode = 0;
    StickyVarNode*   p, *pnext, *pnode;
    //AvmCharacter* pcharacter = ToAvmCharacter(pch);
    Object* pobj = ToAvmInteractiveObj(pch)->GetAS3Obj();

    if (pobj != NULL && pMovieImpl->StickyVariables.Get(path, &_pnode))
    {       
        pnode = static_cast<StickyVarNode*>(_pnode);
        // If found, add variables.     
        // We also remove sticky nodes while keeping permanent ones.
        StickyVarNode*  ppermanent = 0;
        StickyVarNode*  ppermanentTail = 0;

        p = pnode;

        while (p)
        {
            //pcharacter->SetMember(pcharacter->GetASEnvironment(), p->Name, p->mValue);
            Multiname propname(pAVM->GetPublicNamespace(), Value(p->Name));
            pobj->SetProperty(propname, p->mValue);
            pnext = static_cast<StickyVarNode*>(p->pNext);

            if (p->Permanent)
            {
                // If node is permanent, create a permanent-only linked list.
                if (!ppermanent)
                {
                    ppermanent = p;
                }
                else
                {
                    ppermanentTail->pNext = p;
                }

                ppermanentTail = p;
                p->pNext = 0;
            }
            else
            {
                // If not permanent, delete this node.
                delete p;
            }

            p = pnext;
        }


        if (ppermanent)
        {
            // If permanent list was formed, keep it.
            if (ppermanent != pnode)
                pMovieImpl->StickyVariables.Set(path, ppermanent);
        }
        else
        {
            // Otherwise delete hash key.
            pMovieImpl->StickyVariables.Remove(path);
        }
    }
}


// -------------------------------------------------------------------------------------------

// Extracts tokens that are '.' delimited from an AS2-like path. Path is expected to be null
// terminated.
class PathTokenizer
{
    mutable StringDataPtr   Path;
    mutable StringDataPtr   Token;
public:
    PathTokenizer(const char* ppath) : Path(ppath) {}
    bool                    HasToken() const { return Path.GetSize() > 0; }
    const StringDataPtr&    NextToken() const
    {
        SF_ASSERT(HasToken());

        Token = Path.GetNextToken('.');
        Path.TrimLeft( Token.GetSize() );
        if (Path.ToCStr()[0] == '.') Path.TrimLeft(1);

        return Token;
    }    
};


bool MovieRoot::GetASVariableAtPath(Value* pval, const char* ppathToVar) const
{
    if (!ppathToVar) return false;
    const_cast<MovieRoot*>(this)->CheckAvm(); // revise const_cast
    SF_ASSERT(!pAVM->IsException());
    pval->SetUndefined();
    Value resolvedVal;
    // Set the stage to be the current root
    AvmDisplayObjContainer*  pstage = GetAvmMainMovie();
    SF_ASSERT(pstage);
    resolvedVal = Value(pstage->GetAS3Obj());

    // Check in global objects first
    Multiname prop(*pAVM, ppathToVar);

    PropRef globalProp;
    VMAppDomain& ad = pstage->GetAppDomain();
    const ClassTraits::Traits* ctr = pAVM->GetRegisteredClassTraits(prop, ad);
    FindGOProperty(globalProp, *pAVM, pAVM->GetGlobalObjects(), prop, ctr);

    if (globalProp.IsFound())
    {
        Value v;

        if (globalProp.GetSlotValueUnsafe(*pAVM, v))
        {
            pval->Swap(v);
            return true;
        }

        return false;
    }

    // Check for known prefixes and disregard them for further path resolving
    // Certain cases target root1, else stage is used as root
    bool isroot1 = false;
    const char* ppath = ppathToVar;
    /*
    if      (!SFstrncmp(ppath, "_root.", 6))    { ppath += 6; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_root\0", 6))   { ppath += 5; isroot1 = true; }
    else if (!SFstrncmp(ppath, "root.", 5))     { ppath += 5; isroot1 = true; }
    else if (!SFstrncmp(ppath, "root\0", 5))    { ppath += 4; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_level0.", 8))  { ppath += 8; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_level0\0", 8)) { ppath += 7; isroot1 = true; }
    else if (!SFstrncmp(ppath, "stage.", 6))    { ppath += 6; }
    else if (!SFstrncmp(ppath, "stage\0", 6))   { ppath += 6; }
    */
    unsigned offset = 0;
    bool r1allow = true;
    switch (ppath[0])
    {
    case '_':
        if      (!SFstrncmp(ppath+1, "root", 4)) { offset += 5; }
        else if (!SFstrncmp(ppath+1, "level0", 5)) { offset += 7; }
        break;
    case 'r': if (!SFstrncmp(ppath+1, "oot", 3)) { offset += 4; } break;
    case 's': if (!SFstrncmp(ppath+1, "tage", 4)) { offset += 5; r1allow = false; } break;
    }
    switch (ppath[offset])
    {
    case '.':  { offset++; }
    case '\0': { isroot1 = r1allow && true; ppath += offset; }
    }    

    // Get root1 and set it as the current root if needed
    if (isroot1)
    {
        DisplayObjectBase* proot1 = 
            pstage->GetDisplayList().GetDisplayObjectByName(GetBuiltinsMgr().CreateConstString("root1"));
        if (!proot1 || !proot1->IsDisplayObjContainer())
            return false;
        resolvedVal = 
            Value(ToAvmDisplayObjContainer(proot1->CharToDisplayObjContainer())->GetAS3Obj());
    }

    // Parse and traverse objects starting from display list root
    PathTokenizer pt(ppath);
    while (pt.HasToken())
    {
        const StringDataPtr& token = pt.NextToken();
        // Get property of current root element
        ASString propName = GetBuiltinsMgr().CreateString(token.ToCStr(), token.GetSize());
        Multiname propMulti(pAVM->GetPublicNamespace(), Value(propName));
        Value subVal;
        Object* presolvedObj = resolvedVal.GetObject();
        if (!presolvedObj)
        {
            pval->SetUndefined();
            if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
            return false;
        }
        if (!presolvedObj->GetProperty(propMulti, subVal))
        {
            // If property not found, element may be in display list
            // Check if object is a display object container
            if (AreDisplayObjectContainerTraits(presolvedObj))
            {
                AvmDisplayObjContainer* pcurr = 
                    ToAvmDisplayObjContainer(static_cast<Instances::fl_display::DisplayObjectContainer*>(
                        presolvedObj)->pDispObj->CharToDisplayObjContainer());
                AS3::Instances::fl_display::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
                if (pchild)
                {
                    SF_ASSERT(pchild->pDispObj);
                    subVal = Value(pchild);
                }
                else
                {
                    if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
                    return false;
                }
            }
        }        
        resolvedVal = subVal;

        // If resolvedVal is not an object and it's not the last token, then there is 
        // something wrong with the path. Notify the user.
        if (pt.HasToken() && !resolvedVal.IsObject())
        {
            Log* plog = GetLog();
            if (plog)
            {
                StringBuffer s;
                Format(s, "Token '{0}' in path '{1}' was not resolved to a valid Object! "
                    "This may be caused by a property that is not of Object type, or by "
                    "using reserved words/properties for MovieClip names in the display tree.",
                    token, ppathToVar);
                plog->LogError(s);
            }
            resolvedVal.SetNull();
            break;
        }
    }

    *pval = resolvedVal;
    SF_ASSERT(!pAVM->IsException());
    return true;
}

bool MovieRoot::GetVariable(GFx::Value *pval, const char* ppathToVar) const
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to GetVariable");   
    SF_DEBUG_ASSERT(pval, "Error: NULL pval passed to GetVariable");

    Value resolvedVal;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    pval->SetUndefined();
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        ASValue2GFxValue(resolvedVal, pval);
        return true;
    }
    return false;
}

bool MovieRoot::SetVariable(const char* ppathToVar, const GFx::Value& value, Movie::SetVarType setType /* = Movie::SV_Sticky */)
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to SetVariable");   

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Note: SetVariable cannot be used to get access to the global object(s). An arbitrary
    //       variable/path (not targetted to root/_root/stage) will be processed under the 
    //       'root' scope automatically.

    // Extract path and name
    ASString path = GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_);
    ASString name = GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_);
    if (!ExtractPathAndName(ppathToVar, &path, &name))
    {
        // Path extraction failed; ppathToVar is pointing to a root var
        name = GetStringManager()->CreateString(ppathToVar);
        path = GetStringManager()->CreateString("root");
    }
    // Get path (parent) var
    Value parentVal;
    if (!GetASVariableAtPath(&parentVal, path.ToCStr()))
        return false;
    // Put property name into path var
    SF_ASSERT(parentVal.IsObject());
    Object* pparentObj = parentVal.GetObject();
    Multiname prop(pAVM->GetPublicNamespace(), Value(name));
    Value putval;
    GFxValue2ASValue(value, &putval);
    bool setResult = pparentObj->SetProperty(prop, putval);

    if ( (!setResult && (setType != Movie::SV_Normal)) ||
        (setType == Movie::SV_Permanent) )
    {
        // Store in sticky hash.
        AddStickyVariable(path, name, putval, setType);
    }

    if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
    return setResult;
}

unsigned MovieRoot::GetVariableArraySize(const char* ppathToVar)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resolvedVal;
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        SF_ASSERT(resolvedVal.IsObject());
        Object* pobj = resolvedVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array && pobj->GetTraits().IsInstanceTraits())
        {
            Instances::fl::Array* parr = static_cast<Instances::fl::Array*>(pobj);
            return static_cast<unsigned>(parr->GetSize());
        }
    }
    return 0;
}

bool MovieRoot::GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, void* pdata, unsigned count)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resolvedVal;
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        if (!resolvedVal.IsObject()) return false;
        Object* pobj = resolvedVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array && pobj->GetTraits().IsInstanceTraits())
        {
            MovieImpl::ReturnValueHolder* rvHolder = pMovieImpl->GetRetValHolder();
            rvHolder->ResetPos();
            rvHolder->ResizeStringArray(0);

            Instances::fl::Array* parray = static_cast<Instances::fl::Array*>(pobj);
            unsigned arrSize = static_cast<unsigned>(parray->GetSize());
            switch (type)
            {
            case Movie::SA_Int:
                {
                    int* parr = static_cast<int*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (int)val.AsInt();
                        else
                            parr[i] = 0;
                    }
                }
            case Movie::SA_Float:
                {
                    float* parr = static_cast<float*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (float)val.AsNumber();
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Double:
                {
                    Double* parr = static_cast<Double*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (Double)val.AsNumber();
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Value:
                {
                    GFx::Value* parr = static_cast<GFx::Value*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        GFx::Value* pdestVal = &parr[i];
                        pdestVal->SetUndefined();
                        if (!val.IsUndefined())
                            ASValue2GFxValue(val, pdestVal);
                        else
                            pdestVal->SetUndefined();
                    }
                }
                break;
            case Movie::SA_String:
                {
                    const char** parr = static_cast<const char**>(pdata);
                    unsigned n = Alg::Min(arrSize, count);
                    rvHolder->ResizeStringArray(n);
                    for (unsigned i = 0; i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                        {
                            ASString str = val.AsString();
                            parr[i] = str.ToCStr();
                            rvHolder->StringArray[rvHolder->StringArrayPos++] = str;
                        }
                        else
                            parr[i] = NULL;
                    }
                }
                break;
            case Movie::SA_StringW:
                {
                    const wchar_t** parr = static_cast<const wchar_t**>(pdata);
                    // pre-calculate the size of required buffer
                    unsigned i, bufSize = 0;
                    unsigned n = Alg::Min(arrSize, count);
                    rvHolder->ResizeStringArray(n);
                    for (i = 0; i < arrSize; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                        {
                            ASString str = val.AsString();
                            rvHolder->StringArray[i] = str;
                            bufSize += str.GetLength()+1;
                        }
                    }
                    wchar_t* pwBuffer = (wchar_t*)rvHolder->PreAllocateBuffer(bufSize * sizeof(wchar_t));

                    for (i = 0; i < n; ++i)
                    {
                        const char* psrcStr = rvHolder->StringArray[i].ToCStr();
                        wchar_t* pwdstStr = pwBuffer;
                        UInt32 code;
                        while ((code = UTF8Util::DecodeNextChar(&psrcStr)) != 0)
                        {
                            *pwBuffer++ = (wchar_t)code;
                        }
                        *pwBuffer++ = 0;

                        parr[i] = pwdstStr;
                    }
                    rvHolder->ResizeStringArray(0);
                }
                break;
            }
            return true;
        }
    }
    return false;
}


bool MovieRoot::SetVariableArraySize(const char* ppathToVar, unsigned count, Movie::SetVarType setType)
{
    // TODO: Check VM

    Value retVal;
    if (GetASVariableAtPath(&retVal, ppathToVar) && retVal.IsObject())
    {
        Object* pobj = retVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array && pobj->GetTraits().IsInstanceTraits())
        {
            // Array already exists - just resize
            Instances::fl::Array* parray;
            parray = static_cast<Instances::fl::Array*>(pobj);
            if (count != unsigned(parray->GetSize()))
                parray->Resize(count);
            return true;
        }
    }

    // no array found - allocate new one, resize and set
    Pickable<Instances::fl::Array> parray = pAVM->MakeArray();
    parray->Resize(count);
    
    Value val(parray);
    GFx::Value gfxval;
    ASValue2GFxValue(val, &gfxval);
    return SetVariable(ppathToVar, gfxval, setType);
}


bool MovieRoot::SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, const void* pdata, unsigned count, Movie::SetVarType setType)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    AS3::SPtr<Instances::fl::Array> parray;
    Value retVal;
    if(GetASVariableAtPath(&retVal, ppathToVar) && retVal.IsObject())
    {
        Object* pobj = retVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array && pobj->GetTraits().IsInstanceTraits())
            parray = static_cast<Instances::fl::Array*>(pobj);
    }

    bool found = true;
    if (!parray) 
    {
        parray = pAVM->MakeArray();
        found = false;
    }
    if (count + index > unsigned(parray->GetSize()))
        parray->Resize(count + index);

    switch(type)
    {
    case Movie::SA_Int:
        {
            const SInt32* parr = static_cast<const SInt32*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(parr[i]));
            }
            break;
        }
    case Movie::SA_Float:
        {
            const float* parr = static_cast<const float*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(Value::Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Double:
        {
            const Double* parr = static_cast<const Double*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(Value::Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Value:
        {
            const GFx::Value* parr = static_cast<const GFx::Value*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                Value asVal;
                GFxValue2ASValue(parr[i], &asVal);
                parray->Set(i + index, asVal);
            }
            break;
        }
    case Movie::SA_String:
        {
            const char* const* parr = static_cast<const char* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    case Movie::SA_StringW:
        {
            const wchar_t* const* parr = static_cast<const wchar_t* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    }

    if (!found)
    {
        Value val(parray, PickValue);
        GFx::Value gfxval;
        ASValue2GFxValue(val, &gfxval);
        return SetVariable(ppathToVar, gfxval, setType);
    }
    return true;
}


bool MovieRoot::IsAvailable(const char* ppathToVar) const
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to IsAvailable");   

    Value resolvedVal;
    return GetASVariableAtPath(&resolvedVal, ppathToVar);
}

bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult,
                       const GFx::Value* pargs, unsigned numArgs)
{   
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::Invoke", Amp_Profile_Level_Medium);

    // TODO: Check VM

    SF_DEBUG_ASSERT(pmethodName, "Error: NULL pmethodName passed to Invoke");   

    bool retVal = true;

    // There should not be an exception in the VM
    SF_ASSERT(!pAVM->IsException());

    // Setup invoke parameters
    // ** Create storage
    enum { NumValuesOnStack = 10 };
    void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*))/sizeof(void*))];
    Value* pargArray;
    if (numArgs > NumValuesOnStack)
        pargArray = (Value*)
        SF_HEAP_AUTO_ALLOC_ID(this, sizeof(Value) * numArgs, GFx::StatMV_ActionScript_Mem);
    else
        pargArray = (Value*)argArrayOnStack;
    // ** Convert to AS3::Value
    for (unsigned i = 0, n = numArgs; i < n; ++i)
    {
        const GFx::Value& val = pargs[i];
        Value* pdestVal = Scaleform::Construct<Value>(&pargArray[i]);
        GFxValue2ASValue(val, pdestVal);
    }

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Try to resolve invoke alias
    Value* paliasFunc;
    Value resultVal;

    if (pInvokeAliases && (paliasFunc = ResolveInvokeAlias(pmethodName)) != NULL)
        pAVM->ExecuteUnsafe(*paliasFunc, Value::GetUndefined(), resultVal, numArgs, pargArray);
    else
    {
        Value resolvedVal;
        if (GetASVariableAtPath(&resolvedVal, pmethodName))
            pAVM->ExecuteUnsafe(resolvedVal, Value::GetUndefined(), resultVal, numArgs, pargArray);
        else 
            retVal = false;
    }

    if (retVal)
    {
        retVal = !pAVM->IsException();
        if (pAVM->IsException()) pAVM->OutputAndIgnoreException();

        // Convert result to GFx::Value
        if (retVal && presult)
            ASValue2GFxValue(resultVal, presult);
    }

    // Destruct elements in AS3::Value params array
    for (unsigned i = 0, n = numArgs; i < n; ++i)
        pargArray[i].~Value();
    if (numArgs > NumValuesOnStack)
        SF_FREE(pargArray);

    return retVal;
}


bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult, const char* pargFmt, ...)
{
    // Delegate to InvokeArgs
    va_list args;
    va_start(args, pargFmt);
    bool retVal = InvokeArgs(pmethodName, presult, pargFmt, args);
    va_end(args);
    return retVal;
}



void MovieRoot::ParseValueArguments(Array<Value>& arr, const char* pmethodName, 
                                    const char* pargFmt, va_list args) const
{
    LogState* plog = GetLogState();
    arr.Clear();
    if (pargFmt)
    {
        Value arg;
        const char* p = pargFmt;
        for (;;)
        {
            char    c = *p++;
            if (c == 0)
            {
                // End of args.
                break;
            }
            else if (c == '%')
            {
                c = *p++;
                // Here's an arg.
                if (c == 'd')
                {
                    // Integer.
                    arg.SetSInt32(va_arg(args, int));
                    arr.PushBack(arg);
                }
                else if (c == 'u')
                {
                    // Undefined.
                    arg.SetUndefined();
                    arr.PushBack(arg);
                }
                else if (c == 'n')
                {
                    // Null.
                    arg.SetNull();
                    arr.PushBack(arg);
                }
                else if (c == 'b')
                {
                    // Boolean
                    arg.SetBool(bool(va_arg(args, int) != 0));
                    arr.PushBack(arg);
                }
                else if (c == 'f')
                {
                    // Double    
                    arg.SetNumber((Value::Number)va_arg(args, double));
                    arr.PushBack(arg);

                    // MA: What about float? C specification states that "float converts to
                    // double by the standard argument promotion", so this works. But,
                    // what happens on PS2?  Do we needs '%hf' specifier for float?
                }
                else if (c == 'h')
                {
                    c = *p++;
                    if (c == 'f')
                    {
                        // AB: %hf will be treated as double too, since C spec states
                        // that float is converted to double.
                        // Double
                        arg.SetNumber((Value::Number)va_arg(args, double));
                        arr.PushBack(arg);
                        SF_DEBUG_WARNING2(1, "ParseArguments('%s','%s') - '%%hf' will be treated as double", 
                            pmethodName,
                            pargFmt);
                    }
                    else
                    {
                        if (plog)
                            plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%h%c'",
                                pmethodName,
                                pargFmt,
                                c);
                    }
                }
                else if (c == 's')
                {
                    // String                
                    arr.PushBack(GetBuiltinsMgr().CreateString(va_arg(args, const char *)));
                }
                else if (c == 'l')
                {
                    c = *p++;
                    if (c == 's')
                    {
                        // Wide string.
                        arr.PushBack(GetBuiltinsMgr().CreateString(va_arg(args, const wchar_t *)));
                    }
                    else
                    {
                        if (plog)
                            plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%l%c'",
                                pmethodName,
                                pargFmt,
                                c);
                    }
                }
                else
                {
                    // Invalid fmt, warn.
                    if (plog)
                        plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%%c'",
                            pmethodName,
                            pargFmt,
                            c);
                }
            }
            else
            {
                // Invalid arg; warn.                
                if (plog)
                    plog->LogScriptError("ParseArguments('%s','%s') - invalid char '%c'",
                        pmethodName,
                        pargFmt,
                        c);
            }
            // skip all whitespaces
            for (c = *p; c != '\0' && (c == ' ' || c == '\t' || c == ','); c = *++p) 
                ;
        }
    }
}

bool MovieRoot::InvokeArgs(const char* pmethodName, GFx::Value *presult, const char* pargFmt, va_list args)
{
    // TODO: Check VM
    bool retVal;

    SF_DEBUG_ASSERT(pmethodName, "Error: NULL pmethodName passed to InvokeArgs");
    SF_DEBUG_ASSERT(pargFmt, "Error: NULL pargFmt passed to InvokeArgs");

    // There should not be an exception in the VM
    SF_ASSERT(!pAVM->IsException());

    // Parse args into AS3::Value.s
    Array<Value> valArray;
    ParseValueArguments(valArray, pmethodName, pargFmt, args);
    
    unsigned numArgs = static_cast<unsigned>(valArray.GetSize());
    Value* pargArray = numArgs > 0 ? &valArray[0] : NULL;
    
    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Try to resolve invoke alias
    Value* paliasFunc;
    Value resultVal;

    if (pInvokeAliases && (paliasFunc = ResolveInvokeAlias(pmethodName)) != NULL)
        pAVM->ExecuteUnsafe(*paliasFunc, Value::GetUndefined(), resultVal, numArgs, pargArray);
    else
    {
        Value resolvedVal;
        if (GetASVariableAtPath(&resolvedVal, pmethodName))
            pAVM->ExecuteUnsafe(resolvedVal, Value::GetUndefined(), resultVal, numArgs, pargArray);
        else 
            return false;
    }

    retVal = !pAVM->IsException();
    if (pAVM->IsException()) pAVM->IgnoreException();

    // Convert result to GFx::Value
    if (retVal && presult)
        ASValue2GFxValue(resultVal, presult);

    return retVal;
}


// *** Action List management
MovieRoot::ActionQueueType::ActionQueueType(MemoryHeap *pheap)
: pHeap(pheap)
{
    ModId = 1;
    pFreeEntry = NULL;
    FreeEntriesCount = 0;
}

MovieRoot::ActionQueueType::~ActionQueueType()
{
    Clear();

    // free pFreeEntry chain
    for(ActionEntry* pcur = pFreeEntry; pcur; )
    {
        ActionEntry* pnext = pcur->pNextEntry;
        delete pcur;
        pcur = pnext;
    }
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::GetNewEntry()
{
    ActionEntry *p = 0;
    if (pFreeEntry)
    {
        p = pFreeEntry;
        pFreeEntry = pFreeEntry->pNextEntry;
        p->pNextEntry = 0;
        --FreeEntriesCount;
    }
    else
    {
        if ((p = SF_HEAP_NEW(GetMovieHeap()) ActionEntry)==0)
            return 0;
    }
    return p;
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::InsertEntry(ActionLevel lvl)
{
    ActionEntry *p = GetNewEntry();

    ActionQueueEntry& actionQueueEntry = Entries[lvl];

    // Insert and move insert pointer to us.
    if (actionQueueEntry.pInsertEntry == 0) // insert at very beginning
    {
        p->pNextEntry                   = actionQueueEntry.pActionRoot;
        actionQueueEntry.pActionRoot    = p;
    }
    else
    {
        p->pNextEntry = actionQueueEntry.pInsertEntry->pNextEntry;
        actionQueueEntry.pInsertEntry->pNextEntry = p;
    }
    actionQueueEntry.pInsertEntry   = p;
    if (p->pNextEntry == NULL)
        actionQueueEntry.pLastEntry = p;
    ++ModId;

    // Done
    return p;
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::PrependEntry(ActionLevel lvl)
{
    ActionEntry *p = GetNewEntry();

    ActionQueueEntry& actionQueueEntry = Entries[lvl];
    
    // insert at very beginning
    p->pNextEntry                   = actionQueueEntry.pActionRoot;
    actionQueueEntry.pActionRoot    = p;
    if (actionQueueEntry.pInsertEntry == 0) 
        actionQueueEntry.pInsertEntry   = p;
    if (p->pNextEntry == NULL)
        actionQueueEntry.pLastEntry = p;
    ++ModId;

    // Done
    return p;
}

#if 0
MovieRoot::ActionEntry* MovieRoot::ActionQueueType::FindEntry(
    ActionLevel lvl, const ActionEntry& entry)
{
    ActionQueueEntry& queue = Entries[lvl];
    ActionEntry* pentry = queue.pActionRoot;
    for (; pentry != NULL; pentry = pentry->pNextEntry)
    {
        if (*pentry == entry)
            return pentry;
    }
    return NULL;
}
#endif

void MovieRoot::ActionQueueType::AddToFreeList(MovieRoot::ActionEntry* pentry)
{
    pentry->ClearAction();
    if (FreeEntriesCount < 50)
    {
        pentry->pNextEntry = pFreeEntry;
        pFreeEntry = pentry;
        ++FreeEntriesCount;
    }
    else
        delete pentry;
}

void    MovieRoot::ActionQueueType::Clear()
{   
    for (unsigned i = 0; i < AL_Total; ++i)
    {
        MovieRoot::ActionQueueIterator iter((ActionLevel)i, this);
        while(iter.getNext())
            ;
    }
}

MovieRoot::ActionQueueIterator::ActionQueueIterator(
    ActionLevel lvl, MovieRoot::ActionQueueType* pactionQueue, ActionEntry* rootEntry)
{
    SF_ASSERT(pactionQueue);
    pActionQueue    = pactionQueue;
    Level           = lvl;
    ModId           = 0;
    pLastEntry      = 0;
    pRootEntry      = rootEntry;
    pCurEntry       = (pRootEntry) ? pRootEntry->pNextEntry : pActionQueue->Entries[Level].pActionRoot;
}

MovieRoot::ActionQueueIterator::~ActionQueueIterator()
{
    if (pLastEntry)
        pActionQueue->AddToFreeList(pLastEntry); // release entry
}

const MovieRoot::ActionEntry* MovieRoot::ActionQueueIterator::getNext()
{
    if (pActionQueue->ModId != ModId)
    {
        // new actions were added - restart
        ModId = pActionQueue->ModId;
        pCurEntry = (pRootEntry) ? pRootEntry->pNextEntry : pActionQueue->Entries[Level].pActionRoot;
    }
   
    ActionEntry* pentry = pCurEntry;
    if (pCurEntry)
    {
        if (pCurEntry == pActionQueue->Entries[Level].pInsertEntry)
        {
            if (pCurEntry->pNextEntry)
                pActionQueue->Entries[Level].pInsertEntry = pCurEntry->pNextEntry;
            else
                pActionQueue->Entries[Level].pInsertEntry = pRootEntry;
        }
        if (pCurEntry == pActionQueue->Entries[Level].pActionRoot)
            pActionQueue->Entries[Level].pActionRoot = pCurEntry->pNextEntry;
        else if (pRootEntry)
            pRootEntry->pNextEntry = pCurEntry->pNextEntry;
        pCurEntry = pCurEntry->pNextEntry;
    }
    if (pActionQueue->Entries[Level].pActionRoot == NULL)
    {
        pActionQueue->Entries[Level].pInsertEntry = NULL;
        pActionQueue->Entries[Level].pLastEntry = NULL;
    }
    if (pLastEntry)
    {
        pLastEntry->pNextEntry = NULL;
        pActionQueue->AddToFreeList(pLastEntry); // release entry
        // queue is modified; in the case if there other iterators we need to 
        // update ModId to force those iterators to restart.
        ++pActionQueue->ModId;
        ModId = pActionQueue->ModId;
    }
    pLastEntry = pentry;
    return pentry;
}

#if 0
bool MovieRoot::ActionEntry::operator==(const MovieRoot::ActionEntry& e) const
{
    if (Type != e.Type || pCharacter != e.pCharacter) 
        return false;
    switch(Type)
    {
    case Entry_Event:       
        return mEventId == e.mEventId;
    case Entry_Function: 
        return Function == e.Function;
    case Entry_CFunction:        
        return pAS3Obj == e.pAS3Obj && CFunction == e.CFunction;
    default:;
    }
    return true;
}
#endif

// Executes actions in this entry
void    MovieRoot::ActionEntry::Execute(MovieRoot *proot) const
{   
    SF_UNUSED(proot);
    // pCharacter might be null, if only InsertEmptyAction was used, without
    // subsequent call to SetAction.
    if (!pCharacter || pCharacter->IsUnloaded() /*|| pCharacter->IsExecutionAborted()*/)
        return;

    switch(Type)
    {
    case Entry_Event:       
        //ToAvmCharacter(pCharacter)->ExecuteEvent(EventId);      
        ToAvmDisplayObj(pCharacter)->FireEvent(mEventId);
        break;

    case Entry_Function:        
        {
            SF_ASSERT(proot->pAVM);
            Value _this(ToAvmDisplayObj(pCharacter)->GetAS3Obj());
            Value result;
            proot->pAVM->ExecuteUnsafe(Function, _this, result, 0, NULL);
            if (proot->pAVM->IsException())    
            {
                proot->pAVM->OutputAndIgnoreException();
                pCharacter->SetExecutionAborted();
            }
        }
        break;

    case Entry_CFunction:        
        if (CFunction)
        {
            CFunction(*this);
        }
        break;

    case Entry_None:
        break;
    }
}

void    MovieRoot::AddInvokeAlias(const ASString& alias, const Value& closure)
{
    if (!pInvokeAliases)
        pInvokeAliases = new ASStringHash<Value>();
    pInvokeAliases->Set(alias, closure);
}

Value* MovieRoot::ResolveInvokeAlias(const char* pstr)
{
    SF_ASSERT(GetStringManager());
    if (pInvokeAliases)
    {
        ASString str = GetStringManager()->CreateString(pstr);
        return pInvokeAliases->Get(str);
    }
    return NULL;
}

void MovieRoot::AddScriptableMovieClip(DisplayObjContainer* pspr)
{
    if (!pspr->GetParent())
    {
        //pStage->InsertChildToPlayList(pspr);
        pspr->AddToPlayList();
        pspr->ModifyOptimizedPlayList();
    }
}

void MovieRoot::AddToExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext)
{
	if (!ExtensionContextArray)
        ExtensionContextArray = SF_HEAP_NEW(GetMovieHeap()) ArrayLH<Instances::fl_external::ExtensionContext*>();
    
    if (FindExtensionContexts(pExtensionContext) == SF_MAX_UPINT)
        ExtensionContextArray->PushBack(pExtensionContext);
}

void MovieRoot::RemoveFromExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext)
{
	UPInt i = FindExtensionContexts(pExtensionContext);
    if (i != SF_MAX_UPINT)
    {
        (*ExtensionContextArray)[i] = NULL;
    }
}

UPInt MovieRoot::FindExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext)
{
    if (ExtensionContextArray)
    {
        for (UPInt i = 0; i < ExtensionContextArray->GetSize(); ++i)
        {
			if ((*ExtensionContextArray)[i] == pExtensionContext)
                return i;
        }
    }

    return SF_MAX_UPINT;
}

UPInt MovieRoot::FindExtensionContexts(const ASString& extensionID, const ASString& contextType)
{
    if (ExtensionContextArray)
    {
        for (SPInt i = ExtensionContextArray->GetSize() - 1; i >= 0;  --i)
        {
			Instances::fl_external::ExtensionContext* pTempExtCtx = (*ExtensionContextArray)[i];
			if (pTempExtCtx == NULL)
			{
                ExtensionContextArray->RemoveAt(i);
                continue;
            }
			if (extensionID == pTempExtCtx->ExtensionID)
			{
				/* valid cases: contextType is "" and pTempExtCtx->ContextID is null or "" */
				if ((contextType == pTempExtCtx->ContextID) || (contextType.IsEmpty() && pTempExtCtx->ContextID.IsNull()))
					return i;
			}
        }
    }

    return SF_MAX_UPINT;
}

void MovieRoot::NotifyStatusEvent(const EventId* id)
{
	const GFx::StatusEventId& seid = static_cast<const GFx::StatusEventId&>(*id);
	//SF_DEBUG_MESSAGE4(1, "NotifyStatusEvent %p %s %s (%s)\n", seid.ExtensionId, seid.ExtensionId->ToCStr(), seid.Code->ToCStr(), seid.Level->ToCStr());

	// Status event from geolocation.
	if (seid.ExtensionId == NULL)
		this->BroadcastGeolocationStatusEvent(id);
	else
	{
		ASString extensionId = GetStringManager()->CreateString(*seid.ExtensionId); 
		ASString contextId = GetStringManager()->CreateString(*seid.ContextId);

		UPInt i = this->FindExtensionContexts(extensionId, contextId);
		if (i != SF_MAX_UPINT)
			(*ExtensionContextArray)[i]->Dispatch(*id, NULL);

		delete seid.ExtensionId;
		delete seid.ContextId;
	}

	// Release event strings
	delete seid.Code;
	delete seid.Level;
}

//////////////////////////////////////////////////////////////////////////

void MovieRoot::NotifyAccelerometerEvent(const EventId* id, int evtIdAcc)
{
    if (AccelerometerArray)
    {
        for (SPInt i = AccelerometerArray->GetSize() - 1; i >= 0;  --i)
        {
            Instances::fl_sensors::Accelerometer* pAcc = (*AccelerometerArray)[i];
            if (pAcc == NULL)
            {
                // remove empty entries
                AccelerometerArray->RemoveAt(i);
                continue;
            }

            if (pAcc->AccelerometerId == evtIdAcc)
            {
                pAcc->Dispatch(*id, NULL);
                return;
            }
        }
    }
}

void MovieRoot::AddToAccelerometers(Instances::fl_sensors::Accelerometer* pAccelerometer)
{
    if (!AccelerometerArray)
        AccelerometerArray = SF_HEAP_NEW(GetMovieHeap()) ArrayLH<Instances::fl_sensors::Accelerometer* >();
    
    if (FindAccelerometer(pAccelerometer) == SF_MAX_UPINT)
        AccelerometerArray->PushBack(pAccelerometer);
}


UPInt MovieRoot::FindAccelerometer(Instances::fl_sensors::Accelerometer* pAccelerometer)
{
    if (AccelerometerArray)
    {
        for (UPInt i = 0; i < AccelerometerArray->GetSize(); ++i)
        {
            if ((*AccelerometerArray)[i] == pAccelerometer)
                return i;
        }
    }

    return SF_MAX_UPINT;
}

void MovieRoot::RemoveFromAccelerometers(Instances::fl_sensors::Accelerometer* pAccelerometer)
{
    UPInt i = FindAccelerometer(pAccelerometer);
    if (i != SF_MAX_UPINT)
    {
        (*AccelerometerArray)[i] = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////

void MovieRoot::NotifyGeolocationEvent(const EventId* id, int evtIdGeo)
{
    if (GeolocationArray)
    {
        for (SPInt i = GeolocationArray->GetSize() - 1; i >= 0;  --i)
        {
            Instances::fl_sensors::Geolocation* pGeo = (*GeolocationArray)[i];
            if (pGeo == NULL)
            {
                // remove empty entries
                GeolocationArray->RemoveAt(i);
                continue;
            }

            if (pGeo->GeolocationId == evtIdGeo)
            {
                pGeo->Dispatch(*id, NULL);
                return;
            }
        }
    }
}

void MovieRoot::BroadcastGeolocationStatusEvent(const EventId* id)
{
    if (GeolocationArray)
    {
        for (SPInt i = GeolocationArray->GetSize() - 1; i >= 0;  --i)
        {
            Instances::fl_sensors::Geolocation* pGeo = (*GeolocationArray)[i];
            if (pGeo == NULL)
            {
                // remove empty entries
                GeolocationArray->RemoveAt(i);
                continue;
            }

			pGeo->Dispatch(*id, NULL);
        }
    }
}


void MovieRoot::AddToGeolocations(Instances::fl_sensors::Geolocation* pGeolocation)
{
    if (!GeolocationArray)
        GeolocationArray = SF_HEAP_NEW(GetMovieHeap()) ArrayLH<Instances::fl_sensors::Geolocation* >();
    
    if (FindGeolocation(pGeolocation) == SF_MAX_UPINT)
        GeolocationArray->PushBack(pGeolocation);
}


UPInt MovieRoot::FindGeolocation(Instances::fl_sensors::Geolocation* pGeolocation)
{
    if (GeolocationArray)
    {
        for (UPInt i = 0; i < GeolocationArray->GetSize(); ++i)
        {
            if ((*GeolocationArray)[i] == pGeolocation)
                return i;
        }
    }

    return SF_MAX_UPINT;
}

void MovieRoot::RemoveFromGeolocations(Instances::fl_sensors::Geolocation* pGeolocation)
{
    UPInt i = FindGeolocation(pGeolocation);
    if (i != SF_MAX_UPINT)
    {
        (*GeolocationArray)[i] = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
void EventChains::AddToChain(EventId::IdCode evtId, DisplayObject* obj)
{
    AutoPtr<Chain>* ppchain;
    Chain* chain;
    if ((ppchain = Chains.Get(evtId)) == NULL)
    {
        // create new array
        chain = SF_HEAP_AUTO_NEW(this) Chain();
        Chains.Add(evtId, chain);
    }
    else
        chain = ppchain->GetPtr();
    SF_ASSERT(chain);
    if (FindObjectIndexInChain(chain, obj) == SF_MAX_UPINT)
        chain->PushBack(obj);
}

void EventChains::RemoveFromChain(EventId::IdCode evtId, DisplayObject* obj)
{
    AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        UPInt i = FindObjectIndexInChain(chain, obj);
        if (i != SF_MAX_UPINT)
        {
            // nullify; remove in Dispatch to avoid
            // concurrent modification.
            (*chain)[i] = NULL;
        }
    }
}

const EventChains::Chain* EventChains::GetChain(EventId::IdCode evtId) const
{
    const AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
        return ppchain->GetPtr();
    return NULL;
}

void EventChains::Dispatch(EventId::IdCode evtId)
{
   AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        for (UPInt i = 0; i < chain->GetSize();)
        {
            if ((*chain)[i] == NULL)
            {
                // remove empty entries
                chain->RemoveAt(i);
                continue;
            }
            EventId evt(evtId);
            Ptr<DisplayObject> dobj = (*chain)[i];
            ToAvmDisplayObj(dobj)->FireEvent(evt);
            ++i;
        }
    }
}

void EventChains::QueueEvents(EventId::IdCode evtId)
{
    AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        for (UPInt i = 0; i < chain->GetSize();)
        {
            if ((*chain)[i] == NULL)
            {
                // remove empty entries
                chain->RemoveAt(i);
                continue;
            }
            EventId evt(evtId);
            Ptr<DisplayObject> dobj = (*chain)[i];
            ToAvmDisplayObj(dobj)->OnEvent(evt);
            ++i;
        }
    }
}

UPInt EventChains::FindObjectIndexInChain(Chain* chain, DisplayObject* obj)
{
    for (UPInt i = 0, n = chain->GetSize(); i < n; ++i)
    {
        if ((*chain)[i] == obj)
            return i;
    }
    return SF_MAX_UPINT;
}

// ****************************************************************************
// Adds load queue entry based on parsed url and target path 
// (Load/unload Movie)
//
LoadQueueEntry::LoadQueueEntry(Instances::fl_net::URLRequest* request, Instances::fl_display::Loader* loader, LoadMethod method, bool quietOpen)
: GFx::LoadQueueEntry((request)?request->GetUrl().ToCStr():"", method, false, quietOpen)
{ 
    mLoader     = loader;
    mURLRequest = request;
    FirstExec   = true;
	NotifyLoadInitCInterface = NULL;
}

LoadQueueEntry::LoadQueueEntry(Instances::fl_utils::ByteArray* bytes, Instances::fl_display::Loader* loader, LoadMethod method)
: GFx::LoadQueueEntry("", method, false, false)
{ 
    mLoader     = loader;
    FirstExec   = true;
    NotifyLoadInitCInterface = NULL;
    mBytes = SF_HEAP_AUTO_NEW(this) ArrayPOD<UByte>();
    mBytes->Resize(bytes->GetLength());
    memcpy(mBytes->GetDataPtr(), bytes->GetDataPtr(), mBytes->GetSize());
}

LoadQueueEntry::LoadQueueEntry(Instances::fl_net::URLRequest* request, Instances::fl_net::URLLoader* loader, LoadMethod method, bool quietOpen) 
: GFx::LoadQueueEntry((request)?request->GetUrl().ToCStr():"", method, true, quietOpen)
{
    mURLLoader  = loader;
    mURLRequest = request;
    FirstExec   = true;
	NotifyLoadInitCInterface = NULL;
}   

LoadQueueEntry::~LoadQueueEntry()
{
	if (NotifyLoadInitCInterface)
		NotifyLoadInitCInterface = 0;
}

LoadQueueEntryMT_LoadMovie::LoadQueueEntryMT_LoadMovie(
    LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: GFx::LoadQueueEntryMT(pqueueEntry, pmovieRoot)
{
    CharSwitched        = false;
    FirstFrameLoaded    = false;
    BytesLoaded         = 0;

    bool stripped       = false;
    MovieRoot* root = ToAS3Root(pMovieImpl);
    if (root->GetMainContainer())
    {
        stripped = ((root->GetMainContainer()->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
    }
    pPreloadTask = *SF_NEW MoviePreloadTask(pMovieImpl, pqueueEntry->URL, stripped, pqueueEntry->QuietOpen);
    pMovieImpl->GetTaskManager()->AddTask(pPreloadTask);
}

LoadQueueEntryMT_LoadMovie::~LoadQueueEntryMT_LoadMovie()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool LoadQueueEntryMT_LoadMovie::LoadFinished()
{
    bool btaskDone = pPreloadTask->IsDone();
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
    {
        return true;
    }

    //bool extensions = false;

    LoadQueueEntry* qe = static_cast<LoadQueueEntry*>(pQueueEntry);
    MovieRoot*   root = ToAS3Root(pMovieImpl);
    if (btaskDone)
    {
        // Preload task is finished by now. Check if a movedef is created
        MovieDefImpl* pdefImpl = pPreloadTask->GetMoiveDefImpl();
        if (!pdefImpl)
        {
            // A moviedef was not created. This means that loader couldn't load a file. 
            // Creating an empty movie and set it instead of the old one.

            //printf("!!! ERR LoadFinished!\n");
            qe->mLoader->ExecuteErrorEvent(qe->mURLRequest->GetUrl().ToCStr());
			if (qe->NotifyLoadInitCInterface)
				qe->NotifyLoadInitCInterface->ErrorEventCallback();

            qe->FirstExec = false;
            goto error_exit;
        }
        else
        {
            if (qe->FirstExec)
            {
                if (pdefImpl->GetVersion() != ~0u && (pdefImpl->GetVersion() < 9 || pdefImpl->GetDataDef()->GetASVersion() < 3))
                {
                    pQueueEntry->Canceled = true;
                    if (root->GetLogState() && !qe->QuietOpen)
                    {
                        root->GetLogState()->LogScriptWarning("Failed loading SWF \"%s\": ActionScript version mismatch", 
                            qe->mURLRequest->GetUrl().ToCStr());
                    }
                    qe->mLoader->ExecuteErrorEvent(qe->mURLRequest->GetUrl().ToCStr());
                    if (qe->NotifyLoadInitCInterface)
                        qe->NotifyLoadInitCInterface->ErrorEventCallback();
                    pdefImpl = NULL;
                    goto error_exit;
                }
                qe->mLoader->ExecuteOpenEvent();
				if (qe->NotifyLoadInitCInterface)
					qe->NotifyLoadInitCInterface->OpenEventCallback();

                qe->FirstExec = false;

                root->AddLoadedMovieDef(pdefImpl);
            }
            // A moviedef was created successfully.
            if (!CharSwitched)
            {
                //Replace the old character with the new sprite. This operation should be done only once
                //per loading movie.
                DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                SF_UNUSED(pparent);
                CharSwitched = true;
            }

            // The number of loaded bytes has changed since the last iteration and we have a movie loader listener
            // sent a progress notification.
            if (BytesLoaded != pdefImpl->GetBytesLoaded())
            {
                // get Loader
                BytesLoaded = pdefImpl->GetBytesLoaded();
                qe->mLoader->ExecuteProgressEvent(BytesLoaded, pdefImpl->GetFileBytes());
            }

            // when we loaded the first frame process its actions.
            if (!FirstFrameLoaded && 
                (pdefImpl->pBindData->GetBindStateFlags() & MovieDefImpl::BSF_Frame1Loaded) != 0)
            {
                // First frame is loaded.
                if (pdefImpl->GetDataDef()->MovieType == MovieDataDef::MT_Flash)
                {

                    // This branch will be executed when gfxExtensions == true, to 
                    // produce different (better) behavior during the loading. If extensions
                    // are on then we can start the loading movie after the first frame is loaded.
                    // Flash, for some reasons, waits until whole movie is loaded.

                    DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                    ResourceId newCharId = ResourceId(0); // root element always has 0 res id.
                    Ptr<Sprite> pnewChar;
                    if (pdefImpl)
                    {
                        // Trying to cleanup all remaining objects (incl VMAbcFile) before
                        // creating new ones. Might help to avoid classes intersections across
                        // SWF files.
                        if (root->GetAVM())
                            root->GetAVM()->GetGC().ForceCollect(pMovieImpl->AdvanceStats);

                        // CharId of loaded clip is the same as the old one
                        // Also mark clip as loadedSeparately, so that _lockroot works.
                        CharacterCreateInfo ccinfo(pdefImpl->GetDataDef(), pdefImpl);
                        pnewChar = *static_cast<Sprite*>(root->GetASSupport()->CreateCharacterInstance(
                            pMovieImpl, ccinfo, 
                            NULL, newCharId, CharacterDef::Sprite));
                        pnewChar->SetLoadedSeparately();
                        pnewChar->SetTimelineObjectFlag(true);
                        pnewChar->OnEventLoad();

                        pnewChar->AddToPlayList();

                        AvmSprite* avmSpr = ToAvmSprite(pnewChar);
                        //!AB: need to run just InitActions here, it will initialize Abc by
                        // executing DoAbc tag. All further actions will be executed by 
                        // CreateASInstance, from "super" call.
                        Instances::fl_display::LoaderInfo* info = qe->mLoader->GetContentLoaderInfo();
                        if (info != NULL)
                        {
                            SPtr<Instances::fl_system::ApplicationDomain> appDomain = info->applicationDomainGet();
                            if (appDomain)
                            {
                                avmSpr->SetAppDomain(appDomain->GetAppDomain());
                            }
                        }
                        avmSpr->ExecuteInitActionFrameTags(0);

                        // Allocate a default PerspectiveDataType object and set fieldOfView.
                        pnewChar->SetFOV(DEFAULT_FLASH_FOV);   

                        if (!avmSpr->HasAS3Obj())
                        {
                            if (avmSpr->CreateASInstanceNoCtor())
                            {
                                avmSpr->GetAS3Obj()->SetLoaderInfo(*qe->mLoader);
                                avmSpr->CallCtor(true);
                            }
                        }

                        //root->InsertEmptyAction(MovieRoot::AL_Highest)->SetAction(pnewChar, EventId::Event_Initialize);
                        //root->InsertEmptyAction(MovieRoot::AL_High)->SetAction(pnewChar, EventId::Event_Construct);

                        ToAvmDisplayObjContainer(pparent)->AddChild(pnewChar);
                        //pparent->ReplaceChildCharacter(poldChar, pnewChar);
                        // In case permanent variables were assigned.. check them.
                        root->ResolveStickyVariables(pnewChar);

                        // need to update the optimized play list
                        pnewChar->ModifyOptimizedPlayListLocal<Sprite>();
                        //pnewChar->ExecuteFrame0Events();
                        root->DoActions();
                    }
                    
                    // Fire "init" event
                    qe->mLoader->QueueInitEvent(pnewChar, qe->NotifyLoadInitCInterface);
				
                }
                FirstFrameLoaded = true;
            }
            //printf("!!! LoadFinished! %s\n", qe->mURLRequest->GetUrl().ToCStr());

            switch (pdefImpl->pBindData->GetBindStateType())
            {
            case MovieDefImpl::BS_NotStarted:
            case MovieDefImpl::BS_InProgress:
                // If the movie is not completely loaded yet try it on the next iteration.
                return false;
                break;
            case MovieDefImpl::BS_Finished:
                //printf("!!! Finished! %d == %d\n", pdefImpl->GetDataDef()->MovieType, MovieDataDef::MT_Image);
                if (pdefImpl->GetDataDef()->MovieType == MovieDataDef::MT_Image)
                {
                    // bitmap
                    DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                    ResourceId NewCharId = ResourceId(); // Assign invalid id.
                    CharacterCreateInfo ccinfo(pdefImpl->GetDataDef(), pdefImpl);
                    Ptr<AS3::AvmBitmap> pnewChar = *static_cast<AS3::AvmBitmap*>(root->GetASSupport()->CreateCharacterInstance(
                        root->GetMovieImpl(), ccinfo, 
                        NULL, NewCharId, CharacterDef::Bitmap));

                    ResourceHandle rh;
                    if (pdefImpl->GetDataDef()->GetResourceHandle(&rh, ResourceId(CharacterDef::CharId_ImageMovieDef_ImageResource)))
                    {
                        Resource* pres = rh.GetResource(&pdefImpl->GetResourceBinding());
                        //printf("!!! Res: %p\n", pres);
                        if (pres)
                        {
                            if (pres->GetResourceType() == Resource::RT_Image)
                            {
                                //printf("!!! SetImage\n");
                                pnewChar->SetImage(static_cast<ImageResource*>(pres));
                            }
                        }
                    }

                    AvmDisplayObj* avmObj = ToAvmDisplayObj(pnewChar);
                    if (!avmObj->HasAS3Obj())
                    {
                        if (avmObj->CreateASInstanceNoCtor())
                        {
                            avmObj->GetAS3Obj()->SetLoaderInfo(*qe->mLoader);
                            avmObj->CallCtor(true);
                        }
                    }
                    ToAvmDisplayObjContainer(pparent)->AddChild(pnewChar);

                    // Fire "init" event
                    qe->mLoader->QueueInitEvent(pnewChar, qe->NotifyLoadInitCInterface);
                }

                // fire "complete" event
                qe->mLoader->QueueCompleteEvent();
                break;
            case MovieDefImpl::BS_Canceled:
            case MovieDefImpl::BS_Error:
                // A error happened during the movie loading 
                qe->mLoader->ExecuteErrorEvent(qe->mURLRequest->GetUrl().ToCStr());
                qe->FirstExec = false;
                break;
            default:
                SF_ASSERT(false);
                break;
            }
        }
error_exit:
        // Loading finished. Remove the item from the queue.
        return true;
    }
    else
    {
        if (qe->FirstExec)
        {
            qe->mLoader->ExecuteOpenEvent();
            qe->FirstExec = false;
        }
    }   
    return false;
}

bool LoadQueueEntryMT_LoadMovie::IsPreloadingFinished()
{
    return pPreloadTask->IsDone();
}


// *** Load/Unload movie support
void    MovieRoot::AddLoadQueueEntryMT(LoadQueueEntry* pentry)
{
    LoadQueueEntryMT* pentryMT = NULL;
    if (pentry->Type & LoadQueueEntry::LTF_VarsFlag)
    {
#ifdef GFX_AS2_ENABLE_LOADVARS
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadVars(pentry, pMovieImpl);
#endif
    }
    else if (pentry->Type & LoadQueueEntry::LTF_BinaryFlag)
    {
#ifdef GFX_AS2_ENABLE_LOADVARS
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadBinary(pentry, pMovieImpl);
#endif
    }
    /*else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_XMLFlag)
    {
#ifdef GFX_ENABLE_XML
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadXML(pentry, this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->XMLHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->XMLHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByXMLASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    } 
    else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_CSSFlag)
    {
#ifdef GFX_ENABLE_CSS
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadCSS(pentry,this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->CSSHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->CSSHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByCSSASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    }*/
    else 
    {
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadMovie(pentry, pMovieImpl);
        if (pentryMT)
        {
            LoadQueueEntry* asentry = static_cast<LoadQueueEntry*>(pentryMT->GetQueueEntry());
            LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
            while (ptail)    
            {
                LoadQueueEntry* astail = static_cast<LoadQueueEntry*>(ptail->GetQueueEntry());
                astail->Cancel(asentry);
                ptail = ptail->GetNext();
            }
        }
    }
    if (!pentryMT)
    {
        delete pentry;
        return;
    }
    pMovieImpl->AddLoadQueueEntryMT(pentryMT);
}

// ****************************************************************************
// Adds load queue entry based on parsed url and target character 
// (Load/unload Movie)
//
void MovieRoot::AddNewLoadQueueEntry(Instances::fl_net::URLRequest* urlRequest,
                                     Instances::fl_display::Loader* loader,
                                     LoadQueueEntry::LoadMethod method)
{   
    SF_ASSERT(loader);

    LoadQueueEntry* pentry = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntry(urlRequest, loader, method);
    if (pentry)
    {
        bool sync;
        // Check to see if URL is a user image substitute.
        bool userImageProtocol = LoaderImpl::IsProtocolImage(pentry->URL, NULL, &sync);

        if (userImageProtocol && sync)
        {
            // Custom image protocol can be loaded synchronously
            Ptr<LoadStates>  pls = *new LoadStates(GetMovieImpl()->GetLoaderImpl(), 
                                                   GetMovieImpl()->GetStateBagImpl());
            ProcessLoadQueueEntry(pentry, pls);
            delete pentry;
            return;
        }

        if ( pentry->URL.GetLength() > 0 && !userImageProtocol && pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

void MovieRoot::AddNewLoadQueueEntry(Instances::fl_utils::ByteArray* bytes,
                                     Instances::fl_display::Loader* loader,
                                     LoadQueueEntry::LoadMethod method)
{   
    SF_ASSERT(loader);

    LoadQueueEntry*pentry = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntry(bytes, loader, method);
    if (pentry)
    {
        AddLoadQueueEntry(pentry);
    }
}

void MovieRoot::AddNewLoadQueueEntry(Instances::fl_net::URLRequest* urlRequest,
                                     Instances::fl_net::URLLoader* loader,
                                     LoadQueueEntry::LoadMethod method)
{
    SF_ASSERT(loader);

    LoadQueueEntry*pentry = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntry(urlRequest, loader, method);
    if (pentry)
    {
        if (loader->IsLoadingBinary())
            pentry->Type = LoadQueueEntry::LT_LoadBinary;
        else
            pentry->Type = LoadQueueEntry::LT_LoadText;
        if (pentry->URL.GetLength() > 0 && pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

// Fills in a file system path relative to _level0.
bool MovieRoot::GetRootFilePath(String *ppath) const
{       
    if (!GetMainContainer())
    {
        ppath->Clear();
        return false;
    }

    // Get URL.
    *ppath = GetMovieDefImpl()->GetFileURL();

    // Extract path by clipping off file name.
    if (!URLBuilder::ExtractFilePath(ppath))
    {
        ppath->Clear();
        return false;
    }
    return true;
}

Text::Allocator* MovieRoot::GetTextAllocator()
{
    if (!MemContext->TextAllocator)
    {
        MemContext->TextAllocator = *SF_HEAP_NEW(MemContext->Heap) Text::Allocator(MemContext->Heap);
    }
    return MemContext->TextAllocator;
}

void MovieRoot::ProcessLoadQueueEntry(GFx::LoadQueueEntry *pentry, LoadStates* pls)
{
    if (pentry->Canceled)
        return;

    LoadQueueEntry& entry = static_cast<LoadQueueEntry&>(*pentry);
    if (pentry->Type & (LoadQueueEntry::LTF_VarsFlag | LoadQueueEntry::LTF_BinaryFlag))
    {
        // loading data using URLLoader
        bool userVarsProtocol = false;
        bool error = false;

        // @TODO logic to determine if a user protocol was used

        String data;
        ArrayPOD<UByte> binaryData;
        int    fileLen = 0;
        if (userVarsProtocol)
        {
            // @TODO
        }
        else
        {
            String rootPath;
            GetRootFilePath(&rootPath);
            URLBuilder::LocationInfo loc(URLBuilder::File_LoadVars, pentry->URL, rootPath);
            String                   fileName;
            pls->BuildURL(&fileName, loc);

            // File loading protocol
            Ptr<File> pfile;
            Array<UByte> bytes;
            if (URLBuilder::IsProtocol(fileName))
            {
#ifdef SF_ENABLE_HTTP_LOADING
                URLBuilder::UrlMethod sendUrlRequest = URLBuilder::Url_Method_None;
                switch (entry.Method)
                {
                case LoadQueueEntry::LM_Get:
                    sendUrlRequest = URLBuilder::Url_Method_Get;
                    break;
                case LoadQueueEntry::LM_Post:
                    sendUrlRequest = URLBuilder::Url_Method_Post;
                    break;
                case LoadQueueEntry::LM_Put:
                    sendUrlRequest = URLBuilder::Url_Method_Put;
                    break;
                case LoadQueueEntry::LM_Delete:
                    sendUrlRequest = URLBuilder::Url_Method_Delete;
                    break;
                case LoadQueueEntry::LM_None:
                    break;
                default:
                    SF_ASSERT(false); // Unsupported method
                    break;
                }

                if (sendUrlRequest != URLBuilder::Url_Method_None)
                {
                    ASString dataFields(GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_));
                    GlobalSlotIndex slotIdx(0);
                    Value data = entry.mURLRequest->dataGet();
                    if (data.IsObject())
                    {
                        SPtr<Object> dataObj = data.GetObject();
                        if (dataObj->GetName() == "URLVariables")
                        {
                            Instances::fl_net::URLVariables* varData = static_cast<Instances::fl_net::URLVariables*>(dataObj.GetPtr());
                            varData->toString(dataFields);
                            if (sendUrlRequest == URLBuilder::Url_Method_Get)
                            {
                                fileName.AppendChar('?');
                                fileName.AppendString(dataFields.ToCStr());
                                dataFields.Clear();
                            }
                        }
                        else if (dataObj->GetTraits().GetTraitsType() == Traits_ByteArray && dataObj->GetTraits().IsInstanceTraits())
                        {
                            Instances::fl_utils::ByteArray* arrData = static_cast<Instances::fl_utils::ByteArray*>(dataObj.GetPtr());
                            arrData->toString(dataFields);
                        }
                        else
                        {
                            SF_ASSERT(false);
                        }
                    }
                    else if (data.IsString())
                    {
                        dataFields = data.AsString();
                    }

                    Array<String> headers;
                    SPtr<Instances::fl::Array> reqHeaders = entry.mURLRequest->requestHeadersGet();
                    if (reqHeaders)
                    {
                        for (UPInt i = 0; i < reqHeaders->GetSize(); ++i)
                        {

                            Instances::fl_net::URLRequestHeader* header = static_cast<Instances::fl_net::URLRequestHeader*>(reqHeaders->At(i).GetObject());
                            String headerValue(header->name.ToCStr());
                            headerValue += ": ";
                            headerValue += header->value.ToCStr();
                            headers.PushBack(headerValue);
                        }
                    }

                    if (URLBuilder::SendURLRequest(&bytes, fileName, sendUrlRequest, dataFields.ToCStr(), (int)dataFields.GetLength(), &headers, entry.mURLRequest->contentTypeGet().ToCStr())
                        && !bytes.IsEmpty())
                    {
                        pfile = *SF_NEW MemoryFile(fileName, bytes.GetDataPtr(), (int)bytes.GetSize());
                    }
                }
#endif
            }
            else
            {
                pfile = *pls->OpenFile(fileName.ToCStr());
            }
            if (pfile)
            {
                if (entry.mURLLoader->IsLoadingVariables())
                    MovieImpl::ReadTextData(&data, pfile, &fileLen, true);
                else if (entry.mURLLoader->IsLoadingText())
                    MovieImpl::ReadTextData(&data, pfile, &fileLen, false);
                else if (entry.mURLLoader->IsLoadingBinary())
                    MovieImpl::ReadBinaryData(&binaryData, pfile, &fileLen);
            }
            else
            {
                char buf[1024];
                SFsprintf(buf, sizeof(buf), "Can't open %s", fileName.ToCStr());
                entry.mURLLoader->ExecuteErrorEvent(buf);
                error = true;
            }
        }
        if (!error)
        {
            if (entry.mURLLoader->IsLoadingVariables())
            {
                entry.mURLLoader->SetVariablesDataString(data.ToCStr());
            }
            else if (entry.mURLLoader->IsLoadingText())
            {
                entry.mURLLoader->SetTextString(data.ToCStr());
            }
            else if (entry.mURLLoader->IsLoadingBinary())
            {
                entry.mURLLoader->SetBinaryData(binaryData);
            }
            else {} //@TODO other dataTypes
            entry.mURLLoader->ExecuteOpenEvent();
            entry.mURLLoader->ExecuteProgressEvent(fileLen, fileLen);
            entry.mURLLoader->ExecuteCompleteEvent();
        }
    }
    else
    {
        // loading a movie/bitmap using Loader
        String              url(pentry->URL), urlStrGfx;
        bool                stripped = false;
        LogState*           plog = pls->GetLogState();
        Ptr<MovieDefImpl>   pmovieDef;
        unsigned            loadFlags = GetMovieDefImpl()->GetLoadFlags();
        // int                 filelength = 0;

        if (GetMainContainer())
        {
            stripped = ((GetMainContainer()->GetResourceMovieDef()->GetSWFFlags()
                & MovieInfo::SWF_Stripped) != 0);
        }

        // If the file is stripped, its loadMovie may be stripped too,
        // so try to load '.gfx' file first.
        if (stripped)
            urlStrGfx = GetUrlStrGfx(url);

        // Check to see if URL is a user image substitute.
        bool    bilinearImage     = 0;
        bool    userImageProtocol = LoaderImpl::IsProtocolImage(url, &bilinearImage);

        if (userImageProtocol)
        {
            ImageCreator* pimageLoader = pls->GetImageCreator();
            // Create image through image callback
            if (pimageLoader)
            {
                ImageCreateInfo icinfo(ImageCreateInfo::Create_Protocol, GetMovieHeap());
                icinfo.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());
                icinfo.pMovie = GetMovieImpl();
                Ptr<Image> pimg = *pimageLoader->LoadProtocolImage(icinfo, url.ToCStr());
                if (pimg)
                {
                    Ptr<ImageResource> pimageRes = 
                        *SF_HEAP_NEW(GetMovieHeap()) ImageResource(pimg, Resource::Use_Bitmap);
                    if (pimageRes)
                    {
                        pmovieDef = *pMovieImpl->CreateImageMovieDef(pimageRes, bilinearImage,
                            url.ToCStr(), pls);
                    }
                }
                else
                {
                    if (plog)
                    {
                        plog->LogScriptWarning(
                            "ImageCreator::LoadProtocolImage failed to load image \"%s\"", url.ToCStr());
                    }
                    entry.mLoader->ExecuteErrorEvent(url.ToCStr());
                    goto error_exit;
                }
            }
            else
            {
                if (plog)
                {
                    plog->LogScriptWarning(
                        "ImageCreator is not installed, failed to load image \"%s\"", url.ToCStr());
                }
                entry.mLoader->ExecuteErrorEvent(url.ToCStr());
            }
        }
        else
        {
            unsigned lf = loadFlags | Loader::LoadImageFiles | Loader::LoadWaitCompletion;
            if (pentry->QuietOpen)
                lf |= Loader::LoadQuietOpen;

            if (!entry.mBytes.GetPtr())
            {
                // Load the movie, first trying the .gfx file if necessary.
                // Our file loading can automatically detect and load images as well,
                // so we take advantage of that by passing the Loader::LoadImageFiles flag.

                // TBD: We will need to do something with delayed loading here in the future;
                // i.e. if the CreateMovie is executing in a different thread it will need to
                // post messages when it is done so that the handlers can be called.

                String rootPath;
                GetRootFilePath(&rootPath);

                if (urlStrGfx.GetLength() > 0)
                {
                    URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                        urlStrGfx, rootPath);

                    pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
                }
                if (!pmovieDef)
                {
                    URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                        url, rootPath);

                    pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
                }
            }
            else
            {
                // loading from bytes array
                pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, *entry.mBytes.GetPtr(), lf);
            }

            if (pmovieDef)
            {
                if (pmovieDef->GetVersion() != ~0u && (pmovieDef->GetVersion() < 9 || pmovieDef->GetDataDef()->GetASVersion() < 3))
                {
                    if (plog && !pentry->QuietOpen)
                    {
                        plog->LogScriptWarning("Failed loading SWF \"%s\": ActionScript version mismatch", url.ToCStr());
                    }
                    entry.mLoader->ExecuteErrorEvent(url.ToCStr());
                    if (entry.NotifyLoadInitCInterface)
                        entry.NotifyLoadInitCInterface->ErrorEventCallback();
                    pmovieDef = NULL;
                    goto error_exit;
                }
                else
                {
                    // Record file length for progress report below. Once we do
                    // threaded loading this will change.
                    // filelength = pmovieDef->GetFileBytes();
                }
            }
            else 
            {
                if (plog && !pentry->QuietOpen)
                {
                    plog->LogScriptWarning("Failed loading URL \"%s\"", url.ToCStr());
                }
                entry.mLoader->ExecuteErrorEvent(url.ToCStr());
				if (entry.NotifyLoadInitCInterface)
					entry.NotifyLoadInitCInterface->ErrorEventCallback();
                goto error_exit;
            }
        }

        if (pmovieDef)
            AddLoadedMovieDef(pmovieDef);

        entry.mLoader->ExecuteOpenEvent();

		if (entry.NotifyLoadInitCInterface)
			entry.NotifyLoadInitCInterface->OpenEventCallback();

        DisplayObjContainer* pparent = entry.mLoader->GetDisplayObjContainer();
        ResourceId newCharId = ResourceId(); // Assign invalid id.
        Ptr<DisplayObject> pnewChar;
        if (pmovieDef && pmovieDef->GetDataDef()->MovieType == MovieDataDef::MT_Image)
        {
            // bitmap
            CharacterCreateInfo ccinfo(pmovieDef->GetDataDef(), pmovieDef);
            Ptr<AS3::AvmBitmap> pnewBmpChar = *static_cast<AS3::AvmBitmap*>(GetASSupport()->CreateCharacterInstance(
                GetMovieImpl(), ccinfo, NULL, newCharId, CharacterDef::Bitmap));

            ResourceHandle rh;
            if (pmovieDef->GetDataDef()->GetResourceHandle(&rh, ResourceId(CharacterDef::CharId_ImageMovieDef_ImageResource)))
            {
                Resource* pres = rh.GetResource(&pmovieDef->GetResourceBinding());
                if (pres)
                {
                    if (pres->GetResourceType() == Resource::RT_Image)
                    {
                        pnewBmpChar->SetImage(static_cast<ImageResource*>(pres));
                    }
                }
            }

            AvmDisplayObj* avmObj = ToAvmDisplayObj(pnewBmpChar);
            if (!avmObj->HasAS3Obj())
            {
                if (avmObj->CreateASInstanceNoCtor())
                {
                    avmObj->GetAS3Obj()->SetLoaderInfo(*entry.mLoader);
                    avmObj->CallCtor(true);
                }
            }
            ToAvmDisplayObjContainer(pparent)->AddChild(pnewBmpChar);

            pnewChar = pnewBmpChar;
        }
        else
        {
            // movie
            Ptr<Sprite> pnewSprite;
            newCharId = ResourceId(0); // root element always has 0 res id.
            if (pmovieDef)
            {
                // Trying to cleanup all remaining objects (incl VMAbcFile) before
                // creating new ones. Might help to avoid classes intersections across
                // SWF files.
                if (GetAVM())
                    GetAVM()->GetGC().ForceCollect(pMovieImpl->AdvanceStats);

                entry.mLoader->ExecuteProgressEvent(pmovieDef->GetFileBytes(), pmovieDef->GetFileBytes());
                // CharId of loaded clip is the same as the old one
                // Also mark clip as loadedSeparately, so that _lockroot works.
                CharacterCreateInfo ccinfo(pmovieDef->GetDataDef(), pmovieDef);
                pnewSprite = *static_cast<Sprite*>(GetASSupport()->CreateCharacterInstance(
                    GetMovieImpl(), ccinfo, NULL, newCharId, CharacterDef::Sprite));
                pnewSprite->SetLoadedSeparately();
                pnewSprite->SetTimelineObjectFlag(true);
                pnewSprite->OnEventLoad();

                pnewSprite->AddToPlayList();
                // Run actions on Frame 0. This should be done before calling OnLoadInit

                AvmSprite* avmSpr = ToAvmSprite(pnewSprite);
                if (entry.mLoader)
                {
                    Instances::fl_display::LoaderInfo* info = entry.mLoader->GetContentLoaderInfo();
                    if (info != NULL)
                    {
                        SPtr<Instances::fl_system::ApplicationDomain> appDomain = info->applicationDomainGet();
                        if (appDomain)
                        {
                            avmSpr->SetAppDomain(appDomain->GetAppDomain());
                        }
                    }
                }
                //!AB: need to run just InitActions here, it will initialize Abc by
                // executing DoAbc tag. All further actions will be executed by 
                // CreateASInstance, from "super" call.
                avmSpr->ExecuteInitActionFrameTags(0);

                // Allocate a default PerspectiveDataType object and set fieldOfView.
                pnewSprite->SetFOV(DEFAULT_FLASH_FOV);   

                if (!avmSpr->HasAS3Obj())
                {
                    if (avmSpr->CreateASInstanceNoCtor())
                    {
                        avmSpr->GetAS3Obj()->SetLoaderInfo(*entry.mLoader);
                        avmSpr->CallCtor(true);
                    }
                }
                // And attach a new character in place of an old one.
                //AB: we need to replicate the same CreateFrame, Depth and the Name.
                // Along with replicated Id this is important to maintain the loaded
                // movie on the stage. Otherwise, once the parent clip reaches the end
                // of its timeline the original movieclip will be recreated. Flash
                // doesn't recreate the original movie clip in this case, and it also
                // doesn't stop timeline animation for the target movieclip. The only way
                // to achieve both of these features is to copy Id, CreateFrame, Depth and
                // Name from the old character to the new one.
                // See last_to_first_frame_mc_no_reloadX.fla and 
                // test_timeline_anim_after_loadmovie.swf
                //pnewSprite->SetCreateFrame(poldChar->GetCreateFrame());
                //pnewSprite->SetDepth(poldChar->GetDepth());
                //if (!poldChar->HasInstanceBasedName())
                //    pnewSprite->SetName(poldChar->GetName());

                ToAvmDisplayObjContainer(pparent)->AddChild(pnewSprite);
                //pparent->ReplaceChildCharacter(poldChar, pnewSprite);
                // In case permanent variables were assigned.. check them.
                ResolveStickyVariables(pnewSprite);

                // need to update the optimized play list
                pnewSprite->ModifyOptimizedPlayListLocal<Sprite>();
            }
	
            bool charIsLoadedSuccessfully = (pnewSprite.GetPtr() != NULL);
            if (charIsLoadedSuccessfully)
            {
                DoActions();
            } 
            pnewChar = pnewSprite;
        }
        if (!userImageProtocol)
        {
            entry.mLoader->QueueInitEvent(pnewChar, entry.NotifyLoadInitCInterface);
            entry.mLoader->QueueCompleteEvent();
        }
        else
        {
            // If userImageProtocol is used when we can genereate events right away,
            // synchronously
            entry.mLoader->ExecuteInitEvent(pnewChar);
            if (entry.NotifyLoadInitCInterface)
                entry.NotifyLoadInitCInterface->InitEventCallback();
            entry.mLoader->ExecuteCompleteEvent();
        }
    }
error_exit:;
}

void MovieRoot::ProcessLoadVarsMT
(GFx::LoadQueueEntry *pentry, LoadStates* , const String& data, UPInt fileLen, bool succeeded)
{
    LoadQueueEntry& entry = static_cast<LoadQueueEntry&>(*pentry);
    if (!succeeded)
    {
        char buf[1024];
        SFsprintf(buf, sizeof(buf), "Can't open %s", entry.mURLRequest->GetUrl().ToCStr());
        entry.mURLLoader->ExecuteErrorEvent(buf);
    }
    else
    {
        if (entry.mURLLoader->IsLoadingVariables())
        {
            String decodedData;
            ASUtils::Unescape(data.ToCStr(), data.GetSize(), &decodedData);
            entry.mURLLoader->SetVariablesDataString(decodedData.ToCStr());
        }
        else if (entry.mURLLoader->IsLoadingText())
        {
            entry.mURLLoader->SetTextString(data.ToCStr());
        }
        else if (entry.mURLLoader->IsLoadingBinary())
        {
            SF_ASSERT(0); // shouldn't end up here; must be at ProcessLoadBinaryMT
        }
        else {} //@TODO other dataTypes
        entry.mURLLoader->ExecuteOpenEvent();
        entry.mURLLoader->ExecuteProgressEvent(static_cast<UInt32>(fileLen), static_cast<UInt32>(fileLen));
        entry.mURLLoader->ExecuteCompleteEvent();
    }
}

void MovieRoot::ProcessLoadBinaryMT
(GFx::LoadQueueEntry *pentry, LoadStates*, const ArrayPOD<UByte>& data, UPInt fileLen, bool succeeded) 
{ 
    LoadQueueEntry& entry = static_cast<LoadQueueEntry&>(*pentry);
    if (!succeeded)
    {
        char buf[1024];
        SFsprintf(buf, sizeof(buf), "Can't open %s", entry.mURLRequest->GetUrl().ToCStr());
        entry.mURLLoader->ExecuteErrorEvent(buf);
    }
    else
    {
        SF_ASSERT(entry.mURLLoader->IsLoadingBinary());
        
        entry.mURLLoader->SetBinaryData(data);
        
        entry.mURLLoader->ExecuteOpenEvent();
        entry.mURLLoader->ExecuteProgressEvent(static_cast<UInt32>(fileLen), static_cast<UInt32>(fileLen));
        entry.mURLLoader->ExecuteCompleteEvent();
    }
}

bool MovieRoot::CancelMovieLoading(Instances::fl_display::Loader* ploader)
{
    bool rv = false;

    // if we unloading a movieclip we need to go over the list of loading queue which 
    // and cancel all items which are going to load into the same level or target
    // Note, we need to cancel ONLY entries, inserted BEFORE the "unloadMovie" entry!
    LoadQueueEntryMT *plentry = pMovieImpl->pLoadQueueMTHead;
    while(plentry)
    {
        LoadQueueEntry* plqentry = static_cast<LoadQueueEntry*>(plentry->GetQueueEntry());
        if (plqentry->mLoader == ploader)
        {
            plqentry->Canceled = true;
            rv = true;
        }

        plentry = plentry->GetNext();
    }

    // non-progressive loading queue
    LoadQueueEntry *plqentry = static_cast<LoadQueueEntry*>(pMovieImpl->pLoadQueueHead);
    while(plqentry)
    {
        if (plqentry->mLoader == ploader)
        {
            plqentry->Canceled = true;
            rv = true;
        }

        plqentry = static_cast<LoadQueueEntry*>(plqentry->GetNext());
    }

    return rv;
}

// if "stop" parameter is true then the method does the following:
// Tries to unload a child SWF and stops the execution from loaded SWFs. 
// Tries to remove references to EventDispatcher, NetConnection, Timer, Sound, 
// or Video objects of the child SWF file. The following occurs for the child SWF 
// file and the child SWF file's display list:
//    Sounds are stopped. (not done yet)
//    Stage event listeners are removed.
//    Event listeners for enterFrame, frameConstructed, exitFrame, activate and deactivate are removed.
//    Timers are stopped.
//    Camera and Microphone instances are detached (n/a)
//    Movie clips are stopped.
// "gc" - forces garbage collector to be executed after unloading.
void MovieRoot::UnloadMovie(Instances::fl_display::Loader* ploader, bool stop, bool gc)
{
    SF_UNUSED3(ploader, stop, gc);
#ifdef ENABLE_FILE_UNLOADING
    // get the content loader info
    SPtr<Instances::fl_display::LoaderInfo> loaderInfo = ploader->GetContentLoaderInfo();

    SPtr<DisplayObject> contentDisplayObj;

    if (loaderInfo)
    {
        // Save the content display object
        contentDisplayObj = loaderInfo->GetContentDispObj();

        // fire unload event first
        SPtr<Instances::fl_events::Event> evt = loaderInfo->CreateEventObject
            (GetStringManager()->CreateConstString("unload"), false, false);
        loaderInfo->Dispatch(evt.GetPtr(), loaderInfo->GetContentDispObj());

        // remove app domain if nothing else is in it
        loaderInfo->VerifyAppDomain();
    }

    // release a ptr to the loaded content
    ploader->ResetContent();

    CancelMovieLoading(ploader);

    // just remove all children from the loader object.
    GFx::DisplayObjContainer* doc = ploader->GetDisplayObjContainer();

    if (!contentDisplayObj && doc && doc->GetNumChildren() > 0)
    {
        contentDisplayObj = doc->GetChildAt(0)->CharToScriptableObject_Unsafe();
    }
    if (contentDisplayObj)
    {
        contentDisplayObj->OnEventUnload();
        contentDisplayObj->ForceShutdown();

        Ptr<MovieDefImpl> defimpl = contentDisplayObj->GetResourceMovieDef();

        // try to remove event listeners from the swf being unloaded
        if (defimpl)
        {
            bool removed = RemoveLoadedMovieDef(defimpl);
            if (stop)
            {
                // We need to remove event listeners, shutdown timers and unregister fonts
                // ONLY if this is the last loaded SWF with the 'defimpl'.
                if (removed)
                {
                    // first remove all enterFrame, activate, deactivate, etc
                    EventChains::ChainHash::Iterator iter = mEventChains.Chains.Begin();
                    for(; iter != mEventChains.Chains.End(); ++iter)
                    {
                        EventChains::Chain* chain = iter->Second.GetPtr(); 
                        if (chain)
                        {
                            for (UPInt i = 0; i < chain->GetSize(); )
                            {
                                Ptr<DisplayObject> dob = (*chain)[i];
                                if (dob)
                                {
                                    if (dob->GetResourceMovieDef() == defimpl)
                                    {
                                        chain->RemoveAt(i);
                                        continue;
                                    }
                                }
                                ++i;
                            }
                        }
                    }
                    // Remove stage listeners
                    Instances::fl_display::DisplayObject* as3obj = ToAvmDisplayObjContainer(GetStage())->GetAS3Obj();
                    if (as3obj != NULL)
                    {
                        as3obj->RemoveListenersForMovieDef(defimpl);
                    }

                    // Remove timers
                    GetMovieImpl()->ShutdownTimersForMovieDef(defimpl);

                    // Remove registered fonts
                    GetMovieImpl()->UnregisterFonts(defimpl);

                    // Release all AS sound objects
#ifdef GFX_ENABLE_SOUND
                    Sprite *psprite = GetMainTimeline()->CharToSprite();
                    if (psprite)
                        psprite->ReleaseAllSounds(defimpl);
#endif
                    if (MemContext->TextAllocator)
                    {
                        struct TextFormatVisitor : public Text::Allocator::TextFormatVisitor
                        {
                            MovieDefImpl* pDefImpl;
                            TextFormatVisitor(MovieDefImpl* md):pDefImpl(md) {}

                            virtual bool Visit(const Text::TextFormat& tf)
                            {
                                GFx::FontHandle* fh = static_cast<GFx::FontHandle*>(tf.GetFontHandle());
                                if (fh && fh->pSourceMovieDef == pDefImpl)
                                    return false; // remove
                                return true; // keep
                            }
                        } vis(defimpl);
                        MemContext->TextAllocator->VisitTextFormatCache(vis);
                    }

                    // Go through the cached display object in MouseState array and release the ones,
                    // which belong to the Abc being unloaded.
                    for (unsigned i = 0; i < GFX_MAX_MICE_SUPPORTED; ++i)
                    {
                        // check each element of rollover stack
                        for (UPInt j = 0; j < mMouseState[i].RolloverStack.GetSize();)
                        {
                            Ptr<InteractiveObject> obj = mMouseState[i].RolloverStack[j];
                            if (obj && ToAvmDisplayObj(obj)->DoesAbcBelongTo(defimpl))
                            {
                                mMouseState[i].RolloverStack.RemoveAt(j);
                            }
                            else
                                ++j;
                        }

                        // check for the last-mouse-over object
                        if (mMouseState[i].LastMouseOverObj)
                        {
                            if (ToAvmDisplayObj(mMouseState[i].LastMouseOverObj)->DoesAbcBelongTo(defimpl))
                                mMouseState[i].LastMouseOverObj = 0;
                        }
                    }
                }
            }
        }

        ToAvmDisplayObj(contentDisplayObj)->OnDetachFromTimeline();
        if (doc)
            doc->ClearDisplayList();
    }

    if (gc)
        GetAVM()->GetGC().ScheduleCollect(Movie::GCF_Full); //? should it be full collect here?
#endif
}

void MovieRoot::OnMovieDefRelease(MovieDefImpl* defImpl)
{
    LoadedMovieDefs.Remove(defImpl); 
}

void MovieRoot::AddLoadedMovieDef(MovieDefImpl* defImpl)
{
    SF_UNUSED(defImpl);
    SF_ASSERT(defImpl);
    defImpl->AddReleaseNotifier(this);
    LoadedMovieDefInfo* info = LoadedMovieDefs.Get(defImpl);
    if (info)
    {
        //SF_ASSERT(info->pMovieDef == defImpl);
        ++info->UseCnt;
    }
    else
    {
        LoadedMovieDefs.Add(defImpl, LoadedMovieDefInfo());
    }
}

bool MovieRoot::RemoveLoadedMovieDef(MovieDefImpl* defImpl)
{
    SF_ASSERT(defImpl);
    LoadedMovieDefInfo* info = LoadedMovieDefs.Get(defImpl);
    if (info)
    {
        //SF_ASSERT(info->pMovieDef == defImpl);
        if (--info->UseCnt == 0)
        {
            LoadedMovieDefs.Remove(defImpl); 
            defImpl->RemoveReleaseNotifier(this);
            return true;
        }
    }
    return false;
}

DisplayObjContainer* MovieRoot::GetRootMovie(DisplayObject* dobj) const
{
    if (!dobj)
        return GetStage()->GetRoot();
    else
    {
        DisplayObject* r = ToAvmDisplayObj(dobj)->GetRoot();
        if (!r)
            return GetStage()->GetRoot();
        else
        {
            if (r->IsDisplayObjContainer())
                return r->CharToDisplayObjContainer_Unsafe();
            else
                return GetRootMovie(r->GetParent());
        }
    }
}

#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
void MovieRoot::OnDeviceOrientationChanged(const OrientationEvent& e)
{
    Stage* stage = GetStage();
    if (stage)
    {
        // newOrientation is "default", "rotatedLeft", "rotatedRight", "unknown" or "upsideDown"
        const char* no;
        switch(e.Orientation)
        {
        case OrientationEvent::Default:         no = "default"; break;
        case OrientationEvent::RotatedLeft:     no = "rotatedLeft"; break;
        case OrientationEvent::RotatedRight:    no = "rotatedRight"; break;
        case OrientationEvent::UpsideDown:      no = "upsideDown"; break;
        default: no = "unknown";
        }
        stage->OnDeviceOrientationChange(no);
    }
}

void        MovieRoot::OnAppLifecycleEvent(const AppLifecycleEvent& e)
{
    Stage* stage = GetStage();
    if (stage)
    {
        const char* es;
        switch(e.Status)
        {
        case AppLifecycleEvent::OnPause:        es = "onPause"; break;
        case AppLifecycleEvent::OnResume:       es = "onResume"; break;
        default: es = "unknown";
        }
        stage->OnAppLifecycleEvent(es);
    }
}
#endif

bool MovieRoot::FindLibrarySymbolInAllABCs(const Instances::fl_display::DisplayObject* dobj, 
                                           CharacterCreateInfo* pccinfo)
{
    GFx::MovieDefRootNode *pdefNode = GetMovieImpl()->RootMovieDefNodes.GetFirst();
    while(!GetMovieImpl()->RootMovieDefNodes.IsNull(pdefNode))
    {   
        if (dobj->FindLibarySymbol(pccinfo, pdefNode->pDefImpl))
            return true;

        pdefNode = pdefNode->pNext;
    }
    return false;
}

void MovieRoot::SetMouseCursorType(const ASString& cursor, UInt32 mouseIndex)
{
    unsigned cursorType = ~0u;
    if (cursor == "arrow")
        cursorType = MouseCursorEvent::ARROW;
    else if (cursor == "button")
        cursorType = MouseCursorEvent::BUTTON;
    else if (cursor == "hand")
        cursorType = MouseCursorEvent::HAND;
    else if (cursor == "ibeam")
        cursorType = MouseCursorEvent::IBEAM;
    GFx::MouseState* ms = GetMovieImpl()->GetMouseState(mouseIndex);
    SF_ASSERT(ms);
    ChangeMouseCursorType(mouseIndex, cursorType);
    ms->PresetCursorType(cursorType);
}

void MovieRoot::GetMouseCursorTypeString(ASString& result, unsigned cursorType) const
{
    switch(cursorType)
    {
    case MouseCursorEvent::ARROW:  result = GetStringManager()->CreateConstString("arrow"); break;
    case MouseCursorEvent::BUTTON: result = GetStringManager()->CreateConstString("button"); break;
    case MouseCursorEvent::HAND:   result = GetStringManager()->CreateConstString("hand"); break;
    case MouseCursorEvent::IBEAM:  result = GetStringManager()->CreateConstString("ibeam"); break;
    default: result = GetStringManager()->CreateConstString("auto"); 
    }
}

void MovieRoot::GetMouseCursorType(ASString& result, UInt32 mouseIndex)
{
    GFx::MouseState* ms = GetMovieImpl()->GetMouseState(mouseIndex);
    SF_ASSERT(ms);
    GetMouseCursorTypeString(result, ms->GetPresetCursorType());
}

AmpMovieObjectDesc* MovieRoot::GetDisplayObjectsTree(MemoryHeap* heap) const
{
    AmpMovieObjectDesc* objectDesc = NULL;
    DisplayObjContainer* dispObject = GetMainContainer();
    if (dispObject != NULL)
    {
        objectDesc = SF_HEAP_NEW(heap) AmpMovieObjectDesc();
        objectDesc->Description = dispObject->GetName().ToCStr();
        dispObject->GetChildDescTree(objectDesc, heap);        
    }
    return objectDesc;
}

/*
OnExceedLimit and OnFreeSegment are used to control MovieView's memory heap growth.
OnExceedLimit is called then the pre-set heap limit it exceeded.
OnFreeSegment is called then a chunk of memory is freed (not less than heap granularity).

User can set two parameters: UserLevelLimit (in bytes) and HeapLimitMultiplier (0..1].

The heap has initial pre-set limit 128K (so called "dynamic" limit). When this limit
is exceeded then OnExceedLimit is called. In this case it is necessary to determine:
either to collect to free up space, or to expand the heap.  The heuristic used to make the 
decision to collect or expand is taken from the Boehm-Demers-Weiser (BDW) garbage 
collector and memory allocator.
The BDW algorithm is (pseudo-code):

if (allocs since collect >= heap footprint * HeapLimitMultiplier)
collect
else
expand(heap footprint + overlimit + heap footprint * HeapLimitMultiplier)

The default value for HeapLimitMultiplier is 0.25.
Thus, it will collect only if allocated memory since the last collection is more
than 25% (default value of HeapLimitMultiplier) of the current heap size. Otherwise,
it will expand the limit up to requested size plus 25% of the heap size.

If collection occurred, it flushes all possible caches (text/paragraph formats, for example) and 
invoke garbage collector for ActionScript.

If user has specified UserLevelLimit then the algorithm above works the same way
up to that limit. If heap limit exceeds the UserLevelLimit then collection will be
invoked regardless to number of allocations since last collect. The dynamic heap limit
will be set to heap's footprint after collection + delta required for the requested allocation.

OnFreeSegment call reduces the limit by the freeing size.
*/
bool MemoryContextImpl::HeapLimit::OnExceedLimit(MemoryHeap* heap, UPInt overLimit)
{
    UPInt footprint = heap->GetFootprint();
    UPInt heapLimit = heap->GetLimit();
#ifdef SF_TRACE_COLLECTIONS        
    printf("\n! Limit is exceeded. Used: %u, footprint: %u, lim: %u, over limit: %u\n", 
        (unsigned)heap->GetUsedSpace(), (unsigned)footprint, (unsigned)heapLimit, (unsigned)overLimit);
#endif

    SPInt allocsSinceLastCollect = (SPInt)footprint - LastCollectionFootprint;
    UPInt newLimit = (heapLimit + overLimit) + (UPInt)(footprint * HeapLimitMultiplier);

    if (allocsSinceLastCollect >= (SPInt)(footprint * HeapLimitMultiplier) ||
        (UserLevelLimit != 0 && newLimit > UserLevelLimit))
    {
        if (IsInsideAlloc())
        {
            // if called from realloc then just schedule collection. Otherwise,
            // it possible that collection modifies/frees the memory block currently 
            // being reallocated.
            MemContext->ASGC->ScheduleCollect(ASRefCountCollector::Collect_Emergency);

            LastCollectionFootprint = footprint;
        }
        else
        {
            Collect(heap);

            if (UserLevelLimit != 0 && newLimit > UserLevelLimit)
            {
                // check, if user limit is specified. If so, and if it is exceeded
                // then increase the limit just for absolutely required delta to minimize
                // the heap growth.
                SF_ASSERT(LastCollectionFootprint <= footprint);
                if (overLimit > (footprint - LastCollectionFootprint))
                {
                    CurrentLimit = heapLimit + (overLimit - (footprint - LastCollectionFootprint));
                    heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                    printf("-        UserLimit exceeded. increasing limit up to: %u (%u)\n", 
                        (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
                }
                else
                {
                    // even though limit is not changed - set it to heap again to make sure
                    // the acutual heap's limit is set correctly.
                    heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                    printf("-        no limit increase is necessary. Current limit is %u (%u)\n", 
                        (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
                }
            }
        }
    }
    else
    {
        heap->SetLimit(newLimit);

#ifdef SF_TRACE_COLLECTIONS        
        printf("-    increasing limit up to: %u (%u)\n", (unsigned)newLimit, (unsigned)heap->GetLimit());
#endif

        CurrentLimit = heap->GetLimit(); // take an actual value of the limit
    }

    return true;
}
void MemoryContextImpl::HeapLimit::OnFreeSegment(MemoryHeap* heap, UPInt freeingSize)
{
    UPInt oldLimit = CurrentLimit;
    if (oldLimit > UserLevelLimit)
    {
        if (oldLimit > freeingSize)
        {
            CurrentLimit = oldLimit - freeingSize;
            heap->SetLimit(CurrentLimit);
#ifdef SF_TRACE_COLLECTIONS        
            printf("!!   reducing limit from %u to %u (%u)\n", (unsigned)oldLimit, (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif
        }
    }
}
void MemoryContextImpl::HeapLimit::Collect(MemoryHeap* heap)
{
    // The collection part.
    // We may release cached text format objects from Text::Allocator...
    if (MemContext->TextAllocator)
    {
        MemContext->TextAllocator->FlushTextFormatCache(true);
        MemContext->TextAllocator->FlushParagraphFormatCache(true);
    }
#ifdef GFX_AS_ENABLE_GC
#ifdef SF_AMP_SERVER
    AmpServer& ampServer = AmpServer::GetInstance();
    MovieImpl* movie;
    // If garbage collection is used - force collect. Use ForceEmergencyCollect - it
    // guarantees that no new allocations will be made during the collection.
    if (ampServer.FindMovieByHeap(heap, &movie))
    {
        MemContext->ASGC->ForceEmergencyCollect(movie->AdvanceStats);
        movie->Release(); // addref=ed by FindMovieByHeap above
    }
    else
        MemContext->ASGC->ForceEmergencyCollect(NULL);
#else
    MemContext->ASGC->ForceEmergencyCollect(NULL);
#endif // SF_AMP_SERVER
#endif // SF_NO_GC
    LastCollectionFootprint = heap->GetFootprint(); 

#ifdef SF_TRACE_COLLECTIONS        
    printf("+    footprint after collection: %u, used mem: %u\n", 
        (unsigned)heap->GetFootprint(), (unsigned)heap->GetUsedSpace());
#endif
}
void MemoryContextImpl::HeapLimit::Reset(MemoryHeap* heap)
{
    Collect(heap);
    heap->SetLimit(INITIAL_DYNAMIC_LIMIT);   // reset to initial 128 
    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
}

}}} // SF::GFx::AS3


