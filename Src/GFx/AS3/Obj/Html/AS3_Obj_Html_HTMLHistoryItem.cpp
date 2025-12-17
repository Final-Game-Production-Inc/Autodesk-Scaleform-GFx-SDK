//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLHistoryItem.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Html_HTMLHistoryItem.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_html
{
    // const UInt16 HTMLHistoryItem_tito[4] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* HTMLHistoryItem_tit[4] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo HTMLHistoryItem_ti[4] = {
        {ThunkInfo::EmptyFunc, &HTMLHistoryItem_tit[0], "isPost", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHistoryItem_tit[1], "originalUrl", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHistoryItem_tit[2], "title", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHistoryItem_tit[3], "url", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_html
{

    HTMLHistoryItem::HTMLHistoryItem(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::HTMLHistoryItem::HTMLHistoryItem()"
//##protect##"ClassTraits::HTMLHistoryItem::HTMLHistoryItem()"

    }

    Pickable<Traits> HTMLHistoryItem::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLHistoryItem(vm, AS3::fl_html::HTMLHistoryItemCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_html::HTMLHistoryItemCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_html
{
    const TypeInfo HTMLHistoryItemTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_html::HTMLHistoryItem::InstanceType),
        0,
        0,
        4,
        0,
        "HTMLHistoryItem", "flash.html", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo HTMLHistoryItemCI = {
        &HTMLHistoryItemTI,
        ClassTraits::fl_html::HTMLHistoryItem::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_html::HTMLHistoryItem_ti,
        NULL,
    };
}; // namespace fl_html


}}} // namespace Scaleform { namespace GFx { namespace AS3

