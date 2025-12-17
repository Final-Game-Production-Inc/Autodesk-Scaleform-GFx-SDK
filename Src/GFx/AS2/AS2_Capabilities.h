/**************************************************************************

Filename    :   AS2_Capabilities.h
Content     :   System.capabilities reference class for ActionScript 2.0
Created     :   3/27/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_CAPABILITIES_H
#define INC_SF_GFX_CAPABILITIES_H

#include "GFxConfig.h"
#include "GFx/AS2/AS2_Object.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_AS2_ENABLE_CAPABILITIES

// ****************************************************************************
// GAS Capabilities Constructor function class
//
class CapabilitiesCtorFunction : public CFunctionObject
{
public:
    CapabilitiesCtorFunction (ASStringContext *psc);

    virtual bool SetMemberRaw(ASStringContext *psc, const ASString& name, 
        const Value& val, const PropFlags& flags = PropFlags())
    {
        // nothing to set
        SF_UNUSED4(psc,name,val,flags);
        return false;
    }
    
    virtual bool GetMember(Environment *penv, const ASString& name, 
        Value* val);

    virtual Object* CreateNewObject(Environment*) const { return 0; }

    static FunctionRef Register(GlobalContext* pgc);
};
#endif //#ifdef GFX_AS2_ENABLE_CAPABILITIES
}}} //SF::GFx::AS2
#endif  // INC_SF_GFX_CAPABILITIES_H
