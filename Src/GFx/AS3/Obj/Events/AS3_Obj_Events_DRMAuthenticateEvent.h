//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_DRMAuthenticateEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_DRMAuthenticateEvent_H
#define INC_AS3_Obj_Events_DRMAuthenticateEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo DRMAuthenticateEventTI;
    extern const ClassInfo DRMAuthenticateEventCI;
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
    class DRMAuthenticateEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class DRMAuthenticateEvent;
}}

namespace Classes { namespace fl_events
{
    class DRMAuthenticateEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class DRMAuthenticateEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DRMAuthenticateEvent"; }
#endif
    public:
        typedef Classes::fl_events::DRMAuthenticateEvent ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DRMAuthenticateEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class DRMAuthenticateEvent : public Class
    {
        friend class ClassTraits::fl_events::DRMAuthenticateEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::DRMAuthenticateEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::DRMAuthenticateEvent"; }
#endif
    public:
        typedef DRMAuthenticateEvent SelfType;
        typedef DRMAuthenticateEvent ClassType;
        
    private:
        DRMAuthenticateEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* AUTHENTICATION_TYPE_DRM;
        const char* AUTHENTICATION_TYPE_PROXY;
        const char* DRM_AUTHENTICATE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

