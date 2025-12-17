/**************************************************************************

Filename    :   AS2_Xml.cpp
Content     :   XML reference class for ActionScript 2.0
Created     :   3/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/GFx_PlayerImpl.h"
#include "Kernel/SF_File.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/XML/XML_Support.h"
#include "GFx/XML/XML_DOM.h"
#include "GFx/AS2/XML/AS2_XMLShadowRef.h"
#include "GFx/AS2/XML/AS2_Xml.h"
#include "Kernel/SF_HeapNew.h"


//
// XML object's file loader implementations
// An instance of this object is sent to the GFx load queue for
// processing. It is thread safe, and therefore allows files
// to be loaded and processed asynchronously.
//

namespace Scaleform { namespace GFx { namespace AS2 {

//
// This object performs both file loading and parsing
// It is used when the default onData handler is still
// registered with the object
//
class XMLFileLoaderAndParserImpl : public XMLFileLoader
{
    Ptr<XML::SupportBase>     pParser;
    XML::ObjectManager*        pObjectManager;

    UByte*                      pFileData;
    int                         FileLength;

    bool                        bIgnoreWhitespace;

public:
    XMLFileLoaderAndParserImpl(XML::SupportBase* pparser, 
        XML::ObjectManager* objMgr, 
        bool ignorews = false);
    virtual         ~XMLFileLoaderAndParserImpl();

    //
    // File loader implementation
    //
    void            Load(const String& filename, FileOpener* pfo);
    void            InitASXml(Environment* penv, Object* pTarget);
};


//
// AS XML file loader that loads and parses the file
// 

XMLFileLoaderAndParserImpl::XMLFileLoaderAndParserImpl(XML::SupportBase* pparser, 
                                                             XML::ObjectManager* objMgr, 
                                                             bool ignorews) 
                                                             : pParser(pparser), pObjectManager(objMgr), 
                                                             pFileData(NULL), FileLength(0), bIgnoreWhitespace(ignorews)
{}

XMLFileLoaderAndParserImpl::~XMLFileLoaderAndParserImpl()
{
    if (pFileData != NULL)
        SF_FREE(pFileData);
}

//
// Loads and parses the file and returns a XML::Document object
//
void XMLFileLoaderAndParserImpl::Load( const String& filename, FileOpener* pfo )
{
    // Could be on a seperate thread here if thread support is enabled.

    Ptr<File> pFile;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(filename))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, filename) && !bytes.IsEmpty())
        {
            pFile = *SF_NEW MemoryFile(filename, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        pFile = *pfo->OpenFile(filename);
    }
    if (pFile && pFile->IsValid())
    {
        if ((FileLength = pFile->GetLength()) != 0)
        {
            // Load the file into memory
            pFileData = (UByte*) SF_ALLOC(FileLength, StatMV_XML_Mem);
            pFile->Read(pFileData, FileLength);
        }
    }
}

//
// Set up the shadow/realnode pointers and call the onLoad handler
//
void XMLFileLoaderAndParserImpl::InitASXml(Environment* penv, Object* pTarget)
{
    // Back on main thread at this point. Check if target is ok.
    XmlObject* pASObj = static_cast<XmlObject*>(pTarget);

    if ( !pFileData )
    {
        // File loading error
        pASObj->SetLoadedBytes(-1, 0);

        pASObj->NotifyOnLoad(penv, false);
    }
    else
    {
        XML::DOMBuilder documentBuilder(pParser, bIgnoreWhitespace);
        Ptr<XML::Document> ploadedDoc = documentBuilder.ParseString(
            (const char*)pFileData, FileLength, pObjectManager);

        // Cleanup
        SF_FREE(pFileData);
        pFileData = NULL;

        pASObj->pRealNode = ploadedDoc;
        pASObj->pRootNode = *pObjectManager->CreateRootNode(ploadedDoc);
        XMLShadowRef* pshadow = SF_HEAP_NEW(pObjectManager->GetHeap()) XMLShadowRef();
        pshadow->pAttributes = NULL;
        pshadow->pASNode = pASObj;
        ploadedDoc->pShadow = pshadow;

        pASObj->AssignXMLDecl(penv, ploadedDoc);

        if (documentBuilder.IsError() && 
            documentBuilder.GetTotalBytesToLoad() == 0)
        {
            // File parsing error
            pASObj->SetLoadedBytes(-1, (Number)documentBuilder.GetLoadedBytes());

            pASObj->NotifyOnLoad(penv, false);
        }
        else
        {
            // File was parsed OK
            // Set the loaded property to true
            pASObj->SetMemberRaw(penv->GetSC(), 
                penv->GetSC()->CreateConstString("loaded"), true, 
                PropFlags::PropFlag_DontDelete);
            pASObj->SetLoadedBytes((Number)documentBuilder.GetTotalBytesToLoad(), 
                (Number)documentBuilder.GetLoadedBytes());
            pASObj->NotifyOnLoad(penv, true);
        }
    }
}


//
// This object is just for file loading
// It is used when a custom onData handler is registered
// with the XML object
//
class XMLFileLoaderImpl : public XMLFileLoader
{
    UByte*                      pFileData;
    int                         FileLength;

public:
    XMLFileLoaderImpl();
    virtual         ~XMLFileLoaderImpl();

    //
    // File loader implementation
    //
    void Load(const String& filename, FileOpener* pfo);
    void            InitASXml(Environment* penv, Object* pTarget);
};

//
// AS XML file loader that simply loads the file
// 
XMLFileLoaderImpl::XMLFileLoaderImpl()
: pFileData(NULL), FileLength(0)
{}

//
// Destructor
//
XMLFileLoaderImpl::~XMLFileLoaderImpl()
{
    if (pFileData != NULL)
        SF_FREE(pFileData);
}

//
// Open the file
//
void XMLFileLoaderImpl::Load( const String& filename, FileOpener* pfo )
{
    // Could be on a seperate thread here if thread support is enabled.

    Ptr<File> pFile;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(filename))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, filename) && !bytes.IsEmpty())
        {
            pFile = *SF_NEW MemoryFile(filename, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        pFile = *pfo->OpenFile(filename);
    }
    if (pFile && pFile->IsValid())
    {
        if ((FileLength = pFile->GetLength()) != 0)
        {
            // Load the file into memory
            pFileData = (UByte*) SF_ALLOC(FileLength, StatMV_XML_Mem);
            pFile->Read(pFileData, FileLength);
        }
    }
}

//
// Load the data to a GASString and call the onData handler
//
void XMLFileLoaderImpl::InitASXml(Environment* penv, Object* pTarget)
{
    // Back on main thread at this point. Check if target is ok.
    XmlObject* pASObj = static_cast<XmlObject*>(pTarget);

    if (pFileData)
    {
        // Create a GASString and notify object
        pASObj->NotifyOnData(penv, 
            penv->CreateString((const char*)pFileData, FileLength));

        // Cleanup
        SF_FREE(pFileData);
        pFileData = NULL;
    }
    else 
    {
        // error
        pASObj->NotifyOnData(penv, Value::UNDEFINED);
    }
}


// ********************************************************************

//
// Helper function to parse the first parameter (a string) and
// create a dom tree out of it
//
static void XML_LoadString(const FnCall& fn, XmlObject* pnode)
{
    Log* log = fn.GetLog();

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

    Ptr<XML::Document> pdoc = NULL;
    Value text = Value::UNDEFINED;
    if (fn.NArgs > 0)
    {
        text = fn.Arg(0);

        XML::SupportBase* pxmlParser = fn.Env->GetMovieImpl()->GetXMLSupport();
        if (pxmlParser)
        {
            XML::DOMBuilder docb(pxmlParser, true);
            ASString textstr = text.ToString(fn.Env);
            pdoc = docb.ParseString(textstr.ToCStr(), textstr.GetSize(), memMgr);
            pnode->AssignXMLDecl(fn.Env, pdoc);
        }
        else
        {
            if (log != NULL)
                log->LogMessageById(Log_ScriptError, "No XML parser state set for movie!");
        }
    }
    if (NULL == pdoc.GetPtr())
        pdoc = *memMgr->CreateDocument();

    // nodeName is null
    pdoc->Value = memMgr->CreateString("null", 4);

    // Setup shadow fro document
    pnode->pRealNode = pdoc;
    pnode->pRootNode = *memMgr->CreateRootNode(pdoc);
    XMLShadowRef* pshadow = SF_HEAP_NEW(memMgr->GetHeap()) XMLShadowRef();
    pdoc->pShadow = pshadow;
    pshadow->pAttributes = *SF_HEAP_NEW(fn.Env->GetHeap()) XmlNodeObject(fn.Env);
    pshadow->pASNode = static_cast<XmlNodeObject*>(pnode);
}


// 
// Constructor
//
XmlObject::XmlObject(Environment* penv)
:   XmlNodeObject(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_XML));  
    
    BytesLoadedCurrent = -1;    // undefined
    BytesLoadedTotal = -1;      // undefined

    AsBroadcaster::Initialize(penv->GetSC(), this);
    AsBroadcaster::AddListener(penv, this, this);
}


//
// Set the number of bytes to load and the number of bytes to load
// of the current load call
//
void XmlObject::SetLoadedBytes(Number total, Number loaded)
{
    BytesLoadedCurrent = loaded;
    BytesLoadedTotal = total;
}


//
// Helper function to extract the xml declaration string and set it as 
// a member of the AS XML object
//
void XmlObject::AssignXMLDecl(Environment* penv, XML::Document* pdoc)
{
    // <?xml version="1.0" encoding="utf-8" standalone="yes"?>
    StringBuffer xmlDecl = "";
    if (pdoc)
    {
        xmlDecl += "<?"; 
        if (pdoc->XMLVersion.GetSize() > 0)
        {
            xmlDecl += "xml version=\"";
            xmlDecl += pdoc->XMLVersion.ToCStr();
            xmlDecl += "\"";
        }
        if (pdoc->Encoding.GetSize() > 0)
        {
            if (pdoc->XMLVersion.GetSize() > 0)
                xmlDecl += " ";
            xmlDecl += "encoding=\"";
            xmlDecl += pdoc->Encoding.ToCStr();
            xmlDecl += "\"";
        }
        if (pdoc->Standalone != -1)
        {
            if (pdoc->XMLVersion.GetSize() > 0 || pdoc->Encoding.GetSize() > 0)
                xmlDecl += " ";
            if (pdoc->Standalone)
                xmlDecl += "standalone=\"yes\"";
            else
                xmlDecl += "standalone=\"no\"";
        }
        xmlDecl += "?>";
    }
    if (pdoc->XMLVersion.GetSize() > 0 || pdoc->Encoding.GetSize() > 0 || 
        pdoc->Standalone != -1)
    {
        ASStringContext* psc = penv->GetSC();
        SetMember(penv, psc->CreateConstString("xmlDecl"), psc->CreateString(xmlDecl.ToCStr(),xmlDecl.GetSize()));
    }
}


// 
// Callback for XML file load (maybe called from an XML.load invocation)
//
void XmlObject::NotifyOnData(Environment* penv, Value val)
{
    // Flash Doc: Invoked when XML text has been completely downloaded from the 
    // server, or when an error occurs downloading XML text from a server. This 
    // handler is invoked before the XML is parsed, and you can use it to call 
    // a custom parsing routine instead of using the Flash XML parser. The src 
    // parameter is a string that contains XML text downloaded from the server, 
    // unless an error occurs during the download, in which case the src parameter 
    // is undefined. By default, the XML.onData event handler invokes XML.onLoad. 
    // You can override the XML.onData event handler with custom behavior, but 
    // XML.onLoad is not called unless you call it in your implementation of 
    // XML.onData.

    penv->Push(val);
    AsBroadcaster::BroadcastMessage(penv, this, 
        penv->CreateConstString("onData"), 1, penv->GetTopIndex());
    penv->Drop1();
}

// 
// Invoked when Flash Player receives an HTTP status code from the server
//
void XmlObject::NotifyOnHTTPStatus(Environment* penv, Number httpStatus)
{
    // Flash Doc: Invoked when Flash Player receives an HTTP status code from the 
    // server. This handler lets you capture and act on HTTP status codes. The 
    // onHTTPStatus handler is invoked before onData, which triggers calls to onLoad 
    // with a value of undefined if the load fails. It's important to note that 
    // after onHTTPStatus is triggered, onData is always subsequently triggered, 
    // regardless of whether or not you override onHTTPStatus. To best use the 
    // onHTTPStatus handler, write an appropriate function to catch the result of 
    // the onHTTPStatus call; you can then use the result in your onData or onLoad 
    // handler functions. If onHTTPStatus is not invoked, this indicates that the 
    // player did not try to make the URL request. This can happen because the request 
    // violates security sandbox rules for the SWF.

    // Note: This has no effect in GFx

    penv->Push(httpStatus);
    AsBroadcaster::BroadcastMessage(penv, this, 
        penv->CreateConstString("onHTTPStatus"), 1, penv->GetTopIndex());
    penv->Drop1();    
}

// 
// Invoked when a XML.load() or XML.sendAndLoad() operation has ended
//
void XmlObject::NotifyOnLoad(Environment* penv, bool success)
{
    // Flash Doc: Invoked by Flash Player when an XML document is received from the 
    // server. If the XML document is received successfully, the success parameter is 
    // true. If the document was not received, or if an error occurred in receiving 
    // the response from the server, the success parameter is false. The default, 
    // implementation of this method is not active. To override the default implementation, 
    // you must assign a function that contains custom actions.

    SetMemberRaw(penv->GetSC(), penv->GetSC()->CreateConstString("loaded"), 
        Value(success), PropFlags::PropFlag_DontDelete);

    penv->Push(success);
    AsBroadcaster::BroadcastMessage(penv, this, 
        penv->CreateConstString("onLoad"), 1, penv->GetTopIndex());
    penv->Drop1();    
}


// 
// AS to GFx function mapping 
//
const NameFunction XmlProto::FunctionTable[] = 
{
    { "addRequestHeader", &XmlProto::AddRequestHeader },
    { "createElement", &XmlProto::CreateElement },
    { "createTextNode", &XmlProto::CreateTextNode },
    { "getBytesLoaded", &XmlProto::GetBytesLoaded },
    { "getBytesTotal", &XmlProto::GetBytesTotal },
    { "load", &XmlProto::Load },
    { "parseXML", &XmlProto::ParseXML },
    { "send", &XmlProto::Send },
    { "sendAndLoad", &XmlProto::SendAndLoad },
    { 0, 0 }
};


// 
// AS XML class prototype initialization
//
XmlProto::XmlProto(ASStringContext *psc, Object* prototype, 
                         const FunctionRef& constructor)
    : Prototype<XmlObject>(psc, prototype, constructor)
{
    // we make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    // Flash Doc: The MIME content type that is sent to the server when you call 
    // the XML.send() or XML.sendAndLoad() method. The default is 
    // application/x-www-form-urlencoded, which is the standard MIME content type 
    // used for most HTML forms.
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("contentType"), 
        psc->CreateConstString("application/x-www-form-urlencoded"), 
        PropFlags::PropFlag_DontDelete);

    // Flash Doc: Specifies information about the XML document's DOCTYPE declaration. 
    // After the XML text has been parsed into an XML object, the XML.docTypeDecl property 
    // of the XML object is set to the text of the XML document's DOCTYPE declaration 
    // (for example, <!DOCTYPEgreeting SYSTEM "hello.dtd">). This property is set using a 
    // string representation of the DOCTYPE declaration, not an XML node object. 
    // If no DOCTYPE declaration was encountered during a parse operation, the XML.docTypeDecl 
    // property is set to undefined. The XML.toString() method outputs the contents of 
    // XML.docTypeDecl immediately after the XML declaration stored in XML.xmlDecl, and 
    // before any other text in the XML object. If XML.docTypeDecl is undefined, no DOCTYPE 
    // declaration is output.
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("docTypeDecl"), Value::UNDEFINED, 
        PropFlags::PropFlag_DontDelete);

    // Flash Doc: An object containing the XML file's nodes that have an id attribute assigned. 
    // The names of the properties of the object (each containing a node) match the values of 
    // the id attributes. You must use the parse() method on the XML object for the idMap property 
    // to be instantiated. If there is more than one XMLNode with the same id value, the matching 
    // property of the idNode object is that of the last node parsed.
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("idMap"), Value::UNDEFINED, 
        PropFlags::PropFlag_DontDelete);

    // Flash Doc: Default setting is false. When set to true, text nodes that contain only white 
    // space are discarded during the parsing process. Text nodes with leading or trailing white 
    // space are unaffected. 
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("ignoreWhite"), Value(false), 
        PropFlags::PropFlag_DontDelete);  

    // Flash Doc: The property that indicates whether the XML document has successfully 
    // loaded. If there is no custom onLoad() event handler defined for the XML object, 
    // Flash Player sets this property to true when the document-loading process initiated 
    // by the XML.load() call has completed successfully; otherwise, it is false. However, 
    // if you define a custom behavior for the onLoad() event handler for the XML object, 
    // you must be sure to set onload in that function.
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("loaded"), Value::UNDEFINED, 
        PropFlags::PropFlag_DontDelete);

    // Flash Doc: Automatically sets and returns a numeric value that indicates whether an 
    // XML document was successfully parsed into an XML object. The following are the numeric 
    // status codes, with descriptions: 
    // 0 No error; parse was completed successfully. 
    // -2 A CDATA section was not properly terminated. 
    // -3 The XML declaration was not properly terminated. 
    // -4 The DOCTYPE declaration was not properly terminated. 
    // -5 A comment was not properly terminated. 
    // -6 An XML element was malformed. 
    // -7 Out of memory. 
    // -8 An attribute value was not properly terminated. 
    // -9 A start-tag was not matched with an end-tag. 
    // -10 An end-tag was encountered without a matching start-tag. 
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("status"), Value(0), 
        PropFlags::PropFlag_DontDelete);
    
    // Flash Doc: A string that specifies information about a document's XML declaration. After 
    // the XML document is parsed into an XML object, this property is set to the text of the 
    // document's XML declaration. This property is set using a string representation of the XML 
    // declaration, not an XML node object. If no XML declaration is encountered during a parse 
    // operation, the property is set to undefined. The XML.toString() method outputs the contents 
    // of the XML.xmlDecl property before any other text in the XML object. If the XML.xmlDecl 
    // property contains the undefined type, no XML declaration is output.
    //
    XmlObject::SetMemberRaw(psc, psc->CreateConstString("xmlDecl"), Value::UNDEFINED, 
        PropFlags::PropFlag_DontDelete);

    
    // Setup the default onData handler. The function is empty and the load method will 
    // never use this default handler. It exists for posterity.
    SetConstMemberRaw(psc, "onData", Value(psc, &XmlProto::DefaultOnData), 
        PropFlags::PropFlag_DontEnum);
}


// 
// Default XML.onData method
//
void XmlProto::DefaultOnData(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Invoked when XML text has been completely downloaded from the 
    // server, or when an error occurs downloading XML text from a server. This 
    // handler is invoked before the XML is parsed, and you can use it to call a 
    // custom parsing routine instead of using the Flash XML parser. The src 
    // parameter is a string that contains XML text downloaded from the server, 
    // unless an error occurs during the download, in which case the src parameter 
    // is undefined. By default, the XML.onData event handler invokes XML.onLoad. 
    // You can override the XML.onData event handler with custom behavior, but 
    // XML.onLoad is not called unless you call it in your implementation of 
    // XML.onData.
    //

    // Since we bypass the default handler completely, this function is empty.
    // We do this to avoid the overhead incurred with copy the file contents
    // to a GASString and processing the string on the render thread.
    // See the C++ XML::load method for details.
}


// 
// XML.addRequestHeader(header:Object, headerValue:String) : Void
//
void XmlProto::AddRequestHeader(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Adds or changes HTTP request headers (such as Content-Type or 
    // SOAPAction) sent with POST actions. In the first usage, you pass two 
    // strings to the method: header and headerValue. In the second usage, you 
    // pass an array of strings, alternating header names and header values. 

    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "XML.addRequestHeader is not implemented.");
}


// 
// XML.createElement(name:String) : XMLNode
//
void XmlProto::CreateElement(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Creates a new XML element with the name specified in the parameter. 
    // The new element initially has no parent, no children, and no siblings. The 
    // method returns a reference to the newly created XML object that represents the 
    // element. This method and the XML.createTextNode() method are the constructor 
    // methods for creating nodes for an XML object.

    fn.Env->Push(fn.Arg(0));
    fn.Env->Push(1);
    ASString classname = fn.Env->CreateConstString("XMLNode");
    Ptr<Object> asobj = *fn.Env->OperatorNew(classname, 2);
    fn.Result->SetAsObject(asobj);
    fn.Env->Drop2();
}


// 
// XML.createTextNode(value:String) : XMLNode
//
void XmlProto::CreateTextNode(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Creates a new XML text node with the specified text. The new node 
    // initially has no parent, and text nodes cannot have children or siblings. This 
    // method returns a reference to the XML object that represents the new text node. 
    // This method and the XML.createElement() method are the constructor methods for 
    // creating nodes for an XML object.

    fn.Env->Push(fn.Arg(0));
    fn.Env->Push(3);
    ASString classname = fn.Env->CreateConstString("XMLNode");
    Ptr<Object> asobj = *fn.Env->OperatorNew(classname, 2);
    fn.Result->SetAsObject(asobj);
    fn.Env->Drop2();
}


// 
// XML.getBytesLoaded() : Number
//
void XmlProto::GetBytesLoaded(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Returns the number of bytes loaded (streamed) for the XML document. 
    // You can compare the value of getBytesLoaded() with the value of getBytesTotal() 
    // to determine what percentage of an XML document has loaded.

    if (pthis->BytesLoadedCurrent < 0)
    {
        fn.Result->SetUndefined();
    }
    else
    {
        fn.Result->SetNumber(pthis->BytesLoadedCurrent);
    }
}


// 
// XML.getBytesTotal() : Number
//
void XmlProto::GetBytesTotal(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Returns the size, in bytes, of the XML document.    

    if (pthis->BytesLoadedTotal < 0)
    {
        fn.Result->SetUndefined();
    }
    else
    {
        fn.Result->SetNumber(pthis->BytesLoadedTotal);
    }
}


// 
// XML.load(url:String) : Boolean
//
void XmlProto::Load(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Loads an XML document from the specified URL, and replaces the 
    // contents of the specified XML object with the downloaded XML data. The URL 
    // is relative and is called using HTTP. The load process is asynchronous; it 
    // does not finish immediately after the load() method is executed. When the 
    // load() method is executed, the XML object property loaded is set to false. 
    // When the XML data finishes downloading, the loaded property is set to true, 
    // and the onLoad event handler is invoked. The XML data is not parsed until 
    // it is completely downloaded. If the XML object previously contained any XML 
    // trees, they are discarded. You can define a custom function that executes 
    // when the onLoad event handler of the XML object is invoked.

    if (fn.NArgs == 0)
    {
        fn.Result->SetBool(false);
        return;
    }
    ASString urlStr(fn.Arg(0).ToString(fn.Env));

    // We will be bypassing the default onData implementation because it incurs
    // a significant memory and performance overhead. If a custom onData handler
    // has not been set, the XML file is loaded and parsed on a background thread
    // if threaded loading support is enabled. Else only the file will be loaded
    // on a background thread and the custom onData handler will be called using
    // the loaded string (file).
    //

    // Check if default onData handler
    Value onDataHandler;
    pthis->GetMember(fn.Env, fn.Env->CreateConstString("onData"), &onDataHandler);
    FunctionRef odf = onDataHandler.ToFunction(fn.Env);  
    
    if (odf.IsCFunction() && (odf == XmlProto::DefaultOnData))
    {
        // Default handler
        MovieImpl* pmovieroot = fn.Env->GetMovieImpl();
        Ptr<XML::ObjectManager> memMgr;
        if (!pmovieroot->pXMLObjectManager)
        {
            memMgr = *SF_HEAP_NEW(fn.Env->GetHeap()) XML::ObjectManager(pmovieroot);
            pmovieroot->pXMLObjectManager = memMgr;
        }
        else
            memMgr = (XML::ObjectManager*)pmovieroot->pXMLObjectManager;
        XML::SupportBase* pparser = pmovieroot->GetXMLSupport();

        pthis->BytesLoadedCurrent = 0;

        Value ignorews;
        pthis->GetMember(fn.Env, fn.Env->CreateConstString("ignoreWhite"), &ignorews);
        bool bws = ignorews.ToBool(fn.Env);
        Ptr<XMLFileLoader> pxmlLoader = 
            *new XMLFileLoaderAndParserImpl(pparser, memMgr, bws);
        fn.Env->GetAS2Root()->AddXmlLoadQueueEntry(pthis, pxmlLoader, urlStr.ToCStr(), 
            LoadQueueEntry::LM_None);
        fn.Result->SetBool(true);
    }
    else
    {
        // Custom handler
        // MA: XML_FileLoaderImpl must use global heap since referenced from tasks.
        Ptr<XMLFileLoader> pxmlLoader = *new XMLFileLoaderImpl();
        fn.Env->GetAS2Root()->AddXmlLoadQueueEntry(pthis, pxmlLoader, urlStr.ToCStr(), 
                                                     LoadQueueEntry::LM_None);
        fn.Result->SetBool(true);
    }

    pthis->SetMemberRaw(fn.Env->GetSC(), fn.Env->GetSC()->CreateConstString("loaded"), 
        Value(false), PropFlags::PropFlag_DontDelete);
}


// 
// XML.parseXML(value:String) : Void
//

void XmlProto::ParseXML(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Parses the XML text specified in the value parameter, and 
    // populates the specified XML object with the resulting XML tree. Any 
    // existing trees in the XML object are discarded.

    XML_LoadString(fn, pthis);

    // Create the idMap object
    Environment* penv = fn.Env;
    Ptr<Object> idMapObj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(penv);
    SF_ASSERT(pthis->pRealNode);
    XML::Document* pdoc = static_cast<XML::Document*>(pthis->pRealNode);
    for (XML::Node* child = pdoc->FirstChild; child != NULL; 
            child = child->NextSibling)
    {
        if (child->Type == XML::ElementNodeType)
            Xml_CreateIDMap(penv, (XML::ElementNode*)child, pthis->pRootNode, idMapObj);
    }
    pthis->SetMember(penv, penv->CreateConstString("idMap"), Value(idMapObj), 
        PropFlags::PropFlag_DontDelete); 
}


// 
// XML.send(url:String, [target:String], [method:String]) : Boolean
//
void XmlProto::Send(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Encodes the specified XML object into an XML document and sends 
    // it to the specified target URL. 

    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "XML.send is not implemented.");
}


// 
// XML.sendAndLoad(url:String, resultXML:XML)
//
void XmlProto::SendAndLoad(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, XML);
    XmlObject* pthis = (XmlObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Flash Doc: Encodes the specified XML object into an XML document, sends it
    // to the specified URL using the POST method, downloads the server's response, 
    // and loads it into the resultXMLobject specified in the parameters. The server 
    // response loads in the same manner used by the XML.load() method. 

    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "XML.sendAndLoad is not implemented.");
}



// 
// Constructor func ctor
//
XmlCtorFunction::XmlCtorFunction(ASStringContext *psc) 
    : CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}


// 
// Called when the constructor is invoked for the XML class (new XML(...))
//
// XML(test:String)
void XmlCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<XmlObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_XML)
        pnode = static_cast<XmlObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) XmlObject(fn.Env);

    XML_LoadString(fn, pnode);
}

Object* XmlCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) XmlObject(penv);
}

FunctionRef XmlCtorFunction::Register(GlobalContext* pgc)
{
    // Register XMLNode here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_XMLNode)))
        XmlNodeCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) XmlCtorFunction(&sc));
    Ptr<Object> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) XmlProto(&sc, pgc->GetPrototype(ASBuiltin_XMLNode), ctor);
    pgc->SetPrototype(ASBuiltin_XML, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_XML), Value(ctor));

    return ctor;
}


}}} // namespace SF::GFx::AS2

#endif // #ifdef GFX_ENABLE_XML
