//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_StatusEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_StatusEvent_H
#define INC_AS3_Obj_Events_StatusEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo StatusEventTI;
    extern const ClassInfo StatusEventCI;
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
    class StatusEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class StatusEvent;
}}

namespace Classes { namespace fl_events
{
    class StatusEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class StatusEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::StatusEvent;
        
    public:
        typedef StatusEvent SelfType;
        typedef Classes::fl_events::StatusEvent ClassType;
        typedef InstanceTraits::fl_events::StatusEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::StatusEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::StatusEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        StatusEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"

		virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
    public:
        void SetCode(const ASString& t) { Code = t; }
        void SetCode(wchar_t ch);
        void SetCode(const wchar_t* ch);

		void SetLevel(const ASString& t) { Level = t; }
        void SetLevel(wchar_t ch);
        void SetLevel(const wchar_t* ch);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_codeGet, 
            mid_codeSet, 
            mid_levelGet, 
            mid_levelSet, 
            mid_clone, 
            mid_toString, 
        };
        void codeGet(ASString& result);
        void codeSet(const Value& result, const ASString& value);
        void levelGet(ASString& result);
        void levelSet(const Value& result, const ASString& value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString codeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            codeGet(result);
            return result;
        }
        void codeSet(const ASString& value)
        {
            codeSet(Value::GetUndefined(), value);
        }
        ASString levelGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            levelGet(result);
            return result;
        }
        void levelSet(const ASString& value)
        {
            levelSet(Value::GetUndefined(), value);
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
    class StatusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::StatusEvent"; }
#endif
    public:
        typedef Instances::fl_events::StatusEvent InstanceType;

    public:
        StatusEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::StatusEvent> MakeInstance(StatusEvent& t)
        {
            return Pickable<Instances::fl_events::StatusEvent>(new(t.Alloc()) Instances::fl_events::StatusEvent(t));
        }
        SPtr<Instances::fl_events::StatusEvent> MakeInstanceS(StatusEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[8];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class StatusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StatusEvent"; }
#endif
    public:
        typedef Classes::fl_events::StatusEvent ClassType;
        typedef InstanceTraits::fl_events::StatusEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StatusEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class StatusEvent : public Class
    {
        friend class ClassTraits::fl_events::StatusEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::StatusEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::StatusEvent"; }
#endif
    public:
        typedef StatusEvent SelfType;
        typedef StatusEvent ClassType;
        
    private:
        StatusEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* STATUS;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

