//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_PressAndTapGestureEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_PressAndTapGestureEvent_H
#define INC_AS3_Obj_Events_PressAndTapGestureEvent_H

#include "AS3_Obj_Events_GestureEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo PressAndTapGestureEventTI;
    extern const ClassInfo PressAndTapGestureEventCI;
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
    class PressAndTapGestureEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class PressAndTapGestureEvent;
}}

namespace Classes { namespace fl_events
{
    class PressAndTapGestureEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class PressAndTapGestureEvent : public Instances::fl_events::GestureEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::PressAndTapGestureEvent;
        
    public:
        typedef PressAndTapGestureEvent SelfType;
        typedef Classes::fl_events::PressAndTapGestureEvent ClassType;
        typedef InstanceTraits::fl_events::PressAndTapGestureEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::PressAndTapGestureEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::PressAndTapGestureEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        PressAndTapGestureEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
        void InitLocalCoords();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_tapLocalXGet, 
            mid_tapLocalXSet, 
            mid_tapLocalYGet, 
            mid_tapLocalYSet, 
            mid_tapStageXGet, 
            mid_tapStageYGet, 
            mid_clone, 
            mid_toString, 
        };
        void tapLocalXGet(Value::Number& result);
        void tapLocalXSet(const Value& result, Value::Number value);
        void tapLocalYGet(Value::Number& result);
        void tapLocalYSet(const Value& result, Value::Number value);
        void tapStageXGet(Value::Number& result);
        void tapStageYGet(Value::Number& result);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number tapLocalXGet()
        {
            Value::Number result;
            tapLocalXGet(result);
            return result;
        }
        void tapLocalXSet(Value::Number value)
        {
            tapLocalXSet(Value::GetUndefined(), value);
        }
        Value::Number tapLocalYGet()
        {
            Value::Number result;
            tapLocalYGet(result);
            return result;
        }
        void tapLocalYSet(Value::Number value)
        {
            tapLocalYSet(Value::GetUndefined(), value);
        }
        Value::Number tapStageXGet()
        {
            Value::Number result;
            tapStageXGet(result);
            return result;
        }
        Value::Number tapStageYGet()
        {
            Value::Number result;
            tapStageYGet(result);
            return result;
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        Value::Number   TapLocalX, TapLocalY;
        Value::Number   TapStageX, TapStageY;
        bool            LocalInitialized;
		bool			TapStageSet;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class PressAndTapGestureEvent : public fl_events::GestureEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::PressAndTapGestureEvent"; }
#endif
    public:
        typedef Instances::fl_events::PressAndTapGestureEvent InstanceType;

    public:
        PressAndTapGestureEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::PressAndTapGestureEvent> MakeInstance(PressAndTapGestureEvent& t)
        {
            return Pickable<Instances::fl_events::PressAndTapGestureEvent>(new(t.Alloc()) Instances::fl_events::PressAndTapGestureEvent(t));
        }
        SPtr<Instances::fl_events::PressAndTapGestureEvent> MakeInstanceS(PressAndTapGestureEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 8 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class PressAndTapGestureEvent : public fl_events::GestureEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::PressAndTapGestureEvent"; }
#endif
    public:
        typedef Classes::fl_events::PressAndTapGestureEvent ClassType;
        typedef InstanceTraits::fl_events::PressAndTapGestureEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        PressAndTapGestureEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class PressAndTapGestureEvent : public Class
    {
        friend class ClassTraits::fl_events::PressAndTapGestureEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::PressAndTapGestureEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::PressAndTapGestureEvent"; }
#endif
    public:
        typedef PressAndTapGestureEvent SelfType;
        typedef PressAndTapGestureEvent ClassType;
        
    private:
        PressAndTapGestureEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* GESTURE_PRESS_AND_TAP;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

