//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ScreenMouseEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_ScreenMouseEvent_H
#define INC_AS3_Obj_Events_ScreenMouseEvent_H

#include "AS3_Obj_Events_MouseEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo ScreenMouseEventTI;
    extern const ClassInfo ScreenMouseEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class ScreenMouseEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class ScreenMouseEvent;
}}

namespace Classes { namespace fl_events
{
    class ScreenMouseEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class ScreenMouseEvent : public fl_events::MouseEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ScreenMouseEvent"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::MouseEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ScreenMouseEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

