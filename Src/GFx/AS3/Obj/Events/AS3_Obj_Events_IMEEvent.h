//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_IMEEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_IMEEvent_H
#define INC_AS3_Obj_Events_IMEEvent_H

#include "AS3_Obj_Events_TextEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo IMEEventTI;
    extern const ClassInfo IMEEventCI;
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
    class IMEEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class IMEEvent;
}}

namespace Classes { namespace fl_events
{
    class IMEEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class IMEEvent : public Instances::fl_events::TextEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::IMEEvent;
        
    public:
        typedef IMEEvent SelfType;
        typedef Classes::fl_events::IMEEvent ClassType;
        typedef InstanceTraits::fl_events::IMEEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::IMEEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::IMEEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        IMEEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clone, 
            mid_toString, 
        };
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class IMEEvent : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::IMEEvent"; }
#endif
    public:
        typedef Instances::fl_events::IMEEvent InstanceType;

    public:
        IMEEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::IMEEvent> MakeInstance(IMEEvent& t)
        {
            return Pickable<Instances::fl_events::IMEEvent>(new(t.Alloc()) Instances::fl_events::IMEEvent(t));
        }
        SPtr<Instances::fl_events::IMEEvent> MakeInstanceS(IMEEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

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
    class IMEEvent : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IMEEvent"; }
#endif
    public:
        typedef Classes::fl_events::IMEEvent ClassType;
        typedef InstanceTraits::fl_events::IMEEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IMEEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class IMEEvent : public Class
    {
        friend class ClassTraits::fl_events::IMEEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::IMEEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IMEEvent"; }
#endif
    public:
        typedef IMEEvent SelfType;
        typedef IMEEvent ClassType;
        
    private:
        IMEEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* IME_COMPOSITION;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

