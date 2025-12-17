/**************************************************************************

Filename    :   AS2_DropShadowFilter.h
Content     :   DropShadowFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GASDropShadowFilter_H
#define INC_GASDropShadowFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BitmapFilter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class DropShadowFilterObject;
class DropShadowFilterProto;
class DropShadowFilterCtorFunction;


// 
// Actionscript DropShadowFilter object declaration
//

class DropShadowFilterObject : public BitmapFilterObject
{
    friend class DropShadowFilterProto;
    friend class DropShadowFilterCtorFunction;

protected:

//     DropShadowFilterObject();
//     DropShadowFilterObject(const DropShadowFilterObject& obj) : BitmapFilterObject(obj) {}
    DropShadowFilterObject(ASStringContext *psc, Object* pprototype)
        : BitmapFilterObject(psc, pprototype)
    { 
    }

    virtual             ~DropShadowFilterObject();

public:

    DropShadowFilterObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_DropShadowFilter; }

    virtual bool        SetMember(Environment* penv, 
        const ASString& name, const Value& val, 
        const PropFlags& flags = PropFlags());

    virtual bool        GetMember(Environment* penv, 
        const ASString& name, Value* val);
};


class DropShadowFilterProto : public Prototype<DropShadowFilterObject>
{
public:
    DropShadowFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default BitmapFilter object functions
    //
    static void         Clone(const FnCall& fn);
};


class DropShadowFilterCtorFunction : public CFunctionObject
{
public:
    DropShadowFilterCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};


}}} //SF::GFx::AS2
#endif  // SF_NO_FXPLAYER_AS_FILTERS

#endif  // INC_GASDropShadowFilter_H
