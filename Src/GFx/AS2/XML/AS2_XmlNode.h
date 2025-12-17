/**************************************************************************

Filename    :   AS2_XmlNode.h
Content     :   XMLNode reference class for ActionScript 2.0
Created     :   11/30/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_ASXMLNODE_H
#define INC_SF_GFX_ASXMLNODE_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/AS2_ActionTypes.h"

#include "GFx/XML/XML_Document.h"
#include "GFx/AS2/XML/AS2_XMLShadowRef.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class XmlNodeObject;
class XmlNodeProto;
class XmlNodeCtorFunction;


// Forward decl
void Xml_CreateIDMap(Environment* penv, XML::ElementNode* elemNode, XML::RootNode* proot, Object* pobj);

// 
// Actionscript XMLNode object declaration
//

class XmlNodeObject : public Object
{
    friend class XmlNodeProto;
    friend class XmlNodeCtorFunction;

protected:

#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC()
    {
        // Remove itself as the realnode's (C++) shadow reference
        if (pRealNode && pRealNode->pShadow)
        {
            XMLShadowRef* pshadow = static_cast<XMLShadowRef*>(pRealNode->pShadow);
            pshadow->pASNode = NULL;
        }
        pRootNode = NULL;
        Object::Finalize_GC();
    }
#endif // SF_NO_GC

    XmlNodeObject(ASStringContext *psc, Object* pprototype) 
        : Object(psc), pRealNode(NULL)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

    virtual             ~XmlNodeObject();

public:

    Ptr<XML::RootNode>  pRootNode;
    XML::Node*             pRealNode;

    XmlNodeObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_XMLNode; }

    virtual bool        SetMember(Environment* penv, 
                            const ASString& name, const Value& val, 
                            const PropFlags& flags = PropFlags());
    virtual bool        GetMember(Environment* penv, 
                            const ASString& name, Value* val);

    //
    // Enumeration of XMLNode properties for fast comparison
    // in GetMember/SetMember
    //
    enum StandardMember
    {
        M_InvalidMember = -1,
        M_attributes    = 0,
        M_childNodes,
        M_firstChild,
        M_lastChild,
        M_localName,
        M_namespaceURI,
        M_nextSibling,
        M_nodeName,
        M_nodeType,
        M_nodeValue,
        M_parentNode,
        M_prefix,
        M_previousSibling,

        M_XMLNodeMemberCount
    };

    // Looks up a standard member and returns M_InvalidMember if it is not found.
    StandardMember      GetStandardMemberConstant(Environment* penv, 
                                                  const ASString& memberName) const;

    // Initialize the XMLNode standard member hash
    static void         InitializeStandardMembers(GlobalContext& gc, 
                                                  ASStringHash<SByte>& hash);
};


class XmlNodeProto : public Prototype<XmlNodeObject>
{
#ifdef GFX_AS_ENABLE_GC
protected:
    void Finalize_GC()
    {
        XMLNodeMemberMap.~ASStringHash<SByte>();
        Prototype<XmlNodeObject>::Finalize_GC();
    }
#endif // SF_NO_GC
public:
    // A map of XMLNode member names for efficient access. The values 
    //in it are of InteractiveObject::StandardMember type.
    ASStringHash<SByte>        XMLNodeMemberMap;

    XmlNodeProto(ASStringContext *psc, Object* prototype, 
                    const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default XMLNode object functions
    //
    static void         AppendChild(const FnCall& fn);
    static void         CloneNode(const FnCall& fn);
    static void         GetNamespaceForPrefix(const FnCall& fn);
    static void         GetPrefixForNamespace(const FnCall& fn);
    static void         HasChildNodes(const FnCall& fn);
    static void         InsertBefore(const FnCall& fn);
    static void         RemoveNode(const FnCall& fn);
    static void         ToString(const FnCall& fn);
};


class XmlNodeCtorFunction : public CFunctionObject
{
public:
    XmlNodeCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

}}}  //namespace SF::GFx::AS2

#endif  // #ifdef GFX_ENABLE_XML

#endif  // INC_SF_GFX_ASXMLNODE_H
