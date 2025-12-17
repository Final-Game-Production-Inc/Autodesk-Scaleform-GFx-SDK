/**************************************************************************

Filename    :   XML_Document.cpp
Content     :   XML SAX and DOM support
Created     :   December 13, 2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Document.h"

namespace Scaleform {
    namespace GFx {
        namespace XML {

// --------------------------------------------------------------------


// 
// Base node
//

Node::Node(ObjectManager* memMgr, UByte type)
: MemoryManager(memMgr), Value(MemoryManager->EmptyString()), 
Parent(NULL), PrevSibling(NULL), pShadow(NULL), Type(type) {} 

Node::Node(ObjectManager* memMgr, UByte type, 
                       DOMString value)
: MemoryManager(memMgr), Value(value), Parent(NULL), 
PrevSibling(NULL), pShadow(NULL), Type(type) {}

//
// Freeing shadow memory
//
Node::~Node()
{
    if (pShadow)
        delete pShadow;
}


// --------------------------------------------------------------------


// 
// Element node
//

ElementNode::ElementNode(ObjectManager* memMgr)
: Node(memMgr, ElementNodeType),
Prefix(memMgr->EmptyString()), Namespace(memMgr->EmptyString()),
FirstAttribute(NULL), LastAttribute(NULL),
LastChild(NULL) {}

ElementNode::ElementNode(ObjectManager* memMgr, 
                                     DOMString value)
: Node(memMgr, ElementNodeType, value), 
Prefix(memMgr->EmptyString()), Namespace(memMgr->EmptyString()),
FirstAttribute(NULL), LastAttribute(NULL), LastChild(NULL) {}

//
// Debug reporting and freeing attribute memory
//
ElementNode::~ElementNode()
{
#ifdef SF_XML_OBJECT_REFCOUNTING_TRACE
    SF_DEBUG_MESSAGE1(1, "ElementNode::~ElementNode : '%.16s'", 
        Value.ToCStr());
#endif

    // Remove children explicitly to avoid stack overflow caused by
    // destructor chain
    Node* currChild = LastChild, *prevChild = NULL;
    while (currChild)
    {
        // Remove parent references
        currChild->Parent = NULL;
        prevChild = currChild->PrevSibling;
        if (prevChild)
            prevChild->NextSibling = NULL;
        currChild = prevChild;
    }
    if (FirstChild)
        FirstChild = NULL;

    // Remove attributes manually
    for (Attribute* attr = FirstAttribute; attr != NULL; )
    {
        Attribute* atemp = attr->Next;
        delete attr;
        attr = atemp;
    }
}


//
// Deep or shallow copy of the element node
//
Node* ElementNode::Clone(bool deep)
{
    ElementNode* node = MemoryManager->CreateElementNode(Value);
    CloneHelper(node, deep);
    node->Type = Type;
    return node;
}


//
// Clone helper
//
void ElementNode::CloneHelper(ElementNode* clone, bool deep)
{
    // set the same prefix
    clone->Prefix = Prefix;
    // copy attributes
    for (Attribute* attr = FirstAttribute; attr != NULL; attr = attr->Next)
    {
        Attribute* newattr = MemoryManager->CreateAttribute(attr->Name, 
            attr->Value);
        clone->AddAttribute(newattr);
    }
    if (deep)
    {
        // recursive descent for children
        for (Node* child = FirstChild; child != NULL; 
            child = child->NextSibling)
        {            
            Ptr<Node> childClone = *child->Clone(deep);
            clone->AppendChild(childClone);
        }
    }
}


//
// Append a child to the end of the children linked list
//
void ElementNode::AppendChild(Node* xmlNode)
{
    // Precondition check
    SF_ASSERT(xmlNode);
    SF_ASSERT(NULL == xmlNode->Parent);
    SF_ASSERT(NULL == xmlNode->PrevSibling);
    SF_ASSERT(NULL == xmlNode->NextSibling.GetPtr());

    if (LastChild == NULL)
    {
        // No children
        FirstChild = xmlNode;
        LastChild = xmlNode;
        xmlNode->Parent = this;
    }
    else
    {
        SF_ASSERT(NULL == LastChild->NextSibling.GetPtr());
        // Has children
        xmlNode->PrevSibling = LastChild;
        LastChild->NextSibling = xmlNode;
        LastChild = xmlNode;
        xmlNode->Parent = this;
    }
}


//
// Insert a child before the specified child
//
void ElementNode::InsertBefore(Node* child, Node* insert)
{
    // Precondition check
    SF_ASSERT(FirstChild);
    SF_ASSERT(LastChild);
    SF_ASSERT(insert);
    SF_ASSERT(this == insert->Parent);
    SF_ASSERT(child);
    SF_ASSERT(NULL == child->Parent);
    SF_ASSERT(NULL == child->PrevSibling);
    SF_ASSERT(NULL == child->NextSibling.GetPtr());

    Node* temp = insert->PrevSibling;
    insert->PrevSibling = child;
    child->PrevSibling = temp;
    child->NextSibling = insert;
    if (temp)
        temp->NextSibling = child;
    if (insert == FirstChild)
    {
        FirstChild = child;
    }
    child->Parent = this;
}


//
// Remove a child from the children list
//
void ElementNode::RemoveChild(Node* xmlNode)
{
    // Precondition check
    SF_ASSERT(xmlNode);
    SF_ASSERT(xmlNode->Parent == this);

    Ptr<Node> temp = xmlNode;
    if (xmlNode == FirstChild)
    {
        FirstChild = xmlNode->NextSibling;
    }
    if (xmlNode == LastChild)
    {
        LastChild = xmlNode->PrevSibling;
    }
    if (xmlNode->NextSibling)
        xmlNode->NextSibling->PrevSibling = xmlNode->PrevSibling;
    if (xmlNode->PrevSibling)
        xmlNode->PrevSibling->NextSibling = xmlNode->NextSibling;
    xmlNode->Parent = NULL;
    xmlNode->NextSibling = NULL;
    xmlNode->PrevSibling = NULL;
}


//
// Return true if there are children
//
bool ElementNode::HasChildren()
{
    return (NULL != FirstChild.GetPtr());
}


//
// Add an attribute to the attributes list
//
void ElementNode::AddAttribute(Attribute* xmlAttrib)
{
    // Precondition check
    SF_ASSERT(xmlAttrib);
    SF_ASSERT(NULL == xmlAttrib->Next);

    if (FirstAttribute == NULL)
    {
        // No items in attribute list
        FirstAttribute = xmlAttrib;
        LastAttribute = xmlAttrib;
    }
    else
    {
        // There are items in attribute list
        SF_ASSERT(LastAttribute->Next == NULL);
        LastAttribute->Next = xmlAttrib;
        LastAttribute = xmlAttrib;
    }
}

//
// Remove an attribute from the attributes list
//
bool ElementNode::RemoveAttribute(const char* str, unsigned len)
{
    // Find the attribute
    Attribute *found = NULL, *prev = NULL;
    for (Attribute* attr = FirstAttribute; attr != NULL; attr = attr->Next)
    {
        if (strncmp(str, attr->Name.ToCStr(), len) == 0)
        {
            found = attr;
            break;
        }
        prev = attr;
    }
    if (found)
    {
        if (FirstAttribute == found)
        {
            FirstAttribute = found->Next;
        }
        if (LastAttribute == found)
        {
            LastAttribute = prev;
        }
        if (prev)
        {
            prev->Next = found->Next;
        }
        delete found;
        return true;
    }
    else 
        return false;
}


//
// Remove all attributes from the attribute list
//
void ElementNode::ClearAttributes()
{
    Attribute* temp = FirstAttribute;
    for (Attribute* attr = FirstAttribute; attr != NULL; )
    {
        temp = attr->Next;
        delete attr;
        attr = temp;
    }
    FirstAttribute = NULL;
    LastAttribute = NULL;
}


//
// Return true if there are attributes in the attribute list
//
bool ElementNode::HasAttributes()
{
    return (NULL != FirstAttribute);
}


// --------------------------------------------------------------------


// 
// Text node
//

TextNode::TextNode(ObjectManager* memMgr, DOMString value)
: Node(memMgr, TextNodeType, value) {}

// Debug reporting
TextNode::~TextNode()
{
#ifdef SF_XML_OBJECT_REFCOUNTING_TRACE
    SF_DEBUG_MESSAGE1(1, "TextNode::~ElementNode : '%.16s'", Value.ToCStr());
#endif
}

//
// Deep copy or shallow copy of the text node.
//
// Either performs the same operation.
//
Node* TextNode::Clone(bool deep)
{
    SF_UNUSED(deep);
    TextNode* clone = MemoryManager->CreateTextNode(Value);
    clone->Type = Type;
    return clone;
}


// --------------------------------------------------------------------


// 
// Document
//

Document::Document(ObjectManager* memMgr)
: ElementNode(memMgr), XMLVersion(MemoryManager->EmptyString()),
Encoding(MemoryManager->EmptyString()), Standalone(-1) {}

// Debug reporting
Document::~Document()
{
#ifdef SF_XML_OBJECT_REFCOUNTING_TRACE
    SF_DEBUG_MESSAGE(1, "Document::~Document");
#endif
}

//
// Deep or shallow copy of the DOM tree root
//
Node* Document::Clone(bool deep)
{
    Document* doc = MemoryManager->CreateDocument();
    ElementNode::CloneHelper(doc, deep);
    doc->Encoding = Encoding;
    doc->Standalone = Standalone;
    doc->XMLVersion = XMLVersion;
    doc->Type = Type;
    return doc;
}

}}} // namespace SF::GFx::XML

#endif // #ifdef GFX_ENABLE_XML 
