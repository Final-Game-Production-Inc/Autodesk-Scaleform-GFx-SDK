/**************************************************************************

Filename    :   AS2_Xml.h
Content     :   XML reference class for ActionScript 2.0
Created     :   11/30/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_ASXML_H
#define INC_SF_GFX_ASXML_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "Kernel/SF_File.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/XML/AS2_XmlNode.h" 
#include "GFx/AS2/AS2_AsBroadcaster.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//
// Actionscrip XML object declaration
//

// ***** Declared Classes
class XmlObject;
class XmlProto;
class XmlCtorFunction;


class XmlObject : public XmlNodeObject
{
    friend class XmlProto;
    friend class XmlCtorFunction;

    Number       BytesLoadedCurrent;
    Number       BytesLoadedTotal;

protected:
    XmlObject(ASStringContext *psc, Object* pprototype) : 
         XmlNodeObject(psc, pprototype)
    { }
public:
    XmlObject(Environment* penv);

    virtual ObjectType GetObjectType() const { return Object_XML; }

    //
    // XML object callbacks
    //
    void            NotifyOnData(Environment* penv, Value val);
    void            NotifyOnHTTPStatus(Environment* penv, 
                        Number httpStatus);
    void            NotifyOnLoad(Environment* penv, bool success);

    void            SetLoadedBytes(Number total, Number loaded);

    void            AssignXMLDecl(Environment* penv, 
                        XML::Document* pdoc);
};


class XmlProto : public Prototype<XmlObject>
{
public:
    XmlProto(ASStringContext *psc, Object* prototype, 
                const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default XML object functions
    //
    static void     AddRequestHeader(const FnCall& fn);
    static void     CreateElement(const FnCall& fn);
    static void     CreateTextNode(const FnCall& fn);
    static void     GetBytesLoaded(const FnCall& fn);
    static void     GetBytesTotal(const FnCall& fn);
    static void     Load(const FnCall& fn);
    static void     ParseXML(const FnCall& fn);
    static void     Send(const FnCall& fn);
    static void     SendAndLoad(const FnCall& fn);

    static void     DefaultOnData(const FnCall& fn);
};


class XmlCtorFunction : public CFunctionObject
{
public:
    XmlCtorFunction(ASStringContext *psc);

    static void             GlobalCtor(const FnCall& fn);
    virtual Object*      CreateNewObject(Environment* penv) const;

    static FunctionRef   Register(GlobalContext* pgc);
};

}}} // namespace SF::GFx::AS2

#endif  // ##ifdef GFX_ENABLE_XML

#endif  // INC_SF_GFX_ASXML_H
