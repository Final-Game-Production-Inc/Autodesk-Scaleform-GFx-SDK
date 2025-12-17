//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Xml_XMLNodeType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Xml_XMLNodeType_H
#define INC_AS3_Obj_Xml_XMLNodeType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_xml
{
    extern const TypeInfo XMLNodeTypeTI;
    extern const ClassInfo XMLNodeTypeCI;
} // namespace fl_xml

namespace ClassTraits { namespace fl_xml
{
    class XMLNodeType;
}}

namespace InstanceTraits { namespace fl_xml
{
    class XMLNodeType;
}}

namespace Classes { namespace fl_xml
{
    class XMLNodeType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_xml
{
    class XMLNodeType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::XMLNodeType"; }
#endif
    public:
        typedef Classes::fl_xml::XMLNodeType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        XMLNodeType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_xml
{
    class XMLNodeType : public Class
    {
        friend class ClassTraits::fl_xml::XMLNodeType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_xml::XMLNodeTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::XMLNodeType"; }
#endif
    public:
        typedef XMLNodeType SelfType;
        typedef XMLNodeType ClassType;
        
    private:
        XMLNodeType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 ELEMENT_NODE;
        const UInt32 TEXT_NODE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

