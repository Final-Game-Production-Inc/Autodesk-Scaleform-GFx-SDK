//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ProgressEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_ProgressEvent_H
#define INC_AS3_Obj_Events_ProgressEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo ProgressEventTI;
    extern const ClassInfo ProgressEventCI;
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
    class ProgressEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class ProgressEvent;
}}

namespace Classes { namespace fl_events
{
    class ProgressEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class ProgressEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::ProgressEvent;
        
    public:
        typedef ProgressEvent SelfType;
        typedef Classes::fl_events::ProgressEvent ClassType;
        typedef InstanceTraits::fl_events::ProgressEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::ProgressEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::ProgressEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ProgressEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        void SetBytesTotal(UPInt t)  { BytesTotal = t; }
        void SetBytesLoaded(UPInt t) { BytesLoaded = t; }

        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bytesLoadedGet, 
            mid_bytesLoadedSet, 
            mid_bytesTotalGet, 
            mid_bytesTotalSet, 
            mid_clone, 
            mid_toString, 
        };
        void bytesLoadedGet(Value::Number& result);
        void bytesLoadedSet(const Value& result, Value::Number value);
        void bytesTotalGet(Value::Number& result);
        void bytesTotalSet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number bytesLoadedGet()
        {
            Value::Number result;
            bytesLoadedGet(result);
            return result;
        }
        void bytesLoadedSet(Value::Number value)
        {
            bytesLoadedSet(Value::GetUndefined(), value);
        }
        Value::Number bytesTotalGet()
        {
            Value::Number result;
            bytesTotalGet(result);
            return result;
        }
        void bytesTotalSet(Value::Number value)
        {
            bytesTotalSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
    protected:
        UPInt BytesLoaded;
        UPInt BytesTotal;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class ProgressEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ProgressEvent"; }
#endif
    public:
        typedef Instances::fl_events::ProgressEvent InstanceType;

    public:
        ProgressEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::ProgressEvent> MakeInstance(ProgressEvent& t)
        {
            return Pickable<Instances::fl_events::ProgressEvent>(new(t.Alloc()) Instances::fl_events::ProgressEvent(t));
        }
        SPtr<Instances::fl_events::ProgressEvent> MakeInstanceS(ProgressEvent& t)
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
    class ProgressEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ProgressEvent"; }
#endif
    public:
        typedef Classes::fl_events::ProgressEvent ClassType;
        typedef InstanceTraits::fl_events::ProgressEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ProgressEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class ProgressEvent : public Class
    {
        friend class ClassTraits::fl_events::ProgressEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::ProgressEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ProgressEvent"; }
#endif
    public:
        typedef ProgressEvent SelfType;
        typedef ProgressEvent ClassType;
        
    private:
        ProgressEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* PROGRESS;
        const char* SOCKET_DATA;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

