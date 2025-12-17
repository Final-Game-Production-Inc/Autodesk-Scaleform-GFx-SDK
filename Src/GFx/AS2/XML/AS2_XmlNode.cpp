/**************************************************************************

Filename    :   AS2_XmlNode.cpp
Content     :   XMLNode reference class for ActionScript 2.0
Created     :   11/30/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

//
// Known issues and notes:
//
// - Custom properties cannot be set and retrieved. The properties will
//   be destroyed once the ref count goes to 0.
// - After appending a child that already has parent, the child's 
//   namespace is not resolved in Flash. GFx fixes this obvious 'bug'.
//

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/XML/XML_Object.h"
#include "GFx/AS2/XML/AS2_XMLShadowRef.h"
#include "GFx/AS2/XML/AS2_XmlNode.h"
#include "GFx/AS2/AS2_ArrayObject.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//
// Temporary descriptor of XMLNode properties
//
struct GASDocument
{
    const char *                        pName;
    XmlNodeObject::StandardMember    Id;
};

//
// Mapping between const char* and standard member enum
//
GASDocument XmlNodeObject_MemberTable[] =
{
    { "attributes",         XmlNodeObject::M_attributes },
    { "childNodes",         XmlNodeObject::M_childNodes },
    { "firstChild",         XmlNodeObject::M_firstChild },
    { "lastChild",          XmlNodeObject::M_lastChild },
    { "localName",          XmlNodeObject::M_localName },
    { "namespaceURI",       XmlNodeObject::M_namespaceURI },
    { "nextSibling",        XmlNodeObject::M_nextSibling },
    { "nodeName",           XmlNodeObject::M_nodeName },
    { "nodeType",           XmlNodeObject::M_nodeType },
    { "nodeValue",          XmlNodeObject::M_nodeValue },
    { "parentNode",         XmlNodeObject::M_parentNode },
    { "prefix",             XmlNodeObject::M_prefix },
    { "previousSibling",    XmlNodeObject::M_previousSibling },

    // Done.
    { 0,                    XmlNodeObject::M_InvalidMember }
};

//
// Looks up an XMLNode standard member and returns M_InvalidMember if it is not found.
//
XmlNodeObject::StandardMember  XmlNodeObject::GetStandardMemberConstant(Environment* penv, 
                                                                              const ASString& memberName) const
{
    SByte   memberConstant = M_InvalidMember; 
    if (memberName.IsStandardMember())
    {        
        XmlNodeProto* xmlNodeProto = 
            static_cast<XmlNodeProto*>(penv->GetPrototype(ASBuiltin_XMLNode));
        SF_ASSERT(xmlNodeProto);
        xmlNodeProto->XMLNodeMemberMap.GetCaseCheck(memberName, &memberConstant, true);
    }        
    return (StandardMember) memberConstant;
}

//
// Initialize the XMLNode members in the global contexts
//
// This will be used in GetMember/SetMember for fast member comparison
//
void XmlNodeObject::InitializeStandardMembers(GlobalContext& gc, ASStringHash<SByte>& hash)
{
    SF_COMPILER_ASSERT( (sizeof(XmlNodeObject_MemberTable)/sizeof(XmlNodeObject_MemberTable[0]))
        == M_XMLNodeMemberCount + 1);

    // Add all standard members.
    GASDocument* pentry;
    ASStringManager* pstrManager = gc.GetStringManager()->GetStringManager();

    hash.SetCapacity(M_XMLNodeMemberCount);  

    for (pentry = XmlNodeObject_MemberTable; pentry->pName; pentry++)
    {
        ASString name(pstrManager->CreateConstString(pentry->pName, strlen(pentry->pName),
            ASString::Flag_StandardMember));
        hash.Add(name, (SByte)pentry->Id);
    }
}


// 
// Setup the attributes of an element node. This method offloads any
// attributes defined in the C++ side to the AS side for correct behavior.
//
static void SetupAttributes(Environment* penv, XML::ElementNode* preal)
{
    // Special logic to handle attributes
    // Since attributes can hold object references as values, the functionality 
    // provided by the C++ DOM is inadaquete and not compatible. Therefore we will
    // offload all attributes to the AS Object.

    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(preal->pShadow);
    ASStringContext* psc = penv->GetSC();
    pshadow->pAttributes = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
    if (preal->HasAttributes())
    {
        // this code will execute once for unprocessed nodes
        for (XML::Attribute* attr = preal->FirstAttribute; attr != NULL; attr = attr->Next)
        {
            pshadow->pAttributes->SetMember(penv, psc->CreateString(attr->Name.ToCStr()), 
                psc->CreateString(attr->Value.ToCStr()));
        }
        preal->ClearAttributes();
    }
}


//
// Setup the given AS xml node object as a shadow for a real node.
//
static void SetupShadow(Environment* penv, XML::Node* preal, XmlNodeObject* asobj)
{
    SF_ASSERT(preal);
    if (!preal->pShadow)
    {
        preal->pShadow = SF_HEAP_NEW(preal->MemoryManager->GetHeap()) XMLShadowRef();
        if (preal->Type == XML::ElementNodeType)
        {
            XML::ElementNode* pnode = static_cast<XML::ElementNode*>(preal);
            SetupAttributes(penv, pnode);
        }
    }
    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(preal->pShadow);
    pshadow->pASNode = asobj;
    asobj->pRealNode = preal;
}


//
// Create an actionscript shadow for a real node (C++).
//
static Ptr<XmlNodeObject> CreateShadow(Environment* penv, XML::Node* preal, XML::RootNode* proot)
{
    SF_ASSERT(preal);
    Ptr<XmlNodeObject> asobj = *SF_HEAP_NEW(penv->GetHeap()) XmlNodeObject(penv);
    SetupShadow(penv, preal, asobj);
    // Setup root node
    if (proot)
        asobj->pRootNode = proot;
    else
        asobj->pRootNode = *preal->MemoryManager->CreateRootNode(preal);
    return asobj;
}


//
// Fill the Object with id-name -> XMLNode mappings
//
void Xml_CreateIDMap(Environment* penv, XML::ElementNode* elemNode, XML::RootNode* proot, Object* pobj) 
{
    XML::DOMStringCompareWrapper idstr("id", 2);
    for (XML::Node* child = elemNode->FirstChild; child != NULL; child = child->NextSibling)
    {
        if (child->Type == XML::ElementNodeType)
        {
            Xml_CreateIDMap(penv, (XML::ElementNode*)child, proot, pobj);

            // Find attribute with id name
            XML::ElementNode* pnode = static_cast<XML::ElementNode*>(child);
            for (XML::Attribute* attr = pnode->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                if (attr->Name == idstr)
                {
                    // We store a copy of the attr value because SetupShadow will clear the
                    // attributes and move them to the shadow.
                    String v(attr->Value.ToCStr(), attr->Value.GetSize());

                    Ptr<XmlNodeObject> pchildobj = NULL;
                    // pnode does not have a shadow; create one
                    if (NULL == pnode->pShadow)
                        pchildobj = CreateShadow(penv, pnode, proot);
                    else
                    {
                        // pnode does not have an AS counterpart
                        XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->pShadow);
                        pchildobj = pshadow->pASNode;
                        if (NULL == pchildobj.GetPtr())
                        {
                            pchildobj = *SF_HEAP_NEW(penv->GetHeap()) XmlNodeObject(penv);
                            SetupShadow(penv, pnode, pchildobj);
                        }
                    }

                    pobj->SetMember(penv, penv->CreateString(v), 
                        Value(pchildobj));

                    // Break on first attribute matched
                    break;
                }
            }
        }
    }
}


//
// Resolve the namespace of the given element node. This method first looks
// at the attributes of the node itself to determine a namespace declaration.
// If a matching namespace is not found, then it traverses its parent chain to 
// find one.
//
static void ResolveNamespace(Environment* penv, XML::ElementNode* elemNode, XML::RootNode* proot)
{
    ASStringContext* psc = penv->GetSC();
    // Create the property query
    ASString pfxquery = psc->CreateString("xmlns", 5);
    if (elemNode->Prefix.GetSize() > 0)
    {
        pfxquery += ":";
        pfxquery += elemNode->Prefix.ToCStr();
    }
    Value qval(Value::UNDEFINED);

    // -- Reset the namespace
    SF_ASSERT(elemNode);
    elemNode->Namespace = elemNode->MemoryManager->EmptyString();

    // -- Look for namespace declaration in self first
    SF_ASSERT(elemNode->pShadow);
    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(elemNode->pShadow);
    SF_ASSERT(pshadow);
    Object* pattribs = pshadow->pAttributes;
    SF_ASSERT(pattribs);
    pattribs->GetMember(penv, pfxquery, &qval);
    if (!qval.IsUndefined())
    {
        // Found ns declaration in self; bail
        ASString ns = qval.ToString(penv);
        elemNode->Namespace = elemNode->MemoryManager->CreateString(ns.ToCStr(), 
            ns.GetSize());
        return;
    }

    // -- Traverse up parent chain and look for a namespace
    XML::ElementNode* parent = static_cast<XML::ElementNode*>(elemNode->Parent);
    while (parent)
    {
        // create a shadow if one does not exist
        if (NULL == parent->pShadow)
            CreateShadow(penv, parent, proot);
        pshadow = static_cast<XMLShadowRef*>(parent->pShadow);
        SF_ASSERT(pshadow);
        Object* pattribsObjects = pshadow->pAttributes;
        SF_ASSERT(pattribsObjects);
        // Lookup prefix in properties
        pattribsObjects->GetMember(penv, pfxquery, &qval);
        if (!qval.IsUndefined())
        {
            ASString ns = qval.ToString(penv);
            elemNode->Namespace = elemNode->MemoryManager->CreateString(ns.ToCStr(), 
                ns.GetSize());
            break;
        }
        parent = static_cast<XML::ElementNode*>(parent->Parent);
    }

}


// 
// Constructor
//
XmlNodeObject::XmlNodeObject(Environment* penv) 
: Object(penv), pRealNode(NULL)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_XMLNode));    
}


// 
// Destructor
//
XmlNodeObject::~XmlNodeObject()
{
    // Remove itself as the realnode's (C++) shadow reference
    if (pRealNode && pRealNode->pShadow)
    {
        XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pRealNode->pShadow);
        pshadow->pASNode = NULL;
    }
}


// 
// Overloaded SetMember function to intercept and process builtin properties
// Since the actual data is stored in the realnode, each property needs to be
// intercepted and processed individually.
//
bool XmlNodeObject::SetMember(Environment* penv, const ASString& name, 
                                 const Value& val, 
                                 const PropFlags& flags)
{
    Log* log = penv->GetLog();
    // Try to handle this property
    if (pRealNode)
    {         
        StandardMember member = GetStandardMemberConstant(penv, name);

        switch (member)
        {

        // ***** XMLNode.nodeName
        case M_nodeName:

            if (pRealNode)
            {
                // Flash Doc: If the XML object is an XML element (nodeType == 1), nodeName is 
                // the name of the tag that represents the node in the XML file. For example, 
                // TITLE is the nodeName of an HTML TITLE tag. If the XML object is a text node 
                // (nodeType == 3), nodeName is null.
                //
                if (pRealNode->Type == XML::ElementNodeType)
                {
                    XML::ElementNode* elemNode = static_cast<XML::ElementNode*>(pRealNode);
                    // -- Look for prefix declaration, if one exists in node name
                    XML::DOMString prefix( pRealNode->MemoryManager->EmptyString() );
                    ASString str = val.ToString(penv);
                    const char* data = str.ToCStr();
                    const char* colon = strchr(data, ':');
                    if (colon)
                    {
                        // Check prefix
                        prefix = pRealNode->MemoryManager->CreateString(data, colon-data);
                        // Set the localname
                        pRealNode->Value = pRealNode->MemoryManager->CreateString(
                            colon+1, SFstrlen(colon+1));
                    }
                    else
                    {
                        // If node already contains default namespace, it is preserved.
                        pRealNode->Value = pRealNode->MemoryManager->CreateString(str.ToCStr(), 
                            str.GetSize());
                    }
                    elemNode->Prefix = prefix;

                    ResolveNamespace(penv, elemNode, pRootNode);
                }
                else
                {
                    if (log != NULL)
                        log->LogMessageById(Log_ScriptWarning, "XMLNodeObject::SetMember"
                            " - cannot set nodeName of node type %d. Only type 1 allowed", 
                            pRealNode->Type);
                }
            }
            else
            {
                if (log != NULL)
                    log->LogMessageById(Log_ScriptWarning, "XMLNodeObject::SetMember"
                        " - cannot set nodeName of a malformed node");
            }
            return true;


        // ***** XMLNode.nodeValue
        case M_nodeValue:

            if (pRealNode && pRealNode->Type != XML::ElementNodeType)
            {
                // Flash Doc: If the XML object is a text node, the nodeType is 3, and the nodeValue 
                // is the text of the node. If the XML object is an XML element (nodeType is 1), 
                // nodeValue is null and read-only
                //
                ASString str = val.ToString(penv);
                pRealNode->Value = pRealNode->MemoryManager->CreateString(str.ToCStr(), 
                    str.GetSize());
            }
            else
            {
                if (log != NULL)
                    log->LogMessageById(Log_ScriptWarning, "XMLNodeObject::SetMember"
                        " - cannot set nodeValue of a malformed node");
            }
            return true;


        // ***** XMLNode.attributes
        case M_attributes:

            if (pRealNode)
            {
                // Flash Doc: The XML.attributes object contains one variable for each attribute 
                // of the XML instance. Because these variables are defined as part of the 
                // object, they are generally referred to as properties of the object. The value 
                // of each attribute is stored in the corresponding property as a string.
                //
                if (pRealNode->Type == XML::ElementNodeType)
                {
                    SF_ASSERT(pRealNode);
                    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pRealNode->pShadow);
                    SF_ASSERT(pshadow);
                    pshadow->pAttributes = val.ToObject(penv);
                }
                else
                {
                    if (log != NULL)
                        log->LogMessageById(Log_ScriptWarning, "XMLNodeObject::SetMember"
                            " - cannot set attributes of node type %d. Only type 1 allowed", 
                            pRealNode->Type);
                }
            }
            else
            {
                if (log != NULL)
                    log->LogMessageById(Log_ScriptWarning, "XMLNodeObject::SetMember"
                        " - cannot set attributes of a malformed node");
            }
            return true;

        default:
            {
                // Pass along to Object
            }
        }

    }
    return Object::SetMember(penv, name, val, flags);
}


// 
// Overloaded GetMember function to intercept and process builtin properties
// Since the actual data is stored in the realnode, each property needs to be
// intercepted and processed individually.
//
bool XmlNodeObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    ASStringContext* psc = penv->GetSC();
    // Try to handle this property
    if (pRealNode)
    {
        StandardMember member = GetStandardMemberConstant(penv, name);

        switch (member)
        {

        // ***** XMLNode.firstChild
        case M_firstChild:
        
            if (pRealNode) 
            {
                if (pRealNode->Type == XML::ElementNodeType)
                {
                    // Flash Doc: This property is null if the node does not have 
                    // children. This property is undefined if the node is a text node. 
                    // This is a read-only property and cannot be used to manipulate 
                    // child nodes; use the appendChild(), insertBefore(), and 
                    // removeNode() methods to manipulate child nodes.
                    //
                    XML::ElementNode* pnode = static_cast<XML::ElementNode*>(pRealNode);
                    if (pnode->HasChildren())
                    {
                        XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->FirstChild->pShadow);
                        if (NULL == pshadow || NULL == pshadow->pASNode)
                            val->SetAsObject(CreateShadow(penv, pnode->FirstChild, pRootNode));
                        else
                            val->SetAsObject(static_cast<Object*>(pshadow->pASNode));
                    }
                    else
                        val->SetNull();
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.nextSibling
        case M_nextSibling:
        
            if (pRealNode)
            {
                // Flash Doc: This property is null if the node does not have 
                // a next sibling node. This property cannot be used to manipulate 
                // child nodes; use the appendChild(), insertBefore(), and 
                // removeNode() methods to manipulate child nodes.
                //
                XML::Node* pnode = static_cast<XML::Node*>(pRealNode);
                if (pnode->NextSibling)
                {
                    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->NextSibling->pShadow);
                    if (NULL == pshadow || NULL == pshadow->pASNode)
                        val->SetAsObject(CreateShadow(penv, pnode->NextSibling, pRootNode));
                    else
                        val->SetAsObject(static_cast<Object*>(pshadow->pASNode));
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.childNodes
        case M_childNodes:
        
            if (pRealNode && pRealNode->Type == XML::ElementNodeType)
            {
                // Flash Doc: Each element in the array is a reference to an XML object 
                // that represents a child node. This is a read-only property and cannot 
                // be used to manipulate child nodes. Use the appendChild(), insertBefore(), 
                // and removeNode() methods to manipulate child nodes. This property is 
                // undefined for text nodes (nodeType == 3).
                //
                // fill array with children
                Ptr<ArrayObject> parr = *SF_HEAP_NEW(penv->GetHeap()) ArrayObject(penv);
                XML::ElementNode* pnode = static_cast<XML::ElementNode*>(pRealNode);
                for (XML::Node* child = pnode->FirstChild; child != NULL; 
                    child = child->NextSibling)
                {
                    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(child->pShadow);
                    if (NULL == pshadow || NULL == pshadow->pASNode)
                        parr->PushBack(Value(CreateShadow(penv, child, pRootNode)));
                    else
                        parr->PushBack( pshadow->pASNode );
                }
                val->SetAsObject(parr);
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.nodeName
        case M_nodeName:
        
            if (pRealNode)
            {
                // Flash Doc: If the XML object is an XML element (nodeType == 1), nodeName is 
                // the name of the tag that represents the node in the XML file. For example, 
                // TITLE is the nodeName of an HTML TITLE tag. If the XML object is a text node 
                // (nodeType == 3), nodeName is null.
                //
                if (pRealNode->Type == XML::ElementNodeType)
                {
                    XML::ElementNode* pelem = static_cast<XML::ElementNode*>(pRealNode);
                    if (pelem->Prefix.GetSize() > 0)
                    {
                        ASString str = psc->CreateString(pelem->Prefix.ToCStr());
                        //GASString str = psc->CreateConstString(pelem->Prefix.ToCStr());
                        str += ":";
                        str += pelem->Value.ToCStr();
                        val->SetString(str);
                    }
                    else
                    {
                        val->SetString(psc->CreateString(pelem->Value.ToCStr()));
                        //val->SetString(psc->CreateConstString(pelem->Value.ToCStr()));
                    }
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.nodeValue
        case M_nodeValue:
        
            if (pRealNode)
            {
                // Flash Doc: If the XML object is a text node, the nodeType is 3, and the nodeValue 
                // is the text of the node. If the XML object is an XML element (nodeType is 1), 
                // nodeValue is null and read-only
                //
                if (pRealNode->Type == XML::ElementNodeType)
                    val->SetNull();
                else
                    val->SetString(psc->CreateString(pRealNode->Value.ToCStr()));
                    //val->SetString(psc->CreateConstString(pRealNode->Value.ToCStr()));
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.attributes
        case M_attributes:
        
            if (pRealNode) 
            {             
                // Flash Doc: The XML.attributes object contains one variable for each attribute 
                // of the XML instance. Because these variables are defined as part of the 
                // object, they are generally referred to as properties of the object. The value 
                // of each attribute is stored in the corresponding property as a string.
                //
                SF_ASSERT(pRealNode);
                XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pRealNode->pShadow);
                SF_ASSERT(pshadow);
                if (NULL == pshadow->pAttributes.GetPtr())
                    pshadow->pAttributes = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
                val->SetAsObject(pshadow->pAttributes);
            }
            else
                val->SetUndefined();
            return true;

        

        // ***** XMLNode.lastChild
        case M_lastChild:
        
            if (pRealNode)
            {
                if (pRealNode->Type == XML::ElementNodeType)
                {
                    // Flash Doc: The XML.lastChild property is null if the node 
                    // does not have children. This property cannot be used to 
                    // manipulate child nodes; use the appendChild(), insertBefore(), 
                    // and removeNode() methods to manipulate child nodes.
                    //
                    XML::ElementNode* pnode = static_cast<XML::ElementNode*>(pRealNode);
                    if (pnode->HasChildren())
                    {
                        XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->LastChild->pShadow);
                        if (NULL == pshadow || NULL == pshadow->pASNode)
                            val->SetAsObject(CreateShadow(penv, pnode->LastChild, pRootNode));
                        else
                            val->SetAsObject(static_cast<Object*>(pshadow->pASNode));
                    }
                    else
                        val->SetNull();
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.previousSibling
        case M_previousSibling:
        
            if (pRealNode)
            {
                // Flash Doc: The property has a value of null if the node does not 
                // have a previous sibling node. This property cannot be used to 
                // manipulate child nodes; use the appendChild(), insertBefore(), 
                // and removeNode() methods to manipulate child nodes.
                //
                XML::Node* pnode = static_cast<XML::Node*>(pRealNode);
                if (pnode->PrevSibling)
                {
                    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->PrevSibling->pShadow);
                    if (NULL == pshadow || NULL == pshadow->pASNode)
                        val->SetAsObject(CreateShadow(penv, pnode->PrevSibling, pRootNode));
                    else
                        val->SetAsObject(static_cast<Object*>(pshadow->pASNode));
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.parentNode
        case M_parentNode:
        
            if (pRealNode)
            {
                // Flash Doc: An XMLNode value that references the parent node of 
                // the specified XML object, or returns null if the node has no 
                // parent. This is a read-only property and cannot be used to 
                // manipulate child nodes; use the appendChild(), insertBefore(), 
                // and removeNode() methods to manipulate child nodes.
                //
                XML::Node* pnode = static_cast<XML::Node*>(pRealNode);
                if (pnode->Parent)
                {
                    XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pnode->Parent->pShadow);
                    if (NULL == pshadow || NULL == pshadow->pASNode)
                        val->SetAsObject(CreateShadow(penv, pnode->Parent, pRootNode));
                    else
                        val->SetAsObject(static_cast<Object*>(pshadow->pASNode));
                }
                else
                    val->SetNull();
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.localName
        case M_localName:
        
            if (pRealNode && pRealNode->Type == XML::ElementNodeType)
            {
                // Flash Doc: This is the element name without the namespace 
                // prefix. For example, the node 
                // <contact:mailbox/>bob@example.com</contact:mailbox> has the 
                // local name "mailbox", and the prefix "contact", which comprise 
                // the full element name "contact.mailbox". You can access the 
                // namespace prefix via the prefix property of the XML node object. 
                // The nodeName property returns the full name (including the 
                // prefix and the local name).
                //
                val->SetString(psc->CreateString(pRealNode->Value.ToCStr()));
                //val->SetString(psc->CreateConstString(pRealNode->Value.ToCStr()));
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.namespaceURI
        case M_namespaceURI:
        
            if (pRealNode && pRealNode->Type == XML::ElementNodeType)
            {
                // Flash Doc: If the XML node has a prefix, namespaceURI is the 
                // value of the xmlns declaration for that prefix (the URI), 
                // which is typically called the namespace URI. The xmlns 
                // declaration is in the current node or in a node higher in 
                // the XML hierarchy. If the XML node does not have a prefix, 
                // the value of the namespaceURI property depends on whether 
                // there is a default namespace defined (as in 
                // xmlns="http://www.example.com/"). If there is a default 
                // namespace, the value of the namespaceURI property is the 
                // value of the default namespace. If there is no default 
                // namespace, the namespaceURI property for that node is an 
                // empty string (""). You can use the getNamespaceForPrefix() 
                // method to identify the namespace associated with a specific 
                // prefix. The namespaceURI* [*PPS: this should be 'prefix'] 
                // property returns the prefix associated with the node name.
                //
                XML::ElementNode* pnode = static_cast<XML::ElementNode*>(pRealNode);
                val->SetString(psc->CreateString(pnode->Namespace.ToCStr()));
                //val->SetString(psc->CreateConstString(pnode->Namespace.ToCStr()));
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.prefix
        case M_prefix:
        
            if (pRealNode && pRealNode->Type == XML::ElementNodeType)
            {
                // Flash Doc: For example, the node 
                // <contact:mailbox/>bob@example.com</contact:mailbox> 
                // prefix "contact" and the local name "mailbox", which comprise 
                // the full element name "contact.mailbox". The nodeName property 
                // of an XML node object returns the full name (including the 
                // prefix and the local name). You can access the local name 
                // portion of the element's name via the localName property. 
                //
                XML::ElementNode* pnode = static_cast<XML::ElementNode*>(pRealNode);
                val->SetString(psc->CreateString(pnode->Prefix.ToCStr()));
                //val->SetString(psc->CreateConstString(pnode->Prefix.ToCStr()));
            }
            else
                val->SetUndefined();
            return true;
        

        // ***** XMLNode.nodeType
        case M_nodeType:
        
            // Flash Doc: A nodeType value, either 1 for an XML element or 3 for a text node. 
            // The nodeType is a numeric value from the NodeType enumeration in the W3C DOM 
            // Level 1 recommendation: 
            // www.w3.org/tr/1998/REC-DOM-Level-1-19981001/level-one-core.html.
            //
            if (pRealNode)
                val->SetNumber(pRealNode->Type);
            else
                val->SetUndefined();
            return true;
        
        default:
            {
                // Pass along to Object
            }
            
        }
    }

    // if property wasn't gobbled up by the custom handler, pass it along to the base 
    // class
    return Object::GetMember(penv, name, val);
}


// 
// AS to GFx function mapping
//
const NameFunction XmlNodeProto::FunctionTable[] = 
{
    { "appendChild", &XmlNodeProto::AppendChild },
    { "cloneNode", &XmlNodeProto::CloneNode },
    { "getNamespaceForPrefix", &XmlNodeProto::GetNamespaceForPrefix },
    { "getPrefixForNamespace", &XmlNodeProto::GetPrefixForNamespace },
    { "hasChildNodes", &XmlNodeProto::HasChildNodes },
    { "insertBefore", &XmlNodeProto::InsertBefore },
    { "removeNode", &XmlNodeProto::RemoveNode },
    { "toString", &XmlNodeProto::ToString },
    { 0, 0 }
};


// 
// GASXmlNode prototype constructor
//
XmlNodeProto::XmlNodeProto(ASStringContext *psc, Object* prototype, 
                                 const FunctionRef& constructor) :
    Prototype<XmlNodeObject>(psc, prototype, constructor)
{
    // make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("attributes"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("childNodes"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("firstChild"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("lastChild"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);  
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("localName"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("namespaceURI"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("nextSibling"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("nodeName"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("nodeType"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("nodeValue"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete);  
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("parentNode"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("prefix"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
    XmlNodeObject::SetMemberRaw(psc, psc->CreateConstString("previousSibling"), 
        Value::UNDEFINED, PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_ReadOnly);
}


// 
// XMLNode.appendChild(newChild:XMLNode) : Void
//
void XmlNodeProto::AppendChild(const FnCall& fn)
{
    // Flash Doc: Appends the specified node to the XML object's child list. This method 
    // operates directly on the node referenced by the childNode parameter; it does not 
    // append a copy of the node. If the node to be appended already exists in another 
    // tree structure, appending the node to the new location will remove it from its 
    // current location. If the childNode parameter refers to a node that already exists 
    // in another XML tree structure, the appended child node is placed in the new tree 
    // structure after it is removed from its existing parent node.
    //
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Log* log = fn.GetLog();
    if (pthis->pRealNode)
    {        
        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            // Only works for element nodes
            XML::ElementNode* elemNode = (XML::ElementNode*) pthis->pRealNode;
            // Get new child
            if (fn.NArgs > 0)
            {
                Object* p = fn.Arg(0).ToObject(fn.Env);
                if ( NULL == p || p->GetObjectType() != Object_XMLNode )
                {
                    if (log != NULL)
                        log->LogMessageById(Log_ScriptWarning, "XMLNode::appendChild"
                            " - trying to add a child that is not of type XMLNode");
                    // Do nothing (not an XMLNode object)
                    return;
                }
                XmlNodeObject* pchild = static_cast<XmlNodeObject*>(p);
                if (!pchild->pRealNode)
                {
                    // Do nothing (malformed object)
                    return;
                }
                // Check to see if child is parent of this node
                // If so, it is a circular reference
                XML::Node* ptopmost = pthis->pRealNode->Parent;
                while ( ptopmost && ptopmost->Parent != NULL)
                {
                    ptopmost = ptopmost->Parent;
                }
                if (ptopmost == pchild->pRealNode)
                {
                    // If child contains this node, then circular reference
                    // if appended.
                    if (log != NULL)
                        log->LogMessageById(Log_ScriptWarning, "XMLNode::appendChild"
                        " - trying to add a child that is the root of the current tree");
                    return;
                }
                // Keep a ref here else subtree will be nuked
                Ptr<XML::Node> selfptr = pchild->pRealNode;
                // Check if child already has a parent. 
                if (pchild->pRealNode->Parent)
                {
                    // Flash removes the child from its tree and allows
                    // it to be appended to the new tree
                    pchild->pRealNode->Parent->RemoveChild(pchild->pRealNode);
                }
                elemNode->AppendChild(pchild->pRealNode);
                // Setup root node
                pchild->pRootNode = pthis->pRootNode;

                // Resolve floating prefix of child
                if (pchild->pRealNode->Type == XML::ElementNodeType)
                {
                    XML::ElementNode* elemChild = 
                        static_cast<XML::ElementNode*>(pchild->pRealNode);
                    // [PPS] Flash appendChild does not resolve namespace, if namespace 
                    // is already assigned to the node
                    if (elemChild->Namespace.GetSize() == 0)
                    {
                        ResolveNamespace(fn.Env, elemChild, pthis->pRootNode);
                    }
                }
            }
        }
        else
        {
            // Adding to text nodes is non-deterministic behavior
            if (log != NULL)
                log->LogMessageById(Log_ScriptWarning, "XMLNode::appendChild"
                    " - trying to add a child to a text node");
        }
    }
}


// 
// XMLNode.cloneNode(deep:Boolean) : XMLNode
//
void XmlNodeProto::CloneNode(const FnCall& fn)
{
    // Flash Doc: Constructs and returns a new XML node of the same type, 
    // name, value, and attributes as the specified XML object. If deep is 
    // set to true, all child nodes are recursively cloned, resulting in an 
    // exact copy of the original object's document tree. The clone of the 
    // node that is returned is no longer associated with the tree of the 
    // cloned item. Consequently, nextSibling, parentNode, and previousSibling 
    // all have a value of null. If the deep parameter is set to false, or 
    // the my_xml node has no child nodes, firstChild and lastChild are also 
    // null.
    //
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (pthis->pRealNode)
    {        
        bool deepCopy = false;
        if ( fn.NArgs > 0 )
            deepCopy = fn.Arg(0).ToBool(fn.Env);

        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            // Set the real node
            Ptr<XML::Node> realclone = *pthis->pRealNode->Clone(deepCopy);
            // Make a copy of the AS node
            Ptr<XmlNodeObject> clone = CreateShadow(fn.Env, realclone, NULL);
            // Set return value
            fn.Result->SetAsObject(clone);
        }
        else
        {
            // Text nodes and unknown nodes have no deep copy semantics
            XML::TextNode* textNode = (XML::TextNode*) pthis->pRealNode;
            // Set the real node
            Ptr<XML::Node> realclone = *pthis->pRealNode->Clone(deepCopy);
            // Make a copy of the AS node
            Ptr<XmlNodeObject> clone = CreateShadow(fn.Env, realclone, NULL);
            // Set type since node may not be text node
            realclone->Type = textNode->Type;
            // Set return value
            fn.Result->SetAsObject(clone);
        }
    }
}


// 
// XMLNode.getNamespaceForPrefix(prefix:String) : String
//
void XmlNodeProto::GetNamespaceForPrefix(const FnCall& fn)
{
    // Flash Doc: Returns the namespace URI that is associated with the specified 
    // prefix for the node. To determine the URI, getPrefixForNamespace() searches up 
    // the XML hierarchy from the node, as necessary, and returns the namespace URI 
    // of the first xmlns declaration for the given prefix. If no namespace is defined 
    // for the specified prefix, the method returns null. If you specify an empty string 
    // ("") as the prefix and there is a default namespace defined for the node (as 
    // in xmlns="http://www.example.com/"), the method returns that default namespace 
    // URI. 

    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    fn.Result->SetNull();

    Log* log = fn.GetLog();
    if (pthis->pRealNode)
    {        
        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            XML::ElementNode* elemNode = 
                (XML::ElementNode*) pthis->pRealNode;

            // Get query string
            if (fn.NArgs < 1)               
                return;

            ASString str = fn.Arg(0).ToString(fn.Env);         
            // Create the property query
            ASString pfxquery = fn.Env->GetSC()->CreateString("xmlns", 5);
            if (str.GetSize() > 0)
            {
                pfxquery += ":";
                pfxquery += str.ToCStr();
            }
            Value qval(Value::UNDEFINED);

            // -- Look for namespace declaration in self first
            SF_ASSERT(elemNode);
            SF_ASSERT(elemNode->pShadow);
            XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(elemNode->pShadow);
            SF_ASSERT(pshadow);
            Object* pattribs = pshadow->pAttributes;
            SF_ASSERT(pattribs);
            pattribs->GetMember(fn.Env, pfxquery, &qval);
            if (!qval.IsUndefined())
            {
                fn.Result->SetString(qval.ToString(fn.Env));
                return;
            }

            // -- Traverse up parent chain and look for a namespace
            XML::ElementNode* parent = 
                static_cast<XML::ElementNode*>(elemNode->Parent);
            while (parent)
            {
                // Create a shadow if one does not exist
                if (NULL == parent->pShadow)
                    CreateShadow(fn.Env, parent, pthis->pRootNode);
                pshadow = static_cast<XMLShadowRef*>(parent->pShadow);
                SF_ASSERT(pshadow);
                pattribs = pshadow->pAttributes;
                SF_ASSERT(pattribs);
                // lookup prefix in properties
                pattribs->GetMember(fn.Env, pfxquery, &qval);
                if (!qval.IsUndefined())
                {
                    fn.Result->SetString(qval.ToString(fn.Env));
                    return;
                }
                parent = static_cast<XML::ElementNode*>(parent->Parent);
            }
        }
        else
        {
            if (log != NULL)
                log->LogMessageById(Log_ScriptWarning, "XMLNodeProto::"
                    "GetNamespaceForPrefix - only element nodes support this method.");
        }
    }
}


// 
// Member visitor to traverse the attributes.
// This is required because the namespace declarations are in the
// attributes.
//
struct XMLPrefixQuerier : public Object::MemberVisitor
{
    Environment* pEnv;
    ASString &pKey;
    Value &pVal;
    XMLPrefixQuerier(Environment* penv, ASString& pkey, Value& pval) 
        : pEnv(penv), pKey(pkey), pVal(pval) {}
    XMLPrefixQuerier& operator=( const XMLPrefixQuerier& x )
    {
        pEnv = x.pEnv;
        pKey = x.pKey;
        pVal = x.pVal;
        return *this;
    }
    void Visit(const ASString& name, const Value& val, UByte flags)
    {
        SF_UNUSED(flags);
        ASString valstr = val.ToString(pEnv);
        if (valstr.Compare_CaseCheck(pKey, true) && 
            !strncmp(name.ToCStr(), "xmlns", 5))
        {
            pVal.SetString(name);
        }
    }
};

// 
// XMLNode.getPrefixForNamespace(nsURI:String) : String
//
void XmlNodeProto::GetPrefixForNamespace(const FnCall& fn) 
{
    // Flash Doc: Returns the prefix that is associated with the specified 
    // namespace URI for the node. To determine the prefix, 
    // getPrefixForNamespace() searches up the XML hierarchy from the node, 
    // as necessary, and returns the prefix of the first xmlns declaration 
    // with a namespace URI that matches nsURI. If there is no xmlns 
    // assignment for the given URI, the method returns null. If there is an 
    // xmlns assignment for the given URI but no prefix is associated with the 
    // assignment, the method returns an empty string (""). 

    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    fn.Result->SetNull();

    ASStringContext* psc = fn.Env->GetSC();
    Log* log = fn.GetLog();
    if (pthis->pRealNode)
    {        
        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            XML::ElementNode* elemNode = 
                (XML::ElementNode*) pthis->pRealNode;

            // Get query string
            if (fn.NArgs < 1)               
                return;

            ASString str = fn.Arg(0).ToString(fn.Env);  
            Value qval(Value::UNDEFINED);
            XMLPrefixQuerier pq(fn.Env, str, qval);

            // -- Look for prefix declaration in self first
            SF_ASSERT(elemNode);
            SF_ASSERT(elemNode->pShadow);
            XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(elemNode->pShadow);
            SF_ASSERT(pshadow);
            Object* pattribs = pshadow->pAttributes;
            SF_ASSERT(pattribs);            
            pattribs->VisitMembers(psc, &pq, 0);
            if (!qval.IsUndefined())
            {
                fn.Result->SetString(qval.ToString(fn.Env));
            }
            // Get root node ref
            // -- Traverse up parent chain and look for a namespace
            XML::ElementNode* parent = 
                static_cast<XML::ElementNode*>(elemNode->Parent);
            while (fn.Result->IsNull() && parent)
            {
                // Create a shadow if one does not exist
                if (NULL == parent->pShadow)
                    CreateShadow(fn.Env, parent, pthis->pRootNode);
                pshadow = static_cast<XMLShadowRef*>(parent->pShadow);
                SF_ASSERT(pshadow);
                pattribs = pshadow->pAttributes;
                SF_ASSERT(pattribs);
                // lookup prefix in properties
                pattribs->VisitMembers(psc, &pq, 0);
                if (!qval.IsUndefined())
                {
                    fn.Result->SetString(qval.ToString(fn.Env));
                }
                parent = static_cast<XML::ElementNode*>(parent->Parent);
            }

            if (!fn.Result->IsNull())
            {
                // -- Remove the xmlns part
                ASString pfx = fn.Result->ToString(fn.Env);
                const char* data = pfx.ToCStr();
                const char* colon = strchr(data, ':');
                // Bounds checking is not needed because the string is at least 6 chars if
                // there is a colon and at least 5 chars without. this is guaranteed by the
                // member visitor constraint.
                if (colon)
                    fn.Result->SetString(psc->CreateString(data+6, pfx.GetSize()-6));
                else
                    fn.Result->SetString(psc->CreateString(data+5, pfx.GetSize()-5));
            }
        }
        else
        {
            if (log != NULL)
                log->LogMessageById(Log_ScriptWarning, "XMLNodeProto::"
                    "GetNamespaceForPrefix - only element nodes support this method.");
        }
    }
}


// 
// XMLNode.hasChildNodes() : Boolean
//
void XmlNodeProto::HasChildNodes(const FnCall& fn)
{
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    fn.Result->SetBool(false);

    if (pthis->pRealNode)
    {        
        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            XML::ElementNode* elemNode = (XML::ElementNode*) pthis->pRealNode;
            fn.Result->SetBool(elemNode->HasChildren());
        }
    }
}


// 
// XMLNode.insertBefore(newChild:XMLNode, insertPoint:XMLNode) : Void
//
void XmlNodeProto::InsertBefore(const FnCall& fn)
{
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Inserts a newChild node into the XML object's child list, 
    // before the insertPoint node. If insertPoint is not a child of the 
    // XMLNode object, the insertion fails

    if (pthis->pRealNode && pthis->pRealNode->Type == XML::ElementNodeType)
    {
        // Only works for element nodes
        XML::ElementNode* elemNode = (XML::ElementNode*) pthis->pRealNode;
        // Get new child and insertion point
        if (fn.NArgs > 1)
        {
            Object* child = fn.Arg(0).ToObject(fn.Env);
            Object* insert = fn.Arg(1).ToObject(fn.Env);
            if ( NULL == child || child->GetObjectType() != Object_XMLNode )
            {
                // Do nothing (not an XMLNode object)
                return;
            }
            // If insert is invalid, the child is appended. This is 
            // Flash behavior.
            XmlNodeObject* pchild = static_cast<XmlNodeObject*>(child);
            XmlNodeObject* pinsert = static_cast<XmlNodeObject*>(insert);
            if ( pinsert && pinsert->GetObjectType() == Object_XMLNode &&
                pinsert->pRealNode && pinsert->pRealNode->Parent &&
                pinsert->pRealNode->Parent == elemNode)
            {
                // Insertion node is a child of this node
                if (pchild->pRealNode)
                {
                    // Keep a ref here else subtree will be nuked
                    Ptr<XML::Node> selfptr = pchild->pRealNode;
                    if (pchild->pRealNode->Parent)
                    {
                        // Flash removes the child from its tree and allows
                        // it to be appended to the new tree
                        pchild->pRealNode->Parent->RemoveChild(pchild->pRealNode);
                    }
                    elemNode->InsertBefore(pchild->pRealNode, pinsert->pRealNode);
                    pchild->pRootNode = pthis->pRootNode;
                }
            }
            else if (pchild->pRealNode)
            {
                // Keep a ref here else subtree will be nuked
                Ptr<XML::Node> selfptr = pchild->pRealNode;
                // Simply append
                if (pchild->pRealNode->Parent)
                {
                    // Flash removes the child from its tree and allows
                    // it to be appended to the new tree
                    pchild->pRealNode->Parent->RemoveChild(pchild->pRealNode);
                }
                elemNode->AppendChild(pchild->pRealNode);
                pchild->pRootNode = pthis->pRootNode;
            }
        }
    }
}


// 
// XMLNode.removeNode() : Void
//
void XmlNodeProto::RemoveNode(const FnCall& fn)
{
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Removes the specified XML object from its parent. Also 
    // deletes all descendants of the node.

    if (pthis->pRealNode)
    {        
        XML::ElementNode* parent = pthis->pRealNode->Parent;
        if (parent)
        {
            pthis->pRootNode = *parent->MemoryManager->CreateRootNode(pthis->pRealNode);
            parent->RemoveChild(pthis->pRealNode);
        }
    }
}



// 
// Member visitor to traverse the attributes
//
struct XMLAttributeStringBuilder : public Object::MemberVisitor
{
    Environment* pEnv;
    StringBuffer& Dest;
    XMLAttributeStringBuilder(Environment* penv, StringBuffer& dest) 
        : pEnv(penv), Dest(dest) {}
    XMLAttributeStringBuilder& operator=( const XMLAttributeStringBuilder& x )
    {
        pEnv = x.pEnv;
        Dest = x.Dest;
        return *this;
    }
    void Visit(const ASString& name, const Value& val, UByte flags)
    {
        SF_UNUSED(flags);
        Dest += " ";
        Dest += name.ToCStr();
        Dest += "=\"";
        Dest += val.ToString(pEnv).ToCStr();
        Dest += "\"";        
    }
};


// 
// Create an XML string by traversing the DOM tree 
//
static void BuildXMLString(Environment* penv, XML::Node* proot, StringBuffer& dest)
{
    switch (proot->Type)
    {
    case XML::ElementNodeType: 
        {
            XML::ElementNode* pnode = static_cast<XML::ElementNode*>(proot);
            XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(proot->pShadow);

            // If document node, do not print it out - just visit children
            if (pshadow && pshadow->pASNode)
            {
                if (pshadow->pASNode->GetObjectType() == Object::Object_XML)
                {
                    XmlNodeObject* pobj = pshadow->pASNode;
                    // Print xmldecl
                    Value xmldecl;
                    pobj->GetMember(penv, 
                        penv->GetSC()->CreateConstString("xmlDecl"), 
                        &xmldecl);
                    if (!xmldecl.IsUndefined())
                    {
                        dest += xmldecl.ToString(penv).ToCStr();
                        Value ignorews;
                        pobj->GetMember(penv, 
                            penv->GetSC()->CreateConstString("ignoreWhite"), 
                            &ignorews);
                        if (!ignorews.ToBool(penv))
                        {
                            dest += "\n";
                        }
                    }
                    // Print children
                    for (XML::Node* child = pnode->FirstChild; 
                            child != NULL; child = child->NextSibling)
                        BuildXMLString(penv, child, dest);
                    break;
                }
            }

            dest += "<";

            if (pnode->Prefix.GetSize() > 0)
            {
                dest += pnode->Prefix.ToCStr();
                dest += ":";
            }
            dest += proot->Value.ToCStr();
            
            if (pshadow && pshadow->pAttributes)
            {
                // Shadow is present, get the attributes from the shadow
                XMLAttributeStringBuilder attrvis(penv, dest);
                SF_ASSERT(pshadow->pAttributes);
                Object* attrs = pshadow->pAttributes;
                if (attrs)
                    attrs->VisitMembers(penv->GetSC(), &attrvis, 0);
            }
            else
            {
                // Shadow is not present, get attributes from real node
                for (XML::Attribute* attr = pnode->FirstAttribute; 
                        attr != NULL; attr = attr->Next)
                {
                    dest += " ";
                    dest += attr->Name.ToCStr();
                    dest += "=\"";
                    dest += attr->Value.ToCStr();
                    dest += "\"";                    
                }
            }

            if (pnode->HasChildren())
                dest += ">";
            else
                dest += " />";

            for (XML::Node* child = pnode->FirstChild; child != NULL; 
                    child = child->NextSibling)
                BuildXMLString(penv, child, dest);
            if (pnode->HasChildren())
            {
                dest += "</";
                if (pnode->Prefix.GetSize() > 0)
                {
                    dest += pnode->Prefix.ToCStr();
                    dest += ":";
                }
                dest += proot->Value.ToCStr();
                dest += ">";
            }
            break;
        }
    default:    // Text nodes and unknowns
        {
            dest += proot->Value.ToCStr();
        }
    }
}


// 
// XMLNode.toString() : String
//
void XmlNodeProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR2(fn, XMLNode, XML);
    XmlNodeObject* pthis = (XmlNodeObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    StringBuffer str;
    if (pthis->pRealNode)
    {        
        if (pthis->pRealNode->Type == XML::ElementNodeType)
        {
            XML::ElementNode* elemNode = 
                (XML::ElementNode*) pthis->pRealNode;
            BuildXMLString(fn.Env, elemNode, str);
        }
        else // Text nodes and unknown types
        {
            str += pthis->pRealNode->Value.ToCStr();
        }
        fn.Result->SetString(fn.Env->CreateString(str.ToCStr(),str.GetSize()));
    }
    else
    {
        // Flash has inconsistent behavior when a node is created
        // with a type but no value. This malformed node returns
        // '[type Object] : undefined' when toString() is called on it.
        // GFx simply returns undefined, since this case should not happen
        // in production code.
        fn.Result->SetUndefined();
    }
}



// 
// Constructor func ctor
//
XmlNodeCtorFunction::XmlNodeCtorFunction(ASStringContext *psc) 
    : CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

// 
// Called when the constructor is invoked for the XMLNode class 
// (new XMLNode(...))
//
void XmlNodeCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<XmlNodeObject> pnode;
    // Need to check if object is of type XML as well
    if (fn.ThisPtr && 
        (fn.ThisPtr->GetObjectType() == Object::Object_XMLNode || 
         fn.ThisPtr->GetObjectType() == Object::Object_XML))
        pnode = static_cast<XmlNodeObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) XmlNodeObject(fn.Env);

    Log* log = fn.GetLog();

    // Handle the constructor parameters
    Value nodeType = Value::UNDEFINED;
    Value nodeValue = Value::UNDEFINED;
    if (fn.NArgs > 0)
    {
        nodeType = fn.Arg(0);
        if (fn.NArgs > 1)
            nodeValue = fn.Arg(1);
    }

    // Since the XMLNode is created from scratch, a real node must be 
    // explictly created
    if ( !nodeType.IsUndefined() )
    {
        Number nt = nodeType.ToNumber(fn.Env);
        // If node type is defined, a node value is required. if not,
        // default to Object
        if (!nodeValue.IsUndefined())
        {
            // A shadow should not already be assigned to this node.
            SF_ASSERT(pnode->pRealNode == NULL);

            // Get the global (to movie root) object manager
            MovieImpl* pmovieroot = fn.Env->GetMovieImpl();
            Ptr<XML::ObjectManager> memMgr;
            if (!pmovieroot->pXMLObjectManager)
            {
                memMgr = *SF_HEAP_NEW(fn.Env->GetHeap()) XML::ObjectManager(pmovieroot);
                pmovieroot->pXMLObjectManager = memMgr;
            }
            else
                memMgr = (XML::ObjectManager*)pmovieroot->pXMLObjectManager;

            XML::DOMString localname(memMgr->EmptyString());
            XML::DOMString prefix(memMgr->EmptyString());

            ASString s = nodeValue.ToString(fn.Env);

            if (nt == XML::ElementNodeType)
            {            
                // Name may have a prefix
                const char* pfx = strchr(s.ToCStr(), ':');
                if (pfx)
                {
                    // Create a prefix mapping local to this node.
                    // this will have to be resolved if node is attached
                    // to a tree.
                    prefix = memMgr->CreateString(s.ToCStr(), pfx-s.ToCStr());
                    localname = memMgr->CreateString(pfx+1, SFstrlen(pfx));
                }
                else
                    localname = memMgr->CreateString(s.ToCStr(), s.GetSize());

                Ptr<XML::ElementNode> en = *memMgr->CreateElementNode(localname);
                pnode->pRealNode = en;
                pnode->pRootNode = *memMgr->CreateRootNode(pnode->pRealNode);
                XML::ElementNode* elemNode = 
                    static_cast<XML::ElementNode*>(pnode->pRealNode);
                elemNode->Prefix = prefix;
            }
            else if (nt == XML::TextNodeType)
            {
                localname = memMgr->CreateString(s.ToCStr(), s.GetSize());
                Ptr<XML::TextNode> tn = *memMgr->CreateTextNode(localname);
                pnode->pRealNode = tn;
                pnode->pRootNode = *memMgr->CreateRootNode(pnode->pRealNode);
            }
            else
            {
                // If node type is neither 1 or 3, then the values 
                // should not matter. But AS 2.0 converts them into text nodes..
                // Flash Docs: "In Flash Player, the XML class only supports 
                // node types 1 (ELEMENT_NODE) and 3 (TEXT_NODE)." 
                Ptr<XML::TextNode> tn = *memMgr->CreateTextNode(localname);
                pnode->pRealNode = tn;
                pnode->pRootNode = *memMgr->CreateRootNode(pnode->pRealNode);
                pnode->pRealNode->Type = static_cast<UByte>(nt);
            }
            SetupShadow(fn.Env, pnode->pRealNode, pnode);
        }
        else
        {
            if (log != NULL)
                log->LogMessageById(Log_ScriptWarning, "XMLNodeCtorFunction::"
                    "GlobalCtor - malformed XMLNode object");
        }
    }
    else
    {
        if (log != NULL)
            log->LogMessageById(Log_ScriptWarning, "XMLNodeCtorFunction::"
                "GlobalCtor - node type not specified");
    }

    fn.Result->SetAsObject(pnode.GetPtr());
}

Object* XmlNodeCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) XmlNodeObject(penv);
}


FunctionRef XmlNodeCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) XmlNodeCtorFunction(&sc));
    Ptr<XmlNodeProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) XmlNodeProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_XMLNode, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_XMLNode), Value(ctor));

    // Preload the XML and XMLNode member names as GASStrings and 
    // store them locally for fast access
    XmlNodeObject::InitializeStandardMembers(*pgc, proto->XMLNodeMemberMap);

    return ctor;
}

}}} // namespace SF::GFx::AS2

#endif // #ifdef GFX_ENABLE_XML
