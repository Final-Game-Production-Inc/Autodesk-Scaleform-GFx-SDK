/**************************************************************************

Filename    :   XML_DOM.cpp
Content     :   DOM support
Created     :   December 13, 2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Support.h"
#include "GFx/XML/XML_DOM.h"

namespace Scaleform { namespace GFx { namespace XML {

// 
// Debug trace/dump helper functions
//
#ifdef SF_BUILD_DEBUG

//
// Trace the document builder's DOM tree construction in debug output
//
// #define SF_XML_DOCBUILDER_TRACE
//

//
// Dump all constructed DOM trees to standard out
// if provided to the document builder
// (Warning: Avoid this for large files)
//
// #define SF_XML_DOCBUILDER_DOM_TREE_DUMP
//


// Maximum size of strings used for debug printing
#define MAX_DEBUG_STRING_LENGTH 256
// 
// Helper to copy strings for debugging
//
void SafeStringCopyWithSentinel(const char* src, UPInt srcLen, 
                                char* dest, UPInt destSz)
{
    UPInt len = 0;
    if (srcLen  < destSz)
        len = srcLen;
    else
        len = (destSz - 1);
    SFstrncpy(dest, len + 1, src, len);
    // explicit sentinel just in case
    dest[len] = '\0';
}

#ifdef SF_XML_DOCBUILDER_DOM_TREE_DUMP
//
// Prints the DOM tree to stdout
//
char printDOMBuffer[128];
void PrintDOMTree(XML::Node* proot, UPInt level = 0)
{
    for (UPInt i=0; i < level; i++)
        printf("  ");
    switch (proot->Type)
    {
    case XML::ElementNodeType: 
        {
            XML::ElementNode* pnode = static_cast<XML::ElementNode*>(proot);
            if (pnode->Prefix.GetSize() > 0)
            {
                printf("ELEM - '%.16s:%.16s' ns:'%.16s' prefix:'%.16s'"
                    " localname: '%.16s'",
                    pnode->Prefix.ToCStr(),
                    pnode->Value.ToCStr(),
                    pnode->Namespace.ToCStr(),
                    pnode->Prefix.ToCStr(),
                    pnode->Value.ToCStr());
            }
            else
            {
                printf("ELEM - '%.16s' ns:'%.16s' prefix:''"
                    " localname: '%.16s'",
                    pnode->Value.ToCStr(), 
                    pnode->Namespace.ToCStr(), 
                    pnode->Value.ToCStr());
            }
            for (XML::Attribute* attr = pnode->FirstAttribute; 
                attr != NULL; attr = attr->Next)
            {
                printf(" {%.16s,%.16s}", attr->Name.ToCStr(), 
                attr->Value.ToCStr());
            }
            printf("\n");
            for (XML::Node* child = pnode->FirstChild; child != NULL; 
                child = child->NextSibling)
                PrintDOMTree(child, level+1);
            break;
        }
    case XML::TextNodeType: 
        {
            printf("TEXT - '%.16s'\n", proot->Value.ToCStr());
            break;
        }
    default: 
        {
            printf("UNKN\n");
        }
    }
}

#endif  // #ifdef SF_XML_DOCBUILDER_DOM_TREE_DUMP

#endif  // #ifdef SF_BUILD_DEBUG


// 
// Constructor
//
DOMBuilder::DOMBuilder(Ptr<XML::SupportBase> pxmlParser, 
                                           bool ignorews)
    : pXMLParserState(pxmlParser), pLocator(NULL), pDoc(NULL), 
      bIgnoreWhitespace(ignorews), bError(false)
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_MESSAGE(1, "XML::DocumentBuilder::XML::DocumentBuilder");
#endif
}


// 
// Parse an XML file and build a DOM tree out of it
//
Ptr<XML::Document> DOMBuilder::ParseFile(const char* pfilename, 
                                                      FileOpenerBase* pfo, 
                                                      Ptr<XML::ObjectManager> objMgr /* = NULL */)
{
    // reset internal vars
    bError = false;
    TotalBytesToLoad = 0;
    LoadedBytes = 0;

    if (NULL == objMgr.GetPtr())
        objMgr = *new XML::ObjectManager;

    // create new document
    pDoc = *objMgr->CreateDocument();

    // commence parsing
    if (pXMLParserState)
        bError = !pXMLParserState->ParseFile(pfilename, pfo, this);
    // NOTE: The locator could be invalid at this point
    
    // Release internal reference to document
    Ptr<XML::Document> pret = pDoc;
    pDoc = 0;

    if (pret.GetPtr() && bIgnoreWhitespace)
        DropWhiteSpaceNodes(pret);

    return pret;
}

