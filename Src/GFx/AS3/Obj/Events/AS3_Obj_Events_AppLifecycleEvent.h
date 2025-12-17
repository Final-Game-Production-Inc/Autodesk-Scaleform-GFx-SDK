//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_AppLifecycleEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_AppLifecycleEvent_H
#define INC_AS3_Obj_Events_AppLifecycleEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo AppLifecycleEventTI;
    extern const ClassInfo AppLifecycleEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class AppLifecycleEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class AppLifecycleEvent;
}}

namespace Classes { namespace fl_events
{
    class AppLifecycleEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class AppLifecycleEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::AppLifecycleEvent;
        
    public:
        typedef AppLifecycleEvent SelfType;
        typedef Classes::fl_events::AppLifecycleEvent ClassType;
        typedef InstanceTraits::fl_events::AppLifecycleEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AppLifecycleEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::AppLifecycleEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AppLifecycleEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_statusGet, 
            mid_clone, 
            mid_toString, 
        };
        void statusGet(SPtr<Instances::fl::Object>& result);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> statusGet()
        {
            SPtr<Instances::fl::Object> result;
            statusGet(result);
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
        Value Status;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class AppLifecycleEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AppLifecycleEvent"; }
#endif
    public:
        typedef Instances::fl_events::AppLifecycleEvent InstanceType;

    public:
        AppLifecycleEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::AppLifecycleEvent> MakeInstance(AppLifecycleEvent& t)
        {
            return Pickable<Instances::fl_events::AppLifecycleEvent>(new(t.Alloc()) Instances::fl_events::AppLifecycleEvent(t));
        }
        SPtr<Instances::fl_events::AppLifecycleEvent> MakeInstanceS(AppLifecycleEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class AppLifecycleEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AppLifecycleEvent"; }
#endif
    public:
        typedef Classes::fl_events::AppLifecycleEvent ClassType;
        typedef InstanceTraits::fl_events::AppLifecycleEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AppLifecycleEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class AppLifecycleEvent : public Class
    {
        friend class ClassTraits::fl_events::AppLifecycleEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AppLifecycleEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::AppLifecycleEvent"; }
#endif
    public:
        typedef AppLifecycleEvent SelfType;
        typedef AppLifecycleEvent ClassType;
        
    private:
        AppLifecycleEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ONPAUSE;
        const char* ONRESUME;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

