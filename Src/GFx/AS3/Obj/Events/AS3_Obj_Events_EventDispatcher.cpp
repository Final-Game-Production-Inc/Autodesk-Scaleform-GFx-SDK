//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_EventDispatcher.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_EventDispatcher.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_VTable.h" // Because of RemoveListenersForMovieDef()
#include "AS3_Obj_Events_Event.h"
#include "AS3_Obj_Events_MouseEvent.h"
#include "AS3_Obj_Events_TouchEvent.h"
#include "AS3_Obj_Events_TransformGestureEvent.h"
#include "AS3_Obj_Events_PressAndTapGestureEvent.h"
#include "AS3_Obj_Events_KeyboardEvent.h"
#include "Kernel/SF_HeapNew.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "AS3_Obj_Events_ProgressEvent.h"
#ifdef SF_AS3_VERSION_AIR
#include "AS3_Obj_Events_OutputProgressEvent.h"
#endif
#include "AS3_Obj_Events_FocusEvent.h"
#include "AS3_Obj_Events_IOErrorEvent.h"
#include "../Events/AS3_Obj_Events_NetStatusEvent.h"
#include "../GFx/AS3_Obj_Gfx_MouseEventEx.h"
#include "../GFx/AS3_Obj_Gfx_KeyboardEventEx.h"
#include "../GFx/AS3_Obj_Gfx_FocusEventEx.h"
#include "../GFx/AS3_Obj_Gfx_GamePadAnalogEvent.h"
#include "../GFx/AS3_Obj_Gfx_MouseCursorEvent.h"
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
#include "AS3_Obj_Events_AppLifecycleEvent.h"
#include "AS3_Obj_Events_StageOrientationEvent.h"
#endif
#include "AS3_Obj_Events_StatusEvent.h"
#include "AS3_Obj_Events_AccelerometerEvent.h"
#include "AS3_Obj_Events_GeolocationEvent.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc5<Instances::fl_events::EventDispatcher, Instances::fl_events::EventDispatcher::mid_addEventListener, const Value, const ASString&, const Value&, bool, SInt32, bool> TFunc_Instances_EventDispatcher_addEventListener;
typedef ThunkFunc1<Instances::fl_events::EventDispatcher, Instances::fl_events::EventDispatcher::mid_dispatchEvent, bool, Instances::fl_events::Event*> TFunc_Instances_EventDispatcher_dispatchEvent;
typedef ThunkFunc1<Instances::fl_events::EventDispatcher, Instances::fl_events::EventDispatcher::mid_hasEventListener, bool, const ASString&> TFunc_Instances_EventDispatcher_hasEventListener;
typedef ThunkFunc3<Instances::fl_events::EventDispatcher, Instances::fl_events::EventDispatcher::mid_removeEventListener, const Value, const ASString&, const Value&, bool> TFunc_Instances_EventDispatcher_removeEventListener;
typedef ThunkFunc1<Instances::fl_events::EventDispatcher, Instances::fl_events::EventDispatcher::mid_willTrigger, bool, const ASString&> TFunc_Instances_EventDispatcher_willTrigger;

template <> const TFunc_Instances_EventDispatcher_addEventListener::TMethod TFunc_Instances_EventDispatcher_addEventListener::Method = &Instances::fl_events::EventDispatcher::addEventListener;
template <> const TFunc_Instances_EventDispatcher_dispatchEvent::TMethod TFunc_Instances_EventDispatcher_dispatchEvent::Method = &Instances::fl_events::EventDispatcher::dispatchEvent;
template <> const TFunc_Instances_EventDispatcher_hasEventListener::TMethod TFunc_Instances_EventDispatcher_hasEventListener::Method = &Instances::fl_events::EventDispatcher::hasEventListener;
template <> const TFunc_Instances_EventDispatcher_removeEventListener::TMethod TFunc_Instances_EventDispatcher_removeEventListener::Method = &Instances::fl_events::EventDispatcher::removeEventListener;
template <> const TFunc_Instances_EventDispatcher_willTrigger::TMethod TFunc_Instances_EventDispatcher_willTrigger::Method = &Instances::fl_events::EventDispatcher::willTrigger;

namespace Instances { namespace fl_events
{
    EventDispatcher::EventDispatcher(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::EventDispatcher::EventDispatcher()$data"
    , VMRef(t.GetVM())
    , pImpl(NULL)
//##protect##"instance::EventDispatcher::EventDispatcher()$data"
    {
//##protect##"instance::EventDispatcher::EventDispatcher()$code"
//##protect##"instance::EventDispatcher::EventDispatcher()$code"
    }

    void EventDispatcher::addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
    {
//##protect##"instance::EventDispatcher::addEventListener()"
        SF_UNUSED6(result, type, listener, useCapture, priority, useWeakReference);
        if (!pImpl)
            pImpl = SF_HEAP_NEW(GetVM().GetMemoryHeap()) EventDispatcherImpl;
        ASStringHash<ListenersArray*>& listeners = (useCapture) ? pImpl->CaptureListeners : pImpl->Listeners;

        ListenersArray** pplistenersArr = listeners.Get(ASString(type));
        ListenersArray* plistenersArr;
        if (!pplistenersArr)
        {
            plistenersArr = SF_HEAP_NEW(GetVM().GetMemoryHeap()) ListenersArray;
            listeners.Add(ASString(type), plistenersArr);
        }
        else
            plistenersArr  = *pplistenersArr;
        SF_ASSERT(plistenersArr);
        
        // search correct place to insert according to priority
        // also, check if we already have this listener installed
        bool installed = false;
        UPInt i = 0;
        for (UPInt n = plistenersArr->GetSize(); i < n; ++i)
        {
            const Value& func = (*plistenersArr)[i].mFunction;

            if (func.IsValidWeakRef())
            {
                if (StrictEqual(func, listener))
                {
                    installed = true;
                    break;
                }
            }

            if (priority > (int)(*plistenersArr)[i].Priority)
                break;
        }
        if (!installed)
        {
            // check remaining listeners for duplication
            for (UPInt j = i, n = plistenersArr->GetSize(); j < n; ++j)
            {
                const Value& func = (*plistenersArr)[j].mFunction;

                if (func.IsValidWeakRef())
                {
                    if (StrictEqual(func, listener))
                    {
                        installed = true;
                        break;
                    }
                }
            }
        }
        if (!installed) // install
        {
            ASVM& vm = static_cast<ASVM&>(GetVM());
            const StringManager& sm = GetVM().GetStringManager();
            ASString evtType(type);

            // check for special events which require special handling in movieroot...
            EventId::IdCode evtId = EventId::Event_End;
            if (evtType == sm.GetBuiltin(AS3Builtin_enterFrame))
            {
                evtId = EventId::Event_EnterFrame;
                vm.GetMovieImpl()->InvalidateOptAdvanceList();
                pImpl->Flags |= EventDispatcherImpl::Flag_EnterFrame;
            }
            else if (evtType == sm.GetBuiltin(AS3Builtin_frameConstructed))
            {
                evtId = EventId::Event_FrameConstructed;
                pImpl->Flags |= EventDispatcherImpl::Flag_FrameConstructed;
            }
            else if (evtType == sm.GetBuiltin(AS3Builtin_exitFrame))
            {
                evtId = EventId::Event_ExitFrame;
                pImpl->Flags |= EventDispatcherImpl::Flag_ExitFrame;
            }
            else if (evtType == sm.GetBuiltin(AS3Builtin_activate))
            {
                evtId = EventId::Event_Activate;
                pImpl->Flags |= EventDispatcherImpl::Flag_Activate;
            }
            else if (evtType == sm.GetBuiltin(AS3Builtin_deactivate))
            {
                evtId = EventId::Event_Deactivate;
                pImpl->Flags |= EventDispatcherImpl::Flag_Deactivate;
            }
            else if (evtType == sm.GetBuiltin(AS3Builtin_render))
            {
                evtId = EventId::Event_Render;
                pImpl->Flags |= EventDispatcherImpl::Flag_Render;
            }
			else if (evtType == sm.GetBuiltin(AS3Builtin_resize))
			{
				evtId = EventId::Event_Resize;
				pImpl->Flags |= EventDispatcherImpl::Flag_Resize;
			}
            if (evtId != EventId::Event_End)
            {
                if (plistenersArr->GetSize() == 0) // first listener?
                {
                    BuiltinTraitsType btt = GetTraitsType();
                    if (btt >= Traits_DisplayObject_Begin && btt <= Traits_DisplayObject_End && GetTraits().IsInstanceTraits())
                    {
                        fl_display::DisplayObject* dobj = static_cast<fl_display::DisplayObject*>(this);
                        SF_ASSERT(dobj->pDispObj);
                        vm.GetMovieRoot()->AddToEventChain(evtId, dobj->pDispObj);
                    }
                }
            }
            else
            {
                // check for button event handlers
                if (sm.GetBuiltin(AS3Builtin_mouseDown) == evtType ||
                    sm.GetBuiltin(AS3Builtin_mouseUp) == evtType ||
                    sm.GetBuiltin(AS3Builtin_click) == evtType ||
                    sm.GetBuiltin(AS3Builtin_doubleClick) == evtType ||
                    sm.GetBuiltin(AS3Builtin_mouseOver) == evtType ||
                    sm.GetBuiltin(AS3Builtin_mouseOut) == evtType ||
                    sm.GetBuiltin(AS3Builtin_rollOver) == evtType ||
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
                    sm.GetBuiltin(AS3Builtin_touchBegin) == evtType ||
                    sm.GetBuiltin(AS3Builtin_touchEnd) == evtType ||
                    sm.GetBuiltin(AS3Builtin_touchMove) == evtType ||
                    sm.GetBuiltin(AS3Builtin_touchTap) == evtType ||
#endif
                    sm.GetBuiltin(AS3Builtin_rollOut) == evtType)
                {
                    if (useCapture)
                    {
                        if (pImpl->CaptureButtonHandlersCnt < 255)
                            ++pImpl->CaptureButtonHandlersCnt;
                    }
                    else
                    {
                        if (pImpl->ButtonHandlersCnt < 255)
                            ++pImpl->ButtonHandlersCnt;
                    }
                }
            }
            plistenersArr->InsertAt(i, Listener(priority, listener));
            if (useWeakReference)
                (*plistenersArr)[i].mFunction.MakeWeakRef();
        }
//##protect##"instance::EventDispatcher::addEventListener()"
    }
    void EventDispatcher::dispatchEvent(bool& result, Instances::fl_events::Event* event)
    {
//##protect##"instance::EventDispatcher::dispatchEvent()"
        SF_UNUSED1(result);

        if (!event)
        {
            GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM()));
            return;
        }

