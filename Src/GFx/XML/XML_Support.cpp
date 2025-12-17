/**************************************************************************

Filename    :   XML_Support.cpp
Content     :   GFx XML support
Created     :   March 7, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "XML_Support.h"

namespace Scaleform {
namespace GFx {
namespace XML {

//
// Load file given the provided file opener and DOM builder.
//
bool    Support::ParseFile(const char* pfilename, 
                                 FileOpenerBase* pfo, 
                                 ParserHandler* pdb)
{
    SF_ASSERT(pParser.GetPtr());
    return pParser->ParseFile(pfilename, pfo, pdb);
}

//
// Load a string using the provided DOM builder
//
bool    Support::ParseString(const char* pdata, 
                                   UPInt len, 
                                   ParserHandler* pdb)
{
    SF_ASSERT(pParser.GetPtr());
    return pParser->ParseString(pdata, len, pdb);
}

}}} //SF::GFx::XML

#endif  // SF_NO_XML_SUPPORT
