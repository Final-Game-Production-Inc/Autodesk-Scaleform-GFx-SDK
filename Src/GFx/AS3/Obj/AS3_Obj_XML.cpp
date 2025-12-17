//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_XML.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_XML.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Array.h"
#include "AS3_Obj_XMLList.h"
#include "AS3_Obj_QName.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

#ifdef GFX_ENABLE_XML

//////////////////////////////////////////////////////////////////////////////
NamespaceArray::~NamespaceArray()
{
}

void NamespaceArray::AddUnique(const NamespaceArray& other)
{
    const UPInt size = other.GetSize();

    for (UPInt i = 0; i < size; ++i)
        Add(*other.Namespaces[i], true);
}

bool NamespaceArray::Find(const Instances::fl::Namespace& ns) const
{
    bool result = false;
    const UPInt size = GetSize();

    for (UPInt i = 0; i < size; ++i)
    {
        const Instances::fl::Namespace& ansNs = *Namespaces[i];

        // This comparison is not the same as (ansNs == cns).
        if (ansNs.GetUri() == ns.GetUri() && ansNs.GetPrefix() == ns.GetPrefix())
        {
            result = true;
            break;
        }
    }

    return result;
}

bool NamespaceArray::FindByPrefix(const ASString& pref) const
{
    bool result = false;
    const UPInt size = GetSize();

    for (UPInt i = 0; !result && i < size; ++i)
    {
        const Value& ns1prefix = Namespaces[i]->GetPrefix();

        if (!ns1prefix.IsNullOrUndefined())
            result = (ns1prefix.AsString() == pref);
    }

    return result;
}

bool NamespaceArray::FindByUri(const ASString& uri) const
{
    bool result = false;
    const UPInt size = GetSize();

    for (UPInt i = 0; !result && i < size; ++i)
        result = (Namespaces[i]->GetUri() == uri);

    return result;
}

void NamespaceArray::Add(Instances::fl::Namespace& other, bool checkUnique)
{
#if 0
    if (other.GetKind() == Abc::NS_Public && other.GetUri().IsEmpty())
        // Skip default namespace.
        return;
#endif

    if (checkUnique)
    {
        for (UPInt i = 0; i < Namespaces.GetSize(); ++i)
        {
            Instances::fl::Namespace& ns = *Namespaces[i];

            if (ns == other)
                return;
        }
    }

    Namespaces.PushBack(&other);
}

//////////////////////////////////////////////////////////////////////////////
XMLParser::XMLParser(InstanceTraits::fl::XML& itr)
: NsSep(':')
, NodeKind(kNone)
, ITr(itr)
, Parser(XML_ParserCreate(NULL))
, pCurrElem(NULL)
, RootElements(itr.GetVM().GetMemoryHeap())
, Text(itr.GetVM().GetStringManager().CreateEmptyString())
, KindStack(itr.GetVM().GetMemoryHeap())
{
    XML_SetUserData(Parser, this);

    XML_SetElementHandler(Parser, StartElementExpatCallback, EndElementExpatCallback);
    XML_SetCharacterDataHandler(Parser, CharacterDataExpatCallback);
    XML_SetXmlDeclHandler(Parser, DeclExpatCallback);
    XML_SetDoctypeDeclHandler(Parser, StartDoctypeDeclExpatCallback, EndDoctypeDeclExpatCallback);
    XML_SetDefaultHandler(Parser, DefaultExpatCallback);

    const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(ITr.GetClass());
    if (!cl.ignoreComments)
        XML_SetCommentHandler(Parser, CommentExpatCallback);

#if 0
    if (!cl.get_ignoreProcessingInstructions())
        XML_SetCdataSectionHandler(Parser, StartCDataSectionExpatCallback, EndCDataSectionExpatCallback);
#endif

    if (!cl.ignoreProcessingInstructions)
        XML_SetProcessingInstructionHandler(Parser, ProcessingInstructionExpatCallback);
}

XMLParser::~XMLParser()
{
    XML_ParserFree(Parser);
}

void XMLParser::SetNodeKind(Kind k)
{
    if (NodeKind != k)
    {
        switch (NodeKind)
        {
        case kText:
            {
                const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(ITr.GetClass());

                if (cl.ignoreWhitespace)
                    Text = Text.TruncateWhitespace();

                if (!Text.IsEmpty())
                {
                    Pickable<Instances::fl::XML> el = ITr.MakeInstanceText(ITr, Text, pCurrElem);
                    SPtr<Instances::fl::XML> ptr_el = el;
                    // Add it to parent's children.
                    if (pCurrElem != NULL && pCurrElem->GetKind() == Instances::fl::XML::kElement)
                    {
                        pCurrElem->AppendChild(ptr_el);
                    }
                    else
                    {
                        RootElements.PushBack(ptr_el);
                        pCurrElem = ptr_el;
                    }

                    Text.Clear();
                }
            }
            break;
        default:
            break;
        }

        NodeKind = k;
    }
}

CheckResult XMLParser::Parse(const StringDataPtr& str, UPInt& pos, bool isList)
{
    UPInt startPos = pos;

    bool rc = XML_Parse(Parser, str.ToCStr() + startPos, (int)(str.GetSize() - startPos), 1) == XML_STATUS_OK;
    bool shouldAdvancePos = true;

    if (!rc)
    {
        XML_Error ecode = XML_GetErrorCode(Parser);
        VM& vm = ITr.GetVM();
        VM::ErrorID errorID = VM::eXMLMalformedElement;
        Kind k = kNone;

        if (KindStack.GetSize())
            k = KindStack.Back();

        bool shouldThrow = true;
        switch (ecode)
        {
        case XML_ERROR_NONE:
            {
                // PPS: XML_StopParser(parser->Parser, XML_TRUE); gets here

                // PPS: Special case for an XML string that starts with a proc instr
                // PPS: We return true from the parser callback for such a case
                if (pos == startPos && pCurrElem)
                {
                    if (pCurrElem->GetKind() == Instances::fl::XML::kInstruction)
                    {
                        // PPS: Adjust pos appropriately
                        // PPS: Look for closing tags
                        UPInt offset = 0;
                        UPInt len = str.GetSize() - startPos;
                        while (offset < len)
                        {
                            if (str[offset] == '?' && ((offset + 1) < len))
                            {
                                if (str[offset + 1] == '>')
                                {
                                    pos += offset + 2;
                                    return true;
                                }
                            }
                            offset++;
                        }
                    }
                }

            }
            break;
        case XML_ERROR_JUNK_AFTER_DOC_ELEMENT:
            if (isList)
            {
                shouldThrow = false;
                rc = true;
            }
            else if (pCurrElem.GetPtr() && k == kElement)
            {
                errorID = VM::eXMLUnterminatedElementTag;
            }
            else
            {
                errorID = VM::eXMLMarkupMustBeWellFormed;
            }

            break;
        case XML_ERROR_NO_MEMORY:
            errorID = VM::eOutOfMemoryError;
            break;
        case XML_ERROR_TAG_MISMATCH:
            if (k == kElement)
                errorID = VM::eXMLUnterminatedElementTag;
            break;
        case XML_ERROR_NO_ELEMENTS:
            if (isList)
            {
                shouldAdvancePos = false;
                shouldThrow = false;
            }
            if (pCurrElem && k == kElement)
            {
                errorID = VM::eXMLUnterminatedElementTag;
            }
            else
            {
                shouldThrow = false;
            }
            break;
        case XML_ERROR_DUPLICATE_ATTRIBUTE:
            errorID = VM::eXMLDuplicateAttribute;
            break;
        case XML_ERROR_UNCLOSED_TOKEN:
            errorID = VM::eXMLUnterminatedProcessingInstruction;
            break;
        case XML_ERROR_UNCLOSED_CDATA_SECTION:
            errorID = VM::eXMLUnterminatedCData;
            break;
        case XML_ERROR_ABORTED:
            // VM exception.
            shouldThrow = false;
            shouldAdvancePos = false;
            break;
        case XML_ERROR_SYNTAX:
            break;
        case XML_ERROR_INVALID_TOKEN:
            // SS: Throwing VM::eXMLMarkupMustBeWellFormed here will break more 
            // tests than it will fix.
            // errorID = VM::eXMLMarkupMustBeWellFormed;
            if (!pCurrElem)
            {
                shouldThrow = false;
            }
            break;
        case XML_ERROR_PARTIAL_CHAR:
        case XML_ERROR_PARAM_ENTITY_REF:
        case XML_ERROR_UNDEFINED_ENTITY:
        case XML_ERROR_RECURSIVE_ENTITY_REF:
        case XML_ERROR_ASYNC_ENTITY:
        case XML_ERROR_BAD_CHAR_REF:
        case XML_ERROR_BINARY_ENTITY_REF:
        case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF:
        case XML_ERROR_MISPLACED_XML_PI:
        case XML_ERROR_UNKNOWN_ENCODING:
        case XML_ERROR_INCORRECT_ENCODING:
        case XML_ERROR_EXTERNAL_ENTITY_HANDLING:
        case XML_ERROR_NOT_STANDALONE:
        case XML_ERROR_UNEXPECTED_STATE:
        case XML_ERROR_ENTITY_DECLARED_IN_PE:
        case XML_ERROR_FEATURE_REQUIRES_XML_DTD:
        case XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING:
        case XML_ERROR_UNBOUND_PREFIX:
        case XML_ERROR_UNDECLARING_PREFIX:
        case XML_ERROR_INCOMPLETE_PE:
        case XML_ERROR_XML_DECL:
        case XML_ERROR_TEXT_DECL:
        case XML_ERROR_PUBLICID:
        case XML_ERROR_SUSPENDED:
        case XML_ERROR_NOT_SUSPENDED:
        case XML_ERROR_FINISHED:
        case XML_ERROR_SUSPEND_PE:
        case XML_ERROR_RESERVED_PREFIX_XML:
        case XML_ERROR_RESERVED_PREFIX_XMLNS:
        case XML_ERROR_RESERVED_NAMESPACE_URI:
            break;
        }

        if (shouldThrow)
        {
            vm.ThrowTypeError(VM::Error(errorID, vm));
        }
    }

    if (shouldAdvancePos)
    {
        pos += XML_GetCurrentByteIndex(Parser);
    }

    SetNodeKind(kNone);
    return rc;
}

void XMLParser::GetRootNodes(ArrayLH<SPtr<Instances::fl::XML> >& nodes) const
{
    for (UPInt i = 0; i < RootElements.GetSize(); ++i)
    {
        nodes.PushBack(RootElements[i]);
    }
}


static bool IsNsDeclPref(UPInt len, const char* an)
{
    return (len > 4 && an[0] == 'x' && an[1] == 'm' && an[2] == 'l' && an[3] == 'n' && an[4] == 's');
}

void XMLParser::StartElementExpatCallback(
    void *userData,
    const XML_Char *name,
    const XML_Char **atts)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_ASSERT(parser);
    parser->SetNodeKind(kElement);
    parser->PushNodeKind(kElement);
    InstanceTraits::fl::XML& itr = parser->ITr;
    VM& vm = itr.GetVM();
    StringManager& sm = vm.GetStringManager();
    const char* delim;

    // Handle namespace declarations.
    // (Collect them into array "namespaces")
    ArrayDH<SPtr<Instances::fl::Namespace> > namespaces(vm.GetMemoryHeap());

    for (unsigned i = 0; atts[i]; ++i)
    {
        const char* an = atts[i];
        const UPInt len = strlen(an);

        if (len > 0 && an[0] == parser->NsSep)
        {
            vm.ThrowTypeError(VM::Error(VM::eXMLBadQName, vm SF_DEBUG_ARG(an)));
            XML_StopParser(parser->Parser, XML_FALSE);
            return;
        }

        if (IsNsDeclPref(len, an))
        {
            // New namespace declaration.
            ASString ns_prefix = sm.CreateEmptyString();

            if (len > 5 && an[5] == parser->NsSep)
            {
                ns_prefix = sm.CreateString(an + 6);
            }

            const ASString ns_uri = sm.CreateString(atts[++i]);
            InstanceTraits::fl::Namespace& itr_ns = static_cast<InstanceTraits::fl::Namespace&>(vm.GetITraitsNamespace());

            Pickable<Instances::fl::Namespace> ns = itr_ns.MakeInstance(
                Abc::NS_Public,
                ns_uri,
                ns_prefix
                );

            namespaces.PushBack(ns);

            continue;
        }

        ++i;
    }

    delim = strchr(name, parser->NsSep);

    if (delim == name)
    {
        vm.ThrowTypeError(VM::Error(VM::eXMLBadQName, vm SF_DEBUG_ARG(name)));
        XML_StopParser(parser->Parser, XML_FALSE);
        return;
    }

    ASString ns_prefix = sm.CreateEmptyString();
    ASString el_name = sm.CreateEmptyString();

    if (delim == NULL)
        el_name = sm.CreateString(name);
    else
    {
        ns_prefix = sm.CreateString(name, delim - name);
        el_name = sm.CreateString(delim + 1);
    }

    // Find element's namespace.
    Instances::fl::Namespace* ns = NULL;

    // Even if prefix is empty it still can be a default namespace.
    {
        // Resolve namespace by prefix (In the current node).
        const UPInt size = namespaces.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            Instances::fl::Namespace* cur_ns = namespaces[i];
            if (cur_ns->GetPrefix().IsString() && cur_ns->GetPrefix().AsString() == ns_prefix)
            {
                ns = cur_ns;
                break;
            }
        }

        // Resolve namespace by prefix (In parent node).
        if (ns == NULL && parser->pCurrElem.GetPtr() != NULL)
            ns = parser->pCurrElem->FindNamespaceByPrefix(ns_prefix, NULL);
    }

    if (ns == NULL)
    {
        if (ns_prefix.IsEmpty())
        {
            ns = vm.GetDefXMLNamespace();

            if (ns)
            {
                bool found_local = false;

                // Resolve namespace by URI (In the current node).
                const UPInt size = namespaces.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    Instances::fl::Namespace* cur_ns = namespaces[i];

                    if (cur_ns->GetUri() == ns->GetUri())
                    {
                        ns = cur_ns;
                        found_local = true;
                        break;
                    }
                }

                if (!found_local && parser->pCurrElem.GetPtr() != NULL)
                {
                    // Default namespace can have prefix in this XML.
                    Instances::fl::Namespace* local_ns = parser->pCurrElem->FindNamespaceByURI(ns->GetUri(), NULL);
                    if (local_ns)
                        ns = local_ns;
#if 0
                    // SS: TODO
                    else
                        // Default namespace is not a part of this XML.
                        // Let's add it to scope namespaces.
                        AddInScopeNamespace(*ns);
#endif
                }
            }
            else
                // Public namespace is our last resort.
                ns = &vm.GetPublicNamespace();
        }
        else
        {
            // There is no such prefix.
            vm.ThrowTypeError(VM::Error(VM::eXMLPrefixNotBound, vm SF_DEBUG_ARG(ns_prefix) SF_DEBUG_ARG(name)));
            XML_StopParser(parser->Parser, XML_FALSE);
            return;
        }
    }

    SF_ASSERT(ns);
    Pickable<Instances::fl::XMLElement> el = 
        itr.MakeInstanceElement(
        itr, 
        *ns,
        el_name, 
        parser->pCurrElem
        );

    SPtr<Instances::fl::XML> ptr_el(el);
    // Add it to parent's children.
    if (parser->pCurrElem != NULL && parser->pCurrElem->GetKind() == Instances::fl::XML::kElement)
    {
        parser->pCurrElem->AppendChild(ptr_el);
    }
    else
    {
        parser->RootElements.PushBack(ptr_el);
    }

    // Element is created. Let us set up element's namespaces.
    const UPInt size = namespaces.GetSize();
    for (UPInt i = 0; i < size; ++i)
    {
        SPtr<Instances::fl::Namespace>& nsfl = namespaces[i];

        if (nsfl->GetPrefix().IsString())
            el->Namespaces.PushBack(nsfl);
    }

    // Add attributes.
    ASString aname = sm.CreateEmptyString();
    for (unsigned i = 0; atts[i]; ++i)
    {
        const char* attr_name = atts[i];
        const UPInt len = strlen(attr_name);

        if (IsNsDeclPref(len, attr_name))
        {
            // Skip namespace declaration.
            ++i;
            continue;
        }

        const char* delima = strchr(attr_name, parser->NsSep);
        ASString value = sm.CreateString(atts[++i]);

        if (delima == NULL)
        {
            aname = sm.CreateString(attr_name);

#if 0
            // Try default namespace.
            ns = vm.GetDefXMLNamespace();
            if (ns)
            {
                // Default namespace can have prefix in this XML.
                Instances::fl::Namespace* local_ns = ptr_el->FindNamespaceByURI(ns->GetUri(), NULL);
                if (local_ns)
                    ns = local_ns;
            }

            // Public namespace is our last resort.
            if (ns == NULL)
                ns = &vm.GetPublicNamespace();
#else
            // Attributes are not using default namespace.
            ns = &vm.GetPublicNamespace();
#endif
        }
        else
        {
            ASString ans_prefix = sm.CreateString(attr_name, delima - attr_name);
            aname = sm.CreateString(delima + 1);
            
            ns = ptr_el->FindNamespaceByPrefix(ans_prefix, NULL);

#if 0
            if (ns == NULL)
            {
                // Try default namespace.
                ns = vm.GetDefXMLNamespace();
                if (ns)
                {
                    // Default namespace can have prefix in this XML.
                    Instances::fl::Namespace* local_ns = ptr_el->FindNamespaceByURI(ns->GetUri(), NULL);
                    if (local_ns)
                        ns = local_ns;
                }
            }
#endif

            // Attributes are not using default namespace.
            if (ns == NULL)
                ns = &vm.GetPublicNamespace();
        }

        SF_ASSERT(ns);
        el->AddAttr(*ns, aname, value);
    }

    parser->pCurrElem = ptr_el;
}

void XMLParser::EndElementExpatCallback(
    void *userData,
    const XML_Char *name)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_ASSERT(parser);
    parser->SetNodeKind(kElement);
    parser->PopNodeKind();

    while (parser->pCurrElem)
    {
        Instances::fl::XML* cur_el = parser->pCurrElem;
        Instances::fl::XML* cur_parrent = cur_el->Parent;

        if (cur_parrent)
        {
            // We need this explicit "parent" var to prevent deletion on assignment.
            SPtr<Instances::fl::XML> parent(cur_parrent);
            parser->pCurrElem = parent;

            if (cur_el->Text == name)
                break;
        }
        else
            break;
    }
}

void XMLParser::CharacterDataExpatCallback(
    void *userData,
    const XML_Char *s,
    int len)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_ASSERT(parser);
    parser->SetNodeKind(kText);

    parser->Text.Append(s, len);
}

void XMLParser::CommentExpatCallback(
    void *userData,
    const XML_Char *data)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_ASSERT(parser);

    InstanceTraits::fl::XML& itr = parser->ITr;
    VM& vm = itr.GetVM();
    StringManager& sm = vm.GetStringManager();

    parser->SetNodeKind(kComment);

    Pickable<Instances::fl::XML> el = itr.MakeInstanceComment(
        itr,
        sm.CreateString(data),
        parser->pCurrElem
        );
    SPtr<Instances::fl::XML> ptr_el = el;

    // Add it to parent's children.
    if (parser->pCurrElem != NULL && parser->pCurrElem->GetKind() == Instances::fl::XML::kElement)
        parser->pCurrElem->AppendChild(ptr_el);
    else
    {
        parser->pCurrElem = ptr_el;
        parser->RootElements.PushBack(ptr_el);
    }
}