        // If the event is being redispatched, a clone of the event is created automatically. 
        // After an event is dispatched, its target property cannot be changed, so you must 
        // create a new copy of the event for redispatching to work.
        SPtr<Instances::fl_events::Event> newEvent;

        // If event was dispatched already, or, if it is a custom user-defined class - 
        // create a clone.
        if (event->NeedsCloning())
        {
            // first of all, determine if 'event' is a UserDefined class
            if (event->GetTraits().IsUserDefined())
            {
                // need to call AS3 clone()
                Value _this(event);
                Value resultValue;
                Value cloneMethod;
                Multiname cloneMethodName(GetVM().GetPublicNamespace(), GetVM().GetStringManager().CreateConstString("clone"));
                if (event->GetProperty(cloneMethodName, cloneMethod))
                {
                    GetVM().ExecuteUnsafe(cloneMethod, _this, resultValue, 0, NULL);
                    if (GetVM().IsException())
                        return;
                    ASVM& vm = static_cast<ASVM&>(GetVM());
                    if (resultValue.IsObject() && GetVM().IsOfType(resultValue, vm.EventClass->GetClassTraits()))
                    {
                        newEvent = static_cast<Instances::fl_events::Event*>(resultValue.GetObject());
                    }
                }
                else // no clone method found, weird? (!AB)
                    SF_ASSERT(1); // just a bkpt location
            }
            else
            {
                // built-in class, just regular C++ clone() is enough.
                newEvent = event->Clone();
            }
        }
        else
            newEvent = event; // no cloning is necessary
        if (!newEvent)
        {
            Value v(GetVM().GetStringManager().CreateConstString("event"));
            GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG(v)));
            return;
        }
        newEvent->Target = this;
        BuiltinTraitsType tt = GetTraitsType();
        if (tt >= Traits_DisplayObject_Begin && tt <= Traits_DisplayObject_End && GetTraits().IsInstanceTraits())
        {
            Instances::fl_display::DisplayObject* dobj = static_cast<Instances::fl_display::DisplayObject*>(this);
            DoDispatchEvent(newEvent, dobj->pDispObj);
        }
        else
            DoDispatchEvent(newEvent, NULL);

        result = !newEvent->IsDefaultPrevented(); // other conditions to return false here?
//##protect##"instance::EventDispatcher::dispatchEvent()"
    }
    void EventDispatcher::hasEventListener(bool& result, const ASString& type)
    {
//##protect##"instance::EventDispatcher::hasEventListener()"
        result = HasEventHandler(ASString(type), false);
        if (!result)
            result = HasEventHandler(ASString(type), true); // check capture events
//##protect##"instance::EventDispatcher::hasEventListener()"
    }
    void EventDispatcher::removeEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture)
    {
//##protect##"instance::EventDispatcher::removeEventListener()"
        SF_UNUSED1(result);
        if (!pImpl)
            return;
        ASStringHash<ListenersArray*>& listeners = (useCapture) ? pImpl->CaptureListeners : pImpl->Listeners;
        ListenersArray** pplistenersArr = listeners.Get(type);
        if (pplistenersArr)
        {
            ListenersArray* plistenersArr  = *pplistenersArr;
            for (UPInt i = 0, n = plistenersArr->GetSize(); i < n; ++i)
            {
                const Value& func = (*plistenersArr)[i].mFunction;

                if (!func.IsValidWeakRef())
                    continue;

                if (StrictEqual(func, listener))
                {
                    plistenersArr->RemoveAt(i);
                    OnRemoveEventListener(type, useCapture, plistenersArr->GetSize());

                    break;
                }
            }
        }

//##protect##"instance::EventDispatcher::removeEventListener()"
    }
    void EventDispatcher::willTrigger(bool& result, const ASString& type)
    {
//##protect##"instance::EventDispatcher::willTrigger()"
        SF_UNUSED2(result, type);
        
        result = HasEventHandler(type, true);
        if (!result)
            result = HasEventHandler(type, false);
        if (!result)
        {
            BuiltinTraitsType tt = GetTraitsType();
            if (tt >= Traits_DisplayObject_Begin && tt <= Traits_DisplayObject_End && 
                GetTraits().IsInstanceTraits())
            {
                Instances::fl_display::DisplayObject* dobj = static_cast<Instances::fl_display::DisplayObject*>(this);
                EventFlowArrayType captureArray(GetVM().GetMemoryHeap());
                CaptureEventFlow(dobj->pDispObj, captureArray);

                // traverse through event flow array
                for (UPInt i = 0, n = captureArray.GetSize(); i < n; ++i)
                {
                    if (captureArray[i]->HasEventHandler(type, true))
                        result = true;
                    else
                        result = captureArray[i]->HasEventHandler(type, false);
                    if (result)
                        break;
                }
            }
        }
//##protect##"instance::EventDispatcher::willTrigger()"
    }

