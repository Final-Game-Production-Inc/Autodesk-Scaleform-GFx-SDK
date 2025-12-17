//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_EventDispatcher.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_EventDispatcher_H
#define INC_AS3_Obj_Events_EventDispatcher_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_Input.h"
#include "Kernel/SF_ArrayStaticBuff.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo EventDispatcherTI;
    extern const ClassInfo EventDispatcherCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
    extern const TypeInfo IEventDispatcherTI;
    extern const ClassInfo IEventDispatcherCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class EventDispatcher;
}}

namespace InstanceTraits { namespace fl_events
{
    class EventDispatcher;
}}

namespace Classes { namespace fl_events
{
    class EventDispatcher;
}}

//##protect##"forward_declaration"
namespace Instances { 
    namespace fl_events
    {
        class Event;
        class MouseEvent;
        class KeyboardEvent;
        class FocusEvent;
        class TouchEvent;
        class GestureEvent;
        class ProgressEvent;
#ifdef SF_AS3_VERSION_AIR
        class OutputProgressEvent;
#endif
        class IOErrorEvent;
        class AppLifecycleEvent;
        class StageOrientationEvent;
        class NetStatusEvent;
		class StatusEvent;
		class AccelerometerEvent;
		class GeolocationEvent;
    }
    namespace fl_display
    {
        class DisplayObject;
    }
    namespace fl_gfx
    {
        class MouseCursorEvent;
        class GamePadAnalogEvent;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class EventDispatcher : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::EventDispatcher;
        
    public:
        typedef EventDispatcher SelfType;
        typedef Classes::fl_events::EventDispatcher ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::EventDispatcherTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::EventDispatcher"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        EventDispatcher(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        struct Listener
        {
            unsigned    Priority;
            Value       mFunction;    

            Listener():Priority(0) {}
            Listener(unsigned p, const Value& v):Priority(p), mFunction(v) {}
            ~Listener() {}
        };
        typedef ArrayLH<Listener> ListenersArray;

        struct ListenersHash : ASStringHash<ListenersArray*>
        {
            ~ListenersHash()
            {
                for (Iterator it = Begin(); it != End(); ++it)
                {
                    delete it->Second;
                }
            }
            void         ForEachChild_GC(Collector* prcc, GcOp op, const VM& vm, const EventDispatcher& ed, bool useCapture) const;
        };

        struct EventDispatcherImpl : public NewOverrideBase<StatMV_ActionScript_Mem>
        {
            ListenersHash   CaptureListeners;
            ListenersHash   Listeners;
            enum 
            {
                Flag_EnterFrame         = 0x1,
                Flag_Activate           = 0x2,
                Flag_Deactivate         = 0x4,
                Flag_Render             = 0x8,
                Flag_Resize				= 0x10,
                Flag_FrameConstructed   = 0x20,
                Flag_ExitFrame          = 0x40
            };
            UInt16          Flags;
            UInt8           CaptureButtonHandlersCnt;
            UInt8           ButtonHandlersCnt;

            EventDispatcherImpl() : Flags(0), 
                CaptureButtonHandlersCnt(0), ButtonHandlersCnt(0) {}
        };
    protected:
        virtual void         ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        ~EventDispatcher();
        
        typedef ArrayStaticBuff<SPtr<fl_display::DisplayObject>, 32> EventFlowArrayType;

        SPtr<Instances::fl_events::Event>          CreateEventObject(
            const ASString& type, bool bubbles = false, bool cancelable = false);
        SPtr<Instances::fl_events::ProgressEvent>  CreateProgressEventObject(const ASString& type);
#ifdef SF_AS3_VERSION_AIR
        SPtr<Instances::fl_events::OutputProgressEvent>  CreateOutputProgressEventObject(const ASString& type);
#endif
        SPtr<Instances::fl_events::FocusEvent>     CreateFocusEventObject(const ASString& type,
            Instances::fl_display::DisplayObject* relatedObj, unsigned controllerIdx, 
            bool shiftKey = false, unsigned keyCode = 0);
        SPtr<Instances::fl_events::IOErrorEvent>   CreateIOErrorEventObject(const char* errText);
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
        SPtr<Instances::fl_events::AppLifecycleEvent> CreateAppLifecycleEventObject(
            const ASString& type, bool bubbles, bool cancelable, const ASString& event);
        SPtr<Instances::fl_events::StageOrientationEvent> CreateStageOrientationEventObject(
            const ASString& type, bool bubbles, bool cancelable, const ASString& beforeOr, const ASString& afterOr);
#endif
        SPtr<Instances::fl_events::NetStatusEvent> CreateNetStatusEvent(const ASString& code, const ASString& level);
        SPtr<Instances::fl_gfx::MouseCursorEvent> CreateMouseCursorEventObject(
            const ASString& cursor, unsigned controllerIdx);