void XMLParser::DeclExpatCallback(
    void           *userData,
    const XML_Char *version,
    const XML_Char *encoding,
    int             standalone)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_ASSERT(parser);

//     InstanceTraits::fl::XML& itr = parser->ITr;
//     VM& vm = itr.GetVM();
//     StringManager& sm = vm.GetStringManager();
    parser->SetNodeKind(kDecl);
    SF_UNUSED3(version, encoding, standalone);

//     Pickable<Instances::fl::XML> el = itr.MakeInstance(
//         itr,
//         Instances::fl::XML::kInstruction,
//         sm.CreateString(data),
//         parser->pCurrElem
//         );
}

void XMLParser::StartCDataSectionExpatCallback(
    void* userData)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_UNUSED1(parser);
    parser->PushNodeKind(kCData);
}

void XMLParser::EndCDataSectionExpatCallback(
    void* userData)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_UNUSED1(parser);
    parser->PopNodeKind();
}

void XMLParser::StartDoctypeDeclExpatCallback(
    void           *userData,
    const XML_Char *doctypeName,
    const XML_Char *sysid,
    const XML_Char *pubid,
    int            has_internal_subset)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_UNUSED5(parser, doctypeName, sysid, pubid, has_internal_subset);
    parser->PushNodeKind(kDocType);
}

void XMLParser::EndDoctypeDeclExpatCallback(
    void           *userData)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_UNUSED1(parser);
    parser->PopNodeKind();
}

void XMLParser::ProcessingInstructionExpatCallback(
    void*           userData,
    const XML_Char* target,
    const XML_Char* data)
{
    XMLParser* parser = static_cast<XMLParser*>(userData);
    SF_UNUSED2(target, data);

    parser->SetNodeKind(kProcInstr);

    InstanceTraits::fl::XML& itr = parser->ITr;
    VM& vm = itr.GetVM();
    StringManager& sm = vm.GetStringManager();

    Pickable<Instances::fl::XML> el = itr.MakeInstanceProcInstr(
        itr,
        sm.CreateString(target),
        sm.CreateString(data),
        parser->pCurrElem
        );
    SPtr<Instances::fl::XML> ptr_el = el;

    // Add it to parent's children.
    if (parser->pCurrElem != NULL && parser->pCurrElem->GetKind() == Instances::fl::XML::kElement)
    {
        parser->pCurrElem->AppendChild(ptr_el);
    }
    else
    {
        parser->pCurrElem = ptr_el;
        parser->RootElements.PushBack(ptr_el);

        // PPS: We bail parsing if there is no parent
        XML_StopParser(parser->Parser, XML_TRUE);        
    }
}

void XMLParser::DefaultExpatCallback(
    void *userData,
    const XML_Char *s,
    int len)
{
    SF_UNUSED3(userData, s, len);
}

namespace Instances { namespace fl
{
    ///////////////////////////////////////////////////////////////////////
    class EmptyCallBack : public XMLElement::CallBack
    {
    public:
        virtual ~EmptyCallBack() { } 
        EmptyCallBack(XMLElement& el) 
        : XMLElement::CallBack(el) 
        {
        }

        virtual bool Call(UPInt)
        {
            return false;
        }
    };

    class AttrGetFirst : public XMLElement::CallBack
    {
    public:
        virtual ~AttrGetFirst() { }
        AttrGetFirst(XMLElement& el) 
        : XMLElement::CallBack(el) 
        {
        }

        virtual bool Call(UPInt ind)
        {
            if (!First)
                First = Element.GetAttr(ind);

            return true;
        }

        const SPtr<XML>& GetFirst() const
        {
            return First;
        }

    protected:
        SPtr<XML>   First;
    };

    class ChildGetFirst : public XMLElement::CallBack
    {
    public:
        virtual ~ChildGetFirst() { }
        ChildGetFirst(XMLElement& el) 
        : XMLElement::CallBack(el) 
        {
        }

        virtual bool Call(UPInt ind)
        {
            if (!First)
                First = Element.GetChild(ind);

            return true;
        }

        const SPtr<XML>& GetFirst() const
        {
            return First;
        }

    protected:
        SPtr<XML>   First;
    };

    class ChildGet : public XMLElement::CallBack
    {
    public:
        virtual ~ChildGet() { }
        ChildGet(XMLElement& el, Instances::fl::XMLList& l) 
        : XMLElement::CallBack(el) 
        , List(l)
        {
        }

        virtual bool Call(UPInt ind)
        {
            SPtr<XML>& child = Element.GetElementChildren()[ind];
            List.List.PushBack(child);
            return true;
        }

    protected:
        Instances::fl::XMLList& List;
    };

    class AttrGet : public XMLElement::CallBack
    {
    public:
        virtual ~AttrGet() { } 
        AttrGet(XMLElement& el, Instances::fl::XMLList& l) 
        : XMLElement::CallBack(el) 
        , List(l)
        {
        }

        virtual bool Call(UPInt ind)
        {
            List.List.PushBack(Element.GetAttr(ind));
            return true;
        }

    protected:
        Instances::fl::XMLList& List;
    };

    ///////////////////////////////////////////////////////////////////////
    XMLElement::XMLElement(InstanceTraits::Traits& t, Namespace& ns, const ASString& n, XML* p)
    : XML(t, n, p)
    {
        Ns = GetVM().MakeNamespace(Abc::NS_Public, ns.GetUri(), ns.GetPrefix());
    }

    XMLElement::~XMLElement()
    {
    }

    void XMLElement::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        XML::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC(prcc, Ns, op SF_DEBUG_ARG(*this));

        const UPInt nsize = Namespaces.GetSize();
        for (UPInt i = 0; i < nsize; ++i)
            AS3::ForEachChild_GC(prcc, Namespaces[i], op SF_DEBUG_ARG(*this));

        UPInt size;
        size = Attrs.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, Attrs[i], op SF_DEBUG_ARG(*this));

        size = Children.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, Children[i], op SF_DEBUG_ARG(*this));
    }

    XML::Kind XMLElement::GetKind() const
    {
        return kElement;
    }

    Namespace& XMLElement::GetCurrNamespace() const
    {
        return *Ns;
    }

    const XML::NamespaceH* XMLElement::GetInScopeNamespaces() const
    {
        return &Namespaces;
    }

    void XMLElement::AddInScopeNamespace(const Namespace& ns)
    {
        // Step 2.
        const Value& nPrefix = ns.GetPrefix();
        if (nPrefix.IsUndefined())
            return;

        const ASString nPrefixString = nPrefix.IsString() ? nPrefix.AsString() 
            : GetStringManager().CreateEmptyString();

        // Step 2.a.
        if (nPrefixString.IsEmpty() && Ns->GetUri().IsEmpty())
            return;

        // Step 2.b.
        SPInt matchPos = -1;

        // Step 2.c.
        const UPInt sizen = Namespaces.GetSize();
        for (UPInt i = 0; i < sizen; ++i)
        {
            if (nPrefix == Namespaces[i]->GetPrefix())
            {
                matchPos = i;
                break;
            }
        }

        SPtr<Namespace> nsCopy = GetVM().MakeNamespace(Abc::NS_Public, ns.GetUri(), ns.GetPrefix());

        // Step 2.d.
        if (matchPos != -1 && Namespaces[matchPos]->GetUri() != ns.GetUri())
            // Step 2.d.i.
            Namespaces.RemoveAt(matchPos);

        // Step 2.e.
        Namespaces.PushBack(nsCopy);

        // Step 2.f.
        if (Ns->GetPrefix() == nPrefix)
            // Step 2.f.i.
            Ns->SetPrefix(Value::GetUndefined());

        // Step 2.g.
        const UPInt sizea = Attrs.GetSize();
        for (UPInt i = 0; i < sizea; ++i)
        {
            Namespace& nsa = Attrs[i]->GetCurrNamespace();

            if (nsa.GetPrefix() == nPrefix)
                nsa.SetPrefix(Value::GetUndefined());
        }
    }

    void XMLElement::AppendChild(const SPtr<XML>& child)
    {
        Children.PushBack(child);
    }

    CheckResult XMLElement::AppendChild(const Value& child)
    {
        return InsertChildAt(Children.GetSize(), child);
    }

    void XMLElement::GetAttributes(Instances::fl::XMLList& list)
    {
        const UPInt size = Attrs.GetSize();

        for (UPInt i = 0; i < size; ++i)
            list.List.PushBack(GetAttr(i));
    }

    void XMLElement::GetChildren(Instances::fl::XMLList& list, Kind k, const ASString* name)
    {
        if (k == kNone)
            list.List = Children;
        else
        {
            bool all = !name || name->IsEmpty() || (*name == "*");

            const UPInt size = Children.GetSize();
            for (UPInt i = 0; i < size; ++i)
            {
                XML* child = Children[i];
                const Kind child_k = child->GetKind();

                if (child_k == k)
                {
                    if (child_k == kInstruction && name)
                    {
                        XMLProcInstr* instr = static_cast<XMLProcInstr*>(child);

                        if (!all && instr->GetName() != *name)
                            continue;
                    }

                    list.Apppend(*child);
                }
            }
        }
    }

    CheckResult XMLElement::GetChildIndex(UPInt& ind) const
    {
        bool result = false;

        if (Parent)
        {
            SF_ASSERT(Parent->GetKind() == kElement);
            XMLElement* parent = static_cast<XMLElement*>(Parent.GetPtr());
            ArrayLH<SPtr<XML> >& children = parent->GetElementChildren();
            const UPInt size = children.GetSize();

            for (ind = 0; ind < size; ++ind)
            {
                if (children[ind].GetPtr() == this)
                {
                    result = true;
                    break;
                }
            }
        }

        return result;
    }

    SPtr<Instances::fl::QName> XMLElement::GetQName()
    {
        InstanceTraits::fl::QName& qn_itr = static_cast<InstanceTraits::fl::QName&>(GetVM().GetClassTraitsQName().GetInstanceTraits());
        return qn_itr.MakeInstance(qn_itr, Text, Ns);
    }

    void XMLElement::ToString(StringBuffer& buf, SInt32 ident)
    {
        if (XMLElement::HasSimpleContent())
        {
            const UPInt csize = Children.GetSize();
            for (UPInt i = 0; i < csize; ++i)
            {
                XML* child = Children[i];

                if (child->GetKind() == kText)
                    child->ToString(buf, ident);
            }
        }
        else
        {
            ToXMLString(buf, ident, NULL, NULL);
        }
    }

    void XMLElement::MakeNsPrefix(StringBuffer& buf, const Namespace& ns, bool checkDefaultNs) const
    {
        if (checkDefaultNs)
        {
            Namespace* defNs = GetVM().GetDefXMLNamespace();
            if (defNs != NULL && defNs->GetUri() == ns.GetUri() && defNs->GetPrefix() == ns.GetPrefix())
            {
                return;
            }
        }

        const Value& pref = ns.GetPrefix();
        if (pref.IsString() && pref.AsStringNode()->Size != 0)
        {
            const ASStringNode* prefix = pref.AsStringNode();

            buf.AppendString(prefix->pData, prefix->Size);
            buf.AppendChar(':');
        }
    }

    void XMLElement::MakeNsSuffix(StringBuffer& buf, const Namespace& ns, bool checkDefaultNs) const
    {
        if (checkDefaultNs)
        {
            Namespace* defNs = GetVM().GetDefXMLNamespace();
            if (defNs != NULL && defNs->GetUri() == ns.GetUri() && defNs->GetPrefix() == ns.GetPrefix())
            {
                return;
            }
        }

        const Value& pref = ns.GetPrefix();
        if (pref.IsString() && pref.AsStringNode()->Size != 0)
        {
            const ASStringNode* prefix = pref.AsStringNode();

            buf.AppendChar(':');
            buf.AppendString(prefix->pData, prefix->Size);
        }
    }

    // 10.2.1
    void XMLElement::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        // ancestorNamespaces is a list of already declared namespaces.

        VM& vm = GetVM();
        const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(GetInstanceTraits().GetClass());
        SInt32 prettyIndent = cl.prettyIndent;
        bool prettyPrinting = cl.prettyPrinting;

        // Bad value causes pretty printing to be disabled
        if (prettyIndent < 0)
        {
            prettyPrinting = false;
            prettyIndent = 0;
        }

        // Step 3
        if (prettyPrinting && ident > 0)
        {
            AppendIdent(buf, ident);
        }

        // Step 8
        // ancestorsAndDeclarations contains namespaces declared in ancestors and in this node.
        // "declared" means that we do not need to declare them in the current node.
        NamespaceArray ancestorsAndDeclarations(vm.GetMemoryHeap());
        if (ancestorNamespaces != NULL)
            ancestorsAndDeclarations.AddUnique(*ancestorNamespaces);

        // Step 9
        // namespaceDeclarations is a collection of namespaces we need to declare in the current node.
        NamespaceArray namespaceDeclarations(vm.GetMemoryHeap());

        // Step 10
        // Add "InScopeNamespaces".
        // Member "Namespaces" is "InScopeNamespaces".
        const UPInt sizei = Namespaces.GetSize();
        for (UPInt i = 0; i < sizei; ++i)
        {
            // Step 10.a
            Namespace& cns = *Namespaces[i];
            // found == true if this namespace is already declared.
            const bool found = ancestorsAndDeclarations.Find(cns);

            // Step 10.a.ii.
            if (!found)
            {
                // We need to declare cns in this node.
                namespaceDeclarations.Add(cns, true);
                // We do not need to check that cns is unique. We already did that.
                ancestorsAndDeclarations.Add(cns, false);
            }
        }
        
        // SS: Add not declared namespaces to the list of namespace declarations.
        if (usedNotDeclared)
        {
            const UPInt size = usedNotDeclared->GetSize();
            for (UPInt i = 0; i < size; ++i)
            {
                Namespace& cns = usedNotDeclared->Get(i);
                // found == true if this namespace is already declared.
                const bool found = ancestorsAndDeclarations.Find(cns);

                if (!found)
                {
                    // We need to declare cns in this node.
                    namespaceDeclarations.Add(cns, true);
                    // We do not need to check that cns is unique. We already did that.
                    ancestorsAndDeclarations.Add(cns, false);
                }
            }
        }

        // Step 11
        // attrNamespaces constrains namespaces used by attributes (and this node itself).
        // It is possible that they are not declared yet.
        NamespaceArray attrNamespaces(vm.GetMemoryHeap());
        SPtr<Namespace> nsCopy = LookupNamespace(*Ns, &ancestorsAndDeclarations);
        attrNamespaces.Add(*nsCopy);

        const UPInt sizeAttrs = Attrs.GetSize();
        for (UPInt i = 0; i < sizeAttrs; ++i)
        {
            const Namespace& nsAttr = Attrs[i]->GetNamespace();
            SPtr<Namespace> nsCopySp = LookupNamespace(nsAttr, &ancestorsAndDeclarations);
            attrNamespaces.Add(*nsCopySp);
        }

        // Size of attrNamespaces is not going to change inside of this loop.
        const UPInt sizeAN = attrNamespaces.GetSize();
        for (UPInt i = 0; i < sizeAN; ++i)
        {
            Namespace& nsAttr = attrNamespaces[i];
            // found == true if this namespace is already declared somewhere.
            const bool found = ancestorsAndDeclarations.Find(nsAttr);

            if (!found && !nsAttr.GetUri().IsEmpty())
            {
                // Step 11.b
                if (nsAttr.GetPrefix().IsUndefined())
                {
                    // Generate artificial prefix.
                    // 
                    // Step 11.b.i.
                    // Step 11.b.ii.
                    // Note: implementations should prefer the empty string as the 
                    // implementation defined prefix if it is not already used in the set
                    // ancestorsAndDeclarations
                    ASString newPrefix = vm.GetStringManager().CreateEmptyString();
                    bool matchFound = true;

                    while (matchFound)
                    {
                        matchFound = ancestorsAndDeclarations.FindByPrefix(newPrefix);

                        if (matchFound)
                            newPrefix.Append("aaa");
                    }

                    nsAttr.SetPrefix(newPrefix);
                }

                // Step 11.b.iii
                namespaceDeclarations.Add(nsAttr);
                ancestorsAndDeclarations.Add(nsAttr);
            }
        }

        // Step 12
        buf.AppendChar('<');

        // Step 13
#if 1
        MakeNsPrefix(buf, attrNamespaces[0], true);
#else
        // Weird. Something is wrong with this.
        MakeNsPrefix(buf, *Ns, true);
#endif

        // Step 14
        buf.AppendString(Text.ToCStr(), Text.GetSize());

        // Step 16. For each an in attrAndNamespaces.
        // Step 16.a (for attributes)
        for (UPInt i = 0; i < Attrs.GetSize(); ++i)
        {
            // Step 16.a
            buf.AppendChar(' ');

            XMLAttr* an = Attrs[i];

            // Step 16.b.i
#if 0
            // Do not delete this.
            // although specification says we should search in ancestorNamespaces
            // we should search in ancestorsAndDeclarations or in namespaceDeclarations.
            SPtr<Namespace> ans = LookupNamespace(an->GetNamespace(), ancestorNamespaces);
#else
            SPtr<Namespace> ans = LookupNamespace(an->GetNamespace(), &ancestorsAndDeclarations);
#endif

            // Step 16.b.ii
            if (!ans->GetUri().IsEmpty() && ans->GetPrefix().IsUndefined())
            {
                // Step 16.b.ii.1
                ASString newPrefix = vm.GetStringManager().CreateEmptyString();
                bool prefixFound = true;
                const UPInt sizeAD = ancestorsAndDeclarations.GetSize();

                while (prefixFound)
                {
                    prefixFound = ancestorsAndDeclarations.FindByPrefix(newPrefix);

                    if (prefixFound)
                        newPrefix.Append("aaa");
                }

                ans->SetPrefix(newPrefix);

                // Step 16.b.ii.2
                bool nsFound = false;
                for (UPInt j = 0; j < sizeAD; ++j)
                {
                    if (ancestorsAndDeclarations[j].GetPrefix().IsNullOrUndefined())
                    {
                        nsFound = (newPrefix.IsEmpty() || ancestorsAndDeclarations[j].GetUri() == ans->GetUri());
                    }

                    if (ancestorsAndDeclarations[j] == *ans)
                    {
                        nsFound = true;
                        break;
                    }
                }

                if (!nsFound)
                {
                    namespaceDeclarations.Add(*ans);
                }
            }

            // Step 16.b.iii
            MakeNsPrefix(buf, *ans, false);
            // Step 16.b.iv
            buf.AppendString(an->GetText().ToCStr(), an->GetText().GetSize());

            // Step 16.d
            buf.AppendString("=\"");
            // Step 16.e.i
            an->ToXMLString(buf, ident, &ancestorsAndDeclarations, NULL);
            // Step 16.g
            buf.AppendChar('"');
        }

        // Step 16.b (for namespaces)
        for (UPInt i = 0; i < namespaceDeclarations.GetSize(); ++i)
        {
            const Namespace& cns = namespaceDeclarations[i];

            // Step 16.a and 16.c.i
            buf.AppendString(" xmlns");

            // Step 16.c.ii
            SPtr<Namespace> ans = vm.MakeNamespace(Abc::NS_Public, cns.GetUri(), cns.GetPrefix());

            if (!ans->GetUri().IsEmpty() && ans->GetPrefix().IsUndefined())
            {
                // Step 16.c.ii.1
                ASString newPrefix = vm.GetStringManager().CreateEmptyString();
                bool prefixFound = true;

                while (prefixFound)
                {
                    prefixFound = ancestorsAndDeclarations.FindByPrefix(newPrefix);

                    if (prefixFound)
                    {
                        newPrefix.Append("aaa");
                    }
                }

                ans->SetPrefix(newPrefix);
            }

            // Step 16.c.iii
            MakeNsSuffix(buf, *ans, true);

            // Step 16.c.iiii
            buf.AppendString("=\"");
            // Step 16.c.iv
            const ASString& uri = ans->GetUri();
            buf.AppendString(uri.ToCStr(), uri.GetSize());
            // Step 16.c.v
            buf.AppendChar('"');
        }

        // Step 17
        const UPInt xLength = Children.GetSize();
        if (xLength == 0)
        {
            buf.AppendString("/>");
            return;
        }

        // Step 18
        buf.AppendChar('>');

        // Step 19
        bool indentChildren = (xLength > 1 || (xLength == 1 && Children[0]->GetKind() != kText));

        // Steps 20, 21
        SInt32 nextIndentLevel = (prettyPrinting && indentChildren) ? ident + prettyIndent : 0;

        // Step 22
        for (UPInt i = 0; i < xLength; ++i)
        {
            // Step 22.a
            if (prettyPrinting && indentChildren)
            {
                buf.AppendChar('\n');                
            }

            // Step 22.b,c
            Children[i]->ToXMLString(buf, nextIndentLevel, &ancestorsAndDeclarations, NULL);
        }

        // Step 23
        if (prettyPrinting && indentChildren)
        {
            // Step 23.a
            buf.AppendChar('\n');                
            // Step 23.b
            AppendIdent(buf, ident);
        }

        // Step 24
        buf.AppendString("</");

        // Step 25
