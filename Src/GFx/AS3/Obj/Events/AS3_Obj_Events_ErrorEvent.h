//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ErrorEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_ErrorEvent_H
#define INC_AS3_Obj_Events_ErrorEvent_H

#include "AS3_Obj_Events_TextEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo ErrorEventTI;
    extern const ClassInfo ErrorEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class ErrorEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class ErrorEvent;
}}

namespace Classes { namespace fl_events
{
    class ErrorEvent;
}}

//##protect##"forward_declaration"
#ifdef ERROR
#undef ERROR
#endif
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class ErrorEvent : public Instances::fl_events::TextEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::ErrorEvent;
        
    public:
        typedef ErrorEvent SelfType;
        typedef Classes::fl_events::ErrorEvent ClassType;
        typedef InstanceTraits::fl_events::ErrorEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::ErrorEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::ErrorEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ErrorEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
         void AS3Constructor(unsigned argc, const Value* argv);
         virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_errorIDGet, 
            mid_clone, 
            mid_toString, 
        };
        void errorIDGet(SInt32& result);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 errorIDGet()
        {
            SInt32 result;
            errorIDGet(result);
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
        int errorID; 
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class ErrorEvent : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ErrorEvent"; }
#endif
    public:
        typedef Instances::fl_events::ErrorEvent InstanceType;

    public:
        ErrorEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::ErrorEvent> MakeInstance(ErrorEvent& t)
        {
            return Pickable<Instances::fl_events::ErrorEvent>(new(t.Alloc()) Instances::fl_events::ErrorEvent(t));
        }
        SPtr<Instances::fl_events::ErrorEvent> MakeInstanceS(ErrorEvent& t)
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
    class ErrorEvent : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ErrorEvent"; }
#endif
    public:
        typedef Classes::fl_events::ErrorEvent ClassType;
        typedef InstanceTraits::fl_events::ErrorEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ErrorEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class ErrorEvent : public Class
    {
        friend class ClassTraits::fl_events::ErrorEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::ErrorEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ErrorEvent"; }
#endif
    public:
        typedef ErrorEvent SelfType;
        typedef ErrorEvent ClassType;
        
    private:
        ErrorEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ERROR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

