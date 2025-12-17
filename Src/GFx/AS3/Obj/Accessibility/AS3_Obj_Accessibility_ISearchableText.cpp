//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_ISearchableText.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Accessibility_ISearchableText.h"
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
    // const UInt16 ISearchableText_tito[1] = {
    //    0, 
    // };
    const TypeInfo* ISearchableText_tit[1] = {
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ISearchableText_ti[1] = {
        {ThunkInfo::EmptyFunc, &ISearchableText_tit[0], "searchText", "flash.accessibility:ISearchableText", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_accessibility
{

    ISearchableText::ISearchableText(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ISearchableText::ISearchableText()"
//##protect##"ClassTraits::ISearchableText::ISearchableText()"

    }

    Pickable<Traits> ISearchableText::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ISearchableText(vm, AS3::fl_accessibility::ISearchableTextCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_accessibility::ISearchableTextCI));
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
    const TypeInfo ISearchableTextTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_accessibility::ISearchableText::InstanceType),
        0,
        0,
        1,
        0,
        "ISearchableText", "flash.accessibility", NULL,
        TypeInfo::None
    };

    const ClassInfo ISearchableTextCI = {
        &ISearchableTextTI,
        ClassTraits::fl_accessibility::ISearchableText::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_accessibility::ISearchableText_ti,
        NULL,
    };
}; // namespace fl_accessibility


}}} // namespace Scaleform { namespace GFx { namespace AS3

