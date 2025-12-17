//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_EventPhase.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_EventPhase_H
#define INC_AS3_Obj_Events_EventPhase_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo EventPhaseTI;
    extern const ClassInfo EventPhaseCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_events
{
    class EventPhase;
}}

namespace InstanceTraits { namespace fl_events
{
    class EventPhase;
}}

namespace Classes { namespace fl_events
{
    class EventPhase;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class EventPhase : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::EventPhase"; }
#endif
    public:
        typedef Classes::fl_events::EventPhase ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        EventPhase(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class EventPhase : public Class
    {
        friend class ClassTraits::fl_events::EventPhase;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::EventPhaseTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::EventPhase"; }
#endif
    public:
        typedef EventPhase SelfType;
        typedef EventPhase ClassType;
        
    private:
        EventPhase(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 AT_TARGET;
        const UInt32 BUBBLING_PHASE;
        const UInt32 CAPTURING_PHASE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

