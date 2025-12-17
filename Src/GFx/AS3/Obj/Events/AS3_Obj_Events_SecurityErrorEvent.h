//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_SecurityErrorEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_SecurityErrorEvent_H
#define INC_AS3_Obj_Events_SecurityErrorEvent_H

#include "AS3_Obj_Events_ErrorEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo SecurityErrorEventTI;
    extern const ClassInfo SecurityErrorEventCI;
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
    class SecurityErrorEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class SecurityErrorEvent;
}}

namespace Classes { namespace fl_events
{
    class SecurityErrorEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class SecurityErrorEvent : public fl_events::ErrorEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SecurityErrorEvent"; }
#endif
    public:
        typedef Classes::fl_events::SecurityErrorEvent ClassType;
        typedef InstanceTraits::fl_events::ErrorEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SecurityErrorEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class SecurityErrorEvent : public Class
    {
        friend class ClassTraits::fl_events::SecurityErrorEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::SecurityErrorEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SecurityErrorEvent"; }
#endif
    public:
        typedef SecurityErrorEvent SelfType;
        typedef SecurityErrorEvent ClassType;
        
    private:
        SecurityErrorEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* SECURITY_ERROR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