        // return false, if default action is prevented; true, otherwise
        bool Dispatch(const GFx::EventId& evtId, GFx::DisplayObject* dispObject);
        // return false, if default action is prevented; true, otherwise
        bool Dispatch(Instances::fl_events::Event* evtObj, GFx::DisplayObject* dispObject);
        // return false, if default action is prevented; true, otherwise.
        // If dispObject is specified (can be NULL) and exception is thrown then
        // "ExecutionAborted" flag is set for it.
        bool DispatchToTarget(const ASString& type, Instances::fl::Object* target, bool useCapture,
                              GFx::DisplayObject* dispObject);

        bool MayHaveEnterFrameHandler() const;
        bool MayHaveFrameConstructedHandler() const;
        bool MayHaveExitFrameHandler() const;
        // Checks if 'this' has a listener for the specified event
        bool HasEventHandler(const ASString& type, bool useCapture) const;
        bool HasButtonHandlers() const;

        bool MayHaveActivateHandler() const;
        bool MayHaveDeactivateHandler() const;
        bool MayHaveRenderHandler() const;

        // Checks if 'this' OR ANY OF ITS PARENTS (in the case of display object) has the event
        bool WillTrigger(const ASString& type, bool useCapture) const;

        // removes all listeners which functions belong to the specified moviedefimpl
        void RemoveListenersForMovieDef(MovieDefImpl*);
    protected:
        void RemoveListenersForMovieDef(MovieDefImpl* defimpl, ListenersHash& hash);
        void DoDispatchEvent(Event* evt, GFx::DisplayObject* dispObject);
        void CaptureEventFlow(GFx::DisplayObject* dispObject, 
            EventFlowArrayType& evtFlowArray);
        // returns false if propagation was stopped
        bool ExecuteCapturePhase(
            Instances::fl_events::Event* evtObj, EventFlowArrayType& evtFlowArray);
        // returns false if propagation was stopped
        bool ExecuteBubblePhase(
            Instances::fl_events::Event* evtObj, EventFlowArrayType& evtFlowArray);
        // returns false if exception was thrown during the execution of the event.
        // Use evtObj->IsPropagationStopped() to determine if propagation was stopped
        bool DispatchSingleEvent(Instances::fl_events::Event* evtObj, bool useCapture);
        void OnRemoveEventListener(const ASString& evtType, bool useCapture, UPInt listenersArrSize);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_addEventListener, 
            mid_dispatchEvent, 
            mid_hasEventListener, 
            mid_removeEventListener, 
            mid_willTrigger, 
        };
        void addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference);
        void dispatchEvent(bool& result, Instances::fl_events::Event* event);
        void hasEventListener(bool& result, const ASString& type);
        void removeEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture);
        void willTrigger(bool& result, const ASString& type);

        // C++ friendly wrappers for AS3 methods.
        void addEventListener(const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
        {
            addEventListener(Value::GetUndefined(), type, listener, useCapture, priority, useWeakReference);
        }
        bool dispatchEvent(Instances::fl_events::Event* event)
        {
            bool result;
            dispatchEvent(result, event);
            return result;
        }
        bool hasEventListener(const ASString& type)
        {
            bool result;
            hasEventListener(result, type);
            return result;
        }
        void removeEventListener(const ASString& type, const Value& listener, bool useCapture)
        {
            removeEventListener(Value::GetUndefined(), type, listener, useCapture);
        }
        bool willTrigger(const ASString& type)
        {
            bool result;
            willTrigger(result, type);
            return result;
        }

//##protect##"instance$data"
        // We need an explicit reference to VM because there is no way to get
        // it via regular call GetVM() or GetVMSafe() inside of
        // ForEachChild_GC(). (Traits may be deleted at this time)
        AS3::VM&                        VMRef;
        AutoPtr<EventDispatcherImpl>    pImpl;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class EventDispatcher : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::EventDispatcher"; }
#endif
    public:
        typedef Instances::fl_events::EventDispatcher InstanceType;

    public:
        EventDispatcher(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::EventDispatcher> MakeInstance(EventDispatcher& t)
        {
            return Pickable<Instances::fl_events::EventDispatcher>(new(t.Alloc()) Instances::fl_events::EventDispatcher(t));
        }
        SPtr<Instances::fl_events::EventDispatcher> MakeInstanceS(EventDispatcher& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 5 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[16];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class EventDispatcher : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::EventDispatcher"; }
#endif
    public:
        typedef Classes::fl_events::EventDispatcher ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        EventDispatcher(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class EventDispatcher : public Class
    {
        friend class ClassTraits::fl_events::EventDispatcher;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::EventDispatcherTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::EventDispatcher"; }
#endif
    public:
        typedef EventDispatcher SelfType;
        typedef EventDispatcher ClassType;
        
    private:
        EventDispatcher(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        void CacheEventTraits();

        SPtr<Instances::fl_events::Event>           CreateEventObject(const ASString& type, bool bubbles = false, bool cancelable = false);
        SPtr<Instances::fl_events::MouseEvent>      CreateMouseEventObject(
            const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
        SPtr<Instances::fl_events::KeyboardEvent>   CreateKeyboardEventObject(
            const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
        SPtr<Instances::fl_events::ProgressEvent>   CreateProgressEventObject(const ASString& type);
#ifdef SF_AS3_VERSION_AIR
        SPtr<Instances::fl_events::OutputProgressEvent>   CreateOutputProgressEventObject(const ASString& type);
#endif
        SPtr<Instances::fl_events::FocusEvent>      CreateFocusEventObject(const ASString& type, 
            Instances::fl_display::DisplayObject* relatedObj, unsigned controllerIdx, bool shiftKey = false, unsigned keyCode = 0);
        SPtr<Instances::fl_events::IOErrorEvent>    CreateIOErrorEventObject(const char* errText);
#ifdef GFX_ENABLE_ANALOG_GAMEPAD
        SPtr<Instances::fl_gfx::GamePadAnalogEvent> CreateGamePadAnalogEventObject(
            const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
#endif
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
        SPtr<Instances::fl_events::TouchEvent>      CreateTouchEventObject(
            const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
        SPtr<Instances::fl_events::GestureEvent>    CreateGestureEventObject(
            const GFx::EventId& evtId, Instances::fl::Object* target);
#endif
        SPtr<Instances::fl_events::NetStatusEvent>  CreateNetStatusEvent(const ASString& code, const ASString& level);
        SPtr<Instances::fl_gfx::MouseCursorEvent> CreateMouseCursorEventObject(
            const ASString& cursor, unsigned controllerIdx);

#if defined(GFX_ENABLE_MOBILE_APP_SUPPORT)
        SPtr<Instances::fl_events::StageOrientationEvent> CreateStageOrientationEventObject(
            const ASString& type, bool bubbles, bool cancelable, const ASString& beforeOr, const ASString& afterOr);
        SPtr<Instances::fl_events::AppLifecycleEvent> CreateAppLifecycleEventObject(
            const ASString& type, bool bubbles, bool cancelable, const ASString& event);
#endif
		SPtr<Instances::fl_events::StatusEvent> CreateStatusEventObject(
			const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);

		SPtr<Instances::fl_events::AccelerometerEvent> CreateAccelerometerEventObject(
			const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
		SPtr<Instances::fl_events::GeolocationEvent> CreateGeolocationEventObject(
			const GFx::EventId& evtId, const ASString& type, Instances::fl::Object* target);
//##protect##"class_$methods"

//##protect##"class_$data"
        SPtr<InstanceTraits::Traits> EventTraits;
        SPtr<InstanceTraits::Traits> MouseEventTraits;
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

