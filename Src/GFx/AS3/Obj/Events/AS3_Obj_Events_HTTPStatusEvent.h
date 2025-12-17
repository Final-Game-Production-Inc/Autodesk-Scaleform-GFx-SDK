//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_HTTPStatusEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_HTTPStatusEvent_H
#define INC_AS3_Obj_Events_HTTPStatusEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo HTTPStatusEventTI;
    extern const ClassInfo HTTPStatusEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class HTTPStatusEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class HTTPStatusEvent;
}}

namespace Classes { namespace fl_events
{
    class HTTPStatusEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class HTTPStatusEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::HTTPStatusEvent"; }
#endif
    public:
        typedef Classes::fl_events::HTTPStatusEvent ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        HTTPStatusEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class HTTPStatusEvent : public Class
    {
        friend class ClassTraits::fl_events::HTTPStatusEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::HTTPStatusEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::HTTPStatusEvent"; }
#endif
    public:
        typedef HTTPStatusEvent SelfType;
        typedef HTTPStatusEvent ClassType;
        
    private:
        HTTPStatusEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* HTTP_RESPONSE_STATUS;
        const char* HTTP_STATUS;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