#if 1
        MakeNsPrefix(buf, attrNamespaces[0], true);
#else
        // Weird. Something is wrong with this.
        MakeNsPrefix(buf, *Ns, true);
#endif

        // Step 26
        buf.AppendString(Text.ToCStr(), Text.GetSize());
        buf.AppendChar('>');
    }

    void XMLElement::ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const
    {
        // Tag.
        if (Ns->GetPrefix().IsString())
        {
            const ASString pref = Ns->GetPrefix();
            if (FindNamespaceByPrefix(pref, lp) == NULL && nr.Get(pref) == NULL)
                nr.Add(pref);
        }

        // Attributes.
        const UPInt asize = Attrs.GetSize();
        for (UPInt i = 0; i < asize; ++i)
            Attrs[i]->ResolveNamespaces(nr, lp);

        // Children.
        const UPInt csize = Children.GetSize();
        for (UPInt i = 0; i < csize; ++i)
            Children[i]->ResolveNamespaces(nr, lp);
    }

    XML* XMLElement::RemoveNamespace(const Value& ns)
    {
        SPtr<const Namespace> removeNs;

        if (ns.IsNamespace())
            removeNs = &ns.AsNamespace();
        else
            removeNs = GetVM().MakeNamespace(Abc::NS_Private, ns.AsString());

        if (GetNamespace() == *removeNs)
            return this;

        for (UPInt i = 0; i < Attrs.GetSize(); ++i)
        {
            if (Attrs[i]->GetNamespace() == *removeNs)
                return this;
        }

        // Size of Namespaces changes inside of this loop.
        for (UPInt i = 0; i < Namespaces.GetSize(); ++i)
        {
            Namespace& inScopeNs = *Namespaces[i];
            if (inScopeNs.GetUri() == removeNs->GetUri() && 
                (removeNs->GetPrefix().IsUndefined() || removeNs->GetPrefix() == inScopeNs.GetPrefix()))
            {
                Namespaces.RemoveAt(i);
                break;
            }
        }

        for (UPInt i = 0; i < Children.GetSize(); ++i)
            Children[i]->RemoveNamespace(ns);

        return this;
    }


    void XMLElement::AddAttr(Namespace& ns, const ASString& n, const ASString& v)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());

        Attrs.PushBack(itr.MakeInstanceAttr(itr, ns, n, v, this));
    }

    Boolean3 XMLElement::EqualsInternal(const XML& other)
    {
        Boolean3 result = XML::EqualsInternal(other);

        if (result == undefined3)
        {
            const XMLElement& other_el = static_cast<const XMLElement&>(other);

            if (*Ns != *other_el.Ns)
                return false3;

            if (Attrs.GetSize() != other_el.Attrs.GetSize())
                return false3;

            if (Children.GetSize() != other_el.Children.GetSize())
                return false3;

            // Check attributes.
            {
                const UPInt size = Attrs.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    XMLAttr* attr1 = GetAttr(i);
                    bool found = false;

                    for (UPInt j = 0; j < size; ++j)
                    {
                        XMLAttr* attr2 = other_el.GetAttr(j);

                        if (attr1->NameMatches(*attr2) && attr1->GetData() == attr2->GetData())
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        return false3;
                }
            }

            // Check children.
            {
                const UPInt size = Children.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    bool r;

                    // Theoretically, we shouldn't have any problem here.
                    AbstractEqual(r, Value(Children[i]), Value(other_el.Children[i])).DoNotCheck();

                    if (r == false)
                        return false3;
                }
            }

            result = true3;
        }

        return result;
    }

    bool XML::Matches(const Multiname& prop_name) const
    {
        bool result = false;

        if (prop_name.GetName().IsString())
        {
            const ASString name = prop_name.GetName().AsString();

            if (GetName() == name || prop_name.IsAnyType())
            {
                if (prop_name.IsQName())
                {
                    if (prop_name.IsAnyNamespace())
                        result = true;
                    else
                    {
                        // 9.1.1.1 [[Get]] (P)
                        // When the input argument P is an unqualified XML element name, it identifies XML elements in the default namespace.
                        const Instances::fl::Namespace& ns = prop_name.GetNamespace();

#if 0
                        // All XML namespaces are public.
                        if (ns.GetKind() == Abc::NS_Public)
                        {
                            VM& vm = GetVM();
                            Namespace* def_ns = vm.GetDefXMLNamespace();
                            const ASString& def_uri = def_ns ? def_ns->GetUri() : vm.GetPublicNamespace().GetUri();
                            const ASString& uri = prop_name.IsAttr() || !ns.GetUri().IsEmpty() ? ns.GetUri() : def_uri;

                            if (uri == GetNamespace().GetUri())
                                result = true;
                        }
#else
                        VM& vm = GetVM();
                        Namespace* def_ns = vm.GetDefXMLNamespace();
                        const ASString& def_uri = def_ns ? def_ns->GetUri() : vm.GetPublicNamespace().GetUri();

                        const ASString& uri = ns.GetKind() == Abc::NS_Public && (prop_name.IsAttr() || !ns.GetUri().IsEmpty())? ns.GetUri() : def_uri;

                        if (uri == GetNamespace().GetUri())
                            result = true;
#endif
                    }
                }
                else if (prop_name.IsAnyType())
                    result = true;
                else
                {
                    VM& vm = GetVM();
                    const Namespace& cur_ns = GetNamespace();

                    Namespace* def_ns = NULL;
                    if (!prop_name.IsAttr())
                        // Attributes are not using default namespace.
                        def_ns = vm.GetDefXMLNamespace();

                    if (def_ns == NULL)
                        def_ns = &vm.GetPublicNamespace();

                    if (prop_name.ContainsNamespace(vm.GetPublicNamespace()))
                    {
                        // Match against public namespaces.
                        const NamespaceSet& nss = prop_name.GetNamespaceSet();
                        const NamespaceSet::TContainer& namespaces = nss.GetNamespaces();
                        const UPInt size = namespaces.GetSize();

                        for(UPInt j = 0; j < size; ++j)
                        {
                            const Instances::fl::Namespace& ns = *namespaces[j];
                            if (ns.GetKind() == Abc::NS_Public)
                            {
                                if (ns.GetUri().IsEmpty())
                                {
                                    // URI is not set.
                                    // Match against default namespace.
                                    if (def_ns->GetUri() == cur_ns.GetUri())
                                    {
                                        result = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    if (ns.GetUri() == cur_ns.GetUri())
                                    {
                                        result = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // Match against default namespace.
                        if (*def_ns == cur_ns)
                            result = true;
                    }
                }
            }
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool XMLElement::HasSimpleContent() const
    {
        const UPInt size = Children.GetSize();

        for (UPInt i = 0; i < size; ++i)
        {
            if (Children[i]->GetKind() == kElement)
                return false;
        }

        return true;
    }

    bool XMLElement::HasOwnProperty(const ASString& p)
    {
        bool result = false;

        if (p.IsEmpty()) 
            return result;

        EmptyCallBack cb(*this);
        Multiname mn(GetVM().GetPublicNamespace(), p);

        if (mn.IsAttr())
            result = ForEachAttr(mn, cb) != 0;
        else
            result = ForEachChild(mn, cb) != 0;

        return result;
    }

    Pickable<XML> XMLElement::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        Pickable<XMLElement> r = itr.MakeInstanceElement(itr, *Ns, Text, parent);

        UPInt size;

        // Deep copy Attributes.
        size = Attrs.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            Pickable<XMLAttr> copy(static_cast<XMLAttr*>(GetAttr(i)->DeepCopy(r.GetPtr()).GetPtr()));
            r->Attrs.PushBack(copy);
        }

        // Deep copy Children.
        size = Children.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            // PPS: Do not copy comments if ignoreComments is enabled
            //      Do not copy processing instrs if ignoreProcessingInstructions is enabled
            const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(itr.GetClass());
            if ( !(Children[i]->GetKind() == kComment && cl.ignoreComments) &&
                 !(Children[i]->GetKind() == kInstruction && cl.ignoreProcessingInstructions) )
                r->Children.PushBack(Children[i]->DeepCopy(r.GetPtr()));
        }

        // Deep copy namespaces
        for (UPInt i = 0; i < Namespaces.GetSize(); ++i)
        {
            SPtr<Namespace> nsCopy = GetVM().MakeNamespace(Abc::NS_Public);
            *nsCopy = *Namespaces[i];
            r->Namespaces.PushBack(nsCopy);
        }
        
        return r;
    }

    void XMLElement::GetChildren(Instances::fl::XMLList& list, const Multiname& prop_name)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind <= Children.GetSize())
                list.Apppend(*Children[ind]);
        }
        else
        {
            ChildGet cb(*this, list);
            ForEachChild(prop_name, cb);
        }
    }

    // 9.1.1.2 [[Put]] (P, V)
    CheckResult XMLElement::SetProperty(const Multiname& prop_name, const Value& value)
    {
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();

        // Check for numeric properties.
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            // Step 1.
            vm.ThrowTypeError(VM::Error(VM::eXMLAssignmentToIndexedXMLNotAllowed, vm));
            return false;
        }

        Value c;
        if (IsXMLObject(value))
        {
            // Step 4.
            c = Value(static_cast<XML*>(value.GetObject())->DeepCopy(NULL));
        }
        else if (IsXMLListObject(value))
        {
            // Step 4.
            c = Value(static_cast<XMLList*>(value.GetObject())->DeepCopy(NULL));
        }
        else
        {
            // Step 3.
            ASString str = sm.CreateEmptyString();
            if (!value.Convert2String(str))
                return false;

            c = str;
        }

        // Attributes.
        // Step 6.
        if (prop_name.IsAttr())
        {
            // Step 6.b.
            if (IsXMLListObject(c))
            {
                // Step 6.b.
                XMLList* cList = static_cast<XMLList*>(c.GetObject());
                StringBuffer buf(vm.GetMemoryHeap());
                const UPInt size = cList->List.GetSize();

                // Step 6.b.ii.2.
                for (UPInt i = 0; i < size; ++i)
                {
                    if (i > 0)
                        buf.AppendChar(' ');

                    XML* xml = cList->List[i];
                    xml->ToString(buf, 0);
                }

                // Step 6.b.ii.3.
                c = sm.CreateString(buf.ToCStr(), buf.GetSize());
            }
            else if (IsXMLObject(c))
            {
                XML* cXML = static_cast<XML*>(c.GetObject());
                ASString s = sm.CreateEmptyString();

                // Step 6.b.ii.1.
                cXML->AS3toString(s);
                // Step 6.b.ii.3.
                c = s;
            }
            // Step 6.c.
            else
                // Step 6.c.i.
                c = c.AsString();

            // Step 6.d.
            SPtr<XMLAttr> a;
            // Step 6.e.
            for (UPInt j = 0; j < Attrs.GetSize(); ++j)
            {
                XMLAttr* attr = Attrs[j];

                // Step 6.e.i.

                /* This is what specification says. But it doesn't work.
                if (prop_name.GetName() == Attrs[j]->GetText() && 
                    prop_name.ContainsNamespace(Attrs[j]->GetNamespace()))
                */
                if (attr->Matches(prop_name))
                {
                    // Step 6.e.i.1.
                    if (a == NULL)
                        a = Attrs[j];
                    // Step 6.e.i.2.
                    else
                    {
                        if (!DeleteProperty(prop_name))
                            // Exception.
                            return false;
                    }
                }
            }

            // Step 6.f.
            if (a == NULL)
            {
                InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());

                Namespace* ns = prop_name.IsQName() ? &prop_name.GetNamespace() : vm.GetDefXMLNamespace();
                if (ns == NULL)
                    ns = &vm.GetPublicNamespace();

                a = itr.MakeInstanceAttr(itr, *ns, prop_name.GetName(), c, this);
                Attrs.PushBack(a);
                AddInScopeNamespace(*ns);
            }

            // Step 6.f.
            a->SetData(c);

            // Step 6.h.
            return true;
        }

        bool result = true;

        // Elements.
        Value i;

        // Step 9.
        i.SetUndefined();

        ASString propName = sm.CreateEmptyString();
        if (!prop_name.GetName().Convert2String(propName))
            // Exception.
            return false;

        // Step 10.
        const bool primitiveAssign = !IsXMLObject(c) && !IsXMLListObject(c) && !prop_name.IsAnyType();

        // Step 11.
        // Size of Children can change inside of this loop.
        for (UPInt j = Children.GetSize(); j > 0; --j)
        {
            const UPInt k = j - 1;
            XML* child = Children[k];

            // Step 11.a.
            /* This is what specification says.
            if ((prop_name.IsAnyType() || (child->GetKind() == kElement && child->GetName() == propName)) 
                && (prop_name.ContainsNamespace(vm.GetPublicNamespace()) || (child->GetKind() == kElement && prop_name.ContainsNamespace(child->GetNamespace()))))
            */
            if (child->Matches(prop_name))
            {
                // Step 11.a.i.
                if (!i.IsUndefined())
                    Children.RemoveAt(i.AsInt());

                // Step 11.a.ii.
                i.SetSInt32(static_cast<SInt32>(k));
            }
        }

        // Step 12.
        if (i.IsUndefined())
        {
            // Step 12.a.
            i.SetSInt32(static_cast<SInt32>(Children.GetSize()));

            // Step 12.b.
            if (primitiveAssign)
            {
                InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
                // Step 12.b.i.
                Namespace* ns = prop_name.IsQName() && !prop_name.IsAnyNamespace() ? &prop_name.GetNamespace() : vm.GetDefXMLNamespace();

                if (ns == NULL)
                    ns = &vm.GetPublicNamespace();

                // Step 12.b.iii.
                SPtr<XMLElement> y = itr.MakeInstanceElement(itr, *ns, prop_name.GetName(), this);

                // Step 12.b.v.
                if (!Replace(static_cast<UPInt>(i.AsInt()), Value(y)))
                    return false;

                // Step 12.b.vi.
                y->AddInScopeNamespace(*ns);
            }
        }

        // Step 13.
        if (primitiveAssign)
        {
            SPtr<XML> xml = Children[i.AsInt()];

            // Step 13.a.
            xml->DeleteChildren(NULL);

            // Step 13.c.
            if (!c.AsString().IsEmpty())
            {
                if (xml->GetKind() == kElement)
                    result = static_cast<XMLElement*>(xml.GetPtr())->Replace(0, c);
            }
        }
        // Step 14.
        else
            result = Replace(i.AsInt(), c);

        // Step 15.
        return result;
    }

    CheckResult XMLElement::GetProperty(const Multiname& prop_name, XMLList& list)
    {
        // !!! This can be not correct.
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind < Children.GetSize())
                list.Apppend(*Children[ind]);

            return true;
        }

        // Attributes.
        // 9.1.1.1 [[Get]] (P)
        // When the input argument P is an unqualified XML attribute name, it identifies XML attributes in no namespace.
        // Step 4.
        if (prop_name.IsAttr())
        {
            AttrGet cb(*this, list);
            ForEachAttr(prop_name, cb);

            return true;
        }

        // Elements.
        // 9.1.1.1 [[Get]] (P)
        // When the input argument P is an unqualified XML element name, it identifies XML elements in the default namespace.
        
        ChildGet cb(*this, list);
        return ForEachChild(prop_name, cb) != 0;
    }

    CheckResult XMLElement::GetProperty(const Multiname& prop_name, Value& value)
    {
        // 9.1.1.1
        // Step 1.
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind == 0)
            {
                value = this;
                return true;
            }
            else
                value.SetUndefined();

            return false;
        }

        // We have to return XMLList.
        Pickable<Instances::fl::XMLList> result;

        // Step 3.
        result = MakeXMLListInstance(prop_name);

        value = result;

        return GetProperty(prop_name, *result);
    }

    PropRef XMLElement::FindDynamicSlot(const Multiname& mn)
    {
        // Attributes.
        if (mn.IsAttr())
        {
            AttrGetFirst cb(*this);
            if (ForEachAttr(mn, cb) != 0)
                return PropRef(this, cb.GetFirst().GetPtr());

            return PropRef();
        }

        // Elements.
        ChildGetFirst cb(*this);
        if (ForEachChild(mn, cb) != 0)
            return PropRef(this, cb.GetFirst().GetPtr());

        return PropRef();
    }

    CheckResult XMLElement::DeleteProperty(const Multiname& prop_name)
    {
        // Step 3.
        // Attributes.
        if (prop_name.IsAttr())
        {
            // Step 3.a.
            // Size of Attrs can change during iteration.
            for (UPInt i = 0; i < Attrs.GetSize();)
            {
                XMLAttr* attr = GetAttr(i);
                SF_ASSERT(attr);

                // Step 3.a.i.
                if (attr->Matches(prop_name))
                {
                    // Step 3.a.i.1.
                    attr->SetParent(NULL);

                    // Step 3.a.i.2.
                    Attrs.RemoveAt(i);
                }
                else
                    ++i;
            }

            // Step 3.b.
            return true;
        }

        // Step 5.
        // Elements.
        // Size of Children can change during iteration.
        for (UPInt i = 0; i < Children.GetSize();)
        {
            XML* child = Children[i];
            SF_ASSERT(child);

            // Step 5.a.
            if (child->Matches(prop_name))
            {
                // Step 5.a.i.
                child->SetParent(NULL);

                // Step 5.a.ii.
                Children.RemoveAt(i);
            }
            else
                ++i;
        }

        // Step 7.
        return true;
    }

    bool XMLElement::FindAttr(const Multiname& prop_name, UPInt& i) const
    {
        SF_ASSERT(prop_name.IsAttr());

        if (prop_name.GetName().IsString())
        {
            const ASString name = prop_name.GetName().AsString();
            const UPInt size = Attrs.GetSize();
            for (i = 0; i < size; ++i)
            {
                XMLAttr* attr = GetAttr(i);
                SF_ASSERT(attr);

                if (attr->Matches(prop_name))
                    return true;
            }
        }

        return false;
    }

    bool XMLElement::FindChild(const Multiname& prop_name, UPInt& i) const
    {
        SF_ASSERT(!prop_name.IsAttr());

        if (prop_name.GetName().IsString())
        {
            const ASString name = prop_name.GetName().AsString();
            const UPInt size = Children.GetSize();

            for (i = 0; i < size; ++i)
            {
                XML* child = Children[i];
                SF_ASSERT(child);
                const XML::Kind k = child->GetKind();

                if (k == XML::kElement)
                {
                    XMLElement* other = static_cast<XMLElement*>(child);
                    SF_ASSERT(other);

                    if (other->Matches(prop_name))
                        return true;
                }
            }
        }

        return false;
    }

    UInt32 XMLElement::ForEachAttr(const Multiname& prop_name, XMLElement::CallBack& cb)
    {
        SF_ASSERT(prop_name.IsAttr());
        UInt32 result = 0;

        if (prop_name.GetName().IsString())
        {
            const UPInt size = Attrs.GetSize();

            for (UPInt i = 0; i < size; ++i)
            {
                XMLAttr* attr = GetAttr(i);
                SF_ASSERT(attr);

                if (attr->Matches(prop_name))
                {
                    ++result;
                    if (!cb.Call(i))
                        break;
                }
            }
        }

        return result;
    }

    UInt32 XMLElement::ForEachChild(const Multiname& prop_name, XMLElement::CallBack& cb)
    {
        SF_ASSERT(!prop_name.IsAttr());
        UInt32 result = 0;

        const UPInt size = Children.GetSize();

        for (UPInt i = 0; i < size; ++i)
        {
            XML* child = Children[i];
            SF_ASSERT(child);

            if (child->Matches(prop_name))
            {
                ++result;
                if (!cb.Call(i))
                    break;
            }
        }

        return result;
    }

    const Namespace& XMLElement::GetNamespace() const
    {
        return *Ns;
    }

    void XMLElement::SetNamespace(Namespace& ns)
    {
#if 0
        Ns = GetVM().MakeNamespace(Abc::NS_Public, ns.GetUri(), ns.GetPrefix());
#else
        Ns = &ns;
#endif

        // Change namespace of all attributes
        const UPInt size = Attrs.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            XMLAttr* attr = GetAttr(i);
            attr->SetNamespace(ns);
        }
    }

    void XMLElement::SetNamespace2(Namespace& ns)
    {
        Ns = &ns;
    }

    void XMLElement::GetDescendants(XMLList& list, const Multiname& prop_name)
    {
        // Attributes.
        if (prop_name.IsAttr())
        {
            AttrGet cb(*this, list);
            ForEachAttr(prop_name, cb);
        }

        // Elements.
        const UPInt size = Children.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            XML* child = Children[i];
            SF_ASSERT(child);

            if (!prop_name.IsAttr())
            {
                if (child->Matches(prop_name))
                    list.Apppend(*child);
            }

            child->GetDescendants(list, prop_name);
        }
    }

    XML* XMLElement::ToXML(const Value& value) const
    {
        XML* v = NULL;

        if (IsXMLObject(value))
            v = static_cast<XML*>(value.GetObject());
        else if (IsXMLListObject(value))
        {
            XMLList* list = static_cast<XMLList*>(value.GetObject());
            if (list->GetSize() == 1)
                v = list->List[0];
        }

        return v;
    }

    // 9.1.1.6 [[HasProperty]] (P)
    bool XMLElement::HasProperty(const Multiname& prop_name, bool check_prototype)
    {
        SF_UNUSED1(check_prototype);

        // Step 1.
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            return ind == 0;

        EmptyCallBack cb(*this);

        // Step 3.
        if (prop_name.IsAttr())
        {
            // Step 3.a.
            if (ForEachAttr(prop_name, cb) != 0)
                // Step 3.a.i.1.
                return true;

            // Step 3.b.
            return false;
        }

        // Step 4.
        // Element.
        if (ForEachChild(prop_name, cb) != 0)
            // Step 4.a.i.
            return true;
        
        // Step 5.
        return false;
    }

    CheckResult XMLElement::InsertChildAt(UPInt pos, const Value& value)
    {
        VM& vm = GetVM();
        bool result = false;

        if (IsXMLListObject(value))
        {
            XMLList* list = static_cast<XMLList*>(value.GetObject());
            const UPInt size = list->GetSize();

            for (UPInt i = 0; i < size; ++i)
            {
                XML* v = list->List[i];

                // Check for Cyclical Loop.
                XML* node = this;
                while (node)
                {
                    if (node == v)
                    {
                        vm.ThrowTypeError(VM::Error(VM::eXMLIllegalCyclicalLoop, vm));
                        return result;
                    }

                    node = node->GetParent();
                }

                // Insert.
                const UPInt sizeInsert = Children.GetSize();
                if (pos + i < sizeInsert)
                {
                    if (v->GetKind() == kText)
                        Children.InsertAt(pos + i, v->DeepCopy(this));
                    else
                    {
                        Children.InsertAt(pos + i, v);
                        // ??? Should we make a deep copy instead?
                        v->SetParent(this);
                    }

                    result = true;
                }
                else if (pos + i == sizeInsert)
                {
                    if (v->GetKind() == kText)
                        Children.PushBack(v->DeepCopy(this));
                    else
                    {
                        Children.PushBack(v);
                        // ??? Should we make a deep copy instead?
                        v->SetParent(this);
                    }

                    result = true;
                }
            }
        }
        else
        {
            SPtr<XML> v;
            if (IsXMLObject(value))
                v = static_cast<XML*>(value.GetObject());
            else
            {
                // Create new XML object.
                Value _this;
                InstanceTraits::Traits& itr = GetInstanceTraits();

                itr.GetClass().Construct(_this, 1, &value, true);
                if (GetVM().IsException())
                    return result;

                if (_this.IsNull())
                {
                    // Can happen if trying to insert a comment with ignoreComments=false, for example
                    return result;
                }

                v = static_cast<XML*>(_this.GetObject());
            }

            // Check for Cyclical Loop.
            XML* node = this;
            while (node)
            {
                if (node == v)
                {
                    vm.ThrowTypeError(VM::Error(VM::eXMLIllegalCyclicalLoop, vm));
                    return result;
                }

                node = node->GetParent();
            }

            // Insert.
            const UPInt size = Children.GetSize();
            if (pos < size)
            {
                if (v->GetKind() == kText)
                    Children.InsertAt(pos, v->DeepCopy(this));
                else
                {
                    Children.InsertAt(pos, v);
                    // ??? Should we make a deep copy instead?
                    v->SetParent(this);
                }

                result = true;
            }
            else if (pos == size)
            {
                if (v->GetKind() == kText)
                    Children.PushBack(v->DeepCopy(this));
                else
                {
                    Children.PushBack(v);
                    // ??? Should we make a deep copy instead?
                    v->SetParent(this);
                }

                result = true;
            }
        }

        return result;
    }

    CheckResult XMLElement::InsertChildAfter(const Value& child1, const Value& child2)
    {
        bool result = false;

        if (child1.IsNullOrUndefined())
            result = InsertChildAt(0, child2);
        else
        {
            XML* c1 = ToXML(child1);

            if (c1)
            {
                const UPInt size = Children.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    //if (Children[i]->Equals(*c1))
                    if (Children[i].GetPtr() == c1)
                    {
                        result = InsertChildAt((UInt32)(i + 1), child2);
                        break;
                    }
                }
            }
        }

        return result;
    }

    CheckResult XMLElement::InsertChildBefore(const Value& child1, const Value& child2)
    {
        bool result = false;

        if (child1.IsNullOrUndefined())
            result = InsertChildAt((UInt32)Children.GetSize(), child2);
        else
        {
            XML* c1 = ToXML(child1);

            if (c1)
            {
                const UPInt size = Children.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    //if (Children[i]->Equals(*c1))
                    if (Children[i].GetPtr() == c1)
                    {
                        result = InsertChildAt((UInt32)i, child2);
                        break;
                    }
                }
            }
        }

        return result;
    }

    void XMLElement::DeleteChildren(XML* child)
    {
        const UPInt size = Children.GetSize();

        if (child)
        {
            for (UPInt i = 0; i < size; ++i)
            {
                XML* xml = Children[i];

                if (xml == child)
                {
                    if (xml)
                        xml->SetParent(NULL);

                    Children.RemoveAt(i);

                    break;
                }
            }
        }
        else
        {
            for (UPInt i = 0; i < size; ++i)
            {
                XML* xml = Children[i];

                if (xml)
                    xml->SetParent(NULL);
            }

            Children.Clear();
        }
    }

    void XMLElement::DeleteByIndex(UPInt ind)
    {
        // Step 2.a.
        if (ind < Children.GetSize())
        {
            XML* xml = Children[ind];

            // Step 2.a.i.1.
            if (xml)
                xml->SetParent(NULL);

            // Step 2.a.i.2.
            Children.RemoveAt(ind);
        }
    }

    static bool IsWhiteSpaceString(const ASString& str)
    {
        for (unsigned i = 0; i < str.GetSize(); i++)
        {
            if (!ASUtils::IsWhiteSpace(str[i])) return false;
        }
        return true;
    }

    void XMLElement::Normalize()
    {
        XMLText* txt = NULL;
        UPInt size = Children.GetSize();

        for (UPInt i = 0; i < size; ++i)
        {
            XML* child = Children[i];

            if (child->GetKind() == kText)
            {
                XMLText* txt_child = static_cast<XMLText*>(child);

                if (txt)
                {
                    if (!txt_child->GetText().IsEmpty())
                        txt->GetText().Append(txt_child->GetText());

                    Children.RemoveAt(i);
                    --i;
                    --size;
                }
                else
                {
                    // PPS: If empty or whitespace string, then remove
                    if (!txt_child->GetText().IsEmpty() && !IsWhiteSpaceString(txt_child->GetText()) )
                        txt = txt_child;
                    else
                    {
                        Children.RemoveAt(i);
                        --i;
                        --size;
                    }
                }
            }
            else
                txt = NULL;
        }
    }

    Namespace* XMLElement::FindNamespaceByPrefix(const ASString& prefix, XML* lp) const
    {
        Namespace* result = NULL;
        const SPtr<Namespace>* r = NULL;
        const UPInt nsize = Namespaces.GetSize();
        for (UPInt i = 0; i < nsize; ++i)
        {
            const Namespace& ns = *Namespaces[i];
            const Value& pref = ns.GetPrefix();

            if (pref.IsString() && pref.AsString() == prefix)
            {
                r = &Namespaces[i];
                break;
            }
        }

        if (r)
            result = *r;
        else
            result = XML::FindNamespaceByPrefix(prefix, lp);

        return result;
    }

    Namespace* XMLElement::FindNamespaceByURI(const ASString& uri, XML* lp) const
    {
        Namespace* result = NULL;
        const SPtr<Namespace>* r = NULL;
        const UPInt nsize = Namespaces.GetSize();

        for (UPInt i = 0; i < nsize; ++i)
        {
            const Namespace& ns = *Namespaces[i];
            const ASString& cur_uri = ns.GetUri();

            if (cur_uri == uri)
            {
                r = &Namespaces[i];
                break;
            }
        }

        if (r)
            result = *r;
        else
            result = XML::FindNamespaceByPrefix(uri, lp);

        return result;
    }

    // 13.4.4.32
    CheckResult XMLElement::AS3Replace(const Multiname& prop_name, const Value& value)
    {
        VM& vm = GetVM();
        Value c;

        if (IsXMLObject(value))
            // Step 3.
            c = Value(static_cast<XML*>(value.GetObject())->DeepCopy(NULL));
        else if (IsXMLListObject(value))
            // Step 3.
            c = Value(static_cast<XMLList*>(value.GetObject())->DeepCopy(NULL));
        else
        {
            // Step 2.
            ASString s = vm.GetStringManager().CreateEmptyString();
            if (!value.Convert2String(s))
                return false;

            c = s;
        }

        // Step 4.
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            // Step 4.a.
            return Replace(ind, c);

        Value i;

        // Step 6.
        i.SetUndefined();

        // Step 7.
        for (UPInt j = Children.GetSize(); j > 0; --j)
        {
            const UPInt k = j - 1;
            XML* child = Children[k];

            // Step 7.a.
            /* This is from documentation.
            const XML::Kind childKind = child->GetKind();
            if ((prop_name.IsAnyType() || (childKind == kElement && child->GetName() == propString)) 
                && (prop_name.ContainsNamespace(vm.GetPublicNamespace()) || (childKind == kElement && prop_name.ContainsNamespace(child->GetNamespace()))))
            */
            if (child->Matches(prop_name))
            {
                // Step 7.a.i.
                if (!i.IsUndefined())
                    // DeleteByIndex(i) is not the same as Children.RemoveAt(i);
                    DeleteByIndex(i.AsInt());

                // Step 7.a.ii.
                i.SetSInt32(static_cast<SInt32>(k));;
            }
        }

        bool result = true;
        if (!i.IsUndefined())
            // Step 9.
            result = Replace(i.AsInt(), c);

        // Step 10.
        return result;
    }

    CheckResult XMLElement::Replace(const Multiname& prop_name, const Value& value)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            return Replace(ind, value);

        GetVM().ThrowTypeError(VM::Error(VM::eXMLAssignmentToIndexedXMLNotAllowed, GetVM()));
        return false;
    }

    // 9.1.1.12 [[Replace]] (P, V)
    CheckResult XMLElement::Replace(UPInt i, const Value& value)
    {
        bool result = true;

        // Step 4.
        if (i >= Children.GetSize())
        {
            Children.Resize(Children.GetSize() + 1);

            // Step 4.a.
            i = Children.GetSize() - 1;

            // ??? Step 4.b.
            Children[i] = NULL;
        }

        // Step 5.
        if (IsXMLObject(value) && static_cast<XML*>(value.GetObject())->GetKind() != kAttr)
        {
            XML* xmlValue = static_cast<XML*>(value.GetObject());
            
            // Step 5.a.
            if (static_cast<XML*>(value.GetObject())->GetKind() == XML::kElement)
            {
                XML* node = this;
                while (node != NULL)
                {
                    if (node == xmlValue)
                    {
                        GetVM().ThrowTypeError(VM::Error(VM::eXMLIllegalCyclicalLoop, GetVM()));
                        return false;
                    }

                    node = node->GetParent();
                }
            }

            // Step 5.b.
            xmlValue->SetParent(this);
            
            // Step 5.c.
            if (Children[i])
                // Step 5.c.i.
                Children[i]->SetParent(NULL);

            // Step 5.d.
            Children[i] = xmlValue;
        }
        // Step 6.
        else if (IsXMLListObject(value))
        {
            // Step 6.a.
            // Call the [[DeleteByIndex]] method of x with argument P.
            // DeleteByIndex(i) is not the same as Children.RemoveAt(i);
            DeleteByIndex(i);
            // Step 6.b.
            // Call the [[Insert]] method of x with arguments P and V.
            result = InsertChildAt(i, value);
        }
        // Step 7.
        else
        {
            // Step 7.c.
            if (Children[i])
                Children[i]->SetParent(NULL);

            InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
            // Step 7.b.
            // Step 7.d.
            Children[i] = itr.MakeInstanceText(itr, value.AsString(), this);
        }

        return result;
    }

    void XMLElement::SetChildren(const Value& value)
    {
        VM& vm = GetVM();
        Children.Clear();

        if (IsXMLObject(value))
        {
            XML* xml = static_cast<XML*>(value.GetObject());

            // Check for Cyclical Loop.
            XML* node = this;
            while (node)
            {
                if (node == xml)
                    return vm.ThrowTypeError(VM::Error(VM::eXMLIllegalCyclicalLoop, vm));

                node = node->GetParent();
            }

            // Set.
            xml->SetParent(this);
            Children.PushBack(xml);
        }
        else if (IsXMLListObject(value))
        {
            XMLList* list = static_cast<XMLList*>(value.GetObject());
            const UPInt size = list->GetSize();

            for (UPInt i = 0; i < size; ++i)
            {
                XML* xml = list->List[i];

                // Check for Cyclical Loop.
                XML* node = this;
                while (node)
                {
                    if (node == xml)
                        return vm.ThrowTypeError(VM::Error(VM::eXMLIllegalCyclicalLoop, vm));

                    node = node->GetParent();
                }

                xml->SetParent(this);
            }

            // Set.
            Children = list->List;
        }
        else
        {
            // Let's make a text node.
            ASString txt = vm.GetStringManager().CreateEmptyString();
            if (value.Convert2String(txt))
            {
                InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetTraits());
                Pickable<Instances::fl::XML> el = itr.MakeInstanceText(itr, txt, this);

                Children.PushBack(el);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    XMLText::XMLText(InstanceTraits::Traits& t, const ASString& txt, XML* p)
    : XML(t, txt, p)
    {
    }

    XML::Kind XMLText::GetKind() const
    {
        return kText;
    }

    bool XMLText::HasSimpleContent() const
    {
        return true;
    }

    void XMLText::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        SF_UNUSED2(ancestorNamespaces, usedNotDeclared);

        const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(GetInstanceTraits().GetClass());
        SInt32 prettyIndent = cl.prettyIndent;
        bool prettyPrinting = cl.prettyPrinting;

        // Bad value causes pretty printing to be disabled
        if (prettyIndent < 0)
        {
            prettyPrinting = false;
            prettyIndent = 0;
        }

        // Step 3
        if (prettyPrinting && ident > 0)
            AppendIdent(buf, ident);

        // Step 4
        if (static_cast<const Classes::fl::XML&>(GetInstanceTraits().GetClass()).prettyPrinting)
            EscapeElementValue(buf, Text.TruncateWhitespace());
        else
            EscapeElementValue(buf, Text);
    }

    Pickable<XML> XMLText::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstanceText(itr, Text, parent);
    }


    ///////////////////////////////////////////////////////////////////////
    XMLAttr::XMLAttr(InstanceTraits::Traits& t, Namespace& ns, const ASString& n, const ASString& v, XML* p)
    : XML(t, n, p)
    , Data(v)
    {
        Ns = GetVM().MakeNamespace(Abc::NS_Public, ns.GetUri(), ns.GetPrefix());
    }

    XMLAttr::~XMLAttr()
    {
    }

    bool XMLAttr::NameMatches(const XMLAttr& other) const
    {
        if (Text != other.Text)
            return false;

        return (Ns.GetPtr() == NULL && other.Ns.GetPtr() == NULL) ||
            ((Ns.GetPtr() != NULL && other.Ns.GetPtr() != NULL) && *Ns == *other.Ns);
    }

    void XMLAttr::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        XML::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC(prcc, Ns, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC(prcc, Data, op SF_DEBUG_ARG(*this));
    }

    XML::Kind XMLAttr::GetKind() const
    {
        return kAttr;
    }

    Namespace& XMLAttr::GetCurrNamespace() const
    {
        return *Ns;
    }

    void XMLAttr::ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const
    {
        if (Ns->GetPrefix().IsString())
        {
            const ASString pref = Ns->GetPrefix();
            if (FindNamespaceByPrefix(pref, lp) == NULL && nr.Get(pref) == NULL)
                nr.Add(pref);
        }
    }

    void XMLAttr::SetNamespace(Namespace& ns) 
    { 
#if 0
        Ns = GetVM().MakeNamespace(Abc::NS_Public, ns.GetUri(), ns.GetPrefix());
#else
        Ns = &ns;
#endif
    }

    void XMLAttr::SetNamespace2(Namespace& ns) 
    { 
        Ns = &ns;
    }

    SPtr<Instances::fl::QName> XMLAttr::GetQName()
    {
        InstanceTraits::fl::QName& qn_itr = static_cast<InstanceTraits::fl::QName&>(GetVM().GetClassTraitsQName().GetInstanceTraits());

#ifdef SF_URI_AS_NAMESPACE
        return qn_itr.MakeInstance(qn_itr, Text, Ns);
#else
        if (Ns.GetPtr())
            return qn_itr.MakeInstance(qn_itr, Text, Ns->GetUri());

        return qn_itr.MakeInstance(qn_itr, Text, GetVM().GetStringManager().GetBuiltin(AS3Builtin_empty_));
#endif
    }

    void XMLAttr::ToString(StringBuffer& buf, SInt32 /*ident*/)
    {
        buf.AppendString(Data.ToCStr(), Data.GetSize());
    }

    void XMLAttr::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        SF_UNUSED3(ident, ancestorNamespaces, usedNotDeclared);
        EscapeAttributeValue(buf, Data);
    }

    Boolean3 XMLAttr::EqualsInternal(const XML& other)
    {
        Boolean3 result = XML::EqualsInternal(other);

        if (result == undefined3)
            result = Data == static_cast<const XMLAttr&>(other).Data ? true3 : false3;

        return result;
    }

    bool XMLAttr::HasSimpleContent() const
    {
        return true;
    }

    Pickable<XML> XMLAttr::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstanceAttr(itr, *Ns, Text, Data, parent);
    }

    const Namespace& XMLAttr::GetNamespace() const
    {
        return *Ns;
    }

    ///////////////////////////////////////////////////////////////////////
    XMLProcInstr::XMLProcInstr(InstanceTraits::Traits& t, const ASString& n, const ASString& v, XML* p)
    : XML(t, n, p)
    , Data(v)
    {
    }

    XML::Kind XMLProcInstr::GetKind() const
    {
        return XML::kInstruction;
    }

    Pickable<XML> XMLProcInstr::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstanceProcInstr(itr, Text, Data, parent);
    }

    SPtr<Instances::fl::QName> XMLProcInstr::GetQName()
    {
        InstanceTraits::fl::QName& qn_itr = static_cast<InstanceTraits::fl::QName&>(GetVM().GetClassTraitsQName().GetInstanceTraits());
        return qn_itr.MakeInstance(qn_itr, Text, &GetVM().GetPublicNamespace());
    }

    void XMLProcInstr::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        SF_UNUSED2(ancestorNamespaces, usedNotDeclared);
        const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(GetInstanceTraits().GetClass());
        SInt32 prettyIndent = cl.prettyIndent;
        bool prettyPrinting = cl.prettyPrinting;

        // Bad value causes pretty printing to be disabled
        if (prettyIndent < 0)
        {
            prettyPrinting = false;
            prettyIndent = 0;
        }

        if (prettyPrinting && ident > 0)
            AppendIdent(buf, ident);

        buf.AppendString("<?", 2);
        buf.AppendString(Text.ToCStr(), Text.GetSize());
        buf.AppendChar(' ');
        buf.AppendString(Data.ToCStr(), Data.GetSize());
        buf.AppendString("?>", 2);
    }

    Boolean3 XMLProcInstr::EqualsInternal(const XML& other)
    {
        Boolean3 result = XML::EqualsInternal(other);

        if (result == undefined3)
            result = Data == static_cast<const XMLProcInstr&>(other).Data ? true3 : false3;

        return result;
    }


    ///////////////////////////////////////////////////////////////////////
    XMLComment::XMLComment(InstanceTraits::Traits& t, const ASString& n, XML* p)
        : XML(t, n, p)
    {
    }

    XML::Kind XMLComment::GetKind() const
    {
        return XML::kComment;
    }

    Pickable<XML> XMLComment::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstanceComment(itr, Text, parent);
    }

    void XMLComment::ToString(StringBuffer& buf, SInt32 ident)
    {
        ToXMLString(buf, ident, NULL, NULL);
    }

    void XMLComment::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        SF_UNUSED2(ancestorNamespaces, usedNotDeclared);

        const Classes::fl::XML& cl = static_cast<const Classes::fl::XML&>(GetInstanceTraits().GetClass());
        SInt32 prettyIndent = cl.prettyIndent;
        bool prettyPrinting = cl.prettyPrinting;

        // Bad value causes pretty printing to be disabled
        if (prettyIndent < 0)
        {
            prettyPrinting = false;
            prettyIndent = 0;
        }

        // Step 3
        if (prettyPrinting && ident > 0)
        {
            AppendIdent(buf, ident);
        }

        // Step 6
        buf.AppendString("<!--", 4);
        buf.AppendString(Text.ToCStr(), Text.GetSize());
        buf.AppendString("-->", 3);
    }

    Boolean3 XMLComment::EqualsInternal(const XML& other)
    {
        Boolean3 result = XML::EqualsInternal(other);

        if (result == undefined3)
            result = true3;

        return result;
    }

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    Pickable<Instances::fl::XML> XML::MakeInstance(Traits& t, const ASString& n, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XML));
        return Pickable<Instances::fl::XML>(new(mem) Instances::fl::XML(t, n, p));
    }

    Pickable<Instances::fl::XMLElement> XML::MakeInstanceElement(Traits& t, Instances::fl::Namespace& ns, const ASString& n, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XMLElement));
        return Pickable<Instances::fl::XMLElement>(new(mem) Instances::fl::XMLElement(t, ns, n, p));
    }

    Pickable<Instances::fl::XMLText> XML::MakeInstanceText(Traits& t, const ASString& txt, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XMLText));
        return Pickable<Instances::fl::XMLText>(new(mem) Instances::fl::XMLText(t, txt, p));
    }

    Pickable<Instances::fl::XMLText> XML::MakeInstanceText(Traits& t, const StringDataPtr& txt, Instances::fl::XML* p)
    {
        VM& vm = t.GetVM();
        StringManager& sm = vm.GetStringManager();
        const ASString s = sm.CreateString(txt.ToCStr(), txt.GetSize());
        void* mem = Memory::AllocInHeap(vm.GetMemoryHeap(), sizeof(Instances::fl::XMLText));
        return Pickable<Instances::fl::XMLText>(new(mem) Instances::fl::XMLText(t, s, p));
    }

    Pickable<Instances::fl::XMLAttr> XML::MakeInstanceAttr(Traits& t, Instances::fl::Namespace& ns, const ASString& n, const ASString& v, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XMLAttr));
        return Pickable<Instances::fl::XMLAttr>(new(mem) Instances::fl::XMLAttr(t, ns, n, v, p));
    }

    Pickable<Instances::fl::XMLProcInstr> XML::MakeInstanceProcInstr(Traits& t, const ASString& n, const ASString& v, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XMLProcInstr));
        return Pickable<Instances::fl::XMLProcInstr>(new(mem) Instances::fl::XMLProcInstr(t, n, v, p));
    }

    Pickable<Instances::fl::XMLComment> XML::MakeInstanceComment(Traits& t, const ASString& n, Instances::fl::XML* p)
    {
        void* mem = Memory::AllocInHeap(GetVM().GetMemoryHeap(), sizeof(Instances::fl::XMLComment));
        return Pickable<Instances::fl::XMLComment>(new(mem) Instances::fl::XMLComment(t, n, p));
    }
}} // namespace InstanceTraits

