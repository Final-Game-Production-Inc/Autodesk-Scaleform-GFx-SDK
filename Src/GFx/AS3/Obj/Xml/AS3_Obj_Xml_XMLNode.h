//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Xml_XMLNode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Xml_XMLNode_H
#define INC_AS3_Obj_Xml_XMLNode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_xml
{
    extern const TypeInfo XMLNodeTI;
    extern const ClassInfo XMLNodeCI;
} // namespace fl_xml
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_xml
{
    class XMLNode;
}}

namespace InstanceTraits { namespace fl_xml
{
    class XMLNode;
}}

namespace Classes { namespace fl_xml
{
    class XMLNode;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_xml
{
    class XMLNode : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_xml::XMLNode;
        
    public:
        typedef XMLNode SelfType;
        typedef Classes::fl_xml::XMLNode ClassType;
        typedef InstanceTraits::fl_xml::XMLNode TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_xml::XMLNodeTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_xml::XMLNode"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        XMLNode(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        SPtr<Instances::fl_xml::XMLNode> firstChild;
        SPtr<Instances::fl_xml::XMLNode> lastChild;
        SPtr<Instances::fl_xml::XMLNode> nextSibling;
        ASString nodeName;
        UInt32 nodeType;
        ASString nodeValue;
        SPtr<Instances::fl_xml::XMLNode> parentNode;
        SPtr<Instances::fl_xml::XMLNode> previousSibling;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_xml
{
    class XMLNode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::XMLNode"; }
#endif
    public:
        typedef Instances::fl_xml::XMLNode InstanceType;

    public:
        XMLNode(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_xml::XMLNode> MakeInstance(XMLNode& t)
        {
            return Pickable<Instances::fl_xml::XMLNode>(new(t.Alloc()) Instances::fl_xml::XMLNode(t));
        }
        SPtr<Instances::fl_xml::XMLNode> MakeInstanceS(XMLNode& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 14 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[21];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_xml
{
    class XMLNode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::XMLNode"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_xml::XMLNode InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        XMLNode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

