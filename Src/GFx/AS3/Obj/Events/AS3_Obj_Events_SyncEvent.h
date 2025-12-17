//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_SyncEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_SyncEvent_H
#define INC_AS3_Obj_Events_SyncEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo SyncEventTI;
    extern const ClassInfo SyncEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class SyncEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class SyncEvent;
}}

namespace Classes { namespace fl_events
{
    class SyncEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class SyncEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SyncEvent"; }
#endif
    public:
        typedef Classes::fl_events::SyncEvent ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SyncEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class SyncEvent : public Class
    {
        friend class ClassTraits::fl_events::SyncEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::SyncEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SyncEvent"; }
#endif
    public:
        typedef SyncEvent SelfType;
        typedef SyncEvent ClassType;
        
    private:
        SyncEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* SYNC;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