CheckResult IsNameStartChar(int c)
{
    if (c == ':' || c == '_')
    {
        return true;
    }
    if (c >= 'A' && c <= 'Z')
    {
        return true;
    }
    if (c >= 'a' && c <= 'z')
    {
        return true;
    }
    if (c >= 0xC0 && c <= 0xD6)
    {
        return true;
    }
    if (c >= 0xD8 && c <= 0xF6)
    {
        return true;
    }
    if (c >= 0xF8 && c <= 0x2FF)
    {
        return true;
    }
    if (c >= 0x370 && c <= 0x37D)
    {
        return true;
    }
    if (c >= 0x37F && c <= 0x1FFF)
    {
        return true;
    }
    if (c >= 0x200C && c <= 0x200D)
    {
        return true;
    }
    if (c >= 0x2070 && c <= 0x218F)
    {
        return true;
    }
    if (c >= 0x2C00 && c <= 0x2FEF)
    {
        return true;
    }
    if (c >= 0x3001 && c <= 0xD7FF)
    {
        return true;
    }
    if (c >= 0xF900 && c <= 0xFDCF)
    {
        return true;
    }
    if (c >= 0xFDF0 && c <= 0xFFFD)
    {
        return true;
    }
    if (c >= 0x10000 && c <= 0xEFFFF)
    {
        return true;
    }
    return false;
}

