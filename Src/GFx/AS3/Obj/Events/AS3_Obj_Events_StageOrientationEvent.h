//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_StageOrientationEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_StageOrientationEvent_H
#define INC_AS3_Obj_Events_StageOrientationEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo StageOrientationEventTI;
    extern const ClassInfo StageOrientationEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class StageOrientationEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class StageOrientationEvent;
}}

namespace Classes { namespace fl_events
{
    class StageOrientationEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class StageOrientationEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::StageOrientationEvent;
        
    public:
        typedef StageOrientationEvent SelfType;
        typedef Classes::fl_events::StageOrientationEvent ClassType;
        typedef InstanceTraits::fl_events::StageOrientationEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::StageOrientationEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::StageOrientationEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        StageOrientationEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_afterOrientationGet, 
            mid_beforeOrientationGet, 
            mid_clone, 
            mid_toString, 
        };
        void afterOrientationGet(ASString& result);
        void beforeOrientationGet(ASString& result);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString afterOrientationGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            afterOrientationGet(result);
            return result;
        }
        ASString beforeOrientationGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            beforeOrientationGet(result);
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
        Value BeforeOrientation;
        Value AfterOrientation;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class StageOrientationEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::StageOrientationEvent"; }
#endif
    public:
        typedef Instances::fl_events::StageOrientationEvent InstanceType;

    public:
        StageOrientationEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::StageOrientationEvent> MakeInstance(StageOrientationEvent& t)
        {
            return Pickable<Instances::fl_events::StageOrientationEvent>(new(t.Alloc()) Instances::fl_events::StageOrientationEvent(t));
        }
        SPtr<Instances::fl_events::StageOrientationEvent> MakeInstanceS(StageOrientationEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[4];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class StageOrientationEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StageOrientationEvent"; }
#endif
    public:
        typedef Classes::fl_events::StageOrientationEvent ClassType;
        typedef InstanceTraits::fl_events::StageOrientationEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StageOrientationEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class StageOrientationEvent : public Class
    {
        friend class ClassTraits::fl_events::StageOrientationEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::StageOrientationEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::StageOrientationEvent"; }
#endif
    public:
        typedef StageOrientationEvent SelfType;
        typedef StageOrientationEvent ClassType;
        
    private:
        StageOrientationEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ORIENTATION_CHANGE;
        const char* ORIENTATION_CHANGING;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

