//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Xml_XMLNode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Xml_XMLNode.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_xml
{
    XMLNode::XMLNode(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , firstChild()
    , lastChild()
    , nextSibling()
    , nodeName(AS3::DefaultValue<ASString>(GetStringManager()))
    , nodeType()
    , nodeValue(AS3::DefaultValue<ASString>(GetStringManager()))
    , parentNode()
    , previousSibling()
//##protect##"instance::XMLNode::XMLNode()$data"
//##protect##"instance::XMLNode::XMLNode()$data"
    {
//##protect##"instance::XMLNode::XMLNode()$code"
//##protect##"instance::XMLNode::XMLNode()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_xml
{
    // const UInt16 XMLNode::tito[XMLNode::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 6, 7, 9, 11, 13, 15, 16, 19, 20, 
    // };
    const TypeInfo* XMLNode::tit[21] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl_xml::XMLNodeTI, 
        &AS3::fl_xml::XMLNodeTI, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_xml::XMLNodeTI, &AS3::fl_xml::XMLNodeTI, 
        NULL, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo XMLNode::ti[XMLNode::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &XMLNode::tit[0], "attributes", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[1], "attributes", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[3], "childNodes", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[4], "localName", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[5], "namespaceURI", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[6], "prefix", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[7], "appendChild", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[9], "cloneNode", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[11], "getNamespaceForPrefix", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[13], "getPrefixForNamespace", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[15], "hasChildNodes", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[16], "insertBefore", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[19], "removeNode", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLNode::tit[20], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo XMLNode::mi[XMLNode::MemberInfoNum] = {
        {"firstChild", NULL, OFFSETOF(Instances::fl_xml::XMLNode, firstChild), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"lastChild", NULL, OFFSETOF(Instances::fl_xml::XMLNode, lastChild), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"nextSibling", NULL, OFFSETOF(Instances::fl_xml::XMLNode, nextSibling), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"nodeName", NULL, OFFSETOF(Instances::fl_xml::XMLNode, nodeName), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"nodeType", NULL, OFFSETOF(Instances::fl_xml::XMLNode, nodeType), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"nodeValue", NULL, OFFSETOF(Instances::fl_xml::XMLNode, nodeValue), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"parentNode", NULL, OFFSETOF(Instances::fl_xml::XMLNode, parentNode), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"previousSibling", NULL, OFFSETOF(Instances::fl_xml::XMLNode, previousSibling), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
    };


    XMLNode::XMLNode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::XMLNode::XMLNode()"
//##protect##"InstanceTraits::XMLNode::XMLNode()"

    }

    void XMLNode::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<XMLNode&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_xml
{

    XMLNode::XMLNode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::XMLNode::XMLNode()"
//##protect##"ClassTraits::XMLNode::XMLNode()"

    }

    Pickable<Traits> XMLNode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) XMLNode(vm, AS3::fl_xml::XMLNodeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_xml::XMLNodeCI));
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
    const TypeInfo XMLNodeTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_xml::XMLNode::InstanceType),
        0,
        0,
        InstanceTraits::fl_xml::XMLNode::ThunkInfoNum,
        InstanceTraits::fl_xml::XMLNode::MemberInfoNum,
        "XMLNode", "flash.xml", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo XMLNodeCI = {
        &XMLNodeTI,
        ClassTraits::fl_xml::XMLNode::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_xml::XMLNode::ti,
        InstanceTraits::fl_xml::XMLNode::mi,
    };
}; // namespace fl_xml


}}} // namespace Scaleform { namespace GFx { namespace AS3

