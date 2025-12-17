/**************************************************************************

PublicHeader:   GFx
Filename    :   XML_Support.h
Content     :   GFx XML support
Created     :   March 7, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_XMLParserState_H
#define INC_SF_GFX_XMLParserState_H

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "Kernel/SF_Debug.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform { namespace GFx { namespace XML {

class ParserHandler;
class Parser;
class ParserExpat;


// 
// The pluggable XML parser
//
// This interface is implemented by pluggable XML parser instances.
// 
//
class Parser : public RefCountBaseNTS<Parser, Stat_Default_Mem>
{
public:    
    virtual ~Parser() {}

    //
    // Parse methods
    //
    virtual bool    ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                              ParserHandler* pphandler) = 0;
    virtual bool    ParseString(const char* pdata, UPInt len, 
                                ParserHandler* pphandler) = 0;
};


//
// XML support state
//
// This interface is used to define a pluggable XML parser as a Loader
// state. When an instance is registered as a loader state, its methods
// will be called appropriately for loading XML files and strings from
// ActionScript. It can also be used by application developers to parse
// XML files from C++.
//
class SupportBase : public State
{
public:
    SupportBase() : State(State_XMLSupport) {}
    virtual         ~SupportBase() {}

    // Load file given the provided file opener and DOM builder.
    virtual bool        ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                                  ParserHandler* pdb) = 0;

    // Load a string using the provided DOM builder
    virtual bool        ParseString(const char* pdata, UPInt len, 
                                    ParserHandler* pdb) = 0;
};

//
// XML support state implementation
//
// A separate implementation is provided so that XML can be compiled
// out completely from GFx core and the resultant application executable
//
class Support : public SupportBase
{
    Ptr<Parser>  pParser;

public:
    Support(Ptr<Parser> pparser) : pParser(pparser) 
    {
        SF_DEBUG_WARNING(pparser.GetPtr() == NULL, "GFx::XML::Support Ctor - GFx::XML::Parser argument is NULL!");
    }
    ~Support() {}

    bool        ParseFile(const char* pfilename, FileOpenerBase* pfo, 
                          ParserHandler* pdb);

    bool        ParseString(const char* pdata, UPInt len, 
                            ParserHandler* pdb);
};


} //XML

//
// Implementation of Set/GetXMLParserState members of the GFxStateBag class.
// They are defined here because the XMLParserState interface is declared
// here.
//
inline void StateBag::SetXMLSupport(XML::SupportBase *ptr)         
{ 
    SetState(State::State_XMLSupport, ptr); 
}
inline Ptr<XML::SupportBase>  StateBag::GetXMLSupport() const     
{ 
    return *(XML::SupportBase*) GetStateAddRef(State::State_XMLSupport); 
}

}} //SF::GFx

#endif  // GFX_ENABLE_XML

#endif  // INC_SF_GFX_XMLParserState_H