//##protect##"instance$methods"
    EventDispatcher::~EventDispatcher()
    {
    }

    bool EventDispatcher::HasButtonHandlers() const
    {
        if (pImpl)
        {
            //@TODO: check for pImpl->CaptureButtonHandlersCnt == 255 &
            // pImpl->ButtonHandlersCnt == 255: in this case will need to
            // scan listeners...
            return pImpl->CaptureButtonHandlersCnt > 0 || 
                pImpl->ButtonHandlersCnt > 0;
        }
        return false;
    }

    void EventDispatcher::ListenersHash::ForEachChild_GC(Collector* prcc, GcOp op, const VM& vm, const EventDispatcher& ed, bool useCapture) const
    {
        for(ConstIterator it = Begin(); it != End(); ++it)
        {
            ListenersArray* parr = it->Second;
            if (parr)
            {
                if (vm.IsInAS3VMDestructor())
                {
                    // We MAY NOT call OnRemoveEventListener() in AS3::VM 
                    // destructor because data it uses is not available any more.
                    for (UPInt i = 0, n = parr->GetSize(); i < n; ++i)
                    {
                        const Listener& l = (*parr)[i];
                        if (l.mFunction.IsValidWeakRef())
                            AS3::ForEachChild_GC(prcc, l.mFunction, op SF_DEBUG_ARG(ed));
                    }
                }
                else
                {
                    UPInt i = 0;
                    UPInt size = parr->GetSize();

                    while (i < size)
                    {
                        const Listener& l = (*parr)[i];

                        if (l.mFunction.IsValidWeakRef())
                        {
                            AS3::ForEachChild_GC(prcc, l.mFunction, op SF_DEBUG_ARG(ed));
                            ++i;
                        }
                        else
                        {
                            parr->RemoveAt(i);
                            const_cast<EventDispatcher&>(ed).OnRemoveEventListener(it->First, useCapture, --size);
                            // Do not increment i on purpose.
                        }
                    }
                }
            }
        }
    }

    void EventDispatcher::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        // event handlers
        if (pImpl)
        {
            pImpl->CaptureListeners.ForEachChild_GC(prcc, op, VMRef, *this, true);
            pImpl->Listeners.ForEachChild_GC(prcc, op, VMRef, *this, false);
        }
    }

    SPtr<Instances::fl_events::Event> EventDispatcher::CreateEventObject(const ASString& type, bool bubbles, bool cancelable)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateEventObject(type, bubbles, cancelable);
    }
    SPtr<Instances::fl_events::ProgressEvent> EventDispatcher::CreateProgressEventObject(const ASString& type)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateProgressEventObject(type);
    }
#ifdef SF_AS3_VERSION_AIR
    SPtr<Instances::fl_events::OutputProgressEvent> EventDispatcher::CreateOutputProgressEventObject(const ASString& type)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateOutputProgressEventObject(type);
    }
#endif
    SPtr<Instances::fl_events::FocusEvent> EventDispatcher::CreateFocusEventObject(const ASString& type,
        Instances::fl_display::DisplayObject* relatedObj, unsigned controllerIdx,
        bool shiftKey, unsigned keyCode)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateFocusEventObject(type, relatedObj, controllerIdx, shiftKey, keyCode);
    }
    SPtr<Instances::fl_events::IOErrorEvent> EventDispatcher::CreateIOErrorEventObject(const char* errText)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateIOErrorEventObject(errText);
    }
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    SPtr<Instances::fl_events::StageOrientationEvent> EventDispatcher::CreateStageOrientationEventObject(
        const ASString& type, bool bubbles, bool cancelable, const ASString& beforeOr, const ASString& afterOr)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateStageOrientationEventObject(type, bubbles, cancelable, beforeOr, afterOr);
    }

    SPtr<Instances::fl_events::AppLifecycleEvent> EventDispatcher::CreateAppLifecycleEventObject(
        const ASString& type, bool bubbles, bool cancelable, const ASString& evt)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateAppLifecycleEventObject(type, bubbles, cancelable, evt);
    }

#endif

    SPtr<Instances::fl_events::NetStatusEvent> EventDispatcher::CreateNetStatusEvent(const ASString& code, const ASString& level)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateNetStatusEvent(code, level);
    }
    SPtr<Instances::fl_gfx::MouseCursorEvent> EventDispatcher::CreateMouseCursorEventObject(
        const ASString& cursor, unsigned controllerIdx)
    {
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        return evtDispClass.CreateMouseCursorEventObject(cursor, controllerIdx);
    }

    bool EventDispatcher::Dispatch(const GFx::EventId& evtId, GFx::DisplayObject* dispObject)
    {
        //if (dispObject && dispObject->IsExecutionAborted())
        //    return false;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());

        SPtr<fl_events::Event> ev;
        switch(evtId.Id)
        {
        case EventId::Event_Activate:
        case EventId::Event_Deactivate:
            {
                SPtr<fl_events::Event> efe = evtDispClass.CreateEventObject(
                    (evtId.Id == EventId::Event_Activate) ? vm.GetBuiltin(AS3Builtin_activate) : vm.GetBuiltin(AS3Builtin_deactivate), false, false); 
                efe->Target  = this;
                if (!DispatchSingleEvent(efe, false) && dispObject)
                    dispObject->SetExecutionAborted();
                return !efe->IsDefaultPrevented();
            }
        case EventId::Event_EnterFrame:
            {
                SPtr<fl_events::Event> efe = evtDispClass.CreateEventObject(vm.GetBuiltin(AS3Builtin_enterFrame)); 
                efe->Target  = this;
                if (!DispatchSingleEvent(efe, false) && dispObject)
                    dispObject->SetExecutionAborted();
                return !efe->IsDefaultPrevented();
            }
        case EventId::Event_Render:
            {
                SPtr<fl_events::Event> efe = evtDispClass.CreateEventObject(vm.GetBuiltin(AS3Builtin_render)); 
                efe->Target  = this;
                if (!DispatchSingleEvent(efe, false) && dispObject)
                    dispObject->SetExecutionAborted();
                return !efe->IsDefaultPrevented();
            }
		case EventId::Event_Resize:
			{
				SPtr<fl_events::Event> efe = evtDispClass.CreateEventObject(vm.GetBuiltin(AS3Builtin_resize)); 
				efe->Target  = this;
                if (!DispatchSingleEvent(efe, false) && dispObject)
                    dispObject->SetExecutionAborted();
				return !efe->IsDefaultPrevented();
			}
        case EventId::Event_KeyDown:
            {
                ev = evtDispClass.CreateKeyboardEventObject(evtId, vm.GetBuiltin(AS3Builtin_keyDown), this).GetPtr(); 
                break;
            }
        case EventId::Event_KeyUp:
            {
                ev = evtDispClass.CreateKeyboardEventObject(evtId, vm.GetBuiltin(AS3Builtin_keyUp), this).GetPtr(); 
                break;
            }
        case EventId::Event_MouseOver:
            {
                SPtr<MouseEvent> me = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseOver), this); 
                ev = me.GetPtr();
                const MovieRoot::MouseState* ms = vm.GetMovieRoot()->GetAS3MouseState(evtId.MouseIndex);
                if (ms && ms->LastMouseOverObj)
                    me->RelatedObj = static_cast<fl_display::InteractiveObject*>(ToAvmDisplayObj(ms->LastMouseOverObj)->GetAS3Obj());
                else
                    me->RelatedObj = NULL;
                break;
            }
        case EventId::Event_MouseOut:
            {
                SPtr<MouseEvent> me = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseOut), this).GetPtr(); 
                ev = me.GetPtr();
                const MovieRoot::MouseState* ms = vm.GetMovieRoot()->GetAS3MouseState(evtId.MouseIndex);
                if (ms && ms->LastMouseOverObj)
                    me->RelatedObj = static_cast<fl_display::InteractiveObject*>(ToAvmDisplayObj(ms->LastMouseOverObj)->GetAS3Obj());
                else
                    me->RelatedObj = NULL;
                if (me->RelatedObj == this) // same as target? set to null
                    me->RelatedObj = NULL;
                break;
            }
        case EventId::Event_RollOver:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_rollOver), this).GetPtr(); 
                ev->Bubbles = false; // rollOver doesn't bubble
                break;
            }
        case EventId::Event_RollOut:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_rollOut), this).GetPtr(); 
                ev->Bubbles = false; // rollOut doesn't bubble
                break;
            }
        case EventId::Event_MouseMove:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseMove), this).GetPtr(); 
                break;
            }
        case EventId::Event_MouseDown:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseDown), this).GetPtr(); 
                break;
            }
        case EventId::Event_MouseUp:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseUp), this).GetPtr(); 
                break;
            }
        case EventId::Event_Click:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_click), this).GetPtr(); 
                break;
            }
        case EventId::Event_DoubleClick:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_doubleClick), this).GetPtr(); 
                break;
            }
        case EventId::Event_MouseWheel:
            {
                ev = evtDispClass.CreateMouseEventObject(evtId, vm.GetBuiltin(AS3Builtin_mouseWheel), this).GetPtr(); 
                break;
            }

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
        case EventId::Event_GamePadAnalogChange:
            {
                ev = evtDispClass.CreateGamePadAnalogEventObject(evtId, vm.GetBuiltin(AS3Builtin_gamePadAnalogChange), this).GetPtr();
                break;
            }
