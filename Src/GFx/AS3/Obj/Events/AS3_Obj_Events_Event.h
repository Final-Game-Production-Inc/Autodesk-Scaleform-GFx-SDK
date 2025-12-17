//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_Event.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_Event_H
#define INC_AS3_Obj_Events_Event_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class Event;
}}

namespace InstanceTraits { namespace fl_events
{
    class Event;
}}

namespace Classes { namespace fl_events
{
    class Event;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class Event : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::Event;
        
    public:
        typedef Event SelfType;
        typedef Classes::fl_events::Event ClassType;
        typedef InstanceTraits::fl_events::Event TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::EventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::Event"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Event(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        void AS3Constructor(unsigned argc, const Value* argv);
        virtual const char*     GetEventName() const { return "Event"; }
        virtual AS3::Object*    GetEventClass() const;
    public:
        ~Event();
        bool IsPropagationStopped() const    { return PropagationStopped || ImmPropagationStopped; }
        bool IsImmPropagationStopped() const { return ImmPropagationStopped; }
        bool IsDefaultPrevented() const      { return DefaultPrevented; }

        virtual SPtr<Instances::fl_events::Event> Clone() const;
        void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

        // returns necessity of cloning for dispatch.
        bool NeedsCloning() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bubblesGet, 
            mid_cancelableGet, 
            mid_currentTargetGet, 
            mid_eventPhaseGet, 
            mid_targetGet, 
            mid_typeGet, 
            mid_clone, 
            mid_formatToString, 
            mid_isDefaultPrevented, 
            mid_preventDefault, 
            mid_stopImmediatePropagation, 
            mid_stopPropagation, 
            mid_toString, 
        };
        void bubblesGet(bool& result);
        void cancelableGet(bool& result);
        void currentTargetGet(SPtr<Instances::fl::Object>& result);
        void eventPhaseGet(UInt32& result);
        void targetGet(SPtr<Instances::fl::Object>& result);
        void typeGet(ASString& result);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void formatToString(Value& result, unsigned argc, const Value* const argv);
        void isDefaultPrevented(bool& result);
        void preventDefault(const Value& result);
        void stopImmediatePropagation(const Value& result);
        void stopPropagation(const Value& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        bool bubblesGet()
        {
            bool result;
            bubblesGet(result);
            return result;
        }
        bool cancelableGet()
        {
            bool result;
            cancelableGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> currentTargetGet()
        {
            SPtr<Instances::fl::Object> result;
            currentTargetGet(result);
            return result;
        }
        UInt32 eventPhaseGet()
        {
            UInt32 result;
            eventPhaseGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> targetGet()
        {
            SPtr<Instances::fl::Object> result;
            targetGet(result);
            return result;
        }
        ASString typeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            typeGet(result);
            return result;
        }
        SPtr<Instances::fl_events::Event> clone()
        {
            SPtr<Instances::fl_events::Event> result;
            clone(result);
            return result;
        }
        bool isDefaultPrevented()
        {
            bool result;
            isDefaultPrevented(result);
            return result;
        }
        void preventDefault()
        {
            preventDefault(Value::GetUndefined());
        }
        void stopImmediatePropagation()
        {
            stopImmediatePropagation(Value::GetUndefined());
        }
        void stopPropagation()
        {
            stopPropagation(Value::GetUndefined());
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
    public:
        ASString        Type;
        SPtr<Object>    CurrentTarget;
        SPtr<Object>    Target;
        enum
        {
            Phase_Capture = 1,
            Phase_Target  = 2,
            Phase_Bubble  = 3
        };
        unsigned        Phase;
        bool            Bubbles             :1;
        bool            Cancelable          :1;
        bool            DefaultPrevented    :1;
        bool            PropagationStopped  :1;
        bool            ImmPropagationStopped  :1;
        bool            WasDispatched       :1;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class Event : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Event"; }
#endif
    public:
        typedef Instances::fl_events::Event InstanceType;

    public:
        Event(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::Event> MakeInstance(Event& t)
        {
            return Pickable<Instances::fl_events::Event>(new(t.Alloc()) Instances::fl_events::Event(t));
        }
        SPtr<Instances::fl_events::Event> MakeInstanceS(Event& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 13 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class Event : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Event"; }
#endif
    public:
        typedef Classes::fl_events::Event ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Event(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 38 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class Event : public Class
    {
        friend class ClassTraits::fl_events::Event;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::EventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Event"; }
#endif
    public:
        typedef Event SelfType;
        typedef Event ClassType;
        
    private:
        Event(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ACTIVATE;
        const char* ADDED;
        const char* ADDED_TO_STAGE;
        const char* CANCEL;
        const char* CHANGE;
        const char* CLOSE;
        const char* CLOSING;
        const char* COMPLETE;
        const char* CONNECT;
        const char* DEACTIVATE;
        const char* DISPLAYING;
        const char* ENTER_FRAME;
        const char* EXITING;
        const char* EXIT_FRAME;
        const char* FRAME_CONSTRUCTED;
        const char* FULLSCREEN;
        const char* HTML_BOUNDS_CHANGE;
        const char* HTML_DOM_INITIALIZE;
        const char* HTML_RENDER;
        const char* ID3;
        const char* INIT;
        const char* LOCATION_CHANGE;
        const char* MOUSE_LEAVE;
        const char* NETWORK_CHANGE;
        const char* OPEN;
        const char* REMOVED;
        const char* REMOVED_FROM_STAGE;
        const char* RENDER;
        const char* RESIZE;
        const char* SCROLL;
        const char* SELECT;
        const char* SOUND_COMPLETE;
        const char* TAB_CHILDREN_CHANGE;
        const char* TAB_ENABLED_CHANGE;
        const char* TAB_INDEX_CHANGE;
        const char* UNLOAD;
        const char* USER_IDLE;
        const char* USER_PRESENT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

