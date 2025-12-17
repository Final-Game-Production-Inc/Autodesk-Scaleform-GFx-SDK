//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_TextEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_TextEvent_H
#define INC_AS3_Obj_Events_TextEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo TextEventTI;
    extern const ClassInfo TextEventCI;
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
    class TextEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class TextEvent;
}}

namespace Classes { namespace fl_events
{
    class TextEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class TextEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::TextEvent;
        
    public:
        typedef TextEvent SelfType;
        typedef Classes::fl_events::TextEvent ClassType;
        typedef InstanceTraits::fl_events::TextEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TextEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::TextEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
    public:
        void SetText(const ASString& t) { Text = t; }
        void SetText(wchar_t ch);
        void SetText(const wchar_t* ch);
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
    class TextEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextEvent"; }
#endif
    public:
        typedef Instances::fl_events::TextEvent InstanceType;

    public:
        TextEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::TextEvent> MakeInstance(TextEvent& t)
        {
            return Pickable<Instances::fl_events::TextEvent>(new(t.Alloc()) Instances::fl_events::TextEvent(t));
        }
        SPtr<Instances::fl_events::TextEvent> MakeInstanceS(TextEvent& t)
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
    class TextEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextEvent"; }
#endif
    public:
        typedef Classes::fl_events::TextEvent ClassType;
        typedef InstanceTraits::fl_events::TextEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class TextEvent : public Class
    {
        friend class ClassTraits::fl_events::TextEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TextEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TextEvent"; }
#endif
    public:
        typedef TextEvent SelfType;
        typedef TextEvent ClassType;
        
    private:
        TextEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* LINK;
        const char* TEXT_INPUT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

