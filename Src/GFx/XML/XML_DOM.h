/**************************************************************************

Filename    :   XML_DOM.h
Content     :   DOM support
Created     :   December 13, 2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_XML_H
#define INC_SF_GFX_XML_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/GFx_Loader.h"
#include "GFx/GFx_StringHash.h"

#include "GFx/XML/XML_Parser.h"
#include "GFx/XML/XML_Document.h"


//
// This is an implementation of the DOM tree builder. It receives events
// from the parser and builds the tree appropriately, and returns a
// Document object (the root of the DOM). Applications developers
// are free to instantiate an instance of this class to generate a DOM
// of an XML file using the parser state installed on the loader, or
// a separate parser instance. Instances of the document builder are used
// internally by the XML actionscript class.
//

namespace Scaleform { namespace GFx { namespace XML {

// 
// DOM tree builder implementation
//
class DOMBuilder : public ParserHandler
{
public:
    DOMBuilder(Ptr<SupportBase> pxmlParser, bool ignorews = false);

    Ptr<Document>    ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                                      Ptr<ObjectManager> objMgr = NULL);
    Ptr<Document>    ParseString(const char* pdata, UPInt len, 
                                        Ptr<ObjectManager> objMgr = NULL);

    static void             DropWhiteSpaceNodes(Document* document);

    // DOMBuilder implementation
    void                    StartDocument();
    void                    EndDocument();
    void                    StartElement(const StringRef& prefix, 
                                         const StringRef& localname,
                                         const ParserAttributes& atts);
    void                    EndElement(const StringRef& prefix,
                                       const StringRef& localname);
    void                    PrefixMapping(const StringRef& prefix, 
                                          const StringRef& uri);
    void                    Characters(const StringRef& text);
    void                    IgnorableWhitespace(const StringRef& ws);
    void                    SkippedEntity(const StringRef& name);
    void                    SetDocumentLocator(const ParserLocator* plocator);
    void                    Comment(const StringRef& text);
    void                    Error(const ParserException& exception);
    void                    FatalError(const ParserException& exception);
    void                    Warning(const ParserException& exception);

    bool                    IsError() { return bError; }
    UPInt                   GetTotalBytesToLoad() { return TotalBytesToLoad; }
    UPInt                   GetLoadedBytes() { return LoadedBytes; }

private:

    Ptr<SupportBase>     pXMLParserState;

    Ptr<TextNode>    pAppendChainRoot;
    StringBuffer           AppendText;

    // Locator object installed by the parser that keeps track of the
    // parser state.
    const ParserLocator*  pLocator;

    // Stack used to keep track of the element node hierarchies
    Array< Ptr<ElementNode> > ParseStack;

    // Structure used to define xml namespace declaration ownership
    struct PrefixOwnership
    {
        Ptr<Prefix>      mPrefix;
        Ptr<ElementNode> Owner;
        PrefixOwnership() 
            : mPrefix(NULL), Owner(NULL) {}
        PrefixOwnership(Ptr<Prefix> pprefix, Ptr<ElementNode> pnode)
            : mPrefix(pprefix), Owner(pnode) {}
    };
    // Stacks used to keep track of the current namespaces
    Array<PrefixOwnership> PrefixNamespaceStack;
    Array<PrefixOwnership> DefaultNamespaceStack;

    // The document created by the document builder for each parse
    Ptr<Document>    pDoc;

    bool                    bIgnoreWhitespace;
    bool                    bError;

    UPInt                   TotalBytesToLoad;
    UPInt                   LoadedBytes;
};

}}} //namespace SF::GFx::XML

#endif  // #ifdef GFX_ENABLE_XML

#endif // INC_SF_GFX_XML_H
