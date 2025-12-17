/**************************************************************************

PublicHeader:   None
Filename    :   XML_Parser.h
Content     :   SAX2 compliant interface
Created     :   February 21, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_XMLParser_H
#define INC_SF_GFX_XMLParser_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform { namespace GFx { namespace XML {

//
// The classes and structs used by SAX2 parser implementations exist
// here. They define a common interface between the parser implementation
// and the DOM builder. Parsers should invoke the callbacks in the DOM 
// builder interface declared below for appropriate parse events.
//


// 
// Wrapper for a UTF8 string passed to the DOM builder
//
// The data should be managed by the parser. 
//
typedef StringDataPtr StringRef;


// 
// Wrapper for an XML attribute/s passed to the DOM builder
//
struct ParserAttribute
{
    // Attribute name
    StringRef      Name;

    // Attribute value
    StringRef      Value;
};

struct ParserAttributes
{
    // Contiguous array of XMLParserAttribute objects
    ParserAttribute*      Attributes;

    // Number of objects in the array
    UPInt                       Length;
};


// 
// SAX2 Locator2 Object
//
// Register a locator object with the DOM builder before parsing occurs.
// The parser is expected to fill in the locator object values 
// appropriately, since the DOM builder uses the most recent values
// when processing the tree.
//
struct ParserLocator
{
    int                 Column;
    int                 Line;

    // XML declaration 
    StringRef     Encoding;
    StringRef     XMLVersion;
    int                 StandAlone;

    UPInt               TotalBytesToLoad;
    UPInt               LoadedBytes;

    ParserLocator() : StandAlone(-1) {}
};


// 
// SAX2 ParserException Object
//
// When an error occurs in the parser, pass an instance of this class
// to the DOM builder.
//
struct ParserException
{
    // The error description
    StringRef      ErrorMessage;

    ParserException(const StringRef& error)
        : ErrorMessage(error) {}
};


// 
// SAX2 Consolidated Handler
//
// The DOM builder is an interface similar to a SAX2 parser handler. 
// The parser implementation is expected to call the appropriate callback
// method for certain events.
//
//
class ParserHandler : public RefCountBase<ParserHandler, StatMV_XML_Mem>
{
public:
    ParserHandler() { }
    virtual ~ParserHandler() {}

    //
    // Beginning and end of documents
    //
    virtual void        StartDocument() = 0;
    virtual void        EndDocument() = 0;

    //
    // Start and end of a tag element
    //
    virtual void        StartElement(const StringRef& prefix,
                                     const StringRef& localname,
                                     const ParserAttributes& atts) = 0;
    virtual void        EndElement(const StringRef& prefix,
                                   const StringRef& localname) = 0;

    //
    // Namespace declaration. Next element will be the parent of the mappings
    //
    virtual void        PrefixMapping(const StringRef& prefix, 
                                      const StringRef& uri) = 0;
    //
    // Text data, in between tag elements
    //
    virtual void        Characters(const StringRef& text) = 0;

    //
    // Whitespace
    //
    virtual void        IgnorableWhitespace(const StringRef& ws) = 0;

    //
    // Unprocessed elements
    //
    virtual void        SkippedEntity(const StringRef& name) = 0;

    //
    // Parser implementors are REQURIED to set a document locator
    // before any callbacks occur. ParserHandler implementations
    // require a locator object for error reporting and correctly processing
    // the encoding, xmlversion and standalone properties.
    //
    virtual void        SetDocumentLocator(const ParserLocator* plocator) = 0;

    //
    // Comments
    //
    virtual void        Comment(const StringRef& text) = 0;

    // ErrorHandler Callbacks
    virtual void        Error(const ParserException& exception) = 0;
    virtual void        FatalError(const ParserException& exception) = 0;
    virtual void        Warning(const ParserException& exception) = 0;

};

}}} //SF::GFx::XML

#endif  // #ifdef GFX_ENABLE_XML

#endif  // INC_SF_GFX_XMLParser_H