#endif

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
        case EventId::Event_TouchBegin:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchBegin), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchEnd:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchEnd), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchMove:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchMove), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchTap:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchTap), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchOver:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchOver), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchOut:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchOut), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchRollOver:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchRollOver), this).GetPtr(); 
                break;
            }
        case EventId::Event_TouchRollOut:
            {
                ev = evtDispClass.CreateTouchEventObject(evtId, vm.GetBuiltin(AS3Builtin_touchRollOut), this).GetPtr(); 
                break;
            }
        case EventId::Event_GestureBegin:
        case EventId::Event_GestureEnd:
        case EventId::Event_GestureUpdate:
        case EventId::Event_GestureAll:
            {
                ev = evtDispClass.CreateGestureEventObject(evtId, this).GetPtr(); 
                break;
            }
#endif
		case EventId::Event_AccelerometerUpdate:
            {
                ev = evtDispClass.CreateAccelerometerEventObject(evtId, vm.GetBuiltin(AS3Builtin_update), this).GetPtr();
                break;
            }

		case EventId::Event_GeolocationUpdate:
            {
                ev = evtDispClass.CreateGeolocationEventObject(evtId, vm.GetBuiltin(AS3Builtin_update), this).GetPtr();
                break;
            }
		case EventId::Event_Status:
            {
                ev = evtDispClass.CreateStatusEventObject(evtId, vm.GetBuiltin(AS3Builtin_status), this).GetPtr();
                break;
            }

        default: 
            return true;
        }
        SF_ASSERT(ev);
        DoDispatchEvent(ev, dispObject);
        return !ev->IsDefaultPrevented();
    }

    bool EventDispatcher::Dispatch(Instances::fl_events::Event* evtObj, GFx::DisplayObject* dispObject)
    {
        SF_ASSERT(evtObj);
        //if (dispObject && dispObject->IsExecutionAborted())
        //    return false;
        evtObj->Target = this;
        DoDispatchEvent(evtObj, dispObject);
        return !evtObj->IsDefaultPrevented();
    }

    void EventDispatcher::DoDispatchEvent(Event* evt, GFx::DisplayObject* dispObject)
    {
        SF_ASSERT(evt);
        evt->WasDispatched = true;

        //if (dispObject && dispObject->IsExecutionAborted())
        //    return;
        if (dispObject)
        {
            SPtr<EventDispatcher> saved(this);
            Ptr<GFx::DisplayObject> saved2(dispObject);
    
            EventFlowArrayType captureArray(GetVM().GetMemoryHeap());
            CaptureEventFlow(dispObject, captureArray);
            bool status = ExecuteCapturePhase(evt, captureArray);

            if (status)
            {
                evt->Phase = Event::Phase_Target;
                if (!DispatchSingleEvent(evt, false))
                {
                    if (dispObject)
                        dispObject->SetExecutionAborted();
                    status = false;
                }
                if (status)
                {
                    if (!evt->IsPropagationStopped() && evt->Bubbles)
                        ExecuteBubblePhase(evt, captureArray);
                }
            }
        }
        else
        {
            if (!DispatchSingleEvent(evt, false) && dispObject)
                dispObject->SetExecutionAborted();
        }
    }

    bool EventDispatcher::ExecuteCapturePhase(
        Instances::fl_events::Event* evtObj, EventFlowArrayType& evtFlowArray)
    {
        evtObj->Phase = Event::Phase_Capture;
        // run in reverse order since the topmost parent is the last element
        for (SPInt i = (SPInt)evtFlowArray.GetSize() - 1; i >= 0; --i)
        {
            if (!evtFlowArray[i]->DispatchSingleEvent(evtObj, true))
            {
                if (evtFlowArray[i]->pDispObj)
                    evtFlowArray[i]->pDispObj->SetExecutionAborted();
                return false;
            }

            // check if propagation was stopped...
            if (evtObj->IsPropagationStopped())
                return false;
        }
        return true;
    }

    bool EventDispatcher::ExecuteBubblePhase(
        Instances::fl_events::Event* evtObj, EventFlowArrayType& evtFlowArray)
    {
        evtObj->Phase = Event::Phase_Bubble;
        // run in order since the topmost parent is the last element
        for (UPInt i = 0, n = evtFlowArray.GetSize(); i < n; ++i)
        {
            if (!evtFlowArray[i]->DispatchSingleEvent(evtObj, false))
            {
                if (evtFlowArray[i]->pDispObj)
                    evtFlowArray[i]->pDispObj->SetExecutionAborted();
                return false;
            }

            // check if propagation was stopped...
            if (evtObj->IsPropagationStopped())
                return false;
        }
        return true;
    }

    void EventDispatcher::CaptureEventFlow(GFx::DisplayObject* dispObject, 
                                           EventFlowArrayType& evtFlowArray)
    {
        // need to collect an array of the display objects participating in event flow.
        // the last element of the array will contain the topmost parent.
        for (GFx::InteractiveObject* p = dispObject->GetParent(); p; p = p->GetParent())
        {
            Instances::fl_display::DisplayObject* as3p = ToAvmInteractiveObj(p)->GetAS3Obj();
            //SF_ASSERT(as3p);
            if (as3p)
            {
                evtFlowArray.PushBack(as3p);
            }
        }
    }

    bool EventDispatcher::DispatchSingleEvent(Instances::fl_events::Event* evtObj, bool useCapture)
    {
        if (!pImpl)
            return true;
        SPtr<EventDispatcher> saved(this);
        ASStringHash<ListenersArray*>& listeners = (useCapture) ? pImpl->CaptureListeners : pImpl->Listeners;
        ListenersArray** pplistenersArr = listeners.Get(evtObj->Type);
        bool rv = true;
        if (pplistenersArr)
        {
            evtObj->CurrentTarget = this;
            ListenersArray* plistenersArr  = *pplistenersArr;
            // make a copy of listener array to make sure it is immutable
            // during the dispatch. Need to do this only if size of the array
            // is more than 1.
            if (plistenersArr->GetSize() == 1)
            {
                const Value& func = (*plistenersArr)[0].mFunction;

                if (func.IsValidWeakRef())
                {
                    Value _this;
                    Value param(evtObj);
                    Value result;

                    GetVM().ExecuteUnsafe(
                        // Make a strong ref if necessary.
                        func.IsWeakRef() ? Value(func, StrongRefValue) : func, 
                        _this, result, 1, &param);
                    if (GetVM().IsException())
                    {
                        GetVM().OutputAndIgnoreException();
                        rv = false;
                    }
                }
            }
            else if (plistenersArr->GetSize() > 1)
            {
                UByte fixedarr[sizeof(Listener)*10];
                UByte* arr;
                if (plistenersArr->GetSize() <= 10)
                {
                    arr = fixedarr;
                }
                else
                {
                    arr = (UByte*)SF_HEAP_AUTO_ALLOC(this, sizeof(Listener)*plistenersArr->GetSize());
                }
                UPInt n = plistenersArr->GetSize();
                Listener* copiedListeners = reinterpret_cast<Listener*>(arr);
                for(UPInt i = 0; i < n; ++i)
                {
                    new (&copiedListeners[i]) Listener((*plistenersArr)[i]);
                }

                UPInt i = 0;
                for (; i < n; ++i)
                {
                    const Value& func = copiedListeners[i].mFunction;

                    if (func.IsValidWeakRef())
                    {
                        Value _this;
                        Value result;
                        Value param(evtObj);

                        GetVM().ExecuteUnsafe(
                            // Make a strong ref if necessary.
                            func.IsWeakRef() ? Value(func, StrongRefValue) : func, 
                            _this, result, 1, &param);
                        if (GetVM().IsException())
                        {
                            GetVM().OutputAndIgnoreException();
                            rv = false;
                        }

                        // call dtor for used entry
                        copiedListeners[i].~Listener();

                        // check if immediate propagation was stopped...
                        if (evtObj->IsImmPropagationStopped() || !rv)
                            break;
                    }
                    else
                    {
                        // call dtor for used entry
                        copiedListeners[i].~Listener();
                    }
                }

                // cleanup remaining listeners
                for (++i; i < n; ++i)
                {
                    copiedListeners[i].~Listener();
                }
                if (arr != fixedarr)
                    SF_FREE(arr);
            }
        } 
        return rv;
    }

    bool EventDispatcher::DispatchToTarget(const ASString& type, Instances::fl::Object* target, bool useCapture,
                                           GFx::DisplayObject* dispObject)
    {
        if (!pImpl)
            return true;
        //if (dispObject && dispObject->IsExecutionAborted())
        //    return false;
        if (GetVM().IsException())
        {
            ASVM& vm = static_cast<ASVM&>(GetVM());
            if (vm.GetLogState())
                vm.GetLogState()->LogScriptError("Can't dispatch '%s' - exception is not cleared",
                type.ToCStr());
            return true;
        }
        Classes::fl_events::EventDispatcher& evtDispClass = 
            static_cast<Classes::fl_events::EventDispatcher&>(GetClass());
        SPtr<Instances::fl_events::Event> evtObj = evtDispClass.CreateEventObject(type); 
        evtObj->Target = target;
        evtObj->CurrentTarget = target;
        if (!DispatchSingleEvent(evtObj, useCapture) && dispObject)
            dispObject->SetExecutionAborted();
        return !evtObj->IsDefaultPrevented();
    }

    bool EventDispatcher::MayHaveEnterFrameHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_EnterFrame) != 0: false;
    }
    bool EventDispatcher::MayHaveFrameConstructedHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_FrameConstructed) != 0: false;
    }
    bool EventDispatcher::MayHaveExitFrameHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_ExitFrame) != 0: false;
    }

    bool EventDispatcher::MayHaveActivateHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_Activate) != 0 : false;
    }

    bool EventDispatcher::MayHaveDeactivateHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_Deactivate) != 0 : false;
    }

    bool EventDispatcher::MayHaveRenderHandler() const
    {
        return (pImpl) ? (pImpl->Flags & EventDispatcherImpl::Flag_Render) != 0 : false;
    }

    bool EventDispatcher::HasEventHandler(const ASString& type, bool useCapture) const
    {
        if (pImpl)
        {
            ASStringHash<ListenersArray*>& listeners = (useCapture) ? 
                pImpl->CaptureListeners : pImpl->Listeners;
            ListenersArray** pplistenersArr = listeners.Get(type);
            if (pplistenersArr && (*pplistenersArr) && (*pplistenersArr)->GetSize() > 0)
                return true;
        }
        return false;
    }

    bool EventDispatcher::WillTrigger(const ASString& type, bool useCapture) const
    {
        if (HasEventHandler(type, useCapture))
            return true;
        BuiltinTraitsType tt = GetTraitsType();
        if (tt >= Traits_DisplayObject_Begin && tt <= Traits_DisplayObject_End && 
            GetTraits().IsInstanceTraits())
        {
            const Instances::fl_display::DisplayObject* dispObject = static_cast<const Instances::fl_display::DisplayObject*>(this);

            if (dispObject->pDispObj)
            {
                for (GFx::InteractiveObject* p = dispObject->pDispObj->GetParent(); p; p = p->GetParent())
                {
                    Instances::fl_display::DisplayObject* as3p = ToAvmInteractiveObj(p)->GetAS3Obj();
                    //SF_ASSERT(as3p);
                    if (as3p)
                    {
                        if (as3p->HasEventHandler(type, useCapture))
                            return true;
                    }
                }
            }
        }
        return false;
    }

    void EventDispatcher::OnRemoveEventListener(const ASString& evtType, bool useCapture, UPInt listenersArrSize)
    {
        ASVM& vm = static_cast<ASVM&>(GetVMSafe());
        //const StringManager sm& = GetVM().GetStringManager();

        if (vm.GetMovieRoot() == NULL)
            return;

        EventId::IdCode evtId = EventId::Event_End;
        if (evtType == vm.GetBuiltin(AS3Builtin_enterFrame))
        {
            evtId = EventId::Event_EnterFrame;
            vm.GetMovieImpl()->InvalidateOptAdvanceList();
        }
        else if (evtType == vm.GetBuiltin(AS3Builtin_frameConstructed))
            evtId = EventId::Event_FrameConstructed;
        else if (evtType == vm.GetBuiltin(AS3Builtin_exitFrame))
            evtId = EventId::Event_ExitFrame;
        else if (evtType == vm.GetBuiltin(AS3Builtin_activate))
            evtId = EventId::Event_Activate;
        else if (evtType == vm.GetBuiltin(AS3Builtin_deactivate))
        {
            evtId = EventId::Event_Deactivate;
        }
        else if (evtType == vm.GetBuiltin(AS3Builtin_render))
            evtId = EventId::Event_Render;

        if (evtId != EventId::Event_End)
        {
            if (listenersArrSize == 0)
            {
                BuiltinTraitsType btt = GetTraitsType();
                if (btt >= Traits_DisplayObject_Begin && btt <= Traits_DisplayObject_End && GetTraits().IsInstanceTraits())
                {
                    fl_display::DisplayObject* dobj = static_cast<fl_display::DisplayObject*>(this);
                    SF_ASSERT(dobj->pDispObj);
                    vm.GetMovieRoot()->RemoveFromEventChain(evtId, dobj->pDispObj);
                }
            }
        }
        else
        {
            // check for button event handlers
            if (vm.GetBuiltin(AS3Builtin_mouseDown) == evtType ||
                vm.GetBuiltin(AS3Builtin_mouseUp) == evtType ||
                vm.GetBuiltin(AS3Builtin_click) == evtType ||
                vm.GetBuiltin(AS3Builtin_doubleClick) == evtType ||
                vm.GetBuiltin(AS3Builtin_mouseOver) == evtType ||
                vm.GetBuiltin(AS3Builtin_mouseOut) == evtType ||
                vm.GetBuiltin(AS3Builtin_rollOver) == evtType ||
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
                vm.GetBuiltin(AS3Builtin_touchBegin) == evtType ||
                vm.GetBuiltin(AS3Builtin_touchEnd) == evtType ||
                vm.GetBuiltin(AS3Builtin_touchMove) == evtType ||
                vm.GetBuiltin(AS3Builtin_touchTap) == evtType ||
#endif
                vm.GetBuiltin(AS3Builtin_rollOut) == evtType)
            {
                if (useCapture)
                {
                    SF_ASSERT(pImpl->CaptureButtonHandlersCnt > 0);
                    if (pImpl->CaptureButtonHandlersCnt < 255)
                        --pImpl->CaptureButtonHandlersCnt;
                }
                else
                {
                    SF_ASSERT(pImpl->ButtonHandlersCnt > 0);
                    if (pImpl->ButtonHandlersCnt < 255)
                        --pImpl->ButtonHandlersCnt;
                }
            }
        }
    }
    void EventDispatcher::RemoveListenersForMovieDef(MovieDefImpl* defimpl, ListenersHash& hash)
    {
        for(ListenersHash::Iterator it = hash.Begin(); it != hash.End(); ++it)
        {
            ListenersArray* arr = it->Second;
            for (UPInt i = 0; i < arr->GetSize();)
            {
                Listener& l = (*arr)[i];
                VMAbcFile* abc = NULL;

                if (l.mFunction.IsValidWeakRef())
                {
                    // A hack for kVTableInd and kVTableIndClosure.
                    switch (l.mFunction.GetKind())
                    {
                    case Value::kVTableInd:
                        abc = l.mFunction.GetTraits().GetFilePtr();
                        break;
                    case Value::kVTableIndClosure:
                        abc = l.mFunction.GetClosure()->GetTraits().GetFilePtr();
                        break;
                    default:
                        abc = GetVM().GetValueTraits(l.mFunction).GetFilePtr();
                        break;
                    }
                    //VMAbcFile* abc = GetVM().GetValueTraits(l.mFunction).GetFilePtr();

                    if (abc)
                    {
                        const ASVM::AbcFileWithMovieDef& abcF = static_cast<const ASVM::AbcFileWithMovieDef&>(abc->GetAbcFile());
                        if (abcF.pDefImpl == defimpl)
                        {
                            arr->RemoveAt(i);
                            continue;
                        }
                    }
                }
                ++i;
            }
        }
    }
    void EventDispatcher::RemoveListenersForMovieDef(MovieDefImpl* defimpl)
    {
        if (pImpl)
        {
            RemoveListenersForMovieDef(defimpl, pImpl->CaptureListeners);
            RemoveListenersForMovieDef(defimpl, pImpl->Listeners);
        }
    }

    //##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 EventDispatcher::tito[EventDispatcher::ThunkInfoNum] = {
    //    0, 6, 8, 10, 14, 
    // };
    const TypeInfo* EventDispatcher::tit[16] = {
        NULL, &AS3::fl::StringTI, NULL, &AS3::fl::BooleanTI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_events::EventTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo EventDispatcher::ti[EventDispatcher::ThunkInfoNum] = {
        {TFunc_Instances_EventDispatcher_addEventListener::Func, &EventDispatcher::tit[0], "addEventListener", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 0, NULL},
        {TFunc_Instances_EventDispatcher_dispatchEvent::Func, &EventDispatcher::tit[6], "dispatchEvent", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_EventDispatcher_hasEventListener::Func, &EventDispatcher::tit[8], "hasEventListener", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_EventDispatcher_removeEventListener::Func, &EventDispatcher::tit[10], "removeEventListener", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 0, NULL},
        {TFunc_Instances_EventDispatcher_willTrigger::Func, &EventDispatcher::tit[14], "willTrigger", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    EventDispatcher::EventDispatcher(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::EventDispatcher::EventDispatcher()"
//##protect##"InstanceTraits::EventDispatcher::EventDispatcher()"

    }

    void EventDispatcher::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<EventDispatcher&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    EventDispatcher::EventDispatcher(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::EventDispatcher::EventDispatcher()"
//##protect##"class_::EventDispatcher::EventDispatcher()"
    }
//##protect##"class_$methods"
    void EventDispatcher::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Class::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<InstanceTraits::Traits, Mem_Stat>(prcc, EventTraits, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<InstanceTraits::Traits, Mem_Stat>(prcc, MouseEventTraits, op SF_DEBUG_ARG(*this));
    }

    void EventDispatcher::CacheEventTraits()
    {
        if (!EventTraits)
        {
            //GetVM().FindClass
        }
    }

    SPtr<Instances::fl_events::Event>       EventDispatcher::CreateEventObject(const ASString& type, bool bubbles, bool cancelable)
    {
        SPtr<Instances::fl_events::Event> evt;
        Value params[] = { Value(type), Value(bubbles), Value(cancelable) };
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.EventClass, 3, params);
        SF_ASSERT(evt);
        return evt;
    }
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    SPtr<Instances::fl_events::StageOrientationEvent> EventDispatcher::CreateStageOrientationEventObject
        (const ASString& type, bool bubbles, bool cancelable, const ASString& beforeOr, const ASString& afterOr)
    {
        SPtr<Instances::fl_events::StageOrientationEvent> evt;
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Value params[] = { Value(type), Value(bubbles), Value(cancelable), Value(beforeOr), Value(afterOr) };
        asvm.ConstructInstance(evt, asvm.StageOrientationEventClass, 5, params);
        SF_ASSERT(evt);
        return evt;
    }

    SPtr<Instances::fl_events::AppLifecycleEvent> EventDispatcher::CreateAppLifecycleEventObject(
        const ASString& type, bool bubbles, bool cancelable, const ASString& evtName)
    {
        SPtr<Instances::fl_events::AppLifecycleEvent> evt;
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Value params[] = { Value(type), Value(bubbles), Value(cancelable), Value(evtName) };
        asvm.ConstructInstance(evt, asvm.AppLifecycleEventClass, 4, params);
        SF_ASSERT(evt);
        return evt;
    }

#endif

    SPtr<Instances::fl_events::MouseEvent>  EventDispatcher::CreateMouseEventObject(
        const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
    {
        SPtr<Instances::fl_events::MouseEvent> evt;
        Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.ExtensionsEnabled ? asvm.MouseEventExClass : asvm.MouseEventClass, 1, &v);
        SF_ASSERT(evt);
        evt->Target = target;
        if (evtId.Id == GFx::EventId::Event_RollOver || 
            evtId.Id == GFx::EventId::Event_RollOut)
            evt->Bubbles = false;
        else
            evt->Bubbles = true;
        evt->Cancelable = false;
        evt->AltKey  = evtId.KeysState.IsAltPressed();
        evt->CtrlKey = evtId.KeysState.IsCtrlPressed();
        evt->ShiftKey= evtId.KeysState.IsShiftPressed();
        evt->Delta   = evtId.MouseWheelDelta;

        if (asvm.ExtensionsEnabled)
        {
            Instances::fl_gfx::MouseEventEx* evtEx = static_cast<Instances::fl_gfx::MouseEventEx*>(evt.GetPtr());
            evtEx->SetMouseIdx(evtId.MouseIndex);
            evtEx->SetNestingIdx(evtId.RollOverCnt);
            evtEx->SetButtonIdx(evtId.ButtonId);
        }

        ASVM& vm = static_cast<ASVM&>(GetVM());
        const MouseState* ms = vm.GetMovieImpl()->GetMouseState(evtId.MouseIndex);
        SF_ASSERT(ms);
        evt->SetStageCoords(ms->GetLastPosition());

        SF_ASSERT((int)Instances::fl_events::MouseEvent::Button_Middle == (int)GFx::MouseState::MouseButton_Middle);
        
        // For rollOver/Out, mouseOver/Out we need to provide a previous state of buttons.
        // For example, if user is holding a mouse button pressed over a movieclip and then 
        // quickly moves it out of it simultaneously releasing the button, then rollOut
        // event should be generated like the button is still pressed and only then generate
        // a MouseUp event (for a new object, depends where the mouse cursor ends up). The same
        // situation may occur for rollOver when a button is pressed simultaneously with moving a cursor
        // over the movieclip: rollOver should be generated with button unpressed, since MouseDown will
        // be generated after it.
        if (evtId.Id == GFx::EventId::Event_RollOver || 
            evtId.Id == GFx::EventId::Event_RollOut ||
            evtId.Id == GFx::EventId::Event_MouseOver || 
            evtId.Id == GFx::EventId::Event_MouseOut)
        {
            evt->ButtonsMask = ms->GetPrevButtonsState();
        }
        else
            evt->ButtonsMask = ms->GetButtonsState();
        return evt;
    }
    SPtr<Instances::fl_events::KeyboardEvent>  EventDispatcher::CreateKeyboardEventObject(
        const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
    {
        SPtr<Instances::fl_events::KeyboardEvent> evt;
        Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.ExtensionsEnabled ? asvm.KeyboardEventExClass : asvm.KeyboardEventClass, 1, &v);
        SF_ASSERT(evt);
        evt->Target   = target;
        evt->SetEventId(evtId);
        evt->Bubbles  = true;
        evt->Cancelable = false;

        if (asvm.ExtensionsEnabled)
        {
            Instances::fl_gfx::KeyboardEventEx* evtEx = static_cast<Instances::fl_gfx::KeyboardEventEx*>(evt.GetPtr());
            evtEx->SetControllerIdx(evtId.ControllerIndex);
        }

        return evt;    
    }
#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    SPtr<Instances::fl_gfx::GamePadAnalogEvent> EventDispatcher::CreateGamePadAnalogEventObject(
        const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
    {
        SPtr<Instances::fl_gfx::GamePadAnalogEvent> evt;
        const GFx::GamePadAnalogEventId& gpid = static_cast<const GFx::GamePadAnalogEventId&>(evtId);
        Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.GamePadAnalogEventClass, 1, &v);
        SF_ASSERT(evt);
        evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
        evt->Code = evtId.KeyCode;
        evt->ControllerIdx = evtId.ControllerIndex;
        evt->XValue = gpid.AnalogX;
        evt->YValue = gpid.AnalogY;
        return evt;
    }
#endif

	SPtr<Instances::fl_events::StatusEvent> EventDispatcher::CreateStatusEventObject(
		const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
	{
		SPtr<Instances::fl_events::StatusEvent> evt;
		const GFx::StatusEventId& seid = static_cast<const GFx::StatusEventId&>(evtId);
		Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.StatusEventClass, 1, &v);
		SF_ASSERT(evt);
		evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
		evt->Code = seid.Code->ToCStr();
		evt->Level = seid.Level->ToCStr();
        SF_ASSERT(evt);
        return evt;
	}

	SPtr<Instances::fl_events::AccelerometerEvent> EventDispatcher::CreateAccelerometerEventObject(
		const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
	{
		SPtr<Instances::fl_events::AccelerometerEvent> evt;
		const GFx::AccelerometerEventId& aeid = static_cast<const GFx::AccelerometerEventId&>(evtId);
		Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.AccelerometerEventClass, 1, &v);
		SF_ASSERT(evt);
		evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
		evt->timestamp = aeid.Timestamp;
		//SF_DEBUG_MESSAGE3(1, "x=%G\ty=%G\tz=%G\t\n", aeid.AccelerationX, aeid.AccelerationY, aeid.AccelerationZ);
		evt->accelerationX = MakeValueNumber(aeid.AccelerationX);
		evt->accelerationY = MakeValueNumber(aeid.AccelerationY);
		evt->accelerationZ = MakeValueNumber(aeid.AccelerationZ);
        SF_ASSERT(evt);
        return evt;
	}

SPtr<Instances::fl_events::GeolocationEvent> EventDispatcher::CreateGeolocationEventObject(
		const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
	{
		SPtr<Instances::fl_events::GeolocationEvent> evt;
		const GFx::GeolocationEventId& geid = static_cast<const GFx::GeolocationEventId&>(evtId);
		Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.GeolocationEventClass, 1, &v);
		SF_ASSERT(evt);
		evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
		evt->latitude = geid.Latitude;
		evt->longitude = geid.Longitude;
		evt->altitude = geid.Altitude;
		evt->hAccuracy = geid.HAccuracy;
		evt->vAccuracy = geid.VAccuracy;
		evt->speed = geid.Speed;
		evt->heading = geid.Heading;
		evt->timestamp = geid.Timestamp;
        SF_ASSERT(evt);
        return evt;
	}

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    SPtr<Instances::fl_events::TouchEvent>  EventDispatcher::CreateTouchEventObject(
        const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target)
    {
        SPtr<Instances::fl_events::TouchEvent> evt;
		const GFx::TouchEventId& teid = static_cast<const GFx::TouchEventId&>(evtId);
        Value v(type);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(evt, asvm.TouchEventClass, 1, &v);
        SF_ASSERT(evt);
        evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
        evt->AltKey  = evtId.KeysState.IsAltPressed();
        evt->CtrlKey = evtId.KeysState.IsCtrlPressed();
        evt->ShiftKey= evtId.KeysState.IsShiftPressed();
        evt->TouchPointID = evtId.TouchID;
		evt->PrimaryPoint = teid.PrimaryPoint;
		evt->Pressure     = teid.Pressure;
		evt->SizeX        = teid.Contact.Width;
		evt->SizeY        = teid.Contact.Height;
        if (asvm.IsOfType(Value(target), "flash.display.InteractiveObject", asvm.GetCurrentAppDomain()))
            evt->RelatedObj = (Instances::fl_display::InteractiveObject*)target;
        else
            evt->RelatedObj = NULL;

        ASVM& vm = static_cast<ASVM&>(GetVM());
        const MouseState* ms = vm.GetMovieImpl()->GetMouseState(evtId.MouseIndex);
        SF_ASSERT(ms);
        evt->StageX  = ms->GetLastPosition().x;
        evt->StageY  = ms->GetLastPosition().y;

        return evt;
    }
    SPtr<Instances::fl_events::GestureEvent>  EventDispatcher::CreateGestureEventObject(
        const GFx::EventId& evtId, Instances::fl::Object* target)
    {
        SPtr<Instances::fl_events::GestureEvent> evt;
        const GFx::GestureEventId& teid = static_cast<const GFx::GestureEventId&>(evtId);
        ASVM& asvm = static_cast<ASVM&>(GetVM());

        const char* evtGestureType = NULL;
        switch(teid.Gesture)
        {
        case GFx::GestureEventId::Gesture_Pan: 
            evtGestureType = "gesturePan"; 
            break;
        case GFx::GestureEventId::Gesture_Zoom: 
            evtGestureType = "gestureZoom"; 
            break;
        case GFx::GestureEventId::Gesture_Rotate: 
            evtGestureType = "gestureRotate"; 
            break;
        case GFx::GestureEventId::Gesture_Swipe: 
            evtGestureType = "gestureSwipe"; 
            break;
        case GFx::GestureEventId::Gesture_PressAndTap: 
            evtGestureType = "gesturePressAndTap"; 
            break;
        case GFx::GestureEventId::Gesture_TwoFingerTap: 
            evtGestureType = "gestureTwoFingerTap"; 
            break;
        default: SF_ASSERT(0); //? unknown event?
        }
        ASString type(asvm.GetStringManager().CreateConstString(evtGestureType));
        Value v(type);
        if (teid.Gesture == GFx::GestureEventId::Gesture_PressAndTap)
        {
            // press and tap - PressAndTapGestureEvent
            SPtr<Instances::fl_events::PressAndTapGestureEvent> tevt;
            asvm.ConstructInstance(tevt, asvm.PressAndTapGestureEventClass, 1, &v);
            SF_ASSERT(tevt);
            evt = tevt;
            tevt->TapStageX = teid.TapX;
            tevt->TapStageY = teid.TapY;
			tevt->TapStageSet=teid.TapXYUse;
        }
        else if (teid.Gesture == GFx::GestureEventId::Gesture_TwoFingerTap)
        {
            // two finger tap - regular GestureEvent
            SPtr<Instances::fl_events::GestureEvent> tevt;
            asvm.ConstructInstance(tevt, asvm.GestureEventClass, 1, &v);
            SF_ASSERT(tevt);
            evt = tevt;
        }
        else
        {
            SPtr<Instances::fl_events::TransformGestureEvent> tevt;
            asvm.ConstructInstance(tevt, asvm.TransformGestureEventClass, 1, &v);
            SF_ASSERT(tevt);
            evt = tevt;
            tevt->OffsetX = teid.OffsetX;
            tevt->OffsetY = teid.OffsetY;
            tevt->ScaleX  = teid.ScaleX;
            tevt->ScaleY  = teid.ScaleY;
            tevt->Rotation = teid.Rotation;
            switch(evtId.Id)
            {
            case GFx::EventId::Event_GestureBegin:  evt->Phase = Instances::fl_events::GestureEvent::Phase_Begin; break;
            case GFx::EventId::Event_GestureEnd:    evt->Phase = Instances::fl_events::GestureEvent::Phase_End; break;
            case GFx::EventId::Event_GestureUpdate: evt->Phase = Instances::fl_events::GestureEvent::Phase_Update; break;
            case GFx::EventId::Event_GestureAll:    evt->Phase = Instances::fl_events::GestureEvent::Phase_All; break;
            default: SF_ASSERT(0);
            }
        }
        evt->Target = target;
        evt->Bubbles = true;
        evt->Cancelable = false;
        evt->AltKey  = evtId.KeysState.IsAltPressed();
        evt->CtrlKey = evtId.KeysState.IsCtrlPressed();
        evt->ShiftKey= evtId.KeysState.IsShiftPressed();
        evt->StageX  = teid.x;
        evt->StageY  = teid.y;

        return evt;
    }
#endif
    SPtr<Instances::fl_events::ProgressEvent> EventDispatcher::CreateProgressEventObject(const ASString& type)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::ProgressEvent> evt;
        Value params[] = { Value(type), Value(false), Value(false) };
        asvm.ConstructInstance(evt, asvm.ProgressEventClass, sizeof(params)/sizeof(params[0]), params);
        SF_ASSERT(evt);
        return evt;
    }
