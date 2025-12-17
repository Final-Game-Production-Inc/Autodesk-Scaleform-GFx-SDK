//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_TransformGestureEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_TransformGestureEvent_H
#define INC_AS3_Obj_Events_TransformGestureEvent_H

#include "AS3_Obj_Events_GestureEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo TransformGestureEventTI;
    extern const ClassInfo TransformGestureEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class TransformGestureEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class TransformGestureEvent;
}}

namespace Classes { namespace fl_events
{
    class TransformGestureEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class TransformGestureEvent : public Instances::fl_events::GestureEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::TransformGestureEvent;
        
    public:
        typedef TransformGestureEvent SelfType;
        typedef Classes::fl_events::TransformGestureEvent ClassType;
        typedef InstanceTraits::fl_events::TransformGestureEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TransformGestureEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::TransformGestureEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TransformGestureEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
		void InitLocalCoords();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_offsetXGet, 
            mid_offsetXSet, 
            mid_offsetYGet, 
            mid_offsetYSet, 
            mid_rotationGet, 
            mid_rotationSet, 
            mid_scaleXGet, 
            mid_scaleXSet, 
            mid_scaleYGet, 
            mid_scaleYSet, 
            mid_clone, 
            mid_toString, 
        };
        void offsetXGet(Value::Number& result);
        void offsetXSet(const Value& result, Value::Number value);
        void offsetYGet(Value::Number& result);
        void offsetYSet(const Value& result, Value::Number value);
        void rotationGet(Value::Number& result);
        void rotationSet(const Value& result, Value::Number value);
        void scaleXGet(Value::Number& result);
        void scaleXSet(const Value& result, Value::Number value);
        void scaleYGet(Value::Number& result);
        void scaleYSet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number offsetXGet()
        {
            Value::Number result;
            offsetXGet(result);
            return result;
        }
        void offsetXSet(Value::Number value)
        {
            offsetXSet(Value::GetUndefined(), value);
        }
        Value::Number offsetYGet()
        {
            Value::Number result;
            offsetYGet(result);
            return result;
        }
        void offsetYSet(Value::Number value)
        {
            offsetYSet(Value::GetUndefined(), value);
        }
        Value::Number rotationGet()
        {
            Value::Number result;
            rotationGet(result);
            return result;
        }
        void rotationSet(Value::Number value)
        {
            rotationSet(Value::GetUndefined(), value);
        }
        Value::Number scaleXGet()
        {
            Value::Number result;
            scaleXGet(result);
            return result;
        }
        void scaleXSet(Value::Number value)
        {
            scaleXSet(Value::GetUndefined(), value);
        }
        Value::Number scaleYGet()
        {
            Value::Number result;
            scaleYGet(result);
            return result;
        }
        void scaleYSet(Value::Number value)
        {
            scaleYSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        Value::Number	OffsetX, OffsetY;
		Value::Number	LocalOffsetX, LocalOffsetY;
        Value::Number	ScaleX, ScaleY;
        Value::Number	Rotation;
		bool			LocalInitialized;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class TransformGestureEvent : public fl_events::GestureEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TransformGestureEvent"; }
#endif
    public:
        typedef Instances::fl_events::TransformGestureEvent InstanceType;

    public:
        TransformGestureEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::TransformGestureEvent> MakeInstance(TransformGestureEvent& t)
        {
            return Pickable<Instances::fl_events::TransformGestureEvent>(new(t.Alloc()) Instances::fl_events::TransformGestureEvent(t));
        }
        SPtr<Instances::fl_events::TransformGestureEvent> MakeInstanceS(TransformGestureEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[17];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class TransformGestureEvent : public fl_events::GestureEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TransformGestureEvent"; }
#endif
    public:
        typedef Classes::fl_events::TransformGestureEvent ClassType;
        typedef InstanceTraits::fl_events::TransformGestureEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TransformGestureEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class TransformGestureEvent : public Class
    {
        friend class ClassTraits::fl_events::TransformGestureEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TransformGestureEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TransformGestureEvent"; }
#endif
    public:
        typedef TransformGestureEvent SelfType;
        typedef TransformGestureEvent ClassType;
        
    private:
        TransformGestureEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* GESTURE_PAN;
        const char* GESTURE_ROTATE;
        const char* GESTURE_SWIPE;
        const char* GESTURE_ZOOM;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

