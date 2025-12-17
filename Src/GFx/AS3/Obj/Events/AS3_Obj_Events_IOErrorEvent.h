//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_IOErrorEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_IOErrorEvent_H
#define INC_AS3_Obj_Events_IOErrorEvent_H

#include "AS3_Obj_Events_ErrorEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo IOErrorEventTI;
    extern const ClassInfo IOErrorEventCI;
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
    class IOErrorEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class IOErrorEvent;
}}

namespace Classes { namespace fl_events
{
    class IOErrorEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class IOErrorEvent : public Instances::fl_events::ErrorEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::IOErrorEvent;
        
    public:
        typedef IOErrorEvent SelfType;
        typedef Classes::fl_events::IOErrorEvent ClassType;
        typedef InstanceTraits::fl_events::IOErrorEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::IOErrorEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::IOErrorEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        IOErrorEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_textGet, 
            mid_textSet, 
            mid_clone, 
            mid_toString, 
        };
        void textGet(ASString& result);
        void textSet(const Value& result, const ASString& value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString textGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            textGet(result);
            return result;
        }
        void textSet(const ASString& value)
        {
            textSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        ASString Text;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class IOErrorEvent : public fl_events::ErrorEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::IOErrorEvent"; }
#endif
    public:
        typedef Instances::fl_events::IOErrorEvent InstanceType;

    public:
        IOErrorEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::IOErrorEvent> MakeInstance(IOErrorEvent& t)
        {
            return Pickable<Instances::fl_events::IOErrorEvent>(new(t.Alloc()) Instances::fl_events::IOErrorEvent(t));
        }
        SPtr<Instances::fl_events::IOErrorEvent> MakeInstanceS(IOErrorEvent& t)
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
    class IOErrorEvent : public fl_events::ErrorEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IOErrorEvent"; }
#endif
    public:
        typedef Classes::fl_events::IOErrorEvent ClassType;
        typedef InstanceTraits::fl_events::IOErrorEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IOErrorEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class IOErrorEvent : public Class
    {
        friend class ClassTraits::fl_events::IOErrorEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::IOErrorEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IOErrorEvent"; }
#endif
    public:
        typedef IOErrorEvent SelfType;
        typedef IOErrorEvent ClassType;
        
    private:
        IOErrorEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* IO_ERROR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