CheckResult IsNameChar(int c)
{
    if (IsNameStartChar(c))
    {
        return true;
    }
    if (c == '-' || c == '.')
    {
        return true;
    }
    if (c >= '0' && c <= '9')
    {
        return true;
    }
    if (c == 0xB7)
    {
        return true;
    }
    if (c >= 0x0300 && c <= 0x036F)
    {
        return true;
    }
    if (c >= 0x203F && c <= 0x2040)
    {
        return true;
    }
    return false;
}

CheckResult IsNCNameChar(int c)
{
    return (IsNameChar(c) && c != ':');
}

CheckResult IsNCNameStartChar(int c)
{
    return (IsNameStartChar(c) && c != ':');
}

CheckResult IsValidName(const ASString& name)
{
    if (name.GetLength() == 0)
        return false;

    if (!IsNameStartChar(name[0]))
        return false;

    const unsigned nameSize = name.GetLength();
    for (unsigned i = 1; i < nameSize; ++i)
    {
        if (!IsNameChar(name[i]))
            return false;
    }

    return true;
}

Pickable<Instances::fl::Namespace> LookupNamespace(const Instances::fl::Namespace& qName, const NamespaceArray* inScopeNamespaces)
{
    // 13.3.5.4 [[GetNamespace]] ( [ InScopeNamespaces ] )
    Pickable<Instances::fl::Namespace> resultNs = qName.GetVM().MakeNamespace(Abc::NS_Public, qName.GetUri(), qName.GetPrefix());

    // Step 2.
    if (inScopeNamespaces != NULL)
    {
        const UPInt size = inScopeNamespaces->GetSize();

        for (UPInt i = 0; i < size; ++i)
        {
            Instances::fl::Namespace& ns = inScopeNamespaces->Get(i);

            // Step 3.
            // Find a Namespace ns in InScopeNamespaces, such that ns.uri == q.uri.
            // if (ns  == *resultNs)
            if (ns.GetUri() == resultNs->GetUri())
            {
                resultNs->SetPrefix(ns.GetPrefix());
                break;
            }
        }
    }

    return resultNs;
}
#endif

//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    Value GetMethodDefArg<Classes::fl::XML, Classes::fl::XML::mid_AS3setSettings, 0, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Classes::fl::XML, Classes::fl::XML::mid_setSettings, 0, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_prototypeGet, SPtr<Instances::fl::XMLList> > TFunc_Instances_XML_prototypeGet;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_prototypeSet, const Value, const Value&> TFunc_Instances_XML_prototypeSet;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3toString, ASString> TFunc_Instances_XML_AS3toString;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3hasOwnProperty, bool, const ASString&> TFunc_Instances_XML_AS3hasOwnProperty;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3propertyIsEnumerable, Value, unsigned, const Value*> TFunc_Instances_XML_AS3propertyIsEnumerable;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3addNamespace, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_XML_AS3addNamespace;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3appendChild, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_XML_AS3appendChild;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3attribute, SPtr<Instances::fl::XMLList>, const Value&> TFunc_Instances_XML_AS3attribute;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3attributes, SPtr<Instances::fl::XMLList> > TFunc_Instances_XML_AS3attributes;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3child, SPtr<Instances::fl::XMLList>, const Value&> TFunc_Instances_XML_AS3child;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3childIndex, SInt32> TFunc_Instances_XML_AS3childIndex;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3children, SPtr<Instances::fl::XMLList> > TFunc_Instances_XML_AS3children;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3comments, SPtr<Instances::fl::XMLList> > TFunc_Instances_XML_AS3comments;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3contains, bool, const Value&> TFunc_Instances_XML_AS3contains;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3copy, SPtr<Instances::fl::XML> > TFunc_Instances_XML_AS3copy;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3descendants, Value, unsigned, const Value*> TFunc_Instances_XML_AS3descendants;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3elements, Value, unsigned, const Value*> TFunc_Instances_XML_AS3elements;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3hasComplexContent, bool> TFunc_Instances_XML_AS3hasComplexContent;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3hasSimpleContent, bool> TFunc_Instances_XML_AS3hasSimpleContent;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3inScopeNamespaces, SPtr<Instances::fl::Array> > TFunc_Instances_XML_AS3inScopeNamespaces;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3insertChildAfter, Value, const Value&, const Value&> TFunc_Instances_XML_AS3insertChildAfter;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3insertChildBefore, Value, const Value&, const Value&> TFunc_Instances_XML_AS3insertChildBefore;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3length, SInt32> TFunc_Instances_XML_AS3length;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3localName, ASString> TFunc_Instances_XML_AS3localName;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3name, SPtr<Instances::fl::QName> > TFunc_Instances_XML_AS3name;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3namespace_, Value, unsigned, const Value*> TFunc_Instances_XML_AS3namespace_;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3namespaceDeclarations, SPtr<Instances::fl::Array> > TFunc_Instances_XML_AS3namespaceDeclarations;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3nodeKind, ASString> TFunc_Instances_XML_AS3nodeKind;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3normalize, SPtr<Instances::fl::XML> > TFunc_Instances_XML_AS3normalize;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3parent, Value> TFunc_Instances_XML_AS3parent;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3processingInstructions, Value, unsigned, const Value*> TFunc_Instances_XML_AS3processingInstructions;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3prependChild, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_XML_AS3prependChild;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3removeNamespace, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_XML_AS3removeNamespace;
typedef ThunkFunc2<Instances::fl::XML, Instances::fl::XML::mid_AS3replace, SPtr<Instances::fl::XML>, const Value&, const Value&> TFunc_Instances_XML_AS3replace;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3setChildren, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_XML_AS3setChildren;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3setLocalName, const Value, const Value&> TFunc_Instances_XML_AS3setLocalName;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3setName, const Value, const Value&> TFunc_Instances_XML_AS3setName;
typedef ThunkFunc1<Instances::fl::XML, Instances::fl::XML::mid_AS3setNamespace, const Value, const Value&> TFunc_Instances_XML_AS3setNamespace;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3text, SPtr<Instances::fl::XMLList> > TFunc_Instances_XML_AS3text;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3toXMLString, ASString> TFunc_Instances_XML_AS3toXMLString;
typedef ThunkFunc0<Instances::fl::XML, Instances::fl::XML::mid_AS3valueOf, SPtr<Instances::fl::XML> > TFunc_Instances_XML_AS3valueOf;

template <> const TFunc_Instances_XML_prototypeGet::TMethod TFunc_Instances_XML_prototypeGet::Method = &Instances::fl::XML::prototypeGet;
template <> const TFunc_Instances_XML_prototypeSet::TMethod TFunc_Instances_XML_prototypeSet::Method = &Instances::fl::XML::prototypeSet;
template <> const TFunc_Instances_XML_AS3toString::TMethod TFunc_Instances_XML_AS3toString::Method = &Instances::fl::XML::AS3toString;
template <> const TFunc_Instances_XML_AS3hasOwnProperty::TMethod TFunc_Instances_XML_AS3hasOwnProperty::Method = &Instances::fl::XML::AS3hasOwnProperty;
template <> const TFunc_Instances_XML_AS3propertyIsEnumerable::TMethod TFunc_Instances_XML_AS3propertyIsEnumerable::Method = &Instances::fl::XML::AS3propertyIsEnumerable;
template <> const TFunc_Instances_XML_AS3addNamespace::TMethod TFunc_Instances_XML_AS3addNamespace::Method = &Instances::fl::XML::AS3addNamespace;
template <> const TFunc_Instances_XML_AS3appendChild::TMethod TFunc_Instances_XML_AS3appendChild::Method = &Instances::fl::XML::AS3appendChild;
template <> const TFunc_Instances_XML_AS3attribute::TMethod TFunc_Instances_XML_AS3attribute::Method = &Instances::fl::XML::AS3attribute;
template <> const TFunc_Instances_XML_AS3attributes::TMethod TFunc_Instances_XML_AS3attributes::Method = &Instances::fl::XML::AS3attributes;
template <> const TFunc_Instances_XML_AS3child::TMethod TFunc_Instances_XML_AS3child::Method = &Instances::fl::XML::AS3child;
template <> const TFunc_Instances_XML_AS3childIndex::TMethod TFunc_Instances_XML_AS3childIndex::Method = &Instances::fl::XML::AS3childIndex;
template <> const TFunc_Instances_XML_AS3children::TMethod TFunc_Instances_XML_AS3children::Method = &Instances::fl::XML::AS3children;
template <> const TFunc_Instances_XML_AS3comments::TMethod TFunc_Instances_XML_AS3comments::Method = &Instances::fl::XML::AS3comments;
template <> const TFunc_Instances_XML_AS3contains::TMethod TFunc_Instances_XML_AS3contains::Method = &Instances::fl::XML::AS3contains;
template <> const TFunc_Instances_XML_AS3copy::TMethod TFunc_Instances_XML_AS3copy::Method = &Instances::fl::XML::AS3copy;
template <> const TFunc_Instances_XML_AS3descendants::TMethod TFunc_Instances_XML_AS3descendants::Method = &Instances::fl::XML::AS3descendants;
template <> const TFunc_Instances_XML_AS3elements::TMethod TFunc_Instances_XML_AS3elements::Method = &Instances::fl::XML::AS3elements;
template <> const TFunc_Instances_XML_AS3hasComplexContent::TMethod TFunc_Instances_XML_AS3hasComplexContent::Method = &Instances::fl::XML::AS3hasComplexContent;
template <> const TFunc_Instances_XML_AS3hasSimpleContent::TMethod TFunc_Instances_XML_AS3hasSimpleContent::Method = &Instances::fl::XML::AS3hasSimpleContent;
template <> const TFunc_Instances_XML_AS3inScopeNamespaces::TMethod TFunc_Instances_XML_AS3inScopeNamespaces::Method = &Instances::fl::XML::AS3inScopeNamespaces;
template <> const TFunc_Instances_XML_AS3insertChildAfter::TMethod TFunc_Instances_XML_AS3insertChildAfter::Method = &Instances::fl::XML::AS3insertChildAfter;
template <> const TFunc_Instances_XML_AS3insertChildBefore::TMethod TFunc_Instances_XML_AS3insertChildBefore::Method = &Instances::fl::XML::AS3insertChildBefore;
template <> const TFunc_Instances_XML_AS3length::TMethod TFunc_Instances_XML_AS3length::Method = &Instances::fl::XML::AS3length;
template <> const TFunc_Instances_XML_AS3localName::TMethod TFunc_Instances_XML_AS3localName::Method = &Instances::fl::XML::AS3localName;
template <> const TFunc_Instances_XML_AS3name::TMethod TFunc_Instances_XML_AS3name::Method = &Instances::fl::XML::AS3name;
template <> const TFunc_Instances_XML_AS3namespace_::TMethod TFunc_Instances_XML_AS3namespace_::Method = &Instances::fl::XML::AS3namespace_;
template <> const TFunc_Instances_XML_AS3namespaceDeclarations::TMethod TFunc_Instances_XML_AS3namespaceDeclarations::Method = &Instances::fl::XML::AS3namespaceDeclarations;
template <> const TFunc_Instances_XML_AS3nodeKind::TMethod TFunc_Instances_XML_AS3nodeKind::Method = &Instances::fl::XML::AS3nodeKind;
template <> const TFunc_Instances_XML_AS3normalize::TMethod TFunc_Instances_XML_AS3normalize::Method = &Instances::fl::XML::AS3normalize;
template <> const TFunc_Instances_XML_AS3parent::TMethod TFunc_Instances_XML_AS3parent::Method = &Instances::fl::XML::AS3parent;
template <> const TFunc_Instances_XML_AS3processingInstructions::TMethod TFunc_Instances_XML_AS3processingInstructions::Method = &Instances::fl::XML::AS3processingInstructions;
template <> const TFunc_Instances_XML_AS3prependChild::TMethod TFunc_Instances_XML_AS3prependChild::Method = &Instances::fl::XML::AS3prependChild;
template <> const TFunc_Instances_XML_AS3removeNamespace::TMethod TFunc_Instances_XML_AS3removeNamespace::Method = &Instances::fl::XML::AS3removeNamespace;
template <> const TFunc_Instances_XML_AS3replace::TMethod TFunc_Instances_XML_AS3replace::Method = &Instances::fl::XML::AS3replace;
template <> const TFunc_Instances_XML_AS3setChildren::TMethod TFunc_Instances_XML_AS3setChildren::Method = &Instances::fl::XML::AS3setChildren;
template <> const TFunc_Instances_XML_AS3setLocalName::TMethod TFunc_Instances_XML_AS3setLocalName::Method = &Instances::fl::XML::AS3setLocalName;
template <> const TFunc_Instances_XML_AS3setName::TMethod TFunc_Instances_XML_AS3setName::Method = &Instances::fl::XML::AS3setName;
template <> const TFunc_Instances_XML_AS3setNamespace::TMethod TFunc_Instances_XML_AS3setNamespace::Method = &Instances::fl::XML::AS3setNamespace;
template <> const TFunc_Instances_XML_AS3text::TMethod TFunc_Instances_XML_AS3text::Method = &Instances::fl::XML::AS3text;
template <> const TFunc_Instances_XML_AS3toXMLString::TMethod TFunc_Instances_XML_AS3toXMLString::Method = &Instances::fl::XML::AS3toXMLString;
template <> const TFunc_Instances_XML_AS3valueOf::TMethod TFunc_Instances_XML_AS3valueOf::Method = &Instances::fl::XML::AS3valueOf;

