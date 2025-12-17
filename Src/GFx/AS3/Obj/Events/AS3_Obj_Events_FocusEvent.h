//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_FocusEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_FocusEvent_H
#define INC_AS3_Obj_Events_FocusEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo FocusEventTI;
    extern const ClassInfo FocusEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_events
{
    class FocusEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class FocusEvent;
}}

namespace Classes { namespace fl_events
{
    class FocusEvent;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class InteractiveObject;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class FocusEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::FocusEvent;
        
    public:
        typedef FocusEvent SelfType;
        typedef Classes::fl_events::FocusEvent ClassType;
        typedef InstanceTraits::fl_events::FocusEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::FocusEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::FocusEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        FocusEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        Instances::fl_display::InteractiveObject* GetRelatedObj() const
        {
            return RelatedObj;
        }
        void SetRelatedObj(Instances::fl_display::InteractiveObject* obj)
        {
            RelatedObj = obj;
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_directionGet, 
            mid_directionSet, 
            mid_keyCodeGet, 
            mid_keyCodeSet, 
            mid_relatedObjectGet, 
            mid_relatedObjectSet, 
            mid_shiftKeyGet, 
            mid_shiftKeySet, 
            mid_clone, 
            mid_toString, 
        };
        void directionGet(ASString& result);
        void directionSet(const Value& result, const ASString& value);
        void keyCodeGet(UInt32& result);
        void keyCodeSet(const Value& result, UInt32 value);
        void relatedObjectGet(SPtr<Instances::fl_display::InteractiveObject>& result);
        void relatedObjectSet(const Value& result, Instances::fl_display::InteractiveObject* value);
        void shiftKeyGet(bool& result);
        void shiftKeySet(const Value& result, bool value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString directionGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            directionGet(result);
            return result;
        }
        void directionSet(const ASString& value)
        {
            directionSet(Value::GetUndefined(), value);
        }
        UInt32 keyCodeGet()
        {
            UInt32 result;
            keyCodeGet(result);
            return result;
        }
        void keyCodeSet(UInt32 value)
        {
            keyCodeSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::InteractiveObject> relatedObjectGet();
        void relatedObjectSet(Instances::fl_display::InteractiveObject* value)
        {
            relatedObjectSet(Value::GetUndefined(), value);
        }
        bool shiftKeyGet()
        {
            bool result;
            shiftKeyGet(result);
            return result;
        }
        void shiftKeySet(bool value)
        {
            shiftKeySet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        bool                                ShiftKey;
        UInt32                              KeyCode;

    protected:
        SPtr<Instances::fl_display::InteractiveObject>  RelatedObj;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class FocusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::FocusEvent"; }
#endif
    public:
        typedef Instances::fl_events::FocusEvent InstanceType;

    public:
        FocusEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::FocusEvent> MakeInstance(FocusEvent& t)
        {
            return Pickable<Instances::fl_events::FocusEvent>(new(t.Alloc()) Instances::fl_events::FocusEvent(t));
        }
        SPtr<Instances::fl_events::FocusEvent> MakeInstanceS(FocusEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 10 };
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
    class FocusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FocusEvent"; }
#endif
    public:
        typedef Classes::fl_events::FocusEvent ClassType;
        typedef InstanceTraits::fl_events::FocusEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FocusEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class FocusEvent : public Class
    {
        friend class ClassTraits::fl_events::FocusEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::FocusEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FocusEvent"; }
#endif
    public:
        typedef FocusEvent SelfType;
        typedef FocusEvent ClassType;
        
    private:
        FocusEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* FOCUS_IN;
        const char* FOCUS_OUT;
        const char* KEY_FOCUS_CHANGE;
        const char* MOUSE_FOCUS_CHANGE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

