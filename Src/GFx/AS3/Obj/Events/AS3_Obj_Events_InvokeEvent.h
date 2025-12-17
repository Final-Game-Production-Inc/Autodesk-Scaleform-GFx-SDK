//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_InvokeEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_InvokeEvent_H
#define INC_AS3_Obj_Events_InvokeEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo InvokeEventTI;
    extern const ClassInfo InvokeEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl
namespace fl_filesystem
{
    extern const TypeInfo FileTI;
    extern const ClassInfo FileCI;
} // namespace fl_filesystem

namespace ClassTraits { namespace fl_events
{
    class InvokeEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class InvokeEvent;
}}

namespace Classes { namespace fl_events
{
    class InvokeEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class InvokeEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::InvokeEvent"; }
#endif
    public:
        typedef Classes::fl_events::InvokeEvent ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        InvokeEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class InvokeEvent : public Class
    {
        friend class ClassTraits::fl_events::InvokeEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::InvokeEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::InvokeEvent"; }
#endif
    public:
        typedef InvokeEvent SelfType;
        typedef InvokeEvent ClassType;
        
    private:
        InvokeEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* INVOKE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

