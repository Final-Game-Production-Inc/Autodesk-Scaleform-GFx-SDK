/**************************************************************************

Filename    :   XML_Document.h
Content     :   XML DOM support
Created     :   December 13, 2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_XMLDocument_H
#define INC_SF_GFX_XMLDocument_H

#include <GFxConfig.h>

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Object.h"

namespace Scaleform { namespace GFx { namespace XML {

// forward declarations
class DOMBuilder;


struct ShadowRefBase : NewOverrideBase<StatMV_XML_Mem>
{
public:
    virtual ~ShadowRefBase() {}
};


// --------------------------------------------------------------------

// 
// Attribute name/value pair
//
// Not refcounted because they are local to one and only one 
// ElementNode. It is responsible for managing the
// attribute memory.
//
struct Attribute : public NewOverrideBase<StatMV_XML_Mem>
{
    // Attribute name
    DOMString         Name;  

    // Attribute value
    DOMString         Value; 

    // The attributes are stored as a linked list
    Attribute*        Next; 

    Attribute(DOMString name, DOMString value) :
        Name(name), Value(value), Next(NULL) {}
};


// --------------------------------------------------------------------


// 
// Prefix declaration
//
// Holds prefix and namespace value. This is only used during
// DOM tree creation and does not exist in a constructed
// tree.
//
struct Prefix : public RefCountBaseNTS<Prefix, StatMV_XML_Mem>
{
    // Prefix 
    DOMString            Name;  

    // Namespace URL
    DOMString            Value; 

    Prefix(DOMString name, DOMString value)
        : Name(name), Value(value) {}
};


// --------------------------------------------------------------------

// 
// Base class for the DOM nodes
//
// It is the super class of Element and Text nodes
//
struct Node : public RefCountBaseNTS<Node, StatMV_XML_Mem>
{
    // ObjectManager reference
    Ptr<ObjectManager>   MemoryManager;

    // Either qualified name (element node) or text value (text node)
    DOMString         Value;    

    // Parent node
    ElementNode*      Parent;  

    // Only next sibling pointers are reference counted because
    // the children representation follows a first-child next-sibling
    // scheme.
    Node*             PrevSibling;
    Ptr<Node>        NextSibling;

    // A pointer to a structure that is shadowing this node
    ShadowRefBase*        pShadow;

    // Type of node.
    // 1: Element, 3: Text, Rest: Everything else
    UByte                   Type; 

    // Return a clone (deep or shallow) of the node
    // Implementation is left to the subclass
    virtual Node*     Clone(bool deep) = 0;

protected:

    Node(ObjectManager* memMgr, UByte type);
    Node(ObjectManager* memMgr, UByte type, 
        DOMString value);
    virtual ~Node();
};


// --------------------------------------------------------------------


//
// Built-in node types (following AS 2.0 node type constants)
//
const UByte                 ElementNodeType  = 1;
const UByte                 TextNodeType     = 3;


// 
// Element node (<element>text</element>)
//
struct ElementNode : public Node
{
    // The prefix of the element node
    DOMString         Prefix;

    // The namespace URL of the element node
    DOMString         Namespace;

    // None of attributes are reference counted because
    // the elementnode is responsible for managing them.
    // No external references should be held. If needed,
    // a seperate copy should be created.
    Attribute*        FirstAttribute;
    Attribute*        LastAttribute;

    // Only first child pointers are reference counted because
    // the children representation follows a first-child next-sibling
    // scheme.
    Ptr<Node>        FirstChild;
    Node*             LastChild;

    ElementNode(ObjectManager* memMgr);
    ElementNode(ObjectManager* memMgr, DOMString value);
    ~ElementNode();

    // Deep or shallow copy of the element node
    Node*             Clone(bool deep);

    // Child management functions
    void                    AppendChild(Node* xmlNode);
    void                    InsertBefore(Node* child, Node* insert);
    void                    RemoveChild(Node* xmlNode);
    bool                    HasChildren();

    // Attribute management functions
    void                    AddAttribute(Attribute* xmlAttrib);
    bool                    RemoveAttribute(const char* str, unsigned len);
    void                    ClearAttributes();
    bool                    HasAttributes();

protected:

    void                    CloneHelper(ElementNode* clone, bool deep);
};


// --------------------------------------------------------------------


// 
// Text node (<element>text</element>)
//
// Contains no extra member variables besides the ones
// provided by Node. Exists for posterity.
//
struct TextNode : public Node
{
    TextNode(ObjectManager* memMgr, DOMString value);
    ~TextNode();

    // Deep or shallow copy of the text node.
    // Deep copy semantics do not exist for this type.
    Node*             Clone(bool deep);
};


// --------------------------------------------------------------------


// 
// The root of the DOM tree
//
struct Document : public ElementNode
{
    // XML declaration holders
    DOMString         XMLVersion;
    DOMString         Encoding;
    SByte                   Standalone;

    Document(ObjectManager* memMgr);
    ~Document();

    // Deep or shallow copy of the DOM root
    Node*             Clone(bool deep);
};


// --------------------------------------------------------------------

//
// Root object for maintaining correct XML tree lifetimes
// in the ActionScript runtime. 
// All XMLNode objects created in will point to an instance
// of this object. A root node will point to the top most
// node in the actual C++ DOM tree.
//
struct RootNode : public RefCountBaseNTS<RootNode, StatMV_XML_Mem>
{
    Ptr<Node>    pDOMTree;

    RootNode(Node* pdom) : pDOMTree(pdom) {}
};

}}} //SF::GFx::XML

#endif  // #ifdef GFX_ENABLE_XML

#endif // INC_SF_GFX_XML_H
