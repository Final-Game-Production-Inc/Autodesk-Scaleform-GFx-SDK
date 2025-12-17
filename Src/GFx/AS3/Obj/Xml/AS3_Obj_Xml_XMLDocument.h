//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Xml_XMLDocument.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Xml_XMLDocument_H
#define INC_AS3_Obj_Xml_XMLDocument_H

#include "AS3_Obj_Xml_XMLNode.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_xml
{
    extern const TypeInfo XMLDocumentTI;
    extern const ClassInfo XMLDocumentCI;
    extern const TypeInfo XMLNodeTI;
    extern const ClassInfo XMLNodeCI;
} // namespace fl_xml
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_xml
{
    class XMLDocument;
}}

namespace InstanceTraits { namespace fl_xml
{
    class XMLDocument;
}}

namespace Classes { namespace fl_xml
{
    class XMLDocument;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_xml
{
    class XMLDocument : public Instances::fl_xml::XMLNode
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_xml::XMLDocument;
        
    public:
        typedef XMLDocument SelfType;
        typedef Classes::fl_xml::XMLDocument ClassType;
        typedef InstanceTraits::fl_xml::XMLDocument TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_xml::XMLDocumentTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_xml::XMLDocument"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        XMLDocument(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        SPtr<Instances::fl::Object> docTypeDecl;
        SPtr<Instances::fl::Object> idMap;
        bool ignoreWhite;
        SPtr<Instances::fl::Object> xmlDecl;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_xml
{
    class XMLDocument : public fl_xml::XMLNode
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::XMLDocument"; }
#endif
    public:
        typedef Instances::fl_xml::XMLDocument InstanceType;

    public:
        XMLDocument(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_xml::XMLDocument> MakeInstance(XMLDocument& t)
        {
            return Pickable<Instances::fl_xml::XMLDocument>(new(t.Alloc()) Instances::fl_xml::XMLDocument(t));
        }
        SPtr<Instances::fl_xml::XMLDocument> MakeInstanceS(XMLDocument& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[7];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_xml
{
    class XMLDocument : public fl_xml::XMLNode
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::XMLDocument"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_xml::XMLDocument InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        XMLDocument(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

