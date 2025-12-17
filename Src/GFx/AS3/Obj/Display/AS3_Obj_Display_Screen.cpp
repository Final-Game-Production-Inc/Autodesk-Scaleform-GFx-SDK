//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Screen.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Screen.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class Rectangle;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Screen_tito[3] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* Screen_tit[3] = {
        &AS3::fl_geom::RectangleTI, 
        &AS3::fl::int_TI, 
        &AS3::fl_geom::RectangleTI, 
    };
    const ThunkInfo Screen_ti[3] = {
        {ThunkInfo::EmptyFunc, &Screen_tit[0], "bounds", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Screen_tit[1], "colorDepth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Screen_tit[2], "visibleBounds", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_display
{
    Screen::Screen(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Screen::Screen()"
//##protect##"class_::Screen::Screen()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    // const UInt16 Screen_tito[1] = {
    //    0, 
    // };
    const TypeInfo* Screen_tit[2] = {
        &AS3::fl::ArrayTI, &AS3::fl_geom::RectangleTI, 
    };
    const ThunkInfo Screen_ti[1] = {
        {ThunkInfo::EmptyFunc, &Screen_tit[0], "getScreensForRectangle", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Screen::Screen(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Screen::Screen()"
//##protect##"ClassTraits::Screen::Screen()"

    }

    Pickable<Traits> Screen::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Screen(vm, AS3::fl_display::ScreenCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::ScreenCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo ScreenTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_display::Screen::InstanceType),
        1,
        0,
        3,
        0,
        "Screen", "flash.display", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo ScreenCI = {
        &ScreenTI,
        ClassTraits::fl_display::Screen::MakeClassTraits,
        ClassTraits::fl_display::Screen_ti,
        NULL,
        InstanceTraits::fl_display::Screen_ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

