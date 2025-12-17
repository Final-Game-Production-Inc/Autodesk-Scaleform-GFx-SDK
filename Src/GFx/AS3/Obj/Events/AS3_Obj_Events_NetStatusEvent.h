//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_NetStatusEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_NetStatusEvent_H
#define INC_AS3_Obj_Events_NetStatusEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo NetStatusEventTI;
    extern const ClassInfo NetStatusEventCI;
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
    class NetStatusEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class NetStatusEvent;
}}

namespace Classes { namespace fl_events
{
    class NetStatusEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class NetStatusEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::NetStatusEvent;
        
    public:
        typedef NetStatusEvent SelfType;
        typedef Classes::fl_events::NetStatusEvent ClassType;
        typedef InstanceTraits::fl_events::NetStatusEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::NetStatusEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::NetStatusEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NetStatusEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        virtual SPtr<Instances::fl_events::Event> Clone() const;
        virtual const char* GetEventName() const;
        virtual AS3::Object* GetEventClass() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_infoGet, 
            mid_infoSet, 
            mid_clone, 
            mid_toString, 
        };
        void infoGet(SPtr<Instances::fl::Object>& result);
        void infoSet(const Value& result, const Value& value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> infoGet()
        {
            SPtr<Instances::fl::Object> result;
            infoGet(result);
            return result;
        }
        void infoSet(const Value& value)
        {
            infoSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        ASString Code;
        ASString Level;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class NetStatusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NetStatusEvent"; }
#endif
    public:
        typedef Instances::fl_events::NetStatusEvent InstanceType;

    public:
        NetStatusEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::NetStatusEvent> MakeInstance(NetStatusEvent& t)
        {
            return Pickable<Instances::fl_events::NetStatusEvent>(new(t.Alloc()) Instances::fl_events::NetStatusEvent(t));
        }
        SPtr<Instances::fl_events::NetStatusEvent> MakeInstanceS(NetStatusEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class NetStatusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NetStatusEvent"; }
#endif
    public:
        typedef Classes::fl_events::NetStatusEvent ClassType;
        typedef InstanceTraits::fl_events::NetStatusEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NetStatusEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class NetStatusEvent : public Class
    {
        friend class ClassTraits::fl_events::NetStatusEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::NetStatusEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NetStatusEvent"; }
#endif
    public:
        typedef NetStatusEvent SelfType;
        typedef NetStatusEvent ClassType;
        
    private:
        NetStatusEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* NET_STATUS;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