namespace Instances { namespace fl
{
    XML::XML(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::XML::XML()$data"
#ifdef GFX_ENABLE_XML
    , Text(t.GetVM().GetStringManager().CreateEmptyString())
#endif
//##protect##"instance::XML::XML()$data"
    {
//##protect##"instance::XML::XML()$code"
//##protect##"instance::XML::XML()$code"
    }

    void XML::prototypeGet(SPtr<Instances::fl::XMLList>& result)
    {
//##protect##"instance::XML::prototypeGet()"
        SF_UNUSED1(result);

#ifdef GFX_ENABLE_XML
        result = MakeXMLListInstance();
#endif
//##protect##"instance::XML::prototypeGet()"
    }
    void XML::prototypeSet(const Value& result, const Value& p)
    {
//##protect##"instance::XML::prototypeSet()"
        SF_UNUSED2(result, p);
//##protect##"instance::XML::prototypeSet()"
    }
    void XML::AS3toString(ASString& result)
    {
//##protect##"instance::XML::AS3toString()"
        SF_UNUSED1(result);

#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        StringBuffer buf(vm.GetMemoryHeap());

        ToString(buf, 0);
        result = vm.GetStringManager().CreateString(buf.ToCStr(), buf.GetSize());
#endif
//##protect##"instance::XML::AS3toString()"
    }
    void XML::AS3hasOwnProperty(bool& result, const ASString& p)
    {
//##protect##"instance::XML::AS3hasOwnProperty()"
#ifdef GFX_ENABLE_XML
        result = HasOwnProperty(p);
#else
        SF_UNUSED2(p, result);
        WARN_NOT_IMPLEMENTED("XML::AS3hasOwnProperty()");
#endif
//##protect##"instance::XML::AS3hasOwnProperty()"
    }
    void XML::AS3propertyIsEnumerable(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::XML::AS3propertyIsEnumerable()"
#ifdef GFX_ENABLE_XML
        if (argc > 0 && !argv[0].IsNullOrUndefined())
        {
            Multiname prop_name(GetVM().GetPublicNamespace(), argv[0]);
            UInt32 ind;
            if (GetVectorInd(prop_name, ind) && ind == 0)
            {
                result.SetBool(true);
                return;
            }
        }
#else
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("XML::AS3propertyIsEnumerable()");
#endif

        result.SetBool(false);
//##protect##"instance::XML::AS3propertyIsEnumerable()"
    }
    void XML::AS3addNamespace(SPtr<Instances::fl::XML>& result, const Value& ns)
    {
//##protect##"instance::XML::AS3addNamespace()"
#ifdef GFX_ENABLE_XML
        if (!ns.IsNullOrUndefined())
        {
            VM& vm = GetVM();
            Value vns;

            vm.GetClassNamespace().Construct(vns, 1, &ns, true);

            if (vm.IsException())
                return;

            AddInScopeNamespace(vns.AsNamespace());
        }
#else
        SF_UNUSED2(result, ns);
        WARN_NOT_IMPLEMENTED("XML::AS3addNamespace()");
#endif

        result = this;
//##protect##"instance::XML::AS3addNamespace()"
    }
    void XML::AS3appendChild(SPtr<Instances::fl::XML>& result, const Value& child)
    {
//##protect##"instance::XML::AS3appendChild()"
#ifdef GFX_ENABLE_XML
        AppendChild(child).DoNotCheck();
        SF_UNUSED1(result);
#else
        SF_UNUSED2(result, child);
        WARN_NOT_IMPLEMENTED("XML::AS3appendChild()");
#endif
//##protect##"instance::XML::AS3appendChild()"
    }
    void XML::AS3attribute(SPtr<Instances::fl::XMLList>& result, const Value& arg)
    {
//##protect##"instance::XML::AS3attribute()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();

        if (arg.IsNullOrUndefined())
            return vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("arg")));

        Multiname prop_name(vm, arg);
        prop_name.SetAttr();

        if (vm.IsException())
            return;

        // We have to return XMLList.
        result = MakeXMLListInstance(prop_name);

        GetProperty(prop_name, *result).DoNotCheck();
#else
        SF_UNUSED2(result, arg);
        WARN_NOT_IMPLEMENTED("XML::AS3attribute()");
#endif
//##protect##"instance::XML::AS3attribute()"
    }
    void XML::AS3attributes(SPtr<Instances::fl::XMLList>& result)
    {
//##protect##"instance::XML::AS3attributes()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::XMLList> list = MakeXMLListInstance();

        result = list;
        GetAttributes(*list);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3attributes()");
#endif
//##protect##"instance::XML::AS3attributes()"
    }
    void XML::AS3child(SPtr<Instances::fl::XMLList>& result, const Value& propertyName)
    {
//##protect##"instance::XML::AS3child()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();

        if (propertyName.IsNullOrUndefined())
            return vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("propertyName")));

        // This constructor can throw exception.
        Multiname prop_name(vm, propertyName);

        if (vm.IsException())
            return;

        // We have to return XMLList.
        result = MakeXMLListInstance(prop_name);

        // 13.4.4.6.
        // Step 1 and 2.
        GetProperty(prop_name, *result).DoNotCheck();
#else
        SF_UNUSED2(result, propertyName);
        WARN_NOT_IMPLEMENTED("XML::AS3child()");