// 
// Parse an XML string and build a DOM tree out of it
//
Ptr<XML::Document> DOMBuilder::ParseString(const char* pdata, UPInt len, 
                                                        Ptr<XML::ObjectManager> objMgr /* = NULL */)
{
    // Reset internal vars
    bError = false;
    TotalBytesToLoad = 0;
    LoadedBytes = 0;

    if (NULL == objMgr.GetPtr())
        objMgr = *new XML::ObjectManager;

    // Create new document
    pDoc = *objMgr->CreateDocument();

    // Commence parsing
    if (pXMLParserState)
        bError = !pXMLParserState->ParseString(pdata, len, this);
    // NOTE: The locator could be invalid at this point

    // Release internal reference to document
    Ptr<XML::Document> pret = pDoc;
    pDoc = 0;

    if (pret.GetPtr() && bIgnoreWhitespace)
        DropWhiteSpaceNodes(pret);

    return pret;
}

// 
// Document begin reported by XML parser
//
void DOMBuilder::StartDocument()
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    SF_DEBUG_MESSAGE(1, "XML::DocumentBuilder::StartDocument");
#endif

    // Create a parse stack      
    ParseStack.PushBack(pDoc);

    // Copy the total number of bytes to load
    TotalBytesToLoad = pLocator->TotalBytesToLoad;
}


// 
// Document end reported by XML parser
//
void DOMBuilder::EndDocument()
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    char db1[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(pLocator->XMLVersion.ToCStr(), 
        pLocator->XMLVersion.GetSize(),
        db0, MAX_DEBUG_STRING_LENGTH);
    SafeStringCopyWithSentinel(pLocator->Encoding.ToCStr(), 
        pLocator->Encoding.GetSize(),
        db1, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, 
        "XML::DocumentBuilder::EndDocument('%.16s', '%.16s', %d)",
        db0, db1, pLocator->StandAlone);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    // Check parse stack
    SF_ASSERT(ParseStack.GetSize() == 1);
    SF_ASSERT(ParseStack[0].GetPtr() == pDoc.GetPtr());

    // Drop reference to document
    ParseStack.Clear();

    // Check prefix ownership stacks
    SF_ASSERT(PrefixNamespaceStack.GetSize() == 0);
    SF_ASSERT(DefaultNamespaceStack.GetSize() == 0);

    // Set the xml declaration values
    Ptr<XML::ObjectManager> memMgr = pDoc->MemoryManager;
    pDoc->XMLVersion = memMgr->CreateString(pLocator->XMLVersion.ToCStr(),
        pLocator->XMLVersion.GetSize());
    pDoc->Encoding = memMgr->CreateString(pLocator->Encoding.ToCStr(),
        pLocator->Encoding.GetSize());
    pDoc->Standalone = (SByte)pLocator->StandAlone;

#ifdef SF_XML_DOCBUILDER_DOM_TREE_DUMP
    PrintDOMTree(pDoc);
#endif
}


