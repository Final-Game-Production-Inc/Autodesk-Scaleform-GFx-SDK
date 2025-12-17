//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_ISimpleTextSelection.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Accessibility_ISimpleTextSelection.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_accessibility
{
    // const UInt16 ISimpleTextSelection_tito[2] = {
    //    0, 1, 
    // };
    const TypeInfo* ISimpleTextSelection_tit[2] = {
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
    };
    const ThunkInfo ISimpleTextSelection_ti[2] = {
        {ThunkInfo::EmptyFunc, &ISimpleTextSelection_tit[0], "selectionActiveIndex", "flash.accessibility:ISimpleTextSelection", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ISimpleTextSelection_tit[1], "selectionAnchorIndex", "flash.accessibility:ISimpleTextSelection", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_accessibility
{

    ISimpleTextSelection::ISimpleTextSelection(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ISimpleTextSelection::ISimpleTextSelection()"
//##protect##"ClassTraits::ISimpleTextSelection::ISimpleTextSelection()"

    }

    Pickable<Traits> ISimpleTextSelection::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ISimpleTextSelection(vm, AS3::fl_accessibility::ISimpleTextSelectionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_accessibility::ISimpleTextSelectionCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_accessibility
{
    const TypeInfo ISimpleTextSelectionTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_accessibility::ISimpleTextSelection::InstanceType),
        0,
        0,
        2,
        0,
        "ISimpleTextSelection", "flash.accessibility", NULL,
        TypeInfo::None
    };

    const ClassInfo ISimpleTextSelectionCI = {
        &ISimpleTextSelectionTI,
        ClassTraits::fl_accessibility::ISimpleTextSelection::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_accessibility::ISimpleTextSelection_ti,
        NULL,
    };
}; // namespace fl_accessibility


}}} // namespace Scaleform { namespace GFx { namespace AS3