#ifdef SF_AS3_VERSION_AIR
    SPtr<Instances::fl_events::OutputProgressEvent> EventDispatcher::CreateOutputProgressEventObject(const ASString& type)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::OutputProgressEvent> evt;
        Value params[] = { Value(type), Value(false), Value(false) };
        asvm.ConstructInstance(evt, asvm.OutputProgressEventClass, sizeof(params)/sizeof(params[0]), params);
        SF_ASSERT(evt);
        return evt;
    }
#endif
    SPtr<Instances::fl_events::FocusEvent> EventDispatcher::CreateFocusEventObject(
        const ASString& type, Instances::fl_display::DisplayObject* relatedObj, unsigned controllerIdx, 
        bool shiftKey, unsigned keyCode)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::FocusEvent> evt;
        Value params[] = { Value(type), Value(true), Value(false), 
            Value(relatedObj), Value(shiftKey), Value(static_cast<UInt32>(keyCode)) };
        asvm.ConstructInstance(evt, asvm.ExtensionsEnabled ? asvm.FocusEventExClass : asvm.FocusEventClass, sizeof(params)/sizeof(params[0]), params);
        SF_ASSERT(evt);

        if (asvm.ExtensionsEnabled)
        {
            Instances::fl_gfx::FocusEventEx* evtEx = static_cast<Instances::fl_gfx::FocusEventEx*>(evt.GetPtr());
            evtEx->SetControllerIdx(controllerIdx);
        }

        return evt;
    }
    SPtr<Instances::fl_events::IOErrorEvent> EventDispatcher::CreateIOErrorEventObject(const char* errText)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::IOErrorEvent> evt;
        Value params[] = { Value(asvm.GetStringManager().CreateConstString("ioError")), 
            Value(false), Value(false), asvm.GetStringManager().CreateString(errText) };
        Class* result = GetVM().GetClass("flash.events.IOErrorEvent", GetVM().GetCurrentAppDomain());
        if (!result)
            SF_ASSERT(0);
        asvm.ConstructInstance(evt, result, sizeof(params)/sizeof(params[0]), params);
        SF_ASSERT(evt);
        return evt;
    }
    SPtr<Instances::fl_events::NetStatusEvent> EventDispatcher::CreateNetStatusEvent(const ASString& code, const ASString& level)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::NetStatusEvent> evt;

        Class* result = GetVM().GetClass("flash.events.NetStatusEvent", GetVM().GetCurrentAppDomain());
        SF_ASSERT(result);

        Value params[] = {
            Value(asvm.GetStringManager().CreateConstString("netStatus")),
            Value(false), Value(false), code, level
        };
        asvm.ConstructInstance(evt, result, sizeof(params)/sizeof(params[0]), params);
        SF_ASSERT(evt);

        return evt;
    }
    SPtr<Instances::fl_gfx::MouseCursorEvent> EventDispatcher::CreateMouseCursorEventObject(
        const ASString& cursor, unsigned controllerIdx)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SF_ASSERT(asvm.ExtensionsEnabled);
        SPtr<Instances::fl_gfx::MouseCursorEvent> evt;
        Value params[] = { Value(asvm.GetBuiltin(AS3Builtin_mouseCursorChange)), Value(false), Value(true) };
        Class* result = GetVM().GetClass("scaleform.gfx.MouseCursorEvent", GetVM().GetCurrentAppDomain());
        if (!result)
            SF_ASSERT(0);
        else
        {
            asvm.ConstructInstance(evt, result, sizeof(params)/sizeof(params[0]), params);
            SF_ASSERT(evt);

            evt->SetControllerIdx(controllerIdx);
            evt->SetCursor(cursor);
        }

        return evt;
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{

    EventDispatcher::EventDispatcher(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::EventDispatcher::EventDispatcher()"
//##protect##"ClassTraits::EventDispatcher::EventDispatcher()"

    }

    Pickable<Traits> EventDispatcher::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) EventDispatcher(vm, AS3::fl_events::EventDispatcherCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::EventDispatcherCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_events
{
    const TypeInfo* EventDispatcherImplements[] = {
        &fl_events::IEventDispatcherTI,
        NULL
    };

    const TypeInfo EventDispatcherTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::EventDispatcher::InstanceType),
        0,
        0,
        InstanceTraits::fl_events::EventDispatcher::ThunkInfoNum,
        0,
        "EventDispatcher", "flash.events", &fl::ObjectTI,
        EventDispatcherImplements
    };

    const ClassInfo EventDispatcherCI = {
        &EventDispatcherTI,
        ClassTraits::fl_events::EventDispatcher::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_events::EventDispatcher::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

