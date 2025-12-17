//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent_H
#define INC_AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo HTMLUncaughtScriptExceptionEventTI;
    extern const ClassInfo HTMLUncaughtScriptExceptionEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent;
}}

namespace Classes { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent;
        
    public:
        typedef HTMLUncaughtScriptExceptionEvent SelfType;
        typedef Classes::fl_events::HTMLUncaughtScriptExceptionEvent ClassType;
        typedef InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::HTMLUncaughtScriptExceptionEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::HTMLUncaughtScriptExceptionEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        HTMLUncaughtScriptExceptionEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        Value exceptionValue;
        SPtr<Instances::fl::Array> stackTrace;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::HTMLUncaughtScriptExceptionEvent"; }
#endif
    public:
        typedef Instances::fl_events::HTMLUncaughtScriptExceptionEvent InstanceType;

    public:
        HTMLUncaughtScriptExceptionEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::HTMLUncaughtScriptExceptionEvent> MakeInstance(HTMLUncaughtScriptExceptionEvent& t)
        {
            return Pickable<Instances::fl_events::HTMLUncaughtScriptExceptionEvent>(new(t.Alloc()) Instances::fl_events::HTMLUncaughtScriptExceptionEvent(t));
        }
        SPtr<Instances::fl_events::HTMLUncaughtScriptExceptionEvent> MakeInstanceS(HTMLUncaughtScriptExceptionEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::HTMLUncaughtScriptExceptionEvent"; }
#endif
    public:
        typedef Classes::fl_events::HTMLUncaughtScriptExceptionEvent ClassType;
        typedef InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        HTMLUncaughtScriptExceptionEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class HTMLUncaughtScriptExceptionEvent : public Class
    {
        friend class ClassTraits::fl_events::HTMLUncaughtScriptExceptionEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::HTMLUncaughtScriptExceptionEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::HTMLUncaughtScriptExceptionEvent"; }
#endif
    public:
        typedef HTMLUncaughtScriptExceptionEvent SelfType;
        typedef HTMLUncaughtScriptExceptionEvent ClassType;
        
    private:
        HTMLUncaughtScriptExceptionEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        Value UNCAUGHT_SCRIPT_EXCEPTION;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