#endif
//##protect##"instance::XML::AS3child()"
    }
    void XML::AS3childIndex(SInt32& result)
    {
//##protect##"instance::XML::AS3childIndex()"
        result = -1;

#ifdef GFX_ENABLE_XML
        UPInt ind;

        if (GetChildIndex(ind))
            result = static_cast<SInt32>(ind);
#else
        WARN_NOT_IMPLEMENTED("XML::AS3childIndex()");
#endif
//##protect##"instance::XML::AS3childIndex()"
    }
    void XML::AS3children(SPtr<Instances::fl::XMLList>& result)
    {
//##protect##"instance::XML::AS3children()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::XMLList> list = MakeXMLListInstance();
        result = list;
        GetChildren(*list, kNone);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3children()");
#endif
//##protect##"instance::XML::AS3children()"
    }
    void XML::AS3comments(SPtr<Instances::fl::XMLList>& result)
    {
//##protect##"instance::XML::AS3comments()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::XMLList> list = MakeXMLListInstance(*this, GetVM().GetStringManager().CreateNullString(), GetVM().GetPublicNamespace());
        result = list;
        GetChildren(*list, kComment);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3comments()");
#endif
//##protect##"instance::XML::AS3comments()"
    }
    void XML::AS3contains(bool& result, const Value& value)
    {
//##protect##"instance::XML::AS3contains()"
        result = false;

#ifdef GFX_ENABLE_XML
        if (!IsXMLObject(value))
            return;

        Instances::fl::XML* other = static_cast<Instances::fl::XML*>(value.GetObject());
        result = Equals(*other);
#else
        SF_UNUSED1(value);
        WARN_NOT_IMPLEMENTED("XML::AS3contains()");
#endif
//##protect##"instance::XML::AS3contains()"
    }
    void XML::AS3copy(SPtr<Instances::fl::XML>& result)
    {
//##protect##"instance::XML::AS3copy()"
#ifdef GFX_ENABLE_XML
        result = DeepCopy(NULL);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3copy()");
#endif
//##protect##"instance::XML::AS3copy()"
    }
    void XML::AS3descendants(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::XML::AS3descendants()"
#ifdef GFX_ENABLE_XML
        Pickable<XMLList> list = MakeXMLListInstance();
        result = list;

        Multiname mn(GetVM().GetPublicNamespace(), argc == 0 ? Value(GetVM().GetStringManager().CreateConstString("*")) : argv[0]);
        GetDescendants(*list, mn);
#else
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("XML::AS3descendants()");
#endif
//##protect##"instance::XML::AS3descendants()"
    }
    void XML::AS3elements(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::XML::AS3elements()"
#ifdef GFX_ENABLE_XML
        Pickable<XMLList> list = MakeXMLListInstance();
        result = list;

        Multiname mn(GetVM().GetPublicNamespace(), argc == 0 ? Value(GetVM().GetStringManager().CreateConstString("*")) : argv[0]);
        GetChildren(*list, mn);
#else
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("XML::AS3elements()");
#endif
//##protect##"instance::XML::AS3elements()"
    }
    void XML::AS3hasComplexContent(bool& result)
    {
//##protect##"instance::XML::AS3hasComplexContent()"
#ifdef GFX_ENABLE_XML
        result = !HasSimpleContent();
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3hasComplexContent()");
#endif
//##protect##"instance::XML::AS3hasComplexContent()"
    }
    void XML::AS3hasSimpleContent(bool& result)
    {
//##protect##"instance::XML::AS3hasSimpleContent()"
#ifdef GFX_ENABLE_XML
        result = HasSimpleContent();
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3hasSimpleContent()");
#endif
//##protect##"instance::XML::AS3hasSimpleContent()"
    }
    void XML::AS3inScopeNamespaces(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::XML::AS3inScopeNamespaces()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::Array> a = GetVM().MakeArray();
        result = a;

        const NamespaceH* pnsh;

        HashSet<Value, Value::HashFunctor> dupeCheck;

        for (XML* xml = this; xml; xml = xml->GetParent())
        {
            pnsh = xml->GetInScopeNamespaces();
            if (pnsh)
            {
                const NamespaceH& nsh = *pnsh;
                const UPInt size = nsh.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    // PPS: Do not add namespaces with the same prefix as one that has already been added
                    Instances::fl::Namespace* ns = nsh[i];
                    Value prefix = ns->GetPrefix();                    
                    if (!dupeCheck.Get(prefix))
                    {
                        a->PushBack(Value(ns));
                        dupeCheck.Add(prefix);
                    }
                }
            }
        }

        // PPS: Add public(?) namespace by default
        if (a->GetSize() == 0)
            a->PushBack(Value(&GetVM().GetPublicNamespace()));
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3inScopeNamespaces()");
#endif
//##protect##"instance::XML::AS3inScopeNamespaces()"
    }
    void XML::AS3insertChildAfter(Value& result, const Value& child1, const Value& child2)
    {
//##protect##"instance::XML::AS3insertChildAfter()"
#ifdef GFX_ENABLE_XML
        if (InsertChildAfter(child1, child2))
            result = this;
        else
            result.SetUndefined();
#else
        SF_UNUSED3(result, child1, child2);
        WARN_NOT_IMPLEMENTED("XML::AS3insertChildAfter()");
#endif
//##protect##"instance::XML::AS3insertChildAfter()"
    }
    void XML::AS3insertChildBefore(Value& result, const Value& child1, const Value& child2)
    {
//##protect##"instance::XML::AS3insertChildBefore()"
#ifdef GFX_ENABLE_XML
        if (InsertChildBefore(child1, child2))
            result = this;
        else
            result.SetUndefined();
#else
        SF_UNUSED3(result, child1, child2);
        WARN_NOT_IMPLEMENTED("XML::AS3insertChildBefore()");
#endif
//##protect##"instance::XML::AS3insertChildBefore()"
    }
    void XML::AS3length(SInt32& result)
    {
//##protect##"instance::XML::AS3length()"
        result = 1;
//##protect##"instance::XML::AS3length()"
    }
    void XML::AS3localName(ASString& result)
    {
//##protect##"instance::XML::AS3localName()"
#ifdef GFX_ENABLE_XML
        result = Text;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3localName()");
#endif
//##protect##"instance::XML::AS3localName()"
    }
    void XML::AS3name(SPtr<Instances::fl::QName>& result)
    {
//##protect##"instance::XML::AS3name()"
#ifdef GFX_ENABLE_XML
        result = GetQName();
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3name()");
#endif
//##protect##"instance::XML::AS3name()"
    }
    void XML::AS3namespace_(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::XML::AS3namespace_()"
#ifdef GFX_ENABLE_XML
        if (argc == 0)
        {
            Namespace& ns = GetCurrNamespace();
            result = &ns;
        }
        else
        {
            VM& vm = GetVM();
            ASString pref = vm.GetStringManager().CreateEmptyString();

            if (!argv[0].Convert2String(pref))
                return;

            Namespace* pns = FindNamespaceByPrefix(pref, NULL);
            if (pns)
                result = pns;
            else
                result.SetUndefined();
        }
#else
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("XML::AS3namespace_()");
#endif
//##protect##"instance::XML::AS3namespace_()"
    }
    void XML::AS3namespaceDeclarations(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::XML::AS3namespaceDeclarations()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        Pickable<Instances::fl::Array> a = vm.MakeArray();
        result = a;

        XML::Kind kind = GetKind();
        if (kind == kText || kind == kComment || kind == kInstruction || kind == kAttr)
            return;

        NamespaceArray ancestorNS(vm.GetMemoryHeap());
        for (XML* y = GetParent(); y != NULL; y = y->GetParent())
        {
            const NamespaceH* pnsh = y->GetInScopeNamespaces();
            if (pnsh)
            {
                const NamespaceH& nsh = *pnsh;

                for (UPInt i = 0; i < nsh.GetSize(); ++i)
                {
                    if (!nsh[i]->GetPrefix().IsNullOrUndefined())
                    {
                        bool exists = false;
                        for (UPInt j = 0; j < ancestorNS.GetSize(); ++j)
                        {
                            if (ancestorNS[j].GetPrefix() == nsh[i]->GetPrefix())
                            {
                                exists = true;
                                break;
                            }
                        }

                        if (!exists)
                            ancestorNS.Add(*nsh[i]);
                    }
                }
            }        
        }

        NamespaceArray declaredNS(vm.GetMemoryHeap());
        const NamespaceH* pnsh = GetInScopeNamespaces();
        if (pnsh)
        {
            const NamespaceH& nsh = *pnsh;

            for (UPInt i = 0; i < nsh.GetSize(); ++i)
            {
                if (!nsh[i]->GetPrefix().IsNullOrUndefined())
                    declaredNS.Add(*nsh[i]);
            }
        }        

        for (UPInt i = 0; i < declaredNS.GetSize(); ++i)
            a->PushBack(Value(&declaredNS[i]));
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3namespaceDeclarations()");
#endif

//##protect##"instance::XML::AS3namespaceDeclarations()"
    }
    void XML::AS3nodeKind(ASString& result)
    {
//##protect##"instance::XML::AS3nodeKind()"
#ifdef GFX_ENABLE_XML
        const char* kind = NULL;
        switch (GetKind())
        {
        case kNone:
            SF_ASSERT(false);
            break;
        case kElement:
            kind = "element";
            break;
        case kText:
            kind = "text";
            break;
        case kComment:
            kind = "comment";
            break;
        case kInstruction:
            kind = "processing-instruction";
            break;
        case kAttr:
            kind = "attribute";
            break;
        }

        result = kind;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3nodeKind()");
#endif
//##protect##"instance::XML::AS3nodeKind()"
    }
    void XML::AS3normalize(SPtr<Instances::fl::XML>& result)
    {
//##protect##"instance::XML::AS3normalize()"
#ifdef GFX_ENABLE_XML
        Normalize();
#else
        WARN_NOT_IMPLEMENTED("XML::AS3normalize()");
#endif
        result = this;
//##protect##"instance::XML::AS3normalize()"
    }
    void XML::AS3parent(Value& result)
    {
//##protect##"instance::XML::AS3parent()"
#ifdef GFX_ENABLE_XML
        if (Parent.GetPtr() == NULL)
            result.SetUndefined();
        else
            result = Parent;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3parent()");
#endif
//##protect##"instance::XML::AS3parent()"
    }
    void XML::AS3processingInstructions(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::XML::AS3processingInstructions()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::XMLList> list = MakeXMLListInstance();
        result = list;

        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();
        ASString name = sm.CreateEmptyString();

        if (argc > 0) argv[0].Convert2String(name).DoNotCheck();
        GetChildren(*list, kInstruction, &name);
#else
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("XML::AS3processingInstructions()");
#endif
//##protect##"instance::XML::AS3processingInstructions()"
    }
    void XML::AS3prependChild(SPtr<Instances::fl::XML>& result, const Value& value)
    {
//##protect##"instance::XML::AS3prependChild()"
#ifdef GFX_ENABLE_XML
        if (InsertChildAt(0, value))
            result = this;
        else
            result = NULL;
#else
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("XML::AS3prependChild()");
#endif
//##protect##"instance::XML::AS3prependChild()"
    }
    void XML::AS3removeNamespace(SPtr<Instances::fl::XML>& result, const Value& ns)
    {
//##protect##"instance::XML::AS3removeNamespace()"
#ifdef GFX_ENABLE_XML
        result = RemoveNamespace(ns);
#else
        SF_UNUSED2(result, ns);
        WARN_NOT_IMPLEMENTED("XML::AS3removeNamespace()");
#endif
//##protect##"instance::XML::AS3removeNamespace()"
    }
    void XML::AS3replace(SPtr<Instances::fl::XML>& result, const Value& propertyName, const Value& value)
    {
//##protect##"instance::XML::AS3replace()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        const Multiname prop_name(vm, propertyName);

        if (vm.IsException())
            return;

        bool isXmlValue = false;
        if (GetVM().GetValueTraits(value).GetTraitsType() == Traits_String)
        {
            Value xmlValue;
            GetClass().Construct(xmlValue, 1, &value);
            if (GetVM().IsException())
                return;

            if (!AS3Replace(prop_name, xmlValue))
                return;

            isXmlValue = true;
        }

        if (!isXmlValue)
        {
            if (!AS3Replace(prop_name, value))
                return;
        }
#else
        SF_UNUSED2(propertyName, value);
        WARN_NOT_IMPLEMENTED("XML::AS3replace()");
#endif

        result = this;
//##protect##"instance::XML::AS3replace()"
    }
    void XML::AS3setChildren(SPtr<Instances::fl::XML>& result, const Value& value)
    {
//##protect##"instance::XML::AS3setChildren()"
#ifdef GFX_ENABLE_XML
        SetChildren(value);
#else
        SF_UNUSED1(value);
        WARN_NOT_IMPLEMENTED("XML::AS3setChildren()");
#endif
        result = this;
//##protect##"instance::XML::AS3setChildren()"
    }
    void XML::AS3setLocalName(const Value& result, const Value& name)
    {
//##protect##"instance::XML::AS3setLocalName()"
        SF_UNUSED1(result);
#ifdef GFX_ENABLE_XML
        XML::Kind kind = GetKind();

        if (kind == kText || kind == kComment)
            return;

        VM& vm = GetVM();
        if (IsQNameObject(name))
            Text = static_cast<QName*>(name.GetObject())->GetLocalName();
        else if (!name.IsUndefined())
            name.Convert2String(Text).DoNotCheck();

        if (!IsValidName(Text))
            return vm.ThrowTypeError(VM::Error(VM::eXMLInvalidName, vm));
#else
        SF_UNUSED1(name);
        WARN_NOT_IMPLEMENTED("XML::AS3setLocalName()");
#endif
//##protect##"instance::XML::AS3setLocalName()"
    }
    void XML::AS3setName(const Value& result, const Value& name)
    {
//##protect##"instance::XML::AS3setName()"
        SF_UNUSED1(result);
#ifdef GFX_ENABLE_XML
        XML::Kind kind = GetKind();
        if (kind == kText || kind == kComment)
            return;

        VM& vm = GetVM();
        ASString localName = vm.GetStringManager().CreateEmptyString();
        Namespace* ns = NULL;
        if (IsQNameObject(name))
        {
            QName* qName = static_cast<QName*>(name.GetObject());
            if (qName->GetNamespace() != NULL)
                SetNamespace(*qName->GetNamespace());

            localName = qName->GetLocalName();
            ns = qName->GetNamespace();
        }
        else if (!name.IsUndefined())
        {
            if (!name.Convert2String(localName))
                return vm.ThrowTypeError(VM::Error(VM::eXMLInvalidName, vm));
        }

        if (!IsValidName(localName))
            return vm.ThrowTypeError(VM::Error(VM::eXMLInvalidName, vm));

        Text = localName;

        if (ns != NULL)
        {
            if (kind == kAttr)
            {
                if (GetParent() != NULL)
                    GetParent()->AddInScopeNamespace(*ns);
            }
            else if (kind == kElement)
                AddInScopeNamespace(*ns);
        }
        SetNamespace(ns ? *ns : vm.GetPublicNamespace());
#else
        SF_UNUSED1(name);
        WARN_NOT_IMPLEMENTED("XML::AS3setName()");
#endif
//##protect##"instance::XML::AS3setName()"
    }
    void XML::AS3setNamespace(const Value& result, const Value& ns)
    {
//##protect##"instance::XML::AS3setNamespace()"
        // ECMA 13.4.4.36

        SF_UNUSED1(result);

#ifdef GFX_ENABLE_XML
        XML::Kind kind = GetKind();

        // Step 1.
        if (kind == kText || kind == kComment || kind == kInstruction)
            return;

        VM& vm = GetVM();

        // Step 2.
        SPtr<Namespace> ns2 = vm.MakeNamespace(Abc::NS_Public);
        ns2->AS3Constructor(1, &ns);

#if 0
        // This doesn't seem to be necessary.
        if (ns2->GetPrefix().IsUndefined())
            ns2->SetPrefix(vm.GetStringManager().CreateEmptyString());
#endif

        // Step 3.
//         SetNamespace2(*ns2);

        // Step 4.
        if (kind == kAttr)
        {
            // Step 4.a.
            if (GetParent() == NULL)
                return;

            // Step 4.b.
            GetParent()->AddInScopeNamespace(*ns2);
        }
        // Step 5.
        else if (kind == kElement)
            // Step 5.a.
            AddInScopeNamespace(*ns2);

        SetNamespace(*ns2);
#else
        SF_UNUSED1(ns);
        WARN_NOT_IMPLEMENTED("XML::AS3setNamespace()");
#endif
//##protect##"instance::XML::AS3setNamespace()"
    }
    void XML::AS3text(SPtr<Instances::fl::XMLList>& result)
    {
//##protect##"instance::XML::AS3text()"
#ifdef GFX_ENABLE_XML
        Pickable<Instances::fl::XMLList> list = MakeXMLListInstance();
        result = list;
        GetChildren(*list, kText);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3text()");
#endif
//##protect##"instance::XML::AS3text()"
    }
    void XML::AS3toXMLString(ASString& result)
    {
//##protect##"instance::XML::AS3toXMLString()"
        SF_UNUSED1(result);

#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        StringBuffer buf(vm.GetMemoryHeap());

        ToXMLString(buf, 0, NULL, NULL);
        result = vm.GetStringManager().CreateString(buf.ToCStr(), buf.GetSize());
#else
        WARN_NOT_IMPLEMENTED("XML::AS3toXMLString()");
#endif
//##protect##"instance::XML::AS3toXMLString()"
    }
    void XML::AS3valueOf(SPtr<Instances::fl::XML>& result)
    {
//##protect##"instance::XML::AS3valueOf()"
        result = this;
//##protect##"instance::XML::AS3valueOf()"
    }

    SPtr<Instances::fl::XMLList> XML::prototypeGet()
    {
        SPtr<Instances::fl::XMLList> result;
        prototypeGet(result);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3attribute(const Value& arg)
    {
        SPtr<Instances::fl::XMLList> result;
        AS3attribute(result, arg);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3attributes()
    {
        SPtr<Instances::fl::XMLList> result;
        AS3attributes(result);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3child(const Value& propertyName)
    {
        SPtr<Instances::fl::XMLList> result;
        AS3child(result, propertyName);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3children()
    {
        SPtr<Instances::fl::XMLList> result;
        AS3children(result);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3comments()
    {
        SPtr<Instances::fl::XMLList> result;
        AS3comments(result);
        return result;
    }
    SPtr<Instances::fl::Array> XML::AS3inScopeNamespaces()
    {
        SPtr<Instances::fl::Array> result;
        AS3inScopeNamespaces(result);
        return result;
    }
    SPtr<Instances::fl::QName> XML::AS3name()
    {
        SPtr<Instances::fl::QName> result;
        AS3name(result);
        return result;
    }
    SPtr<Instances::fl::Array> XML::AS3namespaceDeclarations()
    {
        SPtr<Instances::fl::Array> result;
        AS3namespaceDeclarations(result);
        return result;
    }
    SPtr<Instances::fl::XMLList> XML::AS3text()
    {
        SPtr<Instances::fl::XMLList> result;
        AS3text(result);
        return result;
    }
//##protect##"instance$methods"
#ifdef GFX_ENABLE_XML

    XML::XML(InstanceTraits::Traits& t, const ASString& n, XML* p) 
    : Instances::fl::Object(t)
    , Text(n)
    , Parent(p)
    {
    }

    XML::~XML()
    {
    }

    void XML::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC(prcc, Parent, op SF_DEBUG_ARG(*this));
    }

    XML::Kind XML::GetKind() const
    {
        return kNone;
    }

    Pickable<Instances::fl::XML> XML::MakeInstance()
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstance(itr);
    }

    Pickable<Instances::fl::XMLList> XML::MakeXMLListInstance()
    {
        InstanceTraits::fl::XMLList& itr = static_cast<InstanceTraits::fl::XMLList&>(GetVM().GetXMLSupport().GetClassTraitsXMLList()->GetInstanceTraits());
        return itr.MakeInstance(itr);
    }

    Pickable<XMLList> XML::MakeXMLListInstance(const Multiname& mn)
    {
        Pickable<XMLList> result;

        if (mn.GetName().IsString())
        {
            Namespace* ns = NULL;
            
            if (mn.IsQName() && !mn.IsAnyNamespace())
            {
                ns = &mn.GetNamespace();

                if (ns->GetPrefix().IsUndefined())
                {
                    // Namespace can have prefix in this XML.
                    Namespace* local_ns = FindNamespaceByURI(ns->GetUri(), NULL);
                    if (local_ns)
                        ns = local_ns;
                }
            }
            else
            {
                ns = GetVM().GetDefXMLNamespace();
                if (ns == NULL)
                    ns = &GetVM().GetPublicNamespace();
                else if (ns->GetPrefix().IsUndefined())
                {
                    // Default namespace can have prefix in this XML.
                    Namespace* local_ns = FindNamespaceByURI(ns->GetUri(), NULL);
                    if (local_ns)
                        ns = local_ns;
                }
            }

            result = MakeXMLListInstance(
                *this,
                mn.GetName().AsString(),
                *ns
                );
        }
        else
            result = MakeXMLListInstance();

        return result;
    }

    Pickable<Instances::fl::XMLList> XML::MakeXMLListInstance(Instances::fl::Object& target_obj, const ASString& target_prop, Namespace& target_ns)
    {
        InstanceTraits::fl::XMLList& itr = static_cast<InstanceTraits::fl::XMLList&>(GetVM().GetXMLSupport().GetClassTraitsXMLList()->GetInstanceTraits());
        return itr.MakeInstance(itr, target_obj, target_prop, target_ns);
    }

    Boolean3 XML::EqualsInternal(const XML& other)
    {
        if (this == &other)
            return true3;

        if (GetKind() != other.GetKind())
            return false3;

        if (Text != other.Text)
            return false3;

        return undefined3;
    }

    bool XML::HasSimpleContent() const
    {
        return false;
    }

    bool XML::HasOwnProperty(const ASString& /*p*/)
    {
        return false;
    }

    const ASString& XML::GetName() const 
    {
        return Text;
    }

    const Namespace& XML::GetNamespace() const
    {
        return GetVM().GetPublicNamespace();
    }

    Namespace& XML::GetCurrNamespace() const
    {
        return GetVM().GetPublicNamespace();
    }

    Pickable<XML> XML::DeepCopy(XML* parent)
    {
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());
        return itr.MakeInstance(itr, Text, parent);
    }

    void XML::GetChildren(Instances::fl::XMLList& /*list*/, const Multiname& /*prop_name*/)
    {
        ; // Do nothing.
    }

    CheckResult XML::GetChildIndex(UPInt&) const
    {
        return false;
    }

    void XML::GetDescendants(XMLList& list, const Multiname& prop_name)
    {
        // Do nothing.
        SF_UNUSED2(prop_name, list); 
    }

    // Can throw exceptions.
    CheckResult XML::InsertChildAt(UPInt pos, const Value& child)
    {
        SF_UNUSED2(pos, child);
        return false;
    }

    CheckResult XML::InsertChildAfter(const Value& child1, const Value& child2)
    {
        SF_UNUSED2(child1, child2);
        return false;
    }

    CheckResult XML::InsertChildBefore(const Value& child1, const Value& child2)
    {
        SF_UNUSED2(child1, child2);
        return false;
    }

    void XML::DeleteChildren(XML*)
    {
        // Do nothing
    }

    void XML::DeleteByIndex(UPInt)
    {
        // Do nothing
    }

    const XML::NamespaceH* XML::GetInScopeNamespaces() const
    {
        return NULL;
    }

    void XML::AddInScopeNamespace(const Namespace& ns)
    {
        SF_UNUSED1(ns);
    }

    void XML::AppendChild(const SPtr<XML>& child)
    {
        SF_UNUSED1(child);
    }

    CheckResult XML::AppendChild(const Value& child)
    {
        SF_UNUSED1(child);
        return true;
    }

    void XML::GetAttributes(Instances::fl::XMLList& /*list*/)
    {
        ; // Do nothing.
    }

    void XML::GetChildren(Instances::fl::XMLList& /*list*/, Kind /*k*/, const ASString* /*name*/)
    {
        ; // Do nothing.
    }

    SPtr<Instances::fl::QName> XML::GetQName()
    {
        return NULL;
    }

    void XML::Normalize()
    {
        ; // Do nothing.
    }

    Namespace* XML::FindNamespaceByPrefix(const ASString& prefix, XML* lp) const
    {
        if (GetParent() != lp)
            return GetParent()->FindNamespaceByPrefix(prefix, lp);

        if (prefix == "xml")
            return &GetVM().GetXMLNamespace();

        return NULL;
    }

    Namespace* XML::FindNamespaceByURI(const ASString& uri, XML* lp) const
    {
        if (GetParent() != lp)
            return GetParent()->FindNamespaceByURI(uri, lp);

        if (uri == NS_XML)
            return &GetVM().GetXMLNamespace();

        return NULL;
    }

    void XML::ResolveNamespaces(HashSetDH<ASString>& nr, XML* lp) const
    {
        SF_UNUSED2(nr, lp);
    }

    XML* XML::RemoveNamespace(const Value& ns)
    {
        SF_UNUSED(ns);
        return this;
    }

    CheckResult XML::Replace(const Multiname& /*prop_name*/, const Value& /*value*/)
    {
        return true; // Do nothing.
    }

    CheckResult XML::AS3Replace(const Multiname& prop_name, const Value& value)
    {
        SF_UNUSED2(prop_name, value);
        return true; // Do nothing.
    }

    void XML::SetChildren(const Value& /*value*/)
    {
        ; // Do nothing.
    }

    void XML::ToString(StringBuffer& buf, SInt32 ident)
    {
        SF_UNUSED1(ident);
        buf.AppendString(Text.ToCStr(), Text.GetSize());
    }

    void XML::ToXMLString(StringBuffer& buf, SInt32 ident, const NamespaceArray* ancestorNamespaces, const NamespaceArray* usedNotDeclared)
    {
        SF_UNUSED3(ident, ancestorNamespaces, usedNotDeclared);
        buf.AppendString(Text.ToCStr(), Text.GetSize());
    }

    static const char* offsets[11] = {
        "",
        " ",
        "  ",
        "   ",
        "    ",
        "     ",
        "      ",
        "       ",
        "        ",
        "         ",
        "          ",
    };

    void XML::AppendIdent(StringBuffer& buf, SInt32 ident)
    {
        while (ident)
        {
            const SInt32 n = Alg::Min<SInt32>(ident, 10);
            buf.AppendString(offsets[n], n);
            ident -= n;
        }
    }
    void XML::EscapeElementValue(StringBuffer& buf, const ASString& v)
    {
        const char* psrc = v.ToCStr();
        const char* pend = psrc + v.GetSize();

        while((psrc < pend))
        {
            UInt32 ch = UTF8Util::DecodeNextChar_Advance0(&psrc);
            switch (ch)
            {
            case '<':
                buf.AppendString("&lt;", 4);
                break;
            case '>':
                buf.AppendString("&gt;", 4);
                break;
            case '\"':
                buf.AppendString("&quot;", 6);
                break;
            case '\'':
                buf.AppendString("&apos;", 6);
                break;
            case '&':
                buf.AppendString("&amp;", 5);
                break;
            default:
                buf.AppendChar(ch);
            }
        }
    }

    void XML::EscapeAttributeValue(StringBuffer& buf, const ASString& v)
    {
        const char* psrc = v.ToCStr();
        const char* pend = psrc + v.GetSize();

        while((psrc < pend))
        {
            UInt32 ch = UTF8Util::DecodeNextChar_Advance0(&psrc);
            switch (ch)
            {
            case 0x9:
                buf.AppendString("&#x9;", 5);
                break;
            case 0xA:
                buf.AppendString("&#xA;", 5);
                break;
            case 0xD:
                buf.AppendString("&#xD;", 5);
                break;
            case '<':
                buf.AppendString("&lt;", 4);
                break;
            /* DO NOT delete this code.
            // For some reason we shouldn't escape '>'.
            case '>':
                buf.AppendString("&gt;", 4);
                break;
            */
            case '\"':
                buf.AppendString("&quot;", 6);
                break;
            case '\'':
                buf.AppendString("&apos;", 6);
                break;
            case '&':
                buf.AppendString("&amp;", 5);
                break;
            default:
                buf.AppendChar(ch);
            }
        }
    }

    void XML::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        StringManager& sm = GetStringManager();

        if (ind.IsValid())
        {
            name = sm.CreateConstString("0");
            return;
        }

        name = sm.GetBuiltin(AS3Builtin_null);
    }

    GlobalSlotIndex XML::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
        if (ind.IsValid())
            return GlobalSlotIndex(0);

        return GlobalSlotIndex(1);
    }

    void XML::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
    {
        if (ind.IsValid())
        {
            value = this;
            return;
        }

        value.SetUndefined();
    }

    CheckResult XML::SetProperty(const Multiname& prop_name, const Value& value)
    {
        SF_UNUSED(value);
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            GetVM().ThrowTypeError(VM::Error(VM::eXMLAssignmentToIndexedXMLNotAllowed, GetVM()));
            return false;
        }
        return false;
    }


    CheckResult XML::GetProperty(const Multiname& prop_name, XMLList& list)
    {
#if 1
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind == 0)
            {
                list.Apppend(*this);
                return true;
            }
        }
#else
        SF_UNUSED2(prop_name, list);
#endif

        return false;
    }

    CheckResult XML::GetProperty(const Multiname& prop_name, Value& value)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind == 0)
            {
                value = this;
                return true;
            }
            else
                value.SetUndefined();
        }

        return false;
    }

    CheckResult XML::DeleteProperty(const Multiname& prop_name)
    {
        return Instances::fl::Object::DeleteProperty(prop_name);
    }

    void XML::SetNamespace(Namespace& ns)
    {
        SF_UNUSED(ns);
    }

    void XML::SetNamespace2(Namespace& ns)
    {
        SF_UNUSED(ns);
    }