// 
// Tag element begin reported by XML parser
//
void DOMBuilder::StartElement(const XML::StringRef& prefix,
                                  const XML::StringRef& localname,
                                  const XML::ParserAttributes& atts)
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    char db1[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(prefix.ToCStr(), prefix.GetSize(), 
        db0, MAX_DEBUG_STRING_LENGTH);
    SafeStringCopyWithSentinel(localname.ToCStr(), localname.GetSize(), 
        db1, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE4(1, 
        "[%d, %d] XML::DocumentBuilder::StartElement('%.16s', '%.16s')", 
        pLocator->Line, pLocator->Column, db0, db1);
    for (UPInt i=0; i < atts.Length; i++)
    {
        XML::ParserAttribute& att = atts.Attributes[i];
        SafeStringCopyWithSentinel(att.Name.ToCStr(), att.Name.GetSize(), 
            db0, MAX_DEBUG_STRING_LENGTH);
        SafeStringCopyWithSentinel(att.Value.ToCStr(), att.Value.GetSize(), 
            db1, MAX_DEBUG_STRING_LENGTH);
        SF_DEBUG_MESSAGE2(1, "\tATTR: '%.16s' -> '%.16s'", db0, db1);
    }
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    // Must have at least root node in stack
    SF_ASSERT(ParseStack.GetSize() > 0);

    Ptr<XML::ObjectManager> memMgr = pDoc->MemoryManager;

    // If appending required, consolidate it
    if (pAppendChainRoot.GetPtr() != NULL)
    {
        Ptr<XML::ElementNode> pnode = ParseStack.Back();
        pnode->AppendChild(pAppendChainRoot);
        pAppendChainRoot->Value = memMgr->CreateString(AppendText.ToCStr(), AppendText.GetSize());
        // Reset append chain
        pAppendChainRoot = NULL;
        AppendText.Clear();
    }

    // Create new node
    Ptr<XML::ElementNode> elemNode = *memMgr->CreateElementNode(
        memMgr->CreateString(localname.ToCStr(), localname.GetSize()));

    // Add attributes
    for (UPInt i=0; i < atts.Length; i++)
    {
        XML::ParserAttribute& att = atts.Attributes[i];
        XML::Attribute* patt = memMgr->CreateAttribute(
            memMgr->CreateString(att.Name.ToCStr(), att.Name.GetSize()),
            memMgr->CreateString(att.Value.ToCStr(), att.Value.GetSize()));
        elemNode->AddAttribute(patt);
    }

    // Assign ownership to any floating namespaces
    if (PrefixNamespaceStack.GetSize() > 0)
    {
        UPInt len = PrefixNamespaceStack.GetSize();
        for (int i=(int)len-1; i >= 0; i--)
        {
            PrefixOwnership& po = PrefixNamespaceStack[i];
            if (NULL != po.Owner.GetPtr())
                break;
            po.Owner = elemNode;
        }
    }
    if (DefaultNamespaceStack.GetSize() > 0)
    {
        PrefixOwnership& po = DefaultNamespaceStack.Back();
        if (NULL == po.Owner.GetPtr())
            po.Owner = elemNode;
    }

    // Assign appropriate namespace to node
    if (prefix.GetSize() > 0)
    {
        bool found = false;
        // Assign prefix namespace - it has higher precedence
        // TODO: change to hash lookup?
        if (PrefixNamespaceStack.GetSize() > 0)
        {
            UPInt len = PrefixNamespaceStack.GetSize();
            for (int i=(int)len-1; i >= 0; i--)
            {
                PrefixOwnership po = PrefixNamespaceStack[i];
                if (po.mPrefix->Name == 
                    XML::DOMStringCompareWrapper(prefix.ToCStr(), prefix.GetSize()))
                {
                    found = true;
                    elemNode->Prefix = po.mPrefix->Name;
                    elemNode->Namespace = po.mPrefix->Value;
                    break;
                }
            }
        }
        if (!found)
        {
            // Prefix was not found. create a dummy one.
            elemNode->Prefix = memMgr->CreateString(prefix.ToCStr(), 
                prefix.GetSize());
            elemNode->Namespace = memMgr->EmptyString();
        }
    }
    else if (DefaultNamespaceStack.GetSize() > 0)
    {
        // Assign default namespace
        PrefixOwnership os = DefaultNamespaceStack.Back();
        elemNode->Prefix = os.mPrefix->Name;
        elemNode->Namespace = os.mPrefix->Value;
    }

    // Attach to parent
    Ptr<XML::ElementNode> parentNode = ParseStack.Back();
    parentNode->AppendChild(elemNode);

    // Push to parse stack
    ParseStack.PushBack(elemNode);
}


// 
// Tag element end reported by XML parser
//
void DOMBuilder::EndElement(const XML::StringRef& prefix,
                                const XML::StringRef& localname)
{
    SF_UNUSED2(prefix, localname);

#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    char db1[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(prefix.ToCStr(), prefix.GetSize(), db0, 
        MAX_DEBUG_STRING_LENGTH);
    SafeStringCopyWithSentinel(localname.ToCStr(), localname.GetSize(), 
        db1, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE4(1, 
        "[%d, %d] XML::DocumentBuilder::EndElement('%.16s', '%.16s')", 
        pLocator->Line, pLocator->Column, db0, db1);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    SF_ASSERT(ParseStack.GetSize() > 1); // must be at least root node + 1

    Ptr<XML::ElementNode> pnode = ParseStack.Back();
    SF_ASSERT(pnode.GetPtr() != pDoc.GetPtr());    // must not be root node    
   
    // If appending required, consolidate it
    if (pAppendChainRoot.GetPtr() != NULL)
    {
        Ptr<XML::ObjectManager> memMgr = pDoc->MemoryManager;
        pnode->AppendChild(pAppendChainRoot);
        // Accumulate append text while removing the nodes
        pAppendChainRoot->Value = memMgr->CreateString(AppendText.ToCStr(), AppendText.GetSize());
        // Reset append chain
        pAppendChainRoot = NULL;
        AppendText.Clear();
    }
    
    // Remove namespace ownership
    // A top-down traversal of the namespace stacks is guaranteed to 
    // return any owned namespaces. if not found, no ownership.
    if (PrefixNamespaceStack.GetSize() > 0)
    {
        UPInt len = PrefixNamespaceStack.GetSize();
        for (int i=(int)len-1; i >= 0; i--)
        {
            PrefixOwnership po = PrefixNamespaceStack[i];
            if (po.Owner != pnode)
                break;
            PrefixNamespaceStack.PopBack();
        }
    }
    if (DefaultNamespaceStack.GetSize() > 0)
    {
        PrefixOwnership po = DefaultNamespaceStack.Back();
        if (po.Owner == pnode)
            DefaultNamespaceStack.PopBack();
    }

    // Pop stack
    ParseStack.PopBack();
}


// 
// Namespace mapping reported by XML parser
//
// Next tag element will define the namespace scope.
//
void DOMBuilder::PrefixMapping(const XML::StringRef& prefix, 
                                          const XML::StringRef& uri)
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    char db1[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(prefix.ToCStr(), prefix.GetSize(), 
        db0, MAX_DEBUG_STRING_LENGTH);
    SafeStringCopyWithSentinel(uri.ToCStr(), uri.GetSize(), db1, 
        MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE4(1, 
        "[%d, %d] XML::DocumentBuilder::StartPrefixMapping('%.16s', '%.16s')", 
        pLocator->Line, pLocator->Column, db0, db1);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    Ptr<XML::ObjectManager> memMgr = pDoc->MemoryManager;

    Ptr<XML::Prefix> pPfxObj = *memMgr->CreatePrefix(
        memMgr->CreateString(prefix.ToCStr(), prefix.GetSize()),
        memMgr->CreateString(uri.ToCStr(), uri.GetSize()));

    // Add prefix namespaces to prefix stack
    if (prefix.GetSize() > 0)
    {
        // Owner will be set when next start element is encountered.
        PrefixNamespaceStack.PushBack(PrefixOwnership(pPfxObj, NULL));
    }
    // Add default namespaces to dns stack
    else
    {
        // Why is this in a separate stack?
        // Elements without a prefix may inherit a default namespace
        // from any of its ancestors. keeping it in a separate stack
        // makes lookup Theta(1).
        // Owner will be set when next start element is encountered.
        DefaultNamespaceStack.PushBack(PrefixOwnership(pPfxObj, NULL));
    }
}


// 
// Text data reported by XML parser
//
void DOMBuilder::Characters(const XML::StringRef& text)
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(text.ToCStr(), text.GetSize(), db0, 
        MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, 
        "[%d, %d] XML::DocumentBuilder::Characters('%.16s')", 
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    SF_ASSERT(ParseStack.GetSize() > 0);

    Ptr<XML::ObjectManager> memMgr = pDoc->MemoryManager;

    if (pAppendChainRoot.GetPtr() == NULL)
    {
        pAppendChainRoot = *memMgr->CreateTextNode(memMgr->EmptyString());
        AppendText.AppendString(text.ToCStr(), text.GetSize());
    }
    else
    {
        AppendText.AppendString(text.ToCStr(), text.GetSize());
    }
}


// 
// Ignored whitespace reported by XML parser
//
void DOMBuilder::IgnorableWhitespace(const XML::StringRef& ws)
{
    SF_UNUSED(ws);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(ws.ToCStr(), ws.GetSize(), db0, 
        MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, 
        "[%d, %d] XML::DocumentBuilder::IgnorableWhitespace('%.16s')", 
        pLocator->Line, pLocator->Column, db0);
#endif

    // copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    // Do nothing
}


// 
// Skipped entity reported by XML parser
//
void DOMBuilder::SkippedEntity(const XML::StringRef& name)
{
    SF_UNUSED(name);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(name.ToCStr(), name.GetSize(), 
        db0, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, 
        "[%d, %d] XML::DocumentBuilder::SkippedEntity('%.16s')", 
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    // Do nothing
}


// 
// Document locater registration
//
void DOMBuilder::SetDocumentLocator(const XML::ParserLocator* plocator)
{
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_MESSAGE(1, "XML::DocumentBuilder::SetDocumentLocator");
#endif

    pLocator = plocator;
}


// 
// Comment found by XML parser
//
void DOMBuilder::Comment(const XML::StringRef& text)
{
    SF_UNUSED(text);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(text.ToCStr(), text.GetSize(), db0, 
        MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, "[%d, %d] XML::DocumentBuilder::Comment('%.16s')", 
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    // Do nothing
}


// 
// Error reported by XML parser
//
void DOMBuilder::Error(const XML::ParserException& exception)
{
    SF_UNUSED(exception);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(exception.ErrorMessage.ToCStr(), 
        exception.ErrorMessage.GetSize(), db0, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, "[%d, %d] XML::DocumentBuilder::Error('%.64s')",
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    SF_DEBUG_ERROR3(1, "Document builder @ line:%d col:%d - %.64s\n",
        pLocator->Line,
        pLocator->Column,
        exception.ErrorMessage.ToCStr());

    bError = true;
}


// 
// Fatal error reported by XML parser
//
void DOMBuilder::FatalError(const XML::ParserException& exception)
{
    SF_UNUSED(exception);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(exception.ErrorMessage.ToCStr(), 
        exception.ErrorMessage.GetSize(), db0, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, "[%d, %d] XML::DocumentBuilder::FatalError('%.64s')",
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    SF_DEBUG_ERROR3(1, "Document builder @ line:%d col:%d - %.64s\n",
        pLocator->Line,
        pLocator->Column,
        exception.ErrorMessage.ToCStr());

    bError = true;
}


// 
// Warning reported by XML parser
//
void DOMBuilder::Warning(const XML::ParserException& exception)
{
    SF_UNUSED(exception);
#ifdef SF_XML_DOCBUILDER_TRACE
    SF_DEBUG_ASSERT(pLocator, "Locator not set!");
    char db0[MAX_DEBUG_STRING_LENGTH];
    SafeStringCopyWithSentinel(exception.ErrorMessage.ToCStr(), 
        exception.ErrorMessage.GetSize(), db0, MAX_DEBUG_STRING_LENGTH);
    SF_DEBUG_MESSAGE3(1, "[%d, %d] XML::DocumentBuilder::Warning('%.64s')",
        pLocator->Line, pLocator->Column, db0);
#endif

    // Copy the number of bytes loaded
    LoadedBytes = pLocator->LoadedBytes;

    SF_DEBUG_WARNING3(1, "Document builder @ line:%d col:%d - %.64s\n",
        pLocator->Line,
        pLocator->Column,
        exception.ErrorMessage.ToCStr());
}


//
// Return true if the text node only contains whitespace
//
bool CheckWhiteSpaceNode(XML::TextNode* textNode)
{
    const char* buffer = textNode->Value.GetBuffer();
    bool allws = true;
    UInt32 utf8char;
    while ( (utf8char = UTF8Util::DecodeNextChar(&buffer)) != 0 )
    {
        if (!SFiswspace((wchar_t)utf8char))
        {
            allws = false;
            break;
        }
    }
    return allws;
}

//
// Helper function for DOM traversal
//
void DropWhiteSpaceNodesHelper(XML::ElementNode* elemNode) 
{
    XML::Node* child = elemNode->FirstChild;
    XML::Node* temp = child;
    while (child != NULL)
    {
        temp = child->NextSibling;
        if (child->Type == XML::ElementNodeType)
            DropWhiteSpaceNodesHelper((XML::ElementNode*)child);
        else if (child->Type == XML::TextNodeType)
        {
            if (CheckWhiteSpaceNode((XML::TextNode*)child))
            {
                // Remove it
                elemNode->RemoveChild(child);
            }
        }
        child = temp;
    }
}


// 
// Drop all text nodes containing only whitespace from the DOM tree.
// This emulates the behavior of not processing unneccessary whitespace 
// in the XML document.
//
void DOMBuilder::DropWhiteSpaceNodes(XML::Document* document)
{
    for (XML::Node* child = document->FirstChild; child != NULL; 
        child = child->NextSibling)
    {
        if (child->Type == XML::ElementNodeType)
            DropWhiteSpaceNodesHelper((XML::ElementNode*)child);
    }
}


}}} //namespace SF::GFx::XML


#endif // #ifdef GFX_ENABLE_XML 
