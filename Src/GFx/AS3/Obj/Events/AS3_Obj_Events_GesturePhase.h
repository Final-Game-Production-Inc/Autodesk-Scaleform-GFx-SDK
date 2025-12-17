//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_GesturePhase.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_GesturePhase_H
#define INC_AS3_Obj_Events_GesturePhase_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo GesturePhaseTI;
    extern const ClassInfo GesturePhaseCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_events
{
    class GesturePhase;
}}

namespace InstanceTraits { namespace fl_events
{
    class GesturePhase;
}}

namespace Classes { namespace fl_events
{
    class GesturePhase;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class GesturePhase : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GesturePhase"; }
#endif
    public:
        typedef Classes::fl_events::GesturePhase ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GesturePhase(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class GesturePhase : public Class
    {
        friend class ClassTraits::fl_events::GesturePhase;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::GesturePhaseTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GesturePhase"; }
#endif
    public:
        typedef GesturePhase SelfType;
        typedef GesturePhase ClassType;
        
    private:
        GesturePhase(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ALL;
        const char* BEGIN;
        const char* END;
        const char* UPDATE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