#endif

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 XML::tito[XML::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 11, 13, 14, 16, 17, 18, 19, 21, 22, 23, 24, 25, 26, 27, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44, 46, 49, 51, 53, 55, 57, 58, 59, 
    // };
    const TypeInfo* XML::tit[60] = {
        &AS3::fl::XMLListTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::XMLTI, NULL, 
        &AS3::fl::XMLTI, NULL, 
        &AS3::fl::XMLListTI, NULL, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::XMLListTI, NULL, 
        &AS3::fl::int_TI, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::BooleanTI, NULL, 
        &AS3::fl::XMLTI, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::ArrayTI, 
        NULL, NULL, NULL, 
        NULL, NULL, NULL, 
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, 
        &AS3::fl::QNameTI, 
        NULL, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::XMLTI, 
        NULL, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::XMLTI, NULL, 
        &AS3::fl::XMLTI, NULL, 
        &AS3::fl::XMLTI, NULL, NULL, 
        &AS3::fl::XMLTI, NULL, 
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL, 
        &AS3::fl::XMLListTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::XMLTI, 
    };
    const ThunkInfo XML::ti[XML::ThunkInfoNum] = {
        {TFunc_Instances_XML_prototypeGet::Func, &XML::tit[0], "prototype", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_prototypeSet::Func, &XML::tit[1], "prototype", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3toString::Func, &XML::tit[3], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3hasOwnProperty::Func, &XML::tit[4], "hasOwnProperty", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3propertyIsEnumerable::Func, &XML::tit[6], "propertyIsEnumerable", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_XML_AS3addNamespace::Func, &XML::tit[7], "addNamespace", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3appendChild::Func, &XML::tit[9], "appendChild", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3attribute::Func, &XML::tit[11], "attribute", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3attributes::Func, &XML::tit[13], "attributes", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3child::Func, &XML::tit[14], "child", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3childIndex::Func, &XML::tit[16], "childIndex", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3children::Func, &XML::tit[17], "children", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3comments::Func, &XML::tit[18], "comments", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3contains::Func, &XML::tit[19], "contains", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3copy::Func, &XML::tit[21], "copy", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3descendants::Func, &XML::tit[22], "descendants", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_XML_AS3elements::Func, &XML::tit[23], "elements", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_XML_AS3hasComplexContent::Func, &XML::tit[24], "hasComplexContent", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3hasSimpleContent::Func, &XML::tit[25], "hasSimpleContent", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3inScopeNamespaces::Func, &XML::tit[26], "inScopeNamespaces", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3insertChildAfter::Func, &XML::tit[27], "insertChildAfter", NS_AS3, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_XML_AS3insertChildBefore::Func, &XML::tit[30], "insertChildBefore", NS_AS3, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_XML_AS3length::Func, &XML::tit[33], "length", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3localName::Func, &XML::tit[34], "localName", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3name::Func, &XML::tit[35], "name", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3namespace_::Func, &XML::tit[36], "namespace", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_XML_AS3namespaceDeclarations::Func, &XML::tit[37], "namespaceDeclarations", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3nodeKind::Func, &XML::tit[38], "nodeKind", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3normalize::Func, &XML::tit[39], "normalize", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3parent::Func, &XML::tit[40], "parent", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3processingInstructions::Func, &XML::tit[41], "processingInstructions", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_XML_AS3prependChild::Func, &XML::tit[42], "prependChild", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3removeNamespace::Func, &XML::tit[44], "removeNamespace", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3replace::Func, &XML::tit[46], "replace", NS_AS3, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_XML_AS3setChildren::Func, &XML::tit[49], "setChildren", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3setLocalName::Func, &XML::tit[51], "setLocalName", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3setName::Func, &XML::tit[53], "setName", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3setNamespace::Func, &XML::tit[55], "setNamespace", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_XML_AS3text::Func, &XML::tit[57], "text", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3toXMLString::Func, &XML::tit[58], "toXMLString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_XML_AS3valueOf::Func, &XML::tit[59], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    XML::XML(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::XML::XML()"
        SetTraitsType(Traits_XML);
//##protect##"InstanceTraits::XML::XML()"

    }

    void XML::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<XML&>(t));
    }

//##protect##"instance_traits$methods"
#ifdef GFX_ENABLE_XML
    void XML::toStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED4(cti, _this, argc, argv);

        if (_this.IsObject() && _this.GetObject() == &vm.GetXMLSupport().GetITraitsXML()->GetClass().GetPrototype())
            // This method is called on a prototype object.
            result = vm.GetStringManager().GetBuiltin(AS3Builtin_empty_);
        else
        {
            if (!IsXMLObject(_this))
                return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm
                    SF_DEBUG_ARG("XML::toStringProto")
                    ));

            Instances::fl::XML* obj = static_cast<Instances::fl::XML*>(_this.GetObject());
            ASString str = vm.GetStringManager().GetBuiltin(AS3Builtin_empty_);

            obj->AS3toString(str);
            // We do not check for exception here on purpose. There should be not much harm.
            result = str;
        }
    }

    void XML::PropertyIsEnumerableProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED4(cti, _this, argc, argv);

        if (_this.IsObject() && _this.GetObject() == &vm.GetXMLSupport().GetITraitsXML()->GetClass().GetPrototype())
            // This method is called on a prototype object.
            static_cast<Instances::fl::Object*>(_this.GetObject())->propertyIsEnumerableProto(cti, vm, _this, result, argc, argv);
        else
        {
            if (!IsXMLObject(_this))
                return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm
                SF_DEBUG_ARG("XML::PropertyIsEnumerableProto")
                ));

            Instances::fl::XML* obj = static_cast<Instances::fl::XML*>(_this.GetObject());
            obj->AS3propertyIsEnumerable(result, argc, argv);
        }
    }

    void XML::HasOwnPropertyProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED4(cti, _this, argc, argv);

        if (_this.IsObject() && _this.GetObject() == &vm.GetXMLSupport().GetITraitsXML()->GetClass().GetPrototype())
            // This method is called on a prototype object.
            static_cast<Instances::fl::Object*>(_this.GetObject())->hasOwnPropertyProto(cti, vm, _this, result, argc, argv);
        else
        {
            if (!IsXMLObject(_this))
                return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm
                SF_DEBUG_ARG("XML::HasOwnPropertyProto")
                ));

            Instances::fl::XML* obj = static_cast<Instances::fl::XML*>(_this.GetObject());
            // PPS: A single argument (string type) is expected
            if (argc < 1 || !argv[0].IsString())
                return vm.ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, vm
                SF_DEBUG_ARG("XML::HasOwnPropertyProto")
                ));

            result.SetBool(obj->HasOwnProperty(argv[0].AsString()));
        }
    }
#endif
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    XML::XML(ClassTraits::Traits& t)
    : Class(t)
    , ignoreComments(true)
    , ignoreProcessingInstructions(true)
    , ignoreWhitespace(true)
    , prettyPrinting(true)
    , prettyIndent(2)
    {
//##protect##"class_::XML::XML()"
#ifdef GFX_ENABLE_XML
        SetDefaultValues();
#endif
//##protect##"class_::XML::XML()"
    }
    void XML::AS3settings(SPtr<Instances::fl::Object>& result)
    {
//##protect##"class_::XML::AS3settings()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();
        Pickable<Instances::fl::Object> obj = vm.MakeObject();

        result = obj;
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreComments"), Value(ignoreComments));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreProcessingInstructions"), Value(ignoreProcessingInstructions));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreWhitespace"), Value(ignoreWhitespace));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("prettyPrinting"), Value(prettyPrinting));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("prettyIndent"), Value(prettyIndent));
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3settings()");
#endif
//##protect##"class_::XML::AS3settings()"
    }
    void XML::settings(SPtr<Instances::fl::Object>& result)
    {
//##protect##"class_::XML::settings()"
#ifdef GFX_ENABLE_XML
        AS3settings(result);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::settings()");
#endif
//##protect##"class_::XML::settings()"
    }
    void XML::AS3setSettings(const Value& result, const Value& o)
    {
//##protect##"class_::XML::AS3setSettings()"
        SF_UNUSED2(result, o);

#ifdef GFX_ENABLE_XML
        if (o.IsNullOrUndefined())
            SetDefaultValues();
        else if (o.IsObject())
        {
            AS3::Object& obj = *o.GetObject();
            VM& vm = GetVM();
            StringManager& sm = vm.GetStringManager();
            const Value* v;

            v = obj.FindDynamicSlot(sm.CreateConstString("ignoreComments"));
            if (v && v->IsBool())
                ignoreComments = *v;

            v = obj.FindDynamicSlot(sm.CreateConstString("ignoreProcessingInstructions"));
            if (v && v->IsBool())
                ignoreProcessingInstructions = *v;

            v = obj.FindDynamicSlot(sm.CreateConstString("ignoreWhitespace"));
            if (v && v->IsBool())
                ignoreWhitespace = *v;

            v = obj.FindDynamicSlot(sm.CreateConstString("prettyPrinting"));
            if (v && v->IsBool())
                prettyPrinting = *v;

            v = obj.FindDynamicSlot(sm.CreateConstString("prettyIndent"));
            if (v && v->IsInt())
                prettyIndent = *v;
        }
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3setSettings()");
#endif
//##protect##"class_::XML::AS3setSettings()"
    }
    void XML::setSettings(const Value& result, const Value& o)
    {
//##protect##"class_::XML::setSettings()"
#ifdef GFX_ENABLE_XML
        AS3setSettings(result, o);
#else
        SF_UNUSED2(result, o);
        WARN_NOT_IMPLEMENTED("XML::setSettings()");
#endif
//##protect##"class_::XML::setSettings()"
    }
    void XML::AS3defaultSettings(SPtr<Instances::fl::Object>& result)
    {
//##protect##"class_::XML::AS3defaultSettings()"
#ifdef GFX_ENABLE_XML
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();
        Pickable<Instances::fl::Object> obj = vm.MakeObject();

        result = obj;
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreComments"), Value(true));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreProcessingInstructions"), Value(true));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("ignoreWhitespace"), Value(true));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("prettyPrinting"), Value(true));
        obj->AddDynamicSlotValuePair(sm.CreateConstString("prettyIndent"), Value(SInt32(2)));
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::AS3defaultSettings()");
#endif
//##protect##"class_::XML::AS3defaultSettings()"
    }
    void XML::defaultSettings(SPtr<Instances::fl::Object>& result)
    {
//##protect##"class_::XML::defaultSettings()"
#ifdef GFX_ENABLE_XML
        AS3defaultSettings(result);
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("XML::defaultSettings()");
#endif
//##protect##"class_::XML::defaultSettings()"
    }
//##protect##"class_$methods"
#ifdef GFX_ENABLE_XML
    void XML::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
    {
        VM& vm = GetVM();
        InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(GetInstanceTraits());

        if (argc > 0)
        {
            const Value& v = argv[0];
            const Traits& tr = vm.GetValueTraits(v);
            const BuiltinTraitsType tt = tr.GetTraitsType();

            if (tr.IsClassTraits())
            {
                // Exception.
                // Pop arguments from stack.
                if (!extCall)
                    vm.OpStack.PopBack(argc);

                return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm
                    SF_DEBUG_ARG("XML::Construct")
                    ));
            }

            if (v.IsNullOrUndefined())
            {
                _this = itr.MakeInstanceText(itr, vm.GetStringManager().CreateEmptyString());
                return;
            }

            if (tr.IsInstanceTraits())
            {
                switch (tt)
                {
                case Traits_Boolean:
                case Traits_SInt:
                case Traits_UInt:
                case Traits_Number:
                    {
                        StringManager& sm = vm.GetStringManager();
                        ASString str = sm.CreateEmptyString();
                        // Convert primitive type to String.
                        // We shouldn't have any problem here.
                        v.Convert2String(str).DoNotCheck();
                        _this = itr.MakeInstanceText(itr, str);
                    }
                    return;
                case Traits_String:
                    {
                        ASString str = v.AsString();

                        if (*ASUtils::SkipWhiteSpace(str.ToCStr(), str.GetSize()) != '<')
                        {
                            // Treat this string as a regular string node.
                            if (ignoreWhitespace)
                                str = str.TruncateWhitespace();

                            InstanceTraits::fl::XML& itrxml = static_cast<InstanceTraits::fl::XML&>(GetClassTraits().GetInstanceTraits());
                            Pickable<Instances::fl::XML> el = itrxml.MakeInstanceText(itrxml, str, NULL);

                            _this = el;
                        }
                        else
                        {
                            // Parse it.
                            XMLParser parser(itr);
                            UPInt pos = 0;

                            if (parser.Parse(StringDataPtr(str.ToCStr(), str.GetSize()), pos))
                            {
                                _this = parser.GetCurrNode();
                            }
                            else
                            {
                                InstanceTraits::fl::XML& itrxml = static_cast<InstanceTraits::fl::XML&>(GetClassTraits().GetInstanceTraits());
                                Pickable<Instances::fl::XML> el = itrxml.MakeInstanceText(itrxml, vm.GetStringManager().CreateEmptyString(), NULL);
                                _this = el;
                            }
                        }
                    }
                    return;
                case Traits_XML:
                    {
                        // We create an object with deep copy
                        Instances::fl::XML& xml = static_cast<Instances::fl::XML&>(*v.GetObject());
                        Pickable<Instances::fl::XML> copy = xml.DeepCopy(NULL);
                        _this = copy;

                        return;
                    }
                    break;
                case Traits_XMLList:
                    {
                        const Instances::fl::XMLList& list = static_cast<const Instances::fl::XMLList&>(*v.GetObject());

                        if (list.List.GetSize() == 1)
                            _this = list.List[0];
                        else
                        {
                            // Exception.
                            // Pop arguments from stack.
                            if (!extCall)
                                vm.OpStack.PopBack(argc);

                            vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm
                                SF_DEBUG_ARG("XML::Construct")
                                ));
                        }

                        return;
                    }
                    break;
                default:
                    break;
                }
            }
        }

        _this = itr.MakeInstanceText(itr, vm.GetStringManager().CreateEmptyString());
    }

    void XML::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        VM& vm = GetVM();
        if (argc > 0)
        {
            const Value& v = argv[0];
            const Traits& tr = vm.GetValueTraits(v);
            const BuiltinTraitsType tt = tr.GetTraitsType();
            if (tr.IsInstanceTraits())
            {
                switch (tt)
                {
                case Traits_XML:
                    {
                        // We shouldn't create new object in this case.
                        result = v;

                        return;
                    }
                    break;
                default:
                    break;
                }
            }
        }

        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    // Created manually.
    const TypeInfo* XML::tit[4] = {
        &AS3::fl::StringTI,
        &AS3::fl::StringTI,
        &AS3::fl::StringTI, &AS3::fl::StringTI,
    };

    const ThunkInfo XML::f[3] = {
        {&InstanceTraits::fl::XML::toStringProto, &XML::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0},
        {&InstanceTraits::fl::XML::PropertyIsEnumerableProto, &XML::tit[1], "propertyIsEnumerable", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
        {&InstanceTraits::fl::XML::HasOwnPropertyProto, &XML::tit[2], "hasOwnProperty", NULL, Abc::NS_Public, CT_Method, 0, 1},
    };

    void XML::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        //InitPrototypeFromVTable(obj);
        InitPrototypeFromVTableCheckType(obj);

        for (unsigned i = 0; i < NUMBEROF(f); ++i)
            AddDynamicFunc(obj, f[i]);

        AddConstructor(obj);
    }

    void XML::SetDefaultValues()
    {
        ignoreComments = true;
        ignoreProcessingInstructions = true;
        ignoreWhitespace = true;
        prettyPrinting = true;
        prettyIndent = 2;
    }
#endif
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl::XML, Classes::fl::XML::mid_AS3settings, SPtr<Instances::fl::Object> > TFunc_Classes_XML_AS3settings;
typedef ThunkFunc0<Classes::fl::XML, Classes::fl::XML::mid_settings, SPtr<Instances::fl::Object> > TFunc_Classes_XML_settings;
typedef ThunkFunc1<Classes::fl::XML, Classes::fl::XML::mid_AS3setSettings, const Value, const Value&> TFunc_Classes_XML_AS3setSettings;
typedef ThunkFunc1<Classes::fl::XML, Classes::fl::XML::mid_setSettings, const Value, const Value&> TFunc_Classes_XML_setSettings;
typedef ThunkFunc0<Classes::fl::XML, Classes::fl::XML::mid_AS3defaultSettings, SPtr<Instances::fl::Object> > TFunc_Classes_XML_AS3defaultSettings;
typedef ThunkFunc0<Classes::fl::XML, Classes::fl::XML::mid_defaultSettings, SPtr<Instances::fl::Object> > TFunc_Classes_XML_defaultSettings;

template <> const TFunc_Classes_XML_AS3settings::TMethod TFunc_Classes_XML_AS3settings::Method = &Classes::fl::XML::AS3settings;
template <> const TFunc_Classes_XML_settings::TMethod TFunc_Classes_XML_settings::Method = &Classes::fl::XML::settings;
template <> const TFunc_Classes_XML_AS3setSettings::TMethod TFunc_Classes_XML_AS3setSettings::Method = &Classes::fl::XML::AS3setSettings;
template <> const TFunc_Classes_XML_setSettings::TMethod TFunc_Classes_XML_setSettings::Method = &Classes::fl::XML::setSettings;
template <> const TFunc_Classes_XML_AS3defaultSettings::TMethod TFunc_Classes_XML_AS3defaultSettings::Method = &Classes::fl::XML::AS3defaultSettings;
template <> const TFunc_Classes_XML_defaultSettings::TMethod TFunc_Classes_XML_defaultSettings::Method = &Classes::fl::XML::defaultSettings;

namespace ClassTraits { namespace fl
{
    const MemberInfo XML::mi[XML::MemberInfoNum] = {
        {"ignoreComments", NULL, OFFSETOF(Classes::fl::XML, ignoreComments), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"ignoreProcessingInstructions", NULL, OFFSETOF(Classes::fl::XML, ignoreProcessingInstructions), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"ignoreWhitespace", NULL, OFFSETOF(Classes::fl::XML, ignoreWhitespace), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"prettyPrinting", NULL, OFFSETOF(Classes::fl::XML, prettyPrinting), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"prettyIndent", NULL, OFFSETOF(Classes::fl::XML, prettyIndent), Abc::NS_Public, SlotInfo::BT_Int, 0},
    };

    // const UInt16 XML::tito[XML::ThunkInfoNum] = {
    //    0, 1, 2, 4, 6, 7, 
    // };
    const TypeInfo* XML::tit[8] = {
        &AS3::fl::ObjectTI, 
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::ObjectTI, 
        &AS3::fl::ObjectTI, 
    };
    const ThunkInfo XML::ti[XML::ThunkInfoNum] = {
        {TFunc_Classes_XML_AS3settings::Func, &XML::tit[0], "settings", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_XML_settings::Func, &XML::tit[1], "settings", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_XML_AS3setSettings::Func, &XML::tit[2], "setSettings", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_XML_setSettings::Func, &XML::tit[4], "setSettings", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_XML_AS3defaultSettings::Func, &XML::tit[6], "defaultSettings", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_XML_defaultSettings::Func, &XML::tit[7], "defaultSettings", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    XML::XML(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::XML::XML()"
#ifdef GFX_ENABLE_XML
        SetTraitsType(Traits_XML);
#endif
//##protect##"ClassTraits::XML::XML()"

    }

    Pickable<Traits> XML::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) XML(vm, AS3::fl::XMLCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::XMLCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo XMLTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject | TypeInfo::Final,
        sizeof(ClassTraits::fl::XML::InstanceType),
        ClassTraits::fl::XML::ThunkInfoNum,
        ClassTraits::fl::XML::MemberInfoNum,
        InstanceTraits::fl::XML::ThunkInfoNum,
        0,
        "XML", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo XMLCI = {
        &XMLTI,
        ClassTraits::fl::XML::MakeClassTraits,
        ClassTraits::fl::XML::ti,
        ClassTraits::fl::XML::mi,
        InstanceTraits::fl::XML::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

