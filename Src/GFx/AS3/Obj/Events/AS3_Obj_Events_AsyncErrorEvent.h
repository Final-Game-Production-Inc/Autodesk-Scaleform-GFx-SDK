//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_AsyncErrorEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_AsyncErrorEvent_H
#define INC_AS3_Obj_Events_AsyncErrorEvent_H

#include "AS3_Obj_Events_ErrorEvent.h"
//##protect##"includes"
#include "../AS3_Obj_Error.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo AsyncErrorEventTI;
    extern const ClassInfo AsyncErrorEventCI;
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
    class AsyncErrorEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class AsyncErrorEvent;
}}

namespace Classes { namespace fl_events
{
    class AsyncErrorEvent;
}}

//##protect##"forward_declaration"
namespace Instances
{
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class AsyncErrorEvent : public Instances::fl_events::ErrorEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::AsyncErrorEvent;
        
    public:
        typedef AsyncErrorEvent SelfType;
        typedef Classes::fl_events::AsyncErrorEvent ClassType;
        typedef InstanceTraits::fl_events::AsyncErrorEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AsyncErrorEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::AsyncErrorEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AsyncErrorEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        ~AsyncErrorEvent();

    private:
        AsyncErrorEvent(const AsyncErrorEvent&);
        AsyncErrorEvent& operator =(const AsyncErrorEvent&);
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        SPtr<Instances::fl::Error> error;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class AsyncErrorEvent : public fl_events::ErrorEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AsyncErrorEvent"; }
#endif
    public:
        typedef Instances::fl_events::AsyncErrorEvent InstanceType;

    public:
        AsyncErrorEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::AsyncErrorEvent> MakeInstance(AsyncErrorEvent& t)
        {
            return Pickable<Instances::fl_events::AsyncErrorEvent>(new(t.Alloc()) Instances::fl_events::AsyncErrorEvent(t));
        }
        SPtr<Instances::fl_events::AsyncErrorEvent> MakeInstanceS(AsyncErrorEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[2];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class AsyncErrorEvent : public fl_events::ErrorEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AsyncErrorEvent"; }
#endif
    public:
        typedef Classes::fl_events::AsyncErrorEvent ClassType;
        typedef InstanceTraits::fl_events::AsyncErrorEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AsyncErrorEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class AsyncErrorEvent : public Class
    {
        friend class ClassTraits::fl_events::AsyncErrorEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AsyncErrorEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::AsyncErrorEvent"; }
#endif
    public:
        typedef AsyncErrorEvent SelfType;
        typedef AsyncErrorEvent ClassType;
        
    private:
        AsyncErrorEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ASYNC_ERROR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

