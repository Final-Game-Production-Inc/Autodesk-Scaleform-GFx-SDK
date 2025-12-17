//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_NativeDragEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_NativeDragEvent_H
#define INC_AS3_Obj_Events_NativeDragEvent_H

#include "AS3_Obj_Events_MouseEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo NativeDragEventTI;
    extern const ClassInfo NativeDragEventCI;
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
    class NativeDragEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class NativeDragEvent;
}}

namespace Classes { namespace fl_events
{
    class NativeDragEvent;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_desktop
    {
        class NativeDragOptions;
        class Clipboard;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class NativeDragEvent : public Instances::fl_events::MouseEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::NativeDragEvent;
        
    public:
        typedef NativeDragEvent SelfType;
        typedef Classes::fl_events::NativeDragEvent ClassType;
        typedef InstanceTraits::fl_events::NativeDragEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::NativeDragEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::NativeDragEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NativeDragEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        SPtr<Instances::fl_desktop::NativeDragOptions> allowedActions;
        SPtr<Instances::fl_desktop::Clipboard> clipboard;
        ASString dropAction;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class NativeDragEvent : public fl_events::MouseEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NativeDragEvent"; }
#endif
    public:
        typedef Instances::fl_events::NativeDragEvent InstanceType;

    public:
        NativeDragEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::NativeDragEvent> MakeInstance(NativeDragEvent& t)
        {
            return Pickable<Instances::fl_events::NativeDragEvent>(new(t.Alloc()) Instances::fl_events::NativeDragEvent(t));
        }
        SPtr<Instances::fl_events::NativeDragEvent> MakeInstanceS(NativeDragEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 3 };
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
    class NativeDragEvent : public fl_events::MouseEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeDragEvent"; }
#endif
    public:
        typedef Classes::fl_events::NativeDragEvent ClassType;
        typedef InstanceTraits::fl_events::NativeDragEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeDragEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 7 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class NativeDragEvent : public Class
    {
        friend class ClassTraits::fl_events::NativeDragEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::NativeDragEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NativeDragEvent"; }
#endif
    public:
        typedef NativeDragEvent SelfType;
        typedef NativeDragEvent ClassType;
        
    private:
        NativeDragEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* NATIVE_DRAG_COMPLETE;
        const char* NATIVE_DRAG_DROP;
        const char* NATIVE_DRAG_ENTER;
        const char* NATIVE_DRAG_EXIT;
        const char* NATIVE_DRAG_OVER;
        const char* NATIVE_DRAG_START;
        const char* NATIVE_DRAG_UPDATE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

