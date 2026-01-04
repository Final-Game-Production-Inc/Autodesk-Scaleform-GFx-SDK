/**************************************************************************

PublicHeader:   XML
Filename    :   XML_Expat.cpp
Content     :   XML Parser implementation using expat
Created     :   March 14, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "Kernel/SF_File.h"
#include "Kernel/SF_MsgFormat.h"
#include "XML/XML_Expat.h"

//
// Maximum number of attributes that will be stored on the stack. This 
// value is used when storing attribute values in a parser handler 
// friendly object format. If there are more attrinutes than the following
// value, then a container is created on the heap and that container is
// then sent to the parser handler. 
//
#define MAX_ATTRIBUTES_ON_STACK 32


// Expat related heders
#define XML_STATIC
#include <expat.h>

namespace Scaleform {
namespace GFx {
namespace XML {

//
// User defined parameter for expat callbacks. This structure is used
// to wrap objects required during expat's parsing loop.
//
struct ExpatHandlerArg
{
    XML_Parser              pParser;
    ParserHandler*       pParserHandler;
    ParserLocator*    pLocator;
};


//
// Expat callback handler implementation
//
class ExpatCallbackHandler
{
public:
    //
    // Expat callback handlers
    //
    static void     XmlDeclExpatCallback(void *userData, 
                                         const XML_Char *version, 
                                         const XML_Char *encoding, 
                                         int standalone);
    static void     StartElementExpatCallback(void *userData, 
                                              const XML_Char *name, 
                                              const XML_Char **atts);
    static void     EndElementExpatCallback(void *userData, 
                                            const XML_Char *name);
    static void     CharacterDataExpatCallback(void *userData, 
                                               const XML_Char *s, 
                                               int len);
    static void     CommentExpatCallback(void *userData, 
                                         const XML_Char* data);
    static void     DefaultExpatCallback(void *userData, 
                                         const XML_Char* s, 
                                         int len);
    //
    // Helper function to fill the locator object
    //
    static void     FillLocator(ExpatHandlerArg* pha);
};


// 
// Parse a file and direct callbacks to the provided handler
//
bool ParserExpat::ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                                  ParserHandler* pparseHandler)
{
    char errbuf[256];

    // Initialize
    bool bsuccess = false;
    ParserLocator locator;
    locator.TotalBytesToLoad = 0;
    locator.LoadedBytes = 0;

    SF_ASSERT(pfo);    
    Ptr<File> pfile = *pfo->OpenFile(pfilename);
    if (pfile && pfile->IsValid())
    {
        int flen = 0;
        if ((flen = pfile->GetLength()) != 0)
        {
            // Load the file into memory
            UByte* td = (UByte*) SF_ALLOC(flen, StatMV_XML_Mem);
            pfile->Read(td, flen);
            
            // Delegate to ParseString
            bsuccess = ParseString((const char*)td, flen, pparseHandler);
        
            // Cleanup
            SF_FREE(td);
        }
        else
        {
            // File was empty
            pparseHandler->SetDocumentLocator(&locator);            
            Format(StringDataPtr(errbuf, sizeof(errbuf)), "File {0} is empty!", pfilename);
            ParserException e(errbuf);
            pparseHandler->Warning(e);
        }
    }
    else
    {
        // File not found
        pparseHandler->SetDocumentLocator(&locator);
        Format(StringDataPtr(errbuf, sizeof(errbuf)), "File {0} does not exist!", pfilename);
        ParserException e(errbuf);
        pparseHandler->FatalError(e);
    }
    return bsuccess;
}


// 
// Parse a string and direct callbacks to the provided handler
//
bool ParserExpat::ParseString(const char* pdata, UPInt len, 
                                    ParserHandler* pparseHandler)
{
    // Initialize
    ParserLocator locator;
    locator.TotalBytesToLoad = len;
    locator.LoadedBytes = 0;

    // Register the locator object with the parse handler
    pparseHandler->SetDocumentLocator(&locator);

    // Create a new Expat xml parser
    XML_Parser parser = XML_ParserCreate(NULL);

    // Set up Expat callbacks
    XML_SetStartElementHandler(parser, ExpatCallbackHandler::StartElementExpatCallback);
    XML_SetEndElementHandler(parser, ExpatCallbackHandler::EndElementExpatCallback);
    XML_SetCharacterDataHandler(parser, ExpatCallbackHandler::CharacterDataExpatCallback);
    XML_SetCommentHandler(parser, ExpatCallbackHandler::CommentExpatCallback);
    XML_SetXmlDeclHandler(parser, ExpatCallbackHandler::XmlDeclExpatCallback);
    XML_SetDefaultHandler(parser, ExpatCallbackHandler::DefaultExpatCallback);

    // Create the user data object
    ExpatHandlerArg harg = {parser, pparseHandler, &locator};
    XML_SetUserData(parser, &harg);

    // Parse
    bool berror = false;
    pparseHandler->StartDocument();
    int sz = (int)len;
    if (XML_Parse(parser, pdata, sz, true) != XML_STATUS_OK)
    {
        // Parse error encountered
        ExpatCallbackHandler::FillLocator(&harg);

        XML_Error ecode = XML_GetErrorCode(parser);
        //
        // Expat <-> AS 2.0 error code mapping (not implemented due to many inconsistencies)
        //
        // XML_ERROR_NONE           = 0
        // XML_ERROR_INVALID_TOKEN  = 0 (ie: XML.parse("http://www.google.com"))
        // XML_ERROR_UNCLOSED_CDATA_SECTION = -2
        // XML_ERROR_UNCLOSED_TOKEN = -3
        // XML_ERROR_INVALID_TOKEN  = -4
        // XML_ERROR_INVALID_TOKEN  = -5
        // XML_ERROR_UNCLOSED_TOKEN = -8
        // XML_ERROR_NO_ELEMENTS    = -9
        // XML_ERROR_TAG_MISMATCH   = -10
        //
        ParserException e(XML_ErrorString(ecode));
        pparseHandler->FatalError(e);
        berror = true;
    }
    else
    {
        // No errors
        ExpatCallbackHandler::FillLocator(&harg);
        pparseHandler->EndDocument();
    }

    // Cleanup
    XML_ParserFree(parser);

    return !berror;
}


// 
// Expat callback for beginning tags
//
void ExpatCallbackHandler::StartElementExpatCallback(void *userData, 
                                                  const XML_Char *name, 
                                                  const XML_Char **atts)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    ParserAttribute astack[MAX_ATTRIBUTES_ON_STACK];
    ParserAttributes attlist;

    // Determine number of attributes
    UPInt sz = 0;
    bool attsInHeap = false;
    for (int i = 0; atts[i]; i += 2, sz++) {}
    attlist.Length = sz;

    // Allocate attribute list if number of attributes
    // cannot fit in stack array
    if (sz > MAX_ATTRIBUTES_ON_STACK)
    {
        attlist.Attributes = new ParserAttribute[sz];
        attsInHeap = true;
    }
    else
    {
        attlist.Attributes = astack;
    }

    // Fill attributes
    int aidx = 0;
    for (UPInt i = 0; atts[i]; i += 2, aidx++) 
    {
        attlist.Attributes[aidx].Name = StringRef(atts[i]);
        attlist.Attributes[aidx].Value = StringRef(atts[i+1]);

        // Look for namespace declarations
        const XML_Char* att = atts[i];
        if ( (att[0] == 'x') && (att[1] == 'm') && (att[2] == 'l') &&
             (att[3] == 'n') && (att[4] == 's') )
        {
            if (att[5] == ':')
            {
                // prefix namespace
                pha->pParserHandler->PrefixMapping(&att[6], atts[i+1]);
            }
            else
            {
                // default namespace?
                if (SFstrlen(att) == 5)
                {
                    pha->pParserHandler->PrefixMapping(StringRef(), 
                        atts[i+1]);
                }
            }
        }
    }

    // Extract prefix, localname
    StringRef prefix;
    StringRef localname;
    const char* delim = strchr(name, ':');
    if (delim == NULL)
    {
        localname = StringRef(name);
    }
    else
    {
        prefix = StringRef(name, delim-name);
        localname = StringRef(delim+1);
    }

    FillLocator(pha);
    pha->pParserHandler->StartElement(prefix, localname, attlist);

    // Cleanup
    if (attsInHeap)
    {
        delete[] attlist.Attributes;
    }
}


// 
// Expat callback for closing tags
//
void ExpatCallbackHandler::EndElementExpatCallback(void *userData, 
                                                const XML_Char *name)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    // Extract prefix, localname
    StringRef prefix;
    StringRef localname;
    const char* delim = strchr(name, ':');
    if (delim == NULL)
    {
        localname = StringRef(name);
    }
    else
    {
        prefix = StringRef(name, delim-name);
        localname = StringRef(delim+1);
    }

    FillLocator(pha);
    pha->pParserHandler->EndElement(prefix, localname);
}


// 
// Expat callback for text data between tag elements
//
void ExpatCallbackHandler::CharacterDataExpatCallback(void *userData, 
                                                   const XML_Char *s, 
                                                   int len)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    StringRef text(s, len);

    FillLocator(pha);
    pha->pParserHandler->Characters(text);
}


// 
// Expat callback for comments
//
void ExpatCallbackHandler::CommentExpatCallback(void *userData, 
                                             const XML_Char* data)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    StringRef comment(data);

    FillLocator(pha);
    pha->pParserHandler->Comment(comment);    
}


// 
// Expat callback for the xml declaration
//
void ExpatCallbackHandler::XmlDeclExpatCallback(void *userData, 
                                             const XML_Char *version, 
                                             const XML_Char *encoding, 
                                             int standalone)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    // No callback delegation. Fill in the locator (which stores the xmlDecl)
    ParserLocator* pLocator = pha->pLocator;
    pLocator->XMLVersion = StringRef(version);
    pLocator->Encoding = StringRef(encoding);
    pLocator->StandAlone = standalone;
}


// 
// Expat callback for unprocessed entities.
//
void ExpatCallbackHandler::DefaultExpatCallback(void *userData, 
                                             const XML_Char* s, 
                                             int len)
{
    SF_DEBUG_ASSERT(userData, "userData is NULL!");
    ExpatHandlerArg* pha = (ExpatHandlerArg*)userData;    

    StringRef unk(s, len);

    FillLocator(pha);
    pha->pParserHandler->SkippedEntity(unk);
}


// 
// Fill the locator object with column and line information.
//
void ExpatCallbackHandler::FillLocator(ExpatHandlerArg* pha)
{
    ParserLocator* pLocator = pha->pLocator;
    XML_Parser pParser = pha->pParser;

    // Column is an offset from begining of line (0 based)
    // For readability, increment by 1 to make it a true
    // column number (1 based)
    pLocator->Column = XML_GetCurrentColumnNumber(pParser) + 1;
    pLocator->Line = XML_GetCurrentLineNumber(pParser);
    pLocator->LoadedBytes = XML_GetCurrentByteIndex(pParser);

}

}}} //SF::GFx::XML

#endif // #ifndef GFX_ENABLE_XML 
