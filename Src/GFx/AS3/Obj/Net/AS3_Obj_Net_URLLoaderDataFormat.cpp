//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLLoaderDataFormat.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLLoaderDataFormat.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_net
{
    URLLoaderDataFormat::URLLoaderDataFormat(ClassTraits::Traits& t)
    : Class(t)
    , BINARY("binary")
    , TEXT("text")
    , VARIABLES("variables")
    {
//##protect##"class_::URLLoaderDataFormat::URLLoaderDataFormat()"
//##protect##"class_::URLLoaderDataFormat::URLLoaderDataFormat()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_net
{
    const MemberInfo URLLoaderDataFormat::mi[URLLoaderDataFormat::MemberInfoNum] = {
        {"BINARY", NULL, OFFSETOF(Classes::fl_net::URLLoaderDataFormat, BINARY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TEXT", NULL, OFFSETOF(Classes::fl_net::URLLoaderDataFormat, TEXT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VARIABLES", NULL, OFFSETOF(Classes::fl_net::URLLoaderDataFormat, VARIABLES), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    URLLoaderDataFormat::URLLoaderDataFormat(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::URLLoaderDataFormat::URLLoaderDataFormat()"
//##protect##"ClassTraits::URLLoaderDataFormat::URLLoaderDataFormat()"

    }

    Pickable<Traits> URLLoaderDataFormat::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLLoaderDataFormat(vm, AS3::fl_net::URLLoaderDataFormatCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLLoaderDataFormatCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo URLLoaderDataFormatTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_net::URLLoaderDataFormat::InstanceType),
        0,
        ClassTraits::fl_net::URLLoaderDataFormat::MemberInfoNum,
        0,
        0,
        "URLLoaderDataFormat", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo URLLoaderDataFormatCI = {
        &URLLoaderDataFormatTI,
        ClassTraits::fl_net::URLLoaderDataFormat::MakeClassTraits,
        NULL,
        ClassTraits::fl_net::URLLoaderDataFormat::mi,
        NULL,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

