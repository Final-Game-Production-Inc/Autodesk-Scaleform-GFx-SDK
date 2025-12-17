//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Xml_XMLNodeType.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Xml_XMLNodeType.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_xml
{
    XMLNodeType::XMLNodeType(ClassTraits::Traits& t)
    : Class(t)
    , ELEMENT_NODE(1)
    , TEXT_NODE(3)
    {
//##protect##"class_::XMLNodeType::XMLNodeType()"
//##protect##"class_::XMLNodeType::XMLNodeType()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_xml
{
    const MemberInfo XMLNodeType::mi[XMLNodeType::MemberInfoNum] = {
        {"ELEMENT_NODE", NULL, OFFSETOF(Classes::fl_xml::XMLNodeType, ELEMENT_NODE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"TEXT_NODE", NULL, OFFSETOF(Classes::fl_xml::XMLNodeType, TEXT_NODE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    XMLNodeType::XMLNodeType(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::XMLNodeType::XMLNodeType()"
//##protect##"ClassTraits::XMLNodeType::XMLNodeType()"

    }

    Pickable<Traits> XMLNodeType::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) XMLNodeType(vm, AS3::fl_xml::XMLNodeTypeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_xml::XMLNodeTypeCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_xml
{
    const TypeInfo XMLNodeTypeTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_xml::XMLNodeType::InstanceType),
        0,
        ClassTraits::fl_xml::XMLNodeType::MemberInfoNum,
        0,
        0,
        "XMLNodeType", "flash.xml", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo XMLNodeTypeCI = {
        &XMLNodeTypeTI,
        ClassTraits::fl_xml::XMLNodeType::MakeClassTraits,
        NULL,
        ClassTraits::fl_xml::XMLNodeType::mi,
        NULL,
        NULL,
    };
}; // namespace fl_xml


}}} // namespace Scaleform { namespace GFx { namespace AS3

